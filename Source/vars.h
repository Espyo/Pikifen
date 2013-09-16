#ifndef GLOBALS_INCLUDED
#define GLOBALS_INCLUDED

#include <vector>

#include <allegro5\allegro_font.h>

#include "leader.h"
#include "particle.h"
#include "pikmin.h"
#include "pikmin_type.h"
#include "spray_type.h"

using namespace std;

extern ALLEGRO_BITMAP* bmp_background;
extern ALLEGRO_BITMAP* bmp_blue;
extern ALLEGRO_BITMAP* bmp_blue_burrowed;
extern ALLEGRO_BITMAP* bmp_blue_idle;
extern ALLEGRO_BITMAP* bmp_bubble;
extern ALLEGRO_BITMAP* bmp_cursor;
extern ALLEGRO_BITMAP* bmp_day_bubble;
extern ALLEGRO_BITMAP* bmp_health_bubble;
extern ALLEGRO_BITMAP* bmp_idle_glow;
extern ALLEGRO_BITMAP* bmp_louie;
extern ALLEGRO_BITMAP* bmp_mouse_cursor;
extern ALLEGRO_BITMAP* bmp_olimar;
extern ALLEGRO_BITMAP* bmp_red;
extern ALLEGRO_BITMAP* bmp_red_burrowed;
extern ALLEGRO_BITMAP* bmp_red_idle;
extern ALLEGRO_BITMAP* bmp_shadow;
extern ALLEGRO_BITMAP* bmp_sun;
extern ALLEGRO_BITMAP* bmp_yellow;
extern ALLEGRO_BITMAP* bmp_yellow_burrowed;
extern ALLEGRO_BITMAP* bmp_yellow_idle;

extern mob*                  closest_party_member;
extern size_t                current_leader;
extern float                 cursor_x;           //Leader's cursor.
extern float                 cursor_y;
extern unsigned int          day;
extern float                 day_minutes;
extern float                 day_minutes_end;  //The day ends when the in-game minutes reach this value.
extern float                 day_minutes_per_irl_sec;  //Every real-life second, these many in-game minutes pass.
extern float                 day_minutes_start;  //The in-game minutes start with this value every day.
extern bool                  daylight_effect;
extern ALLEGRO_FONT*         font;
extern unsigned short        font_h;
extern unsigned char         game_fps;
extern float                 idle_glow_angle;
extern vector<leader>        leaders;
extern float                 mouse_cursor_x;     //The physical mouse's cursor.
extern float                 mouse_cursor_y;
extern vector<particle>      particles;
extern vector<pikmin>        pikmin_list;
extern vector<pikmin_type>   pikmin_types;
extern unsigned short	     scr_h;
extern unsigned short	     scr_w;
extern vector<unsigned long> sprays;             //How many of each spray the player has.
extern vector<spray_type>    spray_types;
extern float                 whistle_radius;
extern float                 whistle_max_hold;   //The whistle area is at max size. Hold the whistle for these many seconds.
extern bool                  whistling;          //Is the whistle currently being blown?

#endif //ifndef GLOBALS_INCLUDED