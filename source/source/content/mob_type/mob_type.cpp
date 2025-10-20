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
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/bridge.h"
#include "../mob_script/gen_mob_fsm.h"
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
 * @param categoryId The ID of the category it belongs to.
 */
MobType::MobType(MOB_CATEGORY categoryId) :
    category(game.mobCategories.get(categoryId)),
    customCategoryName(category->name) {
    
}


/**
 * @brief Destroys the mob type object.
 */
MobType::~MobType() {
    states.clear();
    for(size_t a = 0; a < initActions.size(); a++) {
        delete initActions[a];
    }
    initActions.clear();
}


/**
 * @brief Adds carrying-related states to the FSM.
 */
void MobType::addCarryingStates() {

    EasyFsmCreator efc;
    
    efc.newState("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(GenMobFsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("carriable_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.changeState("being_delivered");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.changeState("carriable_thrown");
        }
    }
    
    efc.newState("carriable_stuck", ENEMY_EXTRA_STATE_CARRIABLE_STUCK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_thrown", ENEMY_EXTRA_STATE_CARRIABLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(GenMobFsm::loseMomentum);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(GenMobFsm::handleDelivery);
        }
    }
    
    
    vector<MobState*> newStates = efc.finish();
    
    states.insert(states.end(), newStates.begin(), newStates.end());
    
}


/**
 * @brief Specifies what animation conversions there are, if any.
 *
 * @return The animation conversions.
 */
AnimConversionVector MobType::getAnimConversions() const {
    return AnimConversionVector();
}


/**
 * @brief Loads properties from a data file, if any.
 */
void MobType::loadCatProperties(DataNode*) { }


/**
 * @brief Loads any resources into memory, if any.
 */
void MobType::loadCatResources(DataNode*) { }


/**
 * @brief Loads mob type data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 * @param folderPath Path to the folder this mob type is in.
 */
