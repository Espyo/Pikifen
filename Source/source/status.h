/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the status effect classes and status effect-related functions.
 */

#ifndef STATUS_INCLUDED
#define STATUS_INCLUDED

#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

using namespace std;

/* ----------------------------------------------------------------------------
 * A status effect type, like "burning", "spicy", "petrified", etc.
 * Any mob under the influence of a status effect will suffer or
 * benefit from changes in some of its values. Some effects can
 * increase the speed, others can decrease attack power. Others
 * can even slowly kill the mob unless they're cleared out, like
 * Pikmin on fire or drowning.
 */
struct status_type {
    string name;
    ALLEGRO_COLOR color;
    
    unsigned char affects; //What types of mobs it affects.
    bool removable_with_whistle;
    float auto_remove_time;
    
    float health_change_ratio; //Health addition/subtraction percentage per second.
    bool causes_panic;
    bool causes_flailing;
    
    float speed_multiplier;
    float attack_multiplier;
    float defense_multiplier;
    float anim_speed_multiplier;
    
    status_type();
};


/* ----------------------------------------------------------------------------
 * Instance of an active status effect on a mob.
 */
struct status {
    status_type* type;
    
    float time_left; //If this status effect auto-removes itself.
    
    void tick(const float delta_t);
    
    status(status_type* type);
};



enum STATUS_AFFECTS_FLAGS {
    STATUS_AFFECTS_PIKMIN = 1,
    STATUS_AFFECTS_ENEMIES = 2,
    STATUS_AFFECTS_LEADERS = 4,
    STATUS_AFFECTS_ENEMY_PIKMIN = 8,
    STATUS_AFFECTS_OBSTACLES = 16,
};

#endif //ifndef STATUS_INCLUDED
