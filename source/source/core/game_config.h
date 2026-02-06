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


#pragma region Regions


namespace GAME_CONFIG {

namespace AESTHETIC_GENERAL_D {
extern const ALLEGRO_COLOR CARRYING_COLOR_MOVE;
extern const ALLEGRO_COLOR CARRYING_COLOR_STOP;
extern const float GAMEPLAY_MSG_CHAR_INTERVAL;
extern const float MOUSE_CURSOR_SPIN_SPEED;
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


namespace GUI_COLORS_D {
extern const ALLEGRO_COLOR BACK;
extern const ALLEGRO_COLOR BAD;
extern const ALLEGRO_COLOR FOCUSED_ITEM;
extern const ALLEGRO_COLOR GOLD;
extern const ALLEGRO_COLOR GOOD;
extern const ALLEGRO_COLOR PAGE_CHANGE;
extern const ALLEGRO_COLOR PAUSE_BG;
extern const ALLEGRO_COLOR PAUSE_VIGNETTE;
extern const ALLEGRO_COLOR SMALL_HEADER;
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
extern const float IDLE_TASK_RANGE;
extern const float MATURITY_POWER_MULT;
extern const float MATURITY_SPEED_MULT;
extern const float STANDARD_HEIGHT;
extern const float STANDARD_RADIUS;
extern const float SWARM_TASK_RANGE;
}


namespace RULES_D {
extern const bool CAN_THROW_LEADERS;
extern const float LEADER_CURSOR_MAX_DIST;
extern const size_t MAX_PIKMIN_IN_FIELD;
extern const float THROW_MAX_DIST;
extern const float WHISTLE_GROWTH_SPEED;
extern const float WHISTLE_MAX_DIST;
extern const float ZOOM_CLOSEST_REACH;
extern const float ZOOM_FARTHEST_REACH;
}


}


#pragma endregion
#pragma region Classes


using namespace GAME_CONFIG;


/**
 * @brief The game's configuration. It controls some rules about the game.
 */
struct GameConfig {

    //--- Public members ---
    
    //General aesthetic details.
    struct {
    
        //Color that represents a non-Onion carriable object when moving.
        ALLEGRO_COLOR carryingColorMove =
            AESTHETIC_GENERAL_D::CARRYING_COLOR_MOVE;
            
        //Color that represents a non-Onion carriable object when stopped.
        ALLEGRO_COLOR carryingColorStop =
            AESTHETIC_GENERAL_D::CARRYING_COLOR_STOP;
            
        //These many seconds until a new character of
        //the gameplay message is drawn.
        float gameplayMsgChInterval =
            AESTHETIC_GENERAL_D::GAMEPLAY_MSG_CHAR_INTERVAL;
            
        //How much the mouse cursor spins per second.
        float mouseCursorSpinSpeed =
            AESTHETIC_GENERAL_D::MOUSE_CURSOR_SPIN_SPEED;
            
        //Color that represents the absence of Pikmin.
        ALLEGRO_COLOR noPikminColor =
            AESTHETIC_GENERAL_D::NO_PIKMIN_COLOR;
            
    } aestheticGen;
    
    //Radar aesthetic details.
    struct {
    
        //Color of the background in the radar.
        ALLEGRO_COLOR backgroundColor = AESTHETIC_RADAR_D::BG_COLOR;
        
        //Color of edges in the radar.
        ALLEGRO_COLOR edgeColor = AESTHETIC_RADAR_D::EDGE_COLOR;
        
        //Color for the highest sector in the radar.
        ALLEGRO_COLOR highestColor = AESTHETIC_RADAR_D::HIGHEST_COLOR;
        
        //Color for the lowest sector in the radar.
        ALLEGRO_COLOR lowestColor = AESTHETIC_RADAR_D::LOWEST_COLOR;
        
    } aestheticRadar;
    
    //Carrying information.
    struct {
    
        //Used for the slowest carrying speed an object can go.
        float speedBaseMult = CARRYING_D::SPEED_BASE_MULT;
        
        //Used for the fastest carrying speed an object can go.
        float speedMaxMult = CARRYING_D::SPEED_MAX_MULT;
        
        //Decreases carry speed by this much per unit of weight.
        float speedWeightMult = CARRYING_D::SPEED_WEIGHT_MULT;
        
    } carrying;
    
    //General game info.
    struct {
    
        //Name of the game.
        string name;
        
        //Version of the game.
        string version;
        
    } general;
    
    //Some general GUI colors.
    struct {
    
        //"Back" buttons.
        ALLEGRO_COLOR back = GUI_COLORS_D::BACK;
        
        //Red for something bad.
        ALLEGRO_COLOR bad = GUI_COLORS_D::BAD;
        
        //Focused GUI item.
        ALLEGRO_COLOR focusedItem = GUI_COLORS_D::FOCUSED_ITEM;
        
        //Gold-like things.
        ALLEGRO_COLOR gold = GUI_COLORS_D::GOLD;
        
        //Green for something good.
        ALLEGRO_COLOR good = GUI_COLORS_D::GOOD;
        
