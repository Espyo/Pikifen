/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used constants.
 */

#ifndef CONST_INCLUDED
#define CONST_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

using namespace std;

/*
 * Version stuff.
 * On every release, update these numbers, and
 * update the numbers on the resouce (.rc) file.
 */
#define VERSION_MAJOR 0
#define VERSION_MINOR 4
#define VERSION_REV 0
#define VERSION_DAY 0
#define VERSION_MONTH 0
#define VERSION_YEAR 0      //The year is 2000 + this.

#define AREA_IMAGE_SIZE             800      //How many pixels the area images are (both width and height; they're square).
#define AUTO_PLUCK_INPUT_INTERVAL   0.5      //How long the player has to press the pluck button again, to make the captain auto-pluck.
#define AUTO_PLUCK_MAX_RADIUS       160      //How far a leader can go to auto-pluck the next Pikmin.
#define CAM_TRANSITION_DURATION     0.5      //How many seconds a camera transition lasts for.
#define CURSOR_INVALID_EFFECT_SPEED M_PI * 4 //How fast the "invalid cursor" effect goes, per second.
#define CURSOR_SAVE_INTERVAL        0.03     //Every X seconds, the cursor's position is saved, to create the trail effect.
#define CURSOR_SAVE_N_SPOTS         10       //Number of positions of the cursor to keep track of.
#define CURSOR_SPIN_SPEED           M_PI     //How much the cursor spins per second.
#define CURSOR_MAX_DIST             200      //The cursor can only be these many units away from the captain.
#define DEF_PIKMIN_SIZE             24       //Default Pikmin size.
#define DEF_PUNCH_STRENGTH          2        //Default leader punch strength.
#define DEF_ROTATION_SPEED          M_PI * 2 //The default rotation speed of a mob type.
#define DEF_WHISTLE_RANGE           80       //The whistle can't go past this radius, by default.
#define DELIVERY_SUCK_TIME          0.5      //How long to suck a mob in for, when being delivered to an Onion/ship.
#define DISMISS_DISTANCE            64       //Dismissed Pikmin go these many units away from the captain.
#define IDLE_GLOW_SPIN_SPEED        M_PI_2   //The idle glow spins these many radians per second.
#define INFO_SPOT_TRIGGER_RANGE     64       //If the current captain is at this distance or closer from an info spot, it gets triggered.
#define LEADER_MOVE_SPEED           100      //Max speed at which a leader can move.
#define MATURITY_POWER_MULT         0.2      //Every level of maturity, this much is added to the power.
#define MATURITY_SPEED_MULT         0.5      //Every level of maturity, this much is added to the speed.
#define MESSAGE_CHAR_INTERVAL       0.02     //These many seconds until a new character of the message is drawn.
#define MIN_GRAB_RANGE              60       //The leader needs to be at least this close to a Pikmin to grab it.
#define MIN_ONION_CHECK_RANGE       64       //The minimum distance a leader must be from the onion in order to check it.
#define MIN_PLUCK_RANGE             30       //The leader needs to be at least this close to a buried Pikmin to pluck it.
#define MOUSE_CURSOR_MOVE_SPEED     500      //How many pixels the mouse cursor moves, per second, when using an analog stick.
#define MOVE_GROUP_ARROW_SPEED      400      //"Move group" arrows move these many units per second.
#define MOVE_GROUP_ARROWS_INTERVAL  0.1      //Seconds that need to pass before another "move group" arrow appears.
#define NECTAR_AMOUNT               5        //A drop of nectar starts with this amount.
#define N_PIKMIN_AI_PORTIONS        4        //Split the Pikmin into n groups. Every frame, only one group's AI is handled.
#define PARTY_SPOT_INTERVAL         1        //Pikmin must be at least these many units away from one another; used when calculating group spots.
#define PIKMIN_MAX_HEALTH           300      //Maximum Pikmin HP.
#define PIKMIN_MIN_ATTACK_RANGE     0        //If there's this gap between a Pikmin and its prey, the Pikmin will attack.
#define PIKMIN_MIN_TASK_RANGE       20       //If there's this gap between a Pikmin and a task, the Pikmin will take the task.
#define SHADOW_MAX_WIDTH            100      //The shadows can't be any wider than this.
#define SHADOW_Y_MULTIPLIER         1        //For every unit above the ground that the mob is on, the shadow goes these many units to the side.
#define SHIP_BEAM_RANGE             30       //The center of a ship's beam reaches this far.
#define SHIP_BEAM_RING_COLOR_SPEED  255      //Red color's index moves these many units per second. (Green is fast and blue is faster still).
#define SMACK_PARTICLE_DUR          0.1      //Duration of the "smack" particle.
#define SUN_METER_SUN_SPIN_SPEED    0.5      //The Sun Meter's sun spins these many radians per second.
#define THROW_DISTANCE_MULTIPLIER   0.49     //When a leader throws a Pikmin, multiply their strength by this.
#define THROW_PARTICLE_INTERVAL     0.02     //A new "mob thrown" particle is spawned every X seconds.
#define UNWHISTLABLE_PERIOD         1        //A mob cannot be whistled to a party during this period.
#define UNTOUCHABLE_PERIOD          3        //A mob cannot be touched to a party during this period.
#define WHISTLE_DOT_INTERVAL        0.03     //Seconds that need to pass before another dot is added.
#define WHISTLE_DOT_SPIN_SPEED      M_PI_2   //A whistle dot spins these many radians a second.
#define WHISTLE_FADE_TIME           0.1      //Time the whistle animations take to fade out.
#define WHISTLE_MAX_HOLD_TIME       1.5      //After the whistle reaches its maximum size, hold it for these many seconds until it stops by itself.
#define WHISTLE_RADIUS_GROWTH_SPEED 180      //The whistle's radius grows these many units per second.
#define WHISTLE_RING_SPEED          600      //Whistle rings move these many units per second.
#define WHISTLE_RINGS_INTERVAL      0.1      //Seconds that need to pass before another whistle ring appears.
#define ZOOM_MAX_LEVEL              2        //Maximum zoom level possible.
#define ZOOM_MAX_LEVEL_EDITOR       4        //Maximum zoom level possible on the area editor.
#define ZOOM_MIN_LEVEL              0.5      //Minimum zoom level possible.
#define ZOOM_MIN_LEVEL_EDITOR       0.05     //Minimum zoom level possible on the area editor.

