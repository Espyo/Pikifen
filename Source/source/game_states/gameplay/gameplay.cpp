/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gameplay state class and
 * gameplay state-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../load.h"
#include "../../misc_structs.h"
#include "../../mobs/converter.h"
#include "../../mobs/pile.h"
#include "../../utils/data_file.h"
#include "../../utils/string_utils.h"


namespace GAMEPLAY {
//How long the HUD moves for when the area is entered.
const float AREA_INTRO_HUD_MOVE_TIME = 3.0f;
//How long it takes for the area name to fade away, in-game.
const float AREA_TITLE_FADE_DURATION = 3.0f;
//How long the "Go!" big message lasts for.
const float BIG_MSG_GO_DUR = 1.5f;
//What text to show in the "Go!" big message.
const string BIG_MSG_GO_TEXT = "GO!";
//How long the "Mission clear!" big message lasts for.
const float BIG_MSG_MISSION_CLEAR_DUR = 4.5f;
//What text to show in the "Mission clear!" big message.
const string BIG_MSG_MISSION_CLEAR_TEXT = "MISSION CLEAR!";
//How long the "Mission failed..." big message lasts for.
const float BIG_MSG_MISSION_FAILED_DUR = 4.5f;
//What text to show in the "Mission failed..." big message.
const string BIG_MSG_MISSION_FAILED_TEXT = "MISSION FAILED...";
//How long the "Ready?" big message lasts for.
const float BIG_MSG_READY_DUR = 2.5f;
//What text to show in the "Ready?" big message.
const string BIG_MSG_READY_TEXT = "READY?";
//Something is only considered off-camera if it's beyond this extra margin.
const float CAMERA_BOX_MARGIN = 128.0f;
//Dampen the camera's movements by this much.
const float CAMERA_SMOOTHNESS_MULT = 4.5f;
//Opacity of the collision bubbles in the maker tool.
const unsigned char COLLISION_OPACITY = 192;
//Maximum alpha of the cursor's trail -- the alpha value near the cursor.
const unsigned char CURSOR_TRAIL_MAX_ALPHA = 72;
//Maximum width of the cursor's trail -- the width value near the cursor.
const float CURSOR_TRAIL_MAX_WIDTH = 30.0f;
//How far the cursor must move from its current spot before the next spot.
const float CURSOR_TRAIL_MIN_SPOT_DIFF = 4.0f;
//Every X seconds, the cursor's position is saved, to create the trail effect.
const float CURSOR_TRAIL_SAVE_INTERVAL = 0.016f;
//Number of positions of the cursor to keep track of.
const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS = 16;
//Width and height of the fog bitmap.
const int FOG_BITMAP_SIZE = 128;
//Dampen the mission goal indicator's movement by this much.
const float GOAL_INDICATOR_SMOOTHNESS_MULT = 5.5f;
//How long the HUD moves for when a menu is entered.
const float MENU_ENTRY_HUD_MOVE_TIME = 0.4f;
//How long the HUD moves for when a menu is exited.
const float MENU_EXIT_HUD_MOVE_TIME = 0.5f;
//Opacity of the throw preview.
const unsigned char PREVIEW_OPACITY = 160;
//Scale of the throw preview's effect texture.
const float PREVIEW_TEXTURE_SCALE = 20.0f;
//Time multiplier for the throw preview's effect texture animation.
const float PREVIEW_TEXTURE_TIME_MULT = 20.0f;
//How frequently should a replay state be saved.
const float REPLAY_SAVE_FREQUENCY = 1.0f;
//Swarming arrows move these many units per second.
const float SWARM_ARROW_SPEED = 400.0f;
//Tree shadows sway this much away from their neutral position.
const float TREE_SHADOW_SWAY_AMOUNT = 8.0f;
//Tree shadows sway this much per second (TAU = full back-and-forth cycle).
const float TREE_SHADOW_SWAY_SPEED = TAU / 8;
}


