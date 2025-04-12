/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin type class and Pikmin type-related functions.
 */

#include "pikmin_type.h"

#include "../../core/const.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob_script/pikmin_fsm.h"
#include "../mob/leader.h"
#include "../other/mob_script.h"


namespace PIKMIN_TYPE {

//How long a Pikmin that got knocked down stays on the floor for, if left alone.
const float DEF_KNOCKED_DOWN_DURATION = 1.8f;

//A whistled Pikmin that got knocked down loses this much in lie-down time.
const float DEF_KNOCKED_DOWN_WHISTLE_BONUS = 1.2f;

}


const float DEFAULT_SPROUT_EVOLUTION_TIME[N_MATURITIES] =
{ 2 * 60, 2 * 60, 3 * 60 };


/**
 * @brief Constructs a new Pikmin type object.
 *
 */
PikminType::PikminType() :
    MobType(MOB_CATEGORY_PIKMIN) {
    
    for(size_t m = 0; m < N_MATURITIES; m++) {
        sproutEvolutionTime[m] = DEFAULT_SPROUT_EVOLUTION_TIME[m];
        bmpTop[m] = nullptr;
        bmpMaturityIcon[m] = nullptr;
    }
    for(size_t s = 0; s < N_PIKMIN_SOUNDS; s++) {
        soundDataIdxs[s] = INVALID;
    }
    
    inactiveLogic =
        INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
    weight = 1;
    showHealth = false;
    
    MobType::Reach idle_attack_reach;
    idle_attack_reach.angle1 = TAU;
    idle_attack_reach.radius1 = game.config.pikmin.idleTaskRange;
    reaches.push_back(idle_attack_reach);
    MobType::Reach swarm_attack_reach;
    swarm_attack_reach.angle1 = TAU;
    swarm_attack_reach.radius1 = game.config.pikmin.swarmTaskRange;
    reaches.push_back(swarm_attack_reach);
    MobType::Reach chase_reach;
    chase_reach.angle1 = TAU;
    chase_reach.radius1 = game.config.pikmin.chaseRange;
    reaches.push_back(chase_reach);
    targetType = MOB_TARGET_FLAG_PLAYER;
    huntableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE;
    hurtableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_FRAGILE;
        
    AreaEditorProp aep_maturity;
    aep_maturity.name = "Maturity";
    aep_maturity.var = "maturity";
    aep_maturity.type = AEMP_TYPE_NR_LIST;
    aep_maturity.defValue = "2";
    aep_maturity.valueList.push_back("Leaf");
    aep_maturity.valueList.push_back("Bud");
    aep_maturity.valueList.push_back("Flower");
    aep_maturity.tooltip = "The Pikmin's starting maturity.";
    areaEditorProps.push_back(aep_maturity);
    
    AreaEditorProp aep_sprout;
    aep_sprout.name = "Sprout";
    aep_sprout.var = "sprout";
    aep_sprout.type = AEMP_TYPE_BOOL;
    aep_sprout.defValue = "false";
    aep_sprout.tooltip =
        "True if this Pikmin spawns as a sprout, "
        "false if it spawns as an idle Pikmin.";
    areaEditorProps.push_back(aep_sprout);
    
    AreaEditorProp aep_follow_link;
    aep_sprout.name = "Follow link as leader";
    aep_sprout.var = "follow_link_as_leader";
    aep_sprout.type = AEMP_TYPE_BOOL;
    aep_sprout.defValue = "false";
    aep_sprout.tooltip =
        "True if this Pikmin should follow its linked object, "
        "as if it were its leader.";
    areaEditorProps.push_back(aep_sprout);
    
    pikmin_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector PikminType::getAnimConversions() const {
    anim_conversion_vector v;
    
#define a(idx, name) \
    v.push_back(std::make_pair(idx, name));
    
    a(PIKMIN_ANIM_IDLING,             "idling");
    a(PIKMIN_ANIM_CALLED,             "called");
    a(PIKMIN_ANIM_WALKING,            "walking");
    a(PIKMIN_ANIM_THROWN,             "thrown");
    a(PIKMIN_ANIM_MOB_LANDING,        "mob_landing");
    a(PIKMIN_ANIM_ATTACKING,          "attacking");
    a(PIKMIN_ANIM_BACKFLIP,           "backflip");
    a(PIKMIN_ANIM_TWIRLING,           "twirling");
    a(PIKMIN_ANIM_SIGHING,            "sighing");
    a(PIKMIN_ANIM_SHAKING,            "shaking");
    a(PIKMIN_ANIM_CARRYING,           "carrying");
    a(PIKMIN_ANIM_CARRYING_STRUGGLE,  "carrying_struggle");
    a(PIKMIN_ANIM_SPROUT,             "sprout");
    a(PIKMIN_ANIM_PLUCKING,           "plucking");
    a(PIKMIN_ANIM_PLUCKING_THROWN,    "plucking_thrown");
    a(PIKMIN_ANIM_KNOCKED_BACK,       "knocked_back");
    a(PIKMIN_ANIM_LYING,              "lying");
    a(PIKMIN_ANIM_GETTING_UP,         "getting_up");
    a(PIKMIN_ANIM_FLAILING,           "flailing");
    a(PIKMIN_ANIM_DRINKING,           "drinking");
    a(PIKMIN_ANIM_PICKING_UP,         "picking_up");
    a(PIKMIN_ANIM_ARMS_OUT,           "arms_out");
    a(PIKMIN_ANIM_PUSHING,            "pushing");
    a(PIKMIN_ANIM_CLIMBING,           "climbing");
    a(PIKMIN_ANIM_SLIDING,            "sliding");
    a(PIKMIN_ANIM_CRUSHED,            "crushed");
    a(PIKMIN_ANIM_KNOCKED_DOWN_DYING, "knocked_down_dying");
    a(PIKMIN_ANIM_DYING,              "dying");
    
#undef a
    
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PikminType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    string attack_method_str;
    string top_leaf_str;
    string top_bud_str;
    string top_flower_str;
    DataNode* attack_method_node = nullptr;
    DataNode* top_leaf_node = nullptr;
    DataNode* top_bud_node = nullptr;
    DataNode* top_flower_node = nullptr;
    
    rs.set("attack_method", attack_method_str, &attack_method_node);
    rs.set("knocked_down_duration", knockedDownDuration);
    rs.set("knocked_down_whistle_bonus", knockedDownWhistleBonus);
    rs.set("can_carry_tools", canCarryTools);
    rs.set("can_fly", canFly);
    rs.set("carry_strength", carryStrength);
    rs.set("max_throw_height", maxThrowHeight);
    rs.set("push_strength", pushStrength);
    rs.set("sprout_evolution_time_1", sproutEvolutionTime[0]);
    rs.set("sprout_evolution_time_2", sproutEvolutionTime[1]);
    rs.set("sprout_evolution_time_3", sproutEvolutionTime[2]);
    rs.set("top_bud", top_bud_str, &top_bud_node);
    rs.set("top_flower", top_flower_str, &top_flower_node);
    rs.set("top_leaf", top_leaf_str, &top_leaf_node);
    
    if(attack_method_node) {
        if(attack_method_str == "latch") {
            attackMethod = PIKMIN_ATTACK_LATCH;
        } else if(attack_method_str == "impact") {
            attackMethod = PIKMIN_ATTACK_IMPACT;
        } else {
            game.errors.report(
                "Unknown Pikmin attack type \"" + attack_method_str + "\"!",
                attack_method_node
            );
        }
    }
    
    for(size_t s = 0; s < sounds.size(); s++) {
        if(sounds[s].name == "called") {
            soundDataIdxs[PIKMIN_SOUND_CALLED] = s;
        } else if(sounds[s].name == "carrying") {
            soundDataIdxs[PIKMIN_SOUND_CARRYING] = s;
        } else if(sounds[s].name == "carrying_grab") {
            soundDataIdxs[PIKMIN_SOUND_CARRYING_GRAB] = s;
        } else if(sounds[s].name == "caught") {
            soundDataIdxs[PIKMIN_SOUND_CAUGHT] = s;
        } else if(sounds[s].name == "dying") {
            soundDataIdxs[PIKMIN_SOUND_DYING] = s;
        } else if(sounds[s].name == "held") {
            soundDataIdxs[PIKMIN_SOUND_HELD] = s;
        } else if(sounds[s].name == "idle") {
            soundDataIdxs[PIKMIN_SOUND_IDLE] = s;
        } else if(sounds[s].name == "thrown") {
            soundDataIdxs[PIKMIN_SOUND_THROWN] = s;
        }
    }
    
    //Always load these since they're necessary for the animation editor.
    bmpTop[0] = game.content.bitmaps.list.get(top_leaf_str, top_leaf_node);
    bmpTop[1] = game.content.bitmaps.list.get(top_bud_str, top_bud_node);
    bmpTop[2] = game.content.bitmaps.list.get(top_flower_str, top_flower_node);
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void PikminType::loadCatResources(DataNode* file) {
    ReaderSetter rs(file);
    
    string icon_str;
    string icon_leaf_str;
    string icon_bud_str;
    string icon_flower_str;
    string icon_onion_str;
    DataNode* icon_node = nullptr;
    DataNode* icon_leaf_node = nullptr;
    DataNode* icon_bud_node = nullptr;
    DataNode* icon_flower_node = nullptr;
    DataNode* icon_onion_node = nullptr;
    
    rs.set("icon", icon_str, &icon_node);
    rs.set("icon_bud", icon_bud_str, &icon_bud_node);
    rs.set("icon_flower", icon_flower_str, &icon_flower_node);
    rs.set("icon_leaf", icon_leaf_str, &icon_leaf_node);
    rs.set("icon_onion", icon_onion_str, &icon_onion_node);
    
    bmpIcon = game.content.bitmaps.list.get(icon_str, icon_node);
    bmpMaturityIcon[0] = game.content.bitmaps.list.get(icon_leaf_str, icon_leaf_node);
    bmpMaturityIcon[1] = game.content.bitmaps.list.get(icon_bud_str, icon_bud_node);
    bmpMaturityIcon[2] = game.content.bitmaps.list.get(icon_flower_str, icon_flower_node);
    
    if(icon_onion_node) {
        bmpOnionIcon = game.content.bitmaps.list.get(icon_onion_str, icon_onion_node);
    }
}


/**
 * @brief Unloads resources from memory.
 */
void PikminType::unloadResources() {
    game.content.bitmaps.list.free(bmpIcon);
    game.content.bitmaps.list.free(bmpMaturityIcon[0]);
    game.content.bitmaps.list.free(bmpMaturityIcon[1]);
    game.content.bitmaps.list.free(bmpMaturityIcon[2]);
    game.content.bitmaps.list.free(bmpTop[0]);
    game.content.bitmaps.list.free(bmpTop[1]);
    game.content.bitmaps.list.free(bmpTop[2]);
    if(bmpOnionIcon) {
        game.content.bitmaps.list.free(bmpOnionIcon);
    }
}
