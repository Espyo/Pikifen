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

#include "status.hpp"

#include "../../core/game.hpp"
#include "../../core/load.hpp"
#include "../../core/misc_functions.hpp"
#include "../../core/misc_structs.hpp"
#include "../../util/general_utils.hpp"


#pragma region Status


/**
 * @brief Constructs a new status object.
 *
 * @param type Its type.
 */
Status::Status(StatusType* type) :
    type(type) {
    
    timeLeft = type->autoRemoveTime;
}


#pragma endregion
#pragma region Status buildup


/**
 * @brief Constructs a new status buildup object.
 *
 * @param type The status type.
 */
StatusBuildup::StatusBuildup(StatusType* type) :
    type(type) {
    
}


#pragma endregion
#pragma region Status cooldown


/**
 * @brief Constructs a new status cooldown object.
 *
 * @param type The status type.
 */
StatusCooldown::StatusCooldown(StatusType* type) :
    type(type),
    timeLeft(type->cooldown) {
    
}


#pragma endregion
#pragma region Status manager


/**
 * @brief Constructs a new status manager object.
 *
 * @param mob The mob it belongs to.
 */
StatusManager::StatusManager(Mob* mob) :
    mob(mob) {
    
}


/**
 * @brief Activates a status, given its type.
 * This adds it to the activated statuses list.
 *
 * @param statusType Its type.
 * @return The new status's object.
 */
Status* StatusManager::activate(StatusType* statusType) {
    Status newStatus(statusType);
    statuses.push_back(newStatus);
    return &statuses.back();
}


/**
 * @brief Applies buildup logic for a status effect.
 *
 * @param statusType Status effect type to use.
 * @param overrideAmount If not FLT_MAX, override the buildup amount by this.
 * @return Whether the buildup reached the necessary amount to
 * activate the actual status effect.
 */
bool StatusManager::applyBuildup(StatusType* statusType, float overrideAmount) {
    //Add it to the buildup list if it's not already there.
    auto buildupIt =
        std::find_if(
            buildups.begin(), buildups.end(),
    [statusType] (const StatusBuildup & s) {
        return s.type == statusType;
    }
        );
        
    if(buildupIt == buildups.end()) {
        StatusBuildup newBuildup(statusType);
        buildups.push_back(newBuildup);
        buildupIt = buildups.end() - 1;
    }
    
    //If it's already at the maximum buildup, nothing needs to be done.
    if(buildupIt->amount >= 1.0f) return true;
    
    //Apply the buildup amount.
    buildupIt->amount +=
        overrideAmount == FLT_MAX ? statusType->buildup : overrideAmount;
    buildupIt->removalTimeLeft = statusType->buildupRemovalDuration;
    
    //If the new amount tipped to the maximum buildup, the status
    //can be activated.
    if(buildupIt->amount >= 1.0f) {
        buildupIt->amount = 1.0f;
        return true;
    }
    
    //Otherwise, this buildup isn't enough to activate the status.
    return false;
}


/**
 * @brief Clears the lists of buildups and cooldowns.
 */
void StatusManager::clearBuildupsAndCooldowns() {
    buildups.clear();
    cooldowns.clear();
}


/**
 * @brief Deactivates a status effect given its type, if it is applied.
 *
 * @param statusType Its type.
 */
void StatusManager::deactivate(StatusType* statusType) {
    deactivateIf(
    [statusType] (Status * sPtr) {
        return sPtr->type == statusType;
    }
    );
}


/**
 * @brief Deactivates the given status effect.
 * This updates the list of active status effects, of buildups,
 * and of cooldowns.
 *
 * @param idx The status effect's index in the list of active status effects.
 */
