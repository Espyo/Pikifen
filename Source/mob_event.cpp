#include "functions.h"
#include "mob_event.h"

mob_action::mob_action(data_node* dn) {
    string n = dn->name;
    data = dn->value;
    
    if(n == "if")                    type = MOB_ACTION_IF;
    else if(n == "move")             type = MOB_ACTION_MOVE;
    else if(n == "play_sound")       type = MOB_ACTION_PLAY_SOUND;
    else if(n == "animation")        type = MOB_ACTION_SET_ANIMATION;
    else if(n == "gravity")          type = MOB_ACTION_SET_GRAVITY;
    else if(n == "health")           type = MOB_ACTION_SET_HEALTH;
    else if(n == "speed")            type = MOB_ACTION_SET_SPEED;
    else if(n == "timer")            type = MOB_ACTION_SET_TIMER;
    else if(n == "var")              type = MOB_ACTION_SET_VAR;
    else if(n == "particle")         type = MOB_ACTION_SPAWN_PARTICLE;
    else if(n == "projectile")       type = MOB_ACTION_SPAWN_PROJECTILE;
    else if(n == "special_function") type = MOB_ACTION_SPECIAL_FUNCTION;
    else if(n == "turn")             type = MOB_ACTION_TURN;
    else if(n == "wait")             type = MOB_ACTION_WAIT;
    else {
        type = MOB_ACTION_UNKNOWN;
        error_log("Unknown script action name \"" + n + "\"!", dn);
    }
}

mob_action::mob_action(unsigned char t, string d) {
    type = t; data = d;
}

/* Runs an action.
 * m:         the mob.
 * ev:        the event this action belongs to.
 * action_nr: used by conditionals to change the flow of the script.
 * Returns true if the script should stop.
 */
bool mob_action::run(mob* m, mob_event* ev, size_t* action_nr) {
    //ToDo error detection.
    
    
    if(type == MOB_ACTION_IF) {
    
        vector<string> words = split(data);
        string var = words[0];
        string value = words[1];
        if(m->vars[var] != value) (*action_nr)++; //If false, skip to the next one.
        
        
        
    } else if(type == MOB_ACTION_MOVE) {
    
        //ToDo relative values.
        if(data == "prey") {
            if(m->focused_prey) {
                m->set_target(0, 0, &m->focused_prey->x, &m->focused_prey->y, false);
            } else {
                m->remove_target(true);
            }
            
        } else if(data == "home") {
            m->set_target(m->home_x, m->home_y, 0, 0, false);
            m->target_code = MOB_TARGET_HOME;
            
        } else if(data == "stop") {
            m->remove_target(true);
            
        } else {
            vector<string> coords = split(data);
            m->set_target(tof(coords[0]), tof(coords[1]), NULL, NULL, false);
        }
        
        
        
    } else if(type == MOB_ACTION_SET_ANIMATION) {
    
        if(m->type->animations.find(data) != m->type->animations.end()) {
            m->anim = animation_instance(&m->type->animations[data]);
            m->anim.start();
        } else {
            error_log("Animation \"" + data + "\" not found, called in the mob type " + m->type->name + "'s script!");
        }
        
        
        
    } else if(type == MOB_ACTION_SET_GRAVITY) {
    
        m->affected_by_gravity = (tob(data));
        
        
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        //ToDo relative values.
        vector<string> words = split(data);
        unsigned short base_nr = 0;
        if(words.size() > 1) if(words[1] == "relative") base_nr = m->health;
        
        m->health = max(0, (base_nr + toi(words[0])));
        
        
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        m->timer = m->timer_interval = tof(data);
        
        
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        vector<string> words = split(data);
        m->vars[words[0]] = words[1];
        
        
        
    } else if(type == MOB_ACTION_SPECIAL_FUNCTION) {
    
        if(data == "die") {
            if(typeid(*m) == typeid(enemy)) {
                enemy* e_ptr = (enemy*) m;
                if(e_ptr->ene_type->drops_corpse) {
                    m->carrier_info = new carrier_info_struct(m, e_ptr->ene_type->max_carriers, false);
                }
            }
        }
        
        
        
    } else if(type == MOB_ACTION_WAIT) {
    
        //ToDo wait for animation, etc.
        if(data == "animation") {
            m->script_wait = -1;
            return true;
        } else {
            float time_to_wait = tof(data);
            if(time_to_wait > 0) {
                m->script_wait = tof(data);
                return true;
            }
        }
        
        
        
    }
    
    return false;
}

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

mob_event::mob_event(unsigned char t, vector<mob_action*> a) {
    type = t; actions = a;
}

void mob_event::run(mob* m, size_t starting_action) {

    //ToDo remove.
    if(starting_action == 0) {
        if(type == MOB_EVENT_SEE_PREY) {
            cout << "See prey event hit.\n";
        } else if(type == MOB_EVENT_LOSE_PREY) {
            cout << "Lose prey event hit.\n";
        } else if(type == MOB_EVENT_NEAR_PREY) {
            cout << "Near prey event hit.\n";
        } else if(type == MOB_EVENT_TIMER) {
            cout << "Timer event hit.\n";
        } else if(type == MOB_EVENT_REACH_HOME) {
            cout << "Reach home event hit.\n";
        }
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