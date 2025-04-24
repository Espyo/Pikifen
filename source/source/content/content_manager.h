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
    vector<string> manifestsSansBase;
    
    //Manifests, with the base pack, organized via the player's options.
    vector<string> manifestsWithBase;
    
    //Manifests, sans the base pack, not organized via the player's options.
    vector<string> manifestsSansBaseRaw;
    
    //Manifests, with the base pack, not organized via the player's options.
    vector<string> manifestsWithBaseRaw;
    
    //List of loaded packs, with the base pack.
    map<string, Pack> list;
    
    //--- Function declarations ---
    
    void clearManifests();
    void fillManifests();
    void loadAll();
    void unloadAll();
    
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
    GlobalAnimContentManager globalAnimDbs;
    
    //GUI definitions.
    GuiContentManager guiDefs;
    
    //Hazards.
    HazardContentManager hazards;
    
    //Liquids.
    LiquidContentManager liquids;
    
    //Misc. configurations.
    MiscConfigContentManager miscConfigs;
    
    //Mob animation databases.
    MobAnimContentManager mobAnimDbs;
    
    //Mob types.
    MobTypeContentManager mobTypes;
    
    //Particle generators.
    ParticleGenContentManager particleGens;
    
    //Songs.
    SongContentManager songs;
    
    //Song tracks.
    SongTrackContentManager songTracks;
    
    //Sounds.
    SoundContentManager sounds;
    
    //Spike damage types.
    SpikeDamageTypeContentManager spikeDamageTypes;
    
    //Spray types.
    SprayTypeContentManager sprayTypes;
    
    //Status types.
    StatusTypeContentManager statusTypes;
    
    //Weather conditions.
    WeatherConditionContentManager weatherConditions;
    
    //Packs.
    PackManager packs;
    
    
    //--- Function declarations ---
    
    ContentManager();
    bool createPack(
        const string &internalName, const string &name,
        const string &description = "", const string &maker = ""
    );
    bool loadAreaAsCurrent(
        const string &requestedAreaPath, ContentManifest* manifPtr,
        CONTENT_LOAD_LEVEL level, bool fromBackup
    );
    void loadAll(const vector<CONTENT_TYPE> &types, CONTENT_LOAD_LEVEL level);
    void reloadPacks();
    void unloadAll(const vector<CONTENT_TYPE> &types);
    void unloadCurrentArea(CONTENT_LOAD_LEVEL level);
    
    private:
    
    //--- Members ---
    
    CONTENT_LOAD_LEVEL loadLevels[N_CONTENT_TYPES];
    
    
    //--- Function declarations ---
    
    ContentTypeManager* getMgrPtr(CONTENT_TYPE type);
    
};