/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay_state::gameplay_state() :
    game_state(),
    after_hours(false),
    area_time_passed(0.0f),
    area_title_fade_timer(GAMEPLAY::AREA_TITLE_FADE_DURATION),
    bmp_fog(nullptr),
    closest_group_member_distant(false),
    cur_leader_nr(0),
    cur_leader_ptr(nullptr),
    day(1),
    day_minutes(0.0f),
    delta_t_mult(1.0f),
    hud(nullptr),
    leader_cursor_sector(nullptr),
    msg_box(nullptr),
    next_mob_id(0),
    particles(0),
    precipitation(0),
    selected_spray(0),
    swarm_angle(0),
    swarm_magnitude(0.0f),
    throw_dest_mob(nullptr),
    throw_dest_sector(nullptr),
    unloading(false),
    went_to_results(false),
    mission_required_mob_amount(0),
    pikmin_born(0),
    pikmin_deaths(0),
    treasures_collected(0),
    treasures_total(0),
    treasure_points_collected(0),
    treasure_points_total(0),
    enemy_deaths(0),
    enemy_total(0),
    enemy_points_collected(0),
    enemy_points_total(0),
    cur_leaders_in_mission_exit(0),
    leaders_kod(0),
    starting_nr_of_leaders(0),
    goal_indicator_ratio(0.0f),
    cur_interlude(INTERLUDE_NONE),
    interlude_time(0.0f),
    cur_big_msg(BIG_MESSAGE_NONE),
    big_msg_time(0.0f),
    cancel_control_id(INVALID),
    close_to_interactable_to_use(nullptr),
    close_to_nest_to_open(nullptr),
    close_to_pikmin_to_pluck(nullptr),
    close_to_ship_to_heal(nullptr),
    cursor_height_diff_light(0.0f),
    cursor_save_timer(GAMEPLAY::CURSOR_TRAIL_SAVE_INTERVAL),
    is_input_allowed(false),
    lightmap_bmp(nullptr),
    main_control_id(INVALID),
    onion_menu(nullptr),
    pause_menu(nullptr),
    paused(false),
    ready_for_input(false),
    swarm_cursor(false) {
    
    closest_group_member[BUBBLE_PREVIOUS] = NULL;
    closest_group_member[BUBBLE_CURRENT] = NULL;
    closest_group_member[BUBBLE_NEXT] = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Changes the amount of sprays of a certain type the player owns.
 * It also animates the correct HUD item, if any.
 * type_nr:
 *   Number of the spray type.
 * amount:
 *   Amount to change by.
 */
void gameplay_state::change_spray_count(
    const size_t type_nr, signed int amount
) {
    spray_stats[type_nr].nr_sprays =
        std::max(
            (signed int) spray_stats[type_nr].nr_sprays + amount,
            (signed int) 0
        );
        
    gui_item* spray_hud_item = NULL;
    if(game.spray_types.size() > 2) {
        if(selected_spray == type_nr) {
            spray_hud_item = hud->spray_1_amount;
        }
    } else {
        if(type_nr == 0) {
            spray_hud_item = hud->spray_1_amount;
        } else {
            spray_hud_item = hud->spray_2_amount;
        }
    }
    if(spray_hud_item) {
        spray_hud_item->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draw the gameplay.
 */
void gameplay_state::do_drawing() {
    do_game_drawing();
    
    if(game.perf_mon) {
        game.perf_mon->leave_state();
    }
}


/* ----------------------------------------------------------------------------
 * Tick the gameplay logic by one frame.
 */
void gameplay_state::do_logic() {
    if(game.perf_mon) {
        if(is_input_allowed) {
            //The first frame will have its speed all broken,
            //because of the long loading time that came before it.
            game.perf_mon->set_paused(false);
            game.perf_mon->enter_state(PERF_MON_STATE_FRAME);
        } else {
            game.perf_mon->set_paused(true);
        }
    }
    
    if(game.maker_tools.change_speed) {
        game.delta_t *= game.maker_tools.change_speed_mult;
    }
    
    if(!paused) {
        do_gameplay_logic(game.delta_t* delta_t_mult);
    }
    do_menu_logic();
    do_aesthetic_logic(game.delta_t* delta_t_mult);
}


/* ----------------------------------------------------------------------------
 * Ends the currently ongoing mission.
 * cleared:
 *   Did the player reach the goal?
 */
void gameplay_state::end_mission(const bool cleared) {
    if(cur_interlude != INTERLUDE_NONE) {
        return;
    }
    cur_interlude = INTERLUDE_MISSION_END;
    interlude_time = 0.0f;
    delta_t_mult = 0.5f;
    leader_movement.reset(); //TODO replace with a better solution.
    
    //Zoom in on the reason, if possible.
    point new_cam_pos = game.cam.target_pos;
    float new_cam_zoom = game.cam.target_zoom;
    if(cleared) {
        mission_goal* goal =
            game.mission_goals[game.cur_area_data.mission.goal];
        if(goal->get_end_zoom_data(this, &new_cam_pos, &new_cam_zoom)) {
            game.cam.target_pos = new_cam_pos;
            game.cam.target_zoom = new_cam_zoom;
        }
        
    } else {
        mission_fail* cond =
            game.mission_fail_conds[mission_fail_reason];
        if(cond->get_end_zoom_data(this, &new_cam_pos, &new_cam_zoom)) {
            game.cam.target_pos = new_cam_pos;
            game.cam.target_zoom = new_cam_zoom;
        }
    }
    
    if(cleared) {
        cur_big_msg = BIG_MESSAGE_MISSION_CLEAR;
    } else {
        cur_big_msg = BIG_MESSAGE_MISSION_FAILED;
    }
    big_msg_time = 0.0f;
    hud->gui.start_animation(
        GUI_MANAGER_ANIM_IN_TO_OUT,
        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/* ----------------------------------------------------------------------------
 * Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void gameplay_state::enter() {
    al_hide_mouse_cursor(game.display);
    update_transformations();
    
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    game.mouse_cursor_s.x = al_get_mouse_state_axis(&mouse_state, 0);
    game.mouse_cursor_s.y = al_get_mouse_state_axis(&mouse_state, 1);
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
    leader_cursor_w = game.mouse_cursor_w;
    leader_cursor_s = game.mouse_cursor_s;
    
    last_enemy_killed_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_hurt_leader_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_pikmin_born_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_pikmin_death_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_ship_that_got_treasure_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    
    goal_indicator_ratio = 0.0f;
    
    hud->gui.hide_items();
    if(went_to_results) {
        game.fade_mgr.start_fade(true, nullptr);
        if(pause_menu) {
            pause_menu->to_delete = true;
        }
    }
    
    ready_for_input = false;
}


/* ----------------------------------------------------------------------------
 * Generates the bitmap that'll draw the fog fade effect.
 * near_radius:
 *   Until this radius, the fog is not present.
 * far_radius:
 *   From this radius on, the fog is fully dense.
 */
ALLEGRO_BITMAP* gameplay_state::generate_fog_bitmap(
    const float near_radius, const float far_radius
) {
    if(far_radius == 0) return NULL;
    
    ALLEGRO_BITMAP* bmp =
        al_create_bitmap(GAMEPLAY::FOG_BITMAP_SIZE, GAMEPLAY::FOG_BITMAP_SIZE);
        
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY
        );
    unsigned char* row = (unsigned char*) region->data;
    
    //We need to draw a radial gradient to represent the fog.
    //Between the center and the "near" radius, the opacity is 0%.
    //From there to the edge, the opacity fades to 100%.
    //Because the every quadrant of the image is the same, just mirrored,
    //we only need to process the pixels on the top-left quadrant and then
    //apply them to the respective pixels on the other quadrants as well.
    
    //This is where the "near" section of the fog is.
    float near_ratio = near_radius / far_radius;
    
#define fill_pixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = cur_a; \
    
    for(int y = 0; y < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); ++y) {
        for(int x = 0; x < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); ++x) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            float cur_ratio =
                dist(
                    point(x, y),
                    point(
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0,
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0
                    )
                ).to_float() / (GAMEPLAY::FOG_BITMAP_SIZE / 2.0);
            cur_ratio = std::min(cur_ratio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            cur_ratio =
                interpolate_number(cur_ratio, near_ratio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            cur_ratio = clamp(cur_ratio, 0.0f, 1.0f);
            unsigned char cur_a = 255 * cur_ratio;
            
            //Save the memory location of the opposite row's pixels.
            unsigned char* opposite_row =
                row + region->pitch * (GAMEPLAY::FOG_BITMAP_SIZE - y - y - 1);
            fill_pixel(x, row);
            fill_pixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, row);
            fill_pixel(x, opposite_row);
            fill_pixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, opposite_row);
        }
        row += region->pitch;
    }
    
#undef fill_pixel
    
    al_unlock_bitmap(bmp);
    bmp = recreate_bitmap(bmp); //Refresh mipmaps.
    return bmp;
}


/* ----------------------------------------------------------------------------
 * Returns the closest group member of a given standby subgroup.
 * In the case all candidate members are out of reach,
 * this returns the closest. Otherwise, it returns the closest
 * and more mature one.
 * Returns NULL if there is no member of that subgroup available.
 * type:
 *   Type to search for.
 */
mob* gameplay_state::get_closest_group_member(subgroup_type* type) {
    if(!cur_leader_ptr) return NULL;
    
    mob* result = NULL;
    
    //Closest members so far for each maturity.
    dist closest_dists[N_MATURITIES];
    mob* closest_ptrs[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        closest_ptrs[m] = NULL;
    }
    
    //Fetch the closest, for each maturity.
    size_t n_members = cur_leader_ptr->group->members.size();
    for(size_t m = 0; m < n_members; ++m) {
    
        mob* member_ptr = cur_leader_ptr->group->members[m];
        if(member_ptr->subgroup_type_ptr != type) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(member_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((pikmin*) member_ptr)->maturity;
        }
        
        dist d(cur_leader_ptr->pos, member_ptr->pos);
        
        if(!closest_ptrs[maturity] || d < closest_dists[maturity]) {
            closest_dists[maturity] = d;
            closest_ptrs[maturity] = member_ptr;
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    dist closest_dist;
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        if(!closest_ptrs[2 - m]) continue;
        if(closest_dists[2 - m] > game.config.group_member_grab_range) continue;
        result = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!result) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; ++m) {
            if(!closest_ptrs[m]) continue;
            
            if(!result || closest_dists[m] < closest_dist) {
                result = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string gameplay_state::get_name() const {
    return "gameplay";
}


/* ----------------------------------------------------------------------------
 * Returns the total amount of Pikmin the player has.
 * This includes Pikmin in the field as well as the Onions, and also
 * Pikmin inside converters.
 */
size_t gameplay_state::get_total_pikmin_amount() {
    //Check Pikmin in the field.
    size_t n_total_pikmin = mobs.pikmin_list.size();
    
    //Check Pikmin inside Onions.
    for(size_t o = 0; o < mobs.onions.size(); ++o) {
        onion* o_ptr = mobs.onions[o];
        for(
            size_t t = 0;
            t < o_ptr->oni_type->nest->pik_types.size();
            ++t
        ) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                n_total_pikmin += o_ptr->nest->pikmin_inside[t][m];
            }
        }
    }
    
    //Check Pikmin inside ships.
    for(size_t s = 0; s < mobs.ships.size(); ++s) {
        ship* s_ptr = mobs.ships[s];
        if(!s_ptr->nest) continue;
        for(
            size_t t = 0;
            t < s_ptr->shi_type->nest->pik_types.size();
            ++t
        ) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                n_total_pikmin += s_ptr->nest->pikmin_inside[t][m];
            }
        }
    }
    
    //Check Pikmin inside converters.
    for(size_t c = 0; c < mobs.converters.size(); ++c) {
        converter* c_ptr = mobs.converters[c];
        n_total_pikmin += c_ptr->amount_in_buffer;
    }
    
    //Return the final sum.
    return n_total_pikmin;
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void gameplay_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    //Handle the Onion menu first so events don't bleed from gameplay to it.
    if(onion_menu) {
        onion_menu->handle_event(ev);
    } else if(pause_menu) {
        pause_menu->handle_event(ev);
    }
    
    //Check if there are system key presses.
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        process_system_key_press(ev.keyboard.keycode);
    }
    
    if (ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
        leader_movement.reset(); //TODO replace with a better solution.
    }
    
    //Decode any inputs that result in gameplay actions.
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_button(actions[a].button, actions[a].pos, actions[a].player);
    }
    
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        if(
            ev.type == ALLEGRO_EVENT_MOUSE_AXES &&
            game.options.mouse_moves_cursor[p]
        ) {
            game.mouse_cursor_s.x = ev.mouse.x;
            game.mouse_cursor_s.y = ev.mouse.y;
            game.mouse_cursor_w = game.mouse_cursor_s;
            
            al_transform_coordinates(
                &game.screen_to_world_transform,
                &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
            );
        }
    }
    
    //Finally, let the HUD handle events.
    hud->gui.handle_event(ev);
    
}


