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

#pragma once

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../content/mob_type/leader_type.h"
#include "../content/mob_type/pikmin_type.h"
#include "../content/other/spray_type.h"


using std::size_t;
using std::vector;


namespace GAME_CONFIG {

namespace AESTHETIC_GENERAL_D {
extern const ALLEGRO_COLOR CARRYING_COLOR_MOVE;
extern const ALLEGRO_COLOR CARRYING_COLOR_STOP;
extern const float CURSOR_SPIN_SPEED;
extern const float GAMEPLAY_MSG_CHAR_INTERVAL;
extern const ALLEGRO_COLOR NO_PIKMIN_COLOR;
}

namespace AESTHETIC_RADAR_D {
extern const ALLEGRO_COLOR BG_COLOR;
extern const ALLEGRO_COLOR EDGE_COLOR;
extern const ALLEGRO_COLOR HIGHEST_COLOR;
extern const ALLEGRO_COLOR LOWEST_COLOR;
}

namespace CARRYING_D {
extern const float SPEED_BASE_MULT;
extern const float SPEED_MAX_MULT;
extern const float SPEED_WEIGHT_MULT;
}

namespace LEADERS_D {
extern const float GROUP_MEMBER_GRAB_RANGE;
extern const float NEXT_PLUCK_RANGE;
extern const float ONION_OPEN_RANGE;
extern const float PLUCK_RANGE;
extern const float STANDARD_HEIGHT;
extern const float STANDARD_RADIUS;
}

namespace MISC_D {
extern const float DAY_MINUTES_END;
extern const float DAY_MINUTES_START;
}

namespace PIKMIN_D {
extern const float CHASE_RANGE;
extern const float IDLE_BUMP_DELAY;
extern const float IDLE_BUMP_RANGE;
extern const float IDLE_TASK_RANGE;
extern const float MATURITY_POWER_MULT;
extern const float MATURITY_SPEED_MULT;
extern const float STANDARD_HEIGHT;
extern const float STANDARD_RADIUS;
extern const float SWARM_TASK_RANGE;
}
    
namespace RULES_D {
extern const bool CAN_THROW_LEADERS;
extern const float CURSOR_MAX_DIST;
extern const size_t MAX_PIKMIN_IN_FIELD;
extern const float THROW_MAX_DIST;
extern const float WHISTLE_GROWTH_SPEED;
extern const float WHISTLE_MAX_DIST;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}

}


using namespace GAME_CONFIG;

/**
 * @brief The game's configuration. It controls some rules about the game.
 */
struct GameConfig {

    //--- Members ---

    //General aesthetic details.
    struct {

        //Color that represents a non-Onion carriable object when moving.
        ALLEGRO_COLOR carrying_color_move =
            AESTHETIC_GENERAL_D::CARRYING_COLOR_MOVE;
        
        //Color that represents a non-Onion carriable object when stopped.
        ALLEGRO_COLOR carrying_color_stop =
            AESTHETIC_GENERAL_D::CARRYING_COLOR_STOP;
    
        //How much the cursor spins per second.
        float cursor_spin_speed =
            AESTHETIC_GENERAL_D::CURSOR_SPIN_SPEED;
    
        //These many seconds until a new character of
        //the gameplay message is drawn.
        float g_msg_ch_interval =
            AESTHETIC_GENERAL_D::GAMEPLAY_MSG_CHAR_INTERVAL;
        
        //Color that represents the absence of Pikmin.
        ALLEGRO_COLOR no_pikmin_color =
            AESTHETIC_GENERAL_D::NO_PIKMIN_COLOR;
        
    } aesthetic_gen;

    //Radar aesthetic details.
    struct {

        //Color of the background in the radar.
        ALLEGRO_COLOR background_color = AESTHETIC_RADAR_D::BG_COLOR;
        
        //Color of edges in the radar.
        ALLEGRO_COLOR edge_color = AESTHETIC_RADAR_D::EDGE_COLOR;
        
        //Color for the highest sector in the radar.
        ALLEGRO_COLOR highest_color = AESTHETIC_RADAR_D::HIGHEST_COLOR;
        
        //Color for the lowest sector in the radar.
        ALLEGRO_COLOR lowest_color = AESTHETIC_RADAR_D::LOWEST_COLOR;
    
    } aesthetic_radar;

    //Carrying information.
    struct {
        
        //Used for the slowest carrying speed an object can go.
        float speed_base_mult = CARRYING_D::SPEED_BASE_MULT;
        
        //Used for the fastest carrying speed an object can go.
        float speed_max_mult = CARRYING_D::SPEED_MAX_MULT;
        
