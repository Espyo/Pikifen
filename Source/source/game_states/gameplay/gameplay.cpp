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
#include "../../mobs/pile.h"
#include "../../misc_structs.h"
#include "../../utils/string_utils.h"


//How long the HUD moves for when the area is entered.
const float gameplay_state::AREA_INTRO_HUD_MOVE_TIME = 3.0f;
//How long it takes for the area name to fade away, in-game.
const float gameplay_state::AREA_TITLE_FADE_DURATION = 3.0f;
//How fast the "invalid cursor" effect goes, per second.
const float gameplay_state::CURSOR_INVALID_EFFECT_SPEED = TAU * 2;
//Every X seconds, the cursor's position is saved, to create the trail effect.
const float gameplay_state::CURSOR_SAVE_INTERVAL = 0.03f;
//The Onion menu can only show, at most, these many Pikmin types per page.
const size_t gameplay_state::ONION_MENU_TYPES_PER_PAGE = 5;
//Swarming arrows move these many units per second.
const float gameplay_state::SWARM_ARROW_SPEED = 400.0f;
//Seconds that need to pass before another swarm arrow appears.
const float gameplay_state::SWARM_ARROWS_INTERVAL = 0.1f;

//Interval between button hold activations, at the slowest speed.
const float gameplay_state::onion_menu_struct::BUTTON_REPEAT_MAX_INTERVAL = 0.3f;
//Interval between button hold activations, at the fastest speed.
const float gameplay_state::onion_menu_struct::BUTTON_REPEAT_MIN_INTERVAL = 0.011f;
//How long it takes for the button hold activation repeats to reach max speed.
const float gameplay_state::onion_menu_struct::BUTTON_REPEAT_RAMP_TIME = 0.9f;
//How many Pikmin types can be on-screen per page.
const size_t gameplay_state::onion_menu_struct::MAX_TYPES_ON_SCREEN = 5;
//How long to let text turn red for.
const float gameplay_state::onion_menu_struct::RED_TEXT_DURATION = 1.0f;


/* ----------------------------------------------------------------------------
 * Initializes the gameplay HUD item manager.
 * item_total:
 *   How many HUD items exist in total.
 */
gameplay_hud_manager::gameplay_hud_manager(const size_t item_total) :
    hud_item_manager(item_total),
    move_in(false),
    move_timer(0),
    offscreen(false) {
    
}


/* ----------------------------------------------------------------------------
 * Retrieves the data necessary for the drawing routine.
 * Returns false if this element shouldn't be drawn.
 * id:
 *   ID of the HUD item.
 * center:
 *   Pointer to place the final center coordinates in, if any.
 * size:
 *   Pointer to place the final dimensions in, if any.
 */