/* ----------------------------------------------------------------------------
 * Initializes the HUD.
 */
void gameplay_state::init_hud() {
    hud = new hud_struct();
}


/* ----------------------------------------------------------------------------
 * Leaves the gameplay state and enters the main menu,
 * or area selection, or etc.
 * target:
 *   Where to leave to.
 */
void gameplay_state::leave(const LEAVE_TARGET target) {
    if(unloading) return;
    
    if(game.perf_mon) {
        //Don't register the final frame, since it won't draw anything.
        game.perf_mon->set_paused(true);
    }
    
    al_show_mouse_cursor(game.display);
    
    switch(target) {
    case LEAVE_TO_RETRY: {
        game.change_state(game.states.gameplay);
        break;
    } case LEAVE_TO_END: {
        went_to_results = true;
        //Change state, but don't unload this one, since the player
        //may pick the "keep playing" option in the results screen.
        game.change_state(game.states.results, false);
        break;
    } case LEAVE_TO_AREA_SELECT: {
        if(game.states.area_ed->quick_play_area_path.empty()) {
            game.states.area_menu->area_type = game.cur_area_data.type;
            game.change_state(game.states.area_menu);
        } else {
            game.change_state(game.states.area_ed);
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the "gameplay" state into memory.
 */
void gameplay_state::load() {
    if(game.perf_mon) {
        game.perf_mon->reset();
        game.perf_mon->enter_state(PERF_MON_STATE_LOADING);
        game.perf_mon->set_paused(false);
    }
    
    size_t errors_reported_at_start = game.errors_reported_so_far;
    went_to_results = false;
    
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    //Game content.
    load_game_content();
    
    //Initialize some important things.
    size_t n_spray_types = game.spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) {
        spray_stats.push_back(spray_stats_struct());
    }
    
    day_minutes = game.cur_area_data.day_time_start;
    area_time_passed = 0.0f;
    paused = false;
    cur_interlude = INTERLUDE_READY;
    interlude_time = 0.0f;
    cur_big_msg = BIG_MESSAGE_READY;
    big_msg_time = 0.0f;
    delta_t_mult = 0.5f;
    game.maker_tools.reset_for_gameplay();
    area_title_fade_timer.start();
    
    after_hours = false;
    pikmin_born = 0;
    pikmin_deaths = 0;
    treasures_collected = 0;
    treasures_total = 0;
    treasure_points_collected = 0;
    treasure_points_total = 0;
    enemy_deaths = 0;
    enemy_total = 0;
    enemy_points_collected = 0;
    enemy_points_total = 0;
    cur_leaders_in_mission_exit = 0;
    mission_required_mob_amount = 0;
    leaders_kod = 0;
    mission_fail_reason = (MISSION_FAIL_CONDITIONS) INVALID;
    notification.reset();
    
    game.framerate_last_avg_point = 0;
    game.framerate_history.clear();
    
    //Load the area.
    string area_folder_name;
    AREA_TYPES area_type;
    get_area_info_from_path(
        path_of_area_to_load, &area_folder_name, &area_type
    );
    load_area(area_folder_name, area_type, false, false);
    
    if(!game.cur_area_data.weather_condition.blackout_strength.empty()) {
        lightmap_bmp = al_create_bitmap(game.win_w, game.win_h);
    }
    if(!game.cur_area_data.weather_condition.fog_color.empty()) {
        bmp_fog =
            generate_fog_bitmap(
                game.cur_area_data.weather_condition.fog_near,
                game.cur_area_data.weather_condition.fog_far
            );
    }
    
    //Generate mobs.
    next_mob_id = 0;
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object generation");
    }
    
    vector<mob*> mobs_per_gen;
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        if(
            m_ptr->category->id == MOB_CATEGORY_PIKMIN &&
            game.states.gameplay->mobs.pikmin_list.size() >=
            game.config.max_pikmin_in_field
        ) {
            continue;
        }
        
        mobs_per_gen.push_back(
            create_mob(
                m_ptr->category, m_ptr->pos, m_ptr->type,
                m_ptr->angle, m_ptr->vars
            )
        );
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen number X
    //does not necessarily correspond to mob number X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) number.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        for(size_t l = 0; l < m_ptr->link_nrs.size(); ++l) {
            mobs_per_gen[m]->links.push_back(mobs_per_gen[m_ptr->link_nrs[l]]);
        }
    }
    
    //Save each path stop's sector.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.path_stops[s]->sector_ptr =
            get_sector(game.cur_area_data.path_stops[s]->pos, NULL, true);
    }
    
    //Sort leaders.
    sort(
        mobs.leaders.begin(), mobs.leaders.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t priority_l1 =
        find(
            game.config.leader_order.begin(),
            game.config.leader_order.end(), l1->lea_type
        ) -
        game.config.leader_order.begin();
        size_t priority_l2 =
        find(
            game.config.leader_order.begin(),
            game.config.leader_order.end(), l2->lea_type
        ) -
        game.config.leader_order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    cur_leader_nr = INVALID;
    cur_leader_ptr = NULL;
    starting_nr_of_leaders = mobs.leaders.size();
    
    if(!mobs.leaders.empty()) {
        change_to_next_leader(true, false, false);
    }
    
    if(cur_leader_ptr) {
        game.cam.set_pos(cur_leader_ptr->pos);
    } else {
        game.cam.set_pos(point());
    }
    game.cam.set_zoom(game.options.zoom_mid_level);
    
    cursor_save_timer.on_end = [this] () {
        cursor_save_timer.start();
        cursor_spots.push_back(game.mouse_cursor_s);
        if(cursor_spots.size() > GAMEPLAY::CURSOR_TRAIL_SAVE_N_SPOTS) {
            cursor_spots.erase(cursor_spots.begin());
        }
    };
    cursor_save_timer.start();
    
    if(cur_leader_ptr) {
        cur_leader_ptr->stop_whistling();
    }
    
    update_closest_group_members();
    
    //Memorize mobs required by the mission.
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        unordered_set<size_t> mission_required_mob_gen_idxs;
        if(game.cur_area_data.mission.goal_all_mobs) {
            MOB_CATEGORIES filter_cat = MOB_CATEGORY_NONE;
            switch(game.cur_area_data.mission.goal) {
            case MISSION_GOAL_COLLECT_TREASURE: {
                filter_cat = MOB_CATEGORY_TREASURES;
                break;
            } case MISSION_GOAL_BATTLE_ENEMIES: {
                filter_cat = MOB_CATEGORY_ENEMIES;
                break;
            } case MISSION_GOAL_GET_TO_EXIT: {
                filter_cat = MOB_CATEGORY_LEADERS;
                break;
            } default: {
                break;
            }
            }
            if(filter_cat != MOB_CATEGORY_NONE) {
                for(size_t m = 0; m < mobs_per_gen.size(); ++m) {
                    if(mobs_per_gen[m]->type->category->id != filter_cat) {
                        continue;
                    }
                    mission_required_mob_gen_idxs.insert(m);
                }
            }
        } else {
            mission_required_mob_gen_idxs =
                game.cur_area_data.mission.goal_mob_idxs;
        }
        
        for(size_t i : mission_required_mob_gen_idxs) {
            mission_required_mob_ids.insert(mobs_per_gen[i]->id);
        }
        
        mission_required_mob_amount = mission_required_mob_ids.size();
    }
    
    //Figure out the total amount of treasures and their points.
    for(size_t t = 0; t < mobs.treasures.size(); ++t) {
        treasures_total++;
        treasure_points_total +=
            mobs.treasures[t]->tre_type->points;
    }
    for(size_t e = 0; e < mobs.enemies.size(); ++e) {
        for(size_t s = 0; s < mobs.enemies[e]->specific_spoils.size(); ++s) {
            mob_type* s_type = mobs.enemies[e]->specific_spoils[s];
            if(s_type->category->id == MOB_CATEGORY_TREASURES) {
                treasures_total++;
                treasure_points_total +=
                    ((treasure_type*) s_type)->points;
            }
        }
    }
    for(size_t p = 0; p < mobs.piles.size(); ++p) {
        pile* p_ptr = mobs.piles[p];
        resource_type* res_type = p_ptr->pil_type->contents;
        if(
            res_type->delivery_result !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasures_total += p_ptr->amount;
        treasure_points_total +=
            p_ptr->amount * res_type->point_amount;
    }
    
    //Figure out the total amount of enemies and their points.
    enemy_total = mobs.enemies.size();
    for(size_t e = 0; e < mobs.enemies.size(); ++e) {
        enemy_points_total += mobs.enemies[e]->ene_type->points;
    }
    
    //Initialize some other things.
    path_mgr.handle_area_load();
    
    init_hud();
    
    map<string, string> spray_strs =
        get_var_map(game.cur_area_data.spray_amounts);
        
    for(auto &s : spray_strs) {
        size_t spray_id = 0;
        for(; spray_id < game.spray_types.size(); ++spray_id) {
            if(game.spray_types[spray_id].name == s.first) {
                break;
            }
        }
        if(spray_id == game.spray_types.size()) {
            log_error(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.cur_area_data.name + "\"!", NULL
            );
            continue;
        }
        
        spray_stats[spray_id].nr_sprays = s2i(s.second);
    }
    
    for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
        if(game.options.controls[0][c].action == BUTTON_THROW) {
            main_control_id = c;
            break;
        }
    }
    for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
        if(game.options.controls[0][c].action == BUTTON_WHISTLE) {
            cancel_control_id = c;
            break;
        }
    }
    
    //Effect caches.
    game.liquid_limit_effect_caches.clear();
    game.liquid_limit_effect_caches.insert(
        game.liquid_limit_effect_caches.begin(),
        game.cur_area_data.edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.liquid_limit_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data.vertexes.begin(),
            game.cur_area_data.vertexes.end()
        ),
        does_edge_have_liquid_limit,
        get_liquid_limit_length,
        get_liquid_limit_color
    );
    game.wall_smoothing_effect_caches.clear();
    game.wall_smoothing_effect_caches.insert(
        game.wall_smoothing_effect_caches.begin(),
        game.cur_area_data.edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_smoothing_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data.vertexes.begin(),
            game.cur_area_data.vertexes.end()
        ),
        does_edge_have_ledge_smoothing,
        get_ledge_smoothing_length,
        get_ledge_smoothing_color
    );
    game.wall_shadow_effect_caches.clear();
    game.wall_shadow_effect_caches.insert(
        game.wall_shadow_effect_caches.begin(),
        game.cur_area_data.edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_shadow_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data.vertexes.begin(),
            game.cur_area_data.vertexes.end()
        ),
        does_edge_have_wall_shadow,
        get_wall_shadow_length,
        get_wall_shadow_color
    );
    
    //TODO Uncomment this when replays are implemented.
    /*
    replay_timer = timer(
        GAMEPLAY::REPLAY_SAVE_FREQUENCY,
    [this] () {
        this->replay_timer.start();
        vector<mob*> obstacles; //TODO
        gameplay_replay.add_state(
            leaders, pikmin_list, enemies, treasures, onions, obstacles,
            cur_leader_nr
        );
    }
    );
    replay_timer.start();
    gameplay_replay.clear();*/
    
    //Report any errors with the loading process.
    if(game.errors_reported_so_far > errors_reported_at_start) {
        print_info(
            "\n\n\nERRORS FOUND!\n"
            "See \"" + ERROR_LOG_FILE_PATH + "\".\n\n\n",
            20.0f, 3.0f
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->set_area_name(game.cur_area_data.name);
        game.perf_mon->leave_state();
    }
    
    enter();
}


