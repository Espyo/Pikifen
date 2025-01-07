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

#include "functions.h"
#include "game.h"
#include "mobs/group_task.h"
#include "mobs/scale.h"
#include "mobs/tool.h"
#include "utils/general_utils.h"
#include "utils/string_utils.h"


using std::set;


/**
 * @brief Constructs a new mob action call object of a certain type.
 *
 * @param type Type of mob action call.
 */
mob_action_call::mob_action_call(MOB_ACTION type) {

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
mob_action_call::mob_action_call(custom_action_code_t code) :
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
bool mob_action_call::load_from_data_node(data_node* dn, mob_type* mt) {

    action = nullptr;
    this->mt = mt;
    
    //First, get the name and arguments.
    vector<string> words = split(dn->name);
    
    for(size_t w = 0; w < words.size(); w++) {
        words[w] = trim_spaces(words[w]);
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
bool mob_action_call::run(
    mob* m, void* custom_data_1, void* custom_data_2
) {
    //Custom code (i.e. instead of text-based script, use actual C++ code).
    if(code) {
        code(m, custom_data_1, custom_data_2);
        return false;
    }
    
    mob_action_run_data data(m, this);
    
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
bool mob_action_loaders::arachnorb_plan_logic(mob_action_call &call) {
    if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME);
    } else if(call.args[0] == "forward") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD);
    } else if(call.args[0] == "cw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN);
    } else if(call.args[0] == "ccw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN);
    } else {
        report_enum_error(call, 0);
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
bool mob_action_loaders::calculate(mob_action_call &call) {
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
        report_enum_error(call, 2);
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
bool mob_action_loaders::focus(mob_action_call &call) {
    return load_mob_target_type(call, 0);
}


/**
 * @brief Loading code for the info getting script actions.
 *
 * @param call Mob action call that called this.
 * @return Whether it succeeded.
 */
bool mob_action_loaders::get_area_info(mob_action_call &call) {
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
bool mob_action_loaders::get_event_info(mob_action_call &call) {
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
bool mob_action_loaders::get_mob_info(mob_action_call &call) {

    if(!load_mob_target_type(call, 1)) {
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
bool mob_action_loaders::hold_focus(mob_action_call &call) {
    size_t p_idx = call.mt->anim_db->find_body_part(call.args[0]);
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
bool mob_action_loaders::if_function(mob_action_call &call) {
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
        report_enum_error(call, 1);
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
bool mob_action_loaders::load_mob_target_type(
    mob_action_call &call, size_t arg_idx
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
        report_enum_error(call, arg_idx);
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
bool mob_action_loaders::move_to_target(mob_action_call &call) {
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
        report_enum_error(call, 0);
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
bool mob_action_loaders::play_sound(mob_action_call &call) {
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
bool mob_action_loaders::receive_status(mob_action_call &call) {
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
bool mob_action_loaders::remove_status(mob_action_call &call) {
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
void mob_action_loaders::report_enum_error(
    mob_action_call &call, size_t arg_idx
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
bool mob_action_loaders::set_animation(mob_action_call &call) {
    size_t a_pos = call.mt->anim_db->find_animation(call.args[0]);
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
bool mob_action_loaders::set_far_reach(mob_action_call &call) {
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
bool mob_action_loaders::set_holdable(mob_action_call &call) {
    for(size_t a = 0; a < call.args.size(); a++) {
        if(call.args[a] == "pikmin") {
            call.args[a] = i2s(HOLDABILITY_FLAG_PIKMIN);
        } else if(call.args[a] == "enemies") {
            call.args[a] = i2s(HOLDABILITY_FLAG_ENEMIES);
        } else {
            report_enum_error(call, a);
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
bool mob_action_loaders::set_near_reach(mob_action_call &call) {
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
bool mob_action_loaders::set_team(mob_action_call &call) {
    size_t team_nr = string_to_team_nr(call.args[0]);
    if(team_nr == INVALID) {
        report_enum_error(call, 0);
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
bool mob_action_loaders::spawn(mob_action_call &call) {
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
bool mob_action_loaders::stabilize_z(mob_action_call &call) {
    if(call.args[0] == "lowest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_TYPE_LOWEST);
    } else if(call.args[0] == "highest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST);
    } else {
        report_enum_error(call, 0);
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
bool mob_action_loaders::start_chomping(mob_action_call &call) {
    for(size_t s = 1; s < call.args.size(); s++) {
        size_t p_nr = call.mt->anim_db->find_body_part(call.args[s]);
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
bool mob_action_loaders::start_particles(mob_action_call &call) {
    if(
        game.content.custom_particle_gen.list.find(call.args[0]) ==
        game.content.custom_particle_gen.list.end()
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
bool mob_action_loaders::turn_to_target(mob_action_call &call) {
    if(call.args[0] == "arachnorb_head_logic") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC);
    } else if(call.args[0] == "focused_mob") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_FOCUSED_MOB);
    } else if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_TURN_TYPE_HOME);
    } else {
        report_enum_error(call, 0);
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
mob_action_param::mob_action_param(
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
mob_action_run_data::mob_action_run_data(mob* m, mob_action_call* call) :
    m(m),
    call(call) {
    
}


/**
 * @brief Code for the health addition mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::add_health(mob_action_run_data &data) {
    data.m->set_health(true, false, s2f(data.args[0]));
}


/**
 * @brief Code for the arachnorb logic plan mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::arachnorb_plan_logic(mob_action_run_data &data) {
    data.m->arachnorb_plan_logic(
        (MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE) s2i(data.args[0])
    );
}


/**
 * @brief Code for the calculation mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::calculate(mob_action_run_data &data) {
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
void mob_action_runners::delete_function(mob_action_run_data &data) {
    data.m->to_delete = true;
}


/**
 * @brief Code for the liquid draining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::drain_liquid(mob_action_run_data &data) {
    sector* s_ptr = get_sector(data.m->pos, nullptr, true);
    if(!s_ptr) return;
    
    vector<sector*> sectors_to_drain;
    
    s_ptr->get_neighbor_sectors_conditionally(
    [] (sector * s) -> bool {
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
void mob_action_runners::finish_dying(mob_action_run_data &data) {
    data.m->finish_dying();
}


/**
 * @brief Code for the focus mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::focus(mob_action_run_data &data) {

    MOB_ACTION_MOB_TARGET_TYPE s = (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[0]);
    mob* target = get_target_mob(data, s);
    
    if(!target) return;
    
    data.m->focus_on_mob(target);
}


/**
 * @brief Code for the follow path randomly mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::follow_path_randomly(mob_action_run_data &data) {
    string label;
    if(data.args.size() >= 1) {
        label = data.args[0];
    }
    
    //We need to decide what the final stop is going to be.
    //First, get all eligible stops.
    vector<path_stop*> choices;
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
            path_stop* s_ptr = game.cur_area_data->path_stops[s];
            if(s_ptr->label == label) {
                choices.push_back(s_ptr);
            }
        }
    }
    
    //Pick a stop from the choices at random, but make sure we don't
    //pick a stop that the mob is practically on already.
    path_stop* final_stop = nullptr;
    if(!choices.empty()) {
        size_t tries = 0;
        while(!final_stop && tries < 5) {
            size_t c = randomi(0, (int) choices.size() - 1);
            if(
                dist(choices[c]->pos, data.m->pos) >
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
    path_follow_settings settings;
    settings.target_point = final_stop ? final_stop->pos : data.m->pos;
    enable_flag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enable_flag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    settings.label = label;
    data.m->follow_path(
        settings, data.m->get_base_speed(), data.m->type->acceleration
    );
}


/**
 * @brief Code for the follow path to absolute mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::follow_path_to_absolute(mob_action_run_data &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    
    path_follow_settings settings;
    settings.target_point = point(x, y);
    enable_flag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    enable_flag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE);
    if(data.args.size() >= 3) {
        settings.label = data.args[2];
    }
    
    data.m->follow_path(
        settings, data.m->get_base_speed(), data.m->type->acceleration
    );
}


/**
 * @brief Code for the angle obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_angle(mob_action_run_data &data) {
    float center_x = s2f(data.args[1]);
    float center_y = s2f(data.args[2]);
    float focus_x = s2f(data.args[3]);
    float focus_y = s2f(data.args[4]);
    float angle = get_angle(point(center_x, center_y), point(focus_x, focus_y));
    angle = rad_to_deg(angle);
    data.m->vars[data.args[0]] = f2s(angle);
}



/**
 * @brief Code for the area info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_area_info(mob_action_run_data &data) {
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
void mob_action_runners::get_chomped(mob_action_run_data &data) {
    if(data.call->parent_event == MOB_EV_HITBOX_TOUCH_EAT) {
        ((mob*) (data.custom_data_1))->chomp(
            data.m,
            (hitbox*) (data.custom_data_2)
        );
    }
}


/**
 * @brief Code for the coordinate from angle obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_coordinates_from_angle(mob_action_run_data &data) {
    float angle = s2f(data.args[2]);
    angle = deg_to_rad(angle);
    float magnitude = s2f(data.args[3]);
    point p = angle_to_coordinates(angle, magnitude);
    data.m->vars[data.args[0]] = f2s(p.x);
    data.m->vars[data.args[1]] = f2s(p.y);
}


/**
 * @brief Code for the distance obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_distance(mob_action_run_data &data) {
    float center_x = s2f(data.args[1]);
    float center_y = s2f(data.args[2]);
    float focus_x = s2f(data.args[3]);
    float focus_y = s2f(data.args[4]);
    data.m->vars[data.args[0]] =
        f2s(
            dist(point(center_x, center_y), point(focus_x, focus_y)).to_float()
        );
}


/**
 * @brief Code for the event info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_event_info(mob_action_run_data &data) {
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
                    (hitbox_interaction*)(data.custom_data_1)
                )->h1->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                data.m->get_closest_hitbox(
                    ((mob*)(data.custom_data_1))->pos
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
            *var = ((hazard*)data.custom_data_1)->manifest->internal_name;
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME: {
        if(data.call->parent_event == MOB_EV_INPUT_RECEIVED) {
            *var =
                game.controls.get_player_action_type_internal_name(
                    ((player_action*) (data.custom_data_1))->action_type_id
                );
        }
        break;
        
    } case MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE: {
        if(data.call->parent_event == MOB_EV_INPUT_RECEIVED) {
            *var = f2s(((player_action*) (data.custom_data_1))->value);
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
                    (hitbox_interaction*)(data.custom_data_1)
                )->h2->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                ((mob*)(data.custom_data_1))->get_closest_hitbox(
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
void mob_action_runners::get_floor_z(mob_action_run_data &data) {
    float x = s2f(data.args[1]);
    float y = s2f(data.args[2]);
    sector* s = get_sector(point(x, y), nullptr, true);
    data.m->vars[data.args[0]] = f2s(s ? s->z : 0);
}


/**
 * @brief Code for the focused mob var getting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_focus_var(mob_action_run_data &data) {
    if(!data.m->focused_mob) return;
    data.m->vars[data.args[0]] =
        data.m->focused_mob->vars[data.args[1]];
}


/**
 * @brief Code for the mob info obtaining mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_mob_info(mob_action_run_data &data) {
    MOB_ACTION_MOB_TARGET_TYPE s = (MOB_ACTION_MOB_TARGET_TYPE) s2i(data.args[1]);
    mob* target = get_target_mob(data, s);
    
    if(!target) return;
    
    string* var = &(data.m->vars[data.args[0]]);
    MOB_ACTION_GET_MOB_INFO_TYPE t =
        (MOB_ACTION_GET_MOB_INFO_TYPE) s2i(data.args[2]);
        
    switch(t) {
    case MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE: {
        *var = f2s(rad_to_deg(target->angle));
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN: {
        *var = i2s(target->chomping_mobs.size());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE: {
        if(target->focused_mob) {
            float d =
                dist(target->pos, target->focused_mob->pos).to_float();
            *var = f2s(d);
        }
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER: {
        if(target->type->category->id == MOB_CATEGORY_GROUP_TASKS) {
            *var = f2s(((group_task*)target)->get_power());
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
        *var = i2s(target->get_latched_pikmin_amount());
        break;
        
    } case MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT: {
        *var = i2s(target->get_latched_pikmin_weight());
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
            scale* s_ptr = (scale*)(target);
            *var = i2s(s_ptr->calculate_cur_weight());
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
 * @brief Code for the real number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_random_real(mob_action_run_data &data) {
    data.m->vars[data.args[0]] =
        f2s(randomf(s2f(data.args[1]), s2f(data.args[2])));
}


/**
 * @brief Code for the integer number randomization mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::get_random_int(mob_action_run_data &data) {
    data.m->vars[data.args[0]] =
        i2s(randomi(s2i(data.args[1]), s2i(data.args[2])));
}


/**
 * @brief Returns the mob matching the mob target type.
 *
 * @param data Data about the action call.
 * @param type Type of target.
 */
mob* get_target_mob(
    mob_action_run_data &data, MOB_ACTION_MOB_TARGET_TYPE type
) {
    switch (type) {
    case MOB_ACTION_MOB_TARGET_TYPE_SELF: {
        return data.m;
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_FOCUS: {
        return data.m->focused_mob;
        break;
    } case MOB_ACTION_MOB_TARGET_TYPE_TRIGGER: {
        return get_trigger_mob(data);
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
void mob_action_runners::hold_focus(mob_action_run_data &data) {
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
void mob_action_runners::if_function(mob_action_run_data &data) {
    string lhs = data.args[0];
    MOB_ACTION_IF_OP op =
        (MOB_ACTION_IF_OP) s2i(data.args[1]);
    string rhs = vector_tail_to_string(data.args, 2);
    
    switch(op) {
    case MOB_ACTION_IF_OP_EQUAL: {
        if(is_number(lhs) && is_number(rhs)) {
            data.return_value = (s2f(lhs) == s2f(rhs));
        } else {
            data.return_value = (lhs == rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_NOT: {
        if(is_number(lhs) && is_number(rhs)) {
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
void mob_action_runners::link_with_focus(mob_action_run_data &data) {
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
void mob_action_runners::load_focus_memory(mob_action_run_data &data) {
    if(data.m->focused_mob_memory.empty()) {
        return;
    }
    
    data.m->focus_on_mob(data.m->focused_mob_memory[s2i(data.args[0])]);
}


/**
 * @brief Code for the move to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::move_to_absolute(mob_action_run_data &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = data.args.size() > 2 ? s2f(data.args[2]) : data.m->z;
    data.m->chase(point(x, y), z);
}


/**
 * @brief Code for the move to relative coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::move_to_relative(mob_action_run_data &data) {
    float x = s2f(data.args[0]);
    float y = s2f(data.args[1]);
    float z = (data.args.size() > 2 ? s2f(data.args[2]) : 0);
    point p = rotate_point(point(x, y), data.m->angle);
    data.m->chase(data.m->pos + p, data.m->z + z);
}


/**
 * @brief Code for the move to target mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::move_to_target(mob_action_run_data &data) {
    MOB_ACTION_MOVE_TYPE t = (MOB_ACTION_MOVE_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS: {
        if(data.m->focused_mob) {
            float a = get_angle(data.m->pos, data.m->focused_mob->pos);
            point offset = point(2000, 0);
            offset = rotate_point(offset, a + TAU / 2.0);
            data.m->chase(data.m->pos + offset, data.m->z);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS: {
        if(data.m->focused_mob) {
            data.m->chase(&data.m->focused_mob->pos, &data.m->focused_mob->z);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_FOCUS_POS: {
        if(data.m->focused_mob) {
            data.m->chase(data.m->focused_mob->pos, data.m->focused_mob->z);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_TYPE_HOME: {
        data.m->chase(data.m->home, data.m->z);
        break;
        
    } case MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC: {
        data.m->arachnorb_foot_move_logic();
        break;
        
    } case MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE: {
        if(data.m->links.empty()) {
            return;
        }
        
        point des;
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
void mob_action_runners::order_release(mob_action_run_data &data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.run_event(MOB_EV_RELEASE_ORDER, nullptr, nullptr);
    }
}


/**
 * @brief Code for the sound playing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::play_sound(mob_action_run_data &data) {
    size_t sound_id = data.m->play_sound(s2i(data.args[0]));
    if(data.args.size() >= 2) {
        data.m->set_var(data.args[1], i2s(sound_id));
    }
}


/**
 * @brief Code for the text printing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::print(mob_action_run_data &data) {
    string text = vector_tail_to_string(data.args, 0);
    print_info(
        "[DEBUG PRINT] " + data.m->type->name + " says:\n" + text,
        10.0f
    );
}


/**
 * @brief Code for the status reception mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::receive_status(mob_action_run_data &data) {
    data.m->apply_status_effect(game.content.status_types.list[data.args[0]], false, false);
}


/**
 * @brief Code for the release mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::release(mob_action_run_data &data) {
    data.m->release_chomped_pikmin();
}


/**
 * @brief Code for the release stored mobs mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::release_stored_mobs(mob_action_run_data &data) {
    data.m->release_stored_mobs();
}


/**
 * @brief Code for the status removal mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::remove_status(mob_action_run_data &data) {
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
void mob_action_runners::save_focus_memory(mob_action_run_data &data) {
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
void mob_action_runners::send_message_to_focus(mob_action_run_data &data) {
    if(!data.m->focused_mob) return;
    data.m->send_message(data.m->focused_mob, data.args[0]);
}


/**
 * @brief Code for the linked mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::send_message_to_links(mob_action_run_data &data) {
    for(size_t l = 0; l < data.m->links.size(); l++) {
        if(data.m->links[l] == data.m) continue;
        if(!data.m->links[l]) continue;
        data.m->send_message(data.m->links[l], data.args[0]);
    }
}


/**
 * @brief Code for the nearby mob message sending mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::send_message_to_nearby(mob_action_run_data &data) {
    float d = s2f(data.args[0]);
    
    for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); m2++) {
        if(game.states.gameplay->mobs.all[m2] == data.m) {
            continue;
        }
        if(dist(data.m->pos, game.states.gameplay->mobs.all[m2]->pos) > d) {
            continue;
        }
        
        data.m->send_message(
            game.states.gameplay->mobs.all[m2], data.args[1]
        );
    }
}


/**
 * @brief Code for the animation setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_animation(mob_action_run_data &data) {
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
    
    data.m->set_animation(
        s2i(data.args[0]), options, false, mob_speed_baseline
    );
}


/**
 * @brief Code for the block paths setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_can_block_paths(mob_action_run_data &data) {
    data.m->set_can_block_paths(s2b(data.args[0]));
}


/**
 * @brief Code for the far reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_far_reach(mob_action_run_data &data) {
    data.m->far_reach = s2i(data.args[0]);
    data.m->update_interaction_span();
}


/**
 * @brief Code for the flying setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_flying(mob_action_run_data &data) {
    if(s2b(data.args[0])) {
        enable_flag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    } else {
        disable_flag(data.m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Code for the gravity setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_gravity(mob_action_run_data &data) {
    data.m->gravity_mult = s2f(data.args[0]);
}


/**
 * @brief Code for the health setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_health(mob_action_run_data &data) {
    data.m->set_health(false, false, s2f(data.args[0]));
}


/**
 * @brief Code for the height setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_height(mob_action_run_data &data) {
    data.m->height = s2f(data.args[0]);
    
    if(data.m->type->walkable) {
        //Update the Z of mobs standing on top of it.
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            mob* m2_ptr = game.states.gameplay->mobs.all[m];
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
void mob_action_runners::set_hiding(mob_action_run_data &data) {
    if(s2b(data.args[0])) {
        enable_flag(data.m->flags, MOB_FLAG_HIDDEN);
    } else {
        disable_flag(data.m->flags, MOB_FLAG_HIDDEN);
    }
}


/**
 * @brief Code for the holdable setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_holdable(mob_action_run_data &data) {
    if(typeid(*(data.m)) == typeid(tool)) {
        unsigned char flags = 0;
        for(size_t i = 0; i < data.args.size(); i++) {
            flags |= s2i(data.args[i]);
        }
        ((tool*) (data.m))->holdability_flags = flags;
    }
}


/**
 * @brief Code for the huntable setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_huntable(mob_action_run_data &data) {
    if(s2b(data.args[0])) {
        disable_flag(data.m->flags, MOB_FLAG_NON_HUNTABLE);
    } else {
        enable_flag(data.m->flags, MOB_FLAG_NON_HUNTABLE);
    }
}


/**
 * @brief Code for the limb animation setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_limb_animation(mob_action_run_data &data) {
    if(!data.m->parent) {
        return;
    }
    if(!data.m->parent->limb_anim.anim_db) {
        return;
    }
    
    size_t a = data.m->parent->limb_anim.anim_db->find_animation(data.args[0]);
    if(a == INVALID) {
        return;
    }
    
    data.m->parent->limb_anim.cur_anim =
        data.m->parent->limb_anim.anim_db->animations[a];
    data.m->parent->limb_anim.to_start();
    
}


/**
 * @brief Code for the near reach setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_near_reach(mob_action_run_data &data) {
    data.m->near_reach = s2i(data.args[0]);
    data.m->update_interaction_span();
}


/**
 * @brief Code for the radius setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_radius(mob_action_run_data &data) {
    data.m->set_radius(s2f(data.args[0]));
}


/**
 * @brief Code for the sector scroll setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_sector_scroll(mob_action_run_data &data) {
    sector* s_ptr = get_sector(data.m->pos, nullptr, true);
    if(!s_ptr) return;
    
    s_ptr->scroll.x = s2f(data.args[0]);
    s_ptr->scroll.y = s2f(data.args[1]);
}


/**
 * @brief Code for the shadow visibility setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_shadow_visibility(mob_action_run_data &data) {
    if(s2b(data.args[0])) {
        disable_flag(data.m->flags, MOB_FLAG_SHADOW_INVISIBLE);
    } else {
        enable_flag(data.m->flags, MOB_FLAG_SHADOW_INVISIBLE);
    }
}


/**
 * @brief Code for the state setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_state(mob_action_run_data &data) {
    data.m->fsm.set_state(
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
void mob_action_runners::set_tangible(mob_action_run_data &data) {
    if(s2b(data.args[0])) {
        disable_flag(data.m->flags, MOB_FLAG_INTANGIBLE);
    } else {
        enable_flag(data.m->flags, MOB_FLAG_INTANGIBLE);
    }
}


/**
 * @brief Code for the team setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_team(mob_action_run_data &data) {
    data.m->team = (MOB_TEAM) s2i(data.args[0]);
}


/**
 * @brief Code for the timer setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_timer(mob_action_run_data &data) {
    data.m->set_timer(s2f(data.args[0]));
}


/**
 * @brief Code for the var setting mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::set_var(mob_action_run_data &data) {
    data.m->set_var(data.args[0], data.args[1]);
}


/**
 * @brief Code for the show message from var mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::show_message_from_var(mob_action_run_data &data) {
    start_message(data.m->vars[data.args[0]], nullptr);
}


/**
 * @brief Code for the spawning mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::spawn(mob_action_run_data &data) {
    data.m->spawn(&data.m->type->spawns[s2i(data.args[0])]);
}


/**
 * @brief Code for the z stabilization mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stabilize_z(mob_action_run_data &data) {
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
void mob_action_runners::start_chomping(mob_action_run_data &data) {
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
void mob_action_runners::start_dying(mob_action_run_data &data) {
    data.m->start_dying();
}


/**
 * @brief Code for the height effect start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::start_height_effect(mob_action_run_data &data) {
    data.m->start_height_effect();
}


/**
 * @brief Code for the particle start mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::start_particles(mob_action_run_data &data) {
    float offset_x = 0;
    float offset_y = 0;
    float offset_z = 0;
    if(data.args.size() > 1) offset_x = s2f(data.args[1]);
    if(data.args.size() > 2) offset_y = s2f(data.args[2]);
    if(data.args.size() > 3) offset_z = s2f(data.args[3]);
    
    particle_generator pg =
        game.content.custom_particle_gen.list[data.args[0]];
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    pg.follow_mob = data.m;
    pg.follow_angle = &data.m->angle;
    pg.follow_pos_offset = point(offset_x, offset_y);
    pg.follow_z_offset = offset_z;
    pg.reset();
    data.m->particle_generators.push_back(pg);
}


/**
 * @brief Code for the stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->stop_turning();
    data.m->stop_following_path();
}


/**
 * @brief Code for the chomp stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop_chomping(mob_action_run_data &data) {
    data.m->chomp_max = 0;
    data.m->chomp_body_parts.clear();
}


/**
 * @brief Code for the height effect stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop_height_effect(mob_action_run_data &data) {
    data.m->stop_height_effect();
}


/**
 * @brief Code for the particle stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop_particles(mob_action_run_data &data) {
    data.m->remove_particle_generator(MOB_PARTICLE_GENERATOR_ID_SCRIPT);
}


/**
 * @brief Code for the sound stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop_sound(mob_action_run_data &data) {
    game.audio.destroy_sound_source(s2i(data.args[0]));
}


/**
 * @brief Code for the vertical stopping mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::stop_vertically(mob_action_run_data &data) {
    data.m->speed_z = 0;
}


/**
 * @brief Code for the focus storing mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::store_focus_inside(mob_action_run_data &data) {
    if(data.m->focused_mob && !data.m->focused_mob->is_stored_inside_mob()) {
        data.m->store_mob_inside(data.m->focused_mob);
    }
}


/**
 * @brief Code for the swallow mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::swallow(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(s2i(data.args[1]));
}


/**
 * @brief Code for the swallow all mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::swallow_all(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(data.m->chomping_mobs.size());
}


/**
 * @brief Code for the teleport to absolute coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::teleport_to_absolute(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->chase(
        point(s2f(data.args[0]), s2f(data.args[1])),
        s2f(data.args[2]),
        CHASE_FLAG_TELEPORT
    );
}


/**
 * @brief Code for the teleport to relative coordinates mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::teleport_to_relative(mob_action_run_data &data) {
    data.m->stop_chasing();
    point p =
        rotate_point(
            point(s2f(data.args[0]), s2f(data.args[1])),
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
void mob_action_runners::throw_focus(mob_action_run_data &data) {
    if(!data.m->focused_mob) {
        return;
    }
    
    if(data.m->focused_mob->holder.m == data.m) {
        data.m->release(data.m->focused_mob, true);
    }
    
    float max_height = s2f(data.args[3]);
    
    if(max_height == 0.0f) {
        //We just want to drop it, not throw it.
        return;
    }
    
    data.m->start_height_effect();
    calculate_throw(
        data.m->focused_mob->pos, data.m->focused_mob->z,
        point(s2f(data.args[0]), s2f(data.args[1])), s2f(data.args[2]),
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
void mob_action_runners::turn_to_absolute(mob_action_run_data &data) {
    if(data.args.size() == 1) {
        //Turn to an absolute angle.
        data.m->face(deg_to_rad(s2f(data.args[0])), nullptr);
    } else {
        //Turn to some absolute coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        data.m->face(get_angle(data.m->pos, point(x, y)), nullptr);
    }
}


/**
 * @brief Code for the turn to a relative angle mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::turn_to_relative(mob_action_run_data &data) {
    if(data.args.size() == 1) {
        //Turn to a relative angle.
        data.m->face(data.m->angle + deg_to_rad(s2f(data.args[0])), nullptr);
    } else {
        //Turn to some relative coordinates.
        float x = s2f(data.args[0]);
        float y = s2f(data.args[1]);
        point p = rotate_point(point(x, y), data.m->angle);
        data.m->face(get_angle(data.m->pos, data.m->pos + p), nullptr);
    }
}


/**
 * @brief Code for the turn to target mob script action.
 *
 * @param data Data about the action call.
 */
void mob_action_runners::turn_to_target(mob_action_run_data &data) {
    MOB_ACTION_TURN_TYPE t = (MOB_ACTION_TURN_TYPE) s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC: {
        data.m->arachnorb_head_turn_logic();
        break;
        
    } case MOB_ACTION_TURN_TYPE_FOCUSED_MOB: {
        if(data.m->focused_mob) {
            data.m->face(0, &data.m->focused_mob->pos);
        }
        break;
        
    } case MOB_ACTION_TURN_TYPE_HOME: {
        data.m->face(get_angle(data.m->pos, data.m->home), nullptr);
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
bool assert_actions(
    const vector<mob_action_call*> &actions, const data_node* dn
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
mob* get_trigger_mob(mob_action_run_data &data) {
    if(
        data.call->parent_event == MOB_EV_OBJECT_IN_REACH ||
        data.call->parent_event == MOB_EV_OPPONENT_IN_REACH ||
        data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED ||
        data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
        data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
        data.call->parent_event == MOB_EV_HELD ||
        data.call->parent_event == MOB_EV_RELEASED ||
        data.call->parent_event == MOB_EV_STARTED_RECEIVING_DELIVERY ||
        data.call->parent_event == MOB_EV_FINISHED_RECEIVING_DELIVERY
    ) {
        return (mob*)(data.custom_data_1);
        
    } else if(
        data.call->parent_event == MOB_EV_RECEIVE_MESSAGE
    ) {
        return(mob*)(data.custom_data_2);
        
    } else if(
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
        data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_N ||
        data.call->parent_event == MOB_EV_DAMAGE
    ) {
        return ((hitbox_interaction*)(data.custom_data_1))->mob2;
        
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
void insert_event_actions(
    mob_event* ev, const vector<mob_action_call*> &actions, bool at_end
) {
    vector<mob_action_call*>::iterator it =
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
void load_actions(
    mob_type* mt, data_node* node,
    vector<mob_action_call*>* out_actions, bitmask_8_t* out_settings
) {
    if(out_settings) *out_settings = 0;
    for(size_t a = 0; a < node->get_nr_of_children(); a++) {
        data_node* action_node = node->get_child(a);
        if(
            out_settings && action_node->name == "custom_actions_after"
        ) {
            enable_flag(*out_settings, EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER);
        } else if(
            out_settings && action_node->name == "global_actions_after"
        ) {
            enable_flag(*out_settings, EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER);
        } else {
            mob_action_call* new_a = new mob_action_call();
            if(new_a->load_from_data_node(action_node, mt)) {
                out_actions->push_back(new_a);
            } else {
                delete new_a;
            }
        }
    }
    assert_actions(*out_actions, node);
}
