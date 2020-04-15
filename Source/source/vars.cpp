/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible variables.
 */

#include <limits.h>

#include "vars.h"

#include "const.h"

using std::map;
using std::size_t;
using std::string;
using std::vector;

ALLEGRO_BITMAP* bmp_checkbox_check = NULL;
ALLEGRO_BITMAP* bmp_cursor = NULL;
ALLEGRO_BITMAP* bmp_cursor_invalid = NULL;
ALLEGRO_BITMAP* bmp_enemy_spirit = NULL;
ALLEGRO_BITMAP* bmp_icon = NULL;
ALLEGRO_BITMAP* bmp_idle_glow = NULL;
ALLEGRO_BITMAP* bmp_message_box = NULL;
ALLEGRO_BITMAP* bmp_mouse_cursor = NULL;
ALLEGRO_BITMAP* bmp_mouse_button_icon[3] = {NULL, NULL, NULL};
ALLEGRO_BITMAP* bmp_mouse_wd_icon = NULL;
ALLEGRO_BITMAP* bmp_mouse_wu_icon = NULL;
ALLEGRO_BITMAP* bmp_notification = NULL;
ALLEGRO_BITMAP* bmp_pikmin_silhouette = NULL;
ALLEGRO_BITMAP* bmp_pikmin_spirit = NULL;
ALLEGRO_BITMAP* bmp_rock = NULL;
ALLEGRO_BITMAP* bmp_shadow = NULL;
ALLEGRO_BITMAP* bmp_smack = NULL;
ALLEGRO_BITMAP* bmp_smoke = NULL;
ALLEGRO_BITMAP* bmp_sparkle = NULL;
ALLEGRO_BITMAP* bmp_spotlight = NULL;
ALLEGRO_BITMAP* bmp_swarm_arrow = NULL;
ALLEGRO_BITMAP* bmp_wave_ring = NULL;

sample_struct sfx_attack;
sample_struct sfx_camera;
sample_struct sfx_pikmin_attack;
sample_struct sfx_pikmin_called;
sample_struct sfx_pikmin_carrying;
sample_struct sfx_pikmin_carrying_grab;
sample_struct sfx_pikmin_caught;
sample_struct sfx_pikmin_dying;
sample_struct sfx_pikmin_held;
sample_struct sfx_pikmin_idle;
sample_struct sfx_pluck;
sample_struct sfx_pikmin_plucked;
sample_struct sfx_pikmin_thrown;
sample_struct sfx_switch_pikmin;
sample_struct sfx_throw;

asset_file_names_struct asset_file_names;

map<string, particle_generator> custom_particle_generators;
ALLEGRO_FONT* font_area_name = NULL;
ALLEGRO_FONT* font_builtin = NULL;
ALLEGRO_FONT* font_counter = NULL;
ALLEGRO_FONT* font_main = NULL;
unsigned int font_counter_h = 0;
unsigned int font_main_h = 0;
ALLEGRO_FONT* font_value = NULL;
map<string, hazard> hazards;
vector<string> leader_order_strings;
map<string, liquid> liquids;
vector<mob_action> mob_actions;
mob_category_manager mob_categories;
vector<string> pikmin_order_strings;
single_animation_suite spark_animation;
map<string, spike_damage_type> spike_damage_types;
vector<spray_type> spray_types;
map<string, status_type> status_types;
map<string, weather> weather_conditions;
