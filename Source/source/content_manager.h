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
 * @brief Manages everything regarding game content, be it assets, types of
 * mobs, etc.
 */
struct content_manager {
    //List of particle generators declared by the user.
    map<string, particle_generator> custom_particle_generators;
    
    //List of hazards.
    map<string, hazard> hazards;
    
    //List of liquids.
    map<string, liquid*> liquids;
    
    //List of all mob types.
    mob_type_lists mob_types;
    
    //List of spike damage types.
    map<string, spike_damage_type> spike_damage_types;
    
    //List of spray types.
    vector<spray_type> spray_types;
    
    //List of status types.
    map<string, status_type*> status_types;
    
    //List of weather conditions.
    map<string, weather> weather_conditions;

    //--- Function declarations ---
    
    content_manager();
    void load(CONTENT_TYPE type, CONTENT_LOAD_LEVEL level);
    void unload(CONTENT_TYPE type, CONTENT_LOAD_LEVEL level);
    
private:

    //--- Members ---

    CONTENT_LOAD_LEVEL load_levels[N_CONTENT_TYPES];

    //--- Function declarations ---

    void load_custom_particle_generator(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_custom_particle_generators(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_hazard(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_hazards(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_liquid(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_liquids(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_mob_types(
        const string &folder, CONTENT_LOAD_LEVEL level
        );
    void load_mob_types_of_category(
        const string &folder, mob_category* category, CONTENT_LOAD_LEVEL level
    );
    void load_spike_damage_type(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_spike_damage_types(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_spray_type(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_spray_types(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_status_type(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_status_types(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void load_weather_condition(
        const string &path, CONTENT_LOAD_LEVEL level
    );
    void load_weather_conditions(
        const string &folder, CONTENT_LOAD_LEVEL level
    );
    void unload_custom_particle_generators(CONTENT_LOAD_LEVEL level);
    void unload_hazards(CONTENT_LOAD_LEVEL level);
    void unload_liquids(CONTENT_LOAD_LEVEL level);
    void unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level);
    void unload_mob_types(CONTENT_LOAD_LEVEL level);
    void unload_mob_types_of_category(
        mob_category* category, CONTENT_LOAD_LEVEL level
    );
    void unload_spike_damage_types(CONTENT_LOAD_LEVEL level);
    void unload_spray_types(CONTENT_LOAD_LEVEL level);
    void unload_status_types(CONTENT_LOAD_LEVEL level);
    void unload_weather_conditions(CONTENT_LOAD_LEVEL level);
};
