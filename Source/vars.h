#ifndef GLOBALS_INCLUDED
#define GLOBALS_INCLUDED

#include <vector>

#include <allegro5\allegro_font.h>

#include "leader.h"
#include "onion.h"
#include "particle.h"
#include "pikmin.h"
#include "pikmin_type.h"
#include "spray_type.h"
#include "status.h"
#include "treasure.h"

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

extern vector<unsigned int>  berries;
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
extern ALLEGRO_FONT*         font_area_name;
extern unsigned short        font_h;
extern unsigned char         game_fps;
extern float                 graphic_scale;
extern float                 idle_glow_angle;
extern vector<leader>        leaders;
extern float                 mouse_cursor_x;     //The physical mouse's cursor.
extern float                 mouse_cursor_y;
extern vector<onion>         onions;
extern unsigned char         particle_quality;
extern vector<particle>      particles;
extern vector<unsigned long> pikmin_in_onions;
extern vector<pikmin>        pikmin_list;
extern vector<pikmin_type>   pikmin_types;
extern unsigned short	     scr_h;
extern unsigned short	     scr_w;
extern bool                  smooth_scaling;     //If false, images that are scaled up and down will look pixelated.
extern vector<unsigned long> sprays;             //How many of each spray the player has.
extern vector<spray_type>    spray_types;
extern vector<status>        statuses;
extern vector<treasure>      treasures;
extern float                 whistle_radius;
extern float                 whistle_max_hold;   //The whistle area is at max size. Hold the whistle for these many seconds.
extern bool                  whistling;          //Is the whistle currently being blown?
extern float                 zoom_level;

#endif //ifndef GLOBALS_INCLUDED