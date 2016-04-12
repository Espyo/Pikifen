/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used constants.
 */

#ifndef CONST_INCLUDED
#define CONST_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

using namespace std;

/*
 * Version stuff.
 * On every release, update these numbers, and
 * update the numbers on the resouce (.rc) file.
 */
const unsigned char VERSION_MAJOR = 0;
const unsigned char VERSION_MINOR = 7;
const unsigned char VERSION_REV   = 1;
const unsigned char VERSION_DAY   = 26;
const unsigned char VERSION_MONTH = 3;
const unsigned int  VERSION_YEAR  = 16;      //The year is 2000 + this.

const float AREA_TITLE_FADE_DURATION    = 3.0f;      //How long it takes for the area name to fade away, in-game.
const float CURSOR_INVALID_EFFECT_SPEED = M_PI * 4.0f; //How fast the "invalid cursor" effect goes, per second.
const float CURSOR_SAVE_INTERVAL        = 0.03f;     //Every X seconds, the cursor's position is saved, to create the trail effect.
const unsigned char CURSOR_SAVE_N_SPOTS = 10;        //Number of positions of the cursor to keep track of.
const float DEF_PUNCH_STRENGTH          = 2.0f;      //Default leader punch strength.
const float DEF_ROTATION_SPEED          = M_PI * 3.5f; //The default rotation speed of a mob type.
const float DEF_WHISTLE_RANGE           = 80.0f;     //The whistle can't go past this radius, by default.
const float DELIVERY_SUCK_TIME          = 1.5f;      //How long to suck a mob in for, when being delivered to an Onion/ship.
const float DISMISS_DISTANCE            = 64.0f;     //Dismissed Pikmin go these many units away from the leader.
const float GROUP_MOVE_ARROW_SPEED      = 400.0f;    //"Move group" arrows move these many units per second.
const float GROUP_MOVE_ARROWS_INTERVAL  = 0.1f;      //Seconds that need to pass before another "move group" arrow appears.
const float IDLE_GLOW_SPIN_SPEED        = M_PI_2;    //The idle glow spins these many radians per second.
const float INFO_PRINT_DURATION         = 10.0f;     //How long the on-screen info is printed for.
const float MOUSE_CURSOR_MOVE_SPEED     = 500.0f;    //How many pixels the mouse cursor moves, per second, when using an analog stick.
const float GROUP_SPOT_INTERVAL         = 1.0f;      //Pikmin must be at least these many units away from one another; used when calculating group spots.
const float MOB_SHADOW_STRETCH_MULT     = 0.5f;      //Multiply the stretch of the shadow by this much.
const float MOB_SHADOW_Y_MULT           = 0.2f;      //For every unit above the ground that the mob is on, the shadow goes these many units to the side.
const float SHIP_BEAM_RANGE             = 30.0f;     //The center of a ship's beam reaches this far.
const unsigned int SHIP_BEAM_RING_COLOR_SPEED = 255; //Red color's index moves these many units per second. (Green is fast and blue is faster still).
const float SMACK_PARTICLE_DUR          = 0.1f;      //Duration of the "smack" particle.
const float SUN_METER_SUN_SPIN_SPEED    = 0.5f;      //The Sun Meter's sun spins these many radians per second.
const float TREE_SHADOW_SWAY_AMOUNT     = 8.0f;      //Tree shadows sway this much away from their neutral position.
const float TREE_SHADOW_SWAY_SPEED      = M_PI_4;    //Tree shadows sway this much per second (M_PI * 2 = full back-and-forth cycle).
const float THROW_DISTANCE_MULTIPLIER   = 0.49f;     //When a leader throws a Pikmin, multiply the horizontal distance by 1/this.
const float THROW_PARTICLE_INTERVAL     = 0.02f;     //A new "mob thrown" particle is spawned every X seconds.
const float THROW_STRENGTH_MULTIPLIER   = 0.65f;     //When a leader throws a Pikmin, multiply the strength by this.
const float WHISTLE_DOT_INTERVAL        = 0.03;      //Seconds that need to pass before another dot is added.
const float WHISTLE_DOT_SPIN_SPEED      = M_PI_2;    //A whistle dot spins these many radians a second.
const float WHISTLE_FADE_TIME           = 0.1f;      //Time the whistle animations take to fade out.
const float WHISTLE_RING_SPEED          = 600.0f;    //Whistle rings move these many units per second.
const float WHISTLE_RINGS_INTERVAL      = 0.1f;      //Seconds that need to pass before another whistle ring appears.
const float ZOOM_MAX_LEVEL_EDITOR       = 4.0f;      //Maximum zoom level possible on the area editor.
const float ZOOM_MIN_LEVEL_EDITOR       = 0.05f;     //Minimum zoom level possible on the area editor.

const unsigned int DEF_FPS   = 60;
const unsigned int DEF_SCR_W = 640;
const unsigned int DEF_SCR_H = 480;

const string GAME_DATA_FOLDER      = "Game_data";
const string TYPES_FOLDER          = GAME_DATA_FOLDER + "/Types";

