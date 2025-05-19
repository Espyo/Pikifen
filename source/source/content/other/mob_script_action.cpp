/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script action classes and
 * related functions.
 */

#include <algorithm>

#include "mob_script_action.h"

#include "../../content/mob/group_task.h"
#include "../../content/mob/scale.h"
#include "../../content/mob/tool.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::set;


/**
 * @brief Constructs a new mob action call object of a certain type.
 *
 * @param type Type of mob action call.
 */
MobActionCall::MobActionCall(MOB_ACTION type) {

    for(size_t a = 0; a < game.mobActions.size(); a++) {
        if(game.mobActions[a].type == type) {
            action = &(game.mobActions[a]);
            break;
        }
    }
}


/**
 * @brief Constructs a new mob action call object meant to run custom code.
 *
 * @param code The function to run.
 */
MobActionCall::MobActionCall(CustomActionCode code) :
    code(code) {
    
    for(size_t a = 0; a < game.mobActions.size(); a++) {
        if(game.mobActions[a].type == MOB_ACTION_UNKNOWN) {
            action = &(game.mobActions[a]);
            break;
        }
    }
}


/**
 * @brief Loads a mob action call from a data node.
 *
 * @param dn The data node.
 * @param mt Mob type this action's fsm belongs to.
 * @return Whether it was successful.
 */
