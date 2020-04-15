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
#include "../game.h"
#include "../load.h"
#include "../mob_fsms/gen_mob_fsm.h"
#include "../mob_script_action.h"
#include "../utils/string_utils.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"

using std::string;

/* ----------------------------------------------------------------------------
 * Creates a non-specific mob type.
 */
mob_type::mob_type(size_t category_id) :
    category(game.mob_categories.get(category_id)),
    main_color(al_map_rgb(128, 128, 128)),
    show_health(true),
    casts_shadow(true),
    radius(0),
    height(0),
    move_speed(0),
    rotation_speed(DEF_ROTATION_SPEED),
    can_free_move(false),
    pushes(false),
    pushable(false),
    pushes_with_hitboxes(false),
    walkable(false),
    max_health(100),
    health_regen(0),
    territory_radius(0),
    max_carriers(0),
    weight(0),
    itch_damage(0),
    itch_time(0),
    target_type(MOB_TARGET_TYPE_NONE),
    huntable_targets(
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_ENEMY
    ),
    hurtable_targets(
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_FRAGILE
    ),
    starting_team(MOB_TEAM_NONE),
    first_state_nr(INVALID),
    death_state_nr(INVALID),
    appears_in_area_editor(true),
    blocks_carrier_pikmin(false),
    default_vulnerability(1.0f),
    spike_damage(nullptr),
    max_span(0.0f) {
    
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
 * Adds carrying-related states to the FSM.
 */
void mob_type::add_carrying_states() {

    easy_fsm_creator efc;
    
    efc.new_state("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EV_CARRY_STUCK); {
            efc.change_state("carriable_stuck");
        }
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("carriable_stuck", ENEMY_EXTRA_STATE_CARRIABLE_STUCK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    vector<mob_state*> new_states = efc.finish();
    
    states.insert(states.end(), new_states.begin(), new_states.end());
    
}


/* ----------------------------------------------------------------------------
 * Specifies what animation conversions there are, if any.
 */
anim_conversion_vector mob_type::get_anim_conversions() {
    return anim_conversion_vector();
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file, if any.
 */
void mob_type::load_properties(data_node* file) { }


/* ----------------------------------------------------------------------------
 * Loads any resources into memory, if any.
 */
void mob_type::load_resources(data_node* file) { }


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
 * Loads a mob type's info from a text file.
 */
void load_mob_type_from_file(
    mob_type* mt, data_node &file,
    const bool load_resources, const string &folder
) {

    reader_setter rs(&file);
    
    string spike_damage_str;
    string target_type_str;
    string huntable_targets_str;
    string hurtable_targets_str;
    string team_str;
    data_node* spike_damage_node;
    data_node* target_type_node;
    data_node* huntable_targets_node;
    data_node* hurtable_targets_node;
    data_node* team_node;
    
    rs.set("appears_in_area_editor", mt->appears_in_area_editor);
    rs.set("blocks_carrier_pikmin", mt->blocks_carrier_pikmin);
    rs.set("can_free_move", mt->can_free_move);
    rs.set(
        "can_hunt", huntable_targets_str, &huntable_targets_node
    );
    rs.set(
        "can_hurt", hurtable_targets_str, &hurtable_targets_node
    );
    rs.set("casts_shadow", mt->casts_shadow);
    rs.set("default_vulnerability", mt->default_vulnerability);
    rs.set("health_regen", mt->health_regen);
    rs.set("height", mt->height);
    rs.set("itch_damage", mt->itch_damage);
    rs.set("itch_time", mt->itch_time);
    rs.set("main_color", mt->main_color);
    rs.set("max_carriers", mt->max_carriers);
    rs.set("max_health", mt->max_health);
    rs.set("move_speed", mt->move_speed);
    rs.set("name", mt->name);
    rs.set("pushable", mt->pushable);
    rs.set("pushes", mt->pushes);
    rs.set("pushes_with_hitboxes", mt->pushes_with_hitboxes);
    rs.set("radius", mt->radius);
    rs.set("rectangular_dimensions", mt->rectangular_dim);
    rs.set("rotation_speed", mt->rotation_speed);
    rs.set("show_health", mt->show_health);
    rs.set("spike_damage", spike_damage_str, &spike_damage_node);
    rs.set("target_type", target_type_str, &target_type_node);
    rs.set("team", team_str, &team_node);
    rs.set("territory_radius", mt->territory_radius);
    rs.set("walkable", mt->walkable);
    rs.set("weight", mt->weight);
    
    mt->rotation_speed = deg_to_rad(mt->rotation_speed);
    
    data_node* vulnerabilities_node = file.get_child_by_name("vulnerabilities");
    for(size_t h = 0; h < vulnerabilities_node->get_nr_of_children(); ++h) {
        data_node* vulnerability_node = vulnerabilities_node->get_child(h);
        auto hazard_it = game.hazards.find(vulnerability_node->name);
        if(hazard_it == game.hazards.end()) {
            log_error(
                "Unknown hazard \"" + vulnerability_node->name + "\"!",
                vulnerability_node
            );
        } else {
            mt->hazard_vulnerabilities[&(hazard_it->second)] =
                s2f(vulnerability_node->value) / 100;
        }
    }
    
    auto sd_it = game.spike_damage_types.find(spike_damage_str);
    if(spike_damage_node) {
        if(sd_it == game.spike_damage_types.end()) {
            log_error(
                "Spike damage type \"" + spike_damage_str + "\" not found!",
                spike_damage_node
            );
        } else {
            mt->spike_damage = &(sd_it->second);
        }
    }
    
    if(team_node) {
        size_t t = string_to_team_nr(team_str);
        if(t != INVALID) {
            mt->starting_team = t;
        } else {
            log_error(
                "Invalid team \"" + team_str + "\"!",
                team_node
            );
        }
    }
    
    data_node* spike_damage_vuln_node =
        file.get_child_by_name("spike_damage_vulnerabilities");
    size_t n_sd_vuln =
        spike_damage_vuln_node->get_nr_of_children();
    for(size_t v = 0; v < n_sd_vuln; ++v) {
    
        data_node* vul_node =
            spike_damage_vuln_node->get_child(v);
            
        auto sdv_it = game.spike_damage_types.find(vul_node->name);
        if(sdv_it == game.spike_damage_types.end()) {
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
        reader_setter spawn_rs(spawn_node);
        mob_type::spawn_struct new_spawn;
        string coords_str;
        
        new_spawn.name = spawn_node->name;
        spawn_rs.set("object", new_spawn.mob_type_name);
        spawn_rs.set("relative", new_spawn.relative);
        spawn_rs.set("coordinates", coords_str);
        spawn_rs.set("angle", new_spawn.angle);
        spawn_rs.set("vars", new_spawn.vars);
        spawn_rs.set("link_object_to_spawn", new_spawn.link_object_to_spawn);
        spawn_rs.set("link_spawn_to_object", new_spawn.link_spawn_to_object);
        spawn_rs.set("momentum", new_spawn.momentum);
        
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
        reader_setter child_rs(child_node);
        mob_type::child_struct new_child;
        
        string limb_draw_method;
        string hold_rotation_method;
        data_node* limb_draw_node;
        data_node* hold_rotation_node;
        
        new_child.name = child_node->name;
        child_rs.set("spawn", new_child.spawn_name);
        child_rs.set("parent_holds", new_child.parent_holds);
        child_rs.set("hold_body_part", new_child.hold_body_part);
        child_rs.set("hold_offset_distance", new_child.hold_offset_dist);
        child_rs.set("hold_offset_angle", new_child.hold_offset_angle);
        child_rs.set(
            "hold_rotation_method", hold_rotation_method, &hold_rotation_node
        );
        child_rs.set("handle_damage", new_child.handle_damage);
        child_rs.set("relay_damage", new_child.relay_damage);
        child_rs.set("handle_events", new_child.handle_events);
        child_rs.set("relay_events", new_child.relay_events);
        child_rs.set("handle_statuses", new_child.handle_statuses);
        child_rs.set("relay_statuses", new_child.relay_statuses);
        child_rs.set("limb_animation", new_child.limb_anim_name);
        child_rs.set("limb_thickness", new_child.limb_thickness);
        child_rs.set("limb_parent_body_part", new_child.limb_parent_body_part);
        child_rs.set("limb_parent_offset", new_child.limb_parent_offset);
        child_rs.set("limb_child_body_part", new_child.limb_child_body_part);
        child_rs.set("limb_child_offset", new_child.limb_child_offset);
        child_rs.set("limb_draw_method", limb_draw_method, &limb_draw_node);
        
        new_child.hold_offset_angle = deg_to_rad(new_child.hold_offset_angle);
        
        if(limb_draw_node) {
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
            } else {
                log_error(
                    "Unknow limb draw method \"" +
                    limb_draw_method + "\"!", limb_draw_node
                );
            }
        }
        
        if(hold_rotation_node) {
            if(hold_rotation_method == "never") {
                new_child.hold_rotation_method =
                    HOLD_ROTATION_METHOD_NEVER;
            } else if(hold_rotation_method == "face_parent") {
                new_child.hold_rotation_method =
                    HOLD_ROTATION_METHOD_FACE_HOLDER;
            } else if(hold_rotation_method == "copy_parent") {
                new_child.hold_rotation_method =
                    HOLD_ROTATION_METHOD_COPY_HOLDER;
            } else {
                log_error(
                    "Unknow parent holding rotation method \"" +
                    hold_rotation_method + "\"!", hold_rotation_node
                );
            }
        }
        
        mt->children.push_back(new_child);
    }
    
    if(target_type_node) {
        size_t target_type_value =
            string_to_mob_target_type(target_type_str);
        if(target_type_value == INVALID) {
            log_error(
                "Unknown target type \"" + target_type_str + "\"!",
                target_type_node
            );
        } else {
            mt->target_type = target_type_value;
        }
    }
    
    vector<string> huntable_targets_strs =
        semicolon_list_to_vector(huntable_targets_str);
    if(huntable_targets_node) {
        mt->huntable_targets = 0;
    }
    for(size_t h = 0; h < huntable_targets_strs.size(); ++h) {
        size_t v = string_to_mob_target_type(huntable_targets_strs[h]);
        if(v == INVALID) {
            log_error(
                "Unknown target type \"" + huntable_targets_strs[h] + "\"!",
                huntable_targets_node
            );
        } else {
            mt->huntable_targets |= (uint16_t) v;
        }
    }
    
    vector<string> hurtable_targets_strs =
        semicolon_list_to_vector(hurtable_targets_str);
    if(hurtable_targets_node) {
        mt->hurtable_targets = 0;
    }
    for(size_t h = 0; h < hurtable_targets_strs.size(); ++h) {
        size_t v = string_to_mob_target_type(hurtable_targets_strs[h]);
        if(v == INVALID) {
            log_error(
                "Unknown target type \"" + hurtable_targets_strs[h] + "\"!",
                hurtable_targets_node
            );
        } else {
            mt->hurtable_targets |= (uint16_t) v;
        }
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
    
    mt->load_properties(&file);
    
    if(load_resources) {
        mt->load_resources(&file);
        mt->anims.create_conversions(mt->get_anim_conversions(), &file);
    }
    
    mt->max_span = mt->radius;
    
    if(load_resources) {
        //Calculate the max span based on the animations's hitboxes.
        for(size_t s = 0; s < mt->anims.sprites.size(); ++s) {
            sprite* s_ptr = mt->anims.sprites[s];
            for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
                hitbox* h_ptr = &s_ptr->hitboxes[h];
                
                float d = dist(point(0, 0), h_ptr->pos).to_float();
                d += h_ptr->radius;
                mt->max_span = std::max(mt->max_span, d);
            }
        }
    }
    
    if(mt->rectangular_dim.x != 0) {
        mt->max_span =
            std::max(
                mt->max_span,
                dist(point(0, 0), mt->rectangular_dim / 2.0).to_float()
            );
    }
}


/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* category = game.mob_categories.get(c);
        load_mob_types(category, load_resources);
    }
    
    //Pikmin type order.
    for(auto &p : game.mob_types.pikmin) {
        if(
            find(
                game.pikmin_order_strings.begin(), game.pikmin_order_strings.end(),
                p.first
            ) == game.pikmin_order_strings.end()
        ) {
            log_error(
                "Pikmin type \"" + p.first + "\" was not found "
                "in the Pikmin order list in the config file!"
            );
            game.pikmin_order_strings.push_back(p.first);
        }
    }
    for(size_t o = 0; o < game.pikmin_order_strings.size(); ++o) {
        string s = game.pikmin_order_strings[o];
        if(game.mob_types.pikmin.find(s) != game.mob_types.pikmin.end()) {
            game.pikmin_order.push_back(game.mob_types.pikmin[s]);
        } else {
            log_error(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    for(auto &l : game.mob_types.leader) {
        if(
            find(
                game.leader_order_strings.begin(), game.leader_order_strings.end(),
                l.first
            ) == game.leader_order_strings.end()
        ) {
            log_error(
                "Leader type \"" + l.first + "\" was not found "
                "in the leader order list in the config file!"
            );
            game.leader_order_strings.push_back(l.first);
        }
    }
    for(size_t o = 0; o < game.leader_order_strings.size(); ++o) {
        string s = game.leader_order_strings[o];
        if(game.mob_types.leader.find(s) != game.mob_types.leader.end()) {
            game.leader_order.push_back(game.mob_types.leader[s]);
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
    game.leader_order.clear();
    game.pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* category = game.mob_categories.get(c);
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
