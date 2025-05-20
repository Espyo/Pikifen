/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Struct that holds the game's configuration, and related functions.
 */

#include "../util/string_utils.h"
#include "game_config.h"


namespace GAME_CONFIG {

namespace AESTHETIC_GENERAL_D {

//Default value for the non-specific carrying movement color.
const ALLEGRO_COLOR CARRYING_COLOR_MOVE =
{ 1.00f, 1.00f, 1.00f, 1.00f};

//Default value for the carrying stopped color.
const ALLEGRO_COLOR CARRYING_COLOR_STOP =
{ 0.38f, 0.75f, 0.75f, 1.00f };

//Default value for the cursor spin speed.
const float CURSOR_SPIN_SPEED = degToRad(180.0f);

//Default value for the gameplay message character interval.
const float GAMEPLAY_MSG_CHAR_INTERVAL = 0.03f;

//Default value for the color that represents no Pikmin.
const ALLEGRO_COLOR NO_PIKMIN_COLOR = { 0.66f, 0.74f, 0.90f, 1.0f };

}


namespace AESTHETIC_RADAR_D {

//Default value for the radar background color.
const ALLEGRO_COLOR BG_COLOR = al_map_rgb(32, 24, 0);

//Default value for the radar edge color.
const ALLEGRO_COLOR EDGE_COLOR = BG_COLOR;

//Default value for the radar highest sector color.
const ALLEGRO_COLOR HIGHEST_COLOR = al_map_rgb(200, 200, 180);

//Default value for the radar lowest sector color.
const ALLEGRO_COLOR LOWEST_COLOR = al_map_rgb(80, 64, 0);

}


namespace CARRYING_D {

//Default value for the carrying speed base multiplier.
const float SPEED_BASE_MULT = 0.3f;

//Default value for the carrying speed maximum multiplier.
const float SPEED_MAX_MULT = 0.8f;

//Default value for the carrying speed weight multiplier.
const float SPEED_WEIGHT_MULT = 0.0004f;

}


namespace LEADERS_D {

//Default value for the group member grab range.
const float GROUP_MEMBER_GRAB_RANGE = 128.0f;

//Default value for the next Pikmin auto-pluck range.
const float NEXT_PLUCK_RANGE = 200.0f;

//Default value for the Onion opening range.
const float ONION_OPEN_RANGE = 24.0f;

//Default value for the pluck range.
const float PLUCK_RANGE = 32.0f;

//Default value for the standard leader height.
const float STANDARD_HEIGHT = 46.0f;

//Default value for the standard leader radius.
const float STANDARD_RADIUS = 16.0f;

}


namespace MISC_D {

//Default value for the day end time.
const float DAY_MINUTES_END = 60 * 19;

//Default value for the day start time.
const float DAY_MINUTES_START = 60 * 7;

}


namespace PIKMIN_D {

//Default value for the Pikmin chase range.
const float CHASE_RANGE = 200.0f;

//Default value for the idle Pikmin bump delay.
const float IDLE_BUMP_DELAY = 5.0f;

//Default value for the idle Pikmin bump range.
const float IDLE_BUMP_RANGE = 50.0f;

//Default value for the idle Pikmin task range.
const float IDLE_TASK_RANGE = 50.0f;

//Default value for the maturity power multiplier.
const float MATURITY_POWER_MULT = 0.1f;

//Default value for the maturity speed multiplier.
const float MATURITY_SPEED_MULT = 0.1f;

//Default value for the standard Pikmin height.
const float STANDARD_HEIGHT = 24.0f;

//Default value for the standard Pikmin radius.
const float STANDARD_RADIUS = 5.0f;

//Default value for the swarming task range.
const float SWARM_TASK_RANGE = 3.0f;

}


namespace RULES_D {

//Default value for whether leaders can throw leaders.
const bool CAN_THROW_LEADERS = true;

//Default value for the cursor maximum distance.
const float CURSOR_MAX_DIST = 200.0f;

//Default value for the maximum number of Pikmin in the field.
const size_t MAX_PIKMIN_IN_FIELD = 100;

//Default value for the maximum throw distance.
const float THROW_MAX_DIST = CURSOR_MAX_DIST;

//Default value for the whistle growth speed.
const float WHISTLE_GROWTH_SPEED = 180.0f;

//Default value for the maximum whistle distance.
const float WHISTLE_MAX_DIST = CURSOR_MAX_DIST;

//Default value for the zoom closest reach.
const float ZOOM_CLOSEST_REACH = 295.0f;

//Default value for the zoom farthest reach.
const float ZOOM_FARTHEST_REACH = 1340.0f;

}


}


