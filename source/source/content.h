/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class representing a piece of game content.
 */

#pragma once

#include <string>

#include "libs/data_file.h"


using std::string;


//Type of content.
enum CONTENT_TYPE {

    //Area.
    CONTENT_TYPE_AREA,
    
    //Bitmap.
    CONTENT_TYPE_BITMAP,
    
    //Custom particle generator.
    CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    
    //Global animation.
    CONTENT_TYPE_GLOBAL_ANIMATION,
    
    //GUI definition.
    CONTENT_TYPE_GUI,
    
    //Hazard.
    CONTENT_TYPE_HAZARD,
    
    //Liquid.
    CONTENT_TYPE_LIQUID,
    
    //Misc. configuration.
    CONTENT_TYPE_MISC,
    
    //Mob animation.
    CONTENT_TYPE_MOB_ANIMATION,
    
    //Mob type.
    CONTENT_TYPE_MOB_TYPE,
    
    //Sample.
    CONTENT_TYPE_SAMPLE,
    
    //Song.
    CONTENT_TYPE_SONG,
    
    //Song track.
    CONTENT_TYPE_SONG_TRACK,
    
    //Spike damage type.
    CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
    
    //Spray type.
    CONTENT_TYPE_SPRAY_TYPE,
    
    //Status type.
    CONTENT_TYPE_STATUS_TYPE,
    
    //Weather condition.
    CONTENT_TYPE_WEATHER_CONDITION,
    
    //Total number of content types.
    N_CONTENT_TYPES,
    
};


//Levels to which content can be loaded.
enum CONTENT_LOAD_LEVEL {

    //Not loaded in any way.
    CONTENT_LOAD_LEVEL_UNLOADED,
    
    //Basic information.
    CONTENT_LOAD_LEVEL_BASIC,
    
    //Nearly fully loaded, but without things that aren't needed for editors.
    CONTENT_LOAD_LEVEL_EDITOR,
    
    //Fully loaded, including resources and dependencies.
    CONTENT_LOAD_LEVEL_FULL,
    
};


/**
 * @brief A manifest record of a piece of content on the disk.
 */
struct content_manifest {

    //--- Members ---
    
    //Internal name. Basically file name sans extension or folder name.
    string internal_name;
    
    //Path to the content, relative to the packages folder.
    string path;
    
    //Package it belongs to.
    string package;
    
    
    //--- Function declarations ---
    
    content_manifest();
    content_manifest(const string &name, const string &path, const string &package);
    void clear();
    void fill_from_path(const string &path);
    
};


/**
 * @brief Represents any piece of game content that can be used in the engine,
 * shared around, belong as part of another piece of content, etc.
 */
class plain_content {
public:

    //--- Members ---
    
    //The content's manifest.
    content_manifest* manifest;
    
};


/**
 * @brief Like the plain_content class, except this includes metadata
 * that can be loaded from and saved to a data file.
 */
class content : public plain_content {
public:

    //--- Members ---
    
    //Optional player/maker-facing name.
    string name;
    
    //Optional description.
    string description;
    
    //Optional tags, separated by semicolon.
    string tags;
    
    //Optional person(s) who made it.
    string maker;
    
    //Optional version name or number.
    string version;
    
    //Optional version of the engine it was made for.
    string engine_version;
    
    //Optional notes for other makers to see.
    string maker_notes;
    
    //Optional notes of any kind.
    string notes;
    
    
protected:

    //--- Function declarations ---
    
    void load_metadata_from_data_node(data_node* node);
    void reset_metadata();
    void save_metadata_to_data_node(data_node* node) const;
    
};
