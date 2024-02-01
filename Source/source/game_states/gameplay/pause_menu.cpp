/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pause menu classes and functions.
 */

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


namespace PAUSE_MENU {
//Path to the leaving confirmation page GUI information file.
const string CONFIRMATION_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_confirmation.txt";
//Path to the GUI information file.
const string GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_menu.txt";
//Path to the help page GUI information file.
const string HELP_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_help.txt";
//Path to the mission page GUI information file.
const string MISSION_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_mission.txt";
//Background color of the radar.
const ALLEGRO_COLOR RADAR_BG_COLOR =
    al_map_rgb(32, 24, 0);
//Path to the radar page GUI information file.
const string RADAR_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_radar.txt";
//Color of the highest sector in the radar.
const ALLEGRO_COLOR RADAR_HIGHEST_COLOR =
    al_map_rgb(200, 200, 180);
//Color of the lowest sector in the radar.
const ALLEGRO_COLOR RADAR_LOWEST_COLOR =
    al_map_rgb(80, 64, 0);
//Maximum radar zoom level.
const float RADAR_MAX_ZOOM = 4.0f;
//Minimum radar zoom level.
const float RADAR_MIN_ZOOM = 0.03f;
}


/* ----------------------------------------------------------------------------
 * Creates a pause menu struct.
 */
pause_menu_struct::pause_menu_struct() :
    bg_alpha_mult(0.0f),
    closing_timer(0.0f),
    to_delete(false),
    closing(false),
    help_category_text(nullptr),
    help_tidbit_list(nullptr),
    confirmation_explanation_text(nullptr),
    cur_tidbit(nullptr),
    leave_target(LEAVE_TO_AREA_SELECT),
    lowest_sector_z(0.0f),
    highest_sector_z(0.0f),
    radar_mouse_down(false) {
    
    init_main_pause_menu();
    init_radar_page();
    init_help_page();
    init_mission_page();
    init_confirmation_page();
    
    //Initialize some radar things.
    bool found_valid_sector = false;
    lowest_sector_z = FLT_MAX;
    highest_sector_z = -FLT_MAX;
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(s_ptr->type == SECTOR_TYPE_BLOCKING) continue;
        lowest_sector_z = std::min(lowest_sector_z, s_ptr->z);
        highest_sector_z = std::max(highest_sector_z, s_ptr->z);
        found_valid_sector = true;
    }
    
    if(!found_valid_sector) {
        lowest_sector_z = -32.0f;
        highest_sector_z = 32.0f;
    }
    
    bool found_valid_edge = false;
    radar_min_coords = point(FLT_MAX, FLT_MAX);
    radar_max_coords = point(-FLT_MAX, -FLT_MAX);
    
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) continue;
        if(
            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING &&
            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
        ) {
            continue;
        }
        radar_min_coords.x =
            std::min(radar_min_coords.x, e_ptr->vertexes[0]->x);
        radar_min_coords.x =
            std::min(radar_min_coords.x, e_ptr->vertexes[1]->x);
        radar_max_coords.x =
            std::max(radar_max_coords.x, e_ptr->vertexes[0]->x);
        radar_max_coords.x =
            std::max(radar_max_coords.x, e_ptr->vertexes[1]->x);
        radar_min_coords.y =
            std::min(radar_min_coords.x, e_ptr->vertexes[0]->y);
        radar_min_coords.y =
            std::min(radar_min_coords.x, e_ptr->vertexes[1]->y);
        radar_max_coords.y =
            std::max(radar_max_coords.x, e_ptr->vertexes[0]->y);
        radar_max_coords.y =
            std::max(radar_max_coords.x, e_ptr->vertexes[1]->y);
        found_valid_edge = true;
    }
    
    if(!found_valid_edge) {
        radar_min_coords = point();
        radar_max_coords = point();
    }
    radar_min_coords = radar_min_coords + 64.0f;
    radar_max_coords = radar_max_coords + 64.0f;
    
    if(game.states.gameplay->cur_leader_ptr) {
        radar_cam.set_pos(game.states.gameplay->cur_leader_ptr->pos);
    }
    radar_cam.set_zoom(0.4f);
}


/* ----------------------------------------------------------------------------
 * Destroys a pause menu struct.
 */
pause_menu_struct::~pause_menu_struct() {
    for(size_t c = 0; c < N_HELP_CATEGORIES; ++c) {
        if(c == HELP_CATEGORY_PIKMIN) continue;
        for(size_t t = 0; t < tidbits.size(); ++t) {
            if(tidbits[(HELP_CATEGORIES) c][t].image) {
                game.bitmaps.detach(tidbits[(HELP_CATEGORIES) c][t].image);
            }
        }
    }
    tidbits.clear();
    
    gui.destroy();
    radar_gui.destroy();
    help_gui.destroy();
    mission_gui.destroy();
    confirmation_gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Adds a new bullet point to either the fail condition list, or the
 * grading explanation list.
 * list:
 *   List to add to.
 * text:
 *   Text.
 * color:
 *   Text color.
 */
void pause_menu_struct::add_bullet(
    list_gui_item* list, const string &text,
    const ALLEGRO_COLOR &color
) {
    size_t bullet_idx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bullet_center_y =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bullet_idx);
        
    bullet_point_gui_item* bullet =
        new bullet_point_gui_item(
        text, game.fonts.standard, color
    );
    bullet->center = point(0.50f, bullet_center_y);
    bullet->size = point(0.96f, BULLET_HEIGHT);
    list->add_child(bullet);
    mission_gui.add_item(bullet);
}


/* ----------------------------------------------------------------------------
 * Either asks the player to confirm if they wish to leave, or leaves outright,
 * based on the player's confirmation question preferences.
 */
