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

#include "content_type_manager.h"


using std::map;
using std::string;


/**
 * @brief Manages everything regarding game content, be it assets, types of
 * mobs, etc.
 */
struct content_manager {

    //--- Members ---
    
    //Areas.
    area_content_manager areas;

    //Bitmaps.
    bitmap_content_manager bitmaps;

    //Custom particle generators.
    custom_particle_gen_content_manager custom_particle_gen;

    //Global animations.
    global_anim_content_manager global_anims;

    //GUI definitions.
    gui_content_manager gui;

    //Hazards.
    hazard_content_manager hazards;

    //Liquids.
    liquid_content_manager liquids;

    //Misc. configurations.
    misc_config_content_manager misc_configs;

    //Mob types.
    mob_type_content_manager mob_types;

    //Samples.
    sample_content_manager samples;

    //Songs.
    song_content_manager songs;

    //Song tracks.
    song_track_content_manager song_tracks;

    //Spike damage types.
    spike_damage_type_content_manager spike_damage_types;

    //Spray types.
    spray_type_content_manager spray_types;

    //Status types.
    status_type_content_manager status_types;

    //Weather conditions.
    weather_condition_content_manager weather_conditions;
    

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

    content_type_manager* get_mgr_ptr(CONTENT_TYPE type);
    
};
