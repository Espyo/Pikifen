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
#include "area/area.h"
#include "mobs/mob_utils.h"
#include "misc_structs.h"
#include "spike_damage.h"
#include "spray_type.h"

using std::map;
using std::string;
using std::vector;


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
 * @brief Responsible for loading and storing game content of a given type
 * into memory.
 */
class content_type_manager {

public:

    //--- Function declarations ---

    virtual void clear_manifest() = 0;
    virtual void fill_manifest() = 0;
    virtual string get_name() const = 0;
    virtual string get_perf_mon_measurement_name() const = 0;
    virtual void load_all(CONTENT_LOAD_LEVEL level) = 0;
    virtual void unload_all(CONTENT_LOAD_LEVEL level) = 0;


protected:
    
    //--- Function declarations ---
    void fill_manifest_map(
        map<string, content_manifest> &manifest, const string &content_path, bool folders
    );
    void fill_manifest_map_from_pkg(
        map<string, content_manifest> &manifest, const string &package_name,
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

    //Manifest, by area type.
    vector<map<string, content_manifest>> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void load_area(
        area_data* area_ptr,
        const string &internal_name, const string &package_name, AREA_TYPE type,
        CONTENT_LOAD_LEVEL level, bool from_backup
    );
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---

    void load_area_into_vector(
        const string &internal_name, const string &package_name, AREA_TYPE type,
        bool from_backup
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
    
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:
    
    //--- Function declarations ---
    void load_generator(
        const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
    );

};


/**
 * @brief Responsible for loading and storing game content
 * global animations into memory.
 */
class global_anim_content_manager : public content_type_manager {

public:

    //--- Members ---

    //List of global animations.
    map<string, single_animation_suite> list;
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_global_animation(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_hazard(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_liquid(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

};


/**
 * @brief Responsible for loading and storing game content
 * misc. configurations into memory.
 */
class misc_config_content_manager : public content_type_manager {

public:

    //--- Members ---

    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;

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
        
    //Manifest, by category.
    vector<map<string, content_manifest> > manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level);
    void unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level);
    void unload_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level);
};


/**
 * @brief Responsible for loading and storing game content
 * audio samples into memory.
 */
class sample_content_manager : public content_type_manager {

public:

    //--- Members ---

    //Manager proper.
    sfx_sample_manager list;
    
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_song(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
    
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_spike_damage_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_spray_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_status_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

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
        
    //Manifest.
    map<string, content_manifest> manifest;


    //--- Function declarations ---

    void clear_manifest() override;
    void fill_manifest() override;
    string get_name() const override;
    string get_perf_mon_measurement_name() const override;
    void load_all(CONTENT_LOAD_LEVEL level) override;
    void unload_all(CONTENT_LOAD_LEVEL level) override;


private:

    //--- Function declarations ---
    void load_weather_condition(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level);

};
