/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the content manager class and related functions.
 */

#pragma once

#include <map>
#include <string>

#include "area/area.h"
#include "hazard.h"
#include "liquid.h"
#include "mobs/mob_utils.h"
#include "particle.h"
#include "spike_damage.h"
#include "spray_type.h"
#include "status.h"
#include "weather.h"


using std::map;
using std::string;


/**
 * @brief A manifest record of a piece of content on the disk.
 */
struct content_manifest {

    //--- Members ---
    
    //Path to the content, relative to the packages folder.
    string path;
    
    //Package it belongs to.
    string package;
    
    //--- Function declarations ---
    
    content_manifest();
    content_manifest(const string &path, const string &package);
    
};


/**
 * @brief Manages everything regarding game content, be it assets, types of
 * mobs, etc.
 */
struct content_manager {

    //--- Members ---
    
    struct {
    
        //Global animations.
        map<string, content_manifest> global_animations;
        
        //Areas, by type.
        vector<map<string, content_manifest> > areas;
        
        //Bitmaps.
        map<string, content_manifest> bitmaps;
        
        //Custom particle generators.
        map<string, content_manifest> custom_particle_generators;
        
        //GUI definitions..
        map<string, content_manifest> gui;
        
        //Hazards.
        map<string, content_manifest> hazards;
        
        //Liquids.
        map<string, content_manifest> liquids;
        
        //Misc. game configurations.
        map<string, content_manifest> misc_configs;
        
        //Mob types.
        vector<map<string, content_manifest> > mob_types;
        
        //Spike damage types.
        map<string, content_manifest> spike_damage_types;
        
        //Spray types.
        map<string, content_manifest> spray_types;
        
        //Status types.
        map<string, content_manifest> status_types;
        
        //Weather conditions.
        map<string, content_manifest> weather_conditions;
        
        //Songs.
        map<string, content_manifest> songs;
        
        //Song tracks.
        map<string, content_manifest> song_tracks;
        
        //Sound effects.
        map<string, content_manifest> sounds;
        
    } manifests;
    
    //List of global animations.
    map<string, single_animation_suite> global_animations;
    
    //List of areas.
    vector<vector<area_data*> > areas;
    
    //Manager of standard bitmaps.
    bitmap_manager bitmaps;
    
    //List of particle generators declared by the user.
    map<string, particle_generator> custom_particle_generators;
    
    //List of GUI definitions.
    map<string, data_node> gui;
    
    //List of hazards.
    map<string, hazard> hazards;
    
    //List of liquids.
    map<string, liquid*> liquids;
    
    //List of all mob types.
    mob_type_lists mob_types;
    
    //List of songs.
    map<string, song> songs;
    
    //List of spike damage types.
    map<string, spike_damage_type> spike_damage_types;
    
    //List of spray types.
    vector<spray_type> spray_types;
    
    //Manager of samples.
    sfx_sample_manager samples;
    
    //List of status types.
    map<string, status_type*> status_types;
    
    //Manager of song tracks.
    audio_stream_manager song_tracks;
    
    //List of weather conditions.
    map<string, weather> weather_conditions;
    
    //--- Function declarations ---
    
    content_manager();
    void load_area_as_current(
        const string &folder_name, const string &package_name, AREA_TYPE type,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    void load_all(CONTENT_TYPE type, CONTENT_LOAD_LEVEL level);
    void unload_all(CONTENT_TYPE type);
    void unload_current_area(CONTENT_LOAD_LEVEL level);
    
private:

    //--- Members ---
    
    CONTENT_LOAD_LEVEL load_levels[N_CONTENT_TYPES];
    
    //--- Function declarations ---
    
    void fill_manifest(
        map<string, content_manifest> &manifest, const string &content_path, bool folders
    );
    void fill_manifest_from_package(
        map<string, content_manifest> &manifest, const string &package_name,
        const string &content_rel_path, bool folders
    );
    void load_global_animation(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_global_animations(CONTENT_LOAD_LEVEL level);
    void load_area(
        area_data* area_ptr,
        const string &internal_name, const string &package_name, AREA_TYPE type,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    void load_area_into_vector(
        const string &internal_name, const string &package_name, AREA_TYPE type,
        bool from_backup
    );
    void load_areas();
    void load_bitmaps(CONTENT_LOAD_LEVEL level);
    void load_custom_particle_generator(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_custom_particle_generators(CONTENT_LOAD_LEVEL level);
    void load_gui_definitions(CONTENT_LOAD_LEVEL level);
    void load_hazard(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_hazards(CONTENT_LOAD_LEVEL level);
    void load_liquid(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_liquids(CONTENT_LOAD_LEVEL level);
    void load_misc(CONTENT_LOAD_LEVEL level);
    void load_mob_types(CONTENT_LOAD_LEVEL level);
    void load_mob_types_of_category(
        mob_category* category, CONTENT_LOAD_LEVEL level
    );
    void load_song(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_songs(CONTENT_LOAD_LEVEL level);
    void load_song_tracks(CONTENT_LOAD_LEVEL level);
    void load_sounds(CONTENT_LOAD_LEVEL level);
    void load_spike_damage_type(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_spike_damage_types(CONTENT_LOAD_LEVEL level);
    void load_spray_type(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_spray_types(CONTENT_LOAD_LEVEL level);
    void load_status_type(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_status_types(CONTENT_LOAD_LEVEL level);
    void load_weather_condition(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_weather_conditions(CONTENT_LOAD_LEVEL level);
    void unload_areas(CONTENT_LOAD_LEVEL level);
    void unload_bitmaps(CONTENT_LOAD_LEVEL level);
    void unload_custom_particle_generators(CONTENT_LOAD_LEVEL level);
    void unload_global_animations(CONTENT_LOAD_LEVEL level);
    void unload_gui(CONTENT_LOAD_LEVEL level);
    void unload_hazards(CONTENT_LOAD_LEVEL level);
    void unload_liquids(CONTENT_LOAD_LEVEL level);
    void unload_misc(CONTENT_LOAD_LEVEL level);
    void unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level);
    void unload_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level);
    void unload_mob_types(CONTENT_LOAD_LEVEL level);
    void unload_song_tracks(CONTENT_LOAD_LEVEL level);
    void unload_songs(CONTENT_LOAD_LEVEL level);
    void unload_sounds(CONTENT_LOAD_LEVEL level);
    void unload_spike_damage_types(CONTENT_LOAD_LEVEL level);
    void unload_spray_types(CONTENT_LOAD_LEVEL level);
    void unload_status_types(CONTENT_LOAD_LEVEL level);
    void unload_weather_conditions(CONTENT_LOAD_LEVEL level);
};
