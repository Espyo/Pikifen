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

    for(size_t a = 0; a < game.mob_actions.size(); a++) {
        if(game.mob_actions[a].type == type) {
            action = &(game.mob_actions[a]);
            break;
        }
    }
}


/**
 * @brief Constructs a new mob action call object meant to run custom code.
 *
 * @param code The function to run.
 */
MobActionCall::MobActionCall(custom_action_code_t code) :
    code(code) {
    
    for(size_t a = 0; a < game.mob_actions.size(); a++) {
        if(game.mob_actions[a].type == MOB_ACTION_UNKNOWN) {
            action = &(game.mob_actions[a]);
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
    for(size_t a = 0; a < game.mob_actions.size(); a++) {
        if(game.mob_actions[a].type == MOB_ACTION_UNKNOWN) continue;
        if(game.mob_actions[a].name == name) {
            action = &(game.mob_actions[a]);
        }
    }
    
    if(!action) {
        game.errors.report("Unknown script action name \"" + name + "\"!", dn);
        return false;
    }
    
    //Check if there are too many or too few arguments.
    size_t mandatory_parameters = action->parameters.size();
    
    if(mandatory_parameters > 0) {
        if(action->parameters[mandatory_parameters - 1].is_extras) {
            mandatory_parameters--;
        }
    }
    
    if(words.size() < mandatory_parameters) {
        game.errors.report(
            "The \"" + action->name + "\" action needs " +
            i2s(mandatory_parameters) + " arguments, but this call only "
            "has " + i2s(words.size()) + "! You're missing the \"" +
            action->parameters[words.size()].name + "\" parameter.",
            dn
        );
        return false;
    }
    
    if(mandatory_parameters == action->parameters.size()) {
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
        size_t param_idx = std::min(w, action->parameters.size() - 1);
        bool is_var = (words[w][0] == '$' && words[w].size() > 1);
        
        if(is_var && words[w].size() >= 2 && words[w][1] == '$') {
            //Two '$' in a row means it's meant to use a literal '$'.
            is_var = false;
            words[w].erase(words[w].begin());
        }
        
        if(is_var) {
            if(action->parameters[param_idx].force_const) {
                game.errors.report(
                    "Argument #" + i2s(w + 1) + " (\"" + words[w] + "\") is a "
                    "variable, but the parameter \"" +
                    action->parameters[param_idx].name + "\" can only be "
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
        arg_is_var.push_back(is_var);
    }
    
    //If this action needs extra parsing, do it now.
    if(action->extra_load_logic) {
        bool success = action->extra_load_logic(*this);
        if(!custom_error.empty()) {
            game.errors.report(custom_error, dn);
        }
        return success;
    }
    
    return true;
}


/**
 * @brief Runs an action.
 *
 * @param m The mob.
 * @param custom_data_1 Custom argument #1 to pass to the code.
 * @param custom_data_2 Custom argument #2 to pass to the code.
 * @return Evaluation result, used only by the "if" actions.
 */
bool MobActionCall::run(
    Mob* m, void* custom_data_1, void* custom_data_2
) {
    //Custom code (i.e. instead of text-based script, use actual C++ code).
    if(code) {
        code(m, custom_data_1, custom_data_2);
        return false;
    }
    
    MobActionRunData data(m, this);
    
    //Fill the arguments. Fetch values from variables if needed.
    data.args = args;
    for(size_t a = 0; a < args.size(); a++) {
        if(arg_is_var[a]) {
            data.args[a] = m->vars[args[a]];
        }
    }
    data.custom_data_1 = custom_data_1;
    data.custom_data_2 = custom_data_2;
    
    action->code(data);
    return data.return_value;
}



/**
 * @brief Loading code for the arachnorb logic plan mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::arachnorbPlanLogic(MobActionCall &call) {
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
bool mob_action_loaders::calculate(MobActionCall &call) {
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
bool mob_action_loaders::focus(MobActionCall &call) {
    return loadMobTargetType(call, 0);
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::getAreaInfo(MobActionCall &call) {
    if(call.args[1] == "day_minutes") {
        call.args[1] = i2s(MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES);
    } else if(call.args[1] == "field_pikmin") {
        call.args[1] = i2s(MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN);
    } else {
        call.custom_error =
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
bool mob_action_loaders::getEventInfo(MobActionCall &call) {
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
        call.custom_error =
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
bool mob_action_loaders::getMobInfo(MobActionCall &call) {

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
        call.custom_error =
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
bool mob_action_loaders::holdFocus(MobActionCall &call) {
    size_t p_idx = call.mt->anim_db->findBodyPart(call.args[0]);
    if(p_idx == INVALID) {
        call.custom_error =
            "Unknown body part \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(p_idx);
    return true;
}


/**
 * @brief Loading code for the "if" mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::ifFunction(MobActionCall &call) {
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
 * @param arg_idx Index number of the mob target type argument.
 */
bool mob_action_loaders::loadMobTargetType(
    MobActionCall &call, size_t arg_idx
) {
    if(call.args[arg_idx] == "self") {
        call.args[arg_idx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_SELF);
    } else if(call.args[arg_idx] == "focus") {
        call.args[arg_idx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_FOCUS);
    } else if(call.args[arg_idx] == "trigger") {
        call.args[arg_idx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_TRIGGER);
    } else if(call.args[arg_idx] == "link") {
        call.args[arg_idx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_LINK);
    } else if(call.args[arg_idx] == "parent") {
        call.args[arg_idx] = i2s(MOB_ACTION_MOB_TARGET_TYPE_PARENT);
    } else {
        reportEnumError(call, arg_idx);
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
bool mob_action_loaders::moveToTarget(MobActionCall &call) {
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
bool mob_action_loaders::playSound(MobActionCall &call) {
    for(size_t s = 0; s < call.mt->sounds.size(); s++) {
        if(call.mt->sounds[s].name == call.args[0]) {
            call.args[0] = i2s(s);
            return true;
        }
    }
    call.custom_error =
        "Unknown sound info block \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the status reception mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::receiveStatus(MobActionCall &call) {
    if(game.content.status_types.list.find(call.args[0]) == game.content.status_types.list.end()) {
        call.custom_error =
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
bool mob_action_loaders::removeStatus(MobActionCall &call) {
    if(game.content.status_types.list.find(call.args[0]) == game.content.status_types.list.end()) {
        call.custom_error =
            "Unknown status effect \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/**
 * @brief Reports an error of an unknown enum value.
 *
 * @param call Mob action call that called this.
 * @param arg_idx Index number of the argument that is an enum.
 */
void mob_action_loaders::reportEnumError(
    MobActionCall &call, size_t arg_idx
) {
    size_t param_idx = std::min(arg_idx, call.action->parameters.size() - 1);
    call.custom_error =
        "The parameter \"" + call.action->parameters[param_idx].name + "\" "
        "does not know what the value \"" +
        call.args[arg_idx] + "\" means!";
}


/**
 * @brief Loading code for the animation setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::setAnimation(MobActionCall &call) {
    size_t a_pos = call.mt->anim_db->findAnimation(call.args[0]);
    if(a_pos == INVALID) {
        call.custom_error =
            "Unknown animation \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(a_pos);
    
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
bool mob_action_loaders::setFarReach(MobActionCall &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); r++) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the holdable setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::setHoldable(MobActionCall &call) {
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
bool mob_action_loaders::setNearReach(MobActionCall &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); r++) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the team setting mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::setTeam(MobActionCall &call) {
    size_t team_nr = stringToTeamNr(call.args[0]);
    if(team_nr == INVALID) {
        reportEnumError(call, 0);
        return false;
    }
    call.args[0] = i2s(team_nr);
    return true;
}


/**
 * @brief Loading code for the spawning mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::spawn(MobActionCall &call) {
    for(size_t s = 0; s < call.mt->spawns.size(); s++) {
        if(call.mt->spawns[s].name == call.args[0]) {
            call.args[0] = i2s(s);
            return true;
        }
    }
    call.custom_error =
        "Unknown spawn info block \"" + call.args[0] + "\"!";
    return false;
}


/**
 * @brief Loading code for the z stabilization mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::stabilizeZ(MobActionCall &call) {
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
bool mob_action_loaders::startChomping(MobActionCall &call) {
    for(size_t s = 1; s < call.args.size(); s++) {
        size_t p_nr = call.mt->anim_db->findBodyPart(call.args[s]);
        if(p_nr == INVALID) {
            call.custom_error =
                "Unknown body part \"" + call.args[s] + "\"!";
            return false;
        }
        call.args[s] = i2s(p_nr);
    }
    return true;
}


/**
 * @brief Loading code for the particle start mob script action.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::startParticles(MobActionCall &call) {
    if(
        game.content.particle_gen.list.find(call.args[0]) ==
        game.content.particle_gen.list.end()
    ) {
        call.custom_error =
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
bool mob_action_loaders::turnToTarget(MobActionCall &call) {
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
 * @param force_const If true, this must be a constant value.
 * If false, it can also be a var.
 * @param is_extras If true, this is an array of them (minimum amount 0).
 */
MobActionParam::MobActionParam(
    const string &name,
    const MOB_ACTION_PARAM type,
    bool force_const,
    bool is_extras
):
    name(name),
    type(type),
    force_const(force_const),
    is_extras(is_extras) {
    
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
void mob_action_runners::addHealth(MobActionRunData &data) {
    data.m->setHealth(true, false, s2f(data.args[0]));
}


/**
 * @brief Code for the arachnorb logic plan mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::arachnorbPlanLogic(MobActionRunData &data) {
    data.m->arachnorbPlanLogic(
        (MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE) s2i(data.args[0])
    );
}


/**
 * @brief Code for the calculation mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::calculate(MobActionRunData &data) {
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
void mob_action_runners::deleteFunction(MobActionRunData &data) {
    data.m->to_delete = true;
}


/**
 * @brief Code for the liquid draining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::drainLiquid(MobActionRunData &data) {
    Sector* s_ptr = getSector(data.m->pos, nullptr, true);
    if(!s_ptr) return;
    
    vector<Sector*> sectors_to_drain;
    
    s_ptr->getNeighborSectorsConditionally(
    [] (Sector * s) -> bool {
        for(size_t h = 0; h < s->hazards.size(); h++) {
            if(s->hazards[h]->associated_liquid) {
                return true;
            }
        }
        return false;
    },
    sectors_to_drain
    );
    
    for(size_t s = 0; s < sectors_to_drain.size(); s++) {
        sectors_to_drain[s]->draining_liquid = true;
        sectors_to_drain[s]->liquid_drain_left =
            GEOMETRY::LIQUID_DRAIN_DURATION;
    }
}


/**
 * @brief Code for the death finish mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::finishDying(MobActionRunData &data) {
    data.m->finishDying();
}


/**
 * @brief Code for the focus mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::focus(MobActionRunData &data) {

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
void mob_action_runners::followPathRandomly(MobActionRunData &data) {
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
            game.cur_area_data->path_stops.begin(),
            game.cur_area_data->path_stops.end()
        );
    } else {
        //If there's a label, we should only pick stops that have the label.
        for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
            PathStop* s_ptr = game.cur_area_data->path_stops[s];
            if(s_ptr->label == label) {
                choices.push_back(s_ptr);
            }
        }
    }
    
    //Pick a stop from the choices at random, but make sure we don't
    //pick a stop that the mob is practically on already.
    PathStop* final_stop = nullptr;
    if(!choices.empty()) {
        size_t tries = 0;
        while(!final_stop && tries < 5) {
            size_t c = game.rng.i(0, (int) choices.size() - 1);
            if(
                Distance(choices[c]->pos, data.m->pos) >
                PATHS::DEF_CHASE_TARGET_DISTANCE
            ) {
                final_stop = choices[c];
                break;
            }
            tries++;
        }
    }
    
    //Go! Though if something went wrong, make it follow a path to nowhere,
    //so it can emit the MOB_EV_REACHED_DESTINATION event, and hopefully
    //make it clear that there was an error.
    PathFollowSettings settings;
    settings.target_point = final_stop ? final_stop->pos : data.m->pos;
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
void mob_action_runners::followPathToAbsolute(MobActionRunData &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    
    PathFollowSettings settings;
    settings.target_point = Point(x, y);
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
void mob_action_runners::getAngle(MobActionRunData &data) {
    float center_x = s2f(data.args[1]);
    float center_y = s2f(data.args[2]);
    float focus_x = s2f(data.args[3]);
    float focus_y = s2f(data.args[4]);
    float angle = getAngle(Point(center_x, center_y), Point(focus_x, focus_y));
    angle = radToDeg(angle);
    data.m->vars[data.args[0]] = f2s(angle);
}



/**
 * @brief Code for the area info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getAreaInfo(MobActionRunData &data) {
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_AREA_INFO_TYPE t =
        (MOB_ACTION_GET_AREA_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES: {
        *var = i2s(game.states.gameplay->day_minutes);
        break;
        
    } case MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN: {
        *var = i2s(game.states.gameplay->mobs.pikmin_list.size());
        break;
        
    }
    }
}


/**
 * @brief Code for the getting chomped mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getChomped(MobActionRunData &data) {
    if(data.call->parent_event == MOB_EV_HITBOX_TOUCH_EAT) {
        ((Mob*) (data.custom_data_1))->chomp(
            data.m,
            (Hitbox*) (data.custom_data_2)
        );
    }
}


/**
 * @brief Code for the coordinate from angle obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getCoordinatesFromAngle(MobActionRunData &data) {
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
void mob_action_runners::getDistance(MobActionRunData &data) {
    float center_x = s2f(data.args[1]);
    float center_y = s2f(data.args[2]);
    float focus_x = s2f(data.args[3]);
    float focus_y = s2f(data.args[4]);
    data.m->vars[data.args[0]] =
        f2s(
            Distance(Point(center_x, center_y), Point(focus_x, focus_y)).toFloat()
        );
}


/**
 * @brief Code for the event info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getEventInfo(MobActionRunData &data) {
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_EV_INFO_TYPE t =
        (MOB_ACTION_GET_EV_INFO_TYPE) s2i(data.args[1]);
        
    switch (t) {
    case MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART: {
        if(
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_N ||
            data.call->parent_event == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.custom_data_1)
                )->h1->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                data.m->getClosestHitbox(
                    ((Mob*)(data.custom_data_1))->pos
                )->body_part_name;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL: {
        if(data.call->parent_event == MOB_EV_FRAME_SIGNAL) {
            *var = i2s(*((size_t*)(data.custom_data_1)));
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_HAZARD: {
        if(
            data.call->parent_event == MOB_EV_TOUCHED_HAZARD ||
            data.call->parent_event == MOB_EV_LEFT_HAZARD
        ) {
            *var = ((Hazard*)data.custom_data_1)->manifest->internal_name;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME: {
        if(data.call->parent_event == MOB_EV_INPUT_RECEIVED) {
            *var =
                game.controls.getPlayerActionTypeInternalName(
                    ((PlayerAction*) (data.custom_data_1))->actionTypeId
                );
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.call->parent_event == MOB_EV_INPUT_RECEIVED) {
            *var = f2s(((PlayerAction*) (data.custom_data_1))->value);
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE: {
        if(data.call->parent_event == MOB_EV_RECEIVE_MESSAGE) {
            *var = *((string*)(data.custom_data_1));
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART: {
        if(
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_N ||
            data.call->parent_event == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (HitboxInteraction*)(data.custom_data_1)
                )->h2->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                ((Mob*)(data.custom_data_1))->getClosestHitbox(
                    data.m->pos
                )->body_part_name;
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
void mob_action_runners::getFloorZ(MobActionRunData &data) {
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
void mob_action_runners::getFocusVar(MobActionRunData &data) {
    if(!data.m->focused_mob) return;
    data.m->vars[data.args[0]] =
        data.m->focused_mob->vars[data.args[1]];
}


/**
 * @brief Code for the mob info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getMobInfo(MobActionRunData &data) {
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
        *var = i2s(target->chomping_mobs.size());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->focused_mob) {
            float d =
                Distance(target->pos, target->focused_mob->pos).toFloat();
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
        if(target->max_health != 0.0f) {
            *var = f2s(target->health / target->max_health);
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
        *var = target->type->category->internal_name;
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE: {
        *var = target->type->manifest->internal_name;
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_STATE: {
        *var = target->fsm.cur_state->name;
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT: {
        if(target->type->category->id == MOB_CATEGORY_SCALES) {
            Scale* s_ptr = (Scale*)(target);
            *var = i2s(s_ptr->calculateCurWeight());
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
void mob_action_runners::getRandomFloat(MobActionRunData &data) {
    data.m->vars[data.args[0]] =
        f2s(game.rng.f(s2f(data.args[1]), s2f(data.args[2])));
}


/**
 * @brief Code for the integer number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::getRandomInt(MobActionRunData &data) {
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
        return data.m->focused_mob;
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
 * @brief Code for the hold focused mob mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::holdFocus(MobActionRunData &data) {
    if(data.m->focused_mob) {
        data.m->hold(
            data.m->focused_mob,
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
void mob_action_runners::ifFunction(MobActionRunData &data) {
    string lhs = data.args[0];
    MOB_ACTION_IF_OP op =
        (MOB_ACTION_IF_OP) s2i(data.args[1]);
    string rhs = vectorTailToString(data.args, 2);
    
    switch(op) {
    case MOB_ACTION_IF_OP_EQUAL: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.return_value = (s2f(lhs) == s2f(rhs));
        } else {
            data.return_value = (lhs == rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_NOT: {
        if(isNumber(lhs) && isNumber(rhs)) {
            data.return_value = (s2f(lhs) != s2f(rhs));
        } else {
            data.return_value = (lhs != rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_LESS: {
        data.return_value = (s2f(lhs) < s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_MORE: {
        data.return_value = (s2f(lhs) > s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_LESS_E: {
        data.return_value = (s2f(lhs) <= s2f(rhs));
        break;
        
    } case MOB_ACTION_IF_OP_MORE_E: {
        data.return_value = (s2f(lhs) >= s2f(rhs));
        break;
        
    }
    }
}


/**
 * @brief Code for the link with focus mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::linkWithFocus(MobActionRunData &data) {
    if(!data.m->focused_mob) {
        return;
    }
    
    for(size_t l = 0; l < data.m->links.size(); l++) {
        if(data.m->links[l] == data.m->focused_mob) {
            //Already linked.
            return;
        }
    }
    
    data.m->links.push_back(data.m->focused_mob);
}


/**
 * @brief Code for the load focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::loadFocusMemory(MobActionRunData &data) {
    if(data.m->focused_mob_memory.empty()) {
        return;
    }
    
    data.m->focusOnMob(data.m->focused_mob_memory[s2i(data.args[0])]);
}


/**
 * @brief Code for the move to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::moveToAbsolute(MobActionRunData &data) {
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
void mob_action_runners::moveToRelative(MobActionRunData &data) {
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
void mob_action_runners::moveToTarget(MobActionRunData &data) {
    MOB_ACTION_MOVE_TYPE t = (MOB_ACTION_MOVE_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.m->focused_mob) {
            float a = getAngle(data.m->pos, data.m->focused_mob->pos);
            Point offset = Point(2000, 0);
            offset = rotatePoint(offset, a + TAU / 2.0);
            data.m->chase(data.m->pos + offset, data.m->z);
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS: {
        if(data.m->focused_mob) {
            data.m->chase(&data.m->focused_mob->pos, &data.m->focused_mob->z);
        } else {
            data.m->stopChasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.m->focused_mob) {
            data.m->chase(data.m->focused_mob->pos, data.m->focused_mob->z);
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
        for(size_t l = 0; l < data.m->links.size(); l++) {
            if(!data.m->links[l]) continue;
            des += data.m->links[l]->pos;
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
void mob_action_runners::orderRelease(MobActionRunData &data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.runEvent(MOB_EV_RELEASE_ORDER, nullptr, nullptr);
    }
}


/**
 * @brief Code for the sound playing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::playSound(MobActionRunData &data) {
    size_t sound_id = data.m->playSound(s2i(data.args[0]));
    if(data.args.size() >= 2) {
        data.m->setVar(data.args[1], i2s(sound_id));
    }
}


/**
 * @brief Code for the text printing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::print(MobActionRunData &data) {
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
void mob_action_runners::receiveStatus(MobActionRunData &data) {
    data.m->applyStatusEffect(game.content.status_types.list[data.args[0]], false, false);
}


/**
 * @brief Code for the release mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::release(MobActionRunData &data) {
    data.m->releaseChompedPikmin();
}


/**
 * @brief Code for the release stored mobs mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::releaseStoredMobs(MobActionRunData &data) {
    data.m->releaseStoredMobs();
}


/**
 * @brief Code for the status removal mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::removeStatus(MobActionRunData &data) {
    for(size_t s = 0; s < data.m->statuses.size(); s++) {
        if(data.m->statuses[s].type->manifest->internal_name == data.args[0]) {
            data.m->statuses[s].to_delete = true;
        }
    }
}


/**
 * @brief Code for the save focused mob memory mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::saveFocusMemory(MobActionRunData &data) {
    if(!data.m->focused_mob) {
        return;
    }
    
    data.m->focused_mob_memory[s2i(data.args[0])] = data.m->focused_mob;
}


/**
 * @brief Code for the focused mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::sendMessageToFocus(MobActionRunData &data) {
    if(!data.m->focused_mob) return;
    data.m->sendScriptMessage(data.m->focused_mob, data.args[0]);
}


/**
 * @brief Code for the linked mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::sendMessageToLinks(MobActionRunData &data) {
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
void mob_action_runners::sendMessageToNearby(MobActionRunData &data) {
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
void mob_action_runners::setAnimation(MobActionRunData &data) {
    START_ANIM_OPTION options = START_ANIM_OPTION_NORMAL;
    float mob_speed_baseline = 0.0f;
    if(data.args.size() > 1) {
        options = (START_ANIM_OPTION) s2i(data.args[1]);
    }
    if(data.args.size() > 2) {
        if(s2b(data.args[2])) {
            mob_speed_baseline = data.m->type->move_speed;
        };
    }
    
    data.m->setAnimation(
        s2i(data.args[0]), options, false, mob_speed_baseline
    );
}


/**
 * @brief Code for the block paths setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setCanBlockPaths(MobActionRunData &data) {
    data.m->setCanBlockPaths(s2b(data.args[0]));
}


/**
 * @brief Code for the far reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setFarReach(MobActionRunData &data) {
    data.m->far_reach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the flying setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setFlying(MobActionRunData &data) {
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
void mob_action_runners::setGravity(MobActionRunData &data) {
    data.m->gravity_mult = s2f(data.args[0]);
}


/**
 * @brief Code for the health setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setHealth(MobActionRunData &data) {
    data.m->setHealth(false, false, s2f(data.args[0]));
}


/**
 * @brief Code for the height setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setHeight(MobActionRunData &data) {
    data.m->height = s2f(data.args[0]);
    
    if(data.m->type->walkable) {
        //Update the Z of mobs standing on top of it.
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* m2_ptr = game.states.gameplay->mobs.all[m];
            if(m2_ptr->standing_on_mob == data.m) {
                m2_ptr->z = data.m->z + data.m->height;
            }
        }
    }
}


/**
 * @brief Code for the hiding setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setHiding(MobActionRunData &data) {
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
void mob_action_runners::setHoldable(MobActionRunData &data) {
    if(typeid(*(data.m)) == typeid(Tool)) {
        unsigned char flags = 0;
        for(size_t i = 0; i < data.args.size(); i++) {
            flags |= s2i(data.args[i]);
        }
        ((Tool*) (data.m))->holdability_flags = flags;
    }
}


/**
 * @brief Code for the huntable setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setHuntable(MobActionRunData &data) {
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
void mob_action_runners::setLimbAnimation(MobActionRunData &data) {
    if(!data.m->parent) {
        return;
    }
    if(!data.m->parent->limb_anim.anim_db) {
        return;
    }
    
    size_t a = data.m->parent->limb_anim.anim_db->findAnimation(data.args[0]);
    if(a == INVALID) {
        return;
    }
    
    data.m->parent->limb_anim.cur_anim =
        data.m->parent->limb_anim.anim_db->animations[a];
    data.m->parent->limb_anim.toStart();
    
}


/**
 * @brief Code for the near reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setNearReach(MobActionRunData &data) {
    data.m->near_reach = s2i(data.args[0]);
    data.m->updateInteractionSpan();
}


/**
 * @brief Code for the radius setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setRadius(MobActionRunData &data) {
    data.m->setRadius(s2f(data.args[0]));
}


/**
 * @brief Code for the sector scroll setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setSectorScroll(MobActionRunData &data) {
    Sector* s_ptr = getSector(data.m->pos, nullptr, true);
    if(!s_ptr) return;
    
    s_ptr->scroll.x = s2f(data.args[0]);
    s_ptr->scroll.y = s2f(data.args[1]);
}


/**
 * @brief Code for the shadow visibility setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setShadowVisibility(MobActionRunData &data) {
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
void mob_action_runners::setState(MobActionRunData &data) {
    data.m->fsm.setState(
        s2i(data.args[0]),
        data.custom_data_1,
        data.custom_data_2
    );
}


/**
 * @brief Code for the tangible setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setTangible(MobActionRunData &data) {
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
void mob_action_runners::setTeam(MobActionRunData &data) {
    data.m->team = (MOB_TEAM) s2i(data.args[0]);
}


/**
 * @brief Code for the timer setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setTimer(MobActionRunData &data) {
    data.m->setTimer(s2f(data.args[0]));
}


/**
 * @brief Code for the var setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::setVar(MobActionRunData &data) {
    data.m->setVar(data.args[0], data.args[1]);
}


/**
 * @brief Code for the show message from var mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::showMessageFromVar(MobActionRunData &data) {
    startGameplayMessage(data.m->vars[data.args[0]], nullptr);
}


/**
 * @brief Code for the spawning mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::spawn(MobActionRunData &data) {
    data.m->spawn(&data.m->type->spawns[s2i(data.args[0])]);
}


/**
 * @brief Code for the z stabilization mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stabilizeZ(MobActionRunData &data) {
    if(data.m->links.empty() || !data.m->links[0]) {
        return;
    }
    
    float best_match_z = data.m->links[0]->z;
    MOB_ACTION_STABILIZE_Z_TYPE t =
        (MOB_ACTION_STABILIZE_Z_TYPE) s2i(data.args[0]);
        
    for(size_t l = 1; l < data.m->links.size(); l++) {
    
        if(!data.m->links[l]) continue;
        
        switch(t) {
        case MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST: {
            if(data.m->links[l]->z > best_match_z) {
                best_match_z = data.m->links[l]->z;
            }
            break;
            
        } case MOB_ACTION_STABILIZE_Z_TYPE_LOWEST: {
            if(data.m->links[l]->z < best_match_z) {
                best_match_z = data.m->links[l]->z;
            }
            break;
            
        }
        }
        
    }
    
    data.m->z = best_match_z + s2f(data.args[1]);
}


/**
 * @brief Code for the chomping start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::startChomping(MobActionRunData &data) {
    data.m->chomp_max = s2i(data.args[0]);
    data.m->chomp_body_parts.clear();
    for(size_t p = 1; p < data.args.size(); p++) {
        data.m->chomp_body_parts.push_back(s2i(data.args[p]));
    }
}


/**
 * @brief Code for the dying start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::startDying(MobActionRunData &data) {
    data.m->startDying();
}


/**
 * @brief Code for the height effect start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::startHeightEffect(MobActionRunData &data) {
    data.m->startHeightEffect();
}


/**
 * @brief Code for the particle start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::startParticles(MobActionRunData &data) {
    float offset_x = 0;
    float offset_y = 0;
    float offset_z = 0;
    if(data.args.size() > 1) offset_x = s2f(data.args[1]);
    if(data.args.size() > 2) offset_y = s2f(data.args[2]);
    if(data.args.size() > 3) offset_z = s2f(data.args[3]);
    
    ParticleGenerator pg =
        standardParticleGenSetup(data.args[0], data.m);
    pg.follow_pos_offset = Point(offset_x, offset_y);
    pg.follow_z_offset = offset_z;
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    data.m->particle_generators.push_back(pg);
}


/**
 * @brief Code for the stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop(MobActionRunData &data) {
    data.m->stopChasing();
    data.m->stopTurning();
    data.m->stopFollowingPath();
}


/**
 * @brief Code for the chomp stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stopChomping(MobActionRunData &data) {
    data.m->chomp_max = 0;
    data.m->chomp_body_parts.clear();
}


/**
 * @brief Code for the height effect stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stopHeightEffect(MobActionRunData &data) {
    data.m->stopHeightEffect();
}


/**
 * @brief Code for the particle stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stopParticles(MobActionRunData &data) {
    data.m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_SCRIPT);
}


/**
 * @brief Code for the sound stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stopSound(MobActionRunData &data) {
    game.audio.destroySoundSource(s2i(data.args[0]));
}


/**
 * @brief Code for the vertical stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stopVertically(MobActionRunData &data) {
    data.m->speed_z = 0;
}


/**
 * @brief Code for the focus storing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::storeFocusInside(MobActionRunData &data) {
    if(data.m->focused_mob && !data.m->focused_mob->isStoredInsideMob()) {
        data.m->storeMobInside(data.m->focused_mob);
    }
}


/**
 * @brief Code for the swallow mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::swallow(MobActionRunData &data) {
    data.m->swallowChompedPikmin(s2i(data.args[0]));
}


/**
 * @brief Code for the swallow all mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::swallowAll(MobActionRunData &data) {
    data.m->swallowChompedPikmin(data.m->chomping_mobs.size());
}


/**
 * @brief Code for the teleport to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::teleportToAbsolute(MobActionRunData &data) {
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
void mob_action_runners::teleportToRelative(MobActionRunData &data) {
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
void mob_action_runners::throwFocus(MobActionRunData &data) {
    if(!data.m->focused_mob) {
        return;
    }
    
    if(data.m->focused_mob->holder.m == data.m) {
        data.m->release(data.m->focused_mob);
    }
    
    float max_height = s2f(data.args[3]);
    
    if(max_height == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.m->startHeightEffect();
    calculateThrow(
        data.m->focused_mob->pos, data.m->focused_mob->z,
        Point(s2f(data.args[0]), s2f(data.args[1])), s2f(data.args[2]),
        max_height, MOB::GRAVITY_ADDER,
        &data.m->focused_mob->speed,
        &data.m->focused_mob->speed_z,
        nullptr
    );
}


/**
 * @brief Code for the turn to an absolute angle mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::turnToAbsolute(MobActionRunData &data) {
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
void mob_action_runners::turnToRelative(MobActionRunData &data) {
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
void mob_action_runners::turnToTarget(MobActionRunData &data) {
    MOB_ACTION_TURN_TYPE t = (MOB_ACTION_TURN_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC: {
        data.m->arachnorbHeadTurnLogic();
        break;
        
    } case MOB_ACTION_TURN_TYPE_FOCUSED_MOB: {
        if(data.m->focused_mob) {
            data.m->face(0, &data.m->focused_mob->pos);
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
    int if_level = 0;
    for(size_t a = 0; a < actions.size(); a++) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            if_level++;
            break;
        } case MOB_ACTION_ELSE: {
            if(if_level == 0) {
                game.errors.report(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            break;
        } case MOB_ACTION_END_IF: {
            if(if_level == 0) {
                game.errors.report(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            if_level--;
            break;
        } default: {
            break;
        }
        }
    }
    if(if_level > 0) {
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
            if(labels.find(name) != labels.end()) {
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
            if(labels.find(name) == labels.end()) {
                game.errors.report(
                    "There is no label called \"" + name + "\", even though "
                    "there are \"goto\" actions that need it!", dn
                );
                return false;
            }
        }
    }
    
    //Check if there are actions after a "set_state" action.
    bool passed_set_state = false;
    for(size_t a = 0; a < actions.size(); a++) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_SET_STATE: {
            passed_set_state = true;
            break;
        } case MOB_ACTION_ELSE: {
            passed_set_state = false;
            break;
        } case MOB_ACTION_END_IF: {
            passed_set_state = false;
            break;
        } case MOB_ACTION_LABEL: {
            passed_set_state = false;
            break;
        } default: {
            if(passed_set_state) {
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
        data.call->parent_event == MOB_EV_OBJECT_IN_REACH ||
        data.call->parent_event == MOB_EV_OPPONENT_IN_REACH ||
        data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED ||
        data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
        data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
        data.call->parent_event == MOB_EV_HELD ||
        data.call->parent_event == MOB_EV_RELEASED ||
        data.call->parent_event == MOB_EV_SWALLOWED ||
        data.call->parent_event == MOB_EV_STARTED_RECEIVING_DELIVERY ||
        data.call->parent_event == MOB_EV_FINISHED_RECEIVING_DELIVERY
    ) {
        return (Mob*)(data.custom_data_1);
        
    } else if(
        data.call->parent_event == MOB_EV_RECEIVE_MESSAGE
    ) {
        return(Mob*)(data.custom_data_2);
        
    } else if(
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_N ||
        data.call->parent_event == MOB_EV_DAMAGE
    ) {
        return ((HitboxInteraction*)(data.custom_data_1))->mob2;
        
    }
    
    return nullptr;
}


/**
 * @brief Add a vector of actions onto a given event.
 *
 * @param ev The event to add actions to.
 * @param actions Vector of actions to insert.
 * @param at_end Are the actions inserted at the end?
 */
void insertEventActions(
    MobEvent* ev, const vector<MobActionCall*> &actions, bool at_end
) {
    vector<MobActionCall*>::iterator it =
        at_end ? ev->actions.end() : ev->actions.begin();
    ev->actions.insert(it, actions.begin(), actions.end());
}


/**
 * @brief Loads actions from a data node.
 *
 * @param mt The type of mob the events are going to.
 * @param node The data node.
 * @param out_actions The oaded actions are returned here.
 * @param out_settings If not nullptr, the settings for how to load the
 * events are returned here.
 */
void loadActions(
    MobType* mt, DataNode* node,
    vector<MobActionCall*>* out_actions, bitmask_8_t* out_settings
) {
    if(out_settings) *out_settings = 0;
    for(size_t a = 0; a < node->getNrOfChildren(); a++) {
        DataNode* action_node = node->getChild(a);
        if(
            out_settings && action_node->name == "custom_actions_after"
        ) {
            enableFlag(*out_settings, EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER);
        } else if(
            out_settings && action_node->name == "global_actions_after"
        ) {
            enableFlag(*out_settings, EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER);
        } else {
            MobActionCall* new_a = new MobActionCall();
            if(new_a->loadFromDataNode(action_node, mt)) {
                out_actions->push_back(new_a);
            } else {
                delete new_a;
            }
        }
    }
    assertActions(*out_actions, node);
}
