/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion finite state machine logic.
 */

#include "onion_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/onion.h"
#include "../mob/pellet.h"
#include "../other/particle.h"


/**
 * @brief Creates the finite state machine for the Onion's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void onion_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idling", ONION_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::start_idling);
        }
        efc.new_event(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receive_mob);
        }
        efc.new_event(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::check_start_generating);
        }
    }
    
    efc.new_state("generating", ONION_STATE_GENERATING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::start_generating);
        }
        efc.new_event(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receive_mob);
        }
        efc.new_event(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::check_stop_generating);
        }
    }
    
    efc.new_state("stopping_generation", ONION_STATE_STOPPING_GENERATION); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::stop_generating);
        }
        efc.new_event(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receive_mob);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::check_start_generating);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_ONION_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_ONION_STATES) + " in enum."
    );
}


/**
 * @brief When an Onion has to check if it started generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the message received.
 * @param info2 Unused.
 */
void onion_fsm::check_start_generating(mob* m, void* info1, void* info2) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "started_generation") {
        m->fsm.set_state(ONION_STATE_GENERATING);
    }
}


/**
 * @brief When an Onion has to check if it stopped generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the message received.
 * @param info2 Unused.
 */
void onion_fsm::check_stop_generating(mob* m, void* info1, void* info2) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "stopped_generation") {
        m->fsm.set_state(ONION_STATE_STOPPING_GENERATION);
    }
}


/**
 * @brief When an Onion finishes receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void onion_fsm::receive_mob(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    mob* delivery = (mob*) info1;
    onion* oni_ptr = (onion*) m;
    size_t seeds = 0;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        seeds = ((enemy*) delivery)->ene_type->pikmin_seeds;
        break;
    } case MOB_CATEGORY_PELLETS: {
        pellet* pel_ptr = (pellet*) delivery;
        if(
            pel_ptr->pel_type->pik_type ==
            delivery->delivery_info->intended_pik_type
        ) {
            seeds = pel_ptr->pel_type->match_seeds;
        } else {
            seeds = pel_ptr->pel_type->non_match_seeds;
        }
        break;
    } default: {
        break;
    }
    }
    
    size_t type_idx = 0;
    for(; type_idx < oni_ptr->oni_type->nest->pik_types.size(); type_idx++) {
        if(
            oni_ptr->oni_type->nest->pik_types[type_idx] ==
            delivery->delivery_info->intended_pik_type
        ) {
            break;
        }
    }
    
    oni_ptr->stop_generating();
    oni_ptr->generation_delay_timer.start();
    oni_ptr->generation_queue[type_idx] += seeds;
    
    particle_generator pg =
        standard_particle_gen_setup(
            game.asset_file_names.part_onion_insertion, oni_ptr
        );
    pg.follow_z_offset -= 2.0f; //Must appear below the Onion's bulb.
    oni_ptr->particle_generators.push_back(pg);
    
}


/**
 * @brief When an Onion starts generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void onion_fsm::start_generating(mob* m, void* info1, void* info2) {
    m->set_animation(ONION_ANIM_GENERATING);
}


/**
 * @brief When an Onion enters the idle state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void onion_fsm::start_idling(mob* m, void* info1, void* info2) {
    m->set_animation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When an Onion stops generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void onion_fsm::stop_generating(mob* m, void* info1, void* info2) {
    m->set_animation(ONION_ANIM_STOPPING_GENERATION);
}