void MobType::loadFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level, const string& folderPath
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter tRS(node);
    
    string customCarrySpotsStr;
    string spikeDamageStr;
    string targetTypeStr;
    string huntableTargetsStr;
    string hurtableTargetsStr;
    string teamStr;
    string inactiveLogicStr;
    DataNode* areaEditorTipsNode = nullptr;
    DataNode* customCarrySpotsNode = nullptr;
    DataNode* spikeDamageNode = nullptr;
    DataNode* targetTypeNode = nullptr;
    DataNode* huntableTargetsNode = nullptr;
    DataNode* hurtableTargetsNode = nullptr;
    DataNode* teamNode = nullptr;
    DataNode* inactiveLogicNode = nullptr;
    
    tRS.set("acceleration", acceleration);
    tRS.set("appears_in_area_editor", appearsInAreaEditor);
    tRS.set(
        "area_editor_recommend_links_from",
        areaEditorRecommendLinksFrom
    );
    tRS.set(
        "area_editor_recommend_links_to",
        areaEditorRecommendLinksTo
    );
    tRS.set("area_editor_tips", areaEditorTips, &areaEditorTipsNode);
    tRS.set("blackout_radius", blackoutRadius);
    tRS.set("can_block_paths", canBlockPaths);
    tRS.set("can_free_move", canFreeMove);
    tRS.set(
        "can_hunt", huntableTargetsStr, &huntableTargetsNode
    );
    tRS.set(
        "can_hurt", hurtableTargetsStr, &hurtableTargetsNode
    );
    tRS.set("can_walk_on_others", canWalkOnOthers);
    tRS.set("casts_shadow", castsShadow);
    tRS.set(
        "custom_carry_spots", customCarrySpotsStr, &customCarrySpotsNode
    );
    tRS.set("custom_category_name", customCategoryName);
    tRS.set("default_vulnerability", defaultVulnerability);
    tRS.set("description", description);
    tRS.set("has_group", hasGroup);
    tRS.set("health_regen", healthRegen);
    tRS.set("height", height);
    tRS.set("inactive_logic", inactiveLogicStr, &inactiveLogicNode);
    tRS.set("itch_damage", itchDamage);
    tRS.set("itch_time", itchTime);
    tRS.set("main_color", mainColor);
    tRS.set("max_carriers", maxCarriers);
    tRS.set("max_health", maxHealth);
    tRS.set("move_speed", moveSpeed);
    tRS.set("pushable", pushable);
    tRS.set("pushes", pushes);
    tRS.set("pushes_softly", pushesSoftly);
    tRS.set("pushes_with_hitboxes", pushesWithHitboxes);
    tRS.set("radius", radius);
    tRS.set("rectangular_dimensions", rectangularDim);
    tRS.set("rotation_speed", rotationSpeed);
    tRS.set("show_health", showHealth);
    tRS.set("spike_damage", spikeDamageStr, &spikeDamageNode);
    tRS.set("target_type", targetTypeStr, &targetTypeNode);
    tRS.set("team", teamStr, &teamNode);
    tRS.set("terrain_radius", terrainRadius);
    tRS.set("territory_radius", territoryRadius);
    tRS.set("use_damage_squash_and_stretch", useDamageSquashAndStretch);
    tRS.set("walkable", walkable);
    tRS.set("weight", weight);
    
    if(areaEditorTipsNode) {
        areaEditorTips = unescapeString(areaEditorTips);
    }
    
    if(!customCarrySpotsStr.empty()) {
        vector<string> points =
            semicolonListToVector(customCarrySpotsStr);
        if(points.size() != maxCarriers) {
            game.errors.report(
                "The number of custom carry spots (" + i2s(points.size()) +
                ") does not match the number of max carriers (" +
                i2s(maxCarriers) + ")!",
                customCarrySpotsNode
            );
        } else {
            for(size_t p = 0; p < points.size(); p++) {
                customCarrySpots.push_back(s2p(points[p]));
            }
        }
    }
    
    rotationSpeed = degToRad(rotationSpeed);
    
    //Vulnerabilities.
    DataNode* vulnerabilitiesNode =
        node->getChildByName("vulnerabilities");
    for(size_t h = 0; h < vulnerabilitiesNode->getNrOfChildren(); h++) {
    
        DataNode* vulnNode = vulnerabilitiesNode->getChild(h);
        auto hazardIt = game.content.hazards.list.find(vulnNode->name);
        vector<string> words = split(vulnNode->value);
        float percentage = defaultVulnerability;
        string statusName;
        bool statusOverrides = false;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            statusName = words[1];
        }
        if(words.size() >= 3) {
            statusOverrides = s2b(words[2]);
        }
        auto statusIt = game.content.statusTypes.list.find(statusName);
        
        if(hazardIt == game.content.hazards.list.end()) {
            game.errors.report(
                "Unknown hazard \"" + vulnNode->name + "\"!",
                vulnNode
            );
            
        } else if(
            !statusName.empty() &&
            statusIt == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + statusName + "\"!",
                vulnNode
            );
            
        } else {
            MobType::Vulnerability& vuln =
                hazardVulnerabilities[&(hazardIt->second)];
            vuln.effectMult = percentage / 100.0f;
            if(!statusName.empty()) {
                vuln.statusToApply = statusIt->second;
            }
            vuln.statusOverrides = statusOverrides;
        }
    }
    
    //Spike damage.
    auto sdIt = game.content.spikeDamageTypes.list.find(spikeDamageStr);
    if(spikeDamageNode) {
        if(sdIt == game.content.spikeDamageTypes.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + spikeDamageStr + "\"!",
                spikeDamageNode
            );
        } else {
            spikeDamage = &(sdIt->second);
        }
    }
    
    //Team.
    if(teamNode) {
        MOB_TEAM t = stringToTeamNr(teamStr);
        if(t != INVALID) {
            startingTeam = t;
        } else {
            game.errors.report(
                "Invalid team \"" + teamStr + "\"!",
                teamNode
            );
        }
    }
    
    //Inactive logic.
    if(inactiveLogicNode) {
        if(inactiveLogicStr == "normal") {
            inactiveLogic =
                0;
        } else if(inactiveLogicStr == "ticks") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_TICKS;
        } else if(inactiveLogicStr == "interactions") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else if(inactiveLogicStr == "all_logic") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else {
            game.errors.report(
                "Invalid inactive logic \"" + inactiveLogicStr + "\"!",
                inactiveLogicNode
            );
        }
    }
    
    //Spike damage vulnerabilities.
    DataNode* spikeDamageVulnNode =
        node->getChildByName("spike_damage_vulnerabilities");
    size_t nSdVuln =
        spikeDamageVulnNode->getNrOfChildren();
    for(size_t v = 0; v < nSdVuln; v++) {
    
        DataNode* vulNode = spikeDamageVulnNode->getChild(v);
        auto sdvIt = game.content.spikeDamageTypes.list.find(vulNode->name);
        vector<string> words = split(vulNode->value);
        float percentage = 1.0f;
        string statusName;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            statusName = words[1];
        }
        auto statusIt = game.content.statusTypes.list.find(statusName);
        
        if(sdvIt == game.content.spikeDamageTypes.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + vulNode->name + "\"!",
                vulNode
            );
            
        } else if(
            !statusName.empty() &&
            statusIt == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + statusName + "\"!",
                vulNode
            );
            
        } else {
            auto& s = spikeDamageVulnerabilities[&(sdvIt->second)];
            s.effectMult = percentage / 100.0f;
            s.statusToApply = statusIt->second;
            
        }
    }
    
    //Status vulnerabilities.
    DataNode* statusVulnNode =
        node->getChildByName("status_vulnerabilities");
    size_t nSVuln =
        statusVulnNode->getNrOfChildren();
    for(size_t v = 0; v < nSVuln; v++) {
    
        DataNode* vulNode = statusVulnNode->getChild(v);
        auto svIt = game.content.statusTypes.list.find(vulNode->name);
        vector<string> words = split(vulNode->value);
        float percentage = 1.0f;
        string statusOverrideName;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            statusOverrideName = words[1];
        }
        auto statusOverrideIt =
            game.content.statusTypes.list.find(statusOverrideName);
            
        if(svIt == game.content.statusTypes.list.end()) {
            game.errors.report(
                "Unknown status type \"" + vulNode->name + "\"!",
                vulNode
            );
            
        } else if(
            !statusOverrideName.empty() &&
            statusOverrideIt == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + statusOverrideName + "\"!",
                vulNode
            );
            
        } else {
            auto& s = statusVulnerabilities[svIt->second];
            s.effectMult = percentage / 100.0f;
            if(statusOverrideIt != game.content.statusTypes.list.end()) {
                s.statusToApply = statusOverrideIt->second;
            }
            s.statusOverrides = true;
        }
        
    }
    
    //Reaches.
    DataNode* reachesNode = node->getChildByName("reaches");
    size_t nReaches = reachesNode->getNrOfChildren();
    for(size_t r = 0; r < nReaches; r++) {
        DataNode* reachNode = reachesNode->getChild(r);
        MobType::Reach newReach;
        newReach.name = reachNode->name;
        vector<string> rStrings = split(reachNode->value);
        
        if(rStrings.size() != 2 && rStrings.size() != 4) {
            game.errors.report(
                "Reach \"" + newReach.name +
                "\" isn't made up of 2 or 4 words!",
                reachNode
            );
            continue;
        }
        
        newReach.radius1 = s2f(rStrings[0]);
        newReach.angle1 = degToRad(s2f(rStrings[1]));
        if(rStrings.size() == 4) {
            newReach.radius2 = s2f(rStrings[2]);
            newReach.angle2 = degToRad(s2f(rStrings[3]));
        }
        reaches.push_back(newReach);
    }
    
    //Spawns.
    DataNode* spawnsNode = node->getChildByName("spawns");
    size_t nSpawns = spawnsNode->getNrOfChildren();
    for(size_t s = 0; s < nSpawns; s++) {
    
        DataNode* spawnNode = spawnsNode->getChild(s);
        ReaderSetter sRS(spawnNode);
        MobType::SpawnInfo newSpawn;
        
        string coordsStr;
        
        newSpawn.name = spawnNode->name;
        sRS.set("object", newSpawn.mobTypeName);
        sRS.set("relative", newSpawn.relative);
        sRS.set("coordinates", coordsStr);
        sRS.set("angle", newSpawn.angle);
        sRS.set("vars", newSpawn.vars);
        sRS.set("link_object_to_spawn", newSpawn.linkObjectToSpawn);
        sRS.set("link_spawn_to_object", newSpawn.linkSpawnToObject);
        sRS.set("momentum", newSpawn.momentum);
        
        if(!coordsStr.empty()) {
            newSpawn.coordsXY = s2p(coordsStr, &newSpawn.coordsZ);
        }
        newSpawn.angle = degToRad(newSpawn.angle);
        
        spawns.push_back(newSpawn);
    }
    
    //Children.
    DataNode* childrenNode = node->getChildByName("children");
    size_t nChildren = childrenNode->getNrOfChildren();
    for(size_t c = 0; c < nChildren; c++) {
    
        DataNode* childNode = childrenNode->getChild(c);
        ReaderSetter cRS(childNode);
        MobType::Child newChild;
        
        string limbDrawMethod;
        string holdRotationMethod;
        DataNode* limbDrawNode = nullptr;
        DataNode* holdRotationNode = nullptr;
        
        newChild.name = childNode->name;
        cRS.set("spawn", newChild.spawnName);
        cRS.set("parent_holds", newChild.parentHolds);
        cRS.set("hold_body_part", newChild.holdBodyPart);
        cRS.set("hold_offset_distance", newChild.holdOffsetDist);
        cRS.set(
            "hold_offset_vertical_distance", newChild.holdOffsetVertDist
        );
        cRS.set("hold_offset_angle", newChild.holdOffsetAngle);
        cRS.set(
            "hold_rotation_method", holdRotationMethod, &holdRotationNode
        );
        cRS.set("handle_damage", newChild.handleDamage);
        cRS.set("relay_damage", newChild.relayDamage);
        cRS.set("handle_events", newChild.handleEvents);
        cRS.set("relay_events", newChild.relayEvents);
        cRS.set("handle_statuses", newChild.handleStatuses);
        cRS.set("relay_statuses", newChild.relayStatuses);
        cRS.set("limb_animation", newChild.limbAnimName);
        cRS.set("limb_thickness", newChild.limbThickness);
        cRS.set("limb_parent_body_part", newChild.limbParentBodyPart);
        cRS.set("limb_parent_offset", newChild.limbParentOffset);
        cRS.set("limb_child_body_part", newChild.limbChildBodyPart);
        cRS.set("limb_child_offset", newChild.limbChildOffset);
        cRS.set("limb_draw_method", limbDrawMethod, &limbDrawNode);
        
        newChild.holdOffsetAngle = degToRad(newChild.holdOffsetAngle);
        
        if(limbDrawNode) {
            if(limbDrawMethod == "below_both") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_BOTH;
            } else if(limbDrawMethod == "below_child") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_CHILD;
            } else if(limbDrawMethod == "below_parent") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_PARENT;
            } else if(limbDrawMethod == "above_parent") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_PARENT;
            } else if(limbDrawMethod == "above_child") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_CHILD;
            } else if(limbDrawMethod == "above_both") {
                newChild.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_BOTH;
            } else {
                game.errors.report(
                    "Unknow limb draw method \"" +
                    limbDrawMethod + "\"!", limbDrawNode
                );
            }
        }
        
        if(holdRotationNode) {
            if(holdRotationMethod == "never") {
                newChild.holdRotationMethod =
                    HOLD_ROTATION_METHOD_NEVER;
            } else if(holdRotationMethod == "face_parent") {
                newChild.holdRotationMethod =
                    HOLD_ROTATION_METHOD_FACE_HOLDER;
            } else if(holdRotationMethod == "copy_parent") {
                newChild.holdRotationMethod =
                    HOLD_ROTATION_METHOD_COPY_HOLDER;
            } else {
                game.errors.report(
                    "Unknow parent holding rotation method \"" +
                    holdRotationMethod + "\"!", holdRotationNode
                );
            }
        }
        
        children.push_back(newChild);
    }
    
    //Sounds.
    DataNode* soundsNode = node->getChildByName("sounds");
    size_t nSounds = soundsNode->getNrOfChildren();
    for(size_t s = 0; s < nSounds; s++) {
    
        DataNode* soundNode = soundsNode->getChild(s);
        ReaderSetter sRS(soundNode);
        MobType::Sound newSound;
        
        string soundINameStr;
        DataNode* soundINameNode = nullptr;
        string typeStr;
        DataNode* typeNode = nullptr;
        string stackModeStr;
        DataNode* stackModeNode = nullptr;
        float volumeFloat = 100.0f;
        float speedFloat = 100.0f;
        bool loopBool = false;
        
        newSound.name = soundNode->name;
        
        sRS.set("sound", soundINameStr, &soundINameNode);
        sRS.set("type", typeStr, &typeNode);
        sRS.set("stack_mode", stackModeStr, &stackModeNode);
        sRS.set("stack_min_pos", newSound.config.stackMinPos);
        sRS.set("loop", loopBool);
        sRS.set("volume", volumeFloat);
        sRS.set("speed", speedFloat);
        sRS.set("volume_deviation", newSound.config.volumeDeviation);
        sRS.set("speed_deviation", newSound.config.speedDeviation);
        sRS.set("random_chance", newSound.config.randomChance);
        sRS.set("random_delay", newSound.config.randomDelay);
        
        newSound.sample =
            game.content.sounds.list.get(soundINameStr, soundINameNode);
            
        if(typeNode) {
            if(typeStr == "gameplay_global") {
                newSound.type = SOUND_TYPE_GAMEPLAY_GLOBAL;
            } else if(typeStr == "gameplay_pos") {
                newSound.type = SOUND_TYPE_GAMEPLAY_POS;
            } else if(typeStr == "ambiance_global") {
                newSound.type = SOUND_TYPE_AMBIANCE_GLOBAL;
            } else if(typeStr == "ambiance_pos") {
                newSound.type = SOUND_TYPE_AMBIANCE_POS;
            } else if(typeStr == "ui") {
                newSound.type = SOUND_TYPE_UI;
            } else {
                game.errors.report(
                    "Unknow sound effect type \"" +
                    typeStr + "\"!", typeNode
                );
            }
        }
        
        if(stackModeNode) {
            if(stackModeStr == "normal") {
                newSound.config.stackMode = SOUND_STACK_MODE_NORMAL;
            } else if(stackModeStr == "override") {
                newSound.config.stackMode = SOUND_STACK_MODE_OVERRIDE;
            } else if(stackModeStr == "never") {
                newSound.config.stackMode = SOUND_STACK_MODE_NEVER;
            } else {
                game.errors.report(
                    "Unknow sound effect stack mode \"" +
                    stackModeStr + "\"!", stackModeNode
                );
            }
        }
        
        if(loopBool) {
            enableFlag(newSound.config.flags, SOUND_FLAG_LOOP);
        }
        
        newSound.config.volume = volumeFloat / 100.0f;
        newSound.config.volume = std::clamp(newSound.config.volume, 0.0f, 1.0f);
        
        newSound.config.speed = speedFloat / 100.0f;
        newSound.config.speed = std::max(0.0f, newSound.config.speed);
        
        newSound.config.volumeDeviation /= 100.0f;
        newSound.config.speedDeviation /= 100.0f;
        
        sounds.push_back(newSound);
    }
    
    //Area editor properties.
    DataNode* aePropsNode =
        node->getChildByName("area_editor_properties");
    size_t nAeProps = aePropsNode->getNrOfChildren();
    for(size_t p = 0; p < nAeProps; p++) {
    
        DataNode* propNode = aePropsNode->getChild(p);
        ReaderSetter pRS(propNode);
        MobType::AreaEditorProp newProp;
        
        string typeStr;
        string listStr;
        DataNode* typeNode = nullptr;
        
        newProp.name = propNode->name;
        
        pRS.set("var", newProp.var);
        pRS.set("type", typeStr, &typeNode);
        pRS.set("def_value", newProp.defValue);
        pRS.set("min_value", newProp.minValue);
        pRS.set("max_value", newProp.maxValue);
        pRS.set("list", listStr);
        pRS.set("tooltip", newProp.tooltip);
        
        if(newProp.var.empty()) {
            game.errors.report(
                "You need to specify the area editor property's name!",
                propNode
            );
        }
        
        if(typeStr == "text") {
            newProp.type = AEMP_TYPE_TEXT;
        } else if(typeStr == "int") {
            newProp.type = AEMP_TYPE_INT;
        } else if(typeStr == "float") {
            newProp.type = AEMP_TYPE_FLOAT;
        } else if(typeStr == "bool") {
            newProp.type = AEMP_TYPE_BOOL;
        } else if(typeStr == "list") {
            newProp.type = AEMP_TYPE_LIST;
        } else if(typeStr == "number_list") {
            newProp.type = AEMP_TYPE_NR_LIST;
        } else {
            game.errors.report(
                "Unknown area editor property type \"" + typeStr + "\"!",
                typeNode
            );
        }
        
        if(newProp.minValue > newProp.maxValue) {
            std::swap(newProp.minValue, newProp.maxValue);
        }
        
        if(
            newProp.type == AEMP_TYPE_LIST ||
            newProp.type == AEMP_TYPE_NR_LIST
        ) {
            if(listStr.empty()) {
                game.errors.report(
                    "For this area editor property type, you need to specify "
                    "a list of values!", propNode
                );
            } else {
                newProp.valueList = semicolonListToVector(listStr);
            }
        }
        
        newProp.tooltip = unescapeString(newProp.tooltip);
        
        areaEditorProps.push_back(newProp);
    }
    
    if(targetTypeNode) {
        MOB_TARGET_FLAG targetTypeValue =
            stringToMobTargetType(targetTypeStr);
        if(targetTypeValue == INVALID) {
            game.errors.report(
                "Unknown target type \"" + targetTypeStr + "\"!",
                targetTypeNode
            );
        } else {
            targetType = targetTypeValue;
        }
    }
    
    vector<string> huntableTargetsStrs =
        semicolonListToVector(huntableTargetsStr);
    if(huntableTargetsNode) {
        huntableTargets = 0;
    }
    for(size_t h = 0; h < huntableTargetsStrs.size(); h++) {
        size_t v = stringToMobTargetType(huntableTargetsStrs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + huntableTargetsStrs[h] + "\"!",
                huntableTargetsNode
            );
        } else {
            huntableTargets |= (Bitmask16) v;
        }
    }
    
    vector<string> hurtableTargetsStrs =
        semicolonListToVector(hurtableTargetsStr);
    if(hurtableTargetsNode) {
        hurtableTargets = 0;
    }
    for(size_t h = 0; h < hurtableTargetsStrs.size(); h++) {
        size_t v = stringToMobTargetType(hurtableTargetsStrs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + hurtableTargetsStrs[h] + "\"!",
                hurtableTargetsNode
            );
        } else {
            hurtableTargets |= (Bitmask16) v;
        }
    }
    
    //Resources.
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        animDb =
            &game.content.mobAnimDbs.list[category->id][manifest->internalName];
        animDb->fillSoundIdxCaches(this);
        
        DataNode scriptFile;
        scriptFile.loadFile(folderPath + "/script.txt", true, true);
        size_t oldNStates = states.size();
        
        DataNode* deathStateNameNode =
            scriptFile.getChildByName("death_state");
        dyingStateName = deathStateNameNode->value;
        
        statesIgnoringDeath =
            semicolonListToVector(
                scriptFile.getChildByName("states_ignoring_death")->value
            );
            
        statesIgnoringSpray =
            semicolonListToVector(
                scriptFile.getChildByName("states_ignoring_spray")->value
            );
            
        statesIgnoringHazard =
            semicolonListToVector(
                scriptFile.getChildByName("states_ignoring_hazard")->value
            );
            
        //Load init actions.
        loadActions(
            this,
            scriptFile.getChildByName("init"), &initActions, nullptr
        );
        //Load the rest of the script.
        loadScript(
            this,
            scriptFile.getChildByName("script"),
            scriptFile.getChildByName("global"),
            &states
        );
        
        if(states.size() > oldNStates) {
        
            DataNode* firstStateNameNode =
                scriptFile.getChildByName("first_state");
            string firstStateName = firstStateNameNode->value;
            
            for(size_t s = 0; s < states.size(); s++) {
                if(states[s]->name == firstStateName) {
                    firstStateIdx = s;
                    break;
                }
            }
            if(firstStateIdx == INVALID) {
                game.errors.report(
                    "Unknown state \"" + firstStateName + "\" "
                    "to set as the first state!",
                    firstStateNameNode
                );
            }
            
            if(!dyingStateName.empty()) {
                for(size_t s = 0; s < states.size(); s++) {
                    if(states[s]->name == dyingStateName) {
                        dyingStateIdx = s;
                        break;
                    }
                }
                if(dyingStateIdx == INVALID) {
                    game.errors.report(
                        "Unknown state \"" + dyingStateName + "\" "
                        "to set as the death state!",
                        deathStateNameNode
                    );
                }
            }
            
            if(category->id == MOB_CATEGORY_ENEMIES) {
                DataNode* reviveStateNameNode =
                    scriptFile.getChildByName("revive_state");
                string reviveStateName = reviveStateNameNode->value;
                
                if(!reviveStateName.empty()) {
                    for(size_t s = 0; s < states.size(); s++) {
                        if(states[s]->name == reviveStateName) {
                            reviveStateIdx = s;
                            break;
                        }
                    }
                    if(reviveStateIdx == INVALID) {
                        game.errors.report(
                            "Unknown state \"" + reviveStateName + "\" "
                            "to set as the revive state!",
                            reviveStateNameNode
                        );
                    }
                } else {
                    reviveStateIdx = firstStateIdx;
                }
            }
        }
    }
    
    //Category-specific properties.
    loadCatProperties(node);
    
    //Category-specific resources.
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        loadCatResources(node);
        animDb->createConversions(getAnimConversions(), node);
    }
    
    physicalSpan =
        calculateMobPhysicalSpan(
            radius,
            (level >= CONTENT_LOAD_LEVEL_FULL ? animDb->hitboxSpan : 0),
            rectangularDim
        );
        
    if(customCategoryName.empty()) {
        customCategoryName = category->name;
    }
}


