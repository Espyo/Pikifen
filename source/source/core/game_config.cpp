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
        ReaderSetter ars(file->getChildByName("aesthetic_general"));
        
        ars.set("carrying_color_move", aestheticGen.carryingColorMove);
        ars.set("carrying_color_stop", aestheticGen.carryingColorStop);
        ars.set("cursor_spin_speed", aestheticGen.cursorSpinSpeed);
        ars.set("gameplay_msg_char_interval", aestheticGen.gameplayMsgChInterval);
        ars.set("no_pikmin_color", aestheticGen.noPikminColor);
        
        aestheticGen.cursorSpinSpeed =
            degToRad(aestheticGen.cursorSpinSpeed);
    }
    
    //Aesthetic radar.
    {
        ReaderSetter ars(file->getChildByName("aesthetic_radar"));
        
        ars.set("background_color", aestheticRadar.backgroundColor);
        ars.set("edge_color", aestheticRadar.edgeColor);
        ars.set("highest_color", aestheticRadar.highestColor);
        ars.set("lowest_color", aestheticRadar.lowestColor);
    }
    
    //Carrying.
    {
        ReaderSetter crs(file->getChildByName("carrying"));
        
        crs.set("speed_base_mult", carrying.speedBaseMult);
        crs.set("speed_max_mult", carrying.speedMaxMult);
        crs.set("speed_weight_mult", carrying.speedWeightMult);
    }
    
    //General.
    {
        ReaderSetter grs(file->getChildByName("general"));
        
        grs.set("game_name", general.name);
        grs.set("game_version", general.version);
    }
    
    //Leaders.
    {
        ReaderSetter lrs(file->getChildByName("leaders"));
        string leader_order_str;
        
        lrs.set("group_member_grab_range", leaders.groupMemberGrabRange);
        lrs.set("next_pluck_range", leaders.nextPluckRange);
        lrs.set("onion_open_range", leaders.onionOpenRange);
        lrs.set("order", leader_order_str);
        lrs.set("pluck_range", leaders.pluckRange);
        lrs.set("standard_height", leaders.standardHeight);
        lrs.set("standard_radius", leaders.standardRadius);
        
        leaders.orderStrings = semicolonListToVector(leader_order_str);
    }
    
    //Misc.
    {
        ReaderSetter mrs(file->getChildByName("misc"));
        string spray_order_str;
        
        mrs.set("day_minutes_end", misc.dayMinutesEnd);
        mrs.set("day_minutes_start", misc.dayMinutesStart);
        mrs.set("spray_order", spray_order_str);
        
        misc.sprayOrderStrings = semicolonListToVector(spray_order_str);
    }
    
    //Pikmin.
    {
        ReaderSetter prs(file->getChildByName("pikmin"));
        string pikmin_order_str;
        
        prs.set("chase_range", pikmin.chaseRange);
        prs.set("idle_bump_delay", pikmin.idleBumpDelay);
        prs.set("idle_bump_range", pikmin.idleBumpRange);
        prs.set("idle_task_range", pikmin.idleTaskRange);
        prs.set("maturity_power_mult", pikmin.maturityPowerMult);
        prs.set("maturity_speed_mult", pikmin.maturitySpeedMult);
        prs.set("order", pikmin_order_str);
        prs.set("standard_height", pikmin.standardHeight);
        prs.set("standard_radius", pikmin.standardRadius);
        prs.set("swarm_task_range", pikmin.swarmTaskRange);
        
        pikmin.orderStrings = semicolonListToVector(pikmin_order_str);
    }
    
    //Rules.
    {
        ReaderSetter rrs(file->getChildByName("rules"));
        
        rrs.set("can_throw_leaders", rules.canThrowLeaders);
        rrs.set("cursor_max_dist", rules.cursorMaxDist);
        rrs.set("max_pikmin_in_field", rules.maxPikminInField);
        rrs.set("throw_max_dist", rules.throwMaxDist);
        rrs.set("whistle_growth_speed", rules.whistleGrowthSpeed);
        rrs.set("whistle_max_dist", rules.whistleMaxDist);
        rrs.set("zoom_closest_reach", rules.zoomClosestReach);
        rrs.set("zoom_farthest_reach", rules.zoomFarthestReach);
    }
}
