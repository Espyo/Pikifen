#ifndef MOB_TYPE_INCLUDED
#define MOB_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "const.h"
#include "mob_event.h"

class mob_type {
public:
    //Technical things.
    string name;
    
    //Detail things.
    ALLEGRO_COLOR main_color;
    
    //Space-related things.
    float size;         //Diameter.
    float move_speed;
    float rotation_speed;
    bool always_active; //If true, this mob is always active, even if it's off-camera.
    
    //Behavior things.
    unsigned int max_health;
    float sight_radius;
    float near_radius;
    unsigned int max_carriers;
    float weight;          //Pikmin strenght needed to carry it.
    
    //Script things.
    vector<mob_event*> events;    //The events and actions.
    
    mob_type() {
        size = move_speed = rotation_speed = 0;
        always_active = false;
        max_health = 0;
        max_carriers = 0;
        weight = 0;
        sight_radius = near_radius = 0;
        rotation_speed = DEF_ROTATION_SPEED;
    }
};

#endif //ifndef MOB_TYPE_INCLUDED