#include "pikmin.h"

pikmin::pikmin(pikmin_type* type, float x, float y, sector* sec) : mob(x, y, 0, type->max_move_speed, sec) {
    this->type = type;
    hazard_time_left = -1;
    enemy_attacking = NULL;
    carrying_mob = NULL;
    maturity = 0;
    burrowed = false;
    pluck_reserved = false;
    size = 20; //ToDo
    
    main_color = type->color;
}

pikmin::~pikmin() { }