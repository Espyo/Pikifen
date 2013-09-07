#ifndef PARTY_FOLLOWER_INCLUDED
#define PARTY_FOLLOWER_INCLUDED

#include "mob.h"

class party_follower : public mob{
public:
	mob* following_leader;

	party_follower(float x, float y, float z, float max_move_speed, sector* sec);
};

#endif //ifndef PARTY_FOLLOWER_INCLUDED