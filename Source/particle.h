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
    float starting_time;
    float size;
    float starting_size;
    ALLEGRO_COLOR color;
    
    particle(unsigned char type, ALLEGRO_BITMAP* bitmap, float x, float y, float speed_x, float speed_y, float friction, float gravity, float time, float size, ALLEGRO_COLOR color);
    bool tick();    //Returns true if the tick was successful. Returns false if its time to live is over.
};

enum PARTICLE_TYPES {
    PARTICLE_TYPE_SQUARE,
    PARTICLE_TYPE_CIRCLE,
    PARTICLE_TYPE_BITMAP,
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    PARTICLE_TYPE_ENEMY_SPIRIT,
};

#endif //ifndef PARTICLE_H