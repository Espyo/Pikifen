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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/leader.h"
#include "../mob_script/pikmin_fsm.h"
#include "../other/script.h"


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
    
    MobType::Reach idleAttackReach;
    idleAttackReach.angle1 = TAU;
    idleAttackReach.radius1 = game.config.pikmin.idleTaskRange;
    reaches.push_back(idleAttackReach);
    MobType::Reach swarmAttackReach;
    swarmAttackReach.angle1 = TAU;
    swarmAttackReach.radius1 = game.config.pikmin.swarmTaskRange;
    reaches.push_back(swarmAttackReach);
    MobType::Reach chaseReach;
    chaseReach.angle1 = TAU;
    chaseReach.radius1 = game.config.pikmin.chaseRange;
    reaches.push_back(chaseReach);
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
        
    AreaEditorProp aepMaturity;
    aepMaturity.name = "Maturity";
    aepMaturity.var = "maturity";
    aepMaturity.type = AEMP_TYPE_NR_LIST;
    aepMaturity.defValue = "2";
    aepMaturity.valueList.push_back("Leaf");
    aepMaturity.valueList.push_back("Bud");
    aepMaturity.valueList.push_back("Flower");
    aepMaturity.tooltip = "The Pikmin's starting maturity.";
    areaEditorProps.push_back(aepMaturity);
    
    AreaEditorProp aepSprout;
    aepSprout.name = "Sprout";
    aepSprout.var = "sprout";
    aepSprout.type = AEMP_TYPE_BOOL;
    aepSprout.defValue = "false";
    aepSprout.tooltip =
        "True if this Pikmin spawns as a sprout, "
        "false if it spawns as an idle Pikmin.";
    areaEditorProps.push_back(aepSprout);
    
    AreaEditorProp aepFollowLink;
    aepFollowLink.name = "Follow link as leader";
    aepFollowLink.var = "follow_link_as_leader";
    aepFollowLink.type = AEMP_TYPE_BOOL;
    aepFollowLink.defValue = "false";
    aepFollowLink.tooltip =
        "True if this Pikmin should follow its linked object "
        "as its leader.";
    areaEditorProps.push_back(aepFollowLink);
    
    PikminFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector PikminType::getAnimConversions() const {
    AnimConversionVector v;
    
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
    a(PIKMIN_ANIM_CARRYING_LIGHT,     "carrying_light");
    a(PIKMIN_ANIM_CARRYING_STRUGGLE,  "carrying_struggle");
    a(PIKMIN_ANIM_SPROUT,             "sprout");
    a(PIKMIN_ANIM_PLUCKING,           "plucking");
    a(PIKMIN_ANIM_PLUCKING_THROWN,    "plucking_thrown");
    a(PIKMIN_ANIM_KNOCKED_BACK,       "knocked_back");
    a(PIKMIN_ANIM_BOUNCED_BACK,       "bounced_back");
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
    ReaderSetter pRS(file);
    
    string attackMethodStr;
    string topLeafStr;
    string topBudStr;
    string topFlowerStr;
    DataNode* attackMethodNode = nullptr;
    DataNode* topLeafNode = nullptr;
    DataNode* topBudNode = nullptr;
    DataNode* topFlowerNode = nullptr;
    
    pRS.set("attack_method", attackMethodStr, &attackMethodNode);
    pRS.set("auto_maturate_interval", autoMaturateInterval);
    pRS.set("can_carry_tools", canCarryTools);
    pRS.set("chills_liquids", chillsLiquids);
    pRS.set("can_fly", canFly);
    pRS.set("can_lose_maturity", canLoseMaturity);
    pRS.set("carry_strength", carryStrength);
    pRS.set("enemy_hit_rate_modifier_latched", enemyHitRateModifierLatched);
    pRS.set("enemy_hit_rate_modifier_standing", enemyHitRateModifierStanding);
    pRS.set("knocked_down_duration", knockedDownDuration);
    pRS.set("knocked_down_whistle_bonus", knockedDownWhistleBonus);
    pRS.set("max_throw_height", maxThrowHeight);
    pRS.set("push_strength", pushStrength);
    pRS.set("sprout_evolution_time_1", sproutEvolutionTime[0]);
    pRS.set("sprout_evolution_time_2", sproutEvolutionTime[1]);
    pRS.set("sprout_evolution_time_3", sproutEvolutionTime[2]);
    pRS.set("top_bud", topBudStr, &topBudNode);
    pRS.set("top_flower", topFlowerStr, &topFlowerNode);
    pRS.set("top_leaf", topLeafStr, &topLeafNode);
    
    if(attackMethodNode) {
        readEnumProp(
            pikminAttackINames, attackMethodStr, &attackMethod,
            "Pikmin attack type", attackMethodNode
        );
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
        } else if(sounds[s].name == "maturing") {
            soundDataIdxs[PIKMIN_SOUND_MATURING] = s;
        } else if(sounds[s].name == "latch") {
            soundDataIdxs[PIKMIN_SOUND_LATCH] = s;
        } else if(sounds[s].name == "seed_landing") {
            soundDataIdxs[PIKMIN_SOUND_SEED_LANDING] = s;
        } else if(sounds[s].name == "suffering") {
            soundDataIdxs[PIKMIN_SOUND_SUFFERING] = s;
        } else if(sounds[s].name == "thrown") {
            soundDataIdxs[PIKMIN_SOUND_THROWN] = s;
        }
    }
    
    //Always load these since they're necessary for the animation editor.
    bmpTop[0] = game.content.bitmaps.list.get(topLeafStr, topLeafNode);
    bmpTop[1] = game.content.bitmaps.list.get(topBudStr, topBudNode);
    bmpTop[2] = game.content.bitmaps.list.get(topFlowerStr, topFlowerNode);
    
    enemyHitRateModifierLatched =
        std::clamp(enemyHitRateModifierLatched / 100.0f, -1.0f, 1.0f);
    enemyHitRateModifierStanding =
        std::clamp(enemyHitRateModifierStanding / 100.0f, -1.0f, 1.0f);
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void PikminType::loadCatResources(DataNode* file) {
    ReaderSetter pRS(file);
    
    string iconStr;
    string iconLeafStr;
    string iconBudStr;
    string iconFlowerStr;
    string iconOnionStr;
    DataNode* iconNode = nullptr;
    DataNode* iconLeafNode = nullptr;
    DataNode* iconBudNode = nullptr;
    DataNode* iconFlowerNode = nullptr;
    DataNode* iconOnionNode = nullptr;
    
    pRS.set("icon", iconStr, &iconNode);
    pRS.set("icon_bud", iconBudStr, &iconBudNode);
    pRS.set("icon_flower", iconFlowerStr, &iconFlowerNode);
    pRS.set("icon_leaf", iconLeafStr, &iconLeafNode);
    pRS.set("icon_onion", iconOnionStr, &iconOnionNode);
    
    bmpIcon = game.content.bitmaps.list.get(iconStr, iconNode);
    bmpMaturityIcon[0] =
        game.content.bitmaps.list.get(iconLeafStr, iconLeafNode);
    bmpMaturityIcon[1] =
        game.content.bitmaps.list.get(iconBudStr, iconBudNode);
    bmpMaturityIcon[2] =
        game.content.bitmaps.list.get(iconFlowerStr, iconFlowerNode);
        
    if(iconOnionNode) {
        bmpOnionIcon =
            game.content.bitmaps.list.get(iconOnionStr, iconOnionNode);
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
