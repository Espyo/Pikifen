/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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

#include "animation.h"
#include "particle.h"

using std::string;


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
    unsigned char affects;
    ALLEGRO_COLOR color;
    ALLEGRO_COLOR tint;
    ALLEGRO_COLOR glow;
    
    bool removable_with_whistle;
    float auto_remove_time;
    
    //Health addition/subtraction percentage per second.
    float health_change_ratio;
    //Increase/decrease in maturity when the status is gained.
    int maturity_change_amount;
    bool causes_disable;
    bool causes_flailing;
    bool causes_panic;
    bool disabled_state_inedible;
    
    float speed_multiplier;
    float attack_multiplier;
    float defense_multiplier;
    float anim_speed_multiplier;
    bool disables_attack;
    bool turns_invisible;
    
    //We need to know this in order to remove the particle generator later.
    bool generates_particles;
    particle_generator* particle_gen;
    point particle_offset_pos;
    float particle_offset_z;
    string animation_name;
    float animation_mob_scale;
    animation_database anim_db;
    animation_instance anim_instance;
    
    status_type();
};


/* ----------------------------------------------------------------------------
 * Instance of an active status effect on a mob.
 */
struct status {
    status_type* type;
    
    float time_left; //If this status effect auto-removes itself.
    bool to_delete;
    
    void tick(const float delta_t);
    
    status(status_type* type);
};



enum STATUS_AFFECTS_FLAGS {
    STATUS_AFFECTS_PIKMIN = 1,
    STATUS_AFFECTS_ENEMIES = 2,
    STATUS_AFFECTS_LEADERS = 4,
    STATUS_AFFECTS_OTHERS = 8,
};

#endif //ifndef STATUS_INCLUDED
