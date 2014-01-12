#ifndef MOB_TYPE_INCLUDED
#define MOB_TYPE_INCLUDED

#include "mob_event.h"

class mob_type {
public:
    //Technical things.
    string name;
    
    //Space-related things.
    float size;
    float move_speed;
    float rotation_speed;
    bool always_active; //If true, this mob is always active, even if it's off-camera.
    
    //Behavior.
    unsigned short max_health;
    float sight_radius;
    float near_radius;
    unsigned int weight;          //Pikmin strenght needed to carry it.
    
    //Script.
    vector<mob_event*> events;    //The events and actions.
    
    mob_type() {
        size = move_speed = rotation_speed = 0;
        always_active = false;
        max_health = 0;
        weight = 0;
        sight_radius = near_radius = 0;
        rotation_speed = 6.28319f; //Todo temporary. Remove.
    }
};

#endif //ifndef MOB_TYPE_INCLUDED