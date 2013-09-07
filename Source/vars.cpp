#include "vars.h"
#include "const.h"

using namespace std;

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