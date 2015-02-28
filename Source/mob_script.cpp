/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script classes and
 * related functions.
 */

#include <algorithm>

#include "functions.h"
#include "mob.h"
#include "mob_script.h"
#include "mob_type.h"
#include "particle.h"
#include "vars.h"

mob_action::mob_action(data_node* dn, vector<mob_state*>* states, mob_type* mt) {
    valid = true;
    type = MOB_ACTION_UNKNOWN;
    sub_type = 0;
    string n = dn->name;
    
    
    if(n == "chomp") {
        type = MOB_ACTION_CHOMP_HITBOXES;
        
        vector<string> hitbox_names = split(dn->value);
        
        for(size_t hn = 0; hn < hitbox_names.size(); hn++) {
            size_t h_pos = mt->anims.find_hitbox(hitbox_names[hn]);
            
            if(h_pos == string::npos) {
                error_log("Hitbox \"" + hitbox_names[hn] + "\" not found!", dn);
                valid = false;
            } else {
                vi.push_back(h_pos);
            }
        }
        
        
        
    } else if(n == "eat") {
        type = MOB_ACTION_EAT;
        
        if(dn->value == "all") {
            sub_type = MOB_ACTION_EAT_ALL;
        } else {
            sub_type = MOB_ACTION_EAT_NUMBER;
            vi.push_back(s2i(dn->value));
        }
        
        
        
    } else if(n == "if") {
        // TODO make this use integers instead of strings, eventually?
        type = MOB_ACTION_IF;
        
        vector<string> words = split(dn->value);
        if(words.size() < 2) {
            error_log("Not enough parts on this if: \"" + dn->value + "\"!", dn);
            valid = false;
        } else {
            vs.push_back(words[0]); vs.push_back(words[1]);
        }
        
        
        
    } else if(n == "move") {
        type = MOB_ACTION_MOVE;
        
        if(dn->value == "opponent") sub_type = MOB_ACTION_MOVE_OPPONENT;
        else if(dn->value == "home") sub_type = MOB_ACTION_MOVE_HOME;
        else if(dn->value == "stop") sub_type = MOB_ACTION_MOVE_STOP;
        else {
        
            vector<string> string_coords = split(dn->value);
            
            if(string_coords.empty()) valid = false;
            else {
                if(string_coords[0] == "relative") {
                    sub_type = MOB_ACTION_MOVE_REL_COORDS;
                    if(string_coords.size() < 3) valid = false;
                    else {
                        for(size_t sc = 1; sc < string_coords.size(); sc++) vf.push_back(s2f(string_coords[sc]));
                    }
                } else {
                    sub_type = MOB_ACTION_MOVE_COORDS;
                    for(size_t sc = 0; sc < string_coords.size(); sc++) vf.push_back(s2f(string_coords[sc]));
                }
            }
            
            if(!valid) {
                error_log("Invalid location \"" + dn->value + "\"!", dn);
            }
        }
        
        
        
    } else if(n == "play_sound") {
        type = MOB_ACTION_PLAY_SOUND;
        
        
        
    } else if(n == "animation") {
        type = MOB_ACTION_SET_ANIMATION;
        
        size_t f_pos = mt->anims.find_animation(dn->value);
        if(f_pos == string::npos) {
            error_log("Unknown animation \"" + dn->value + "\"!", dn);
            valid = false;
        } else {
            vi.push_back(f_pos);
        }
        
        
        
    } else if(n == "gravity") {
        type = MOB_ACTION_SET_GRAVITY;
        
        vi.push_back(s2b(dn->value));
        
        
        
    } else if(n == "health") {
        type = MOB_ACTION_SET_HEALTH;
        
        vector<string> words = split(dn->value);
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
            error_log("Invalid health amount \"" + dn->value + "\"!", dn);
        }
        
        
        
    } else if(n == "speed") {
        type = MOB_ACTION_SET_SPEED;
        
        
        
    } else if(n == "state") {
        type = MOB_ACTION_SET_STATE;
        for(size_t s = 0; s < states->size(); s++) {
            if(states->at(s)->name == dn->value) {
                vi.push_back(s);
                break;
            }
        }
        if(vi.empty()) {
            error_log("Unknown state \"" + dn->value + "\"!", dn);
        }
        
        
        
    } else if(n == "timer") {
        type = MOB_ACTION_SET_TIMER;
        
        vf.push_back(s2f(dn->value));
        
        
        
    } else if(n == "var") {
        type = MOB_ACTION_SET_VAR;
        
        vector<string> words = split(dn->value);
        if(words.size() < 2) {
            error_log("Not enough info to set a variable!", dn);
            valid = false;
        } else {
            vs = words;
        }
        
        
        
    } else if(n == "particle") {
        type = MOB_ACTION_SPAWN_PARTICLE;
        
        
        
    } else if(n == "projectile") {
        type = MOB_ACTION_SPAWN_PROJECTILE;
        
        
        
    } else if(n == "special_function") {
        type = MOB_ACTION_SPECIAL_FUNCTION;
        
        if(dn->value == "die_start") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_DIE_START;
        } else if(dn->value == "die_end") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_DIE_END;
        } else if(dn->value == "loop") {
            sub_type = MOB_ACTION_SPECIAL_FUNCTION_LOOP;
        } else {
            error_log("Unknown special function \"" + dn->value + "\"!", dn);
            valid = false;
        }
        
        
        
    } else if(n == "turn") {
        type = MOB_ACTION_TURN;
        
        
        
    } else if(n == "wait") {
        type = MOB_ACTION_WAIT;
        
        if(dn->value == "animation") {
            sub_type = MOB_ACTION_WAIT_ANIMATION;
        } else {
            sub_type = MOB_ACTION_WAIT_TIME;
            vf.push_back(s2f(dn->value));
        }
        
        
        
    } else {
        type = MOB_ACTION_UNKNOWN;
        error_log("Unknown script action name \"" + n + "\"!", dn);
        valid = false;
        
        
        
    }
}

