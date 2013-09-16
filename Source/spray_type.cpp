#include "spray_type.h"

spray_type::spray_type(bool burpable, ALLEGRO_COLOR main_color, bool can_drop_blobs, unsigned int berries_needed){
	this->burpable = burpable;
	this->main_color = main_color;
	this->can_drop_blobs = can_drop_blobs;
	this->berries_needed = berries_needed;
}