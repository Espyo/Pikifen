/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "functions.h"
#include "ship.h"
#include "treasure.h"

/* ----------------------------------------------------------------------------
 * Creates a treasure.
 */
treasure::treasure(const float x, const float y, treasure_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    tre_type(type),
    buried(s2f(get_var_value(vars, "buried", "0"))) {
    
    become_carriable(true);
    
    set_animation(ANIM_IDLE);
    
}


void treasure::handle_delivery(mob* m, void* info1, void* info2) {
    treasure* t_ptr = (treasure*) m;
    ship* s_ptr = (ship*) t_ptr->carrying_target;
    float value = t_ptr->tre_type->value;
    
    s_ptr->fsm.run_event(MOB_EVENT_RECEIVE_DELIVERY, (void*) &value);
    
    t_ptr->to_delete = true;
}
