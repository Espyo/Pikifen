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
struct PackManager {

    //--- Members ---
    
    //Manifests, sans the base pack, organized via the player's options.
    vector<string> manifests_sans_base;
    
    //Manifests, with the base pack, organized via the player's options.
    vector<string> manifests_with_base;
    
    //Manifests, sans the base pack, not organized via the player's options.
    vector<string> manifests_sans_base_raw;
    
    //Manifests, with the base pack, not organized via the player's options.
    vector<string> manifests_with_base_raw;
    
    //List of loaded packs, with the base pack.
    map<string, Pack> list;
    
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
struct ContentManager {

    //--- Members ---
    
    //Areas.
    AreaContentManager areas;
    
    //Bitmaps.
    BitmapContentManager bitmaps;
    
    //Global animation databases.
    GlobalAnimContentManager global_anim_dbs;
    
    //GUI definitions.
    GuiContentManager gui_defs;
    
    //Hazards.
    HazardContentManager hazards;
    
    //Liquids.
    LiquidContentManager liquids;
    
    //Misc. configurations.
    MiscConfigContentManager misc_configs;
    
    //Mob animation databases.
    MobAnimContentManager mob_anim_dbs;
    
    //Mob types.
    MobTypeContentManager mob_types;
    
    //Particle generators.
    ParticleGenContentManager particle_gen;
    
    //Songs.
    SongContentManager songs;
    
    //Song tracks.
    SongTrackContentManager song_tracks;
    
    //Sounds.
    SoundContentManager sounds;
    
    //Spike damage types.
    SpikeDamageTypeContentManager spike_damage_types;
    
    //Spray types.
    SprayTypeContentManager spray_types;
    
    //Status types.
    StatusTypeContentManager status_types;
    
    //Weather conditions.
    WeatherConditionContentManager weather_conditions;
    
    //Packs.
    PackManager packs;
    
    
    //--- Function declarations ---
    
    ContentManager();
    bool create_pack(
        const string &internal_name, const string &name,
        const string &description = "", const string &maker = ""
    );
    bool load_area_as_current(
        const string &requested_area_path, ContentManifest* manif_ptr,
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
    
    ContentTypeManager* get_mgr_ptr(CONTENT_TYPE type);
    
};
