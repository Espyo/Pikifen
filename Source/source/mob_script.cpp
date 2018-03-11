/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script classes and
 * related functions.
 */

//#define DEBUG_FSM

#include <algorithm>
#include <iostream>

#include "functions.h"
#include "mobs/mob.h"
#include "mobs/mob_fsm.h"
#include "mob_script.h"
#include "mobs/mob_type.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a new mob action, given a data node.
 * dn:     the data node.
 * states: if this action messes with states, this points to the external
 *   vector containing the states.
 * mt:     mob type this action's fsm belongs to.
 */
mob_action::mob_action(
    data_node* dn, vector<mob_state*>* states, mob_type* mt
) :
    type(MOB_ACTION_UNKNOWN),
    sub_type(0),
    code(nullptr),
    valid(true) {
    
    vector<string> words = split(dn->name);
    string n = words[0];
    string v = "";
    if(words.size() > 1) {
        v = dn->name.substr(n.size(), string::npos);
        v = trim_spaces(v);
    }
    
    if(words[0] == "chomp") {
    
        type = MOB_ACTION_CHOMP_HITBOXES;
        
        words.erase(words.begin());
        if(!words.empty()) {
            //The first one is actually the number of Pikmin it can eat at most.
            vf.push_back(s2f(words[0]));
            words.erase(words.begin());
        }
        
        for(size_t hn = 0; hn < words.size(); ++hn) {
            size_t h_pos = mt->anims.find_body_part(words[hn]);
            
            if(h_pos == INVALID) {
                log_error("Unknown hitbox \"" + words[hn] + "\"!", dn);
                valid = false;
            } else {
                vi.push_back(h_pos);
            }
        }
        
        
    } else if(words[0] == "eat") {
    
        type = MOB_ACTION_EAT;
        
        if(v == "all") {
            sub_type = MOB_ACTION_EAT_ALL;
        } else {
            sub_type = MOB_ACTION_EAT_NUMBER;
            vi.push_back(s2i(v));
        }
        
        
    } else if(words[0] == "else") {
    
        type = MOB_ACTION_ELSE;
        
        
    } else if(words[0] == "endif") {
    
        type = MOB_ACTION_ENDIF;
        
        
    } else if(words[0] == "focus") {
    
        type = MOB_ACTION_FOCUS;
        
        
    } else if(words[0] == "hide") {
    
        type = MOB_ACTION_HIDE;
        
        if(words.size() == 1) {
            log_error("The hide action requires a true or false value!", dn);
            valid = false;
        } else {
            vi.push_back(s2b(words[1]));
        }
        
        
    } else if(words[0] == "if") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF;
        
        words.erase(words.begin());
        if(words.size() < 2) {
            log_error(
                "Not enough parts on this if: \"" + v + "\"!",
                dn
            );
            valid = false;
        } else {
            vs.push_back(words[0]); vs.push_back(words[1]);
        }
        
        
    } else if(words[0] == "if_less") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF_LESS;
        
        words.erase(words.begin());
        if(words.size() < 2) {
            log_error(
                "Not enough parts on this if_less: \"" + v + "\"!",
                dn
            );
            valid = false;
        } else {
            vs.push_back(words[0]); vs.push_back(words[1]);
        }
        
        
    } else if(words[0] == "if_more") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF_MORE;
        
        words.erase(words.begin());
        if(words.size() < 2) {
            log_error(
                "Not enough parts on this if_more: \"" + v + "\"!",
                dn
            );
            valid = false;
        } else {
            vs.push_back(words[0]); vs.push_back(words[1]);
        }
        
        
    } else if(words[0] == "if_not") {
    
        //TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF_NOT;
        
        words.erase(words.begin());
        if(words.size() < 2) {
            log_error(
                "Not enough parts on this if_not: \"" + v + "\"!",
                dn
            );
            valid = false;
        } else {
            vs.push_back(words[0]); vs.push_back(words[1]);
        }
        
        
    } else if(words[0] == "move") {
    
        type = MOB_ACTION_MOVE;
        
        if(v == "focused_mob") {
            sub_type = MOB_ACTION_MOVE_FOCUSED_MOB;
        } else if(v == "home") {
            sub_type = MOB_ACTION_MOVE_HOME;
        } else if(v == "stop") {
            sub_type = MOB_ACTION_MOVE_STOP;
        } else if(v == "stop vertically") {
            sub_type = MOB_ACTION_MOVE_STOP_VERTICALLY;
        } else {
        
            words.erase(words.begin());
            
            if(words.empty()) valid = false;
            else {
                if(words[0] == "vertically") {
                    sub_type = MOB_ACTION_MOVE_VERTICALLY;
                    if(words.size() < 2) valid = false;
                    else {
                        vf.push_back(s2f(words[1]));
                    }
                    
                } else if(words[0] == "randomly") {
                    sub_type = MOB_ACTION_MOVE_RANDOMLY;
                    
                } else if(words[0] == "relative") {
                    sub_type = MOB_ACTION_MOVE_REL_COORDS;
                    if(words.size() < 3) valid = false;
                    else {
                        for(size_t sc = 1; sc < words.size(); ++sc) {
                            vf.push_back(s2f(words[sc]));
                        }
                    }
                    
                } else {
                    sub_type = MOB_ACTION_MOVE_COORDS;
                    for(size_t sc = 0; sc < words.size(); ++sc) {
                        vf.push_back(s2f(words[sc]));
                    }
                }
            }
            
            if(!valid) {
                log_error("Invalid move location \"" + v + "\"!", dn);
            }
        }
        
        
    } else if(n == "play_sound") {
    
        type = MOB_ACTION_PLAY_SOUND;
        
        
    } else if(n == "animation") {
    
        type = MOB_ACTION_SET_ANIMATION;
        
        size_t f_pos = mt->anims.find_animation(v);
        if(f_pos == INVALID) {
            log_error("Unknown animation \"" + v + "\"!", dn);
            valid = false;
        } else {
            vi.push_back(f_pos);
        }
        
        
    } else if(n == "gravity") {
    
        type = MOB_ACTION_SET_GRAVITY;
        
        vf.push_back(s2f(v));
        
        
    } else if(n == "health") {
    
        type = MOB_ACTION_SET_HEALTH;
        
        words.erase(words.begin());
        if(words.empty()) {
            valid = false;
        } else {
            if(words[0] == "relative") {
                if(words.size() < 2) {
                    valid = false;
                } else {
                    sub_type = MOB_ACTION_SET_HEALTH_RELATIVE;
                    vf.push_back(s2f(words[1]));
                }
            } else {
                sub_type = MOB_ACTION_SET_HEALTH_ABSOLUTE;
                vf.push_back(s2f(words[0]));
            }
        }
        
        if(!valid) {
            log_error("Invalid health data \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "speed") {
    
        type = MOB_ACTION_SET_SPEED;
        
        //TODO
        
        
    } else if(n == "state" && states) {
    
        type = MOB_ACTION_CHANGE_STATE;
        
        for(size_t s = 0; s < states->size(); ++s) {
            if(states->at(s)->name == v) {
                vi.push_back(s);
                break;
            }
        }
        if(vi.empty()) {
            log_error("Unknown state \"" + v + "\"!", dn);
        }
        
        
    } else if(n == "timer") {
    
        type = MOB_ACTION_SET_TIMER;
        
        vf.push_back(s2f(v));
        
        
    } else if(n == "far_reach" || n == "near_reach") {
    
        if(n == "far_reach") {
            type = MOB_ACTION_SET_FAR_REACH;
        } else {
            type = MOB_ACTION_SET_NEAR_REACH;
        }
        
        if(v.empty()) {
            vi.push_back(INVALID);
            return;
        }
        
        for(size_t r = 0; r < mt->reaches.size(); ++r) {
            if(mt->reaches[r].name == v) {
                vi.push_back(r);
                return;
            }
        }
        
        log_error(
            "Reach-setting action refers to non-existent reach \"" +
            v + "\"!", dn
        );
        valid = false;
        
        
    } else if(n == "var") {
    
        type = MOB_ACTION_SET_VAR;
        
        words.erase(words.begin());
        if(words.size() < 2) {
            log_error("Not enough info to set a variable!", dn);
            valid = false;
        } else {
            vs = words;
        }
        
        
    } else if(n == "inc_var") {
    
        type = MOB_ACTION_INC_VAR;
        
        words.erase(words.begin());
        if(words.empty()) {
            log_error("Not enough info to increment a variable!", dn);
            valid = false;
        } else {
            vs = words;
        }
        
        
    } else if(n == "particle") {
    
        type = MOB_ACTION_PARTICLE;
        if(!v.empty()) {
            if(
                custom_particle_generators.find(v) ==
                custom_particle_generators.end()
            ) {
                log_error(
                    "Particle generator \"" + v + "\" not found!", dn
                );
            } else {
                vs.push_back(v);
            }
        }
        
        
    } else if(n == "projectile") {
    
        type = MOB_ACTION_SPAWN_PROJECTILE;
        
        
    } else if(n == "special_function") {
    
        type = MOB_ACTION_SPECIAL_FUNCTION;
        
        if(v == "die_start") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_DIE_START;
        } else if(v == "die_end") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_DIE_END;
        } else if(v == "delete") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_DELETE;
        } else if(v == "hazard") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_HAZARD;
        } else if(v == "spray") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_SPRAY;
        } else {
            log_error("Unknown special function \"" + v + "\"!", dn);
            valid = false;
        }
        
        
    } else if(n == "turn") {
    
        type = MOB_ACTION_TURN;
        
        
    } else if(n == "wait") {
    
        type = MOB_ACTION_WAIT;
        
        if(v == "animation") {
            sub_type = MOB_ACTION_WAIT_ANIMATION;
        } else {
            sub_type = MOB_ACTION_WAIT_TIME;
            vf.push_back(s2f(v));
        }
        
        
    } else {
    
        type = MOB_ACTION_UNKNOWN;
        log_error("Unknown script action name \"" + n + "\"!", dn);
        valid = false;
        
        
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty mob action.
 * type:     the action type.
 * sub_type: sub-type, if any.
 */
mob_action::mob_action(unsigned char type, unsigned char sub_type) :
    type(type),
    sub_type(sub_type),
    code(nullptr),
    valid(true) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new mob action that runs custom code.
 * code: the function to run.
 */
mob_action::mob_action(custom_action_code code) :
    type(MOB_ACTION_UNKNOWN),
    sub_type(MOB_ACTION_UNKNOWN),
    code(code),
    valid(true) {
    
}


/* ----------------------------------------------------------------------------
 * Runs an action.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 * Return value is only used by the "if" actions, to indicate their
 *   evaluation result.
 */
bool mob_action::run(
    mob* m, void* custom_data_1, void* custom_data_2
) {

    //Custom code (i.e. instead of text-based script, use actual code).
    if(code) {
        code(m, custom_data_1, custom_data_2);
        return false;
    }
    
    if(type == MOB_ACTION_CHANGE_STATE) {
    
        m->fsm.set_state(vi[0], custom_data_1, custom_data_2);
        
        
    } else if(type == MOB_ACTION_CHOMP_HITBOXES) {
    
        m->chomp_hitboxes = vi;
        m->chomp_max = (vf.empty() ? 0 : (size_t) vf[0]);
        
        
    } else if(type == MOB_ACTION_EAT) {
    
        if(sub_type == MOB_ACTION_EAT_ALL) {
            m->eat(m->chomping_pikmin.size());
        } else {
            m->eat(vi[0]);
        }
        
        
    } else if(type == MOB_ACTION_FOCUS) {
    
        m->focused_mob = (mob*) custom_data_1;
        
        
    } else if(type == MOB_ACTION_HIDE) {
    
        if(!vi.empty()) m->hide = vi[0];
        
        
    } else if(type == MOB_ACTION_IF) {
    
        if(vs.size() >= 2) {
            return (m->vars[vs[0]] == vs[1]);
        }
        
        
    } else if(type == MOB_ACTION_IF_LESS) {
    
        if(vs.size() >= 2) {
            return (s2i(m->vars[vs[0]]) < s2i(vs[1]));
        }
        
        
    } else if(type == MOB_ACTION_IF_MORE) {
    
        if(vs.size() >= 2) {
            return (s2i(m->vars[vs[0]]) > s2i(vs[1]));
        }
        
        
    } else if(type == MOB_ACTION_IF_NOT) {
    
        if(vs.size() >= 2) {
            return (m->vars[vs[0]] != vs[1]);
        }
        
        
    } else if(type == MOB_ACTION_MOVE) {
    
        if(sub_type == MOB_ACTION_MOVE_FOCUSED_MOB) {
            if(m->focused_mob) {
                m->chase(point(), &m->focused_mob->pos, false);
            } else {
                m->stop_chasing();
            }
            
        } else if(sub_type == MOB_ACTION_MOVE_HOME) {
            m->chase(m->home, NULL, false);
            
        } else if(sub_type == MOB_ACTION_MOVE_STOP) {
            m->stop_chasing();
            m->intended_angle = m->angle;
            
        } else if(sub_type == MOB_ACTION_MOVE_STOP_VERTICALLY) {
            m->speed_z = 0;
            
        } else if(sub_type == MOB_ACTION_MOVE_COORDS) {
            if(vf.size() >= 2) {
                m->chase(point(vf[0], vf[1]), NULL, false);
            }
            
        } else if(sub_type == MOB_ACTION_MOVE_VERTICALLY) {
            if(!vf.empty()) {
                //TODO replace this with something prettier in the future.
                m->z += vf[0];
            }
            
        } else if(sub_type == MOB_ACTION_MOVE_RANDOMLY) {
            m->chase(
                point(
                    m->pos.x + randomf(-1000, 1000),
                    m->pos.y + randomf(-1000, 1000)
                ),
                NULL, false
            );
            
        } else if(sub_type == MOB_ACTION_MOVE_REL_COORDS) {
            if(vf.size() >= 2) {
                m->chase(
                    point(
                        m->pos.x + vf[0],
                        m->pos.y + vf[1]
                    ),
                    NULL, false
                );
            }
            
        }
        
        
    } else if(type == MOB_ACTION_PARTICLE) {
    
        if(vs.empty()) {
            m->remove_particle_generator(MOB_PARTICLE_GENERATOR_SCRIPT);
        } else {
            if(
                custom_particle_generators.find(vs[0]) !=
                custom_particle_generators.end()
            ) {
                particle_generator pg = custom_particle_generators[vs[0]];
                pg.id = MOB_PARTICLE_GENERATOR_SCRIPT;
                pg.follow = &m->pos;
                pg.reset();
                m->particle_generators.push_back(pg);
            }
        }
        
        
    } else if(type == MOB_ACTION_SET_ANIMATION) {
    
        if(!vi.empty()) {
            m->set_animation(vi[0], false);
        }
        
        
    } else if(type == MOB_ACTION_SET_GRAVITY) {
    
        if(!vf.empty()) {
            m->gravity_mult = vf[0];
        }
        
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        if(!vf.empty()) {
            m->set_health(
                sub_type == MOB_ACTION_SET_HEALTH_RELATIVE,
                false,
                vf[0]
            );
        }
        
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        float t;
        if(vf.empty()) t = 0;
        else t = vf[0];
        m->set_timer(t);
        
        
    } else if(type == MOB_ACTION_SET_FAR_REACH) {
    
        m->far_reach = vi[0];
        
    } else if(type == MOB_ACTION_SET_NEAR_REACH) {
    
        m->near_reach = vi[0];
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        if(vs.size() >= 2) {
            m->set_var(vs[0], vs[1]);
        }
        
        
    } else if(type == MOB_ACTION_INC_VAR) {
    
        if(!vs.empty()) {
            int nr = s2i(m->vars[vs[0]]);
            m->set_var(vs[0], i2s(nr + 1));
        }
        
        
    } else if(type == MOB_ACTION_SPECIAL_FUNCTION) {
    
        if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_DIE_START) {
        
            m->start_dying();
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_DIE_END) {
        
            m->finish_dying();
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_DELETE) {
        
            m->to_delete = true;
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_HAZARD) {
        
            gen_mob_fsm::touch_hazard(m, custom_data_1, NULL);
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_SPRAY) {
        
            gen_mob_fsm::touch_spray(m, custom_data_1, NULL);
            
        }
        
        
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Runs a mob event. Basically runs all actions within.
 * m:             the mob.
 * custom_data_1: custom argument #1 to pass to the code.
 * custom_data_2: custom argument #2 to pass to the code.
 */
void mob_event::run(mob* m, void* custom_data_1, void* custom_data_2) {
    for(size_t a = 0; a < actions.size(); ++a) {
    
        if(actions[a]->type == MOB_ACTION_IF) {
            //If statement. Look out for its return value, and
            //change the flow accordingly.
            
            if(!actions[a]->run(m, custom_data_1, custom_data_2)) {
                //If it returned true, execution continues as normal, but
                //if it returned false, skip to the "else" or "end if" actions.
                size_t next_a = a + 1;
                size_t depth = 0;
                for(; next_a < actions.size(); ++next_a) {
                    if(actions[next_a]->type == MOB_ACTION_IF) {
                        depth++;
                    } else if(actions[next_a]->type == MOB_ACTION_ELSE) {
                        if(depth == 0) break;
                    } else if(actions[next_a]->type == MOB_ACTION_ENDIF) {
                        if(depth == 0) break;
                        else depth--;
                    }
                }
                a = next_a;
                
            }
            
        } else if(actions[a]->type == MOB_ACTION_ELSE) {
            //If we actually managed to read an "else", that means we were
            //running through the normal execution of a "then" section.
            //Jump to the "end if".
            size_t next_a = a + 1;
            size_t depth = 0;
            for(; next_a < actions.size(); ++next_a) {
                if(actions[next_a]->type == MOB_ACTION_IF) {
                    depth++;
                } else if(actions[next_a]->type == MOB_ACTION_ENDIF) {
                    if(depth == 0) break;
                    else depth--;
                }
            }
            a = next_a;
            
        } else if(actions[a]->type == MOB_ACTION_ENDIF) {
            //Nothing to do.
            
        } else {
            //Normal action.
            actions[a]->run(m, custom_data_1, custom_data_2);
            if(actions[a]->type == MOB_ACTION_CHANGE_STATE) break;
            
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
 * d: the data node.
 * a: its actions.
 */
mob_event::mob_event(data_node* d, const vector<mob_action*> &a) :
    actions(a) {
    
#define r(name, number) \
    else if(n == (name)) type = (number)
    
    string n = d->name;
    if(n == "on_enter")  type = MOB_EVENT_ON_ENTER;
    r("on_leave",               MOB_EVENT_ON_LEAVE);
    r("on_animation_end",       MOB_EVENT_ANIMATION_END);
    r("on_damage",              MOB_EVENT_DAMAGE);
    r("on_far_from_home",       MOB_EVENT_FAR_FROM_HOME);
    r("on_focus_off_reach",     MOB_EVENT_FOCUS_OFF_REACH);
    r("on_itch",                MOB_EVENT_ITCH);
    r("on_mouth_empty",         MOB_EVENT_MOUTH_EMPTY);
    r("on_mouth_occupied",      MOB_EVENT_MOUTH_OCCUPIED);
    r("on_object_in_reach",     MOB_EVENT_OBJECT_IN_REACH);
    r("on_opponent_in_reach",   MOB_EVENT_OPPONENT_IN_REACH);
    r("on_pikmin_land",         MOB_EVENT_PIKMIN_LANDED);
    r("on_pikmin_latch",        MOB_EVENT_PIKMIN_LATCHED);
    r("on_pikmin_touch",        MOB_EVENT_PIKMIN_TOUCHED);
    r("on_reach_destination",   MOB_EVENT_REACHED_DESTINATION);
    r("on_revival",             MOB_EVENT_REVIVED);
    r("on_touch_hazard",        MOB_EVENT_TOUCHED_HAZARD);
    r("on_touch_opponent",      MOB_EVENT_TOUCHED_OPPONENT);
    r("on_timer",               MOB_EVENT_TIMER);
    r("on_wall",                MOB_EVENT_TOUCHED_WALL);
    else {
        type = MOB_EVENT_UNKNOWN;
        log_error("Unknown script event name \"" + n + "\"!", d);
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new event.
 * t: the event type.
 * a: its actions.
 */
mob_event::mob_event(const unsigned char t, const vector<mob_action*> &a) :
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
        for(unsigned char p = N_PREV_STATES - 1; p > 0; --p) {
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
    cur_state(nullptr) {
    
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
                mob_action* action = ev->actions[a];
                
                if(
                    action->type == MOB_ACTION_CHANGE_STATE &&
                    !action->vs.empty()
                ) {
                    string state_name = action->vs[0];
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
                    
                    action->vs.clear();
                    action->vi.clear();
                    action->vi.push_back(state_nr);
                    
                }
            }
        }
    }
    return starting_state_nr;
}


/* ----------------------------------------------------------------------------
 * Loads the actions to be run when the mob initializes.
 * mt:      The type of mob the actions are going to.
 * node:    The data node.
 * actions: Vector of actions to be filled.
 */
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action>* actions
) {
    for(size_t a = 0; a < node->get_nr_of_children(); ++a) {
        actions->push_back(mob_action(node->get_child(a), NULL, mt));
    }
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
            vector<mob_action*> actions;
            
            for(size_t a = 0; a < event_node->get_nr_of_children(); ++a) {
                data_node* action_node = event_node->get_child(a);
                actions.push_back(new mob_action(action_node, states, mt));
            }
            
            events.push_back(new mob_event(event_node, actions));
            
        }
        
        //Inject a damage event.
        vector<mob_action*> da_actions;
        da_actions.push_back(new mob_action(gen_mob_fsm::be_attacked));
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
            vector<mob_action*> de_actions;
            de_actions.push_back(new mob_action(gen_mob_fsm::die));
            events.push_back(new mob_event(MOB_EVENT_DEATH, de_actions));
        }
        
        //Inject a bottomless pit event.
        vector<mob_action*> bp_actions;
        bp_actions.push_back(new mob_action(gen_mob_fsm::fall_down_pit));
        events.push_back(new mob_event(MOB_EVENT_BOTTOMLESS_PIT, bp_actions));
        
        //Inject a spray event.
        if(
            find(
                mt->states_ignoring_spray.begin(),
                mt->states_ignoring_spray.end(),
                state_node->name
            ) == mt->states_ignoring_spray.end()
        ) {
            vector<mob_action*> s_actions;
            s_actions.push_back(new mob_action(gen_mob_fsm::touch_spray));
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
 * Creates a new action for the current event, one that changes
 * the mob's state to something else.
 */
void easy_fsm_creator::change_state(const string &new_state) {
    cur_event->actions.push_back(new mob_action(MOB_ACTION_CHANGE_STATE));
    cur_event->actions.back()->vs.push_back(new_state);
}


/* ----------------------------------------------------------------------------
 * Creates a new action for the current event, one that
 * runs some custom code.
 */
void easy_fsm_creator::run(custom_action_code code) {
    cur_event->actions.push_back(new mob_action(code));
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
hitbox_touch_info::hitbox_touch_info(
    mob* mob2, hitbox* h1, hitbox* h2
) {
    this->mob2 = mob2;
    this->h1   = h1;
    this->h2   = h2;
}
