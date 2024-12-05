/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Status effect classes and status effect-related functions.
 */

#include <algorithm>

#include "status.h"

#include "game.h"
#include "load.h"
#include "misc_structs.h"
#include "utils/general_utils.h"


/**
 * @brief Constructs a new status object.
 *
 * @param type Its type.
 */
status::status(status_type* type) :
    type(type) {
    
    time_left = type->auto_remove_time;
}


/**
 * @brief Ticks a status effect instance's time by one frame of logic,
 * but does not tick its effects logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void status::tick(float delta_t) {
    if(type->auto_remove_time > 0.0f) {
        time_left -= delta_t;
        if(time_left <= 0.0f) {
            to_delete = true;
        }
    }
}


/**
 * @brief Loads status type data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void status_type::load_from_data_node(data_node* node, CONTENT_LOAD_LEVEL level) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    reader_setter rs(node);
    
    bool affects_pikmin_bool = false;
    bool affects_leaders_bool = false;
    bool affects_enemies_bool = false;
    bool affects_others_bool = false;
    string reapply_rule_str;
    string sc_type_str;
    string particle_offset_str;
    string particle_gen_str;
    data_node* reapply_rule_node = nullptr;
    data_node* sc_type_node = nullptr;
    data_node* particle_gen_node = nullptr;
    
    rs.set("color",                   color);
    rs.set("tint",                    tint);
    rs.set("glow",                    glow);
    rs.set("affects_pikmin",          affects_pikmin_bool);
    rs.set("affects_leaders",         affects_leaders_bool);
    rs.set("affects_enemies",         affects_enemies_bool);
    rs.set("affects_others",          affects_others_bool);
    rs.set("removable_with_whistle",  removable_with_whistle);
    rs.set("remove_on_hazard_leave",  remove_on_hazard_leave);
    rs.set("auto_remove_time",        auto_remove_time);
    rs.set("reapply_rule",            reapply_rule_str, &reapply_rule_node);
    rs.set("health_change",           health_change);
    rs.set("health_change_ratio",     health_change_ratio);
    rs.set("state_change_type",       sc_type_str, &sc_type_node);
    rs.set("state_change_name",       state_change_name);
    rs.set("animation_change",        animation_change);
    rs.set("speed_multiplier",        speed_multiplier);
    rs.set("attack_multiplier",       attack_multiplier);
    rs.set("defense_multiplier",      defense_multiplier);
    rs.set("maturity_change_amount",  maturity_change_amount);
    rs.set("disables_attack",         disables_attack);
    rs.set("turns_inedible",          turns_inedible);
    rs.set("turns_invisible",         turns_invisible);
    rs.set("anim_speed_multiplier",   anim_speed_multiplier);
    rs.set("freezes_animation",       freezes_animation);
    rs.set("shaking_effect",          shaking_effect);
    rs.set("overlay_animation",       overlay_animation);
    rs.set("overlay_anim_mob_scale",  overlay_anim_mob_scale);
    rs.set("particle_generator",      particle_gen_str, &particle_gen_node);
    rs.set("particle_offset",         particle_offset_str);
    rs.set("replacement_on_timeout",  replacement_on_timeout_str);
    
    affects = 0;
    if(affects_pikmin_bool) {
        enable_flag(affects, STATUS_AFFECTS_FLAG_PIKMIN);
    }
    if(affects_leaders_bool) {
        enable_flag(affects, STATUS_AFFECTS_FLAG_LEADERS);
    }
    if(affects_enemies_bool) {
        enable_flag(affects, STATUS_AFFECTS_FLAG_ENEMIES);
    }
    if(affects_others_bool) {
        enable_flag(affects, STATUS_AFFECTS_FLAG_OTHERS);
    }
    
    if(reapply_rule_node) {
        if(reapply_rule_str == "keep_time") {
            reapply_rule = STATUS_REAPPLY_RULE_KEEP_TIME;
        } else if(reapply_rule_str == "reset_time") {
            reapply_rule = STATUS_REAPPLY_RULE_RESET_TIME;
        } else if(reapply_rule_str == "add_time") {
            reapply_rule = STATUS_REAPPLY_RULE_ADD_TIME;
        } else {
            game.errors.report(
                "Unknown reapply rule \"" +
                reapply_rule_str + "\"!", reapply_rule_node
            );
        }
    }
    
    if(sc_type_node) {
        if(sc_type_str == "flailing") {
            state_change_type = STATUS_STATE_CHANGE_FLAILING;
        } else if(sc_type_str == "helpless") {
            state_change_type = STATUS_STATE_CHANGE_HELPLESS;
        } else if(sc_type_str == "panic") {
            state_change_type = STATUS_STATE_CHANGE_PANIC;
        } else if(sc_type_str == "custom") {
            state_change_type = STATUS_STATE_CHANGE_CUSTOM;
        } else {
            game.errors.report(
                "Unknown state change type \"" +
                sc_type_str + "\"!", sc_type_node
            );
        }
    }
    
    if(particle_gen_node) {
        if(
            game.content.custom_particle_gen.list.find(particle_gen_str) ==
            game.content.custom_particle_gen.list.end()
        ) {
            game.errors.report(
                "Unknown particle generator \"" +
                particle_gen_str + "\"!", particle_gen_node
            );
        } else {
            generates_particles =
                true;
            particle_gen =
                &game.content.custom_particle_gen.list[particle_gen_str];
            particle_offset_pos =
                s2p(particle_offset_str, &particle_offset_z);
        }
    }
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        if(!overlay_animation.empty()) {
            overlay_anim.init_to_first_anim(&game.content.global_anim_dbs.list[overlay_animation]);
        }
    }
}
