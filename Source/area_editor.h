/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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

#include "game_state.h"
#include "LAFI/gui.h"
#include "LAFI/widget.h"
#include "sector.h"

using namespace std;

class area_editor : public game_state {
private:
    enum AREA_EDITOR_PICKER_TYPES {
        AREA_EDITOR_PICKER_AREA,
        AREA_EDITOR_PICKER_SECTOR_TYPE,
        AREA_EDITOR_PICKER_MOB_CATEGORY,
        AREA_EDITOR_PICKER_MOB_TYPE,
    };
    
    enum EDITOR_ERROR_TYPES {
        EET_NONE_YET,
        EET_NONE,
        EET_INTERSECTING_EDGES, //Two edges intersect.
        EET_LONE_EDGE,             //A edge is all by itself.
        EET_OVERLAPPING_VERTEXES,  //Two vertexes in the same spot.
        EET_BAD_SECTOR,            //A sector is corrupted.
        EET_MISSING_TEXTURE,       //A sector is without texture.
        EET_UNKNOWN_TEXTURE,       //A texture is not found in the game files.
        EET_LANDING_SITE,          //No landing site sector exists.
        EET_TYPELESS_MOB,          //Mob with no type.
        EET_MOB_OOB,               //Mob out of bounds.
        EET_MOB_IN_WALL,           //Mob stuck in a wall.
        EET_INVALID_SHADOW,        //Invalid tree shadow image.
    };
    
    static const float GRID_INTERVAL;
    
    bool                         bg_aspect_ratio;
    ALLEGRO_BITMAP*              bg_bitmap;
    string                       bg_file_name;
    float                        bg_x;
    float                        bg_y;
    float                        bg_w;
    float                        bg_h;
    unsigned char                bg_a;
    mob_gen*                     cur_mob;
    sector*                      cur_sector;
    tree_shadow*                 cur_shadow;
    float                        double_click_time;
    mob_gen*                     error_mob_ptr;
    sector*                      error_sector_ptr;
    tree_shadow*                 error_shadow_ptr;
    string                       error_string;
    unsigned char                error_type;
    vertex*                      error_vertex_ptr;
    string                       file_name;
    lafi::gui*                   gui;
    bool                         holding_m1;
    bool                         holding_m2;
    unsigned char                mode;
    size_t                       moving_thing; //Current vertex, object or shadow being moved.
    float                        moving_thing_x; //Relative X coordinate of the point where the vertex, object or shadow was grabbed.
    float                        moving_thing_y;
    sector*                      on_sector;
    unsigned char                sec_mode; //Secondary/sub mode.
    bool                         shift_pressed;
    bool                         show_bg;
    bool                         show_shadows;
    lafi::widget*                wum; //Widget under mouse.
    
    void adv_textures_to_gui();
    void bg_to_gui();
    void center_camera(float min_x, float min_y, float max_x, float max_y);
    void change_background(string new_file_name);
    void change_to_right_frame(bool hide_all = false);
    void leave();
    void find_errors();
    void goto_error();
    void gui_to_bg();
    void gui_to_mob();
    void gui_to_sector();
    void gui_to_shadow();
    void gui_to_adv_textures();
    bool is_edge_valid(edge* l);
    void load_area();
    void mob_to_gui();
    void open_picker(unsigned char type);
    void pick(string name, unsigned char type);
    void save_area();
    void sector_to_gui();
    void shadow_to_gui();
    float snap_to_grid(const float c);
    void update_review_frame();
    
public:

    vector<edge_intersection> intersecting_edges;
    unordered_set<sector*>       non_simples;
    unordered_set<edge*>      lone_edges;
    
    area_editor();
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void load();
    virtual void unload();
    
    void set_bg_file_name(string n);
    void set_bg_x(float x);
    void set_bg_y(float y);
    void set_bg_w(float w);
    void set_bg_h(float h);
    void set_bg_a(unsigned char a);
    
};

#endif //ifndef AREA_EDITOR_INCLUDED
