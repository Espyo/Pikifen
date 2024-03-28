/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pause menu classes and functions.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


namespace PAUSE_MENU {

//Path to the leaving confirmation page GUI information file.
const string CONFIRMATION_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_confirmation.txt";
    
//Control lockout time after entering the menu.
const float ENTRY_LOCKOUT_TIME = 0.15f;

//Interval between calculations of the Go Here path.
const float GO_HERE_CALC_INTERVAL = 0.15f;

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
    
//Default radar zoom level.
const float RADAR_DEF_ZOOM = 0.4f;

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

//How long an Onion waits before fading to the next color.
const float RADAR_ONION_COLOR_FADE_CYCLE_DUR = 1.0f;

//How long an Onion fades between two colors.
const float RADAR_ONION_COLOR_FADE_DUR = 0.2f;

//Max radar pan speed when not using mouse, in pixels per second.
const float RADAR_PAN_SPEED = 600.0f;

//Max radar zoom speed when not using mouse, in amount per second.
const float RADAR_ZOOM_SPEED = 2.5f;

//Path to the status page GUI information file.
const string STATUS_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_status.txt";
    
}


/**
 * @brief Constructs a new pause menu struct object.
 *
 * @param start_on_radar True if the page to start on should be the radar,
 * false if it should be the system page.
 */
pause_menu_t::pause_menu_t(bool start_on_radar) {
    
    pages.push_back(PAUSE_MENU_PAGE_SYSTEM);
    pages.push_back(PAUSE_MENU_PAGE_RADAR);
    pages.push_back(PAUSE_MENU_PAGE_STATUS);
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        pages.push_back(PAUSE_MENU_PAGE_MISSION);
    }
    
    init_main_pause_menu();
    init_radar_page();
    init_status_page();
    init_mission_page();
    init_help_page();
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
    
    if(!found_valid_sector || lowest_sector_z == highest_sector_z) {
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
            std::min(radar_min_coords.y, e_ptr->vertexes[0]->y);
        radar_min_coords.y =
            std::min(radar_min_coords.y, e_ptr->vertexes[1]->y);
        radar_max_coords.y =
            std::max(radar_max_coords.y, e_ptr->vertexes[0]->y);
        radar_max_coords.y =
            std::max(radar_max_coords.y, e_ptr->vertexes[1]->y);
        found_valid_edge = true;
    }
    
    if(!found_valid_edge) {
        radar_min_coords = point();
        radar_max_coords = point();
    }
    radar_min_coords = radar_min_coords - 16.0f;
    radar_max_coords = radar_max_coords + 16.0f;
    
    radar_selected_leader = game.states.gameplay->cur_leader_ptr;
    
    if(radar_selected_leader) {
        radar_cam.set_pos(radar_selected_leader->pos);
    }
    radar_cam.set_zoom(game.states.gameplay->radar_zoom);
    
    //Start the process.
    opening_lockout_timer = PAUSE_MENU::ENTRY_LOCKOUT_TIME;
    gui_manager* first_gui = start_on_radar ? &radar_gui : &gui;
    first_gui->responsive = true;
    first_gui->start_animation(
        GUI_MANAGER_ANIM_UP_TO_CENTER, GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/**
 * @brief Destroys the pause menu struct object.
 */
pause_menu_t::~pause_menu_t() {
    for(size_t c = 0; c < N_HELP_CATEGORIES; ++c) {
        if(c == HELP_CATEGORY_PIKMIN) continue;
        for(size_t t = 0; t < tidbits.size(); ++t) {
            if(tidbits[(HELP_CATEGORY) c][t].image) {
                game.bitmaps.free(tidbits[(HELP_CATEGORY) c][t].image);
            }
        }
    }
    tidbits.clear();
    
    gui.destroy();
    radar_gui.destroy();
    status_gui.destroy();
    mission_gui.destroy();
    help_gui.destroy();
    confirmation_gui.destroy();
    
    game.bitmaps.free(bmp_radar_cursor);
    game.bitmaps.free(bmp_radar_pikmin);
    game.bitmaps.free(bmp_radar_treasure);
    game.bitmaps.free(bmp_radar_enemy);
    game.bitmaps.free(bmp_radar_leader_bubble);
    game.bitmaps.free(bmp_radar_onion_skeleton);
    game.bitmaps.free(bmp_radar_onion_bulb);
    game.bitmaps.free(bmp_radar_ship);
    game.bitmaps.free(bmp_radar_path);
    bmp_radar_cursor = nullptr;
    bmp_radar_pikmin = nullptr;
    bmp_radar_treasure = nullptr;
    bmp_radar_enemy = nullptr;
    bmp_radar_leader_bubble = nullptr;
    bmp_radar_onion_skeleton = nullptr;
    bmp_radar_onion_bulb = nullptr;
    bmp_radar_ship = nullptr;
    bmp_radar_path = nullptr;
}


/**
 * @brief Adds a new bullet point to either the fail condition list, or the
 * grading explanation list.
 *
 * @param list List to add to.
 * @param text Text.
 * @param color Text color.
 */
void pause_menu_t::add_bullet(
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


/**
 * @brief Adds a new line to one of the Pikmin status boxes.
 *
 * @param list List to add to.
 * @param pik_type Relevant Pikmin type, if applicable.
 * @param group_text Text to display on the "group" cell.
 * @param idle_text Text to display on the "idle" cell.
 * @param field_text Text to display on the "field" cell.
 * @param onion_text Text to display on the "Onion" cell.
 * @param total_text Text to display on the "total" cell.
 * @param new_text Text to display on the "new" cell.
 * @param lost_text Text to display on the "lost" cell.
 * @param is_single True if this is a box with a single row.
 * @param is_totals True if this is the totals box.
 */
void pause_menu_t::add_pikmin_status_line(
    list_gui_item* list,
    pikmin_type* pik_type,
    const string &group_text,
    const string &idle_text,
    const string &field_text,
    const string &onion_text,
    const string &total_text,
    const string &new_text,
    const string &lost_text,
    bool is_single, bool is_totals
) {

    const float x1 = 0.00f;
    const float x2 = 1.00f;
    const float working_width = x2 - x1;
    const float item_x_interval = working_width / 8.0f;
    const float first_x = x1 + item_x_interval / 2.0f;
    const float item_width = item_x_interval - 0.01f;
    
    const float y1 = is_single ? 0.0f : list->get_child_bottom();
    const float item_height = is_single ? 1.0f : 0.17f;
    const float item_y_spacing = is_single ? 0.0f : 0.03f;
    const float item_y = y1 + item_height / 2.0f + item_y_spacing;
    
    ALLEGRO_FONT* font =
        (is_single && !is_totals) ? game.fonts.standard : game.fonts.counter;
    string tooltip_start =
        pik_type ?
        "Number of " + pik_type->name + " " :
        "Total number of Pikmin ";
    bool can_select = pik_type || is_totals;
    
    if(pik_type) {
    
        //Pikmin type.
        gui_item* type_item = new gui_item();
        type_item->on_draw =
        [pik_type] (const point & center, const point & size) {
            draw_bitmap_in_box(
                pik_type->bmp_icon, center, size, true
            );
        };
        type_item->center =
            point(
                first_x + item_x_interval * 0,
                item_y
            );
        type_item->size = point(item_width, item_height);
        list->add_child(type_item);
        status_gui.add_item(type_item);
        
    } else if(is_totals) {
    
        //Totals header.
        text_gui_item* totals_header_item =
            new text_gui_item("Total", game.fonts.area_name);
        totals_header_item->center =
            point(
                first_x + item_x_interval * 0,
                item_y
            );
        totals_header_item->size = point(item_width, item_height);
        list->add_child(totals_header_item);
        status_gui.add_item(totals_header_item);
        
    }
    
    //Group Pikmin.
    text_gui_item* group_text_item =
        new text_gui_item(group_text, font);
    group_text_item->selectable = can_select;
    group_text_item->show_selection_box = can_select;
    group_text_item->center =
        point(
            first_x + item_x_interval * 1,
            item_y
        );
    group_text_item->size = point(item_width, item_height);
    if(can_select) {
        group_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "in your active leader's group.";
        };
    }
    if(group_text == "0") {
        group_text_item->color = change_alpha(group_text_item->color, 128);
    }
    list->add_child(group_text_item);
    status_gui.add_item(group_text_item);
    
    //Idle Pikmin.
    text_gui_item* idle_text_item =
        new text_gui_item(idle_text, font);
    idle_text_item->selectable = can_select;
    idle_text_item->show_selection_box = can_select;
    idle_text_item->center =
        point(
            first_x + item_x_interval * 2,
            item_y
        );
    idle_text_item->size = point(item_width, item_height);
    if(can_select) {
        idle_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "idling in the field.";
        };
    }
    if(idle_text == "0") {
        idle_text_item->color = change_alpha(idle_text_item->color, 128);
    }
    list->add_child(idle_text_item);
    status_gui.add_item(idle_text_item);
    
    //Field Pikmin.
    text_gui_item* field_text_item =
        new text_gui_item(field_text, font);
    field_text_item->selectable = can_select;
    field_text_item->show_selection_box = can_select;
    field_text_item->center =
        point(
            first_x + item_x_interval * 3,
            item_y
        );
    field_text_item->size = point(item_width, item_height);
    if(can_select) {
        field_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "out in the field.";
        };
    }
    if(field_text == "0") {
        field_text_item->color = change_alpha(field_text_item->color, 128);
    }
    list->add_child(field_text_item);
    status_gui.add_item(field_text_item);
    
    //Onion Pikmin.
    text_gui_item* onion_text_item =
        new text_gui_item(onion_text, font);
    onion_text_item->selectable = can_select;
    onion_text_item->show_selection_box = can_select;
    onion_text_item->center =
        point(
            first_x + item_x_interval * 4,
            item_y
        );
    onion_text_item->size = point(item_width, item_height);
    if(can_select) {
        onion_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "inside Onions.";
        };
    }
    if(onion_text == "0") {
        onion_text_item->color = change_alpha(onion_text_item->color, 128);
    }
    list->add_child(onion_text_item);
    status_gui.add_item(onion_text_item);
    
    //Total Pikmin.
    text_gui_item* total_text_item =
        new text_gui_item(total_text, font, COLOR_GOLD);
    total_text_item->selectable = can_select;
    total_text_item->show_selection_box = can_select;
    total_text_item->center =
        point(
            first_x + item_x_interval * 5,
            item_y
        );
    total_text_item->size = point(item_width, item_height);
    if(can_select) {
        total_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "you have.";
        };
    }
    if(total_text == "0") {
        total_text_item->color = change_alpha(total_text_item->color, 128);
    }
    list->add_child(total_text_item);
    status_gui.add_item(total_text_item);
    
    //New Pikmin.
    text_gui_item* new_text_item =
        new text_gui_item(new_text, font, al_map_rgb(210, 255, 210));
    new_text_item->selectable = can_select;
    new_text_item->show_selection_box = can_select;
    new_text_item->center =
        point(
            first_x + item_x_interval * 6,
            item_y
        );
    new_text_item->size = point(item_width, item_height);
    if(can_select) {
        new_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "born today.";
        };
    }
    if(new_text == "0") {
        new_text_item->color = change_alpha(new_text_item->color, 128);
    }
    list->add_child(new_text_item);
    status_gui.add_item(new_text_item);
    
    //Lost Pikmin.
    text_gui_item* lost_text_item =
        new text_gui_item(lost_text, font, al_map_rgb(255, 210, 210));
    lost_text_item->selectable = can_select;
    lost_text_item->show_selection_box = can_select;
    lost_text_item->center =
        point(
            first_x + item_x_interval * 7,
            item_y
        );
    lost_text_item->size = point(item_width, item_height);
    if(can_select) {
        lost_text_item->on_get_tooltip =
        [tooltip_start] () {
            return tooltip_start + "lost today.";
        };
    }
    if(lost_text == "0") {
        lost_text_item->color = change_alpha(lost_text_item->color, 128);
    }
    list->add_child(lost_text_item);
    status_gui.add_item(lost_text_item);
}


