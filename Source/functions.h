#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include "data_file.h"
#include "leader.h"
#include "mob_event.h"
#include "spec_objs/onion.h"
#include "pikmin.h"
#include "sector.h"

//Returns the distance between two points.
#define dist(x1, y1, x2, y2) sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2)))
//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + to_string((long long) (n)))
//Returns the modulus of a number, regardless of it being a float or negative.
#define mod(n, d) ((n) - (d) * floor((n) / (d)))
//Normalizes an angle between the range [-M_PI, M_PI]
#define normalize_angle(a) (mod((a), M_PI*2) - M_PI)
//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)

void               active_control();
void               add_to_party(mob* party_leader, mob* new_member);
void               angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord);
ALLEGRO_COLOR      change_alpha(ALLEGRO_COLOR c, unsigned char a);
void               coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude);
void               create_mob(mob* m);
void               delete_mob(mob* m);
void               dismiss();
void               draw_compressed_text(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags, float max_w, float max_h, string text);
void               draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color);
void               draw_health(float cx, float cy, unsigned int health, unsigned int max_health, float radius = 20, bool just_chart = false);
void               draw_sector(sector &s, float x, float y);
void               draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch);
void               draw_sprite(ALLEGRO_BITMAP* bmp, float cx, float cy, float w, float h, float angle = 0, ALLEGRO_COLOR tint = al_map_rgb(255, 255, 255));
void               draw_text_lines(ALLEGRO_FONT* f, ALLEGRO_COLOR c, float x, float y, int fl, unsigned char va, string text);
void               drop_mob(pikmin* p);
void               error_log(string s, data_node* d = NULL);
vector<string>     folder_to_vector(string folder_name, bool folders);
void               generate_area_images();
pikmin*            get_closest_buried_pikmin(float x, float y, float* d, bool ignore_reserved);
ALLEGRO_COLOR      get_daylight_color();
float              get_leader_to_group_center_dist(mob* l);
mob_event*         get_mob_event(mob* m, unsigned char e);
ALLEGRO_TRANSFORM  get_world_to_screen_transform();
void               give_pikmin_to_onion(onion* o, unsigned amount);
ALLEGRO_COLOR      interpolate_color(float n, float n1, float n2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2);
void               load_area(string name);
//ToDo try to figure out why in the world uncommenting this gives retarded errors. void               load_control(unsigned char action, unsigned char player, string name, data_node& file, string def)
ALLEGRO_BITMAP*    load_bmp(string filename);
data_node          load_data_file(string filename);
vector<ext_frame>  load_frames(data_node* frames_node);
vector<hitbox>     load_hitboxes(data_node* frame_node);
void               load_mob_types(string folder, unsigned char type);
void               load_options();
sample_struct      load_sample(string filename);
vector<mob_event*> load_script(data_node* node);
void               load_game_content();
void               make_uncarriable(mob* m);
void               move_point(float x, float y, float tx, float ty, float speed, float reach_radius, float* mx, float* my, float* angle, bool* reached);
void               pluck_pikmin(leader* l, pikmin* p);
inline float       random(float min, float max);
void               random_particle_explosion(unsigned char type, float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_fire(unsigned char type, float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_splash(unsigned char type, float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void               random_particle_spray(unsigned char type, float origin_x, float origin_y, float angle, ALLEGRO_COLOR color);
void               remove_from_party(mob* member);
void               save_options();
vector<string>     split(string text, string del = " ", bool inc_empty = false, bool inc_del = false);
void               start_camera_pan(int final_x, int final_y);
void               start_camera_zoom(float final_zoom_level);
void               start_carrying(mob* m, pikmin* np, pikmin* lp);
void               start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
void               stop_whistling();
string             str_to_lower(string s);
//bool               temp_point_inside_sector(float x, float y, vector<linedef> &linedefs);
void               use_spray(size_t spray_nr);

inline void al_fwrite(ALLEGRO_FILE* f, string s);
inline string btos(bool b);
inline bool tob(string s);
ALLEGRO_COLOR toc(string s);
inline double tof(string s);
inline int toi(string s);

#endif //ifndef FUNCTIONS_H