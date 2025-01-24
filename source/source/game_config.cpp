/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Struct that holds the game's configuration, and related functions.
 */

#include "game_config.h"
#include "utils/string_utils.h"


namespace GAME_CONFIG {

//Default values for whether leaders can throw leaders.
const bool DEF_CAN_THROW_LEADERS = true;

//Default value for the non-specific carrying movement color.
const ALLEGRO_COLOR DEF_CARRYING_COLOR_MOVE =
{ 1.00f, 1.00f, 1.00f, 1.00f};

//Default value for the carrying stopped color.
const ALLEGRO_COLOR DEF_CARRYING_COLOR_STOP =
{ 0.40f, 0.75f, 0.75f, 1.00f };

//Default value for the carrying speed base multiplier.
const float DEF_CARRYING_SPEED_BASE_MULT = 0.5f;

//Default value for the carrying speed maximum multiplier.
const float DEF_CARRYING_SPEED_MAX_MULT = 0.8f;

//Default value for the carrying speed weight multiplier.
const float DEF_CARRYING_SPEED_WEIGHT_MULT = 0.0004f;

//Default value for the cursor maximum distance.
const float DEF_CURSOR_MAX_DIST = 200.0f;

//Default value for the cursor spin speed.
const float DEF_CURSOR_SPIN_SPEED = 180.0f;

//Default value for the day end time.
const float DEF_DAY_MINUTES_END = 60 * 19;

//Default value for the day time passing speed.
const float DEF_DAY_MINUTES_PER_IRL_SEC = 2;

//Default value for the day start time.
const float DEF_DAY_MINUTES_START = 60 * 7;

//Default value for the group member grab range.
const float DEF_GROUP_MEMBER_GRAB_RANGE = 64.0f;

//Default value for the idle Pikmin bump delay.
const float DEF_IDLE_BUMP_DELAY = 5.0f;

//Default value for the idle Pikmin bump range.
const float DEF_IDLE_BUMP_RANGE = 50.0f;

//Default value for the idle Pikmin task range.
const float DEF_IDLE_TASK_RANGE = 50.0f;

//Default value for the maturity power multiplier.
const float DEF_MATURITY_POWER_MULT = 0.1f;

//Default value for the maturity speed multiplier.
const float DEF_MATURITY_SPEED_MULT = 0.1f;

//Default value for the maximum number of Pikmin in the field.
const size_t DEF_MAX_PIKMIN_IN_FIELD = 100;

//Default value for the message character interval.
const float DEF_MESSAGE_CHAR_INTERVAL = 0.03f;

//Default value for the next Pikmin auto-pluck range.
const float DEF_NEXT_PLUCK_RANGE = 160.0f;

//Default value for the color that represents no Pikmin.
const ALLEGRO_COLOR DEF_NO_PIKMIN_COLOR = { 0.75f, 0.90f, 0.90f, 1.0f };

//Default value for the Onion opening range.
const float DEF_ONION_OPEN_RANGE = 24.0f;

//Default value for the Pikmin chase range.
const float DEF_PIKMIN_CHASE_RANGE = 200.0f;

//Default value for the pluck range.
const float DEF_PLUCK_RANGE = 30.0f;

//Default value for the radar background color.
const ALLEGRO_COLOR DEF_RADAR_BG_COLOR = al_map_rgb(32, 24, 0);

//Default value for the radar edge color.
const ALLEGRO_COLOR DEF_RADAR_EDGE_COLOR = DEF_RADAR_BG_COLOR;

//Default value for the radar highest sector color.
const ALLEGRO_COLOR DEF_RADAR_HIGHEST_COLOR = al_map_rgb(200, 200, 180);

//Default value for the radar lowest sector color.
const ALLEGRO_COLOR DEF_RADAR_LOWEST_COLOR = al_map_rgb(80, 64, 0);

//Default value for the standard leader height.
const float DEF_STANDARD_LEADER_HEIGHT = 46.0f;

//Default value for the standard leader radius.
const float DEF_STANDARD_LEADER_RADIUS = 16.0f;

//Default value for the standard Pikmin height.
const float DEF_STANDARD_PIKMIN_HEIGHT = 24.0f;

//Default value for the standard Pikmin radius.
const float DEF_STANDARD_PIKMIN_RADIUS = 5.0f;

//Default value for the swarming task range.
const float DEF_SWARM_TASK_RANGE = 3.0f;

//Default value for the maximum throw distance.
const float DEF_THROW_MAX_DIST = DEF_CURSOR_MAX_DIST;

//Default value for the whistle growth speed.
const float DEF_WHISTLE_GROWTH_SPEED = 180.0f;

//Default value for the maximum whistle distance.
const float DEF_WHISTLE_MAX_DIST = DEF_CURSOR_MAX_DIST;

//Default value for the maximum zoom level.
const float DEF_ZOOM_MAX_LEVEL = 3.0f;

//Default value for the minimum zoom level.
const float DEF_ZOOM_MIN_LEVEL = 0.66f;

}


/**
 * @brief Loads the game's config from a file.
 *
 * @param file File to load from.
 */
void game_config::load(data_node* file) {
    reader_setter rs(file);
    string pikmin_order_str;
    string leader_order_str;
    string spray_order_str;
    
    rs.set("game_name", name);
    rs.set("game_version", version);
    
    rs.set("carrying_color_move", carrying_color_move);
    rs.set("carrying_color_stop", carrying_color_stop);
    rs.set("carrying_speed_base_mult", carrying_speed_base_mult);
    rs.set("carrying_speed_max_mult", carrying_speed_max_mult);
    rs.set("carrying_speed_weight_mult", carrying_speed_weight_mult);
    
    rs.set("day_minutes_start", day_minutes_start);
    rs.set("day_minutes_end", day_minutes_end);
    
    rs.set("pikmin_order", pikmin_order_str);
    rs.set("standard_pikmin_height", standard_pikmin_height);
    rs.set("standard_pikmin_radius", standard_pikmin_radius);
    
    rs.set("leader_order", leader_order_str);
    rs.set("standard_leader_height", standard_leader_height);
    rs.set("standard_leader_radius", standard_leader_radius);
    rs.set("spray_order", spray_order_str);
    
    rs.set("idle_bump_delay", idle_bump_delay);
    rs.set("idle_bump_range", idle_bump_range);
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
    rs.set("no_pikmin_color", no_pikmin_color);
    rs.set("onion_open_range", onion_open_range);
    rs.set("group_member_grab_range", group_member_grab_range);
    rs.set("pluck_range", pluck_range);
    rs.set("throw_max_dist", throw_max_dist);
    rs.set("whistle_growth_speed", whistle_growth_speed);
    rs.set("whistle_max_dist", whistle_max_dist);

    rs.set("radar_background_color", radar_background_color);
    rs.set("radar_edge_color", radar_edge_color);
    rs.set("radar_highest_color", radar_highest_color);
    rs.set("radar_lowest_color", radar_lowest_color);
    
    rs.set("message_char_interval", message_char_interval);
    rs.set("zoom_max_level", zoom_max_level);
    rs.set("zoom_min_level", zoom_min_level);
    
    pikmin_order_strings = semicolon_list_to_vector(pikmin_order_str);
    leader_order_strings = semicolon_list_to_vector(leader_order_str);
    spray_order_strings = semicolon_list_to_vector(spray_order_str);
    cursor_spin_speed = deg_to_rad(cursor_spin_speed);
}
