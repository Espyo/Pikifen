/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob type class and mob type-related functions.
 */

#ifndef MOB_TYPE_INCLUDED
#define MOB_TYPE_INCLUDED

#include <map>

#include <allegro5/allegro.h>

#include "animation.h"
#include "mob_event.h"

using namespace std;

/*
 * A mob type.
 * There are specific types, like Pikmin,
 * leader, etc., but these are used
 * to create more generic mob types,
 * like some teleporter pad, or a door.
 */
class mob_type {
public:
    //Technical things.
    string name;
    
    //Visual things.
    animation_set anims;
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
    
    mob_type();
};

void load_mob_types(const string folder, const unsigned char type);

#endif //ifndef MOB_TYPE_INCLUDED