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

bmp_manager bitmaps("");
ALLEGRO_BITMAP* bmp_error = NULL;
map<string, bouncer_type*> bouncer_types;
vector<bouncer*> bouncers;
vector<bridge*> bridges;
map<string, bridge_type*> bridge_types;
button_manager buttons;
vector<converter*> converters;
map<string, converter_type*> converter_types;
map<string, mob_type*> custom_mob_types;
map<string, particle_generator> custom_particle_generators;
map<string, decoration_type*> decoration_types;
vector<decoration*> decorations;
vector<drop*> drops;
map<string, drop_type*> drop_types;
map<string, enemy_type*> enemy_types;
vector<enemy*> enemies;
ALLEGRO_FONT* font_area_name = NULL;
ALLEGRO_FONT* font_builtin = NULL;
ALLEGRO_FONT* font_counter = NULL;
ALLEGRO_FONT* font_main = NULL;
unsigned int font_counter_h = 0;
unsigned int font_main_h = 0;
ALLEGRO_FONT* font_value = NULL;
vector<group_task*> group_tasks;
map<string, group_task_type*> group_task_types;
map<string, hazard> hazards;
hud_item_manager hud_items(N_HUD_ITEMS);
float hud_coords[N_HUD_ITEMS][4];
vector<interactable*> interactables;
map<string, interactable_type*> interactable_types;
vector<leader*> leaders;
vector<string> leader_order_strings;
map<string, leader_type*> leader_types;
map<string, liquid> liquids;
vector<mob_action> mob_actions;
mob_category_manager mob_categories;
vector<mob*> mobs;
map<string, onion_type*> onion_types;
vector<onion*> onions;
particle_manager particles(0);
map<string, pellet_type*> pellet_types;
vector<pellet*> pellets;
vector<pile*> piles;
map<string, pile_type*> pile_types;
vector<point> precipitation;
timer precipitation_timer(0);
vector<pikmin*> pikmin_list;
vector<string> pikmin_order_strings;
map<string, pikmin_type*> pikmin_types;
vector<resource*> resources;
map<string, resource_type*> resource_types;
map<string, scale_type*> scale_types;
vector<scale*> scales;
sector_types_manager sector_types;
replay session_replay;
map<string, ship_type*> ship_types;
vector<ship*> ships;
single_animation_suite spark_animation;
map<string, mob_type*> spec_mob_types;
map<string, spike_damage_type> spike_damage_types;
vector<spray_type> spray_types;
map<string, status_type> status_types;
subgroup_type_manager subgroup_types;
bmp_manager textures(TEXTURES_FOLDER_NAME);
map<string, tool_type*> tool_types;
vector<tool*> tools;
map<string, track_type*> track_types;
vector<track*> tracks;
map<string, treasure_type*> treasure_types;
vector<treasure*> treasures;
map<string, weather> weather_conditions;
