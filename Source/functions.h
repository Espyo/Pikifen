#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include "controls.h"
#include "data_file.h"
#include "leader.h"
#include "mob_event.h"
#include "spec_objs/onion.h"
#include "pikmin.h"
#include "sector.h"

//Returns the distance between two points.
#define dist(x1, y1, x2, y2) sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2)))
//Converts a float (or double) to a string.
#define ftos(n) to_string((long double) (n))
//Sets a lafi widget to invisible and disabled.
#define hide_widget(w) (w)->flags |= LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED;
//Converts an integer (or long) to a string.
#define itos(n) to_string((long long) (n))
//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + itos((n)))
//Returns the modulus of a number, regardless of it being a float or negative.
#define mod(n, d) ((n) - (d) * floor((n) / (d)))
//Adds a new animation conversion on load_mob_types().
#define new_anim_conversion(id, name) anim_conversions.push_back(make_pair<size_t, string>((id), (name)))
//Normalizes an angle between the range [-M_PI, M_PI]
#define normalize_angle(a) (mod((a), M_PI*2) - M_PI)
//Sets a lafi widget to visible and enabled.
#define show_widget(w) (w)->flags &= ~(LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED);
//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)
//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range ((pik_ptr->following_party == cur_leader_ptr && moving_group_intensity) ? 0 : PIKMIN_MIN_TASK_RANGE)

void               active_control();
void               add_to_party(mob* party_leader, mob* new_member);
void               angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord);
void               attack(mob* m1, mob* m2, const bool m1_is_pikmin, const float damage, const float angle, const float knockback, const float new_invuln_period, const float new_knockdown_period);
ALLEGRO_COLOR      change_alpha(const ALLEGRO_COLOR c, const unsigned char a);
void               coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude);
void               create_mob(mob* m);
void               delete_mob(mob* m);
void               dismiss();
void               draw_control(const ALLEGRO_FONT* const font, const control_info c, const float x, const float y, const float max_w, const float max_h);
void               draw_compressed_text(const ALLEGRO_FONT* const font, const ALLEGRO_COLOR color, const float x, const float y, const int flags, const unsigned char valign, const float max_w, const float max_h, const string text);
void               draw_fraction(const float cx, const float cy, const unsigned int current, const unsigned int needed, const ALLEGRO_COLOR color);
void               draw_health(const float cx, const float cy, const unsigned int health, const unsigned int max_health, const float radius = 20, const bool just_chart = false);
void               draw_sector(sector &s, const float x, const float y);
void               draw_shadow(const float cx, const float cy, const float size, const float delta_z, const float shadow_stretch);
void               draw_sprite(ALLEGRO_BITMAP* bmp, const float cx, const float cy, const float w, const float h, const float angle = 0, const ALLEGRO_COLOR tint = al_map_rgb(255, 255, 255));
void               draw_text_lines(const ALLEGRO_FONT* const f, const ALLEGRO_COLOR c, const float x, const float y, const int fl, const unsigned char va, const string text);
void               drop_mob(pikmin* p);
void               error_log(string s, data_node* d = NULL);
bool               find_in_vector(const vector<string> v, const string s);
void               focus_mob(mob* m1, mob* m2, const bool is_near, const bool call_event);
vector<string>     folder_to_vector(string folder_name, const bool folders);
void               generate_area_images();
pikmin*            get_closest_buried_pikmin(const float x, const float y, float* d, const bool ignore_reserved);
hitbox_instance*   get_closest_hitbox(const float x, const float y, mob* m);
ALLEGRO_COLOR      get_daylight_color();
hitbox_instance*   get_hitbox_instance(mob* m, const size_t nr);
float              get_leader_to_group_center_dist(mob* l);
mob_event*         get_mob_event(mob* m, const unsigned char e, const bool query = false);
ALLEGRO_TRANSFORM  get_world_to_screen_transform();
void               give_pikmin_to_onion(onion* o, const unsigned amount);
void               go_pluck(leader* l, pikmin* p);
ALLEGRO_COLOR      interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR c1, const ALLEGRO_COLOR c2);
animation_set      load_animation_set(data_node* frames_node);
void               load_area(const string name);
//ToDo try to figure out why in the world uncommenting this gives retarded errors. void               load_control(unsigned char action, unsigned char player, string name, data_node& file, string def)
ALLEGRO_BITMAP*    load_bmp(const string filename, data_node* node = NULL);
data_node          load_data_file(const string filename);
vector<hitbox>     load_hitboxes(data_node* frame_node);
void               load_mob_types(const string folder, const unsigned char type);
void               load_options();
sample_struct      load_sample(const string filename, ALLEGRO_MIXER* const mixer);
vector<mob_event*> load_script(mob_type* mt, data_node* node);
void               load_game_content();
void               make_uncarriable(mob* m);
void               move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached);
void               pluck_pikmin(leader* new_leader, pikmin* p, leader* leader_who_plucked);
float              randomf(float min, float max);
int                randomi(int min, int max);
void               random_particle_explosion(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float center_x, const float center_y, const float speed_min, const float speed_max, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color);
void               random_particle_fire(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color);
void               random_particle_splash(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color);
void               random_particle_spray(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const float angle, const ALLEGRO_COLOR color);
void               remove_from_party(mob* member);
void               rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y);
void               save_options();
bool               should_attack(mob* m1, mob* m2);
vector<string>     split(string text, const string del = " ", const bool inc_empty = false, const bool inc_del = false);
void               start_camera_pan(const int final_x, const int final_y);
void               start_camera_zoom(const float final_zoom_level);
void               start_carrying(mob* m, pikmin* np, pikmin* lp);
void               start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
void               stop_auto_pluck(leader* l);
void               stop_whistling();
string             str_to_lower(string s);
//bool               temp_point_inside_sector(float x, float y, vector<linedef> &linedefs);
void               unfocus_mob(mob* m1, mob* m2, const bool call_event);
void               use_spray(const size_t spray_nr);

void al_fwrite(ALLEGRO_FILE* f, string s);
string btos(bool b);
bool tob(string s);
ALLEGRO_COLOR toc(string s);
double tof(string s);
int toi(string s);

#endif //ifndef FUNCTIONS_H