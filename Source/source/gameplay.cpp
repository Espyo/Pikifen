/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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

//How long the HUD moves for when the area is entered.
const float gameplay::AREA_INTRO_HUD_MOVE_TIME = 3.0f;


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
    bmp_fog(nullptr),
    bmp_hard_bubble(nullptr),
    bmp_message_box(nullptr),
    bmp_no_pikmin_bubble(nullptr),
    bmp_sun(nullptr) {
    
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
            cur_ratio = min(cur_ratio, 1.0f);
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
    bmp = recreate_bitmap(bmp);
    return bmp;
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
    if(!cur_area_data.weather_condition.fog_color.empty()) {
        bmp_fog =
            generate_fog_bitmap(
                cur_area_data.weather_condition.fog_near,
                cur_area_data.weather_condition.fog_far
            );
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
    
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    mouse_cursor_s.x = al_get_mouse_state_axis(&mouse_state, 0);
    mouse_cursor_s.y = al_get_mouse_state_axis(&mouse_state, 1);
    mouse_cursor_w = mouse_cursor_s;
    al_transform_coordinates(
        &screen_to_world_transform,
        &mouse_cursor_w.x, &mouse_cursor_w.y
    );
    leader_cursor_w = mouse_cursor_w;
    leader_cursor_s = mouse_cursor_s;
    
    day_minutes = day_minutes_start;
    area_time_passed = 0;
    
    for(size_t c = 0; c < controls[0].size(); ++c) {
        if(controls[0][c].action == BUTTON_THROW) {
            click_control_id = c;
            break;
        }
    }
    for(size_t c = 0; c < controls[0].size(); ++c) {
        if(controls[0][c].action == BUTTON_WHISTLE) {
            whistle_control_id = c;
            break;
        }
    }
    
    al_hide_mouse_cursor(display);
    
    area_title_fade_timer.start();
    hud_items.start_move(true, AREA_INTRO_HUD_MOVE_TIME);
    
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
    load_weather();
    load_spike_damage_types();
    
    //Mob types.
    load_mob_types(true);
    
    for(size_t p = 0; p < pikmin_order.size(); ++p) {
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pikmin_order[p]
        );
    }
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_BOMB);
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_LEADER);
    
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
    
    hud_items.set_item(
        item, s2f(words[0]), s2f(words[1]), s2f(words[2]), s2f(words[3])
    );
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
        delete_mob(*mobs.begin(), true);
    }
    
    if(lightmap_bmp) {
        al_destroy_bitmap(lightmap_bmp);
        lightmap_bmp = NULL;
    }
    
    unload_area_textures();
    unload_area();
    
    spray_amounts.clear();
    particles.clear();
    
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
    if(bmp_fog) {
        al_destroy_bitmap(bmp_fog);
        bmp_fog = NULL;
    }
    
    cur_message.clear();
    info_print_text.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded game content.
 */
void gameplay::unload_game_content() {
    weather_conditions.clear();
    
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
