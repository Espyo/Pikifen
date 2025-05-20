/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile finite state machine logic.
 */

#include <algorithm>

#include "pile_fsm.h"

#include "../../core/const.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/pile.h"
#include "../mob/resource.h"
#include "gen_mob_fsm.h"


using std::size_t;


/**
 * @brief Creates the finite state machine for the pile's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void PileFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idling", PILE_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(PileFsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PileFsm::beAttacked);
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_PILE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PILE_STATES) + " in enum."
    );
}


/**
 * @brief Handles being attacked, and checks if it must drop another
 * resource or not.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PileFsm::beAttacked(Mob* m, void* info1, void* info2) {
    GenMobFsm::beAttacked(m, info1, info2);
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pile* pilPtr = (Pile*) m;
    
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
    
    Resource* resourceToPickUp = nullptr;
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
                getAngle(pilPtr->pos, pikminToStartCarrying->pos);
            spawnPos =
                pikminToStartCarrying->pos +
                angleToCoordinates(
                    spawnAngle, game.config.pikmin.standardRadius * 1.5
                );
        } else {
            spawnPos = pilPtr->pos;
            spawnZ = pilPtr->height + 32.0f;
            spawnAngle = game.rng.f(0, TAU);
            spawnHSpeed = pilPtr->radius * 3;
            spawnVSpeed = 600.0f;
        }
        
        Resource* newResource =
            (
                (Resource*)
                createMob(
                    game.mobCategories.get(MOB_CATEGORY_RESOURCES),
                    spawnPos, pilPtr->pilType->contents,
                    spawnAngle, "",
        [pilPtr] (Mob * m) { ((Resource*) m)->originPile = pilPtr; }
                )
            );
            
        newResource->z = spawnZ;
        newResource->speed.x = cos(spawnAngle) * spawnHSpeed;
        newResource->speed.y = sin(spawnAngle) * spawnHSpeed;
        newResource->speedZ = spawnVSpeed;
        newResource->links = pilPtr->links;
        
        if(r == 0) {
            resourceToPickUp = newResource;
        }
    }
    
    if(pikminToStartCarrying) {
        pikminToStartCarrying->forceCarry(resourceToPickUp);
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
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PileFsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Pile* pilPtr = (Pile*) m;
    pilPtr->update();
}
