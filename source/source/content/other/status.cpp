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
#include "../../core/misc_functions.h"
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
 * @brief Applies one of the status's particle generators to a mob.
 *
 * @param m The mob.
 * @param pg The particle generator.
 */
void Status::applyParticles(Mob* m, ParticleGenerator* pg) {
    ParticleGenerator newPg = *pg;
    newPg.restartTimer();
    newPg.followMob = m;
    newPg.followAngle = &m->angle;
    newPg.followPosOffset = type->particleOffsetPos;
    newPg.followZOffset = type->particleOffsetZ;
    if(type->particleScaleReaches) {
        newPg.emission.circleInnerDist =
            (newPg.emission.circleInnerDist / 100.0f) * m->radius;
        newPg.emission.circleOuterDist =
            (newPg.emission.circleOuterDist / 100.0f) * m->radius;
        newPg.emission.rectInnerDist =
            (newPg.emission.rectInnerDist / 100.0f) * m->radius;
        newPg.emission.rectOuterDist =
            (newPg.emission.rectOuterDist / 100.0f) * m->radius;
    }
    if(type->particleScaleSizes) {
        adjustKeyframeInterpolatorValues<float>(
            newPg.baseParticle.size,
        [ = ] (const float & s) {
            return (s / 100.0f) * m->radius;
        }
        );
        newPg.sizeDeviation = (newPg.sizeDeviation / 100.0f) * m->radius;
    }
    m->particleGenerators.push_back(newPg);
}