void pause_menu_struct::confirm_or_leave() {
    bool do_confirmation = false;
    switch(game.options.leaving_confirmation_mode) {
    case LEAVING_CONFIRMATION_NEVER: {
        do_confirmation = false;
        break;
    } case LEAVING_CONFIRMATION_1_MIN: {
        do_confirmation =
            game.states.gameplay->gameplay_time_passed >= 60.0f;
        break;
    } case LEAVING_CONFIRMATION_ALWAYS: {
        do_confirmation = true;
        break;
    } case N_LEAVING_CONFIRMATION_MODES: {
        break;
    }
    }
    
    if(do_confirmation) {
        switch(leave_target) {
        case LEAVE_TO_RETRY: {
            confirmation_explanation_text->text =
                "If you retry, you will LOSE all of your progress "
                "and start over. Are you sure you want to retry?";
            break;
        } case LEAVE_TO_END: {
            confirmation_explanation_text->text =
                "If you end now, you will stop playing and will go to the "
                "results menu.";
            if(game.cur_area_data.type == AREA_TYPE_MISSION) {
                if(
                    game.cur_area_data.mission.goal ==
                    MISSION_GOAL_END_MANUALLY
                ) {
                    confirmation_explanation_text->text +=
                        " The goal of this mission is to end through here, so "
                        "make sure you've done everything you need first.";
                } else {
                    confirmation_explanation_text->text +=
                        " This will end the mission as a fail, "
                        "even though you may still get a medal from it.";
                    if(
                        game.cur_area_data.mission.grading_mode ==
                        MISSION_GRADING_POINTS
                    ) {
                        confirmation_explanation_text->text +=
                            " Note that since you fail the mission, you may "
                            "lose out on some points. You should check the "
                            "pause menu's mission page for more information.";
                    }
                    
                }
            }
            confirmation_explanation_text->text +=
                " Are you sure you want to end?";
            break;
        } case LEAVE_TO_AREA_SELECT: {
            confirmation_explanation_text->text =
                "If you quit, you will LOSE all of your progress and instantly "
                "stop playing. Are you sure you want to quit?";
            break;
        }
        }
        
        gui.responsive = false;
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        confirmation_gui.responsive = true;
        confirmation_gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        
    } else {
        start_leaving_gameplay();
    }
}


/* ----------------------------------------------------------------------------
 * Creates a button meant for changing to a page either to the left or to the
 * right of the current one.
 * text:
 *   Text to display on the button, sans arrow.
 * tooltip_name:
 *   Name of the page for displaying in the tooltip.
 * left:
 *   True if this page is to the left of the current, false if to the right.
 * cur_gui:
 *   Pointer to the current page's GUI manager.
 * new_gui:
 *   Pointer to the new page's GUI manager.
 */
