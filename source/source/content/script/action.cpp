/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Script action classes and related functions.
 */


#include "action.h"

#include "../../core/game.h"
#include "script_utils.h"


#pragma region Script action block definition


/**
 * @brief Confirms if the "if", "else", "end_if", "goto", and "label" actions in
 * a given vector of actions are all okay, and there are no mismatches, like
 * for instance, an "else" without an "if".
 * Also checks if there are actions past a "set_state" action.
 * If something goes wrong, it throws the errors to the error log.
 *
 * @param dn Data node from where these actions came.
 * @return Whether it succeeded.
 */
bool ScriptActionBlockDef::assertActions(DataNode* dn) {
    //Check if the "if"-related actions are okay.
    int depth = 0;
    vector<bool> seenElseAction;
    forIdx(a, list) {
        switch(list[a]->actionType->type) {
        case SCRIPT_ACTION_IF: {
            depth++;
            seenElseAction.push_back(false);
            break;
        } case SCRIPT_ACTION_ELSE: {
            if(depth == 0) {
                game.errors.report(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            seenElseAction.back() = true;
            break;
        } case SCRIPT_ACTION_ELSE_IF: {
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
        } case SCRIPT_ACTION_END_IF: {
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
    forIdx(a, list) {
        if(list[a]->actionType->type == SCRIPT_ACTION_LABEL) {
            const string& name = list[a]->args[0];
            if(isInContainer(labels, name)) {
                game.errors.report(
                    "There are multiple labels called \"" + name + "\"!", dn
                );
                return false;
            }
            labels.insert(name);
        }
    }
    forIdx(a, list) {
        if(list[a]->actionType->type == SCRIPT_ACTION_GOTO) {
            const string& name = list[a]->args[0];
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
    forIdx(a, list) {
        switch(list[a]->actionType->type) {
        case SCRIPT_ACTION_SET_STATE: {
            passedSetState = true;
            break;
        } case SCRIPT_ACTION_ELSE: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_ELSE_IF: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_END_IF: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_LABEL: {
            passedSetState = false;
            break;
        } default: {
            if(passedSetState) {
                game.errors.report(
                    "There is an action \"" + list[a]->actionType->name + "\" "
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
 * @brief Loads a block from a data node.
 *
 * @param node The node.
 * @param scriptDef Script definition it belongs to, if any.
 * @param outFlags If not nullptr, the EVENT_LOAD_FLAG_* flags are
 * returned here.
 * @return Whether everything succeeded.
 */
bool ScriptActionBlockDef::loadFromDataNode(
    DataNode* node, ScriptDef* scriptDef, Bitmask8* outFlags
) {
    if(outFlags) *outFlags = 0;
    for(size_t a = 0; a < node->getNrOfChildren(); a++) {
        DataNode* actionNode = node->getChild(a);
        if(outFlags && actionNode->name == "custom_actions_after") {
            enableFlag(*outFlags, EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER);
        } else if(outFlags && actionNode->name == "global_actions_after") {
            enableFlag(*outFlags, EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER);
        } else {
            ScriptActionDef* newA = new ScriptActionDef();
            if(newA->loadFromDataNode(actionNode, scriptDef)) {
                list.push_back(newA);
            } else {
                delete newA;
            }
        }
    }
    
    return assertActions(node);
}


/**
 * @brief Runs a block of actions.
 *
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 */
void ScriptActionBlockDef::run(
    ScriptVM* scriptVM, void* customData1, void* customData2
) {
    enum FLOW_CODE {
        FLOW_CODE_NONE,
        FLOW_CODE_CONDITION,
        FLOW_CODE_CONDITION_OTHER_BRANCH,
        FLOW_CODE_JUMP,
        FLOW_CODE_DO_NOTHING,
    };
    
    FLOW_CODE flowCodeToRun = FLOW_CODE_NONE;
    bool processElseIfCondition = false;
    
    forIdx(a, list) {
    
        //Prevent infinite loops.
        game.nConsecutiveScriptActions++;
        if(
            game.nConsecutiveScriptActions >
            GAME::MAX_CONSECUTIVE_SCRIPT_ACTIONS
        ) [[unlikely]] {
            ScriptActionInstRunData data(scriptVM, list[a]);
            ScriptActionRunners::reportActionError(
                data,
                "Failed to run action! Since the game already ran " +
                i2s(game.nConsecutiveScriptActions - 1) +
                " actions in a row, it's probably in an infinite loop "
                "infinite loops. Please correct the script to make sure "
                "this doesn't happen!"
            );
            return;
        }
        
        //Figure out the flow code to run.
        switch(list[a]->actionType->type) {
        case SCRIPT_ACTION_IF: {
            flowCodeToRun = FLOW_CODE_CONDITION;
            break;
        } case SCRIPT_ACTION_ELSE_IF: {
            if(processElseIfCondition) {
                flowCodeToRun = FLOW_CODE_CONDITION;
                processElseIfCondition = false;
            } else {
                flowCodeToRun = FLOW_CODE_CONDITION_OTHER_BRANCH;
            }
            break;
        } case SCRIPT_ACTION_ELSE: {
            flowCodeToRun = FLOW_CODE_CONDITION_OTHER_BRANCH;
            break;
        } case SCRIPT_ACTION_GOTO: {
            flowCodeToRun = FLOW_CODE_JUMP;
            break;
        } case SCRIPT_ACTION_END_IF:
        case SCRIPT_ACTION_LABEL: {
            flowCodeToRun = FLOW_CODE_DO_NOTHING;
            break;
        } default: {
            flowCodeToRun = FLOW_CODE_NONE;
            break;
        }
        }
        
        //Run the flow code.
        switch(flowCodeToRun) {
        case FLOW_CODE_CONDITION: {
            //Condition statement. Look out for its return value, and
            //change the flow accordingly.
            bool conditionValue =
                list[a]->run(scriptVM, customData1, customData2);
                
            if(conditionValue) {
                //Returned true. Execution continues as normal.
            } else {
                //Returned false. Skip to the "else", "else if",
                //or "end if" actions.
                size_t nextActionIdx = list.size();
                size_t depth = 0;
                
                for(size_t a2 = a + 1; a2 < list.size(); a2++) {
                    SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                    if(a2Type == SCRIPT_ACTION_IF) {
                        depth++;
                    } else if(a2Type == SCRIPT_ACTION_ELSE) {
                        if(depth == 0) {
                            nextActionIdx = a2 + 1;
                            break;
                        }
                    } else if(a2Type == SCRIPT_ACTION_ELSE_IF) {
                        if(depth == 0) {
                            processElseIfCondition = true;
                            nextActionIdx = a2;
                            break;
                        }
                    } else if(a2Type == SCRIPT_ACTION_END_IF) {
                        if(depth == 0) {
                            nextActionIdx = a2 + 1;
                            break;
                        } else {
                            depth--;
                        }
                    }
                }
                a = nextActionIdx - 1;
                
            }
            
            break;
            
        } case FLOW_CODE_CONDITION_OTHER_BRANCH: {
            //If we actually managed to read an "else" or "else if",
            //that means we were running through the normal execution of some
            //"then" section. Jump to the "end if".
            size_t nextActionIdx = list.size();
            size_t depth = 0;
            
            for(size_t a2 = a + 1; a2 < list.size(); a2++) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(a2Type == SCRIPT_ACTION_IF) {
                    depth++;
                } else if(a2Type == SCRIPT_ACTION_END_IF) {
                    if(depth == 0) {
                        nextActionIdx = a2 + 1;
                        break;
                    } else {
                        depth--;
                    }
                }
            }
            a = nextActionIdx - 1;
            break;
            
        } case FLOW_CODE_JUMP: {
            //Find the label that matches.
            size_t nextActionIdx = list.size();
            forIdx(a2, list) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(a2Type == SCRIPT_ACTION_LABEL) {
                    if(list[a]->args[0] == list[a2]->args[0]) {
                        nextActionIdx = a2 + 1;
                        break;
                    }
                }
            }
            a = nextActionIdx - 1;
            break;
            
        } case FLOW_CODE_DO_NOTHING: {
            //Nothing to do.
            break;
            
        } case FLOW_CODE_NONE: {
            //Normal action.
            list[a]->run(scriptVM, customData1, customData2);
            //If the state got changed, jump out.
            if(list[a]->actionType->type == SCRIPT_ACTION_SET_STATE) return;
            
            break;
        }
        }
    }
}


/**
 * @brief Unloads the action block definition and its contents from memory.
 */
void ScriptActionBlockDef::unload() {
    forIdx(a, list) {
        list[a]->unload();
        delete list[a];
    }
    list.clear();
}


#pragma endregion
#pragma region Script action definition


/**
 * @brief Constructs a new script action call object of a certain type.
 *
 * @param type Type of script action call.
 */
ScriptActionDef::ScriptActionDef(SCRIPT_ACTION type) {
    forIdx(a, game.scriptActionTypes) {
        if(game.scriptActionTypes[a].type == type) {
            actionType = &(game.scriptActionTypes[a]);
            break;
        }
    }
}


/**
 * @brief Constructs a new script action call object meant to run custom code.
 *
 * @param code The function to run.
 */
ScriptActionDef::ScriptActionDef(ScriptActionCustomCode code) :
    customCode(code) {
    
    forIdx(a, game.scriptActionTypes) {
        if(game.scriptActionTypes[a].type == SCRIPT_ACTION_UNKNOWN) {
            actionType = &(game.scriptActionTypes[a]);
            break;
        }
    }
}


/**
 * @brief Loads a script action call from a data node.
 *
 * @param node The data node.
 * @param scriptDef Script definition it belongs to, if any.
 * @param isDataNodeRelevant Whether the action was loaded from a data node
 * that is relevant to mob type or area it belongs to, or if it got loaded
 * from an external source.
 * @return Whether it was successful.
 */
bool ScriptActionDef::loadFromDataNode(
    DataNode* node, ScriptDef* scriptDef, bool isDataNodeRelevant
) {
    actionType = nullptr;
    if(isDataNodeRelevant) {
        dataFileLine = node->lineNr;
    }
    
    //First, get the name and arguments.
    vector<string> words = split(node->name);
    
    forIdx(w, words) {
        words[w] = trimSpaces(words[w]);
    }
    
    string name = words[0];
    words.erase(words.begin());
    
    //Find the corresponding action type.
    forIdx(a, game.scriptActionTypes) {
        if(game.scriptActionTypes[a].type == SCRIPT_ACTION_UNKNOWN) continue;
        if(game.scriptActionTypes[a].name == name) {
            actionType = &(game.scriptActionTypes[a]);
        }
    }
    
    //Check if it is recognized.
    if(!actionType) {
        game.errors.report(
            "Unknown script action name \"" + name + "\"!", node
        );
        return false;
    }
    
    //Check if this action is allowed in the current context.
    if(!scriptDef->checkContextFlags(actionType->contexts)) {
        game.errors.report(
            "The action \"" + name + "\" can't be used in an " +
            scriptDef->getContextName() + " script context!", node
        );
        return false;
    }
    
    //Check if there are too many or too few arguments.
    size_t nMandatoryParams = 0;
    size_t nFixedParams = 0;
    bool hasVectorParam = false;
    forIdx(p, actionType->parameters) {
        ScriptActionTypeParam* pPtr = &actionType->parameters[p];
        if(hasFlag(pPtr->flags, SCRIPT_ACTION_PARAM_FLAG_VECTOR)) {
            hasVectorParam = true;
        } else {
            nFixedParams++;
        }
        if(!hasFlag(pPtr->flags, SCRIPT_ACTION_PARAM_FLAG_OPTIONAL)) {
            nMandatoryParams++;
        }
    }
    
    if(words.size() < nMandatoryParams) {
        game.errors.report(
            "The \"" + actionType->name + "\" action needs " +
            i2s(nMandatoryParams) + " arguments, but this call only "
            "has " + i2s(words.size()) + "! You're missing the \"" +
            actionType->parameters[words.size()].name + "\" parameter.",
            node
        );
        return false;
    }
    
    if(!hasVectorParam) {
        if(words.size() > nFixedParams) {
            game.errors.report(
                "The \"" + actionType->name + "\" action only needs " +
                i2s(actionType->parameters.size()) + " arguments, "
                "but this call has " + i2s(words.size()) + "!",
                node
            );
            return false;
        }
    }
    
    //Fetch the arguments, and check if any of them are not allowed.
    forIdx(w, words) {
        size_t paramIdx = w;
        if(hasVectorParam && w > actionType->parameters.size() - 1) {
            //This word just belongs to the final parameter.
            paramIdx = actionType->parameters.size() - 1;
        }
        bool isVar = (words[w][0] == '$' && words[w].size() > 1);
        
        if(isVar && words[w].size() >= 2 && words[w][1] == '$') {
            //Two '$' in a row means it's meant to use a literal '$'.
            isVar = false;
            words[w].erase(words[w].begin());
        }
        
        if(isVar) {
            if(
                hasFlag(
                    actionType->parameters[paramIdx].flags,
                    SCRIPT_ACTION_PARAM_FLAG_CONST
                )
            ) {
                game.errors.report(
                    "Argument #" + i2s(w + 1) + " (\"" + words[w] + "\") is a "
                    "variable, but the parameter \"" +
                    actionType->parameters[paramIdx].name + "\" can only be "
                    "constant!",
                    node
                );
                return false;
            }
            
            words[w].erase(words[w].begin()); //Remove the '$'.
            
            if(words[w].empty()) {
                game.errors.report(
                    "Argument #" + i2s(w) + " is trying to use a variable "
                    "with no name!",
                    node
                );
                return false;
            }
        }
        
        args.push_back(words[w]);
        argIsVar.push_back(isVar);
    }
    
    //Check if any optional parameters were left out.
    while(words.size() < nFixedParams) {
        size_t paramIdx = words.size();
        words.push_back(actionType->parameters[paramIdx].defValue);
        args.push_back(words.back());
        argIsVar.push_back(false);
    }
    
    return true;
}


/**
 * @brief Runs a script action.
 *
 * @param scriptVM The script VM responsible.
 * @param customData1 Custom argument #1 to pass to the code.
 * @param customData2 Custom argument #2 to pass to the code.
 * @return Evaluation result, used only by the "if" actions.
 */
bool ScriptActionDef::run(
    ScriptVM* scriptVM, void* customData1, void* customData2
) {
    //Custom code (i.e. instead of script actions, use actual C++ code).
    if(customCode) {
        customCode(scriptVM, customData1, customData2);
        return false;
    }
    
    ScriptActionInstRunData data(scriptVM, this);
    
    //Fill the arguments. Fetch values from variables if needed.
    data.args = args;
    forIdx(a, args) {
        if(argIsVar[a]) {
            data.args[a] = scriptVM->vars[args[a]];
        }
    }
    data.customData1 = customData1;
    data.customData2 = customData2;
    
    actionType->code(data);
    return data.returnValue;
}


/**
 * @brief Unloads the state definition and its contents from memory.
 */
void ScriptActionDef::unload() {
    args.clear();
    argIsVar.clear();
}


#pragma endregion