bool MobActionCall::loadFromDataNode(DataNode* dn, MobType* mt) {

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
    for(size_t a = 0; a < game.mobActions.size(); a++) {
        if(game.mobActions[a].type == MOB_ACTION_UNKNOWN) continue;
        if(game.mobActions[a].name == name) {
            action = &(game.mobActions[a]);
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
bool MobActionCall::run(
    Mob* m, void* customData1, void* customData2
) {
    //Custom code (i.e. instead of text-based script, use actual C++ code).
    if(code) {
        code(m, customData1, customData2);
        return false;
    }
    
    MobActionRunData data(m, this);
    
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



/**
 * @brief Loading code for the arachnorb logic plan mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::arachnorbPlanLogic(MobActionCall &call) {
    if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME);
    } else if(call.args[0] == "forward") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD);
    } else if(call.args[0] == "cw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN);
    } else if(call.args[0] == "ccw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN);
    } else {
        reportEnumError(call, 0);
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the calculation mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::calculate(MobActionCall &call) {
    if(call.args[2] == "+") {
        call.args[2] = i2s(MOB_ACTION_CALCULATE_TYPE_SUM);
    } else if(call.args[2] == "-") {
        call.args[2] = i2s(MOB_ACTION_CALCULATE_TYPE_SUBTRACT);
    } else if(call.args[2] == "*") {
        call.args[2] = i2s(MOB_ACTION_CALCULATE_TYPE_MULTIPLY);
    } else if(call.args[2] == "/") {
        call.args[2] = i2s(MOB_ACTION_CALCULATE_TYPE_DIVIDE);
    } else if(call.args[2] == "%") {
        call.args[2] = i2s(MOB_ACTION_CALCULATE_TYPE_MODULO);
    } else {
        reportEnumError(call, 2);
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the focus mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::focus(MobActionCall &call) {
    return loadMobTargetType(call, 0);
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getAreaInfo(MobActionCall &call) {
    if(call.args[1] == "day_minutes") {
        call.args[1] = i2s(MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES);
    } else if(call.args[1] == "field_pikmin") {
        call.args[1] = i2s(MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN);
    } else {
        call.customError =
            "Unknown info type \"" + call.args[0] + "\"! "
            "Try using \"get_mob_info\" or \"get_event_info\".";
        return false;
    }
    
    return true;
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getEventInfo(MobActionCall &call) {
    if(call.args[1] == "body_part") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART);
    } else if(call.args[1] == "frame_signal") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL);
    } else if(call.args[1] == "hazard") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_HAZARD);
    } else if(call.args[1] == "input_name") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME);
    } else if(call.args[1] == "input_value") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE);
    } else if(call.args[1] == "message") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE);
    } else if(call.args[1] == "other_body_part") {
        call.args[1] = i2s(MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART);
    } else {
        call.customError =
            "Unknown info type \"" + call.args[1] + "\"! "
            "Try using \"get_mob_info\" or \"get_area_info\".";
        return false;
    }
    
    return true;
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::getMobInfo(MobActionCall &call) {

    if(!loadMobTargetType(call, 1)) {
        return false;
    }
    
    if(call.args[2] == "angle") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE);
    } else if(call.args[2] == "chomped_pikmin") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN);
    } else if(call.args[2] == "focus_distance") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE);
    } else if(call.args[2] == "group_task_power") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER);
    } else if(call.args[2] == "health") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH);
    } else if(call.args[2] == "health_ratio") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO);
    } else if(call.args[2] == "id") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_ID);
    } else if(call.args[2] == "latched_pikmin") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN);
    } else if(call.args[2] == "latched_pikmin_weight") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT);
    } else if(call.args[2] == "mob_category") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY);
    } else if(call.args[2] == "mob_type") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE);
    } else if(call.args[2] == "state") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_STATE);
    } else if(call.args[2] == "weight") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT);
    } else if(call.args[2] == "x") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_X);
    } else if(call.args[2] == "y") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_Y);
    } else if(call.args[2] == "z") {
        call.args[2] = i2s(MOB_ACTION_GET_MOB_INFO_TYPE_Z);
    } else {
        call.customError =
            "Unknown info type \"" + call.args[0] + "\"! "
            "Try using \"get_event_info\" or \"get_area_info\".";
        return false;
    }
    
    return true;
}


/**
 * @brief Loading code for the hold focused mob mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::holdFocus(MobActionCall &call) {
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
bool MobActionLoaders::ifFunction(MobActionCall &call) {
    if(call.args[1] == "=") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_EQUAL);
    } else if(call.args[1] == "!=") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_NOT);
    } else if(call.args[1] == "<") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_LESS);
    } else if(call.args[1] == ">") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_MORE);
    } else if(call.args[1] == "<=") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_LESS_E);
    } else if(call.args[1] == ">=") {
        call.args[1] = i2s(MOB_ACTION_IF_OP_MORE_E);
    } else {
        reportEnumError(call, 1);
        return false;
    }
    return true;
}


/**
 * @brief Loads a mob target type from an action call.
 *
 * @param call Mob action call that called this.
 * @param argIdx Index number of the mob target type argument.
 */
bool MobActionLoaders::loadMobTargetType(
    MobActionCall &call, size_t argIdx
) {
    if(call.args[argIdx] == "self") {
        call.args[argIdx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_SELF);
    } else if(call.args[argIdx] == "focus") {
        call.args[argIdx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_FOCUS);
    } else if(call.args[argIdx] == "trigger") {
        call.args[argIdx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_TRIGGER);
    } else if(call.args[argIdx].find("link")==0) {
        call.args.resize(17,"");
        call.args[16]= call.args[argIdx].substr(4);
        call.args[argIdx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_LINK);
    }
    else if(call.args[argIdx] == "parent") {
        call.args[argIdx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_PARENT);
    } else {
        reportEnumError(call, argIdx);
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the move to target mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::moveToTarget(MobActionCall &call) {
    if(call.args[0] == "arachnorb_foot_logic") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC);
    } else if(call.args[0] == "away_from_focused_mob") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS);
    } else if(call.args[0] == "focused_mob") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_FOCUS);
    } else if(call.args[0] == "focused_mob_position") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_FOCUS_POS);
    } else if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_HOME);
    } else if(call.args[0] == "linked_mob_average") {
        call.args[0] = i2s(MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE);
    } else {
        reportEnumError(call, 0);
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the sound playing mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::playSound(MobActionCall &call) {
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
bool MobActionLoaders::receiveStatus(MobActionCall &call) {
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
bool MobActionLoaders::removeStatus(MobActionCall &call) {
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
    MobActionCall &call, size_t argIdx
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
bool MobActionLoaders::setAnimation(MobActionCall &call) {
    size_t aPos = call.mt->animDb->findAnimation(call.args[0]);
    if(aPos == INVALID) {
        call.customError =
            "Unknown animation \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(aPos);
    
    for(size_t a = 1; a < call.args.size(); a++) {
        if(call.args[a] == "no_restart") {
            call.args[a] = i2s(START_ANIM_OPTION_NO_RESTART);
        } else if(call.args[a] == "random_time") {
            call.args[a] = i2s(START_ANIM_OPTION_RANDOM_TIME);
        } else if(call.args[a] == "random_time_on_spawn") {
            call.args[a] = i2s(START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN);
        } else {
            call.args[a] = i2s(START_ANIM_OPTION_NORMAL);
        }
    }
    
    return true;
}


/**
 * @brief Loading code for the far reach setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setFarReach(MobActionCall &call) {
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
bool MobActionLoaders::setHoldable(MobActionCall &call) {
    for(size_t a = 0; a < call.args.size(); a++) {
        if(call.args[a] == "pikmin") {
            call.args[a] = i2s(HOLDABILITY_FLAG_PIKMIN);
        } else if(call.args[a] == "enemies") {
            call.args[a] = i2s(HOLDABILITY_FLAG_ENEMIES);
        } else {
            reportEnumError(call, a);
            return false;
        }
    }
    return true;
}


/**
 * @brief Loading code for the near reach setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::setNearReach(MobActionCall &call) {
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
bool MobActionLoaders::setTeam(MobActionCall &call) {
    size_t teamNr = stringToTeamNr(call.args[0]);
    if(teamNr == INVALID) {
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
bool MobActionLoaders::spawn(MobActionCall &call) {
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
bool MobActionLoaders::stabilizeZ(MobActionCall &call) {
    if(call.args[0] == "lowest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_TYPE_LOWEST);
    } else if(call.args[0] == "highest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST);
    } else {
        reportEnumError(call, 0);
        return false;
    }
    return true;
}


/**
 * @brief Loading code for the chomping start mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool MobActionLoaders::startChomping(MobActionCall &call) {
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
bool MobActionLoaders::startParticles(MobActionCall &call) {
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
bool MobActionLoaders::turnToTarget(MobActionCall &call) {
    if(call.args[0] == "arachnorb_head_logic") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC);
    } else if(call.args[0] == "focused_mob") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_FOCUSED_MOB);
    } else if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_HOME);
    } else {
        reportEnumError(call, 0);
        return false;
    }
    return true;
}


/**
 * @brief Constructs a new mob action param::mob action param object.
 *
 * @param name Name of the parameter.
 * @param type Type of parameter.
 * @param forceConst If true, this must be a constant value.
 * If false, it can also be a var.
 * @param isExtras If true, this is an array of them (minimum amount 0).
 */
MobActionParam::MobActionParam(
    const string &name, const MOB_ACTION_PARAM type,
    bool forceConst, bool isExtras
):
    name(name),
    type(type),
    forceConst(forceConst),
    isExtras(isExtras) {
    
}


/**
 * @brief Constructs a new mob action run data object.
 *
 * @param m The mob responsible.
 * @param call Mob action call that called this.
 */
MobActionRunData::MobActionRunData(Mob* m, MobActionCall* call) :
    m(m),
    call(call) {
    
}


/**
 * @brief Code for the health addition mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::addHealth(MobActionRunData &data) {
    data.m->setHealth(true, false, s2f(data.args[0]));
}


/**
 * @brief Code for the arachnorb logic plan mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::arachnorbPlanLogic(MobActionRunData &data) {
    data.m->arachnorbPlanLogic(
        (MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE) s2i(data.args[0])
    );
}


/**
 * @brief Code for the calculation mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::calculate(MobActionRunData &data) {
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
        
    }
    }
    
    data.m->vars[data.args[0]] = f2s(result);
}


/**
 * @brief Code for the deletion mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::deleteFunction(MobActionRunData &data) {
    data.m->toDelete = true;
}


/**
 * @brief Code for the liquid draining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::drainLiquid(MobActionRunData &data) {
    Sector* sPtr = getSector(data.m->pos, nullptr, true);
    if(!sPtr) return;
    
    vector<Sector*> sectorsToDrain;
    
    sPtr->getNeighborSectorsConditionally(
    [] (Sector * s) -> bool {
        return s->hazard && s->hazard->associatedLiquid;
    },
    sectorsToDrain
    );
    
    for(size_t s = 0; s < sectorsToDrain.size(); s++) {
        sectorsToDrain[s]->drainingLiquid = true;
        sectorsToDrain[s]->liquidDrainLeft =
            GEOMETRY::LIQUID_DRAIN_DURATION;
    }
}


/**
 * @brief Code for the death finish mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::finishDying(MobActionRunData &data) {
    data.m->finishDying();
}


/**
 * @brief Code for the focus mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::focus(MobActionRunData &data) {

    MOB_ACTION_MOB_TARGET_TYPE s = (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    Mob* target = getTargetMob(data, s);
    
    if(!target) return;
    
    data.m->focusOnMob(target);
}


/**
 * @brief Code for the follow path randomly mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::followPathRandomly(MobActionRunData &data) {
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
void MobActionRunners::followPathToAbsolute(MobActionRunData &data) {
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
void MobActionRunners::getAngle(MobActionRunData &data) {
    float centerX = s2f(data.args[1]);
    float centerY = s2f(data.args[2]);
    float focusX = s2f(data.args[3]);
    float focusY = s2f(data.args[4]);
    float angle = getAngle(Point(centerX, centerY), Point(focusX, focusY));
    angle = radToDeg(angle);
    data.m->vars[data.args[0]] = f2s(angle);
}



/**
 * @brief Code for the area info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getAreaInfo(MobActionRunData &data) {
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
void MobActionRunners::getChomped(MobActionRunData &data) {
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
void MobActionRunners::getCoordinatesFromAngle(MobActionRunData &data) {
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
void MobActionRunners::getDistance(MobActionRunData &data) {
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
void MobActionRunners::getEventInfo(MobActionRunData &data) {
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
            *var =
                game.controls.getPlayerActionTypeInternalName(
                    ((PlayerAction*) (data.customData1))->actionTypeId
                );
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.call->parentEvent == MOB_EV_INPUT_RECEIVED) {
            *var = f2s(((PlayerAction*) (data.customData1))->value);
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
void MobActionRunners::getFloorZ(MobActionRunData &data) {
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
void MobActionRunners::getFocusVar(MobActionRunData &data) {
    if(!data.m->focusedMob) return;
    data.m->vars[data.args[0]] =
        data.m->focusedMob->vars[data.args[1]];
}


/**
 * @brief Code for the mob info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getMobInfo(MobActionRunData &data) {
    MOB_ACTION_MOB_TARGET_TYPE s = (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[1]);
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
            *var = 0.0f;
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
        *var = target->type->manifest->internalName;
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
void MobActionRunners::getRandomFloat(MobActionRunData &data) {
    data.m->vars[data.args[0]] =
        f2s(game.rng.f(s2f(data.args[1]), s2f(data.args[2])));
}


/**
 * @brief Code for the integer number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::getRandomInt(MobActionRunData &data) {
    data.m->vars[data.args[0]] =
        i2s(game.rng.i(s2i(data.args[1]), s2i(data.args[2])));
}


/**
 * @brief Returns the mob matching the mob target type.
 *
 * @param data Data about the action call.
 * @param type Type of target.
 */
Mob* getTargetMob(
    MobActionRunData &data, MOB_ACTION_MOB_TARGET_TYPE type
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
    
        if(data.args[16] == "" && !data.m->link_anon_size ==0 && data.m->links.find("0")!= data.m->links.end() && data.m->links["0"]) {
            return data.m->links["0"];
        }
        if(data.args[16] != "" && data.m->links.find(data.args[16]) != data.m->links.end()){
            return data.m->links[data.args[16]];
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
 * @brief Code for the hold focused mob mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::holdFocus(MobActionRunData &data) {
    if(data.m->focusedMob) {
        data.m->hold(
            data.m->focusedMob,
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
void MobActionRunners::ifFunction(MobActionRunData &data) {
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
 * @brief Code for the link with focus mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::linkWithFocus(MobActionRunData &data) {
    if(!data.m->focusedMob) {
        return;
    }
    
    for (const auto& [identifier, link] : data.m->links) {
        if(link == data.m->focusedMob) {
            //Already linked.
            return;
        }
    }
    if (data.args.size() <= 0){
        data.m->push_anonymous_link(data.m->focusedMob);
    }else{
        data.m->links[data.args[0]] = data.m->focusedMob;
    }
}


/**
 * @brief Code for the load focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::loadFocusMemory(MobActionRunData &data) {
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
void MobActionRunners::moveToAbsolute(MobActionRunData &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = data.args.size() > 2 ? s2f(data.args[2]) : data.m->z;
    data.m->chase(Point(x, y), z);
}


/**
 * @brief Code for the move to relative coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::moveToRelative(MobActionRunData &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = (data.args.size() > 2 ? s2f(data.args[2]) : 0);
    Point p = rotatePoint(Point(x, y), data.m->angle);
    data.m->chase(data.m->pos + p, data.m->z + z);
}


/**
 * @brief Code for the move to target mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::moveToTarget(MobActionRunData &data) {
    MOB_ACTION_MOVE_TYPE t = (MOB_ACTION_MOVE_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.m->focusedMob) {
            float a = getAngle(data.m->pos, data.m->focusedMob->pos);
            Point offset = Point(2000, 0);
            offset = rotatePoint(offset, a + TAU / 2.0);
            data.m->chase(data.m->pos + offset, data.m->z);
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS: {
        if(data.m->focusedMob) {
            data.m->chase(&data.m->focusedMob->pos, &data.m->focusedMob->z);
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.m->focusedMob) {
            data.m->chase(data.m->focusedMob->pos, data.m->focusedMob->z);
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_HOME: {
        data.m->chase(data.m->home, data.m->z);
        break;
        
    } case MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC: {
        data.m->arachnorbFootMoveLogic();
        break;
        
    } case MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE: {
        if(data.m->links.empty()) {
            return;
        }
        
        Point des;
        for (const auto& [identifier, link] : data.m->links) {
            if(!link) continue;
            des += link->pos;
        }
        des = des / data.m->links.size();
        
        data.m->chase(des, data.m->z);
        break;
        
    }
    }
}


/**
 * @brief Code for the release order mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::orderRelease(MobActionRunData &data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.runEvent(MOB_EV_RELEASE_ORDER, nullptr, nullptr);
    }
}


/**
 * @brief Code for the sound playing mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::playSound(MobActionRunData &data) {
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
void MobActionRunners::print(MobActionRunData &data) {
    string text = vectorTailToString(data.args, 0);
    printInfo(
        "[DEBUG PRINT] " + data.m->type->name + " says:\n" + text,
        10.0f
    );
}


/**
 * @brief Code for the status reception mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::receiveStatus(MobActionRunData &data) {
    data.m->applyStatusEffect(game.content.statusTypes.list[data.args[0]], false, false);
}


/**
 * @brief Code for the release mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::release(MobActionRunData &data) {
    data.m->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::releaseStoredMobs(MobActionRunData &data) {
    data.m->releaseStoredMobs();
}


/**
 * @brief Code for the status removal mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::removeStatus(MobActionRunData &data) {
    for(size_t s = 0; s < data.m->statuses.size(); s++) {
        if(data.m->statuses[s].type->manifest->internalName == data.args[0]) {
            data.m->statuses[s].toDelete = true;
        }
    }
}


/**
 * @brief Code for the save focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::saveFocusMemory(MobActionRunData &data) {
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
void MobActionRunners::sendMessageToFocus(MobActionRunData &data) {
    if(!data.m->focusedMob) return;
    data.m->sendScriptMessage(data.m->focusedMob, data.args[0]);
}


/**
 * @brief Code for the linked mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::sendMessageToLinks(MobActionRunData &data) {
    string receipient = data.args.size() > 1? data.args[1] : "0";
    for (const auto& [identifier, link] : data.m->links) {
        if(link == data.m) continue;
        if(!link) continue;
        if(receipient != "0" && identifier != receipient) continue;
        data.m->sendScriptMessage(link, data.args[0]);
    }
}


/**
 * @brief Code for the nearby mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::sendMessageToNearby(MobActionRunData &data) {
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
void MobActionRunners::setAnimation(MobActionRunData &data) {
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
void MobActionRunners::setCanBlockPaths(MobActionRunData &data) {
    data.m->setCanBlockPaths(s2b(data.args[0]));
}


/**
 * @brief Code for the far reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setFarReach(MobActionRunData &data) {
    data.m->farReach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the flying setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setFlying(MobActionRunData &data) {
    if(s2b(data.args[0])) {
        enableFlag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    } else {
        disableFlag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Code for the gravity setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setGravity(MobActionRunData &data) {
    data.m->gravityMult = s2f(data.args[0]);
}


/**
 * @brief Code for the health setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHealth(MobActionRunData &data) {
    data.m->setHealth(false, false, s2f(data.args[0]));
}


/**
 * @brief Code for the height setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setHeight(MobActionRunData &data) {
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
void MobActionRunners::setHiding(MobActionRunData &data) {
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
void MobActionRunners::setHoldable(MobActionRunData &data) {
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
void MobActionRunners::setHuntable(MobActionRunData &data) {
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
void MobActionRunners::setLimbAnimation(MobActionRunData &data) {
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
void MobActionRunners::setNearReach(MobActionRunData &data) {
    data.m->nearReach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the radius setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setRadius(MobActionRunData &data) {
    data.m->setRadius(s2f(data.args[0]));
}


/**
 * @brief Code for the sector scroll setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setSectorScroll(MobActionRunData &data) {
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
void MobActionRunners::setShadowVisibility(MobActionRunData &data) {
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
void MobActionRunners::setState(MobActionRunData &data) {
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
void MobActionRunners::setTangible(MobActionRunData &data) {
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
void MobActionRunners::setTeam(MobActionRunData &data) {
    data.m->team = (MOB_TEAM) s2i(data.args[0]);
}


/**
 * @brief Code for the timer setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setTimer(MobActionRunData &data) {
    data.m->setTimer(s2f(data.args[0]));
}


/**
 * @brief Code for the var setting mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::setVar(MobActionRunData &data) {
    data.m->setVar(data.args[0], data.args[1]);
}


/**
 * @brief Code for the show message from var mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::showMessageFromVar(MobActionRunData &data) {
    startGameplayMessage(data.m->vars[data.args[0]], nullptr);
}


/**
 * @brief Code for the spawning mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::spawn(MobActionRunData &data) {
    data.m->spawn(&data.m->type->spawns[s2i(data.args[0])]);
}


/**
 * @brief Code for the z stabilization mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stabilizeZ(MobActionRunData &data) {
    if(data.m->links.empty() || !data.m->links.begin()->second) {
        return;
    }
    
    float bestMatchZ = data.m->links.begin()->second->z;
    MOB_ACTION_STABILIZE_Z_TYPE t =
        (MOB_ACTION_STABILIZE_Z_TYPE) s2i(data.args[0]);
        
    for (const auto& [identifier, link] : data.m->links) {
    
        if(!link) continue;
        
        switch(t) {
        case MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST: {
            if(link->z > bestMatchZ) {
                bestMatchZ = link->z;
            }
            break;
            
        } case MOB_ACTION_STABILIZE_Z_TYPE_LOWEST: {
            if(link->z < bestMatchZ) {
                bestMatchZ = link->z;
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
void MobActionRunners::startChomping(MobActionRunData &data) {
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
void MobActionRunners::startDying(MobActionRunData &data) {
    data.m->startDying();
}


/**
 * @brief Code for the height effect start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startHeightEffect(MobActionRunData &data) {
    data.m->startHeightEffect();
}


/**
 * @brief Code for the particle start mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::startParticles(MobActionRunData &data) {
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
void MobActionRunners::stop(MobActionRunData &data) {
    data.m->stopChasing();
    data.m->stopTurning();
    data.m->stopFollowingPath();
}


/**
 * @brief Code for the chomp stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopChomping(MobActionRunData &data) {
    data.m->chompMax = 0;
    data.m->chompBodyParts.clear();
}


/**
 * @brief Code for the height effect stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopHeightEffect(MobActionRunData &data) {
    data.m->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopParticles(MobActionRunData &data) {
    data.m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_SCRIPT);
}


/**
 * @brief Code for the sound stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopSound(MobActionRunData &data) {
    game.audio.destroySoundSource(s2i(data.args[0]));
}


/**
 * @brief Code for the vertical stopping mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::stopVertically(MobActionRunData &data) {
    data.m->speedZ = 0;
}


/**
 * @brief Code for the focus storing mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::storeFocusInside(MobActionRunData &data) {
    if(data.m->focusedMob && !data.m->focusedMob->isStoredInsideMob()) {
        data.m->storeMobInside(data.m->focusedMob);
    }
}


/**
 * @brief Code for the swallow mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::swallow(MobActionRunData &data) {
    data.m->swallowChompedPikmin(s2i(data.args[0]));
}


/**
 * @brief Code for the swallow all mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::swallowAll(MobActionRunData &data) {
    data.m->swallowChompedPikmin(data.m->chompingMobs.size());
}


/**
 * @brief Code for the teleport to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void MobActionRunners::teleportToAbsolute(MobActionRunData &data) {
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
void MobActionRunners::teleportToRelative(MobActionRunData &data) {
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
void MobActionRunners::throwFocus(MobActionRunData &data) {
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
void MobActionRunners::turnToAbsolute(MobActionRunData &data) {
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
void MobActionRunners::turnToRelative(MobActionRunData &data) {
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
void MobActionRunners::turnToTarget(MobActionRunData &data) {
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
    const vector<MobActionCall*> &actions, const DataNode* dn
) {
    //Check if the "if"-related actions are okay.
    int ifLevel = 0;
    for(size_t a = 0; a < actions.size(); a++) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            ifLevel++;
            break;
        } case MOB_ACTION_ELSE: {
            if(ifLevel == 0) {
                game.errors.report(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            break;
        } case MOB_ACTION_END_IF: {
            if(ifLevel == 0) {
                game.errors.report(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            ifLevel--;
            break;
        } default: {
            break;
        }
        }
    }
    if(ifLevel > 0) {
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
            const string &name = actions[a]->args[0];
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
            const string &name = actions[a]->args[0];
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
 * @brief Gets the mob that triggered an event.
 *
 * @param data Data about the action call.
 * @return The mob.
 */
Mob* getTriggerMob(MobActionRunData &data) {
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
        data.call->parentEvent == MOB_EV_FINISHED_RECEIVING_DELIVERY
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
    MobEvent* ev, const vector<MobActionCall*> &actions, bool atEnd
) {
    vector<MobActionCall*>::iterator it =
        atEnd ? ev->actions.end() : ev->actions.begin();
    ev->actions.insert(it, actions.begin(), actions.end());
}


/**
 * @brief Loads actions from a data node.
 *
 * @param mt The type of mob the events are going to.
 * @param node The data node.
 * @param outActions The oaded actions are returned here.
 * @param outSettings If not nullptr, the settings for how to load the
 * events are returned here.
 */
void loadActions(
    MobType* mt, DataNode* node,
    vector<MobActionCall*>* outActions, Bitmask8* outSettings
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
            MobActionCall* newA = new MobActionCall();
            if(newA->loadFromDataNode(actionNode, mt)) {
                outActions->push_back(newA);
            } else {
                delete newA;
            }
        }
    }
    assertActions(*outActions, node);
}
