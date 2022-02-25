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


//Flags that control what sorts of mob a status effect affects.
enum STATUS_AFFECTS_FLAGS {
    //Affects Pikmin.
    STATUS_AFFECTS_PIKMIN = 1,
    //Affects enemies.
    STATUS_AFFECTS_ENEMIES = 2,
    //Affects leaders.
    STATUS_AFFECTS_LEADERS = 4,
    //Affects other mobs.
    STATUS_AFFECTS_OTHERS = 8,
};


//What mob script state the status effect changes to.
enum STATUS_STATE_CHANGES {
    //None.
    STATUS_STATE_CHANGE_NONE,
    //Pikmin flailing state.
    STATUS_STATE_CHANGE_FLAILING,
    //Pikmin helpless state.
    STATUS_STATE_CHANGE_HELPLESS,
    //Pikmin panic state.
    STATUS_STATE_CHANGE_PANIC,
    //A custom state.
    STATUS_STATE_CHANGE_CUSTOM,
};


//Rule to follow when re-applying a status effect.
enum STATUS_REAPPLY_RULES {
    //Keep the same auto-remove time as before.
    STATUS_REAPPLY_KEEP_TIME,
    //Reset the auto-remove time.
    STATUS_REAPPLY_RESET_TIME,
    //Add more time to the auto-remove time.
    STATUS_REAPPLY_ADD_TIME,
};


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
    bool remove_on_hazard_leave;
    float auto_remove_time;
    STATUS_REAPPLY_RULES reapply_rule;
    
    //Health addition/subtraction per second.
    float health_change;
    //Health addition/subtraction percentage per second.
    float health_change_ratio;
    //Increase/decrease in maturity when the status is gained.
    int maturity_change_amount;
    
    STATUS_STATE_CHANGES state_change_type;
    string state_change_name;
    string animation_change;
    
    float speed_multiplier;
    float attack_multiplier;
    float defense_multiplier;
    float anim_speed_multiplier;
    bool disables_attack;
    bool turns_inedible;
    bool turns_invisible;
    bool freezes_animation;
    
    //We need to know this in order to remove the particle generator later.
    bool generates_particles;
    particle_generator* particle_gen;
    point particle_offset_pos;
    float particle_offset_z;
    float shaking_effect;
    string overlay_animation;
    float overlay_anim_mob_scale;
    animation_database overlay_anim_db;
    animation_instance overlay_anim_instance;
    status_type* replacement_on_timeout;
    
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



#endif //ifndef STATUS_INCLUDED
