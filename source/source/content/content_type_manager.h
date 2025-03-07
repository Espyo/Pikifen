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
    virtual void clear_manifests() = 0;
    virtual void fill_manifests() = 0;
    virtual string get_name() const = 0;
    virtual string get_perf_mon_measurement_name() const = 0;
    virtual void load_all(CONTENT_LOAD_LEVEL level) = 0;
    virtual void unload_all(CONTENT_LOAD_LEVEL level) = 0;
    
    
protected:

    //--- Function declarations ---
    void fill_manifests_map(
        map<string, ContentManifest> &manifests, const string &content_path, bool folders
    );
    void fill_manifests_map_from_pack(
        map<string, ContentManifest> &manifests, const string &pack_name,
        const string &content_rel_path, bool folders
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    ContentManifest* find_manifest(const string &area_name, const string &pack, AREA_TYPE type);
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    bool load_area(
        Area* area_ptr, const string &requested_area_path,
        ContentManifest* manif_ptr,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    string manifest_to_path(
        const ContentManifest &manifest, AREA_TYPE type
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        AREA_TYPE* out_type = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    
    void load_area_into_vector(
        ContentManifest* manifest, AREA_TYPE type, bool from_backup
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const ContentManifest &manifest,
        const string &extension
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_animation_db(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_hazard(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * liquids into memory.
 */
class LiquidContentManager : public ContentTypeManager {

public:

    //--- Members ---
    
    //List of liquids.
    map<string, Liquid*> list;
    
    //Manifests.
    map<string, ContentManifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_liquid(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const ContentManifest &manifest, const string &category,
        const string &type
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        string* out_category = nullptr, string* out_type = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void fill_cat_manifests_from_pack(
        MobCategory* category, const string &pack_name
    );
    void load_animation_db(ContentManifest* manifest, CONTENT_LOAD_LEVEL level, MOB_CATEGORY category_id);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const ContentManifest &manifest, const string &category
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        string* out_category = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_mob_types_of_category(MobCategory* category, CONTENT_LOAD_LEVEL level);
    void unload_mob_type(MobType* mt, CONTENT_LOAD_LEVEL level);
    void unload_mob_types_of_category(MobCategory* category, CONTENT_LOAD_LEVEL level);
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_generator(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_song(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const ContentManifest &manifest, const string &extension
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const ContentManifest &manifest, const string &extension
    ) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_spike_damage_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_spray_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_status_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
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
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const ContentManifest &manifest) const;
    void path_to_manifest(
        const string &path, ContentManifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_weather_condition(ContentManifest* manifest, CONTENT_LOAD_LEVEL level);
    
};
