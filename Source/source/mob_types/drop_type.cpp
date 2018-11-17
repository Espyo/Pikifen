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


/* ----------------------------------------------------------------------------
 * Creates a type of drop.
 */
drop_type::drop_type() :
    mob_type(MOB_CATEGORY_DROPS),
    consumer(DROP_CONSUMER_PIKMIN),
    effect(DROP_EFFECT_MATURATE),
    total_doses(1),
    increase_amount(2),
    spray_type_to_increase(nullptr) {
    
    drop_fsm::create_fsm(this);
}


drop_type::~drop_type() { }


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void drop_type::load_resources(data_node* file) {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector drop_type::get_anim_conversions() {
    anim_conversion_vector v;
    //TODO
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void drop_type::unload_resources() {
    //TODO
}
