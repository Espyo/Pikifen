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
void onion_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", ONION_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::startIdling);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receiveMob);
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::checkStartGenerating);
        }
    }
    
    efc.newState("generating", ONION_STATE_GENERATING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::startGenerating);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receiveMob);
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::checkStopGenerating);
        }
    }
    
    efc.newState("stopping_generation", ONION_STATE_STOPPING_GENERATION); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(onion_fsm::stopGenerating);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(onion_fsm::receiveMob);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(onion_fsm::checkStartGenerating);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
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
void onion_fsm::checkStartGenerating(Mob* m, void* info1, void* info2) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "started_generation") {
        m->fsm.setState(ONION_STATE_GENERATING);
    }
}


/**
 * @brief When an Onion has to check if it stopped generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the message received.
 * @param info2 Unused.
 */
void onion_fsm::checkStopGenerating(Mob* m, void* info1, void* info2) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "stopped_generation") {
        m->fsm.setState(ONION_STATE_STOPPING_GENERATION);
    }
}


/**
 * @brief When an Onion finishes receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void onion_fsm::receiveMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Mob* delivery = (Mob*) info1;
    Onion* oni_ptr = (Onion*) m;
    size_t seeds = 0;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        seeds = ((Enemy*) delivery)->ene_type->pikmin_seeds;
        
        if(game.cur_area_data->mission.enemy_points_on_collection) {
            game.states.gameplay->enemy_points_collected += ((Enemy*) delivery)->ene_type->points;
        }
        
        break;
    } case MOB_CATEGORY_PELLETS: {
        Pellet* pel_ptr = (Pellet*) delivery;
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
    
    oni_ptr->stopGenerating();
    oni_ptr->generation_delay_timer.start();
    oni_ptr->generation_queue[type_idx] += seeds;
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sys_content_names.part_onion_insertion, oni_ptr
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
void onion_fsm::startGenerating(Mob* m, void* info1, void* info2) {
    m->setAnimation(ONION_ANIM_GENERATING);
}


/**
 * @brief When an Onion enters the idle state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void onion_fsm::startIdling(Mob* m, void* info1, void* info2) {
    m->setAnimation(
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
void onion_fsm::stopGenerating(Mob* m, void* info1, void* info2) {
    m->setAnimation(ONION_ANIM_STOPPING_GENERATION);
}
