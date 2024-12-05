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

#include "audio.h"
#include "content.h"
#include "area/area.h"
#include "mobs/mob_utils.h"
#include "misc_structs.h"
#include "spike_damage.h"
#include "spray_type.h"

using std::map;
using std::string;
using std::vector;


/**
 * @brief Responsible for loading and storing game content of a given type
 * into memory.
 */
class content_type_manager {

public:

    //--- Function declarations ---
    
    virtual void clear_manifests() = 0;
    virtual void fill_manifests() = 0;
    virtual string get_name() const = 0;
    virtual string get_perf_mon_measurement_name() const = 0;
    virtual void load_all(CONTENT_LOAD_LEVEL level) = 0;
    virtual void unload_all(CONTENT_LOAD_LEVEL level) = 0;
    
    
protected:

    //--- Function declarations ---
    void fill_manifests_map(
        map<string, content_manifest> &manifests, const string &content_path, bool folders
    );
    void fill_manifests_map_from_pack(
        map<string, content_manifest> &manifests, const string &pack_name,
        const string &content_rel_path, bool folders
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * areas into memory.
 */
class area_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of loaded areas.
    vector<vector<area_data*> > list;
    
    //Manifests, by area type.
    vector<map<string, content_manifest>> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    content_manifest* find_manifest(const string &area_name, const string &pack, AREA_TYPE type);
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    bool load_area(
        area_data* area_ptr, const string &requested_area_path,
        content_manifest* manif_ptr,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    string manifest_to_path(
        const content_manifest &manifest, AREA_TYPE type
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        AREA_TYPE* out_type = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    
    void load_area_into_vector(
        content_manifest* manifest, AREA_TYPE type, bool from_backup
    );
    
};


/**
 * @brief Responsible for loading and storing game content
 * bitmaps into memory.
 */
class bitmap_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //Manager proper.
    bitmap_manager list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const content_manifest &manifest,
        const string &extension
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * custom particle generators into memory.
 */
class custom_particle_gen_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of custom particle generators.
    map<string, particle_generator> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_generator(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * global animations into memory.
 */
class global_anim_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of animations.
    map<string, animation_database> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_animation_db(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * GUI definitions into memory.
 */
class gui_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of GUI definitions.
    map<string, data_node> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * hazards into memory.
 */
class hazard_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of hazards.
    map<string, hazard> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_hazard(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * liquids into memory.
 */
class liquid_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of liquids.
    map<string, liquid*> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_liquid(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * misc. configurations into memory.
 */
class misc_config_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_system_anim(
        data_node* anim_def_file, const string &name,
        animation_instance &anim
    );
    
};



/**
 * @brief Responsible for loading and storing game content
 * mob animations into memory.
 */
class mob_anim_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of animations, by category.
    vector<map<string, animation_database> > list;
    
    //Manifests, by category.
    vector<map<string, content_manifest> > manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const content_manifest &manifest, const string &category,
        const string &type
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        string* out_category = nullptr, string* out_type = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void fill_cat_manifests_from_pack(
        mob_category* category, const string &pack_name
    );
    void load_animation_db(content_manifest* manifest, CONTENT_LOAD_LEVEL level, MOB_CATEGORY category_id);
    
};


/**
 * @brief Responsible for loading and storing game content
 * mob types into memory.
 */
class mob_type_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of all mob types.
    mob_type_lists list;
    
    //Manifests, by category.
    vector<map<string, content_manifest> > manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const content_manifest &manifest, const string &category
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        string* out_category = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level);
    void unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level);
    void unload_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level);
};


/**
 * @brief Responsible for loading and storing game content
 * songs into memory.
 */
class song_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of liquids.
    map<string, song> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_song(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * song tracks into memory.
 */
class song_track_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //Manager proper.
    audio_stream_manager list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const content_manifest &manifest, const string &extension
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * sound effects into memory.
 */
class sound_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //Manager proper.
    sample_manager list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(
        const content_manifest &manifest, const string &extension
    ) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr,
        string* out_extension = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
};


/**
 * @brief Responsible for loading and storing game content
 * spike damage types into memory.
 */
class spike_damage_type_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of spike damage types.
    map<string, spike_damage_type> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_spike_damage_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * spray types into memory.
 */
class spray_type_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of spray types.
    map<string, spray_type> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_spray_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * status types into memory.
 */
class status_type_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of status types.
    map<string, status_type*> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_status_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Responsible for loading and storing game content
 * weather conditions into memory.
 */
class weather_condition_content_manager : public content_type_manager {

public:

    //--- Members ---
    
    //List of weather conditions.
    map<string, weather> list;
    
    //Manifests.
    map<string, content_manifest> manifests;
    
    
    //--- Function declarations ---
    
    void clear_manifests() override;
    void fill_manifests() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    string manifest_to_path(const content_manifest &manifest) const;
    void path_to_manifest(
        const string &path, content_manifest* out_manifest = nullptr
    ) const;
    void unload_all(CONTENT_LOAD_LEVEL level) override;
    
    
private:

    //--- Function declarations ---
    void load_weather_condition(content_manifest* manifest, CONTENT_LOAD_LEVEL level);
    
};
