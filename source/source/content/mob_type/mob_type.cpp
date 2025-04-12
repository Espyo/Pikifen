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
MobType::MobType(MOB_CATEGORY category_id) :
    category(game.mobCategories.get(category_id)),
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
            efc.run(gen_mob_fsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("carriable_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
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
            efc.run(gen_mob_fsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_thrown", ENEMY_EXTRA_STATE_CARRIABLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(gen_mob_fsm::loseMomentum);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(gen_mob_fsm::handleDelivery);
        }
    }
    
    
    vector<MobState*> new_states = efc.finish();
    
    states.insert(states.end(), new_states.begin(), new_states.end());
    
}


/**
 * @brief Specifies what animation conversions there are, if any.
 *
 * @return The animation conversions.
 */
anim_conversion_vector MobType::getAnimConversions() const {
    return anim_conversion_vector();
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
 * @param folder_path Path to the folder this mob type is in.
 */
void MobType::loadFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level, const string &folder_path
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string custom_carry_spots_str;
    string spike_damage_str;
    string target_type_str;
    string huntable_targets_str;
    string hurtable_targets_str;
    string team_str;
    string inactive_logic_str;
    DataNode* area_editor_tips_node = nullptr;
    DataNode* custom_carry_spots_node = nullptr;
    DataNode* spike_damage_node = nullptr;
    DataNode* target_type_node = nullptr;
    DataNode* huntable_targets_node = nullptr;
    DataNode* hurtable_targets_node = nullptr;
    DataNode* team_node = nullptr;
    DataNode* inactive_logic_node = nullptr;
    
    rs.set("acceleration", acceleration);
    rs.set("appears_in_area_editor", appearsInAreaEditor);
    rs.set(
        "area_editor_recommend_links_from",
        areaEditorRecommendLinksFrom
    );
    rs.set(
        "area_editor_recommend_links_to",
        areaEditorRecommendLinksTo
    );
    rs.set("area_editor_tips", areaEditorTips, &area_editor_tips_node);
    rs.set("blackout_radius", blackoutRadius);
    rs.set("can_block_paths", canBlockPaths);
    rs.set("can_free_move", canFreeMove);
    rs.set(
        "can_hunt", huntable_targets_str, &huntable_targets_node
    );
    rs.set(
        "can_hurt", hurtable_targets_str, &hurtable_targets_node
    );
    rs.set("can_walk_on_others", canWalkOnOthers);
    rs.set("casts_shadow", castsShadow);
    rs.set(
        "custom_carry_spots", custom_carry_spots_str, &custom_carry_spots_node
    );
    rs.set("custom_category_name", customCategoryName);
    rs.set("default_vulnerability", defaultVulnerability);
    rs.set("description", description);
    rs.set("has_group", hasGroup);
    rs.set("health_regen", healthRegen);
    rs.set("height", height);
    rs.set("inactive_logic", inactive_logic_str, &inactive_logic_node);
    rs.set("itch_damage", itchDamage);
    rs.set("itch_time", itchTime);
    rs.set("main_color", mainColor);
    rs.set("max_carriers", maxCarriers);
    rs.set("max_health", maxHealth);
    rs.set("move_speed", moveSpeed);
    rs.set("pushable", pushable);
    rs.set("pushes", pushes);
    rs.set("pushes_softly", pushesSoftly);
    rs.set("pushes_with_hitboxes", pushesWithHitboxes);
    rs.set("radius", radius);
    rs.set("rectangular_dimensions", rectangularDim);
    rs.set("rotation_speed", rotationSpeed);
    rs.set("show_health", showHealth);
    rs.set("spike_damage", spike_damage_str, &spike_damage_node);
    rs.set("target_type", target_type_str, &target_type_node);
    rs.set("team", team_str, &team_node);
    rs.set("terrain_radius", terrainRadius);
    rs.set("territory_radius", territoryRadius);
    rs.set("walkable", walkable);
    rs.set("weight", weight);
    
    if(area_editor_tips_node) {
        areaEditorTips = unescapeString(areaEditorTips);
    }
    
    if(!custom_carry_spots_str.empty()) {
        vector<string> points =
            semicolonListToVector(custom_carry_spots_str);
        if(points.size() != maxCarriers) {
            game.errors.report(
                "The number of custom carry spots (" + i2s(points.size()) +
                ") does not match the number of max carriers (" +
                i2s(maxCarriers) + ")!",
                custom_carry_spots_node
            );
        } else {
            for(size_t p = 0; p < points.size(); p++) {
                customCarrySpots.push_back(s2p(points[p]));
            }
        }
    }
    
    rotationSpeed = degToRad(rotationSpeed);
    
    //Vulnerabilities.
    DataNode* vulnerabilities_node =
        node->getChildByName("vulnerabilities");
    for(size_t h = 0; h < vulnerabilities_node->getNrOfChildren(); h++) {
    
        DataNode* vuln_node = vulnerabilities_node->getChild(h);
        auto hazard_it = game.content.hazards.list.find(vuln_node->name);
        vector<string> words = split(vuln_node->value);
        float percentage = defaultVulnerability;
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
        auto status_it = game.content.statusTypes.list.find(status_name);
        
        if(hazard_it == game.content.hazards.list.end()) {
            game.errors.report(
                "Unknown hazard \"" + vuln_node->name + "\"!",
                vuln_node
            );
            
        } else if(
            !status_name.empty() && status_it == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_name + "\"!",
                vuln_node
            );
            
        } else {
            MobType::Vulnerability &vuln =
                hazardVulnerabilities[&(hazard_it->second)];
            vuln.effectMult = percentage / 100.0f;
            if(!status_name.empty()) {
                vuln.statusToApply = status_it->second;
            }
            vuln.statusOverrides = status_overrides;
        }
    }
    
    //Spike damage.
    auto sd_it = game.content.spikeDamageTypes.list.find(spike_damage_str);
    if(spike_damage_node) {
        if(sd_it == game.content.spikeDamageTypes.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + spike_damage_str + "\"!",
                spike_damage_node
            );
        } else {
            spikeDamage = &(sd_it->second);
        }
    }
    
    //Team.
    if(team_node) {
        MOB_TEAM t = stringToTeamNr(team_str);
        if(t != INVALID) {
            startingTeam = t;
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
            inactiveLogic =
                0;
        } else if(inactive_logic_str == "ticks") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_TICKS;
        } else if(inactive_logic_str == "interactions") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else if(inactive_logic_str == "all_logic") {
            inactiveLogic =
                INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
        } else {
            game.errors.report(
                "Invalid inactive logic \"" + inactive_logic_str + "\"!",
                inactive_logic_node
            );
        }
    }
    
    //Spike damage vulnerabilities.
    DataNode* spike_damage_vuln_node =
        node->getChildByName("spike_damage_vulnerabilities");
    size_t n_sd_vuln =
        spike_damage_vuln_node->getNrOfChildren();
    for(size_t v = 0; v < n_sd_vuln; v++) {
    
        DataNode* vul_node = spike_damage_vuln_node->getChild(v);
        auto sdv_it = game.content.spikeDamageTypes.list.find(vul_node->name);
        vector<string> words = split(vul_node->value);
        float percentage = 1.0f;
        string status_name;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            status_name = words[1];
        }
        auto status_it = game.content.statusTypes.list.find(status_name);
        
        if(sdv_it == game.content.spikeDamageTypes.list.end()) {
            game.errors.report(
                "Unknown spike damage type \"" + vul_node->name + "\"!",
                vul_node
            );
            
        } else if(
            !status_name.empty() && status_it == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_name + "\"!",
                vul_node
            );
            
        } else {
            auto &s = spikeDamageVulnerabilities[&(sdv_it->second)];
            s.effectMult = percentage / 100.0f;
            s.statusToApply = status_it->second;
            
        }
    }
    
    //Status vulnerabilities.
    DataNode* status_vuln_node =
        node->getChildByName("status_vulnerabilities");
    size_t n_s_vuln =
        status_vuln_node->getNrOfChildren();
    for(size_t v = 0; v < n_s_vuln; v++) {
    
        DataNode* vul_node = status_vuln_node->getChild(v);
        auto sv_it = game.content.statusTypes.list.find(vul_node->name);
        vector<string> words = split(vul_node->value);
        float percentage = 1.0f;
        string status_override_name;
        if(!words.empty()) {
            percentage = s2f(words[0]);
        }
        if(words.size() >= 2) {
            status_override_name = words[1];
        }
        auto status_override_it = game.content.statusTypes.list.find(status_override_name);
        
        if(sv_it == game.content.statusTypes.list.end()) {
            game.errors.report(
                "Unknown status type \"" + vul_node->name + "\"!",
                vul_node
            );
            
        } else if(
            !status_override_name.empty() &&
            status_override_it == game.content.statusTypes.list.end()
        ) {
            game.errors.report(
                "Unknown status type \"" + status_override_name + "\"!",
                vul_node
            );
            
        } else {
            auto &s = statusVulnerabilities[sv_it->second];
            s.effectMult = percentage / 100.0f;
            if(status_override_it != game.content.statusTypes.list.end()) {
                s.statusToApply = status_override_it->second;
            }
            s.statusOverrides = true;
        }
        
    }
    
    //Reaches.
    DataNode* reaches_node = node->getChildByName("reaches");
    size_t n_reaches = reaches_node->getNrOfChildren();
    for(size_t r = 0; r < n_reaches; r++) {
    
        MobType::Reach new_reach;
        new_reach.name = reaches_node->getChild(r)->name;
        vector<string> r_strings = split(reaches_node->getChild(r)->value);
        
        if(r_strings.size() != 2 && r_strings.size() != 4) {
            game.errors.report(
                "Reach \"" + new_reach.name +
                "\" isn't made up of 2 or 4 words!",
                reaches_node->getChild(r)
            );
            continue;
        }
        
        new_reach.radius1 = s2f(r_strings[0]);
        new_reach.angle1 = degToRad(s2f(r_strings[1]));
        if(r_strings.size() == 4) {
            new_reach.radius2 = s2f(r_strings[2]);
            new_reach.angle2 = degToRad(s2f(r_strings[3]));
        }
        reaches.push_back(new_reach);
    }
    
    //Spawns.
    DataNode* spawns_node = node->getChildByName("spawns");
    size_t n_spawns = spawns_node->getNrOfChildren();
    for(size_t s = 0; s < n_spawns; s++) {
    
        DataNode* spawn_node = spawns_node->getChild(s);
        ReaderSetter spawn_rs(spawn_node);
        MobType::SpawnInfo new_spawn;
        string coords_str;
        
        new_spawn.name = spawn_node->name;
        spawn_rs.set("object", new_spawn.mobTypeName);
        spawn_rs.set("relative", new_spawn.relative);
        spawn_rs.set("coordinates", coords_str);
        spawn_rs.set("angle", new_spawn.angle);
        spawn_rs.set("vars", new_spawn.vars);
        spawn_rs.set("link_object_to_spawn", new_spawn.linkObjectToSpawn);
        spawn_rs.set("link_spawn_to_object", new_spawn.linkSpawnToObject);
        spawn_rs.set("momentum", new_spawn.momentum);
        
        if(!coords_str.empty()) {
            new_spawn.coordsXY = s2p(coords_str, &new_spawn.coordsZ);
        }
        new_spawn.angle = degToRad(new_spawn.angle);
        
        spawns.push_back(new_spawn);
    }
    
    //Children.
    DataNode* children_node = node->getChildByName("children");
    size_t n_children = children_node->getNrOfChildren();
    for(size_t c = 0; c < n_children; c++) {
    
        DataNode* child_node = children_node->getChild(c);
        ReaderSetter child_rs(child_node);
        MobType::Child new_child;
        
        string limb_draw_method;
        string hold_rotation_method;
        DataNode* limb_draw_node;
        DataNode* hold_rotation_node;
        
        new_child.name = child_node->name;
        child_rs.set("spawn", new_child.spawnName);
        child_rs.set("parent_holds", new_child.parentHolds);
        child_rs.set("hold_body_part", new_child.holdBodyPart);
        child_rs.set("hold_offset_distance", new_child.holdOffsetDist);
        child_rs.set(
            "hold_offset_vertical_distance", new_child.holdOffsetVertDist
        );
        child_rs.set("hold_offset_angle", new_child.holdOffsetAngle);
        child_rs.set(
            "hold_rotation_method", hold_rotation_method, &hold_rotation_node
        );
        child_rs.set("handle_damage", new_child.handleDamage);
        child_rs.set("relay_damage", new_child.relayDamage);
        child_rs.set("handle_events", new_child.handleEvents);
        child_rs.set("relay_events", new_child.relayEvents);
        child_rs.set("handle_statuses", new_child.handleStatuses);
        child_rs.set("relay_statuses", new_child.relayStatuses);
        child_rs.set("limb_animation", new_child.limbAnimName);
        child_rs.set("limb_thickness", new_child.limbThickness);
        child_rs.set("limb_parent_body_part", new_child.limbParentBodyPart);
        child_rs.set("limb_parent_offset", new_child.limbParentOffset);
        child_rs.set("limb_child_body_part", new_child.limbChildBodyPart);
        child_rs.set("limb_child_offset", new_child.limbChildOffset);
        child_rs.set("limb_draw_method", limb_draw_method, &limb_draw_node);
        
        new_child.holdOffsetAngle = degToRad(new_child.holdOffsetAngle);
        
        if(limb_draw_node) {
            if(limb_draw_method == "below_both") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_BOTH;
            } else if(limb_draw_method == "below_child") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_CHILD;
            } else if(limb_draw_method == "below_parent") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_BELOW_PARENT;
            } else if(limb_draw_method == "above_parent") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_PARENT;
            } else if(limb_draw_method == "above_child") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_CHILD;
            } else if(limb_draw_method == "above_both") {
                new_child.limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_BOTH;
            } else {
                game.errors.report(
                    "Unknow limb draw method \"" +
                    limb_draw_method + "\"!", limb_draw_node
                );
            }
        }
        
        if(hold_rotation_node) {
            if(hold_rotation_method == "never") {
                new_child.holdRotationMethod =
                    HOLD_ROTATION_METHOD_NEVER;
            } else if(hold_rotation_method == "face_parent") {
                new_child.holdRotationMethod =
                    HOLD_ROTATION_METHOD_FACE_HOLDER;
            } else if(hold_rotation_method == "copy_parent") {
                new_child.holdRotationMethod =
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
    DataNode* sounds_node = node->getChildByName("sounds");
    size_t n_sounds = sounds_node->getNrOfChildren();
    for(size_t s = 0; s < n_sounds; s++) {
    
        DataNode* sound_node = sounds_node->getChild(s);
        ReaderSetter sound_rs(sound_node);
        MobType::Sound new_sound;
        
        string file_str;
        DataNode* file_node;
        string type_str;
        DataNode* type_node;
        string stack_mode_str;
        DataNode* stack_mode_node;
        float volume_float = 100.0f;
        float speed_float = 100.0f;
        bool loop_bool = false;
        
        new_sound.name = sound_node->name;
        
        sound_rs.set("file", file_str, &file_node);
        sound_rs.set("type", type_str, &type_node);
        sound_rs.set("stack_mode", stack_mode_str, &stack_mode_node);
        sound_rs.set("stack_min_pos", new_sound.config.stackMinPos);
        sound_rs.set("loop", loop_bool);
        sound_rs.set("volume", volume_float);
        sound_rs.set("speed", speed_float);
        sound_rs.set("volume_deviation", new_sound.config.gainDeviation);
        sound_rs.set("speed_deviation", new_sound.config.speedDeviation);
        sound_rs.set("random_delay", new_sound.config.randomDelay);
        
        new_sound.sample = game.content.sounds.list.get(file_str, file_node);
        
        if(type_node) {
            if(type_str == "gameplay_global") {
                new_sound.type = SOUND_TYPE_GAMEPLAY_GLOBAL;
            } else if(type_str == "gameplay_pos") {
                new_sound.type = SOUND_TYPE_GAMEPLAY_POS;
            } else if(type_str == "ambiance_global") {
                new_sound.type = SOUND_TYPE_AMBIANCE_GLOBAL;
            } else if(type_str == "ambiance_pos") {
                new_sound.type = SOUND_TYPE_AMBIANCE_POS;
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
                new_sound.config.stackMode = SOUND_STACK_MODE_NORMAL;
            } else if(stack_mode_str == "override") {
                new_sound.config.stackMode = SOUND_STACK_MODE_OVERRIDE;
            } else if(stack_mode_str == "never") {
                new_sound.config.stackMode = SOUND_STACK_MODE_NEVER;
            } else {
                game.errors.report(
                    "Unknow sound effect stack mode \"" +
                    stack_mode_str + "\"!", stack_mode_node
                );
            }
        }
        
        if(loop_bool) {
            enableFlag(new_sound.config.flags, SOUND_FLAG_LOOP);
        }
        
        new_sound.config.gain = volume_float / 100.0f;
        new_sound.config.gain = std::clamp(new_sound.config.gain, 0.0f, 1.0f);
        
        new_sound.config.speed = speed_float / 100.0f;
        new_sound.config.speed = std::max(0.0f, new_sound.config.speed);
        
        new_sound.config.gainDeviation /= 100.0f;
        new_sound.config.speedDeviation /= 100.0f;
        
        sounds.push_back(new_sound);
    }
    
    //Area editor properties.
    DataNode* ae_props_node =
        node->getChildByName("area_editor_properties");
    size_t n_ae_props = ae_props_node->getNrOfChildren();
    for(size_t p = 0; p < n_ae_props; p++) {
    
        DataNode* prop_node = ae_props_node->getChild(p);
        ReaderSetter prop_rs(prop_node);
        string type_str;
        string list_str;
        DataNode* type_node = nullptr;
        
        MobType::AreaEditorProp new_prop;
        new_prop.name = prop_node->name;
        
        prop_rs.set("var", new_prop.var);
        prop_rs.set("type", type_str, &type_node);
        prop_rs.set("def_value", new_prop.defValue);
        prop_rs.set("min_value", new_prop.minValue);
        prop_rs.set("max_value", new_prop.maxValue);
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
        } else if(type_str == "float") {
            new_prop.type = AEMP_TYPE_FLOAT;
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
        
        if(new_prop.minValue > new_prop.maxValue) {
            std::swap(new_prop.minValue, new_prop.maxValue);
        }
        
        if(new_prop.type == AEMP_TYPE_LIST || new_prop.type == AEMP_TYPE_NR_LIST) {
            if(list_str.empty()) {
                game.errors.report(
                    "For this area editor property type, you need to specify "
                    "a list of values!", prop_node
                );
            } else {
                new_prop.valueList = semicolonListToVector(list_str);
            }
        }
        
        new_prop.tooltip = unescapeString(new_prop.tooltip);
        
        areaEditorProps.push_back(new_prop);
    }
    
    if(target_type_node) {
        MOB_TARGET_FLAG target_type_value =
            stringToMobTargetType(target_type_str);
        if(target_type_value == INVALID) {
            game.errors.report(
                "Unknown target type \"" + target_type_str + "\"!",
                target_type_node
            );
        } else {
            targetType = target_type_value;
        }
    }
    
    vector<string> huntable_targets_strs =
        semicolonListToVector(huntable_targets_str);
    if(huntable_targets_node) {
        huntableTargets = 0;
    }
    for(size_t h = 0; h < huntable_targets_strs.size(); h++) {
        size_t v = stringToMobTargetType(huntable_targets_strs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + huntable_targets_strs[h] + "\"!",
                huntable_targets_node
            );
        } else {
            huntableTargets |= (bitmask_16_t) v;
        }
    }
    
    vector<string> hurtable_targets_strs =
        semicolonListToVector(hurtable_targets_str);
    if(hurtable_targets_node) {
        hurtableTargets = 0;
    }
    for(size_t h = 0; h < hurtable_targets_strs.size(); h++) {
        size_t v = stringToMobTargetType(hurtable_targets_strs[h]);
        if(v == INVALID) {
            game.errors.report(
                "Unknown target type \"" + hurtable_targets_strs[h] + "\"!",
                hurtable_targets_node
            );
        } else {
            hurtableTargets |= (bitmask_16_t) v;
        }
    }
    
    //Resources.
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        animDb = &game.content.mobAnimDbs.list[category->id][manifest->internalName];
        animDb->fillSoundIdxCaches(this);
        
        DataNode script_file;
        script_file.loadFile(folder_path + "/script.txt", true, true);
        size_t old_n_states = states.size();
        
        DataNode* death_state_name_node =
            script_file.getChildByName("death_state");
        dyingStateName = death_state_name_node->value;
        
        statesIgnoringDeath =
            semicolonListToVector(
                script_file.getChildByName("states_ignoring_death")->value
            );
            
        statesIgnoringSpray =
            semicolonListToVector(
                script_file.getChildByName("states_ignoring_spray")->value
            );
            
        statesIgnoringHazard =
            semicolonListToVector(
                script_file.getChildByName("states_ignoring_hazard")->value
            );
            
        //Load init actions.
        loadActions(
            this,
            script_file.getChildByName("init"), &initActions, nullptr
        );
        //Load the rest of the script.
        loadScript(
            this,
            script_file.getChildByName("script"),
            script_file.getChildByName("global"),
            &states
        );
        
        if(states.size() > old_n_states) {
        
            DataNode* first_state_name_node =
                script_file.getChildByName("first_state");
            string first_state_name = first_state_name_node->value;
            
            for(size_t s = 0; s < states.size(); s++) {
                if(states[s]->name == first_state_name) {
                    firstStateIdx = s;
                    break;
                }
            }
            if(firstStateIdx == INVALID) {
                game.errors.report(
                    "Unknown state \"" + first_state_name + "\" "
                    "to set as the first state!",
                    first_state_name_node
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
                        death_state_name_node
                    );
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
 * @param base_anim_total How many base animations exist.
 * @return The vector.
 */
anim_conversion_vector
MobTypeWithAnimGroups::getAnimConversionsWithGroups(
    const anim_conversion_vector &v, size_t base_anim_total
) const {
    anim_conversion_vector new_v;
    
    for(size_t g = 0; g < animationGroupSuffixes.size(); g++) {
        for(size_t c = 0; c < v.size(); c++) {
            new_v.push_back(
                make_pair(
                    g * base_anim_total + v[c].first,
                    v[c].second + animationGroupSuffixes[g]
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
void createSpecialMobTypes() {
    MobCategory* custom_category =
        game.mobCategories.get(MOB_CATEGORY_CUSTOM);
        
    MobType* bridge_component_type = custom_category->createType();
    bridge_component_type->name = "Bridge component";
    bridge_component_type->blackoutRadius = 0;
    bridge_component_type->appearsInAreaEditor = false;
    bridge_component_type->castsShadow = false;
    bridge_component_type->customCategoryName = "Misc";
    bridge_component_type->height = 8.0f;
    bridge_component_type->physicalSpan = 8.0f;
    bridge_component_type->radius = 8.0f;
    bridge_component_type->walkable = true;
    bridge_component_type->drawMobCallback = Bridge::drawComponent;
    bridge_component_type->pushes = true;
    bridge_component_type->pushesSoftly = false;
    custom_category->registerType("bridge_component", bridge_component_type);
}
