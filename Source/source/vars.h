/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "animation.h"
#include "const.h"
#include "controls.h"
#include "game_state.h"
#include "hitbox.h"
#include "LAFI/gui.h"
#include "LAFI/label.h"
#include "liquid.h"
#include "mob_script_action.h"
#include "mobs/bouncer.h"
#include "mobs/bridge.h"
#include "mobs/converter.h"
#include "mobs/decoration.h"
#include "mobs/drop.h"
#include "mobs/group_task.h"
#include "mobs/interactable.h"
#include "mobs/leader.h"
#include "mobs/onion.h"
#include "mobs/pellet.h"
#include "mobs/pikmin.h"
#include "mobs/pile.h"
#include "mobs/resource.h"
#include "mobs/scale.h"
#include "mobs/ship.h"
#include "mobs/tool.h"
#include "mobs/track.h"
#include "mobs/treasure.h"
#include "particle.h"
#include "replay.h"
#include "sector.h"
#include "spray_type.h"
#include "status.h"
#include "utils/geometry_utils.h"
#include "weather.h"

using std::size_t;
using std::string;
using std::vector;

//Bitmaps.
extern ALLEGRO_BITMAP* bmp_checkbox_check;
extern ALLEGRO_BITMAP* bmp_cursor;
extern ALLEGRO_BITMAP* bmp_cursor_invalid;
extern ALLEGRO_BITMAP* bmp_enemy_spirit;
extern ALLEGRO_BITMAP* bmp_icon;
extern ALLEGRO_BITMAP* bmp_idle_glow;
extern ALLEGRO_BITMAP* bmp_mouse_button_icon[3];
extern ALLEGRO_BITMAP* bmp_mouse_cursor;
extern ALLEGRO_BITMAP* bmp_mouse_wd_icon;
extern ALLEGRO_BITMAP* bmp_mouse_wu_icon;
extern ALLEGRO_BITMAP* bmp_notification;
extern ALLEGRO_BITMAP* bmp_pikmin_silhouette;
extern ALLEGRO_BITMAP* bmp_pikmin_spirit;
extern ALLEGRO_BITMAP* bmp_rock;
extern ALLEGRO_BITMAP* bmp_shadow;
extern ALLEGRO_BITMAP* bmp_smack;
extern ALLEGRO_BITMAP* bmp_smoke;
extern ALLEGRO_BITMAP* bmp_sparkle;
extern ALLEGRO_BITMAP* bmp_spotlight;
extern ALLEGRO_BITMAP* bmp_swarm_arrow;
extern ALLEGRO_BITMAP* bmp_wave_ring;

//Sound effects.
extern sample_struct sfx_attack;
extern sample_struct sfx_camera;
extern sample_struct sfx_pikmin_attack;
extern sample_struct sfx_pikmin_called;
extern sample_struct sfx_pikmin_carrying;
extern sample_struct sfx_pikmin_carrying_grab;
extern sample_struct sfx_pikmin_caught;
extern sample_struct sfx_pikmin_dying;
extern sample_struct sfx_pikmin_held;
extern sample_struct sfx_pikmin_idle;
extern sample_struct sfx_pluck;
extern sample_struct sfx_pikmin_plucked;
extern sample_struct sfx_pikmin_thrown;
extern sample_struct sfx_switch_pikmin;
extern sample_struct sfx_throw;

struct asset_file_names_struct {
    string area_name_font;
    string checkbox_check;
    string cursor;
    string cursor_invalid;
    string counter_font;
    string editor_icons;
    string enemy_spirit;
    string swarm_arrow;
    string icon;
    string idle_glow;
    string main_font;
    string main_menu;
    string mouse_button_icon[3];
    string mouse_cursor;
    string mouse_wd_icon;
    string mouse_wu_icon;
    string notification;
    string pikmin_silhouette;
    string pikmin_spirit;
    string rock;
    string shadow;
    string smack;
    string smoke;
    string sparkle;
    string spotlight;
    string value_font;
    string wave_ring;
};
extern asset_file_names_struct asset_file_names;

//General globals.

