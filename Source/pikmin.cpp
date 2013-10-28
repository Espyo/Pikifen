#include "pikmin.h"

pikmin::pikmin(pikmin_type* type, float x, float y, sector* sec) : mob(x, y, 0, type->max_move_speed, sec){
	this->type = type;
	hazard_time_left = -1;
	enemy_attacking = NULL;
	carrying_treasure = NULL;
	maturity = 0;
	burrowed = false;
	size = 20; //ToDo

	main_color = type->color;
}

/*pikmin::pikmin(const pikmin& p2)
: mob(p2){
	type = p2.type;
	hazard_time_left = p2.hazard_time_left;
	enemy_attacking = p2.enemy_attacking;
	carrying_treasure = p2.carrying_treasure;
	carrying_spot = p2.carrying_spot;
	maturity = p2.maturity;
	burrowed = p2.burrowed;
}*/

pikmin::~pikmin(){ }