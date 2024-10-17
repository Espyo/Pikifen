/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Content manager class and related functions.
 */

#include "content_manager.h"

#include <algorithm>

#include "game.h"
#include "load.h"
#include "mob_types/mob_type.h"
#include "utils/allegro_utils.h"


/**
 * @brief Loads some game content.
 * 
 * @param type Type of game content to load.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load(CONTENT_TYPE type, bool load_resources) {
    string folder = GAME_DATA_FOLDER_PATH;

    switch(type) {
    case CONTENT_TYPE_CUSTOM_PARTICLE_GEN: {
        load_custom_particle_generators(folder, load_resources);
        break;
    } case CONTENT_TYPE_HAZARD: {
        load_hazards(folder, load_resources);
        break;
    } case CONTENT_TYPE_LIQUID: {
        load_liquids(folder, load_resources);
        break;
    } case CONTENT_TYPE_MOB_TYPE: {
        load_mob_types(folder, load_resources);
        break;
    } case CONTENT_TYPE_SPIKE_DAMAGE_TYPE: {
        load_spike_damage_types(folder, load_resources);
        break;
    } case CONTENT_TYPE_SPRAY_TYPE: {
        load_spray_types(folder, load_resources);
        break;
    } case CONTENT_TYPE_STATUS_TYPE: {
        load_status_types(folder, load_resources);
        break;
    } case CONTENT_TYPE_WEATHER_CONDITION: {
        load_weather_conditions(folder, load_resources);
        break;
    }
    }
}


/**
 * @brief Loads a user-made particle generator.
 * 
 * @param path Path to the particle generator.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_custom_particle_generator(
    const string& path, bool load_resources
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    particle_generator new_pg;
    new_pg.path = path;
    new_pg.load_from_data_node(&file, load_resources);
    custom_particle_generators[new_pg.name] = new_pg;
}


/**
 * @brief Loads user-made particle generators.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_custom_particle_generators(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Custom particle generators");
    }
    
    vector<string> generator_files =
        folder_to_vector(PARTICLE_GENERATORS_FOLDER_PATH, false);
        
    for(size_t g = 0; g < generator_files.size(); g++) {
        load_custom_particle_generator(
            PARTICLE_GENERATORS_FOLDER_PATH + "/" + generator_files[g],
            load_resources
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a hazard.
 * 
 * @param path Path to the hazard.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_hazard(
    const string& path, bool load_resources
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    hazard new_h;
    new_h.path = path;
    new_h.load_from_data_node(&file);
    hazards[new_h.name] = new_h;
}


/**
 * @brief Loads hazards.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_hazards(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Hazards");
    }
    
    vector<string> hazard_files =
        folder_to_vector(HAZARDS_FOLDER_PATH, false);
        
    for(size_t h = 0; h < hazard_files.size(); h++) {
        load_hazard(
            HAZARDS_FOLDER_PATH + "/" + hazard_files[h],
            load_resources
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a liquid.
 * 
 * @param path Path to the liquid.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_liquid(
    const string& path, bool load_resources
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    liquid* new_l = new liquid();
    new_l->path = path;
    new_l->load_from_data_node(&file, load_resources);
    liquids[new_l->name] = new_l;
}


/**
 * @brief Loads liquids.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_liquids(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Liquid types");
    }
    
    vector<string> liquid_files =
        folder_to_vector(LIQUIDS_FOLDER_PATH, false);
        
    for(size_t l = 0; l < liquid_files.size(); l++) {
        load_liquid(
            LIQUIDS_FOLDER_PATH + "/" + liquid_files[l],
            load_resources
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads mob types.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_mob_types(const string& folder, bool load_resources) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) {
            continue;
        }
        
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        if(game.perf_mon) {
            game.perf_mon->start_measurement(
                "Object types -- " + category->name
            );
        }
        
        load_mob_types_of_category(folder, category, load_resources);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
    }
    
    //Pikmin type order.
    vector<string> missing_pikmin_order_types;
    for(auto &p : mob_types.pikmin) {
        if(
            find(
                game.config.pikmin_order_strings.begin(),
                game.config.pikmin_order_strings.end(),
                p.first
            ) == game.config.pikmin_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_pikmin_order_types.push_back(p.first);
        }
    }
    if(!missing_pikmin_order_types.empty()) {
        std::sort(
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
        game.config.pikmin_order_strings.insert(
            game.config.pikmin_order_strings.end(),
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.pikmin_order_strings.size(); o++) {
        string s = game.config.pikmin_order_strings[o];
        if(mob_types.pikmin.find(s) != mob_types.pikmin.end()) {
            game.config.pikmin_order.push_back(mob_types.pikmin[s]);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    vector<string> missing_leader_order_types;
    for(auto &l : mob_types.leader) {
        if(
            find(
                game.config.leader_order_strings.begin(),
                game.config.leader_order_strings.end(),
                l.first
            ) == game.config.leader_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_leader_order_types.push_back(l.first);
        }
    }
    if(!missing_leader_order_types.empty()) {
        std::sort(
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
        game.config.leader_order_strings.insert(
            game.config.leader_order_strings.end(),
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.leader_order_strings.size(); o++) {
        string s = game.config.leader_order_strings[o];
        if(mob_types.leader.find(s) != mob_types.leader.end()) {
            game.config.leader_order.push_back(mob_types.leader[s]);
        } else {
            game.errors.report(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the config file!"
            );
        }
    }
    
    //Create the special mob types.
    create_special_mob_types();
}


/**
 * @brief Loads the mob types from a category's folder.
 *
 * @param folder Folder to load from.
 * @param category Pointer to the mob category.
 * @param load_resources False if you don't need the images and sounds,
 * so it loads faster.
 */
