#ifndef PIKMIN_INCLUDED
#define PIKMIN_INCLUDED

class leader;

#include "enemy.h"
#include "leader.h"
#include "party_follower.h"
#include "pikmin_type.h"
#include "treasure.h"

class treasure;

class pikmin : public mob{
public:
	pikmin_type* type;
	float hazard_time_left;     //Time it has left until it drowns/chokes/etc.
	enemy* enemy_attacking;     //Enemy it's attacking.
	treasure* carrying_treasure;//Treasure it's carrying.
	unsigned char maturity;     //0: leaf. 1: bud. 2: flower.
	bool burrowed;

	pikmin(pikmin_type* type, float x, float y, sector* sec);
};

#endif //ifndef PIKMIN_INCLUDED
