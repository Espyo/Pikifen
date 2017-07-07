/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gameplay state class and
 * gameplay state-related functions.
 */

#include <algorithm>

#include "gameplay.h"

#include "drawing.h"
#include "functions.h"
#include "load.h"
#include "misc_structs.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay::gameplay() :
    game_state(),
    bmp_bubble(nullptr),
    bmp_counter_bubble_group(nullptr),
    bmp_counter_bubble_field(nullptr),
    bmp_counter_bubble_standby(nullptr),
    bmp_counter_bubble_total(nullptr),
    bmp_day_bubble(nullptr),
    bmp_distant_pikmin_marker(nullptr),
    bmp_hard_bubble(nullptr),
    bmp_message_box(nullptr),
    bmp_no_pikmin_bubble(nullptr),
    bmp_sun(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the "gameplay" state into memory.
 */
void gameplay::load() {
    ready_for_input = false;
    
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    //Game content.
    load_game_content();
    
    //Initializing game things.
    size_t n_spray_types = spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) { spray_amounts.push_back(0); }
    
    load_area(area_to_load, false, false);
    load_area_textures();
    
    if(!cur_area_data.weather_condition.blackout_strength.empty()) {
        lightmap_bmp = al_create_bitmap(scr_w, scr_h);
    }
    
    //Generate mobs.
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        
        create_mob(
            m_ptr->category, m_ptr->pos, m_ptr->type, m_ptr->angle, m_ptr->vars
        );
    }
    
    //Sort leaders.
    sort(
        leaders.begin(), leaders.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t priority_l1 =
        find(leader_order.begin(), leader_order.end(), l1->lea_type) -
        leader_order.begin();
        size_t priority_l2 =
        find(leader_order.begin(), leader_order.end(), l2->lea_type) -
        leader_order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    cur_leader_nr = 0;
    cur_leader_ptr = leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    
    cam_pos = cam_final_pos = cur_leader_ptr->pos;
    cam_zoom = cam_final_zoom = zoom_mid_level;
    update_transformations();
    
    leader_cursor_w.x = cur_leader_ptr->pos.x + cursor_max_dist / 2.0;
    leader_cursor_w.y = cur_leader_ptr->pos.y;
    leader_cursor_s = leader_cursor_w;
    al_transform_coordinates(
        &world_to_screen_transform,
        &leader_cursor_s.x, &leader_cursor_s.y
    );
    mouse_cursor_w = leader_cursor_w;
    mouse_cursor_s = leader_cursor_s;
    al_set_mouse_xy(display, mouse_cursor_s.x, mouse_cursor_s.y);
    
    day_minutes = day_minutes_start;
    area_time_passed = 0;
    
    for(size_t c = 0; c < controls[0].size(); ++c) {
        if(controls[0][c].action == BUTTON_THROW) {
            click_control_id = c;
            break;
        }
    }
    
    al_hide_mouse_cursor(display);
    
    area_title_fade_timer.start();
    
    //Aesthetic stuff.
    cur_message_char_timer =
        timer(
            message_char_interval,
    [] () {
        cur_message_char_timer.start();
        cur_message_char++;
    }
        );
        
    //Debug stuff for convenience.
    //TODO remove.
    for(size_t s = 0; s < spray_types.size(); ++s) {
        spray_amounts[s] = 20;
    }
    
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
    
    //Mob types.
    load_mob_types(true);
    
    for(size_t p = 0; p < pikmin_order.size(); ++p) {
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pikmin_order[p]
        );
    }
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_BOMB);
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_LEADER);
    
    //Weather.
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions =
        weather_file.get_nr_of_children_by_name("weather");
        
    for(size_t wc = 0; wc < n_weather_conditions; ++wc) {
        data_node* cur_weather = weather_file.get_child_by_name("weather", wc);
        
        string name = cur_weather->get_child_by_name("name")->value;
        if(name.empty()) name = "default";
        
        //Lighting.
        vector<pair<size_t, string> > lighting_table =
            get_weather_table(cur_weather->get_child_by_name("lighting"));
            
        vector<pair<size_t, ALLEGRO_COLOR> > lighting;
        for(size_t p = 0; p < lighting_table.size(); ++p) {
            lighting.push_back(
                make_pair(
                    lighting_table[p].first,
                    s2c(lighting_table[p].second)
                )
            );
        }
        
        if(lighting.empty()) {
            log_error("Weather condition " + name + " has no lighting!");
        }
        
        //Sun's strength.
        vector<pair<size_t, string> > sun_strength_table =
            get_weather_table(cur_weather->get_child_by_name("sun_strength"));
            
        vector<pair<size_t, unsigned char> > sun_strength;
        for(size_t p = 0; p < sun_strength_table.size(); ++p) {
            sun_strength.push_back(
                make_pair(
                    sun_strength_table[p].first,
                    s2i(sun_strength_table[p].second)
                )
            );
        }
        
        //Blackout effect's strength.
        vector<pair<size_t, string> > blackout_strength_table =
            get_weather_table(
                cur_weather->get_child_by_name("blackout_strength")
            );
            
        vector<pair<size_t, unsigned char> > blackout_strength;
        for(size_t p = 0; p < blackout_strength_table.size(); ++p) {
            blackout_strength.push_back(
                make_pair(
                    blackout_strength_table[p].first,
                    s2i(blackout_strength_table[p].second)
                )
            );
        }
        
        //Precipitation.
        unsigned char precipitation_type =
            s2i(
                cur_weather->get_child_by_name(
                    "precipitation_type"
                )->get_value_or_default(i2s(PRECIPITATION_TYPE_NONE))
            );
        interval precipitation_frequency =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_frequency"
                )->value
            );
        interval precipitation_speed =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_speed"
                )->value
            );
        interval precipitation_angle =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_angle"
                )->get_value_or_default(f2s((M_PI + M_PI_2)))
            );
            
        //Save.
        weather_conditions[name] =
            weather(
                name, lighting, sun_strength, blackout_strength,
                precipitation_type, precipitation_frequency,
                precipitation_speed, precipitation_angle
            );
    }
}