/**
 * @brief Ticks a status effect instance's time by one frame of logic,
 * but does not tick its effects logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Status::tick(float deltaT) {
    if(type->autoRemoveTime > 0.0f && state == STATUS_STATE_ACTIVE) {
        timeLeft -= deltaT;
        if(timeLeft <= 0.0f) {
            state = STATUS_STATE_TO_DELETE;
        }
    }
    if(
        type->buildup != 0.0f && type->buildupRemovalDuration != 0.0f &&
        buildup < 1.0f
    ) {
        buildupRemovalTimeLeft -= deltaT;
        if(buildupRemovalTimeLeft <= 0.0f) {
            state = STATUS_STATE_TO_DELETE;
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
    ReaderSetter sRS(node);
    
    string affectsStr;
    string reapplyRuleStr;
    string scTypeStr;
    string particleOffsetStr;
    string pgStr;
    string pgStartStr;
    string pgEndStr;
    DataNode* affectsNode = nullptr;
    DataNode* reapplyRuleNode = nullptr;
    DataNode* scTypeNode = nullptr;
    DataNode* pgNode = nullptr;
    DataNode* pgStartNode = nullptr;
    DataNode* pgEndNode = nullptr;
    DataNode* buildupNode = nullptr;
    
    sRS.set("color",                    color);
    sRS.set("tint",                     tint);
    sRS.set("colorize",                 colorize);
    sRS.set("affects",                  affectsStr);
    sRS.set("removable_with_whistle",   removableWithWhistle);
    sRS.set("remove_on_hazard_leave",   removeOnHazardLeave);
    sRS.set("auto_remove_time",         autoRemoveTime);
    sRS.set("reapply_rule",             reapplyRuleStr, &reapplyRuleNode);
    sRS.set("health_change",            healthChange);
    sRS.set("health_change_ratio",      healthChangeRatio);
    sRS.set("state_change_type",        scTypeStr, &scTypeNode);
    sRS.set("state_change_name",        stateChangeName);
    sRS.set("animation_change",         animationChange);
    sRS.set("speed_multiplier",         speedMultiplier);
    sRS.set("attack_multiplier",        attackMultiplier);
    sRS.set("defense_multiplier",       defenseMultiplier);
    sRS.set("maturity_change_amount",   maturityChangeAmount);
    sRS.set("disables_attack",          disablesAttack);
    sRS.set("turns_inedible",           turnsInedible);
    sRS.set("turns_invisible",          turnsInvisible);
    sRS.set("anim_speed_multiplier",    animSpeedMultiplier);
    sRS.set("freezes_animation",        freezesAnimation);
    sRS.set("shaking_effect",           shakingEffect);
    sRS.set("shaking_effect_on_end",    shakingEffectOnEnd);
    sRS.set("overlay_animation",        overlayAnimation);
    sRS.set("overlay_anim_mob_scale",   overlayAnimMobScale);
    sRS.set("particle_generator",       pgStr, &pgNode);
    sRS.set("particle_generator_start", pgStartStr, &pgStartNode);
    sRS.set("particle_generator_end",   pgEndStr, &pgEndNode);
    sRS.set("particle_offset",          particleOffsetStr);
    sRS.set("particle_scale_reaches",   particleScaleReaches);
    sRS.set("particle_scale_sizes",     particleScaleSizes);
    sRS.set("replacement_on_timeout",   replacementOnTimeoutStr);
    sRS.set("buildup",                  buildup, &buildupNode);
    sRS.set("buildup_removal_duration", buildupRemovalDuration);
    
    affects = 0;
    vector<string> affectsStrParts = semicolonListToVector(affectsStr);
    for(size_t a = 0; a < affectsStrParts.size(); a++) {
        if(affectsStrParts[a] == "pikmin") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_PIKMIN);
        } else if(affectsStrParts[a] == "leaders") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_LEADERS);
        } else if(affectsStrParts[a] == "enemies") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_ENEMIES);
        } else if(affectsStrParts[a] == "others") {
            enableFlag(affects, STATUS_AFFECTS_FLAG_OTHERS);
        } else {
            game.errors.report(
                "Unknown affect target \"" + affectsStrParts[a] + "\"!",
                affectsNode
            );
        }
    }
    
    if(reapplyRuleNode) {
        if(reapplyRuleStr == "keep_time") {
            reapplyRule = STATUS_REAPPLY_RULE_KEEP_TIME;
        } else if(reapplyRuleStr == "reset_time") {
            reapplyRule = STATUS_REAPPLY_RULE_RESET_TIME;
        } else if(reapplyRuleStr == "add_time") {
            reapplyRule = STATUS_REAPPLY_RULE_ADD_TIME;
        } else {
            game.errors.report(
                "Unknown reapply rule \"" +
                reapplyRuleStr + "\"!", reapplyRuleNode
            );
        }
    }
    
    if(scTypeNode) {
        if(scTypeStr == "flailing") {
            stateChangeType = STATUS_STATE_CHANGE_FLAILING;
        } else if(scTypeStr == "helpless") {
            stateChangeType = STATUS_STATE_CHANGE_HELPLESS;
        } else if(scTypeStr == "panic") {
            stateChangeType = STATUS_STATE_CHANGE_PANIC;
        } else if(scTypeStr == "custom") {
            stateChangeType = STATUS_STATE_CHANGE_CUSTOM;
        } else {
            game.errors.report(
                "Unknown state change type \"" +
                scTypeStr + "\"!", scTypeNode
            );
        }
    }
    
    const auto loadPg =
    [this] (DataNode * node, const string& str, ParticleGenerator** pg) {
        if(node) {
            if(!isInMap(game.content.particleGens.list, str)) {
                game.errors.report(
                    "Unknown particle generator \"" +
                    str + "\"!", node
                );
            } else {
                *pg = &game.content.particleGens.list[str];
                
            }
        }
    };
    
    loadPg(pgNode, pgStr, &particleGen);
    loadPg(pgStartNode, pgStartStr, &particleGenStart);
    loadPg(pgEndNode, pgEndStr, &particleGenEnd);
    
    particleOffsetPos =
        s2p(particleOffsetStr, &particleOffsetZ);
        
    if(buildupNode) {
        buildup /= 100.0f;
    }
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        if(!overlayAnimation.empty()) {
            overlayAnim.initToFirstAnim(
                &game.content.globalAnimDbs.list[overlayAnimation]
            );
        }
    }
    
    if(node->getNrOfChildrenByName("sound_start") > 0) {
        soundStart.loadFromDataNode(node->getChildByName("sound_start"));
    }
    
    if(node->getNrOfChildrenByName("sound_end") > 0) {
        soundEnd.loadFromDataNode(node->getChildByName("sound_end"));
    }
}
