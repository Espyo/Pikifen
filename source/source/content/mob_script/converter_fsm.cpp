/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter finite state machine logic.
 */

#include "converter_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/converter.h"


/**
 * @brief Creates the finite state machine for the converter's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void ConverterFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", CONVERTER_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ConverterFsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_THROWN_PIKMIN_LANDED); {
            efc.run(ConverterFsm::handlePikmin);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(ConverterFsm::handleObjectTouch);
        }
    }
    
    efc.newState("bumped", CONVERTER_STATE_BUMPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ConverterFsm::bumped);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(ConverterFsm::finishBeingBumped);
            efc.changeState("closing");
        }
    }
    
    efc.newState("closing", CONVERTER_STATE_CLOSING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(ConverterFsm::openOrSpit);
        }
    }
    
    efc.newState("spitting", CONVERTER_STATE_SPITTING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ConverterFsm::spit);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(ConverterFsm::openOrDie);
        }
    }
    
    efc.newState("opening", CONVERTER_STATE_OPENING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ConverterFsm::open);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    efc.newState("dying", CONVERTER_STATE_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ConverterFsm::startDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(ConverterFsm::finishDying);
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_CONVERTER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_CONVERTER_STATES) + " in enum."
    );
}


/**
 * @brief Enters the idle state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    conPtr->setAnimation(
        conPtr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_IDLING, N_CONVERTER_ANIMS, conPtr->currentTypeIdx
        ),
        START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    conPtr->curBaseAnimIdx = CONVERTER_ANIM_IDLING;
    conPtr->typeChangeTimer.start();
}


/**
 * @brief Does a little bumpy animation after a leader touches it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::bumped(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    conPtr->setAnimation(
        conPtr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_BUMPED, N_CONVERTER_ANIMS, conPtr->currentTypeIdx
        )
    );
    conPtr->curBaseAnimIdx = CONVERTER_ANIM_BUMPED;
    conPtr->typeChangeTimer.stop();
    conPtr->autoConversionTimer.stop();
}


/**
 * @brief Makes the converter close after it gets bumped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::finishBeingBumped(Mob* m, void* info1, void* info2) {
    ((Converter*) m)->close();
}


/**
 * @brief Makes the converter vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::finishDying(Mob* m, void* info1, void* info2) {
    m->toDelete = true;
}


/**
 * @brief Handles an object bumping against it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::handleObjectTouch(Mob* m, void* info1, void* info2) {
    Mob* bumper = (Mob*) info1;
    if(bumper->type->category->id == MOB_CATEGORY_LEADERS) {
        m->fsm.setState(CONVERTER_STATE_BUMPED);
    }
}


/**
 * @brief Code to handle a Pikmin having been thrown inside.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::handlePikmin(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    Pikmin* pikPtr = (Pikmin*) info1;
    
    if(conPtr->amountInBuffer == conPtr->conType->bufferSize) {
        //A Pikmin tried to sneak in in the middle of a conversion! Denied.
        return;
    }
    
    conPtr->amountInBuffer++;
    if(
        conPtr->conType->sameTypeCountsForOutput ||
        pikPtr->pikType != conPtr->currentType
    ) {
        conPtr->inputPikminLeft--;
    }
    conPtr->typeChangeTimer.stop();
    conPtr->autoConversionTimer.start();
    
    pikPtr->toDelete = true;
    
    if(
        conPtr->inputPikminLeft == 0 ||
        conPtr->amountInBuffer == conPtr->conType->bufferSize
    ) {
        conPtr->close();
    }
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parConverterInsertion, m
        );
    m->particleGenerators.push_back(pg);
    conPtr->playSound(conPtr->conType->soundReceptionIdx);
}


/**
 * @brief Makes the converter open up.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::open(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    conPtr->setAnimation(
        conPtr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_OPENING, N_CONVERTER_ANIMS, conPtr->currentTypeIdx
        )
    );
    conPtr->curBaseAnimIdx = CONVERTER_ANIM_OPENING;
}


/**
 * @brief Changes to the opening state or the dying state, depending
 * on whether it can still output Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::openOrDie(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    if(conPtr->inputPikminLeft == 0) {
        conPtr->fsm.setState(CONVERTER_STATE_DYING);
    } else {
        conPtr->fsm.setState(CONVERTER_STATE_OPENING);
    }
}


/**
 * @brief Changes to the opening state or the spitting state, depending
 * on whether it has Pikmin in the buffer or not.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::openOrSpit(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    if(conPtr->amountInBuffer == 0) {
        conPtr->fsm.setState(CONVERTER_STATE_OPENING);
    } else {
        conPtr->fsm.setState(CONVERTER_STATE_SPITTING);
    }
}


/**
 * @brief Spits out the converted seeds.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::spit(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    conPtr->setAnimation(
        conPtr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_SPITTING, N_CONVERTER_ANIMS, conPtr->currentTypeIdx
        )
    );
    conPtr->curBaseAnimIdx = CONVERTER_ANIM_SPITTING;
    conPtr->spit();
}


/**
 * @brief Makes the converter start dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ConverterFsm::startDying(Mob* m, void* info1, void* info2) {
    Converter* conPtr = (Converter*) m;
    
    conPtr->setAnimation(
        conPtr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_DYING, N_CONVERTER_ANIMS, conPtr->currentTypeIdx
        )
    );
    conPtr->curBaseAnimIdx = CONVERTER_ANIM_DYING;
}
