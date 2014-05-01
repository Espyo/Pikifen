#ifndef PARTICLE_H
#define PARTICLE_H

#include <allegro5/allegro.h>

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
    
    particle(const unsigned char type, ALLEGRO_BITMAP* const bitmap, const float x, const float y, const float speed_x, const float speed_y, const float friction, const float gravity, const float duration, const float size, const ALLEGRO_COLOR color);
    bool tick();    //Returns true if the tick was successful. Returns false if its time to live is over.
};

enum PARTICLE_TYPES {
    PARTICLE_TYPE_SQUARE,
    PARTICLE_TYPE_CIRCLE,
    PARTICLE_TYPE_BITMAP,
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    PARTICLE_TYPE_ENEMY_SPIRIT,
    PARTICLE_TYPE_SMACK,
};

#endif //ifndef PARTICLE_H