bool gameplay_hud_manager::get_draw_data(
    const size_t id, point* center, point* size
) const {
    if(offscreen) return false;
    if(!hud_item_manager::get_draw_data(id, NULL, NULL)) {
        return false;
    }
    const hud_item* h = &items[id];
    
    point normal_coords, final_coords;
    normal_coords.x = h->center.x * game.win_w;
    normal_coords.y = h->center.y * game.win_h;
    
    if(move_timer.time_left == 0.0f) {
        final_coords = normal_coords;
        
    } else {
        point start_coords, end_coords;
        unsigned char ease_method;
        point offscreen_coords;
        
        float angle = get_angle(point(0.5, 0.5), h->center);
        offscreen_coords.x = h->center.x + cos(angle);
        offscreen_coords.y = h->center.y + sin(angle);
        offscreen_coords.x *= game.win_w;
        offscreen_coords.y *= game.win_h;
        
        if(move_in) {
            start_coords = offscreen_coords;
            end_coords = normal_coords;
            ease_method = EASE_OUT;
        } else {
            start_coords = normal_coords;
            end_coords = offscreen_coords;
            ease_method = EASE_IN;
        }
        
        final_coords.x =
            interpolate_number(
                ease(ease_method, 1 - move_timer.get_ratio_left()),
                0, 1, start_coords.x, end_coords.x
            );
        final_coords.y =
            interpolate_number(
                ease(ease_method, 1 - move_timer.get_ratio_left()),
                0, 1, start_coords.y, end_coords.y
            );
    }
    
    if(center) {
        *center = final_coords;
    }
    if(size) {
        size->x = h->size.x * game.win_w;
        size->y = h->size.y * game.win_h;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Starts a movement animation.
 * in:
 *   Are the items moving into view, or out of view?
 * duration:
 *   How long this animation lasts for.
 */
void gameplay_hud_manager::start_move(const bool in, const float duration) {
    move_in = in;
    move_timer.start(duration);
}


/* ----------------------------------------------------------------------------
 * Ticks the manager one frame in time.
 * time:
 *   Seconds to tick by.
 */
void gameplay_hud_manager::tick(const float time) {
    hud_item_manager::tick(time);
    move_timer.tick(time);
    if(!move_in && move_timer.time_left == 0.0f) {
        offscreen = true;
    } else {
        offscreen = false;
    }
}


/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay_state::gameplay_state() :
    game_state(),
    after_hours(false),
    area_time_passed(0.0f),
    area_title_fade_timer(AREA_TITLE_FADE_DURATION),
    closest_group_member(nullptr),
    closest_group_member_distant(false),
    cur_leader_nr(0),
    cur_leader_ptr(nullptr),
    day_minutes(0.0f),
    hud_items(N_HUD_ITEMS),
    leader_cursor_mob(nullptr),
    leader_cursor_sector(nullptr),
    msg_box(nullptr),
    particles(0),
    precipitation(0),
    swarm_angle(0),
    swarm_magnitude(0.0f),
    bmp_bubble(nullptr),
    bmp_counter_bubble_group(nullptr),
    bmp_counter_bubble_field(nullptr),
    bmp_counter_bubble_standby(nullptr),
    bmp_counter_bubble_total(nullptr),
    bmp_day_bubble(nullptr),
    bmp_distant_pikmin_marker(nullptr),
    bmp_fog(nullptr),
    bmp_hard_bubble(nullptr),
    bmp_message_box(nullptr),
    bmp_no_pikmin_bubble(nullptr),
    bmp_sun(nullptr),
    cancel_control_id(INVALID),
    close_to_interactable_to_use(nullptr),
    close_to_nest_to_open(nullptr),
    close_to_pikmin_to_pluck(nullptr),
    close_to_ship_to_heal(nullptr),
    cursor_height_diff_light(0.0f),
    cursor_save_timer(CURSOR_SAVE_INTERVAL),
    day(1),
    is_input_allowed(false),
    lightmap_bmp(nullptr),
    main_control_id(INVALID),
    onion_menu(nullptr),
    paused(false),
    ready_for_input(false),
    selected_spray(0),
    swarm_next_arrow_timer(SWARM_ARROWS_INTERVAL),
    swarm_cursor(false),
    throw_can_reach_cursor(true),
    went_to_results(false) {
    
    swarm_next_arrow_timer.on_end = [this] () {
        swarm_next_arrow_timer.start();
        swarm_arrows.push_back(0);
    };
    swarm_next_arrow_timer.start();
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
        do_gameplay_logic();
    }
    do_menu_logic();
    do_aesthetic_logic();
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
    
    hud_items.start_move(true, AREA_INTRO_HUD_MOVE_TIME);
    if(went_to_results) {
        game.fade_mgr.start_fade(true, nullptr);
    }
    
    ready_for_input = false;
}


const int FOG_BITMAP_SIZE = 128;

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
    
    ALLEGRO_BITMAP* bmp = al_create_bitmap(FOG_BITMAP_SIZE, FOG_BITMAP_SIZE);
    
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
    
    //How close to the edge is the current pixel? 0 = center, 1 = edge.
    float cur_ratio = 0;
    //Alpha to use for this pixel.
    unsigned char cur_a = 0;
    //This is where the "near" section of the fog is.
    float near_ratio = near_radius / far_radius;
    //Memory location of the opposite row's pixels.
    unsigned char* opposite_row;
    
#define fill_pixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = cur_a; \
    
    for(int y = 0; y < ceil(FOG_BITMAP_SIZE / 2.0); ++y) {
        for(int x = 0; x < ceil(FOG_BITMAP_SIZE / 2.0); ++x) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            cur_ratio =
                dist(
                    point(x, y),
                    point(FOG_BITMAP_SIZE / 2.0, FOG_BITMAP_SIZE / 2.0)
                ).to_float() / (FOG_BITMAP_SIZE / 2.0);
            cur_ratio = std::min(cur_ratio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            cur_ratio =
                interpolate_number(cur_ratio, near_ratio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            cur_ratio = clamp(cur_ratio, 0.0f, 1.0f);
            cur_a = 255 * cur_ratio;
            
            opposite_row = row + region->pitch * (FOG_BITMAP_SIZE - y - y - 1);
            fill_pixel(x, row);
            fill_pixel(FOG_BITMAP_SIZE - x - 1, row);
            fill_pixel(x, opposite_row);
            fill_pixel(FOG_BITMAP_SIZE - x - 1, opposite_row);
        }
        row += region->pitch;
    }
    
#undef fill_pixel
    
    al_unlock_bitmap(bmp);
    bmp = recreate_bitmap(bmp); //Refresh mipmaps.
    return bmp;
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string gameplay_state::get_name() const {
    return "gameplay";
}


/* ----------------------------------------------------------------------------
 * Leaves the gameplay state, returning to the main menu, or wherever else.
 */
void gameplay_state::leave() {
    game.fade_mgr.start_fade(
        false,
    [this] () {
    
        if(game.perf_mon) {
            //Don't register the final frame, since it won't draw anything.
            game.perf_mon->set_paused(true);
        }
        
        al_show_mouse_cursor(game.display);
        
        if(game.states.area_ed->quick_play_area.empty()) {
            game.states.results->time_taken = area_time_passed;
            went_to_results = true;
            //Change state, but don't unload this one, since the player
            //may pick the "keep playing" option in the results screen.
            game.change_state(game.states.results, false);
        } else {
            game.change_state(game.states.area_ed);
        }
        
    }
    );
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
    
    //Initializing game things.
    size_t n_spray_types = game.spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) {
        spray_stats.push_back(spray_stats_struct());
    }
    
    load_area(area_to_load, false, false);
    
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
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object generation");
    }
    
    vector<mob*> mobs_per_gen;
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        mobs_per_gen.push_back(
            create_mob(
                m_ptr->category, m_ptr->pos, m_ptr->type,
                m_ptr->angle, m_ptr->vars
            )
        );
    }
    
    //Panic check -- If there are no leaders, abort.
    if(mobs.leaders.empty()) {
        show_message_box(
            game.display, "No leaders!", "No leaders!",
            "This area has no leaders! You need at least one "
            "in order to play.",
            NULL, ALLEGRO_MESSAGEBOX_WARN
        );
        leave();
        return;
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
    
    path_mgr.handle_area_load();
    
    cur_leader_nr = 0;
    cur_leader_ptr = mobs.leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    cur_leader_ptr->active = true;
    
    game.cam.set_pos(cur_leader_ptr->pos);
    game.cam.set_zoom(game.options.zoom_mid_level);
    
    cursor_save_timer.on_end = [this] () {
        cursor_save_timer.start();
        cursor_spots.push_back(game.mouse_cursor_s);
        if(cursor_spots.size() > CURSOR_SAVE_N_SPOTS) {
            cursor_spots.erase(cursor_spots.begin());
        }
    };
    cursor_save_timer.start();
    
    cur_leader_ptr->stop_whistling();
    
    day_minutes = game.config.day_minutes_start;
    area_time_passed = 0.0f;
    after_hours = false;
    
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
    
    game.states.results->reset();
    game.states.results->area_name = game.cur_area_data.name;
    game.states.results->enemies_total = mobs.enemies.size();
    for(size_t t = 0; t < mobs.treasures.size(); ++t) {
        game.states.results->points_total +=
            mobs.treasures[t]->tre_type->points;
    }
    for(size_t e = 0; e < mobs.enemies.size(); ++e) {
        for(size_t s = 0; s < mobs.enemies[e]->specific_spoils.size(); ++s) {
            mob_type* s_type = mobs.enemies[e]->specific_spoils[s];
            if(s_type->category->id == MOB_CATEGORY_TREASURES) {
                game.states.results->points_total +=
                    ((treasure_type*) s_type)->points;
            }
        }
    }
    for(size_t p = 0; p < mobs.piles.size(); ++p) {
        pile* p_ptr = mobs.piles[p];
        resource_type* res_type = p_ptr->pil_type->contents;
        if(res_type->delivery_result != RESOURCE_DELIVERY_RESULT_ADD_POINTS) {
            continue;
        }
        game.states.results->points_total +=
            p_ptr->amount * res_type->point_amount;
    }
    
    //TODO Uncomment this when replays are implemented.
    /*
    replay_timer = timer(
        REPLAY_SAVE_FREQUENCY,
    [this] () {
        this->replay_timer.start();
        vector<mob*> obstacles; //TODO
        session_replay.add_state(
            leaders, pikmin_list, enemies, treasures, onions, obstacles,
            cur_leader_nr
        );
    }
    );
    replay_timer.start();
    session_replay.clear();*/
    
    area_title_fade_timer.start();
    
    if(game.errors_reported_so_far > errors_reported_at_start) {
        print_info(
            "\n\n\nERRORS FOUND!\n"
            "See \"" + ERROR_LOG_FILE_PATH + "\".\n\n\n",
            20.0f, 3.0f
        );
    }
    
    game.framerate_last_avg_point = 0;
    game.framerate_history.clear();
    
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
    load_hud_info();
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
 * Loads HUD coordinates of a specific HUD item.
 * item:
 *   Item to load the coordinates for.
 * data:
 *   String containing the coordinate data.
 */
void gameplay_state::load_hud_coordinates(const int item, string data) {
    vector<string> words = split(data);
    if(data.size() < 4) return;
    
    hud_items.set_item(
        item, s2f(words[0]), s2f(words[1]), s2f(words[2]), s2f(words[3])
    );
}


/* ----------------------------------------------------------------------------
 * Loads all gameplay HUD info.
 */
void gameplay_state::load_hud_info() {
    data_node file(MISC_FOLDER_PATH + "/HUD.txt");
    if(!file.file_was_opened) return;
    
    if(game.perf_mon) {
        game.perf_mon->start_measurement("HUD info");
    }
    
    //Hud coordinates.
    data_node* positions_node = file.get_child_by_name("positions");
    
#define loader(id, name) \
    load_hud_coordinates(id, positions_node->get_child_by_name(name)->value)
    
    loader(HUD_ITEM_TIME,                  "time");
    loader(HUD_ITEM_DAY_BUBBLE,            "day_bubble");
    loader(HUD_ITEM_DAY_NUMBER,            "day_number");
    loader(HUD_ITEM_LEADER_1_ICON,         "leader_1_icon");
    loader(HUD_ITEM_LEADER_2_ICON,         "leader_2_icon");
    loader(HUD_ITEM_LEADER_3_ICON,         "leader_3_icon");
    loader(HUD_ITEM_LEADER_1_HEALTH,       "leader_1_health");
    loader(HUD_ITEM_LEADER_2_HEALTH,       "leader_2_health");
    loader(HUD_ITEM_LEADER_3_HEALTH,       "leader_3_health");
    loader(HUD_ITEM_PIKMIN_STANDBY_ICON,   "pikmin_standby_icon");
    loader(HUD_ITEM_PIKMIN_STANDBY_M_ICON, "pikmin_standby_m_icon");
    loader(HUD_ITEM_PIKMIN_STANDBY_NR,     "pikmin_standby_nr");
    loader(HUD_ITEM_PIKMIN_STANDBY_X,      "pikmin_standby_x");
    loader(HUD_ITEM_PIKMIN_GROUP_NR,       "pikmin_group_nr");
    loader(HUD_ITEM_PIKMIN_FIELD_NR,       "pikmin_field_nr");
    loader(HUD_ITEM_PIKMIN_TOTAL_NR,       "pikmin_total_nr");
    loader(HUD_ITEM_PIKMIN_SLASH_1,        "pikmin_slash_1");
    loader(HUD_ITEM_PIKMIN_SLASH_2,        "pikmin_slash_2");
    loader(HUD_ITEM_PIKMIN_SLASH_3,        "pikmin_slash_3");
    loader(HUD_ITEM_SPRAY_1_ICON,          "spray_1_icon");
    loader(HUD_ITEM_SPRAY_1_AMOUNT,        "spray_1_amount");
    loader(HUD_ITEM_SPRAY_1_BUTTON,        "spray_1_button");
    loader(HUD_ITEM_SPRAY_2_ICON,          "spray_2_icon");
    loader(HUD_ITEM_SPRAY_2_AMOUNT,        "spray_2_amount");
    loader(HUD_ITEM_SPRAY_2_BUTTON,        "spray_2_button");
    loader(HUD_ITEM_SPRAY_PREV_ICON,       "spray_prev_icon");
    loader(HUD_ITEM_SPRAY_PREV_BUTTON,     "spray_prev_button");
    loader(HUD_ITEM_SPRAY_NEXT_ICON,       "spray_next_icon");
    loader(HUD_ITEM_SPRAY_NEXT_BUTTON,     "spray_next_button");
    
#undef loader
    
    //Bitmaps.
    data_node* bitmaps_node = file.get_child_by_name("files");
    
#define loader(var, name) \
    var = \
          game.bitmaps.get( \
                            bitmaps_node->get_child_by_name(name)->value, \
                            bitmaps_node->get_child_by_name(name) \
                          );
    
    loader(bmp_bubble,                 "bubble");
    loader(bmp_counter_bubble_field,   "counter_bubble_field");
    loader(bmp_counter_bubble_group,   "counter_bubble_group");
    loader(bmp_counter_bubble_standby, "counter_bubble_standby");
    loader(bmp_counter_bubble_total,   "counter_bubble_total");
    loader(bmp_day_bubble,             "day_bubble");
    loader(bmp_distant_pikmin_marker,  "distant_pikmin_marker");
    loader(bmp_hard_bubble,            "hard_bubble");
    loader(bmp_message_box,            "message_box");
    loader(bmp_no_pikmin_bubble,       "no_pikmin_bubble");
    loader(bmp_sun,                    "sun");
    
#undef loader
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay_state::unload() {
    al_show_mouse_cursor(game.display);
    
    path_mgr.clear();
    
    cur_leader_ptr = NULL;
    
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
    
    spray_stats.clear();
    particles.clear();
    
    unload_game_content();
    
    game.bitmaps.detach(bmp_bubble);
    game.bitmaps.detach(bmp_counter_bubble_field);
    game.bitmaps.detach(bmp_counter_bubble_group);
    game.bitmaps.detach(bmp_counter_bubble_standby);
    game.bitmaps.detach(bmp_counter_bubble_total);
    game.bitmaps.detach(bmp_day_bubble);
    game.bitmaps.detach(bmp_distant_pikmin_marker);
    game.bitmaps.detach(bmp_hard_bubble);
    game.bitmaps.detach(bmp_message_box);
    game.bitmaps.detach(bmp_no_pikmin_bubble);
    game.bitmaps.detach(bmp_sun);
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
    game.maker_tools.info_print_text.clear();
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
 * Updates the variable that indicates what the closest
 * group member of the standby subgroup is.
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * NULL if there is no member of that subgroup available.
 */
void gameplay_state::update_closest_group_member() {
    //Closest members so far for each maturity.
    dist closest_dists[N_MATURITIES];
    mob* closest_ptrs[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        closest_ptrs[m] = NULL;
    }
    
    game.states.gameplay->closest_group_member = NULL;
    
    //Fetch the closest, for each maturity.
    size_t n_members = cur_leader_ptr->group->members.size();
    for(size_t m = 0; m < n_members; ++m) {
    
        mob* member_ptr = cur_leader_ptr->group->members[m];
        if(
            member_ptr->subgroup_type_ptr !=
            cur_leader_ptr->group->cur_standby_type
        ) {
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
        game.states.gameplay->closest_group_member = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!game.states.gameplay->closest_group_member) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; ++m) {
            if(!closest_ptrs[m]) continue;
            
            if(
                !game.states.gameplay->closest_group_member ||
                closest_dists[m] < closest_dist
            ) {
                game.states.gameplay->closest_group_member = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
        
    }
    
    if(
        fabs(
            game.states.gameplay->closest_group_member->z -
            cur_leader_ptr->z
        ) >
        SECTOR_STEP
    ) {
        //If the group member is beyond a step, it's obviously above or below
        //a wall, compared to the leader. No grabbing allowed.
        game.states.gameplay->closest_group_member_distant = true;
    } else {
        game.states.gameplay->closest_group_member_distant =
            closest_dist > game.config.group_member_grab_range;
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


/* ----------------------------------------------------------------------------
 * Initializes the Onion menu HUD item manager.
 * item_total:
 *   How many HUD items exist in total.
 */
onion_hud_manager::onion_hud_manager(const size_t item_total) :
    hud_item_manager(item_total) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an Onion menu struct.
 * n_ptr:
 *   Pointer to the nest information struct.
 * leader_ptr:
 *   Leader responsible.
 */
gameplay_state::onion_menu_struct::onion_menu_struct(
    pikmin_nest_struct* n_ptr, leader* l_ptr
) :
    n_ptr(n_ptr),
    l_ptr(l_ptr),
    select_all(false),
    page(0),
    cursor_button(INVALID),
    button_hold_id(INVALID),
    button_hold_time(0.0f),
    button_hold_next_activation(0.0f),
    nr_pages(0),
    to_delete(false) {
    
    for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); ++t) {
        types.push_back(
            onion_menu_type_struct(t, n_ptr->nest_type->pik_types[t])
        );
    }
    
    nr_pages = ceil(types.size() / (float) MAX_TYPES_ON_SCREEN);
    
    hud = new onion_hud_manager(N_ONION_HUD_ITEMS);
    hud->set_item(ONION_HUD_ITEM_TITLE,       50,  7, 90, 20);
    hud->set_item(ONION_HUD_ITEM_CANCEL,      16, 87, 18, 11);
    hud->set_item(ONION_HUD_ITEM_OK,          84, 87, 18, 11);
    hud->set_item(ONION_HUD_ITEM_FIELD,       50, 77, 18,  4);
    hud->set_item(ONION_HUD_ITEM_SEL_ALL,     50, 89, 24,  6);
    hud->set_item(ONION_HUD_ITEM_O1_BUTTON,   50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_O2_BUTTON,   50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_O3_BUTTON,   50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_O4_BUTTON,   50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_O5_BUTTON,   50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_O1_AMOUNT,   50, 29, 12,  4);
    hud->set_item(ONION_HUD_ITEM_O2_AMOUNT,   50, 29, 12,  4);
    hud->set_item(ONION_HUD_ITEM_O3_AMOUNT,   50, 29, 12,  4);
    hud->set_item(ONION_HUD_ITEM_O4_AMOUNT,   50, 29, 12,  4);
    hud->set_item(ONION_HUD_ITEM_O5_AMOUNT,   50, 29, 12,  4);
    hud->set_item(ONION_HUD_ITEM_P1_BUTTON,   50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_P2_BUTTON,   50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_P3_BUTTON,   50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_P4_BUTTON,   50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_P5_BUTTON,   50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_P1_AMOUNT,   50, 51, 12,  4);
    hud->set_item(ONION_HUD_ITEM_P2_AMOUNT,   50, 51, 12,  4);
    hud->set_item(ONION_HUD_ITEM_P3_AMOUNT,   50, 51, 12,  4);
    hud->set_item(ONION_HUD_ITEM_P4_AMOUNT,   50, 51, 12,  4);
    hud->set_item(ONION_HUD_ITEM_P5_AMOUNT,   50, 51, 12,  4);
    hud->set_item(ONION_HUD_ITEM_OALL_BUTTON, 50, 20,  9, 12);
    hud->set_item(ONION_HUD_ITEM_PALL_BUTTON, 50, 60,  9, 12);
    hud->set_item(ONION_HUD_ITEM_PREV_PAGE,    5, 40,  8, 10);
    hud->set_item(ONION_HUD_ITEM_NEXT_PAGE,   95, 40,  8, 11);
    hud->set_item(ONION_HUD_ITEM_O_L_MORE,     5, 20,  3,  4);
    hud->set_item(ONION_HUD_ITEM_O_R_MORE,    95, 20,  3,  4);
    hud->set_item(ONION_HUD_ITEM_P_L_MORE,     5, 60,  3,  4);
    hud->set_item(ONION_HUD_ITEM_P_R_MORE,    95, 60,  3,  4);
    
    update_caches();
}


/* ----------------------------------------------------------------------------
 * Destroys an Onion menu struct.
 */
gameplay_state::onion_menu_struct::~onion_menu_struct() {
    delete hud;
}


/* ----------------------------------------------------------------------------
 * Activates the button currently being held down, either because it was
 * just pressed, or because it was held long enough for another activation.
 */
void gameplay_state::onion_menu_struct::activate_held_button() {
    //Individual Onion button.
    for(size_t t = 0; t < on_screen_types.size(); ++t) {
        int hud_item_id = ONION_HUD_ITEM_O1_BUTTON + t;
        if(button_hold_id == hud_item_id) {
            add_to_onion(on_screen_types[t]->type_idx);
            return;
        }
    }
    
    //Individual Pikmin button.
    for(size_t t = 0; t < on_screen_types.size(); ++t) {
        int hud_item_id = ONION_HUD_ITEM_P1_BUTTON + t;
        if(button_hold_id == hud_item_id) {
            add_to_group(on_screen_types[t]->type_idx);
            return;
        }
    }
    
    //Combined Onion button.
    if(button_hold_id == ONION_HUD_ITEM_OALL_BUTTON) {
        add_all_to_onion();
        return;
    }
    
    //Combined Pikmin button press.
    if(button_hold_id == ONION_HUD_ITEM_PALL_BUTTON) {
        add_all_to_group();
        return;
    }
}


/* ----------------------------------------------------------------------------
 * Adds one Pikmin of each type from Onion to the group, if possible.
 */
void gameplay_state::onion_menu_struct::add_all_to_group() {
    for(size_t t = 0; t < types.size(); ++t) {
        add_to_group(t);
    }
}


/* ----------------------------------------------------------------------------
 * Adds one Pikmin of each type from the group to the Onion, if possible.
 */
void gameplay_state::onion_menu_struct::add_all_to_onion() {
    for(size_t t = 0; t < types.size(); ++t) {
        add_to_onion(t);
    }
}


/* ----------------------------------------------------------------------------
 * Adds one Pikmin from the Onion to the group, if possible.
 * type_idx:
 *   Index of the Onion's Pikmin type.
 */
void gameplay_state::onion_menu_struct::add_to_group(const size_t type_idx) {
    size_t real_onion_amount =
        n_ptr->get_amount_by_type(n_ptr->nest_type->pik_types[type_idx]);
        
    //First, check if there are enough in the Onion to take out.
    if(real_onion_amount - types[type_idx].delta <= 0) {
        make_widget_red(
            ONION_HUD_ITEM_O1_AMOUNT + types[type_idx].on_screen_idx
        );
        return;
    }
    
    //Next, check if the addition won't make the field amount hit the limit.
    int total_delta = 0;
    for(size_t t = 0; t < types.size(); ++t) {
        total_delta += types[t].delta;
    }
    if(
        game.states.gameplay->mobs.pikmin_list.size() + total_delta >=
        game.config.max_pikmin_in_field
    ) {
        make_widget_red(ONION_HUD_ITEM_FIELD);
        return;
    }
    
    types[type_idx].delta++;
}


/* ----------------------------------------------------------------------------
 * Adds one Pikmin from the group to the Onion, if possible.
 * type_idx:
 *   Index of the Onion's Pikmin type.
 */
void gameplay_state::onion_menu_struct::add_to_onion(const size_t type_idx) {
    size_t real_group_amount =
        l_ptr->group->get_amount_by_type(n_ptr->nest_type->pik_types[type_idx]);
        
    if(real_group_amount + types[type_idx].delta <= 0) {
        if(types[type_idx].on_screen_idx != INVALID) {
            make_widget_red(
                ONION_HUD_ITEM_P1_AMOUNT + types[type_idx].on_screen_idx
            );
        }
        return;
    }
    
    types[type_idx].delta--;
}


/* ----------------------------------------------------------------------------
 * Confirms the player's changes, and sets up the Pikmin to climb up the
 * Onion, if any, and sets up the Onion to spit out Pikmin, if any.
 */
void gameplay_state::onion_menu_struct::confirm() {
    for(size_t t = 0; t < types.size(); ++t) {
        if(types[t].delta > 0) {
            n_ptr->request_pikmin(t, types[t].delta, l_ptr);
        } else if(types[t].delta < 0) {
            l_ptr->order_pikmin_to_onion(
                types[t].pik_type, n_ptr, -types[t].delta
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Flips to the specified page of Pikmin types.
 * page:
 *   Index of the new page.
 */
void gameplay_state::onion_menu_struct::go_to_page(const size_t page) {
    this->page = page;
    update_caches();
}


/* ----------------------------------------------------------------------------
 * Makes a given widget turn red.
 * id:
 *   ID of the widget.
 */
void gameplay_state::onion_menu_struct::make_widget_red(const size_t id) {
    red_widgets[id] = RED_TEXT_DURATION;
}


/* ----------------------------------------------------------------------------
 * Ticks the Onion menu by one frame.
 * time:
 *   How many seconds to tick by.
 */
void gameplay_state::onion_menu_struct::tick(const float delta_t) {

    //Correct the amount of wanted group members, if they are invalid.
    int total_delta = 0;
    
    for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); ++t) {
        //Get how many the player really has with them.
        int real_group_amount =
            l_ptr->group->get_amount_by_type(
                n_ptr->nest_type->pik_types[t]
            );
            
        //Make sure the player can't request to store more than what they have.
        types[t].delta = std::max(-real_group_amount, (int) types[t].delta);
        
        //Get how many are really in the Onion.
        int real_onion_amount =
            n_ptr->get_amount_by_type(n_ptr->nest_type->pik_types[t]);
            
        //Make sure the player can't request to call more than the Onion has.
        types[t].delta = std::min(real_onion_amount, (int) types[t].delta);
        
        //Calculate the total delta.
        total_delta += types[t].delta;
    }
    
    //Make sure the player can't request to have more than the field limit.
    int delta_over_limit =
        game.states.gameplay->mobs.pikmin_list.size() + total_delta -
        game.config.max_pikmin_in_field;
        
    while(delta_over_limit > 0) {
        vector<size_t> candidate_types;
        
        for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); ++t) {
            int real_group_amount =
                l_ptr->group->get_amount_by_type(
                    n_ptr->nest_type->pik_types[t]
                );
                
            if((-types[t].delta) < real_group_amount) {
                //It's possible to take away from this type's delta request.
                candidate_types.push_back(t);
            }
        }
        
        //Figure out the type with the largest delta.
        size_t best_type = 0;
        int best_type_delta = types[candidate_types[0]].delta;
        for(size_t t = 1; t < candidate_types.size(); ++t) {
            if(types[candidate_types[t]].delta > best_type_delta) {
                best_type = candidate_types[t];
                best_type_delta = types[candidate_types[t]].delta;
            }
        }
        
        //Finally, remove one request from this type.
        types[best_type].delta--;
        delta_over_limit--;
    }
    
    //Figure out what amount-related button is under the cursor, if any.
    size_t old_cursor_button = cursor_button;
    cursor_button = INVALID;
    
    if(!select_all) {
        for(size_t t = 0; t < on_screen_types.size(); ++t) {
            int hud_item_id = ONION_HUD_ITEM_O1_BUTTON + t;
            if(hud->is_mouse_in(hud_item_id)) {
                cursor_button = hud_item_id;
                break;
            }
            hud_item_id = ONION_HUD_ITEM_P1_BUTTON + t;
            if(hud->is_mouse_in(hud_item_id)) {
                cursor_button = hud_item_id;
                break;
            }
        }
    } else  {
        if(hud->is_mouse_in(ONION_HUD_ITEM_OALL_BUTTON)) {
            cursor_button = ONION_HUD_ITEM_OALL_BUTTON;
        } else if(hud->is_mouse_in(ONION_HUD_ITEM_PALL_BUTTON)) {
            cursor_button = ONION_HUD_ITEM_PALL_BUTTON;
        }
    }
    
    if(cursor_button != old_cursor_button) {
        button_hold_id = INVALID;
    }
    
    //Repeat the held button, if any.
    if(button_hold_id != INVALID) {
        button_hold_time += delta_t;
        button_hold_next_activation -= delta_t;
        
        while(button_hold_next_activation <= 0.0f) {
            activate_held_button();
            button_hold_next_activation +=
                clamp(
                    interpolate_number(
                        button_hold_time,
                        0, BUTTON_REPEAT_RAMP_TIME,
                        BUTTON_REPEAT_MAX_INTERVAL, BUTTON_REPEAT_MIN_INTERVAL
                    ),
                    BUTTON_REPEAT_MIN_INTERVAL,
                    BUTTON_REPEAT_MAX_INTERVAL
                );
        }
    }
    
    //Animate red text, if any.
    for(auto w = red_widgets.begin(); w != red_widgets.end();) {
        w->second -= delta_t;
        if(w->second <= 0.0f) {
            w = red_widgets.erase(w);
        } else {
            ++w;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Toggles the "select all" mode.
 */
void gameplay_state::onion_menu_struct::toggle_select_all() {
    select_all = !select_all;
    
    update_caches();
}


/* ----------------------------------------------------------------------------
 * Updates the caches.
 */
void gameplay_state::onion_menu_struct::update_caches() {
    on_screen_types.clear();
    
    for(size_t t = 0; t < types.size(); ++t) {
        types[t].on_screen_idx = INVALID;
    }
    
    for(
        size_t t = page * ONION_MENU_TYPES_PER_PAGE;
        t < (page + 1) * ONION_MENU_TYPES_PER_PAGE &&
        t < n_ptr->nest_type->pik_types.size();
        ++t
    ) {
        types[t].on_screen_idx = on_screen_types.size();
        on_screen_types.push_back(&types[t]);
    }
    
    float splits = on_screen_types.size() + 1;
    float leftmost = 0.50f;
    float rightmost = 0.50f;
    for(size_t t = 0; t < on_screen_types.size(); ++t) {
        float x = 1.0f / splits * (t + 1);
        hud->items[ONION_HUD_ITEM_O1_BUTTON + t].center.x = x;
        hud->items[ONION_HUD_ITEM_O1_AMOUNT + t].center.x = x;
        hud->items[ONION_HUD_ITEM_P1_BUTTON + t].center.x = x;
        hud->items[ONION_HUD_ITEM_P1_AMOUNT + t].center.x = x;
        leftmost =
            std::min(
                leftmost,
                x - hud->items[ONION_HUD_ITEM_O1_BUTTON].size.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                x + hud->items[ONION_HUD_ITEM_O1_BUTTON].size.x / 2.0f
            );
    }
    
    if(nr_pages > 1) {
        leftmost =
            std::min(
                leftmost,
                hud->items[ONION_HUD_ITEM_O_L_MORE].center.x -
                hud->items[ONION_HUD_ITEM_O_L_MORE].size.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                hud->items[ONION_HUD_ITEM_O_R_MORE].center.x +
                hud->items[ONION_HUD_ITEM_O_R_MORE].size.x / 2.0f
            );
    }
    
    hud->items[ONION_HUD_ITEM_OALL_BUTTON].size.x = rightmost - leftmost;
    hud->items[ONION_HUD_ITEM_PALL_BUTTON].size.x = rightmost - leftmost;
}


/* ----------------------------------------------------------------------------
 * Creates an Onion menu Pikmin type struct.
 */
gameplay_state::onion_menu_type_struct::onion_menu_type_struct(
    const size_t idx, pikmin_type* pik_type
) :
    delta(0),
    type_idx(idx),
    pik_type(pik_type) {
    
}
