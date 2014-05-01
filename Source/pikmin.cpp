#include "pikmin.h"

pikmin::pikmin(const float x, const float y, sector* sec, pikmin_type* type) : mob(x, y, 0, type, sec) {
    this->pik_type = type;
    hazard_time_left = -1;
    attacking_mob = NULL;
    latched = false;
    attack_time = 0;
    being_chomped = false;
    carrying_mob = NULL;
    wants_to_carry = NULL;
    maturity = 0;
    pluck_reserved = false;
    team = MOB_TEAM_PLAYER_1; //ToDo
}

pikmin::~pikmin() { }

float pikmin::get_base_speed() {
    return pik_type->move_speed + pik_type->move_speed * this->maturity * MATURITY_SPEED_MULT;
}
