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

#include "script_actions.h"

#include "../../content/mob/group_task.h"
#include "../../content/mob/scale.h"
#include "../../content/mob/tool.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::set;


#pragma region Action call


/**
 * @brief Constructs a new script action call object of a certain type.
 *
 * @param type Type of script action call.
 */
ScriptActionCall::ScriptActionCall(SCRIPT_ACTION type) {

    for(size_t a = 0; a < game.scriptActions.size(); a++) {
        if(game.scriptActions[a].type == type) {
            action = &(game.scriptActions[a]);
            break;
        }
    }
}


/**
 * @brief Constructs a new script action call object meant to run custom code.
 *
 * @param code The function to run.
 */
ScriptActionCall::ScriptActionCall(CustomActionCode code) :
    code(code) {
    
    for(size_t a = 0; a < game.scriptActions.size(); a++) {
        if(game.scriptActions[a].type == SCRIPT_ACTION_UNKNOWN) {
            action = &(game.scriptActions[a]);
            break;
        }
    }
}


/**
 * @brief Loads a script action call from a data node.
 *
 * @param dn The data node.
 * @param mt Mob type this action's fsm belongs to.
 * @return Whether it was successful.
 */
bool ScriptActionCall::loadFromDataNode(DataNode* dn, MobType* mt) {

    action = nullptr;
    this->mt = mt;
    
    //First, get the name and arguments.
    vector<string> words = split(dn->name);
    
    for(size_t w = 0; w < words.size(); w++) {
        words[w] = trimSpaces(words[w]);
    }
    
    string name = words[0];
    words.erase(words.begin());
    
    //Find the corresponding action.
    for(size_t a = 0; a < game.scriptActions.size(); a++) {
        if(game.scriptActions[a].type == SCRIPT_ACTION_UNKNOWN) continue;
        if(game.scriptActions[a].name == name) {
            action = &(game.scriptActions[a]);
        }
    }
    
    if(!action) {
        game.errors.report("Unknown script action name \"" + name + "\"!", dn);
        return false;
    }
    
    //Check if there are too many or too few arguments.
    size_t mandatoryParams = action->parameters.size();
    
    if(mandatoryParams > 0) {
        if(action->parameters[mandatoryParams - 1].isExtras) {
            mandatoryParams--;
        }
    }
    
    if(words.size() < mandatoryParams) {
        game.errors.report(
            "The \"" + action->name + "\" action needs " +
            i2s(mandatoryParams) + " arguments, but this call only "
            "has " + i2s(words.size()) + "! You're missing the \"" +
            action->parameters[words.size()].name + "\" parameter.",
            dn
        );
        return false;
    }
    
    if(mandatoryParams == action->parameters.size()) {
        if(words.size() > action->parameters.size()) {
            game.errors.report(
                "The \"" + action->name + "\" action only needs " +
                i2s(action->parameters.size()) + " arguments, but this call "
                "has " + i2s(words.size()) + "!",
                dn
            );
            return false;
        }
    }
    
    //Fetch the arguments, and check if any of them are not allowed.
    for(size_t w = 0; w < words.size(); w++) {
        size_t paramIdx = std::min(w, action->parameters.size() - 1);
        bool isVar = (words[w][0] == '$' && words[w].size() > 1);
        
        if(isVar && words[w].size() >= 2 && words[w][1] == '$') {
            //Two '$' in a row means it's meant to use a literal '$'.
            isVar = false;
            words[w].erase(words[w].begin());
        }
        
        if(isVar) {
            if(action->parameters[paramIdx].forceConst) {
                game.errors.report(
                    "Argument #" + i2s(w + 1) + " (\"" + words[w] + "\") is a "
                    "variable, but the parameter \"" +
                    action->parameters[paramIdx].name + "\" can only be "
                    "constant!",
                    dn
                );
                return false;
            }
            
            words[w].erase(words[w].begin()); //Remove the '$'.
            
            if(words[w].empty()) {
                game.errors.report(
                    "Argument #" + i2s(w) + " is trying to use a variable "
                    "with no name!",
                    dn
                );
                return false;
            }
        }
        
        args.push_back(words[w]);
        argIsVar.push_back(isVar);
    }
    
    //If this action needs extra parsing, do it now.
    if(action->extraLoadLogic) {
        bool success = action->extraLoadLogic(*this);
        if(!customError.empty()) {
            game.errors.report(customError, dn);
        }
        return success;
    }
    
    return true;
}


/**
 * @brief Runs an action.
 *
 * @param m The mob.
 * @param customData1 Custom argument #1 to pass to the code.
 * @param customData2 Custom argument #2 to pass to the code.
 * @return Evaluation result, used only by the "if" actions.
 */
bool ScriptActionCall::run(
    Mob* m, void* customData1, void* customData2
) {
    //Custom code (i.e. instead of text-based script, use actual C++ code).
    if(code) {
        code(m, customData1, customData2);
        return false;
    }
    
    ScriptActionRunData data(m, this);
    
    //Fill the arguments. Fetch values from variables if needed.
    data.args = args;
    for(size_t a = 0; a < args.size(); a++) {
        if(argIsVar[a]) {
            data.args[a] = m->vars[args[a]];
        }
    }
    data.customData1 = customData1;
    data.customData2 = customData2;
    
    action->code(data);
    return data.returnValue;
}


#pragma endregion
#pragma region Action loaders


/**
 * @brief Loading code for the arachnorb logic plan mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::arachnorbPlanLogic(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE type =
        enumGetValue(
            mobActionArachnorbPlanLogicTypeINames, call.args[0], &found
        );
    if(!found) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the calculation mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::calculate(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_CALCULATE_TYPE type =
        enumGetValue(mobActionCalculateTypeINames, call.args[2], &found);
    if(!found) {
        reportEnumError(call, 2);
        return false;
    }
    call.args[2] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the ease number mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::easeNumber(ScriptActionCall& call) {
    bool found;
    EASE_METHOD method =
        enumGetValue(easeMethodINames, call.args[2], &found);
    if(!found) {
        reportEnumError(call, 2);
        return false;
    }
    call.args[2] = i2s(method);
    return true;
}


/**
 * @brief Loading code for the focus mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::focus(ScriptActionCall& call) {
    return loadMobTargetType(call, 0);
}


/**
 * @brief Loading code for the follow mob as leader mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::followMobAsLeader(ScriptActionCall& call) {
    return loadMobTargetType(call, 0);
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getAreaInfo(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_GET_AREA_INFO_TYPE type =
        enumGetValue(mobActionGetAreaInfoTypeINames, call.args[1], &found);
    if(!found) {
        call.customError =
            "Unknown info type \"" + call.args[1] + "\"! "
            "Try using \"get_mob_info\" or \"get_event_info\".";
        return false;
    }
    call.args[1] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getEventInfo(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_GET_EV_INFO_TYPE type =
        enumGetValue(mobActionGetEvInfoTypeINames, call.args[1], &found);
    if(!found) {
        call.customError =
            "Unknown info type \"" + call.args[1] + "\"! "
            "Try using \"get_mob_info\" or \"get_area_info\".";
        return false;
    }
    call.args[1] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getMobInfo(ScriptActionCall& call) {
    if(!loadMobTargetType(call, 1)) {
        return false;
    }
    
    bool found;
    MOB_ACTION_GET_MOB_INFO_TYPE type =
        enumGetValue(mobActionGetMobInfoTypeINames, call.args[2], &found);
    if(!found) {
        call.customError =
            "Unknown info type \"" + call.args[2] + "\"! "
            "Try using \"get_event_info\" or \"get_area_info\".";
        return false;
    }
    call.args[2] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the hold focused mob mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::holdFocus(ScriptActionCall& call) {
    size_t pIdx = call.mt->animDb->findBodyPart(call.args[0]);
    if(pIdx == INVALID) {
        call.customError =
            "Unknown body part \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(pIdx);
    return true;
}


/**
 * @brief Loading code for the "if" mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::ifFunction(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_IF_OP op =
        enumGetValue(mobActionIfOpINames, call.args[1], &found);
    if(!found) {
        reportEnumError(call, 1);
        return false;
    }
    call.args[1] = i2s(op);
    return true;
}


/**
 * @brief Loads a mob target type from an action call.
 *
 * @param call Mob action call that called this.
 * @param argIdx Index number of the mob target type argument.
 */
