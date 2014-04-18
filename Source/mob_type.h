#ifndef MOB_TYPE_INCLUDED
#define MOB_TYPE_INCLUDED

#include <map>

#include <allegro5/allegro.h>

#include "animation.h"
#include "const.h"
#include "mob_event.h"

using namespace std;

class mob_type {
public:
    //Technical things.
    string name;
    
    //Visual things.
    animation_set anim;
    ALLEGRO_COLOR main_color;
    
    //Space-related things.
    float size;         //Diameter.
    float move_speed;
    float rotation_speed;
    bool always_active; //If true, this mob is always active, even if it's off-camera.
    
    //Behavior things.
    float max_health;
    float sight_radius;
    float near_radius;
    unsigned int max_carriers;
    float weight;          //Pikmin strenght needed to carry it.
    unsigned char chomp_max_victims; //The maximum number of victims in a chomp.
    float big_damage_interval;
    
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
        big_damage_interval = 0;
    }
};

#endif //ifndef MOB_TYPE_INCLUDED