button_gui_item* pause_menu_struct::create_page_button(
    const string &text, const string &tooltip_name,
    bool left,
    gui_manager* cur_gui, gui_manager* new_gui
) {
    button_gui_item* new_button =
        new button_gui_item(
        left ?
        "< " + text :
        text + " >",
        game.fonts.standard
    );
    new_button->on_activate =
    [left, cur_gui, new_gui] (const point &) {
        cur_gui->responsive = false;
        cur_gui->start_animation(
            left ?
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT :
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        new_gui->responsive = true;
        new_gui->start_animation(
            left ?
            GUI_MANAGER_ANIM_LEFT_TO_CENTER :
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    new_button->on_get_tooltip =
    [tooltip_name] () {
        return "Go to the pause menu's " + tooltip_name + " page.";
    };
    
    return new_button;
}


/* ----------------------------------------------------------------------------
 * Draws the pause menu.
 */
void pause_menu_struct::draw() {
    gui.draw();
    radar_gui.draw();
    help_gui.draw();
    mission_gui.draw();
    confirmation_gui.draw();
}


/* ----------------------------------------------------------------------------
 * Draws the radar itself.
 * center:
 *   Center coordinates of the radar on-screen.
 * size:
 *   Width and height of the radar on-screen.
 */
void pause_menu_struct::draw_radar(
    const point &center, const point &size
) {
    //Setup.
    ALLEGRO_TRANSFORM old_transform;
    int old_cr_x = 0;
    int old_cr_y = 0;
    int old_cr_w = 0;
    int old_cr_h = 0;
    al_copy_transform(&old_transform, al_get_current_transform());
    
    world_to_radar_screen_transform = game.identity_transform;
    al_translate_transform(
        &world_to_radar_screen_transform,
        -radar_cam.pos.x + center.x / radar_cam.zoom,
        -radar_cam.pos.y + center.y / radar_cam.zoom
    );
    al_scale_transform(
        &world_to_radar_screen_transform, radar_cam.zoom, radar_cam.zoom
    );
    
    radar_screen_to_world_transform = world_to_radar_screen_transform;
    al_invert_transform(&radar_screen_to_world_transform);
    
    al_get_clipping_rectangle(&old_cr_x, &old_cr_y, &old_cr_w, &old_cr_h);
    
    al_use_transform(&world_to_radar_screen_transform);
    al_set_clipping_rectangle(
        center.x - size.x / 2.0f,
        center.y - size.y / 2.0f,
        size.x,
        size.y
    );
    
    //Background fill.
    al_clear_to_color(PAUSE_MENU::RADAR_BG_COLOR);
    
    //Draw each sector.
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        
        if(s_ptr->type == SECTOR_TYPE_BLOCKING) continue;
        ALLEGRO_COLOR color =
            interpolate_color(
                s_ptr->z, lowest_sector_z, highest_sector_z,
                PAUSE_MENU::RADAR_LOWEST_COLOR,
                PAUSE_MENU::RADAR_HIGHEST_COLOR
            );
            
        for(size_t h = 0; h < s_ptr->hazards.size(); ++h) {
            if(!s_ptr->hazards[h]->associated_liquid) continue;
            color =
                interpolate_color(
                    0.80f, 0.0f, 1.0f,
                    color, s_ptr->hazards[h]->associated_liquid->radar_color
                );
        }
        
        for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
            ALLEGRO_VERTEX av[3];
            for(size_t v = 0; v < 3; ++v) {
                av[v].u = 0;
                av[v].v = 0;
                av[v].x = s_ptr->triangles[t].points[v]->x;
                av[v].y = s_ptr->triangles[t].points[v]->y;
                av[v].z = 0;
                av[v].color = color;
            }
            
            al_draw_prim(
                av, NULL, NULL,
                0, 3, ALLEGRO_PRIM_TRIANGLE_LIST
            );
        }
    }
    
    //Draw each edge.
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        
        if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) {
            //The other side is already the void, so no need for an edge.
            continue;
        }
        
        if(
            fabs(e_ptr->sectors[0]->z - e_ptr->sectors[1]->z) <=
            GEOMETRY::STEP_HEIGHT
        ) {
            //Step.
            continue;
        }
        
        al_draw_line(
            e_ptr->vertexes[0]->x,
            e_ptr->vertexes[0]->y,
            e_ptr->vertexes[1]->x,
            e_ptr->vertexes[1]->y,
            PAUSE_MENU::RADAR_BG_COLOR,
            1.5f / radar_cam.zoom
        );
    }
    
    //Onion icons.
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); ++o) {
        onion* o_ptr = game.states.gameplay->mobs.onions[o];
        
        al_draw_filled_circle(
            o_ptr->pos.x, o_ptr->pos.y, 10.0f / radar_cam.zoom,
            al_map_rgb(200, 200, 32)
        );
    }
    
    //Ship icons.
    for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); ++s) {
        ship* s_ptr = game.states.gameplay->mobs.ships[s];
        
        al_draw_filled_circle(
            s_ptr->pos.x, s_ptr->pos.y, 10.0f / radar_cam.zoom,
            al_map_rgb(32, 200, 200)
        );
    }
    
    //Enemy icons.
    for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); ++e) {
        enemy* e_ptr = game.states.gameplay->mobs.enemies[e];
        
        al_draw_filled_circle(
            e_ptr->pos.x, e_ptr->pos.y, 10.0f / radar_cam.zoom,
            al_map_rgb(32, 200, 32)
        );
    }
    
    //Leader icons.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l_ptr = game.states.gameplay->mobs.leaders[l];
        
        al_draw_circle(
            l_ptr->pos.x, l_ptr->pos.y,
            24.0f / radar_cam.zoom,
            COLOR_WHITE, 2.0f / radar_cam.zoom
        );
        draw_bitmap(
            l_ptr->lea_type->bmp_icon, l_ptr->pos,
            point(48.0f / radar_cam.zoom, 48.0f / radar_cam.zoom)
        );
    }
    
    //Treasure icons.
    //TODO piles of nuggets?
    for(size_t t = 0; t < game.states.gameplay->mobs.treasures.size(); ++t) {
        treasure* t_ptr = game.states.gameplay->mobs.treasures[t];
        
        draw_filled_diamond(
            t_ptr->pos, 24.0f / radar_cam.zoom,
            COLOR_GOLD
        );
    }
    
    //Pikmin icons.
    for(size_t p = 0; p < game.states.gameplay->mobs.pikmin_list.size(); ++p) {
        pikmin* p_ptr = game.states.gameplay->mobs.pikmin_list[p];
        
        al_draw_filled_circle(
            p_ptr->pos.x, p_ptr->pos.y,
            8.0f / radar_cam.zoom,
            p_ptr->pik_type->main_color
        );
    }
    
    //Return to normal drawing.
    al_use_transform(&old_transform);
    al_set_clipping_rectangle(old_cr_x, old_cr_y, old_cr_w, old_cr_h);
    
    //North indicator.
    point north_ind_center(
        center.x - size.x / 2.0f + 20.0f,
        center.y - size.y / 2.0f + 20.0f
    );
    al_draw_filled_circle(
        north_ind_center.x, north_ind_center.y,
        12.0f, PAUSE_MENU::RADAR_BG_COLOR
    );
    draw_compressed_text(
        game.fonts.slim,
        PAUSE_MENU::RADAR_HIGHEST_COLOR,
        point(
            north_ind_center.x,
            north_ind_center.y + 4.0f
        ),
        ALLEGRO_ALIGN_CENTER,
        TEXT_VALIGN_CENTER,
        point(12.0f, 12.0f),
        "N"
    );
    al_draw_filled_triangle(
        north_ind_center.x,
        north_ind_center.y - 12.0f,
        north_ind_center.x - 6.0f,
        north_ind_center.y - 6.0f,
        north_ind_center.x + 6.0f,
        north_ind_center.y - 6.0f,
        PAUSE_MENU::RADAR_HIGHEST_COLOR
    );
    
    //Area name.
    point area_name_size(
        size.x * 0.40f,
        20.0f
    );
    point area_name_center(
        center.x + size.x / 2.0f - area_name_size.x / 2.0f - 8.0f,
        center.y - size.y / 2.0f + area_name_size.y / 2.0f + 8.0f
    );
    draw_filled_rounded_rectangle(
        area_name_center, area_name_size,
        12.0f, PAUSE_MENU::RADAR_BG_COLOR
    );
    draw_compressed_text(
        game.fonts.standard, PAUSE_MENU::RADAR_HIGHEST_COLOR,
        area_name_center, ALLEGRO_ALIGN_CENTER,
        TEXT_VALIGN_CENTER, area_name_size,
        game.cur_area_data.name
    );
    
    //Draw some scan lines.
    float scan_line_y = center.y - size.y / 2.0f;
    while(scan_line_y < center.y + size.y / 2.0f) {
        al_draw_line(
            center.x - size.x / 2.0f,
            scan_line_y,
            center.x + size.x / 2.0f,
            scan_line_y,
            al_map_rgba(255, 255, 255, 8),
            2.0f
        );
        scan_line_y += 16.0f;
    }
    float scan_line_x = center.x - size.x / 2.0f;
    while(scan_line_x < center.x + size.x / 2.0f) {
        al_draw_line(
            scan_line_x,
            center.y - size.y / 2.0f,
            scan_line_x,
            center.y + size.y / 2.0f,
            al_map_rgba(255, 255, 255, 8),
            2.0f
        );
        scan_line_x += 16.0f;
    }
    
    //Draw a rectangle all around.
    draw_rounded_rectangle(
        center, size,
        8.0f,
        COLOR_TRANSPARENT_WHITE, 3.0f
    );
}


/* ----------------------------------------------------------------------------
 * Draws some help page tidbit's text.
 * font:
 *   Font to use.
 * where:
 *   Coordinates to draw the text on.
 * max_size:
 *   Maximum width or height the text can occupy. A value of zero in
 *   one of these coordinates makes it not have a limit in that dimension.
 * text:
 *   Text to draw.
 */
