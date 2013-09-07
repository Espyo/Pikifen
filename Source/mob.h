#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <vector>

#include "sector.h"

using namespace std;

class mob{
public:
	mob(float x, float y, float z, float max_move_speed, sector* sec);

	//Planned moving.
	float planned_moving_angle;
	float planned_moving_intensity;

	//Actual moving and other physics.
	float x, y, z;		//Coordinates. Z is height, the higher the value, the higher in the sky.
	float speed_x, speed_y, speed_z;  //Physics only. Don't touch.
	float move_speed;                 //Speed it's moving at, in the indicated angle.
	float max_move_speed;      //It can't move any faster than this (pixels per second).
	float acceleration;        //Speed multiplies by this much each second.
	float angle;		//0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
	float size;			//Radius, in units. Used mostly for movement.
	
	sector* sec;        //Sector it's on.

	//Target things.
	float target_x, target_y;  //When movement is automatic, this is the spot the mob is trying to go to.
	bool go_to_target;         //If true, it'll try to go to the target spot on its own.

	//Others
	mob* following_party;  //The current mob is following this mob's party.
	vector<mob*> party;
	
	void tick();

	virtual ~mob(); //Needed so that typeid works.
};

#endif //ifndef MOB_INCLUDED
