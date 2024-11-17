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

#include "../const.h"
#include "../functions.h"
#include "../game.h"
#include "../mob_fsms/pikmin_fsm.h"
#include "../mob_script.h"
#include "../mobs/leader.h"
#include "../utils/string_utils.h"


namespace PIKMIN_TYPE {

//How long a Pikmin that got knocked down stays on the floor for, if left alone.
const float DEF_KNOCKED_DOWN_DURATION = 1.8f;

//A whistled Pikmin that got knocked down loses this much in lie-down time.
const float DEF_KNOCKED_DOWN_WHISTLE_BONUS = 1.2f;

}


const float DEFAULT_SPROUT_EVOLUTION_TIME[NR_MATURITIES] =
{ 2 * 60, 2 * 60, 3 * 60 };


/**
 * @brief Constructs a new Pikmin type object.
 *
 */
pikmin_type::pikmin_type() :
    mob_type(MOB_CATEGORY_PIKMIN) {
    
    for(size_t m = 0; m < NR_MATURITIES; m++) {
        sprout_evolution_time[m] = DEFAULT_SPROUT_EVOLUTION_TIME[m];
        bmp_top[m] = nullptr;
        bmp_maturity_icon[m] = nullptr;
    }
    for(size_t s = 0; s < N_PIKMIN_SOUNDS; s++) {
        sound_data_idxs[s] = INVALID;
    }
    
    inactive_logic =
        INACTIVE_LOGIC_FLAG_TICKS | INACTIVE_LOGIC_FLAG_INTERACTIONS;
    weight = 1;
    show_health = false;
    
    mob_type::reach_t idle_attack_reach;
    idle_attack_reach.angle_1 = TAU;
    idle_attack_reach.radius_1 = game.config.idle_task_range;
    reaches.push_back(idle_attack_reach);
    mob_type::reach_t swarm_attack_reach;
    swarm_attack_reach.angle_1 = TAU;
    swarm_attack_reach.radius_1 = game.config.swarm_task_range;
    reaches.push_back(swarm_attack_reach);
    mob_type::reach_t chase_reach;
    chase_reach.angle_1 = TAU;
    chase_reach.radius_1 = game.config.pikmin_chase_range;
    reaches.push_back(chase_reach);
    target_type = MOB_TARGET_FLAG_PLAYER;
    huntable_targets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE;
    hurtable_targets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE |
        MOB_TARGET_FLAG_FRAGILE;
        
    area_editor_prop_t aep_maturity;
    aep_maturity.name = "Maturity";
    aep_maturity.var = "maturity";
    aep_maturity.type = AEMP_TYPE_NR_LIST;
    aep_maturity.def_value = "2";
    aep_maturity.value_list.push_back("Leaf");
    aep_maturity.value_list.push_back("Bud");
    aep_maturity.value_list.push_back("Flower");
    aep_maturity.tooltip = "The Pikmin's starting maturity.";
    area_editor_props.push_back(aep_maturity);
    
    area_editor_prop_t aep_sprout;
    aep_sprout.name = "Sprout";
    aep_sprout.var = "sprout";
    aep_sprout.type = AEMP_TYPE_BOOL;
    aep_sprout.def_value = "false";
    aep_sprout.tooltip =
        "True if this Pikmin spawns as a sprout, "
        "false if it spawns as an idle Pikmin.";
    area_editor_props.push_back(aep_sprout);
    
    area_editor_prop_t aep_follow_link;
    aep_sprout.name = "Follow link as leader";
    aep_sprout.var = "follow_link_as_leader";
    aep_sprout.type = AEMP_TYPE_BOOL;
    aep_sprout.def_value = "false";
    aep_sprout.tooltip =
        "True if this Pikmin should follow its linked object, "
        "as if it were its leader.";
    area_editor_props.push_back(aep_sprout);
    
    pikmin_fsm::create_fsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector pikmin_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(PIKMIN_ANIM_IDLING,       "idling"));
    v.push_back(std::make_pair(PIKMIN_ANIM_WALKING,      "walking"));
    v.push_back(std::make_pair(PIKMIN_ANIM_THROWN,       "thrown"));
    v.push_back(std::make_pair(PIKMIN_ANIM_MOB_LANDING,  "mob_landing"));
    v.push_back(std::make_pair(PIKMIN_ANIM_ATTACKING,    "attacking"));
    v.push_back(std::make_pair(PIKMIN_ANIM_GRABBING,     "grabbing"));
    v.push_back(std::make_pair(PIKMIN_ANIM_SIGHING,      "sighing"));
    v.push_back(std::make_pair(PIKMIN_ANIM_SHAKING,      "shaking"));
    v.push_back(std::make_pair(PIKMIN_ANIM_CARRYING,     "carrying"));
    v.push_back(std::make_pair(PIKMIN_ANIM_SPROUT,       "sprout"));
    v.push_back(std::make_pair(PIKMIN_ANIM_PLUCKING,     "plucking"));
    v.push_back(std::make_pair(PIKMIN_ANIM_KNOCKED_BACK, "knocked_back"));
    v.push_back(std::make_pair(PIKMIN_ANIM_LYING,        "lying"));
    v.push_back(std::make_pair(PIKMIN_ANIM_GETTING_UP,   "getting_up"));
    v.push_back(std::make_pair(PIKMIN_ANIM_FLAILING,     "flailing"));
    v.push_back(std::make_pair(PIKMIN_ANIM_DRINKING,     "drinking"));
    v.push_back(std::make_pair(PIKMIN_ANIM_PICKING_UP,   "picking_up"));
    v.push_back(std::make_pair(PIKMIN_ANIM_SLIDING,      "sliding"));
    v.push_back(std::make_pair(PIKMIN_ANIM_CRUSHED,      "crushed"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void pikmin_type::load_cat_properties(data_node* file) {
    reader_setter rs(file);
    string attack_method_str;
    data_node* attack_method_node = nullptr;
    
    rs.set("attack_method", attack_method_str, &attack_method_node);
    rs.set("knocked_down_duration", knocked_down_duration);
    rs.set("knocked_down_whistle_bonus", knocked_down_whistle_bonus);
    rs.set("can_carry_tools", can_carry_tools);
    rs.set("can_fly", can_fly);
    rs.set("carry_strength", carry_strength);
    rs.set("max_throw_height", max_throw_height);
    rs.set("push_strength", push_strength);
    rs.set("sprout_evolution_time_1", sprout_evolution_time[0]);
    rs.set("sprout_evolution_time_2", sprout_evolution_time[1]);
    rs.set("sprout_evolution_time_3", sprout_evolution_time[2]);
    
    if(attack_method_node) {
        if(attack_method_str == "latch") {
            attack_method = PIKMIN_ATTACK_LATCH;
        } else if(attack_method_str == "impact") {
            attack_method = PIKMIN_ATTACK_IMPACT;
        } else {
            game.errors.report(
                "Unknown Pikmin attack type \"" + attack_method_str + "\"!",
                attack_method_node
            );
        }
    }
    
    for(size_t s = 0; s < sounds.size(); s++) {
        if(sounds[s].name == "attack") {
            sound_data_idxs[PIKMIN_SOUND_ATTACK] = s;
        } else if(sounds[s].name == "called") {
            sound_data_idxs[PIKMIN_SOUND_CALLED] = s;
        } else if(sounds[s].name == "carrying") {
            sound_data_idxs[PIKMIN_SOUND_CARRYING] = s;
        } else if(sounds[s].name == "carrying_grab") {
            sound_data_idxs[PIKMIN_SOUND_CARRYING_GRAB] = s;
        } else if(sounds[s].name == "caught") {
            sound_data_idxs[PIKMIN_SOUND_CAUGHT] = s;
        } else if(sounds[s].name == "dying") {
            sound_data_idxs[PIKMIN_SOUND_DYING] = s;
        } else if(sounds[s].name == "held") {
            sound_data_idxs[PIKMIN_SOUND_HELD] = s;
        } else if(sounds[s].name == "idle") {
            sound_data_idxs[PIKMIN_SOUND_IDLE] = s;
        } else if(sounds[s].name == "plucked") {
            sound_data_idxs[PIKMIN_SOUND_PLUCKED] = s;
        } else if(sounds[s].name == "thrown") {
            sound_data_idxs[PIKMIN_SOUND_THROWN] = s;
        }
    }
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void pikmin_type::load_cat_resources(data_node* file) {
    reader_setter rs(file);
    
    string top_leaf_str;
    string top_bud_str;
    string top_flower_str;
    string icon_str;
    string icon_leaf_str;
    string icon_bud_str;
    string icon_flower_str;
    string icon_onion_str;
    data_node* top_leaf_node = nullptr;
    data_node* top_bud_node = nullptr;
    data_node* top_flower_node = nullptr;
    data_node* icon_node = nullptr;
    data_node* icon_leaf_node = nullptr;
    data_node* icon_bud_node = nullptr;
    data_node* icon_flower_node = nullptr;
    data_node* icon_onion_node = nullptr;
    
    rs.set("icon", icon_str, &icon_node);
    rs.set("icon_bud", icon_bud_str, &icon_bud_node);
    rs.set("icon_flower", icon_flower_str, &icon_flower_node);
    rs.set("icon_leaf", icon_leaf_str, &icon_leaf_node);
    rs.set("icon_onion", icon_onion_str, &icon_onion_node);
    rs.set("top_bud", top_bud_str, &top_bud_node);
    rs.set("top_flower", top_flower_str, &top_flower_node);
    rs.set("top_leaf", top_leaf_str, &top_leaf_node);
    
    bmp_icon = game.content.bitmaps.list.get(icon_str, icon_node);
    bmp_maturity_icon[0] = game.content.bitmaps.list.get(icon_leaf_str, icon_leaf_node);
    bmp_maturity_icon[1] = game.content.bitmaps.list.get(icon_bud_str, icon_bud_node);
    bmp_maturity_icon[2] = game.content.bitmaps.list.get(icon_flower_str, icon_flower_node);
    bmp_top[0] = game.content.bitmaps.list.get(top_leaf_str, top_leaf_node);
    bmp_top[1] = game.content.bitmaps.list.get(top_bud_str, top_bud_node);
    bmp_top[2] = game.content.bitmaps.list.get(top_flower_str, top_flower_node);
    
    if(icon_onion_node) {
        bmp_onion_icon = game.content.bitmaps.list.get(icon_onion_str, icon_onion_node);
    }
}


/**
 * @brief Unloads resources from memory.
 */
void pikmin_type::unload_resources() {
    game.content.bitmaps.list.free(bmp_icon);
    game.content.bitmaps.list.free(bmp_maturity_icon[0]);
    game.content.bitmaps.list.free(bmp_maturity_icon[1]);
    game.content.bitmaps.list.free(bmp_maturity_icon[2]);
    game.content.bitmaps.list.free(bmp_top[0]);
    game.content.bitmaps.list.free(bmp_top[1]);
    game.content.bitmaps.list.free(bmp_top[2]);
    if(bmp_onion_icon) {
        game.content.bitmaps.list.free(bmp_onion_icon);
    }
}
