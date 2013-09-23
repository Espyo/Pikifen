#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

class pikmin;

#include "const.h"
#include "mob.h"
#include "party_follower.h"
#include "pikmin.h"
#include "sector.h"

using namespace std;

class leader : public mob{
public:
	unsigned int health;
	unsigned int max_health;
	mob* holding_pikmin;
	sample_struct sfx_whistle;
	sample_struct sfx_dismiss;
	sample_struct sfx_name_call;

	leader(float x, float y, sector* sec);
};

#endif //ifndef LEADER_INCLUDED