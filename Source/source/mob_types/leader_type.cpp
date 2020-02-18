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

#include "../const.h"
#include "../functions.h"
#include "../load.h"
#include "../mob_fsms/gen_mob_fsm.h"
#include "../mob_fsms/leader_fsm.h"
#include "../utils/string_utils.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of leader.
 */
leader_type::leader_type() :
    mob_type(MOB_CATEGORY_LEADERS),
    whistle_range(DEF_WHISTLE_RANGE),
    max_throw_height(0),
    bmp_icon(nullptr) {
    
    main_color = al_map_rgb(128, 128, 128);
    show_health = false;
    target_type = MOB_TARGET_TYPE_PLAYER;
    huntable_targets =
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_ENEMY;
    hurtable_targets =
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_TYPE_FRAGILE;
        
    leader_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void leader_type::load_parameters(data_node* file) {
    whistle_range =
        s2f(
            file->get_child_by_name("whistle_range")->get_value_or_default(
                f2s(DEF_WHISTLE_RANGE)
            )
        );
    max_throw_height =
        s2f(
            file->get_child_by_name(
                "max_throw_height"
            )->get_value_or_default("130")
        );
        
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
    v.push_back(make_pair(LEADER_ANIM_PUNCHING,     "punching"));
    v.push_back(make_pair(LEADER_ANIM_LYING,        "lying"));
    v.push_back(make_pair(LEADER_ANIM_PAIN,         "pain"));
    v.push_back(make_pair(LEADER_ANIM_KNOCKED_DOWN, "knocked_down"));
    v.push_back(make_pair(LEADER_ANIM_SPRAYING,     "spraying"));
    v.push_back(make_pair(LEADER_ANIM_DRINKING,     "drinking"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void leader_type::unload_resources() {
    bitmaps.detach(bmp_icon);
    //TODO these samples are only being destroyed here because
    //they're being created in load_resource() with load_samples.
    //When the loading changes, update this unload accordingly.
    sfx_dismiss.destroy();
    sfx_name_call.destroy();
    sfx_whistle.destroy();
}


leader_type::~leader_type() { }
