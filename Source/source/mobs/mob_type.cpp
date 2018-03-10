/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob type class and mob type-related functions.
 */

#include <algorithm>

#include "../const.h"
#include "../functions.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "../load.h"
#include "mob_fsm.h"
#include "mob_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"
#include "../vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates a non-specific mob type.
 */
mob_type::mob_type(size_t category_id) :
    category(mob_categories.get(category_id)),
    main_color(al_map_rgb(128, 128, 128)),
    show_health(true),
    casts_shadow(true),
    radius(0),
    height(0),
    move_speed(0),
    rotation_speed(DEF_ROTATION_SPEED),
    always_active(false),
    pushes(false),
    pushable(false),
    pushes_with_hitboxes(false),
    max_health(0),
    health_regen(0),
    territory_radius(0),
    max_carriers(0),
    weight(0),
    itch_damage(0),
    itch_time(0),
    first_state_nr(INVALID),
    is_obstacle(false),
    spike_damage(nullptr),
    create_mob_func(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a mob type.
 */
mob_type::~mob_type() {
    states.clear();
    anims.destroy();
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file, if any.
 */
void mob_type::load_parameters(data_node* file) {
    if(load_parameters_func) {
        load_parameters_func(file);
    }
}


/* ----------------------------------------------------------------------------
 * Loads any resources into memory, if any.
 */
void mob_type::load_resources(data_node* file) {
    if(load_resources_func) {
        load_resources_func(file);
    }
}


/* ----------------------------------------------------------------------------
 * Specifies what animation conversions there are, if any.
 */
anim_conversion_vector mob_type::get_anim_conversions() {
    if(get_anim_conversions_func) {
        return get_anim_conversions_func();
    }
    return anim_conversion_vector();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded resources from memory.
 */
void mob_type::unload_resources() {
    if(unload_resources_func) {
        unload_resources_func();
    }
}


/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    //Special mob types.
    create_special_mob_types();
    for(auto mt = spec_mob_types.begin(); mt != spec_mob_types.end(); ++mt) {
        string folder = SPECIAL_MOBS_FOLDER_PATH + "/" + mt->first;
        data_node file(folder + "/Data.txt");
        if(!file.file_was_opened) continue;
        
        load_mob_type_from_file(mt->second, file, load_resources, folder);
    }
    
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* category = mob_categories.get(c);
        load_mob_types(category, load_resources);
    }
    
    //Pikmin type order.
    for(auto p = pikmin_types.begin(); p != pikmin_types.end(); ++p) {
        if(
            find(
                pikmin_order_strings.begin(), pikmin_order_strings.end(),
                p->first
            ) == pikmin_order_strings.end()
        ) {
            log_error(
                "Pikmin type \"" + p->first + "\" was not found "
                "in the Pikmin order list in the config file!"
            );
            pikmin_order_strings.push_back(p->first);
        }
    }
    for(size_t o = 0; o < pikmin_order_strings.size(); ++o) {
        string s = pikmin_order_strings[o];
        if(pikmin_types.find(s) != pikmin_types.end()) {
            pikmin_order.push_back(pikmin_types[s]);
        } else {
            log_error(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    for(auto l = leader_types.begin(); l != leader_types.end(); ++l) {
        if(
            find(
                leader_order_strings.begin(), leader_order_strings.end(),
                l->first
            ) == leader_order_strings.end()
        ) {
            log_error(
                "Leader type \"" + l->first + "\" was not found "
                "in the leader order list in the config file!"
            );
            leader_order_strings.push_back(l->first);
        }
    }
    for(size_t o = 0; o < leader_order_strings.size(); ++o) {
        string s = leader_order_strings[o];
        if(leader_types.find(s) != leader_types.end()) {
            leader_order.push_back(leader_types[s]);
        } else {
            log_error(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the config file!"
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the mob types from a category's folder.
 * category:       Pointer to the mob category.
 * load_resources: False if you don't need the images and sounds,
 *   so it loads faster.
 */
void load_mob_types(mob_category* category, bool load_resources) {
    if(category->folder.empty()) return;
    bool folder_found;
    vector<string> types =
        folder_to_vector(category->folder, true, &folder_found);
    if(!folder_found) {
        log_error("Folder \"" + category->folder + "\" not found!");
    }
    
    for(size_t t = 0; t < types.size(); ++t) {
    
        data_node file(category->folder + "/" + types[t] + "/Data.txt");
        if(!file.file_was_opened) continue;
        
        mob_type* mt;
        mt = category->create_type();
        
        load_mob_type_from_file(
            mt, file, load_resources, category->folder + "/" + types[t]
        );
        
        category->register_type(mt);
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Creates the special mob types.
 */
void create_special_mob_types() {
    mob_category* cat = mob_categories.get(MOB_CATEGORY_SPECIAL);
    
    //Info spot.
    mob_type* info_spot_mt = new mob_type(MOB_CATEGORY_SPECIAL);
    info_spot_mt->name = "Info spot";
    info_spot_mt->radius = 16;
    info_spot_mt->create_mob_func =
    [] (const point pos, const float angle, const string & vars) -> mob* {
        info_spot* m = new info_spot(pos, angle, vars);
        info_spots.push_back(m);
        return m;
    };
    info_spot_mt->erase_mob_func =
    [] (mob * m) {
        info_spots.erase(
            find(info_spots.begin(), info_spots.end(), (info_spot*) m)
        );
    };
    cat->register_type(info_spot_mt);
    
    //Nectar.
    mob_type* nectar_mt = new mob_type(MOB_CATEGORY_SPECIAL);
    nectar_mt->name = "Nectar";
    nectar_mt->always_active = true;
    nectar_mt->radius = 8;
    nectar_mt->create_mob_func =
    [] (const point pos, const float angle, const string & vars) -> mob* {
        nectar* m = new nectar(pos, vars);
        nectars.push_back(m);
        return m;
    };
    nectar_mt->erase_mob_func =
    [] (mob * m) {
        nectars.erase(
            find(nectars.begin(), nectars.end(), (nectar*) m)
        );
    };
    cat->register_type(nectar_mt);
}


/* ----------------------------------------------------------------------------
 * Loads a mob type's info from a text file.
 */
void load_mob_type_from_file(
    mob_type* mt, data_node &file,
    const bool load_resources, const string &folder
) {

    string spike_damage_name;
    
    reader_setter rs(&file);
    rs.set("name",                mt->name);
    rs.set("always_active",       mt->always_active);
    rs.set("main_color",          mt->main_color);
    rs.set("max_carriers",        mt->max_carriers);
    rs.set("max_health",          mt->max_health);
    rs.set("health_regen",        mt->health_regen);
    rs.set("itch_damage",         mt->itch_damage);
    rs.set("itch_time",           mt->itch_time);
    rs.set("move_speed",          mt->move_speed);
    rs.set("rotation_speed",      mt->rotation_speed);
    rs.set("territory_radius",    mt->territory_radius);
    rs.set("radius",              mt->radius);
    rs.set("height",              mt->height);
    rs.set("weight",              mt->weight);
    rs.set("pushes",              mt->pushes);
    rs.set("pushable",            mt->pushable);
    rs.set("show_health",         mt->show_health);
    rs.set("casts_shadow",        mt->casts_shadow);
    rs.set("is_obstacle",         mt->is_obstacle);
    rs.set("spike_damage",        spike_damage_name);
    
    mt->rotation_speed = deg_to_rad(mt->rotation_speed);
    
    auto sd_it = spike_damage_types.find(spike_damage_name);
    if(!spike_damage_name.empty()) {
        if(sd_it == spike_damage_types.end()) {
            log_error(
                "Spike damage type \"" + spike_damage_name + "\" not found!",
                file.get_child_by_name("spike_damage")
            );
        } else {
            mt->spike_damage = &(sd_it->second);
        }
    }
    
    data_node* hazards_node = file.get_child_by_name("resistances");
    vector<string> hazards_strs = semicolon_list_to_vector(hazards_node->value);
    for(size_t h = 0; h < hazards_strs.size(); ++h) {
        string hazard_name = hazards_strs[h];
        if(hazards.find(hazard_name) == hazards.end()) {
            log_error("Unknown hazard \"" + hazard_name + "\"!", hazards_node);
        } else {
            mt->resistances.push_back(&(hazards[hazard_name]));
        }
    }
    
    data_node* spike_damage_vuln_node =
        file.get_child_by_name("spike_damage_vulnerabilities");
    size_t n_sd_vuln =
        spike_damage_vuln_node->get_nr_of_children();
    for(size_t v = 0; v < n_sd_vuln; ++v) {
    
        data_node* vul_node =
            spike_damage_vuln_node->get_child(v);
            
        auto sdv_it = spike_damage_types.find(vul_node->name);
        if(sdv_it == spike_damage_types.end()) {
            log_error(
                "Spike damage type \"" + vul_node->name + "\" not found!",
                vul_node
            );
        } else {
            mt->spike_damage_vulnerabilities[&(sdv_it->second)] =
                s2f(vul_node->value) / 100;
        }
    }
    
    data_node* reaches_node = file.get_child_by_name("reaches");
    size_t n_reaches = reaches_node->get_nr_of_children();
    for(size_t r = 0; r < n_reaches; ++r) {
    
        mob_type::reach_struct new_reach;
        new_reach.name = reaches_node->get_child(r)->name;
        vector<string> r_strings = split(reaches_node->get_child(r)->value);
        
        if(r_strings.size() != 2 && r_strings.size() != 4) {
            log_error(
                "Reach \"" + new_reach.name +
                "\" isn't made up of 2 or 4 words!",
                reaches_node->get_child(r)
            );
            continue;
        }
        
        new_reach.radius_1 = s2f(r_strings[0]);
        new_reach.angle_1 = deg_to_rad(s2f(r_strings[1]));
        if(r_strings.size() == 4) {
            new_reach.radius_2 = s2f(r_strings[2]);
            new_reach.angle_2 = deg_to_rad(s2f(r_strings[3]));
        }
        mt->reaches.push_back(new_reach);
    }
    
    if(load_resources) {
        data_node anim_file = load_data_file(folder + "/Animations.txt");
        mt->anims = load_animation_database_from_file(&anim_file);
        mt->anims.fix_body_part_pointers();
        
        data_node script_file;
        script_file.load_file(folder + "/Script.txt", true, true);
        size_t old_n_states = mt->states.size();
        load_init_actions(
            mt, script_file.get_child_by_name("init"), &mt->init_actions
        );
        load_script(mt, script_file.get_child_by_name("script"), &mt->states);
        
        if(mt->states.size() > old_n_states) {
        
            string first_state_name;
            data_node* first_state_node;
            for(size_t n = 0; n < script_file.get_nr_of_children(); ++n) {
                data_node* dn = script_file.get_child(n);
                string s = get_var_value(dn->name, "first_state", "");
                if(!s.empty()) {
                    first_state_name = s;
                    first_state_node;
                    break;
                }
            }
            
            for(size_t s = 0; s < mt->states.size(); ++s) {
                if(mt->states[s]->name == first_state_name) {
                    mt->first_state_nr = s;
                    break;
                }
            }
            if(!first_state_name.empty() && mt->first_state_nr == INVALID) {
                log_error(
                    "The name of the first state \"" +
                    first_state_name + "\" is invalid!",
                    (first_state_node ? first_state_node : &script_file)
                );
            }
        }
    }
    
    mt->load_parameters(&file);
    
    if(load_resources) {
        mt->load_resources(&file);
        mt->anims.create_conversions(mt->get_anim_conversions());
    }
}


/* ----------------------------------------------------------------------------
 * Unloads a type of mob.
 */
void unload_mob_type(mob_type* mt, const bool unload_resources) {
    if(unload_resources) {
        mt->anims.destroy();
        unload_script(mt);
        
        mt->unload_resources();
    }
}


/* ----------------------------------------------------------------------------
 * Unloads all loaded types of mob from memory.
 */
void unload_mob_types(const bool unload_resources) {
    leader_order.clear();
    pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* category = mob_categories.get(c);
        unload_mob_types(category, unload_resources);
    }
}


/* ----------------------------------------------------------------------------
 * Unloads all loaded types of mob from a category.
 * category:         Pointer to the mob category.
 * unload_resources: False if you don't need to unload images or sounds,
 *   since they never got loaded in the first place.
 */
void unload_mob_types(mob_category* category, bool unload_resources) {

    vector<string> type_names;
    category->get_type_names(type_names);
    
    for(size_t t = 0; t < type_names.size(); ++t) {
        mob_type* mt = category->get_type(type_names[t]);
        unload_mob_type(mt, unload_resources);
    }
    
    category->clear_types();
}


/* ----------------------------------------------------------------------------
 * Adds carrying-related states to the FSM.
 */
void mob_type::add_carrying_states() {

    easy_fsm_creator efc;
    
    efc.new_state("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    vector<mob_state*> new_states = efc.finish();
    fix_states(new_states, "");
    
    states.insert(states.end(), new_states.begin(), new_states.end());
    
}
