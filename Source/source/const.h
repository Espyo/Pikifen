/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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

#include "utils/math_utils.h"


using std::size_t;
using std::string;

//Engine version stuff. For every release, update these numbers, and
//update the numbers in the resouce (.rc) file.
constexpr unsigned char VERSION_MAJOR = 0;
constexpr unsigned char VERSION_MINOR = 22;
constexpr unsigned char VERSION_REV   = 0;

//Cross-platform way of representing an invalid index.
constexpr size_t INVALID = UINT32_MAX;
//Cross-platform way of representing a float value of "invalid" or similar.
constexpr float LARGE_FLOAT = 999999.0f;

//Full-white opaque color.
constexpr ALLEGRO_COLOR COLOR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
//Full-black opaque color.
constexpr ALLEGRO_COLOR COLOR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
//Fully-transparent color, in black.
constexpr ALLEGRO_COLOR COLOR_EMPTY = { 0.0f, 0.0f, 0.0f, 0.0f };

//The following constants need to be in this file because C++ sucks.
//Too many files need these constants, and because of the one-definition rule,
//translation unit operations, and more nonsense, it's impossible to implement
//these elsewhere. At least they're really important and global, so I guess
//they fit here in and of themselves.

//Number of Pikmin maturities.
constexpr size_t N_MATURITIES = 3;
//Maximum number of players that can play the game.
constexpr size_t MAX_PLAYERS = 4;
//Name to display on the "none" option in a combobox.
const string NONE_OPTION = "(None)";
//Smallest possible window height.
constexpr int SMALLEST_WIN_HEIGHT = 480;
//Smallest possible window width.
constexpr int SMALLEST_WIN_WIDTH = 640;

//Relative path to the folder with the game data.
const string GAME_DATA_FOLDER_PATH = "Game_data";
//Relative path to the folder with the user data.
const string USER_DATA_FOLDER_PATH = "User_data";

//Name of the folder with mission areas, be it in the game data or user data.
const string MISSION_AREA_FOLDER_NAME = "Areas/Simple";
//Name of the folder with simple areas, be it in the game data or user data.
const string SIMPLE_AREA_FOLDER_NAME = "Areas/Simple";

//Relative path to the folder with the global animations.
const string ANIMATIONS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Animations";
//Relative path to the folder with the audio files.
const string AUDIO_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Audio";
//Relative path to the folder with the graphics.
const string GRAPHICS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Graphics";
//Relative path to the folder with the GUI configurations.
const string GUI_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/GUI";
//Relative path to the folder with the hazards.
const string HAZARDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Hazards";
//Relative path to the folder with the liquids.
const string LIQUIDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Liquids";
//Relative path to the folder with the mob types.
const string MOB_TYPES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Types";
//Relative path to the folder with the misc. game data.
const string MISC_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Misc";
//Relative path to the folder with the particle generators.
const string PARTICLE_GENERATORS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Particle_generators";
//Relative path to the folder with the spike damage types.
const string SPIKE_DAMAGES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Spike_damage_types";
//Relative path to the folder with the sprays.
const string SPRAYS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Sprays";
//Relative path to the folder with the status effects.
const string STATUSES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Statuses";
//Name of the folder with all sector textures.
const string TEXTURES_FOLDER_NAME =
    "Textures";
//Relative path to the folder with the sector textures.
const string TEXTURES_FOLDER_PATH =
    GRAPHICS_FOLDER_PATH + "/" + TEXTURES_FOLDER_NAME;
//Relative path to the folder with the weather conditions.
const string WEATHER_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Weather";

//Relative path to the folder with the game configuration.
const string CONFIG_FILE =
    MISC_FOLDER_PATH + "/Config.txt";
//Relative path to the folder with the system animations list.
const string SYSTEM_ANIMATIONS_FILE_PATH =
    MISC_FOLDER_PATH + "/System_animations.txt";
//Relative path to the folder with the system asset file names.
const string SYSTEM_ASSET_FILE_NAMES_FILE_PATH =
    MISC_FOLDER_PATH + "/System_asset_file_names.txt";

//Relative path to the file with the error log.
const string ERROR_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Error_log.txt";
//Relative path to the file with the maker tools configuration.
const string MAKER_TOOLS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Tools.txt";
//Relative path to the file with the user options data.
const string OPTIONS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Options.txt";
//Relative path to the file with the performance log.
const string PERFORMANCE_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Performance_log.txt";

#endif //ifndef CONST_INCLUDED
