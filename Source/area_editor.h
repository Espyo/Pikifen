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
    AREA_EDITOR_PICKER_SECTOR_TYPE,
};

void adv_textures_to_gui();
void bg_to_gui();
void center_camera(float min_x, float max_x, float min_y, float max_y);
void change_to_right_frame(bool hide_all = false);
void do_logic();
void find_errors();
void goto_error();
void gui_to_bg();
void gui_to_sector();
void gui_to_adv_textures();
void handle_controls(ALLEGRO_EVENT ev);
void load();
void load_area();
void open_picker(unsigned char type);
void pick(string name, unsigned char type);
void save_area();
void sector_to_gui();
float snap_to_grid(const float c);
void update_review_frame();

};

#endif //ifndef AREA_EDITOR_INCLUDED