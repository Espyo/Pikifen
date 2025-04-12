/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader type class and leader type-related functions.
 */

#include "leader_type.h"

#include "../../core/const.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../util/string_utils.h"
#include "../mob_script/gen_mob_fsm.h"
#include "../mob_script/leader_fsm.h"


namespace LEADER_TYPE {

//How long a leader that got knocked down stays on the floor for, if left alone.
const float DEF_KNOCKED_DOWN_DURATION = 1.8f;

//A whistled leader that got knocked down loses this much in lie-down time.
const float DEF_KNOCKED_DOWN_WHISTLE_BONUS = 1.2f;

//The whistle can't go past this radius, by default.
const float DEF_WHISTLE_RANGE = 80.0f;

}


/**
 * @brief Constructs a new leader type object.
 */
LeaderType::LeaderType() :
    MobType(MOB_CATEGORY_LEADERS) {
    
    inactiveLogic =
        INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
    mainColor = al_map_rgb(128, 128, 128);
    showHealth = false;
    targetType = MOB_TARGET_FLAG_PLAYER;
    hasGroup = true;
    huntableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY;
    hurtableTargets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_FRAGILE;
        
    for(size_t s = 0; s < N_LEADER_SOUNDS; s++) {
        soundDataIdxs[s] = INVALID;
    }
    
    leader_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
anim_conversion_vector LeaderType::getAnimConversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(LEADER_ANIM_IDLING,       "idling"));
    v.push_back(std::make_pair(LEADER_ANIM_CALLED,       "called"));
    v.push_back(std::make_pair(LEADER_ANIM_WALKING,      "walking"));
    v.push_back(std::make_pair(LEADER_ANIM_PLUCKING,     "plucking"));
    v.push_back(std::make_pair(LEADER_ANIM_GETTING_UP,   "getting_up"));
    v.push_back(std::make_pair(LEADER_ANIM_DISMISSING,   "dismissing"));
    v.push_back(std::make_pair(LEADER_ANIM_THROWING,     "throwing"));
    v.push_back(std::make_pair(LEADER_ANIM_WHISTLING,    "whistling"));
    v.push_back(std::make_pair(LEADER_ANIM_PUNCHING,     "punching"));
    v.push_back(std::make_pair(LEADER_ANIM_LYING,        "lying"));
    v.push_back(std::make_pair(LEADER_ANIM_PAIN,         "pain"));
    v.push_back(std::make_pair(LEADER_ANIM_KNOCKED_BACK, "knocked_back"));
    v.push_back(std::make_pair(LEADER_ANIM_SPRAYING,     "spraying"));
    v.push_back(std::make_pair(LEADER_ANIM_DRINKING,     "drinking"));
    v.push_back(std::make_pair(LEADER_ANIM_KO,           "ko"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void LeaderType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    rs.set("knocked_down_duration", knockedDownDuration);
    rs.set("knocked_down_whistle_bonus", knockedDownWhistleBonus);
    rs.set("max_throw_height", maxThrowHeight);
    rs.set("whistle_range", whistleRange);
    
    for(size_t s = 0; s < sounds.size(); s++) {
        if(sounds[s].name == "whistling") {
            soundDataIdxs[LEADER_SOUND_WHISTLING] = s;
        } else if(sounds[s].name == "dismissing") {
            soundDataIdxs[LEADER_SOUND_DISMISSING] = s;
        } else if(sounds[s].name == "name_call") {
            soundDataIdxs[LEADER_SOUND_NAME_CALL] = s;
        }
    }
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void LeaderType::loadCatResources(DataNode* file) {
    ReaderSetter rs(file);
    
    string dismiss_sound_str;
    string icon_str;
    string name_call_sound_str;
    string whistle_sound_str;
    DataNode* icon_node = nullptr;
    
    rs.set("dismiss_sound", dismiss_sound_str);
    rs.set("icon", icon_str, &icon_node);
    rs.set("name_call_sound", name_call_sound_str);
    rs.set("whistle_sound", whistle_sound_str);
    
    bmpIcon = game.content.bitmaps.list.get(icon_str, icon_node);
}


/**
 * @brief Unloads resources from memory.
 */
void LeaderType::unloadResources() {
    game.content.bitmaps.list.free(bmpIcon);
}
