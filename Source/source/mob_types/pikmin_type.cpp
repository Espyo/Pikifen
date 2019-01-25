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
#include "../mobs/leader.h"
#include "../mob_fsms/pikmin_fsm.h"
#include "../mob_script.h"
#include "../utils/string_utils.h"
#include "../vars.h"

const float DEFAULT_SPROUT_EVOLUTION_TIME[N_MATURITIES] =
{ 2 * 60, 2 * 60, 3 * 60 };

/* ----------------------------------------------------------------------------
 * Creates a type of Pikmin.
 */
pikmin_type::pikmin_type() :
    mob_type(MOB_CATEGORY_PIKMIN),
    carry_strength(1),
    throw_strength_mult(1.0),
    max_throw_height(0),
    has_onion(true),
    can_dig(false),
    can_fly(false),
    can_swim(false),
    can_latch(true),
    can_carry_bomb_rocks(false),
    bmp_icon(nullptr) {
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        sprout_evolution_time[m] = DEFAULT_SPROUT_EVOLUTION_TIME[m];
        bmp_top[m] = NULL;
        bmp_maturity_icon[m] = NULL;
    }
    
    weight = 1;
    show_health = false;
    
    mob_type::reach_struct idle_attack_reach;
    idle_attack_reach.angle_1 = TAU;
    idle_attack_reach.radius_1 = idle_task_range;
    reaches.push_back(idle_attack_reach);
    mob_type::reach_struct group_move_attack_reach;
    group_move_attack_reach.angle_1 = TAU;
    group_move_attack_reach.radius_1 = group_move_task_range;
    reaches.push_back(group_move_attack_reach);
    mob_type::reach_struct chase_reach;
    chase_reach.angle_1 = TAU;
    chase_reach.radius_1 = pikmin_chase_range;
    reaches.push_back(chase_reach);
    
    pikmin_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void pikmin_type::load_parameters(data_node* file) {

    reader_setter rs(file);
    
    rs.set("throw_strength_mult", throw_strength_mult);
    rs.set("can_carry_bomb_rocks", can_carry_bomb_rocks);
    rs.set("can_dig", can_dig);
    rs.set("can_latch", can_latch);
    rs.set("can_swim", can_swim);
    rs.set("carry_strength", carry_strength);
    rs.set("has_onion", has_onion);
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        rs.set("sprout_evolution_time_" + i2s(m + 1), sprout_evolution_time[m]);
    }
    
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
    v.push_back(make_pair(PIKMIN_ANIM_IDLING,     "idling"));
    v.push_back(make_pair(PIKMIN_ANIM_WALKING,    "walking"));
    v.push_back(make_pair(PIKMIN_ANIM_THROWN,     "thrown"));
    v.push_back(make_pair(PIKMIN_ANIM_ATTACKING,  "attacking"));
    v.push_back(make_pair(PIKMIN_ANIM_GRABBING,   "grabbing"));
    v.push_back(make_pair(PIKMIN_ANIM_SIGHING,    "sighing"));
    v.push_back(make_pair(PIKMIN_ANIM_CARRYING,   "carrying"));
    v.push_back(make_pair(PIKMIN_ANIM_SPROUT,     "sprout"));
    v.push_back(make_pair(PIKMIN_ANIM_PLUCKING,   "plucking"));
    v.push_back(make_pair(PIKMIN_ANIM_LYING,      "lying"));
    v.push_back(make_pair(PIKMIN_ANIM_DRINKING,   "drinking"));
    v.push_back(make_pair(PIKMIN_ANIM_PICKING_UP, "picking_up"));
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


pikmin_type::~pikmin_type() { }
