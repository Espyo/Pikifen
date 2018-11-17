/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop type class and drop type-related functions.
 */

#include "drop_type.h"
#include "../mobs/drop_fsm.h"
#include "../functions.h"


/* ----------------------------------------------------------------------------
 * Creates a type of drop.
 */
drop_type::drop_type() :
    mob_type(MOB_CATEGORY_DROPS),
    consumer(DROP_CONSUMER_PIKMIN),
    effect(DROP_EFFECT_MATURATE),
    total_doses(1),
    increase_amount(2),
    spray_type_to_increase(nullptr),
    status_to_give(nullptr) {
    
    drop_fsm::create_fsm(this);
}


drop_type::~drop_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void drop_type::load_parameters(data_node* file) {
    reader_setter rs(file);
    
    string consumer_str;
    string effect_str;
    string spray_name_str;
    string status_name_str;
    
    rs.set("consumer", consumer_str);
    rs.set("effect", effect_str);
    rs.set("total_doses", total_doses);
    rs.set("increase_amount", increase_amount);
    rs.set("spray_type_to_increase", spray_name_str);
    rs.set("status_to_give", status_name_str);
    
    if(consumer_str == "pikmin") {
        consumer = DROP_CONSUMER_PIKMIN;
    } else if(consumer_str == "leaders") {
        consumer = DROP_CONSUMER_LEADERS;
    } else {
        log_error("Unknown consumer \"" + consumer_str + "\"!", file);
    }
    
    if(effect_str == "maturate") {
        effect = DROP_EFFECT_MATURATE;
    } else if(effect_str == "increase_sprays") {
        effect = DROP_EFFECT_INCREASE_SPRAYS;
    } else if(effect_str == "give_status") {
        effect = DROP_EFFECT_GIVE_STATUS;
    } else {
        log_error("Unknown drop effect \"" + effect_str + "\"!", file);
    }
    
    if(!spray_name_str.empty()) {
        for(size_t s = 0; s < spray_types.size(); ++s) {
            if(spray_types[s].name == spray_name_str) {
                spray_type_to_increase = &spray_types[s];
                break;
            }
        }
        if(!spray_type_to_increase) {
            log_error("Unknown spray type \"" + spray_name_str + "\"!", file);
        }
    }
    
    if(!status_name_str.empty()) {
        auto s = status_types.find(status_name_str);
        if(s != status_types.end()) {
            status_to_give = &(s->second);
        } else {
            log_error("Unknown status type \"" + status_name_str + "\"!", file);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector drop_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(DROP_ANIM_IDLING, "idling"));
    v.push_back(make_pair(DROP_ANIM_FALLING, "falling"));
    v.push_back(make_pair(DROP_ANIM_LANDING, "landing"));
    v.push_back(make_pair(DROP_ANIM_BUMPED, "bumped"));
    return v;
}