/**
 * @brief Calculates the Go Here path from the selected leader to the radar
 * cursor, if applicable, and stores the results in go_here_path and
 * go_here_path_result.
 */
void pause_menu_t::calculate_go_here_path() {
    radar_cursor_leader = nullptr;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l_ptr = game.states.gameplay->mobs.leaders[l];
        if(dist(l_ptr->pos, radar_cursor) <= 24.0f / radar_cam.zoom) {
            radar_cursor_leader = l_ptr;
            break;
        }
    }
    
    if(
        !radar_selected_leader ||
        radar_cursor_leader ||
        dist(radar_selected_leader->pos, radar_cursor) < 128.0f
    ) {
        go_here_path.clear();
        go_here_path_result = PATH_RESULT_ERROR;
        return;
    }
    
    if(!radar_selected_leader->fsm.get_event(LEADER_EV_GO_HERE)) {
        go_here_path.clear();
        go_here_path_result = PATH_RESULT_ERROR;
        return;
    }
    
    sector* cursor_sector = get_sector(radar_cursor, nullptr, true);
    
    if(!cursor_sector || cursor_sector->type == SECTOR_TYPE_BLOCKING) {
        go_here_path.clear();
        go_here_path_result = PATH_RESULT_ERROR;
        return;
    }
    
    path_follow_settings settings;
    settings.flags =
        PATH_FOLLOW_FLAG_CAN_CONTINUE | PATH_FOLLOW_FLAG_LIGHT_LOAD;
    settings.invulnerabilities =
        radar_selected_leader->group->get_group_invulnerabilities(
            radar_selected_leader
        );
        
    go_here_path_result =
        get_path(
            radar_selected_leader->pos,
            radar_cursor,
            settings,
            go_here_path, nullptr, nullptr, nullptr
        );
}


/**
 * @brief Either asks the player to confirm if they wish to leave, or leaves
 * outright, based on the player's confirmation question preferences.
 *
 */