        //Decreases carry speed by this much per unit of weight.
        float speed_weight_mult = CARRYING_D::SPEED_WEIGHT_MULT;

    } carrying;

    //General game info.
    struct {
    
        //Name of the game.
        string name;
        
        //Version of the game.
        string version;

    } general;

    //Leader-related properties.
    struct {

        //A leader can grab a group member only within this range.
        float group_member_grab_range = LEADERS_D::GROUP_MEMBER_GRAB_RANGE;
        
        //How far a leader can go to auto-pluck the next Pikmin.
        float next_pluck_range = LEADERS_D::NEXT_PLUCK_RANGE;
        
        //Onions can be opened if the leader is within this distance.
        float onion_open_range = LEADERS_D::ONION_OPEN_RANGE;

        //List of leader types, ordered by the game configuration.
        vector<LeaderType*> order;
    
        //Loaded strings representing the standard leader order. Used for later.
        vector<string> order_strings;
        
        //A leader can start the plucking mode if they're this close.
        float pluck_range = LEADERS_D::PLUCK_RANGE;
        
        //A standard leader is this tall.
        float standard_height = LEADERS_D::STANDARD_HEIGHT;
    
        //A standard leader has this radius.
        float standard_radius = LEADERS_D::STANDARD_RADIUS;

    } leaders;

    //Misc.
    struct {
        
        //The day ends when the in-game minutes reach this value.
        float day_minutes_end = GAME_CONFIG::MISC_D::DAY_MINUTES_END;
        
        //The in-game minutes start with this value every day.
        float day_minutes_start = GAME_CONFIG::MISC_D::DAY_MINUTES_START;

        //List of spray types, ordered by the game configuration.
        vector<SprayType*> spray_order;
        
        //Loaded strings representing the standard spray order. Used for later.
        vector<string> spray_order_strings;
    
    } misc;

    //Pikmin-related properties.
    struct {

        //Pikmin will only chase enemies in this range.
        float chase_range = PIKMIN_D::CHASE_RANGE;
            
        //Idle Pikmin are only bumped if away from a leader for these many secs.
        float idle_bump_delay = PIKMIN_D::IDLE_BUMP_DELAY;
            
        //Idle Pikmin will be bumped by a leader within this distance.
        float idle_bump_range = PIKMIN_D::IDLE_BUMP_RANGE;

        //Idle Pikmin will go for a task if they are within this distance of it.
        float idle_task_range = PIKMIN_D::IDLE_TASK_RANGE;

        //Every level of maturity, multiply the attack power by 1 + this much.
        float maturity_power_mult = PIKMIN_D::MATURITY_POWER_MULT;

        //Every level of maturity, multiply the speed by 1 + this much.
        float maturity_speed_mult = PIKMIN_D::MATURITY_SPEED_MULT;

        //List of Pikmin types, ordered by the game configuration.
        vector<PikminType*> order;

        //Loaded strings representing the standard Pikmin order. Used for later.
        vector<string> order_strings;

        //A standard Pikmin is this tall.
        float standard_height = PIKMIN_D::STANDARD_HEIGHT;

        //A standard Pikmin has this radius.
        float standard_radius = PIKMIN_D::STANDARD_RADIUS;

        //Pikmin that are swarming can go for tasks within this range.
        float swarm_task_range = PIKMIN_D::SWARM_TASK_RANGE;

    } pikmin;

    //General gameplay rules.
    struct {

        //Can a leader throw other leaders?
        bool can_throw_leaders = RULES_D::CAN_THROW_LEADERS;
    
        //Maximum distance from the leader the cursor can go.
        float cursor_max_dist = RULES_D::CURSOR_MAX_DIST;
        
        //Maximum number of Pikmin that can be out in the field at once.
        size_t max_pikmin_in_field = RULES_D::MAX_PIKMIN_IN_FIELD;

        //Maximum distance from the leader that a throw can be aimed to.
        float throw_max_dist = RULES_D::THROW_MAX_DIST;
        
        //Speed at which the whistle grows.
        float whistle_growth_speed = RULES_D::WHISTLE_GROWTH_SPEED;
        
        //Maximum distance from the leader that the whistle can start from.
        float whistle_max_dist = RULES_D::WHISTLE_MAX_DIST;
        
        //The closest zoom level the player can get.
        float zoom_max_level = RULES_D::ZOOM_MAX_LEVEL;
        
        //The farthest zoom level the player can get.
        float zoom_min_level = RULES_D::ZOOM_MIN_LEVEL;

    } rules;

    
    //--- Function declarations ---
    
    void load(DataNode* file);
    
};
