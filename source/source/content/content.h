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

#include "../lib/data_file/data_file.h"


using std::string;


//Type of content.
enum CONTENT_TYPE {

    //Area.
    CONTENT_TYPE_AREA,
    
    //Bitmap.
    CONTENT_TYPE_BITMAP,
    
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
    
    //Particle generator.
    CONTENT_TYPE_PARTICLE_GEN,
    
    //Sound.
    CONTENT_TYPE_SOUND,
    
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
 * @brief A manifest record of a piece of content in the disk.
 */
struct ContentManifest {

    //--- Public members ---
    
    //Internal name. Basically file name sans extension or folder name.
    string internalName;
    
    //Path to the content, relative to the packs folder.
    string path;
    
    //Pack it belongs to.
    string pack;
    
    
    //--- Public function declarations ---
    
    ContentManifest();
    ContentManifest(const string& name, const string& path, const string& pack);
    void clear();
    void fillFromPath(const string& path);
    
};


/**
 * @brief Represents any piece of game content that can be used in the engine,
 * shared around, belong as part of another piece of content, etc.
 */
class PlainContent {
public:

    //--- Public members ---
    
    //The content's manifest.
    ContentManifest* manifest = nullptr;
    
};


/**
 * @brief Like the PlainContent class, except this includes metadata
 * that can be loaded from and saved to a data file.
 */
class Content : public PlainContent {
public:

    //--- Public members ---
    
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
    string engineVersion;
    
    //Optional notes for other makers to see.
    string makerNotes;
    
    //Optional notes of any kind.
    string notes;
    
    
    //--- Public function declarations ---
    
    void loadMetadataFromDataNode(DataNode* node);
    void resetMetadata();
    void saveMetadataToDataNode(DataNode* node) const;
    
};


/**
 * @brief Data about an installed pack.
 */
struct Pack {

    //--- Public members ---
    
    //Optional player/maker-facing name.
    string name;
    
    //Optional description.
    string description;
    
    //Optional person(s) who made it.
    string maker;
    
    //Optional version name or number.
    string version;
    
    //Optional version of the engine it was made for.
    string engineVersion;
    
    //Optional tags, separated by semicolon.
    string tags;
    
    //Optional list of packs it depends on, separated by semicolon.
    string dependencies;
    
    //Optional list of packs it conflicts with.
    string conflicts;
    
    //Optional notes of any kind.
    string notes;
    
};
