/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader type class and leader type-related functions.
 */

#include "functions.h"
#include "leader_type.h"
#include "const.h"
#include "vars.h"

leader_type::leader_type() :
    whistle_range(DEF_WHISTLE_RANGE),
    punch_strength(DEF_PUNCH_STRENGTH),
    pluck_delay(0.6),
    bmp_icon(nullptr) {
    main_color = al_map_rgb(128, 128, 128);
}

void leader_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    pluck_delay = s2f(file->get_child_by_name("pluck_delay")->value);
    whistle_range = s2f(file->get_child_by_name("whistle_range")->get_value_or_default(f2s(DEF_WHISTLE_RANGE)));
    punch_strength = s2i(file->get_child_by_name("punch_strength")->value); //TODO default.
    
    if(load_resources) {
        sfx_dismiss = load_sample(file->get_child_by_name("dismiss_sfx")->value, mixer); //TODO don't use load_sample.
        sfx_name_call = load_sample(file->get_child_by_name("name_call_sfx")->value, mixer); //TODO don't use load_sample.
        sfx_whistle = load_sample(file->get_child_by_name("whistle_sfx")->value, mixer); //TODO don't use load_sample.
        bmp_icon = bitmaps.get(file->get_child_by_name("icon")->value, file);
    }
    
    anim_conversions->push_back(make_pair(LEADER_ANIM_IDLE,      "idle"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_WALK,      "walk"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_PLUCK,     "pluck"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_GET_UP,    "get_up"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_DISMISS,   "dismiss"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_THROW,     "thrown"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_WHISTLING, "whistling"));
    anim_conversions->push_back(make_pair(LEADER_ANIM_LIE,       "lie"));
}
