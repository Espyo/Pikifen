/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle class and particle-related functions.
 */

#include "functions.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a particle.
 * type:     The type of particle. Use PARTICLE_TYPE_*.
 * bitmap:   Bitmap to use. NULL if none.
 * x, y:     Starting coordinates.
 * speed_*:  Speed at which it moves, in units per second.
 * friction: Every second, the speed is multiplied by this much.
 * gravity:  Every second, the vertical speed is multiplied by this.
 * duration: How long its lifespan is.
 * size:     Size; diameter.
 * color:    Its color.
 */
particle::particle(const unsigned char type, ALLEGRO_BITMAP* const bitmap, const float x, const float y, const float speed_x, const float speed_y, const float friction, const float gravity, const float duration, const float size, const ALLEGRO_COLOR color) {
    this->type = type;
    this->bitmap = bitmap;
    this->x = x; this->y = y;
    starting_x = x; starting_y = y;
    this->speed_x = speed_x; this->speed_y = speed_y;
    this->friction = friction;
    this->gravity = gravity;
    this->time = duration;
    this->duration = duration;
    this->size = size;
    this->starting_size = size;
    this->color = color;
}


/* ----------------------------------------------------------------------------
 * Makes a particle follow a game tick.
 * Returns false if its lifespan is over and it should be deleted.
 */
bool particle::tick() {
    time -= delta_t;
    
    if(time <= 0) return false;
    
    x += delta_t* speed_x;
    y += delta_t* speed_y;
    
    if(friction != 0) {
        speed_x *= 1 - (delta_t* friction);
        speed_y *= 1 - (delta_t* friction);
    }
    
    if(gravity != 0) {
        speed_y += (delta_t) * gravity;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Generates random particles in an explosion fashion:
 ** they scatter from the center point at random angles,
 ** and drift off until they vanish.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the explosion.
 * speed_*:  Their speed is random within this range, inclusive.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_explosion(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float center_x, const float center_y, const float speed_min, const float speed_max, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle = randomf(0, M_PI * 2);
        float speed = randomf(speed_min, speed_max);
        
        float speed_x = cos(angle) * speed;
        float speed_y = sin(angle) * speed;
        
        particles.push_back(
            particle(
                type,
                bmp,
                center_x, center_y,
                speed_x, speed_y,
                1,
                0,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}


/* ----------------------------------------------------------------------------
 * Generates random particles in a fire fashion:
 ** the particles go up and speed up as time goes by.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the fire.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_fire(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                randomf(-6, 6),
                randomf(-10, -20),
                0,
                -1,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}


/* ----------------------------------------------------------------------------
 * Generates random particles in a splash fashion:
 ** the particles go up and are scattered horizontally,
 ** and then go down with the effect of gravity.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the splash.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_splash(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                randomf(-2, 2),
                randomf(-2, -4),
                0, 0.5,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}


/* ----------------------------------------------------------------------------
 * Generates random particles in a spray fashion:
 ** the particles go in the pointed direction,
 ** and move gradually slower as they fade into the air.
 ** Used on actual sprays in-game.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * origin_*: Origin point of the spray.
 * angle:    Angle to shoot at.
 * color:    Color of the particles.
 */
void random_particle_spray(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const float angle, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(35, 40);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle_offset = randomf(-M_PI_4, M_PI_4);
        
        float power = randomf(30, 90);
        float speed_x = cos(angle + angle_offset) * power;
        float speed_y = sin(angle + angle_offset) * power;
        
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                speed_x, speed_y,
                1,
                0,
                randomf(3, 4),
                randomf(28, 32),
                color
            )
        );
    }
}
