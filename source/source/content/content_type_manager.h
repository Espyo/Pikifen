/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the content type manager classes and related functions.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "../core/audio.h"
#include "../core/misc_structs.h"
#include "area/area.h"
#include "content.h"
#include "mob/mob_utils.h"
#include "other/spike_damage.h"
#include "other/spray_type.h"

using std::map;
using std::string;
using std::vector;


/**
 * @brief Responsible for loading and storing game content of a given type
 * into memory.
 */
class ContentTypeManager {

public:

    //--- Function declarations ---
    
    virtual ~ContentTypeManager() = default;
    virtual void clearManifests() = 0;
    virtual void fillManifests() = 0;
    virtual string getName() const = 0;
    virtual string getPerfMonMeasurementName() const = 0;
    virtual void loadAll(CONTENT_LOAD_LEVEL level) = 0;
    virtual void unloadAll(CONTENT_LOAD_LEVEL level) = 0;
    
    
protected:

    //--- Function declarations ---
    void fillManifestsMap(
        map<string, ContentManifest>& manifests,
        const string& contentPath, bool folders
    );
    void fillManifestsMapFromPack(
        map<string, ContentManifest>& manifests, const string& packName,
        const string& contentRelPath, bool folders
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * areas into memory.
 */
class AreaContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of loaded areas.
    vector<vector<Area*> > list;
    
    //Manifests, by area type.
    vector<map<string, ContentManifest>> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    ContentManifest* findManifest(
        const string& areaName, const string& pack, AREA_TYPE type
    );
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    bool loadArea(
        Area* areaPtr, const string& requestedAreaPath,
        ContentManifest* manifPtr,
        CONTENT_LOAD_LEVEL level, bool fromBackup
    );
    string manifestToPath(
        const ContentManifest& manifest, AREA_TYPE type
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        AREA_TYPE* outType = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    
    void loadAreaIntoVector(
        ContentManifest* manifest, AREA_TYPE type, bool fromBackup
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * bitmaps into memory.
 */
class BitmapContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //Manager proper.
    BitmapManager list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(
        const ContentManifest& manifest,
        const string& extension
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        string* outExtension = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * global animations into memory.
 */
class GlobalAnimContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of animations.
    map<string, AnimationDatabase> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadAnimationDb(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * GUI definitions into memory.
 */
class GuiContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of GUI definitions.
    map<string, DataNode> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * hazards into memory.
 */
class HazardContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of hazards.
    map<string, Hazard> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadHazard(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * liquids into memory.
 */
class LiquidContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of liquid types.
    map<string, LiquidType*> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadLiquid(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * misc. configurations into memory.
 */
class MiscConfigContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
};



/**
 * @brief Responsible for loading and storing game content
 * mob animations into memory.
 */
class MobAnimContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of animations, by category.
    vector<map<string, AnimationDatabase> > list;
    
    //Manifests, by category.
    vector<map<string, ContentManifest> > manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(
        const ContentManifest& manifest, const string& category,
        const string& type
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        string* outCategory = nullptr, string* outType = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void fillCatManifestsFromPack(
        MobCategory* category, const string& packName
    );
    void loadAnimationDb(
        ContentManifest* manifest,
        CONTENT_LOAD_LEVEL level, MOB_CATEGORY categoryId
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * mob types into memory.
 */
class MobTypeContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of all mob types.
    MobTypeLists list;
    
    //Manifests, by category.
    vector<map<string, ContentManifest> > manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(
        const ContentManifest& manifest, const string& category
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        string* outCategory = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadMobTypesOfCategory(
        MobCategory* category, CONTENT_LOAD_LEVEL level
    );
    void unloadMobType(MobType* mt, CONTENT_LOAD_LEVEL level);
    void unloadMobTypesOfCategory(
        MobCategory* category, CONTENT_LOAD_LEVEL level
    );
};


/**
 * @brief Responsible for loading and storing game content
 * particle generators into memory.
 */
class ParticleGenContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of particle generators.
    map<string, ParticleGenerator> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadGenerator(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * songs into memory.
 */
class SongContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of liquids.
    map<string, Song> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadSong(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * song tracks into memory.
 */
class SongTrackContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //Manager proper.
    AudioStreamManager list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(
        const ContentManifest& manifest, const string& extension
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        string* outExtension = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * sound effects into memory.
 */
class SoundContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //Manager proper.
    SampleManager list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(
        const ContentManifest& manifest, const string& extension
    ) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr,
        string* outExtension = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * spike damage types into memory.
 */
class SpikeDamageTypeContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of spike damage types.
    map<string, SpikeDamageType> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadSpikeDamageType(
        ContentManifest* manifest, CONTENT_LOAD_LEVEL level
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * spray types into memory.
 */
class SprayTypeContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of spray types.
    map<string, SprayType> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadSprayType(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * status types into memory.
 */
class StatusTypeContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of status types.
    map<string, StatusType*> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadStatusType(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * weather conditions into memory.
 */
class WeatherConditionContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of weather conditions.
    map<string, Weather> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clearManifests() override;
    void fillManifests() override;
    string getName() const override;
    string getPerfMonMeasurementName() const override;
    void loadAll(CONTENT_LOAD_LEVEL level) override;
    string manifestToPath(const ContentManifest& manifest) const;
    void pathToManifest(
        const string& path, ContentManifest* outManifest = nullptr
    ) const;
    void unloadAll(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void loadWeatherCondition(
        ContentManifest* manifest, CONTENT_LOAD_LEVEL level
    );
    
};