void mob_action::run(mob* m, size_t* action_nr) {
    if(type == MOB_ACTION_CHOMP_HITBOXES) {
    
        m->chomp_hitboxes = vi;
        
        
        
    } else if(type == MOB_ACTION_EAT) {
    
        if(sub_type == MOB_ACTION_EAT_ALL) {
            for(size_t p = 0; p < m->chomping_pikmin.size(); p++) {
                m->chomping_pikmin[p]->health = 0;
            }
        }
        // TODO other cases besides eating all.
        m->chomping_pikmin.clear();
        
        
        
    } else if(type == MOB_ACTION_IF) {
    
        // TODO check for vs size.
        if(m->vars[vs[0]] != vs[1]) (*action_nr)++; // If false, skip to the next one.
        
        
        
    } else if(type == MOB_ACTION_MOVE) {
    
        // TODO relative values.
        // TODO check for vf size.
        if(sub_type == MOB_ACTION_MOVE_OPPONENT) {
            if(m->focused_opponent) {
                m->set_target(0, 0, &m->focused_opponent->x, &m->focused_opponent->y, false);
            } else {
                m->remove_target(true);
            }
            
        } else if(sub_type == MOB_ACTION_MOVE_HOME) {
            m->set_target(m->home_x, m->home_y, 0, 0, false);
            m->target_code = MOB_TARGET_HOME;
            
        } else if(sub_type == MOB_ACTION_MOVE_STOP) {
            m->remove_target(true);
            
        } else if(sub_type == MOB_ACTION_MOVE_COORDS) {
            m->set_target(vf[0], vf[1], NULL, NULL, false);
            
        } else if(sub_type == MOB_ACTION_MOVE_REL_COORDS) {
            m->set_target(m->x + vf[0], m->y + vf[1], NULL, NULL, false);
            
        }
        
        
        
    } else if(type == MOB_ACTION_SET_ANIMATION) {
    
        if(!vi.empty()) {
            m->anim.change(vi[0], false, false, false);
        }
        
        
        
    } else if(type == MOB_ACTION_SET_GRAVITY) {
    
        // TODO check vi's size.
        m->affected_by_gravity = vi[0];
        
        
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        // TODO check vf size's.
        unsigned short base_nr = 0;
        if(sub_type == MOB_ACTION_SET_HEALTH_RELATIVE) base_nr = m->health;
        
        m->health = max(0.0f, (float) (base_nr + vf[0]));
        
        
        
    } else if(type == MOB_ACTION_SET_STATE) {
    
        m->fsm.set_state(vi[0]);
        
        
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        // TODO check vf's size.
        m->timer = m->timer_interval = vf[0];
        
        
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        // TODO check vs' size.
        m->vars[vs[0]] = vs[1];
        
        
        
    } else if(type == MOB_ACTION_SPECIAL_FUNCTION) {
    
        if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_DIE_START) {
        
            if(typeid(*m) == typeid(enemy)) {
                random_particle_explosion(PARTICLE_TYPE_BITMAP, bmp_sparkle, m->x, m->y, 100, 140, 20, 40, 1, 2, 64, 64, al_map_rgb(255, 192, 192));
            }
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_DIE_END) {
        
            if(typeid(*m) == typeid(enemy)) {
                enemy* e_ptr = (enemy*) m;
                if(e_ptr->ene_type->drops_corpse) {
                    m->carrier_info = new carrier_info_struct(m, e_ptr->ene_type->max_carriers, false);
                }
                particles.push_back(
                    particle(
                        PARTICLE_TYPE_ENEMY_SPIRIT, bmp_enemy_spirit, m->x, m->y,
                        0, -50, 0.5, 0, 2, 64, al_map_rgb(255, 192, 255)
                    )
                );
            }
            
        }
        
        
        
    }
}

void mob_event::run(mob* m) {
    for(size_t a = 0; a < actions.size(); a++) {
        actions[a]->run(m, &a);
    }
}

