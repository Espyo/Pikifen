/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Struct that holds the game's configuration, and related functions.
 */

#include "functions.h"
#include "game_config.h"


//Default values for the different options.
const bool game_config::DEF_CAN_THROW_LEADERS = true;
const unsigned char game_config::DEF_CARRYING_COLOR_MOVE[3] =
{ 255, 255, 255 };
const unsigned char game_config::DEF_CARRYING_COLOR_STOP[3] =
{ 96, 192, 192 };
const float game_config::DEF_CARRYING_SPEED_BASE_MULT = 0.5f;
const float game_config::DEF_CARRYING_SPEED_MAX_MULT = 0.8f;
const float game_config::DEF_CARRYING_SPEED_WEIGHT_MULT = 0.0004f;
const float game_config::DEF_CURSOR_MAX_DIST = 200.0f;
const float game_config::DEF_CURSOR_SPIN_SPEED = 180.0f;
const float game_config::DEF_DAY_MINUTES_END = 60 * 19;
const float game_config::DEF_DAY_MINUTES_PER_IRL_SEC = 2;
const float game_config::DEF_DAY_MINUTES_START = 60 * 7;
const float game_config::DEF_IDLE_TASK_RANGE = 50.0f;
const float game_config::DEF_MATURITY_POWER_MULT = 0.1f;
const float game_config::DEF_MATURITY_SPEED_MULT = 0.1f;
const size_t game_config::DEF_MAX_PIKMIN_IN_FIELD = 100;
const float game_config::DEF_MESSAGE_CHAR_INTERVAL = 0.02f;
const float game_config::DEF_NEXT_PLUCK_RANGE = 160.0f;
const float game_config::DEF_ONION_OPEN_RANGE = 24.0f;
const float game_config::DEF_PIKMIN_CHASE_RANGE = 200.0f;
const float game_config::DEF_PIKMIN_GRAB_RANGE = 64.0f;
const float game_config::DEF_PLUCK_RANGE = 30.0f;
const float game_config::DEF_STANDARD_PIKMIN_HEIGHT = 24.0f;
const float game_config::DEF_STANDARD_PIKMIN_RADIUS = 5.0f;
const float game_config::DEF_SWARM_TASK_RANGE = 0.0f;
const float game_config::DEF_WHISTLE_GROWTH_SPEED = 180.0f;
const float game_config::DEF_ZOOM_MAX_LEVEL = 3.0f;
const float game_config::DEF_ZOOM_MIN_LEVEL = 0.66f;


/* ----------------------------------------------------------------------------
 * Creates a game config struct.
 */
game_config::game_config() :
    can_throw_leaders(DEF_CAN_THROW_LEADERS),
    carrying_speed_base_mult(DEF_CARRYING_SPEED_BASE_MULT),
    carrying_speed_max_mult(DEF_CARRYING_SPEED_MAX_MULT),
    carrying_speed_weight_mult(DEF_CARRYING_SPEED_WEIGHT_MULT),
    cursor_max_dist(DEF_CURSOR_MAX_DIST),
    cursor_spin_speed(DEF_CURSOR_SPIN_SPEED),
    day_minutes_end(DEF_DAY_MINUTES_END),
    day_minutes_per_irl_sec(DEF_DAY_MINUTES_PER_IRL_SEC),
    day_minutes_start(DEF_DAY_MINUTES_START),
    idle_task_range(DEF_IDLE_TASK_RANGE),
    maturity_power_mult(DEF_MATURITY_POWER_MULT),
    maturity_speed_mult(DEF_MATURITY_SPEED_MULT),
    max_pikmin_in_field(DEF_MAX_PIKMIN_IN_FIELD),
    message_char_interval(DEF_MESSAGE_CHAR_INTERVAL),
    next_pluck_range(DEF_NEXT_PLUCK_RANGE),
    onion_open_range(DEF_ONION_OPEN_RANGE),
    pikmin_chase_range(DEF_PIKMIN_CHASE_RANGE),
    pikmin_grab_range(DEF_PIKMIN_GRAB_RANGE),
    pluck_range(DEF_PLUCK_RANGE),
    standard_pikmin_height(DEF_STANDARD_PIKMIN_HEIGHT),
    standard_pikmin_radius(DEF_STANDARD_PIKMIN_RADIUS),
    swarm_task_range(DEF_SWARM_TASK_RANGE),
    whistle_growth_speed(DEF_WHISTLE_GROWTH_SPEED),
    zoom_max_level(DEF_ZOOM_MAX_LEVEL),
    zoom_min_level(DEF_ZOOM_MIN_LEVEL) {
    
    carrying_color_move =
        al_map_rgb(
            DEF_CARRYING_COLOR_MOVE[0],
            DEF_CARRYING_COLOR_MOVE[1],
            DEF_CARRYING_COLOR_MOVE[2]
        );
    carrying_color_stop =
        al_map_rgb(
            DEF_CARRYING_COLOR_STOP[0],
            DEF_CARRYING_COLOR_STOP[1],
            DEF_CARRYING_COLOR_STOP[2]
        );
}


/* ----------------------------------------------------------------------------
 * Loads the game's config from a file.
 */
void game_config::load(data_node* file) {
    reader_setter rs(file);
    string pikmin_order_str;
    string leader_order_str;
    
    rs.set("game_name", name);
    rs.set("game_version", version);
    
    rs.set("carrying_color_move", carrying_color_move);
    rs.set("carrying_color_stop", carrying_color_stop);
    rs.set("carrying_speed_base_mult", carrying_speed_base_mult);
    rs.set("carrying_speed_max_mult", carrying_speed_max_mult);
    rs.set("carrying_speed_weight_mult", carrying_speed_weight_mult);
    
    rs.set("day_minutes_start", day_minutes_start);
    rs.set("day_minutes_end", day_minutes_end);
    rs.set("day_minutes_per_irl_sec", day_minutes_per_irl_sec);
    
    rs.set("pikmin_order", pikmin_order_str);
    rs.set("standard_pikmin_height", standard_pikmin_height);
    rs.set("standard_pikmin_radius", standard_pikmin_radius);
    
    rs.set("leader_order", leader_order_str);
    
    rs.set("idle_task_range", idle_task_range);
    rs.set("swarm_task_range", swarm_task_range);
    rs.set("pikmin_chase_range", pikmin_chase_range);
    rs.set("max_pikmin_in_field", max_pikmin_in_field);
    rs.set("maturity_power_mult", maturity_power_mult);
    rs.set("maturity_speed_mult", maturity_speed_mult);
    
    rs.set("can_throw_leaders", can_throw_leaders);
    rs.set("cursor_max_dist", cursor_max_dist);
    rs.set("cursor_spin_speed", cursor_spin_speed);
    rs.set("next_pluck_range", next_pluck_range);
    rs.set("onion_open_range", onion_open_range);
    rs.set("pikmin_grab_range", pikmin_grab_range);
    rs.set("pluck_range", pluck_range);
    rs.set("whistle_growth_speed", whistle_growth_speed);
    
    rs.set("message_char_interval", message_char_interval);
    rs.set("zoom_max_level", zoom_max_level);
    rs.set("zoom_min_level", zoom_min_level);
    
    pikmin_order_strings = semicolon_list_to_vector(pikmin_order_str);
    leader_order_strings = semicolon_list_to_vector(leader_order_str);
    cursor_spin_speed = deg_to_rad(cursor_spin_speed);
}
