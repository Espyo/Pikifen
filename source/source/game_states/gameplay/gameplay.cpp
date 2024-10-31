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
#include "../../libs/data_file.h"
#include "../../load.h"
#include "../../misc_structs.h"
#include "../../mobs/converter.h"
#include "../../mobs/pile.h"
#include "../../mobs/resource.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"


namespace GAMEPLAY {

//How long the HUD moves for when the area is entered.
const float AREA_INTRO_HUD_MOVE_TIME = 3.0f;

//How long it takes for the area name to fade away, in-game.
const float AREA_TITLE_FADE_DURATION = 1.0f;

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

//Distance between current leader and boss before the boss music kicks in.
const float BOSS_MUSIC_DISTANCE = 300.0f;

//Name of the boss theme song.
const string BOSS_SONG_NAME = "boss";

//Name of the boss victory theme song.
const string BOSS_VICTORY_SONG_NAME = "boss_victory";

//Something is only considered off-camera if it's beyond this extra margin.
const float CAMERA_BOX_MARGIN = 128.0f;

//Dampen the camera's movements by this much.
const float CAMERA_SMOOTHNESS_MULT = 4.5f;

//Opacity of the collision bubbles in the maker tool.
const unsigned char COLLISION_OPACITY = 192;

//If an enemy is this close to the active leader, turn on the song's enemy mix.
const float ENEMY_MIX_DISTANCE = 150.0f;

//Width and height of the fog bitmap.
const int FOG_BITMAP_SIZE = 128;

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


/**
 * @brief Changes the amount of sprays of a certain type the player owns.
 * It also animates the correct HUD item, if any.
 *
 * @param type_idx Index number of the spray type.
 * @param amount Amount to change by.
 */
void gameplay_state::change_spray_count(
    size_t type_idx, signed int amount
) {
    spray_stats[type_idx].nr_sprays =
        std::max(
            (signed int) spray_stats[type_idx].nr_sprays + amount,
            (signed int) 0
        );
        
    gui_item* spray_hud_item = nullptr;
    if(game.content.spray_types.list.size() > 2) {
        if(selected_spray == type_idx) {
            spray_hud_item = hud->spray_1_amount;
        }
    } else {
        if(type_idx == 0) {
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


/**
 * @brief Draws the gameplay.
 */
void gameplay_state::do_drawing() {
    do_game_drawing();
    
    if(game.perf_mon) {
        game.perf_mon->leave_state();
    }
}


/**
 * @brief Tick the gameplay logic by one frame.
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
    
    float regular_delta_t = game.delta_t;
    
    if(game.maker_tools.change_speed) {
        game.delta_t *= game.maker_tools.change_speed_mult;
    }
    
    //Controls.
    vector<player_action> player_actions = game.controls.new_frame();
    for(size_t a = 0; a < player_actions.size(); a++) {
        handle_player_action(player_actions[a]);
        if(onion_menu) onion_menu->handle_player_action(player_actions[a]);
        if(pause_menu) pause_menu->handle_player_action(player_actions[a]);
    }
    
    //Game logic.
    if(!paused) {
        game.statistics.gameplay_time += regular_delta_t;
        do_gameplay_logic(game.delta_t* delta_t_mult);
    }
    do_menu_logic();
    do_aesthetic_logic(game.delta_t* delta_t_mult);
}


/**
 * @brief Ends the currently ongoing mission.
 *
 * @param cleared Did the player reach the goal?
 */
void gameplay_state::end_mission(bool cleared) {
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
            game.mission_goals[game.cur_area_data->mission.goal];
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


/**
 * @brief Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void gameplay_state::enter() {
    update_transformations();
    
    last_enemy_killed_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_hurt_leader_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_pikmin_born_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_pikmin_death_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    last_ship_that_got_treasure_pos = point(LARGE_FLOAT, LARGE_FLOAT);
    
    mission_fail_reason = (MISSION_FAIL_COND) INVALID;
    goal_indicator_ratio = 0.0f;
    fail_1_indicator_ratio = 0.0f;
    fail_2_indicator_ratio = 0.0f;
    score_indicator = 0.0f;
    
    paused = false;
    cur_interlude = INTERLUDE_READY;
    interlude_time = 0.0f;
    cur_big_msg = BIG_MESSAGE_READY;
    big_msg_time = 0.0f;
    delta_t_mult = 0.5f;
    boss_music_state = BOSS_MUSIC_STATE_NEVER_PLAYED;
    
    if(!game.states.area_ed->quick_play_area_path.empty()) {
        //If this is an area editor quick play, skip the "Ready..." interlude.
        interlude_time = GAMEPLAY::BIG_MSG_READY_DUR;
        big_msg_time = GAMEPLAY::BIG_MSG_READY_DUR;
    }
    
    hud->gui.hide_items();
    if(went_to_results) {
        game.fade_mgr.start_fade(true, nullptr);
        if(pause_menu) {
            pause_menu->to_delete = true;
        }
    }
    
    ready_for_input = false;
    
    game.mouse_cursor.reset();
    leader_cursor_w = game.mouse_cursor.w_pos;
    leader_cursor_s = game.mouse_cursor.s_pos;
    
    notification.reset();
    
    if(cur_leader_ptr) {
        cur_leader_ptr->stop_whistling();
    }
    update_closest_group_members();
}


/**
 * @brief Generates the bitmap that'll draw the fog fade effect.
 *
 * @param near_radius Until this radius, the fog is not present.
 * @param far_radius From this radius on, the fog is fully dense.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* gameplay_state::generate_fog_bitmap(
    float near_radius, float far_radius
) {
    if(far_radius == 0) return nullptr;
    
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
    
    for(int y = 0; y < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); y++) {
        for(int x = 0; x < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); x++) {
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


/**
 * @brief Returns how many Pikmin are in the field in the current area.
 * This also checks inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t gameplay_state::get_amount_of_field_pikmin(const pikmin_type* filter) {
    size_t total = 0;
    
    //Check the Pikmin mobs.
    for(size_t p = 0; p < mobs.pikmin_list.size(); p++) {
        pikmin* p_ptr = mobs.pikmin_list[p];
        if(filter && p_ptr->pik_type != filter) continue;
        total++;
    }
    
    //Check Pikmin inside converters.
    for(size_t c = 0; c < mobs.converters.size(); c++) {
        converter* c_ptr = mobs.converters[c];
        if(filter && c_ptr->current_type != filter) continue;
        total += c_ptr->amount_in_buffer;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are in the group.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t gameplay_state::get_amount_of_group_pikmin(const pikmin_type* filter) {
    size_t total = 0;
    
    if(!cur_leader_ptr) return 0;
    
    for(size_t m = 0; m < cur_leader_ptr->group->members.size(); m++) {
        mob* m_ptr = cur_leader_ptr->group->members[m];
        if(m_ptr->type->category->id != MOB_CATEGORY_PIKMIN) continue;
        if(filter && m_ptr->type != filter) continue;
        total++;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are idling in the area.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t gameplay_state::get_amount_of_idle_pikmin(const pikmin_type* filter) {
    size_t total = 0;
    
    for(size_t p = 0; p < mobs.pikmin_list.size(); p++) {
        pikmin* p_ptr = mobs.pikmin_list[p];
        if(filter && p_ptr->type != filter) continue;
        if(
            p_ptr->fsm.cur_state->id == PIKMIN_STATE_IDLING ||
            p_ptr->fsm.cur_state->id == PIKMIN_STATE_IDLING_H
        ) {
            total++;
        }
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are inside of Onions in the current area.
 * This also checks ships.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long gameplay_state::get_amount_of_onion_pikmin(const pikmin_type* filter) {
    long total = 0;
    
    //Check Onions proper.
    for(size_t o = 0; o < mobs.onions.size(); o++) {
        onion* o_ptr = mobs.onions[o];
        for(
            size_t t = 0;
            t < o_ptr->oni_type->nest->pik_types.size();
            t++
        ) {
            if(filter && o_ptr->oni_type->nest->pik_types[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < NR_MATURITIES; m++) {
                total += o_ptr->nest->pikmin_inside[t][m];
            }
        }
    }
    
    //Check ships.
    for(size_t s = 0; s < mobs.ships.size(); s++) {
        ship* s_ptr = mobs.ships[s];
        if(!s_ptr->nest) continue;
        for(
            size_t t = 0;
            t < s_ptr->shi_type->nest->pik_types.size();
            t++
        ) {
            if(filter && s_ptr->shi_type->nest->pik_types[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < NR_MATURITIES; m++) {
                total += s_ptr->nest->pikmin_inside[t][m];
            }
        }
    }
    return total;
}


/**
 * @brief Returns the total amount of Pikmin the player has.
 * This includes Pikmin in the field as well as the Onions, and also
 * Pikmin inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long gameplay_state::get_amount_of_total_pikmin(const pikmin_type* filter) {
    long total = 0;
    
    //Check Pikmin in the field and inside converters.
    total += get_amount_of_field_pikmin(filter);
    
    //Check Pikmin inside Onions and ships.
    total += get_amount_of_onion_pikmin(filter);
    
    //Return the final sum.
    return total;
}


/**
 * @brief Returns the closest group member of a given standby subgroup.
 * In the case all candidate members are out of reach,
 * this returns the closest. Otherwise, it returns the closest
 * and more mature one.
 *
 * @param type Type to search for.
 * @return The member, or nullptr if there is no member of that subgroup available.
 */
mob* gameplay_state::get_closest_group_member(const subgroup_type* type) {
    if(!cur_leader_ptr) return nullptr;
    
    mob* result = nullptr;
    
    //Closest members so far for each maturity.
    dist closest_dists[NR_MATURITIES];
    mob* closest_ptrs[NR_MATURITIES];
    for(unsigned char m = 0; m < NR_MATURITIES; m++) {
        closest_ptrs[m] = nullptr;
    }
    
    //Fetch the closest, for each maturity.
    size_t n_members = cur_leader_ptr->group->members.size();
    for(size_t m = 0; m < n_members; m++) {
    
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
    for(unsigned char m = 0; m < NR_MATURITIES; m++) {
        if(!closest_ptrs[2 - m]) continue;
        if(closest_dists[2 - m] > game.config.group_member_grab_range) continue;
        result = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!result) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < NR_MATURITIES; m++) {
            if(!closest_ptrs[m]) continue;
            
            if(!result || closest_dists[m] < closest_dist) {
                result = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
    }
    
    return result;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string gameplay_state::get_name() const {
    return "gameplay";
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
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
        game.controls.release_all();
    }
    
    //Finally, let the HUD handle events.
    hud->gui.handle_event(ev);
    
}


/**
 * @brief Initializes the HUD.
 */
void gameplay_state::init_hud() {
    hud = new hud_t();
}


/**
 * @brief Leaves the gameplay state and enters the main menu,
 * or area selection, or etc.
 *
 * @param target Where to leave to.
 */
void gameplay_state::leave(const GAMEPLAY_LEAVE_TARGET target) {
    if(unloading) return;
    
    if(game.perf_mon) {
        //Don't register the final frame, since it won't draw anything.
        game.perf_mon->set_paused(true);
    }
    
    game.audio.stop_all_playbacks();
    game.audio.set_current_song("");
    boss_music_state = BOSS_MUSIC_STATE_NEVER_PLAYED;
    save_statistics();
    
    switch(target) {
    case GAMEPLAY_LEAVE_TARGET_RETRY: {
        game.change_state(game.states.gameplay);
        break;
    } case GAMEPLAY_LEAVE_TARGET_END: {
        went_to_results = true;
        //Change state, but don't unload this one, since the player
        //may pick the "keep playing" option in the results screen.
        game.change_state(game.states.results, false);
        break;
    } case GAMEPLAY_LEAVE_TARGET_AREA_SELECT: {
        if(game.states.area_ed->quick_play_area_path.empty()) {
            game.states.area_menu->area_type = game.cur_area_data->type;
            game.change_state(game.states.area_menu);
        } else {
            game.change_state(game.states.area_ed);
        }
        break;
    }
    }
}


/**
 * @brief Loads the "gameplay" state into memory.
 */
void gameplay_state::load() {
    if(game.perf_mon) {
        game.perf_mon->reset();
        game.perf_mon->enter_state(PERF_MON_STATE_LOADING);
        game.perf_mon->set_paused(false);
    }
    
    loading = true;
    game.errors.prepare_area_load();
    went_to_results = false;
    
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    game.statistics.area_entries++;
    
    //Game content.
    load_game_content();
    
    //Initialize some important things.
    for(size_t s = 0; s < game.content.spray_types.list.size(); s++) {
        spray_stats.push_back(spray_stats_t());
    }
    
    area_time_passed = 0.0f;
    gameplay_time_passed = 0.0f;
    game.maker_tools.reset_for_gameplay();
    area_title_fade_timer.start();
    
    after_hours = false;
    pikmin_born = 0;
    pikmin_deaths = 0;
    treasures_collected = 0;
    treasures_total = 0;
    goal_treasures_collected = 0;
    goal_treasures_total = 0;
    treasure_points_collected = 0;
    treasure_points_total = 0;
    enemy_deaths = 0;
    enemy_total = 0;
    enemy_points_collected = 0;
    enemy_points_total = 0;
    cur_leaders_in_mission_exit = 0;
    mission_required_mob_amount = 0;
    mission_score = 0;
    old_mission_score = 0;
    old_mission_goal_cur = 0;
    old_mission_fail_1_cur = 0;
    old_mission_fail_2_cur = 0;
    nr_living_leaders = 0;
    leaders_kod = 0;
    
    game.framerate_last_avg_point = 0;
    game.framerate_history.clear();
    
    boss_music_state = BOSS_MUSIC_STATE_NEVER_PLAYED;
    game.audio.set_current_song("");
    game.audio.on_song_finished = [this] (const string &name) {
        if(name == GAMEPLAY::BOSS_VICTORY_SONG_NAME) {
            switch(boss_music_state) {
            case BOSS_MUSIC_STATE_VICTORY: {
                game.audio.set_current_song(game.cur_area_data->song_name, false);
                boss_music_state = BOSS_MUSIC_STATE_PAUSED;
            } default: {
                break;
            }
            }
        }
    };
    
    //Load the area.
    string area_folder_name;
    AREA_TYPE area_type;
    string package;
    get_area_info_from_path(
        path_of_area_to_load, &area_folder_name, &area_type, &package
    );
    game.content.load_area_as_current(
        area_folder_name, package, area_type, CONTENT_LOAD_LEVEL_FULL, false
    );
    
    if(!game.cur_area_data->weather_condition.blackout_strength.empty()) {
        lightmap_bmp = al_create_bitmap(game.win_w, game.win_h);
    }
    if(!game.cur_area_data->weather_condition.fog_color.empty()) {
        bmp_fog =
            generate_fog_bitmap(
                game.cur_area_data->weather_condition.fog_near,
                game.cur_area_data->weather_condition.fog_far
            );
    }
    
    //Generate mobs.
    next_mob_id = 0;
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object generation");
    }
    
    vector<mob*> mobs_per_gen;
    
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        mob_gen* m_ptr = game.cur_area_data->mob_generators[m];
        bool valid = true;
        
        if(!m_ptr->type) {
            valid = false;
        } else if(
            m_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
            game.states.gameplay->mobs.pikmin_list.size() >=
            game.config.max_pikmin_in_field
        ) {
            valid = false;
        }
        
        if(valid) {
            mob* new_mob =
                create_mob(
                    m_ptr->type->category, m_ptr->pos, m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                );
            mobs_per_gen.push_back(new_mob);
        } else {
            mobs_per_gen.push_back(nullptr);
        }
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen index X
    //does not necessarily correspond to mob index X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) index.
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        mob_gen* gen_ptr = game.cur_area_data->mob_generators[m];
        mob* mob_ptr = mobs_per_gen[m];
        if(!mob_ptr) continue;
        
        for(size_t l = 0; l < gen_ptr->link_idxs.size(); l++) {
            size_t link_target_gen_idx = gen_ptr->link_idxs[l];
            mob* link_target_mob_ptr = mobs_per_gen[link_target_gen_idx];
            mob_ptr->links.push_back(link_target_mob_ptr);
        }
    }
    
    //Mobs stored inside other. Same logic as mob links.
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        mob_gen* holdee_gen_ptr = game.cur_area_data->mob_generators[m];
        if(holdee_gen_ptr->stored_inside == INVALID) continue;
        mob* holdee_ptr = mobs_per_gen[m];
        mob* holder_mob_ptr = mobs_per_gen[holdee_gen_ptr->stored_inside];
        holder_mob_ptr->store_mob_inside(holdee_ptr);
    }
    
    //Save each path stop's sector.
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        game.cur_area_data->path_stops[s]->sector_ptr =
            get_sector(game.cur_area_data->path_stops[s]->pos, nullptr, true);
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
    
    //In case a leader is stored in another mob,
    //update the available list.
    update_available_leaders();
    
    cur_leader_idx = INVALID;
    cur_leader_ptr = nullptr;
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
    
    //Memorize mobs required by the mission.
    if(game.cur_area_data->type == AREA_TYPE_MISSION) {
        unordered_set<size_t> mission_required_mob_gen_idxs;
        
        if(game.cur_area_data->mission.goal_all_mobs) {
            for(size_t m = 0; m < mobs_per_gen.size(); m++) {
                if(
                    mobs_per_gen[m] &&
                    game.mission_goals[game.cur_area_data->mission.goal]->
                    is_mob_applicable(mobs_per_gen[m]->type)
                ) {
                    mission_required_mob_gen_idxs.insert(m);
                }
            }
            
        } else {
            mission_required_mob_gen_idxs =
                game.cur_area_data->mission.goal_mob_idxs;
        }
        
        for(size_t i : mission_required_mob_gen_idxs) {
            mission_remaining_mob_ids.insert(mobs_per_gen[i]->id);
        }
        mission_required_mob_amount = mission_remaining_mob_ids.size();
        
        if(game.cur_area_data->mission.goal == MISSION_GOAL_COLLECT_TREASURE) {
            //Since the collect treasure goal can accept piles and resources
            //meant to add treasure points, we'll need some special treatment.
            for(size_t i : mission_required_mob_gen_idxs) {
                if(
                    mobs_per_gen[i]->type->category->id ==
                    MOB_CATEGORY_PILES
                ) {
                    pile* pil_ptr = (pile*) mobs_per_gen[i];
                    goal_treasures_total += pil_ptr->amount;
                } else {
                    goal_treasures_total++;
                }
            }
        }
    }
    
    //Figure out the total amount of treasures and their points.
    for(size_t t = 0; t < mobs.treasures.size(); t++) {
        treasures_total++;
        treasure_points_total +=
            mobs.treasures[t]->tre_type->points;
    }
    for(size_t p = 0; p < mobs.piles.size(); p++) {
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
    for(size_t r = 0; r < mobs.resources.size(); r++) {
        resource* r_ptr = mobs.resources[r];
        if(
            r_ptr->res_type->delivery_result !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasures_total++;
        treasure_points_total += r_ptr->res_type->point_amount;
    }
    
    //Figure out the total amount of enemies and their points.
    enemy_total = mobs.enemies.size();
    for(size_t e = 0; e < mobs.enemies.size(); e++) {
        enemy_points_total += mobs.enemies[e]->ene_type->points;
    }
    
    //Initialize the area's active cells.
    float area_width =
        game.cur_area_data->bmap.n_cols * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    float area_height =
        game.cur_area_data->bmap.n_rows * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    size_t nr_area_cell_cols =
        ceil(area_width / GEOMETRY::AREA_CELL_SIZE) + 1;
    size_t nr_area_cell_rows =
        ceil(area_height / GEOMETRY::AREA_CELL_SIZE) + 1;
        
    area_active_cells.clear();
    area_active_cells.assign(
        nr_area_cell_cols, vector<bool>(nr_area_cell_rows, false)
    );
    
    //Initialize some other things.
    path_mgr.handle_area_load();
    
    init_hud();
    
    day_minutes = game.cur_area_data->day_time_start;
    
    map<string, string> spray_strs =
        get_var_map(game.cur_area_data->spray_amounts);
        
    for(auto &s : spray_strs) {
        size_t spray_idx = 0;
        for(; spray_idx < game.config.spray_order.size(); spray_idx++) {
            if(game.config.spray_order[spray_idx]->internal_name == s.first) {
                break;
            }
        }
        if(spray_idx == game.content.spray_types.list.size()) {
            game.errors.report(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.cur_area_data->name + "\"!", nullptr
            );
            continue;
        }
        
        spray_stats[spray_idx].nr_sprays = s2i(s.second);
    }
    
    //Effect caches.
    game.liquid_limit_effect_caches.clear();
    game.liquid_limit_effect_caches.insert(
        game.liquid_limit_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.liquid_limit_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
        ),
        does_edge_have_liquid_limit,
        get_liquid_limit_length,
        get_liquid_limit_color
    );
    game.wall_smoothing_effect_caches.clear();
    game.wall_smoothing_effect_caches.insert(
        game.wall_smoothing_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_smoothing_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
        ),
        does_edge_have_ledge_smoothing,
        get_ledge_smoothing_length,
        get_ledge_smoothing_color
    );
    game.wall_shadow_effect_caches.clear();
    game.wall_shadow_effect_caches.insert(
        game.wall_shadow_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_shadow_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
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
            cur_leader_idx
        );
    }
    );
    replay_timer.start();
    gameplay_replay.clear();*/
    
    //Report any errors with the loading process.
    game.errors.report_area_load_errors();
    
    if(game.perf_mon) {
        game.perf_mon->set_area_name(game.cur_area_data->name);
        game.perf_mon->leave_state();
    }
    
    enter();
    
    loading = false;
}


/**
 * @brief Loads all of the game's content.
 */
void gameplay_state::load_game_content() {
    game.content.load_all(CONTENT_TYPE_GUI, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_CUSTOM_PARTICLE_GEN, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_GLOBAL_ANIMATION, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_LIQUID, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_STATUS_TYPE, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_SPRAY_TYPE, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_HAZARD, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_WEATHER_CONDITION, CONTENT_LOAD_LEVEL_FULL);
    game.content.load_all(CONTENT_TYPE_SPIKE_DAMAGE_TYPE, CONTENT_LOAD_LEVEL_FULL);
    
    //Mob types.
    game.content.load_all(CONTENT_TYPE_MOB_TYPE, CONTENT_LOAD_LEVEL_FULL);
    
    //Register leader sub-group types.
    for(size_t p = 0; p < game.config.pikmin_order.size(); p++) {
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, game.config.pikmin_order[p],
            game.config.pikmin_order[p]->bmp_icon
        );
    }
    
