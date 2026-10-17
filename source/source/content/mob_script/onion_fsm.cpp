/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion finite-state machine logic.
 */

#include "onion_fsm.hpp"

#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/string_utils.hpp"
#include "../mob/onion.hpp"
#include "../mob/pellet.hpp"
#include "../other/particle.hpp"
#include "gen_mob_fsm.hpp"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the Onion's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void OnionFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", ONION_STATE_IDLING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(OnionFsm::startIdling);
        }
        efc.newEvent(FSM_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(FSM_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
            efc.run(OnionFsm::doReceptionJump);
        }
        efc.newEvent(FSM_EV_HITBOX_TOUCH_N_A); {
            efc.run(GenMobFsm::beAttacked);
        }
        efc.newEvent(FSM_EV_RECEIVE_SCRIPT_MESSAGE); {
            efc.run(OnionFsm::checkStartGenerating);
        }
        efc.newEvent(FSM_EV_ZERO_HEALTH); {
            efc.changeState("dead");
        }
    }
    
    efc.newState("generating", ONION_STATE_GENERATING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(OnionFsm::startGenerating);
        }
        efc.newEvent(FSM_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(FSM_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(FSM_EV_HITBOX_TOUCH_N_A); {
            efc.run(GenMobFsm::beAttacked);
        }
        efc.newEvent(FSM_EV_RECEIVE_SCRIPT_MESSAGE); {
            efc.run(OnionFsm::checkStopGenerating);
        }
        efc.newEvent(FSM_EV_ZERO_HEALTH); {
            efc.changeState("dead");
        }
    }
    
    efc.newState("stopping_generation", ONION_STATE_STOPPING_GENERATION); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(OnionFsm::stopGenerating);
        }
        efc.newEvent(FSM_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(FSM_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(FSM_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(FSM_EV_HITBOX_TOUCH_N_A); {
            efc.run(GenMobFsm::beAttacked);
        }
        efc.newEvent(FSM_EV_RECEIVE_SCRIPT_MESSAGE); {
            efc.run(OnionFsm::checkStartGenerating);
        }
        efc.newEvent(FSM_EV_ZERO_HEALTH); {
            efc.changeState("dead");
        }
    }
    
    efc.newState("received", ONION_STATE_RECEIVED); {
        efc.newEvent(FSM_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::startDelivery);
        }
        efc.newEvent(FSM_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(OnionFsm::receiveMob);
        }
        efc.newEvent(FSM_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(FSM_EV_HITBOX_TOUCH_N_A); {
            efc.run(GenMobFsm::beAttacked);
        }
        efc.newEvent(FSM_EV_RECEIVE_SCRIPT_MESSAGE); {
            efc.run(OnionFsm::checkStartGenerating);
        }
        efc.newEvent(FSM_EV_ZERO_HEALTH); {
            efc.changeState("dead");
        }
    }
    
    efc.newState("dead", ONION_STATE_DEAD); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(OnionFsm::die);
        }
    }
    
    typ->scriptDef.fsm.states = efc.finish();
    typ->scriptDef.fsm.setFirstState("idling");
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->scriptDef.fsm.states.size() == N_ONION_STATES,
        i2s(typ->scriptDef.fsm.states.size()) + " registered, " +
        i2s(N_ONION_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When an Onion has to check if it started generating Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the script message received.
 * @param info2 Unused.
 */
void OnionFsm::checkStartGenerating(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "started_generation") {
        scriptVM->fsm.setState(ONION_STATE_GENERATING);
    }
}


/**
 * @brief When an Onion has to check if it stopped generating Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the script message received.
 * @param info2 Unused.
 */
void OnionFsm::checkStopGenerating(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    if(!info1) return;
    string* msg = (string*) info1;
    if(*msg == "stopped_generation") {
        scriptVM->fsm.setState(ONION_STATE_STOPPING_GENERATION);
    }
}


/**
 * @brief When an Onion dies.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the script message received.
 * @param info2 Unused.
 */
void OnionFsm::die(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->startDying();
    oniPtr->activated = false;
}


/**
 * @brief When an Onion has received something and is in a state
 * where it should change to the appropriate animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void OnionFsm::doReceptionJump(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->setAnimation(ONION_ANIM_RECEIVED);
    scriptVM->fsm.setState(ONION_STATE_RECEIVED);
}


/**
 * @brief When an Onion finishes receiving a mob carried by Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void OnionFsm::receiveMob(ScriptVM* scriptVM, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    Mob* delivery = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    //Figure out how many nutrients to receive.
    size_t nutrients = 0;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        nutrients = ((Enemy*) delivery)->eneType->pikminSeeds;
        
        game.states.gameplay->enemyCollectionPointsObtained +=
            ((Enemy*) delivery)->eneType->points;
        game.states.gameplay->lastCollectedEnemyPos = oniPtr->center;
        
        break;
    } case MOB_CATEGORY_PELLETS: {
        Pellet* pelPtr = (Pellet*) delivery;
        if(
            pelPtr->pelType->pikType ==
            delivery->deliveryInfo->intendedPikType
        ) {
            nutrients = pelPtr->pelType->matchNutrients;
        } else {
            nutrients = pelPtr->pelType->nonMatchNutrients;
        }
        break;
    } default: {
        break;
    }
    }
    
    //Either accept the nutrients or forward them.
    Onion* nutrientReceiver = nullptr;
    if(
        oniPtr->oniType->sendNutrientsToLink &&
        !oniPtr->links.empty() &&
        oniPtr->links[0]->type->category->id == MOB_CATEGORY_ONIONS
    ) {
        nutrientReceiver = (Onion*) oniPtr->links[0];
    } else {
        nutrientReceiver = oniPtr;
    }
    nutrientReceiver->receiveNutrients(
        delivery->deliveryInfo->intendedPikType, nutrients
    );
    
    //Update the beaming status.
    oniPtr->mobsBeingBeamed--;
    
    if(oniPtr->mobsBeingBeamed == 0 && oniPtr->soundBeamId != 0) {
        game.audio.destroySoundSource(oniPtr->soundBeamId);
        oniPtr->soundBeamId = 0;
    }
    
    //Particles and sounds.
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
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void OnionFsm::startDelivery(ScriptVM* scriptVM, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->mobsBeingBeamed++;
    if(oniPtr->mobsBeingBeamed == 1 && oniPtr->soundBeamId == 0) {
        oniPtr->soundBeamId = oniPtr->playSound(oniPtr->oniType->soundBeamIdx);
    }
}



/**
 * @brief When an Onion starts generating Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void OnionFsm::startGenerating(ScriptVM* scriptVM, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->setAnimation(ONION_ANIM_GENERATING);
}


/**
 * @brief When an Onion enters the idle state.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void OnionFsm::startIdling(ScriptVM* scriptVM, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->setAnimation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When an Onion stops generating Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob being received.
 * @param info2 Unused.
 */
void OnionFsm::stopGenerating(ScriptVM* scriptVM, void* info1, void* info2) {
    Onion* oniPtr = (Onion*) scriptVM->mob;
    
    oniPtr->setAnimation(ONION_ANIM_STOPPING_GENERATION);
}


#pragma endregion
