/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy type class and enemy type-related functions.
 */

#include "enemy_type.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob_script/gen_mob_fsm.h"


/**
 * @brief Constructs a new enemy type object.
 */
EnemyType::EnemyType() :
    MobType(MOB_CATEGORY_ENEMIES) {
    
    targetType = MOB_TARGET_FLAG_ENEMY;
    huntableTargets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER;
    hurtableTargets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_FRAGILE;
    startingTeam = MOB_TEAM_ENEMY_1;
    useDamageSquashAndStretch = true;
}


/**
 * @brief Creates and adds carrying-related states to the FSM.
 * This should be called before any other states are loaded, since it is
 * using hardcoded state index numbers starting at 0, and trusts
 * that that will always be the case (i.e. loaded states would come after).
 */
void EnemyType::createAndAddCarryingStates() {
    EasyFsmCreator efc;
    
    efc.newState("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(GenMobFsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("carriable_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.changeState("being_delivered");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.changeState("carriable_thrown");
        }
    }
    
    efc.newState("carriable_stuck", ENEMY_EXTRA_STATE_CARRIABLE_STUCK); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.changeState("carriable_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("carriable_thrown", ENEMY_EXTRA_STATE_CARRIABLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(GenMobFsm::loseMomentum);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("carriable_moving");
        }
    }
    
    efc.newState("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(GenMobFsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(GenMobFsm::handleDelivery);
        }
    }
    
    
    vector<FsmStateDef*> newStates = efc.finish();
    scriptDef.fsm.states.insert(
        scriptDef.fsm.states.begin(),
        newStates.begin(), newStates.end()
    );
    scriptDef.fsm.compileStates();
}


/**
 * @brief Does extra processing to a loaded FSM state.
 * This creates and adds convenient events to it.
 */
void EnemyType::handleLoadedScriptState(FsmStateDef* state) {
    vector<FsmEventDef*> newEvents;
    
    //Inject a damage event.
    if(!state->events[MOB_EV_HITBOX_TOUCH_N_A]) {
        vector<ScriptActionDef*> daActions;
        daActions.push_back(
            new ScriptActionDef(GenMobFsm::beAttacked)
        );
        newEvents.push_back(
            new FsmEventDef(MOB_EV_HITBOX_TOUCH_N_A, daActions)
        );
    }
    
    //Inject a zero health event.
    if(
        state->name != dyingStateName &&
        !state->events[MOB_EV_ZERO_HEALTH] &&
        !isInContainer(statesIgnoringDeath, state->name) &&
        !dyingStateName.empty()
    ) {
        vector<ScriptActionDef*> zhActions;
        zhActions.push_back(new ScriptActionDef(GenMobFsm::goToDyingState));
        newEvents.push_back(
            new FsmEventDef(MOB_EV_ZERO_HEALTH, zhActions)
        );
    }
    
    //Inject a bottomless pit event.
    if(!state->events[MOB_EV_BOTTOMLESS_PIT]) {
        vector<ScriptActionDef*> bpActions;
        bpActions.push_back(
            new ScriptActionDef(GenMobFsm::fallDownPit)
        );
        newEvents.push_back(
            new FsmEventDef(MOB_EV_BOTTOMLESS_PIT, bpActions)
        );
    }
    
    //Inject a spray touch event.
    if(
        !state->events[MOB_EV_TOUCHED_SPRAY] &&
        !isInContainer(statesIgnoringSpray, state->name)
    ) {
        vector<ScriptActionDef*> spActions;
        spActions.push_back(
            new ScriptActionDef(GenMobFsm::touchSpray)
        );
        newEvents.push_back(
            new FsmEventDef(MOB_EV_TOUCHED_SPRAY, spActions)
        );
    }
    
    //Inject a hazard event.
    if(
        !state->events[MOB_EV_TOUCHED_HAZARD] &&
        !isInContainer(statesIgnoringHazard, state->name)
    ) {
        vector<ScriptActionDef*> haActions;
        haActions.push_back(
            new ScriptActionDef(GenMobFsm::touchHazard)
        );
        newEvents.push_back(
            new FsmEventDef(MOB_EV_TOUCHED_HAZARD, haActions)
        );
    }
    
    state->mergeEvents(newEvents, vector<Bitmask8>(newEvents.size(), 0));
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void EnemyType::loadCatProperties(DataNode* file) {
    ReaderSetter eRS(file);
    
    eRS.set("allow_ground_attacks", allowGroundAttacks);
    eRS.set("revive_time", reviveTime);
    eRS.set("pikmin_seeds", pikminSeeds);
    eRS.set("points", points);
}


/**
 * @brief Loads script data from the script's data file, before the script
 * proper is loaded.
 *
 * @param file File to read from.
 */
void EnemyType::loadCatScriptDataPre(DataNode* file) {
    ReaderSetter sRS(file);
    string statesIgnoringDeathStr;
    string statesIgnoringSprayStr;
    string statesIgnoringHazardStr;
    
    sRS.set("death_state", dyingStateName);
    sRS.set("states_ignoring_death", statesIgnoringDeathStr);
    sRS.set("states_ignoring_spray", statesIgnoringSprayStr);
    sRS.set("states_ignoring_hazard", statesIgnoringHazardStr);
    
    statesIgnoringDeath = semicolonListToVector(statesIgnoringDeathStr);
    statesIgnoringSpray = semicolonListToVector(statesIgnoringSprayStr);
    statesIgnoringHazard = semicolonListToVector(statesIgnoringHazardStr);
    
    createAndAddCarryingStates();
}


/**
 * @brief Loads script data from the script's data file, after the script
 * proper is loaded.
 *
 * @param file File to read from.
 */
void EnemyType::loadCatScriptDataPos(DataNode* file) {
    if(!dyingStateName.empty()) {
        for(size_t s = 0; s < scriptDef.fsm.states.size(); s++) {
            if(scriptDef.fsm.states[s]->name == dyingStateName) {
                dyingStateIdx = s;
                break;
            }
        }
        if(dyingStateIdx == INVALID) {
            game.errors.report(
                "Unknown state \"" + dyingStateName + "\" "
                "to set as the death state!",
                file->getChildByName("death_state")
            );
        }
    }
    
    DataNode* reviveStateNameNode =
        file->getChildByName("revive_state");
    string reviveStateName = reviveStateNameNode->value;
    
    if(!reviveStateName.empty()) {
        for(size_t s = 0; s < scriptDef.fsm.states.size(); s++) {
            if(scriptDef.fsm.states[s]->name == reviveStateName) {
                reviveStateIdx = s;
                break;
            }
        }
        if(reviveStateIdx == INVALID) {
            game.errors.report(
                "Unknown state \"" + reviveStateName + "\" "
                "to set as the revive state!",
                reviveStateNameNode
            );
        }
    } else {
        reviveStateIdx = scriptDef.fsm.firstStateIdx;
    }
}