void StatusManager::deactivate(size_t idx) {
    Status* sPtr = &statuses[idx];
    
    //Remove it from the buildup list, if needed.
    if(sPtr->type->buildup > 0.0f) {
        for(size_t b = 0; b < buildups.size(); ) {
            if(buildups[b].type == sPtr->type) {
                eraseInVector(buildups, b);
            } else {
                b++;
            }
        }
    }
    
    //Add it to the cooldown list, if needed.
    if(sPtr->type->cooldown > 0.0f) {
        StatusCooldown newCooldown(sPtr->type);
        cooldowns.push_back(newCooldown);
    }
    
    //Add it to the deactivated list.
    deactivated.push_back(*sPtr);
    
    //Remove it from the activated list.
    eraseInVector(statuses, idx);
}


/**
 * @brief Deactivates all status effects.
 */
void StatusManager::deactivateAll() {
    deactivateIf(
    [] (Status * sPtr) {
        return true;
    }
    );
}


/**
 * @brief Deactivates any status effects that match the given condition.
 *
 * @param condition The condition. The first parameter is a pointer to the
 * status effect.
 */
void StatusManager::deactivateIf(
    const std::function<bool(Status*)>& condition
) {
    for(size_t s = 0; s < statuses.size(); ) {
        Status* sPtr = &statuses[s];
        if(condition(sPtr)) {
            deactivate(s);
        } else {
            s++;
        }
    }
}


/**
 * @brief Deletes any pending deactivated statuses.
 */
void StatusManager::deleteDeactivated() {
    deactivated.clear();
}


/**
 * @brief Returns a ready-only reference to the list of buildups.
 *
 * @return The list.
 */
const vector<StatusBuildup>& StatusManager::getBuildups() const {
    return buildups;
}


/**
 * @brief Returns a ready-only reference to the list of cooldowns.
 *
 * @return The list.
 */
const vector<StatusCooldown>& StatusManager::getCooldowns() const {
    return cooldowns;
}


/**
 * @brief Returns a ready-only reference to the list of statuses
 * that were deactivated this frame.
 *
 * @return The list.
 */
const vector<Status>& StatusManager::getDeactivated() const {
    return deactivated;
}


/**
 * @brief Returns a ready-only reference to the list of currently
 * active statuses.
 *
 * @return The list.
 */
const vector<Status>& StatusManager::getList() const {
    return statuses;
}


/**
 * @brief Handles the mob having left a hazard.
 */
void StatusManager::handleHazardLeave() {
    deactivateIf(
    [] (Status * sPtr) {
        if(sPtr->type->removeOnHazardLeave) {
            return true;
        }
        return false;
    }
    );
}


/**
 * @brief Handles the mob being knocked down.
 */
void StatusManager::handleKnockdown() {
    deactivateIf(
    [] (Status * sPtr) {
        return sPtr->type->removeOnKnockDown;
    }
    );
}


/**
 * @brief Handles the mob having come upon a source that is trying to
 * inflict a status effect.
 *
 * @param statusType Status effect type to use.
 * @param overrideBuildup If not FLT_MAX, override the status buildup amount
 * with this.
 * @param forceReapplyResetTime If true, forces the reapply rule to
 * be reset time.
 * @return If the status got activated, this returns its data.
 * Otherwise, returns nullptr.
 */