/**
 * @brief Loads the game's config from a file.
 *
 * @param file File to load from.
 */
void GameConfig::load(DataNode* file) {

    //Aesthetic general.
    {
        ReaderSetter aRS(file->getChildByName("aesthetic_general"));
        
        aRS.set("carrying_color_move", aestheticGen.carryingColorMove);
        aRS.set("carrying_color_stop", aestheticGen.carryingColorStop);
        aRS.set("cursor_spin_speed", aestheticGen.cursorSpinSpeed);
        aRS.set(
            "gameplay_msg_char_interval", aestheticGen.gameplayMsgChInterval
        );
        aRS.set("no_pikmin_color", aestheticGen.noPikminColor);
        
        aestheticGen.cursorSpinSpeed =
            degToRad(aestheticGen.cursorSpinSpeed);
    }
    
    //Aesthetic radar.
    {
        ReaderSetter aRS(file->getChildByName("aesthetic_radar"));
        
        aRS.set("background_color", aestheticRadar.backgroundColor);
        aRS.set("edge_color", aestheticRadar.edgeColor);
        aRS.set("highest_color", aestheticRadar.highestColor);
        aRS.set("lowest_color", aestheticRadar.lowestColor);
    }
    
    //Carrying.
    {
        ReaderSetter cRS(file->getChildByName("carrying"));
        
        cRS.set("speed_base_mult", carrying.speedBaseMult);
        cRS.set("speed_max_mult", carrying.speedMaxMult);
        cRS.set("speed_weight_mult", carrying.speedWeightMult);
    }
    
    //General.
    {
        ReaderSetter gRS(file->getChildByName("general"));
        
        gRS.set("game_name", general.name);
        gRS.set("game_version", general.version);
    }
    
    //Leaders.
    {
        ReaderSetter lRS(file->getChildByName("leaders"));
        
        string leaderOrderStr;
        
        lRS.set("group_member_grab_range", leaders.groupMemberGrabRange);
        lRS.set("next_pluck_range", leaders.nextPluckRange);
        lRS.set("onion_open_range", leaders.onionOpenRange);
        lRS.set("order", leaderOrderStr);
        lRS.set("pluck_range", leaders.pluckRange);
        lRS.set("standard_height", leaders.standardHeight);
        lRS.set("standard_radius", leaders.standardRadius);
        
        leaders.orderStrings = semicolonListToVector(leaderOrderStr);
    }
    
    //Misc.
    {
        ReaderSetter mRS(file->getChildByName("misc"));
        
        string sprayOrderStr;
        
        mRS.set("day_minutes_end", misc.dayMinutesEnd);
        mRS.set("day_minutes_start", misc.dayMinutesStart);
        mRS.set("spray_order", sprayOrderStr);
        
        misc.sprayOrderStrings = semicolonListToVector(sprayOrderStr);
    }
    
    //Pikmin.
    {
        ReaderSetter pRS(file->getChildByName("pikmin"));
        
        string pikminOrderStr;
        
        pRS.set("chase_range", pikmin.chaseRange);
        pRS.set("idle_bump_delay", pikmin.idleBumpDelay);
        pRS.set("idle_bump_range", pikmin.idleBumpRange);
        pRS.set("idle_task_range", pikmin.idleTaskRange);
        pRS.set("maturity_power_mult", pikmin.maturityPowerMult);
        pRS.set("maturity_speed_mult", pikmin.maturitySpeedMult);
        pRS.set("order", pikminOrderStr);
        pRS.set("standard_height", pikmin.standardHeight);
        pRS.set("standard_radius", pikmin.standardRadius);
        pRS.set("swarm_task_range", pikmin.swarmTaskRange);
        
        pikmin.orderStrings = semicolonListToVector(pikminOrderStr);
    }
    
    //Rules.
    {
        ReaderSetter rRS(file->getChildByName("rules"));
        
        rRS.set("can_throw_leaders", rules.canThrowLeaders);
        rRS.set("cursor_max_dist", rules.cursorMaxDist);
        rRS.set("max_pikmin_in_field", rules.maxPikminInField);
        rRS.set("throw_max_dist", rules.throwMaxDist);
        rRS.set("whistle_growth_speed", rules.whistleGrowthSpeed);
        rRS.set("whistle_max_dist", rules.whistleMaxDist);
        rRS.set("zoom_closest_reach", rules.zoomClosestReach);
        rRS.set("zoom_farthest_reach", rules.zoomFarthestReach);
    }
}
