/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
 * Creates a type of Pikmin.
 */
pikmin_type::pikmin_type() :
    mob_type(MOB_CATEGORY_PIKMIN),
    carry_strength(1),
    attack_power(1),
    carry_speed(1),
    bmp_icon(nullptr),
    throw_strength_mult(1.0),
    has_onion(true),
    can_dig(false),
    can_fly(false),
    can_swim(false),
    can_latch(true),
    can_carry_bomb_rocks(false) {
    
    bmp_top[0] = NULL;
    bmp_top[1] = NULL;
    bmp_top[2] = NULL;
    bmp_maturity_icon[0] = NULL;
    bmp_maturity_icon[1] = NULL;
    bmp_maturity_icon[2] = NULL;
    
    weight = 1;
    show_health = false;
    
    mob_type::reach_struct idle_attack_reach;
    idle_attack_reach.angle_1 = M_PI;
    idle_attack_reach.radius_1 = idle_task_range;
    reaches.push_back(idle_attack_reach);
    mob_type::reach_struct group_move_attack_reach;
    group_move_attack_reach.angle_1 = M_PI;
    group_move_attack_reach.radius_1 = group_move_task_range;
    reaches.push_back(group_move_attack_reach);
    mob_type::reach_struct chase_reach;
    chase_reach.angle_1 = M_PI;
    chase_reach.radius_1 = pikmin_chase_range;
    reaches.push_back(chase_reach);
    
    pikmin_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads data about the Pikmin type from a data file.
 */
void pikmin_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    attack_power = s2f(file->get_child_by_name("attack_power")->value);
    throw_strength_mult =
        s2f(
            file->get_child_by_name(
                "throw_strength_mult"
            )->get_value_or_default("1")
        );
    can_carry_bomb_rocks =
        s2b(
            file->get_child_by_name("can_carry_bomb_rocks")->value
        );
    can_dig = s2b(file->get_child_by_name("can_dig")->value);
    can_latch = s2b(file->get_child_by_name("can_latch")->value);
    can_swim = s2b(file->get_child_by_name("can_swim")->value);
    carry_speed = s2f(file->get_child_by_name("carry_speed")->value);
    carry_strength = s2f(file->get_child_by_name("carry_strength")->value);
    has_onion = s2b(file->get_child_by_name("has_onion")->value);
    
    data_node* hazards_node = file->get_child_by_name("resistances");
    vector<string> hazards_strs = semicolon_list_to_vector(hazards_node->value);
    for(size_t h = 0; h < hazards_strs.size(); ++h) {
        string hazard_name = hazards_strs[h];
        if(hazards.find(hazard_name) == hazards.end()) {
            log_error("Unknown hazard \"" + hazard_name + "\"!", hazards_node);
        } else {
            resistances.push_back(&(hazards[hazard_name]));
        }
    }
    
    if(load_resources) {
        bmp_top[0] =
            bitmaps.get(file->get_child_by_name("top_leaf")->value, file);
        bmp_top[1] =
            bitmaps.get(file->get_child_by_name("top_bud")->value, file);
        bmp_top[2] =
            bitmaps.get(file->get_child_by_name("top_flower")->value, file);
        bmp_icon =
            bitmaps.get(file->get_child_by_name("icon")->value, file);
        bmp_maturity_icon[0] =
            bitmaps.get(file->get_child_by_name("icon_leaf")->value, file);
        bmp_maturity_icon[1] =
            bitmaps.get(file->get_child_by_name("icon_bud")->value, file);
        bmp_maturity_icon[2] =
            bitmaps.get(file->get_child_by_name("icon_flower")->value, file);
    }
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_IDLING,    "idling"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_WALKING,   "walking"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_THROWN,    "thrown"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_ATTACKING, "attacking"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_GRABBING,  "grabbing"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_SIGHING,   "sighing"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_CARRYING,  "carrying"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_BURIED,    "buried"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_PLUCKING,  "plucking"));
    anim_conversions->push_back(make_pair(PIKMIN_ANIM_LYING,     "lying"));
    
    pikmin_in_onions[this] =
        s2i(file->get_child_by_name("onion_starting_number")->value);
        
    max_throw_height =
        get_max_throw_height(get_throw_z_speed(throw_strength_mult));
}
