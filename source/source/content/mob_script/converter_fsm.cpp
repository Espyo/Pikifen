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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/converter.h"


/**
 * @brief Creates the finite state machine for the converter's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void converter_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", CONVERTER_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_THROWN_PIKMIN_LANDED); {
            efc.run(converter_fsm::handlePikmin);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(converter_fsm::handleObjectTouch);
        }
    }
    
    efc.newState("bumped", CONVERTER_STATE_BUMPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::bumped);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finishBeingBumped);
            efc.changeState("closing");
        }
    }
    
    efc.newState("closing", CONVERTER_STATE_CLOSING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::openOrSpit);
        }
    }
    
    efc.newState("spitting", CONVERTER_STATE_SPITTING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::spew);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::openOrDie);
        }
    }
    
    efc.newState("opening", CONVERTER_STATE_OPENING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::open);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    efc.newState("dying", CONVERTER_STATE_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::startDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finishDying);
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
void converter_fsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    con_ptr->setAnimation(
        con_ptr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_IDLING, N_CONVERTER_ANIMS, con_ptr->currentTypeIdx
        ),
        START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    con_ptr->curBaseAnimIdx = CONVERTER_ANIM_IDLING;
    con_ptr->typeChangeTimer.start();
}


/**
 * @brief Does a little bumpy animation after a leader touches it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::bumped(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    con_ptr->setAnimation(
        con_ptr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_BUMPED, N_CONVERTER_ANIMS, con_ptr->currentTypeIdx
        )
    );
    con_ptr->curBaseAnimIdx = CONVERTER_ANIM_BUMPED;
    con_ptr->typeChangeTimer.stop();
    con_ptr->autoConversionTimer.stop();
}


/**
 * @brief Makes the converter close after it gets bumped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::finishBeingBumped(Mob* m, void* info1, void* info2) {
    ((Converter*) m)->close();
}


/**
 * @brief Makes the converter vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::finishDying(Mob* m, void* info1, void* info2) {
    m->toDelete = true;
}


/**
 * @brief Handles an object bumping against it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::handleObjectTouch(Mob* m, void* info1, void* info2) {
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
void converter_fsm::handlePikmin(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    Pikmin* pik_ptr = (Pikmin*) info1;
    
    if(con_ptr->amountInBuffer == con_ptr->conType->bufferSize) {
        //A Pikmin tried to sneak in in the middle of a conversion! Denied.
        return;
    }
    
    con_ptr->amountInBuffer++;
    if(
        con_ptr->conType->sameTypeCountsForOutput ||
        pik_ptr->pikType != con_ptr->currentType
    ) {
        con_ptr->inputPikminLeft--;
    }
    con_ptr->typeChangeTimer.stop();
    con_ptr->autoConversionTimer.start();
    
    pik_ptr->toDelete = true;
    
    if(
        con_ptr->inputPikminLeft == 0 ||
        con_ptr->amountInBuffer == con_ptr->conType->bufferSize
    ) {
        con_ptr->close();
    }
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parConverterInsertion, m
        );
    m->particleGenerators.push_back(pg);
}


/**
 * @brief Makes the converter open up.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::open(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    con_ptr->setAnimation(
        con_ptr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_OPENING, N_CONVERTER_ANIMS, con_ptr->currentTypeIdx
        )
    );
    con_ptr->curBaseAnimIdx = CONVERTER_ANIM_OPENING;
}


/**
 * @brief Changes to the opening state or the dying state, depending
 * on whether it can still output Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::openOrDie(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    if(con_ptr->inputPikminLeft == 0) {
        con_ptr->fsm.setState(CONVERTER_STATE_DYING);
    } else {
        con_ptr->fsm.setState(CONVERTER_STATE_OPENING);
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
void converter_fsm::openOrSpit(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    if(con_ptr->amountInBuffer == 0) {
        con_ptr->fsm.setState(CONVERTER_STATE_OPENING);
    } else {
        con_ptr->fsm.setState(CONVERTER_STATE_SPITTING);
    }
}


/**
 * @brief Spews out the converted seeds.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::spew(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    con_ptr->setAnimation(
        con_ptr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_SPITTING, N_CONVERTER_ANIMS, con_ptr->currentTypeIdx
        )
    );
    con_ptr->curBaseAnimIdx = CONVERTER_ANIM_SPITTING;
    con_ptr->spew();
}


/**
 * @brief Makes the converter start dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::startDying(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    con_ptr->setAnimation(
        con_ptr->getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_DYING, N_CONVERTER_ANIMS, con_ptr->currentTypeIdx
        )
    );
    con_ptr->curBaseAnimIdx = CONVERTER_ANIM_DYING;
}
