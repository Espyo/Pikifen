#ifndef SPRAY_TYPE_INCLUDED
#define SPRAY_TYPE_INCLUDED

#include <allegro5\allegro.h>

class spray_type{
public:
	bool burpable;             //If true, the spray is applied to the front. If false, to the back.
	
	ALLEGRO_COLOR main_color;

	unsigned int berries_needed; //How many berries are needed in order to concot a new spray. 0 means there are no berries for this spray type.
	bool can_drop_blobs;         //Is it possible for the game to randomly give spray blobs of this spray type?

	spray_type(bool burpable, ALLEGRO_COLOR main_color, bool can_drop_blobs = true, unsigned int berries_needed = 10);
};

#endif //ifndef SPRAY_TYPE_INCLUDED