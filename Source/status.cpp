#include "status.h"

status::status(float speed_multiplier, float attack_multiplier, float defense_multiplier, float state_speed_multiplier, bool freezes_everything, ALLEGRO_COLOR color, unsigned char affects) {
    this->speed_multiplier = speed_multiplier;
    this->attack_multiplier = attack_multiplier;
    this->defense_multiplier = defense_multiplier;
    this->state_speed_multiplier = state_speed_multiplier;
    this->freezes_everything = freezes_everything;
    this->color = color;
    this->affects = affects;
}