#ifndef ENEMY_TYPE_INCLUDED
#define ENEMY_TYPE_INCLUDED

#include <string>
#include <vector>

#include "element.h"

using namespace std;

class enemy_type{
public:
	//Technical things.
	string name;
	enemy_state* initial_state;
	
	//Space-related things.
	float size;
	float movement_speed;
	
	//Behavior.
	unsigned short max_hp;
	unsigned char weight;			//Pikmin needed to carry.
	float sight_radius;
	float follow_range;				//Max distance it'll follow prey for until it goes back.
	vector<element*> body_type;
	unsigned char pikmin_seeds;
	unsigned short pokos;
	bool can_sleep;
	bool can_regenerate;
	float revive_rate;
};

#endif //ifndef ENEMY_TYPE_INCLUDED