    vector<string> tool_types_vector;
    for(auto &t : game.content.mob_types.list.tool) {
        tool_types_vector.push_back(t.first);
    }
    sort(tool_types_vector.begin(), tool_types_vector.end());
    for(size_t t = 0; t < tool_types_vector.size(); t++) {
        tool_type* tt_ptr = game.content.mob_types.list.tool[tool_types_vector[t]];
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, tt_ptr, tt_ptr->bmp_icon
        );
    }
    
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_LEADER);
}


/**
 * @brief Starts the fade out to leave the gameplay state.
 *
 * @param target Where to leave to.
 */
void gameplay_state::start_leaving(const GAMEPLAY_LEAVE_TARGET target) {
    game.fade_mgr.start_fade( false, [this, target] () { leave(target); });
}


/**
 * @brief Unloads the "gameplay" state from memory.
 */
void gameplay_state::unload() {
    unloading = true;
    
    if(hud) {
        hud->gui.destroy();
        delete hud;
        hud = nullptr;
    }
    
    cur_leader_idx = INVALID;
    cur_leader_ptr = nullptr;
    
    close_to_interactable_to_use = nullptr;
    close_to_nest_to_open = nullptr;
    close_to_pikmin_to_pluck = nullptr;
    close_to_ship_to_heal = nullptr;
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    while(!mobs.all.empty()) {
        delete_mob(*mobs.all.begin(), true);
    }
    
    if(lightmap_bmp) {
        al_destroy_bitmap(lightmap_bmp);
        lightmap_bmp = nullptr;
    }
    
    mission_remaining_mob_ids.clear();
    path_mgr.clear();
    spray_stats.clear();
    particles.clear();
    
    leader_movement.reset(); //TODO replace with a better solution.
    
    unload_game_content();
    game.content.unload_current_area(CONTENT_LOAD_LEVEL_FULL);
    
    if(bmp_fog) {
        al_destroy_bitmap(bmp_fog);
        bmp_fog = nullptr;
    }
    
    if(msg_box) {
        delete msg_box;
        msg_box = nullptr;
    }
    if(onion_menu) {
        delete onion_menu;
        onion_menu = nullptr;
    }
    if(pause_menu) {
        delete pause_menu;
        pause_menu = nullptr;
    }
    game.maker_tools.info_print_text.clear();
    
    unloading = false;
}


