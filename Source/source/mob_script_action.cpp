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
#include "mobs/scale.h"
#include "mobs/tool.h"
#include "utils/string_utils.h"


using std::set;


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action.
 */
mob_action::mob_action() :
    type(MOB_ACTION_UNKNOWN),
    code(nullptr),
    extra_load_logic(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action call, of a certain type.
 * type:
 *   Type of mob action call.
 */
mob_action_call::mob_action_call(MOB_ACTION_TYPES type) :
    action(nullptr),
    code(nullptr),
    parent_event(MOB_EV_UNKNOWN),
    mt(nullptr) {
    
    for(size_t a = 0; a < game.mob_actions.size(); ++a) {
        if(game.mob_actions[a].type == type) {
            action = &(game.mob_actions[a]);
            break;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action call that is meant to run custom code.
 * code:
 *   The function to run.
 */
mob_action_call::mob_action_call(custom_action_code code):
    action(nullptr),
    code(code),
    parent_event(MOB_EV_UNKNOWN),
    mt(nullptr) {
    
    for(size_t a = 0; a < game.mob_actions.size(); ++a) {
        if(game.mob_actions[a].type == MOB_ACTION_UNKNOWN) {
            action = &(game.mob_actions[a]);
            break;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads a mob action call from a data node.
 * dn:
 *   The data node.
 * mt:
 *   Mob type this action's fsm belongs to.
 */
bool mob_action_call::load_from_data_node(data_node* dn, mob_type* mt) {

    action = NULL;
    this->mt = mt;
    
    //First, get the name and arguments.
    vector<string> words = split(dn->name);
    
    for(size_t w = 0; w < words.size(); ++w) {
        words[w] = trim_spaces(words[w]);
    }
    
    string name = words[0];
    if(!words.empty()) {
        words.erase(words.begin());
    }
    
    //Find the corresponding action.
    for(size_t a = 0; a < game.mob_actions.size(); ++a) {
        if(game.mob_actions[a].type == MOB_ACTION_UNKNOWN) continue;
        if(game.mob_actions[a].name == name) {
            action = &(game.mob_actions[a]);
        }
    }
    
    if(!action) {
        log_error("Unknown script action name \"" + name + "\"!", dn);
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
        log_error(
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
            log_error(
                "The \"" + action->name + "\" action only needs " +
                i2s(action->parameters.size()) + " arguments, but this call "
                "has " + i2s(words.size()) + "!",
                dn
            );
            return false;
        }
    }
    
    //Fetch the arguments, and check if any of them are not allowed.
    for(size_t w = 0; w < words.size(); ++w) {
        size_t param_nr = std::min(w, action->parameters.size() - 1);
        bool is_var = (words[w][0] == '$' && words[w].size() > 1);
        
        if(is_var && words[w].size() >= 2 && words[w][1] == '$') {
            //Two '$' in a row means it's meant to use a literal '$'.
            is_var = false;
            words[w].erase(words[w].begin());
        }
        
        if(is_var) {
            if(action->parameters[param_nr].force_const) {
                log_error(
                    "Argument #" + i2s(w) + " (\"" + words[w] + "\") is a "
                    "variable, but the parameter \"" +
                    action->parameters[param_nr].name + "\" can only be "
                    "constant!",
                    dn
                );
                return false;
            }
            
            words[w].erase(words[w].begin()); //Remove the '$'.
            
            if(words[w].empty()) {
                log_error(
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
            log_error(custom_error, dn);
        }
        return success;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Runs an action.
 * Return value is only used by the "if" actions, to indicate their
 *   evaluation result.
 * m:
 *   The mob.
 * custom_data_1:
 *   Custom argument #1 to pass to the code.
 * custom_data_2:
 *   Custom argument #2 to pass to the code.
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
    for(size_t a = 0; a < args.size(); ++a) {
        if(arg_is_var[a]) {
            data.args[a] = m->vars[args[a]];
        }
    }
    data.custom_data_1 = custom_data_1;
    data.custom_data_2 = custom_data_2;
    
    action->code(data);
    return data.return_value;
}



/* ----------------------------------------------------------------------------
 * Loading code for the arachnorb logic plan mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::arachnorb_plan_logic(mob_action_call &call) {
    if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME);
    } else if(call.args[0] == "forward") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD);
    } else if(call.args[0] == "cw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN);
    } else if(call.args[0] == "ccw_turn") {
        call.args[0] = i2s(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN);
    } else {
        report_enum_error(call, 0);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the calculation mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::calculate(mob_action_call &call) {
    if(call.args[2] == "+") {
        call.args[2] = i2s(MOB_ACTION_SET_VAR_SUM);
    } else if(call.args[2] == "-") {
        call.args[2] = i2s(MOB_ACTION_SET_VAR_SUBTRACT);
    } else if(call.args[2] == "*") {
        call.args[2] = i2s(MOB_ACTION_SET_VAR_MULTIPLY);
    } else if(call.args[2] == "/") {
        call.args[2] = i2s(MOB_ACTION_SET_VAR_DIVIDE);
    } else if(call.args[2] == "%") {
        call.args[2] = i2s(MOB_ACTION_SET_VAR_MODULO);
    } else {
        report_enum_error(call, 2);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the focus mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::focus(mob_action_call &call) {
    if(call.args[0] == "link") {
        call.args[0] = i2s(MOB_ACTION_FOCUS_LINK);
    } else if(call.args[0] == "parent") {
        call.args[0] = i2s(MOB_ACTION_FOCUS_PARENT);
    } else if(call.args[0] == "trigger") {
        call.args[0] = i2s(MOB_ACTION_FOCUS_TRIGGER);
    } else {
        report_enum_error(call, 0);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the info getting script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::get_info(mob_action_call &call) {
    if(call.args[1] == "body_part") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_BODY_PART);
    } else if(call.args[1] == "chomped_pikmin") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_CHOMPED_PIKMIN);
    } else if(call.args[1] == "day_minutes") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_DAY_MINUTES);
    } else if(call.args[1] == "field_pikmin") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_FIELD_PIKMIN);
    } else if(call.args[1] == "frame_signal") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_FRAME_SIGNAL);
    } else if(call.args[1] == "health") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_HEALTH);
    } else if(call.args[1] == "latched_pikmin") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_LATCHED_PIKMIN);
    } else if(call.args[1] == "latched_pikmin_weight") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_LATCHED_PIKMIN_WEIGHT);
    } else if(call.args[1] == "message") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_MESSAGE);
    } else if(call.args[1] == "message_sender") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_MESSAGE_SENDER);
    } else if(call.args[1] == "mob_category") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_MOB_CATEGORY);
    } else if(call.args[1] == "mob_type") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_MOB_TYPE);
    } else if(call.args[1] == "other_body_part") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_OTHER_BODY_PART);
    } else if(call.args[1] == "weight") {
        call.args[1] = i2s(MOB_ACTION_GET_INFO_WEIGHT);
    } else {
        report_enum_error(call, 1);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the "if" mob script action.
 * call:
 *   Mob action call that called this.
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


/* ----------------------------------------------------------------------------
 * Loading code for the move to target mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::move_to_target(mob_action_call &call) {
    if(call.args[0] == "arachnorb_foot_logic") {
        call.args[0] = i2s(MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC);
    } else if(call.args[0] == "away_from_focused_mob") {
        call.args[0] = i2s(MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB);
    } else if(call.args[0] == "focused_mob") {
        call.args[0] = i2s(MOB_ACTION_MOVE_FOCUSED_MOB);
    } else if(call.args[0] == "focused_mob_position") {
        call.args[0] = i2s(MOB_ACTION_MOVE_FOCUSED_MOB_POS);
    } else if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_MOVE_HOME);
    } else if(call.args[0] == "linked_mob_average") {
        call.args[0] = i2s(MOB_ACTION_MOVE_LINKED_MOB_AVERAGE);
    } else {
        report_enum_error(call, 0);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the status reception mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::receive_status(mob_action_call &call) {
    if(game.status_types.find(call.args[0]) == game.status_types.end()) {
        call.custom_error =
            "Unknown status effect \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the status removal mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::remove_status(mob_action_call &call) {
    if(game.status_types.find(call.args[0]) == game.status_types.end()) {
        call.custom_error =
            "Unknown status effect \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Reports an error of an unknown enum value.
 * call:
 *   Mob action call that called this.
 * arg_nr:
 *   Index number of the argument that is an enum.
 */
void mob_action_loaders::report_enum_error(
    mob_action_call &call, const size_t arg_nr
) {
    size_t param_nr = std::min(arg_nr, call.action->parameters.size() - 1);
    call.custom_error =
        "The parameter \"" + call.action->parameters[param_nr].name + "\" "
        "does not know what the value \"" +
        call.args[arg_nr] + "\" means!";
}


/* ----------------------------------------------------------------------------
 * Loading code for the animation setting mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::set_animation(mob_action_call &call) {
    size_t a_pos = call.mt->anims.find_animation(call.args[0]);
    if(a_pos == INVALID) {
        call.custom_error =
            "Unknown animation \"" + call.args[0] + "\"!";
        return false;
    }
    call.args[0] = i2s(a_pos);
    
    for(size_t a = 1; a < call.args.size(); ++a) {
        if(call.args[a] == "no_restart") {
            call.args[a] = i2s(MOB_ACTION_SET_ANIMATION_NO_RESTART);
        } else {
            call.args[a] = "0";
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the far reach setting mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::set_far_reach(mob_action_call &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); ++r) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the holdable setting mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::set_holdable(mob_action_call &call) {
    for(size_t a = 0; a < call.args.size(); ++a) {
        if(call.args[a] == "pikmin") {
            call.args[a] = i2s(HOLDABLE_BY_PIKMIN);
        } else if(call.args[a] == "enemies") {
            call.args[a] = i2s(HOLDABLE_BY_ENEMIES);
        } else {
            report_enum_error(call, a);
            return false;
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the near reach setting mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::set_near_reach(mob_action_call &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); ++r) {
        if(call.mt->reaches[r].name == call.args[0]) {
            call.args[0] = i2s(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the team setting mob script action.
 * call:
 *   Mob action call that called this.
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


/* ----------------------------------------------------------------------------
 * Loading code for the spawning mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::spawn(mob_action_call &call) {
    for(size_t s = 0; s < call.mt->spawns.size(); ++s) {
        if(call.mt->spawns[s].name == call.args[0]) {
            call.args[0] = i2s(s);
            return true;
        }
    }
    call.custom_error =
        "Unknown spawn info block \"" + call.args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the z stabilization mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::stabilize_z(mob_action_call &call) {
    if(call.args[0] == "lowest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_LOWEST);
    } else if(call.args[0] == "highest") {
        call.args[0] = i2s(MOB_ACTION_STABILIZE_Z_HIGHEST);
    } else {
        report_enum_error(call, 0);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the chomping start mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::start_chomping(mob_action_call &call) {
    for(size_t s = 1; s < call.args.size(); ++s) {
        size_t p_nr = call.mt->anims.find_body_part(call.args[s]);
        if(p_nr == INVALID) {
            call.custom_error =
                "Unknown body part \"" + call.args[s] + "\"!";
            return false;
        }
        call.args[s] = i2s(p_nr);
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the particle start mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::start_particles(mob_action_call &call) {
    if(
        game.custom_particle_generators.find(call.args[0]) ==
        game.custom_particle_generators.end()
    ) {
        call.custom_error =
            "Unknown particle generator \"" + call.args[0] + "\"!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the turn to target mob script action.
 * call:
 *   Mob action call that called this.
 */
bool mob_action_loaders::turn_to_target(mob_action_call &call) {
    if(call.args[0] == "arachnorb_head_logic") {
        call.args[0] = i2s(MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC);
    } else if(call.args[0] == "focused_mob") {
        call.args[0] = i2s(MOB_ACTION_TURN_FOCUSED_MOB);
    } else if(call.args[0] == "home") {
        call.args[0] = i2s(MOB_ACTION_TURN_HOME);
    } else {
        report_enum_error(call, 0);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action parameter struct.
 * name:
 *   Name of the parameter.
 * type:
 *   Type of parameter.
 * force_const:
 *   If true, this must be a constant value. If false, it can also be a var.
 * is_extras:
 *   If true, this is an array of them (minimum amount 0).
 */
mob_action_param::mob_action_param(
    const string &name,
    const MOB_ACTION_PARAM_TYPE type,
    const bool force_const,
    const bool is_extras
):
    name(name),
    type(type),
    force_const(force_const),
    is_extras(is_extras) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action run data struct.
 * m:
 *   The mob responsible.
 * call:
 *   Mob action call that called this.
 */
mob_action_run_data::mob_action_run_data(mob* m, mob_action_call* call) :
    m(m),
    call(call),
    custom_data_1(nullptr),
    custom_data_2(nullptr),
    return_value(false) {
    
}


/* ----------------------------------------------------------------------------
 * Code for the health addition mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::add_health(mob_action_run_data &data) {
    data.m->set_health(true, false, s2f(data.args[0]));
}


/* ----------------------------------------------------------------------------
 * Code for the arachnorb logic plan mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::arachnorb_plan_logic(mob_action_run_data &data) {
    data.m->arachnorb_plan_logic(s2i(data.args[0]));
}


/* ----------------------------------------------------------------------------
 * Code for the calculation mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::calculate(mob_action_run_data &data) {
    float lhs = s2f(data.args[1]);
    size_t op = s2i(data.args[2]);
    float rhs = s2f(data.args[3]);
    float result = 0;
    
    switch(op) {
    case MOB_ACTION_SET_VAR_SUM: {
        result = lhs + rhs;
        break;
        
    } case MOB_ACTION_SET_VAR_SUBTRACT: {
        result = lhs - rhs;
        break;
        
    } case MOB_ACTION_SET_VAR_MULTIPLY: {
        result = lhs * rhs;
        break;
        
    } case MOB_ACTION_SET_VAR_DIVIDE: {
        if(rhs == 0) {
            result = 0;
        } else {
            result = lhs / rhs;
        }
        break;
        
    } case MOB_ACTION_SET_VAR_MODULO: {
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


/* ----------------------------------------------------------------------------
 * Code for the deletion mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::delete_function(mob_action_run_data &data) {
    data.m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Code for the liquid draining mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::drain_liquid(mob_action_run_data &data) {
    sector* s_ptr = get_sector(data.m->pos, NULL, true);
    if(!s_ptr) return;
    
    vector<sector*> sectors_to_drain;
    
    s_ptr->get_neighbor_sectors_conditionally(
    [] (sector * s) -> bool {
        for(size_t h = 0; h < s->hazards.size(); ++h) {
            if(s->hazards[h]->associated_liquid) {
                return true;
            }
        }
        if(s->type == SECTOR_TYPE_BRIDGE) {
            return true;
        }
        if(s->type == SECTOR_TYPE_BRIDGE_RAIL) {
            return true;
        }
        return false;
    },
    sectors_to_drain
    );
    
    for(size_t s = 0; s < sectors_to_drain.size(); ++s) {
        sectors_to_drain[s]->draining_liquid = true;
        sectors_to_drain[s]->liquid_drain_left = LIQUID_DRAIN_DURATION;
    }
}


/* ----------------------------------------------------------------------------
 * Code for the death finish mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::finish_dying(mob_action_run_data &data) {
    data.m->finish_dying();
}


/* ----------------------------------------------------------------------------
 * Code for the focus mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::focus(mob_action_run_data &data) {
    size_t t = s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_FOCUS_LINK: {
        if(!data.m->links.empty()) {
            data.m->focus_on_mob(data.m->links[0]);
        }
        break;
        
    } case MOB_ACTION_FOCUS_PARENT: {
        if(data.m->parent) {
            data.m->focus_on_mob(data.m->parent->m);
        }
        break;
        
    } case MOB_ACTION_FOCUS_TRIGGER: {
        if(
            data.call->parent_event == MOB_EV_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EV_OPPONENT_IN_REACH ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED ||
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT
        ) {
        
            data.m->focus_on_mob((mob*) (data.custom_data_1));
            
        } else if(
            data.call->parent_event == MOB_EV_RECEIVE_MESSAGE
        ) {
        
            data.m->focus_on_mob((mob*) (data.custom_data_2));
            
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the mob script action for getting chomped.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::get_chomped(mob_action_run_data &data) {
    if(data.call->parent_event == MOB_EV_HITBOX_TOUCH_EAT) {
        ((mob*) (data.custom_data_1))->chomp(
            data.m,
            (hitbox*) (data.custom_data_2)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Code for the focused mob var getting script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::get_focus_var(mob_action_run_data &data) {
    if(!data.m->focused_mob) return;
    data.m->vars[data.args[0]] =
        data.m->focused_mob->vars[data.args[1]];
}


/* ----------------------------------------------------------------------------
 * Code for the info obtaining mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::get_info(mob_action_run_data &data) {
    string* var = &(data.m->vars[data.args[0]]);
    size_t t = s2i(data.args[1]);
    
    switch(t) {
    case MOB_ACTION_GET_INFO_CHOMPED_PIKMIN: {
        *var = i2s(data.m->chomping_mobs.size());
        break;
        
    } case MOB_ACTION_GET_INFO_DAY_MINUTES: {
        *var = i2s(game.states.gameplay_st->day_minutes);
        break;
        
    } case MOB_ACTION_GET_INFO_FIELD_PIKMIN: {
        *var = i2s(game.states.gameplay_st->mobs.pikmin_list.size());
        break;
        
    } case MOB_ACTION_GET_INFO_FRAME_SIGNAL: {
        if(data.call->parent_event == MOB_EV_FRAME_SIGNAL) {
            *var = i2s(*((size_t*) (data.custom_data_1)));
        }
        break;
        
    } case MOB_ACTION_GET_INFO_HEALTH: {
        *var = i2s(data.m->health);
        break;
        
    } case MOB_ACTION_GET_INFO_LATCHED_PIKMIN: {
        *var = i2s(data.m->get_latched_pikmin_amount());
        break;
        
    } case MOB_ACTION_GET_INFO_LATCHED_PIKMIN_WEIGHT: {
        *var = i2s(data.m->get_latched_pikmin_weight());
        break;
        
    } case MOB_ACTION_GET_INFO_MESSAGE: {
        if(data.call->parent_event == MOB_EV_RECEIVE_MESSAGE) {
            *var = *((string*) (data.custom_data_1));
        }
        break;
        
    } case MOB_ACTION_GET_INFO_MESSAGE_SENDER: {
        if(data.call->parent_event == MOB_EV_RECEIVE_MESSAGE) {
            *var = ((mob*) (data.custom_data_2))->type->name;
        }
        break;
        
    } case MOB_ACTION_GET_INFO_MOB_CATEGORY: {
        if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EV_OPPONENT_IN_REACH
        ) {
            *var = ((mob*) (data.custom_data_1))->type->category->name;
        }
        break;
        
    } case MOB_ACTION_GET_INFO_MOB_TYPE: {
        if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EV_OPPONENT_IN_REACH ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var = ((mob*) (data.custom_data_1))->type->name;
        }
        break;
        
    } case MOB_ACTION_GET_INFO_BODY_PART: {
        if(
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (hitbox_interaction*) (data.custom_data_1)
                )->h1->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                data.m->get_closest_hitbox(
                    ((mob*) (data.custom_data_1))->pos,
                    INVALID, NULL
                )->body_part_name;
        }
        break;
        
    } case MOB_ACTION_GET_INFO_OTHER_BODY_PART: {
        if(
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_A_N ||
            data.call->parent_event == MOB_EV_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EV_DAMAGE
        ) {
            *var =
                (
                    (hitbox_interaction*) (data.custom_data_1)
                )->h2->body_part_name;
        } else if(
            data.call->parent_event == MOB_EV_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EV_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EV_THROWN_PIKMIN_LANDED
        ) {
            *var =
                ((mob*) (data.custom_data_1))->get_closest_hitbox(
                    data.m->pos, INVALID, NULL
                )->body_part_name;
        }
        break;
        
    } case MOB_ACTION_GET_INFO_WEIGHT: {
        if(data.m->type->category->id == MOB_CATEGORY_SCALES) {
            scale* s_ptr = (scale*) (data.m);
            *var = i2s(s_ptr->calculate_cur_weight());
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the decimal number randomization mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::get_random_decimal(mob_action_run_data &data) {
    data.m->vars[data.args[0]] =
        f2s(randomf(s2f(data.args[1]), s2f(data.args[2])));
}


/* ----------------------------------------------------------------------------
 * Code for the integer number randomization mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::get_random_int(mob_action_run_data &data) {
    data.m->vars[data.args[0]] =
        i2s(randomi(s2i(data.args[1]), s2i(data.args[2])));
}


/* ----------------------------------------------------------------------------
 * Code for the "if" mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::if_function(mob_action_run_data &data) {
    string lhs = data.args[0];
    size_t op = s2i(data.args[1]);
    string rhs = vector_tail_to_string(data.args, 2);
    
    switch(op) {
    case MOB_ACTION_IF_OP_EQUAL: {
        if(is_number(lhs)) {
            data.return_value = (s2f(lhs) == s2f(rhs));
        } else {
            data.return_value = (lhs == rhs);
        }
        break;
        
    } case MOB_ACTION_IF_OP_NOT: {
        if(is_number(lhs)) {
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


/* ----------------------------------------------------------------------------
 * Code for the move to absolute coordinates mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::move_to_absolute(mob_action_run_data &data) {
    data.m->chase(
        point(s2f(data.args[0]), s2f(data.args[1])), NULL,
        false
    );
}


/* ----------------------------------------------------------------------------
 * Code for the move to relative coordinates mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::move_to_relative(mob_action_run_data &data) {
    point p =
        rotate_point(
            point(s2f(data.args[0]), s2f(data.args[1])),
            data.m->angle
        );
    data.m->chase(data.m->pos + p, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Code for the move to target mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::move_to_target(mob_action_run_data &data) {
    size_t t = s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB: {
        if(data.m->focused_mob) {
            float a = get_angle(data.m->pos, data.m->focused_mob->pos);
            point offset = point(2000, 0);
            offset = rotate_point(offset, a + TAU / 2.0);
            data.m->chase(data.m->pos + offset, NULL, false);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_FOCUSED_MOB: {
        if(data.m->focused_mob) {
            data.m->chase(point(), &data.m->focused_mob->pos, false);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_FOCUSED_MOB_POS: {
        if(data.m->focused_mob) {
            data.m->chase(data.m->focused_mob->pos, NULL, false);
        } else {
            data.m->stop_chasing();
        }
        break;
        
    } case MOB_ACTION_MOVE_HOME: {
        data.m->chase(data.m->home, NULL, false);
        break;
        
    } case MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC: {
        data.m->arachnorb_foot_move_logic();
        break;
        
    } case MOB_ACTION_MOVE_LINKED_MOB_AVERAGE: {
        if(data.m->links.empty()) {
            return;
        }
        
        point des;
        for(size_t l = 0; l < data.m->links.size(); ++l) {
            des += data.m->links[l]->pos;
        }
        des = des / data.m->links.size();
        
        data.m->chase(des, NULL, false);
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the release order mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::order_release(mob_action_run_data &data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.run_event(MOB_EV_RELEASE_ORDER, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the sound playing mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::play_sound(mob_action_run_data &data) {

}


/* ----------------------------------------------------------------------------
 * Code for the text printing mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::print(mob_action_run_data &data) {
    string text = vector_tail_to_string(data.args, 0);
    print_info(
        "[DEBUG PRINT] " + data.m->type->name + " says:\n" + text,
        10.0f
    );
}


/* ----------------------------------------------------------------------------
 * Code for the status reception mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::receive_status(mob_action_run_data &data) {
    data.m->apply_status_effect(game.status_types[data.args[0]], true, false);
}


/* ----------------------------------------------------------------------------
 * Code for the release mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::release(mob_action_run_data &data) {
    data.m->release_chomped_pikmin();
}


/* ----------------------------------------------------------------------------
 * Code for the status removal mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::remove_status(mob_action_run_data &data) {
    for(size_t s = 0; s < data.m->statuses.size(); ++s) {
        if(data.m->statuses[s].type->name == data.args[0]) {
            data.m->statuses[s].to_delete = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the focused mob message sending mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::send_message_to_focus(mob_action_run_data &data) {
    if(!data.m->focused_mob) return;
    data.m->send_message(data.m->focused_mob, data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the linked mob message sending mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::send_message_to_links(mob_action_run_data &data) {
    for(size_t l = 0; l < data.m->links.size(); ++l) {
        if(data.m->links[l] == data.m) continue;
        data.m->send_message(data.m->links[l], data.args[0]);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the nearby mob message sending mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::send_message_to_nearby(mob_action_run_data &data) {
    float d = s2f(data.args[0]);
    
    for(size_t m2 = 0; m2 < game.states.gameplay_st->mobs.all.size(); ++m2) {
        if(game.states.gameplay_st->mobs.all[m2] == data.m) continue;
        if(dist(data.m->pos, game.states.gameplay_st->mobs.all[m2]->pos) > d) continue;
        
        data.m->send_message(game.states.gameplay_st->mobs.all[m2], data.args[1]);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the animation setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_animation(mob_action_run_data &data) {
    bool must_restart =
        (
            data.args.size() > 1 &&
            s2i(data.args[1]) == MOB_ACTION_SET_ANIMATION_NO_RESTART
        );
    data.m->set_animation(s2i(data.args[0]), false, !must_restart);
}


/* ----------------------------------------------------------------------------
 * Code for the far reach setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_far_reach(mob_action_run_data &data) {
    data.m->far_reach = s2i(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the gravity setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_gravity(mob_action_run_data &data) {
    data.m->gravity_mult = s2f(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the health setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_health(mob_action_run_data &data) {
    data.m->set_health(false, false, s2f(data.args[0]));
}


/* ----------------------------------------------------------------------------
 * Code for the height setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_height(mob_action_run_data &data) {
    data.m->height = s2f(data.args[0]);
    
    if(data.m->type->walkable) {
        //Update the Z of mobs standing on top of it.
        for(size_t m = 0; m < game.states.gameplay_st->mobs.all.size(); ++m) {
            if(game.states.gameplay_st->mobs.all[m]->standing_on_mob == data.m) {
                game.states.gameplay_st->mobs.all[m]->z = data.m->z + data.m->height;
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the hiding setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_hiding(mob_action_run_data &data) {
    data.m->hide = s2b(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the holdable setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_holdable(mob_action_run_data &data) {
    if(typeid(*(data.m)) == typeid(tool)) {
        size_t flags = 0;
        for(size_t i = 0; i < data.args.size(); ++i) {
            flags |= s2i(data.args[i]);
        }
        ((tool*) (data.m))->holdability_flags = flags;
    }
}


/* ----------------------------------------------------------------------------
 * Code for the huntable setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_huntable(mob_action_run_data &data) {
    data.m->is_huntable = s2b(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the limb animation setting mob script action.
 * data:
 *   Data about the action call.
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
    data.m->parent->limb_anim.start();
    
}


/* ----------------------------------------------------------------------------
 * Code for the near reach setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_near_reach(mob_action_run_data &data) {
    data.m->near_reach = s2i(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the sector scroll setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_sector_scroll(mob_action_run_data &data) {
    sector* s_ptr = get_sector(data.m->pos, NULL, true);
    if(!s_ptr) return;
    
    s_ptr->scroll.x = s2f(data.args[0]);
    s_ptr->scroll.y = s2f(data.args[1]);
}


/* ----------------------------------------------------------------------------
 * Code for the state setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_state(mob_action_run_data &data) {
    data.m->fsm.set_state(
        s2i(data.args[0]),
        data.custom_data_1,
        data.custom_data_2
    );
}


/* ----------------------------------------------------------------------------
 * Code for the tangible setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_tangible(mob_action_run_data &data) {
    data.m->tangible = s2b(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the team setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_team(mob_action_run_data &data) {
    data.m->team = s2i(data.args[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the timer setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_timer(mob_action_run_data &data) {
    data.m->set_timer(s2f(data.args[0]));
}


/* ----------------------------------------------------------------------------
 * Code for the var setting mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::set_var(mob_action_run_data &data) {
    data.m->set_var(data.args[0], data.args[1]);
}


/* ----------------------------------------------------------------------------
 * Code for the show message from var mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::show_message_from_var(mob_action_run_data &data) {
    start_message(data.m->vars[data.args[0]], NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the spawning mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::spawn(mob_action_run_data &data) {
    data.m->spawn(&data.m->type->spawns[s2i(data.args[0])]);
}


/* ----------------------------------------------------------------------------
 * Code for the z stabilization mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stabilize_z(mob_action_run_data &data) {
    if(data.m->links.empty()) {
        return;
    }
    
    float best_match_z = data.m->links[0]->z;
    size_t t = s2i(data.args[0]);
    
    for(size_t l = 1; l < data.m->links.size(); ++l) {
    
        switch(t) {
        case MOB_ACTION_STABILIZE_Z_HIGHEST: {
            if(data.m->links[l]->z > best_match_z) {
                best_match_z = data.m->links[l]->z;
            }
            break;
            
        } case MOB_ACTION_STABILIZE_Z_LOWEST: {
            if(data.m->links[l]->z < best_match_z) {
                best_match_z = data.m->links[l]->z;
            }
            break;
            
        }
        }
        
    }
    
    data.m->z = best_match_z + s2f(data.args[1]);
}


/* ----------------------------------------------------------------------------
 * Code for the chomping start mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::start_chomping(mob_action_run_data &data) {
    data.m->chomp_max = s2i(data.args[0]);
    data.m->chomp_body_parts.clear();
    for(size_t p = 1; p < data.args.size(); ++p) {
        data.m->chomp_body_parts.push_back(s2i(data.args[p]));
    }
}


/* ----------------------------------------------------------------------------
 * Code for the dying start mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::start_dying(mob_action_run_data &data) {
    data.m->start_dying();
}


/* ----------------------------------------------------------------------------
 * Code for the height effect start mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::start_height_effect(mob_action_run_data &data) {
    data.m->start_height_effect();
}


/* ----------------------------------------------------------------------------
 * Code for the particle start mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::start_particles(mob_action_run_data &data) {
    float offset_x = 0;
    float offset_y = 0;
    float offset_z = 0;
    if(data.args.size() > 1) offset_x = s2f(data.args[1]);
    if(data.args.size() > 2) offset_y = s2f(data.args[2]);
    if(data.args.size() > 3) offset_z = s2f(data.args[3]);
    
    particle_generator pg = game.custom_particle_generators[data.args[0]];
    pg.id = MOB_PARTICLE_GENERATOR_SCRIPT;
    pg.follow_mob = data.m;
    pg.follow_angle = &data.m->angle;
    pg.follow_pos_offset = point(offset_x, offset_y);
    pg.follow_z_offset = offset_z;
    pg.reset();
    data.m->particle_generators.push_back(pg);
}


/* ----------------------------------------------------------------------------
 * Code for the stopping mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stop(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->stop_turning();
}


/* ----------------------------------------------------------------------------
 * Code for the chomp stopping mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stop_chomping(mob_action_run_data &data) {
    data.m->chomp_max = 0;
    data.m->chomp_body_parts.clear();
}


/* ----------------------------------------------------------------------------
 * Code for the height effect stopping mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stop_height_effect(mob_action_run_data &data) {
    data.m->stop_height_effect();
}


/* ----------------------------------------------------------------------------
 * Code for the particle stopping mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stop_particles(mob_action_run_data &data) {
    data.m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
}


/* ----------------------------------------------------------------------------
 * Code for the vertical stopping mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::stop_vertically(mob_action_run_data &data) {
    data.m->speed_z = 0;
}


/* ----------------------------------------------------------------------------
 * Code for the swallow mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::swallow(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(s2i(data.args[1]));
}


/* ----------------------------------------------------------------------------
 * Code for the swallow all mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::swallow_all(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(data.m->chomping_mobs.size());
}


/* ----------------------------------------------------------------------------
 * Code for the teleport to absolute coordinates mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::teleport_to_absolute(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->chase(point(s2f(data.args[0]), s2f(data.args[1])), NULL, true);
    data.m->z = s2f(data.args[2]);
}


/* ----------------------------------------------------------------------------
 * Code for the teleport to relative coordinates mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::teleport_to_relative(mob_action_run_data &data) {
    data.m->stop_chasing();
    point p =
        rotate_point(
            point(s2f(data.args[0]), s2f(data.args[1])),
            data.m->angle
        );
    data.m->chase(data.m->pos + p, NULL, true);
    data.m->z += s2f(data.args[2]);
}


/* ----------------------------------------------------------------------------
 * Code for the turn to an absolute angle mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::turn_to_absolute(mob_action_run_data &data) {
    data.m->face(deg_to_rad(s2f(data.args[0])), NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the turn to a relative angle mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::turn_to_relative(mob_action_run_data &data) {
    data.m->face(data.m->angle + deg_to_rad(s2f(data.args[0])), NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the turn to target mob script action.
 * data:
 *   Data about the action call.
 */
void mob_action_runners::turn_to_target(mob_action_run_data &data) {
    size_t t = s2i(data.args[0]);
    
    switch(t) {
    case MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC: {
        data.m->arachnorb_head_turn_logic();
        break;
        
    } case MOB_ACTION_TURN_FOCUSED_MOB: {
        if(data.m->focused_mob) {
            data.m->face(0, &data.m->focused_mob->pos);
        }
        break;
        
    } case MOB_ACTION_TURN_HOME: {
        data.m->face(get_angle(data.m->pos, data.m->home), NULL);
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Confirms if the "if", "else", "end_if", "goto", and "label" actions in
 * a given vector of actions are all okay, and there are no mismatches, like
 * for instance, an "else" without an "if".
 * Also checks if there are actions past a "set_state" action.
 * If everything is okay, returns true. If not, throws errors to the
 * error log and returns false.
 * actions:
 *   The vector of actions to check.
 * dn:
 *   Data node from where these actions came.
 */
bool assert_actions(
    const vector<mob_action_call*> &actions, data_node* dn
) {
    //Check if the "if"-related actions are okay.
    int if_level = 0;
    for(size_t a = 0; a < actions.size(); ++a) {
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            if_level++;
            break;
        } case MOB_ACTION_ELSE: {
            if(if_level == 0) {
                log_error(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            break;
        } case MOB_ACTION_END_IF: {
            if(if_level == 0) {
                log_error(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            if_level--;
            break;
        }
        }
    }
    if(if_level > 0) {
        log_error(
            "Some \"if\" actions don't have a matching \"end_if\" action!",
            dn
        );
        return false;
    }
    
    //Check if the "goto"-related actions are okay.
    set<string> labels;
    for(size_t a = 0; a < actions.size(); ++a) {
        if(actions[a]->action->type == MOB_ACTION_LABEL) {
        
            string name = actions[a]->args[0];
            
            if(labels.find(name) != labels.end()) {
                log_error(
                    "There are multiple labels called \"" + name + "\"!", dn
                );
                return false;
            }
            
            labels.insert(name);
        }
    }
    for(size_t a = 0; a < actions.size(); ++a) {
        if(actions[a]->action->type == MOB_ACTION_GOTO) {
            string name = actions[a]->args[0];
            if(labels.find(name) == labels.end()) {
                log_error(
                    "There is no label called \"" + name + "\", even though "
                    "there are \"goto\" actions that need it!", dn
                );
                return false;
            }
        }
    }
    
    //Check if there are actions after a "set_state" action.
    bool passed_set_state = false;
    for(size_t a = 0; a < actions.size(); ++a) {
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
                log_error(
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


/* ----------------------------------------------------------------------------
 * Loads the actions to be run when the mob initializes.
 * mt:
 *   The type of mob the actions are going to.
 * node:
 *   The data node.
 * actions:
 *   Vector of actions to be filled.
 */
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action_call*>* actions
) {
    for(size_t a = 0; a < node->get_nr_of_children(); ++a) {
        mob_action_call* new_a = new mob_action_call();
        if(new_a->load_from_data_node(node->get_child(a), mt)) {
            actions->push_back(new_a);
        } else {
            delete new_a;
        }
    }
    assert_actions(*actions, node);
}