        //Page change buttons.
        ALLEGRO_COLOR pageChange = GUI_COLORS_D::PAGE_CHANGE;
        
        //Pause background.
        ALLEGRO_COLOR pauseBg = GUI_COLORS_D::PAUSE_BG;
        
        //Pause vignette.
        ALLEGRO_COLOR pauseVignette = GUI_COLORS_D::PAUSE_VIGNETTE;
        
        //Small headers.
        ALLEGRO_COLOR smallHeader = GUI_COLORS_D::SMALL_HEADER;
        
    } guiColors;
    
    //Leader-related properties.
    struct {
    
        //A leader can grab a group member only within this range.
        float groupMemberGrabRange = LEADERS_D::GROUP_MEMBER_GRAB_RANGE;
        
        //How far a leader can go to auto-pluck the next Pikmin.
        float nextPluckRange = LEADERS_D::NEXT_PLUCK_RANGE;
        
        //Onions can be opened if the leader is within this distance.
        float onionOpenRange = LEADERS_D::ONION_OPEN_RANGE;
        
        //List of leader types, ordered by the game configuration.
        vector<LeaderType*> order;
        
        //Loaded strings representing the standard leader order. Used for later.
        vector<string> orderStrings;
        
        //A leader can start the plucking mode if they're this close.
        float pluckRange = LEADERS_D::PLUCK_RANGE;
        
        //A standard leader is this tall.
        float standardHeight = LEADERS_D::STANDARD_HEIGHT;
        
        //A standard leader has this radius.
        float standardRadius = LEADERS_D::STANDARD_RADIUS;
        
    } leaders;
    
    //Misc.
    struct {
    
        //The day ends when the in-game minutes reach this value.
        float dayMinutesEnd = GAME_CONFIG::MISC_D::DAY_MINUTES_END;
        
        //The in-game minutes start with this value every day.
        float dayMinutesStart = GAME_CONFIG::MISC_D::DAY_MINUTES_START;
        
        //List of spray types, ordered by the game configuration.
        vector<SprayType*> sprayOrder;
        
        //Loaded strings representing the standard spray order. Used for later.
        vector<string> sprayOrderStrings;
        
    } misc;
    
    //Pikmin-related properties.
    struct {
    
        //Pikmin will only chase enemies in this range.
        float chaseRange = PIKMIN_D::CHASE_RANGE;
        
        //Idle Pikmin are only bumped if away from a leader for these many secs.
        float idleBumpDelay = PIKMIN_D::IDLE_BUMP_DELAY;
        
        //Idle Pikmin will go for a task if they are within this distance of it.
        float idleTaskRange = PIKMIN_D::IDLE_TASK_RANGE;
        
        //Every level of maturity, multiply the attack power by 1 + this much.
        float maturityPowerMult = PIKMIN_D::MATURITY_POWER_MULT;
        
        //Every level of maturity, multiply the speed by 1 + this much.
        float maturitySpeedMult = PIKMIN_D::MATURITY_SPEED_MULT;
        
        //List of Pikmin types, ordered by the game configuration.
        vector<PikminType*> order;
        
        //Loaded strings representing the standard Pikmin order. Used for later.
        vector<string> orderStrings;
        
        //A standard Pikmin is this tall.
        float standardHeight = PIKMIN_D::STANDARD_HEIGHT;
        
        //A standard Pikmin has this radius.
        float standardRadius = PIKMIN_D::STANDARD_RADIUS;
        
        //Pikmin that are swarming can go for tasks within this range.
        float swarmTaskRange = PIKMIN_D::SWARM_TASK_RANGE;
        
    } pikmin;
    
    //General gameplay rules.
    struct {
    
        //Can a leader throw other leaders?
        bool canThrowLeaders = RULES_D::CAN_THROW_LEADERS;
        
        //Maximum distance from the leader their cursor can go.
        float leaderCursorMaxDist = RULES_D::LEADER_CURSOR_MAX_DIST;
        
        //Maximum number of Pikmin that can be out in the field at once.
        size_t maxPikminInField = RULES_D::MAX_PIKMIN_IN_FIELD;
        
        //Maximum distance from the leader that a throw can be aimed to.
        float throwMaxDist = RULES_D::THROW_MAX_DIST;
        
        //Speed at which the whistle grows.
        float whistleGrowthSpeed = RULES_D::WHISTLE_GROWTH_SPEED;
        
        //Maximum distance from the leader that the whistle can start from.
        float whistleMaxDist = RULES_D::WHISTLE_MAX_DIST;
        
        //The camera reach for the closest zoom level. In reality,
        //the camera keeps on-view an area of these many pixels squared.
        float zoomClosestReach = RULES_D::ZOOM_CLOSEST_REACH;
        
        //The camera reach for the farthest zoom level. In reality,
        //the camera keeps on-view an area of these many pixels squared.
        float zoomFarthestReach = RULES_D::ZOOM_FARTHEST_REACH;
        
    } rules;
    
    
    //--- Public function declarations ---
    
    void load(DataNode* file);
    
};


#pragma endregion
