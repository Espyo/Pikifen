/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <allegro5/allegro.h>

enum PARTICLE_TYPES {
    PARTICLE_TYPE_SQUARE,
    PARTICLE_TYPE_CIRCLE,
    PARTICLE_TYPE_BITMAP,
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    PARTICLE_TYPE_ENEMY_SPIRIT,
    PARTICLE_TYPE_SMACK,
};

enum PARTICLE_GENERATOR_TYPES {
    PARTICLE_GENERATOR_PLAIN,
    PARTICLE_GENERATOR_EXPLOSION,
    PARTICLE_GENERATOR_FIRE,
    PARTICLE_GENERATOR_SPLASH,
    PARTICLE_GENERATOR_SPRAY,
};


//The particle manager can hold up information on these many particles, at most.
const size_t N_PARTICLES = 100;


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
    float x, y;
    //Current size, in diameter.
    float size;
    //Current movement speed.
    float speed_x, speed_y;
    //Current color.
    ALLEGRO_COLOR color;
    
    //Other stuff.
    //If true, this is only drawn if drawing before mobs. Else, after.
    bool before_mobs;
    
    particle(
        const unsigned char type = PARTICLE_TYPE_BITMAP,
        const float x = 0.0f, const float y = 0.0f,
        const float size = 0.0f, const float duration = 0.0f
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
    particle particles[N_PARTICLES];
    size_t count;
    void remove(const size_t pos);
    
public:
    void add(particle p);
    void tick_all(const float delta_t);
    void draw_all(const bool before_mobs);
    void clear();
    
    particle_manager();
    
};


/* ----------------------------------------------------------------------------
 * Base class for the particle generator.
 * A particle generator creates particles in a steady flow and/or in a pattern.
 */
struct particle_generator {
private:
    float emission_timer;
    float emission_interval;
    
public:
    //Optional ID, if you need to identify it later on.
    size_t id;
    //All particles created are based on this one.
    particle base_particle;
    //Number of particles to spawn.
    size_t number;
    
    //Maximum random deviations of...
    size_t number_deviation;
    float duration_deviation;
    float friction_deviation;
    float gravity_deviation;
    float size_deviation;
    float x_deviation;
    float y_deviation;
    float speed_x_deviation;
    float speed_y_deviation;
    float angle;
    float angle_deviation;
    float speed;
    float speed_deviation;
    float* follow_x;
    float* follow_y;
    
    particle_generator(
        const float emission_interval,
        particle base_particle, const size_t number
    );
    void tick(const float delta_t, particle_manager &manager);
    void emit(particle_manager &manager);
    
};

#endif //ifndef PARTICLE_H
