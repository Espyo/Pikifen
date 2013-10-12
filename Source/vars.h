#ifndef GLOBALS_INCLUDED
#define GLOBALS_INCLUDED

#include <vector>

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

#include "const.h"
#include "info_spot.h"
#include "leader.h"
#include "nectar.h"
#include "onion.h"
#include "particle.h"
#include "pikmin.h"
#include "pikmin_type.h"
#include "spray_type.h"
#include "status.h"
#include "treasure.h"

using namespace std;

extern ALLEGRO_BITMAP* bmp_background;
extern ALLEGRO_BITMAP* bmp_blue[3];
extern ALLEGRO_BITMAP* bmp_blue_burrowed[3];
extern ALLEGRO_BITMAP* bmp_blue_idle[3];
extern ALLEGRO_BITMAP* bmp_blue_onion;
extern ALLEGRO_BITMAP* bmp_bubble;
extern ALLEGRO_BITMAP* bmp_cursor;
extern ALLEGRO_BITMAP* bmp_day_bubble;
extern ALLEGRO_BITMAP* bmp_health_bubble;
extern ALLEGRO_BITMAP* bmp_idle_glow;
extern ALLEGRO_BITMAP* bmp_louie;
extern ALLEGRO_BITMAP* bmp_mouse_cursor;
extern ALLEGRO_BITMAP* bmp_move_group_arrow;
extern ALLEGRO_BITMAP* bmp_olimar;
extern ALLEGRO_BITMAP* bmp_president;
extern ALLEGRO_BITMAP* bmp_red[3];
extern ALLEGRO_BITMAP* bmp_red_burrowed[3];
extern ALLEGRO_BITMAP* bmp_red_idle[3];
extern ALLEGRO_BITMAP* bmp_red_onion;
extern ALLEGRO_BITMAP* bmp_shadow;
extern ALLEGRO_BITMAP* bmp_sun;
extern ALLEGRO_BITMAP* bmp_ub_spray;
extern ALLEGRO_BITMAP* bmp_us_spray;
extern ALLEGRO_BITMAP* bmp_yellow[3];
extern ALLEGRO_BITMAP* bmp_yellow_burrowed[3];
extern ALLEGRO_BITMAP* bmp_yellow_idle[3];
extern ALLEGRO_BITMAP* bmp_yellow_onion;

extern sample_struct sfx_camera;
extern sample_struct sfx_dismiss;
extern sample_struct sfx_louie_whistle;
extern sample_struct sfx_louie_name_call;
extern sample_struct sfx_president_name_call;
extern sample_struct sfx_olimar_whistle;
extern sample_struct sfx_olimar_name_call;
extern sample_struct sfx_president_whistle;
extern sample_struct sfx_president_name_call;
extern sample_struct sfx_pikmin_called;
extern sample_struct sfx_pikmin_held;
extern sample_struct sfx_pikmin_plucked;
extern sample_struct sfx_pikmin_thrown;
extern sample_struct sfx_throw;

extern vector<unsigned int>  berries;
extern ALLEGRO_BITMAP*       bmp_error;
extern float                 cam_trans_pan_final_x;
extern float                 cam_trans_pan_final_y;
extern float                 cam_trans_pan_initi_x;
extern float                 cam_trans_pan_initi_y;
extern float                 cam_trans_pan_time_left;
extern float                 cam_trans_zoom_final_level;
extern float                 cam_trans_zoom_initi_level;
extern float                 cam_trans_zoom_time_left;
extern float                 cam_x;
extern float                 cam_y;
extern float                 cam_zoom;
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
extern vector<info_spot*>    info_spots;
extern vector<leader*>       leaders;
extern float                 mouse_cursor_x;     //The physical mouse's cursor.
extern float                 mouse_cursor_y;
extern float                 mouse_cursor_speed_x;
extern float                 mouse_cursor_speed_y;
extern vector<float>         move_group_arrows;  //Distance of the arrows that appear when the "move group to cursor" button is held.
extern float                 move_group_next_arrow_time; //Time remaining until the next arrow on the "move group arrows" appears.
extern float                 moving_group_angle;
extern float                 moving_group_intensity;
extern bool                  moving_group_to_cursor;       //Is the "move group to cursor" button being pressed?
extern vector<nectar*>       nectars;
extern vector<onion*>        onions;
extern unsigned char         particle_quality;
extern vector<particle>      particles;
extern bool                  paused;
extern vector<unsigned long> pikmin_in_onions;
extern vector<pikmin*>       pikmin_list;
extern vector<pikmin_type>   pikmin_types;
extern bool                  running;
extern unsigned short	     scr_h;
extern unsigned short	     scr_w;
extern unsigned int          selected_spray;
extern bool                  smooth_scaling;     //If false, images that are scaled up and down will look pixelated.
extern vector<unsigned long> sprays;             //How many of each spray the player has.
extern vector<spray_type>    spray_types;
extern vector<status>        statuses;
extern string                total_error_log;
extern vector<treasure*>     treasures;
extern float                 whistle_dot_offset; //How much each dot of the whistle should spin.
extern float                 whistle_dot_radius[6]; //Radius of every 6th dot.
extern float                 whistle_fade_radius; //Radius the whistle was at pre-fade.
extern float                 whistle_fade_time;  //Time left for the whistle's fading animations.
extern bool                  whistle_is_circle;  //If true, the whistle radius is merely drawn as a circle. Used to improve performance.
extern float                 whistle_max_hold;   //The whistle area is at max size. Hold the whistle for these many seconds.
extern float                 whistle_next_dot_time;
extern float                 whistle_next_ring_time;
extern float                 whistle_radius;
extern vector<unsigned char> whistle_ring_colors;
extern unsigned char         whistle_ring_prev_color;
extern vector<float>         whistle_rings;
extern bool                  whistling;          //Is the whistle currently being blown?

#endif //ifndef GLOBALS_INCLUDED