Status* StatusManager::handleStatusSource(
    StatusType* statusType, float overrideBuildup, bool forceReapplyResetTime
) {
    //Check if this status is blocked from being received, due to it
    //being in cooldown.
    forIdx(s, cooldowns) {
        StatusCooldown* cooldownPtr = &cooldowns[s];
        if(cooldownPtr->type == statusType) {
            return nullptr;
        }
    }
    
    //Check if we're instead just applying some buildup.
    if(statusType->buildup > 0.0f) {
        if(!applyBuildup(statusType, overrideBuildup)) {
            return nullptr;
        }
    }
    
    //Check if it's already active.
    //If so, just do something to the time left and then quit out.
    Status* existingStatusPtr = nullptr;
    forIdx(ms, statuses) {
        if(statuses[ms].type == statusType) {
            existingStatusPtr = &statuses[ms];
            break;
        }
    }
    
    if(existingStatusPtr) {
        STATUS_REAPPLY_RULE reapplyRule = statusType->reapplyRule;
        if(forceReapplyResetTime) {
            reapplyRule = STATUS_REAPPLY_RULE_RESET_TIME;
        }
        
        switch(reapplyRule) {
        case STATUS_REAPPLY_RULE_KEEP_TIME: {
            break;
        }
        case STATUS_REAPPLY_RULE_RESET_TIME: {
            existingStatusPtr->timeLeft = statusType->autoRemoveTime;
            break;
        }
        case STATUS_REAPPLY_RULE_ADD_TIME: {
            existingStatusPtr->timeLeft += statusType->autoRemoveTime;
            break;
        }
        }
        
        return nullptr;
    }
    
    //This status is not already active. Let's activate it.
    return activate(statusType);
}


/**
 * @brief Handles the leader having woken up.
 */
void StatusManager::handleWakingUp() {
    deactivateIf(
    [this] (Status * sPtr) {
        Leader* leaPtr = (Leader*) mob;
        if(leaPtr->leaType->sleepingStatus) {
            if(sPtr->type == leaPtr->leaType->sleepingStatus) {
                return true;
            }
        }
        return false;
    }
    );
}


/**
 * @brief Handles the mob being whistled.
 *
 * @return Whether the mob got saved from a deadly status by this whistle.
 */
bool StatusManager::handleWhistle() {
    bool savedByWhistle = false;
    deactivateIf(
    [&savedByWhistle] (Status * sPtr) {
        if(sPtr->type->removeOnWhistle) {
            if(
                sPtr->type->healthChange < 0.0f ||
                sPtr->type->healthChangeRatio < 0.0f
            ) {
                savedByWhistle = true;
            }
            return true;
        }
        return false;
    }
    );
    
    return savedByWhistle;
}


/**
 * @brief Returns whether there are any buildups in progress.
 *
 * @return Whether there are buildups.
 */
bool StatusManager::hasBuildups() const {
    return !buildups.empty();
}


/**
 * @brief Returns whether there are any cooldowns in effect.
 *
 * @return Whether there are cooldowns.
 */
bool StatusManager::hasCooldowns() const {
    return !cooldowns.empty();
}


