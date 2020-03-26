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


leader_type::~leader_type() { }


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
 * Loads properties from a data file.
 */
void leader_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("max_throw_height", max_throw_height);
    rs.set("whistle_range", whistle_range);
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void leader_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    string dismiss_sfx_str;
    string icon_str;
    string name_call_sfx_str;
    string whistle_sfx_str;
    data_node* icon_node;
    
    rs.set("dismiss_sfx", dismiss_sfx_str);
    rs.set("icon", icon_str, &icon_node);
    rs.set("name_call_sfx", name_call_sfx_str);
    rs.set("whistle_sfx", whistle_sfx_str);
    
    //TODO Replace load_sample with something better when it exists.
    bmp_icon = bitmaps.get(icon_str, icon_node);
    sfx_dismiss = load_sample(dismiss_sfx_str, mixer);
    sfx_name_call = load_sample(name_call_sfx_str, mixer);
    sfx_whistle = load_sample(whistle_sfx_str, mixer);
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
