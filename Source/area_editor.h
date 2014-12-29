/*
 * Copyright (c) André 'Espyo' Silva 2014.
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

#include <allegro5/allegro.h>

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

#define GRID_INTERVAL 32



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
void load();
void load_area();
void mob_to_gui();
void open_picker(unsigned char type);
void pick(string name, unsigned char type);
void save_area();
void sector_to_gui();
void shadow_to_gui();
float snap_to_grid(const float c);
void update_review_frame();

};

#endif //ifndef AREA_EDITOR_INCLUDED