void content_manager::load_mob_types_of_category(const string& folder, mob_category* category, bool load_resources) {
    if(category->folder_path.empty()) return;
    bool folder_found;
    vector<string> types =
        folder_to_vector(category->folder_path, true, &folder_found);
    if(!folder_found) {
        game.errors.report(
            "Mob category folder \"" + category->folder_path + "\" not found!"
        );
    }
    
    for(size_t t = 0; t < types.size(); t++) {
        string type_folder_name =
            types[t];
        string type_folder_path =
            category->folder_path + "/" + type_folder_name;
            
        data_node file(type_folder_path + "/Data.txt");
        if(!file.file_was_opened) continue;
        
        mob_type* mt;
        mt = category->create_type();
        mt->load_from_data_node(&file, load_resources, type_folder_path);
        category->register_type(mt);
        mt->folder_name = type_folder_name;
        mt->path = type_folder_path;
        
    }
    
}


/**
 * @brief Loads a spike damage type.
 * 
 * @param path Path to the spike damage type.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_spike_damage_type(const string& path, bool load_resources) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    spike_damage_type new_t;
    new_t.path = path;
    new_t.load_from_data_node(&file);
    spike_damage_types[new_t.name] = new_t;
}


/**
 * @brief Loads spike damage types.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_spike_damage_types(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spike damage types");
    }
    
    vector<string> type_files =
        folder_to_vector(SPIKE_DAMAGES_FOLDER_PATH, false);
        
    for(size_t t = 0; t < type_files.size(); t++) {
        load_spike_damage_type(
            SPIKE_DAMAGES_FOLDER_PATH + "/" + type_files[t],
            load_resources
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a spray type.
 * 
 * @param path Path to the spray type.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_spray_type(const string& path, bool load_resources) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    spray_type new_t;
    new_t.path = path;
    new_t.load_from_data_node(&file, load_resources);
    spray_types.push_back(new_t);
}


/**
 * @brief Loads spray types.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_spray_types(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spray types");
    }
    
    vector<string> type_files =
        folder_to_vector(SPRAYS_FOLDER_PATH, false);
        
    for(size_t t = 0; t < type_files.size(); t++) {
        load_spray_type(
            SPRAYS_FOLDER_PATH + "/" + type_files[t],
            load_resources
        );
    }
    
    //Spray type order.
    vector<string> missing_spray_order_types;
    for(size_t t = 0; t < spray_types.size(); t++) {
        if(
            find(
                game.config.spray_order_strings.begin(),
                game.config.spray_order_strings.end(),
                spray_types[t].name
            ) == game.config.spray_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_spray_order_types.push_back(spray_types[t].name);
        }
    }
    if(!missing_spray_order_types.empty()) {
        std::sort(
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
        game.config.spray_order_strings.insert(
            game.config.spray_order_strings.end(),
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
    }
    vector<spray_type> temp;
    for(size_t o = 0; o < game.config.spray_order_strings.size(); o++) {
        string s = game.config.spray_order_strings[o];
        bool found = false;
        for(size_t t = 0; t < spray_types.size(); t++) {
            if(spray_types[t].name == s) {
                temp.push_back(spray_types[t]);
                found = true;
            }
        }
        
        if(!found) {
            game.errors.report(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the config file!"
            );
        }
    }
    spray_types = temp;
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a status type.
 * 
 * @param path Path to the status type.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_status_type(const string& path, bool load_resources) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    status_type* new_t = new status_type();
    new_t->path = path;
    new_t->load_from_data_node(&file, load_resources);
    status_types[new_t->name] = new_t;
}


/**
 * @brief Loads status types.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_status_types(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Status types");
    }
    
    vector<string> type_files =
        folder_to_vector(STATUSES_FOLDER_PATH, false);
    vector<status_type*> types_with_replacements;
    vector<string> types_with_replacements_names;
    
    for(size_t t = 0; t < type_files.size(); t++) {
        load_status_type(
            STATUSES_FOLDER_PATH + "/" + type_files[t],
            load_resources
        );
    }

    for(auto& s : status_types) {
        if(!s.second->replacement_on_timeout_str.empty()) {
            types_with_replacements.push_back(s.second);
            types_with_replacements_names.push_back(
                s.second->replacement_on_timeout_str
            );
        }
    }
    
    for(size_t s = 0; s < types_with_replacements.size(); s++) {
        string rn = types_with_replacements_names[s];
        bool found = false;
        for(auto &s2 : status_types) {
            if(s2.first == rn) {
                types_with_replacements[s]->replacement_on_timeout =
                    s2.second;
                found = true;
                break;
            }
        }
        if(found) continue;
        
        game.errors.report(
            "The status effect type \"" +
            types_with_replacements[s]->name +
            "\" has a replacement effect called \"" + rn + "\", but there is "
            "no status effect with that name!"
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a weather condition.
 * 
 * @param path Path to the weather condition.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_weather_condition(const string& path, bool load_resources) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    weather new_w;
    new_w.path = path;
    new_w.load_from_data_node(&file);
    weather_conditions[new_w.name] = new_w;
}


/**
 * @brief Loads weather conditions.
 * 
 * @param folder Folder to load from.
 * @param load_resources If true, things like bitmaps and the like will be
 * loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void content_manager::load_weather_conditions(const string& folder, bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Weather");
    }
    
    vector<string> weather_files =
        folder_to_vector(WEATHER_FOLDER_PATH, false);
        
    for(size_t w = 0; w < weather_files.size(); w++) {
        load_weather_condition(
            WEATHER_FOLDER_PATH + "/" + weather_files[w],
            load_resources
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Unloads some loaded content.
 * 
 * @param type Type of content to unload.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload(CONTENT_TYPE type, bool unload_resources) {
    switch(type) {
    case CONTENT_TYPE_CUSTOM_PARTICLE_GEN: {
        unload_custom_particle_generators(unload_resources);
        break;
    } case CONTENT_TYPE_HAZARD: {
        unload_hazards(unload_resources);
        break;
    } case CONTENT_TYPE_LIQUID: {
        unload_liquids(unload_resources);
        break;
    } case CONTENT_TYPE_MOB_TYPE: {
        unload_mob_types(unload_resources);
        break;
    } case CONTENT_TYPE_SPIKE_DAMAGE_TYPE: {
        unload_spike_damage_types(unload_resources);
        break;
    } case CONTENT_TYPE_SPRAY_TYPE: {
        unload_spray_types(unload_resources);
        break;
    } case CONTENT_TYPE_STATUS_TYPE: {
        unload_status_types(unload_resources);
        break;
    } case CONTENT_TYPE_WEATHER_CONDITION: {
        unload_weather_conditions(unload_resources);
        break;
    }
    }
}


/**
 * @brief Unloads loaded user-made particle generators.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_custom_particle_generators(bool unload_resources) {
    for(
        auto g = custom_particle_generators.begin();
        g != custom_particle_generators.end();
        ++g
    ) {
        game.bitmaps.free(g->second.base_particle.bitmap);
    }
    custom_particle_generators.clear();
}


/**
 * @brief Unloads loaded hazards.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_hazards(bool unload_resources) {
    hazards.clear();
}


/**
 * @brief Unloads loaded liquids.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_liquids(bool unload_resources) {
    for(auto &l : liquids) {
        l.second->anim_db.destroy();
        delete l.second;
    }
    liquids.clear();
}


/**
 * @brief Unloads a type of mob.
 *
 * @param mt Mob type to unload.
 * @param unload_resources False if you don't need to unload images or sounds,
 * since they never got loaded in the first place.
 */
