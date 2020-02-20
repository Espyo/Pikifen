/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <vector>

#include <allegro5/allegro.h>

#include "utils/geometry_utils.h"
#include "world_component.h"

class mob;

enum PARTICLE_TYPES {
    PARTICLE_TYPE_SQUARE,
    PARTICLE_TYPE_CIRCLE,
    PARTICLE_TYPE_BITMAP,
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    PARTICLE_TYPE_ENEMY_SPIRIT,
    PARTICLE_TYPE_SMACK,
    PARTICLE_TYPE_DING,
};

enum PARTICLE_PRIORITIES {
    PARTICLE_PRIORITY_LOW,
    PARTICLE_PRIORITY_MEDIUM,
    PARTICLE_PRIORITY_HIGH,
};


/* ----------------------------------------------------------------------------
 * A particle is best described with examples:
 * A puff of smoke, a sparkle, a smack.
 * There are several different types, which
 * change the way they look, how they behave over time, etc.
 */
struct particle {
    //Behavior stats.
    //Type. Use PARTICLE_TYPE_*.
    unsigned char type;
    //How long its lifespan is.
    float duration;
    //Bitmap to use, if any.
    ALLEGRO_BITMAP* bitmap;
    //Every second, speed is lost by this much.
    float friction;
    //Every second, the vertical speed is increased by this.
    float gravity;
    //Every second, the size is increased by this much.
    float size_grow_speed;
    
    //Current state.
    //Current time left to live. 0 means it's dead.
    float time;
    //Current coordinates.
    point pos;
    //Current Z.
    float z;
    //Current size, in diameter.
    float size;
    //Current movement speed.
    point speed;
    //Current color.
    ALLEGRO_COLOR color;
    
    //Other stuff.
    //Priority. If we reached the particle limit, only spawn
    //this particle if it can replace a lower-priority one.
    unsigned char priority;
    
    particle(
        const unsigned char type = PARTICLE_TYPE_BITMAP,
        const point &pos = point(), const float z = 0.0f,
        const float size = 0.0f,
        const float duration = 0.0f, const unsigned char priority = 255
    );
    void tick(const float delta_t);
    void draw();
};


/* ----------------------------------------------------------------------------
 * Manages a list of particles, allows the addition of new ones, etc.
 */
struct particle_manager {
private:
    //This list works as follows:
    //The first "count" particles are alive.
    //The next particle is the beginning of the dead ones.
    //When a particle is deleted, swap places between it and the first
    //"dead" particle, to preserve the list's logic.
    //When a particle is added, if the entire list is filled with live ones,
    //delete the one on position 0 (presumably the oldest).
    particle* particles;
    size_t count;
    size_t max_nr;
    void remove(const size_t pos);
    
public:
    void add(particle p);
    void clear();
    void fill_component_list(
        vector<world_component> &list,
        const point &cam_tl = point(), const point &cam_br = point()
    );
    size_t get_count();
    void tick_all(const float delta_t);
    
    particle_manager(const size_t &max_nr = 0);
    particle_manager &operator=(const particle_manager &pg);
    ~particle_manager();
    
};


/* ----------------------------------------------------------------------------
 * Base class for the particle generator.
 * A particle generator creates particles in a steady flow and/or in a pattern.
 */
struct particle_generator {
private:
    float emission_timer;
    
public:
    //Optional ID, if you need to identify it later on.
    size_t id;
    //All particles created are based on this one.
    particle base_particle;
    //Number of particles to spawn.
    size_t number;
    //Interval at which to emit a new one. 0 means once only.
    float emission_interval;
    //Follow the given mob's coordinates.
    mob* follow_mob;
    //Offset the follow mob coordinates by this.
    point follow_pos_offset;
    //Offset the follow mob Z by this.
    float follow_z_offset;
    //Follow the given angle. e.g. a mob's angle.
    float* follow_angle;
    
    //Maximum random deviations of...
    size_t number_deviation;
    float duration_deviation;
    float friction_deviation;
    float gravity_deviation;
    float size_deviation;
    point pos_deviation;
    point speed_deviation;
    float angle;
    float angle_deviation;
    float total_speed;
    float total_speed_deviation;
    
    particle_generator(
        const float emission_interval = 0.0f,
        const particle &base_particle = particle(), const size_t number = 1
    );
    void tick(const float delta_t, particle_manager &manager);
    void emit(particle_manager &manager);
    void reset();
    
};

#endif //ifndef PARTICLE_H
