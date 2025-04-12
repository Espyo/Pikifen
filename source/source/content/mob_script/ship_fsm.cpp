/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship finite state machine logic.
 */

#include "ship_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/resource.h"
#include "../mob/ship.h"
#include "../other/particle.h"


/**
 * @brief Creates the finite state machine for the ship's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void ship_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", SHIP_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ship_fsm::setAnim);
        }
        efc.newEvent(MOB_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(ship_fsm::startDelivery);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(ship_fsm::receiveMob);
        }
    }
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_SHIP_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_SHIP_STATES) + " in enum."
    );
}


/**
 * @brief When a ship finishes receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob.
 * @param info2 Unused.
 */
void ship_fsm::receiveMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Mob* delivery = (Mob*) info1;
    Ship* shi_ptr = (Ship*) m;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_ENEMIES: {
        if(game.curAreaData->mission.enemyPointsOnCollection) {
            game.states.gameplay->enemyPointsCollected += ((Enemy*) delivery)->eneType->points;
        }
        break;
        
    }
    case MOB_CATEGORY_TREASURES: {
        Treasure* tre_ptr = (Treasure*) delivery;
        game.states.gameplay->treasuresCollected++;
        game.states.gameplay->treasurePointsCollected +=
            tre_ptr->treType->points;
        game.states.gameplay->lastShipThatGotTreasurePos = m->pos;
        
        if(game.curAreaData->mission.goal == MISSION_GOAL_COLLECT_TREASURE) {
            auto it =
                game.states.gameplay->missionRemainingMobIds.find(
                    delivery->id
                );
            if(it != game.states.gameplay->missionRemainingMobIds.end()) {
                game.states.gameplay->missionRemainingMobIds.erase(it);
                game.states.gameplay->goalTreasuresCollected++;
            }
        }
        break;
        
    } case MOB_CATEGORY_RESOURCES: {
        Resource* res_ptr = (Resource*) delivery;
        switch(res_ptr->resType->deliveryResult) {
        case RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS: {
            game.states.gameplay->treasuresCollected++;
            game.states.gameplay->treasurePointsCollected +=
                res_ptr->resType->pointAmount;
            game.states.gameplay->lastShipThatGotTreasurePos = m->pos;
            if(
                game.curAreaData->mission.goal ==
                MISSION_GOAL_COLLECT_TREASURE
            ) {
                unordered_set<size_t> &goal_mobs =
                    game.states.gameplay->missionRemainingMobIds;
                auto it = goal_mobs.find(delivery->id);
                if(it != goal_mobs.end()) {
                    goal_mobs.erase(it);
                    game.states.gameplay->goalTreasuresCollected++;
                } else if(res_ptr->originPile) {
                    it = goal_mobs.find(res_ptr->originPile->id);
                    if(it != goal_mobs.end()) {
                        game.states.gameplay->goalTreasuresCollected++;
                    }
                }
            }
            break;
        } case RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS: {
            size_t type_idx = res_ptr->resType->sprayToConcoct;
            game.states.gameplay->sprayStats[type_idx].nrIngredients++;
            if(
                game.states.gameplay->sprayStats[type_idx].nrIngredients >=
                game.config.misc.sprayOrder[type_idx]->ingredientsNeeded
            ) {
                game.states.gameplay->sprayStats[type_idx].nrIngredients -=
                    game.config.misc.sprayOrder[type_idx]->ingredientsNeeded;
                game.states.gameplay->changeSprayCount(type_idx, 1);
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
    
    shi_ptr->mobsBeingBeamed--;
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parOnionInsertion, shi_ptr
        );
    pg.followPosOffset = shi_ptr->shiType->receptacleOffset;
    pg.followZOffset -= 2.0f; //Must appear below the ship's receptacle.
    shi_ptr->particleGenerators.push_back(pg);
    
}


/**
 * @brief When a ship needs to enter its default "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ship_fsm::setAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(
        SHIP_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When a ship starts receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ship_fsm::startDelivery(Mob* m, void* info1, void* info2) {
    Ship* shi_ptr = (Ship*) m;
    shi_ptr->mobsBeingBeamed++;
}
