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
void               angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord);
void               attack(mob* m1, mob* m2, bool m1_is_pikmin, float damage, float angle, float knockback, float new_invuln_period, float new_knockdown_period);
ALLEGRO_COLOR      change_alpha(ALLEGRO_COLOR c, unsigned char a);
void               coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude);
void               create_mob(mob* m);
void               delete_mob(mob* m);
void               dismiss();
void               draw_control(ALLEGRO_FONT* font, control_info c, float x, float y, float max_w, float max_h);
void               draw_compressed_text(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags, unsigned char valign, float max_w, float max_h, string text);
void               draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color);
void               draw_health(float cx, float cy, unsigned int health, unsigned int max_health, float radius = 20, bool just_chart = false);
void               draw_sector(sector &s, float x, float y);
void               draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch);
void               draw_sprite(ALLEGRO_BITMAP* bmp, float cx, float cy, float w, float h, float angle = 0, ALLEGRO_COLOR tint = al_map_rgb(255, 255, 255));
void               draw_text_lines(ALLEGRO_FONT* f, ALLEGRO_COLOR c, float x, float y, int fl, unsigned char va, string text);
void               drop_mob(pikmin* p);
void               error_log(string s, data_node* d = NULL);
bool               find_in_vector(vector<string> v, string s);
void               focus_mob(mob* m1, mob* m2, bool is_near, bool call_event);
vector<string>     folder_to_vector(string folder_name, bool folders);
void               generate_area_images();
pikmin*            get_closest_buried_pikmin(float x, float y, float* d, bool ignore_reserved);
hitbox_instance*   get_closest_hitbox(float x, float y, mob* m);
ALLEGRO_COLOR      get_daylight_color();
hitbox_instance*   get_hitbox(mob* m, string name);
float              get_leader_to_group_center_dist(mob* l);
mob_event*         get_mob_event(mob* m, unsigned char e, bool query = false);
ALLEGRO_TRANSFORM  get_world_to_screen_transform();
void               give_pikmin_to_onion(onion* o, unsigned amount);
void               go_pluck(leader* l, pikmin* p);
ALLEGRO_COLOR      interpolate_color(float n, float n1, float n2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2);
animation_set      load_animation_set(data_node* frames_node);
void               load_area(string name);
//ToDo try to figure out why in the world uncommenting this gives retarded errors. void               load_control(unsigned char action, unsigned char player, string name, data_node& file, string def)
ALLEGRO_BITMAP*    load_bmp(string filename, data_node* node = NULL);
data_node          load_data_file(string filename);
vector<hitbox>     load_hitboxes(data_node* frame_node);
void               load_mob_types(string folder, unsigned char type);
void               load_options();
sample_struct      load_sample(string filename, ALLEGRO_MIXER* mixer);
vector<mob_event*> load_script(data_node* node);
void               load_game_content();
void               make_uncarriable(mob* m);
void               move_point(float x, float y, float tx, float ty, float speed, float reach_radius, float* mx, float* my, float* angle, bool* reached);
void               pluck_pikmin(leader* new_leader, pikmin* p, leader* leader_who_plucked);
float              randomf(float min, float max);
int                randomi(int min, int max);
void               random_particle_explosion(unsigned char type, ALLEGRO_BITMAP* bmp, float center_x, float center_y, float speed_min, float speed_max, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_fire(unsigned char type, ALLEGRO_BITMAP* bmp, float origin_x, float origin_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_splash(unsigned char type, ALLEGRO_BITMAP* bmp, float origin_x, float origin_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_spray(unsigned char type, ALLEGRO_BITMAP* bmp, float origin_x, float origin_y, float angle, ALLEGRO_COLOR color);
void               remove_from_party(mob* member);
void               rotate_point(float x, float y, float angle, float* final_x, float* final_y);
void               save_options();
bool               should_attack(mob* m1, mob* m2);
vector<string>     split(string text, string del = " ", bool inc_empty = false, bool inc_del = false);
void               start_camera_pan(int final_x, int final_y);
void               start_camera_zoom(float final_zoom_level);
void               start_carrying(mob* m, pikmin* np, pikmin* lp);
void               start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
void               stop_auto_pluck(leader* l);
void               stop_whistling();
string             str_to_lower(string s);
//bool               temp_point_inside_sector(float x, float y, vector<linedef> &linedefs);
void               unfocus_mob(mob* m1, mob* m2, bool call_event);
void               use_spray(size_t spray_nr);

void al_fwrite(ALLEGRO_FILE* f, string s);
string btos(bool b);
bool tob(string s);
ALLEGRO_COLOR toc(string s);
double tof(string s);
int toi(string s);

#endif //ifndef FUNCTIONS_H