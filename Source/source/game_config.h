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


struct game_config {

    static const bool DEF_CAN_THROW_LEADERS;
    static const unsigned char DEF_CARRYING_COLOR_MOVE[3];
    static const unsigned char DEF_CARRYING_COLOR_STOP[3];
    static const float DEF_CARRYING_SPEED_BASE_MULT;
    static const float DEF_CARRYING_SPEED_MAX_MULT;
    static const float DEF_CARRYING_SPEED_WEIGHT_MULT;
    static const float DEF_CURSOR_MAX_DIST;
    static const float DEF_CURSOR_SPIN_SPEED;
    static const float DEF_DAY_MINUTES_END;
    static const float DEF_DAY_MINUTES_PER_IRL_SEC;
    static const float DEF_DAY_MINUTES_START;
    static const float DEF_GROUP_MEMBER_GRAB_RANGE;
    static const float DEF_IDLE_TASK_RANGE;
    static const float DEF_MATURITY_POWER_MULT;
    static const float DEF_MATURITY_SPEED_MULT;
    static const size_t DEF_MAX_PIKMIN_IN_FIELD;
    static const float DEF_MESSAGE_CHAR_INTERVAL;
    static const float DEF_NEXT_PLUCK_RANGE;
    static const float DEF_ONION_OPEN_RANGE;
    static const float DEF_PIKMIN_CHASE_RANGE;
    static const float DEF_PLUCK_RANGE;
    static const float DEF_STANDARD_PIKMIN_HEIGHT;
    static const float DEF_STANDARD_PIKMIN_RADIUS;
    static const float DEF_SWARM_TASK_RANGE;
    static const float DEF_WHISTLE_GROWTH_SPEED;
    static const float DEF_ZOOM_MAX_LEVEL;
    static const float DEF_ZOOM_MIN_LEVEL;
    
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
    //Maximum distance away from the leader the cursor can go.
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
    //Version of the game.
    string version;
    //Speed at which the whistle grows.
    float whistle_growth_speed;
    //The closest zoom level the player can get.
    float zoom_max_level;
    //The farthest zoom level the player can get.
    float zoom_min_level;
    
    void load(data_node* file);
    
    game_config();
};


#endif //ifndef GAME_CONFIG_INCLUDED
