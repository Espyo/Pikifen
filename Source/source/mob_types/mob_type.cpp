/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob type class and mob type-related functions.
 */

#include <algorithm>

#include "mob_type.h"

#include "../const.h"
#include "../functions.h"
#include "../load.h"
#include "../mob_fsms/gen_mob_fsm.h"
#include "../utils/string_utils.h"
#include "../vars.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"

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
    can_free_move(false),
    always_active(false),
    pushes(false),
    pushable(false),
    pushes_with_hitboxes(false),
    walkable(false),
    max_health(0),
    health_regen(0),
    territory_radius(0),
    max_carriers(0),
    weight(0),
    itch_damage(0),
    itch_time(0),
    first_state_nr(INVALID),
    death_state_nr(INVALID),
    appears_in_area_editor(true),
    is_obstacle(false),
    is_projectile(false),
    blocks_carrier_pikmin(false),
    projectiles_can_damage(true),
    default_vulnerability(1.0f),
    spike_damage(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a mob type.
 */
mob_type::~mob_type() {
    states.clear();
    anims.destroy();
    for(size_t a = 0; a < init_actions.size(); ++a) {
        delete init_actions[a];
    }
    init_actions.clear();
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file, if any.
 */
void mob_type::load_parameters(data_node* file) { }


/* ----------------------------------------------------------------------------
 * Loads any resources into memory, if any.
 */
void mob_type::load_resources(data_node* file) { }


/* ----------------------------------------------------------------------------
 * Specifies what animation conversions there are, if any.
 */
anim_conversion_vector mob_type::get_anim_conversions() {
    return anim_conversion_vector();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded resources from memory.
 */
void mob_type::unload_resources() { }


/* ----------------------------------------------------------------------------
 * Grabs an animation conversion vector, filled with base animations,
 * and outputs one that combines all base animations with their groups.
 */
anim_conversion_vector
mob_type_with_anim_groups::get_anim_conversions_with_groups(
    const anim_conversion_vector &v, const size_t base_anim_total
) {
    anim_conversion_vector new_v;
    
    for(size_t g = 0; g < animation_group_suffixes.size(); ++g) {
        for(size_t c = 0; c < v.size(); ++c) {
            new_v.push_back(
                make_pair(
                    g * base_anim_total + v[c].first,
                    v[c].second + animation_group_suffixes[g]
                )
            );
        }
    }
    
    return new_v;
}



/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
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
        
        mt->folder_name = types[t];
        
    }
    
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
    rs.set("name",                   mt->name);
    rs.set("always_active",          mt->always_active);
    rs.set("main_color",             mt->main_color);
    rs.set("max_carriers",           mt->max_carriers);
    rs.set("max_health",             mt->max_health);
    rs.set("health_regen",           mt->health_regen);
    rs.set("itch_damage",            mt->itch_damage);
    rs.set("itch_time",              mt->itch_time);
    rs.set("move_speed",             mt->move_speed);
    rs.set("rotation_speed",         mt->rotation_speed);
    rs.set("can_free_move",          mt->can_free_move);
    rs.set("territory_radius",       mt->territory_radius);
    rs.set("radius",                 mt->radius);
    rs.set("rectangular_dimensions", mt->rectangular_dim);
    rs.set("height",                 mt->height);
    rs.set("weight",                 mt->weight);
    rs.set("pushable",               mt->pushable);
    rs.set("pushes",                 mt->pushes);
    rs.set("pushes_with_hitboxes",   mt->pushes_with_hitboxes);
    rs.set("walkable",               mt->walkable);
    rs.set("show_health",            mt->show_health);
    rs.set("casts_shadow",           mt->casts_shadow);
    rs.set("appears_in_area_editor", mt->appears_in_area_editor);
    rs.set("is_obstacle",            mt->is_obstacle);
    rs.set("is_projectile",          mt->is_projectile);
    rs.set("blocks_carrier_pikmin",  mt->blocks_carrier_pikmin);
    rs.set("projectiles_can_damage", mt->projectiles_can_damage);
    rs.set("default_vulnerability",  mt->default_vulnerability);
    rs.set("spike_damage",           spike_damage_name);
    
    mt->rotation_speed = deg_to_rad(mt->rotation_speed);
    
    data_node* vulnerabilities_node = file.get_child_by_name("vulnerabilities");
    for(size_t h = 0; h < vulnerabilities_node->get_nr_of_children(); ++h) {
        data_node* vulnerability_node = vulnerabilities_node->get_child(h);
        auto hazard_it = hazards.find(vulnerability_node->name);
        if(hazard_it == hazards.end()) {
            log_error(
                "Unknown hazard \"" + vulnerability_node->name + "\"!",
                vulnerability_node
            );
        } else {
            mt->hazard_vulnerabilities[&(hazard_it->second)] =
                s2f(vulnerability_node->value) / 100;
        }
    }
    
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
    
    data_node* spawns_node = file.get_child_by_name("spawns");
    size_t n_spawns = spawns_node->get_nr_of_children();
    for(size_t s = 0; s < n_spawns; ++s) {
    
        data_node* spawn_node = spawns_node->get_child(s);
        reader_setter rs(spawn_node);
        mob_type::spawn_struct new_spawn;
        string coords_str;
        
        new_spawn.name = spawn_node->name;
        rs.set("object", new_spawn.mob_type_name);
        rs.set("relative", new_spawn.relative);
        rs.set("coordinates", coords_str);
        rs.set("angle", new_spawn.angle);
        rs.set("vars", new_spawn.vars);
        rs.set("link_object_to_spawn", new_spawn.link_object_to_spawn);
        rs.set("link_spawn_to_object", new_spawn.link_spawn_to_object);
        
        if(!coords_str.empty()) {
            new_spawn.coords_xy = s2p(coords_str, &new_spawn.coords_z);
        }
        new_spawn.angle = deg_to_rad(new_spawn.angle);
        
        mt->spawns.push_back(new_spawn);
    }
    
    data_node* children_node = file.get_child_by_name("children");
    size_t n_children = children_node->get_nr_of_children();
    for(size_t c = 0; c < n_children; ++c) {
    
        data_node* child_node = children_node->get_child(c);
        reader_setter rs(child_node);
        mob_type::child_struct new_child;
        
        string limb_draw_method;
        
        new_child.name = child_node->name;
        rs.set("spawn", new_child.spawn_name);
        rs.set("parent_holds", new_child.parent_holds);
        rs.set("hold_body_part", new_child.hold_body_part);
        rs.set("hold_offset_distance", new_child.hold_offset_dist);
        rs.set("hold_offset_angle", new_child.hold_offset_angle);
        rs.set("handle_damage", new_child.handle_damage);
        rs.set("relay_damage", new_child.relay_damage);
        rs.set("handle_events", new_child.handle_events);
        rs.set("relay_events", new_child.relay_events);
        rs.set("handle_statuses", new_child.handle_statuses);
        rs.set("relay_statuses", new_child.relay_statuses);
        rs.set("limb_animation", new_child.limb_anim_name);
        rs.set("limb_thickness", new_child.limb_thickness);
        rs.set("limb_parent_body_part", new_child.limb_parent_body_part);
        rs.set("limb_parent_offset", new_child.limb_parent_offset);
        rs.set("limb_child_body_part", new_child.limb_child_body_part);
        rs.set("limb_child_offset", new_child.limb_child_offset);
        rs.set("limb_draw_method", limb_draw_method);
        
        new_child.hold_offset_angle = deg_to_rad(new_child.hold_offset_angle);
        if(limb_draw_method == "below_both") {
            new_child.limb_draw_method = LIMB_DRAW_BELOW_BOTH;
        } else if(limb_draw_method == "below_child") {
            new_child.limb_draw_method = LIMB_DRAW_BELOW_CHILD;
        } else if(limb_draw_method == "below_parent") {
            new_child.limb_draw_method = LIMB_DRAW_BELOW_PARENT;
        } else if(limb_draw_method == "above_parent") {
            new_child.limb_draw_method = LIMB_DRAW_ABOVE_PARENT;
        } else if(limb_draw_method == "above_child") {
            new_child.limb_draw_method = LIMB_DRAW_ABOVE_CHILD;
        } else if(limb_draw_method == "above_both") {
            new_child.limb_draw_method = LIMB_DRAW_ABOVE_BOTH;
        }
        
        mt->children.push_back(new_child);
    }
    
    if(load_resources) {
        data_node anim_file = load_data_file(folder + "/Animations.txt");
        mt->anims = load_animation_database_from_file(&anim_file);
        mt->anims.fix_body_part_pointers();
        
        data_node script_file;
        script_file.load_file(folder + "/Script.txt", true, true);
        size_t old_n_states = mt->states.size();
        
        data_node* death_state_name_node =
            script_file.get_child_by_name("death_state");
        mt->death_state_name = death_state_name_node->value;
        
        mt->states_ignoring_death =
            split(
                script_file.get_child_by_name("states_ignoring_death")->value,
                ";"
            );
        for(size_t s = 0; s < mt->states_ignoring_death.size(); ++s) {
            mt->states_ignoring_death[s] =
                trim_spaces(mt->states_ignoring_death[s]);
        }
        
        mt->states_ignoring_spray =
            split(
                script_file.get_child_by_name("states_ignoring_spray")->value,
                ";"
            );
        for(size_t s = 0; s < mt->states_ignoring_spray.size(); ++s) {
            mt->states_ignoring_spray[s] =
                trim_spaces(mt->states_ignoring_spray[s]);
        }
        
        load_init_actions(
            mt, script_file.get_child_by_name("init"), &mt->init_actions
        );
        load_script(mt, script_file.get_child_by_name("script"), &mt->states);
        
        if(mt->states.size() > old_n_states) {
        
            data_node* first_state_name_node =
                script_file.get_child_by_name("first_state");
            string first_state_name = first_state_name_node->value;
            
            for(size_t s = 0; s < mt->states.size(); ++s) {
                if(mt->states[s]->name == first_state_name) {
                    mt->first_state_nr = s;
                    break;
                }
            }
            if(mt->first_state_nr == INVALID) {
                log_error(
                    "The name of the first state \"" +
                    first_state_name + "\" is invalid!",
                    first_state_name_node
                );
            }
            
            if(!mt->death_state_name.empty()) {
                for(size_t s = 0; s < mt->states.size(); ++s) {
                    if(mt->states[s]->name == mt->death_state_name) {
                        mt->death_state_nr = s;
                        break;
                    }
                }
                if(mt->death_state_nr == INVALID) {
                    log_error(
                        "The name of the death state \"" +
                        mt->death_state_name + "\" is invalid!",
                        death_state_name_node
                    );
                }
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
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EVENT_CARRY_STUCK); {
            efc.change_state("carriable_stuck");
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("carriable_stuck", ENEMY_EXTRA_STATE_CARRIABLE_STUCK); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("carriable_moving");
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