/**
 * @brief Unloads loaded game content.
 */
void gameplay_state::unload_game_content() {
    subgroup_types.clear();
    
    game.content.unload_all(CONTENT_TYPE_WEATHER_CONDITION);
    game.content.unload_all(CONTENT_TYPE_MOB_TYPE);
    game.content.unload_all(CONTENT_TYPE_SPIKE_DAMAGE_TYPE);
    game.content.unload_all(CONTENT_TYPE_HAZARD);
    game.content.unload_all(CONTENT_TYPE_SPRAY_TYPE);
    game.content.unload_all(CONTENT_TYPE_STATUS_TYPE);
    game.content.unload_all(CONTENT_TYPE_LIQUID);
    game.content.unload_all(CONTENT_TYPE_GLOBAL_ANIMATION);
    game.content.unload_all(CONTENT_TYPE_CUSTOM_PARTICLE_GEN);
    game.content.unload_all(CONTENT_TYPE_GUI);
}


/**
 * @brief Updates the list of leaders available to be controlled.
 */
void gameplay_state::update_available_leaders() {
    //Build the list.
    available_leaders.clear();
    for(size_t l = 0; l < mobs.leaders.size(); l++) {
        if(mobs.leaders[l]->health <= 0.0f) continue;
        if(mobs.leaders[l]->to_delete) continue;
        if(mobs.leaders[l]->is_stored_inside_mob()) continue;
        available_leaders.push_back(mobs.leaders[l]);
    }
    
    if(available_leaders.empty()) {
        return;
    }
    
    //Sort it so that it follows the expected leader order.
    //If there are multiple leaders of the same type, leaders with a lower
    //mob index number come first.
    std::sort(
        available_leaders.begin(), available_leaders.end(),
    [] (const leader * l1, const leader * l2) -> bool {
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
    for(size_t l = 0; l < available_leaders.size(); l++) {
        if(available_leaders[l] == cur_leader_ptr) {
            cur_leader_idx = l;
            break;
        }
    }
}


/**
 * @brief Updates the variables that indicate what the closest
 * group member of the standby subgroup is, for the current
 * standby subgroup, the previous, and the next.
 *
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * Sets to nullptr if there is no member of that subgroup available.
 */
void gameplay_state::update_closest_group_members() {
    closest_group_member[BUBBLE_RELATION_PREVIOUS] = nullptr;
    closest_group_member[BUBBLE_RELATION_CURRENT] = nullptr;
    closest_group_member[BUBBLE_RELATION_NEXT] = nullptr;
    closest_group_member_distant = false;
    
    if(!cur_leader_ptr) return;
    if(cur_leader_ptr->group->members.empty()) {
        cur_leader_ptr->update_throw_variables();
        return;
    }
    
    //Get the closest group members for the three relevant subgroup types.
    subgroup_type* prev_type;
    cur_leader_ptr->group->get_next_standby_type(true, &prev_type);
    
    if(prev_type) {
        closest_group_member[BUBBLE_RELATION_PREVIOUS] =
            get_closest_group_member(prev_type);
    }
    
    if(cur_leader_ptr->group->cur_standby_type) {
        closest_group_member[BUBBLE_RELATION_CURRENT] =
            get_closest_group_member(cur_leader_ptr->group->cur_standby_type);
    }
    
    subgroup_type* next_type;
    cur_leader_ptr->group->get_next_standby_type(false, &next_type);
    
    if(next_type) {
        closest_group_member[BUBBLE_RELATION_NEXT] =
            get_closest_group_member(next_type);
    }
    
    //Update whether the current subgroup type's closest member is distant.
    if(!closest_group_member[BUBBLE_RELATION_CURRENT]) {
        return;
    }
    
    //Figure out if it can be reached, or if it's too distant.
    if(
        cur_leader_ptr->ground_sector &&
        !cur_leader_ptr->standing_on_mob &&
        !cur_leader_ptr->ground_sector->hazards.empty()
    ) {
        if(
            !closest_group_member[BUBBLE_RELATION_CURRENT]->
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
            closest_group_member[BUBBLE_RELATION_CURRENT]->pos,
            cur_leader_ptr->pos
        ) >
        game.config.group_member_grab_range
    ) {
        //The group member is physically too far away.
        closest_group_member_distant = true;
    }
    
    cur_leader_ptr->update_throw_variables();
}


/**
 * @brief Updates the transformations, with the current camera coordinates,
 * zoom, etc.
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


/**
 * @brief Constructs a new message box info object.
 *
 * @param text Text to display.
 * @param speaker_icon If not nullptr, use this bitmap to represent who
 * is talking.
 */
msg_box_t::msg_box_t(const string &text, ALLEGRO_BITMAP* speaker_icon):
    speaker_icon(speaker_icon) {
    
    string message = unescape_string(text);
    if(message.size() && message.back() == '\n') {
        message.pop_back();
    }
    vector<string_token> tokens = tokenize_string(message);
    set_string_token_widths(
        tokens, game.sys_assets.fnt_standard, game.sys_assets.fnt_slim,
        al_get_font_line_height(game.sys_assets.fnt_standard), true
    );
    
    vector<string_token> line;
    for(size_t t = 0; t < tokens.size(); t++) {
        if(tokens[t].type == STRING_TOKEN_LINE_BREAK) {
            tokens_per_line.push_back(line);
            line.clear();
        } else {
            line.push_back(tokens[t]);
        }
    }
    if(!line.empty()) {
        tokens_per_line.push_back(line);
    }
}


/**
 * @brief Handles the user having pressed the button to continue the message,
 * or to skip to showing everything in the current section.
 */
void msg_box_t::advance() {
    if(
        transition_timer > 0.0f ||
        misinput_protection_timer > 0.0f ||
        swipe_timer > 0.0f
    ) return;
    
    size_t last_token = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t line_idx = cur_section * 3 + l;
        if(line_idx >= tokens_per_line.size()) break;
        last_token += tokens_per_line[line_idx].size();
    }
    
    if(cur_token >= last_token + 1) {
        if(cur_section >= ceil(tokens_per_line.size() / 3.0f) - 1) {
            //End of the message. Start closing the message box.
            close();
        } else {
            //Start swiping to go to the next section.
            swipe_timer = MSG_BOX::TOKEN_SWIPE_DURATION;
        }
    } else {
        //Skip the text typing and show everything in this section.
        skipped_at_token = cur_token;
        cur_token = last_token + 1;
    }
}


/**
 * @brief Closes the message box, even if it is still writing something.
 */
void msg_box_t::close() {
    if(!transition_in && transition_timer > 0.0f) return;
    transition_in = false;
    transition_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void msg_box_t::tick(float delta_t) {
    size_t tokens_in_section = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t line_idx = cur_section * 3 + l;
        if(line_idx >= tokens_per_line.size()) break;
        tokens_in_section += tokens_per_line[line_idx].size();
    }
    
    //Animate the swipe animation.
    if(swipe_timer > 0.0f) {
        swipe_timer -= delta_t;
        if(swipe_timer <= 0.0f) {
            //Go to the next section.
            swipe_timer = 0.0f;
            cur_section++;
            total_token_anim_time = 0.0f;
            total_skip_anim_time = 0.0f;
            skipped_at_token = INVALID;
        }
    }
    
    if(!transition_in || transition_timer == 0.0f) {
    
        //Animate the text.
        if(game.config.message_char_interval == 0.0f) {
            skipped_at_token = 0;
            cur_token = tokens_in_section + 1;
        } else {
            total_token_anim_time += delta_t;
            if(skipped_at_token == INVALID) {
                size_t prev_token = cur_token;
                cur_token =
                    total_token_anim_time / game.config.message_char_interval;
                cur_token =
                    std::min(cur_token, tokens_in_section + 1);
                if(
                    cur_token == tokens_in_section + 1 &&
                    prev_token != cur_token
                ) {
                    //We've reached the last token organically.
                    //Start a misinput protection timer, so the player
                    //doesn't accidentally go to the next section when they
                    //were just trying to skip the text.
                    misinput_protection_timer =
                        MSG_BOX::MISINPUT_PROTECTION_DURATION;
                }
            } else {
                total_skip_anim_time += delta_t;
            }
        }
        
    }
    
    //Animate the transition.
    transition_timer -= delta_t;
    transition_timer = std::max(0.0f, transition_timer);
    if(!transition_in && transition_timer == 0.0f) {
        to_delete = true;
    }
    
    //Misinput protection logic.
    misinput_protection_timer -= delta_t;
    misinput_protection_timer = std::max(0.0f, misinput_protection_timer);
    
    //Button opacity logic.
    if(
        transition_timer == 0.0f &&
        misinput_protection_timer == 0.0f &&
        swipe_timer == 0.0f &&
        cur_token >= tokens_in_section + 1
    ) {
        advance_button_alpha =
            std::min(
                advance_button_alpha +
                MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * delta_t,
                1.0f
            );
    } else {
        advance_button_alpha =
            std::max(
                0.0f,
                advance_button_alpha -
                MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * delta_t
            );
    }
}
