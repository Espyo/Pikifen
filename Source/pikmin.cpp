#include "pikmin.h"

pikmin::pikmin(float x, float y, sector* sec, pikmin_type* type) : mob(x, y, 0, type, sec) {
    this->pik_type = type;
    hazard_time_left = -1;
    attacking_enemy = NULL;
    carrying_mob = NULL;
    wants_to_carry = NULL;
    maturity = 0;
    pluck_reserved = false;
}

pikmin::~pikmin() { }