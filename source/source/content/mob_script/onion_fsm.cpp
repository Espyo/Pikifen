/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion finite-state machine logic.
 */

#include "onion_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/onion.h"
#include "../mob/pellet.h"
#include "../other/particle.h"


/**
 * @brief Creates the finite-state machine for the Onion's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void OnionFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", ONION_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(OnionFsm::startIdling);
        }
        efc.newEvent(MOB_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(OnionFsm::checkStartGenerating);
        }
    }
    
    efc.newState("generating", ONION_STATE_GENERATING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(OnionFsm::startGenerating);
        }
        efc.newEvent(MOB_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(OnionFsm::checkStopGenerating);
        }
    }
    
    efc.newState("stopping_generation", ONION_STATE_STOPPING_GENERATION); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(OnionFsm::stopGenerating);
        }
        efc.newEvent(MOB_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_RECEIVE_MESSAGE); {
            efc.run(OnionFsm::checkStartGenerating);
        }
    }
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
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
void OnionFsm::checkStartGenerating(Mob* m, void* info1, void* info2) {
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
void OnionFsm::checkStopGenerating(Mob* m, void* info1, void* info2) {
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
void OnionFsm::receiveMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Mob* delivery = (Mob*) info1;
    Onion* oniPtr = (Onion*) m;
    size_t seeds = 0;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        seeds = ((Enemy*) delivery)->eneType->pikminSeeds;
        
        if(game.curAreaData->missionOld.enemyPointsOnCollection) {
            game.states.gameplay->enemyPointsCollected +=
                ((Enemy*) delivery)->eneType->points;
        }
        
        break;
    } case MOB_CATEGORY_PELLETS: {
        Pellet* pelPtr = (Pellet*) delivery;
        if(
            pelPtr->pelType->pikType ==
            delivery->deliveryInfo->intendedPikType
        ) {
            seeds = pelPtr->pelType->matchSeeds;
        } else {
            seeds = pelPtr->pelType->nonMatchSeeds;
        }
        break;
    } default: {
        break;
    }
    }
    
    size_t typeIdx = 0;
    for(; typeIdx < oniPtr->oniType->nest->pikTypes.size(); typeIdx++) {
        if(
            oniPtr->oniType->nest->pikTypes[typeIdx] ==
            delivery->deliveryInfo->intendedPikType
        ) {
            break;
        }
    }
    
    oniPtr->mobsBeingBeamed--;
    
    if(oniPtr->mobsBeingBeamed == 0 && oniPtr->soundBeamId != 0) {
        game.audio.destroySoundSource(oniPtr->soundBeamId);
        oniPtr->soundBeamId = 0;
    }
    
    oniPtr->stopGenerating();
    oniPtr->generationDelayTimer.start();
    oniPtr->generationQueue[typeIdx] += seeds;
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parOnionInsertion, oniPtr
        );
    pg.followZOffset -= 2.0f; //Must appear below the Onion's bulb.
    oniPtr->particleGenerators.push_back(pg);
    
    oniPtr->playSound(oniPtr->oniType->soundReceptionIdx);
}


/**
 * @brief When an Onion starts receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void OnionFsm::startDelivery(Mob* m, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) m;
    oniPtr->mobsBeingBeamed++;
    if(oniPtr->mobsBeingBeamed == 1 && oniPtr->soundBeamId == 0) {
        oniPtr->soundBeamId = oniPtr->playSound(oniPtr->oniType->soundBeamIdx);
    }
}



/**
 * @brief When an Onion starts generating Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void OnionFsm::startGenerating(Mob* m, void* info1, void* info2) {
    m->setAnimation(ONION_ANIM_GENERATING);
}


/**
 * @brief When an Onion enters the idle state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void OnionFsm::startIdling(Mob* m, void* info1, void* info2) {
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
void OnionFsm::stopGenerating(Mob* m, void* info1, void* info2) {
    m->setAnimation(ONION_ANIM_STOPPING_GENERATION);
}
