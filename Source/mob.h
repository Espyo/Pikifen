#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "const.h"
#include "pikmin_type.h"
#include "sector.h"

using namespace std;

struct group_spot_info;

class mob;

struct carrier_info_struct{
	unsigned int max_carriers;
	bool carry_to_ship; //If true, this is carried to the ship. Otherwise, it's carried to an Onion.
	vector<float> carrier_spots_x; //These are the relative coordinates of each spot. They avoid calculating several sines and cosines over and over.
	vector<float> carrier_spots_y;

	size_t current_n_carriers;     //This is to avoid going through the vector to find out how many are carrying the mob.
	vector<mob*> carrier_spots;    //Pikmin carrying, and their spots.
	pikmin_type* decided_type;     //Current Onion type it's being taken to.

	carrier_info_struct(mob* m, unsigned int max_carriers, bool carry_to_ship);
	~carrier_info_struct();
};

class mob{
public:
	mob(float x, float y, float z, float move_speed, sector* sec);
	//mob(const mob& m2);
	//mob& operator=(const mob& m2);
	virtual ~mob(); //Needed so that typeid works.

	//Flags.
	bool to_delete; //If true, this mob should be deleted.
	bool reached_destination;

	//Detail things.
	ALLEGRO_COLOR main_color;

	//Planned moving.
	float planned_moving_angle;
	float planned_moving_intensity;

	//Actual moving and other physics.
	float x, y, z;		//Coordinates. Z is height, the higher the value, the higher in the sky.
	float speed_x, speed_y, speed_z;  //Physics only. Don't touch.
	float move_speed;                 //Normal moving speed.
	float move_speed_mult;            //Multiply the normal moving speed by this.
	float acceleration;        //Speed multiplies by this much each second.
	float angle;		//0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
	float size;			//Diameter, in units. Used mostly for movement.
	
	sector* sec;        //Sector it's on.

	//Target things.
	float target_x, target_y;  //When movement is automatic, this is the spot the mob is trying to go to.
	float *target_rel_x, *target_rel_y; //Follow these coordinates.
	bool go_to_target;         //If true, it'll try to go to the target spot on its own.
	bool gtt_instant; //If true, teleport instantly.
	void set_target(float target_x, float target_y, float *target_rel_x, float *target_rel_y, bool instant);
	void remove_target(bool stop);

	//Party things.
	mob* following_party;  //The current mob is following this mob's party.
	vector<mob*> party;
	bool was_thrown;       //Is the mob airborne because it was thrown?
	float uncallable_period; //During this period, the mob cannot be called into a party.
	group_spot_info* group_spots;

	//Carrying.
	unsigned int weight;
	carrier_info_struct* carrier_info; //This is a pointer because most mobs aren't carriable. Might as well save RAM.
	
	void tick();
};

#endif //ifndef MOB_INCLUDED
