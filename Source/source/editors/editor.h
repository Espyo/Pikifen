/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general editor-related functions.
 */

#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED

#include <string>
#include <vector>

#include "../game_state.h"

using std::map;
using std::string;
using std::vector;

/*
 * A generic class for an editor.
 * It comes with some common stuff, mostly GUI stuff.
 */
class editor : public game_state {
public:

    editor();
    virtual ~editor() = default;
    
    virtual void do_drawing() = 0;
    virtual void do_logic() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    virtual string get_name() const = 0;
    
protected:

    static const float KEYBOARD_CAM_ZOOM;
    static const float DOUBLE_CLICK_TIMEOUT;
    
    
    struct transformation_controller {
    public:
        bool keep_aspect_ratio;
        bool allow_rotation;
        
        void draw_handles();
        bool handle_mouse_down(const point pos);
        void handle_mouse_up();
        bool handle_mouse_move(const point pos);
        point get_center() const;
        point get_size() const;
        float get_angle() const;
        void set_center(const point &center);
        void set_size(const point &size);
        void set_angle(const float angle);
        transformation_controller();
        
    private:
        static const float HANDLE_RADIUS;
        static const float ROTATION_HANDLE_THICKNESS;
        signed char moving_handle;
        point center;
        point size;
        float angle;
        ALLEGRO_TRANSFORM align_transform;
        ALLEGRO_TRANSFORM disalign_transform;
        float radius;
        point pre_move_size;
        float pre_rotation_angle;
        float pre_rotation_mouse_angle;
        point get_handle_pos(const unsigned char handle) const;
        void update();
        
    };
    
    
    //Top-left corner of the canvas.
    point canvas_tl;
    //Bottom right corner of the canvas.
    point canvas_br;
    //X coordinate of the canvas GUI separator. -1 = undefined.
    int canvas_separator_x;
    //If the next click is within this time, it's a double-click.
    float double_click_time;
    //Is the Ctrl key currently pressed down?
    bool is_ctrl_pressed;
    //Is the GUI currently in focus? False if it's the canvas.
    bool is_gui_focused;
    //Is the left mouse button currently pressed down?
    bool is_m1_pressed;
    //Is the right mouse button currently pressed down?
    bool is_m2_pressed;
    //Is the middle mouse button currently pressed down?
    bool is_m3_pressed;
    //Is the Shift key currently pressed down?
    bool is_shift_pressed;
    //Number of the mouse button pressed.
    size_t last_mouse_click;
    //Has the user picked any content to load yet?
    bool loaded_content_yet;
    //Has the user made any unsaved changes yet?
    bool made_new_changes;
    //Is this a real mouse drag, or just a shaky click?
    bool mouse_drag_confirmed;
    //Starting coordinates of a raw mouse drag.
    point mouse_drag_start;
    //Current state.
    size_t state;
    //Current sub-state.
    size_t sub_state;
    //Maximum zoom level allowed.
    float zoom_max_level;
    //Minimum zoom level allowed.
    float zoom_min_level;
    
    
    //Standard functions.
    void center_camera(
        const point &min_coords, const point &max_coords
    );
    void do_logic_post();
    void do_logic_pre();
    bool is_mouse_in_gui(const point &mouse_coords) const;
    void leave();
    void update_canvas_coordinates();
    void zoom(const float new_zoom, const bool anchor_cursor = true);
    
    
    //Input handler functions.
    virtual void handle_key_char_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_char_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_key_down_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_down_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_key_up_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_up_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_up(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_up(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_update(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_up(const ALLEGRO_EVENT &ev);
    
private:

};

#endif //ifndef EDITOR_INCLUDED
