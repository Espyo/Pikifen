/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource finite state machine logic.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "resource.h"
#include "resource_fsm.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the resource's logic.
 */
void resource_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idle_waiting", RESOURCE_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(resource_fsm::start_waiting);
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(resource_fsm::lose_momentum);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(resource_fsm::vanish);
        }
    }
    
    efc.new_state("idle_moving", RESOURCE_STATE_IDLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(resource_fsm::handle_start_moving);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.run(resource_fsm::handle_dropped);
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EVENT_CARRY_STUCK); {
            efc.change_state("idle_stuck");
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("idle_stuck", RESOURCE_STATE_IDLE_STUCK); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.run(resource_fsm::handle_dropped);
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("being_delivered", RESOURCE_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(resource_fsm::handle_delivery);
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle_waiting");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_RESOURCE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_RESOURCE_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When the resource is fully delivered. This should only run code that cannot
 * be handled by ships or Onions.
 */
void resource_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    resource* r_ptr = (resource*) m;
    if(
        r_ptr->res_type->delivery_result ==
        RESOURCE_DELIVERY_RESULT_DAMAGE_MOB
    ) {
        r_ptr->focused_mob->set_health(
            true, false, -r_ptr->res_type->damage_mob_amount
        );
    }
}


/* ----------------------------------------------------------------------------
 * When the resource is dropped.
 */
void resource_fsm::handle_dropped(mob* m, void* info1, void* info2) {
    resource* r_ptr = (resource*) m;
    if(!r_ptr->res_type->vanish_on_drop) return;
    
    if(r_ptr->res_type->vanish_delay == 0) {
        resource_fsm::vanish(m, info1, info2);
    } else {
        r_ptr->set_timer(r_ptr->res_type->vanish_delay);
    }
}


/* ----------------------------------------------------------------------------
 * When the resource starts moving.
 */
void resource_fsm::handle_start_moving(mob* m, void* info1, void* info2) {
    resource* r_ptr = (resource*) m;
    r_ptr->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When the resource lands from being launched in the air.
 */
void resource_fsm::lose_momentum(mob* m, void* info1, void* info2) {
    m->speed.x = 0;
    m->speed.y = 0;
    m->speed_z = 0;
}


/* ----------------------------------------------------------------------------
 * When a resource starts idling, waiting to be carried.
 */
void resource_fsm::start_waiting(mob* m, void* info1, void* info2) {
    resource* r_ptr = (resource*) m;
    
    r_ptr->become_carriable(r_ptr->res_type->carrying_destination);
    r_ptr->carry_info->must_return = true;
    r_ptr->carry_info->return_point = r_ptr->origin_pile->pos;
    r_ptr->carry_info->return_dist =
        r_ptr->origin_pile->type->radius +
        standard_pikmin_radius +
        idle_task_range / 2.0f;
        
    r_ptr->set_animation(RESOURCE_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Vanishes, either disappearing for good, or returning to its origin pile.
 */
void resource_fsm::vanish(mob* m, void* info1, void* info2) {
    resource* r_ptr = (resource*) m;
    if(r_ptr->res_type->return_to_pile_on_vanish) {
        r_ptr->origin_pile->change_amount(+1);
    }
    
    r_ptr->become_uncarriable();
    r_ptr->to_delete = true;
}