void pause_menu_t::confirm_or_leave() {
    bool do_confirmation = false;
    switch(game.options.leaving_confirmation_mode) {
    case LEAVING_CONFIRMATION_MODE_NEVER: {
        do_confirmation = false;
        break;
    } case LEAVING_CONFIRMATION_MODE_1_MIN: {
        do_confirmation =
            game.states.gameplay->gameplay_time_passed >= 60.0f;
        break;
    } case LEAVING_CONFIRMATION_MODE_ALWAYS: {
        do_confirmation = true;
        break;
    } case N_LEAVING_CONFIRMATION_MODES: {
        break;
    }
    }
    
    if(do_confirmation) {
        switch(leave_target) {
        case GAMEPLAY_LEAVE_TARGET_RETRY: {
            confirmation_explanation_text->text =
                "If you retry, you will LOSE all of your progress "
                "and start over. Are you sure you want to retry?";
            break;
        } case GAMEPLAY_LEAVE_TARGET_END: {
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
                        MISSION_GRADING_MODE_POINTS
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
        } case GAMEPLAY_LEAVE_TARGET_AREA_SELECT: {
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


/**
 * @brief Creates a button meant for changing to a page either to the left or
 * to the right of the current one.
 *
 * @param target_page Which page this button leads to.
 * @param left True if this page is to the left of the current,
 * false if to the right.
 * @param cur_gui Pointer to the current page's GUI manager.
 * @return The button.
 */
button_gui_item* pause_menu_t::create_page_button(
    PAUSE_MENU_PAGE target_page, bool left,
    gui_manager* cur_gui
) {
    string page_name;
    string tooltip_name;
    switch(target_page) {
    case PAUSE_MENU_PAGE_SYSTEM: {
        page_name = "System";
        tooltip_name = "system";
        break;
    } case PAUSE_MENU_PAGE_RADAR: {
        page_name = "Radar";
        tooltip_name = "radar";
        break;
    } case PAUSE_MENU_PAGE_STATUS: {
        page_name = "Status";
        tooltip_name = "status";
        break;
    } case PAUSE_MENU_PAGE_MISSION: {
        page_name = "Mission";
        tooltip_name = "mission";
        break;
    }
    }
    
    button_gui_item* new_button =
        new button_gui_item(
        left ?
        "< " + page_name :
        page_name + " >",
        game.fonts.standard
    );
    new_button->on_activate =
    [this, cur_gui, target_page, left] (const point &) {
        switch_page(cur_gui, target_page, left);
    };
    new_button->on_get_tooltip =
    [tooltip_name] () {
        return "Go to the pause menu's " + tooltip_name + " page.";
    };
    
    return new_button;
}


/**
 * @brief Creates the buttons and input GUI items that allow switching pages.
 *
 * @param cur_page Page that these creations belong to.
 * @param cur_gui Pointer to the current page's GUI manager.
 */
void pause_menu_t::create_page_buttons(
    PAUSE_MENU_PAGE cur_page, gui_manager* cur_gui
) {
    size_t cur_page_idx =
        std::distance(
            pages.begin(),
            std::find(pages.begin(), pages.end(), cur_page)
        );
    size_t left_page_idx = sum_and_wrap(cur_page_idx, -1, pages.size());
    size_t right_page_idx = sum_and_wrap(cur_page_idx, 1, pages.size());
    
    //Left page button.
    button_gui_item* left_page_button =
        create_page_button(pages[left_page_idx], true, cur_gui);
    cur_gui->add_item(left_page_button, "left_page");
    
    //Left page input icon.
    gui_item* left_page_input = new gui_item();
    left_page_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        player_input i =
            game.controls.find_bind(PLAYER_ACTION_TYPE_MENU_PAGE_LEFT).input;
        if(i.type == INPUT_TYPE_NONE) return;
        draw_player_input_icon(game.fonts.slim, i, true, center, size);
    };
    cur_gui->add_item(left_page_input, "left_page_input");
    
    //Right page button.
    button_gui_item* right_page_button =
        create_page_button(pages[right_page_idx], false, cur_gui);
    cur_gui->add_item(right_page_button, "right_page");
    
    //Right page input icon.
    gui_item* right_page_input = new gui_item();
    right_page_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        player_input i =
            game.controls.find_bind(PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT).input;
        if(i.type == INPUT_TYPE_NONE) return;
        draw_player_input_icon(game.fonts.slim, i, true, center, size);
    };
    cur_gui->add_item(right_page_input, "right_page_input");
}


/**
 * @brief Draws the pause menu.
 */
void pause_menu_t::draw() {
    gui.draw();
    radar_gui.draw();
    status_gui.draw();
    mission_gui.draw();
    help_gui.draw();
    confirmation_gui.draw();
}


/**
 * @brief Draws a segment of the Go Here path.
 *
 * @param start Starting point.
 * @param end Ending point.
 * @param color Color of the segment.
 * @param texture_point Pointer to a variable keeping track of what point of
 * the texture we've drawn so far for this path, so that the effect is seamless
 * between segments.
 */
void pause_menu_t::draw_go_here_segment(
    const point &start, const point &end,
    const ALLEGRO_COLOR &color, float* texture_point
) {
    const float PATH_SEGMENT_THICKNESS = 12.0f / radar_cam.zoom;
    const float PATH_SEGMENT_TIME_MULT = 10.0f;
    
    ALLEGRO_VERTEX av[4];
    for(unsigned char a = 0; a < 4; ++a) {
        av[a].color = color;
        av[a].z = 0.0f;
    }
    int bmp_h = al_get_bitmap_height(bmp_radar_path);
    float texture_scale = bmp_h / PATH_SEGMENT_THICKNESS / radar_cam.zoom;
    float angle = get_angle(start, end);
    float distance = dist(start, end).to_float() * radar_cam.zoom;
    float texture_offset = game.time_passed * PATH_SEGMENT_TIME_MULT;
    float texture_start = *texture_point;
    float texture_end = texture_start + distance;
    point rot_offset = rotate_point(point(0, PATH_SEGMENT_THICKNESS), angle);
    
    av[0].x = start.x - rot_offset.x;
    av[0].y = start.y - rot_offset.y;
    av[1].x = start.x + rot_offset.x;
    av[1].y = start.y + rot_offset.y;
    av[2].x = end.x - rot_offset.x;
    av[2].y = end.y - rot_offset.y;
    av[3].x = end.x + rot_offset.x;
    av[3].y = end.y + rot_offset.y;
    
    av[0].u = (texture_start - texture_offset) * texture_scale;
    av[0].v = 0.0f;
    av[1].u = (texture_start - texture_offset) * texture_scale;
    av[1].v = bmp_h;
    av[2].u = (texture_end - texture_offset) * texture_scale;
    av[2].v = 0.0f;
    av[3].u = (texture_end - texture_offset) * texture_scale;
    av[3].v = bmp_h;
    
    al_draw_prim(
        av, nullptr, bmp_radar_path, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
    );
    
    *texture_point = texture_end;
}


/**
 * @brief Draws the radar itself.
 *
 * @param center Center coordinates of the radar on-screen.
 * @param size Width and height of the radar on-screen.
 */
void pause_menu_t::draw_radar(
    const point &center, const point &size
) {
    //Setup.
    ALLEGRO_TRANSFORM old_transform;
    int old_cr_x = 0;
    int old_cr_y = 0;
    int old_cr_w = 0;
    int old_cr_h = 0;
    al_copy_transform(&old_transform, al_get_current_transform());
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
                av, nullptr, nullptr,
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
        onion* o_ptr =
            game.states.gameplay->mobs.onions[o];
        vector<pikmin_type*>* pik_types_ptr =
            &o_ptr->nest->nest_type->pik_types;
            
        size_t nr_pik_types = pik_types_ptr->size();
        if(nr_pik_types > 0) {
            float fade_cycle_pos =
                std::min(
                    (float) fmod(
                        game.time_passed,
                        PAUSE_MENU::RADAR_ONION_COLOR_FADE_CYCLE_DUR
                    ),
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_DUR
                );
                
            size_t pik_type_idx_target =
                (int) (
                    game.time_passed /
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_CYCLE_DUR
                ) % nr_pik_types;
            size_t pik_type_idx_prev =
                (pik_type_idx_target + nr_pik_types - 1) % nr_pik_types;
                
            ALLEGRO_COLOR target_color =
                interpolate_color(
                    fade_cycle_pos, 0.0f,
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_DUR,
                    pik_types_ptr->at(pik_type_idx_prev)->main_color,
                    pik_types_ptr->at(pik_type_idx_target)->main_color
                );
                
            draw_bitmap(
                bmp_radar_onion_bulb, o_ptr->pos,
                point(24.0f / radar_cam.zoom, 24.0f / radar_cam.zoom),
                0.0f,
                target_color
            );
        }
        draw_bitmap(
            bmp_radar_onion_skeleton, o_ptr->pos,
            point(24.0f / radar_cam.zoom, 24.0f / radar_cam.zoom)
        );
    }
    
    //Ship icons.
    for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); ++s) {
        ship* s_ptr = game.states.gameplay->mobs.ships[s];
        
        draw_bitmap(
            bmp_radar_ship, s_ptr->pos,
            point(24.0f / radar_cam.zoom, 24.0f / radar_cam.zoom)
        );
    }
    
    //Enemy icons.
    for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); ++e) {
        enemy* e_ptr = game.states.gameplay->mobs.enemies[e];
        
        draw_bitmap(
            bmp_radar_enemy, e_ptr->pos,
            point(24.0f / radar_cam.zoom, 24.0f / radar_cam.zoom),
            game.time_passed
        );
    }
    
    //Leader icons.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l_ptr = game.states.gameplay->mobs.leaders[l];
        
        draw_bitmap(
            l_ptr->lea_type->bmp_icon, l_ptr->pos,
            point(40.0f / radar_cam.zoom, 40.0f / radar_cam.zoom)
        );
        draw_bitmap(
            bmp_radar_leader_bubble, l_ptr->pos,
            point(48.0f / radar_cam.zoom, 48.0f / radar_cam.zoom),
            0.0f,
            radar_selected_leader == l_ptr ?
            al_map_rgb(0, 255, 255) :
            COLOR_WHITE
        );
        draw_filled_equilateral_triangle(
            l_ptr->pos +
            rotate_point(point(25.0f / radar_cam.zoom, 0.0f), l_ptr->angle),
            6.0f / radar_cam.zoom,
            l_ptr->angle,
            radar_selected_leader == l_ptr ?
            al_map_rgb(0, 255, 255) :
            COLOR_WHITE
        );
    }
    
    //Treasure icons.
    //TODO piles of nuggets?
    for(size_t t = 0; t < game.states.gameplay->mobs.treasures.size(); ++t) {
        treasure* t_ptr = game.states.gameplay->mobs.treasures[t];
        
        draw_bitmap(
            bmp_radar_treasure, t_ptr->pos,
            point(32.0f / radar_cam.zoom, 32.0f / radar_cam.zoom),
            sin(game.time_passed * 2.0f) * (TAU * 0.05f)
        );
    }
    
    //Pikmin icons.
    for(size_t p = 0; p < game.states.gameplay->mobs.pikmin_list.size(); ++p) {
        pikmin* p_ptr = game.states.gameplay->mobs.pikmin_list[p];
        
        draw_bitmap(
            bmp_radar_pikmin, p_ptr->pos,
            point(16.0f / radar_cam.zoom, 16.0f / radar_cam.zoom),
            0.0f,
            p_ptr->pik_type->main_color
        );
    }
    
    //Currently-active Go Here paths.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l_ptr = game.states.gameplay->mobs.leaders[l];
        if(!l_ptr->mid_go_here) continue;
        
        float path_texture_point = 0.0f;
        ALLEGRO_COLOR color = al_map_rgba(120, 140, 170, 192);
        
        switch(l_ptr->path_info->result) {
        case PATH_RESULT_DIRECT:
        case PATH_RESULT_DIRECT_NO_STOPS: {
            //Go directly from A to B.
            
            draw_go_here_segment(
                l_ptr->pos,
                l_ptr->path_info->settings.target_point,
                color, &path_texture_point
            );
            
            break;
            
        } case PATH_RESULT_NORMAL_PATH:
        case PATH_RESULT_PATH_WITH_SINGLE_STOP:
        case PATH_RESULT_PATH_WITH_OBSTACLES: {
    
            size_t first_stop = l_ptr->path_info->cur_path_stop_idx;
            if(first_stop >= l_ptr->path_info->path.size()) continue;
            
            draw_go_here_segment(
                l_ptr->pos,
                l_ptr->path_info->path[first_stop]->pos,
                color, &path_texture_point
            );
            for(
                size_t s = first_stop + 1;
                s < l_ptr->path_info->path.size();
                ++s
            ) {
                draw_go_here_segment(
                    l_ptr->path_info->path[s - 1]->pos,
                    l_ptr->path_info->path[s]->pos,
                    color, &path_texture_point
                );
            }
            draw_go_here_segment(
                l_ptr->path_info->path.back()->pos,
                l_ptr->path_info->settings.target_point,
                color, &path_texture_point
            );
            
            break;
            
        } default: {
    
            break;
        }
        }
    }
    
    //Go Here choice path.
    float path_texture_point = 0.0f;
    switch(go_here_path_result) {
    case PATH_RESULT_DIRECT:
    case PATH_RESULT_DIRECT_NO_STOPS: {
        //Go directly from A to B.
        
        draw_go_here_segment(
            radar_selected_leader->pos,
            radar_cursor,
            al_map_rgb(64, 200, 240), &path_texture_point
        );
        
        break;
        
    } case PATH_RESULT_NORMAL_PATH:
    case PATH_RESULT_PATH_WITH_SINGLE_STOP:
    case PATH_RESULT_PATH_WITH_OBSTACLES: {
        //Regular path.
        ALLEGRO_COLOR color;
        if(go_here_path_result == PATH_RESULT_PATH_WITH_OBSTACLES) {
            color = al_map_rgb(200, 64, 64);
        } else {
            color = al_map_rgb(64, 200, 240);
        }
        
        if(!go_here_path.empty()) {
            draw_go_here_segment(
                radar_selected_leader->pos,
                go_here_path[0]->pos,
                color, &path_texture_point
            );
            for(size_t s = 1; s < go_here_path.size(); ++s) {
                draw_go_here_segment(
                    go_here_path[s - 1]->pos,
                    go_here_path[s]->pos,
                    color, &path_texture_point
                );
            }
            draw_go_here_segment(
                go_here_path.back()->pos,
                radar_cursor,
                color, &path_texture_point
            );
        }
        
        break;
        
    } default: {

        break;
    }
    }
    
    //Radar cursor.
    draw_bitmap(
        bmp_radar_cursor, radar_cursor,
        point(48.0f / radar_cam.zoom, 48.0f / radar_cam.zoom),
        game.time_passed * TAU * 0.3f
    );
    
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
        TEXT_VALIGN_MODE_CENTER,
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
        TEXT_VALIGN_MODE_CENTER, area_name_size,
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


