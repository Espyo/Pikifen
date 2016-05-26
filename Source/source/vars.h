/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the globally accessible variables.
 */

#ifndef VARS_INCLUDED
#define VARS_INCLUDED

#include <map>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

#include "animation.h"
#include "const.h"
#include "controls.h"
#include "game_state.h"
#include "mobs/gate.h"
#include "hitbox.h"
#include "LAFI/gui.h"
#include "LAFI/label.h"
#include "mobs/leader.h"
#include "liquid.h"
#include "misc_structs.h"
#include "mobs/onion.h"
#include "particle.h"
#include "mobs/pikmin.h"
#include "mobs/pikmin_type.h"
#include "sector.h"
#include "mobs/ship.h"
#include "mobs/ship_type.h"
#include "spray_type.h"
#include "mobs/bridge.h"
#include "mobs/info_spot.h"
#include "mobs/nectar.h"
#include "mobs/pellet.h"
#include "status.h"
#include "mobs/treasure.h"
#include "weather.h"

using namespace std;

//Bitmaps.
extern ALLEGRO_BITMAP* bmp_bubble;
extern ALLEGRO_BITMAP* bmp_checkbox_check;
extern ALLEGRO_BITMAP* bmp_cursor;
extern ALLEGRO_BITMAP* bmp_day_bubble;
extern ALLEGRO_BITMAP* bmp_enemy_spirit;
extern ALLEGRO_BITMAP* bmp_group_move_arrow;
extern ALLEGRO_BITMAP* bmp_hard_bubble;
extern ALLEGRO_BITMAP* bmp_icon;
extern ALLEGRO_BITMAP* bmp_idle_glow;
extern ALLEGRO_BITMAP* bmp_info_spot;
extern ALLEGRO_BITMAP* bmp_message_box;
extern ALLEGRO_BITMAP* bmp_mouse_cursor;
extern ALLEGRO_BITMAP* bmp_mouse_button_icon[3];
extern ALLEGRO_BITMAP* bmp_mouse_wd_icon;
extern ALLEGRO_BITMAP* bmp_mouse_wu_icon;
extern ALLEGRO_BITMAP* bmp_nectar;
extern ALLEGRO_BITMAP* bmp_no_pikmin;
extern ALLEGRO_BITMAP* bmp_notification;
extern ALLEGRO_BITMAP* bmp_number_bubble;
extern ALLEGRO_BITMAP* bmp_pikmin_spirit;
extern ALLEGRO_BITMAP* bmp_red[3];
extern ALLEGRO_BITMAP* bmp_shadow;
extern ALLEGRO_BITMAP* bmp_smack;
extern ALLEGRO_BITMAP* bmp_smoke;
extern ALLEGRO_BITMAP* bmp_sparkle;
extern ALLEGRO_BITMAP* bmp_sun;
extern ALLEGRO_BITMAP* bmp_sun_bubble;
extern ALLEGRO_BITMAP* bmp_ub_spray;
extern ALLEGRO_BITMAP* bmp_us_spray;
extern ALLEGRO_BITMAP* bmp_wave_ring;

//Sound effects.
extern sample_struct sfx_attack;
extern sample_struct sfx_camera;
extern sample_struct sfx_dismiss;
extern sample_struct sfx_louie_whistle;
extern sample_struct sfx_louie_name_call;
extern sample_struct sfx_president_name_call;
extern sample_struct sfx_olimar_whistle;
extern sample_struct sfx_olimar_name_call;
extern sample_struct sfx_president_whistle;
extern sample_struct sfx_president_name_call;
extern sample_struct sfx_pikmin_attack;
extern sample_struct sfx_pikmin_called;
extern sample_struct sfx_pikmin_carrying;
extern sample_struct sfx_pikmin_carrying_grab;
extern sample_struct sfx_pikmin_caught;
extern sample_struct sfx_pikmin_dying;
extern sample_struct sfx_pikmin_held;
extern sample_struct sfx_pikmin_idle;
extern sample_struct sfx_pikmin_pluck;
extern sample_struct sfx_pikmin_plucked;
extern sample_struct sfx_pikmin_thrown;
extern sample_struct sfx_switch_pikmin;
extern sample_struct sfx_throw;

//General globals.

