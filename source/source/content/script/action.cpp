/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Script action classes and related functions.
 */


#include "action.hpp"

#include "../../core/game.hpp"
#include "script_utils.hpp"


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
 * @param actionList List of actions so far.
 * @param isDataNodeRelevant Whether the action was loaded from a data node
 * that is relevant to mob type or area it belongs to, or if it got loaded
 * from an external source.
 * @return Whether it was successful.
 */
bool ScriptActionDef::loadFromDataNode(
    DataNode* node, ScriptDef* scriptDef,
    const vector<ScriptActionDef*>& actionList, bool isDataNodeRelevant
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
        game.log.report(
            LOG_LEVEL_ERROR,
            "Unknown script action name \"" + name + "\"!", node
        );
        return false;
    }
    
    //Check if it's deprecated.
    if(!actionType->deprecationReplacement.empty()) {
        if(game.makerTools.useWarnings) {
            game.log.report(
                LOG_LEVEL_WARNING,
                "The action \"" + actionType->name + "\" is deprecated "
                "and may be removed in the future! Please use \"" +
                actionType->deprecationReplacement + "\" instead."
            );
        }
    }
    
    //Check if this action is allowed in the current context.
    if(!scriptDef->checkContextFlags(actionType->contexts)) {
        bool canUse = false;
        if(hasFlag(actionType->contexts, getIdxBitmask(SCRIPT_CONTEXT_MOB))) {
            if(
                !actionList.empty() &&
                actionList.back()->actionType->type ==
                SCRIPT_ACTION_RUN_NEXT_ACTION_AS
            ) {
                //If it can be used in a mob context and we're specifying
                //a surrogate mob just before, then yes, we can run it.
                canUse = true;
            }
        }
        if(!canUse) {
            game.log.report(
                LOG_LEVEL_ERROR,
                "The action \"" + name + "\" can't be used in an " +
                scriptDef->getContextName() + " script context!", node
            );
            return false;
        }
    }
    
    //Check if there are too many or too few arguments.
    size_t nMandatoryParams = 0;
    size_t nFixedParams = 0;
    bool hasVectorParam = false;
    forIdx(p, actionType->parameters) {
        CommandParam* pPtr = &actionType->parameters[p];
        if(hasFlag(pPtr->flags, COMMAND_PARAM_FLAG_VECTOR)) {
            hasVectorParam = true;
        } else {
            nFixedParams++;
        }
        if(!hasFlag(pPtr->flags, COMMAND_PARAM_FLAG_OPTIONAL)) {
            nMandatoryParams++;
        }
    }
    
    if(words.size() < nMandatoryParams) {
        game.log.report(
            LOG_LEVEL_ERROR,
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
            game.log.report(
                LOG_LEVEL_ERROR,
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
                    COMMAND_PARAM_FLAG_CONST
                )
            ) {
                game.log.report(
                    LOG_LEVEL_ERROR,
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
                game.log.report(
                    LOG_LEVEL_ERROR,
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
            data.args[a].clear();
            scriptVM->vars.getValue(args[a], data.args[a]);
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
#pragma region Script action list definition


/**
 * @brief Compiles some caches and confirms if the condition-related,
 * loop-related, and jump-related actions are all okay, and that there are no
 * mismatches, like for instance, an "else" without an "if".
 * Also checks if there are actions past a "set_state" action, and more.
 * If something goes wrong, it throws the errors to the log.
 *
 * @param dn Data node from where these actions came. Used for reporting errors.
 * @return Whether everything is okay.
 */
bool ScriptActionListDef::compile(DataNode* dn) {
    vector<SCRIPT_CONSTRUCT_TYPE> constructStack;
    vector<bool> seenElseStack;
    
    depthsCache.clear();
    
    //Check if there are no depth mismatches.
    //This also builds the depths cache.
    forIdx(a, list) {
        bool ok = true;
        int depthToSave = constructStack.size();
        
        const auto assertStack =
            [&constructStack, &ok, &dn, &a, this] (
                SCRIPT_CONSTRUCT_TYPE curExpectedConstruct
        ) {
            if(
                constructStack.empty() ||
                constructStack.back() != curExpectedConstruct
            ) {
                string foundActionStr =
                    list[a]->actionType->name;
                string expectedBlockStr =
                    enumGetName(
                        constructTypeActionNames, curExpectedConstruct
                    );
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "Found " + aAn(foundActionStr, true) + " \"" +
                    foundActionStr + "\" action outside of " +
                    aAn(expectedBlockStr, true) + " \"" +
                    expectedBlockStr + "\" block!",
                    dn
                );
                ok = false;
            }
        };
        
        switch(list[a]->actionType->type) {
        case SCRIPT_ACTION_DO_WHILE: {
            constructStack.push_back(SCRIPT_CONSTRUCT_TYPE_DO_WHILE);
            break;
            
        } case SCRIPT_ACTION_FOR: {
            constructStack.push_back(SCRIPT_CONSTRUCT_TYPE_FOR);
            break;
            
        } case SCRIPT_ACTION_FOR_EACH: {
            constructStack.push_back(SCRIPT_CONSTRUCT_TYPE_FOR_EACH);
            break;
            
        } case SCRIPT_ACTION_IF: {
            constructStack.push_back(SCRIPT_CONSTRUCT_TYPE_IF);
            seenElseStack.push_back(false);
            break;
            
        } case SCRIPT_ACTION_WHILE_DO: {
            constructStack.push_back(SCRIPT_CONSTRUCT_TYPE_WHILE_DO);
            break;
            
        } case SCRIPT_ACTION_ELSE: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_IF);
            seenElseStack.back() = true;
            depthToSave = constructStack.size() - 1;
            break;
        }
        case SCRIPT_ACTION_ELSE_IF: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_IF);
            if(seenElseStack.back()) {
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "Found an \"else_if\" action after an \"else\" action!",
                    dn
                );
                return false;
            }
            depthToSave = constructStack.size() - 1;
            break;
            
        } case SCRIPT_ACTION_END_DO_WHILE: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_DO_WHILE);
            constructStack.pop_back();
            depthToSave = constructStack.size();
            break;
            
        } case SCRIPT_ACTION_END_FOR: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_FOR);
            constructStack.pop_back();
            depthToSave = constructStack.size();
            break;
            
        } case SCRIPT_ACTION_END_FOR_EACH: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_FOR_EACH);
            constructStack.pop_back();
            depthToSave = constructStack.size();
            break;
            
        } case SCRIPT_ACTION_END_IF: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_IF);
            constructStack.pop_back();
            depthToSave = constructStack.size();
            break;
            
        } case SCRIPT_ACTION_END_WHILE_DO: {
            assertStack(SCRIPT_CONSTRUCT_TYPE_WHILE_DO);
            constructStack.pop_back();
            depthToSave = constructStack.size();
            break;
            
        } default: {
            break;
            
        }
        }
        
        if(!ok) return false;
        depthsCache.push_back(depthToSave);
    }
    
    //Check if there are no blocks that started but never ended.
    while(!constructStack.empty()) {
        string foundBlockStr =
            enumGetName(
                constructTypeActionNames, constructStack.back()
            );
        game.log.report(
            LOG_LEVEL_ERROR,
            "Found " + aAn(foundBlockStr, true) + " \"" +
            foundBlockStr + "\" block that has no end!",
            dn
        );
        return false;
    }
    
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
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            seenElseAction.back() = true;
            break;
        } case SCRIPT_ACTION_ELSE_IF: {
            if(depth == 0) {
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "Found an \"else_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            if(seenElseAction.back()) {
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "Found an \"else_if\" action after an \"else\" action!",
                    dn
                );
                return false;
            }
            break;
        } case SCRIPT_ACTION_END_IF: {
            if(depth == 0) {
                game.log.report(
                    LOG_LEVEL_ERROR,
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
        game.log.report(
            LOG_LEVEL_ERROR,
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
                game.log.report(
                    LOG_LEVEL_ERROR,
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
                game.log.report(
                    LOG_LEVEL_ERROR,
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
        } case SCRIPT_ACTION_END_DO_WHILE: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_END_FOR: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_END_FOR_EACH: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_END_IF: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_END_WHILE_DO: {
            passedSetState = false;
            break;
        } case SCRIPT_ACTION_LABEL: {
            passedSetState = false;
            break;
        } default: {
            if(passedSetState) {
                game.log.report(
                    LOG_LEVEL_ERROR,
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
    
    //Check if there are missing actions after a "run_next_action_as".
    forIdx(a, list) {
        if(list[a]->actionType->type == SCRIPT_ACTION_RUN_NEXT_ACTION_AS) {
            if(a == list.size() - 1) {
                game.log.report(
                    LOG_LEVEL_ERROR,
                    "There is a \"" + list[a]->actionType->name + "\" "
                    "with no other action after it!", dn
                );
                return false;
            }
        }
    }
    
    return true;
}


/**
 * @brief Returns the process type of the given action.
 *
 * @param actionIdx The action's index in the list.
 * @param mustProcessElseIfConditionPtr Pointer to a variable that dictates
 * whether the condition of an "else_if" action should be processed.
 * @return The type.
 */
ScriptActionListDef::PROCESS_TYPE
ScriptActionListDef::getActionProcessType(
    size_t actionIdx, bool* mustProcessElseIfConditionPtr
) const {
    ScriptActionDef* curAction = list[actionIdx];
    PROCESS_TYPE result;
    
    switch(curAction->actionType->type) {
    case SCRIPT_ACTION_END_DO_WHILE:
    case SCRIPT_ACTION_FOR:
    case SCRIPT_ACTION_FOR_EACH:
    case SCRIPT_ACTION_IF:
    case SCRIPT_ACTION_WHILE_DO: {
        //We must check the condition and then decide where to go.
        result = PROCESS_TYPE_CHECK_CONDITION;
        break;
        
    } case SCRIPT_ACTION_ELSE_IF: {
        if(*mustProcessElseIfConditionPtr) {
            //We landed here after the "false" branch of the previous
            //condition. We need to check this new condition.
            result = PROCESS_TYPE_CHECK_CONDITION;
            *mustProcessElseIfConditionPtr = false;
        } else {
            //We landed here after the "true" branch of the previous
            //condition. We need to jump to the end of the current construct.
            result = PROCESS_TYPE_JUMP_TO_CONSTRUCT_END;
        }
        break;
        
    } case SCRIPT_ACTION_ELSE: {
        //We landed here either after the "true" or the "false" branch of
        //the previous condition. Either way we don't need the "else" or
        //its branch. We need to jump to the end of the current construct.
        result = PROCESS_TYPE_JUMP_TO_CONSTRUCT_END;
        break;
        
    } case SCRIPT_ACTION_END_FOR:
    case SCRIPT_ACTION_END_FOR_EACH:
    case SCRIPT_ACTION_END_WHILE_DO:
    case SCRIPT_ACTION_GOTO: {
        //Jump elsewhere.
        result = PROCESS_TYPE_JUMP_ELSEWHERE;
        break;
        
    } case SCRIPT_ACTION_DO_WHILE:
    case SCRIPT_ACTION_END_IF:
    case SCRIPT_ACTION_LABEL: {
        //Nothing to do here. These are just end points for other jumps.
        result = PROCESS_TYPE_DO_NOTHING;
        break;
        
    } default: {
        //Normal action.
        result = PROCESS_TYPE_NORMAL;
        break;
        
    }
    }
    
    return result;
}


/**
 * @brief Loads a list from a data node.
 *
 * @param node The node.
 * @param scriptDef Script definition it belongs to, if any.
 * @param outFlags If not nullptr, the EVENT_LOAD_FLAG_* flags are
 * returned here.
 * @return Whether everything succeeded.
 */
bool ScriptActionListDef::loadFromDataNode(
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
            if(newA->loadFromDataNode(actionNode, scriptDef, list)) {
                list.push_back(newA);
            } else {
                delete newA;
            }
        }
    }
    
    return true;
}


/**
 * @brief Processes an action in the list. This might or might not run
 * the action's code, and then returns what the next action index should be.
 *
 * @param actionIdx Index of the action to process.
 * @param processType Type of process.
 * @param mustProcessElseIfConditionPtr Pointer to a variable that dictates
 * whether the condition of an "else_if" action should be processed.
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 * @return The next action's index.
 */
size_t ScriptActionListDef::processAction(
    size_t actionIdx, ScriptActionListDef::PROCESS_TYPE processType,
    bool* mustProcessElseIfConditionPtr, ScriptVM* scriptVM,
    void* customData1, void* customData2
) {
    ScriptActionDef* curAction = list[actionIdx];
    
    switch(processType) {
    case PROCESS_TYPE_CHECK_CONDITION: {
        //Run the action and check the condition. Then either proceed to
        //the next sequential action or jump to another action accordingly.
        return
            processActionCheckCondition(
                actionIdx, mustProcessElseIfConditionPtr,
                scriptVM, customData1, customData2
            );
        break;
        
    } case PROCESS_TYPE_JUMP_TO_CONSTRUCT_END: {
        //Do not run this action. Then jump to the end of the condition.
        return
            processActionJumpToConstructEnd(
                actionIdx, scriptVM, customData1, customData2
            );
        break;
        
    } case PROCESS_TYPE_JUMP_ELSEWHERE: {
        //Do not run this action.
        //Then jump to the corresponding label action.
        return
            processActionJumpToLabel(
                actionIdx, scriptVM, customData1, customData2
            );
        break;
        
    } case PROCESS_TYPE_DO_NOTHING: {
        //Do not run this action. Then proceed to the next sequential action.
        return actionIdx + 1;
        break;
        
    } case PROCESS_TYPE_NORMAL: {
        //Run the action. Then proceed to the next sequential action.
        curAction->run(scriptVM, customData1, customData2);
        
        //Reset the surrogate, if applicable.
        if(
            scriptVM->nextActionSurrogateMob &&
            curAction->actionType->type != SCRIPT_ACTION_RUN_NEXT_ACTION_AS
        ) {
            scriptVM->nextActionSurrogateMob = nullptr;
        }
        
        //If the state got changed, jump out.
        if(curAction->actionType->type == SCRIPT_ACTION_SET_STATE) {
            return list.size();
        } else {
            return actionIdx + 1;
        }
        
        break;
    }
    }
    
    //If anything goes wrong, quit the list.
    return list.size();
}


/**
 * @brief Processes an action in the list with the "check condition"
 * processing type. This run's the action's code,
 * and then returns what the next action index should be.
 *
 * @param actionIdx Index of the action to process.
 * @param mustProcessElseIfConditionPtr Pointer to a variable that dictates
 * whether the condition of an "else_if" action should be processed.
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 * @return The next action's index.
 */
size_t ScriptActionListDef::processActionCheckCondition(
    size_t actionIdx, bool* mustProcessElseIfConditionPtr,
    ScriptVM* scriptVM, void* customData1, void* customData2
) {
    ScriptActionDef* curAction = list[actionIdx];
    
    bool conditionValue =
        curAction->run(scriptVM, customData1, customData2);
        
    switch(curAction->actionType->type) {
    case SCRIPT_ACTION_END_DO_WHILE: {
        if(conditionValue) {
            //Returned true. Go to the start of the loop for another go.
            for(int a2 = actionIdx - 1; a2 >= 0; a2--) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(
                    a2Type == SCRIPT_ACTION_DO_WHILE &&
                    depthsCache[a2] == depthsCache[actionIdx]
                ) {
                    return a2 + 1;
                }
            }
            
        } else {
            //Returned false. Leave the loop by letting execution
            //continue to the next sequential action.
            return actionIdx + 1;
            
        }
        
        break;
        
    } case SCRIPT_ACTION_ELSE_IF:
    case SCRIPT_ACTION_IF: {
        if(conditionValue) {
            //Returned true. Execution continues as normal to the next
            //sequential action, which is the start of the "true" branch.
            return actionIdx + 1;
            
        } else {
            //Returned false. Search for the next "else", "else_if",
            //or "end_if" action.
            for(size_t a2 = actionIdx + 1; a2 < list.size(); a2++) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                
                switch(a2Type) {
                case SCRIPT_ACTION_ELSE: {
                    //If this is our condition's "else", we want
                    //to run the code after it.
                    if(depthsCache[a2] == depthsCache[actionIdx]) {
                        return a2 + 1;
                    }
                    break;
                    
                } case SCRIPT_ACTION_ELSE_IF: {
                    //If this is our condition's next "else_if", we want
                    //to check the condition and decide again based on that.
                    if(depthsCache[a2] == depthsCache[actionIdx]) {
                        *mustProcessElseIfConditionPtr = true;
                        return a2;
                    }
                    break;
                    
                } case SCRIPT_ACTION_END_IF: {
                    //If this is our condition's "end_if", we're done with
                    //the condition. Execution continues like normal after.
                    if(depthsCache[a2] == depthsCache[actionIdx]) {
                        return a2 + 1;
                    }
                    break;
                    
                } default: {
                    break;
                    
                }
                }
            }
            
        }
        
        break;
        
    } case SCRIPT_ACTION_FOR: {
        if(conditionValue) {
            //Returned true. Continue to the next sequential action
            //in order to enter the loop.
            return actionIdx + 1;
            
        } else {
            //Returned false. Go to the end of the loop.
            for(size_t a2 = actionIdx + 1; a2 < list.size(); a2++) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(
                    a2Type == SCRIPT_ACTION_END_FOR &&
                    depthsCache[a2] == depthsCache[actionIdx]
                ) {
                    return a2 + 1;
                }
            }
            
        }
        
        break;
        
    } case SCRIPT_ACTION_FOR_EACH: {
        if(conditionValue) {
            //Returned true. Continue to the next sequential action
            //in order to enter the loop.
            return actionIdx + 1;
            
        } else {
            //Returned false. Go to the end of the loop.
            for(size_t a2 = actionIdx + 1; a2 < list.size(); a2++) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(
                    a2Type == SCRIPT_ACTION_END_FOR_EACH &&
                    depthsCache[a2] == depthsCache[actionIdx]
                ) {
                    return a2 + 1;
                }
            }
            
        }
        
        break;
        
    } case SCRIPT_ACTION_WHILE_DO: {
        if(conditionValue) {
            //Returned true. Continue to the next sequential action
            //in order to enter the loop.
            return actionIdx + 1;
            
        } else {
            //Returned false. Go to the end of the loop.
            for(size_t a2 = actionIdx + 1; a2 < list.size(); a2++) {
                SCRIPT_ACTION a2Type = list[a2]->actionType->type;
                if(
                    a2Type == SCRIPT_ACTION_END_WHILE_DO &&
                    depthsCache[a2] == depthsCache[actionIdx]
                ) {
                    return a2 + 1;
                }
            }
            
        }
        
        break;
        
    } default: {
        break;
        
    }
    }
    
    //If anything goes wrong, quit the list.
    return list.size();
}


/**
 * @brief Processes an action in the list with the "jump to construct end"
 * processing type. This returns what the next action index should be.
 *
 * @param actionIdx Index of the action to process.
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 * @return The next action's index.
 */
size_t ScriptActionListDef::processActionJumpToConstructEnd(
    size_t actionIdx, ScriptVM* scriptVM, void* customData1, void* customData2
) {
    for(size_t a2 = actionIdx + 1; a2 < list.size(); a2++) {
        SCRIPT_ACTION a2Type = list[a2]->actionType->type;
        
        if(
            a2Type == SCRIPT_ACTION_END_IF &&
            depthsCache[a2] == depthsCache[actionIdx]
        ) {
            //If this is our condition's "end_if", we're done with
            //the condition. Execution continues like normal after.
            //If not, we just update the current depth.
            return a2 + 1;
        }
    }
    
    //If anything goes wrong, quit the list.
    return list.size();
}


/**
 * @brief Processes an action in the list with the "jump to label"
 * processing type. This returns what the next action index should be.
 *
 * @param actionIdx Index of the action to process.
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 * @return The next action's index.
 */
size_t ScriptActionListDef::processActionJumpToLabel(
    size_t actionIdx, ScriptVM* scriptVM, void* customData1, void* customData2
) {
    ScriptActionDef* curAction = list[actionIdx];
    
    switch(curAction->actionType->type) {
    case SCRIPT_ACTION_END_FOR: {
        for(int a2 = actionIdx - 1; a2 >= 0; a2--) {
            SCRIPT_ACTION a2Type = list[a2]->actionType->type;
            if(
                a2Type == SCRIPT_ACTION_FOR &&
                depthsCache[a2] == depthsCache[actionIdx]
            ) {
                game.scriptExecAuxData.forLoopEntryNeedsIncrement = true;
                return a2;
            }
        }
        break;
        
    } case SCRIPT_ACTION_END_FOR_EACH: {
        for(int a2 = actionIdx - 1; a2 >= 0; a2--) {
            SCRIPT_ACTION a2Type = list[a2]->actionType->type;
            if(
                a2Type == SCRIPT_ACTION_FOR_EACH &&
                depthsCache[a2] == depthsCache[actionIdx]
            ) {
                game.scriptExecAuxData.forLoopEntryNeedsIncrement = true;
                return a2;
            }
        }
        break;
        
    } case SCRIPT_ACTION_END_WHILE_DO: {
        for(int a2 = actionIdx - 1; a2 >= 0; a2--) {
            SCRIPT_ACTION a2Type = list[a2]->actionType->type;
            if(
                a2Type == SCRIPT_ACTION_WHILE_DO &&
                depthsCache[a2] == depthsCache[actionIdx]
            ) {
                return a2;
            }
        }
        break;
        
    } case SCRIPT_ACTION_GOTO: {
        forIdx(a2, list) {
            SCRIPT_ACTION a2Type = list[a2]->actionType->type;
            if(a2Type == SCRIPT_ACTION_LABEL) {
                if(curAction->args[0] == list[a2]->args[0]) {
                    return a2 + 1;
                }
            }
        }
        break;
        
    } default: {
        break;
        
    }
    }
    
    //If anything goes wrong, quit the list.
    return list.size();
}


/**
 * @brief Runs a list of actions.
 *
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom data #1.
 * @param customData2 Custom data #2.
 */
void ScriptActionListDef::run(
    ScriptVM* scriptVM, void* customData1, void* customData2
) {
    size_t curActionIdx = 0;
    bool mustProcessElseIfCondition = false;
    
    while(curActionIdx < list.size()) {
        ScriptActionDef* curAction = list[curActionIdx];
        
        //First, check the infinite loop safeguard.
        game.scriptExecAuxData.nConsecutiveActions++;
        if(
            game.scriptExecAuxData.nConsecutiveActions >
            GAME::MAX_CONSECUTIVE_SCRIPT_ACTIONS
        ) [[unlikely]] {
            ScriptActionInstRunData data(scriptVM, curAction);
            ScriptActionUtils::reportActionError(
                data,
                "Failed to run action! Since the game already ran " +
                i2s(game.scriptExecAuxData.nConsecutiveActions - 1) +
                " actions in a row, it's probably in an infinite loop. "
                "Please correct the script to make sure "
                "this doesn't happen!"
            );
            return;
        }
        
        //Decide how to process it.
        PROCESS_TYPE processType =
            getActionProcessType(curActionIdx, &mustProcessElseIfCondition);
            
        //Process it.
        curActionIdx =
            processAction(
                curActionIdx, processType, &mustProcessElseIfCondition,
                scriptVM, customData1, customData2
            );
    }
}


/**
 * @brief Unloads the action list definition and its contents from memory.
 */
void ScriptActionListDef::unload() {
    forIdx(a, list) {
        list[a]->unload();
        delete list[a];
    }
    list.clear();
}
