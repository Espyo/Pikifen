#ifndef MISC_STRUCTS_INCLUDED
#define MISC_STRUCTS_INCLUDED

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "mob.h"

using namespace std;

struct sample_struct{
	ALLEGRO_SAMPLE*		sample;	//Pointer to the sample.
	ALLEGRO_SAMPLE_ID	id;		//Sample id.

	sample_struct(ALLEGRO_SAMPLE* s=NULL);
};

struct group_spot_info{
	/* Group spots. The way this works is that a Pikmin group surrounds a central point.
	 * There are several wheels surrounding the original spot,
	 * starting from the center and growing in size, each with several spots of their own.
	 * A Pikmin occupies the central spot first.
	 * Then other Pikmin come by, and occupy spots at random on the next wheel.
	 * When that wheel has all of its spots full, the next wheel will be used, and so on.
	 */
	vector<vector<float> > x_coords;
	vector<vector<float> > y_coords;
	unsigned n_wheels;

	vector<vector<mob*> > mobs_in_spots;
	unsigned current_wheel;
	unsigned n_current_wheel_members;

	group_spot_info(unsigned max_mobs, float spot_size);
	void add(mob* m, float* x, float* y);
	void remove(mob* m);
};

#endif //ifndef MISC_STRUCTS_INCLUDED