/* ----------------------------------------------------------------------------
 * Loads all gameplay HUD info.
 */
void gameplay::load_hud_info() {
    data_node file = data_node(MISC_FOLDER_PATH + "/HUD.txt");
    if(!file.file_was_opened) return;
    
    //Hud coordinates.
    data_node* positions_node = file.get_child_by_name("positions");
    
#define loader(id, name) \
    load_hud_coordinates(id, positions_node->get_child_by_name(name)->value)
    
    loader(HUD_ITEM_TIME,                "time");
    loader(HUD_ITEM_DAY_BUBBLE,          "day_bubble");
    loader(HUD_ITEM_DAY_NUMBER,          "day_number");
    loader(HUD_ITEM_LEADER_1_ICON,       "leader_1_icon");
    loader(HUD_ITEM_LEADER_2_ICON,       "leader_2_icon");
    loader(HUD_ITEM_LEADER_3_ICON,       "leader_3_icon");
    loader(HUD_ITEM_LEADER_1_HEALTH,     "leader_1_health");
    loader(HUD_ITEM_LEADER_2_HEALTH,     "leader_2_health");
    loader(HUD_ITEM_LEADER_3_HEALTH,     "leader_3_health");
    loader(HUD_ITEM_PIKMIN_STANDBY_ICON, "pikmin_standby_icon");
    loader(HUD_ITEM_PIKMIN_STANDBY_NR,   "pikmin_standby_nr");
    loader(HUD_ITEM_PIKMIN_STANDBY_X,    "pikmin_standby_x");
    loader(HUD_ITEM_PIKMIN_GROUP_NR,     "pikmin_group_nr");
    loader(HUD_ITEM_PIKMIN_FIELD_NR,     "pikmin_field_nr");
    loader(HUD_ITEM_PIKMIN_TOTAL_NR,     "pikmin_total_nr");
    loader(HUD_ITEM_PIKMIN_SLASH_1,      "pikmin_slash_1");
    loader(HUD_ITEM_PIKMIN_SLASH_2,      "pikmin_slash_2");
    loader(HUD_ITEM_PIKMIN_SLASH_3,      "pikmin_slash_3");
    loader(HUD_ITEM_SPRAY_1_ICON,        "spray_1_icon");
    loader(HUD_ITEM_SPRAY_1_AMOUNT,      "spray_1_amount");
    loader(HUD_ITEM_SPRAY_1_KEY,         "spray_1_key");
    loader(HUD_ITEM_SPRAY_2_ICON,        "spray_2_icon");
    loader(HUD_ITEM_SPRAY_2_AMOUNT,      "spray_2_amount");
    loader(HUD_ITEM_SPRAY_2_KEY,         "spray_2_key");
    loader(HUD_ITEM_SPRAY_PREV_ICON,     "spray_prev_icon");
    loader(HUD_ITEM_SPRAY_PREV_KEY,      "spray_prev_key");
    loader(HUD_ITEM_SPRAY_NEXT_ICON,     "spray_next_icon");
    loader(HUD_ITEM_SPRAY_NEXT_KEY,      "spray_next_key");
    
#undef loader
    
    //On the HUD file, coordinates range from 0 to 100,
    //and 0 width or height means "keep aspect ratio with the other component".
    //Let's pre-bake these values, such that all widths and heights at 0
    //get set to -1 (draw_sprite and stuff like that expect -1 for these cases),
    //and all other coordinates transform from percentages
    //to screen coordinates.
    //Widths AND heights that are both set to 0 should stay that way though.
    
    for(int i = 0; i < N_HUD_ITEMS; ++i) {
        if(hud_coords[i][2] == 0 && hud_coords[i][3] != 0) {
            hud_coords[i][2] = -1;
        } else if(hud_coords[i][3] == 0 && hud_coords[i][2] != 0) {
            hud_coords[i][3] = -1;
        }
        
        hud_coords[i][0] *= scr_w;
        hud_coords[i][1] *= scr_h;
        if(hud_coords[i][2] != -1) {
            hud_coords[i][2] *= scr_w;
        }
        if(hud_coords[i][3] != -1) {
            hud_coords[i][3] *= scr_h;
        }
    }
    
    //Bitmaps.
    data_node* bitmaps_node = file.get_child_by_name("files");
    
#define loader(var, name) \
    var = \
          bitmaps.get( \
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
 * Loads HUD coordinates of a specific HUD item.
 */
void gameplay::load_hud_coordinates(const int item, string data) {
    vector<string> words = split(data);
    if(data.size() < 4) return;
    
    for(unsigned char c = 0; c < 4; ++c) {
        hud_coords[item][c] = s2f(words[c]) / 100.0f;
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay::unload() {
    al_show_mouse_cursor(display);
    
    cur_leader_ptr = NULL;
    cam_pos = cam_final_pos = point();
    cam_zoom = cam_final_zoom = 1.0f;
    
    while(!mobs.empty()) {
        delete_mob(*mobs.begin());
    }
    
    if(lightmap_bmp) {
        al_destroy_bitmap(lightmap_bmp);
        lightmap_bmp = NULL;
    }
    
    unload_area_textures();
    unload_area();
    
    spray_amounts.clear();
    
    unload_game_content();
    
    bitmaps.detach(bmp_bubble);
    bitmaps.detach(bmp_counter_bubble_field);
    bitmaps.detach(bmp_counter_bubble_group);
    bitmaps.detach(bmp_counter_bubble_standby);
    bitmaps.detach(bmp_counter_bubble_total);
    bitmaps.detach(bmp_day_bubble);
    bitmaps.detach(bmp_distant_pikmin_marker);
    bitmaps.detach(bmp_hard_bubble);
    bitmaps.detach(bmp_message_box);
    bitmaps.detach(bmp_no_pikmin_bubble);
    bitmaps.detach(bmp_sun);
}


/* ----------------------------------------------------------------------------
 * Unloads loaded game content.
 */
void gameplay::unload_game_content() {
    weather_conditions.clear();
    
    subgroup_types.clear();
    
    unload_mob_types(true);
    
    unload_hazards();
    unload_spray_types();
    unload_status_types(true);
    unload_liquids();
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void gameplay::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + scr_w / 2.0 / cam_zoom,
        -cam_pos.y + scr_h / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Tick the gameplay logic by one frame.
 */
void gameplay::do_logic() {
    if(creator_tool_change_speed) {
        delta_t *= creator_tool_change_speed_mult;
    }
    
    do_gameplay_logic();
    do_aesthetic_logic();
}


/* ----------------------------------------------------------------------------
 * Draw the gameplay.
 */
void gameplay::do_drawing() {
    do_game_drawing();
}
