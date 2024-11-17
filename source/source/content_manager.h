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
 * @brief Manages everything regarding the installed game content packs.
 */
struct pack_manager {

    //--- Members ---
    
    //Manifests, sans the base pack.
    vector<string> manifests_sans_base;
    
    //Manifests, with the base pack.
    vector<string> manifests_with_base;
    
    //List of loaded packs, with the base pack.
    map<string, pack> list;
    
    //--- Function declarations ---
    
    void clear_manifests();
    void fill_manifests();
    void load_all();
    void unload_all();
    
};


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
    
    //Animations.
    global_anim_content_manager global_anims;
    
    //GUI definitions.
    gui_content_manager gui;
    
    //Hazards.
    hazard_content_manager hazards;
    
    //Liquids.
    liquid_content_manager liquids;
    
    //Misc. configurations.
    misc_config_content_manager misc_configs;
    
    //Mob animations.
    mob_anim_content_manager mob_anims;
    
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
    
    //Packs.
    pack_manager packs;
    
    
    //--- Function declarations ---
    
    content_manager();
    bool create_pack(
        const string &internal_name, const string &name,
        const string &description = "", const string &maker = ""
    );
    void load_area_as_current(
        content_manifest* manifest, AREA_TYPE type,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    void load_all(const vector<CONTENT_TYPE> &types, CONTENT_LOAD_LEVEL level);
    void reload_packs();
    void unload_all(const vector<CONTENT_TYPE> &types);
    void unload_current_area(CONTENT_LOAD_LEVEL level);
    
    private:
    
    //--- Members ---
    
    CONTENT_LOAD_LEVEL load_levels[N_CONTENT_TYPES];
    
    
    //--- Function declarations ---
    
    content_type_manager* get_mgr_ptr(CONTENT_TYPE type);
    
};
