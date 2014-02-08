#include "particle.h"
#include "vars.h"

particle::particle(unsigned char type, ALLEGRO_BITMAP* bitmap, float x, float y, float speed_x, float speed_y, float friction, float gravity, float time, float size, ALLEGRO_COLOR color) {
    this->type = type;
    this->bitmap = bitmap;
    this->x = x; this->y = y;
    starting_x = x; starting_y = y;
    this->speed_x = speed_x; this->speed_y = speed_y;
    this->friction = friction;
    this->gravity = gravity;
    this->time = time;
    this->starting_time = time;
    this->size = size;
    this->starting_size = size;
    this->color = color;
}

bool particle::tick() {
    time -= 1.0f / game_fps;
    
    if(time <= 0) return false;
    
    x += (1.0f / game_fps) * speed_x;
    y += (1.0f / game_fps) * speed_y;
    
    if(friction != 0) {
        speed_x *= 1 - ((1.0f / game_fps) * friction);
        speed_y *= 1 - ((1.0f / game_fps) * friction);
    }
    
    if(gravity != 0) {
        speed_y += (1.0f / game_fps) * gravity;
    }
    
    return true;
}