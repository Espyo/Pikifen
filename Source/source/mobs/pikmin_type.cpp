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

const float DEFAULT_BURIED_EVOLUTION_TIME[N_MATURITIES] =
    { 2 * 60, 2 * 60, 3 * 60 };

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
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        buried_evolution_time[m] = DEFAULT_BURIED_EVOLUTION_TIME[m];
        bmp_top[m] = NULL;
        bmp_maturity_icon[m] = NULL;
    }
    
    weight = 1;
    show_health = false;
    
    mob_type::reach_struct idle_attack_reach;
    idle_attack_reach.angle_1 = M_PI * 2;
    idle_attack_reach.radius_1 = idle_task_range;
    reaches.push_back(idle_attack_reach);
    mob_type::reach_struct group_move_attack_reach;
    group_move_attack_reach.angle_1 = M_PI * 2;
    group_move_attack_reach.radius_1 = group_move_task_range;
    reaches.push_back(group_move_attack_reach);
    mob_type::reach_struct chase_reach;
    chase_reach.angle_1 = M_PI * 2;
    chase_reach.radius_1 = pikmin_chase_range;
    reaches.push_back(chase_reach);
    
    pikmin_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void pikmin_type::load_parameters(data_node* file) {
    
    reader_setter rs(file);
    
    rs.set("attack_power", attack_power);
    rs.set("throw_strength_mult", throw_strength_mult);
    rs.set("can_carry_bomb_rocks", can_carry_bomb_rocks);
    rs.set("can_dig", can_dig);
    rs.set("can_latch", can_latch);
    rs.set("can_swim", can_swim);
    rs.set("carry_speed", carry_speed);
    rs.set("carry_strength", carry_strength);
    rs.set("has_onion", has_onion);
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        rs.set("buried_evolution_time_" + i2s(m + 1), buried_evolution_time[m]);
    }
    
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
    
    pikmin_in_onions[this] =
        s2i(file->get_child_by_name("onion_starting_number")->value);
        
    max_throw_height =
        get_max_throw_height(get_throw_z_speed(throw_strength_mult));
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void pikmin_type::load_resources(data_node* file) {
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


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pikmin_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(PIKMIN_ANIM_IDLING,    "idling"));
    v.push_back(make_pair(PIKMIN_ANIM_WALKING,   "walking"));
    v.push_back(make_pair(PIKMIN_ANIM_THROWN,    "thrown"));
    v.push_back(make_pair(PIKMIN_ANIM_ATTACKING, "attacking"));
    v.push_back(make_pair(PIKMIN_ANIM_GRABBING,  "grabbing"));
    v.push_back(make_pair(PIKMIN_ANIM_SIGHING,   "sighing"));
    v.push_back(make_pair(PIKMIN_ANIM_CARRYING,  "carrying"));
    v.push_back(make_pair(PIKMIN_ANIM_BURIED,    "buried"));
    v.push_back(make_pair(PIKMIN_ANIM_PLUCKING,  "plucking"));
    v.push_back(make_pair(PIKMIN_ANIM_LYING,     "lying"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void pikmin_type::unload_resources() {
    bitmaps.detach(bmp_top[0]);
    bitmaps.detach(bmp_top[1]);
    bitmaps.detach(bmp_top[2]);
    bitmaps.detach(bmp_icon);
    bitmaps.detach(bmp_maturity_icon[0]);
    bitmaps.detach(bmp_maturity_icon[1]);
    bitmaps.detach(bmp_maturity_icon[2]);
}
