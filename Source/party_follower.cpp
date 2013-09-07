#include <cstddef>
#include "party_follower.h"

party_follower::party_follower(float x, float y, float z, float max_move_speed, sector* sec) : mob(x, y, z, max_move_speed, sec){
	following_leader = NULL;
}