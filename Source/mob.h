#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <vector>

#include <allegro5\allegro.h>

#include "const.h"
#include "sector.h"

using namespace std;

class mob;

struct carrier_info_struct{
	size_t current_n_carriers;     //This is to avoid going through the vector to find out how many are carrying the mob.
	vector<mob*> carrier_spots;    //Pikmin carrying, and their spots.
	vector<float> carrier_spots_x; //These are the relative coordinates of each spot. They avoid calculating several sines and cosines over and over.
	vector<float> carrier_spots_y;

	carrier_info_struct(mob* m);
};

class mob{
public:
	mob(float x, float y, float z, float max_move_speed, sector* sec);

	//Detail things.
	ALLEGRO_COLOR main_color;

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

	//Party things.
	mob* following_party;  //The current mob is following this mob's party.
	vector<mob*> party;
	bool was_thrown;       //Is the mob airborne because it was thrown?
	float uncallable_period; //During this period, the mob cannot be called into a party.

	//Carrying.
	unsigned int weight;
	unsigned int max_carriers;
	carrier_info_struct* carrier_info; //This is a pointer because most mobs aren't carriable. Might as well save RAM.
	
	void tick();

	virtual ~mob(); //Needed so that typeid works.
};

#endif //ifndef MOB_INCLUDED
