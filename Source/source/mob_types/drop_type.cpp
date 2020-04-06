/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop type class and drop type-related functions.
 */

#include "drop_type.h"

#include "../functions.h"
#include "../mob_fsms/drop_fsm.h"


/* ----------------------------------------------------------------------------
 * Creates a type of drop.
 */
drop_type::drop_type() :
    mob_type(MOB_CATEGORY_DROPS),
    consumer(DROP_CONSUMER_PIKMIN),
    effect(DROP_EFFECT_MATURATE),
    total_doses(1),
    increase_amount(2),
    spray_type_to_increase(INVALID),
    status_to_give(nullptr),
    shrink_speed(40.0f) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    drop_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector drop_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(std::make_pair(DROP_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(DROP_ANIM_FALLING, "falling"));
    v.push_back(std::make_pair(DROP_ANIM_LANDING, "landing"));
    v.push_back(std::make_pair(DROP_ANIM_BUMPED, "bumped"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void drop_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string consumer_str;
    string effect_str;
    string spray_name_str;
    string status_name_str;
    data_node* consumer_node;
    data_node* effect_node;
    data_node* spray_name_node;
    data_node* status_name_node;
    data_node* total_doses_node;
    
    rs.set("consumer", consumer_str, &consumer_node);
    rs.set("effect", effect_str, &effect_node);
    rs.set("increase_amount", increase_amount);
    rs.set("shrink_speed", shrink_speed);
    rs.set("spray_type_to_increase", spray_name_str, &spray_name_node);
    rs.set("status_to_give", status_name_str, &status_name_node);
    rs.set("total_doses", total_doses, &total_doses_node);
    
    if(consumer_str == "pikmin") {
        consumer = DROP_CONSUMER_PIKMIN;
    } else if(consumer_str == "leaders") {
        consumer = DROP_CONSUMER_LEADERS;
    } else {
        log_error("Unknown consumer \"" + consumer_str + "\"!", consumer_node);
    }
    
    if(effect_str == "maturate") {
        effect = DROP_EFFECT_MATURATE;
    } else if(effect_str == "increase_sprays") {
        effect = DROP_EFFECT_INCREASE_SPRAYS;
    } else if(effect_str == "give_status") {
        effect = DROP_EFFECT_GIVE_STATUS;
    } else {
        log_error("Unknown drop effect \"" + effect_str + "\"!", effect_node);
    }
    
    if(effect == DROP_EFFECT_INCREASE_SPRAYS) {
        for(size_t s = 0; s < spray_types.size(); ++s) {
            if(spray_types[s].name == spray_name_str) {
                spray_type_to_increase = s;
                break;
            }
        }
        if(spray_type_to_increase == INVALID) {
            log_error(
                "Unknown spray type \"" + spray_name_str + "\"!",
                spray_name_node
            );
        }
    }
    
    if(status_name_node) {
        auto s = status_types.find(status_name_str);
        if(s != status_types.end()) {
            status_to_give = &(s->second);
        } else {
            log_error(
                "Unknown status type \"" + status_name_str + "\"!",
                status_name_node
            );
        }
    }
    
    if(total_doses == 0) {
        log_error(
            "The number of total doses cannot be zero!", total_doses_node
        );
    }
    
    shrink_speed /= 100.0f;
}
