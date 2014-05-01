#include "particle.h"
#include "vars.h"

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

bool particle::tick() {
    time -= delta_t;
    
    if(time <= 0) return false;
    
    x += (delta_t) * speed_x;
    y += (delta_t) * speed_y;
    
    if(friction != 0) {
        speed_x *= 1 - ((delta_t) * friction);
        speed_y *= 1 - ((delta_t) * friction);
    }
    
    if(gravity != 0) {
        speed_y += (delta_t) * gravity;
    }
    
    return true;
}