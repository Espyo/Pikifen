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
	size = 1;

	go_to_target = false;
	gtt_instant = false;
	target_x = x;
	target_y = y;
	target_rel_x = NULL;
	target_rel_y = NULL;

	following_party = NULL;
	was_thrown = false;
	uncallable_period = 0;
}

void mob::tick(){
	float delta_t_mult = (1.0f / game_fps);

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
		float final_target_x = target_x, final_target_y = target_y;
		if(target_rel_x) final_target_x+=*target_rel_x;
		if(target_rel_y) final_target_y+=*target_rel_y;

		if(gtt_instant){

			x = final_target_x;
			y = final_target_y;
			speed_x = speed_y = 0;

		}else if(x != final_target_x || y != final_target_y){
						
			float dx = final_target_x - x, dy = final_target_y - y;
			if(fabs(dx) < 0.001){
				dx = 0;
				x = final_target_x;
			}
			if(fabs(dy) < 0.001){
				y = final_target_y;
				dy = 0;
			}
			float dist = sqrt(dx * dx + dy * dy);

			if(dist > 0){
				float move_amount = min(dist * game_fps / 2, max_move_speed);

				if(move_amount == 0)
					move_amount = move_amount;
			
				dx *= move_amount / dist;
				dy *= move_amount / dist;

				speed_x = dx;
				speed_y = dy;
			}
		}
	}
	
	//ToDo collisions

	//Other things.
	if(uncallable_period > 0){
		uncallable_period -= (1.0 / game_fps);
		if(uncallable_period < 0) uncallable_period = 0;
	}
}

void mob::set_target(float target_x, float target_y, float *target_rel_x, float *target_rel_y, bool instant){
	this->target_x = target_x; this->target_y = target_y;
	this->target_rel_x = target_rel_x; this->target_rel_y = target_rel_y;
	this->gtt_instant = instant;

	go_to_target = true;
}

void mob::remove_target(bool stop){
	go_to_target = false;
	if(stop){
		speed_x = 0;
		speed_y = 0;
	}
}

/*mob::mob(const mob& m2){
	main_color = m2.main_color;
	planned_moving_angle = m2.planned_moving_angle;
	planned_moving_intensity = m2.planned_moving_intensity;
	x = m2.x; y = m2.y; z = m2.z;
	speed_x = m2.speed_x; speed_y = m2.speed_y; speed_z = m2.speed_z;
	move_speed = m2.move_speed;
	acceleration = m2.acceleration;
	angle = m2.angle;
	size = m2.size;
	sec = m2.sec;
	target_x = m2.target_x; target_y = m2.target_y;
	target_rel_x = m2.target_rel_x; target_rel_y = m2.target_rel_y;
	go_to_target = m2.go_to_target;
	gtt_instant = m2.gtt_instant;
	following_party = m2.following_party;
	party = m2.party;
	was_thrown = m2.was_thrown;
	uncallable_period = m2.uncallable_period;
	weight = m2.weight;
	max_carriers = m2.max_carriers;
	carrier_info = m2.carrier_info;
}*/

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
