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

#include "action_types.hpp"

#include "../../content/mob/group_task.hpp"
#include "../../content/mob/scale.hpp"
#include "../../content/mob/tool.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/general_utils.hpp"
#include "../../util/string_utils.hpp"


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
#pragma region Action runner functions


/**
 * @brief Code for the absolute number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::absoluteNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = fabs(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the health addition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::addHealth(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& healthArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->setHealth(true, false, s2f(healthArg));
}


/**
 * @brief Code for the list item retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::addListItem(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& newItemArg = data.args[2];
    const string& numberArg = data.args[3];
    const string& delArg = data.args[4];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int idx = s2i(numberArg) - 1;
    if(!isIdxValid(idx, items)) idx = items.size();
    
    items.insert(items.begin() + idx, newItemArg);
    string newListStr = join(items, delChar);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, newListStr);
}


/**
 * @brief Code for the string addition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::addToString(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& baseStrArg = data.args[1];
    const string& newContentArg = data.args[2];
    const string& addSpaceArg = data.args[3];
    
    //Main logic.
    string result = baseStrArg;
    if(s2b(addSpaceArg)) result += " ";
    result += newContentArg;
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the arachnorb logic plan script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::arachnorbPlanLogic(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& typeArg = data.args[0];
    
    //Main logic.
    bool typeFound;
    SCRIPT_ACTION_ARACHNORB_PLAN_LOGIC_TYPE type =
        enumGetValue(
            scriptActionArachnorbPlanLogicTypeINames, typeArg, &typeFound
        );
        
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown arachnorb plan logic type \"" + typeArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->arachnorbPlanLogic(type);
}


/**
 * @brief Code for the being chomped script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::beChomped(ScriptActionInstRunData& data) {
    //Main logic.
    if(data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_EAT) {
        ((Mob*) (data.customData1))->chomp(
            data.scriptVM->getRunnerMob(),
            (Hitbox*) (data.customData2)
        );
    }
}


/**
 * @brief Code for the calculation script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::calculate(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& lhsArg = data.args[1];
    const string& opArg = data.args[2];
    const string& rhsArg = data.args[3];
    
    //Main logic.
    bool opFound;
    SCRIPT_ACTION_CALCULATE_TYPE op =
        enumGetValue(scriptActionCalculateTypeINames, opArg, &opFound);
        
    if(!opFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown operator \"" + opArg + "\"!"
        );
        return;
    }
    
    float lhs = s2f(lhsArg);
    float rhs = s2f(rhsArg);
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
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the ceil number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::ceilNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = ceil(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the clamp number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::clampNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    const string& lowerArg = data.args[2];
    const string& upperArg = data.args[3];
    
    //Main logic.
    float lower = s2f(lowerArg);
    float upper = s2f(upperArg);
    if(lower > upper) std::swap(lower, upper);
    float result = std::clamp((float) s2f(numberArg), lower, upper);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the variable clearing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::clearVar(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& varArg = data.args[0];
    
    //Main logic.
    if(data.scriptVM->getRunnerScriptVM()->vars.contains(varArg)) {
        data.scriptVM->getRunnerScriptVM()->vars.setValue(varArg, string());
    }
}


/**
 * @brief Code for the deletion script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::deleteAction(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->toDelete = true;
}


/**
 * @brief Code for the liquid draining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::drainLiquid(ScriptActionInstRunData& data) {
    //Main logic.
    Sector* sPtr =
        getSector(data.scriptVM->getRunnerMob()->center, nullptr, true);
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
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    const string& methodArg = data.args[2];
    
    //Main logic.
    bool methodFound;
    EASE_METHOD method =
        enumGetValue(easeMethodINames, methodArg, &methodFound);
        
    if(!methodFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown easing method \"" + methodArg + "\"!"
        );
        return;
    }
    
    float result = ease(s2f(numberArg), method);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the do-while loop condition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::endDoWhile(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& lhsArg = data.args[0];
    const string& opArg = data.args[1];
    string rhsArg = vectorTailToString(data.args, 2);
    
    //Main logic.
    bool opFound;
    SCRIPT_ACTION_IF_OP op =
        enumGetValue(scriptActionIfOpINames, opArg, &opFound);
    if(!opFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown operator \"" + opArg + "\"!"
        );
        return;
    }
    
    bool result = ScriptActionUtils::doScriptCondition(lhsArg, op, rhsArg);
    
    //Store the result.
    data.returnValue = result;
}


/**
 * @brief Code for the death finish script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::finishDying(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->finishDying();
}


/**
 * @brief Code for the floor number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::floorNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = floor(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the focus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::focus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& targetTypeArg = data.args[0];
    
    //Main logic.
    SCRIPT_ACTION_MOB_TARGET_TYPE targetType =
        ScriptActionUtils::getMobTargetType(data, targetTypeArg);
    Mob* target = ScriptActionUtils::getTargetMob(data, targetType);
    
    if(!target) return;
    data.scriptVM->getRunnerScriptVM()->focusOnMob(target);
}


/**
 * @brief Code for the focus on ID script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::focusOnId(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& idArg = data.args[0];
    
    //Main logic.
    Mob* target = game.states.gameplay->getMobById(s2i(idArg));
    
    if(!target) return;
    data.scriptVM->getRunnerScriptVM()->focusOnMob(target);
}


/**
 * @brief Code for the follow mob as leader script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followMobAsLeader(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& targetTypeArg = data.args[0];
    const string& silentArg = data.args[1];
    
    //Main logic.
    SCRIPT_ACTION_MOB_TARGET_TYPE targetType =
        ScriptActionUtils::getMobTargetType(data, targetTypeArg);
    Mob* target = ScriptActionUtils::getTargetMob(data, targetType);
    
    if(!target) return;
    if(target->health <= 0.0f) return;
    
    data.scriptVM->getRunnerMob()->leaveGroup();
    bool silent = s2b(silentArg);
    
    if(
        data.scriptVM->getRunnerMob()->type->category->id ==
        MOB_CATEGORY_PIKMIN
    ) {
        data.scriptVM->getRunnerMob()->scriptVM.fsm.runEvent(
            FSM_EV_WHISTLED, (void*) target, (void*) silent
        );
    } else {
        target->addToGroup(data.scriptVM->getRunnerMob());
    }
}


/**
 * @brief Code for the follow path randomly script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followPathRandomly(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& labelArg = data.args[0];
    
    //Main logic.
    //We need to decide what the final stop is going to be.
    //First, get all eligible stops.
    vector<PathStop*> choices;
    if(labelArg.empty()) {
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
            if(sPtr->label == labelArg) {
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
                Distance(
                    choices[c]->center, data.scriptVM->getRunnerMob()->center
                ) > PATHS::DEF_CHASE_TARGET_DISTANCE
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
    settings.targetPoint =
        finalStop ? finalStop->center : data.scriptVM->getRunnerMob()->center;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    settings.label = labelArg;
    data.scriptVM->getRunnerMob()->followPath(
        settings, data.scriptVM->getRunnerMob()->getBaseSpeed(),
        data.scriptVM->getRunnerMob()->type->acceleration
    );
}


/**
 * @brief Code for the follow path to absolute script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::followPathToAbsolute(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& labelArg = data.args[2];
    
    //Main logic.
    float x = s2f(xArg);
    float y = s2f(yArg);
    
    PathFollowSettings settings;
    settings.targetPoint = Point(x, y);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    settings.label = labelArg;
    
    data.scriptVM->getRunnerMob()->followPath(
        settings, data.scriptVM->getRunnerMob()->getBaseSpeed(),
        data.scriptVM->getRunnerMob()->type->acceleration
    );
}


/**
 * @brief Code for the for loop condition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::forAction(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& iteratorVarArg = data.args[0];
    const string& targetArg = data.args[1];
    const string& startingValueArg = data.args[2];
    const string& jumpArg = data.args[3];
    
    //Main logic.
    int startingValue = s2i(startingValueArg);
    int jump = s2i(jumpArg);
    
    int iterator = startingValue;
    data.scriptVM->getRunnerScriptVM()->vars.getValue(iteratorVarArg, iterator);
    if(game.scriptExecAuxData.forLoopEntryNeedsIncrement) {
        iterator += jump;
        game.scriptExecAuxData.forLoopEntryNeedsIncrement = false;
    } else {
        iterator = startingValue;
    }
    data.scriptVM->getRunnerScriptVM()->vars.setValue(iteratorVarArg, iterator);
    
    bool result =
        ScriptActionUtils::doScriptCondition(
            i2s(iterator),
            jump >= 0 ?
            SCRIPT_ACTION_IF_OP_LESS_E :
            SCRIPT_ACTION_IF_OP_MORE_E,
            targetArg
        );
        
    //Store the result.
    data.returnValue = result;
}


/**
 * @brief Code for the for-each loop condition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::forEach(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& iteratorVarArg = data.args[0];
    const string& itemVarArg = data.args[1];
    const string& listArg = data.args[2];
    const string& delArg = data.args[3];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    int iterator = 1;
    data.scriptVM->getRunnerScriptVM()->vars.getValue(iteratorVarArg, iterator);
    if(game.scriptExecAuxData.forLoopEntryNeedsIncrement) {
        iterator++;
        game.scriptExecAuxData.forLoopEntryNeedsIncrement = false;
    } else {
        iterator = 1;
    }
    data.scriptVM->getRunnerScriptVM()->vars.setValue(iteratorVarArg, iterator);
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int idx = iterator - 1;
    if(!isIdxValid(idx, items)) idx = items.size() - 1;
    string item;
    
    if(isIdxValid(idx, items)) {
        item = trimSpaces(items[(size_t) idx]);
    }
    
    data.scriptVM->getRunnerScriptVM()->vars.setValue(itemVarArg, item);
    
    bool result = iterator <= (int) items.size();
    
    //Store the result.
    data.returnValue = result;
}


/**
 * @brief Code for the angle retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngle(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& xArg = data.args[1];
    const string& yArg = data.args[2];
    const string& focusXArg = data.args[3];
    const string& focusYArg = data.args[4];
    
    //Main logic.
    Point center(s2f(xArg), s2f(yArg));
    Point focus(s2f(focusXArg), s2f(focusYArg));
    float angle = getAngle(center, focus);
    angle = radToDeg(angle);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, angle);
}


/**
 * @brief Code for the angle closest difference retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngleCwDiff(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& angle1Arg = data.args[1];
    const string& angle2Arg = data.args[2];
    
    //Main logic.
    float angle1 = degToRad(s2f(angle1Arg));
    float angle2 = degToRad(s2f(angle2Arg));
    float diff = ::getAngleCwDiff(angle1, angle2);
    diff = radToDeg(diff);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, diff);
}


/**
 * @brief Code for the angle smallest difference retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAngleSmallestDiff(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& angle1Arg = data.args[1];
    const string& angle2Arg = data.args[2];
    
    //Main logic.
    float angle1 = degToRad(s2f(angle1Arg));
    float angle2 = degToRad(s2f(angle2Arg));
    float diff = ::getAngleSmallestDiff(angle1, angle2);
    diff = radToDeg(diff);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, diff);
}



/**
 * @brief Code for the area info retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getAreaInfo(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& typeArg = data.args[1];
    
    //Main logic.
    string result;
    bool typeFound;
    SCRIPT_ACTION_GET_AREA_INFO_TYPE type =
        enumGetValue(scriptActionGetAreaInfoTypeINames, typeArg, &typeFound);
        
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch(type) {
    case SCRIPT_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES: {
        result = i2s(game.states.gameplay->dayMinutes);
        break;
        
    } case SCRIPT_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN: {
        result = i2s(game.states.gameplay->mobs.pikmin.size());
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the coordinate from angle retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getCoordinatesFromAngle(
    ScriptActionInstRunData& data
) {
    //Get the arguments.
    const string& xDestVarArg = data.args[0];
    const string& yDestVarArg = data.args[1];
    const string& angleArg = data.args[2];
    const string& magnitudeArg = data.args[3];
    
    //Main logic.
    float angle = s2f(angleArg);
    angle = degToRad(angle);
    float magnitude = s2f(magnitudeArg);
    Point p = angleToCoordinates(angle, magnitude);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(xDestVarArg, p.x);
    data.scriptVM->getRunnerScriptVM()->vars.setValue(yDestVarArg, p.y);
}


/**
 * @brief Code for the distance retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getDistance(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& centerXArg = data.args[1];
    const string& centerYArg = data.args[2];
    const string& focusXArg = data.args[3];
    const string& focusYArg = data.args[4];
    
    //Main logic.
    float centerX = s2f(centerXArg);
    float centerY = s2f(centerYArg);
    float focusX = s2f(focusXArg);
    float focusY = s2f(focusYArg);
    float dist =
        Distance(Point(centerX, centerY), Point(focusX, focusY)).toFloat();
        
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, dist);
}


/**
 * @brief Code for the event info retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getEventInfo(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& typeArg = data.args[1];
    
    //Main logic.
    string result;
    bool typeFound;
    SCRIPT_ACTION_GET_EV_INFO_TYPE type =
        enumGetValue(scriptActionGetEvInfoTypeINames, typeArg, &typeFound);
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch(type) {
    case SCRIPT_ACTION_GET_EV_INFO_TYPE_BODY_PART: {
        if(
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_A_N ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_A ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_N ||
            data.actionDef->parentEvent == FSM_EV_DAMAGE
        ) {
            result =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h1->bodyPartName;
        } else if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OBJECT ||
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OPPONENT ||
            data.actionDef->parentEvent == FSM_EV_THROWN_PIKMIN_LANDED
        ) {
            if(data.scriptVM->getRunnerMob()) {
                result =
                    data.scriptVM->getRunnerMob()->getClosestHitbox(
                        ((Mob*)(data.customData1))->center
                    )->bodyPartName;
            }
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL: {
        if(data.actionDef->parentEvent == FSM_EV_FRAME_SIGNAL) {
            result = i2s(*((size_t*)(data.customData1)));
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_HAZARD: {
        if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_HAZARD ||
            data.actionDef->parentEvent == FSM_EV_LEFT_HAZARD
        ) {
            result = ((Hazard*)data.customData1)->manifest->internalName;
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_INPUT_NAME: {
        if(data.actionDef->parentEvent == FSM_EV_INPUT_RECEIVED) {
            PLAYER_ACTION_TYPE playerActionTypeId =
                (PLAYER_ACTION_TYPE)
                ((Inpution::Action*) (data.customData1))->actionTypeId;
            result =
                game.controls.getActionTypeById(
                    playerActionTypeId
                ).internalName;
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.actionDef->parentEvent == FSM_EV_INPUT_RECEIVED) {
            result = f2s(((Inpution::Action*) (data.customData1))->value);
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_SCRIPT_MESSAGE: {
        if(data.actionDef->parentEvent == FSM_EV_RECEIVE_SCRIPT_MESSAGE) {
            result = *((string*)(data.customData1));
        }
        break;
        
    } case SCRIPT_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART: {
        if(
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_A_N ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_A ||
            data.actionDef->parentEvent == FSM_EV_HITBOX_TOUCH_N_N ||
            data.actionDef->parentEvent == FSM_EV_DAMAGE
        ) {
            result =
                (
                    (HitboxInteraction*)(data.customData1)
                )->h2->bodyPartName;
        } else if(
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OBJECT ||
            data.actionDef->parentEvent == FSM_EV_TOUCHED_OPPONENT ||
            data.actionDef->parentEvent == FSM_EV_THROWN_PIKMIN_LANDED
        ) {
            if(data.customData1 && data.scriptVM->getRunnerMob()) {
                result =
                    ((Mob*)(data.customData1))->getClosestHitbox(
                        data.scriptVM->getRunnerMob()->center
                    )->bodyPartName;
            }
        }
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the floor Z retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getFloorZ(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& xArg = data.args[1];
    const string& yArg = data.args[2];
    
    //Main logic.
    Point p(s2f(xArg), s2f(yArg));
    Sector* s = getSector(p, nullptr, true);
    float result = s ? s->floorZ : 0.0f;
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the focused mob var retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getFocusVar(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& focusVarArg = data.args[1];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) return;
    string result;
    data.scriptVM->getRunnerScriptVM()->focusedMob->scriptVM.vars.getValue(
        focusVarArg, result
    );
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the leader Pikmin count retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getLeaderPikminCount(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& targetArg = data.args[1];
    const string& typeArg = data.args[2];
    
    //Main logic.
    SCRIPT_ACTION_MOB_TARGET_TYPE targetType =
        ScriptActionUtils::getMobTargetType(data, targetArg);
    Mob* target = ScriptActionUtils::getTargetMob(data, targetType);
    
    if(!target) return;
    
    size_t result;
    if(!target->group) {
        result = 0;
    } else if(typeArg.empty()) {
        result = target->group->members.size();
    } else {
        result = target->group->getAmountByType(typeArg);
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the list item retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getListItem(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& numberArg = data.args[2];
    const string& delArg = data.args[3];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int idx = s2i(numberArg) - 1;
    if(!isIdxValid(idx, items)) idx = items.size() - 1;
    string item;
    
    if(isIdxValid(idx, items)) {
        item = trimSpaces(items[(size_t) idx]);
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, item);
}


/**
 * @brief Code for the list item number retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getListItemNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& itemArg = data.args[2];
    const string& delArg = data.args[3];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int number = 0;
    
    forIdx(i, items) {
        if(items[i] == itemArg) {
            number = i + 1;
            break;
        }
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, number);
}


/**
 * @brief Code for the list size retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getListSize(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& delArg = data.args[2];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    size_t size = getSplitCount(listArg, delChar, true);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, size);
}


/**
 * @brief Code for the misc info retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMiscInfo(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& typeArg = data.args[1];
    
    //Main logic.
    string result;
    bool typeFound;
    SCRIPT_ACTION_GET_MISC_INFO_TYPE type =
        enumGetValue(scriptActionGetMiscInfoTypeINames, typeArg, &typeFound);
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch(type) {
    case SCRIPT_ACTION_GET_MISC_INFO_TYPE_DAY_MINUTES: {
        result = i2s(game.states.gameplay->dayMinutes);
        break;
        
    } case SCRIPT_ACTION_GET_MISC_INFO_TYPE_DELTA_T: {
        result = f2s(game.deltaT);
        break;
        
    } case SCRIPT_ACTION_GET_MISC_INFO_TYPE_FIELD_PIKMIN: {
        result = i2s(game.states.gameplay->mobs.pikmin.size());
        break;
        
    } case SCRIPT_ACTION_GET_MISC_INFO_PLAYER_1_LEADER_ID:
    case SCRIPT_ACTION_GET_MISC_INFO_PLAYER_2_LEADER_ID:
    case SCRIPT_ACTION_GET_MISC_INFO_PLAYER_3_LEADER_ID:
    case SCRIPT_ACTION_GET_MISC_INFO_PLAYER_4_LEADER_ID: {
        int playerNr =
            (int) type -
            (int) SCRIPT_ACTION_GET_MISC_INFO_PLAYER_1_LEADER_ID +
            1;
        result = "0";
        forIdx(p, game.states.gameplay->players) {
            Player& player = game.states.gameplay->players[p];
            if(player.playerNr != playerNr) continue;
            if(!player.leaderPtr) continue;
            result = i2s(player.leaderPtr->id);
            break;
        }
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the mission metric retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMissionMetric(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& metricArg = data.args[1];
    const string& numberArg = data.args[2];
    const string& getAutoTargetArg = data.args[3];
    
    //Main logic.
    if(game.curArea->type != AREA_TYPE_MISSION) {
        return;
    }
    
    string result;
    bool metricFound;
    MISSION_METRIC metric =
        enumGetValue(missionMetricINames, metricArg, &metricFound);
    if(!metricFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown mission metric \"" + metricArg + "\"!"
        );
        return;
    }
    
    MissionMetricType* metricTypePtr = game.missionMetricTypes[metric];
    size_t indexParam = s2i(numberArg) - 1;
    
    if(s2b(getAutoTargetArg)) {
        result = i2s(metricTypePtr->getTarget(indexParam, INVALID));
    } else {
        result = i2s(metricTypePtr->getAmount(indexParam));
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the region mob ID retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMobIdsInRegion(
    ScriptActionInstRunData& data
) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& regionNumberArg = data.args[1];
    
    //Main logic.
    vector<string> idsStrs;
    size_t regionIdx = s2i(regionNumberArg) - 1;
    if(regionIdx >= game.states.gameplay->areaRegions.size()) {
        ScriptActionUtils::reportActionError(
            data,
            "Area region number " + regionNumberArg + " doesn't exist!"
        );
        return;
    }
    AreaRegion* rPtr = game.curArea->regions[regionIdx];
    
    forIdx(m, game.states.gameplay->mobs.all) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        
        bool isInside;
        getClosestPointInRotatedRectangle(
            mPtr->center, Rect(rPtr->pose.pos, rPtr->pose.size),
            rPtr->pose.angle, &isInside
        );
        if(!isInside) continue;
        
        idsStrs.push_back(i2s(mPtr->id));
    }
    string result = join(idsStrs, ",");
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the var mob ID retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMobIdsWithVar(
    ScriptActionInstRunData& data
) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& varArg = data.args[1];
    const string& valueArg = data.args[2];
    
    //Main logic.
    vector<string> idsStrs;
    forIdx(m, game.states.gameplay->mobs.all) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        const map<string, string>& mPtrVars = mPtr->scriptVM.vars.toMap();
        
        auto it = mPtrVars.find(varArg);
        if(it == mPtrVars.end()) continue;
        if(!valueArg.empty()) {
            if(it->second != valueArg) continue;
        }
        
        idsStrs.push_back(i2s(mPtr->id));
    }
    string result = join(idsStrs, ",");
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the mob info retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getMobInfo(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& targetArg = data.args[1];
    const string& typeArg = data.args[2];
    
    //Main logic.
    SCRIPT_ACTION_MOB_TARGET_TYPE targetType =
        ScriptActionUtils::getMobTargetType(data, targetArg);
    Mob* target = ScriptActionUtils::getTargetMob(data, targetType);
    
    if(!target) return;
    
    string result;
    bool typeFound;
    SCRIPT_ACTION_GET_MOB_INFO_TYPE type =
        enumGetValue(scriptActionGetMobInfoTypeINames, typeArg, &typeFound);
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch(type) {
    case SCRIPT_ACTION_GET_MOB_INFO_TYPE_ANGLE: {
        result = f2s(radToDeg(target->angle));
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_ANIM: {
        if(target->anim.curAnim) {
            result = target->anim.curAnim->name;
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN: {
        result = i2s(target->chompingMobs.size());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->scriptVM.focusedMob) {
            float d =
                Distance(target->center, target->scriptVM.focusedMob->center).
                toFloat();
            result = f2s(d);
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER: {
        if(target->type->category->id == MOB_CATEGORY_GROUP_TASKS) {
            result = f2s(((GroupTask*)target)->getPower());
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_HEALTH: {
        result = i2s(target->health);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO: {
        if(target->maxHealth != 0.0f) {
            result = f2s(target->health / target->maxHealth);
        } else {
            result = "0";
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_ID: {
        result = i2s(target->id);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN: {
        result = i2s(target->getLatchedPikminAmount());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT: {
        result = i2s(target->getLatchedPikminWeight());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MAX_HEALTH: {
        result = i2s(target->maxHealth);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY: {
        result = target->type->category->internalName;
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_LINK_IDS: {
        vector<string> linkIdsStrs;
        forIdx(l, target->links) {
            linkIdsStrs.push_back(i2s(target->links[l]->id));
        }
        result = join(linkIdsStrs, ",");
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE: {
        if(target->type->manifest) {
            result = target->type->manifest->internalName;
        } else {
            result = "";
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_STATE: {
        result = target->scriptVM.fsm.curState->name;
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_TEAM: {
        result = enumGetName(mobTeamINames, target->team);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_WEIGHT: {
        if(target->type->category->id == MOB_CATEGORY_SCALES) {
            Scale* sPtr = (Scale*)(target);
            result = i2s(sPtr->calculateCurWeight());
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_X: {
        result = f2s(target->center.x);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Y: {
        result = f2s(target->center.y);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Z: {
        result = f2s(target->bottomZ);
        break;
    }
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the float number randomization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getRandomFloat(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& minArg = data.args[1];
    const string& maxArg = data.args[2];
    
    //Main logic.
    float result = game.rng.f(s2f(minArg), s2f(maxArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the integer number randomization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getRandomInt(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& minArg = data.args[1];
    const string& maxArg = data.args[2];
    
    //Main logic.
    int result = game.rng.i(s2i(minArg), s2i(maxArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the var presence retrieval script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getVarPresence(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& varArg = data.args[1];
    
    //Main logic.
    bool exists = data.scriptVM->getRunnerScriptVM()->vars.contains(varArg);
    if(exists) {
        string value;
        data.scriptVM->getRunnerScriptVM()->vars.getValue(varArg, value);
        exists = !value.empty();
    }
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, exists);
}


/**
 * @brief Code for the hold focused mob script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::holdFocus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& bodyPartArg = data.args[0];
    const string& aboveArg = data.args[1];
    const string& rotationArg = data.args[2];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) {
        return;
    }
    
    size_t partIdx =
        data.scriptVM->getRunnerMob()->anim.animDb->findBodyPart(bodyPartArg);
    if(partIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown body part \"" + bodyPartArg + "\"!"
        );
        return;
    }

    bool rotationFound;
    HOLD_ROTATION_METHOD rotationMethod =
        enumGetValue(holdRotationMethodINames, rotationArg, &rotationFound);
        
    if(!rotationFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown rotation method \"" + rotationArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->hold(
        data.scriptVM->getRunnerScriptVM()->focusedMob,
        HOLD_TYPE_PURPOSE_GENERAL,
        partIdx, 0.0f, 0.0f, 0.5f, s2b(aboveArg),
        rotationMethod
    );
}


/**
 * @brief Code for the "if" script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::ifAction(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& lhsArg = data.args[0];
    const string& opArg = data.args[1];
    string rhsArg = vectorTailToString(data.args, 2);
    
    //Main logic.
    bool opFound;
    SCRIPT_ACTION_IF_OP op =
        enumGetValue(scriptActionIfOpINames, opArg, &opFound);
    if(!opFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown operator \"" + opArg + "\"!"
        );
        return;
    }
    
    bool result = ScriptActionUtils::doScriptCondition(lhsArg, op, rhsArg);
    
    //Store the result.
    data.returnValue = result;
}


/**
 * @brief Code for the interpolate number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::interpolateNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& inputArg = data.args[1];
    const string& inputStartArg = data.args[2];
    const string& inputEndArg = data.args[3];
    const string& outputStartArg = data.args[4];
    const string& outputEndArg = data.args[5];
    
    //Main logic.
    float result =
        ::interpolateNumber(
            s2f(inputArg), s2f(inputStartArg), s2f(inputEndArg),
            s2f(outputStartArg), s2f(outputEndArg)
        );
        
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the link with focus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::linkWithFocus(ScriptActionInstRunData& data) {
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) {
        return;
    }
    
    forIdx(l, data.scriptVM->getRunnerMob()->links) {
        if(
            data.scriptVM->getRunnerMob()->links[l] ==
            data.scriptVM->getRunnerScriptVM()->focusedMob
        ) {
            //Already linked.
            return;
        }
    }
    
    data.scriptVM->getRunnerMob()->links.push_back(
        data.scriptVM->getRunnerScriptVM()->focusedMob
    );
}


/**
 * @brief Code for the load focused mob memory script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::loadFocusMemory(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& slotArg = data.args[0];
    
    //Main logic.
    string varName = "_focus_memory_" + slotArg;
    size_t id;
    if(!data.scriptVM->getRunnerScriptVM()->vars.getValue(varName, id)) {
        return;
    }
    Mob* target = game.states.gameplay->getMobById(id);
    if(!target) return;
    data.scriptVM->getRunnerScriptVM()->focusOnMob(target);
}


/**
 * @brief Code for the max number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::maxNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& firstNrArg = data.args[1];
    const string& secondNrArg = data.args[2];
    
    //Main logic.
    float result = std::max(s2f(firstNrArg), s2f(secondNrArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the min number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::minNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& firstNrArg = data.args[1];
    const string& secondNrArg = data.args[2];
    
    //Main logic.
    float result = std::min(s2f(firstNrArg), s2f(secondNrArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the move to absolute coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::moveToAbsolute(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    float x = s2f(xArg);
    float y = s2f(yArg);
    float z = zArg.empty() ? data.scriptVM->getRunnerMob()->bottomZ : s2f(zArg);
    data.scriptVM->getRunnerMob()->chase(
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
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    float x = s2f(xArg);
    float y = s2f(yArg);
    float z = zArg.empty() ? 0.0f : s2f(zArg);
    Point p = rotatePoint(Point(x, y), data.scriptVM->getRunnerMob()->angle);
    data.scriptVM->getRunnerMob()->chase(
        data.scriptVM->getRunnerMob()->center + p,
        data.scriptVM->getRunnerMob()->bottomZ + z,
        CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
    );
}


/**
 * @brief Code for the move to target script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::moveToTarget(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& targetArg = data.args[0];
    
    //Main logic.
    bool targetFound;
    SCRIPT_ACTION_MOVE_TYPE targetType =
        enumGetValue(scriptActionMoveTypeINames, targetArg, &targetFound);
    if(!targetFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown target type \"" + targetArg + "\"!"
        );
        return;
    }
    
    switch(targetType) {
    case SCRIPT_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.scriptVM->getRunnerScriptVM()->focusedMob) {
            float a =
                getAngle(
                    data.scriptVM->getRunnerMob()->center,
                    data.scriptVM->getRunnerScriptVM()->focusedMob->center
                );
            Point offset = Point(2000, 0);
            offset = rotatePoint(offset, a + TAU / 2.0);
            data.scriptVM->getRunnerMob()->chase(
                data.scriptVM->getRunnerMob()->center + offset,
                data.scriptVM->getRunnerMob()->bottomZ,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->getRunnerMob()->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_FOCUS: {
        if(data.scriptVM->getRunnerScriptVM()->focusedMob) {
            data.scriptVM->getRunnerMob()->chase(
                &data.scriptVM->getRunnerScriptVM()->focusedMob->center,
                &data.scriptVM->getRunnerScriptVM()->focusedMob->bottomZ,
                Point(), 0.0f,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->getRunnerMob()->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.scriptVM->getRunnerScriptVM()->focusedMob) {
            data.scriptVM->getRunnerMob()->chase(
                data.scriptVM->getRunnerScriptVM()->focusedMob->center,
                data.scriptVM->getRunnerScriptVM()->focusedMob->bottomZ,
                CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
            );
        } else {
            data.scriptVM->getRunnerMob()->stopChasing();
        }
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_HOME: {
        data.scriptVM->getRunnerMob()->chase(
            data.scriptVM->getRunnerMob()->home,
            data.scriptVM->getRunnerMob()->bottomZ,
            CHASE_FLAG_ACCEPT_LOWER_Z_GROUNDED
        );
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC: {
        data.scriptVM->getRunnerMob()->arachnorbFootMoveLogic();
        break;
        
    } case SCRIPT_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE: {
        if(data.scriptVM->getRunnerMob()->links.empty()) {
            return;
        }
        
        Point des;
        forIdx(l, data.scriptVM->getRunnerMob()->links) {
            if(!data.scriptVM->getRunnerMob()->links[l]) continue;
            des += data.scriptVM->getRunnerMob()->links[l]->center;
        }
        des = des / data.scriptVM->getRunnerMob()->links.size();
        
        data.scriptVM->getRunnerMob()->chase(
            des,
            data.scriptVM->getRunnerMob()->bottomZ,
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
    //Main logic.
    Mob* holder = data.scriptVM->getRunnerMob()->holder.m;
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
    //Get the arguments.
    const string& soundArg = data.args[0];
    const string& destVarArg = data.args[1];
    
    //Main logic.
    size_t soundDataIdx = INVALID;
    forIdx(s, data.scriptVM->getRunnerMob()->type->sounds) {
        if(data.scriptVM->getRunnerMob()->type->sounds[s].name == soundArg) {
            soundDataIdx = s;
            break;
        }
    }
    
    if(soundDataIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown sound info block \"" + soundArg + "\"!"
        );
        return;
    }
    
    size_t soundId = data.scriptVM->getRunnerMob()->playSound(soundDataIdx);
    
    //Store the result.
    if(!destVarArg.empty()) {
        data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, soundId);
    }
}


/**
 * @brief Code for the text printing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::print(ScriptActionInstRunData& data) {
    //Get the arguments.
    string textArg = vectorTailToString(data.args, 0);
    
    //Main logic.
    string speaker =
        data.scriptVM->getRunnerMob() ?
        data.scriptVM->getRunnerMob()->type->name :
        "Area";
    string line =
        "(" + speaker + " said:) " + textArg;
        
    game.console.write(line, false);
}


/**
 * @brief Code for the status reception script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::receiveStatus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& statusArg = data.args[0];
    
    //Main logic.
    auto it = game.content.statusTypes.list.find(statusArg);
    if(it == game.content.statusTypes.list.end()) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown status effect \"" + statusArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->applyStatus(it->second, false, false);
}


/**
 * @brief Code for the release script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::release(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::releaseStoredMobs(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->releaseStoredMobs();
}


/**
 * @brief Code for the list item removal script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::removeListItem(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& numberArg = data.args[2];
    const string& delArg = data.args[3];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int idx = s2i(numberArg) - 1;
    if(!isIdxValid(idx, items)) idx = items.size() - 1;
    
    if(isIdxValid(idx, items)) {
        items.erase(items.begin() + idx);
    }
    string newListStr = join(items, delChar);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, newListStr);
}


/**
 * @brief Code for the status removal script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::removeStatus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& statusArg = data.args[0];
    
    //Main logic.
    auto it = game.content.statusTypes.list.find(statusArg);
    if(it == game.content.statusTypes.list.end()) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown status effect \"" + statusArg + "\"!"
        );
        return;
    }
    
    forIdx(s, data.scriptVM->getRunnerMob()->statuses) {
        if(data.scriptVM->getRunnerMob()->statuses[s].type == it->second) {
            data.scriptVM->getRunnerMob()->statuses[s].prevState =
                data.scriptVM->getRunnerMob()->statuses[s].state;
            data.scriptVM->getRunnerMob()->statuses[s].state =
                STATUS_STATE_TO_DELETE;
        }
    }
}


/**
 * @brief Code for the round number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::roundNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = round(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the run next action as script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::runNextActionAs(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& targetTypeArg = data.args[0];
    
    //Main logic.
    SCRIPT_ACTION_MOB_TARGET_TYPE targetType =
        ScriptActionUtils::getMobTargetType(data, targetTypeArg);
    Mob* target = ScriptActionUtils::getTargetMob(data, targetType);
    
    data.scriptVM->nextActionSurrogateMob = target;
}


/**
 * @brief Code for the save focused mob memory script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::saveFocusMemory(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& slotArg = data.args[0];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) {
        return;
    }
    
    string varName = "_focus_memory_" + slotArg;
    string varValue = i2s(data.scriptVM->getRunnerScriptVM()->focusedMob->id);
    data.scriptVM->getRunnerScriptVM()->vars.setValue(varName, varValue);
}


/**
 * @brief Code for the all mob script message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToAllMobs(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& msgArg = data.args[0];
    
    //Main logic.
    string msgStr = msgArg;
    forIdx(m, game.states.gameplay->mobs.all) {
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->getRunnerMob(),
            game.states.gameplay->mobs.all[m], msgStr
        );
    }
}


/**
 * @brief Code for the area script message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToArea(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& msgArg = data.args[0];
    
    //Main logic.
    string msgStr = msgArg;
    game.states.gameplay->sendScriptMessage(
        data.scriptVM->getRunnerMob(), &game.states.gameplay->scriptVM, msgStr
    );
}


/**
 * @brief Code for the focused mob script message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToFocus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& msgArg = data.args[0];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) return;
    string msgStr = msgArg;
    game.states.gameplay->sendScriptMessage(
        data.scriptVM->getRunnerMob(),
        data.scriptVM->getRunnerScriptVM()->focusedMob, msgStr
    );
}


/**
 * @brief Code for the linked mob script message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToLinks(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& msgArg = data.args[0];
    
    //Main logic.
    forIdx(l, data.scriptVM->getRunnerMob()->links) {
        if(
            data.scriptVM->getRunnerMob()->links[l] ==
            data.scriptVM->getRunnerMob()
        ) {
            continue;
        }
        if(!data.scriptVM->getRunnerMob()->links[l]) {
            continue;
        }
        
        string msgStr = msgArg;
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->getRunnerMob(),
            data.scriptVM->getRunnerMob()->links[l], msgStr
        );
    }
}


/**
 * @brief Code for the nearby mob script message sending script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::sendMessageToNearby(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& distArg = data.args[0];
    const string& msgArg = data.args[1];
    
    //Main logic.
    Distance d(s2f(distArg));
    
    forIdx(m2, game.states.gameplay->mobs.all) {
        if(
            game.states.gameplay->mobs.all[m2] ==
            data.scriptVM->getRunnerMob()
        ) {
            continue;
        }
        if(
            Distance(
                data.scriptVM->getRunnerMob()->center,
                game.states.gameplay->mobs.all[m2]->center
            ) > d
        ) {
            continue;
        }
        
        string msgStr = msgArg;
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->getRunnerMob(),
            game.states.gameplay->mobs.all[m2], msgStr
        );
    }
}


/**
 * @brief Code for the animation setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setAnimation(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& animArg = data.args[0];
    const string& optionArg = data.args[1];
    const string& mobSpeedArg = data.args[2];
    
    //Main logic.
    size_t animIdx =
        data.scriptVM->getRunnerMob()->type->animDb->findAnimation(animArg);
    if(animIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown animation \"" + animArg + "\"!"
        );
        return;
    }
    
    bool optionFound;
    START_ANIM_OPTION option =
        enumGetValue(startAnimOptionINames, optionArg, &optionFound);
    if(!optionFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown animation start option \"" + optionArg + "\"!"
        );
        return;
    }
    
    float mobSpeedBaseline = 0.0f;
    if(s2b(mobSpeedArg)) {
        mobSpeedBaseline = data.scriptVM->getRunnerMob()->type->moveSpeed;
    };
    
    data.scriptVM->getRunnerMob()->setAnimation(
        animIdx, option, false, mobSpeedBaseline
    );
}


/**
 * @brief Code for the block paths setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setCanBlockPaths(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->setCanBlockPaths(s2b(valueArg));
}


/**
 * @brief Code for the far reach setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFarReach(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& reachArg = data.args[0];
    
    //Main logic.
    size_t reachIdx = INVALID;
    forIdx(r, data.scriptVM->getRunnerMob()->type->reaches) {
        if(data.scriptVM->getRunnerMob()->type->reaches[r].name == reachArg) {
            reachIdx = r;
        }
    }
    
    if(reachIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown reach \"" + reachArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->farReach = reachIdx;
    data.scriptVM->getRunnerMob()->updateInteractionSpan();
}


/**
 * @brief Code for the flying setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFlying(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
        enableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_CAN_MOVE_MIDAIR
        );
    } else {
        disableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_CAN_MOVE_MIDAIR
        );
    }
}


/**
 * @brief Code for the focused mob var setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setFocusVar(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& varArg = data.args[0];
    const string& valueArg = data.args[1];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) return;
    data.scriptVM->getRunnerScriptVM()->focusedMob->scriptVM.vars.setValue(
        varArg, valueArg
    );
}


/**
 * @brief Code for the gravity setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setGravity(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& gravityArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->gravityMult = s2f(gravityArg);
}


/**
 * @brief Code for the health setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHealth(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& healthArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->setHealth(false, false, s2f(healthArg));
}


/**
 * @brief Code for the height setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHeight(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& heightArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->height = s2f(heightArg);
    
    if(data.scriptVM->getRunnerMob()->type->walkable) {
        //Update the Z of mobs standing on top of it.
        forIdx(m, game.states.gameplay->mobs.all) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m];
            if(m2Ptr->standingOnMob == data.scriptVM->getRunnerMob()) {
                m2Ptr->bottomZ =
                    data.scriptVM->getRunnerMob()->bottomZ +
                    data.scriptVM->getRunnerMob()->height;
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
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
        enableFlag(data.scriptVM->getRunnerMob()->flags, MOB_FLAG_HIDDEN);
    } else {
        disableFlag(data.scriptVM->getRunnerMob()->flags, MOB_FLAG_HIDDEN);
    }
}


/**
 * @brief Code for the holdable setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHoldable(ScriptActionInstRunData& data) {
    //Main logic.
    if(typeid(*(data.scriptVM->getRunnerMob())) != typeid(Tool)) {
        return;
    }
    
    unsigned char flags = 0;
    forIdx(a, data.args) {
        const string& ruleArg = data.args[a];
        bool flagFound;
        HOLDABILITY_FLAG flag =
            enumGetValue(holdabilityFlagINames, ruleArg, &flagFound);
            
        if(!flagFound) {
            ScriptActionUtils::reportActionError(
                data,
                "Unknown holdable rule \"" + ruleArg + "\"!"
            );
            return;
        }
        
        flags |= flag;
    }
    
    ((Tool*) (data.scriptVM->getRunnerMob()))->holdabilityFlags = flags;
}


/**
 * @brief Code for the huntable setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setHuntable(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
        disableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_NON_HUNTABLE
        );
    } else {
        enableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_NON_HUNTABLE
        );
    }
}


/**
 * @brief Code for the limb animation setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setLimbAnimation(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& animArg = data.args[0];
    
    //Main logic.
    if(!data.scriptVM->getRunnerMob()->parent) {
        return;
    }
    if(!data.scriptVM->getRunnerMob()->parent->limbAnim.animDb) {
        return;
    }
    
    size_t a =
        data.scriptVM->getRunnerMob()->parent->
        limbAnim.animDb->findAnimation(animArg);
    if(a == INVALID) {
        return;
    }
    
    data.scriptVM->getRunnerMob()->parent->limbAnim.curAnim =
        data.scriptVM->getRunnerMob()->parent->limbAnim.animDb->animations[a];
    data.scriptVM->getRunnerMob()->parent->limbAnim.toStart();
    
}


/**
 * @brief Code for the list item setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setListItem(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& listArg = data.args[1];
    const string& newItemArg = data.args[2];
    const string& numberArg = data.args[3];
    const string& delArg = data.args[4];
    
    //Main logic.
    bool delFound;
    SCRIPT_ACTION_LIST_DELIMITER delType =
        enumGetValue(scriptActionListDelimiterINames, delArg, &delFound);
        
    if(!delFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    vector<string> items = split(listArg, delChar, true);
    int idx = s2i(numberArg) - 1;
    if(!isIdxValid(idx, items)) idx = items.size() - 1;
    
    if(isIdxValid(idx, items)) {
        items[(size_t) idx] = newItemArg;
    }
    string newListStr = join(items, delChar);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, newListStr);
}


/**
 * @brief Code for the mission metric script slot setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setMissionMetricScriptSlot(
    ScriptActionInstRunData& data
) {
    //Get the arguments.
    const string& slotNumberArg = data.args[0];
    const string& valueArg = data.args[1];
    
    //Main logic.
    if(game.curArea->type != AREA_TYPE_MISSION) return;
    size_t slotNumber = s2i(slotNumberArg) - 1;
    int value = s2i(valueArg);
    game.states.gameplay->missionMetricScriptSlots[slotNumber] = value;
}


/**
 * @brief Code for the near reach setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setNearReach(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& reachArg = data.args[0];
    
    //Main logic.
    size_t reachIdx = INVALID;
    forIdx(r, data.scriptVM->getRunnerMob()->type->reaches) {
        if(data.scriptVM->getRunnerMob()->type->reaches[r].name == reachArg) {
            reachIdx = r;
        }
    }
    
    if(reachIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown reach \"" + reachArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->nearReach = reachIdx;
    data.scriptVM->getRunnerMob()->updateInteractionSpan();
}


/**
 * @brief Code for the radius setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setRadius(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& radiusArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->setRadius(s2f(radiusArg));
}


/**
 * @brief Code for the sector scroll setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setSectorScroll(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    
    //Main logic.
    Sector* sPtr =
        getSector(data.scriptVM->getRunnerMob()->center, nullptr, true);
    if(!sPtr) return;
    
    sPtr->scroll.x = s2f(xArg);
    sPtr->scroll.y = s2f(yArg);
}


/**
 * @brief Code for the shadow visibility setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setShadowVisibility(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
        disableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_SHADOW_INVISIBLE
        );
    } else {
        enableFlag(
            data.scriptVM->getRunnerMob()->flags, MOB_FLAG_SHADOW_INVISIBLE
        );
    }
}


/**
 * @brief Code for the state setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setState(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& stateArg = data.args[0];
    
    //Main logic.
    size_t stateIdx = INVALID;
    forIdx(s, data.scriptVM->getRunnerScriptVM()->scriptDef->fsm.states) {
        if(
            data.scriptVM->getRunnerScriptVM()->
            scriptDef->fsm.states[s]->name == stateArg
        ) {
            stateIdx = s;
            break;
        }
    }
    
    if(stateIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown state \"" + stateArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->scriptVM.fsm.setState(
        stateIdx, data.customData1, data.customData2
    );
}


/**
 * @brief Code for the tangible setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTangible(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
        disableFlag(data.scriptVM->getRunnerMob()->flags, MOB_FLAG_INTANGIBLE);
    } else {
        enableFlag(data.scriptVM->getRunnerMob()->flags, MOB_FLAG_INTANGIBLE);
    }
}


/**
 * @brief Code for the team setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTeam(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& teamArg = data.args[0];
    
    //Main logic.
    bool teamFound;
    MOB_TEAM team = enumGetValue(mobTeamINames, teamArg, &teamFound);
    if(!teamFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown team \"" + teamArg + "\"!"
        );
        return;
    }
    data.scriptVM->getRunnerMob()->setTeam(team);
}


/**
 * @brief Code for the timer setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setTimer(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& durationArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerScriptVM()->setTimer(s2f(durationArg));
}


/**
 * @brief Code for the var setting script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::setVar(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& varArg = data.args[0];
    const string& valueArg = data.args[1];
    
    //Main logic.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(varArg, valueArg);
}


/**
 * @brief Code for the shake camera script action.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::shakeCamera(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& amountArg = data.args[0];
    
    //Main logic.
    forIdx(p, game.states.gameplay->players) {
        Player& player = game.states.gameplay->players[p];
        float d =
            Distance(
                data.scriptVM->getRunnerMob()->center, player.view.cam.center
            ).toFloat();
        float strengthMult =
            ::interpolateNumber(
                d, 0.0f, DRAWING::CAM_SHAKE_DROPOFF_DIST, 1.0f, 0.0f
            );
        player.view.shaker.shake(s2f(amountArg) / 100.0f * strengthMult);
    }
}


/**
 * @brief Code for the show cutscene message script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::showCutsceneMessage(ScriptActionInstRunData& data) {
    //Get the arguments.
    string textArg = vectorTailToString(data.args, 0);
    
    //Main logic.
    startCutsceneMessage(textArg, nullptr);
}


/**
 * @brief Code for the show cutscene message from var script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::showMessageFromVar(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& varArg = data.args[0];
    
    //Main logic.
    string text;
    data.scriptVM->getRunnerScriptVM()->vars.getValue(varArg, text);
    startCutsceneMessage(text, nullptr);
}


/**
 * @brief Code for the sign number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::signNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = sign(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the spawning script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::spawn(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& spawnArg = data.args[0];
    
    //Main logic.
    size_t spawnIdx = INVALID;
    forIdx(s, data.scriptVM->getRunnerMob()->type->spawns) {
        if(data.scriptVM->getRunnerMob()->type->spawns[s].name == spawnArg) {
            spawnIdx = s;
            break;
        }
    }
    
    if(spawnIdx == INVALID) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown spawn info block \"" + spawnArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->getRunnerMob()->spawn(
        &data.scriptVM->getRunnerMob()->type->spawns[spawnIdx]
    );
}


/**
 * @brief Code for the square root number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::squareRootNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    float result = (float) sqrt(s2f(numberArg));
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the z stabilization script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stabilizeZ(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& typeArg = data.args[0];
    const string& offsetArg = data.args[1];
    
    //Main logic.
    if(
        data.scriptVM->getRunnerMob()->links.empty() ||
        !data.scriptVM->getRunnerMob()->links[0]
    ) {
        return;
    }
    
    float bestMatchZ = data.scriptVM->getRunnerMob()->links[0]->bottomZ;
    bool typeFound;
    SCRIPT_ACTION_STABILIZE_Z_TYPE type =
        enumGetValue(scriptActionStabilizeZTypeINames, typeArg, &typeFound);
        
    if(!typeFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown stabilization reference \"" + typeArg + "\"!"
        );
        return;
    }
    
    for(size_t l = 1; l < data.scriptVM->getRunnerMob()->links.size(); l++) {
        if(!data.scriptVM->getRunnerMob()->links[l]) continue;
        
        switch(type) {
        case SCRIPT_ACTION_STABILIZE_Z_TYPE_HIGHEST: {
            if(data.scriptVM->getRunnerMob()->links[l]->bottomZ > bestMatchZ) {
                bestMatchZ = data.scriptVM->getRunnerMob()->links[l]->bottomZ;
            }
            break;
            
        } case SCRIPT_ACTION_STABILIZE_Z_TYPE_LOWEST: {
            if(data.scriptVM->getRunnerMob()->links[l]->bottomZ < bestMatchZ) {
                bestMatchZ = data.scriptVM->getRunnerMob()->links[l]->bottomZ;
            }
            break;
            
        }
        }
        
    }
    
    data.scriptVM->getRunnerMob()->bottomZ = bestMatchZ + s2f(offsetArg);
}


/**
 * @brief Code for the chomping start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startChomping(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& maxArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->chompMax = s2i(maxArg);
    data.scriptVM->getRunnerMob()->chompBodyParts.clear();
    
    for(size_t a = 1; a < data.args.size(); a++) {
        const string& partArg = data.args[a];
        size_t partIdx =
            data.scriptVM->getRunnerMob()->type->animDb->findBodyPart(partArg);
            
        if(partIdx == INVALID) {
            ScriptActionUtils::reportActionError(
                data,
                "Unknown body part \"" + partArg + "\"!"
            );
            return;
        }
        
        data.scriptVM->getRunnerMob()->chompBodyParts.push_back(partIdx);
    }
}


/**
 * @brief Code for the dying start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startDying(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->startDying();
}


/**
 * @brief Code for the height effect start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startHeightEffect(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->startHeightEffect();
}


/**
 * @brief Code for the particle start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startParticles(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& genArg = data.args[0];
    const string& xArg = data.args[1];
    const string& yArg = data.args[2];
    const string& zArg = data.args[3];
    
    //Main logic.
    float offsetX = s2f(xArg);
    float offsetY = s2f(yArg);
    float offsetZ = s2f(zArg);
    
    if(!isInMap(game.content.particleGens.list, genArg)) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown particle generator \"" + genArg + "\"!"
        );
        return;
    }
    
    ParticleGenerator pg =
        standardParticleGenSetup(genArg, data.scriptVM->getRunnerMob());
    pg.followPosOffset = Point(offsetX, offsetY);
    pg.followZOffset = offsetZ;
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    data.scriptVM->getRunnerMob()->particleGenerators.push_back(pg);
}


/**
 * @brief Code for the stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stop(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->stopChasing();
    data.scriptVM->getRunnerMob()->stopTurning();
    data.scriptVM->getRunnerMob()->stopFollowingPath();
}


/**
 * @brief Code for the chomp stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopChomping(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->chompMax = 0;
    data.scriptVM->getRunnerMob()->chompBodyParts.clear();
}


/**
 * @brief Code for the height effect stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopHeightEffect(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopParticles(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->deleteParticleGenerator(
        MOB_PARTICLE_GENERATOR_ID_SCRIPT
    );
}


/**
 * @brief Code for the sound stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopSound(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& idArg = data.args[0];
    
    //Main logic.
    game.audio.destroySoundSource(s2i(idArg));
}


/**
 * @brief Code for the vertical stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopVertically(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->speedZ = 0;
}


/**
 * @brief Code for the focus storing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::storeFocusInside(ScriptActionInstRunData& data) {
    //Main logic.
    if(
        data.scriptVM->getRunnerScriptVM()->focusedMob &&
        !data.scriptVM->getRunnerScriptVM()->focusedMob->
        isMobOrParentStoredInside()
    ) {
        data.scriptVM->getRunnerMob()->storeMobInside(
            data.scriptVM->getRunnerScriptVM()->focusedMob
        );
    }
}


/**
 * @brief Code for the swallow script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::swallow(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& amountArg = data.args[0];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->swallowChompedPikmin(s2i(amountArg));
}


/**
 * @brief Code for the swallow all script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::swallowAll(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerMob()->swallowChompedPikmin(
        data.scriptVM->getRunnerMob()->chompingMobs.size()
    );
}


/**
 * @brief Code for the teleport to absolute coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::teleportToAbsolute(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->stopChasing();
    data.scriptVM->getRunnerMob()->chase(
        Point(s2f(xArg), s2f(yArg)), s2f(zArg), CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the teleport to relative coordinates script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::teleportToRelative(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    data.scriptVM->getRunnerMob()->stopChasing();
    Point p =
        rotatePoint(
            Point(s2f(xArg), s2f(yArg)),
            data.scriptVM->getRunnerMob()->angle
        );
    data.scriptVM->getRunnerMob()->chase(
        data.scriptVM->getRunnerMob()->center + p,
        data.scriptVM->getRunnerMob()->bottomZ + s2f(zArg),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the throw focused mob script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::throwFocus(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    const string& maxHeightArg = data.args[3];
    
    //Main logic.
    if(!data.scriptVM->getRunnerScriptVM()->focusedMob) {
        return;
    }
    
    if(
        data.scriptVM->getRunnerScriptVM()->focusedMob->holder.m ==
        data.scriptVM->getRunnerScriptVM()->getRunnerMob()
    ) {
        data.scriptVM->getRunnerMob()->release(
            data.scriptVM->getRunnerScriptVM()->focusedMob
        );
    }
    
    float maxHeight = s2f(maxHeightArg);
    if(maxHeight == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.scriptVM->getRunnerMob()->startHeightEffect();
    calculateThrow(
        data.scriptVM->getRunnerScriptVM()->focusedMob->center,
        data.scriptVM->getRunnerScriptVM()->focusedMob->bottomZ,
        Point(s2f(xArg), s2f(yArg)), s2f(zArg),
        maxHeight, MOB::GRAVITY_ADDER,
        &data.scriptVM->getRunnerScriptVM()->focusedMob->speed,
        &data.scriptVM->getRunnerScriptVM()->focusedMob->speedZ,
        nullptr
    );
}


/**
 * @brief Code for the truncate number script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::truncateNumber(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    
    //Main logic.
    int result = s2i(numberArg);
    
    //Store the result.
    data.scriptVM->getRunnerScriptVM()->vars.setValue(destVarArg, result);
}


/**
 * @brief Code for the turn to an absolute angle script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::turnToAbsolute(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& angleOrXArg = data.args[0];
    const string& yArg = data.args[1];
    
    //Main logic.
    if(yArg.empty()) {
        //Turn to an absolute angle.
        data.scriptVM->getRunnerMob()->face(
            degToRad(s2f(angleOrXArg)), nullptr
        );
    } else {
        //Turn to some absolute coordinates.
        float x = s2f(angleOrXArg);
        float y = s2f(yArg);
        data.scriptVM->getRunnerMob()->face(
            getAngle(data.scriptVM->getRunnerMob()->center, Point(x, y)),
            nullptr
        );
    }
}


/**
 * @brief Code for the turn to a relative angle script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::turnToRelative(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& angleOrXArg = data.args[0];
    const string& yArg = data.args[1];
    
    //Main logic.
    if(yArg.empty()) {
        //Turn to a relative angle.
        data.scriptVM->getRunnerMob()->face(
            data.scriptVM->getRunnerMob()->angle + degToRad(s2f(angleOrXArg)),
            nullptr
        );
    } else {
        //Turn to some relative coordinates.
        float x = s2f(angleOrXArg);
        float y = s2f(yArg);
        Point p =
            rotatePoint(Point(x, y), data.scriptVM->getRunnerMob()->angle);
        data.scriptVM->getRunnerMob()->face(
            getAngle(
                data.scriptVM->getRunnerMob()->center,
                data.scriptVM->getRunnerMob()->center + p
            ),
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
    //Get the arguments.
    const string& targetArg = data.args[0];
    
    //Main logic.
    bool targetFound;
    SCRIPT_ACTION_TURN_TYPE target =
        enumGetValue(scriptActionTurnTypeINames, targetArg, &targetFound);
    if(!targetFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown target type \"" + targetArg + "\"!"
        );
        return;
    }
    
    switch(target) {
    case SCRIPT_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC: {
        data.scriptVM->getRunnerMob()->arachnorbHeadTurnLogic();
        break;
        
    } case SCRIPT_ACTION_TURN_TYPE_FOCUSED_MOB: {
        if(data.scriptVM->getRunnerScriptVM()->focusedMob) {
            data.scriptVM->getRunnerMob()->face(
                0, &data.scriptVM->getRunnerScriptVM()->focusedMob->center
            );
        }
        break;
        
    } case SCRIPT_ACTION_TURN_TYPE_HOME: {
        data.scriptVM->getRunnerMob()->face(
            getAngle(data.scriptVM->getRunnerMob()->center,
                     data.scriptVM->getRunnerMob()->home),
            nullptr
        );
        break;
        
    }
    }
}


/**
 * @brief Code for the unfocus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::unfocus(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->getRunnerScriptVM()->unfocusFromMob();
}


/**
 * @brief Code for the while-do loop condition script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::whileDo(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& lhsArg = data.args[0];
    const string& opArg = data.args[1];
    string rhsArg = vectorTailToString(data.args, 2);
    
    //Main logic.
    bool opFound;
    SCRIPT_ACTION_IF_OP op =
        enumGetValue(scriptActionIfOpINames, opArg, &opFound);
    if(!opFound) {
        ScriptActionUtils::reportActionError(
            data,
            "Unknown operator \"" + opArg + "\"!"
        );
        return;
    }
    
    bool result = ScriptActionUtils::doScriptCondition(lhsArg, op, rhsArg);
    
    //Store the result.
    data.returnValue = result;
}


#pragma endregion
#pragma region Action util functions


/**
 * @brief Returns whether a script condition is true or not.
 *
 * @param lhs The left-hand side comparand.
 * @param op The operator.
 * @param rhs The right-hand side comparand.
 * @return Whether it is true.
 */
