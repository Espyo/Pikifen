#include "pikmin.h"

pikmin::pikmin(pikmin_type* type, float x, float y, sector* sec) : mob(x, y, 0, type->max_move_speed, sec){
	this->type = type;
	hazard_time_left = -1;
	enemy_attacking = NULL;
	carrying_treasure = NULL;
	maturity = 0;
	burrowed = false;
	size = 18; //ToDo

	main_color = type->color;
}