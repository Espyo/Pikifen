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

//Folder names (not paths).
namespace FOLDER_NAMES {

    //Game data folder.
    const string GAME_DATA = "Game_data";

    //User data folder.
    const string USER_DATA = "User_data";

    //Base content package folder.
    const string BASE_PKG = "Base";

    //Audio folder.
    const string AUDIO = "Audio";

    //Music folder.
    const string MUSIC = "Music";

    //Songs folder.
    const string SONGS = "Songs";

    //Song tracks folder.
    const string SONG_TRACKS = "Tracks";

    //Sound effects folder.
    const string SOUNDS = "Sounds";

    //Areas folder.
    const string AREAS = "Areas";

    //Simple areas folder.
    const string SIMPLE_AREAS = "Simple";

    //Mission areas folder.
    const string MISSION_AREAS = "Mission";

    //Global animations folder.
    const string ANIMATIONS = "Animations";

    //Graphics folder.
    const string GRAPHICS = "Graphics";

    //Textures folder.
    const string TEXTURES = "Textures";

    //GUI configurations folder.
    const string GUI = "GUI";

    //Hazards folder.
    const string HAZARDS = "Hazards";

    //Liquids folder.
    const string LIQUIDS = "Liquids";

    //Misc. folder.
    const string MISC = "Misc";

    //Mob types folder.
    const string MOB_TYPES = "Types";

    //Particle generators folder.
    const string PARTICLE_GENERATORS = "Particle_generators";

    //Spike damage types folder.
    const string SPIKE_DAMAGES_TYPES = "Spike_damage_types";

    //Spray types folder.
    const string SPRAYS = "Sprays";

    //Statuses folder.
    const string STATUSES = "Statuses";

    //Weather definitions.
    const string WEATHER = "Weather";

}

//File names (not paths).
namespace FILE_NAMES {

    //Area main data file.
    const string AREA_MAIN_DATA = "Data.txt";

    //Area geometry file.
    const string AREA_GEOMETRY = "Geometry.txt";

    //Area main data backup file.
    const string AREA_MAIN_DATA_BACKUP = "Data_backup.txt";

    //Area geometry backup file.
    const string AREA_GEOMETRY_BACKUP = "Geometry_backup.txt";

    //Game configuration file.
    const string GAME_CONFIG = "Config.txt";

    //System animations file.
    const string SYSTEM_ANIMS = "System_animations.txt";

    //System animation file names file.
    const string SYSTEM_ASSET_FILE_NAMES = "System_asset_file_names.txt";

    //Error log file.
    const string ERROR_LOG = "Error_log.txt";
    
    //Mission records file.
    const string MISSION_RECORDS = "Mission_records.txt";
    
    //Maker tools file.
    const string MAKER_TOOLS = "Tools.txt";
    
    //Options file.
    const string OPTIONS = "Options.txt";

    //Performance log file.
    const string PERFORMANCE_LOG = "Performance_log.txt";

    //Statistics file.
    const string STATISTICS = "Statistics.txt";

}

//Paths to folders from the engine's root folder.
namespace FOLDER_PATHS_FROM_ROOT {

    //Game data folder.
    const string GAME_DATA =
        FOLDER_NAMES::GAME_DATA;

    //Base package folder.
    const string BASE_PKG =
        GAME_DATA + "/" + FOLDER_NAMES::BASE_PKG;

    //User data folder.
    const string USER_DATA =
        FOLDER_NAMES::USER_DATA;
};

//Paths to files from the engine's root folder.
namespace FILE_PATHS_FROM_ROOT {

    //Error log.
    const string ERROR_LOG =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::ERROR_LOG;
    
    //Mission records.
    const string MISSION_RECORDS =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::MISSION_RECORDS;
    
    //Options.
    const string OPTIONS =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::OPTIONS;
    
    //Performance log.
    const string PERFORMANCE_LOG =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::PERFORMANCE_LOG;
    
    //Statistics.
    const string STATISTICS =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::STATISTICS;
    
    //Maker tools.
    const string MAKER_TOOLS =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + FILE_NAMES::MAKER_TOOLS;
    
};

//Paths to game content folders from a package's folder.
namespace FOLDER_PATHS_FROM_PKG {

    //Audio folder.
    const string AUDIO =
        FOLDER_NAMES::AUDIO;

    //Music folder.
    const string MUSIC =
        AUDIO + "/" + FOLDER_NAMES::MUSIC;

    //Songs folder.
    const string SONGS =
        MUSIC + "/" + FOLDER_NAMES::SONGS;

    //Song tracks folder.
    const string SONG_TRACKS =
        MUSIC + "/" + FOLDER_NAMES::SONG_TRACKS;

    //Sound effects folder.
    const string SOUNDS =
        AUDIO + "/" + FOLDER_NAMES::SOUNDS;

    //Areas folder.
    const string AREAS =
        FOLDER_NAMES::AREAS;

    //Simple areas folder.
    const string SIMPLE_AREAS =
        AREAS + "/" + FOLDER_NAMES::SIMPLE_AREAS;

    //Mission areas folder.
    const string MISSION_AREAS =
        AREAS + "/" + FOLDER_NAMES::MISSION_AREAS;

    //Global animations folder.
    const string ANIMATIONS =
        FOLDER_NAMES::ANIMATIONS;

    //Graphics folder.
    const string GRAPHICS =
        FOLDER_NAMES::GRAPHICS;

    //Textures folder.
    const string TEXTURES =
        GRAPHICS + "/" + FOLDER_NAMES::TEXTURES;

    //GUI configurations folder.
    const string GUI =
        FOLDER_NAMES::GUI;

    //Hazards folder.
    const string HAZARDS =
        FOLDER_NAMES::HAZARDS;

    //Liquids folder.
    const string LIQUIDS =
        FOLDER_NAMES::LIQUIDS;

    //Misc. folder.
    const string MISC =
        FOLDER_NAMES::MISC;

    //Mob types folder.
    const string MOB_TYPES =
        FOLDER_NAMES::MOB_TYPES;

    //Particle generators folder.
    const string PARTICLE_GENERATORS =
        FOLDER_NAMES::PARTICLE_GENERATORS;

    //Spike damage types folder.
    const string SPIKE_DAMAGES_TYPES =
        FOLDER_NAMES::SPIKE_DAMAGES_TYPES;

    //Spray types folder.
    const string SPRAYS =
        FOLDER_NAMES::SPRAYS;

    //Statuses folder.
    const string STATUSES =
        FOLDER_NAMES::STATUSES;

    //Weather definitions.
    const string WEATHER =
        FOLDER_NAMES::WEATHER;

};

//Paths to game content files from a package's folder.
namespace FILE_PATHS_FROM_PKG {
    //Game configuration file.
    const string GAME_CONFIG =
        FOLDER_PATHS_FROM_PKG::MISC + FILE_NAMES::GAME_CONFIG;

    //System animations file.
    const string SYSTEM_ANIMS =
        FOLDER_PATHS_FROM_PKG::MISC + FILE_NAMES::SYSTEM_ANIMS;

    //System animation file names file.
    const string SYSTEM_ASSET_FILE_NAMES =
        FOLDER_PATHS_FROM_PKG::MISC + FILE_NAMES::SYSTEM_ASSET_FILE_NAMES;

    
}
