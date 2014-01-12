#include "pikmin.h"

pikmin::pikmin(float x, float y, sector* sec, pikmin_type* type) : mob(x, y, 0, type, sec) {
    this->type = type;
    hazard_time_left = -1;
    enemy_attacking = NULL;
    carrying_mob = NULL;
    maturity = 0;
    burrowed = false;
    pluck_reserved = false;
    
    main_color = type->color;
}

pikmin::~pikmin() { }