/**
 * @brief Unloads loaded resources from memory.
 */
void MobType::unloadResources() {
    for(size_t s = 0; s < sounds.size(); s++) {
        game.content.sounds.list.free(sounds[s].name);
    }
}


/**
 * @brief Grabs an animation conversion vector, filled with base animations,
 * and outputs one that combines all base animations with their groups.
 *
 * @param v The animation conversion vector.
 * @param baseAnimTotal How many base animations exist.
 * @return The vector.
 */
AnimConversionVector
MobTypeWithAnimGroups::getAnimConversionsWithGroups(
    const AnimConversionVector& v, size_t baseAnimTotal
) const {
    AnimConversionVector newV;
    
    for(size_t g = 0; g < animationGroupSuffixes.size(); g++) {
        for(size_t c = 0; c < v.size(); c++) {
            newV.push_back(
                make_pair(
                    g * baseAnimTotal + v[c].first,
                    v[c].second + animationGroupSuffixes[g]
                )
            );
        }
    }
    
    return newV;
}


/**
 * @brief Creates special mob types, needed by the engine,
 * that are beyond the ones loaded from the game data folder.
 */
void createSpecialMobTypes() {
    MobCategory* customCategory =
        game.mobCategories.get(MOB_CATEGORY_CUSTOM);
        
    MobType* bridgeComponentType = customCategory->createType();
    bridgeComponentType->name = "Bridge component";
    bridgeComponentType->blackoutRadius = 0;
    bridgeComponentType->appearsInAreaEditor = false;
    bridgeComponentType->castsShadow = false;
    bridgeComponentType->customCategoryName = "Misc";
    bridgeComponentType->height = 8.0f;
    bridgeComponentType->physicalSpan = 8.0f;
    bridgeComponentType->radius = 8.0f;
    bridgeComponentType->walkable = true;
    bridgeComponentType->drawMobCallback = Bridge::drawComponent;
    bridgeComponentType->pushes = true;
    bridgeComponentType->pushesSoftly = false;
    customCategory->registerType("bridge_component", bridgeComponentType);
}
