#include "pikmin_type.h"
#include "const.h"

pikmin_type::pikmin_type() {
    attack_attribute = 0;
    carry_strength = 1;
    attack_power = 1;
    weight = 1;
    max_move_speed = 1;
    carry_speed = 1;
    size = DEF_PIKMIN_SIZE;
    has_onion = true;
    can_dig = false;
    can_fly = false;
    can_swim = false;
    can_latch = true;
    can_carry_bomb_rocks = false;
}