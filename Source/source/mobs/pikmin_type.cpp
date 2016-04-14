/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin type class and Pikmin type-related functions.
 */

#include "pikmin_type.h"

#include "../const.h"
#include "../functions.h"
#include "leader.h"
#include "../mob_script.h"
#include "pikmin_fsm.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin type.
 */
pikmin_type::pikmin_type() :
    mob_type(),
    carry_strength(1),
    attack_power(1),
    carry_speed(1),
    attack_interval(0.8),
    throw_height_mult(1.0),
    has_onion(true),
    can_dig(false),
    can_fly(false),
    can_swim(false),
    can_latch(true),
    can_carry_bomb_rocks(false) {
    
    bmp_top[0] = NULL;
    bmp_top[1] = NULL;
    bmp_top[2] = NULL;
    bmp_icon[0] = NULL;
    bmp_icon[1] = NULL;
    bmp_icon[2] = NULL;
    
    weight = 1;
    show_health = false;
    
    pikmin_fsm::create_fsm(this);
}


void pikmin_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    attack_power = s2f(file->get_child_by_name("attack_power")->value);
    attack_interval = s2f(file->get_child_by_name("attack_interval")->get_value_or_default("0.8"));
    throw_height_mult = s2f(file->get_child_by_name("throw_height_mult")->get_value_or_default("1"));
    can_carry_bomb_rocks = s2b(file->get_child_by_name("can_carry_bomb_rocks")->value);
    can_dig = s2b(file->get_child_by_name("can_dig")->value);
    can_latch = s2b(file->get_child_by_name("can_latch")->value);
    can_swim = s2b(file->get_child_by_name("can_swim")->value);
    carry_speed = s2f(file->get_child_by_name("carry_speed")->value);
    carry_strength = s2f(file->get_child_by_name("carry_strength")->value);
    has_onion = s2b(file->get_child_by_name("has_onion")->value);
    
    data_node* hazards_node = file->get_child_by_name("resistances");
    vector<string> hazards_strs = split(hazards_node->value, ";");
    for(size_t h = 0; h < hazards_strs.size(); ++h) {
        string hazard_name = hazards_strs[h];
        if(hazards.find(hazard_name) == hazards.end()) {
            error_log("Hazard \"" + hazard_name + "\" not found!", hazards_node);
        } else {
            resistences.push_back(&(hazards[hazard_name]));
        }
    }
    
    if(load_resources) {
        bmp_top[0] =  bitmaps.get(file->get_child_by_name("top_leaf")->value,    file);
        bmp_top[1] =  bitmaps.get(file->get_child_by_name("top_bud")->value,     file);
        bmp_top[2] =  bitmaps.get(file->get_child_by_name("top_flower")->value,  file);
        bmp_icon[0] = bitmaps.get(file->get_child_by_name("icon_leaf")->value,   file);
        bmp_icon[1] = bitmaps.get(file->get_child_by_name("icon_bud")->value,    file);
        bmp_icon[2] = bitmaps.get(file->get_child_by_name("icon_flower")->value, file);
    }
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_IDLE,     "idle"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_WALK,     "walk"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_THROWN,   "thrown"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_ATTACK,   "attack"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_GRAB,     "grab"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_SIGH,     "sigh"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_CARRY,    "carry"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_BURROWED, "burrowed"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_PLUCKING, "plucking"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_LYING,    "lying"));
    
    pikmin_in_onions[this] = s2i(file->get_child_by_name("onion_starting_number")->value);
}
