/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob event class and mob event-related functions.
 */

#include <algorithm>

#include "functions.h"
#include "mob_event.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a mob action.
 * mt: Mob type this belongs to.
 * dn: Data node from which the creation information will be obtained.
 */
mob_action::mob_action(mob_type* mt, data_node* dn) {
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
        //ToDo make this use integers instead of strings, eventually?
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
        
        if(dn->value == "prey") sub_type = MOB_ACTION_MOVE_PREY;
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


/* ----------------------------------------------------------------------------
 * Runs an action.
 * m:         the mob.
 * ev:        the event this action belongs to.
 * action_nr: used by conditionals to change the flow of the script.
 * Returns true if the script should stop.
 */
bool mob_action::run(mob* m, mob_event* e, size_t* action_nr) {
    if(type == MOB_ACTION_CHOMP_HITBOXES) {
    
        m->chomp_hitboxes = vi;
        
        
        
    } else if(type == MOB_ACTION_EAT) {
    
        if(sub_type == MOB_ACTION_EAT_ALL) {
            for(size_t p = 0; p < m->chomping_pikmin.size(); p++) {
                m->chomping_pikmin[p]->health = 0;
            }
        }
        //ToDo other cases besides eating all.
        m->chomping_pikmin.clear();
        
        
        
    } else if(type == MOB_ACTION_IF) {
    
        //ToDo check for vs size.
        if(m->vars[vs[0]] != vs[1]) (*action_nr)++; //If false, skip to the next one.
        
        
        
    } else if(type == MOB_ACTION_MOVE) {
    
        //ToDo relative values.
        //ToDo check for vf size.
        if(sub_type == MOB_ACTION_MOVE_PREY) {
            if(m->focused_prey) {
                m->set_target(0, 0, &m->focused_prey->x, &m->focused_prey->y, false);
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
    
        //ToDo check vi's size.
        m->affected_by_gravity = vi[0];
        
        
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        //ToDo check vf size's.
        unsigned short base_nr = 0;
        if(sub_type == MOB_ACTION_SET_HEALTH_RELATIVE) base_nr = m->health;
        
        m->health = max(0.0f, (float) (base_nr + vf[0]));
        
        
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        //ToDo check vf's size.
        m->timer = m->timer_interval = vf[0];
        
        
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        //ToDo check vs' size.
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
            
        } else if(sub_type == MOB_ACTION_SPECIAL_FUNCTION_LOOP) {
            m->events_queued[e->type] = 2;
            
        }
        
        
        
    } else if(type == MOB_ACTION_WAIT) {
    
        //ToDo check vf's size.
        if(sub_type == MOB_ACTION_WAIT_ANIMATION) {
            m->script_wait = -1;
            return true;
        } else {
            float time_to_wait = vf[0];
            if(time_to_wait > 0) {
                m->script_wait = vf[0];
                return true;
            }
        }
        
        
        
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the pointer to a mob event, if the mob is listening to that event.
 * Returns NULL if this event can't run, because there's already another event going on, or because the mob is dead.
 * If query is true, then the caller only wants to know of the existence of the event, not actually do something with it.
   * This makes it return the pointer if it exists, regardless of it being able to run or not.
 */
mob_event* get_mob_event(mob* m, const unsigned char et, const bool query) {
    if(m->dead && et != MOB_EVENT_DEATH) return NULL;
    size_t n_events = m->type->events.size();
    for(size_t ev_nr = 0; ev_nr < n_events; ev_nr++) {
    
        mob_event* ev = m->type->events[ev_nr];
        if(ev->type == et) {
            if(query) return ev;
            if(m->script_wait != 0 && m->script_wait_event != ev && et != MOB_EVENT_DEATH) return NULL;
            return ev;
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Loads a script from a data node.
 */
vector<mob_event*> load_script(mob_type* mt, data_node* node) {

    vector<mob_event*> events;
    
    for(size_t e = 0; e < node->get_nr_of_children(); e++) {
        data_node* event_node = node->get_child(e);
        
        vector<mob_action*> actions;
        
        for(size_t a = 0; a < event_node->get_nr_of_children(); a++) {
            data_node* action_node = event_node->get_child(a);
            
            actions.push_back(new mob_action(mt, action_node));
        }
        
        events.push_back(new mob_event(event_node, actions));
        
    }
    
    return events;
}


/* ----------------------------------------------------------------------------
 * Creates a mob event using data from a data node.
 * d: The data node.
 * a: A vector of actions this event should run when triggered.
 */
mob_event::mob_event(data_node* d, vector<mob_action*> a) {
    string n = d->name;
    if(n == "on_attack_hit")        type = MOB_EVENT_ATTACK_HIT;
    else if(n == "on_attack_miss")  type = MOB_EVENT_ATTACK_MISS;
    else if(n == "on_big_damage")   type = MOB_EVENT_BIG_DAMAGE;
    else if(n == "on_damage")       type = MOB_EVENT_DAMAGE;
    else if(n == "on_death")        type = MOB_EVENT_DEATH;
    else if(n == "on_enter_hazard") type = MOB_EVENT_ENTER_HAZARD;
    else if(n == "on_idle")         type = MOB_EVENT_IDLE;
    else if(n == "on_leave_hazard") type = MOB_EVENT_LEAVE_HAZARD;
    else if(n == "on_lose_object")  type = MOB_EVENT_LOSE_OBJECT;
    else if(n == "on_lose_prey")    type = MOB_EVENT_LOSE_PREY;
    else if(n == "on_near_object")  type = MOB_EVENT_NEAR_OBJECT;
    else if(n == "on_near_prey")    type = MOB_EVENT_NEAR_PREY;
    else if(n == "on_pikmin_land")  type = MOB_EVENT_PIKMIN_LAND;
    else if(n == "on_pikmin_latch") type = MOB_EVENT_PIKMIN_LATCH;
    else if(n == "on_pikmin_touch") type = MOB_EVENT_PIKMIN_TOUCH;
    else if(n == "on_reach_home")   type = MOB_EVENT_REACH_HOME;
    else if(n == "on_revival")      type = MOB_EVENT_REVIVAL;
    else if(n == "on_see_object")   type = MOB_EVENT_SEE_OBJECT;
    else if(n == "on_see_prey")     type = MOB_EVENT_SEE_PREY;
    else if(n == "on_spawn")        type = MOB_EVENT_SPAWN;
    else if(n == "on_timer")        type = MOB_EVENT_TIMER;
    else if(n == "on_wall")         type = MOB_EVENT_WALL;
    else {
        type = MOB_EVENT_UNKNOWN;
        error_log("Unknown script event name \"" + n + "\"!", d);
    }
    
    actions = a;
}


/* ----------------------------------------------------------------------------
 * Creates a mob event with a pre-known type ID.
 * t: The type.
 * a: A vector of actions this event should run when triggered.
 */
mob_event::mob_event(const unsigned char t, vector<mob_action*> a) {
    type = t; actions = a;
}


/* ----------------------------------------------------------------------------
 * Runs a mob event.
 * m:               The mob that's requesting this event.
 * starting_action: Start on this action. If the execution was paused
   * (e.g. by a wait), it should continue where it left off.
 */
void mob_event::run(mob* m, const size_t starting_action) {

    //ToDo remove.
    if(starting_action == 0) {
        string n = "#" + to_string((long long) type);
        if(type == MOB_EVENT_SEE_PREY) n = "SEE PREY";
        else if(type == MOB_EVENT_LOSE_PREY) n = "LOSE PREY";
        else if(type == MOB_EVENT_NEAR_PREY) n = "NEAR PREY";
        else if(type == MOB_EVENT_TIMER) n = "TIMER";
        else if(type == MOB_EVENT_REACH_HOME) n = "REACH HOME";
        else if(type == MOB_EVENT_SPAWN) n = "SPAWN";
        else if(type == MOB_EVENT_ATTACK_HIT) n = "ATTACK HIT";
        else if(type == MOB_EVENT_DEATH) n = "DEATH";
        else if(type == MOB_EVENT_BIG_DAMAGE) n = "BIG_DAMAGE";
        
        cout << "Event hit: " << n << "\n";
    }
    
    for(size_t a = starting_action; a < actions.size(); a++) {
        if(actions[a]->run(m, this, &a)) {
            a++;
            m->script_wait_event = this;
            m->script_wait_action = a;
            return;
        }
    }
}
