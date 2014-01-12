#include "functions.h"
#include "mob_event.h"

//Returns true if the script should stop.
bool mob_action::run(mob_event* parent) {
    //ToDo error detection.
    
    if(type == MOB_ACTION_MOVE) {
    
        //ToDo relative values.
        if(data == "pikmin") {
            if(m->focused_pikmin)
                m->set_target(0, 0, &m->focused_pikmin->x, &m->focused_pikmin->y, false);
            else
                m->remove_target(true);
                
        } else if(data == "home") {
            m->set_target(m->home_x, m->home_y, 0, 0, false);
            
        } else {
            vector<string> coords = split(data);
            m->set_target(tof(coords[0]), tof(coords[1]), NULL, NULL, false);
        }
        
    } else if(type == MOB_ACTION_SET_GRAVITY) {
    
        m->affected_by_gravity = (tob(data));
        
    } else if(type == MOB_ACTION_SET_HEALTH) {
    
        //ToDo relative values.
        m->health = toi(data);
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        m->timer = m->timer_interval = tof(data);
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        vector<string> words = split(data);
        m->vars[words[0]] = words[1];
        
    } else if(type == MOB_ACTION_WAIT) {
    
        //ToDo wait for animation, etc.
        m->script_wait = tof(data);
        return true;
        
    }
    
    return false;
}

void mob_event::run(bool from_the_start) {
    if(from_the_start) current_action = 0;
    
    //ToDo remove.
    if(type == MOB_EVENT_SEE_PIKMIN) {
        cout << "See Pikmin event hit.\n";
    } else if(type == MOB_EVENT_LOSE_PIKMIN) {
        cout << "Lose Pikmin event hit.\n";
    } else if(type == MOB_EVENT_NEAR_PIKMIN) {
        cout << "Near Pikmin event hit.\n";
    } else if(type == MOB_EVENT_TIMER) {
        cout << "Timer event hit.\n";
    }
    
    for(; current_action < actions.size(); current_action++) {
        if(actions[current_action]->run(this)) {
            current_action++;
            m->script_wait_event = this;
            return;
        }
    }
}