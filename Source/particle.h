/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <allegro5/allegro.h>

/* ----------------------------------------------------------------------------
 * A particle is best described with examples:
 * A puff of smoke, a sparkle, a smack.
 * There are several different types, which
 * change the way they behave over time, how they move, etc.
 */
class particle {
public:
    unsigned char type;
    ALLEGRO_BITMAP* bitmap;
    float x, y;
    float starting_x, starting_y;
    float speed_x, speed_y;
    float friction;
    float gravity;
    float time;
    float duration;
    float size;
    float starting_size;
    ALLEGRO_COLOR color;
    
    particle(const unsigned char type, ALLEGRO_BITMAP* const bitmap, const float x, const float y, const float speed_x, const float speed_y, const float friction, const float gravity, const float duration, const float size, const ALLEGRO_COLOR &color);
    bool tick();    //Returns true if the tick was successful. Returns false if its time to live is over.
};



void random_particle_explosion(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float center_x, const float center_y, const float speed_min, const float speed_max, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR &color);
void random_particle_fire(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR &color);
void random_particle_splash(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR &color);
void random_particle_spray(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const float angle, const ALLEGRO_COLOR &color);



enum PARTICLE_TYPES {
    PARTICLE_TYPE_SQUARE,
    PARTICLE_TYPE_CIRCLE,
    PARTICLE_TYPE_BITMAP,
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    PARTICLE_TYPE_ENEMY_SPIRIT,
    PARTICLE_TYPE_SMACK,
};

#endif //ifndef PARTICLE_H
