/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible variables.
 */

#include <limits.h>

#include "area_editor.h"
#include "vars.h"
#include "const.h"

using namespace std;

ALLEGRO_BITMAP* bmp_bubble = NULL;
ALLEGRO_BITMAP* bmp_checkbox_check = NULL;
ALLEGRO_BITMAP* bmp_cursor = NULL;
ALLEGRO_BITMAP* bmp_day_bubble = NULL;
ALLEGRO_BITMAP* bmp_enemy_spirit = NULL;
ALLEGRO_BITMAP* bmp_hard_bubble = NULL;
ALLEGRO_BITMAP* bmp_icon = NULL;
ALLEGRO_BITMAP* bmp_idle_glow = NULL;
ALLEGRO_BITMAP* bmp_info_spot = NULL;
ALLEGRO_BITMAP* bmp_message_box = NULL;
ALLEGRO_BITMAP* bmp_mouse_cursor = NULL;
ALLEGRO_BITMAP* bmp_group_move_arrow = NULL;
ALLEGRO_BITMAP* bmp_nectar = NULL;
ALLEGRO_BITMAP* bmp_no_pikmin = NULL;
ALLEGRO_BITMAP* bmp_number_bubble = NULL;
ALLEGRO_BITMAP* bmp_pikmin_spirit = NULL;
ALLEGRO_BITMAP* bmp_shadow = NULL;
ALLEGRO_BITMAP* bmp_ship = NULL;
ALLEGRO_BITMAP* bmp_smack = NULL;
ALLEGRO_BITMAP* bmp_smoke = NULL;
ALLEGRO_BITMAP* bmp_sparkle = NULL;
ALLEGRO_BITMAP* bmp_sun = NULL;
ALLEGRO_BITMAP* bmp_sun_bubble = NULL;
ALLEGRO_BITMAP* bmp_tp = NULL;
ALLEGRO_BITMAP* bmp_ub_spray = NULL;
ALLEGRO_BITMAP* bmp_us_spray = NULL;

ALLEGRO_BITMAP* bmp_test = NULL;

sample_struct sfx_attack;
sample_struct sfx_camera;
sample_struct sfx_dismiss;
sample_struct sfx_louie_whistle;
sample_struct sfx_louie_name_call;
sample_struct sfx_olimar_whistle;
sample_struct sfx_olimar_name_call;
sample_struct sfx_president_whistle;
sample_struct sfx_president_name_call;
sample_struct sfx_pikmin_attack;
sample_struct sfx_pikmin_called;
sample_struct sfx_pikmin_carrying;
sample_struct sfx_pikmin_carrying_grab;
sample_struct sfx_pikmin_caught;
sample_struct sfx_pikmin_dying;
sample_struct sfx_pikmin_held;
sample_struct sfx_pikmin_idle;
sample_struct sfx_pikmin_pluck;
sample_struct sfx_pikmin_plucked;
sample_struct sfx_pikmin_thrown;
sample_struct sfx_switch_pikmin;
sample_struct sfx_throw;