#define DEF_FPS 30
#define DEF_SCR_W 640
#define DEF_SCR_H 480

#define AUDIO_FOLDER           GAME_DATA_FOLDER "/Audio"
#define AREA_FOLDER            GAME_DATA_FOLDER "/Areas"
#define CONFIGURATIONS_FOLDER  GAME_DATA_FOLDER "/Configurations"
#define ENEMIES_FOLDER         TYPES_FOLDER "/Enemies"
#define GAME_DATA_FOLDER       "Game_data"
#define GRAPHICS_FOLDER        GAME_DATA_FOLDER "/Graphics"
#define LEADERS_FOLDER         TYPES_FOLDER "/Leaders"
#define ONIONS_FOLDER          TYPES_FOLDER "/Onions"
#define OTHER_TYPES_FOLDER     TYPES_FOLDER "/Others"
#define PELLETS_FOLDER         TYPES_FOLDER "/Pellets"
#define PIKMIN_FOLDER          TYPES_FOLDER "/Pikmin"
#define TEXTURES_FOLDER        GRAPHICS_FOLDER "/Textures"
#define TREASURES_FOLDER       TYPES_FOLDER "/Treasures"
#define TYPES_FOLDER           GAME_DATA_FOLDER "/Types"
#define WEATHER_FILE           CONFIGURATIONS_FOLDER "/Weather.txt"


enum EDITOR_MODES {
    //Animation editor.
    EDITOR_MODE_MAIN,
    EDITOR_MODE_ANIMATION,
    EDITOR_MODE_FRAME,
    EDITOR_MODE_HITBOX,
    EDITOR_MODE_TOP,
    //Area editor
    EDITOR_MODE_SECTORS,
    EDITOR_MODE_ADV_TEXTURE_SETTINGS,
    EDITOR_MODE_OBJECTS,
    EDITOR_MODE_SHADOWS,
    EDITOR_MODE_BG,
    EDITOR_MODE_REVIEW,
};

enum EDITOR_SEC_MODES {
    ESM_NONE,
    ESM_NEW_SECTOR,
    ESM_NEW_OBJECT,
    ESM_NEW_SHADOW,
    ESM_BG_MOUSE,   //BG transformation being controlled by mouse.
    ESM_TEXTURE_VIEW,
};

enum SCREENS {
    SCREEN_MAIN_MENU,
    SCREEN_GAME,
    SCREEN_AREA_EDITOR,
    SCREEN_ANIMATION_EDITOR,
};

#define N_WHISTLE_RING_COLORS 8;
const unsigned char WHISTLE_RING_COLORS[8][3] = {
    {255, 255, 0},
    {255, 0, 0},
    {255, 0, 255},
    {128, 0, 255},
    {0, 0, 255},
    {0, 255, 255},
    {0, 255, 0},
    {128, 255, 0}
};

#endif //ifndef CONST_INCLUDED