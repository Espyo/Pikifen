/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * All script action classes and related functions.
 */

#include <algorithm>

#include "action_types.h"

#include "../../content/mob/group_task.h"
#include "../../content/mob/scale.h"
#include "../../content/mob/tool.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::set;


#pragma region Action instance run data


/**
 * @brief Constructs a new script action run data object.
 *
 * @param scriptVM The script VM responsible.
 * @param actionDef The action's definition.
 */
ScriptActionInstRunData::ScriptActionInstRunData(
    ScriptVM* scriptVM, ScriptActionDef* actionDef
) :
    scriptVM(scriptVM),
    actionDef(actionDef) {
    
}


#pragma endregion
#pragma region Action loaders


/**
 * @brief Loading code for the arachnorb logic plan script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::arachnorbPlanLogic(
    ScriptActionDef& def, MobType* mt
) {
    bool found;
    SCRIPT_ACTION_ARACHNORB_PLAN_LOGIC_TYPE type =
        enumGetValue(
            scriptActionArachnorbPlanLogicTypeINames, def.args[0], &found
        );
    if(!found) {
        reportEnumError(def, 0);
        return false;
    }
    def.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the calculation script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::calculate(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_CALCULATE_TYPE type =
        enumGetValue(scriptActionCalculateTypeINames, def.args[2], &found);
    if(!found) {
        reportEnumError(def, 2);
        return false;
    }
    def.args[2] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the ease number script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::easeNumber(ScriptActionDef& def, MobType* mt) {
    bool found;
    EASE_METHOD method =
        enumGetValue(easeMethodINames, def.args[2], &found);
    if(!found) {
        reportEnumError(def, 2);
        return false;
    }
    def.args[2] = i2s(method);
    return true;
}


/**
 * @brief Loading code for the focus script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::focus(ScriptActionDef& def, MobType* mt) {
    return loadMobTargetType(def, 0);
}


/**
 * @brief Loading code for the follow mob as leader script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::followMobAsLeader(ScriptActionDef& def, MobType* mt) {
    return loadMobTargetType(def, 0);
}


/**
 * @brief Loading code for the area info getting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::getAreaInfo(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_GET_AREA_INFO_TYPE type =
        enumGetValue(scriptActionGetAreaInfoTypeINames, def.args[1], &found);
    if(!found) {
        def.customError =
            "Unknown info type \"" + def.args[1] + "\"! "
            "Did you mean to use a different \"get_*_info\" action?";
        return false;
    }
    def.args[1] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the event info getting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::getEventInfo(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_GET_EV_INFO_TYPE type =
        enumGetValue(scriptActionGetEvInfoTypeINames, def.args[1], &found);
    if(!found) {
        def.customError =
            "Unknown info type \"" + def.args[1] + "\"! "
            "Did you mean to use a different \"get_*_info\" action?";
        return false;
    }
    def.args[1] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the misc. info getting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::getMiscInfo(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_GET_MISC_INFO_TYPE type =
        enumGetValue(scriptActionGetMiscInfoTypeINames, def.args[1], &found);
    if(!found) {
        def.customError =
            "Unknown info type \"" + def.args[1] + "\"! "
            "Did you mean to use a different \"get_*_info\" action?";
        return false;
    }
    def.args[1] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the mob info getting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::getMobInfo(ScriptActionDef& def, MobType* mt) {
    if(!loadMobTargetType(def, 1)) {
        return false;
    }
    
    bool found;
    SCRIPT_ACTION_GET_MOB_INFO_TYPE type =
        enumGetValue(scriptActionGetMobInfoTypeINames, def.args[2], &found);
    if(!found) {
        def.customError =
            "Unknown info type \"" + def.args[2] + "\"! "
            "Did you mean to use a different \"get_*_info\" action?";
        return false;
    }
    def.args[2] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the hold focused mob script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::holdFocus(ScriptActionDef& def, MobType* mt) {
    size_t pIdx = mt->animDb->findBodyPart(def.args[0]);
    if(pIdx == INVALID) {
        def.customError =
            "Unknown body part \"" + def.args[0] + "\"!";
        return false;
    }
    def.args[0] = i2s(pIdx);
    return true;
}


/**
 * @brief Loading code for the "if" script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::ifFunction(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_IF_OP op =
        enumGetValue(scriptActionIfOpINames, def.args[1], &found);
    if(!found) {
        reportEnumError(def, 1);
        return false;
    }
    def.args[1] = i2s(op);
    return true;
}


/**
 * @brief Loads a mob target type from an action call.
 *
 * @param def The action's definition.
 * @param argIdx Index number of the mob target type argument.
 */
