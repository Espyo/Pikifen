#include "functions.h"
#include "mob_event.h"

//Returns true if the script should stop.
bool mob_action::run(mob* m) {
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
        vector<string> words = split(data);
        unsigned short base_nr = 0;
        if(words[0] == "relative") base_nr = m->health;
        
        m->health = max(0, (m->health + toi(words[1])));
        
    } else if(type == MOB_ACTION_SET_TIMER) {
    
        m->timer = m->timer_interval = tof(data);
        
    } else if(type == MOB_ACTION_SET_VAR) {
    
        vector<string> words = split(data);
        m->vars[words[0]] = words[1];
        
    } else if(type == MOB_ACTION_WAIT) {
    
        //ToDo wait for animation, etc.
        float time_to_wait = tof(data);
        if(time_to_wait > 0) {
            m->script_wait = tof(data);
            return true;
        }
        
    }
    
    return false;
}

void mob_event::run(mob* m, size_t starting_action) {

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
    
    for(size_t a = starting_action; a < actions.size(); a++) {
        if(actions[a]->run(m)) {
            a++;
            m->script_wait_event = this;
            m->script_wait_action = a;
            return;
        }
    }
}