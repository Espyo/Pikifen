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
#include "../LAFI/frame.h"
#include "../LAFI/gui.h"
#include "../LAFI/label.h"
#include "../misc_structs.h"

using namespace std;

/*
 * A generic class for an editor.
 * It comes with some common stuff, like a "you have unsaved changes!"
 * warning manager, information for the gui, etc.
 */
class editor : public game_state {
private:
    vector<pair<string, string> > picker_elements;
    size_t cur_picker_id;
    
protected:

    static const int   EDITOR_ICON_BMP_PADDING;
    static const int   EDITOR_ICON_BMP_SIZE;
    static const float KEYBOARD_CAM_ZOOM;
    static const float DOUBLE_CLICK_TIMEOUT;
    static const float STATUS_OVERRIDE_IMPORTANT_DURATION;
    static const float STATUS_OVERRIDE_UNIMPORTANT_DURATION;
    static const float UNSAVED_CHANGES_WARNING_DURATION;
    static const int   UNSAVED_CHANGES_WARNING_HEIGHT;
    static const int   UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
    static const int   UNSAVED_CHANGES_WARNING_WIDTH;
    
    enum EDITOR_ICONS {
        ICON_SAVE,
        ICON_LOAD,
        ICON_QUIT,
        ICON_HITBOXES,
        ICON_REFERENCE,
        ICON_INFO,
        ICON_HELP,
        ICON_PLAY_PAUSE,
        ICON_NEXT,
        ICON_PREVIOUS,
        ICON_ADD,
        ICON_REMOVE,
        ICON_MOVE_RIGHT,
        ICON_MOVE_LEFT,
        ICON_SELECT_NONE,
        ICON_DUPLICATE,
        ICON_ADD_STOP,
        ICON_ADD_LINK,
        ICON_ADD_1W_LINK,
        ICON_REMOVE_STOP,
        ICON_REMOVE_LINK,
        ICON_ADD_CIRCLE_SECTOR,
        ICON_VERTEXES,
        ICON_EDGES,
        ICON_SECTORS,
        ICON_MOBS,
        ICON_PATHS,
        ICON_DETAILS,
        ICON_REVIEW,
        ICON_TOOLS,
        ICON_OPTIONS,
        ICON_UNDO,
        ICON_ORIGIN,
        ICON_MOB_RADIUS,
        ICON_PIKMIN_SILHOUETTE,
        ICON_ANIMATIONS,
        ICON_SPRITES,
        ICON_BODY_PARTS,
        ICON_PLAY,
        ICON_SNAP_GRID,
        ICON_SNAP_VERTEXES,
        ICON_SNAP_EDGES,
        ICON_SNAP_NOTHING,
        
        N_EDITOR_ICONS
    };
    
    struct transformation_controller {
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
        point get_handle_pos(const unsigned char handle);
        void update();
        
    public:
        bool keep_aspect_ratio;
        bool allow_rotation;
        
        void draw_handles();
        bool handle_mouse_down(const point pos);
        void handle_mouse_up();
        bool handle_mouse_move(const point pos);
        point get_center();
        point get_size();
        float get_angle();
        void set_center(const point &center);
        void set_size(const point &size);
        void set_angle(const float angle);
        transformation_controller();
    };
    
    /*
     * Makes setting variables from LAFI widgets easier.
     */
    struct gui_to_var_helper {
    private:
        map<bool*, bool> bools;
        map<int*, int> ints;
        map<float*, float> floats;
        map<unsigned char*, unsigned char> uchars;
        map<string*, string> strings;
        map<ALLEGRO_COLOR*, ALLEGRO_COLOR> colors;
        map<point*, point> points;
    public:
        void register_bool(bool* var, const bool gui_value);
        void register_int(int* var, const int gui_value);
        void register_float(float* var, const float gui_value);
        void register_uchar(
            unsigned char* var, const unsigned char gui_value
        );
        void register_string(
            string* var, const string &gui_value
        );
        void register_color(
            ALLEGRO_COLOR* var, const ALLEGRO_COLOR &gui_value
        );
        void register_point(
            point* var, const point &gui_value
        );
        bool all_equal();
        void set_all();
    };
    