/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void gameplay_state::load_game_content() {
    load_custom_particle_generators(true);
    load_liquids(true);
    load_status_types(true);
    load_spray_types(true);
    load_hazards();
    load_weather();
    load_spike_damage_types();
    
    //Mob types.
    load_mob_types(true);
    
    //Register leader sub-group types.
    for(size_t p = 0; p < game.config.pikmin_order.size(); ++p) {
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, game.config.pikmin_order[p],
            game.config.pikmin_order[p]->bmp_icon
        );
    }
    
    vector<string> tool_types_vector;
    for(auto &t : game.mob_types.tool) {
        tool_types_vector.push_back(t.first);
    }
    sort(tool_types_vector.begin(), tool_types_vector.end());
    for(size_t t = 0; t < tool_types_vector.size(); ++t) {
        tool_type* tt_ptr = game.mob_types.tool[tool_types_vector[t]];
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, tt_ptr, tt_ptr->bmp_icon
        );
    }
    
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_LEADER);
}


/* ----------------------------------------------------------------------------
 * Starts the fade out to leave the gameplay state.
 * target:
 *   Where to leave to.
 */
void gameplay_state::start_leaving(const LEAVE_TARGET target) {
    game.fade_mgr.start_fade( false, [this, target] () { leave(target); });
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay_state::unload() {
    unloading = true;
    
    al_show_mouse_cursor(game.display);
    
    if(hud) {
        hud->gui.destroy();
        delete hud;
        hud = NULL;
    }
    
    cur_leader_nr = INVALID;
    cur_leader_ptr = NULL;
    
    close_to_interactable_to_use = NULL;
    close_to_nest_to_open = NULL;
    close_to_pikmin_to_pluck = NULL;
    close_to_ship_to_heal = NULL;
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    while(!mobs.all.empty()) {
        delete_mob(*mobs.all.begin(), true);
    }
    
    if(lightmap_bmp) {
        al_destroy_bitmap(lightmap_bmp);
        lightmap_bmp = NULL;
    }
    
    unload_area();
    
    mission_required_mob_ids.clear();
    
    path_mgr.clear();
    spray_stats.clear();
    particles.clear();
    
    leader_movement.reset(); //TODO replace with a better solution.
    
    unload_game_content();
    
    if(bmp_fog) {
        al_destroy_bitmap(bmp_fog);
        bmp_fog = NULL;
    }
    
    if(msg_box) {
        delete msg_box;
        msg_box = NULL;
    }
    if(onion_menu) {
        delete onion_menu;
        onion_menu = NULL;
    }
    if(pause_menu) {
        delete pause_menu;
        pause_menu = NULL;
    }
    game.maker_tools.info_print_text.clear();
    
    unloading = false;
}


/* ----------------------------------------------------------------------------
 * Unloads loaded game content.
 */
void gameplay_state::unload_game_content() {
    unload_weather();
    
    subgroup_types.clear();
    
    unload_mob_types(true);
    
    unload_spike_damage_types();
    unload_hazards();
    unload_spray_types();
    unload_status_types(true);
    unload_liquids();
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Updates the list of leaders available to be controlled.
 */
void gameplay_state::update_available_leaders() {
    //Build the list.
    available_leaders.clear();
    for(size_t l = 0; l < mobs.leaders.size(); ++l) {
        if(mobs.leaders[l]->health <= 0.0f) continue;
        if(mobs.leaders[l]->to_delete) continue;
        available_leaders.push_back(mobs.leaders[l]);
    }
    
    if(available_leaders.empty()) {
        return;
    }
    
    //Sort it so that it follows the expected leader order.
    //If there are multiple leaders of the same type, leaders with a lower
    //mob ID number come first.
    std::sort(
        available_leaders.begin(), available_leaders.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t l1_order_idx = INVALID;
        size_t l2_order_idx = INVALID;
        for(size_t t = 0; t < game.config.leader_order.size(); t++) {
            if(game.config.leader_order[t] == l1->type) l1_order_idx = t;
            if(game.config.leader_order[t] == l2->type) l2_order_idx = t;
        }
        if(l1_order_idx == l2_order_idx) {
            return l1->id < l2->id;
        }
        return l1_order_idx < l2_order_idx;
    }
    );
    
    //Update the current leader's index, which could've changed.
    for(size_t l = 0; l < available_leaders.size(); ++l) {
        if(available_leaders[l] == cur_leader_ptr) {
            cur_leader_nr = l;
            break;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Updates the variables that indicate what the closest
 * group member of the standby subgroup is, for the current
 * standby subgroup, the previous, and the next.
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * Sets to NULL if there is no member of that subgroup available.
 */
void gameplay_state::update_closest_group_members() {
    closest_group_member[BUBBLE_PREVIOUS] = NULL;
    closest_group_member[BUBBLE_CURRENT] = NULL;
    closest_group_member[BUBBLE_NEXT] = NULL;
    closest_group_member_distant = false;
    
    if(!cur_leader_ptr) return;
    if(cur_leader_ptr->group->members.empty()) return;
    
    //Get the closest group members for the three relevant subgroup types.
    subgroup_type* prev_type;
    cur_leader_ptr->group->get_next_standby_type(true, &prev_type);
    
    if(prev_type) {
        closest_group_member[BUBBLE_PREVIOUS] =
            get_closest_group_member(prev_type);
    }
    
    if(cur_leader_ptr->group->cur_standby_type) {
        closest_group_member[BUBBLE_CURRENT] =
            get_closest_group_member(cur_leader_ptr->group->cur_standby_type);
    }
    
    subgroup_type* next_type;
    cur_leader_ptr->group->get_next_standby_type(false, &next_type);
    
    if(next_type) {
        closest_group_member[BUBBLE_NEXT] =
            get_closest_group_member(next_type);
    }
    
    //Update whether the current subgroup type's closest member is distant.
    if(!closest_group_member[BUBBLE_CURRENT]) {
        return;
    }
    
    //Figure out if it can be reached, or if it's too distant.
    if(
        cur_leader_ptr->ground_sector &&
        !cur_leader_ptr->ground_sector->hazards.empty()
    ) {
        if(
            !closest_group_member[BUBBLE_CURRENT]->
            is_resistant_to_hazards(
                cur_leader_ptr->ground_sector->hazards
            )
        ) {
            //The leader is on a hazard that the member isn't resistent to.
            //Don't let the leader grab it.
            closest_group_member_distant = true;
        }
    }
    
    if(
        dist(
            closest_group_member[BUBBLE_CURRENT]->pos,
            cur_leader_ptr->pos
        ) >
        game.config.group_member_grab_range
    ) {
        //The group member is physically too far away.
        closest_group_member_distant = true;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void gameplay_state::update_transformations() {
    //World coordinates to screen coordinates.
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + game.win_w / 2.0 / game.cam.zoom,
        -game.cam.pos.y + game.win_h / 2.0 / game.cam.zoom
    );
    al_scale_transform(
        &game.world_to_screen_transform, game.cam.zoom, game.cam.zoom
    );
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}
