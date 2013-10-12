#ifndef PARTICLE_H
#define PARTICLE_H

#include <allegro5/allegro.h>

class particle{
public:
	float x, y;
	float starting_x, starting_y;
	float speed_x, speed_y;
	float friction;
	float gravity;
	float time;
	float starting_time;
	float size;
	float starting_size;
	ALLEGRO_COLOR color;

	particle(float x, float y, float speed_x, float speed_y, float friction, float gravity, float time, float size, ALLEGRO_COLOR color);
	bool tick();    //Returns true if the tick was successful. Returns false if its time to live is over.
};

#endif //ifndef PARTICLE_H