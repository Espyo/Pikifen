/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
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

#include <functional>
#include <map>

#include <allegro5/allegro.h>

#include "animation.h"
#include "mob_script.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * A mob type.
 * There are specific types, like Pikmin,
 * leader, etc., but these are used
 * to create more generic mob types,
 * like some teleporter pad, or a door.
 */
class mob_type {
public:
    // Technical things.
    string name;
    
    // Visual things.
    animation_set anims;
    ALLEGRO_COLOR main_color;
    
    // Space-related things.
    float radius;         // Diameter.
    float move_speed;
    float rotation_speed;
    bool always_active; // If true, this mob is always active, even if it's off-camera.
    
    // Behavior things.
    float max_health;
    float sight_radius;
    float near_radius;
    unsigned int max_carriers;
    float weight;          // Pikmin strenght needed to carry it.
    unsigned char chomp_max_victims; // The maximum number of victims in a chomp.
    float big_damage_interval;
    
    // Script things.
    vector<mob_state*> states;    // The states, events and actions.
    size_t first_state_nr;        // Number of the state a mob starts at.
    
    // Used by the special mob types, as it is not possible to control which type of mob to create without a list.
    function<void(float x, float y, float angle, const string &vars)> create_mob; // Creates a mob of this type.
    
    mob_type();
};



void load_mob_types(const string folder, const unsigned char category, bool load_resources);
void load_mob_types(bool load_resources);



enum mob_categories {
    MOB_CATEGORY_NONE,
    MOB_CATEGORY_PIKMIN,
    MOB_CATEGORY_ONIONS,
    MOB_CATEGORY_LEADERS,
    MOB_CATEGORY_ENEMIES,
    MOB_CATEGORY_TREASURES,
    MOB_CATEGORY_PELLETS,
    MOB_CATEGORY_SPECIAL,
    MOB_CATEGORY_SHIPS,
};

const size_t ANIM_IDLE = 0;

#endif // ifndef MOB_TYPE_INCLUDED
