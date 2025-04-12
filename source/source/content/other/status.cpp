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

#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_structs.h"
#include "../../util/general_utils.h"


/**
 * @brief Constructs a new status object.
 *
 * @param type Its type.
 */
Status::Status(StatusType* type) :
    type(type) {
    
    timeLeft = type->autoRemoveTime;
}


/**
 * @brief Ticks a status effect instance's time by one frame of logic,
 * but does not tick its effects logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Status::tick(float delta_t) {
    if(type->autoRemoveTime > 0.0f) {
        timeLeft -= delta_t;
        if(timeLeft <= 0.0f) {
            toDelete = true;
        }
    }
}


/**
 * @brief Loads status type data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void StatusType::loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string affects_str;
    string reapply_rule_str;
    string sc_type_str;
    string particle_offset_str;
    string particle_gen_str;
    DataNode* affects_node = nullptr;
    DataNode* reapply_rule_node = nullptr;
    DataNode* sc_type_node = nullptr;
    DataNode* particle_gen_node = nullptr;
    
    rs.set("color",                   color);
    rs.set("tint",                    tint);
    rs.set("glow",                    glow);
    rs.set("affects",                 affects_str);
    rs.set("removable_with_whistle",  removableWithWhistle);
    rs.set("remove_on_hazard_leave",  removeOnHazardLeave);
    rs.set("auto_remove_time",        autoRemoveTime);
    rs.set("reapply_rule",            reapply_rule_str, &reapply_rule_node);
    rs.set("health_change",           healthChange);
    rs.set("health_change_ratio",     healthChangeRatio);
    rs.set("state_change_type",       sc_type_str, &sc_type_node);
    rs.set("state_change_name",       stateChangeName);
    rs.set("animation_change",        animationChange);
    rs.set("speed_multiplier",        speedMultiplier);
    rs.set("attack_multiplier",       attackMultiplier);
    rs.set("defense_multiplier",      defenseMultiplier);
    rs.set("maturity_change_amount",  maturityChangeAmount);
    rs.set("disables_attack",         disablesAttack);
    rs.set("turns_inedible",          turnsInedible);
    rs.set("turns_invisible",         turnsInvisible);
    rs.set("anim_speed_multiplier",   animSpeedMultiplier);
    rs.set("freezes_animation",       freezesAnimation);
    rs.set("shaking_effect",          shakingEffect);
    rs.set("overlay_animation",       overlayAnimation);
    rs.set("overlay_anim_mob_scale",  overlayAnimMobScale);
    rs.set("particle_generator",      particle_gen_str, &particle_gen_node);
    rs.set("particle_offset",         particle_offset_str);
    rs.set("replacement_on_timeout",  replacementOnTimeoutStr);
    
    affects = 0;
    vector<string> affects_str_parts = semicolonListToVector(affects_str);
    for(size_t a = 0; a < affects_str_parts.size(); a++) {
        if(affects_str_parts[a] == "pikmin") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_PIKMIN);
        } else if(affects_str_parts[a] == "leaders") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_LEADERS);
        } else if(affects_str_parts[a] == "enemies") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_ENEMIES);
        } else if(affects_str_parts[a] == "others") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_OTHERS);
        } else {
            game.errors.report(
                "Unknown affect target \"" + affects_str_parts[a] + "\"!",
                affects_node
            );
        }
    }
    
    if(reapply_rule_node) {
        if(reapply_rule_str == "keep_time") {
            reapplyRule = STATUS_REAPPLY_RULE_KEEP_TIME;
        } else if(reapply_rule_str == "reset_time") {
            reapplyRule = STATUS_REAPPLY_RULE_RESET_TIME;
        } else if(reapply_rule_str == "add_time") {
            reapplyRule = STATUS_REAPPLY_RULE_ADD_TIME;
        } else {
            game.errors.report(
                "Unknown reapply rule \"" +
                reapply_rule_str + "\"!", reapply_rule_node
            );
        }
    }
    
    if(sc_type_node) {
        if(sc_type_str == "flailing") {
            stateChangeType = STATUS_STATE_CHANGE_FLAILING;
        } else if(sc_type_str == "helpless") {
            stateChangeType = STATUS_STATE_CHANGE_HELPLESS;
        } else if(sc_type_str == "panic") {
            stateChangeType = STATUS_STATE_CHANGE_PANIC;
        } else if(sc_type_str == "custom") {
            stateChangeType = STATUS_STATE_CHANGE_CUSTOM;
        } else {
            game.errors.report(
                "Unknown state change type \"" +
                sc_type_str + "\"!", sc_type_node
            );
        }
    }
    
    if(particle_gen_node) {
        if(
            game.content.particleGens.list.find(particle_gen_str) ==
            game.content.particleGens.list.end()
        ) {
            game.errors.report(
                "Unknown particle generator \"" +
                particle_gen_str + "\"!", particle_gen_node
            );
        } else {
            generatesParticles =
                true;
            particleGen =
                &game.content.particleGens.list[particle_gen_str];
            particleOffsetPos =
                s2p(particle_offset_str, &particleOffsetZ);
        }
    }
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        if(!overlayAnimation.empty()) {
            overlayAnim.initToFirstAnim(&game.content.globalAnimDbs.list[overlayAnimation]);
        }
    }
}
