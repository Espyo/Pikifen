#ifndef INFO_POINT_INCLUDED
#define INFO_POINT_INCLUDED

#include <string>

#include "mob.h"

using namespace std;

class info_spot : public mob{
public:
	string text;
	bool fullscreen;
	unsigned int text_w;

	info_spot(float x, float y, sector* sec, string text, bool fullscreen, ALLEGRO_FONT* font);
};

#endif //ifndef INFO_POINT_INCLUDED