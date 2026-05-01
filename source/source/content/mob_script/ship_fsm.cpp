/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship finite-state machine logic.
 */

#include "ship_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/resource.h"
#include "../mob/ship.h"
#include "../other/particle.h"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the ship's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void ShipFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", SHIP_STATE_IDLING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(ShipFsm::setAnim);
        }
        efc.newEvent(FSM_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(ShipFsm::startDelivery);
        }
        efc.newEvent(FSM_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(ShipFsm::receiveMob);
        }
    }
    
    typ->scriptDef.fsm.states = efc.finish();
    typ->scriptDef.fsm.setFirstState("idling");
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->scriptDef.fsm.states.size() == N_SHIP_STATES,
        i2s(typ->scriptDef.fsm.states.size()) + " registered, " +
        i2s(N_SHIP_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When a ship finishes receiving a mob carried by Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob.
 * @param info2 Unused.
 */
void ShipFsm::receiveMob(ScriptVM* scriptVM, void* info1, void* info2) {
    Ship* shiPtr = (Ship*) scriptVM->mob;
    Mob* delivery = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        game.states.gameplay->enemyCollectionPointsObtained +=
            ((Enemy*) delivery)->eneType->points;
        game.states.gameplay->lastCollectedEnemyPos = shiPtr->center;
        break;
        
    }
    case MOB_CATEGORY_TREASURES: {
        Treasure* trePtr = (Treasure*) delivery;
        game.states.gameplay->treasuresCollected++;
        game.states.gameplay->treasurePointsObtained +=
            trePtr->treType->points;
        game.states.gameplay->lastCollectedTreasurePos = shiPtr->center;
        break;
        
    } case MOB_CATEGORY_RESOURCES: {
        Resource* resPtr = (Resource*) delivery;
        switch(resPtr->resType->deliveryResult) {
        case RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS: {
            game.states.gameplay->treasuresCollected++;
            game.states.gameplay->treasurePointsObtained +=
                resPtr->resType->pointAmount;
            game.states.gameplay->lastCollectedTreasurePos = shiPtr->center;
            break;
        } case RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS: {
            if(resPtr->deliveryInfo->playerTeamIdx != INVALID) {
                PlayerTeam* team =
                    &game.states.gameplay->playerTeams[
                        resPtr->deliveryInfo->playerTeamIdx
                    ];
                size_t typeIdx = resPtr->resType->sprayToConcoct;
                team->sprayStats[typeIdx].nrIngredients++;
                if(
                    team->sprayStats[typeIdx].nrIngredients >=
                    game.config.misc.sprayOrder[typeIdx]->ingredientsNeeded
                ) {
                    team->sprayStats[typeIdx].nrIngredients -=
                        game.config.misc.sprayOrder[typeIdx]->ingredientsNeeded;
                    game.states.gameplay->changeSprayCount(team, typeIdx, 1);
                    game.states.gameplay->showInventoryUpdateDisplay(
                        team,
                        game.config.misc.sprayOrder[typeIdx]->
                        manifest->internalName,
                        1
                    );
                }
            }
            break;
        } case RESOURCE_DELIVERY_RESULT_DAMAGE_MOB:
        case RESOURCE_DELIVERY_RESULT_STAY: {
            break;
        }
        }
        break;
        
    } default: {
        break;
    }
    }
    
    shiPtr->mobsBeingBeamed--;
    
    if(shiPtr->mobsBeingBeamed == 0 && shiPtr->soundBeamId != 0) {
        game.audio.destroySoundSource(shiPtr->soundBeamId);
        shiPtr->soundBeamId = 0;
    }
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parOnionInsertion, shiPtr
        );
    pg.followPosOffset = shiPtr->shiType->receptacleOffset;
    pg.followZOffset -= 2.0f; //Must appear below the ship's receptacle.
    shiPtr->particleGenerators.push_back(pg);
    
    shiPtr->playSound(shiPtr->shiType->soundReceptionIdx);
}


/**
 * @brief When a ship needs to enter its default "idling" animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ShipFsm::setAnim(ScriptVM* scriptVM, void* info1, void* info2) {
    Ship* shiPtr = (Ship*) scriptVM->mob;
    
    shiPtr->setAnimation(
        SHIP_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When a ship starts receiving a mob carried by Pikmin.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ShipFsm::startDelivery(ScriptVM* scriptVM, void* info1, void* info2) {
    Ship* shiPtr = (Ship*) scriptVM->mob;
    
    shiPtr->mobsBeingBeamed++;
    if(shiPtr->mobsBeingBeamed == 1 && shiPtr->soundBeamId == 0) {
        shiPtr->soundBeamId = shiPtr->playSound(shiPtr->shiType->soundBeamIdx);
    }
}


#pragma endregion