extern ALLEGRO_FONT* allegro_font;
extern vector<vector<ALLEGRO_BITMAP*> > area_images;
extern float area_images_scale;
extern int area_image_size;
//Top-left corner of the area, in world coordinates.
extern float area_images_x1;
extern float area_images_y1;
//How much real time has passed since the area was loaded.
extern float area_time_passed;
extern timer area_title_fade_timer;
//Name of the area to load, from the area select.
extern string area_to_load;
extern vector<unsigned int> berries;
extern bmp_manager bitmaps;
extern ALLEGRO_BITMAP* bmp_error;
extern vector<bridge*> bridges;
extern float cam_final_x;
extern float cam_final_y;
extern float cam_final_zoom;
extern float cam_x;
extern float cam_y;
extern float cam_zoom;
extern ALLEGRO_COLOR carrying_color_move;
extern ALLEGRO_COLOR carrying_color_stop;
extern float carrying_speed_base_mult;
extern float carrying_speed_max_mult;
extern float carrying_speed_weight_mult;
extern vector<vector<control_info> > controls;
extern size_t click_control_id;
extern mob* closest_group_member;
extern area_data cur_area_data;
extern size_t cur_leader_nr;
extern leader* cur_leader_ptr;
extern string cur_message;
extern size_t cur_message_char;
extern timer cur_message_char_timer;
extern size_t cur_message_section;
extern ALLEGRO_BITMAP* cur_message_speaker;
//The message stops scrolling when it reaches one of these characters.
extern vector<size_t> cur_message_stopping_chars;
extern unsigned char cur_game_state_nr;
extern float cur_sun_strength;
extern float cursor_angle;
extern float cursor_height_diff_light;
//Effect for the invalid cursor fading in or out.
//The opacity is calculated using this number's sign.
extern float cursor_invalid_effect;
//Maximum distance away from the leader the cursor can go.
extern float cursor_max_dist;
//Movement of the cursor via non-mouse.
extern movement_struct cursor_movement;
//Time left until the position of the cursor is saved on the vector.
extern timer cursor_save_timer;
//How much the cursor spins per second.
extern float cursor_spin_speed;
//Spots the cursor has been through. Used for the faint trail left behind it.
extern vector<point> cursor_spots;
//Leader's cursor.
extern float cursor_x;
extern float cursor_y;
extern unsigned int day;
extern float day_minutes;
//The day ends when the in-game minutes reach this value.
extern float day_minutes_end;
//Every real-life second, these many in-game minutes pass.
extern float day_minutes_per_irl_sec;
//The in-game minutes start with this value every day.
extern float day_minutes_start;
extern bool daylight_effect;
//Time between the previous frame and the current.
extern double delta_t;
extern string dev_tool_area_image_name;
extern bool dev_tool_area_image_shadows;
extern int dev_tool_area_image_size;
extern bool dev_tool_change_speed;
extern float dev_tool_change_speed_mult;
extern string dev_tool_auto_start_option;
extern string dev_tool_auto_start_mode;
extern unsigned char dev_tool_keys[10];
extern pikmin_type* dev_tool_last_pikmin_type;
extern ALLEGRO_DISPLAY* display;
extern bool draw_cursor_trail;
extern map<string, enemy_type*> enemy_types;
extern vector<enemy*> enemies;
extern fade_manager fade_mgr;
extern ALLEGRO_FONT* font_main;
extern ALLEGRO_FONT* font_area_name;
extern ALLEGRO_FONT* font_counter;
extern unsigned int font_counter_h;
extern unsigned int font_main_h;
//Font for the carrying / money values.
extern ALLEGRO_FONT* font_value;
extern unsigned int framerate_counter;
extern timer framerate_update_timer;
extern unsigned short game_fps;
extern string game_name;
extern map<size_t, game_state*> game_states;
extern string game_version;
extern vector<gate*> gates;
extern map<string, gate_type*> gate_types;
extern float group_move_angle;
//Distance of the arrows that appear
//when the "move group to cursor" button is held.
extern vector<float> group_move_arrows;
//General intensity of the group move in the specified angle.
extern float group_move_intensity;
//Time remaining until the next arrow on the "move group arrows" appears.
extern timer group_move_next_arrow_timer;
//Is the "move group to cursor" button being pressed?
extern bool group_move_go_to_cursor;
extern float group_move_task_range;
//Joystick coordinates for the group movement.
extern movement_struct group_movement;
extern vector<vector<float> > group_spots_x;
extern vector<vector<float> > group_spots_y;
extern map<string, hazard> hazards;
extern float hud_coords[N_HUD_ITEMS][4];
extern float idle_task_range;
extern string info_print_text;
extern timer info_print_timer;
extern float info_spot_trigger_range;
extern vector<info_spot*> info_spots;
extern bool is_game_running;
extern map<ALLEGRO_JOYSTICK*, int> joystick_numbers;
extern map<string, liquid> liquids;
extern vector<leader*> leaders;
//How hard the joystick is pressed in each direction ([0, 1]);
extern movement_struct leader_movement;
extern map<string, leader_type*> leader_types;
//Every level of maturity, multiply the power by 1 + this much.
extern float maturity_power_mult;
//Every level of maturity, multiply the attack by 1 + this much.
extern float maturity_speed_mult;
extern size_t max_pikmin_in_field;
//These many seconds until a new character of the message is drawn.
extern float message_char_interval;
extern ALLEGRO_MIXER* mixer;
extern mob_category_manager mob_categories;
extern vector<mob*> mobs;
//The physical mouse's cursor.
extern float mouse_cursor_x;
extern float mouse_cursor_y;
extern bool mouse_cursor_valid;
extern bool mouse_moves_cursor[4];
//A drop of nectar starts with this amount.
extern size_t nectar_amount;
extern vector<nectar*> nectars;
//How far a leader can go to auto-pluck the next Pikmin.
extern float next_pluck_range;
//Have there been no errors in this play session?
extern bool no_error_logs_today;
extern float onion_open_range;
extern map<string, onion_type*> onion_types;
extern vector<onion*> onions;
extern unsigned char particle_quality;
extern particle_manager particles;
extern bool paused;
extern map<string, pellet_type*> pellet_types;
extern vector<pellet*> pellets;
extern vector<point> percipitation;
extern timer percipitation_timer;
extern float pikmin_grab_range;
extern map<pikmin_type*, unsigned long> pikmin_in_onions;
extern vector<pikmin*> pikmin_list;
extern map<string, pikmin_type*> pikmin_types;
extern float pluck_range;
//If true, the whistle radius is merely drawn as a circle.
//Used to improve performance.
extern bool pretty_whistle;
//Time since start, on the previous frame.
//Used to calculate the time difference between the current and last frames.
extern double prev_frame_time;
//The group move intensity on the previous frame.
//Used to figure out if, on 0-intensity,
//we should put the group's center near the leader or let it go there by itself.
extern float prev_group_move_intensity;
//Ready for gameplay-related input?
extern bool ready_for_input;
//Is delta_t meant to be reset for the next frame?
extern bool reset_delta_t;
extern unsigned short scr_h;
extern unsigned short scr_w;
extern sector_types_manager sector_types;
extern unsigned int selected_spray;
extern unsigned char ship_beam_ring_color[3];
extern bool ship_beam_ring_color_up[3];
extern map<string, ship_type*> ship_types;
extern vector<ship*> ships;
extern bool show_framerate;
//If false, images that are scaled up and down will look pixelated.
extern bool smooth_scaling;
extern map<string, mob_type*> spec_mob_types;
//How many of each spray the player has.
extern vector<unsigned long> spray_amounts;
extern vector<spray_type> spray_types;
extern map<string, status_type> status_types;
extern float transition_time;
extern bool transition_fade_in;
extern map<string, treasure_type*> treasure_types;
extern vector<treasure*> treasures;
//Voice from which the sound effects play.
extern ALLEGRO_VOICE* voice;
extern map<string, weather> weather_conditions;
//Radius of every 6th dot.
extern float whistle_dot_radius[6];
//Radius the whistle was at pre-fade.
extern float whistle_fade_radius;
//Time left for the whistle's fading animations.
extern timer whistle_fade_timer;
extern float whistle_growth_speed;
extern timer whistle_next_dot_timer;
extern timer whistle_next_ring_timer;
extern float whistle_radius;
extern vector<unsigned char> whistle_ring_colors;
extern unsigned char whistle_ring_prev_color;
extern vector<float> whistle_rings;
//Is the whistle currently being blown?
extern bool whistling;
//Should we force the window's positioning
//(on some systems it appears out-of-bounds by default)
extern bool window_pos_hack;
extern float zoom_max_level;
extern float zoom_min_level;


#endif //ifndef VARS_INCLUDED