void content_manager::unload_mob_type(mob_type* mt, bool unload_resources) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* s_ptr = mt->sounds[s].sample;
        if(!s) continue;
        game.audio.samples.free(s_ptr);
    }
    if(unload_resources) {
        mt->anims.destroy();
        unload_script(mt);
        
        mt->unload_resources();
    }
}


/**
 * @brief Unloads loaded mob types.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_mob_types(bool unload_resources) {
    game.config.leader_order.clear();
    game.config.pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        unload_mob_types_of_category(category, unload_resources);
    }
}


/**
 * @brief Unloads all loaded types of mob from a category.
 *
 * @param category Pointer to the mob category.
 * @param unload_resources False if you don't need to unload images or sounds,
 * since they never got loaded in the first place.
 */
void content_manager::unload_mob_types_of_category(mob_category* category, bool unload_resources) {

    vector<string> type_names;
    category->get_type_names(type_names);
    
    for(size_t t = 0; t < type_names.size(); t++) {
        mob_type* mt = category->get_type(type_names[t]);
        unload_mob_type(mt, unload_resources);
    }
    
    category->clear_types();
}


/**
 * @brief Unloads loaded spike damage types.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_spike_damage_types(bool unload_resources) {
    spike_damage_types.clear();
}


/**
 * @brief Unloaded loaded spray types.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_spray_types(bool unload_resources) {
    for(size_t s = 0; s < spray_types.size(); s++) {
        game.bitmaps.free(spray_types[s].bmp_spray);
    }
    spray_types.clear();
}


/**
 * @brief Unloaded loaded status types.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_status_types(bool unload_resources) {
    for(auto &s : status_types) {
        if(unload_resources) {
            s.second->overlay_anim_db.destroy();
        }
        delete s.second;
    }
    status_types.clear();
}


/**
 * @brief Unloads loaded weather conditions.
 * @param unload_resources If resources got loaded, set this to true to
 * unload them.
 */
void content_manager::unload_weather_conditions(bool unload_resources) {
    weather_conditions.clear();
}
