#include "pikmin.h"

pikmin::pikmin(pikmin_type* type, float x, float y, sector* sec) : mob(x, y, 0, type->max_move_speed, sec){
	this->type = type;
	hazard_time_left = -1;
	enemy_attacking = NULL;
	maturity = 0;
	burrowed = false;

	main_color = type->color;
}