ALLEGRO_FONT*                    allegro_font = NULL;
vector<vector<ALLEGRO_BITMAP*> > area_images;
float                            area_images_scale = 1;
int                              area_image_size = 0;
float                            area_images_x1 = 0;
float                            area_images_y1 = 0;
timer                            area_title_fade_timer(AREA_TITLE_FADE_DURATION);
string                           area_to_load;
vector<unsigned int>             berries;
bmp_manager                      bitmaps;
ALLEGRO_BITMAP*                  bmp_error = NULL;
vector<bridge*>                  bridges;
float                            cam_trans_pan_final_x = 0;
float                            cam_trans_pan_final_y = 0;
float                            cam_trans_pan_initial_x = 0;
float                            cam_trans_pan_initial_y = 0;
timer                            cam_trans_pan_timer(CAM_TRANSITION_DURATION);
float                            cam_trans_zoom_final_level = 1;
float                            cam_trans_zoom_initial_level = 1;
timer                            cam_trans_zoom_timer(CAM_TRANSITION_DURATION);
float                            cam_x = 0;
float                            cam_y = 0;
float                            cam_zoom = 1;
mob*                             closest_party_member = NULL;
vector<vector<control_info> >    controls;
area_data                        cur_area_data;
size_t                           cur_leader_nr = 0;
leader*                          cur_leader_ptr = nullptr;
string                           cur_message;
size_t                           cur_message_char = 0;
timer                            cur_message_char_timer(MESSAGE_CHAR_INTERVAL);
size_t                           cur_message_section = 0;
ALLEGRO_BITMAP*                  cur_message_speaker = NULL;
vector<size_t>                   cur_message_stopping_chars;
unsigned char                    cur_game_state_nr = GAME_STATE_GAME;
float                            cur_sun_strength = 1;
float                            cursor_angle = 0;
float                            cursor_height_diff_light = 0;
float                            cursor_invalid_effect = 0;
movement_struct                  cursor_movement;
timer                            cursor_save_timer(CURSOR_SAVE_INTERVAL);
float                            cursor_spin_angle = 0;
vector<point>                    cursor_spots;
float                            cursor_x = 0;
float                            cursor_y = 0;
unsigned int                     day = 1;
float                            day_minutes = 60 * 7;
float                            day_minutes_end = 60 * 19;
float                            day_minutes_per_irl_sec = 2;
float                            day_minutes_start = 60 * 7;
bool                             daylight_effect = true;
double                           delta_t = 0;
string                           dev_tool_area_image_name;
bool                             dev_tool_area_image_shadows = true;
int                              dev_tool_area_image_size = 2048;
unsigned char                    dev_tool_keys[9];
pikmin_type*                     dev_tool_last_pikmin_type = NULL;
ALLEGRO_DISPLAY*                 display = NULL;
bool                             draw_cursor_trail = true;
double                           prev_frame_time = 0;
map<string, enemy_type*>         enemy_types;
vector<enemy*>                   enemies;
fade_manager                     fade_mgr;
ALLEGRO_FONT*                    font = NULL;
ALLEGRO_FONT*                    font_area_name = NULL;
ALLEGRO_FONT*                    font_counter = NULL;
unsigned int                     font_counter_h = 0;
unsigned int                     font_h = 0;
ALLEGRO_FONT*                    font_value = NULL;
unsigned int                     framerate_counter = 30;
timer                            framerate_update_timer(0.3f);
unsigned short                   game_fps = DEF_FPS;
string                           game_name;
map<size_t, game_state*>         game_states;
string                           game_version;
vector<gate*>                    gates;
map<string, gate_type*>          gate_types;
float                            group_move_angle = 0;
vector<float>                    group_move_arrows;
float                            group_move_intensity = 0;
timer                            group_move_next_arrow_timer(GROUP_MOVE_ARROWS_INTERVAL);
bool                             group_move_go_to_cursor = false;
movement_struct                  group_movement;
vector<vector<float> >           group_spots_x;
vector<vector<float> >           group_spots_y;
float                            idle_glow_angle = 0;
string                           info_print_text;
timer                            info_print_timer = timer(INFO_PRINT_DURATION, [] () { info_print_text.clear(); });
vector<info_spot*>               info_spots;
map<ALLEGRO_JOYSTICK*, int>      joystick_numbers;
vector<leader*>                  leaders;
movement_struct                  leader_movement;
map<string, leader_type*>        leader_types;
unsigned                         max_pikmin_in_field = 100;
ALLEGRO_MIXER*                   mixer = NULL;
mob_category_manager             mob_categories;
vector<mob*>                     mobs;
float                            mouse_cursor_x = scr_w / 2 + CURSOR_MAX_DIST;
float                            mouse_cursor_y = scr_h / 2;
bool                             mouse_cursor_valid = true;
bool                             mouse_moves_cursor[4] = {true, false, false, false};
vector<nectar*>                  nectars;
bool                             no_error_logs_today = true;
map<string, onion_type*>         onion_types;
vector<onion*>                   onions;
unsigned char                    particle_quality = 2;
vector<particle>                 particles;
bool                             paused = false;
map<string, pellet_type*>        pellet_types;
vector<pellet*>                  pellets;
vector<point>                    percipitation;
timer                            percipitation_timer(0);
unsigned char                    pikmin_ai_portion = 0; //On this frame, handle the nth portion of the Pikmin's AI. There are N_PIKMIN_AI_PORTIONS;
map<pikmin_type*, unsigned long> pikmin_in_onions;
vector<pikmin*>                  pikmin_list;
map<string, pikmin_type*>        pikmin_types;
bool                             pretty_whistle = true;
float                            prev_group_move_intensity = 0;
bool                             reset_delta_t = true;
bool                             running = true;
unsigned short                   scr_h = DEF_SCR_H;
unsigned short                   scr_w = DEF_SCR_W;
sector_types_manager             sector_types;
unsigned int                     selected_spray = 0;
unsigned char                    ship_beam_ring_color[3] = {0, 0, 0};
bool                             ship_beam_ring_color_up[3] = {true, true, true};
map<string, ship_type*>          ship_types;
vector<ship*>                    ships;
bool                             show_framerate = false;
bool                             smooth_scaling = true;
map<string, mob_type*>           spec_mob_types;
vector<unsigned long>            spray_amounts;
vector<spray_type>               spray_types;
vector<status>                   statuses;
float                            sun_meter_sun_angle = 0;
timer                            throw_particle_timer(THROW_PARTICLE_INTERVAL);
map<string, treasure_type*>      treasure_types;
vector<treasure*>                treasures;
float                            tree_shadow_sway = 0;
ALLEGRO_VOICE*                   voice = NULL;
map<string, weather>             weather_conditions;
float                            whistle_dot_offset = 0;
float                            whistle_dot_radius[6] = { -1, -1, -1, -1, -1, -1 };
float                            whistle_fade_radius = 0;
timer                            whistle_fade_timer(WHISTLE_FADE_TIME);
timer                            whistle_next_dot_timer(WHISTLE_DOT_INTERVAL);
timer                            whistle_next_ring_timer(WHISTLE_RINGS_INTERVAL);
float                            whistle_radius = 0;
vector<unsigned char>            whistle_ring_colors;
unsigned char                    whistle_ring_prev_color = 0;
vector<float>                    whistle_rings;
bool                             whistling = false;
bool                             window_pos_hack = false;

