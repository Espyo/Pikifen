#ifndef HITBOX_INCLUDED
#define HITBOX_INCLUDED

#include <vector>

#include "element.h"

class hitbox{
	float x,y;	//Relative coordinates.
	float radius;
	float speed_x, speed_y;	//Movement speed, units per second.
	vector<element*> elements;
	float defense;	//If 0, this is not a damageable hitbox. Otherwise, it's a multiplier: if 1, damage is suffered as normal.
	float offense;	//If 0, this is not a damage hitbox. Otherwise, it's a multiplier: if 1, damage is dealt as normal.
};

#endif //ifndef