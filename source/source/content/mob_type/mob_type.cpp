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

#include "../../core/const.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob_script/gen_mob_fsm.h"
#include "../mob/bridge.h"
#include "../other/mob_script_action.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"


using std::string;


namespace MOB_TYPE {

//Index of the default "idling" animation in an animation database.
const size_t ANIM_IDLING = 0;

//The default acceleration of a mob type.
const float DEF_ACCELERATION = 400.0f;

//The default rotation speed of a mob type.
const float DEF_ROTATION_SPEED = 630.0f;

}


/**
 * @brief Constructs a new mob type object.
 *
 * @param category_id The ID of the category it belongs to.
 */
mob_type::mob_type(MOB_CATEGORY category_id) :
    category(game.mob_categories.get(category_id)),
    custom_category_name(category->name) {
    
}


/**
 * @brief Destroys the mob type object.
 */
mob_type::~mob_type() {
    states.clear();
    for(size_t a = 0; a < init_actions.size(); a++) {
        delete init_actions[a];
    }
    init_actions.clear();
}


/**
 * @brief Adds carrying-related states to the FSM.
 */
void mob_type::add_carrying_states() {

    easy_fsm_creator efc;
    
    efc.new_state("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EV_PATH_BLOCKED); {
            efc.change_state("carriable_stuck");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.change_state("carriable_thrown");
        }
    }
    
    efc.new_state("carriable_stuck", ENEMY_EXTRA_STATE_CARRIABLE_STUCK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("carriable_moving");
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("carriable_thrown", ENEMY_EXTRA_STATE_CARRIABLE_THROWN); {
        efc.new_event(MOB_EV_LANDED); {
            efc.run(gen_mob_fsm::lose_momentum);
            efc.run(gen_mob_fsm::carry_get_path);
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


/**
 * @brief Specifies what animation conversions there are, if any.
 *
 * @return The animation conversions.
 */
anim_conversion_vector mob_type::get_anim_conversions() const {
    return anim_conversion_vector();
}


/**
 * @brief Loads properties from a data file, if any.
 */
void mob_type::load_cat_properties(data_node*) { }


/**
 * @brief Loads any resources into memory, if any.
 */
void mob_type::load_cat_resources(data_node*) { }


/**
 * @brief Loads mob type data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 * @param folder_path Path to the folder this mob type is in.
 */
void mob_type::load_from_data_node(
    data_node* node, CONTENT_LOAD_LEVEL level, const string &folder_path
) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    reader_setter rs(node);
    
    string custom_carry_spots_str;
    string spike_damage_str;
    string target_type_str;
    string huntable_targets_str;
    string hurtable_targets_str;
    string team_str;
    string inactive_logic_str;
    data_node* area_editor_tips_node = nullptr;
    data_node* custom_carry_spots_node = nullptr;
    data_node* spike_damage_node = nullptr;
    data_node* target_type_node = nullptr;
    data_node* huntable_targets_node = nullptr;
    data_node* hurtable_targets_node = nullptr;
    data_node* team_node = nullptr;
    data_node* inactive_logic_node = nullptr;
    
    rs.set("acceleration", acceleration);
    rs.set("appears_in_area_editor", appears_in_area_editor);
    rs.set(
        "area_editor_recommend_links_from",
        area_editor_recommend_links_from
    );
    rs.set(
        "area_editor_recommend_links_to",
        area_editor_recommend_links_to
    );
    rs.set("area_editor_tips", area_editor_tips, &area_editor_tips_node);
    rs.set("blackout_radius", blackout_radius);
    rs.set("can_block_paths", can_block_paths);
    rs.set("can_free_move", can_free_move);
    rs.set(
        "can_hunt", huntable_targets_str, &huntable_targets_node
    );
    rs.set(
        "can_hurt", hurtable_targets_str, &hurtable_targets_node
    );
    rs.set("can_walk_on_others", can_walk_on_others);
    rs.set("casts_shadow", casts_shadow);
    rs.set(
        "custom_carry_spots", custom_carry_spots_str, &custom_carry_spots_node
    );
    rs.set("custom_category_name", custom_category_name);
    rs.set("default_vulnerability", default_vulnerability);
    rs.set("description", description);
    rs.set("has_group", has_group);
    rs.set("health_regen", health_regen);
    rs.set("height", height);
    rs.set("inactive_logic", inactive_logic_str, &inactive_logic_node);
    rs.set("itch_damage", itch_damage);
    rs.set("itch_time", itch_time);
    rs.set("main_color", main_color);
    rs.set("max_carriers", max_carriers);
    rs.set("max_health", max_health);
    rs.set("move_speed", move_speed);
    rs.set("pushable", pushable);
    rs.set("pushes", pushes);
    rs.set("pushes_softly", pushes_softly);
    rs.set("pushes_with_hitboxes", pushes_with_hitboxes);
    rs.set("radius", radius);
    rs.set("rectangular_dimensions", rectangular_dim);
    rs.set("rotation_speed", rotation_speed);
    rs.set("show_health", show_health);
    rs.set("spike_damage", spike_damage_str, &spike_damage_node);
    rs.set("target_type", target_type_str, &target_type_node);
    rs.set("team", team_str, &team_node);
    rs.set("terrain_radius", terrain_radius);
    rs.set("territory_radius", territory_radius);
    rs.set("walkable", walkable);
    rs.set("weight", weight);
    
    if(area_editor_tips_node) {
        area_editor_tips = unescape_string(area_editor_tips);
    }
    
    if(!custom_carry_spots_str.empty()) {
        vector<string> points =
            semicolon_list_to_vector(custom_carry_spots_str);
        if(points.size() != max_carriers) {
            game.errors.report(
                "The number of custom carry spots (" + i2s(points.size()) +
                ") does not match the number of max carriers (" +
                i2s(max_carriers) + ")!",
                custom_carry_spots_node
            );
        } else {
            for(size_t p = 0; p < points.size(); p++) {
                custom_carry_spots.push_back(s2p(points[p]));
            }
        }
    }
    
    rotation_speed = deg_to_rad(rotation_speed);
    
    //Vulnerabilities.
    data_node* vulnerabilities_node =
        node->get_child_by_name("vulnerabilities");
    for(size_t h = 0; h < vulnerabilities_node->get_nr_of_children(); h++) {
    
        data_node* vuln_node = vulnerabilities_node->get_child(h);
        auto hazard_it = game.content.hazards.list.find(vuln_node->name);
        vector<string> words = split(vuln_node->value);
        float percentage = default_vulnerability;
        string status_name;
        bool status_overrides = false;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            status_name = words[1];
        }
        if(words.size() >= 3) {
            status_overrides = s2b(words[2]);
        }
        auto status_it = game.content.status_types.list.find(status_name);
        
        if(hazard_it == game.content.hazards.list.end()) {
            game.errors.report(
                "Unknown hazard \"" + vuln_node->name + "\"!",
                vuln_node
            );
            
        } else if(
            !status_name.empty() && status_it == game.content.status_types.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_name + "\"!",
                vuln_node
            );
            
        } else {
            mob_type::vulnerability_t &vuln =
                hazard_vulnerabilities[&(hazard_it->second)];
            vuln.damage_mult = percentage / 100.0f;
            if(!status_name.empty()) {
                vuln.status_to_apply = status_it->second;
            }
            vuln.status_overrides = status_overrides;
        }
    }
    
    //Spike damage.
    auto sd_it = game.content.spike_damage_types.list.find(spike_damage_str);
    if(spike_damage_node) {
        if(sd_it == game.content.spike_damage_types.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + spike_damage_str + "\"!",
                spike_damage_node
            );
        } else {
            spike_damage = &(sd_it->second);
        }
    }
    
    //Team.
    if(team_node) {
        MOB_TEAM t = string_to_team_nr(team_str);
        if(t != INVALID) {
            starting_team = t;
        } else {
            game.errors.report(
                "Invalid team \"" + team_str + "\"!",
                team_node
            );
        }
    }
    
    //Inactive logic.
    if(inactive_logic_node) {
        if(inactive_logic_str == "normal") {
            inactive_logic =
                0;
        } else if(inactive_logic_str == "ticks") {
            inactive_logic =
                INACTIVE_LOGIC_FLAG_TICKS;
        } else if(inactive_logic_str == "interactions") {
            inactive_logic =
                INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else if(inactive_logic_str == "all_logic") {
            inactive_logic =
                INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else {
            game.errors.report(
                "Invalid inactive logic \"" + inactive_logic_str + "\"!",
                inactive_logic_node
            );
        }
    }
    
    //Spike damage vulnerabilities.
    data_node* spike_damage_vuln_node =
        node->get_child_by_name("spike_damage_vulnerabilities");
    size_t n_sd_vuln =
        spike_damage_vuln_node->get_nr_of_children();
    for(size_t v = 0; v < n_sd_vuln; v++) {
    
        data_node* vul_node = spike_damage_vuln_node->get_child(v);
        auto sdv_it = game.content.spike_damage_types.list.find(vul_node->name);
        vector<string> words = split(vul_node->value);
        float percentage = 1.0f;
        string status_name;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            status_name = words[1];
        }
        auto status_it = game.content.status_types.list.find(status_name);
        
        if(sdv_it == game.content.spike_damage_types.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + vul_node->name + "\"!",
                vul_node
            );
            
        } else if(
            !status_name.empty() && status_it == game.content.status_types.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_name + "\"!",
                vul_node
            );
            
        } else {
            auto &s = spike_damage_vulnerabilities[&(sdv_it->second)];
            s.damage_mult = percentage / 100.0f;
            s.status_to_apply = status_it->second;
            
        }
    }
    
    //Status vulnerabilities.
    data_node* status_vuln_node =
        node->get_child_by_name("status_vulnerabilities");
    size_t n_s_vuln =
        status_vuln_node->get_nr_of_children();
    for(size_t v = 0; v < n_s_vuln; v++) {
    
        data_node* vul_node = status_vuln_node->get_child(v);
        auto sv_it = game.content.status_types.list.find(vul_node->name);
        vector<string> words = split(vul_node->value);
        float percentage = 1.0f;
        string status_override_name;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            status_override_name = words[1];
        }
        auto status_override_it = game.content.status_types.list.find(status_override_name);
        
        if(sv_it == game.content.status_types.list.end()) {
            game.errors.report(
                "Unknown status type \"" + vul_node->name + "\"!",
                vul_node
            );
            
        } else if(
            !status_override_name.empty() &&
            status_override_it == game.content.status_types.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_override_name + "\"!",
                vul_node
            );
            
        } else {
            auto &s = status_vulnerabilities[sv_it->second];
            s.damage_mult = percentage / 100.0f;
            if(status_override_it != game.content.status_types.list.end()) {
                s.status_to_apply = status_override_it->second;
            }
            s.status_overrides = true;
        }
        
    }
    
    //Reaches.
    data_node* reaches_node = node->get_child_by_name("reaches");
    size_t n_reaches = reaches_node->get_nr_of_children();
    for(size_t r = 0; r < n_reaches; r++) {
    
        mob_type::reach_t new_reach;
        new_reach.name = reaches_node->get_child(r)->name;
        vector<string> r_strings = split(reaches_node->get_child(r)->value);
        
        if(r_strings.size() != 2 && r_strings.size() != 4) {
            game.errors.report(
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
        reaches.push_back(new_reach);
    }
    
    //Spawns.
    data_node* spawns_node = node->get_child_by_name("spawns");
    size_t n_spawns = spawns_node->get_nr_of_children();
    for(size_t s = 0; s < n_spawns; s++) {
    
        data_node* spawn_node = spawns_node->get_child(s);
        reader_setter spawn_rs(spawn_node);
        mob_type::spawn_t new_spawn;
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
        
        spawns.push_back(new_spawn);
    }
    
    //Children.
    data_node* children_node = node->get_child_by_name("children");
    size_t n_children = children_node->get_nr_of_children();
    for(size_t c = 0; c < n_children; c++) {
    
        data_node* child_node = children_node->get_child(c);
        reader_setter child_rs(child_node);
        mob_type::child_t new_child;
        
        string limb_draw_method;
        string hold_rotation_method;
        data_node* limb_draw_node;
        data_node* hold_rotation_node;
        
        new_child.name = child_node->name;
        child_rs.set("spawn", new_child.spawn_name);
        child_rs.set("parent_holds", new_child.parent_holds);
        child_rs.set("hold_body_part", new_child.hold_body_part);
        child_rs.set("hold_offset_distance", new_child.hold_offset_dist);
        child_rs.set(
            "hold_offset_vertical_distance", new_child.hold_offset_vert_dist
        );
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
                new_child.limb_draw_method = LIMB_DRAW_METHOD_BELOW_BOTH;
            } else if(limb_draw_method == "below_child") {
                new_child.limb_draw_method = LIMB_DRAW_METHOD_BELOW_CHILD;
            } else if(limb_draw_method == "below_parent") {
                new_child.limb_draw_method = LIMB_DRAW_METHOD_BELOW_PARENT;
            } else if(limb_draw_method == "above_parent") {
                new_child.limb_draw_method = LIMB_DRAW_METHOD_ABOVE_PARENT;
            } else if(limb_draw_method == "above_child") {
                new_child.limb_draw_method = LIMB_DRAW_METHOD_ABOVE_CHILD;
            } else if(limb_draw_method == "above_both") {
                new_child.limb_draw_method = LIMB_DRAW_METHOD_ABOVE_BOTH;
            } else {
                game.errors.report(
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
                game.errors.report(
                    "Unknow parent holding rotation method \"" +
                    hold_rotation_method + "\"!", hold_rotation_node
                );
            }
        }
        
        children.push_back(new_child);
    }
    
    //Sounds.
    data_node* sounds_node = node->get_child_by_name("sounds");
    size_t n_sounds = sounds_node->get_nr_of_children();
    for(size_t s = 0; s < n_sounds; s++) {
    
        data_node* sound_node = sounds_node->get_child(s);
        reader_setter sound_rs(sound_node);
        mob_type::sound_t new_sound;
        
        string file_str;
        data_node* file_node;
        string type_str;
        data_node* type_node;
        string stack_mode_str;
        data_node* stack_mode_node;
        float volume_float = 100.0f;
        float speed_float = 100.0f;
        bool loop_bool = false;
        
        new_sound.name = sound_node->name;
        
        sound_rs.set("file", file_str, &file_node);
        sound_rs.set("type", type_str, &type_node);
        sound_rs.set("stack_mode", stack_mode_str, &stack_mode_node);
        sound_rs.set("stack_min_pos", new_sound.config.stack_min_pos);
        sound_rs.set("loop", loop_bool);
        sound_rs.set("volume", volume_float);
        sound_rs.set("speed", speed_float);
        sound_rs.set("volume_deviation", new_sound.config.gain_deviation);
        sound_rs.set("speed_deviation", new_sound.config.speed_deviation);
        sound_rs.set("random_delay", new_sound.config.random_delay);
        
        new_sound.sample = game.content.sounds.list.get(file_str, file_node);
        
        if(type_node) {
            if(type_str == "world_global") {
                new_sound.type = SOUND_TYPE_WORLD_GLOBAL;
            } else if(type_str == "world_pos") {
                new_sound.type = SOUND_TYPE_WORLD_POS;
            } else if(type_str == "world_ambiance") {
                new_sound.type = SOUND_TYPE_WORLD_AMBIANCE;
            } else if(type_str == "ui") {
                new_sound.type = SOUND_TYPE_UI;
            } else {
                game.errors.report(
                    "Unknow sound effect type \"" +
                    type_str + "\"!", type_node
                );
            }
        }
        
        if(stack_mode_node) {
            if(stack_mode_str == "normal") {
                new_sound.config.stack_mode = SOUND_STACK_MODE_NORMAL;
            } else if(stack_mode_str == "override") {
                new_sound.config.stack_mode = SOUND_STACK_MODE_OVERRIDE;
            } else if(stack_mode_str == "never") {
                new_sound.config.stack_mode = SOUND_STACK_MODE_NEVER;
            } else {
                game.errors.report(
                    "Unknow sound effect stack mode \"" +
                    stack_mode_str + "\"!", stack_mode_node
                );
            }
        }
        
        if(loop_bool) {
            enable_flag(new_sound.config.flags, SOUND_FLAG_LOOP);
        }
        
        new_sound.config.gain = volume_float / 100.0f;
        new_sound.config.gain = clamp(new_sound.config.gain, 0.0f, 1.0f);
        
        new_sound.config.speed = speed_float / 100.0f;
        new_sound.config.speed = std::max(0.0f, new_sound.config.speed);
        
        new_sound.config.gain_deviation /= 100.0f;
        new_sound.config.speed_deviation /= 100.0f;
        
        sounds.push_back(new_sound);
    }
    
    //Area editor properties.
    data_node* ae_props_node =
        node->get_child_by_name("area_editor_properties");
    size_t n_ae_props = ae_props_node->get_nr_of_children();
    for(size_t p = 0; p < n_ae_props; p++) {
    
        data_node* prop_node = ae_props_node->get_child(p);
        reader_setter prop_rs(prop_node);
        string type_str;
        string list_str;
        data_node* type_node = nullptr;
        
        mob_type::area_editor_prop_t new_prop;
        new_prop.name = prop_node->name;
        
        prop_rs.set("var", new_prop.var);
        prop_rs.set("type", type_str, &type_node);
        prop_rs.set("def_value", new_prop.def_value);
        prop_rs.set("min_value", new_prop.min_value);
        prop_rs.set("max_value", new_prop.max_value);
        prop_rs.set("list", list_str);
        prop_rs.set("tooltip", new_prop.tooltip);
        
        if(new_prop.var.empty()) {
            game.errors.report(
                "You need to specify the area editor property's name!",
                prop_node
            );
        }
        
        if(type_str == "text") {
            new_prop.type = AEMP_TYPE_TEXT;
        } else if(type_str == "int") {
            new_prop.type = AEMP_TYPE_INT;
        } else if(type_str == "real") {
            new_prop.type = AEMP_TYPE_REAL;
        } else if(type_str == "bool") {
            new_prop.type = AEMP_TYPE_BOOL;
        } else if(type_str == "list") {
            new_prop.type = AEMP_TYPE_LIST;
        } else if(type_str == "number_list") {
            new_prop.type = AEMP_TYPE_NR_LIST;
        } else {
            game.errors.report(
                "Unknown area editor property type \"" + type_str + "\"!",
                type_node
            );
        }
        
        if(new_prop.min_value > new_prop.max_value) {
            std::swap(new_prop.min_value, new_prop.max_value);
        }
        
        if(new_prop.type == AEMP_TYPE_LIST || new_prop.type == AEMP_TYPE_NR_LIST) {
            if(list_str.empty()) {
                game.errors.report(
                    "For this area editor property type, you need to specify "
                    "a list of values!", prop_node
                );
            } else {
                new_prop.value_list = semicolon_list_to_vector(list_str);
            }
        }
        
        new_prop.tooltip = unescape_string(new_prop.tooltip);
        
        area_editor_props.push_back(new_prop);
    }
    
    if(target_type_node) {
        MOB_TARGET_FLAG target_type_value =
            string_to_mob_target_type(target_type_str);
        if(target_type_value == INVALID) {
            game.errors.report(
                "Unknown target type \"" + target_type_str + "\"!",
                target_type_node
            );
        } else {
            target_type = target_type_value;
        }
    }
    
    vector<string> huntable_targets_strs =
        semicolon_list_to_vector(huntable_targets_str);
    if(huntable_targets_node) {
        huntable_targets = 0;
    }
    for(size_t h = 0; h < huntable_targets_strs.size(); h++) {
        size_t v = string_to_mob_target_type(huntable_targets_strs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + huntable_targets_strs[h] + "\"!",
                huntable_targets_node
            );
        } else {
            huntable_targets |= (bitmask_16_t) v;
        }
    }
    
    vector<string> hurtable_targets_strs =
        semicolon_list_to_vector(hurtable_targets_str);
    if(hurtable_targets_node) {
        hurtable_targets = 0;
    }
    for(size_t h = 0; h < hurtable_targets_strs.size(); h++) {
        size_t v = string_to_mob_target_type(hurtable_targets_strs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + hurtable_targets_strs[h] + "\"!",
                hurtable_targets_node
            );
        } else {
            hurtable_targets |= (bitmask_16_t) v;
        }
    }
    
    //Resources.
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        anim_db = &game.content.mob_anim_dbs.list[category->id][manifest->internal_name];
        anim_db->fill_sound_idx_caches(this);
        
        data_node script_file;
        script_file.load_file(folder_path + "/script.txt", true, true);
        size_t old_n_states = states.size();
        
        data_node* death_state_name_node =
            script_file.get_child_by_name("death_state");
        death_state_name = death_state_name_node->value;
        
        states_ignoring_death =
            semicolon_list_to_vector(
                script_file.get_child_by_name("states_ignoring_death")->value
            );
            
        states_ignoring_spray =
            semicolon_list_to_vector(
                script_file.get_child_by_name("states_ignoring_spray")->value
            );
            
        states_ignoring_hazard =
            semicolon_list_to_vector(
                script_file.get_child_by_name("states_ignoring_hazard")->value
            );
            
        //Load init actions.
        load_actions(
            this,
            script_file.get_child_by_name("init"), &init_actions, nullptr
        );
        //Load the rest of the script.
        load_script(
            this,
            script_file.get_child_by_name("script"),
            script_file.get_child_by_name("global"),
            &states
        );
        
        if(states.size() > old_n_states) {
        
            data_node* first_state_name_node =
                script_file.get_child_by_name("first_state");
            string first_state_name = first_state_name_node->value;
            
            for(size_t s = 0; s < states.size(); s++) {
                if(states[s]->name == first_state_name) {
                    first_state_idx = s;
                    break;
                }
            }
            if(first_state_idx == INVALID) {
                game.errors.report(
                    "Unknown state \"" + first_state_name + "\" "
                    "to set as the first state!",
                    first_state_name_node
                );
            }
            
            if(!death_state_name.empty()) {
                for(size_t s = 0; s < states.size(); s++) {
                    if(states[s]->name == death_state_name) {
                        death_state_idx = s;
                        break;
                    }
                }
                if(death_state_idx == INVALID) {
                    game.errors.report(
                        "Unknown state \"" + death_state_name + "\" "
                        "to set as the death state!",
                        death_state_name_node
                    );
                }
            }
        }
    }
    
    //Category-specific properties.
    load_cat_properties(node);
    
    //Category-specific resources.
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        load_cat_resources(node);
        anim_db->create_conversions(get_anim_conversions(), node);
    }
    
    physical_span =
        calculate_mob_physical_span(
            radius,
            (level >= CONTENT_LOAD_LEVEL_FULL ? anim_db->hitbox_span : 0),
            rectangular_dim
        );
        
    if(custom_category_name.empty()) {
        custom_category_name = category->name;
    }
}


