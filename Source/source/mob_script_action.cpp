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

#include "mob_script_action.h"

#include "utils/string_utils.h"
#include "functions.h"


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action call, of a certain type.
 */
mob_action_call::mob_action_call(MOB_ACTION_TYPES type) :
    action(nullptr),
    code(nullptr) {
    
    for(size_t a = 0; a < mob_actions.size(); ++a) {
        if(mob_actions[a].type == type) {
            action = &(mob_actions[a]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action call that is meant to run custom code.
 * code: the function to run.
 */
mob_action_call::mob_action_call(custom_action_code code):
    action(nullptr),
    code(code) {
    
}


/* ----------------------------------------------------------------------------
 * Loads a mob action call from a data node.
 * dn:     the data node.
 * states: if this action messes with states, this points to the external
 *   vector containing the states.
 * mt:     mob type this action's fsm belongs to.
 */
bool mob_action_call::load_from_data_node(
    data_node* dn, vector<mob_state*>* states, mob_type* mt
) {

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
    for(size_t a = 0; a < mob_actions.size(); ++a) {
        if(mob_actions[a].type == MOB_ACTION_UNKNOWN) continue;
        if(mob_actions[a].name == name) {
            action = &(mob_actions[a]);
        }
    }
    
    if(!action) {
        log_error("Unknown script action name \"" + name + "\"!", dn);
        return false;
    }
    
    //Parse the arguments to make sure they're all good.
    
    vector<size_t> enum_arg_s_indexes;
    vector<size_t> enum_arg_i_indexes;
    size_t mandatory_parameters = action->parameters.size();
    
    if(mandatory_parameters > 0) {
        MOB_ACTION_PARAM_TYPE last_type =
            action->parameters[mandatory_parameters - 1].type;
        if(
            last_type == MOB_ACTION_PARAM_FREE_INT_EXTRAS ||
            last_type == MOB_ACTION_PARAM_FREE_FLOAT_EXTRAS ||
            last_type == MOB_ACTION_PARAM_FREE_BOOL_EXTRAS ||
            last_type == MOB_ACTION_PARAM_FREE_STRING_EXTRAS ||
            last_type == MOB_ACTION_PARAM_CONST_INT_EXTRAS ||
            last_type == MOB_ACTION_PARAM_CONST_FLOAT_EXTRAS ||
            last_type == MOB_ACTION_PARAM_CONST_BOOL_EXTRAS ||
            last_type == MOB_ACTION_PARAM_CONST_STRING_EXTRAS ||
            last_type == MOB_ACTION_PARAM_ENUM_EXTRAS
        ) {
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
    
    if(mandatory_parameters != action->parameters.size()) {
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
    
    for(size_t w = 0; w < words.size(); ++w) {
        size_t param_nr = w;
        param_nr = min(param_nr, action->parameters.size() - 1);
        MOB_ACTION_PARAM_TYPE param_type = action->parameters[param_nr].type;
        bool is_var = (words[w][0] == '$' && words[w].size() > 1);
        
        if(is_var) {
            if(
                param_type == MOB_ACTION_PARAM_CONST_INT ||
                param_type == MOB_ACTION_PARAM_CONST_FLOAT ||
                param_type == MOB_ACTION_PARAM_CONST_STRING ||
                param_type == MOB_ACTION_PARAM_CONST_BOOL ||
                param_type == MOB_ACTION_PARAM_CONST_INT_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_FLOAT_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_STRING_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_BOOL_EXTRAS ||
                param_type == MOB_ACTION_PARAM_ENUM ||
                param_type == MOB_ACTION_PARAM_ENUM_EXTRAS
            ) {
                log_error(
                    "Argument #" + i2s(w) + " (\"" + words[w] + "\") is a "
                    "variable, but the parameter \"" +
                    action->parameters[param_nr].name + "\" can only be "
                    "constant!"
                );
                return false;
            }
            
            s_args.push_back(words[w]);
            arg_is_var.push_back(true);
            
        } else {
        
            if(
                param_type == MOB_ACTION_PARAM_FREE_INT ||
                param_type == MOB_ACTION_PARAM_CONST_INT ||
                param_type == MOB_ACTION_PARAM_FREE_INT_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_INT_EXTRAS
            ) {
                i_args.push_back(s2i(words[w]));
                
            } else if(
                param_type == MOB_ACTION_PARAM_FREE_FLOAT ||
                param_type == MOB_ACTION_PARAM_CONST_FLOAT ||
                param_type == MOB_ACTION_PARAM_FREE_FLOAT_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_FLOAT_EXTRAS
            ) {
                f_args.push_back(s2f(words[w]));
                
            } else if(
                param_type == MOB_ACTION_PARAM_FREE_BOOL ||
                param_type == MOB_ACTION_PARAM_CONST_BOOL ||
                param_type == MOB_ACTION_PARAM_FREE_BOOL_EXTRAS ||
                param_type == MOB_ACTION_PARAM_CONST_BOOL_EXTRAS
            ) {
                f_args.push_back(s2b(words[w]));
                
            } else {
            
                if(
                    param_type == MOB_ACTION_PARAM_ENUM ||
                    param_type == MOB_ACTION_PARAM_ENUM_EXTRAS
                ) {
                    enum_arg_s_indexes.push_back(s_args.size());
                    enum_arg_i_indexes.push_back(i_args.size());
                    i_args.push_back(0); //Placeholder.
                }
                
                s_args.push_back(words[w]);
                
            }
            
            arg_is_var.push_back(false);
            
        }
    }
    
    if(action->extra_load_logic) {
        bool success = action->extra_load_logic(*this);
        
        size_t deletions = 0;
        for(size_t e = 0; e < enum_results.size(); ++e) {
            size_t s_nr = enum_arg_s_indexes[e] - deletions;
            
            if(enum_results[e] == INVALID) {
                log_error(
                    "Unknown value for argument \"" +
                    s_args[s_nr] + "\"!", dn
                );
                return false;
            }
            
            s_args.erase(s_args.begin() + s_nr);
            i_args[enum_arg_i_indexes[e]] = enum_results[e];
            deletions++;
        }
        
        if(!custom_error.empty()) {
            log_error(custom_error, dn);
        }
        return success;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Runs an action.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 * Return value is only used by the "if" actions, to indicate their
 *   evaluation result.
 */
bool mob_action_call::run(
    mob* m, void* custom_data_1, void* custom_data_2,
    const size_t parent_event
) {
    //Custom code (i.e. instead of text-based script, use actual code).
    if(code) {
        code(m, custom_data_1, custom_data_2);
        return false;
    }
    
    mob_action_run_data data(m, this);
    //TODO get arguments.
    data.custom_data_1 = custom_data_1;
    data.custom_data_2 = custom_data_2;
    
    return action->code;
}



/* ----------------------------------------------------------------------------
 * Code for the health addition mob script action.
 */
void mob_action_runners::add_health(mob_action_run_data &data) {
    data.m->set_health(true, false, data.f_params[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the arachnorb logic plan mob script action.
 */
void mob_action_runners::arachnorb_plan_logic(mob_action_run_data &data) {
    data.m->arachnorb_plan_logic(data.i_params[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the calculation mob script action.
 */
void mob_action_runners::calculate(mob_action_run_data &data) {
    float lhs, rhs, result;
    if(data.s_params[1].empty()) {
        lhs = data.f_params[0];
    } else {
        lhs = s2f(data.m->vars[data.s_params[1]]);
    }
    
    if(data.s_params[2].empty()) {
        rhs = data.f_params[1];
    } else {
        rhs = s2f(data.m->vars[data.s_params[2]]);
    }
    
    if(data.i_params[0] == MOB_ACTION_SET_VAR_SUM) {
        result = lhs + rhs;
    } else if(data.i_params[0] == MOB_ACTION_SET_VAR_SUBTRACT) {
        result = lhs - rhs;
    } else if(data.i_params[0] == MOB_ACTION_SET_VAR_MULTIPLY) {
        result = lhs * rhs;
    } else if(data.i_params[0] == MOB_ACTION_SET_VAR_DIVIDE) {
        if(rhs == 0) {
            result = 0;
        } else {
            result = lhs / rhs;
        }
    } else {
        if(rhs == 0) {
            result = 0;
        } else {
            result = fmod(lhs, rhs);
        }
    }
    
    data.m->vars[data.s_params[0]] = f2s(result);
}


/* ----------------------------------------------------------------------------
 * Code for the deletion mob script action.
 */
void mob_action_runners::delete_function(mob_action_run_data &data) {
    data.m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Code for the death finish mob script action.
 */
void mob_action_runners::finish_dying(mob_action_run_data &data) {
    data.m->finish_dying();
}


/* ----------------------------------------------------------------------------
 * Code for the focus mob script action.
 */
void mob_action_runners::focus(mob_action_run_data &data) {
    if(data.i_params[0] == MOB_ACTION_FOCUS_PARENT && data.m->parent) {
        data.m->focus_on_mob(data.m->parent->m);
        
    } else if(data.i_params[0] == MOB_ACTION_FOCUS_TRIGGER) {
        if(
            data.call->parent_event == MOB_EVENT_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EVENT_OPPONENT_IN_REACH ||
            data.call->parent_event == MOB_EVENT_PIKMIN_LANDED ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OPPONENT
        ) {
            data.m->focus_on_mob((mob*) (data.custom_data_1));
            
        } else if(
            data.call->parent_event == MOB_EVENT_RECEIVE_MESSAGE
        ) {
            data.m->focus_on_mob((mob*) (data.custom_data_2));
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the mob script action for getting chomped.
 */
void mob_action_runners::get_chomped(mob_action_run_data &data) {
    if(data.call->parent_event == MOB_EVENT_HITBOX_TOUCH_EAT) {
        ((mob*) (data.custom_data_1))->chomp(data.m, (hitbox*) (data.custom_data_2));
    }
}


/* ----------------------------------------------------------------------------
 * Code for the info obtaining mob script action.
 */
void mob_action_runners::get_info(mob_action_run_data &data) {
    string* var = &(data.m->vars[data.s_params[0]]);
    
    if(data.i_params[0] == MOB_ACTION_GET_INFO_CHOMPED_PIKMIN) {
        *var = i2s(data.m->chomping_mobs.size());
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_DAY_MINUTES) {
        *var = i2s(day_minutes);
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_FIELD_PIKMIN) {
        *var = i2s(pikmin_list.size());
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_FRAME_SIGNAL) {
        if(data.call->parent_event == MOB_EVENT_FRAME_SIGNAL) {
            *var = i2s(*((size_t*) (data.custom_data_1)));
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_HEALTH) {
        *var = i2s(data.m->health);
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_LATCHED_PIKMIN) {
        *var = i2s(data.m->get_latched_pikmin_amount());
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_LATCHED_PIKMIN_WEIGHT) {
        *var = i2s(data.m->get_latched_pikmin_weight());
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_MESSAGE) {
        if(data.call->parent_event == MOB_EVENT_RECEIVE_MESSAGE) {
            *var = *((string*) (data.custom_data_1));
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_MESSAGE_SENDER) {
        if(data.call->parent_event == MOB_EVENT_RECEIVE_MESSAGE) {
            *var = ((mob*) (data.custom_data_2))->type->name;
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_MOB_CATEGORY) {
        if(
            data.call->parent_event == MOB_EVENT_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EVENT_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EVENT_OPPONENT_IN_REACH
        ) {
            *var = ((mob*) (data.custom_data_1))->type->category->name;
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_MOB_TYPE) {
        if(
            data.call->parent_event == MOB_EVENT_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EVENT_OBJECT_IN_REACH ||
            data.call->parent_event == MOB_EVENT_OPPONENT_IN_REACH ||
            data.call->parent_event == MOB_EVENT_PIKMIN_LANDED
        ) {
            *var = ((mob*) (data.custom_data_1))->type->name;
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_BODY_PART) {
        if(
            data.call->parent_event == MOB_EVENT_HITBOX_TOUCH_N ||
            data.call->parent_event == MOB_EVENT_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EVENT_DAMAGE
        ) {
            *var =
                (
                    (hitbox_interaction*) (data.custom_data_1)
                )->h1->body_part_name;
        } else if(
            data.call->parent_event == MOB_EVENT_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EVENT_PIKMIN_LANDED
        ) {
            *var =
                data.m->get_closest_hitbox(
                    ((mob*) (data.custom_data_1))->pos,
                    INVALID, NULL
                )->body_part_name;
        }
        
    } else if(data.i_params[0] == MOB_ACTION_GET_INFO_OTHER_BODY_PART) {
        if(
            data.call->parent_event == MOB_EVENT_HITBOX_TOUCH_N ||
            data.call->parent_event == MOB_EVENT_HITBOX_TOUCH_N_A ||
            data.call->parent_event == MOB_EVENT_DAMAGE
        ) {
            *var =
                (
                    (hitbox_interaction*) (data.custom_data_1)
                )->h2->body_part_name;
        } else if(
            data.call->parent_event == MOB_EVENT_TOUCHED_OBJECT ||
            data.call->parent_event == MOB_EVENT_TOUCHED_OPPONENT ||
            data.call->parent_event == MOB_EVENT_PIKMIN_LANDED
        ) {
            *var =
                ((mob*) (data.custom_data_1))->get_closest_hitbox(
                    data.m->pos, INVALID, NULL
                )->body_part_name;
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Code for the "if" mob script action.
 */
void mob_action_runners::if_function(mob_action_run_data &data) {
    string lhs = data.s_params[0];
    string rhs = data.s_params[1];
    
    if(data.i_params[0] == MOB_ACTION_IF_OP_EQUAL) {
        if(is_number(lhs)) {
            data.return_value = (s2f(lhs) == s2f(rhs));
        } else {
            data.return_value = (lhs == rhs);
        }
    } else if(data.i_params[0] == MOB_ACTION_IF_OP_NOT) {
        if(is_number(lhs)) {
            data.return_value = (s2f(lhs) != s2f(rhs));
        } else {
            data.return_value = (lhs != rhs);
        }
    } else if(data.i_params[0] == MOB_ACTION_IF_OP_LESS) {
        data.return_value = (s2f(lhs) < s2f(rhs));
    } else if(data.i_params[0] == MOB_ACTION_IF_OP_MORE) {
        data.return_value = (s2f(lhs) > s2f(rhs));
    } else if(data.i_params[0] == MOB_ACTION_IF_OP_LESS_E) {
        data.return_value = (s2f(lhs) <= s2f(rhs));
    } else if(data.i_params[0] == MOB_ACTION_IF_OP_MORE_E) {
        data.return_value = (s2f(lhs) >= s2f(rhs));
    }
}


/* ----------------------------------------------------------------------------
 * Code for the move to absolute coordinates mob script action.
 */
void mob_action_runners::move_to_absolute(mob_action_run_data &data) {
    data.m->chase(point(data.f_params[0], data.f_params[1]), NULL, false);
}


/* ----------------------------------------------------------------------------
 * Code for the move to relative coordinates mob script action.
 */
void mob_action_runners::move_to_relative(mob_action_run_data &data) {
    point p = rotate_point(point(data.f_params[0], data.f_params[1]), data.m->angle);
    data.m->chase(data.m->pos + p, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Code for the move to target mob script action.
 */
void mob_action_runners::move_to_target(mob_action_run_data &data) {
    if(data.i_params[0] == MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB) {
        if(data.m->focused_mob) {
            float a = get_angle(data.m->pos, data.m->focused_mob->pos);
            point offset = point(2000, 0);
            offset = rotate_point(offset, a + TAU / 2.0);
            data.m->chase(data.m->pos + offset, NULL, false);
        } else {
            data.m->stop_chasing();
        }
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_FOCUSED_MOB) {
        if(data.m->focused_mob) {
            data.m->chase(point(), &data.m->focused_mob->pos, false);
        } else {
            data.m->stop_chasing();
        }
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_FOCUSED_MOB_POS) {
        if(data.m->focused_mob) {
            data.m->chase(data.m->focused_mob->pos, NULL, false);
        } else {
            data.m->stop_chasing();
        }
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_HOME) {
        data.m->chase(data.m->home, NULL, false);
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC) {
        data.m->arachnorb_foot_move_logic();
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_LINKED_MOB_AVERAGE) {
        if(data.m->links.empty()) {
            data.return_value = false;
            return;
        }
        
        point des;
        for(size_t l = 0; l < data.m->links.size(); ++l) {
            des += data.m->links[l]->pos;
        }
        des = des / data.m->links.size();
        
        data.m->chase(des, NULL, false);
        
    } else if(data.i_params[0] == MOB_ACTION_MOVE_RANDOMLY) {
        data.m->chase(
            point(
                data.m->pos.x + randomf(-1000, 1000),
                data.m->pos.y + randomf(-1000, 1000)
            ),
            NULL, false
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Code for the release order mob script action.
 */
void mob_action_runners::order_release(mob_action_run_data &data) {
    if(data.m->holder.m) {
        data.m->holder.m->fsm.run_event(MOB_EVENT_RELEASE_ORDER, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the sound playing mob script action.
 */
void mob_action_runners::play_sound(mob_action_run_data &data) {

}


/* ----------------------------------------------------------------------------
 * Code for the timer randomization mob script action.
 */
void mob_action_runners::randomize_timer(mob_action_run_data &data) {
    data.m->set_timer(randomf(data.f_params[0], data.f_params[1]));
}


/* ----------------------------------------------------------------------------
 * Code for the var randomization mob script action.
 */
void mob_action_runners::randomize_var(mob_action_run_data &data) {
    data.m->vars[data.s_params[0]] = i2s(randomi(data.i_params[0], data.i_params[1]));
}


/* ----------------------------------------------------------------------------
 * Code for the status reception mob script action.
 */
void mob_action_runners::receive_status(mob_action_run_data &data) {
    data.m->apply_status_effect(&status_types[data.s_params[0]], true, false);
}


/* ----------------------------------------------------------------------------
 * Code for the release mob script action.
 */
void mob_action_runners::release(mob_action_run_data &data) {
    data.m->release_chomped_pikmin();
}


/* ----------------------------------------------------------------------------
 * Code for the status removal mob script action.
 */
void mob_action_runners::remove_status(mob_action_run_data &data) {
    for(size_t s = 0; s < data.m->statuses.size(); ++s) {
        if(data.m->statuses[s].type->name == data.s_params[0]) {
            data.m->statuses[s].to_delete = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the linked mob message sending mob script action.
 */
void mob_action_runners::send_message_to_links(mob_action_run_data &data) {
    for(size_t l = 0; l < data.m->links.size(); ++l) {
        if(data.m->links[l] == data.m) continue;
        data.m->send_message(data.m->links[l], data.s_params[0]);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the nearby mob message sending mob script action.
 */
void mob_action_runners::send_message_to_nearby(mob_action_run_data &data) {
    for(size_t m2 = 0; m2 < mobs.size(); ++m2) {
        if(mobs[m2] == data.m) continue;
        if(dist(data.m->pos, mobs[m2]->pos) > data.f_params[0]) continue;
        data.m->send_message(mobs[m2], data.s_params[0]);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the animation setting mob script action.
 */
void mob_action_runners::set_animation(mob_action_run_data &data) {
    data.m->set_animation(
        data.i_params[0],
        false,
        data.i_params[1] != MOB_ACTION_SET_ANIMATION_NO_RESTART
    );
}


/* ----------------------------------------------------------------------------
 * Code for the far reach setting mob script action.
 */
void mob_action_runners::set_far_reach(mob_action_run_data &data) {
    data.m->far_reach = data.i_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the gravity setting mob script action.
 */
void mob_action_runners::set_gravity(mob_action_run_data &data) {
    data.m->gravity_mult = data.f_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the health setting mob script action.
 */
void mob_action_runners::set_health(mob_action_run_data &data) {
    data.m->set_health(false, false, data.f_params[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the hiding setting mob script action.
 */
void mob_action_runners::set_hiding(mob_action_run_data &data) {
    data.m->hide = data.i_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the holdable setting mob script action.
 */
void mob_action_runners::set_holdable(mob_action_run_data &data) {
    if(typeid(*(data.m)) == typeid(tool)) {
        size_t flags = 0;
        for(size_t i = 0; i < data.i_params.size(); ++i) {
            flags |= data.i_params[i];
        }
        ((tool*) (data.m))->holdability_flags = flags;
    }
}


/* ----------------------------------------------------------------------------
 * Code for the limb animation setting mob script action.
 */
void mob_action_runners::set_limb_animation(mob_action_run_data &data) {
    if(!data.m->parent) {
        data.return_value = false;
        return;
    }
    if(!data.m->parent->limb_anim.anim_db) {
        data.return_value = false;
        return;
    }
    
    size_t a = data.m->parent->limb_anim.anim_db->find_animation(data.s_params[0]);
    if(a == INVALID) {
        data.return_value = false;
        return;
    }
    
    data.m->parent->limb_anim.cur_anim =
        data.m->parent->limb_anim.anim_db->animations[a];
    data.m->parent->limb_anim.start();
    
}


/* ----------------------------------------------------------------------------
 * Code for the near reach setting mob script action.
 */
void mob_action_runners::set_near_reach(mob_action_run_data &data) {
    data.m->near_reach = data.i_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the state setting mob script action.
 */
void mob_action_runners::set_state(mob_action_run_data &data) {
    data.m->fsm.set_state(
        data.i_params[0],
        data.custom_data_1,
        data.custom_data_2
    );
}


/* ----------------------------------------------------------------------------
 * Code for the tangible setting mob script action.
 */
void mob_action_runners::set_tangible(mob_action_run_data &data) {
    data.m->tangible = (bool) data.i_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the team setting mob script action.
 */
void mob_action_runners::set_team(mob_action_run_data &data) {
    data.m->team = data.i_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the timer setting mob script action.
 */
void mob_action_runners::set_timer(mob_action_run_data &data) {
    data.m->set_timer(data.f_params[0]);
}


/* ----------------------------------------------------------------------------
 * Code for the var setting mob script action.
 */
void mob_action_runners::set_var(mob_action_run_data &data) {
    data.m->set_var(data.s_params[0], data.s_params[1]);
}


/* ----------------------------------------------------------------------------
 * Code for the show message from var mob script action.
 */
void mob_action_runners::show_message_from_var(mob_action_run_data &data) {
    start_message(data.m->vars[data.s_params[0]], NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the spawning mob script action.
 */
void mob_action_runners::spawn(mob_action_run_data &data) {
    data.return_value = data.m->spawn(&data.m->type->spawns[data.i_params[0]]);
}


/* ----------------------------------------------------------------------------
 * Code for the z stabilization mob script action.
 */
void mob_action_runners::stabilize_z(mob_action_run_data &data) {
    if(data.m->links.empty()) {
        data.return_value = false;
        return;
    }
    
    float best_match_z = data.m->links[0]->z;
    for(size_t l = 1; l < data.m->links.size(); ++l) {
        if(
            data.i_params[0] == MOB_ACTION_STABILIZE_Z_HIGHEST &&
            data.m->links[l]->z > best_match_z
        ) {
            best_match_z = data.m->links[l]->z;
        } else if(
            data.i_params[0] == MOB_ACTION_STABILIZE_Z_LOWEST &&
            data.m->links[l]->z < best_match_z
        ) {
            best_match_z = data.m->links[l]->z;
        }
    }
    
    data.m->z = best_match_z + data.f_params[0];
}


/* ----------------------------------------------------------------------------
 * Code for the chomping start mob script action.
 */
void mob_action_runners::start_chomping(mob_action_run_data &data) {
    data.m->chomp_max = data.i_params[0];
    data.m->chomp_body_parts.clear();
    for(size_t p = 1; p < data.i_params.size(); ++p) {
        data.m->chomp_body_parts.push_back(data.i_params[p]);
    }
}


/* ----------------------------------------------------------------------------
 * Code for the dying start mob script action.
 */
void mob_action_runners::start_dying(mob_action_run_data &data) {
    data.m->start_dying();
}


/* ----------------------------------------------------------------------------
 * Code for the height effect start mob script action.
 */
void mob_action_runners::start_height_effect(mob_action_run_data &data) {
    data.m->start_height_effect();
}


/* ----------------------------------------------------------------------------
 * Code for the particle start mob script action.
 */
void mob_action_runners::start_particles(mob_action_run_data &data) {
    if(data.s_params.empty()) {
        data.m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
    } else {
        if(
            custom_particle_generators.find(data.s_params[0]) !=
            custom_particle_generators.end()
        ) {
            particle_generator pg = custom_particle_generators[data.s_params[0]];
            pg.id = MOB_PARTICLE_GENERATOR_SCRIPT;
            pg.follow_mob = data.m;
            pg.follow_angle = &data.m->angle;
            pg.follow_pos_offset = point(data.f_params[0], data.f_params[1]);
            pg.follow_z_offset = data.f_params[2];
            pg.reset();
            data.m->particle_generators.push_back(pg);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code for the stopping mob script action.
 */
void mob_action_runners::stop(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->stop_turning();
}


/* ----------------------------------------------------------------------------
 * Code for the chomp stopping mob script action.
 */
void mob_action_runners::stop_chomping(mob_action_run_data &data) {
    data.m->chomp_max = 0;
    data.m->chomp_body_parts.clear();
}


/* ----------------------------------------------------------------------------
 * Code for the height effect stopping mob script action.
 */
void mob_action_runners::stop_height_effect(mob_action_run_data &data) {
    data.m->stop_height_effect();
}


/* ----------------------------------------------------------------------------
 * Code for the particle stopping mob script action.
 */
void mob_action_runners::stop_particles(mob_action_run_data &data) {
    data.m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
}

/* ----------------------------------------------------------------------------
 * Code for the vertical stopping mob script action.
 */
void mob_action_runners::stop_vertically(mob_action_run_data &data) {
    data.m->speed_z = 0;
}


/* ----------------------------------------------------------------------------
 * Code for the swallow mob script action.
 */
void mob_action_runners::swallow(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(data.i_params[1]);
}


/* ----------------------------------------------------------------------------
 * Code for the swallow all mob script action.
 */
void mob_action_runners::swallow_all(mob_action_run_data &data) {
    data.m->swallow_chomped_pikmin(data.m->chomping_mobs.size());
}


/* ----------------------------------------------------------------------------
 * Code for the teleport to absolute coordinates mob script action.
 */
void mob_action_runners::teleport_to_absolute(mob_action_run_data &data) {
    data.m->stop_chasing();
    data.m->chase(point(data.f_params[0], data.f_params[1]), NULL, true);
    data.m->z = data.f_params[2];
}


/* ----------------------------------------------------------------------------
 * Code for the teleport to relative coordinates mob script action.
 */
void mob_action_runners::teleport_to_relative(mob_action_run_data &data) {
    data.m->stop_chasing();
    point p = rotate_point(point(data.f_params[0], data.f_params[1]), data.m->angle);
    data.m->chase(data.m->pos + p, NULL, true);
    data.m->z += data.f_params[2];
}


/* ----------------------------------------------------------------------------
 * Code for the turn to an absolute angle mob script action.
 */
void mob_action_runners::turn_to_absolute(mob_action_run_data &data) {
    data.m->face(data.f_params[0], NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the turn to a relative angle mob script action.
 */
void mob_action_runners::turn_to_relative(mob_action_run_data &data) {
    data.m->face(data.m->angle + data.f_params[0], NULL);
}


/* ----------------------------------------------------------------------------
 * Code for the turn to target mob script action.
 */
void mob_action_runners::turn_to_target(mob_action_run_data &data) {
    if(data.i_params[0] == MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC) {
        data.m->arachnorb_head_turn_logic();
    } else if(data.i_params[0] == MOB_ACTION_TURN_FOCUSED_MOB && data.m->focused_mob) {
        data.m->face(0, &data.m->focused_mob->pos);
    } else if(data.i_params[0] == MOB_ACTION_TURN_HOME) {
        data.m->face(get_angle(data.m->pos, data.m->home), NULL);
    } else if(data.i_params[0] == MOB_ACTION_TURN_RANDOMLY) {
        data.m->face(randomf(0, TAU), NULL);
    }
}


/* ----------------------------------------------------------------------------
 * Loading code for the arachnorb logic plan mob script action.
 */
bool mob_action_loaders::arachnorb_plan_logic(mob_action_call &call) {
    if(call.s_args[0] == "home") {
        call.enum_results.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME);
    } else if(call.s_args[0] == "forward") {
        call.enum_results.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD);
    } else if(call.s_args[0] == "cw_turn") {
        call.enum_results.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN);
    } else if(call.s_args[0] == "ccw_turn") {
        call.enum_results.push_back(MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the calculation mob script action.
 */
bool mob_action_loaders::calculate(mob_action_call &call) {
    if(call.s_args[1] == "+") {
        call.enum_results.push_back(MOB_ACTION_SET_VAR_SUM);
    } else if(call.s_args[1] == "-") {
        call.enum_results.push_back(MOB_ACTION_SET_VAR_SUBTRACT);
    } else if(call.s_args[1] == "*") {
        call.enum_results.push_back(MOB_ACTION_SET_VAR_MULTIPLY);
    } else if(call.s_args[1] == "/") {
        call.enum_results.push_back(MOB_ACTION_SET_VAR_DIVIDE);
    } else if(call.s_args[1] == "%") {
        call.enum_results.push_back(MOB_ACTION_SET_VAR_MODULO);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the focus mob script action.
 */
bool mob_action_loaders::focus(mob_action_call &call) {
    if(call.s_args[0] == "parent") {
        call.enum_results.push_back(MOB_ACTION_FOCUS_PARENT);
    } else if(call.s_args[0] == "trigger") {
        call.enum_results.push_back(MOB_ACTION_FOCUS_TRIGGER);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the "if" mob script action.
 */
bool mob_action_loaders::if_function(mob_action_call &call) {
    if(call.s_args[1] == "=") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_EQUAL);
    } else if(call.s_args[1] == "!=") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_NOT);
    } else if(call.s_args[1] == "<") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_LESS);
    } else if(call.s_args[1] == ">") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_MORE);
    } else if(call.s_args[1] == "<=") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_LESS_E);
    } else if(call.s_args[1] == ">=") {
        call.enum_results.push_back(MOB_ACTION_IF_OP_MORE_E);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the move to target mob script action.
 */
bool mob_action_loaders::move_to_target(mob_action_call &call) {
    if(call.s_args[0] == "arachnorb_foot_logic") {
        call.enum_results.push_back(MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC);
    } else if(call.s_args[0] == "away_from_focused_mob") {
        call.enum_results.push_back(MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB);
    } else if(call.s_args[0] == "focused_mob") {
        call.enum_results.push_back(MOB_ACTION_MOVE_FOCUSED_MOB);
    } else if(call.s_args[0] == "focused_mob_position") {
        call.enum_results.push_back(MOB_ACTION_MOVE_FOCUSED_MOB_POS);
    } else if(call.s_args[0] == "home") {
        call.enum_results.push_back(MOB_ACTION_MOVE_HOME);
    } else if(call.s_args[0] == "linked_mob_average") {
        call.enum_results.push_back(MOB_ACTION_MOVE_LINKED_MOB_AVERAGE);
    } else if(call.s_args[0] == "randomly") {
        call.enum_results.push_back(MOB_ACTION_MOVE_RANDOMLY);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the status reception mob script action.
 */
bool mob_action_loaders::receive_status(mob_action_call &call) {
    if(status_types.find(call.s_args[0]) == status_types.end()) {
        call.custom_error =
            "Unknown status effect \"" + call.s_args[0] + "\"!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the status removal mob script action.
 */
bool mob_action_loaders::remove_status(mob_action_call &call) {
    if(status_types.find(call.s_args[0]) == status_types.end()) {
        call.custom_error =
            "Unknown status effect \"" + call.s_args[0] + "\"!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the animation setting mob script action.
 */
bool mob_action_loaders::set_animation(mob_action_call &call) {
    size_t f_pos = call.mt->anims.find_animation(call.s_args[0]);
    if(f_pos == INVALID) {
        call.custom_error =
            "Unknown animation \"" + call.s_args[0] + "\"!";
        return false;
    }
    
    for(size_t s = 1; s < call.s_args.size(); ++s) {
        if(call.s_args[s] == "no_restart") {
            call.enum_results.push_back(MOB_ACTION_SET_ANIMATION_NO_RESTART);
        } else {
            call.enum_results.push_back(0);
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the far reach setting mob script action.
 */
bool mob_action_loaders::set_far_reach(mob_action_call &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); ++r) {
        if(call.mt->reaches[r].name == call.s_args[0]) {
            call.enum_results.push_back(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.s_args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the holdable setting mob script action.
 */
bool mob_action_loaders::set_holdable(mob_action_call &call) {
    bool ok = true;
    for(size_t s = 0; s < call.s_args.size(); ++s) {
        if(call.s_args[s] == "pikmin") {
            call.enum_results.push_back(HOLDABLE_BY_PIKMIN);
        } else if(call.s_args[s] == "enemies") {
            call.enum_results.push_back(HOLDABLE_BY_ENEMIES);
        } else {
            call.enum_results.push_back(INVALID);
            ok = false;
        }
    }
    
    return ok;
}


/* ----------------------------------------------------------------------------
 * Loading code for the near reach setting mob script action.
 */
bool mob_action_loaders::set_near_reach(mob_action_call &call) {
    for(size_t r = 0; r < call.mt->reaches.size(); ++r) {
        if(call.mt->reaches[r].name == call.s_args[0]) {
            call.enum_results.push_back(r);
            return true;
        }
    }
    call.custom_error = "Unknown reach \"" + call.s_args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the team setting mob script action.
 */
bool mob_action_loaders::set_team(mob_action_call &call) {
    size_t team_nr = string_to_team_nr(call.s_args[0]);
    call.enum_results.push_back(team_nr);
    return (team_nr != INVALID);
}


/* ----------------------------------------------------------------------------
 * Loading code for the spawning mob script action.
 */
bool mob_action_loaders::spawn(mob_action_call &call) {
    for(size_t s = 0; s < call.mt->spawns.size(); ++s) {
        if(call.mt->spawns[s].name == call.s_args[0]) {
            call.enum_results.push_back(s);
            return true;
        }
    }
    call.custom_error = "Unknown spawn info block \"" + call.s_args[0] + "\"!";
    return false;
}


/* ----------------------------------------------------------------------------
 * Loading code for the z stabilization mob script action.
 */
bool mob_action_loaders::stabilize_z(mob_action_call &call) {
    if(call.s_args[0] == "lowest") {
        call.enum_results.push_back(MOB_ACTION_STABILIZE_Z_LOWEST);
    } else if(call.s_args[0] == "highest") {
        call.enum_results.push_back(MOB_ACTION_STABILIZE_Z_HIGHEST);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the chomping start mob script action.
 */
bool mob_action_loaders::start_chomping(mob_action_call &call) {
    bool ok = true;
    for(size_t s = 0; s < call.s_args.size(); ++s) {
        size_t p_nr = call.mt->anims.find_body_part(call.s_args[s]);
        if(p_nr == INVALID) {
            ok = false;
        }
        call.enum_results.push_back(p_nr);
    }
    
    return ok;
}


/* ----------------------------------------------------------------------------
 * Loading code for the particle start mob script action.
 */
bool mob_action_loaders::start_particles(mob_action_call &call) {
    if(
        custom_particle_generators.find(call.s_args[0]) ==
        custom_particle_generators.end()
    ) {
        call.custom_error =
            "Particle generator \"" + call.s_args[0] + "\" not found!";
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loading code for the turn to target mob script action.
 */
bool mob_action_loaders::turn_to_target(mob_action_call &call) {
    if(call.s_args[0] == "arachnorb_head_logic") {
        call.enum_results.push_back(MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC);
    } else if(call.s_args[0] == "focused_mob") {
        call.enum_results.push_back(MOB_ACTION_TURN_FOCUSED_MOB);
    } else if(call.s_args[0] == "home") {
        call.enum_results.push_back(MOB_ACTION_TURN_HOME);
    } else if(call.s_args[0] == "randomly") {
        call.enum_results.push_back(MOB_ACTION_TURN_RANDOMLY);
    } else {
        call.enum_results.push_back(INVALID);
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action parameter struct.
 */
mob_action_param::mob_action_param(
    const MOB_ACTION_PARAM_TYPE type, const string &name
):
    type(type),
    name(name) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action.
 */
mob_action::mob_action() :
    type(MOB_ACTION_UNKNOWN),
    code(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action run data struct.
 */
mob_action_run_data::mob_action_run_data(mob* m, mob_action_call* call) :
    m(m),
    call(call),
    i_params(call->i_args),
    f_params(call->f_args),
    s_params(call->s_args),
    custom_data_1(nullptr),
    custom_data_2(nullptr),
    return_value(false) {
    
}


/* ----------------------------------------------------------------------------
 * Confirms if the "if", "else", and "end_if" actions in a given vector of
 * actions are all okay, and there are no mismatches, like for instance,
 * an "else" without an "if".
 * If everything is okay, returns true. If not, throws errors to the
 * error log and returns false.
 * actions: The vector of actions to check.
 * dn:      Data node from where these actions came.
 */
bool assert_if_actions(const vector<mob_action_call*> &actions, data_node* dn) {
    int level = 0;
    for(size_t a = 0; a < actions.size(); ++a) {
        if(actions[a]->action->type == MOB_ACTION_IF) {
            level++;
        } else if(actions[a]->action->type == MOB_ACTION_ELSE) {
            if(level == 0) {
                log_error(
                    "Found an \"else\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
        } else if(actions[a]->action->type == MOB_ACTION_END_IF) {
            if(level == 0) {
                log_error(
                    "Found an \"end_if\" action without a matching "
                    "\"if\" action!", dn
                );
                return false;
            }
            level--;
        }
    }
    if(level > 0) {
        log_error(
            "Some \"if\" actions don't have a matching \"end_if\" action!",
            dn
        );
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Loads the actions to be run when the mob initializes.
 * mt:      The type of mob the actions are going to.
 * node:    The data node.
 * actions: Vector of actions to be filled.
 */
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action_call*>* actions
) {
    for(size_t a = 0; a < node->get_nr_of_children(); ++a) {
        mob_action_call* new_a = new mob_action_call();
        if(new_a->load_from_data_node(node->get_child(a), NULL, mt)) {
            actions->push_back(new_a);
        } else {
            delete new_a;
        }
    }
    assert_if_actions(*actions, node);
}
