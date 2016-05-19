/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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
#include "mob_fsm.h"
#include "../const.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of leader.
 */
leader_type::leader_type() :
    mob_type(),
    whistle_range(DEF_WHISTLE_RANGE),
    punch_strength(DEF_PUNCH_STRENGTH),
    throw_height_mult(1.0),
    pluck_delay(0.6),
    bmp_icon(nullptr) {

    main_color = al_map_rgb(128, 128, 128);
    show_health = false;

    leader_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads data about the leader type from a data file.
 */
void leader_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
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
    throw_height_mult =
        s2f(
            file->get_child_by_name("throw_height_mult")->get_value_or_default(
                "1"
            )
        );

    if(load_resources) {
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

#define new_conversion(id, name) \
    anim_conversions->push_back(make_pair((id), (name)))

    new_conversion(LEADER_ANIM_IDLE,         "idle");
    new_conversion(LEADER_ANIM_WALK,         "walk");
    new_conversion(LEADER_ANIM_PLUCK,        "pluck");
    new_conversion(LEADER_ANIM_GET_UP,       "get_up");
    new_conversion(LEADER_ANIM_DISMISS,      "dismiss");
    new_conversion(LEADER_ANIM_THROW,        "thrown");
    new_conversion(LEADER_ANIM_WHISTLING,    "whistling");
    new_conversion(LEADER_ANIM_LIE,          "lie");
    new_conversion(LEADER_ANIM_PAIN,         "pain");
    new_conversion(LEADER_ANIM_KNOCKED_DOWN, "knocked_down");
    new_conversion(LEADER_ANIM_SPRAYING,     "spraying");

#undef new_conversion

}
