#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

class pikmin;

#include "mob.h"
#include "party_follower.h"
#include "pikmin.h"
#include "sector.h"

using namespace std;

class leader : public mob{
public:
	unsigned int health;
	mob* holding_pikmin;

	leader(float x, float y, sector* sec);
};

#endif //ifndef LEADER_INCLUDED