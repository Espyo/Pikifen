/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used constants.
 */

#pragma once

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
constexpr unsigned char VERSION_MINOR = 25;
constexpr unsigned char VERSION_REV   = 0;

//The following constants need to be in this file because C++ sucks.
//Too many files need these constants, and because of the one-definition rule,
//translation unit operations, and more nonsense, it's impossible to implement
//these elsewhere. At least they're really important and global, so I guess
//they fit here in and of themselves.

//Number of Pikmin maturities.
constexpr size_t NR_MATURITIES = 3;

//Maximum number of players that can play the game.
constexpr size_t MAX_PLAYERS = 4;

//Name to display on the "none" option in a combobox.
const string NONE_OPTION = "(None)";

//Smallest possible window height.
constexpr int SMALLEST_WIN_HEIGHT = 480;

//Smallest possible window width.
constexpr int SMALLEST_WIN_WIDTH = 640;

//Real-world centimeters per in-game-world pixel.
constexpr float CM_PER_PIXEL = 0.05f;

//Path to the folder with the game data,
//relative to the program root folder.
const string GAME_DATA_FOLDER_PATH = "Game_data";

//Path to the folder with the user data,
//relative to the program root folder.
const string USER_DATA_FOLDER_PATH = "User_data";

//Name of the folder with area type folders inside.
const string AREA_TYPES_FOLDER_NAME = "Areas";

//Name of the folder with mission areas, be it in the game data or user data.
const string MISSION_AREA_FOLDER_NAME = "Areas/Mission";

//Name of the folder with simple areas, be it in the game data or user data.
const string SIMPLE_AREA_FOLDER_NAME = "Areas/Simple";

//Path to the folder with the global animations,
//relative to the program root folder.
const string ANIMATIONS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Animations";

//Name of the file with an area's data.
const string AREA_DATA_FILE_NAME =
    "Data.txt";

//Name of the file with a backup of an area's data.
const string AREA_DATA_BACKUP_FILE_NAME =
    "Data_backup.txt";

//Name of the file with an area's geometry.
const string AREA_GEOMETRY_FILE_NAME =
    "Geometry.txt";

//Name of the file with a backup of an area's geometry.
const string AREA_GEOMETRY_BACKUP_FILE_NAME =
    "Geometry_backup.txt";

//Path to the folder with the audio files,
//relative to the program root folder.
const string AUDIO_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Audio";

//Path to the folder with the music files,
//relative to the program root folder.
const string AUDIO_MUSIC_FOLDER_PATH =
    AUDIO_FOLDER_PATH + "/Music";

//Path to the folder with the sound files,
//relative to the program root folder.
const string AUDIO_SOUNDS_FOLDER_PATH =
    AUDIO_FOLDER_PATH + "/Sounds";

//Path to the folder with the song files,
//relative to the program root folder.
const string AUDIO_SONG_FOLDER_PATH =
    AUDIO_MUSIC_FOLDER_PATH + "/Songs";

//Path to the folder with the music track files,
//relative to the program root folder.
const string AUDIO_TRACK_FOLDER_PATH =
    AUDIO_MUSIC_FOLDER_PATH + "/Tracks";

//Path to the folder with the graphics,
//relative to the program root folder.
const string GRAPHICS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Graphics";

//Path to the folder with the GUI configurations,
//relative to the program root folder.
const string GUI_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/GUI";

//Path to the folder with the hazards,
//relative to the program root folder.
const string HAZARDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Hazards";

//Path to the folder with the liquids,
//relative to the program root folder.
const string LIQUIDS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Liquids";

//Path to the folder with the mob types,
//relative to the program root folder.
const string MOB_TYPES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Types";

//Path to the folder with the misc. game data,
//relative to the program root folder.
const string MISC_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Misc";

//Path to the folder with the particle generators,
//relative to the program root folder.
const string PARTICLE_GENERATORS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Particle_generators";

//Path to the folder with the spike damage types,
//relative to the program root folder.
const string SPIKE_DAMAGES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Spike_damage_types";

//Path to the folder with the sprays,
//relative to the program root folder.
const string SPRAYS_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Sprays";

//Path to the folder with the status effects,
//relative to the program root folder.
const string STATUSES_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Statuses";

//Name of the folder with all sector textures.
const string TEXTURES_FOLDER_NAME =
    "Textures";

//Path to the folder with the sector textures,
//relative to the program root folder.
const string TEXTURES_FOLDER_PATH =
    GRAPHICS_FOLDER_PATH + "/" + TEXTURES_FOLDER_NAME;

//Path to the folder with the weather conditions,
//relative to the program root folder.
const string WEATHER_FOLDER_PATH =
    GAME_DATA_FOLDER_PATH + "/Weather";

//Path to the folder with the game configuration,
//relative to the program root folder.
const string CONFIG_FILE =
    MISC_FOLDER_PATH + "/Config.txt";

//Path to the folder with the system animations list,
//relative to the program root folder.
const string SYSTEM_ANIMATIONS_FILE_PATH =
    MISC_FOLDER_PATH + "/System_animations.txt";

//Path to the folder with the system asset file names,
//relative to the program root folder.
const string SYSTEM_ASSET_FILE_NAMES_FILE_PATH =
    MISC_FOLDER_PATH + "/System_asset_file_names.txt";

//Path to the file with the error log,
//relative to the program root folder.
const string ERROR_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Error_log.txt";

//Path to the file with the mission records,
//relative to the program root folder.
const string MISSION_RECORDS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Mission_records.txt";

//Path to the file with the maker tools configuration,
//relative to the program root folder.
const string MAKER_TOOLS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Tools.txt";

//Path to the file with the user options data,
//relative to the program root folder.
const string OPTIONS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Options.txt";

//Path to the file with the performance log,
//relative to the program root folder.
const string PERFORMANCE_LOG_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Performance_log.txt";

//Path to the file with the lifetime statistics,
//relative to the program root folder.
const string STATISTICS_FILE_PATH =
    USER_DATA_FOLDER_PATH + "/Statistics.txt";