mob_event* mob_state::get_event(const unsigned short type) {
    if(this->events.empty()) return NULL;
    for(auto e = this->events.begin(); e != this->events.end(); e++) {
        if((*e)->type == type) return *e;
    }
    
    return NULL;
}


mob_event* mob_fsm::get_event(const unsigned short type) {
    if(!cur_state) return NULL;
    return cur_state->get_event(type);
}

void mob_fsm::run_event(const unsigned short type, mob* m) {
    mob_event* e = get_event(type);
    if(e) e->run(m);
}

mob_event::mob_event(data_node* d, vector<mob_action*> a) {
    string n = d->name;
    if(n == "on_enter")              type = MOB_EVENT_ON_ENTER;
    else if(n == "on_leave")         type = MOB_EVENT_ON_LEAVE;
    else if(n == "on_animation_end") type = MOB_EVENT_ANIMATION_END;
    else if(n == "on_attack_hit")    type = MOB_EVENT_ATTACK_HIT;
    else if(n == "on_attack_miss")   type = MOB_EVENT_ATTACK_MISS;
    else if(n == "on_big_damage")    type = MOB_EVENT_BIG_DAMAGE;
    else if(n == "on_damage")        type = MOB_EVENT_DAMAGE;
    else if(n == "on_death")         type = MOB_EVENT_DEATH;
    else if(n == "on_enter_hazard")  type = MOB_EVENT_ENTER_HAZARD;
    else if(n == "on_idle")          type = MOB_EVENT_IDLE;
    else if(n == "on_leave_hazard")  type = MOB_EVENT_LEAVE_HAZARD;
    else if(n == "on_lose_object")   type = MOB_EVENT_LOSE_OBJECT;
    else if(n == "on_lose_opponent") type = MOB_EVENT_LOSE_OPPONENT;
    else if(n == "on_near_object")   type = MOB_EVENT_NEAR_OBJECT;
    else if(n == "on_near_opponent") type = MOB_EVENT_NEAR_OPPONENT;
    else if(n == "on_pikmin_land")   type = MOB_EVENT_PIKMIN_LAND;
    else if(n == "on_pikmin_latch")  type = MOB_EVENT_PIKMIN_LATCH;
    else if(n == "on_pikmin_touch")  type = MOB_EVENT_PIKMIN_TOUCH;
    else if(n == "on_reach_home")    type = MOB_EVENT_REACH_HOME;
    else if(n == "on_revival")       type = MOB_EVENT_REVIVAL;
    else if(n == "on_see_object")    type = MOB_EVENT_SEE_OBJECT;
    else if(n == "on_see_opponent")  type = MOB_EVENT_SEE_OPPONENT;
    else if(n == "on_timer")         type = MOB_EVENT_TIMER;
    else if(n == "on_wall")          type = MOB_EVENT_WALL;
    else {
        type = MOB_EVENT_UNKNOWN;
        error_log("Unknown script event name \"" + n + "\"!", d);
    }
    actions = a;
}

mob_event::mob_event(const unsigned short t, vector<mob_action*> a) {
    type = t; actions = a;
}

mob_state::mob_state(const string name, vector<mob_event*> e) {
    this->name = name;
    events = e;
}


void mob_fsm::set_state(const size_t new_state) {
    if(new_state >= m->type->states.size()) return;
    // Run the code to leave the current state.
    if(cur_state) {
        m->fsm.run_event(MOB_EVENT_ON_LEAVE, m);
    }
    
    // Switch states.
    m->fsm.cur_state = m->type->states[new_state];
    
    // Run the code to enter the new state.
    m->fsm.run_event(MOB_EVENT_ON_ENTER, m);
}


mob_fsm::mob_fsm(mob* m) {
    cur_state = NULL;
    if(!m) return;
    this->m = m;
}


vector<mob_state*> load_script(mob_type* mt, data_node* node) {
    vector<mob_state*> states;
    size_t n_states = node->get_nr_of_children();
    
    for(size_t s = 0; s < n_states; s++) {
    
        data_node* state_node = node->get_child(s);
        //Let's save the state now, so that the state switching events
        //can now what numbers the events they need correspond to.
        states.push_back(new mob_state(state_node->name));
    }
    
    for(size_t s = 0; s < n_states; s++) {
        data_node* state_node = node->get_child(s);
        vector<mob_event*> events;
        size_t n_events = state_node->get_nr_of_children();
        
        for(size_t e = 0; e < n_events; e++) {
        
            data_node* event_node = state_node->get_child(e);
            vector<mob_action*> actions;
            
            for(size_t a = 0; a < event_node->get_nr_of_children(); a++) {
                data_node* action_node = event_node->get_child(a);
                actions.push_back(new mob_action(action_node, &states, mt));
            }
            
            events.push_back(new mob_event(event_node, actions));
            
        }
        
        states[s]->events = events;
        
    }
    
    return states;
}