bool MobActionLoaders::loadMobTargetType(
    ScriptActionCall& call, size_t argIdx
) {
    bool found;
    MOB_ACTION_MOB_TARGET_TYPE type =
        enumGetValue(mobActionMobTargetTypeINames, call.args[argIdx], &found);
    if(!found) {
        reportEnumError(call, argIdx);
        return false;
    }
    call.args[argIdx] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the move to target mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::moveToTarget(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_MOVE_TYPE type =
        enumGetValue(mobActionMoveTypeINames, call.args[0], &found);
    if(!found) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the sound playing mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::playSound(ScriptActionCall& call) {
    for(size_t s = 0; s < call.mt->sounds.size(); s++) {
        if(call.mt->sounds[s].name == call.args[0]) {
            call.args[0] = i2s(s);
            return true;
        }
    }
    call.customError =
        "Unknown sound info block \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the status reception mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::receiveStatus(ScriptActionCall& call) {
    if(!isInMap(game.content.statusTypes.list, call.args[0])) {
        call.customError =
            "Unknown status effect \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the status removal mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::removeStatus(ScriptActionCall& call) {
    if(!isInMap(game.content.statusTypes.list, call.args[0])) {
        call.customError =
            "Unknown status effect \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Reports an error of an unknown enum value.
 *
 * @param call Mob action call that called this.
 * @param argIdx Index number of the argument that is an enum.
 */
void MobActionLoaders::reportEnumError(
    ScriptActionCall& call, size_t argIdx
) {
    size_t paramIdx = std::min(argIdx, call.action->parameters.size() - 1);
    call.customError =
        "The parameter \"" + call.action->parameters[paramIdx].name + "\" "
        "does not know what the value \"" +
        call.args[argIdx] + "\" means!";
}


/**
 * @brief Loading code for the animation setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setAnimation(ScriptActionCall& call) {
    size_t aPos = call.mt->animDb->findAnimation(call.args[0]);
    if(aPos == INVALID) {
        call.customError =
            "Unknown animation \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(aPos);
    
    if(call.args.size() > 1) {
        bool optionFound;
        START_ANIM_OPTION option =
            enumGetValue(startAnimOptionINames, call.args[1], &optionFound);
        if(!optionFound) {
            call.customError =
                "Unknown animation start option \"" + call.args[1] + "\"!";
            return false;
        }
        call.args[1] = i2s(option);
    }
    
    return true;
}


/**
 * @brief Loading code for the far reach setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setFarReach(ScriptActionCall& call) {
    for(size_t r = 0; r < call.mt->reaches.size(); r++) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.customError = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the holdable setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setHoldable(ScriptActionCall& call) {
    for(size_t a = 0; a < call.args.size(); a++) {
        bool found;
        HOLDABILITY_FLAG flag =
            enumGetValue(holdabilityFlagINames, call.args[a], &found);
        if(!found) {
            reportEnumError(call, a);
            return false;
        }
        call.args[a] = i2s(flag);
    }
    return true;
}


/**
 * @brief Loading code for the near reach setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setNearReach(ScriptActionCall& call) {
    for(size_t r = 0; r < call.mt->reaches.size(); r++) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.customError = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the team setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setTeam(ScriptActionCall& call) {
    bool found;
    MOB_TEAM teamNr = enumGetValue(mobTeamINames, call.args[0], &found);
    if(!found) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(teamNr);
    return true;
}


/**
 * @brief Loading code for the spawning mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::spawn(ScriptActionCall& call) {
    for(size_t s = 0; s < call.mt->spawns.size(); s++) {
        if(call.mt->spawns[s].name == call.args[0]) {
            call.args[0] = i2s(s);
            return true;
        }
    }
    call.customError =
        "Unknown spawn info block \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the z stabilization mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::stabilizeZ(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_STABILIZE_Z_TYPE type =
        enumGetValue(mobActionStabilizeZTypeINames, call.args[0], &found);
    if(!found) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(type);
    return true;
}


/**
 * @brief Loading code for the chomping start mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::startChomping(ScriptActionCall& call) {
    for(size_t s = 1; s < call.args.size(); s++) {
        size_t pNr = call.mt->animDb->findBodyPart(call.args[s]);
        if(pNr == INVALID) {
            call.customError =
                "Unknown body part \"" + call.args[s] + "\"!";
            return false;
        }
        call.args[s] = i2s(pNr);
    }
    return true;
}


/**
 * @brief Loading code for the particle start mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::startParticles(ScriptActionCall& call) {
    if(!isInMap(game.content.particleGens.list, call.args[0])) {
        call.customError =
            "Unknown particle generator \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the turn to target mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::turnToTarget(ScriptActionCall& call) {
    bool found;
    MOB_ACTION_TURN_TYPE type =
        enumGetValue(mobActionTurnTypeINames, call.args[0], &found);
    if(!found) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(type);
    return true;
}


#pragma endregion
#pragma region Action param


/**
 * @brief Constructs a new mob action param::mob action param object.
 *
 * @param name Name of the parameter.
 * @param type Type of parameter.
 * @param forceConst If true, this must be a constant value.
 * If false, it can also be a var.
 * @param isExtras If true, this is an array of them (minimum amount 0).
 */
ScriptActionParam::ScriptActionParam(
    const string& name, const SCRIPT_ACTION_PARAM type,
    bool forceConst, bool isExtras
):
    name(name),
    type(type),
    forceConst(forceConst),
    isExtras(isExtras) {
    
}


#pragma endregion
#pragma region Action run data


/**
 * @brief Constructs a new mob action run data object.
 *
 * @param m The mob responsible.
 * @param call Mob action call that called this.
 */
ScriptActionRunData::ScriptActionRunData(Mob* m, ScriptActionCall* call) :
    m(m),
    call(call) {
    
}


#pragma endregion
#pragma region Action runners


/**
 * @brief Code for the absolute number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::absoluteNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] = f2s(fabs(s2f(data.args[1])));
}


/**
 * @brief Code for the health addition mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::addHealth(ScriptActionRunData& data) {
    data.m->setHealth(true, false, s2f(data.args[0]));
}


/**
 * @brief Code for the arachnorb logic plan mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::arachnorbPlanLogic(ScriptActionRunData& data) {
    data.m->arachnorbPlanLogic(
        (MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE) s2i(data.args[0])
    );
}


/**
 * @brief Code for the calculation mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::calculate(ScriptActionRunData& data) {
    float lhs = s2f(data.args[1]);
    MOB_ACTION_CALCULATE_TYPE op =
        (MOB_ACTION_CALCULATE_TYPE) s2i(data.args[2]);
    float rhs = s2f(data.args[3]);
    float result = 0;
    
    switch(op) {
    case MOB_ACTION_CALCULATE_TYPE_SUM: {
        result = lhs + rhs;
        break;
        
    } case MOB_ACTION_CALCULATE_TYPE_SUBTRACT: {
        result = lhs - rhs;
        break;
        
    } case MOB_ACTION_CALCULATE_TYPE_MULTIPLY: {
        result = lhs * rhs;
        break;
        
    } case MOB_ACTION_CALCULATE_TYPE_DIVIDE: {
        if(rhs == 0) {
            result = 0;
        } else {
            result = lhs / rhs;
        }
        break;
        
    } case MOB_ACTION_CALCULATE_TYPE_MODULO: {
        if(rhs == 0) {
            result = 0;
        } else {
            result = fmod(lhs, rhs);
        }
        break;
        
    } case MOB_ACTION_CALCULATE_TYPE_POWER: {
        result = pow(lhs, rhs);
        break;
        
    }
    }
    
    data.m->vars[data.args[0]] = f2s(result);
}


/**
 * @brief Code for the ceil number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::ceilNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] = f2s(ceil(s2f(data.args[1])));
}


/**
 * @brief Code for the deletion mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::deleteFunction(ScriptActionRunData& data) {
    data.m->toDelete = true;
}


/**
 * @brief Code for the liquid draining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::drainLiquid(ScriptActionRunData& data) {
    Sector* sPtr = getSector(data.m->pos, nullptr, true);
    if(!sPtr) return;
    if(!sPtr->liquid) return;
    sPtr->liquid->startDraining();
}


/**
 * @brief Code for the ease number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::easeNumber(ScriptActionRunData& data) {
    EASE_METHOD method =
        (EASE_METHOD) s2i(data.args[2]);
    data.m->vars[data.args[0]] = f2s(ease(s2f(data.args[1]), method));
}


/**
 * @brief Code for the death finish mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::finishDying(ScriptActionRunData& data) {
    data.m->finishDying();
}


/**
 * @brief Code for the floor number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::floorNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] = f2s(floor(s2f(data.args[1])));
}


/**
 * @brief Code for the focus mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::focus(ScriptActionRunData& data) {
    MOB_ACTION_MOB_TARGET_TYPE s =
        (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    Mob* target = getTargetMob(data, s);
    
    if(!target) return;
    
    data.m->focusOnMob(target);
}


/**
 * @brief Code for the follow mob as leader mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::followMobAsLeader(ScriptActionRunData& data) {
    MOB_ACTION_MOB_TARGET_TYPE s =
        (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    Mob* target = getTargetMob(data, s);
    bool silent = false;
    if(data.args.size() >= 2) {
        silent = s2b(data.args[1]);
    }
    
    if(!target) return;
    if(target->health <= 0.0f) return;
    
    data.m->leaveGroup();
    
    if(data.m->type->category->id == MOB_CATEGORY_PIKMIN) {
        data.m->fsm.runEvent(MOB_EV_WHISTLED, (void*) target, (void*) silent);
    } else {
        target->addToGroup(data.m);
    }
}


/**
 * @brief Code for the follow path randomly mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::followPathRandomly(ScriptActionRunData& data) {
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
            game.curAreaData->pathStops.begin(),
            game.curAreaData->pathStops.end()
        );
    } else {
        //If there's a label, we should only pick stops that have the label.
        for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
            PathStop* sPtr = game.curAreaData->pathStops[s];
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
                Distance(choices[c]->pos, data.m->pos) >
                PATHS::DEF_CHASE_TARGET_DISTANCE
            ) {
                finalStop = choices[c];
                break;
            }
            tries++;
        }
    }
    
    //Go! Though if something went wrong, make it follow a path to nowhere,
    //so it can emit the MOB_EV_REACHED_DESTINATION event, and hopefully
    //make it clear that there was an error.
    PathFollowSettings settings;
    settings.targetPoint = finalStop ? finalStop->pos : data.m->pos;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    settings.label = label;
    data.m->followPath(
        settings, data.m->getBaseSpeed(), data.m->type->acceleration
    );
}


/**
 * @brief Code for the follow path to absolute mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::followPathToAbsolute(ScriptActionRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    
    PathFollowSettings settings;
    settings.targetPoint = Point(x, y);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    if(data.args.size() >= 3) {
        settings.label = data.args[2];
    }
    
    data.m->followPath(
        settings, data.m->getBaseSpeed(), data.m->type->acceleration
    );
}


/**
 * @brief Code for the angle obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getAngle(ScriptActionRunData& data) {
    float centerX = s2f(data.args[1]);
    float centerY = s2f(data.args[2]);
    float focusX = s2f(data.args[3]);
    float focusY = s2f(data.args[4]);
    float angle = getAngle(Point(centerX, centerY), Point(focusX, focusY));
    angle = radToDeg(angle);
    data.m->vars[data.args[0]] = f2s(angle);
}


/**
 * @brief Code for the angle closest difference obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getAngleCwDiff(ScriptActionRunData& data) {
    float angle1 = degToRad(s2f(data.args[1]));
    float angle2 = degToRad(s2f(data.args[2]));
    float diff = ::getAngleCwDiff(angle1, angle2);
    diff = radToDeg(diff);
    data.m->vars[data.args[0]] = f2s(diff);
}


/**
 * @brief Code for the angle smallest difference obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getAngleSmallestDiff(ScriptActionRunData& data) {
    float angle1 = degToRad(s2f(data.args[1]));
    float angle2 = degToRad(s2f(data.args[2]));
    float diff = ::getAngleSmallestDiff(angle1, angle2);
    diff = radToDeg(diff);
    data.m->vars[data.args[0]] = f2s(diff);
}



/**
 * @brief Code for the area info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getAreaInfo(ScriptActionRunData& data) {
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_AREA_INFO_TYPE t =
        (MOB_ACTION_GET_AREA_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES: {
        *var = i2s(game.states.gameplay->dayMinutes);
        break;
        
    } case MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN: {
        *var = i2s(game.states.gameplay->mobs.pikmin.size());
        break;
        
    }
    }
}


/**
 * @brief Code for the getting chomped mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getChomped(ScriptActionRunData& data) {
    if(data.call->parentEvent == MOB_EV_HITBOX_TOUCH_EAT) {
        ((Mob*) (data.customData1))->chomp(
            data.m,
            (Hitbox*) (data.customData2)
        );
    }
}


/**
 * @brief Code for the coordinate from angle obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getCoordinatesFromAngle(ScriptActionRunData& data) {
    float angle = s2f(data.args[2]);
    angle = degToRad(angle);
    float magnitude = s2f(data.args[3]);
    Point p = angleToCoordinates(angle, magnitude);
    data.m->vars[data.args[0]] = f2s(p.x);
    data.m->vars[data.args[1]] = f2s(p.y);
}


/**
 * @brief Code for the distance obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getDistance(ScriptActionRunData& data) {
    float centerX = s2f(data.args[1]);
    float centerY = s2f(data.args[2]);
    float focusX = s2f(data.args[3]);
    float focusY = s2f(data.args[4]);
    data.m->vars[data.args[0]] =
        f2s(
            Distance(Point(centerX, centerY), Point(focusX, focusY)).toFloat()
        );
}


/**
 * @brief Code for the event info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getEventInfo(ScriptActionRunData& data) {
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_EV_INFO_TYPE t =
        (MOB_ACTION_GET_EV_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART: {
        if(
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_N ||
            data.call->parentEvent == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h1->bodyPartName;
        } else if(
            data.call->parentEvent == MOB_EV_TOUCHED_OBJECT ||
            data.call->parentEvent == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parentEvent == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                data.m->getClosestHitbox(
                    ((Mob*)(data.customData1))->pos
                )->bodyPartName;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL: {
        if(data.call->parentEvent == MOB_EV_FRAME_SIGNAL) {
            *var = i2s(*((size_t*)(data.customData1)));
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_HAZARD: {
        if(
            data.call->parentEvent == MOB_EV_TOUCHED_HAZARD ||
            data.call->parentEvent == MOB_EV_LEFT_HAZARD
        ) {
            *var = ((Hazard*)data.customData1)->manifest->internalName;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME: {
        if(data.call->parentEvent == MOB_EV_INPUT_RECEIVED) {
            PLAYER_ACTION_TYPE playerActionTypeId =
                (PLAYER_ACTION_TYPE)
                ((Inpution::Action*) (data.customData1))->actionTypeId;
            *var =
                game.controls.getActionTypeById(
                    playerActionTypeId
                ).internalName;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.call->parentEvent == MOB_EV_INPUT_RECEIVED) {
            *var = f2s(((Inpution::Action*) (data.customData1))->value);
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE: {
        if(data.call->parentEvent == MOB_EV_RECEIVE_MESSAGE) {
            *var = *((string*)(data.customData1));
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART: {
        if(
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_N ||
            data.call->parentEvent == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h2->bodyPartName;
        } else if(
            data.call->parentEvent == MOB_EV_TOUCHED_OBJECT ||
            data.call->parentEvent == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parentEvent == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                ((Mob*)(data.customData1))->getClosestHitbox(
                    data.m->pos
                )->bodyPartName;
        }
        break;
        
    }
    }
}


/**
 * @brief Code for the floor Z obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getFloorZ(ScriptActionRunData& data) {
    float x = s2f(data.args[1]);
    float y = s2f(data.args[2]);
    Sector* s = getSector(Point(x, y), nullptr, true);
    data.m->vars[data.args[0]] = f2s(s ? s->z : 0);
}


/**
 * @brief Code for the focused mob var getting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getFocusVar(ScriptActionRunData& data) {
    if(!data.m->focusedMob) return;
    data.m->vars[data.args[0]] =
        data.m->focusedMob->vars[data.args[1]];
}


/**
 * @brief Code for the mob info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getMobInfo(ScriptActionRunData& data) {
    MOB_ACTION_MOB_TARGET_TYPE s =
        (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[1]);
    Mob* target = getTargetMob(data, s);
    
    if(!target) return;
    
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_MOB_INFO_TYPE t =
        (MOB_ACTION_GET_MOB_INFO_TYPE) s2i(data.args[2]);
        
    switch(t) {
    case MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE: {
        *var = f2s(radToDeg(target->angle));
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN: {
        *var = i2s(target->chompingMobs.size());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->focusedMob) {
            float d =
                Distance(target->pos, target->focusedMob->pos).toFloat();
            *var = f2s(d);
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER: {
        if(target->type->category->id == MOB_CATEGORY_GROUP_TASKS) {
            *var = f2s(((GroupTask*)target)->getPower());
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH: {
        *var = i2s(target->health);
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO: {
        if(target->maxHealth != 0.0f) {
            *var = f2s(target->health / target->maxHealth);
        } else {
            *var = "0";
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_ID: {
        *var = i2s(target->id);
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN: {
        *var = i2s(target->getLatchedPikminAmount());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT: {
        *var = i2s(target->getLatchedPikminWeight());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY: {
        *var = target->type->category->internalName;
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE: {
        if(target->type->manifest) {
            *var = target->type->manifest->internalName;
        } else {
            *var = "";
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_STATE: {
        *var = target->fsm.curState->name;
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT: {
        if(target->type->category->id == MOB_CATEGORY_SCALES) {
            Scale* sPtr = (Scale*)(target);
            *var = i2s(sPtr->calculateCurWeight());
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_X: {
        *var = f2s(target->pos.x);
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_Y: {
        *var = f2s(target->pos.y);
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_Z: {
        *var = f2s(target->z);
        break;
    }
    }
}


/**
 * @brief Code for the float number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getRandomFloat(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] =
        f2s(game.rng.f(s2f(data.args[1]), s2f(data.args[2])));
}


/**
 * @brief Code for the integer number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getRandomInt(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] =
        i2s(game.rng.i(s2i(data.args[1]), s2i(data.args[2])));
}


/**
 * @brief Code for the hold focused mob mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::holdFocus(ScriptActionRunData& data) {
    if(data.m->focusedMob) {
        data.m->hold(
            data.m->focusedMob, HOLD_TYPE_PURPOSE_GENERAL,
            s2i(data.args[0]), 0.0f, 0.0f, 0.5f,
            data.args.size() >= 2 ? s2b(data.args[1]) : false,
            HOLD_ROTATION_METHOD_COPY_HOLDER
        );
    }
}


/**
 * @brief Code for the "if" mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::ifFunction(ScriptActionRunData& data) {
    string lhs = data.args[0];
    MOB_ACTION_IF_OP op =
        (MOB_ACTION_IF_OP) s2i(data.args[1]);
    string rhs = vectorTailToString(data.args, 2);
    
    switch(op) {
    case MOB_ACTION_IF_OP_EQUAL: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.returnValue = (s2f(lhs) == s2f(rhs));
        } else {
            data.returnValue = (lhs == rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_NOT: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.returnValue = (s2f(lhs) != s2f(rhs));
        } else {
            data.returnValue = (lhs != rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_LESS: {
        data.returnValue = (s2f(lhs) < s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_MORE: {
        data.returnValue = (s2f(lhs) > s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_LESS_E: {
        data.returnValue = (s2f(lhs) <= s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_MORE_E: {
        data.returnValue = (s2f(lhs) >= s2f(rhs));
        break;
        
    }
    }
}


/**
 * @brief Code for the interpolate number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::interpolateNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] =
        f2s(
            ::interpolateNumber(
                s2f(data.args[1]), s2f(data.args[2]), s2f(data.args[3]),
                s2f(data.args[4]), s2f(data.args[5])
            )
        );
}


/**
 * @brief Code for the link with focus mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::linkWithFocus(ScriptActionRunData& data) {
    if(!data.m->focusedMob) {
        return;
    }
    
    for(size_t l = 0; l < data.m->links.size(); l++) {
        if(data.m->links[l] == data.m->focusedMob) {
            //Already linked.
            return;
        }
    }
    
    data.m->links.push_back(data.m->focusedMob);
}


/**
 * @brief Code for the load focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::loadFocusMemory(ScriptActionRunData& data) {
    if(data.m->focusedMobMemory.empty()) {
        return;
    }
    
    data.m->focusOnMob(data.m->focusedMobMemory[s2i(data.args[0])]);
}


/**
 * @brief Code for the move to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::moveToAbsolute(ScriptActionRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = data.args.size() > 2 ? s2f(data.args[2]) : data.m->z;
    data.m->chase(
        Point(x, y), z,
        CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
    );
}


/**
 * @brief Code for the move to relative coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::moveToRelative(ScriptActionRunData& data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = (data.args.size() > 2 ? s2f(data.args[2]) : 0);
    Point p = rotatePoint(Point(x, y), data.m->angle);
    data.m->chase(
        data.m->pos + p, data.m->z + z,
        CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
    );
}


/**
 * @brief Code for the move to target mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::moveToTarget(ScriptActionRunData& data) {
    MOB_ACTION_MOVE_TYPE t = (MOB_ACTION_MOVE_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.m->focusedMob) {
            float a = getAngle(data.m->pos, data.m->focusedMob->pos);
            Point offset = Point(2000, 0);
            offset = rotatePoint(offset, a + TAU / 2.0);
            data.m->chase(
                data.m->pos + offset,
                data.m->z,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS: {
        if(data.m->focusedMob) {
            data.m->chase(
                &data.m->focusedMob->pos,
                &data.m->focusedMob->z,
                Point(), 0.0f,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.m->focusedMob) {
            data.m->chase(
                data.m->focusedMob->pos,
                data.m->focusedMob->z,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_HOME: {
        data.m->chase(
            data.m->home,
            data.m->z,
            CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
        );
        break;
        
    } case MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC: {
        data.m->arachnorbFootMoveLogic();
        break;
        
    } case MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE: {
        if(data.m->links.empty()) {
            return;
        }
        
        Point des;
        for(size_t l = 0; l < data.m->links.size(); l++) {
            if(!data.m->links[l]) continue;
            des += data.m->links[l]->pos;
        }
        des = des / data.m->links.size();
        
        data.m->chase(
            des,
            data.m->z,
            CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
        );
        break;
        
    }
    }
}


/**
 * @brief Code for the release order mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::orderRelease(ScriptActionRunData& data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.runEvent(MOB_EV_RELEASE_ORDER, nullptr, nullptr);
    }
}


/**
 * @brief Code for the sound playing mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::playSound(ScriptActionRunData& data) {
    size_t soundId = data.m->playSound(s2i(data.args[0]));
    if(data.args.size() >= 2) {
        data.m->setVar(data.args[1], i2s(soundId));
    }
}


/**
 * @brief Code for the text printing mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::print(ScriptActionRunData& data) {
    size_t seconds = floor(game.states.gameplay->gameplayTimePassed);
    size_t centiseconds =
        (game.states.gameplay->gameplayTimePassed - seconds) * 100;
    string timestamp =
        resizeString(i2s(seconds), 4, true, true, true, ' ') + "." +
        resizeString(i2s(centiseconds), 2, true, true, true, '0');
        
    string scriptText = vectorTailToString(data.args, 0);
    game.states.gameplay->printActionLogLines.push_back(
        "[@" + timestamp + "s " + data.m->type->name + " said:] " + scriptText
    );
    if(game.states.gameplay->printActionLogLines.size() > 10) {
        game.states.gameplay->printActionLogLines.erase(
            game.states.gameplay->printActionLogLines.begin()
        );
    }
    
    string log;
    for(
        size_t l = 0; l < game.states.gameplay->printActionLogLines.size(); l++
    ) {
        log += "\n" + game.states.gameplay->printActionLogLines[l];
    }
    
    game.console.write("=== DEBUG MOB SCRIPT PRINTS ===" + log, 15.0f);
}


/**
 * @brief Code for the status reception mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::receiveStatus(ScriptActionRunData& data) {
    data.m->applyStatus(
        game.content.statusTypes.list[data.args[0]], false, false
    );
}


/**
 * @brief Code for the release mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::release(ScriptActionRunData& data) {
    data.m->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::releaseStoredMobs(ScriptActionRunData& data) {
    data.m->releaseStoredMobs();
}


/**
 * @brief Code for the status removal mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::removeStatus(ScriptActionRunData& data) {
    for(size_t s = 0; s < data.m->statuses.size(); s++) {
        if(data.m->statuses[s].type->manifest->internalName == data.args[0]) {
            data.m->statuses[s].prevState = data.m->statuses[s].state;
            data.m->statuses[s].state = STATUS_STATE_TO_DELETE;
        }
    }
}


/**
 * @brief Code for the round number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::roundNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] = f2s(round(s2f(data.args[1])));
}


/**
 * @brief Code for the save focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::saveFocusMemory(ScriptActionRunData& data) {
    if(!data.m->focusedMob) {
        return;
    }
    
    data.m->focusedMobMemory[s2i(data.args[0])] = data.m->focusedMob;
}


/**
 * @brief Code for the focused mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::sendMessageToFocus(ScriptActionRunData& data) {
    if(!data.m->focusedMob) return;
    data.m->sendScriptMessage(data.m->focusedMob, data.args[0]);
}


/**
 * @brief Code for the linked mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::sendMessageToLinks(ScriptActionRunData& data) {
    for(size_t l = 0; l < data.m->links.size(); l++) {
        if(data.m->links[l] == data.m) continue;
        if(!data.m->links[l]) continue;
        data.m->sendScriptMessage(data.m->links[l], data.args[0]);
    }
}


/**
 * @brief Code for the nearby mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::sendMessageToNearby(ScriptActionRunData& data) {
    float d = s2f(data.args[0]);
    
    for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); m2++) {
        if(game.states.gameplay->mobs.all[m2] == data.m) {
            continue;
        }
        if(Distance(data.m->pos, game.states.gameplay->mobs.all[m2]->pos) > d) {
            continue;
        }
        
        data.m->sendScriptMessage(
            game.states.gameplay->mobs.all[m2], data.args[1]
        );
    }
}


/**
 * @brief Code for the animation setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setAnimation(ScriptActionRunData& data) {
    START_ANIM_OPTION options = START_ANIM_OPTION_NORMAL;
    float mobSpeedBaseline = 0.0f;
    if(data.args.size() > 1) {
        options = (START_ANIM_OPTION) s2i(data.args[1]);
    }
    if(data.args.size() > 2) {
        if(s2b(data.args[2])) {
            mobSpeedBaseline = data.m->type->moveSpeed;
        };
    }
    
    data.m->setAnimation(
        s2i(data.args[0]), options, false, mobSpeedBaseline
    );
}


/**
 * @brief Code for the block paths setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setCanBlockPaths(ScriptActionRunData& data) {
    data.m->setCanBlockPaths(s2b(data.args[0]));
}


/**
 * @brief Code for the far reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setFarReach(ScriptActionRunData& data) {
    data.m->farReach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the flying setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setFlying(ScriptActionRunData& data) {
    if(s2b(data.args[0])) {
        enableFlag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    } else {
        disableFlag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Code for the focused mob var setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setFocusVar(ScriptActionRunData& data) {
    if(!data.m->focusedMob) return;
    data.m->focusedMob->vars[data.args[0]] = data.args[1];
}


/**
 * @brief Code for the gravity setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setGravity(ScriptActionRunData& data) {
    data.m->gravityMult = s2f(data.args[0]);
}


/**
 * @brief Code for the health setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHealth(ScriptActionRunData& data) {
    data.m->setHealth(false, false, s2f(data.args[0]));
}


/**
 * @brief Code for the height setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHeight(ScriptActionRunData& data) {
    data.m->height = s2f(data.args[0]);
    
    if(data.m->type->walkable) {
        //Update the Z of mobs standing on top of it.
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m];
            if(m2Ptr->standingOnMob == data.m) {
                m2Ptr->z = data.m->z + data.m->height;
            }
        }
    }
}


/**
 * @brief Code for the hiding setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHiding(ScriptActionRunData& data) {
    if(s2b(data.args[0])) {
        enableFlag(data.m->flags, MOB_FLAG_HIDDEN);
    } else {
        disableFlag(data.m->flags, MOB_FLAG_HIDDEN);
    }
}


/**
 * @brief Code for the holdable setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHoldable(ScriptActionRunData& data) {
    if(typeid(*(data.m)) == typeid(Tool)) {
        unsigned char flags = 0;
        for(size_t i = 0; i < data.args.size(); i++) {
            flags |= s2i(data.args[i]);
        }
        ((Tool*) (data.m))->holdabilityFlags = flags;
    }
}


/**
 * @brief Code for the huntable setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHuntable(ScriptActionRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.m->flags, MOB_FLAG_NON_HUNTABLE);
    } else {
        enableFlag(data.m->flags, MOB_FLAG_NON_HUNTABLE);
    }
}


/**
 * @brief Code for the limb animation setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setLimbAnimation(ScriptActionRunData& data) {
    if(!data.m->parent) {
        return;
    }
    if(!data.m->parent->limbAnim.animDb) {
        return;
    }
    
    size_t a = data.m->parent->limbAnim.animDb->findAnimation(data.args[0]);
    if(a == INVALID) {
        return;
    }
    
    data.m->parent->limbAnim.curAnim =
        data.m->parent->limbAnim.animDb->animations[a];
    data.m->parent->limbAnim.toStart();
    
}


/**
 * @brief Code for the near reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setNearReach(ScriptActionRunData& data) {
    data.m->nearReach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the radius setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setRadius(ScriptActionRunData& data) {
    data.m->setRadius(s2f(data.args[0]));
}


/**
 * @brief Code for the sector scroll setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setSectorScroll(ScriptActionRunData& data) {
    Sector* sPtr = getSector(data.m->pos, nullptr, true);
    if(!sPtr) return;
    
    sPtr->scroll.x = s2f(data.args[0]);
    sPtr->scroll.y = s2f(data.args[1]);
}


/**
 * @brief Code for the shadow visibility setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setShadowVisibility(ScriptActionRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.m->flags, MOB_FLAG_SHADOW_INVISIBLE);
    } else {
        enableFlag(data.m->flags, MOB_FLAG_SHADOW_INVISIBLE);
    }
}


/**
 * @brief Code for the state setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setState(ScriptActionRunData& data) {
    data.m->fsm.setState(
        s2i(data.args[0]),
        data.customData1,
        data.customData2
    );
}


/**
 * @brief Code for the tangible setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setTangible(ScriptActionRunData& data) {
    if(s2b(data.args[0])) {
        disableFlag(data.m->flags, MOB_FLAG_INTANGIBLE);
    } else {
        enableFlag(data.m->flags, MOB_FLAG_INTANGIBLE);
    }
}


/**
 * @brief Code for the team setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setTeam(ScriptActionRunData& data) {
    data.m->team = (MOB_TEAM) s2i(data.args[0]);
}


/**
 * @brief Code for the timer setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setTimer(ScriptActionRunData& data) {
    data.m->setTimer(s2f(data.args[0]));
}


/**
 * @brief Code for the var setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setVar(ScriptActionRunData& data) {
    data.m->setVar(data.args[0], data.args[1]);
}


/**
 * @brief Code for the shake camera script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::shakeCamera(ScriptActionRunData& data) {
    for(size_t p = 0; p < game.states.gameplay->players.size(); p++) {
        Player* pPtr = &game.states.gameplay->players[p];
        float d = Distance(data.m->pos, pPtr->view.cam.pos).toFloat();
        float strengthMult =
            ::interpolateNumber(
                d, 0.0f, DRAWING::CAM_SHAKE_DROPOFF_DIST, 1.0f, 0.0f
            );
        pPtr->view.shaker.shake(s2f(data.args[0]) / 100.0f * strengthMult);
    }
}


/**
 * @brief Code for the show message from var mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::showMessageFromVar(ScriptActionRunData& data) {
    startGameplayMessage(data.m->vars[data.args[0]], nullptr);
}


/**
 * @brief Code for the spawning mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::spawn(ScriptActionRunData& data) {
    data.m->spawn(&data.m->type->spawns[s2i(data.args[0])]);
}


/**
 * @brief Code for the square root number mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::squareRootNumber(ScriptActionRunData& data) {
    data.m->vars[data.args[0]] = f2s((float) sqrt(s2f(data.args[1])));
}


/**
 * @brief Code for the z stabilization mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stabilizeZ(ScriptActionRunData& data) {
    if(data.m->links.empty() || !data.m->links[0]) {
        return;
    }
    
    float bestMatchZ = data.m->links[0]->z;
    MOB_ACTION_STABILIZE_Z_TYPE t =
        (MOB_ACTION_STABILIZE_Z_TYPE) s2i(data.args[0]);
        
    for(size_t l = 1; l < data.m->links.size(); l++) {
    
        if(!data.m->links[l]) continue;
        
        switch(t) {
        case MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST: {
            if(data.m->links[l]->z > bestMatchZ) {
                bestMatchZ = data.m->links[l]->z;
            }
            break;
            
        } case MOB_ACTION_STABILIZE_Z_TYPE_LOWEST: {
            if(data.m->links[l]->z < bestMatchZ) {
                bestMatchZ = data.m->links[l]->z;
            }
            break;
            
        }
        }
        
    }
    
    data.m->z = bestMatchZ + s2f(data.args[1]);
}


/**
 * @brief Code for the chomping start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startChomping(ScriptActionRunData& data) {
    data.m->chompMax = s2i(data.args[0]);
    data.m->chompBodyParts.clear();
    for(size_t p = 1; p < data.args.size(); p++) {
        data.m->chompBodyParts.push_back(s2i(data.args[p]));
    }
}


/**
 * @brief Code for the dying start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startDying(ScriptActionRunData& data) {
    data.m->startDying();
}


/**
 * @brief Code for the height effect start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startHeightEffect(ScriptActionRunData& data) {
    data.m->startHeightEffect();
}


/**
 * @brief Code for the particle start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startParticles(ScriptActionRunData& data) {
    float offsetX = 0;
    float offsetY = 0;
    float offsetZ = 0;
    if(data.args.size() > 1) offsetX = s2f(data.args[1]);
    if(data.args.size() > 2) offsetY = s2f(data.args[2]);
    if(data.args.size() > 3) offsetZ = s2f(data.args[3]);
    
    ParticleGenerator pg =
        standardParticleGenSetup(data.args[0], data.m);
    pg.followPosOffset = Point(offsetX, offsetY);
    pg.followZOffset = offsetZ;
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    data.m->particleGenerators.push_back(pg);
}


/**
 * @brief Code for the stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stop(ScriptActionRunData& data) {
    data.m->stopChasing();
    data.m->stopTurning();
    data.m->stopFollowingPath();
}


/**
 * @brief Code for the chomp stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopChomping(ScriptActionRunData& data) {
    data.m->chompMax = 0;
    data.m->chompBodyParts.clear();
}


/**
 * @brief Code for the height effect stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopHeightEffect(ScriptActionRunData& data) {
    data.m->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopParticles(ScriptActionRunData& data) {
    data.m->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_SCRIPT);
}


/**
 * @brief Code for the sound stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopSound(ScriptActionRunData& data) {
    game.audio.destroySoundSource(s2i(data.args[0]));
}


/**
 * @brief Code for the vertical stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopVertically(ScriptActionRunData& data) {
    data.m->speedZ = 0;
}


/**
 * @brief Code for the focus storing mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::storeFocusInside(ScriptActionRunData& data) {
    if(data.m->focusedMob && !data.m->focusedMob->isStoredInsideMob()) {
        data.m->storeMobInside(data.m->focusedMob);
    }
}


/**
 * @brief Code for the swallow mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::swallow(ScriptActionRunData& data) {
    data.m->swallowChompedPikmin(s2i(data.args[0]));
}


/**
 * @brief Code for the swallow all mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::swallowAll(ScriptActionRunData& data) {
    data.m->swallowChompedPikmin(data.m->chompingMobs.size());
}


/**
 * @brief Code for the teleport to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::teleportToAbsolute(ScriptActionRunData& data) {
    data.m->stopChasing();
    data.m->chase(
        Point(s2f(data.args[0]), s2f(data.args[1])),
        s2f(data.args[2]),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the teleport to relative coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::teleportToRelative(ScriptActionRunData& data) {
    data.m->stopChasing();
    Point p =
        rotatePoint(
            Point(s2f(data.args[0]), s2f(data.args[1])),
            data.m->angle
        );
    data.m->chase(
        data.m->pos + p,
        data.m->z + s2f(data.args[2]),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the throw focused mob mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::throwFocus(ScriptActionRunData& data) {
    if(!data.m->focusedMob) {
        return;
    }
    
    if(data.m->focusedMob->holder.m == data.m) {
        data.m->release(data.m->focusedMob);
    }
    
    float maxHeight = s2f(data.args[3]);
    
    if(maxHeight == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.m->startHeightEffect();
    calculateThrow(
        data.m->focusedMob->pos, data.m->focusedMob->z,
        Point(s2f(data.args[0]), s2f(data.args[1])), s2f(data.args[2]),
        maxHeight, MOB::GRAVITY_ADDER,
        &data.m->focusedMob->speed,
        &data.m->focusedMob->speedZ,
        nullptr
    );
}


/**
 * @brief Code for the turn to an absolute angle mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::turnToAbsolute(ScriptActionRunData& data) {
    if(data.args.size() == 1) {
        //Turn to an absolute angle.
        data.m->face(degToRad(s2f(data.args[0])), nullptr);
    } else {
        //Turn to some absolute coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        data.m->face(getAngle(data.m->pos, Point(x, y)), nullptr);
    }
}


/**
 * @brief Code for the turn to a relative angle mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::turnToRelative(ScriptActionRunData& data) {
    if(data.args.size() == 1) {
        //Turn to a relative angle.
        data.m->face(data.m->angle + degToRad(s2f(data.args[0])), nullptr);
    } else {
        //Turn to some relative coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        Point p = rotatePoint(Point(x, y), data.m->angle);
        data.m->face(getAngle(data.m->pos, data.m->pos + p), nullptr);
    }
}


/**
 * @brief Code for the turn to target mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::turnToTarget(ScriptActionRunData& data) {
    MOB_ACTION_TURN_TYPE t = (MOB_ACTION_TURN_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC: {
        data.m->arachnorbHeadTurnLogic();
        break;
        
    } case MOB_ACTION_TURN_TYPE_FOCUSED_MOB: {
        if(data.m->focusedMob) {
            data.m->face(0, &data.m->focusedMob->pos);
        }
        break;
        
    } case MOB_ACTION_TURN_TYPE_HOME: {
        data.m->face(getAngle(data.m->pos, data.m->home), nullptr);
        break;
        
    }
    }
}


#pragma endregion
#pragma region Global functions


/**
 * @brief Confirms if the "if", "else", "end_if", "goto", and "label" actions in
 * a given vector of actions are all okay, and there are no mismatches, like
 * for instance, an "else" without an "if".
 * Also checks if there are actions past a "set_state" action.
 * If something goes wrong, it throws the errors to the error log.
 *
 * @param actions The vector of actions to check.
 * @param dn Data node from where these actions came.
 * @return Whether it succeeded.
 */
bool assertActions(
    const vector<ScriptActionCall*>& actions, const DataNode* dn
) {
    //Check if the "if"-related actions are okay.
    int depth = 0;
    vector<bool> seenElseAction;
    for(size_t a = 0; a < actions.size(); a++) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            depth++;
            seenElseAction.push_back(false);
            break;
        } case MOB_ACTION_ELSE: {
            if(depth == 0) {
                game.errors.report(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            seenElseAction.back() = true;
            break;
        } case MOB_ACTION_ELSE_IF: {
            if(depth == 0) {
                game.errors.report(
                    "Found an \"else_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            if(seenElseAction.back()) {
                game.errors.report(
                    "Found an \"else_if\" action after an \"else\" action!",
                    dn
                );
                return false;
            }
            break;
        } case MOB_ACTION_END_IF: {
            if(depth == 0) {
                game.errors.report(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            depth--;
            seenElseAction.pop_back();
            break;
        } default: {
            break;
        }
        }
    }
    if(depth > 0) {
        game.errors.report(
            "Some \"if\" actions don't have a matching \"end_if\" action!",
            dn
        );
        return false;
    }
    
    //Check if the "goto"-related actions are okay.
    set<string> labels;
    for(size_t a = 0; a < actions.size(); a++) {
        if(actions[a]->action->type == MOB_ACTION_LABEL) {
            const string& name = actions[a]->args[0];
            if(isInContainer(labels, name)) {
                game.errors.report(
                    "There are multiple labels called \"" + name + "\"!", dn
                );
                return false;
            }
            labels.insert(name);
        }
    }
    for(size_t a = 0; a < actions.size(); a++) {
        if(actions[a]->action->type == MOB_ACTION_GOTO) {
            const string& name = actions[a]->args[0];
            if(!isInContainer(labels, name)) {
                game.errors.report(
                    "There is no label called \"" + name + "\", even though "
                    "there are \"goto\" actions that need it!", dn
                );
                return false;
            }
        }
    }
    
    //Check if there are actions after a "set_state" action.
    bool passedSetState = false;
    for(size_t a = 0; a < actions.size(); a++) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_SET_STATE: {
            passedSetState = true;
            break;
        } case MOB_ACTION_ELSE: {
            passedSetState = false;
            break;
        } case MOB_ACTION_ELSE_IF: {
            passedSetState = false;
            break;
        } case MOB_ACTION_END_IF: {
            passedSetState = false;
            break;
        } case MOB_ACTION_LABEL: {
            passedSetState = false;
            break;
        } default: {
            if(passedSetState) {
                game.errors.report(
                    "There is an action \"" + actions[a]->action->name + "\" "
                    "placed after a \"set_state\" action, which means it will "
                    "never get run! Make sure you didn't mean to call it "
                    "before the \"set_state\" action.", dn
                );
                return false;
            }
            break;
        }
        }
    }
    
    return true;
}


/**
 * @brief Returns the mob matching the mob target type.
 *
 * @param data Data about the action call.
 * @param type Type of target.
 */
Mob* getTargetMob(
    ScriptActionRunData& data, MOB_ACTION_MOB_TARGET_TYPE type
) {
    switch (type) {
    case MOB_ACTION_MOB_TARGET_TYPE_SELF: {
        return data.m;
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_FOCUS: {
        return data.m->focusedMob;
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_TRIGGER: {
        return getTriggerMob(data);
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_LINK: {
        if(!data.m->links.empty() && data.m->links[0]) {
            return data.m->links[0];
        }
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_PARENT: {
        if(data.m->parent) {
            return data.m->parent->m;
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
Mob* getTriggerMob(ScriptActionRunData& data) {
    if(
        data.call->parentEvent == MOB_EV_OBJECT_IN_REACH ||
        data.call->parentEvent == MOB_EV_OPPONENT_IN_REACH ||
        data.call->parentEvent == MOB_EV_THROWN_PIKMIN_LANDED ||
        data.call->parentEvent == MOB_EV_TOUCHED_OBJECT ||
        data.call->parentEvent == MOB_EV_TOUCHED_OPPONENT ||
        data.call->parentEvent == MOB_EV_HELD ||
        data.call->parentEvent == MOB_EV_RELEASED ||
        data.call->parentEvent == MOB_EV_SWALLOWED ||
        data.call->parentEvent == MOB_EV_STARTED_RECEIVING_DELIVERY ||
        data.call->parentEvent == MOB_EV_FINISHED_RECEIVING_DELIVERY ||
        data.call->parentEvent == MOB_EV_ACTIVE_LEADER_CHANGED
    ) {
        return (Mob*)(data.customData1);
        
    } else if(
        data.call->parentEvent == MOB_EV_RECEIVE_MESSAGE
    ) {
        return(Mob*)(data.customData2);
        
    } else if(
        data.call->parentEvent == MOB_EV_HITBOX_TOUCH_A_N ||
        data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_A ||
        data.call->parentEvent == MOB_EV_HITBOX_TOUCH_N_N ||
        data.call->parentEvent == MOB_EV_DAMAGE
    ) {
        return ((HitboxInteraction*)(data.customData1))->mob2;
        
    }
    
    return nullptr;
}


/**
 * @brief Add a vector of actions onto a given event.
 *
 * @param ev The event to add actions to.
 * @param actions Vector of actions to insert.
 * @param atEnd Are the actions inserted at the end?
 */
void insertEventActions(
    ScriptEvent* ev, const vector<ScriptActionCall*>& actions, bool atEnd
) {
    vector<ScriptActionCall*>::iterator it =
        atEnd ? ev->actions.end() : ev->actions.begin();
    ev->actions.insert(it, actions.begin(), actions.end());
}


/**
 * @brief Loads actions from a data node.
 *
 * @param mt The type of mob the events are going to.
 * @param node The data node.
 * @param outActions The loaded actions are returned here.
 * @param outSettings If not nullptr, the settings for how to load the
 * events are returned here.
 */
void loadActions(
    MobType* mt, DataNode* node,
    vector<ScriptActionCall*>* outActions, Bitmask8* outSettings
) {
    if(outSettings) *outSettings = 0;
    for(size_t a = 0; a < node->getNrOfChildren(); a++) {
        DataNode* actionNode = node->getChild(a);
        if(
            outSettings && actionNode->name == "custom_actions_after"
        ) {
            enableFlag(*outSettings, EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER);
        } else if(
            outSettings && actionNode->name == "global_actions_after"
        ) {
            enableFlag(*outSettings, EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER);
        } else {
            ScriptActionCall* newA = new ScriptActionCall();
            if(newA->loadFromDataNode(actionNode, mt)) {
                outActions->push_back(newA);
            } else {
                delete newA;
            }
        }
    }
    assertActions(*outActions, node);
}


#pragma endregion