bool ScriptActionUtils::doScriptCondition(
    const string& lhs, SCRIPT_ACTION_IF_OP op, const string& rhs
) {
    switch(op) {
    case SCRIPT_ACTION_IF_OP_EQUAL: {
        if(isNumber(lhs) && isNumber(rhs)) {
            return s2f(lhs) == s2f(rhs);
        } else {
            return lhs == rhs;
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_NOT: {
        if(isNumber(lhs) && isNumber(rhs)) {
            return s2f(lhs) != s2f(rhs);
        } else {
            return lhs != rhs;
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS: {
        return s2f(lhs) < s2f(rhs);
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE: {
        return s2f(lhs) > s2f(rhs);
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS_E: {
        return s2f(lhs) <= s2f(rhs);
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE_E: {
        return s2f(lhs) >= s2f(rhs);
        break;
        
    }
    }
    
    return false;
}


/**
 * @brief Returns a mob script action mob target type from an action call.
 *
 * @param data Data about the action call.
 * @param name The type's name.
 * @return The type.
 */
SCRIPT_ACTION_MOB_TARGET_TYPE ScriptActionUtils::getMobTargetType(
    const ScriptActionInstRunData& data, const string& name
) {
    bool found;
    SCRIPT_ACTION_MOB_TARGET_TYPE type =
        enumGetValue(scriptActionMobTargetTypeINames, name, &found);
        
    if(!found) {
        reportActionError(
            data,
            "Unknown mob target type \"" + name + "\"!"
        );
    }
    
    return type;
}


/**
 * @brief Returns the mob matching the mob target type.
 *
 * @param data Data about the action call.
 * @param type Type of target.
 */
Mob* ScriptActionUtils::getTargetMob(
    ScriptActionInstRunData& data, SCRIPT_ACTION_MOB_TARGET_TYPE type
) {
    switch (type) {
    case SCRIPT_ACTION_MOB_TARGET_TYPE_SELF: {
        return data.scriptVM->getRunnerMob();
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_FOCUS: {
        return data.scriptVM->getRunnerScriptVM()->focusedMob;
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_TRIGGER: {
        return getTriggerMob(data);
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_LINK: {
        if(
            data.scriptVM->getRunnerMob() &&
            !data.scriptVM->getRunnerMob()->links.empty() &&
            data.scriptVM->getRunnerMob()->links[0]
        ) {
            return data.scriptVM->getRunnerMob()->links[0];
        }
        break;
    } case SCRIPT_ACTION_MOB_TARGET_TYPE_PARENT: {
        if(
            data.scriptVM->getRunnerMob() &&
            data.scriptVM->getRunnerMob()->parent
        ) {
            return data.scriptVM->getRunnerMob()->parent->m;
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
Mob* ScriptActionUtils::getTriggerMob(ScriptActionInstRunData& data) {
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
        data.actionDef->parentEvent == FSM_EV_RECEIVE_SCRIPT_MESSAGE
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


/**
 * @brief Reports an error with an action.
 *
 * @param data Data about the action call.
 * @param info What information to report.
 */
void ScriptActionUtils::reportActionError(
    const ScriptActionInstRunData& data, const string& info
) {
    string filePath;
    if(data.actionDef->dataFileLine != 0) {
        if(data.scriptVM->getRunnerMob()) {
            filePath =
                data.scriptVM->getRunnerMob()->type->manifest->path + "/" +
                FILE_NAMES::MOB_TYPE_SCRIPT;
        } else {
            filePath =
                game.curArea->manifest->path + "/" +
                FILE_NAMES::AREA_SCRIPT;
        }
    }
    
    if(filePath.empty()) {
        game.errors.report(info);
    } else {
        game.errors.report(info, filePath, data.actionDef->dataFileLine);
    }
}


#pragma endregion