void pause_menu_struct::draw_tidbit(
    const ALLEGRO_FONT* const font, const point &where,
    const point &max_size, const string &text
) {
    //Get the tokens that make up the tidbit.
    vector<string_token> tokens = tokenize_string(text);
    if(tokens.empty()) return;
    
    int line_height = al_get_font_line_height(font);
    
    set_string_token_widths(tokens, font, game.fonts.slim, line_height, true);
    
    //Split long lines.
    vector<vector<string_token> > tokens_per_line =
        split_long_string_with_tokens(tokens, max_size.x);
        
    if(tokens_per_line.empty()) return;
    
    //Figure out if we need to scale things vertically.
    //Control bind icons that are bitmaps will have their width unchanged,
    //otherwise this would turn into a cat-and-mouse game of the Y scale
    //shrinking causing a token width to shrink, which could cause the
    //Y scale to grow, ad infinitum.
    float y_scale = 1.0f;
    if(tokens_per_line.size() * line_height > max_size.y) {
        y_scale = max_size.y / (tokens_per_line.size() * (line_height + 4));
    }
    
    //Draw!
    for(size_t l = 0; l < tokens_per_line.size(); ++l) {
        draw_string_tokens(
            tokens_per_line[l], game.fonts.standard, game.fonts.slim,
            true,
            point(
                where.x,
                where.y + l * (line_height + 4) * y_scale -
                (tokens_per_line.size() * line_height * y_scale / 2.0f)
            ),
            ALLEGRO_ALIGN_CENTER, point(max_size.x, line_height * y_scale)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Fills the list of mission fail conditions.
 * list:
 *   List item to fill.
 */
void pause_menu_struct::fill_mission_fail_list(list_gui_item* list) {
    for(size_t f = 0; f < game.mission_fail_conds.size(); ++f) {
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_index_bitmask(f)
            )
        ) {
            mission_fail* cond = game.mission_fail_conds[f];
            
            string description =
                cond->get_player_description(&game.cur_area_data.mission);
            add_bullet(list, description, al_map_rgb(255, 200, 200));
            
            float percentage = 0.0f;
            int cur =
                cond->get_cur_amount(game.states.gameplay);
            int req =
                cond->get_req_amount(game.states.gameplay);
            if(req != 0.0f) {
                percentage = cur / (float) req;
            }
            percentage *= 100;
            string status = cond->get_status(cur, req, percentage);
            
            if(status.empty()) continue;
            add_bullet(list, "    " + status);
        }
    }
    
    if(game.cur_area_data.mission.fail_conditions == 0) {
        add_bullet(list, "(None)");
    }
}


/* ----------------------------------------------------------------------------
 * Fills the list of mission grading information.
 * list:
 *   List item to fill.
 */
void pause_menu_struct::fill_mission_grading_list(list_gui_item* list) {
    switch(game.cur_area_data.mission.grading_mode) {
    case MISSION_GRADING_POINTS: {
        add_bullet(
            list,
            "Your medal depends on your score:"
        );
        add_bullet(
            list,
            "    Platinum: " +
            i2s(game.cur_area_data.mission.platinum_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Gold: " +
            i2s(game.cur_area_data.mission.gold_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Silver: " +
            i2s(game.cur_area_data.mission.silver_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Bronze: " +
            i2s(game.cur_area_data.mission.bronze_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        
        vector<string> score_notes;
        for(size_t c = 0; c < game.mission_score_criteria.size(); ++c) {
            mission_score_criterion* c_ptr =
                game.mission_score_criteria[c];
            int mult = c_ptr->get_multiplier(&game.cur_area_data.mission);
            if(mult != 0) {
                score_notes.push_back(
                    "    " + c_ptr->get_name() + " x " + i2s(mult) + "."
                );
            }
        }
        if(!score_notes.empty()) {
            add_bullet(
                list,
                "Your score is calculated like so:"
            );
            for(size_t s = 0; s < score_notes.size(); ++s) {
                add_bullet(list, score_notes[s]);
            }
        } else {
            add_bullet(
                list,
                "In this mission, your score will always be 0."
            );
        }
        
        vector<string> loss_notes;
        for(size_t c = 0; c < game.mission_score_criteria.size(); ++c) {
            mission_score_criterion* c_ptr =
                game.mission_score_criteria[c];
            if(
                has_flag(
                    game.cur_area_data.mission.point_loss_data,
                    get_index_bitmask(c)
                )
            ) {
                loss_notes.push_back("    " + c_ptr->get_name());
            }
        }
        if(!loss_notes.empty()) {
            add_bullet(
                list,
                "If you fail, you'll lose your score for:"
            );
            for(size_t l = 0; l < loss_notes.size(); ++l) {
                add_bullet(list, loss_notes[l]);
            }
        }
        break;
    } case MISSION_GRADING_GOAL: {
        add_bullet(
            list,
            "You get a platinum medal if you clear the goal."
        );
        add_bullet(
            list,
            "You get no medal if you fail."
        );
        break;
    } case MISSION_GRADING_PARTICIPATION: {
        add_bullet(
            list,
            "You get a platinum medal just by playing the mission."
        );
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Returns a string representing the player's status towards the mission goal.
 */
string pause_menu_struct::get_mission_goal_status() {
    float percentage = 0.0f;
    int cur =
        game.mission_goals[game.cur_area_data.mission.goal]->
        get_cur_amount(game.states.gameplay);
    int req =
        game.mission_goals[game.cur_area_data.mission.goal]->
        get_req_amount(game.states.gameplay);
    if(req != 0.0f) {
        percentage = cur / (float) req;
    }
    percentage *= 100;
    return
        game.mission_goals[game.cur_area_data.mission.goal]->
        get_status(cur, req, percentage);
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void pause_menu_struct::handle_event(const ALLEGRO_EVENT &ev) {
    gui.handle_event(ev);
    radar_gui.handle_event(ev);
    help_gui.handle_event(ev);
    mission_gui.handle_event(ev);
    confirmation_gui.handle_event(ev);
    
    //Handle some radar logic.
    point radar_center;
    point radar_size;
    radar_gui.get_item_draw_info(radar_item, &radar_center, &radar_size);
    bool mouse_in_radar =
        is_point_in_rectangle(
            game.mouse_cursor.s_pos,
            radar_center, radar_size
        );
        
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(radar_gui.responsive && mouse_in_radar) {
            radar_mouse_down = true;
        }
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        radar_mouse_down = false;
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(radar_mouse_down && (ev.mouse.dx != 0.0f || ev.mouse.dy != 0.0f)) {
            pan_radar(point(-ev.mouse.dx, -ev.mouse.dy));
        } else if(mouse_in_radar && ev.mouse.dz != 0.0f) {
            zoom_radar(ev.mouse.dz * 0.1f);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a player action.
 * action:
 *   Data about the player action.
 */
void pause_menu_struct::handle_player_action(const player_action &action) {
    gui.handle_player_action(action);
    radar_gui.handle_player_action(action);
    help_gui.handle_player_action(action);
    mission_gui.handle_player_action(action);
    confirmation_gui.handle_player_action(action);
}


/* ----------------------------------------------------------------------------
 * Initializes the leaving confirmation page.
 */
void pause_menu_struct::init_confirmation_page() {
    data_node gui_file(PAUSE_MENU::CONFIRMATION_GUI_FILE_PATH);
    
    //Menu items.
    confirmation_gui.register_coords("cancel",           19, 83, 30, 10);
    confirmation_gui.register_coords("confirm",          81, 83, 30, 10);
    confirmation_gui.register_coords("header",           50,  8, 92,  8);
    confirmation_gui.register_coords("explanation",      50, 40, 84, 20);
    confirmation_gui.register_coords("options_reminder", 50, 69, 92, 10);
    confirmation_gui.register_coords("tooltip",          50, 96, 96,  4);
    confirmation_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Cancel button.
    confirmation_gui.back_item =
        new button_gui_item(
        "Cancel", game.fonts.standard
    );
    confirmation_gui.back_item->on_activate =
    [this] (const point &) {
        confirmation_gui.responsive = false;
        confirmation_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    confirmation_gui.back_item->on_get_tooltip =
    [] () { return "Return to the pause menu."; };
    confirmation_gui.add_item(confirmation_gui.back_item, "cancel");
    
    //Confirm button.
    button_gui_item* confirm_button =
        new button_gui_item("Confirm", game.fonts.standard);
    confirm_button->on_activate =
    [this] (const point &) {
        start_leaving_gameplay();
    };
    confirm_button->on_get_tooltip =
    [] () {
        return "Yes, I'm sure.";
    };
    confirmation_gui.add_item(confirm_button, "confirm");
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item("Are you sure?", game.fonts.area_name);
    confirmation_gui.add_item(header_text, "header");
    
    //Explanation text.
    confirmation_explanation_text =
        new text_gui_item("", game.fonts.standard);
    confirmation_explanation_text->line_wrap = true;
    confirmation_gui.add_item(confirmation_explanation_text, "explanation");
    
    //Options reminder text.
    text_gui_item* options_reminder_text =
        new text_gui_item(
        "You can disable this confirmation question in the options menu.",
        game.fonts.standard
    );
    confirmation_gui.add_item(options_reminder_text, "options_reminder");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_tidbit(
            game.fonts.standard, center, size,
            confirmation_gui.get_current_tooltip()
        );
    };
    confirmation_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    confirmation_gui.set_selected_item(confirmation_gui.back_item);
    confirmation_gui.responsive = false;
    confirmation_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the help page.
 */
void pause_menu_struct::init_help_page() {
    const vector<string> category_node_names {
        "gameplay_basics", "advanced_gameplay", "controls", "", "objects"
    };
    data_node gui_file(PAUSE_MENU::HELP_GUI_FILE_PATH);
    
    //Load the tidbits.
    data_node* tidbits_node = gui_file.get_child_by_name("tidbits");
    
    for(size_t c = 0; c < N_HELP_CATEGORIES; ++c) {
        if(category_node_names[c].empty()) continue;
        data_node* category_node =
            tidbits_node->get_child_by_name(category_node_names[c]);
        size_t n_tidbits = category_node->get_nr_of_children();
        vector<tidbit> &category_tidbits = tidbits[(HELP_CATEGORIES) c];
        category_tidbits.reserve(n_tidbits);
        for(size_t t = 0; t < n_tidbits; ++t) {
            vector<string> parts =
                split(category_node->get_child(t)->name, ";");
            tidbit new_t;
            new_t.name = parts.size() > 0 ? parts[0] : "";
            new_t.description = parts.size() > 1 ? parts[1] : "";
            new_t.image = parts.size() > 2 ? game.bitmaps.get(parts[2]) : NULL;
            category_tidbits.push_back(new_t);
        }
    }
    for(size_t p = 0; p < game.config.pikmin_order.size(); ++p) {
        tidbit new_t;
        new_t.name = game.config.pikmin_order[p]->name;
        new_t.description = game.config.pikmin_order[p]->description;
        new_t.image = game.config.pikmin_order[p]->bmp_icon;
        tidbits[HELP_CATEGORY_PIKMIN].push_back(new_t);
    }
    
    //Menu items.
    help_gui.register_coords("back",        12,  5, 20,  6);
    help_gui.register_coords("gameplay1",   22, 15, 36,  6);
    help_gui.register_coords("gameplay2",   22, 23, 36,  6);
    help_gui.register_coords("controls",    22, 31, 36,  6);
    help_gui.register_coords("pikmin",      22, 39, 36,  6);
    help_gui.register_coords("objects",     22, 47, 36,  6);
    help_gui.register_coords("manual",      22, 54, 36,  4);
    help_gui.register_coords("category",    71,  5, 54,  6);
    help_gui.register_coords("list",        69, 39, 50, 54);
    help_gui.register_coords("list_scroll", 96, 39,  2, 54);
    help_gui.register_coords("image",       16, 83, 28, 30);
    help_gui.register_coords("tooltip",     65, 83, 66, 30);
    help_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Back button.
    help_gui.back_item =
        new button_gui_item(
        "Back", game.fonts.standard
    );
    help_gui.back_item->on_activate =
    [this] (const point &) {
        help_gui.responsive = false;
        help_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    help_gui.back_item->on_get_tooltip =
    [] () { return "Return to the pause menu."; };
    help_gui.add_item(help_gui.back_item, "back");
    
    //Gameplay basics button.
    button_gui_item* gameplay1_button =
        new button_gui_item("Gameplay basics", game.fonts.standard);
    gameplay1_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_GAMEPLAY1);
    };
    gameplay1_button->on_get_tooltip =
    [] () {
        return "Show help about basic gameplay features.";
    };
    help_gui.add_item(gameplay1_button, "gameplay1");
    
    //Gameplay advanced button.
    button_gui_item* gameplay2_button =
        new button_gui_item("Advanced gameplay", game.fonts.standard);
    gameplay2_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_GAMEPLAY2);
    };
    gameplay2_button->on_get_tooltip =
    [] () {
        return "Show advanced gameplay tips.";
    };
    help_gui.add_item(gameplay2_button, "gameplay2");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.fonts.standard);
    controls_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_CONTROLS);
    };
    controls_button->on_get_tooltip =
    [] () {
        return "Show game controls and certain actions you can perform.";
    };
    help_gui.add_item(controls_button, "controls");
    
    //Pikmin button.
    button_gui_item* pikmin_button =
        new button_gui_item("Pikmin types", game.fonts.standard);
    pikmin_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_PIKMIN);
    };
    pikmin_button->on_get_tooltip =
    [] () {
        return "Show a description of each Pikmin type.";
    };
    help_gui.add_item(pikmin_button, "pikmin");
    
    //Objects button.
    button_gui_item* objects_button =
        new button_gui_item("Objects", game.fonts.standard);
    objects_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_OBJECTS);
    };
    objects_button->on_get_tooltip =
    [] () {
        return "Show help about some noteworthy objects you'll find.";
    };
    help_gui.add_item(objects_button, "objects");
    
    //Manual text.
    bullet_point_gui_item* manual_bullet =
        new bullet_point_gui_item("More help...", game.fonts.standard);
    manual_bullet->on_get_tooltip = [] () {
        return
            "For more help on other subjects, check out the "
            "manual in the game's folder.";
    };
    help_gui.add_item(manual_bullet, "manual");
    
    //Category text.
    help_category_text = new text_gui_item("", game.fonts.standard);
    help_gui.add_item(help_category_text, "category");
    
    //Tidbit list box.
    help_tidbit_list = new list_gui_item();
    help_gui.add_item(help_tidbit_list, "list");
    
    //Tidbit list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = help_tidbit_list;
    help_gui.add_item(list_scroll, "list_scroll");
    
    //Image item.
    gui_item* image_item = new gui_item();
    image_item->on_draw =
    [this] (const point & center, const point & size) {
        if(cur_tidbit == NULL) return;
        if(cur_tidbit->image == NULL) return;
        draw_bitmap_in_box(
            cur_tidbit->image,
            center, size, false
        );
    };
    help_gui.add_item(image_item, "image");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_tidbit(
            game.fonts.standard, center, size,
            help_gui.get_current_tooltip()
        );
    };
    help_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    help_gui.set_selected_item(help_gui.back_item);
    help_gui.responsive = false;
    help_gui.hide_items();
    help_gui.on_selection_changed =
    [this] () {
        cur_tidbit = NULL;
    };
}


/* ----------------------------------------------------------------------------
 * Initializes the pause menu's main menu.
 */
void pause_menu_struct::init_main_pause_menu() {
    //Menu items.
    gui.register_coords("header",        50,  5, 52,  6);
    gui.register_coords("left_page",     12,  5, 20,  6);
    gui.register_coords("right_page",    88,  5, 20,  6);
    gui.register_coords("line",          50, 11, 96,  2);
    gui.register_coords("area_name",     50, 20, 96,  8);
    gui.register_coords("area_subtitle", 50, 27, 88,  6);
    gui.register_coords("continue",      13, 88, 22,  8);
    gui.register_coords("retry",         50, 41, 52, 10);
    gui.register_coords("end",           50, 53, 52, 10);
    gui.register_coords("help",          50, 65, 52, 10);
    gui.register_coords("quit",          87, 88, 22,  8);
    gui.register_coords("tooltip",       50, 96, 96,  4);
    gui.read_coords(
        data_node(PAUSE_MENU::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "PAUSED", game.fonts.area_name,
        COLOR_TRANSPARENT_WHITE
    );
    gui.add_item(header_text, "header");
    
    //Left page button.
    button_gui_item* left_page_button;
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        left_page_button =
            create_page_button(
                "Mission", "mission",
                true,
                &gui, &mission_gui
            );
    } else {
        left_page_button =
            create_page_button(
                "Radar", "radar",
                true,
                &gui, &radar_gui
            );
    }
    gui.add_item(left_page_button, "left_page");
    
    //Right page button.
    button_gui_item* right_page_button =
        create_page_button(
            "Radar", "radar",
            false,
            &gui, &radar_gui
        );
    gui.add_item(right_page_button, "right_page");
    
    //Line.
    gui_item* line = new gui_item();
    line->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            point(size.x, 3.0f),
            2.0f,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.add_item(line, "line");
    
    //Area name.
    text_gui_item* area_name_text =
        new text_gui_item(
        game.cur_area_data.name, game.fonts.area_name,
        change_alpha(COLOR_GOLD, 192)
    );
    gui.add_item(area_name_text, "area_name");
    
    //Area subtitle.
    text_gui_item* area_subtitle_text =
        new text_gui_item(
        get_subtitle_or_mission_goal(
            game.cur_area_data.subtitle, game.cur_area_data.type,
            game.cur_area_data.mission.goal
        ),
        game.fonts.area_name,
        change_alpha(COLOR_WHITE, 192)
    );
    gui.add_item(area_subtitle_text, "area_subtitle");
    
    //Continue button.
    gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        start_closing();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    gui.add_item(gui.back_item, "continue");
    
    //Retry button.
    button_gui_item* retry_button =
        new button_gui_item(
        game.cur_area_data.type == AREA_TYPE_SIMPLE ?
        "Restart exploration" :
        "Retry mission",
        game.fonts.standard
    );
    retry_button->on_activate =
    [this] (const point &) {
        leave_target = LEAVE_TO_RETRY;
        confirm_or_leave();
    };
    retry_button->on_get_tooltip =
    [] () {
        return
            game.cur_area_data.type == AREA_TYPE_SIMPLE ?
            "Restart this area's exploration." :
            "Retry the mission from the start.";
    };
    gui.add_item(retry_button, "retry");
    
    //End button.
    button_gui_item* end_button =
        new button_gui_item(
        game.cur_area_data.type == AREA_TYPE_SIMPLE ?
        "End exploration" :
        "End mission",
        game.fonts.standard
    );
    end_button->on_activate =
    [this] (const point &) {
        leave_target = LEAVE_TO_END;
        confirm_or_leave();
    };
    end_button->on_get_tooltip =
    [] () {
        bool as_fail =
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_index_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
            );
        return
            game.cur_area_data.type == AREA_TYPE_SIMPLE ?
            "End this area's exploration." :
            as_fail ?
            "End this mission as a fail." :
            "End this mission successfully.";
    };
    gui.add_item(end_button, "end");
    
    //Help button.
    button_gui_item* help_button =
        new button_gui_item("Help", game.fonts.standard);
    help_button->on_activate =
    [this] (const point &) {
        gui.responsive = false;
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        help_gui.responsive = true;
        help_gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    help_button->on_get_tooltip =
    [] () { return "Some quick help and tips about how to play."; };
    gui.add_item(help_button, "help");
    
    //Quit button.
    button_gui_item* quit_button =
        new button_gui_item(
        game.states.area_ed->quick_play_area_path.empty() ?
        "Quit" :
        "Back to editor",
        game.fonts.standard
    );
    quit_button->on_activate =
    [this] (const point &) {
        leave_target = LEAVE_TO_AREA_SELECT;
        confirm_or_leave();
    };
    quit_button->on_get_tooltip =
    [] () {
        return
            "Lose your progress and return to the " +
            string(
                game.states.area_ed->quick_play_area_path.empty() ?
                "area selection menu" :
                "area editor"
            ) + ".";
    };
    gui.add_item(quit_button, "quit");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item);
    gui.start_animation(
        GUI_MANAGER_ANIM_UP_TO_CENTER, GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the mission page.
 */
void pause_menu_struct::init_mission_page() {
    data_node gui_file(PAUSE_MENU::MISSION_GUI_FILE_PATH);
    
    //Menu items.
    mission_gui.register_coords("header",         50,  5, 52,  6);
    mission_gui.register_coords("left_page",      12,  5, 20,  6);
    mission_gui.register_coords("right_page",     88,  5, 20,  6);
    mission_gui.register_coords("line",           50, 11, 96,  2);
    mission_gui.register_coords("continue",       10, 16, 16,  4);
    mission_gui.register_coords("goal_header",    50, 16, 60,  4);
    mission_gui.register_coords("goal",           50, 22, 96,  4);
    mission_gui.register_coords("goal_status",    50, 26, 96,  4);
    mission_gui.register_coords("fail_header",    50, 32, 96,  4);
    mission_gui.register_coords("fail_list",      48, 48, 92, 24);
    mission_gui.register_coords("fail_scroll",    97, 48,  2, 24);
    mission_gui.register_coords("grading_header", 50, 64, 96,  4);
    mission_gui.register_coords("grading_list",   48, 80, 92, 24);
    mission_gui.register_coords("grading_scroll", 97, 80,  2, 24);
    mission_gui.register_coords("tooltip",        50, 96, 96,  4);
    mission_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "MISSION", game.fonts.area_name,
        COLOR_TRANSPARENT_WHITE
    );
    mission_gui.add_item(header_text, "header");
    
    //Left page button.
    button_gui_item* left_page_button =
        create_page_button(
            "Radar", "radar",
            true,
            &mission_gui, &radar_gui
        );
    mission_gui.add_item(left_page_button, "left_page");
    
    //Right page button.
    button_gui_item* right_page_button =
        create_page_button(
            "System", "system",
            false,
            &mission_gui, &gui
        );
    mission_gui.add_item(right_page_button, "right_page");
    
    //Line.
    gui_item* line = new gui_item();
    line->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            point(size.x, 3.0f),
            2.0f,
            COLOR_TRANSPARENT_WHITE
        );
    };
    mission_gui.add_item(line, "line");
    
    //Continue button.
    mission_gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    mission_gui.back_item->on_activate =
    [this] (const point &) {
        mission_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        start_closing();
    };
    mission_gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    mission_gui.add_item(mission_gui.back_item, "continue");
    
    //Goal header text.
    text_gui_item* goal_header_text =
        new text_gui_item("Goal", game.fonts.area_name);
    mission_gui.add_item(goal_header_text, "goal_header");
    
    //Goal explanation text.
    text_gui_item* goal_text =
        new text_gui_item(
        game.mission_goals[game.cur_area_data.mission.goal]->
        get_player_description(&game.cur_area_data.mission),
        game.fonts.standard,
        al_map_rgb(255, 255, 200)
    );
    mission_gui.add_item(goal_text, "goal");
    
    //Goal status text.
    text_gui_item* goal_status_text =
        new text_gui_item(
        get_mission_goal_status(),
        game.fonts.standard
    );
    mission_gui.add_item(goal_status_text, "goal_status");
    
    //Fail conditions header text.
    text_gui_item* fail_header_text =
        new text_gui_item("Fail conditions", game.fonts.area_name);
    mission_gui.add_item(fail_header_text, "fail_header");
    
    //Fail condition explanation list.
    list_gui_item* mission_fail_list = new list_gui_item();
    mission_gui.add_item(mission_fail_list, "fail_list");
    fill_mission_fail_list(mission_fail_list);
    
    //Fail condition explanation scrollbar.
    scroll_gui_item* fail_scroll = new scroll_gui_item();
    fail_scroll->list_item = mission_fail_list;
    mission_gui.add_item(fail_scroll, "fail_scroll");
    
    //Grading header text.
    text_gui_item* grading_header_text =
        new text_gui_item("Grading", game.fonts.area_name);
    mission_gui.add_item(grading_header_text, "grading_header");
    
    //Grading explanation list.
    list_gui_item* mission_grading_list = new list_gui_item();
    mission_gui.add_item(mission_grading_list, "grading_list");
    fill_mission_grading_list(mission_grading_list);
    
    //Grading explanation scrollbar.
    scroll_gui_item* grading_scroll = new scroll_gui_item();
    grading_scroll->list_item = mission_grading_list;
    mission_gui.add_item(grading_scroll, "grading_scroll");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&mission_gui);
    mission_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    mission_gui.set_selected_item(mission_gui.back_item);
    mission_gui.responsive = false;
    mission_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the radar page.
 */
