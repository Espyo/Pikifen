/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the struct that holds the game's configuration,
 * and related functions.
 */

#ifndef GAME_CONFIG_INCLUDED
#define GAME_CONFIG_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "mob_types/leader_type.h"
#include "mob_types/pikmin_type.h"


using std::size_t;
using std::vector;


namespace GAME_CONFIG {
extern const bool DEF_CAN_THROW_LEADERS;
extern const ALLEGRO_COLOR DEF_CARRYING_COLOR_MOVE;
extern const ALLEGRO_COLOR DEF_CARRYING_COLOR_STOP;
extern const float DEF_CARRYING_SPEED_BASE_MULT;
extern const float DEF_CARRYING_SPEED_MAX_MULT;
extern const float DEF_CARRYING_SPEED_WEIGHT_MULT;
extern const float DEF_CURSOR_MAX_DIST;
extern const float DEF_CURSOR_SPIN_SPEED;
extern const float DEF_DAY_MINUTES_END;
extern const float DEF_DAY_MINUTES_PER_IRL_SEC;
extern const float DEF_DAY_MINUTES_START;
extern const float DEF_GROUP_MEMBER_GRAB_RANGE;
extern const float DEF_IDLE_TASK_RANGE;
extern const float DEF_MATURITY_POWER_MULT;
extern const float DEF_MATURITY_SPEED_MULT;
extern const size_t DEF_MAX_PIKMIN_IN_FIELD;
extern const float DEF_MESSAGE_CHAR_INTERVAL;
extern const float DEF_NEXT_PLUCK_RANGE;
extern const ALLEGRO_COLOR DEF_NO_PIKMIN_COLOR;
extern const float DEF_ONION_OPEN_RANGE;
extern const float DEF_PIKMIN_CHASE_RANGE;
extern const float DEF_PLUCK_RANGE;
extern const float DEF_STANDARD_PIKMIN_HEIGHT;
extern const float DEF_STANDARD_PIKMIN_RADIUS;
extern const float DEF_SWARM_TASK_RANGE;
extern const float DEF_THROW_MAX_DIST;
extern const float DEF_WHISTLE_GROWTH_SPEED;
extern const float DEF_WHISTLE_MAX_DIST;
extern const float DEF_ZOOM_MAX_LEVEL;
extern const float DEF_ZOOM_MIN_LEVEL;
}


/* ----------------------------------------------------------------------------
 * The game's configuration. It controls some rules about the game.
 */
struct game_config {
    //Can a leader throw other leaders?
    bool can_throw_leaders;
    //Color that represents a non-Onion carriable object when moving.
    ALLEGRO_COLOR carrying_color_move;
    //Color that represents a non-Onion carriable object when stopped.
    ALLEGRO_COLOR carrying_color_stop;
    //Used for the slowest carrying speed an object can go.
    float carrying_speed_base_mult;
    //Used for the fastest carrying speed an object can go.
    float carrying_speed_max_mult;
    //Decreases carry speed by this much per unit of weight.
    float carrying_speed_weight_mult;
    //Maximum distance from the leader the cursor can go.
    float cursor_max_dist;
    //How much the cursor spins per second.
    float cursor_spin_speed;
    //The day ends when the in-game minutes reach this value.
    float day_minutes_end;
    //Every real-life second, these many in-game minutes pass.
    float day_minutes_per_irl_sec;
    //The in-game minutes start with this value every day.
    float day_minutes_start;
    //A leader can grab a group member only within this range.
    float group_member_grab_range;
    //Idle Pikmin will go for a task if they are within this distance of it.
    float idle_task_range;
    //Standard leader order.
    vector<leader_type*> leader_order;
    //Loaded strings representing the standard leader order. Used for later.
    vector<string> leader_order_strings;
    //Every level of maturity, multiply the attack power by 1 + this much.
    float maturity_power_mult;
    //Every level of maturity, multiply the speed by 1 + this much.
    float maturity_speed_mult;
    //Maximum number of Pikmin that can be out in the field at once.
    size_t max_pikmin_in_field;
    //These many seconds until a new character of the message is drawn.
    float message_char_interval;
    //Name of the game.
    string name;
    //How far a leader can go to auto-pluck the next Pikmin.
    float next_pluck_range;
    //Color that represents the absence of Pikmin.
    ALLEGRO_COLOR no_pikmin_color;
    //Onions can be opened if the leader is within this distance.
    float onion_open_range;
    //Pikmin will only chase enemies in this range.
    float pikmin_chase_range;
    //Standard Pikmin order.
    vector<pikmin_type*> pikmin_order;
    //Loaded strings representing the standard Pikmin order. Used for later.
    vector<string> pikmin_order_strings;
    //A leader can start the plucking mode if they're this close.
    float pluck_range;
    //Loaded strings representing the standard spray order. Used for later.
    vector<string> spray_order_strings;
    //A standard Pikmin is this tall.
    float standard_pikmin_height;
    //A standard Pikmin has this radius.
    float standard_pikmin_radius;
    //Pikmin that are swarming can go for tasks within this range.
    float swarm_task_range;
    //Maximum distance from the leader that a throw can be aimed to.
    float throw_max_dist;
    //Version of the game.
    string version;
    //Speed at which the whistle grows.
    float whistle_growth_speed;
    //Maximum distance from the leader that the whistle can start from.
    float whistle_max_dist;
    //The closest zoom level the player can get.
    float zoom_max_level;
    //The farthest zoom level the player can get.
    float zoom_min_level;
    
    void load(data_node* file);
    
    game_config();
};


#endif //ifndef GAME_CONFIG_INCLUDED
