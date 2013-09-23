#include "const.h"
#include "functions.h"
#include "mob.h"
#include "vars.h"

mob::mob(float x, float y, float z, float max_move_speed, sector* sec){
	this->x = x;
	this->y = y;
	this->z = z;
	this->max_move_speed = max_move_speed;
	this->sec = sec;

	speed_x = speed_y = speed_z = 0;
	move_speed = 0;
	angle = 0;
	go_to_target = false;
	target_x = x;
	target_y = y;
	size = 1;

	following_party = NULL;
	was_thrown = false;
	uncallable_period = 0;
}

void mob::tick(){
	float delta_t_mult = (1.0f / game_fps);

	//Planned movement.
	/*if(z == sec->floor){
		//If you're in mid-air, it doesn't matter where you plan to move.
		angle_to_coordinates(angle, move_speed, &speed_x, &speed_y);
	}*/

	//Movement.
	bool was_airborne = z > sec->floors[0].z;
	x += delta_t_mult * speed_x;
	y += delta_t_mult * speed_y;
	z += delta_t_mult * speed_z;
	
	if(z <= sec->floors[0].z && was_airborne){
		z = sec->floors[0].z;
		speed_x = 0;
		speed_y = 0;
		speed_z = 0;
		was_thrown = false;
	}

	//Gravity.
	if(speed_z != 0){
		speed_z += (1.0f / game_fps) * (GRAVITY_ADDER);
	}

	//Automated movement.
	if(go_to_target && speed_z == 0){
		float square_radius = delta_t_mult * max_move_speed * 0.5;

		if(
			x >= target_x - square_radius &&
			x <= target_x + square_radius &&
			y >= target_y - square_radius &&
			y <= target_y + square_radius){
				//Already there. No need to move.
				speed_x = speed_y = 0;
		}else{
			angle = atan2(target_y - y, target_x - x);
			speed_x = cos(angle) * max_move_speed;
			speed_y = sin(angle) * max_move_speed;
		}
	}
	
	//ToDo collisions

	//Other things.
	if(uncallable_period > 0){
		uncallable_period -= (1.0 / game_fps);
		if(uncallable_period < 0) uncallable_period = 0;
	}
}

mob::~mob(){}

carrier_info_struct::carrier_info_struct(mob* m){
	current_n_carriers = 0;
	for(size_t c=0; c<m->max_carriers; c++){
		carrier_spots.push_back(NULL);
		float angle = (M_PI*2) / m->max_carriers * c;
		carrier_spots_x.push_back(cos(angle) * m->size);
		carrier_spots_y.push_back(sin(angle) * m->size);
	}
}