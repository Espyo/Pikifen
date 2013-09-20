#include <allegro5\allegro_font.h>

#include "info_spot.h"

info_spot::info_spot(float x, float y, sector* sec, string text, bool fullscreen, ALLEGRO_FONT* font)
: mob(x, y, sec->floors[0].z, 0, sec){

	this->text = text;
	this->fullscreen = fullscreen;

	text_w = al_get_text_width(font, text.c_str());

	size = 32;
}