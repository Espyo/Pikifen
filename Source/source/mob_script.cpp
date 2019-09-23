/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script classes and
 * related functions.
 */

//#define DEBUG_FSM

#include <algorithm>
#include <iostream>

#include "mob_script.h"

#include "functions.h"
#include "mob_fsms/gen_mob_fsm.h"
#include "mob_script_action.h"
#include "mob_types/mob_type.h"
#include "mobs/mob.h"
#include "particle.h"
#include "utils/string_utils.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Runs a mob event. Basically runs all actions within.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 */
void mob_event::run(mob* m, void* custom_data_1, void* custom_data_2) {
    if(m->parent && m->parent->relay_events) {
        m->parent->m->fsm.run_event(type, custom_data_1, custom_data_2);
        if(!m->parent->handle_events) {
            return;
        }
    }
    
    for(size_t a = 0; a < actions.size(); ++a) {
    
        if(actions[a]->action->type == MOB_ACTION_IF) {
            //If statement. Look out for its return value, and
            //change the flow accordingly.
            
            if(!actions[a]->run(m, custom_data_1, custom_data_2, type)) {
                //If it returned true, execution continues as normal, but
                //if it returned false, skip to the "else" or "end if" actions.
                size_t next_a = a + 1;
                size_t depth = 0;
                for(; next_a < actions.size(); ++next_a) {
                    if(actions[next_a]->action->type == MOB_ACTION_IF) {
                        depth++;
                    } else if(actions[next_a]->action->type == MOB_ACTION_ELSE) {
                        if(depth == 0) break;
                    } else if(actions[next_a]->action->type == MOB_ACTION_END_IF) {
                        if(depth == 0) break;
                        else depth--;
                    }
                }
                a = next_a;
                
            }
            
        } else if(actions[a]->action->type == MOB_ACTION_ELSE) {
            //If we actually managed to read an "else", that means we were
            //running through the normal execution of a "then" section.
            //Jump to the "end if".
            size_t next_a = a + 1;
            size_t depth = 0;
            for(; next_a < actions.size(); ++next_a) {
                if(actions[next_a]->action->type == MOB_ACTION_IF) {
                    depth++;
                } else if(actions[next_a]->action->type == MOB_ACTION_END_IF) {
                    if(depth == 0) break;
                    else depth--;
                }
            }
            a = next_a;
            
        } else if(actions[a]->action->type == MOB_ACTION_END_IF) {
            //Nothing to do.
            
        } else {
            //Normal action.
            actions[a]->run(m, custom_data_1, custom_data_2, type);
            //If the state got changed, jump out.
            if(actions[a]->action->type == MOB_ACTION_SET_STATE) break;
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns a pointer to an event of the given type in the state,
 * if it exists.
 * type: the event's type.
 */
mob_event* mob_state::get_event(const size_t type) {
    return events[type];
}


/* ----------------------------------------------------------------------------
 * Returns a pointer to an event of the given type in the current state,
 * if it exists.
 * type: the event's type.
 */
mob_event* mob_fsm::get_event(const size_t type) {
    if(!cur_state) return NULL;
    return cur_state->events[type];
}


/* ----------------------------------------------------------------------------
 * Runs an event in the current state, if it exists.
 * type:          the event's type.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_1: custom argument #1 to pass to the code.
 */
void mob_fsm::run_event(
    const size_t type, void* custom_data_1, void* custom_data_2
) {
    mob_event* e = get_event(type);
    if(e) {
        e->run(m, custom_data_1, custom_data_2);
    } else {
    
#ifdef DEBUG_FSM
        cout <<
             "Missing event on run_event() - Mob " <<
             m << ", event " << type << ", state " <<
             (this->cur_state ? this->cur_state->name : "[None]") <<
             endl;
#endif
             
        return;
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new event given a data node.
 * node:    the data node.
 * actions: its actions.
 */
mob_event::mob_event(data_node* node, const vector<mob_action_call*> &actions) :
    actions(actions) {
    
#define r(name, number) \
    else if(n == (name)) type = (number)
    
    string n = node->name;
    if(n == "on_enter") type = MOB_EVENT_ON_ENTER;
    r("on_leave",              MOB_EVENT_ON_LEAVE);
    r("on_tick",               MOB_EVENT_ON_TICK);
    r("on_animation_end",      MOB_EVENT_ANIMATION_END);
    r("on_damage",             MOB_EVENT_DAMAGE);
    r("on_far_from_home",      MOB_EVENT_FAR_FROM_HOME);
    r("on_focus_off_reach",    MOB_EVENT_FOCUS_OFF_REACH);
    r("on_frame_signal",       MOB_EVENT_FRAME_SIGNAL);
    r("on_held",               MOB_EVENT_HELD);
    r("on_hitbox_touch_eat",   MOB_EVENT_HITBOX_TOUCH_EAT);
    r("on_itch",               MOB_EVENT_ITCH);
    r("on_land",               MOB_EVENT_LANDED);
    r("on_object_in_reach",    MOB_EVENT_OBJECT_IN_REACH);
    r("on_opponent_in_reach",  MOB_EVENT_OPPONENT_IN_REACH);
    r("on_pikmin_land",        MOB_EVENT_PIKMIN_LANDED);
    r("on_receive_message",    MOB_EVENT_RECEIVE_MESSAGE);
    r("on_released",           MOB_EVENT_RELEASED);
    r("on_reach_destination",  MOB_EVENT_REACHED_DESTINATION);
    r("on_touch_hazard",       MOB_EVENT_TOUCHED_HAZARD);
    r("on_touch_object",       MOB_EVENT_TOUCHED_OBJECT);
    r("on_touch_opponent",     MOB_EVENT_TOUCHED_OPPONENT);
    r("on_touch_wall",         MOB_EVENT_TOUCHED_WALL);
    r("on_timer",              MOB_EVENT_TIMER);
    else {
        type = MOB_EVENT_UNKNOWN;
        log_error("Unknown script event name \"" + n + "\"!", node);
    }
    
    for(size_t a = 0; a < this->actions.size(); ++a) {
        this->actions[a]->parent_event = (MOB_EVENT_TYPES) type;
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new event.
 * t: the event type.
 * a: its actions.
 */
mob_event::mob_event(const unsigned char t, const vector<mob_action_call*> &a) :
    type(t),
    actions(a) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new state.
 * name: the state's name.
 */
mob_state::mob_state(const string &name) :
    name(name),
    id(INVALID) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = nullptr;
    }
}

/* ----------------------------------------------------------------------------
 * Creates a new state.
 * name: the state's name.
 * e:    its events.
 */
mob_state::mob_state(const string &name, mob_event* evs[N_MOB_EVENTS]) :
    name(name),
    id(INVALID) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = evs[e];
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty state.
 * name: the state's name.
 * id:   its ID, for sorting on the vector of states.
 */
mob_state::mob_state(const string &name, const size_t id) :
    name(name),
    id(id) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
        events[e] = nullptr;
    }
}


/* ----------------------------------------------------------------------------
 * Changes the fsm to use a different state.
 * info*: data to pass on to the code after the state change.
 *   This data comes from the event that started all of this.
 */
void mob_fsm::set_state(const size_t new_state, void* info1, void* info2) {

    //Run the code to leave the current state.
    if(cur_state) {
        for(unsigned char p = STATE_HISTORY_SIZE - 1; p > 0; --p) {
            prev_state_names[p] = prev_state_names[p - 1];
        }
        prev_state_names[0] = cur_state->name;
        run_event(MOB_EVENT_ON_LEAVE, info1, info2);
    }
    
    //Uncomment this to be notified about state changes on stdout.
    /*if(cur_state) {
        cout << "State " << cur_state->name << " -> "
        << m->type->states[new_state]->name << "\n";
    }*/
    
    if(new_state < m->type->states.size() && new_state != INVALID) {
        //Switch states.
        cur_state = m->type->states[new_state];
        
        //Run the code to enter the new state.
        run_event(MOB_EVENT_ON_ENTER, info1, info2);
    }
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob FSM.
 */
mob_fsm::mob_fsm(mob* m) :
    cur_state(nullptr),
    first_state_override(INVALID) {
    
    if(!m) return;
    this->m = m;
}


/* ----------------------------------------------------------------------------
 * Fixes some things in the list of states.
 * For instance, state-switching actions that use
 * a name instead of a number.
 * states:         the vector of states.
 * starting_state: name of the starting state for the mob.
 * Returns the number of the starting state.
 */
size_t fix_states(vector<mob_state*> &states, const string &starting_state) {
    size_t starting_state_nr = INVALID;
    //Fix actions that change the state that are using a string.
    for(size_t s = 0; s < states.size(); ++s) {
        mob_state* state = states[s];
        if(state->name == starting_state) starting_state_nr = s;
        
        for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
            mob_event* ev = state->events[e];
            if(!ev) continue;
            
            for(size_t a = 0; a < ev->actions.size(); ++a) {
                mob_action_call* action = ev->actions[a];
                
                if(
                    action->action->type == MOB_ACTION_SET_STATE &&
                    !action->s_args.empty()
                ) {
                    string state_name = action->s_args[0];
                    size_t state_nr = 0;
                    bool found_state = false;
                    
                    for(; state_nr < states.size(); ++state_nr) {
                        if(states[state_nr]->name == state_name) {
                            found_state = true;
                            break;
                        }
                    }
                    
                    if(!found_state) {
                        state_nr = INVALID;
                        log_error(
                            "State \"" + state->name +
                            "\" has an action to switch "
                            "to an unknown state: \"" + state_name + "\"!",
                            nullptr
                        );
                    }
                    
                    action->s_args.clear();
                    action->i_args.clear();
                    action->i_args.push_back(state_nr);
                    
                }
            }
        }
    }
    return starting_state_nr;
}


/* ----------------------------------------------------------------------------
 * Loads the states off of a data node.
 * mt:     The type of mob the states are going to.
 * node:   The data node.
 * states: Vector of states to place the new states on.
 */
void load_script(mob_type* mt, data_node* node, vector<mob_state*>* states) {
    size_t n_new_states = node->get_nr_of_children();
    size_t old_n_states = states->size();
    
    for(size_t s = 0; s < n_new_states; ++s) {
    
        data_node* state_node = node->get_child(s);
        //Let's save the state now, so that the state switching events
        //can now what numbers the events they need correspond to.
        states->push_back(new mob_state(state_node->name));
    }
    
    for(size_t s = 0; s < n_new_states; ++s) {
        data_node* state_node = node->get_child(s);
        vector<mob_event*> events;
        size_t n_events = state_node->get_nr_of_children();
        
        for(size_t e = 0; e < n_events; ++e) {
        
            data_node* event_node = state_node->get_child(e);
            vector<mob_action_call*> actions;
            
            for(size_t a = 0; a < event_node->get_nr_of_children(); ++a) {
                data_node* action_node = event_node->get_child(a);
                mob_action_call* new_a = new mob_action_call();
                if(new_a->load_from_data_node(action_node, states, mt)) {
                    actions.push_back(new_a);
                } else {
                    delete new_a;
                }
            }
            
            events.push_back(new mob_event(event_node, actions));
            
            assert_if_actions(actions, event_node);
            
        }
        
        //Inject a damage event.
        vector<mob_action_call*> da_actions;
        da_actions.push_back(new mob_action_call(gen_mob_fsm::be_attacked));
        events.push_back(new mob_event(MOB_EVENT_HITBOX_TOUCH_N_A, da_actions));
        
        //Inject a death event.
        if(
            state_node->name != mt->death_state_name &&
            find(
                mt->states_ignoring_death.begin(),
                mt->states_ignoring_death.end(),
                state_node->name
            ) == mt->states_ignoring_death.end() &&
            !mt->death_state_name.empty()
        ) {
            vector<mob_action_call*> de_actions;
            de_actions.push_back(new mob_action_call(gen_mob_fsm::die));
            events.push_back(new mob_event(MOB_EVENT_DEATH, de_actions));
        }
        
        //Inject a bottomless pit event.
        vector<mob_action_call*> bp_actions;
        bp_actions.push_back(new mob_action_call(gen_mob_fsm::fall_down_pit));
        events.push_back(new mob_event(MOB_EVENT_BOTTOMLESS_PIT, bp_actions));
        
        //Inject a spray event.
        if(
            find(
                mt->states_ignoring_spray.begin(),
                mt->states_ignoring_spray.end(),
                state_node->name
            ) == mt->states_ignoring_spray.end()
        ) {
            vector<mob_action_call*> s_actions;
            s_actions.push_back(new mob_action_call(gen_mob_fsm::touch_spray));
            events.push_back(new mob_event(MOB_EVENT_TOUCHED_SPRAY, s_actions));
        }
        
        //Connect the events to the state.
        for(size_t e = 0; e < events.size(); ++e) {
            size_t ev_type = events[e]->type;
            states->at(s + old_n_states)->events[ev_type] = events[e];
        }
        
        states->at(s + old_n_states)->id = s + old_n_states;
        
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the states from memory.
 * mt: the type of mob.
 */
void unload_script(mob_type* mt) {
    for(size_t s = 0; s < mt->states.size(); ++s) {
        mob_state* s_ptr = mt->states[s];
        
        for(size_t e = 0; e < N_MOB_EVENTS; ++e) {
            mob_event* e_ptr = s_ptr->events[e];
            if(!e_ptr) continue;
            
            for(size_t a = 0; a < e_ptr->actions.size(); ++a) {
                delete e_ptr->actions[a];
            }
            
            e_ptr->actions.clear();
            delete e_ptr;
            
        }
        
        delete s_ptr;
        
    }
    mt->states.clear();
}


/* ----------------------------------------------------------------------------
 * Creates the easy fsm creator.
 */
easy_fsm_creator::easy_fsm_creator() :
    cur_state(nullptr),
    cur_event(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Finishes the state that is currently under construction, if any.
 */
void easy_fsm_creator::commit_state() {
    if(!cur_state) return;
    commit_event();
    cur_state = NULL;
}


/* ----------------------------------------------------------------------------
 * Finishes the event that is currently under construction, if any.
 */
void easy_fsm_creator::commit_event() {
    if(!cur_event) return;
    cur_event = NULL;
}


/* ----------------------------------------------------------------------------
 * Finishes the previous state, if any, creates a new state,
 * and starts tracking for the creation of its events.
 */
void easy_fsm_creator::new_state(const string &name, const size_t id) {
    commit_state();
    cur_state = new mob_state(name, id);
    states.push_back(cur_state);
}


/* ----------------------------------------------------------------------------
 * Finishes the previous event, if any, creates a new event for the
 * current state, and starts tracking for the creation of its actions.
 */
void easy_fsm_creator::new_event(const unsigned char type) {
    commit_event();
    cur_event = new mob_event(type);
    cur_state->events[type] = cur_event;
}


/* ----------------------------------------------------------------------------
 * Creates a new action call for the current event, one that changes
 * the mob's state to something else.
 */
void easy_fsm_creator::change_state(const string &new_state) {
    cur_event->actions.push_back(new mob_action_call(MOB_ACTION_SET_STATE));
    cur_event->actions.back()->s_args.push_back(new_state);
}


/* ----------------------------------------------------------------------------
 * Creates a new action for the current event, one that
 * runs some custom code.
 */
void easy_fsm_creator::run(custom_action_code code) {
    cur_event->actions.push_back(new mob_action_call(code));
}


/* ----------------------------------------------------------------------------
 * Finishes any event or state under construction and returns the
 * final vector of states.
 */
vector<mob_state*> easy_fsm_creator::finish() {
    commit_event();
    commit_state();
    sort(
        states.begin(), states.end(),
    [] (mob_state * ms1, mob_state * ms2) -> bool {
        return ms1->id < ms2->id;
    }
    );
    return states;
}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about an event where two hitboxes touch.
 * mob2: the other mob.
 * h1:   the current mob's hitbox.
 * h2:   the other mob's hitbox.
 */
hitbox_interaction::hitbox_interaction(
    mob* mob2, hitbox* h1, hitbox* h2
) {
    this->mob2 = mob2;
    this->h1   = h1;
    this->h2   = h2;
}