bool ScriptActionLoaders::loadMobTargetType(
    ScriptActionDef& def, size_t argIdx
) {
    bool found;
    SCRIPT_ACTION_MOB_TARGET_TYPE type =
        enumGetValue(scriptActionMobTargetTypeINames, def.args[argIdx], &found);
    if(!found) {
        reportEnumError(def, argIdx);
        return false;
    }
    def.args[argIdx] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the move to target script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::moveToTarget(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_MOVE_TYPE type =
        enumGetValue(scriptActionMoveTypeINames, def.args[0], &found);
    if(!found) {
        reportEnumError(def, 0);
        return false;
    }
    def.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the sound playing script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::playSound(ScriptActionDef& def, MobType* mt) {
    forIdx(s, mt->sounds) {
        if(mt->sounds[s].name == def.args[0]) {
            def.args[0] = i2s(s);
            return true;
        }
    }
    def.customError =
        "Unknown sound info block \"" + def.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the status reception script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::receiveStatus(ScriptActionDef& def, MobType* mt) {
    if(!isInMap(game.content.statusTypes.list, def.args[0])) {
        def.customError =
            "Unknown status effect \"" + def.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the status removal script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::removeStatus(ScriptActionDef& def, MobType* mt) {
    if(!isInMap(game.content.statusTypes.list, def.args[0])) {
        def.customError =
            "Unknown status effect \"" + def.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Reports an error of an unknown enum value.
 *
 * @param def The action's definition.
 * @param argIdx Index number of the argument that is an enum.
 */
void ScriptActionLoaders::reportEnumError(
    ScriptActionDef& def, size_t argIdx
) {
    size_t paramIdx = std::min(argIdx, def.actionType->parameters.size() - 1);
    def.customError =
        "The parameter \"" + def.actionType->parameters[paramIdx].name + "\" "
        "does not know what the value \"" +
        def.args[argIdx] + "\" means!";
}


/**
 * @brief Loading code for the animation setting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::setAnimation(ScriptActionDef& def, MobType* mt) {
    size_t aPos = mt->animDb->findAnimation(def.args[0]);
    if(aPos == INVALID) {
        def.customError =
            "Unknown animation \"" + def.args[0] + "\"!";
        return false;
    }
    def.args[0] = i2s(aPos);
    
    if(def.args.size() > 1) {
        bool optionFound;
        START_ANIM_OPTION option =
            enumGetValue(startAnimOptionINames, def.args[1], &optionFound);
        if(!optionFound) {
            def.customError =
                "Unknown animation start option \"" + def.args[1] + "\"!";
            return false;
        }
        def.args[1] = i2s(option);
    }
    
    return true;
}


/**
 * @brief Loading code for the far reach setting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::setFarReach(ScriptActionDef& def, MobType* mt) {
    forIdx(r, mt->reaches) {
        if(mt->reaches[r].name == def.args[0]) {
            def.args[0] = i2s(r);
            return true;
        }
    }
    def.customError = "Unknown reach \"" + def.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the holdable setting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::setHoldable(ScriptActionDef& def, MobType* mt) {
    forIdx(a, def.args) {
        bool found;
        HOLDABILITY_FLAG flag =
            enumGetValue(holdabilityFlagINames, def.args[a], &found);
        if(!found) {
            reportEnumError(def, a);
            return false;
        }
        def.args[a] = i2s(flag);
    }
    return true;
}


/**
 * @brief Loading code for the near reach setting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::setNearReach(ScriptActionDef& def, MobType* mt) {
    forIdx(r, mt->reaches) {
        if(mt->reaches[r].name == def.args[0]) {
            def.args[0] = i2s(r);
            return true;
        }
    }
    def.customError = "Unknown reach \"" + def.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the team setting script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::setTeam(ScriptActionDef& def, MobType* mt) {
    bool found;
    MOB_TEAM teamNr = enumGetValue(mobTeamINames, def.args[0], &found);
    if(!found) {
        reportEnumError(def, 0);
        return false;
    }
    def.args[0] = i2s(teamNr);
    return true;
}


/**
 * @brief Loading code for the spawning script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::spawn(ScriptActionDef& def, MobType* mt) {
    forIdx(s, mt->spawns) {
        if(mt->spawns[s].name == def.args[0]) {
            def.args[0] = i2s(s);
            return true;
        }
    }
    def.customError =
        "Unknown spawn info block \"" + def.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the z stabilization script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::stabilizeZ(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_STABILIZE_Z_TYPE type =
        enumGetValue(scriptActionStabilizeZTypeINames, def.args[0], &found);
    if(!found) {
        reportEnumError(def, 0);
        return false;
    }
    def.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the chomping start script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::startChomping(ScriptActionDef& def, MobType* mt) {
    for(size_t s = 1; s < def.args.size(); s++) {
        size_t pNr = mt->animDb->findBodyPart(def.args[s]);
        if(pNr == INVALID) {
            def.customError =
                "Unknown body part \"" + def.args[s] + "\"!";
            return false;
        }
        def.args[s] = i2s(pNr);
    }
    return true;
}


/**
 * @brief Loading code for the particle start script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::startParticles(ScriptActionDef& def, MobType* mt) {
    if(!isInMap(game.content.particleGens.list, def.args[0])) {
        def.customError =
            "Unknown particle generator \"" + def.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the turn to target script action type.
 *
 * @param def The action's definition.
 * @param mt Mob type it belongs to, if any.
 * @return Whether it succeeded.
 */
bool ScriptActionLoaders::turnToTarget(ScriptActionDef& def, MobType* mt) {
    bool found;
    SCRIPT_ACTION_TURN_TYPE type =
        enumGetValue(scriptActionTurnTypeINames, def.args[0], &found);
    if(!found) {
        reportEnumError(def, 0);
        return false;
    }
    def.args[0] = i2s(type);
    return true;
}


#pragma endregion
#pragma region Action runners


/**
 * @brief Code for the absolute number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::absoluteNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] = f2s(fabs(s2f(data.args[1])));
}


/**
 * @brief Code for the health addition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::addHealth(ScriptActionInstRunData& data) {
    data.scriptVM->mob->setHealth(true, false, s2f(data.args[0]));
}


/**
 * @brief Code for the arachnorb logic plan script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::arachnorbPlanLogic(ScriptActionInstRunData& data) {
    data.scriptVM->mob->arachnorbPlanLogic(
        (SCRIPT_ACTION_ARACHNORB_PLAN_LOGIC_TYPE) s2i(data.args[0])
    );
}


/**
 * @brief Code for the calculation script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::calculate(ScriptActionInstRunData& data) {
    float lhs = s2f(data.args[1]);
    SCRIPT_ACTION_CALCULATE_TYPE op =
        (SCRIPT_ACTION_CALCULATE_TYPE) s2i(data.args[2]);
    float rhs = s2f(data.args[3]);
    float result = 0;
    
    switch(op) {
    case SCRIPT_ACTION_CALCULATE_TYPE_SUM: {
        result = lhs + rhs;
        break;
        
    } case SCRIPT_ACTION_CALCULATE_TYPE_SUBTRACT: {
        result = lhs - rhs;
        break;
        
    } case SCRIPT_ACTION_CALCULATE_TYPE_MULTIPLY: {
        result = lhs * rhs;
        break;
        
    } case SCRIPT_ACTION_CALCULATE_TYPE_DIVIDE: {
        if(rhs == 0) {
            result = 0;
        } else {
            result = lhs / rhs;
        }
        break;
        
    } case SCRIPT_ACTION_CALCULATE_TYPE_MODULO: {
        if(rhs == 0) {
            result = 0;
        } else {
            result = fmod(lhs, rhs);
        }
        break;
        
    } case SCRIPT_ACTION_CALCULATE_TYPE_POWER: {
        result = pow(lhs, rhs);
        break;
        
    }
    }
    
    data.scriptVM->vars[data.args[0]] = f2s(result);
}


/**
 * @brief Code for the ceil number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::ceilNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] = f2s(ceil(s2f(data.args[1])));
}


/**
 * @brief Code for the deletion script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::deleteFunction(ScriptActionInstRunData& data) {
    data.scriptVM->mob->toDelete = true;
}


/**
 * @brief Code for the liquid draining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::drainLiquid(ScriptActionInstRunData& data) {
    Sector* sPtr = getSector(data.scriptVM->mob->pos, nullptr, true);
    if(!sPtr) return;
    if(!sPtr->liquid) return;
    sPtr->liquid->startDraining();
}


/**
 * @brief Code for the ease number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::easeNumber(ScriptActionInstRunData& data) {
    EASE_METHOD method =
        (EASE_METHOD) s2i(data.args[2]);
    data.scriptVM->vars[data.args[0]] = f2s(ease(s2f(data.args[1]), method));
}


/**
 * @brief Code for the death finish script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::finishDying(ScriptActionInstRunData& data) {
    data.scriptVM->mob->finishDying();
}


/**
 * @brief Code for the floor number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::floorNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] = f2s(floor(s2f(data.args[1])));
}


/**
 * @brief Code for the focus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::focus(ScriptActionInstRunData& data) {
    SCRIPT_ACTION_MOB_TARGET_TYPE s =
        (SCRIPT_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    Mob* target = getTargetMob(data, s);
    
    if(!target) return;
    
    data.scriptVM->focusOnMob(target);
}


/**
 * @brief Code for the follow mob as leader script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followMobAsLeader(ScriptActionInstRunData& data) {
    SCRIPT_ACTION_MOB_TARGET_TYPE s =
        (SCRIPT_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    Mob* target = getTargetMob(data, s);
    bool silent = false;
    if(data.args.size() >= 2) {
        silent = s2b(data.args[1]);
    }
    
    if(!target) return;
    if(target->health <= 0.0f) return;
    
    data.scriptVM->mob->leaveGroup();
    
    if(data.scriptVM->mob->type->category->id == MOB_CATEGORY_PIKMIN) {
        data.scriptVM->fsm.runEvent(
            FSM_EV_WHISTLED, (void*) target, (void*) silent
        );
    } else {
        target->addToGroup(data.scriptVM->mob);
    }
}


/**
 * @brief Code for the follow path randomly script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followPathRandomly(ScriptActionInstRunData& data) {
    string label;
    if(data.args.size() >= 1) {
        label = data.args[0];
    }
    
    //We need to decide what the final stop is going to be.
    //First, get all eligible stops.
    vector<PathStop*> choices;
    if(label.empty()) {
        //If there's no label, then any stop is eligible.
        choices.insert(
            choices.end(),
            game.curArea->pathStops.begin(),
            game.curArea->pathStops.end()
        );
    } else {
        //If there's a label, we should only pick stops that have the label.
        forIdx(s, game.curArea->pathStops) {
            PathStop* sPtr = game.curArea->pathStops[s];
            if(sPtr->label == label) {
                choices.push_back(sPtr);
            }
        }
    }
    
    //Pick a stop from the choices at random, but make sure we don't
    //pick a stop that the mob is practically on already.
    PathStop* finalStop = nullptr;
    if(!choices.empty()) {
        size_t tries = 0;
        while(!finalStop && tries < 5) {
            size_t c = game.rng.i(0, (int) choices.size() - 1);
            if(
                Distance(choices[c]->pos, data.scriptVM->mob->pos) >
                PATHS::DEF_CHASE_TARGET_DISTANCE
            ) {
                finalStop = choices[c];
                break;
            }
            tries++;
        }
    }
    
    //Go! Though if something went wrong, make it follow a path to nowhere,
    //so it can emit the FSM_EV_REACHED_DESTINATION event, and hopefully
    //make it clear that there was an error.
    PathFollowSettings settings;
    settings.targetPoint = finalStop ? finalStop->pos : data.scriptVM->mob->pos;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    settings.label = label;
    data.scriptVM->mob->followPath(
        settings, data.scriptVM->mob->getBaseSpeed(),
        data.scriptVM->mob->type->acceleration
    );
}


/**
 * @brief Code for the follow path to absolute script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followPathToAbsolute(ScriptActionInstRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    
    PathFollowSettings settings;
    settings.targetPoint = Point(x, y);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    if(data.args.size() >= 3) {
        settings.label = data.args[2];
    }
    
    data.scriptVM->mob->followPath(
        settings, data.scriptVM->mob->getBaseSpeed(),
        data.scriptVM->mob->type->acceleration
    );
}


/**
 * @brief Code for the angle obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngle(ScriptActionInstRunData& data) {
    float centerX = s2f(data.args[1]);
    float centerY = s2f(data.args[2]);
    float focusX = s2f(data.args[3]);
    float focusY = s2f(data.args[4]);
    float angle = getAngle(Point(centerX, centerY), Point(focusX, focusY));
    angle = radToDeg(angle);
    data.scriptVM->vars[data.args[0]] = f2s(angle);
}


/**
 * @brief Code for the angle closest difference obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngleCwDiff(ScriptActionInstRunData& data) {
    float angle1 = degToRad(s2f(data.args[1]));
    float angle2 = degToRad(s2f(data.args[2]));
    float diff = ::getAngleCwDiff(angle1, angle2);
    diff = radToDeg(diff);
    data.scriptVM->vars[data.args[0]] = f2s(diff);
}


/**
 * @brief Code for the angle smallest difference obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngleSmallestDiff(ScriptActionInstRunData& data) {
    float angle1 = degToRad(s2f(data.args[1]));
    float angle2 = degToRad(s2f(data.args[2]));
    float diff = ::getAngleSmallestDiff(angle1, angle2);
    diff = radToDeg(diff);
    data.scriptVM->vars[data.args[0]] = f2s(diff);
}



/**
 * @brief Code for the area info obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAreaInfo(ScriptActionInstRunData& data) {
    string* var = &(data.scriptVM->vars[data.args[0]]);
    SCRIPT_ACTION_GET_AREA_INFO_TYPE t =
        (SCRIPT_ACTION_GET_AREA_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case SCRIPT_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES: {
        *var = i2s(game.states.gameplay->dayMinutes);
        break;
        
    } case SCRIPT_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN: {
        *var = i2s(game.states.gameplay->mobs.pikmin.size());
        break;
        
    }
    }
}


/**
 * @brief Code for the getting chomped script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getChomped(ScriptActionInstRunData& data) {
    if(data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_EAT) {
        ((Mob*) (data.customData1))->chomp(
            data.scriptVM->mob,
            (Hitbox*) (data.customData2)
        );
    }
}


/**
 * @brief Code for the coordinate from angle obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getCoordinatesFromAngle(
    ScriptActionInstRunData& data
) {
    float angle = s2f(data.args[2]);
    angle = degToRad(angle);
    float magnitude = s2f(data.args[3]);
    Point p = angleToCoordinates(angle, magnitude);
    data.scriptVM->vars[data.args[0]] = f2s(p.x);
    data.scriptVM->vars[data.args[1]] = f2s(p.y);
}


/**
 * @brief Code for the distance obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getDistance(ScriptActionInstRunData& data) {
    float centerX = s2f(data.args[1]);
    float centerY = s2f(data.args[2]);
    float focusX = s2f(data.args[3]);
    float focusY = s2f(data.args[4]);
    data.scriptVM->vars[data.args[0]] =
        f2s(
            Distance(Point(centerX, centerY), Point(focusX, focusY)).toFloat()
        );
}


/**
 * @brief Code for the event info obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getEventInfo(ScriptActionInstRunData& data) {
    string* var = &(data.scriptVM->vars[data.args[0]]);
    SCRIPT_ACTION_GET_EV_INFO_TYPE t =
        (SCRIPT_ACTION_GET_EV_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case SCRIPT_ACTION_GET_EV_INFO_TYPE_BODY_PART: {
        if(
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_A_N ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_A ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_N ||
            data.actionDef->parentEvent == FSM_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h1->bodyPartName;
        } else if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OBJECT ||
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OPPONENT ||
            data.actionDef->parentEvent == FSM_EV_THROWN_PIKMIN_LANDED
        ) {
            if(data.scriptVM->mob) {
                *var =
                    data.scriptVM->mob->getClosestHitbox(
                        ((Mob*)(data.customData1))->pos
                    )->bodyPartName;
            }
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL: {
        if(data.actionDef->parentEvent == FSM_EV_FRAME_SIGNAL) {
            *var = i2s(*((size_t*)(data.customData1)));
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_HAZARD: {
        if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_HAZARD ||
            data.actionDef->parentEvent == FSM_EV_LEFT_HAZARD
        ) {
            *var = ((Hazard*)data.customData1)->manifest->internalName;
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_INPUT_NAME: {
        if(data.actionDef->parentEvent == FSM_EV_INPUT_RECEIVED) {
            PLAYER_ACTION_TYPE playerActionTypeId =
                (PLAYER_ACTION_TYPE)
                ((Inpution::Action*) (data.customData1))->actionTypeId;
            *var =
                game.controls.getActionTypeById(
                    playerActionTypeId
                ).internalName;
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.actionDef->parentEvent == FSM_EV_INPUT_RECEIVED) {
            *var = f2s(((Inpution::Action*) (data.customData1))->value);
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_MESSAGE: {
        if(data.actionDef->parentEvent == FSM_EV_RECEIVE_MESSAGE) {
            *var = *((string*)(data.customData1));
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART: {
        if(
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_A_N ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_A ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_N ||
            data.actionDef->parentEvent == FSM_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h2->bodyPartName;
        } else if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OBJECT ||
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OPPONENT ||
            data.actionDef->parentEvent == FSM_EV_THROWN_PIKMIN_LANDED
        ) {
            if(data.customData1 && data.scriptVM->mob) {
                *var =
                    ((Mob*)(data.customData1))->getClosestHitbox(
                        data.scriptVM->mob->pos
                    )->bodyPartName;
            }
        }
        break;
        
    }
    }
}


/**
 * @brief Code for the floor Z obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getFloorZ(ScriptActionInstRunData& data) {
    float x = s2f(data.args[1]);
    float y = s2f(data.args[2]);
    Sector* s = getSector(Point(x, y), nullptr, true);
    data.scriptVM->vars[data.args[0]] = f2s(s ? s->z : 0);
}


/**
 * @brief Code for the focused mob var getting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getFocusVar(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) return;
    data.scriptVM->vars[data.args[0]] =
        data.scriptVM->focusedMob->scriptVM.vars[data.args[1]];
}


/**
 * @brief Code for the misc info obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMiscInfo(ScriptActionInstRunData& data) {
    string* var = &(data.scriptVM->vars[data.args[0]]);
    SCRIPT_ACTION_GET_MISC_INFO_TYPE t =
        (SCRIPT_ACTION_GET_MISC_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case SCRIPT_ACTION_GET_MISC_INFO_TYPE_DELTA_T: {
        *var = f2s(game.deltaT);
        break;
        
    }
    }
}


/**
 * @brief Code for the mob info obtaining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMobInfo(ScriptActionInstRunData& data) {
    SCRIPT_ACTION_MOB_TARGET_TYPE s =
        (SCRIPT_ACTION_MOB_TARGET_TYPE) s2i(data.args[1]);
    Mob* target = getTargetMob(data, s);
    
    if(!target) return;
    
    string* var = &(data.scriptVM->vars[data.args[0]]);
    SCRIPT_ACTION_GET_MOB_INFO_TYPE t =
        (SCRIPT_ACTION_GET_MOB_INFO_TYPE) s2i(data.args[2]);
        
    switch(t) {
    case SCRIPT_ACTION_GET_MOB_INFO_TYPE_ANGLE: {
        *var = f2s(radToDeg(target->angle));
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN: {
        *var = i2s(target->chompingMobs.size());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->scriptVM.focusedMob) {
            float d =
                Distance(target->pos, target->scriptVM.focusedMob->pos).
                toFloat();
            *var = f2s(d);
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER: {
        if(target->type->category->id == MOB_CATEGORY_GROUP_TASKS) {
            *var = f2s(((GroupTask*)target)->getPower());
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_HEALTH: {
        *var = i2s(target->health);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO: {
        if(target->maxHealth != 0.0f) {
            *var = f2s(target->health / target->maxHealth);
        } else {
            *var = "0";
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_ID: {
        *var = i2s(target->id);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN: {
        *var = i2s(target->getLatchedPikminAmount());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT: {
        *var = i2s(target->getLatchedPikminWeight());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY: {
        *var = target->type->category->internalName;
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE: {
        if(target->type->manifest) {
            *var = target->type->manifest->internalName;
        } else {
            *var = "";
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_STATE: {
        *var = target->scriptVM.fsm.curState->name;
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_WEIGHT: {
        if(target->type->category->id == MOB_CATEGORY_SCALES) {
            Scale* sPtr = (Scale*)(target);
            *var = i2s(sPtr->calculateCurWeight());
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_X: {
        *var = f2s(target->pos.x);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Y: {
        *var = f2s(target->pos.y);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Z: {
        *var = f2s(target->z);
        break;
    }
    }
}


/**
 * @brief Code for the float number randomization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getRandomFloat(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] =
        f2s(game.rng.f(s2f(data.args[1]), s2f(data.args[2])));
}


/**
 * @brief Code for the integer number randomization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getRandomInt(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] =
        i2s(game.rng.i(s2i(data.args[1]), s2i(data.args[2])));
}


/**
 * @brief Code for the hold focused mob script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::holdFocus(ScriptActionInstRunData& data) {
    if(data.scriptVM->focusedMob) {
        data.scriptVM->mob->hold(
            data.scriptVM->focusedMob, HOLD_TYPE_PURPOSE_GENERAL,
            s2i(data.args[0]), 0.0f, 0.0f, 0.5f,
            data.args.size() >= 2 ? s2b(data.args[1]) : false,
            HOLD_ROTATION_METHOD_COPY_HOLDER
        );
    }
}


/**
 * @brief Code for the "if" script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::ifFunction(ScriptActionInstRunData& data) {
    string lhs = data.args[0];
    SCRIPT_ACTION_IF_OP op =
        (SCRIPT_ACTION_IF_OP) s2i(data.args[1]);
    string rhs = vectorTailToString(data.args, 2);
    
    switch(op) {
    case SCRIPT_ACTION_IF_OP_EQUAL: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.returnValue = (s2f(lhs) == s2f(rhs));
        } else {
            data.returnValue = (lhs == rhs);
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_NOT: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.returnValue = (s2f(lhs) != s2f(rhs));
        } else {
            data.returnValue = (lhs != rhs);
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS: {
        data.returnValue = (s2f(lhs) < s2f(rhs));
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE: {
        data.returnValue = (s2f(lhs) > s2f(rhs));
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS_E: {
        data.returnValue = (s2f(lhs) <= s2f(rhs));
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE_E: {
        data.returnValue = (s2f(lhs) >= s2f(rhs));
        break;
        
    }
    }
}


/**
 * @brief Code for the interpolate number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::interpolateNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] =
        f2s(
            ::interpolateNumber(
                s2f(data.args[1]), s2f(data.args[2]), s2f(data.args[3]),
                s2f(data.args[4]), s2f(data.args[5])
            )
        );
}


/**
 * @brief Code for the link with focus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::linkWithFocus(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    forIdx(l, data.scriptVM->mob->links) {
        if(data.scriptVM->mob->links[l] == data.scriptVM->focusedMob) {
            //Already linked.
            return;
        }
    }
    
    data.scriptVM->mob->links.push_back(data.scriptVM->focusedMob);
}


/**
 * @brief Code for the load focused mob memory script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::loadFocusMemory(ScriptActionInstRunData& data) {
    if(data.scriptVM->mob->focusedMobMemory.empty()) {
        return;
    }
    
    data.scriptVM->focusOnMob(
        data.scriptVM->mob->focusedMobMemory[s2i(data.args[0])]
    );
}


/**
 * @brief Code for the move to absolute coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::moveToAbsolute(ScriptActionInstRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = data.args.size() > 2 ? s2f(data.args[2]) : data.scriptVM->mob->z;
    data.scriptVM->mob->chase(
        Point(x, y), z,
        CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
    );
}


/**
 * @brief Code for the move to relative coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::moveToRelative(ScriptActionInstRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = (data.args.size() > 2 ? s2f(data.args[2]) : 0);
    Point p = rotatePoint(Point(x, y), data.scriptVM->mob->angle);
    data.scriptVM->mob->chase(
        data.scriptVM->mob->pos + p, data.scriptVM->mob->z + z,
        CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
    );
}


/**
 * @brief Code for the move to target script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::moveToTarget(ScriptActionInstRunData& data) {
    SCRIPT_ACTION_MOVE_TYPE t = (SCRIPT_ACTION_MOVE_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case SCRIPT_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.scriptVM->focusedMob) {
            float a =
                getAngle(
                    data.scriptVM->mob->pos,
                    data.scriptVM->focusedMob->pos
                );
            Point offset = Point(2000, 0);
            offset = rotatePoint(offset, a + TAU / 2.0);
            data.scriptVM->mob->chase(
                data.scriptVM->mob->pos + offset,
                data.scriptVM->mob->z,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->mob->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_FOCUS: {
        if(data.scriptVM->focusedMob) {
            data.scriptVM->mob->chase(
                &data.scriptVM->focusedMob->pos,
                &data.scriptVM->focusedMob->z,
                Point(), 0.0f,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->mob->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.scriptVM->focusedMob) {
            data.scriptVM->mob->chase(
                data.scriptVM->focusedMob->pos,
                data.scriptVM->focusedMob->z,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->mob->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_HOME: {
        data.scriptVM->mob->chase(
            data.scriptVM->mob->home,
            data.scriptVM->mob->z,
            CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
        );
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC: {
        data.scriptVM->mob->arachnorbFootMoveLogic();
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE: {
        if(data.scriptVM->mob->links.empty()) {
            return;
        }
        
        Point des;
        forIdx(l, data.scriptVM->mob->links) {
            if(!data.scriptVM->mob->links[l]) continue;
            des += data.scriptVM->mob->links[l]->pos;
        }
        des = des / data.scriptVM->mob->links.size();
        
        data.scriptVM->mob->chase(
            des,
            data.scriptVM->mob->z,
            CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
        );
        break;
        
    }
    }
}


/**
 * @brief Code for the release order script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::orderRelease(ScriptActionInstRunData& data) {
    Mob* holder = data.scriptVM->mob->holder.m;
    if(holder) {
        holder->scriptVM.fsm.runEvent(FSM_EV_RELEASE_ORDER, nullptr, nullptr);
    }
}


/**
 * @brief Code for the sound playing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::playSound(ScriptActionInstRunData& data) {
    size_t soundId = data.scriptVM->mob->playSound(s2i(data.args[0]));
    if(data.args.size() >= 2) {
        data.scriptVM->setVar(data.args[1], i2s(soundId));
    }
}


/**
 * @brief Code for the text printing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::print(ScriptActionInstRunData& data) {
    size_t seconds = floor(game.states.gameplay->gameplayTimePassed);
    size_t centiseconds =
        (game.states.gameplay->gameplayTimePassed - seconds) * 100;
    string timestamp =
        resizeString(i2s(seconds), 4, true, true, true, ' ') + "." +
        resizeString(i2s(centiseconds), 2, true, true, true, '0');
        
    string scriptText = vectorTailToString(data.args, 0);
    string speaker =
        data.scriptVM->mob ?
        data.scriptVM->mob->type->name :
        "Area";
    game.states.gameplay->printActionLogLines.push_back(
        "[@" + timestamp + "s " + speaker + " said:] " + scriptText
    );
    if(game.states.gameplay->printActionLogLines.size() > 10) {
        game.states.gameplay->printActionLogLines.erase(
            game.states.gameplay->printActionLogLines.begin()
        );
    }
    
    string log;
    forIdx(l, game.states.gameplay->printActionLogLines) {
        log += "\n" + game.states.gameplay->printActionLogLines[l];
    }
    
    game.console.write("=== DEBUG SCRIPT PRINTS ===" + log, 15.0f);
}


/**
 * @brief Code for the status reception script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::receiveStatus(ScriptActionInstRunData& data) {
    data.scriptVM->mob->applyStatus(
        game.content.statusTypes.list[data.args[0]], false, false
    );
}


/**
 * @brief Code for the release script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::release(ScriptActionInstRunData& data) {
    data.scriptVM->mob->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::releaseStoredMobs(ScriptActionInstRunData& data) {
    data.scriptVM->mob->releaseStoredMobs();
}


/**
 * @brief Code for the status removal script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::removeStatus(ScriptActionInstRunData& data) {
    forIdx(s, data.scriptVM->mob->statuses) {
        if(
            data.scriptVM->mob->statuses[s].type->manifest->internalName ==
            data.args[0]
        ) {
            data.scriptVM->mob->statuses[s].prevState =
                data.scriptVM->mob->statuses[s].state;
            data.scriptVM->mob->statuses[s].state = STATUS_STATE_TO_DELETE;
        }
    }
}


/**
 * @brief Code for the round number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::roundNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] = f2s(round(s2f(data.args[1])));
}


/**
 * @brief Code for the save focused mob memory script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::saveFocusMemory(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    data.scriptVM->mob->focusedMobMemory[s2i(data.args[0])] =
        data.scriptVM->focusedMob;
}


/**
 * @brief Code for the focused mob message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToFocus(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) return;
    game.states.gameplay->sendScriptMessage(
        data.scriptVM->mob, data.scriptVM->focusedMob, data.args[0]
    );
}


/**
 * @brief Code for the linked mob message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToLinks(ScriptActionInstRunData& data) {
    forIdx(l, data.scriptVM->mob->links) {
        if(data.scriptVM->mob->links[l] == data.scriptVM->mob) continue;
        if(!data.scriptVM->mob->links[l]) continue;
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->mob, data.scriptVM->mob->links[l], data.args[0]
        );
    }
}


/**
 * @brief Code for the nearby mob message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToNearby(ScriptActionInstRunData& data) {
    float d = s2f(data.args[0]);
    
    forIdx(m2, game.states.gameplay->mobs.all) {
        if(game.states.gameplay->mobs.all[m2] == data.scriptVM->mob) {
            continue;
        }
        if(
            Distance(
                data.scriptVM->mob->pos,
                game.states.gameplay->mobs.all[m2]->pos
            ) > d
        ) {
            continue;
        }
        
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->mob, game.states.gameplay->mobs.all[m2], data.args[1]
        );
    }
}


/**
 * @brief Code for the animation setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setAnimation(ScriptActionInstRunData& data) {
    START_ANIM_OPTION options = START_ANIM_OPTION_NORMAL;
    float mobSpeedBaseline = 0.0f;
    if(data.args.size() > 1) {
        options = (START_ANIM_OPTION) s2i(data.args[1]);
    }
    if(data.args.size() > 2) {
        if(s2b(data.args[2])) {
            mobSpeedBaseline = data.scriptVM->mob->type->moveSpeed;
        };
    }
    
    data.scriptVM->mob->setAnimation(
        s2i(data.args[0]), options, false, mobSpeedBaseline
    );
}


/**
 * @brief Code for the block paths setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setCanBlockPaths(ScriptActionInstRunData& data) {
    data.scriptVM->mob->setCanBlockPaths(s2b(data.args[0]));
}


/**
 * @brief Code for the far reach setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFarReach(ScriptActionInstRunData& data) {
    data.scriptVM->mob->farReach = s2i(data.args[0]);
    data.scriptVM->mob->updateInteractionSpan();
}


/**
 * @brief Code for the flying setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFlying(ScriptActionInstRunData& data) {
    if(s2b(data.args[0])) {
        enableFlag(data.scriptVM->mob->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    } else {
        disableFlag(data.scriptVM->mob->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Code for the focused mob var setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFocusVar(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) return;
    data.scriptVM->focusedMob->scriptVM.vars[data.args[0]] = data.args[1];
}


/**
 * @brief Code for the gravity setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setGravity(ScriptActionInstRunData& data) {
    data.scriptVM->mob->gravityMult = s2f(data.args[0]);
}


/**
 * @brief Code for the health setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHealth(ScriptActionInstRunData& data) {
    data.scriptVM->mob->setHealth(false, false, s2f(data.args[0]));
}


/**
 * @brief Code for the height setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHeight(ScriptActionInstRunData& data) {
    data.scriptVM->mob->height = s2f(data.args[0]);
    
    if(data.scriptVM->mob->type->walkable) {
        //Update the Z of mobs standing on top of it.
        forIdx(m, game.states.gameplay->mobs.all) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m];
            if(m2Ptr->standingOnMob == data.scriptVM->mob) {
                m2Ptr->z = data.scriptVM->mob->z + data.scriptVM->mob->height;
            }
        }
    }
}


/**
 * @brief Code for the hiding setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHiding(ScriptActionInstRunData& data) {
    if(s2b(data.args[0])) {
        enableFlag(data.scriptVM->mob->flags, MOB_FLAG_HIDDEN);
    } else {
        disableFlag(data.scriptVM->mob->flags, MOB_FLAG_HIDDEN);
    }
}


/**
 * @brief Code for the holdable setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHoldable(ScriptActionInstRunData& data) {
    if(typeid(*(data.scriptVM->mob)) == typeid(Tool)) {
        unsigned char flags = 0;
        forIdx(i, data.args) {
            flags |= s2i(data.args[i]);
        }
        ((Tool*) (data.scriptVM->mob))->holdabilityFlags = flags;
    }
}


/**
 * @brief Code for the huntable setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHuntable(ScriptActionInstRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.scriptVM->mob->flags, MOB_FLAG_NON_HUNTABLE);
    } else {
        enableFlag(data.scriptVM->mob->flags, MOB_FLAG_NON_HUNTABLE);
    }
}


/**
 * @brief Code for the limb animation setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setLimbAnimation(ScriptActionInstRunData& data) {
    if(!data.scriptVM->mob->parent) {
        return;
    }
    if(!data.scriptVM->mob->parent->limbAnim.animDb) {
        return;
    }
    
    size_t a =
        data.scriptVM->mob->parent->limbAnim.animDb->findAnimation(
            data.args[0]
        );
    if(a == INVALID) {
        return;
    }
    
    data.scriptVM->mob->parent->limbAnim.curAnim =
        data.scriptVM->mob->parent->limbAnim.animDb->animations[a];
    data.scriptVM->mob->parent->limbAnim.toStart();
    
}


/**
 * @brief Code for the near reach setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setNearReach(ScriptActionInstRunData& data) {
    data.scriptVM->mob->nearReach = s2i(data.args[0]);
    data.scriptVM->mob->updateInteractionSpan();
}


/**
 * @brief Code for the radius setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setRadius(ScriptActionInstRunData& data) {
    data.scriptVM->mob->setRadius(s2f(data.args[0]));
}


/**
 * @brief Code for the sector scroll setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setSectorScroll(ScriptActionInstRunData& data) {
    Sector* sPtr = getSector(data.scriptVM->mob->pos, nullptr, true);
    if(!sPtr) return;
    
    sPtr->scroll.x = s2f(data.args[0]);
    sPtr->scroll.y = s2f(data.args[1]);
}


/**
 * @brief Code for the shadow visibility setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setShadowVisibility(ScriptActionInstRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.scriptVM->mob->flags, MOB_FLAG_SHADOW_INVISIBLE);
    } else {
        enableFlag(data.scriptVM->mob->flags, MOB_FLAG_SHADOW_INVISIBLE);
    }
}


/**
 * @brief Code for the state setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setState(ScriptActionInstRunData& data) {
    data.scriptVM->fsm.setState(
        s2i(data.args[0]), data.customData1, data.customData2
    );
}


/**
 * @brief Code for the tangible setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTangible(ScriptActionInstRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.scriptVM->mob->flags, MOB_FLAG_INTANGIBLE);
    } else {
        enableFlag(data.scriptVM->mob->flags, MOB_FLAG_INTANGIBLE);
    }
}


/**
 * @brief Code for the team setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTeam(ScriptActionInstRunData& data) {
    data.scriptVM->mob->setTeam((MOB_TEAM) s2i(data.args[0]));
}


/**
 * @brief Code for the timer setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTimer(ScriptActionInstRunData& data) {
    data.scriptVM->setTimer(s2f(data.args[0]));
}


/**
 * @brief Code for the var setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setVar(ScriptActionInstRunData& data) {
    data.scriptVM->setVar(data.args[0], data.args[1]);
}


/**
 * @brief Code for the shake camera script action.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::shakeCamera(ScriptActionInstRunData& data) {
    forIdx(p, game.states.gameplay->players) {
        Player* pPtr = &game.states.gameplay->players[p];
        float d =
            Distance(data.scriptVM->mob->pos, pPtr->view.cam.pos).toFloat();
        float strengthMult =
            ::interpolateNumber(
                d, 0.0f, DRAWING::CAM_SHAKE_DROPOFF_DIST, 1.0f, 0.0f
            );
        pPtr->view.shaker.shake(s2f(data.args[0]) / 100.0f * strengthMult);
    }
}


/**
 * @brief Code for the show message from var script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::showMessageFromVar(ScriptActionInstRunData& data) {
    startGameplayMessage(data.scriptVM->vars[data.args[0]], nullptr);
}


/**
 * @brief Code for the spawning script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::spawn(ScriptActionInstRunData& data) {
    data.scriptVM->mob->spawn(
        &data.scriptVM->mob->type->spawns[s2i(data.args[0])]
    );
}


/**
 * @brief Code for the square root number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::squareRootNumber(ScriptActionInstRunData& data) {
    data.scriptVM->vars[data.args[0]] = f2s((float) sqrt(s2f(data.args[1])));
}


/**
 * @brief Code for the z stabilization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stabilizeZ(ScriptActionInstRunData& data) {
    if(data.scriptVM->mob->links.empty() || !data.scriptVM->mob->links[0]) {
        return;
    }
    
    float bestMatchZ = data.scriptVM->mob->links[0]->z;
    SCRIPT_ACTION_STABILIZE_Z_TYPE t =
        (SCRIPT_ACTION_STABILIZE_Z_TYPE) s2i(data.args[0]);
        
    for(size_t l = 1; l < data.scriptVM->mob->links.size(); l++) {
    
        if(!data.scriptVM->mob->links[l]) continue;
        
        switch(t) {
        case SCRIPT_ACTION_STABILIZE_Z_TYPE_HIGHEST: {
            if(data.scriptVM->mob->links[l]->z > bestMatchZ) {
                bestMatchZ = data.scriptVM->mob->links[l]->z;
            }
            break;
            
        } case SCRIPT_ACTION_STABILIZE_Z_TYPE_LOWEST: {
            if(data.scriptVM->mob->links[l]->z < bestMatchZ) {
                bestMatchZ = data.scriptVM->mob->links[l]->z;
            }
            break;
            
        }
        }
        
    }
    
    data.scriptVM->mob->z = bestMatchZ + s2f(data.args[1]);
}


/**
 * @brief Code for the chomping start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startChomping(ScriptActionInstRunData& data) {
    data.scriptVM->mob->chompMax = s2i(data.args[0]);
    data.scriptVM->mob->chompBodyParts.clear();
    for(size_t p = 1; p < data.args.size(); p++) {
        data.scriptVM->mob->chompBodyParts.push_back(s2i(data.args[p]));
    }
}


/**
 * @brief Code for the dying start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startDying(ScriptActionInstRunData& data) {
    data.scriptVM->mob->startDying();
}


/**
 * @brief Code for the height effect start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startHeightEffect(ScriptActionInstRunData& data) {
    data.scriptVM->mob->startHeightEffect();
}


/**
 * @brief Code for the particle start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startParticles(ScriptActionInstRunData& data) {
    float offsetX = 0;
    float offsetY = 0;
    float offsetZ = 0;
    if(data.args.size() > 1) offsetX = s2f(data.args[1]);
    if(data.args.size() > 2) offsetY = s2f(data.args[2]);
    if(data.args.size() > 3) offsetZ = s2f(data.args[3]);
    
    ParticleGenerator pg =
        standardParticleGenSetup(data.args[0], data.scriptVM->mob);
    pg.followPosOffset = Point(offsetX, offsetY);
    pg.followZOffset = offsetZ;
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    data.scriptVM->mob->particleGenerators.push_back(pg);
}


/**
 * @brief Code for the stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stop(ScriptActionInstRunData& data) {
    data.scriptVM->mob->stopChasing();
    data.scriptVM->mob->stopTurning();
    data.scriptVM->mob->stopFollowingPath();
}


/**
 * @brief Code for the chomp stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopChomping(ScriptActionInstRunData& data) {
    data.scriptVM->mob->chompMax = 0;
    data.scriptVM->mob->chompBodyParts.clear();
}


/**
 * @brief Code for the height effect stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopHeightEffect(ScriptActionInstRunData& data) {
    data.scriptVM->mob->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopParticles(ScriptActionInstRunData& data) {
    data.scriptVM->mob->deleteParticleGenerator(
        MOB_PARTICLE_GENERATOR_ID_SCRIPT
    );
}


/**
 * @brief Code for the sound stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopSound(ScriptActionInstRunData& data) {
    game.audio.destroySoundSource(s2i(data.args[0]));
}


/**
 * @brief Code for the vertical stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopVertically(ScriptActionInstRunData& data) {
    data.scriptVM->mob->speedZ = 0;
}


/**
 * @brief Code for the focus storing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::storeFocusInside(ScriptActionInstRunData& data) {
    if(
        data.scriptVM->focusedMob &&
        !data.scriptVM->focusedMob->isStoredInsideMob()
    ) {
        data.scriptVM->mob->storeMobInside(data.scriptVM->focusedMob);
    }
}


/**
 * @brief Code for the swallow script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::swallow(ScriptActionInstRunData& data) {
    data.scriptVM->mob->swallowChompedPikmin(s2i(data.args[0]));
}


/**
 * @brief Code for the swallow all script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::swallowAll(ScriptActionInstRunData& data) {
    data.scriptVM->mob->swallowChompedPikmin(
        data.scriptVM->mob->chompingMobs.size()
    );
}


/**
 * @brief Code for the teleport to absolute coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::teleportToAbsolute(ScriptActionInstRunData& data) {
    data.scriptVM->mob->stopChasing();
    data.scriptVM->mob->chase(
        Point(s2f(data.args[0]), s2f(data.args[1])),
        s2f(data.args[2]),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the teleport to relative coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::teleportToRelative(ScriptActionInstRunData& data) {
    data.scriptVM->mob->stopChasing();
    Point p =
        rotatePoint(
            Point(s2f(data.args[0]), s2f(data.args[1])),
            data.scriptVM->mob->angle
        );
    data.scriptVM->mob->chase(
        data.scriptVM->mob->pos + p,
        data.scriptVM->mob->z + s2f(data.args[2]),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the throw focused mob script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::throwFocus(ScriptActionInstRunData& data) {
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    if(data.scriptVM->focusedMob->holder.m == data.scriptVM->mob) {
        data.scriptVM->mob->release(data.scriptVM->focusedMob);
    }
    
    float maxHeight = s2f(data.args[3]);
    
    if(maxHeight == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.scriptVM->mob->startHeightEffect();
    calculateThrow(
        data.scriptVM->focusedMob->pos, data.scriptVM->focusedMob->z,
        Point(s2f(data.args[0]), s2f(data.args[1])), s2f(data.args[2]),
        maxHeight, MOB::GRAVITY_ADDER,
        &data.scriptVM->focusedMob->speed,
        &data.scriptVM->focusedMob->speedZ,
        nullptr
    );
}


/**
 * @brief Code for the turn to an absolute angle script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::turnToAbsolute(ScriptActionInstRunData& data) {
    if(data.args.size() == 1) {
        //Turn to an absolute angle.
        data.scriptVM->mob->face(degToRad(s2f(data.args[0])), nullptr);
    } else {
        //Turn to some absolute coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        data.scriptVM->mob->face(
            getAngle(data.scriptVM->mob->pos, Point(x, y)), nullptr
        );
    }
}


/**
 * @brief Code for the turn to a relative angle script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::turnToRelative(ScriptActionInstRunData& data) {
    if(data.args.size() == 1) {
        //Turn to a relative angle.
        data.scriptVM->mob->face(
            data.scriptVM->mob->angle + degToRad(s2f(data.args[0])),
            nullptr
        );
    } else {
        //Turn to some relative coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        Point p = rotatePoint(Point(x, y), data.scriptVM->mob->angle);
        data.scriptVM->mob->face(
            getAngle(data.scriptVM->mob->pos, data.scriptVM->mob->pos + p),
            nullptr
        );
    }
}


/**
 * @brief Code for the turn to target script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::turnToTarget(ScriptActionInstRunData& data) {
    SCRIPT_ACTION_TURN_TYPE t = (SCRIPT_ACTION_TURN_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case SCRIPT_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC: {
        data.scriptVM->mob->arachnorbHeadTurnLogic();
        break;
        
    } case SCRIPT_ACTION_TURN_TYPE_FOCUSED_MOB: {
        if(data.scriptVM->focusedMob) {
            data.scriptVM->mob->face(0, &data.scriptVM->focusedMob->pos);
        }
        break;
        
    } case SCRIPT_ACTION_TURN_TYPE_HOME: {
        data.scriptVM->mob->face(
            getAngle(data.scriptVM->mob->pos, data.scriptVM->mob->home),
            nullptr
        );
        break;
        
    }
    }
}


#pragma endregion
#pragma region Action type param


/**
 * @brief Constructs a new script action type parameter object.
 *
 * @param name Name of the parameter.
 * @param type Type of parameter.
 * @param forceConst If true, this must be a constant value.
 * If false, it can also be a var.
 * @param isExtras If true, this is an array of them (minimum amount 0).
 */
ScriptActionTypeParam::ScriptActionTypeParam(
    const string& name, const SCRIPT_ACTION_PARAM type,
    bool forceConst, bool isExtras
):
    name(name),
    type(type),
    forceConst(forceConst),
    isExtras(isExtras) {
    
}


#pragma endregion
#pragma region Global functions


/**
 * @brief Returns the mob matching the mob target type.
 *
 * @param data Data about the action call.
 * @param type Type of target.
 */
Mob* getTargetMob(
    ScriptActionInstRunData& data, SCRIPT_ACTION_MOB_TARGET_TYPE type
) {
    switch (type) {
    case SCRIPT_ACTION_MOB_TARGET_TYPE_SELF: {
        return data.scriptVM->mob;
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_FOCUS: {
        return data.scriptVM->focusedMob;
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_TRIGGER: {
        return getTriggerMob(data);
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_LINK: {
        if(
            data.scriptVM->mob &&
            !data.scriptVM->mob->links.empty() &&
            data.scriptVM->mob->links[0]
        ) {
            return data.scriptVM->mob->links[0];
        }
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_PARENT: {
        if(data.scriptVM->mob && data.scriptVM->mob->parent) {
            return data.scriptVM->mob->parent->m;
        }
        break;
    }
    }
    
    return nullptr;
}


/**
 * @brief Gets the mob that triggered an event.
 *
 * @param data Data about the action call.
 * @return The mob.
 */
Mob* getTriggerMob(ScriptActionInstRunData& data) {
    if(
        data.actionDef->parentEvent == FSM_EV_OBJECT_IN_REACH ||
        data.actionDef->parentEvent == FSM_EV_OPPONENT_IN_REACH ||
        data.actionDef->parentEvent == FSM_EV_THROWN_PIKMIN_LANDED ||
        data.actionDef->parentEvent == FSM_EV_TOUCHED_OBJECT ||
        data.actionDef->parentEvent == FSM_EV_TOUCHED_OPPONENT ||
        data.actionDef->parentEvent == FSM_EV_HELD ||
        data.actionDef->parentEvent == FSM_EV_RELEASED ||
        data.actionDef->parentEvent == FSM_EV_SWALLOWED ||
        data.actionDef->parentEvent == FSM_EV_STARTED_RECEIVING_DELIVERY ||
        data.actionDef->parentEvent == FSM_EV_FINISHED_RECEIVING_DELIVERY ||
        data.actionDef->parentEvent == FSM_EV_ACTIVE_LEADER_CHANGED
    ) {
        return (Mob*)(data.customData1);
        
    } else if(
        data.actionDef->parentEvent == FSM_EV_RECEIVE_MESSAGE
    ) {
        return(Mob*)(data.customData2);
        
    } else if(
        data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_A_N ||
        data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_A ||
        data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_N ||
        data.actionDef->parentEvent == FSM_EV_DAMAGE
    ) {
        return ((HitboxInteraction*)(data.customData1))->mob2;
        
    }
    
    return nullptr;
}


#pragma endregion
