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

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "load.h"
#include "misc_structs.h"
#include "utils/string_utils.h"

//How long the HUD moves for when the area is entered.
const float gameplay::AREA_INTRO_HUD_MOVE_TIME = 3.0f;
//How long it takes for the area name to fade away, in-game.
const float gameplay::AREA_TITLE_FADE_DURATION = 3.0f;
//How fast the "invalid cursor" effect goes, per second.
const float gameplay::CURSOR_INVALID_EFFECT_SPEED = TAU * 2;
//Every X seconds, the cursor's position is saved, to create the trail effect.
const float gameplay::CURSOR_SAVE_INTERVAL = 0.03f;
//Swarming arrows move these many units per second.
const float gameplay::SWARM_ARROW_SPEED = 400.0f;
//Seconds that need to pass before another swarm arrow appears.
const float gameplay::SWARM_ARROWS_INTERVAL = 0.1f;

/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay::gameplay() :
    game_state(),
    area_time_passed(0.0f),
    area_title_fade_timer(AREA_TITLE_FADE_DURATION),
    closest_group_member(nullptr),
    closest_group_member_distant(false),
    cur_leader_nr(0),
    cur_leader_ptr(nullptr),
    day_minutes(0.0f),
    leader_cursor_mob(nullptr),
    leader_cursor_sector(nullptr),
    msg_box(nullptr),
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
    close_to_onion_to_open(nullptr),
    close_to_pikmin_to_pluck(nullptr),
    close_to_ship_to_heal(nullptr),
    cursor_height_diff_light(0.0f),
    cursor_save_timer(CURSOR_SAVE_INTERVAL),
    day(1),
    hud_items(N_HUD_ITEMS),
    is_input_allowed(false),
    lightmap_bmp(nullptr),
    main_control_id(INVALID),
    particles(0),
    paused(false),
    precipitation(0),
    ready_for_input(false),
    selected_spray(0),
    swarm_next_arrow_timer(SWARM_ARROWS_INTERVAL),
    swarm_cursor(false),
    throw_can_reach_cursor(true) {
    
    swarm_next_arrow_timer.on_end = [this] () {
        swarm_next_arrow_timer.start();
        swarm_arrows.push_back(0);
    };
    swarm_next_arrow_timer.start();
}


/* ----------------------------------------------------------------------------
 * Draw the gameplay.
 */
void gameplay::do_drawing() {
    do_game_drawing();
}


/* ----------------------------------------------------------------------------
 * Tick the gameplay logic by one frame.
 */
void gameplay::do_logic() {
    if(game.creator_tools.change_speed) {
        game.delta_t *= game.creator_tools.change_speed_mult;
    }
    
    do_gameplay_logic();
    do_aesthetic_logic();
}


const int FOG_BITMAP_SIZE = 128;

/* ----------------------------------------------------------------------------
 * Generates the bitmap that'll draw the fog fade effect.
 */
ALLEGRO_BITMAP* gameplay::generate_fog_bitmap(
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
string gameplay::get_name() {
    return "gameplay";
}


/* ----------------------------------------------------------------------------
 * Leaves the gameplay state, returning to the main menu, or wherever else.
 */
void gameplay::leave() {
    if(game.area_editor_state->quick_play_area.empty()) {
        game.change_state(game.main_menu_state);
    } else {
        game.change_state(game.area_editor_state);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the "gameplay" state into memory.
 */
void gameplay::load() {
    size_t errors_reported_at_start = game.errors_reported_so_far;
    
    ready_for_input = false;
    
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
    if(mobs.leader.empty()) {
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
        mobs.leader.begin(), mobs.leader.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t priority_l1 =
        find(game.config.leader_order.begin(), game.config.leader_order.end(), l1->lea_type) -
        game.config.leader_order.begin();
        size_t priority_l2 =
        find(game.config.leader_order.begin(), game.config.leader_order.end(), l2->lea_type) -
        game.config.leader_order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    cur_leader_nr = 0;
    cur_leader_ptr = mobs.leader[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    cur_leader_ptr->active = true;
    
    game.cam.set_pos(cur_leader_ptr->pos);
    game.cam.set_zoom(game.options.zoom_mid_level);
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
    
    al_hide_mouse_cursor(game.display);
    
    area_title_fade_timer.start();
    hud_items.start_move(true, AREA_INTRO_HUD_MOVE_TIME);
    
    //Aesthetic stuff.
    
    if(game.errors_reported_so_far > errors_reported_at_start) {
        print_info(
            "\n\n\nERRORS FOUND!\n"
            "See \"" + ERROR_LOG_FILE_PATH + "\".\n\n\n",
            20.0f, 3.0f
        );
    }
    
    game.framerate_last_avg_point = 0;
    game.framerate_history.clear();
    
}


/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void gameplay::load_game_content() {
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
 */
void gameplay::load_hud_coordinates(const int item, string data) {
    vector<string> words = split(data);
    if(data.size() < 4) return;
    
    hud_items.set_item(
        item, s2f(words[0]), s2f(words[1]), s2f(words[2]), s2f(words[3])
    );
}


/* ----------------------------------------------------------------------------
 * Loads all gameplay HUD info.
 */
void gameplay::load_hud_info() {
    data_node file(MISC_FOLDER_PATH + "/HUD.txt");
    if(!file.file_was_opened) return;
    
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
    
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay::unload() {
    al_show_mouse_cursor(game.display);
    
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
    
    if(msg_box) delete msg_box;
    game.creator_tools.info_print_text.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded game content.
 */
void gameplay::unload_game_content() {
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
void gameplay::update_closest_group_member() {
    //Closest members so far for each maturity.
    dist closest_dists[N_MATURITIES];
    mob* closest_ptrs[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        closest_ptrs[m] = NULL;
    }
    
    game.gameplay_state->closest_group_member = NULL;
    
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
        if(closest_dists[2 - m] > game.config.pikmin_grab_range) continue;
        game.gameplay_state->closest_group_member = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!game.gameplay_state->closest_group_member) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; ++m) {
            if(!closest_ptrs[m]) continue;
            
            if(
                !game.gameplay_state->closest_group_member ||
                closest_dists[m] < closest_dist
            ) {
                game.gameplay_state->closest_group_member = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
        
    }
    
    if(
        fabs(game.gameplay_state->closest_group_member->z - cur_leader_ptr->z) >
        SECTOR_STEP
    ) {
        //If the group member is beyond a step, it's obviously above or below
        //a wall, compared to the leader. No grabbing allowed.
        game.gameplay_state->closest_group_member_distant = true;
    } else {
        game.gameplay_state->closest_group_member_distant =
            closest_dist > game.config.pikmin_grab_range;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void gameplay::update_transformations() {
    //World coordinates to screen coordinates.
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + game.win_w / 2.0 / game.cam.zoom,
        -game.cam.pos.y + game.win_h / 2.0 / game.cam.zoom
    );
    al_scale_transform(&game.world_to_screen_transform, game.cam.zoom, game.cam.zoom);
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}
