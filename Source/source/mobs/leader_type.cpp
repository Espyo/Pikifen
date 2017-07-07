/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader type class and leader type-related functions.
 */

#include "../functions.h"
#include "leader_fsm.h"
#include "leader_type.h"
#include "../load.h"
#include "mob_fsm.h"
#include "../const.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of leader.
 */
leader_type::leader_type() :
    mob_type(MOB_CATEGORY_LEADERS),
    whistle_range(DEF_WHISTLE_RANGE),
    punch_strength(DEF_PUNCH_STRENGTH),
    throw_strength_mult(1.0),
    pluck_delay(0.6),
    bmp_icon(nullptr) {
    
    main_color = al_map_rgb(128, 128, 128);
    show_health = false;
    
    leader_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void leader_type::load_parameters(data_node* file) {
    pluck_delay =
        s2f(file->get_child_by_name("pluck_delay")->value);
    whistle_range =
        s2f(
            file->get_child_by_name("whistle_range")->get_value_or_default(
                f2s(DEF_WHISTLE_RANGE)
            )
        );
    punch_strength =
        s2i(file->get_child_by_name("punch_strength")->value); //TODO default.
    throw_strength_mult =
        s2f(
            file->get_child_by_name(
                "throw_strength_mult"
            )->get_value_or_default("1")
        );
        
    max_throw_height =
        get_max_throw_height(get_throw_z_speed(throw_strength_mult));
        
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void leader_type::load_resources(data_node* file) {
    //TODO don't use load_sample for these.
    sfx_dismiss =
        load_sample(file->get_child_by_name("dismiss_sfx")->value, mixer);
    sfx_name_call =
        load_sample(file->get_child_by_name("name_call_sfx")->value, mixer);
    sfx_whistle =
        load_sample(file->get_child_by_name("whistle_sfx")->value, mixer);
    bmp_icon =
        bitmaps.get(file->get_child_by_name("icon")->value, file);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector leader_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(LEADER_ANIM_IDLING,       "idling"));
    v.push_back(make_pair(LEADER_ANIM_WALKING,      "walking"));
    v.push_back(make_pair(LEADER_ANIM_PLUCKING,     "plucking"));
    v.push_back(make_pair(LEADER_ANIM_GETTING_UP,   "getting_up"));
    v.push_back(make_pair(LEADER_ANIM_DISMISSING,   "dismissing"));
    v.push_back(make_pair(LEADER_ANIM_THROWING,     "throwing"));
    v.push_back(make_pair(LEADER_ANIM_WHISTLING,    "whistling"));
    v.push_back(make_pair(LEADER_ANIM_LYING,        "lying"));
    v.push_back(make_pair(LEADER_ANIM_PAIN,         "pain"));
    v.push_back(make_pair(LEADER_ANIM_KNOCKED_DOWN, "knocked_down"));
    v.push_back(make_pair(LEADER_ANIM_SPRAYING,     "spraying"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void leader_type::unload_resources() {
    bitmaps.detach(bmp_icon);
}
