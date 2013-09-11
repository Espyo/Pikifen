#include "vars.h"
#include "const.h"

using namespace std;

ALLEGRO_BITMAP* bmp_background = NULL;
ALLEGRO_BITMAP* bmp_blue = NULL;
ALLEGRO_BITMAP* bmp_blue_burrowed = NULL;
ALLEGRO_BITMAP* bmp_bubble = NULL;
ALLEGRO_BITMAP* bmp_cursor = NULL;
ALLEGRO_BITMAP* bmp_day_bubble = NULL;
ALLEGRO_BITMAP* bmp_health_bubble = NULL;
ALLEGRO_BITMAP* bmp_louie = NULL;
ALLEGRO_BITMAP* bmp_mouse_cursor = NULL;
ALLEGRO_BITMAP* bmp_olimar = NULL;
ALLEGRO_BITMAP* bmp_red = NULL;
ALLEGRO_BITMAP* bmp_red_burrowed = NULL;
ALLEGRO_BITMAP* bmp_shadow = NULL;
ALLEGRO_BITMAP* bmp_sun = NULL;
ALLEGRO_BITMAP* bmp_yellow = NULL;
ALLEGRO_BITMAP* bmp_yellow_burrowed = NULL;

mob*                closest_party_member = NULL;
size_t              current_leader = 0;
float               cursor_x = 0;
float               cursor_y = 0;
unsigned int        day = 0;
float               day_minutes = 420;
float               day_minutes_end = 1140;
float               day_minutes_per_irl_sec = 20;//2.75;
float               day_minutes_start = 420;
bool                daylight_effect = true;
ALLEGRO_FONT*       font = NULL;
unsigned short      font_h;
unsigned char       game_fps = DEF_FPS;
vector<leader>      leaders;
float               mouse_cursor_x = 0;
float               mouse_cursor_y = 0;
vector<particle>    particles;
vector<pikmin>      pikmin_list;
vector<pikmin_type> pikmin_types;
unsigned short      scr_h = DEF_SCR_W;
unsigned short      scr_w = DEF_SCR_H;
float               whistle_radius = 0;
float               whistle_max_hold = 0;
bool                whistling = false;