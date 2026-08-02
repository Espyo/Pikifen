/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile finite-state machine logic.
 */

#include <algorithm>

#include "pile_fsm.hpp"

#include "../../core/const.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/string_utils.hpp"
#include "../mob/pile.hpp"
#include "../mob/resource.hpp"
#include "gen_mob_fsm.hpp"


using std::size_t;


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the pile's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void PileFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", PILE_STATE_IDLING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PileFsm::becomeIdle);
        }
        efc.newEvent(FSM_EV_HITBOX_TOUCH_N_A); {
            efc.run(PileFsm::beAttacked);
        }
    }
    
    
    typ->scriptDef.fsm.states = efc.finish();
    typ->scriptDef.fsm.setFirstState("idling");
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->scriptDef.fsm.states.size() == N_PILE_STATES,
        i2s(typ->scriptDef.fsm.states.size()) + " registered, " +
        i2s(N_PILE_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief Handles being attacked, and checks if it must drop another
 * resource or not.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PileFsm::beAttacked(ScriptVM* scriptVM, void* info1, void* info2) {
    Pile* pilPtr = (Pile*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    GenMobFsm::beAttacked(scriptVM, info1, info2);
    
    size_t amountBefore = pilPtr->amount;
    int intendedAmount =
        ceil(pilPtr->health / pilPtr->pilType->healthPerResource);
    int amountToSpawn = (int) pilPtr->amount - intendedAmount;
    amountToSpawn = std::max((int) 0, amountToSpawn);
    
    if(amountToSpawn == 0) return;
    
    if(amountToSpawn > 1 && !pilPtr->pilType->canDropMultiple) {
        //Can't drop multiple? Let's knock that number down.
        amountToSpawn = 1;
        intendedAmount = (int) (pilPtr->amount - 1);
        pilPtr->health =
            pilPtr->pilType->healthPerResource * intendedAmount;
    }
    
    Mob* mobToPickUp = nullptr;
    Pikmin* pikminToStartCarrying = nullptr;
    
    for(size_t r = 0; r < (size_t) amountToSpawn; r++) {
        Point spawnPos;
        float spawnZ = 0;
        float spawnAngle = 0;
        float spawnHSpeed = 0;
        float spawnVSpeed = 0;
        
        if(r == 0 && info->mob2->type->category->id == MOB_CATEGORY_PIKMIN) {
            pikminToStartCarrying = (Pikmin*) (info->mob2);
            //If this was a Pikmin's attack, spawn the first resource nearby
            //so it can pick it up.
            spawnAngle =
                getAngle(pilPtr->center, pikminToStartCarrying->center);
            spawnPos =
                pikminToStartCarrying->center +
                angleToCoordinates(
                    spawnAngle, game.config.pikmin.standardRadius * 1.5
                );
        } else {
            spawnPos = pilPtr->center;
            spawnZ = pilPtr->height + 32.0f;
            spawnAngle = game.rng.f(0, TAU);
            spawnHSpeed = pilPtr->radius * 3;
            spawnVSpeed = 600.0f;
        }
        
        Mob* newMob = nullptr;
        
        if(pilPtr->pilType->contentsResource) {
            newMob =
                createMob(
                    pilPtr->pilType->contentsResource->category, spawnPos,
                    pilPtr->pilType->contentsResource, spawnAngle, ""
                );
                
        } else {
            newMob =
                pilPtr->spawn(
                    &pilPtr->type->spawns[pilPtr->pilType->contentsSpawnIdx]
                );
            newMob->center = spawnPos;
            newMob->face(spawnAngle, nullptr, true);
        }
        
        newMob->bottomZ = spawnZ;
        newMob->speed.x = cos(spawnAngle) * spawnHSpeed;
        newMob->speed.y = sin(spawnAngle) * spawnHSpeed;
        newMob->speedZ = spawnVSpeed;
        newMob->links = pilPtr->links;
        
        if(newMob->type->category->id == MOB_CATEGORY_RESOURCES) {
            ((Resource*) newMob)->originPile = pilPtr;
        }
        
        if(r == 0 && newMob->carryInfo) {
            mobToPickUp = newMob;
        }
        
        string droppedResourceMsg = "dropped_resource";
        game.states.gameplay->sendScriptMessage(
            pilPtr, pilPtr, droppedResourceMsg
        );
    }
    
    if(pikminToStartCarrying && mobToPickUp) {
        pikminToStartCarrying->forceCarry(mobToPickUp);
    }
    
    pilPtr->amount = intendedAmount;
    
    if(amountBefore == pilPtr->pilType->maxAmount) {
        pilPtr->rechargeTimer.start();
    }
    pilPtr->update();
}


/**
 * @brief When a pile starts idling.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PileFsm::becomeIdle(ScriptVM* scriptVM, void* info1, void* info2) {
    Pile* pilPtr = (Pile*) scriptVM->mob;
    
    pilPtr->update();
}


#pragma endregion