    ALLEGRO_BITMAP*         bmp_editor_icons;
    //If the next click is within this time, it's a double-click.
    point                   canvas_tl;
    point                   canvas_br;
    float                   double_click_time;
    vector<ALLEGRO_BITMAP*> editor_icons;
    lafi::frame*            frm_picker;
    lafi::frame*            frm_toolbar;
    lafi::gui*              gui;
    bool                    holding_m1;
    bool                    holding_m2;
    bool                    holding_m3;
    //Current X coordinate of the ImGui column separator for the canvas/panel.
    int imgui_canvas_column_separator_x;
    //Is Ctrl pressed down?
    bool                    is_ctrl_pressed;
    //Is the GUI currently what's in focus, i.e. the last thing clicked?
    bool                    is_gui_focused;
    //Is Shift pressed down?
    bool                    is_shift_pressed;
    //Number of the mouse button pressed.
    size_t                  last_mouse_click;
    lafi::label*            lbl_status_bar;
    //Has the user picked any content to load yet?
    bool                    loaded_content_yet;
    bool                    made_new_changes;
    //Is this a mouse drag, or just a shaky click?
    bool                    mouse_drag_confirmed;
    //Starting coordinates of a raw mouse drag.
    point                   mouse_drag_start;
    //Current state.
    size_t                  state;
    //Current sub-state.
    size_t                  sub_state;
    //Status bar override text.
    string                  status_override_text;
    //Time left to show the status bar override text for.
    timer                   status_override_timer;
    point                   unsaved_changes_warning_pos;
    timer                   unsaved_changes_warning_timer;
    float                   zoom_max_level;
    float                   zoom_min_level;
    
    bool check_new_unsaved_changes(lafi::widget* caller_widget);
    void center_camera(
        const point &min_coords, const point &max_coords
    );
    void close_changes_warning();
    void create_changes_warning_frame();
    void create_picker_frame();
    void create_status_bar();
    void create_toolbar_frame();
    void do_logic_pre();
    void do_logic_post();
    void draw_unsaved_changes_warning();
    void emit_status_bar_message(const string &text, const bool important);
    void generate_and_open_picker(
        const size_t id, const vector<pair<string, string> > &elements,
        const string &title, const bool can_make_new
    );
    bool is_mouse_in_gui(const point &mouse_coords);
    void leave();
    void populate_picker(const string &filter);
    void update_canvas_coordinates();
    void update_status_bar(
        const bool omit_coordinates = false,
        const bool reverse_y_coordinate = false
    );
    void zoom(const float new_zoom, const bool anchor_cursor = true);
    
    virtual void custom_picker_cancel_action();
    virtual void hide_all_frames() = 0;
    virtual void change_to_right_frame() = 0;
    virtual void create_new_from_picker(
        const size_t picker_id, const string &name
    ) = 0;
    virtual void pick(
        const size_t picker_id, const string &name, const string &category
    ) = 0;
    
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
    
    //LAFI helper functions.
    float get_angle_picker_angle(
        lafi::widget* parent, const string &picker_name
    );
    string get_button_text(
        lafi::widget* parent, const string &button_name
    );
    bool get_checkbox_check(
        lafi::widget* parent, const string &checkbox_name
    );
    string get_label_text(
        lafi::widget* parent, const string &label_name
    );
    string get_textbox_text(
        lafi::widget* parent, const string &textbox_name
    );
    bool get_radio_selection(
        lafi::widget* parent, const string &radio_name
    );
    void set_angle_picker_angle(
        lafi::widget* parent, const string &picker_name, const float angle
    );
    void set_button_text(
        lafi::widget* parent, const string &button_name, const string &text
    );
    void set_checkbox_check(
        lafi::widget* parent, const string &checkbox_name, const bool check
    );
    void set_label_text(
        lafi::widget* parent, const string &label_name, const string &text
    );
    void set_textbox_text(
        lafi::widget* parent, const string &textbox_name, const string &text
    );
    void set_radio_selection(
        lafi::widget* parent, const string &radio_name, const bool selection
    );
    
public:

    editor();
    virtual ~editor();
    
    virtual void do_drawing() = 0;
    virtual void do_logic() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
};

#endif //ifndef EDITOR_INCLUDED
