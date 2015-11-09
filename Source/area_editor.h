/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <string>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "LAFI/gui.h"
#include "LAFI/widget.h"
#include "sector.h"

using namespace std;

namespace area_editor {

enum AREA_EDITOR_PICKER_TYPES {
    AREA_EDITOR_PICKER_AREA,
    AREA_EDITOR_PICKER_SECTOR_TYPE,
    AREA_EDITOR_PICKER_MOB_CATEGORY,
    AREA_EDITOR_PICKER_MOB_TYPE,
};

enum EDITOR_ERROR_TYPES {
    EET_NONE_YET,
    EET_NONE,
    EET_INTERSECTING_LINEDEFS, //Two linedefs intersect.
    EET_LONE_LINE,             //A linedef is all by itself.
    EET_OVERLAPPING_VERTICES,  //Two vertices in the same spot.
    EET_BAD_SECTOR,            //A sector is corrupted.
    EET_MISSING_TEXTURE,       //A sector is without texture.
    EET_UNKNOWN_TEXTURE,       //A texture is not found in the game files.
    EET_LANDING_SITE,          //No landing site sector exists.
    EET_TYPELESS_MOB,          //Mob with no type.
    EET_MOB_OOB,               //Mob out of bounds.
    EET_MOB_IN_WALL,           //Mob stuck in a wall.
    EET_INVALID_SHADOW,        //Invalid tree shadow image.
};

const float GRID_INTERVAL = 32.0f;



void adv_textures_to_gui();
void bg_to_gui();
void center_camera(float min_x, float min_y, float max_x, float max_y);
void change_background(string new_file_name);
void change_to_right_frame(bool hide_all = false);
void do_logic();
void find_errors();
void goto_error();
void gui_to_bg();
void gui_to_mob();
void gui_to_sector();
void gui_to_shadow();
void gui_to_adv_textures();
void handle_controls(ALLEGRO_EVENT ev);
bool is_linedef_valid(linedef* l);
void load();
void load_area();
void mob_to_gui();
void open_picker(unsigned char type);
void pick(string name, unsigned char type);
void save_area();
void sector_to_gui();
void shadow_to_gui();
float snap_to_grid(const float c);
void unload();
void update_review_frame();

extern bool                         bg_aspect_ratio;
extern ALLEGRO_BITMAP*              bg_bitmap;
extern string                       bg_file_name;
extern float                        bg_x;
extern float                        bg_y;
extern float                        bg_w;
extern float                        bg_h;
extern unsigned char                bg_a;
extern mob_gen*                     cur_mob;
extern sector*                      cur_sector;
extern tree_shadow*                 cur_shadow;
extern float                        double_click_time;
extern mob_gen*                     error_mob_ptr;
extern sector*                      error_sector_ptr;
extern tree_shadow*                 error_shadow_ptr;
extern string                       error_string;
extern unsigned char                error_type;
extern vertex*                      error_vertex_ptr;
extern string                       file_name;
extern lafi::gui*                   gui;
extern bool                         holding_m1;
extern bool                         holding_m2;
extern vector<linedef_intersection> intersecting_lines;
extern unordered_set<linedef*>      lone_lines;
extern unsigned char                mode;
extern size_t                       moving_thing; //Current vertex, object or shadow being moved.
extern float                        moving_thing_x; //Relative X coordinate of the point where the vertex, object or shadow was grabbed.
extern float                        moving_thing_y;
extern unordered_set<sector*>       non_simples;
extern sector*                      on_sector;
extern unsigned char                sec_mode; //Secondary/sub mode.
extern bool                         shift_pressed;
extern bool                         show_bg;
extern bool                         show_shadows;
extern lafi::widget*                wum; //Widget under mouse.

};

#endif //ifndef AREA_EDITOR_INCLUDED
