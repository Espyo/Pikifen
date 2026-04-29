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
#pragma region Action runners


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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    data.scriptVM->mob->setHealth(true, false, s2f(healthArg));
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = newListStr;
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
    data.scriptVM->vars[destVarArg] = result;
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
        reportActionError(
            data,
            "Unknown arachnorb plan logic type \"" + typeArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->arachnorbPlanLogic(type);
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    if(data.scriptVM->vars.contains(varArg)) {
        data.scriptVM->vars[varArg].clear();
    }
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
    data.scriptVM->vars[destVarArg] = f2s(result);
}


/**
 * @brief Code for the deletion script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::deleteFunction(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->toDelete = true;
}


/**
 * @brief Code for the liquid draining script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::drainLiquid(ScriptActionInstRunData& data) {
    //Main logic.
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
    //Get the arguments.
    const string& destVarArg = data.args[0];
    const string& numberArg = data.args[1];
    const string& methodArg = data.args[2];
    
    //Main logic.
    bool methodFound;
    EASE_METHOD method =
        enumGetValue(easeMethodINames, methodArg, &methodFound);
        
    if(!methodFound) {
        reportActionError(
            data,
            "Unknown easing method \"" + methodArg + "\"!"
        );
        return;
    }
    
    float result = ease(s2f(numberArg), method);
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = f2s(result);
}


/**
 * @brief Code for the death finish script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::finishDying(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->finishDying();
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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
        getMobTargetType(data, targetTypeArg);
    Mob* target = getTargetMob(data, targetType);
    
    if(!target) return;
    data.scriptVM->focusOnMob(target);
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
        getMobTargetType(data, targetTypeArg);
    Mob* target = getTargetMob(data, targetType);
    
    if(!target) return;
    if(target->health <= 0.0f) return;
    
    data.scriptVM->mob->leaveGroup();
    bool silent = s2b(silentArg);
    
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
    settings.label = labelArg;
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
    data.scriptVM->vars[destVarArg] = f2s(angle);
}


/**
 * @brief Code for the angle closest difference obtaining script action type.
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
    data.scriptVM->vars[destVarArg] = f2s(diff);
}


/**
 * @brief Code for the angle smallest difference obtaining script action type.
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
    data.scriptVM->vars[destVarArg] = f2s(diff);
}



/**
 * @brief Code for the area info obtaining script action type.
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
        reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch (type) {
    case SCRIPT_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES: {
        result = i2s(game.states.gameplay->dayMinutes);
        break;
        
    } case SCRIPT_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN: {
        result = i2s(game.states.gameplay->mobs.pikmin.size());
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = result;
}


/**
 * @brief Code for the getting chomped script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::getChomped(ScriptActionInstRunData& data) {
    //Main logic.
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
    data.scriptVM->vars[xDestVarArg] = f2s(p.x);
    data.scriptVM->vars[yDestVarArg] = f2s(p.y);
}


/**
 * @brief Code for the distance obtaining script action type.
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
    data.scriptVM->vars[destVarArg] = f2s(dist);
}


/**
 * @brief Code for the event info obtaining script action type.
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
        reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch (type) {
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
            if(data.scriptVM->mob) {
                result =
                    data.scriptVM->mob->getClosestHitbox(
                        ((Mob*)(data.customData1))->pos
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
            if(data.customData1 && data.scriptVM->mob) {
                result =
                    ((Mob*)(data.customData1))->getClosestHitbox(
                        data.scriptVM->mob->pos
                    )->bodyPartName;
            }
        }
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = result;
}


/**
 * @brief Code for the floor Z obtaining script action type.
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
    float result = s ? s->z : 0.0f;
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    if(!data.scriptVM->focusedMob) return;
    string result = data.scriptVM->focusedMob->scriptVM.vars[focusVarArg];
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = result;
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = item;
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = i2s(number);
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
        reportActionError(
            data,
            "Unknown list delimiter \"" + delArg + "\"!"
        );
        return;
    }
    
    string delChar = enumGetName(scriptActionListDelimiterChars, delType);
    size_t size = getSplitCount(listArg, delChar, true);
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = i2s(size);
}


/**
 * @brief Code for the misc info obtaining script action type.
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
        reportActionError(
            data,
            "Unknown info type \"" + typeArg + "\"! "
            "Did you mean to use a different \"get_*_info\" action?"
        );
        return;
    }
    
    switch (type) {
    case SCRIPT_ACTION_GET_MISC_INFO_TYPE_DELTA_T: {
        result = f2s(game.deltaT);
        break;
        
    }
    }
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = result;
}


/**
 * @brief Code for the mob info obtaining script action type.
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
        getMobTargetType(data, targetArg);
    Mob* target = getTargetMob(data, targetType);
    
    if(!target) return;
    
    string result;
    bool typeFound;
    SCRIPT_ACTION_GET_MOB_INFO_TYPE type =
        enumGetValue(scriptActionGetMobInfoTypeINames, typeArg, &typeFound);
    if(!typeFound) {
        reportActionError(
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
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN: {
        result = i2s(target->chompingMobs.size());
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->scriptVM.focusedMob) {
            float d =
                Distance(target->pos, target->scriptVM.focusedMob->pos).
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
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY: {
        result = target->type->category->internalName;
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
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_WEIGHT: {
        if(target->type->category->id == MOB_CATEGORY_SCALES) {
            Scale* sPtr = (Scale*)(target);
            result = i2s(sPtr->calculateCurWeight());
        }
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_X: {
        result = f2s(target->pos.x);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Y: {
        result = f2s(target->pos.y);
        break;
        
    } case SCRIPT_ACTION_GET_MOB_INFO_TYPE_Z: {
        result = f2s(target->z);
        break;
    }
    }
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = result;
}


/**
 * @brief Returns a mob script action mob target type from an action call.
 *
 * @param name The type's name.
 * @return The type.
 */
