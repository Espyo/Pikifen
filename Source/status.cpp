#include "status.h"

status::status(const float speed_multiplier, const float attack_multiplier, const float defense_multiplier, const bool freezes_everything, const ALLEGRO_COLOR color, const unsigned char affects) {
    this->speed_multiplier = speed_multiplier;
    this->attack_multiplier = attack_multiplier;
    this->defense_multiplier = defense_multiplier;
    this->freezes_everything = freezes_everything;
    this->color = color;
    this->affects = affects;
}