#ifndef PIKMIN_TYPE_INCLUDED
#define PIKMIN_TYPE_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "element.h"

using namespace std;

class pikmin_type{
public:
	string name;
	ALLEGRO_COLOR color;
	vector<element*> resistences;
	unsigned char attack_attribute;
	float carry_strength;
	float attack_power;
	float weight;
	float max_move_speed;
	float carry_speed;
	float size;
	bool has_onion;
	bool can_dig;
	bool can_fly;
	bool can_swim;
	bool can_latch;
	bool can_carry_bomb_rocks;

	pikmin_type();
};

#endif //ifndef PIKMIN_TYPE_INCLUDED