SCRIPT_ACTION_MOB_TARGET_TYPE ScriptActionRunners::getMobTargetType(
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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    data.scriptVM->vars[destVarArg] = i2s(result);
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
    bool exists = data.scriptVM->vars.contains(varArg);
    if(exists) {
        exists = !data.scriptVM->vars[varArg].empty();
    }
    
    //Store the result.
    data.scriptVM->vars[destVarArg] = b2s(exists);
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
    
    //Main logic.
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    size_t partIdx = data.scriptVM->mob->anim.animDb->findBodyPart(bodyPartArg);
    if(partIdx == INVALID) {
        reportActionError(
            data,
            "Unknown body part \"" + bodyPartArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->hold(
        data.scriptVM->focusedMob, HOLD_TYPE_PURPOSE_GENERAL,
        partIdx, 0.0f, 0.0f, 0.5f, s2b(aboveArg),
        HOLD_ROTATION_METHOD_COPY_HOLDER
    );
}


/**
 * @brief Code for the "if" script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::ifFunction(ScriptActionInstRunData& data) {
    //Get the arguments.
    const string& lhsArg = data.args[0];
    const string& opArg = data.args[1];
    string rhsArg = vectorTailToString(data.args, 2);
    
    //Main logic.
    bool result = false;
    bool opFound;
    SCRIPT_ACTION_IF_OP op =
        enumGetValue(scriptActionIfOpINames, opArg, &opFound);
    if(!opFound) {
        reportActionError(
            data,
            "Unknown operator \"" + opArg + "\"!"
        );
        return;
    }
    
    switch(op) {
    case SCRIPT_ACTION_IF_OP_EQUAL: {
        if(isNumber(lhsArg) && isNumber(rhsArg)) {
            result = (s2f(lhsArg) == s2f(rhsArg));
        } else {
            result = (lhsArg == rhsArg);
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_NOT: {
        if(isNumber(lhsArg) && isNumber(rhsArg)) {
            result = (s2f(lhsArg) != s2f(rhsArg));
        } else {
            result = (lhsArg != rhsArg);
        }
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS: {
        result = (s2f(lhsArg) < s2f(rhsArg));
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE: {
        result = (s2f(lhsArg) > s2f(rhsArg));
        break;
        
    } case SCRIPT_ACTION_IF_OP_LESS_E: {
        result = (s2f(lhsArg) <= s2f(rhsArg));
        break;
        
    } case SCRIPT_ACTION_IF_OP_MORE_E: {
        result = (s2f(lhsArg) >= s2f(rhsArg));
        break;
        
    }
    }
    
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
    data.scriptVM->vars[destVarArg] = f2s(result);
}


/**
 * @brief Code for the link with focus script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::linkWithFocus(ScriptActionInstRunData& data) {
    //Main logic.
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
    //Get the arguments.
    const string& slotArg = data.args[0];
    
    //Main logic.
    if(data.scriptVM->mob->focusedMobMemory.empty()) {
        return;
    }
    
    data.scriptVM->focusOnMob(
        data.scriptVM->mob->focusedMobMemory[s2i(slotArg)]
    );
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
    float z = zArg.empty() ? data.scriptVM->mob->z : s2f(zArg);
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
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    float x = s2f(xArg);
    float y = s2f(yArg);
    float z = zArg.empty() ? 0.0f : s2f(zArg);
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
    //Get the arguments.
    const string& targetArg = data.args[0];
    
    //Main logic.
    bool targetFound;
    SCRIPT_ACTION_MOVE_TYPE targetType =
        enumGetValue(scriptActionMoveTypeINames, targetArg, &targetFound);
    if(!targetFound) {
        reportActionError(
            data,
            "Unknown target type \"" + targetArg + "\"!"
        );
        return;
    }
    
    switch(targetType) {
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
    //Main logic.
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
    //Get the arguments.
    const string& soundArg = data.args[0];
    const string& destVarArg = data.args[1];
    
    //Main logic.
    size_t soundDataIdx = INVALID;
    forIdx(s, data.scriptVM->mob->type->sounds) {
        if(data.scriptVM->mob->type->sounds[s].name == soundArg) {
            soundDataIdx = s;
            break;
        }
    }
    
    if(soundDataIdx == INVALID) {
        reportActionError(
            data,
            "Unknown sound info block \"" + soundArg + "\"!"
        );
        return;
    }
    
    size_t soundId = data.scriptVM->mob->playSound(soundDataIdx);
    
    //Store the result.
    if(!destVarArg.empty()) data.scriptVM->vars[destVarArg] = i2s(soundId);
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
    size_t seconds = floor(game.states.gameplay->gameplayTimePassed);
    size_t centiseconds =
        (game.states.gameplay->gameplayTimePassed - seconds) * 100;
    string timestamp =
        resizeString(i2s(seconds), 4, true, true, true, ' ') + "." +
        resizeString(i2s(centiseconds), 2, true, true, true, '0');
        
    string speaker =
        data.scriptVM->mob ?
        data.scriptVM->mob->type->name :
        "Area";
    game.states.gameplay->printActionLogLines.push_back(
        "[@" + timestamp + "s " + speaker + " said:] " + textArg
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
    //Get the arguments.
    const string& statusArg = data.args[0];
    
    //Main logic.
    auto it = game.content.statusTypes.list.find(statusArg);
    if(it == game.content.statusTypes.list.end()) {
        reportActionError(
            data,
            "Unknown status effect \"" + statusArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->applyStatus(it->second, false, false);
}


/**
 * @brief Code for the release script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::release(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::releaseStoredMobs(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->releaseStoredMobs();
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = newListStr;
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
        reportActionError(
            data,
            "Unknown status effect \"" + statusArg + "\"!"
        );
        return;
    }
    
    forIdx(s, data.scriptVM->mob->statuses) {
        if(data.scriptVM->mob->statuses[s].type == it->second) {
            data.scriptVM->mob->statuses[s].prevState =
                data.scriptVM->mob->statuses[s].state;
            data.scriptVM->mob->statuses[s].state = STATUS_STATE_TO_DELETE;
        }
    }
}


/**
 * @brief Reports an error with an action.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::reportActionError(
    const ScriptActionInstRunData& data, const string& info
) {
    string filePath;
    if(data.actionDef->dataFileLine != 0) {
        if(data.scriptVM->mob) {
            filePath =
                data.scriptVM->mob->type->manifest->path + "/" +
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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    data.scriptVM->mob->focusedMobMemory[s2i(slotArg)] =
        data.scriptVM->focusedMob;
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
    if(!data.scriptVM->focusedMob) return;
    string msgStr = msgArg;
    game.states.gameplay->sendScriptMessage(
        data.scriptVM->mob, data.scriptVM->focusedMob, msgStr
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
    forIdx(l, data.scriptVM->mob->links) {
        if(data.scriptVM->mob->links[l] == data.scriptVM->mob) continue;
        if(!data.scriptVM->mob->links[l]) continue;
        string msgStr = msgArg;
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->mob, data.scriptVM->mob->links[l], msgStr
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
        
        string msgStr = msgArg;
        game.states.gameplay->sendScriptMessage(
            data.scriptVM->mob, game.states.gameplay->mobs.all[m2], msgStr
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
    size_t animIdx = data.scriptVM->mob->type->animDb->findAnimation(animArg);
    if(animIdx == INVALID) {
        reportActionError(
            data,
            "Unknown animation \"" + animArg + "\"!"
        );
        return;
    }
    
    bool optionFound;
    START_ANIM_OPTION option =
        enumGetValue(startAnimOptionINames, optionArg, &optionFound);
    if(!optionFound) {
        reportActionError(
            data,
            "Unknown animation start option \"" + optionArg + "\"!"
        );
        return;
    }
    
    float mobSpeedBaseline = 0.0f;
    if(s2b(mobSpeedArg)) {
        mobSpeedBaseline = data.scriptVM->mob->type->moveSpeed;
    };
    
    data.scriptVM->mob->setAnimation(
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
    data.scriptVM->mob->setCanBlockPaths(s2b(valueArg));
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
    forIdx(r, data.scriptVM->mob->type->reaches) {
        if(data.scriptVM->mob->type->reaches[r].name == reachArg) {
            reachIdx = r;
        }
    }
    
    if(reachIdx == INVALID) {
        reportActionError(
            data,
            "Unknown reach \"" + reachArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->farReach = reachIdx;
    data.scriptVM->mob->updateInteractionSpan();
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
    //Get the arguments.
    const string& varArg = data.args[0];
    const string& valueArg = data.args[1];
    
    //Main logic.
    if(!data.scriptVM->focusedMob) return;
    data.scriptVM->focusedMob->scriptVM.vars[varArg] = valueArg;
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
    data.scriptVM->mob->gravityMult = s2f(gravityArg);
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
    data.scriptVM->mob->setHealth(false, false, s2f(healthArg));
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
    data.scriptVM->mob->height = s2f(heightArg);
    
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
    //Get the arguments.
    const string& valueArg = data.args[0];
    
    //Main logic.
    if(s2b(valueArg)) {
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
    //Main logic.
    if(typeid(*(data.scriptVM->mob)) != typeid(Tool)) {
        return;
    }
    
    unsigned char flags = 0;
    forIdx(a, data.args) {
        const string& ruleArg = data.args[a];
        bool flagFound;
        HOLDABILITY_FLAG flag =
            enumGetValue(holdabilityFlagINames, ruleArg, &flagFound);
            
        if(!flagFound) {
            reportActionError(
                data,
                "Unknown holdable rule \"" + ruleArg + "\"!"
            );
            return;
        }
        
        flags |= flag;
    }
    
    ((Tool*) (data.scriptVM->mob))->holdabilityFlags = flags;
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
    //Get the arguments.
    const string& animArg = data.args[0];
    
    //Main logic.
    if(!data.scriptVM->mob->parent) {
        return;
    }
    if(!data.scriptVM->mob->parent->limbAnim.animDb) {
        return;
    }
    
    size_t a =
        data.scriptVM->mob->parent->limbAnim.animDb->findAnimation(animArg);
    if(a == INVALID) {
        return;
    }
    
    data.scriptVM->mob->parent->limbAnim.curAnim =
        data.scriptVM->mob->parent->limbAnim.animDb->animations[a];
    data.scriptVM->mob->parent->limbAnim.toStart();
    
}


/**
 * @brief Code for the list item retrieval script action type.
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
        reportActionError(
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
    data.scriptVM->vars[destVarArg] = newListStr;
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
    forIdx(r, data.scriptVM->mob->type->reaches) {
        if(data.scriptVM->mob->type->reaches[r].name == reachArg) {
            reachIdx = r;
        }
    }
    
    if(reachIdx == INVALID) {
        reportActionError(
            data,
            "Unknown reach \"" + reachArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->nearReach = reachIdx;
    data.scriptVM->mob->updateInteractionSpan();
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
    data.scriptVM->mob->setRadius(s2f(radiusArg));
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
    Sector* sPtr = getSector(data.scriptVM->mob->pos, nullptr, true);
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
    //Get the arguments.
    const string& stateArg = data.args[0];
    
    //Main logic.
    size_t stateIdx = INVALID;
    forIdx(s, data.scriptVM->scriptDef->fsm.states) {
        if(data.scriptVM->scriptDef->fsm.states[s]->name == stateArg) {
            stateIdx = s;
            break;
        }
    }
    
    if(stateIdx == INVALID) {
        reportActionError(
            data,
            "Unknown state \"" + stateArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->fsm.setState(
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
    //Get the arguments.
    const string& teamArg = data.args[0];
    
    //Main logic.
    bool teamFound;
    MOB_TEAM team = enumGetValue(mobTeamINames, teamArg, &teamFound);
    if(!teamFound) {
        reportActionError(
            data,
            "Unknown team \"" + teamArg + "\"!"
        );
        return;
    }
    data.scriptVM->mob->setTeam(team);
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
    data.scriptVM->setTimer(s2f(durationArg));
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
    data.scriptVM->setVar(varArg, valueArg);
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
        Player* pPtr = &game.states.gameplay->players[p];
        float d =
            Distance(data.scriptVM->mob->pos, pPtr->view.cam.pos).toFloat();
        float strengthMult =
            ::interpolateNumber(
                d, 0.0f, DRAWING::CAM_SHAKE_DROPOFF_DIST, 1.0f, 0.0f
            );
        pPtr->view.shaker.shake(s2f(amountArg) / 100.0f * strengthMult);
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
    startCutsceneMessage(data.scriptVM->vars[varArg], nullptr);
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
    forIdx(s, data.scriptVM->mob->type->spawns) {
        if(data.scriptVM->mob->type->spawns[s].name == spawnArg) {
            spawnIdx = s;
            break;
        }
    }
    
    if(spawnIdx == INVALID) {
        reportActionError(
            data,
            "Unknown spawn info block \"" + spawnArg + "\"!"
        );
        return;
    }
    
    data.scriptVM->mob->spawn(
        &data.scriptVM->mob->type->spawns[spawnIdx]
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
    data.scriptVM->vars[destVarArg] = f2s(result);
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
    if(data.scriptVM->mob->links.empty() || !data.scriptVM->mob->links[0]) {
        return;
    }
    
    float bestMatchZ = data.scriptVM->mob->links[0]->z;
    bool typeFound;
    SCRIPT_ACTION_STABILIZE_Z_TYPE type =
        enumGetValue(scriptActionStabilizeZTypeINames, typeArg, &typeFound);
        
    if(!typeFound) {
        reportActionError(
            data,
            "Unknown stabilization reference \"" + typeArg + "\"!"
        );
        return;
    }
    
    for(size_t l = 1; l < data.scriptVM->mob->links.size(); l++) {
        if(!data.scriptVM->mob->links[l]) continue;
        
        switch(type) {
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
    
    data.scriptVM->mob->z = bestMatchZ + s2f(offsetArg);
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
    data.scriptVM->mob->chompMax = s2i(maxArg);
    data.scriptVM->mob->chompBodyParts.clear();
    
    for(size_t a = 1; a < data.args.size(); a++) {
        const string& partArg = data.args[a];
        size_t partIdx =
            data.scriptVM->mob->type->animDb->findBodyPart(partArg);
            
        if(partIdx == INVALID) {
            reportActionError(
                data,
                "Unknown body part \"" + partArg + "\"!"
            );
            return;
        }
        
        data.scriptVM->mob->chompBodyParts.push_back(partIdx);
    }
}


/**
 * @brief Code for the dying start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startDying(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->startDying();
}


/**
 * @brief Code for the height effect start script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::startHeightEffect(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->startHeightEffect();
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
        reportActionError(
            data,
            "Unknown particle generator \"" + genArg + "\"!"
        );
        return;
    }
    
    ParticleGenerator pg =
        standardParticleGenSetup(genArg, data.scriptVM->mob);
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
    //Main logic.
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
    //Main logic.
    data.scriptVM->mob->chompMax = 0;
    data.scriptVM->mob->chompBodyParts.clear();
}


/**
 * @brief Code for the height effect stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopHeightEffect(ScriptActionInstRunData& data) {
    //Main logic.
    data.scriptVM->mob->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::stopParticles(ScriptActionInstRunData& data) {
    //Main logic.
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
    data.scriptVM->mob->speedZ = 0;
}


/**
 * @brief Code for the focus storing script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::storeFocusInside(ScriptActionInstRunData& data) {
    //Main logic.
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
    //Get the arguments.
    const string& amountArg = data.args[0];
    
    //Main logic.
    data.scriptVM->mob->swallowChompedPikmin(s2i(amountArg));
}


/**
 * @brief Code for the swallow all script action type.
 *
 * @param data Data about the action call.
 */
void ScriptActionRunners::swallowAll(ScriptActionInstRunData& data) {
    //Main logic.
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
    //Get the arguments.
    const string& xArg = data.args[0];
    const string& yArg = data.args[1];
    const string& zArg = data.args[2];
    
    //Main logic.
    data.scriptVM->mob->stopChasing();
    data.scriptVM->mob->chase(
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
    data.scriptVM->mob->stopChasing();
    Point p =
        rotatePoint(
            Point(s2f(xArg), s2f(yArg)),
            data.scriptVM->mob->angle
        );
    data.scriptVM->mob->chase(
        data.scriptVM->mob->pos + p,
        data.scriptVM->mob->z + s2f(zArg),
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
    if(!data.scriptVM->focusedMob) {
        return;
    }
    
    if(data.scriptVM->focusedMob->holder.m == data.scriptVM->mob) {
        data.scriptVM->mob->release(data.scriptVM->focusedMob);
    }
    
    float maxHeight = s2f(maxHeightArg);
    if(maxHeight == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.scriptVM->mob->startHeightEffect();
    calculateThrow(
        data.scriptVM->focusedMob->pos, data.scriptVM->focusedMob->z,
        Point(s2f(xArg), s2f(yArg)), s2f(zArg),
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
    //Get the arguments.
    const string& angleOrXArg = data.args[0];
    const string& yArg = data.args[1];
    
    //Main logic.
    if(yArg.empty()) {
        //Turn to an absolute angle.
        data.scriptVM->mob->face(degToRad(s2f(angleOrXArg)), nullptr);
    } else {
        //Turn to some absolute coordinates.
        float x = s2f(angleOrXArg);
        float y = s2f(yArg);
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
    //Get the arguments.
    const string& angleOrXArg = data.args[0];
    const string& yArg = data.args[1];
    
    //Main logic.
    if(yArg.empty()) {
        //Turn to a relative angle.
        data.scriptVM->mob->face(
            data.scriptVM->mob->angle + degToRad(s2f(angleOrXArg)),
            nullptr
        );
    } else {
        //Turn to some relative coordinates.
        float x = s2f(angleOrXArg);
        float y = s2f(yArg);
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
    //Get the arguments.
    const string& targetArg = data.args[0];
    
    //Main logic.
    bool targetFound;
    SCRIPT_ACTION_TURN_TYPE target =
        enumGetValue(scriptActionTurnTypeINames, targetArg, &targetFound);
    if(!targetFound) {
        reportActionError(
            data,
            "Unknown target type \"" + targetArg + "\"!"
        );
        return;
    }
    
    switch(target) {
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
 * @param flags Flags. Use SCRIPT_ACTION_PARAM_FLAG.
 * @param defValue If this is optional, specify its default value here.
 */
ScriptActionTypeParam::ScriptActionTypeParam(
    const string& name, const SCRIPT_ACTION_PARAM_TYPE type,
    Bitmask8 flags, const string& defValue
):
    name(name),
    type(type),
    flags(flags),
    defValue(defValue) {
    
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


#pragma endregion