/**
 * @brief Ticks status effect logic by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void StatusManager::tick(float deltaT) {
    //Tick auto-remove timers.
    forIdx(s, statuses) {
        Status* sPtr = &statuses[s];
        if(sPtr->type->autoRemoveTime > 0.0f) {
            sPtr->timeLeft -= deltaT;
        }
    }
    
    //Delete statuses whose auto-remove timer is up.
    deactivateIf(
    [] (Status * sPtr) {
        if(
            sPtr->type->autoRemoveTime > 0.0f &&
            sPtr->timeLeft <= 0.0f
        ) {
            return true;
        }
        return false;
    }
    );
    
    //Tick buildups.
    for(size_t s = 0; s < buildups.size(); ) {
        StatusBuildup* buildupPtr = &buildups[s];
        bool toDelete = false;
        
        if(
            buildupPtr->type->buildup != 0.0f &&
            buildupPtr->type->buildupRemovalDuration != 0.0f &&
            buildupPtr->amount < 1.0f
        ) {
            buildupPtr->removalTimeLeft -= deltaT;
            if(buildupPtr->removalTimeLeft <= 0.0f) {
                toDelete = true;
            }
        }
        
        if(toDelete) {
            eraseInVector(buildups, s);
        } else {
            s++;
        }
    }
    
    //Tick cooldowns.
    for(size_t s = 0; s < cooldowns.size(); ) {
        StatusCooldown* cooldownPtr = &cooldowns[s];
        bool toDelete = false;
        
        cooldownPtr->timeLeft -= deltaT;
        if(cooldownPtr->timeLeft <= 0.0f) {
            toDelete = true;
        }
        
        if(toDelete) {
            eraseInVector(cooldowns, s);
        } else {
            s++;
        }
    }
}


#pragma endregion
#pragma region Status type


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
    string topReplacementStr;
    DataNode* affectsNode = nullptr;
    DataNode* reapplyRuleNode = nullptr;
    DataNode* scTypeNode = nullptr;
    DataNode* pgNode = nullptr;
    DataNode* pgStartNode = nullptr;
    DataNode* pgEndNode = nullptr;
    DataNode* buildupNode = nullptr;
    DataNode* topReplacementNode = nullptr;
    
    //DEPRECATED in 1.2.0 by "remove_on_whistle".
    deprecatedReaderSetter(
        sRS, "removable_with_whistle", "use \"remove_on_whistle\"",
        removeOnWhistle
    );
    
    sRS.set("color",                    color);
    sRS.set("tint",                     tint);
    sRS.set("colorize",                 colorize);
    sRS.set("affects",                  affectsStr);
    sRS.set("remove_on_hazard_leave",   removeOnHazardLeave);
    sRS.set("remove_on_knock_down",     removeOnKnockDown);
    sRS.set("remove_on_whistle",        removeOnWhistle);
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
    sRS.set("causes_betrayal",          causesBetrayal);
    sRS.set("disables_attack",          disablesAttack);
    sRS.set("turns_inedible",           turnsInedible);
    sRS.set("turns_invisible",          turnsInvisible);
    sRS.set("anim_speed_multiplier",    animSpeedMultiplier);
    sRS.set("freezes_animation",        freezesAnimation);
    sRS.set("shaking_effect",           shakingEffect);
    sRS.set("shaking_effect_on_end",    shakingEffectOnEnd);
    sRS.set("overlay_animation",        overlayAnimation);
    sRS.set("overlay_anim_mob_scale",   overlayAnimMobScale);
    sRS.set("top_replacement",          topReplacementStr, &topReplacementNode);
    sRS.set("particle_generator",       pgStr, &pgNode);
    sRS.set("particle_generator_start", pgStartStr, &pgStartNode);
    sRS.set("particle_generator_end",   pgEndStr, &pgEndNode);
    sRS.set("particle_offset",          particleOffsetStr);
    sRS.set("particle_scale_reaches",   particleScaleReaches);
    sRS.set("particle_scale_sizes",     particleScaleSizes);
    sRS.set("replacement_on_timeout",   replacementOnTimeoutStr);
    sRS.set("buildup",                  buildup, &buildupNode);
    sRS.set("buildup_removal_duration", buildupRemovalDuration);
    sRS.set("cooldown",                 cooldown);
    
    affects = 0;
    vector<string> affectsStrParts = semicolonListToVector(affectsStr);
    forIdx(a, affectsStrParts) {
        STATUS_AFFECTS_FLAG af;
        if(
            readEnumProp(
                statusAffectsFlagINames, affectsStrParts[a], &af,
                "affect target", affectsNode
            )
        ) {
            affects |= (Bitmask8) af;
        }
    }
    
    if(reapplyRuleNode) {
        readEnumProp(
            statusReapplyRuleINames, reapplyRuleStr, &reapplyRule,
            "reapply rule", reapplyRuleNode
        );
    }
    
    if(scTypeNode) {
        readEnumProp(
            statusStateChangeINames, scTypeStr, &stateChangeType,
            "state change type", scTypeNode
        );
    }
    
    const auto loadPg =
    [this] (DataNode * node, const string& str, ParticleGenerator** pg) {
        if(node) {
            if(!isInMap(game.content.particleGens.list, str)) {
                game.log.report(
                    LOG_LEVEL_ERROR,
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
        
        if(topReplacementNode) {
            topReplacementBmp =
                game.content.bitmaps.list.get(
                    topReplacementStr, topReplacementNode
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


#pragma endregion