extern float area_editor_backup_interval;
extern float area_editor_grid_interval;
extern bool area_editor_show_edge_length;
extern bool area_editor_show_territory;
extern size_t area_editor_snap_threshold;
extern size_t area_editor_undo_limit;
extern unsigned char area_editor_view_mode;
extern timer area_title_fade_timer;
extern bmp_manager bitmaps;
extern ALLEGRO_BITMAP* bmp_error;
extern map<string, bouncer_type*> bouncer_types;
extern vector<bouncer*> bouncers;
extern vector<bridge*> bridges;
extern map<string, bridge_type*> bridge_types;
extern button_manager buttons;
//Minimum and maximum coordinates that are on-camera.
extern point cam_box[2];
extern point cam_final_pos;
extern float cam_final_zoom;
extern point cam_pos;
extern float cam_zoom;
extern bool can_throw_leaders;
extern ALLEGRO_COLOR carrying_color_move;
extern ALLEGRO_COLOR carrying_color_stop;
extern float carrying_speed_base_mult;
extern float carrying_speed_max_mult;
extern float carrying_speed_weight_mult;
extern vector<vector<control_info> > controls;
extern vector<converter*> converters;
extern map<string, converter_type*> converter_types;
extern bool creator_tool_area_image_shadows;
extern int creator_tool_area_image_size;
extern bool creator_tool_area_image_mobs;
extern string creator_tool_auto_start_option;
extern string creator_tool_auto_start_mode;
extern bool creator_tool_change_speed;
extern float creator_tool_change_speed_mult;
extern bool creator_tool_geometry_info;
extern bool creator_tool_hitboxes;
extern mob* creator_tool_info_lock;
extern pikmin_type* creator_tool_last_pikmin_type;
extern float creator_tool_mob_hurting_ratio;
//For each key (F2 - F11, 0 - 9), what tool is bound to it?
extern unsigned char creator_tool_keys[20];
extern bool creator_tools_enabled;
extern size_t cur_leader_nr;
extern leader* cur_leader_ptr;
extern string cur_message;
extern size_t cur_message_char;
extern timer cur_message_char_timer;
extern size_t cur_message_section;
extern ALLEGRO_BITMAP* cur_message_speaker;
//The message stops scrolling when it reaches one of these characters.
extern vector<size_t> cur_message_stopping_chars;
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
//Is the cursor in the window, and is the window active?
extern bool cursor_ready;
//Time left until the position of the cursor is saved on the vector.
extern timer cursor_save_timer;
//How much the cursor spins per second.
extern float cursor_spin_speed;
//Spots the cursor has been through. Used for the faint trail left behind it.
extern vector<point> cursor_spots;
extern map<string, mob_type*> custom_mob_types;
extern map<string, particle_generator> custom_particle_generators;
extern unsigned int day;
extern float day_minutes;
//The day ends when the in-game minutes reach this value.
extern float day_minutes_end;
//Every real-life second, these many in-game minutes pass.
extern float day_minutes_per_irl_sec;
//The in-game minutes start with this value every day.
extern float day_minutes_start;
extern map<string, decoration_type*> decoration_types;
extern vector<decoration*> decorations;
extern ALLEGRO_DISPLAY* display;
extern bool draw_cursor_trail;
extern vector<drop*> drops;
extern map<string, drop_type*> drop_types;
extern bool editor_mmb_pan;
extern float editor_mouse_drag_threshold;
//How many errors have been reported this application session.
extern size_t errors_reported_today;
extern map<string, enemy_type*> enemy_types;
extern vector<enemy*> enemies;
extern fade_manager fade_mgr;
extern ALLEGRO_FONT* font_area_name;
extern ALLEGRO_FONT* font_builtin;
extern ALLEGRO_FONT* font_counter;
extern ALLEGRO_FONT* font_main;
extern unsigned int font_counter_h;
extern unsigned int font_main_h;
//Font for the carrying / money values.
extern ALLEGRO_FONT* font_value;
extern vector<float> framerate_history;
extern size_t framerate_last_avg_point;
extern string game_name;
extern string game_version;
extern vector<group_task*> group_tasks;
extern map<string, group_task_type*> group_task_types;
extern map<string, hazard> hazards;
extern hud_item_manager hud_items;
extern float hud_coords[N_HUD_ITEMS][4];
extern ALLEGRO_TRANSFORM identity_transform;
extern float idle_task_range;
extern float info_print_duration;
extern float info_print_fade_duration;
extern string info_print_text;
extern timer info_print_timer;
extern bool intended_scr_fullscreen;
extern int intended_scr_h;
extern int intended_scr_w;
extern vector<interactable*> interactables;
extern map<string, interactable_type*> interactable_types;
//Is input enabled in general, for reasons outside the ready_for_input variable?
extern bool is_input_allowed;
extern float joystick_min_deadzone;
extern float joystick_max_deadzone;
extern map<ALLEGRO_JOYSTICK*, int> joystick_numbers;
extern map<string, liquid> liquids;
extern vector<leader*> leaders;
//Mob the leader's cursor is on top of, if any.
extern mob* leader_cursor_mob;
//Leader's cursor, in screen coordinates.
extern point leader_cursor_s;
//Sector the leader's cursor is on, if any.
extern sector* leader_cursor_sector;
//Leader's cursor, in world coordinates.
extern point leader_cursor_w;
//How hard the joystick is pressed in each direction ([0, 1]);
extern movement_struct leader_movement;
extern vector<leader_type*> leader_order;
extern vector<string> leader_order_strings;
extern map<string, leader_type*> leader_types;
extern ALLEGRO_BITMAP* lightmap_bmp;
//Loading screen main text buffer.
extern ALLEGRO_BITMAP* loading_text_bmp;
//Loading screen subtext buffer.
extern ALLEGRO_BITMAP* loading_subtext_bmp;
//Every level of maturity, multiply the power by 1 + this much.
extern float maturity_power_mult;
//Every level of maturity, multiply the attack by 1 + this much.
extern float maturity_speed_mult;
extern size_t max_particles;
extern size_t max_pikmin_in_field;
//These many seconds until a new character of the message is drawn.
extern float message_char_interval;
extern bool mipmaps_enabled;
extern ALLEGRO_MIXER* mixer;
extern vector<mob_action> mob_actions;
extern mob_category_manager mob_categories;
extern vector<mob*> mobs;
//The physical mouse's cursor, in screen coordinates.
extern point mouse_cursor_s;
//The physical mouse's cursor, in world coordinates.
extern point mouse_cursor_w;
extern bool mouse_cursor_valid;
extern bool mouse_moves_cursor[MAX_PLAYERS];
//How far a leader can go to auto-pluck the next Pikmin.
extern float next_pluck_range;
extern float onion_open_range;
extern map<string, onion_type*> onion_types;
extern vector<onion*> onions;
extern unsigned char particle_quality;
extern particle_manager particles;
extern bool paused;
extern map<string, pellet_type*> pellet_types;
extern vector<pellet*> pellets;
extern vector<point> precipitation;
extern timer precipitation_timer;
extern float pikmin_chase_range;
extern float pikmin_grab_range;
extern vector<pikmin*> pikmin_list;
extern vector<pikmin_type*> pikmin_order;
extern vector<string> pikmin_order_strings;
extern map<string, pikmin_type*> pikmin_types;
extern vector<pile*> piles;
extern map<string, pile_type*> pile_types;
extern float pluck_range;
//If true, the whistle radius is merely drawn as a circle.
//Used to improve performance.
extern bool pretty_whistle;
//Time since start, on the previous frame.
//The first frame shouldn't allow for input just yet, because
//some things are still being set up within the first logic loop.
//So forbid input until the second frame.
extern bool ready_for_input;
extern vector<resource*> resources;
extern map<string, resource_type*> resource_types;
extern map<string, scale_type*> scale_types;
extern vector<scale*> scales;
extern bool scr_fullscreen;
extern int scr_h;
extern int scr_w;
extern ALLEGRO_TRANSFORM screen_to_world_transform;
extern sector_types_manager sector_types;
extern size_t selected_spray;
extern replay session_replay;
extern unsigned char ship_beam_ring_color[3];
extern bool ship_beam_ring_color_up[3];
extern map<string, ship_type*> ship_types;
extern vector<ship*> ships;
extern bool show_system_info;
//If false, images that are scaled up and down will look pixelated.
extern bool smooth_scaling;
extern single_animation_suite spark_animation;
extern map<string, mob_type*> spec_mob_types;
extern map<string, spike_damage_type> spike_damage_types;
//How many of each spray/ingredients the player has.
extern vector<spray_stats_struct> spray_stats;
extern vector<spray_type> spray_types;
extern float standard_pikmin_height;
extern float standard_pikmin_radius;
extern map<string, status_type> status_types;
extern subgroup_type_manager subgroup_types;
extern float swarm_angle;
//Distance of the arrows that appear
//when the "swarm to cursor" button is held.
extern vector<float> swarm_arrows;
//General intensity of the swarm in the specified angle.
extern float swarm_magnitude;
//Time remaining until the next arrow on the list of swarm arrows appears.
extern timer swarm_next_arrow_timer;
//Is the "swarm to cursor" button being pressed?
extern bool swarm_cursor;
extern float swarm_task_range;
//Joystick coordinates for swarming.
extern movement_struct swarm_movement;
extern bmp_manager textures;
extern bool throw_can_reach_cursor;
extern map<string, tool_type*> tool_types;
extern vector<tool*> tools;
extern map<string, track_type*> track_types;
extern vector<track*> tracks;
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
extern bool window_position_hack;
extern ALLEGRO_TRANSFORM world_to_screen_transform;
extern float zoom_max_level;
extern float zoom_mid_level;
extern float zoom_min_level;


#endif //ifndef VARS_INCLUDED
