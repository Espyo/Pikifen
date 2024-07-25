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


//Type of content.
enum CONTENT_TYPE {
    //Custom particle generator.
    CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    
    //Hazard.
    CONTENT_TYPE_HAZARD,
    
    //Liquid.
    CONTENT_TYPE_LIQUID,
    
    //Mob type.
    CONTENT_TYPE_MOB_TYPE,
    
    //Spike damage type.
    CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
    
    //Spray type.
    CONTENT_TYPE_SPRAY_TYPE,
    
    //Status type.
    CONTENT_TYPE_STATUS_TYPE,
    
    //Weather condition.
    CONTENT_TYPE_WEATHER_CONDITION,
};


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
    
    void load(CONTENT_TYPE type, bool load_resources);
    void unload(CONTENT_TYPE type, bool unload_resources);
    
    private:
    void load_custom_particle_generators(const string &folder, bool load_resources);
    void load_hazards(const string &folder, bool load_resources);
    void load_liquids(const string &folder, bool load_resources);
    void load_mob_types(const string &folder, bool load_resources);
    void load_mob_types_of_category(const string &folder, mob_category* category, bool load_resources);
    void load_spike_damage_types(const string &folder, bool load_resources);
    void load_spray_types(const string &folder, bool load_resources);
    void load_status_types(const string &folder, bool load_resources);
    void load_weather_conditions(const string &folder, bool load_resources);
    void unload_custom_particle_generators(bool unload_resources);
    void unload_hazards(bool unload_resources);
    void unload_liquids(bool unload_resources);
    void unload_mob_type(mob_type* mt, bool unload_resources);
    void unload_mob_types(bool unload_resources);
    void unload_mob_types_of_category(mob_category* category, bool unload_resources);
    void unload_spike_damage_types(bool unload_resources);
    void unload_spray_types(bool unload_resources);
    void unload_status_types(bool unload_resources);
    void unload_weather_conditions(bool unload_resources);
};
