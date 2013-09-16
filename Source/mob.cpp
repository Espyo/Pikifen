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

	following_party = NULL;
	was_thrown = false;
}

void mob::tick(){
	float delta_t_mult = (1.0f / game_fps);

	//Planned movement.
	/*if(z == sec->floor){
		//If you're in mid-air, it doesn't matter where you plan to move.
		angle_to_coordinates(angle, move_speed, &speed_x, &speed_y);
	}*/

	//Movement.
	bool was_airborne = z > sec->floor;
	x += delta_t_mult * speed_x;
	y += delta_t_mult * speed_y;
	z += delta_t_mult * speed_z;
	
	if(z <= sec->floor && was_airborne){
		z = sec->floor;
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
	if(go_to_target){
		float square_radius = delta_t_mult * max_move_speed * 0.5;

		if(
			x >= target_x - square_radius &&
			x <= target_x + square_radius &&
			y >= target_y - square_radius &&
			y <= target_y + square_radius){
				//Already there. No need to move.
				speed_x = speed_y = 0;
		}else{
			//ToDo stop movement if it's located in a small square around the target.
			angle = atan2(target_y - y, target_x - x);
			speed_x = cos(angle) * max_move_speed;
			speed_y = sin(angle) * max_move_speed;
		}
	}
	
	//ToDo collisions
}

mob::~mob(){}