/**
 * @brief Draws some help page tidbit's text.
 *
 * @param font Font to use.
 * @param where Coordinates to draw the text on.
 * @param max_size Maximum width or height the text can occupy.
 * A value of zero in one of these coordinates makes it not have a
 * limit in that dimension.
 * @param text Text to draw.
 */
void pause_menu_t::draw_tidbit(
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


/**
 * @brief Fills the list of mission fail conditions.
 *
 * @param list List item to fill.
 */
void pause_menu_t::fill_mission_fail_list(list_gui_item* list) {
    for(size_t f = 0; f < game.mission_fail_conds.size(); ++f) {
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(f)
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


/**
 * @brief Fills the list of mission grading information.
 *
 * @param list List item to fill.
 */
void pause_menu_t::fill_mission_grading_list(list_gui_item* list) {
    switch(game.cur_area_data.mission.grading_mode) {
    case MISSION_GRADING_MODE_POINTS: {
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
                    get_idx_bitmask(c)
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
    } case MISSION_GRADING_MODE_GOAL: {
        add_bullet(
            list,
            "You get a platinum medal if you clear the goal."
        );
        add_bullet(
            list,
            "You get no medal if you fail."
        );
        break;
    } case MISSION_GRADING_MODE_PARTICIPATION: {
        add_bullet(
            list,
            "You get a platinum medal just by playing the mission."
        );
        break;
    }
    }
}


/**
 * @brief Returns a string representing the player's status towards the
 * mission goal.
 *
 * @return The status.
 */
string pause_menu_t::get_mission_goal_status() {
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


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void pause_menu_t::handle_event(const ALLEGRO_EVENT &ev) {
    gui.handle_event(ev);
    radar_gui.handle_event(ev);
    status_gui.handle_event(ev);
    mission_gui.handle_event(ev);
    help_gui.handle_event(ev);
    confirmation_gui.handle_event(ev);
    
    //Handle some radar logic.
    point radar_center;
    point radar_size;
    radar_gui.get_item_draw_info(radar_item, &radar_center, &radar_size);
    bool mouse_in_radar =
        radar_gui.responsive &&
        is_point_in_rectangle(
            game.mouse_cursor.s_pos,
            radar_center, radar_size
        );
        
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(mouse_in_radar) {
            radar_mouse_down = true;
            radar_mouse_down_point = game.mouse_cursor.s_pos;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(mouse_in_radar && !radar_mouse_dragging) {
            //Clicked somewhere.
            radar_confirm();
        }
        
        radar_mouse_down = false;
        radar_mouse_dragging = false;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(
            radar_mouse_down &&
            (
                fabs(game.mouse_cursor.s_pos.x - radar_mouse_down_point.x) >
                4.0f ||
                fabs(game.mouse_cursor.s_pos.y - radar_mouse_down_point.y) >
                4.0f
            )
        ) {
            //Consider the mouse down as part of a mouse drag, not a click.
            radar_mouse_dragging = true;
        }
        
        if(
            radar_mouse_dragging &&
            (ev.mouse.dx != 0.0f || ev.mouse.dy != 0.0f)
        ) {
            //Pan the radar around.
            pan_radar(point(-ev.mouse.dx, -ev.mouse.dy));
            
        } else if(
            mouse_in_radar && ev.mouse.dz != 0.0f
        ) {
            //Zoom in or out, using the radar/mouse cursor as the anchor.
            zoom_radar_with_mouse(ev.mouse.dz * 0.1f, radar_center, radar_size);
            
        }
    }
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void pause_menu_t::handle_player_action(const player_action &action) {
    if(opening_lockout_timer > 0.0f) {
        //Don't accept inputs shortly after the menu opens.
        //This helps errant inputs from before the menu bleeding into the menu
        //immediately after it opens, like the "radar toggle" action.
        return;
    }
    if(closing) return;
    
    bool handled_by_radar = false;
    
    if(radar_gui.responsive) {
        switch(action.action_type_id) {
        case PLAYER_ACTION_TYPE_RADAR: {
            if(action.value >= 0.5f) {
                start_closing(&radar_gui);
                handled_by_radar = true;
            }
            break;
        } case PLAYER_ACTION_TYPE_RADAR_RIGHT: {
            radar_pan.right = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_RADAR_UP: {
            radar_pan.up = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_RADAR_LEFT: {
            radar_pan.left = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_RADAR_DOWN: {
            radar_pan.down = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_RADAR_ZOOM_IN: {
            radar_zoom_in = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT: {
            radar_zoom_out = action.value;
            handled_by_radar = true;
            break;
        } case PLAYER_ACTION_TYPE_MENU_OK: {
            radar_confirm();
            handled_by_radar = true;
            break;
        }
        }
    }
    
    if(!handled_by_radar) {
        //Only let the GUIs handle it if the radar didn't need it, otherwise
        //we could see GUI item selections move around or such because
        //radar and menus actions share binds.
        gui.handle_player_action(action);
        radar_gui.handle_player_action(action);
        status_gui.handle_player_action(action);
        mission_gui.handle_player_action(action);
        help_gui.handle_player_action(action);
        confirmation_gui.handle_player_action(action);
        
        switch(action.action_type_id) {
        case PLAYER_ACTION_TYPE_MENU_PAGE_LEFT:
        case PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT: {
            if(action.value >= 0.5f) {
                gui_manager* cur_gui = &gui;
                PAUSE_MENU_PAGE cur_page = PAUSE_MENU_PAGE_SYSTEM;
                if(radar_gui.responsive) {
                    cur_gui = &radar_gui;
                    cur_page = PAUSE_MENU_PAGE_RADAR;
                } else if(status_gui.responsive) {
                    cur_gui = &status_gui;
                    cur_page = PAUSE_MENU_PAGE_STATUS;
                } else if(mission_gui.responsive) {
                    cur_gui = &mission_gui;
                    cur_page = PAUSE_MENU_PAGE_MISSION;
                }
                size_t cur_page_idx =
                    std::distance(
                        pages.begin(),
                        std::find(pages.begin(), pages.end(), cur_page)
                    );
                size_t new_page_idx =
                    sum_and_wrap(
                        cur_page_idx,
                        action.action_type_id == PLAYER_ACTION_TYPE_MENU_PAGE_LEFT ?
                        -1 :
                        1,
                        pages.size()
                    );
                switch_page(
                    cur_gui,
                    pages[new_page_idx],
                    action.action_type_id == PLAYER_ACTION_TYPE_MENU_PAGE_LEFT
                );
            }
            
            break;
        }
        }
    }
}


/**
 * @brief Initializes the leaving confirmation page.
 */
void pause_menu_t::init_confirmation_page() {
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
    confirmation_gui.set_selected_item(confirmation_gui.back_item, true);
    confirmation_gui.responsive = false;
    confirmation_gui.hide_items();
}


/**
 * @brief Initializes the help page.
 */
void pause_menu_t::init_help_page() {
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
        vector<tidbit> &category_tidbits = tidbits[(HELP_CATEGORY) c];
        category_tidbits.reserve(n_tidbits);
        for(size_t t = 0; t < n_tidbits; ++t) {
            vector<string> parts =
                split(category_node->get_child(t)->name, ";");
            tidbit new_t;
            new_t.name = parts.size() > 0 ? parts[0] : "";
            new_t.description = parts.size() > 1 ? parts[1] : "";
            new_t.image = parts.size() > 2 ? game.bitmaps.get(parts[2]) : nullptr;
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
        if(cur_tidbit == nullptr) return;
        if(cur_tidbit->image == nullptr) return;
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
    help_gui.set_selected_item(help_gui.back_item, true);
    help_gui.responsive = false;
    help_gui.hide_items();
    help_gui.on_selection_changed =
    [this] () {
        cur_tidbit = nullptr;
    };
}


/**
 * @brief Initializes the pause menu's main menu.
 */
void pause_menu_t::init_main_pause_menu() {
    //Menu items.
    gui.register_coords("header",           50,  5, 52,  6);
    gui.register_coords("left_page",        12,  5, 20,  6);
    gui.register_coords("left_page_input",  3,   7,  4,  4);
    gui.register_coords("right_page",       88,  5, 20,  6);
    gui.register_coords("right_page_input", 97,  7,  4,  4);
    gui.register_coords("line",             50, 11, 96,  2);
    gui.register_coords("area_name",        50, 20, 96,  8);
    gui.register_coords("area_subtitle",    50, 27, 88,  6);
    gui.register_coords("continue",         13, 88, 22,  8);
    gui.register_coords("retry",            50, 41, 52, 10);
    gui.register_coords("end",              50, 53, 52, 10);
    gui.register_coords("help",             50, 65, 52, 10);
    gui.register_coords("quit",             87, 88, 22,  8);
    gui.register_coords("tooltip",          50, 96, 96,  4);
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
    
    //Page buttons and inputs.
    create_page_buttons(PAUSE_MENU_PAGE_SYSTEM, &gui);
    
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
        start_closing(&gui);
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
        leave_target = GAMEPLAY_LEAVE_TARGET_RETRY;
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
        leave_target = GAMEPLAY_LEAVE_TARGET_END;
        confirm_or_leave();
    };
    end_button->on_get_tooltip =
    [] () {
        bool as_fail =
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
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
        leave_target = GAMEPLAY_LEAVE_TARGET_AREA_SELECT;
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
    gui.set_selected_item(gui.back_item, true);
    gui.responsive = false;
    gui.hide_items();
}


/**
 * @brief Initializes the mission page.
 */
void pause_menu_t::init_mission_page() {
    data_node gui_file(PAUSE_MENU::MISSION_GUI_FILE_PATH);
    
    //Menu items.
    mission_gui.register_coords("header",           50,  5, 52,  6);
    mission_gui.register_coords("left_page",        12,  5, 20,  6);
    mission_gui.register_coords("left_page_input",   3,  7,  4,  4);
    mission_gui.register_coords("right_page",       88,  5, 20,  6);
    mission_gui.register_coords("right_page_input", 97,  7,  4,  4);
    mission_gui.register_coords("line",             50, 11, 96,  2);
    mission_gui.register_coords("continue",         10, 16, 16,  4);
    mission_gui.register_coords("goal_header",      50, 16, 60,  4);
    mission_gui.register_coords("goal",             50, 22, 96,  4);
    mission_gui.register_coords("goal_status",      50, 26, 96,  4);
    mission_gui.register_coords("fail_header",      50, 32, 96,  4);
    mission_gui.register_coords("fail_list",        48, 48, 92, 24);
    mission_gui.register_coords("fail_scroll",      97, 48,  2, 24);
    mission_gui.register_coords("grading_header",   50, 64, 96,  4);
    mission_gui.register_coords("grading_list",     48, 80, 92, 24);
    mission_gui.register_coords("grading_scroll",   97, 80,  2, 24);
    mission_gui.register_coords("tooltip",          50, 96, 96,  4);
    mission_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "MISSION", game.fonts.area_name,
        COLOR_TRANSPARENT_WHITE
    );
    mission_gui.add_item(header_text, "header");
    
    //Page buttons and inputs.
    create_page_buttons(PAUSE_MENU_PAGE_MISSION, &mission_gui);
    
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
        start_closing(&mission_gui);
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
    mission_gui.set_selected_item(mission_gui.back_item, true);
    mission_gui.responsive = false;
    mission_gui.hide_items();
}


/**
 * @brief Initializes the radar page.
 */
void pause_menu_t::init_radar_page() {
    data_node gui_file(PAUSE_MENU::RADAR_GUI_FILE_PATH);
    
    //Assets.
    data_node* bitmaps_node = gui_file.get_child_by_name("files");
    
#define loader(var, name) \
    var = \
          game.bitmaps.get( \
                            bitmaps_node->get_child_by_name(name)->value, \
                            bitmaps_node->get_child_by_name(name) \
                          );
    
    loader(bmp_radar_cursor,         "cursor");
    loader(bmp_radar_pikmin,         "pikmin");
    loader(bmp_radar_treasure,       "treasure");
    loader(bmp_radar_enemy,          "enemy");
    loader(bmp_radar_leader_bubble,  "leader_bubble");
    loader(bmp_radar_onion_skeleton, "onion_skeleton");
    loader(bmp_radar_onion_bulb,     "onion_bulb");
    loader(bmp_radar_ship,           "ship");
    loader(bmp_radar_path,           "path");
    
#undef loader
    
    //Menu items.
    radar_gui.register_coords("header",              50,     5,    52,    6);
    radar_gui.register_coords("left_page",           12,     5,    20,    6);
    radar_gui.register_coords("left_page_input",      3,     7,     4,    4);
    radar_gui.register_coords("right_page",          88,     5,    20,    6);
    radar_gui.register_coords("right_page_input",    97,     7,     4,    4);
    radar_gui.register_coords("line",                50,    11,    96,    2);
    radar_gui.register_coords("continue",            10,    16,    16,    4);
    radar_gui.register_coords("radar",               37.5,  56.25, 70,   72.5);
    radar_gui.register_coords("group_pikmin_label",  86.25, 77.5,  22.5,  5);
    radar_gui.register_coords("group_pikmin_number", 86.25, 85,    22.5,  5);
    radar_gui.register_coords("idle_pikmin_label",   86.25, 62.5,  22.5,  5);
    radar_gui.register_coords("idle_pikmin_number",  86.25, 70,    22.5,  5);
    radar_gui.register_coords("field_pikmin_label",  86.25, 47.5,  22.5,  5);
    radar_gui.register_coords("field_pikmin_number", 86.25, 55,    22.5,  5);
    radar_gui.register_coords("cursor_info",         86.25, 33.75, 22.5, 17.5);
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
    
    //Page buttons and inputs.
    create_page_buttons(PAUSE_MENU_PAGE_RADAR, &radar_gui);
    
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
        start_closing(&radar_gui);
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
        i2s(game.states.gameplay->get_amount_of_group_pikmin()),
        game.fonts.counter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(group_pik_nr_text, "group_pikmin_number");
    
    //Idle Pikmin label text.
    text_gui_item* idle_pik_label_text =
        new text_gui_item(
        "Idle Pikmin:", game.fonts.standard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radar_gui.add_item(idle_pik_label_text, "idle_pikmin_label");
    
    //Idle Pikmin number text.
    text_gui_item* idle_pik_nr_text =
        new text_gui_item(
        i2s(game.states.gameplay->get_amount_of_idle_pikmin()),
        game.fonts.counter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(idle_pik_nr_text, "idle_pikmin_number");
    
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
        i2s(game.states.gameplay->get_amount_of_field_pikmin()),
        game.fonts.counter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radar_gui.add_item(field_pik_nr_text, "field_pikmin_number");
    
    //Cursor info text.
    text_gui_item* cursor_info_text =
        new text_gui_item("", game.fonts.standard);
    cursor_info_text->line_wrap = true;
    cursor_info_text->on_draw =
    [this, cursor_info_text] (const point & center, const point & size) {
        if(cursor_info_text->text.empty()) return;
        
        //Draw the text.
        int line_height = al_get_font_line_height(cursor_info_text->font);
        vector<string_token> tokens = tokenize_string(cursor_info_text->text);
        set_string_token_widths(
            tokens, game.fonts.standard, game.fonts.slim, line_height, false
        );
        vector<vector<string_token> > tokens_per_line =
            split_long_string_with_tokens(tokens, size.x);
        float text_h = tokens_per_line.size() * line_height;
        
        for(size_t l = 0; l < tokens_per_line.size(); ++l) {
            draw_string_tokens(
                tokens_per_line[l], game.fonts.standard, game.fonts.slim,
                false,
                point(
                    center.x,
                    center.y - text_h / 2.0f + l * line_height
                ),
                cursor_info_text->flags,
                point(size.x, line_height)
            );
        }
        
        //Draw a box around it.
        draw_rounded_rectangle(
            center, size, 8.0f, COLOR_TRANSPARENT_WHITE, 2.0f
        );
        
        //Draw a connection from here to the radar cursor.
        point line_anchor(center.x - size.x / 2.0f - 16.0f, center.y);
        point cursor_screen_pos = radar_cursor;
        al_transform_coordinates(
            &world_to_radar_screen_transform,
            &cursor_screen_pos.x, &cursor_screen_pos.y
        );
        
        al_draw_line(
            center.x - size.x / 2.0f, center.y,
            line_anchor.x, line_anchor.y,
            COLOR_TRANSPARENT_WHITE, 2.0f
        );
        
        cursor_screen_pos =
            cursor_screen_pos +
            rotate_point(
                point(24.0f, 0.0f),
                get_angle(cursor_screen_pos, line_anchor)
            );
        al_draw_line(
            line_anchor.x, line_anchor.y,
            cursor_screen_pos.x, cursor_screen_pos.y,
            COLOR_TRANSPARENT_WHITE, 2.0f
        );
    };
    cursor_info_text->on_tick =
    [this, cursor_info_text] (float delta_t) {
        if(radar_cursor_leader) {
            cursor_info_text->text =
                (
                    radar_cursor_leader == radar_selected_leader ?
                    "" :
                    "\\k menu_ok \\k "
                ) + radar_cursor_leader->type->name;
        } else if(
            radar_selected_leader &&
            !radar_selected_leader->fsm.get_event(LEADER_EV_GO_HERE)
        ) {
            cursor_info_text->text =
                "Can't go here... Leader is busy!";
            cursor_info_text->color = COLOR_WHITE;
        } else {
            switch(go_here_path_result) {
            case PATH_RESULT_DIRECT:
            case PATH_RESULT_DIRECT_NO_STOPS:
            case PATH_RESULT_NORMAL_PATH:
            case PATH_RESULT_PATH_WITH_SINGLE_STOP: {
                cursor_info_text->text = "\\k menu_ok \\k Go here!";
                cursor_info_text->color = COLOR_GOLD;
                break;
            } case PATH_RESULT_PATH_WITH_OBSTACLES: {
                cursor_info_text->text = "Can't go here... Path blocked!";
                cursor_info_text->color = COLOR_WHITE;
                break;
            } case PATH_RESULT_END_STOP_UNREACHABLE: {
                cursor_info_text->text = "Can't go here...";
                cursor_info_text->color = COLOR_WHITE;
                break;
            } default: {
                cursor_info_text->text.clear();
                cursor_info_text->color = COLOR_WHITE;
                break;
            }
            }
        }
    };
    radar_gui.add_item(cursor_info_text, "cursor_info");
    
    //Instructions text.
    text_gui_item* instructions_text =
        new text_gui_item(
        "\\k menu_radar_up \\k"
        "\\k menu_radar_left \\k"
        "\\k menu_radar_down \\k"
        "\\k menu_radar_right \\k Pan   "
        "\\k menu_radar_zoom_in \\k"
        "\\k menu_radar_zoom_out \\k Zoom",
        game.fonts.slim,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    instructions_text->line_wrap = true;
    radar_gui.add_item(instructions_text, "instructions");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&radar_gui);
    radar_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    radar_gui.set_selected_item(nullptr);
    radar_gui.responsive = false;
    radar_gui.hide_items();
}


/**
 * @brief Initializes the status page.
 */
void pause_menu_t::init_status_page() {
    data_node gui_file(PAUSE_MENU::STATUS_GUI_FILE_PATH);
    
    //Menu items.
    status_gui.register_coords("header",           50,     5,   52,    6);
    status_gui.register_coords("left_page",        12,     5,   20,    6);
    status_gui.register_coords("left_page_input",   3,     7,    4,    4);
    status_gui.register_coords("right_page",       88,     5,   20,    6);
    status_gui.register_coords("right_page_input", 97,     7,    4,    4);
    status_gui.register_coords("line",             50,    11,   96,    2);
    status_gui.register_coords("continue",         10,    16,   16,    4);
    status_gui.register_coords("list_header",      50,    23,   88,    6);
    status_gui.register_coords("list",             50,    56,   88,   56);
    status_gui.register_coords("list_scroll",      97,    56,    2,   56);
    status_gui.register_coords("totals",           50,    89,   88,    6);
    status_gui.register_coords("instructions",     58.75, 16,   77.5,  4);
    status_gui.register_coords("tooltip",          50,    96,   96,    4);
    status_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "STATUS", game.fonts.area_name,
        COLOR_TRANSPARENT_WHITE
    );
    status_gui.add_item(header_text, "header");
    
    //Page buttons and inputs.
    create_page_buttons(PAUSE_MENU_PAGE_STATUS, &status_gui);
    
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
    status_gui.add_item(line, "line");
    
    //Continue button.
    status_gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    status_gui.back_item->on_activate =
    [this] (const point &) {
        start_closing(&status_gui);
    };
    status_gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    status_gui.add_item(status_gui.back_item, "continue");
    
    //Pikmin list header box.
    list_gui_item* list_header = new list_gui_item();
    list_header->on_draw =
    [] (const point &, const point &) {};
    status_gui.add_item(list_header, "list_header");
    
    //Pikmin list box.
    pikmin_list = new list_gui_item();
    status_gui.add_item(pikmin_list, "list");
    
    //Pikmin list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = pikmin_list;
    status_gui.add_item(list_scroll, "list_scroll");
    
    //Pikmin totals box.
    list_gui_item* totals = new list_gui_item();
    totals->on_draw =
    [] (const point &, const point &) {};
    status_gui.add_item(totals, "totals");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&status_gui);
    status_gui.add_item(tooltip_text, "tooltip");
    
    //Setup the list header.
    add_pikmin_status_line(
        list_header,
        nullptr,
        "Group",
        "Idle",
        "Field",
        "Onion",
        "Total",
        "New",
        "Lost",
        true, false
    );
    
    size_t total_in_group = 0;
    size_t total_idling = 0;
    size_t total_on_field = 0;
    long total_in_onion = 0;
    long grand_total = 0;
    long total_new = 0;
    long total_lost = 0;
    
    //Setup the list rows.
    for(size_t p = 0; p < game.config.pikmin_order.size(); ++p) {
        pikmin_type* pt_ptr = game.config.pikmin_order[p];
        
        size_t in_group =
            game.states.gameplay->get_amount_of_group_pikmin(pt_ptr);
        size_t idling =
            game.states.gameplay->get_amount_of_idle_pikmin(pt_ptr);
        size_t on_field =
            game.states.gameplay->get_amount_of_field_pikmin(pt_ptr);
        long in_onion =
            game.states.gameplay->get_amount_of_onion_pikmin(pt_ptr);
        long total = on_field + in_onion;
        
        long new_piks = 0;
        auto new_it =
            game.states.gameplay->pikmin_born_per_type.find(pt_ptr);
        if(new_it != game.states.gameplay->pikmin_born_per_type.end()) {
            new_piks = new_it->second;
        }
        long lost = 0;
        auto lost_it =
            game.states.gameplay->pikmin_deaths_per_type.find(pt_ptr);
        if(lost_it != game.states.gameplay->pikmin_deaths_per_type.end()) {
            lost = lost_it->second;
        }
        
        if(total + new_piks + lost > 0) {
            add_pikmin_status_line(
                pikmin_list,
                pt_ptr,
                i2s(in_group),
                i2s(idling),
                i2s(on_field),
                i2s(in_onion),
                i2s(total),
                i2s(new_piks),
                i2s(lost),
                false, false
            );
        }
        
        total_in_group += in_group;
        total_idling += idling;
        total_on_field += on_field;
        total_in_onion += in_onion;
        grand_total += total;
        total_new += new_piks;
        total_lost += lost;
    }
    
    //Setup the list totals.
    add_pikmin_status_line(
        totals,
        nullptr,
        i2s(total_in_group),
        i2s(total_idling),
        i2s(total_on_field),
        i2s(total_in_onion),
        i2s(grand_total),
        i2s(total_new),
        i2s(total_lost),
        true, true
    );
    
    //Finishing touches.
    status_gui.set_selected_item(status_gui.back_item, true);
    status_gui.responsive = false;
    status_gui.hide_items();
}


/**
 * @brief Pans the radar by an amount.
 *
 * @param amount How much to pan by.
 */
void pause_menu_t::pan_radar(point amount) {
    point delta = amount / radar_cam.zoom;
    radar_cam.pos += delta;
    radar_cam.pos.x =
        clamp(radar_cam.pos.x, radar_min_coords.x, radar_max_coords.x);
    radar_cam.pos.y =
        clamp(radar_cam.pos.y, radar_min_coords.y, radar_max_coords.y);
}


/**
 * @brief Populates the help page's list of tidbits.
 *
 * @param category Category of tidbits to use.
 */
void pause_menu_t::populate_help_tidbits(const HELP_CATEGORY category) {
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


/**
 * @brief When the player confirms their action in the radar.
 */
void pause_menu_t::radar_confirm() {
    calculate_go_here_path();
    
    if(radar_cursor_leader) {
        //Select a leader.
        radar_selected_leader = radar_cursor_leader;
        
    } else if(
        go_here_path_result == PATH_RESULT_DIRECT ||
        go_here_path_result == PATH_RESULT_DIRECT_NO_STOPS ||
        go_here_path_result == PATH_RESULT_NORMAL_PATH ||
        go_here_path_result == PATH_RESULT_PATH_WITH_SINGLE_STOP
    ) {
        //Start Go Here.
        radar_selected_leader->fsm.run_event(
            LEADER_EV_GO_HERE, (void*) &radar_cursor
        );
        start_closing(&radar_gui);
        
    }
}


/**
 * @brief Starts the closing process.
 *
 * @param cur_gui The currently active GUI manager.
 */
void pause_menu_t::start_closing(gui_manager* cur_gui) {
    cur_gui->start_animation(
        GUI_MANAGER_ANIM_CENTER_TO_UP,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    game.states.gameplay->hud->gui.start_animation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    closing = true;
    closing_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
    
    game.states.gameplay->radar_zoom = radar_cam.zoom;
}


/**
 * @brief Starts the process of leaving the gameplay state.
 */
void pause_menu_t::start_leaving_gameplay() {
    if(
        leave_target == GAMEPLAY_LEAVE_TARGET_END &&
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        )
    ) {
        game.states.gameplay->mission_fail_reason =
            MISSION_FAIL_COND_PAUSE_MENU;
    }
    game.states.gameplay->start_leaving(leave_target);
}


/**
 * @brief Switches pages in the pause menu.
 *
 * @param cur_gui Pointer to the current page's GUI manager.
 * @param new_page The new page to switch to.
 * @param left Is the new page to the left of the current one, or the right?
 */
void pause_menu_t::switch_page(
    gui_manager* cur_gui, PAUSE_MENU_PAGE new_page, bool left
) {
    gui_manager* new_gui = nullptr;
    switch(new_page) {
    case PAUSE_MENU_PAGE_SYSTEM: {
        new_gui = &gui;
        break;
    } case PAUSE_MENU_PAGE_RADAR: {
        new_gui = &radar_gui;
        break;
    } case PAUSE_MENU_PAGE_STATUS: {
        new_gui = &status_gui;
        break;
    } case PAUSE_MENU_PAGE_MISSION: {
        new_gui = &mission_gui;
        break;
    }
    }
    
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
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void pause_menu_t::tick(const float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
    radar_gui.tick(delta_t);
    status_gui.tick(delta_t);
    mission_gui.tick(delta_t);
    help_gui.tick(delta_t);
    confirmation_gui.tick(delta_t);
    
    //Tick the background.
    const float bg_alpha_mult_speed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bg_alpha_mult_speed : bg_alpha_mult_speed;
    bg_alpha_mult = clamp(bg_alpha_mult + diff * delta_t, 0.0f, 1.0f);
    
    //Tick the menu opening and closing.
    if(opening_lockout_timer > 0.0f) {
        opening_lockout_timer -= delta_t;
    }
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
    
    //Tick radar things.
    point radar_center;
    point radar_size;
    radar_gui.get_item_draw_info(radar_item, &radar_center, &radar_size);
    
    update_radar_transformations(radar_center, radar_size);
    
    if(radar_gui.responsive) {
    
        point radar_pan_coords;
        float dummy_angle;
        float dummy_magnitude;
        radar_pan.get_info(&radar_pan_coords, &dummy_angle, &dummy_magnitude);
        if(radar_pan_coords.x != 0.0f || radar_pan_coords.y != 0.0f) {
            pan_radar(radar_pan_coords * PAUSE_MENU::RADAR_PAN_SPEED * delta_t);
        }
        
        if(radar_zoom_in && !radar_zoom_out) {
            zoom_radar(PAUSE_MENU::RADAR_ZOOM_SPEED * delta_t);
        } else if(radar_zoom_out && !radar_zoom_in) {
            zoom_radar(-PAUSE_MENU::RADAR_ZOOM_SPEED * delta_t);
        }
        
        bool mouse_in_radar =
            is_point_in_rectangle(
                game.mouse_cursor.s_pos,
                radar_center, radar_size
            );
            
        if(mouse_in_radar) {
            radar_cursor = game.mouse_cursor.s_pos;
            al_transform_coordinates(
                &radar_screen_to_world_transform,
                &radar_cursor.x, &radar_cursor.y
            );
        } else {
            radar_cursor = radar_cam.pos;
        }
        
        go_here_calc_time -= delta_t;
        if(go_here_calc_time <= 0.0f) {
            go_here_calc_time = PAUSE_MENU::GO_HERE_CALC_INTERVAL;
            
            calculate_go_here_path();
        }
        
    }
    
}


/**
 * @brief Updates the radar transformations.
 *
 * @param radar_center Coordinates of the radar's center.
 * @param radar_size Dimensions of the radar.
 */
void pause_menu_t::update_radar_transformations(
    const point &radar_center, const point &radar_size
) {
    world_to_radar_screen_transform = game.identity_transform;
    al_translate_transform(
        &world_to_radar_screen_transform,
        -radar_cam.pos.x + radar_center.x / radar_cam.zoom,
        -radar_cam.pos.y + radar_center.y / radar_cam.zoom
    );
    al_scale_transform(
        &world_to_radar_screen_transform, radar_cam.zoom, radar_cam.zoom
    );
    
    radar_screen_to_world_transform = world_to_radar_screen_transform;
    al_invert_transform(&radar_screen_to_world_transform);
}


/**
 * @brief Zooms the radar by an amount.
 *
 * @param amount How much to zoom by.
 */
void pause_menu_t::zoom_radar(float amount) {
    float delta = amount * radar_cam.zoom;
    radar_cam.zoom += delta;
    radar_cam.zoom =
        clamp(
            radar_cam.zoom,
            PAUSE_MENU::RADAR_MIN_ZOOM, PAUSE_MENU::RADAR_MAX_ZOOM
        );
}


/**
 * @brief Zooms the radar by an amount, anchored on the radar cursor.
 *
 * @param amount How much to zoom by.
 * @param radar_center Coordinates of the radar's center.
 * @param radar_size Dimensions of the radar.
 */
void pause_menu_t::zoom_radar_with_mouse(
    float amount, const point &radar_center, const point &radar_size
) {
    //Keep a backup of the old cursor coordinates.
    point old_cursor_pos = radar_cursor;
    
    //Do the zoom.
    zoom_radar(amount);
    update_radar_transformations(radar_center, radar_size);
    
    //Figure out where the cursor will be after the zoom.
    radar_cursor = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &radar_screen_to_world_transform,
        &radar_cursor.x, &radar_cursor.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the cursor ends up where it was before.
    pan_radar(
        point(
            (old_cursor_pos.x - radar_cursor.x) * radar_cam.zoom,
            (old_cursor_pos.y - radar_cursor.y) * radar_cam.zoom
        )
    );
    
    //Update the cursor coordinates again.
    update_radar_transformations(radar_center, radar_size);
    radar_cursor = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &radar_screen_to_world_transform,
        &radar_cursor.x, &radar_cursor.y
    );
}