const string AUDIO_FOLDER          = GAME_DATA_FOLDER + "/Audio";
const string AREA_FOLDER           = GAME_DATA_FOLDER + "/Areas";
const string ENEMIES_FOLDER        = TYPES_FOLDER + "/Enemies";
const string GATES_FOLDER          = TYPES_FOLDER + "/Gates";
const string GRAPHICS_FOLDER       = GAME_DATA_FOLDER + "/Graphics";
const string LEADERS_FOLDER        = TYPES_FOLDER + "/Leaders";
const string MISC_FOLDER           = GAME_DATA_FOLDER + "/Misc";
const string ONIONS_FOLDER         = TYPES_FOLDER + "/Onions";
const string OTHER_TYPES_FOLDER    = TYPES_FOLDER + "/Others";
const string PELLETS_FOLDER        = TYPES_FOLDER + "/Pellets";
const string PIKMIN_FOLDER         = TYPES_FOLDER + "/Pikmin";
const string SPECIAL_MOBS_FOLDER   = TYPES_FOLDER + "/Special";
const string SHIPS_FOLDER          = TYPES_FOLDER + "/Ships";
const string TEXTURES_FOLDER       = GRAPHICS_FOLDER + "/Textures";
const string TREASURES_FOLDER      = TYPES_FOLDER + "/Treasures";
const string WEATHER_FILE          = MISC_FOLDER + "/Weather.txt";
const string CONFIG_FILE           = MISC_FOLDER + "/Config.txt";


enum EDITOR_MODES {
    //Common.
    EDITOR_MODE_MAIN,
    EDITOR_MODE_TOOLS,
    EDITOR_MODE_OPTIONS,
    //Animation editor.
    EDITOR_MODE_ANIMATION,
    EDITOR_MODE_FRAME,
    EDITOR_MODE_HITBOX,
    EDITOR_MODE_HITBOX_INSTANCES,
    EDITOR_MODE_TOP,
    //Area editor
    EDITOR_MODE_SECTORS,
    EDITOR_MODE_ADV_TEXTURE_SETTINGS,
    EDITOR_MODE_TEXTURE,
    EDITOR_MODE_OBJECTS,
    EDITOR_MODE_PATHS,
    EDITOR_MODE_SHADOWS,
    EDITOR_MODE_GUIDE,
    EDITOR_MODE_REVIEW,
};

enum EDITOR_SEC_MODES {
    ESM_NONE,
    ESM_NEW_SECTOR,
    ESM_NEW_OBJECT,
    ESM_DUPLICATE_OBJECT,
    ESM_NEW_STOP,
    ESM_NEW_LINK1,  //Click #1.
    ESM_NEW_LINK2,  //Click #2.
    ESM_NEW_1WLINK1, //One-way link, click #1.
    ESM_NEW_1WLINK2, //One-way link, click #2.
    ESM_DEL_STOP,
    ESM_DEL_LINK,
    ESM_NEW_SHADOW,
    ESM_GUIDE_MOUSE,   //Guide transformation being controlled by mouse.
    ESM_TEXTURE_VIEW,
};

enum DEV_TOOL_IDS {
    DEV_TOOL_NONE,
    DEV_TOOL_AREA_IMAGE,
    DEV_TOOL_COORDINATES,
    DEV_TOOL_CHANGE_SPEED,
    DEV_TOOL_HURT_MOB,
    DEV_TOOL_MOB_INFO,
    DEV_TOOL_NEW_PIKMIN,
    DEV_TOOL_TELEPORT,
};

enum HUD_ELEMENTS {
    HUD_ITEM_TIME,
    HUD_ITEM_DAY_BUBBLE,
    HUD_ITEM_DAY_NUMBER,
    HUD_ITEM_LEADER_1_ICON,
    HUD_ITEM_LEADER_2_ICON,
    HUD_ITEM_LEADER_3_ICON,
    HUD_ITEM_LEADER_1_HEALTH,
    HUD_ITEM_LEADER_2_HEALTH,
    HUD_ITEM_LEADER_3_HEALTH,
    HUD_ITEM_PIKMIN_STANDBY_ICON,
    HUD_ITEM_PIKMIN_STANDBY_NR,
    HUD_ITEM_PIKMIN_STANDBY_X,
    HUD_ITEM_PIKMIN_GROUP_NR,
    HUD_ITEM_PIKMIN_FIELD_NR,
    HUD_ITEM_PIKMIN_TOTAL_NR,
    HUD_ITEM_PIKMIN_SLASH_1,
    HUD_ITEM_PIKMIN_SLASH_2,
    HUD_ITEM_PIKMIN_SLASH_3,
    HUD_ITEM_SPRAY_1_ICON,
    HUD_ITEM_SPRAY_1_AMOUNT,
    HUD_ITEM_SPRAY_1_KEY,
    HUD_ITEM_SPRAY_2_ICON,
    HUD_ITEM_SPRAY_2_AMOUNT,
    HUD_ITEM_SPRAY_2_KEY,
    HUD_ITEM_SPRAY_PREV_ICON,
    HUD_ITEM_SPRAY_PREV_KEY,
    HUD_ITEM_SPRAY_NEXT_ICON,
    HUD_ITEM_SPRAY_NEXT_KEY,
    
    N_HUD_ITEMS,
};

const unsigned char N_WHISTLE_RING_COLORS = 8;
const unsigned char WHISTLE_RING_COLORS[N_WHISTLE_RING_COLORS][3] = {
    {255, 255, 0  },
    {255, 0,   0  },
    {255, 0,   255},
    {128, 0,   255},
    {0,   0,   255},
    {0,   255, 255},
    {0,   255, 0  },
    {128, 255, 0  }
};

const size_t INVALID = UINT32_MAX;

#endif //ifndef CONST_INCLUDED