/**
 * @brief Unloads loaded resources from memory.
 */
void mob_type::unload_resources() {
    for(size_t s = 0; s < sounds.size(); s++) {
        game.content.sounds.list.free(sounds[s].name);
    }
}


/**
 * @brief Grabs an animation conversion vector, filled with base animations,
 * and outputs one that combines all base animations with their groups.
 *
 * @param v The animation conversion vector.
 * @param base_anim_total How many base animations exist.
 * @return The vector.
 */
anim_conversion_vector
mob_type_with_anim_groups::get_anim_conversions_with_groups(
    const anim_conversion_vector &v, size_t base_anim_total
) const {
    anim_conversion_vector new_v;
    
    for(size_t g = 0; g < animation_group_suffixes.size(); g++) {
        for(size_t c = 0; c < v.size(); c++) {
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


/**
 * @brief Creates special mob types, needed by the engine,
 * that are beyond the ones loaded from the game data folder.
 */
void create_special_mob_types() {
    mob_category* custom_category =
        game.mob_categories.get(MOB_CATEGORY_CUSTOM);
        
    mob_type* bridge_component_type = custom_category->create_type();
    bridge_component_type->name = "Bridge component";
    bridge_component_type->blackout_radius = 0;
    bridge_component_type->appears_in_area_editor = false;
    bridge_component_type->casts_shadow = false;
    bridge_component_type->custom_category_name = "Misc";
    bridge_component_type->height = 8.0f;
    bridge_component_type->physical_span = 8.0f;
    bridge_component_type->radius = 8.0f;
    bridge_component_type->walkable = true;
    bridge_component_type->draw_mob_callback = bridge::draw_component;
    bridge_component_type->pushes = true;
    bridge_component_type->pushes_softly = false;
    custom_category->register_type("bridge_component", bridge_component_type);
}