void pause_menu_struct::init_radar_page() {
    data_node gui_file(PAUSE_MENU::RADAR_GUI_FILE_PATH);
    
    //Menu items.
    radar_gui.register_coords("header",              50,     5,    52,    6);
    radar_gui.register_coords("left_page",           12,     5,    20,    6);
    radar_gui.register_coords("right_page",          88,     5,    20,    6);
    radar_gui.register_coords("line",                50,    11,    96,    2);
    radar_gui.register_coords("continue",            10,    16,    16,    4);
    radar_gui.register_coords("radar",               37.5,  56.25, 70,   72.5);
    radar_gui.register_coords("field_pikmin_label",  86.25, 27.5,  22.5,  5);
    radar_gui.register_coords("field_pikmin_number", 86.25, 35,    22.5,  5);
    radar_gui.register_coords("idle_pikmin_label",   86.25, 42.5,  22.5,  5);
    radar_gui.register_coords("idle_pikmin_number",  86.25, 50,    22.5,  5);
    radar_gui.register_coords("group_pikmin_label",  86.25, 57.5,  22.5,  5);
    radar_gui.register_coords("group_pikmin_number", 86.25, 65,    22.5,  5);
    radar_gui.register_coords("cursor_info",         86.25, 80,    22.5, 20);
    radar_gui.register_coords("instructions",        58.75, 16,    77.5,  4);
    radar_gui.register_coords("tooltip",             50,    96,    96,    4);
    radar_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "RADAR", game.fonts.area_name,
        COLOR_TRANSPARENT_WHITE
    );
    radar_gui.add_item(header_text, "header");
    
    //Left page button.
    button_gui_item* left_page_button =
        create_page_button(
            "System", "system",
            true,
            &radar_gui, &gui
        );
    radar_gui.add_item(left_page_button, "left_page");
    
    //Right page button.
    button_gui_item* right_page_button;
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        right_page_button =
            create_page_button(
                "Mission", "mission",
                false,
                &radar_gui, &mission_gui
            );
    } else {
        right_page_button =
            create_page_button(
                "System", "system",
                false,
                &radar_gui, &gui
            );
    }
    radar_gui.add_item(right_page_button, "right_page");
    
    //Line.
    gui_item* line = new gui_item();
    line->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            point(size.x, 3.0f),
            2.0f,
            COLOR_TRANSPARENT_WHITE
        );
    };
    radar_gui.add_item(line, "line");
    
    //Continue button.
    radar_gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    radar_gui.back_item->on_activate =
    [this] (const point &) {
        radar_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        start_closing();
    };
    radar_gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    radar_gui.add_item(radar_gui.back_item, "continue");
    
    //Radar item.
    radar_item = new gui_item();
    radar_item->on_draw =
    [this] (const point & center, const point & size) {
        draw_radar(center, size);
    };
    radar_gui.add_item(radar_item, "radar");
    
    //Field Pikmin label text.
    text_gui_item* field_pik_label_text =
        new text_gui_item(
        "Field Pikmin:", game.fonts.standard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radar_gui.add_item(field_pik_label_text, "field_pikmin_label");
    
    //Field Pikmin number text.
    text_gui_item* field_pik_nr_text =
        new text_gui_item(
        i2s(game.states.gameplay->mobs.pikmin_list.size()),
        game.fonts.counter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(field_pik_nr_text, "field_pikmin_number");
    
    //Idle Pikmin label text.
    text_gui_item* idle_pik_label_text =
        new text_gui_item(
        "Idle Pikmin:", game.fonts.standard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radar_gui.add_item(idle_pik_label_text, "idle_pikmin_label");
    
    //Idle Pikmin number text.
    size_t nr_idle_pikmin = 0;
    for(size_t p = 0; p < game.states.gameplay->mobs.pikmin_list.size(); ++p) {
        pikmin* pik_ptr = game.states.gameplay->mobs.pikmin_list[p];
        if(
            pik_ptr->fsm.cur_state->id == PIKMIN_STATE_IDLING ||
            pik_ptr->fsm.cur_state->id == PIKMIN_STATE_IDLING_H
        ) {
            nr_idle_pikmin++;
        }
    }
    text_gui_item* idle_pik_nr_text =
        new text_gui_item(
        i2s(nr_idle_pikmin), game.fonts.counter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(idle_pik_nr_text, "idle_pikmin_number");
    
    //Group Pikmin label text.
    text_gui_item* group_pik_label_text =
        new text_gui_item(
        "Group Pikmin:", game.fonts.standard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radar_gui.add_item(group_pik_label_text, "group_pikmin_label");
    
    //Group Pikmin number text.
    text_gui_item* group_pik_nr_text =
        new text_gui_item(
        i2s(game.states.gameplay->nr_group_pikmin), game.fonts.counter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(group_pik_nr_text, "group_pikmin_number");
    
    //Cursor info text.
    text_gui_item* cursor_info_text =
        new text_gui_item("", game.fonts.area_name); //TODO
    radar_gui.add_item(cursor_info_text, "cursor_info");
    
    //Instructions text.
    text_gui_item* instructions_text =
        new text_gui_item(
        "", game.fonts.slim,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    ); //TODO
    radar_gui.add_item(instructions_text, "instructions");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&radar_gui);
    radar_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    radar_gui.set_selected_item(radar_gui.back_item);
    radar_gui.responsive = false;
    radar_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Pans the radar by an amount.
 * amount:
 *   How much to pan by.
 */
void pause_menu_struct::pan_radar(point amount) {
    point delta = amount / radar_cam.zoom;
    radar_cam.pos += delta;
    radar_cam.pos.x =
        clamp(radar_cam.pos.x, radar_min_coords.x, radar_max_coords.x);
    radar_cam.pos.y =
        clamp(radar_cam.pos.y, radar_min_coords.y, radar_max_coords.y);
}


/* ----------------------------------------------------------------------------
 * Populates the help page's list of tidbits.
 * category:
 *   Category of tidbits to use.
 */
void pause_menu_struct::populate_help_tidbits(const HELP_CATEGORIES category) {
    vector<tidbit> &tidbit_list = tidbits[category];
    
    switch(category) {
    case HELP_CATEGORY_GAMEPLAY1: {
        help_category_text->text = "Gameplay basics";
        break;
    } case HELP_CATEGORY_GAMEPLAY2: {
        help_category_text->text = "Advanced gameplay";
        break;
    } case HELP_CATEGORY_CONTROLS: {
        help_category_text->text = "Controls";
        break;
    } case HELP_CATEGORY_PIKMIN: {
        help_category_text->text = "Pikmin";
        break;
    } case HELP_CATEGORY_OBJECTS: {
        help_category_text->text = "Objects";
        break;
    } case N_HELP_CATEGORIES: {
        break;
    }
    }
    
    help_tidbit_list->delete_all_children();
    
    for(size_t t = 0; t < tidbit_list.size(); ++t) {
        tidbit* t_ptr = &tidbit_list[t];
        bullet_point_gui_item* tidbit_bullet =
            new bullet_point_gui_item(
            t_ptr->name,
            game.fonts.standard
        );
        tidbit_bullet->center = point(0.50f, 0.045f + t * 0.10f);
        tidbit_bullet->size = point(1.0f, 0.09f);
        tidbit_bullet->on_get_tooltip = [this, t_ptr] () {
            return t_ptr->description;
        };
        tidbit_bullet->on_selected = [this, t_ptr] () {
            cur_tidbit = t_ptr;
        };
        tidbit_bullet->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
        );
        help_tidbit_list->add_child(tidbit_bullet);
        help_gui.add_item(tidbit_bullet);
    }
    
    help_category_text->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_HIGH
    );
}


/* ----------------------------------------------------------------------------
 * Starts the closing process.
 */
void pause_menu_struct::start_closing() {
    closing = true;
    closing_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/* ----------------------------------------------------------------------------
 * Starts the process of leaving the gameplay state.
 */
void pause_menu_struct::start_leaving_gameplay() {
    if(
        leave_target == LEAVE_TO_END &&
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            get_index_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        )
    ) {
        game.states.gameplay->mission_fail_reason =
            MISSION_FAIL_COND_PAUSE_MENU;
    }
    game.states.gameplay->start_leaving(leave_target);
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void pause_menu_struct::tick(const float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
    radar_gui.tick(delta_t);
    help_gui.tick(delta_t);
    mission_gui.tick(delta_t);
    confirmation_gui.tick(delta_t);
    
    //Tick the background.
    const float bg_alpha_mult_speed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bg_alpha_mult_speed : bg_alpha_mult_speed;
    bg_alpha_mult = clamp(bg_alpha_mult + diff * delta_t, 0.0f, 1.0f);
    
    //Tick the menu closing.
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Zooms the radar by an amount.
 * amount:
 *   How much to zoom by.
 */
void pause_menu_struct::zoom_radar(float amount) {
    float delta = amount * radar_cam.zoom;
    radar_cam.zoom += delta;
    radar_cam.zoom =
        clamp(
            radar_cam.zoom,
            PAUSE_MENU::RADAR_MIN_ZOOM, PAUSE_MENU::RADAR_MAX_ZOOM
        );
}
