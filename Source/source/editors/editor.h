/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general editor-related functions.
 */

#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED

#include <string>
#include <vector>

#include "../LAFI/frame.h"
#include "../LAFI/gui.h"
#include "../game_state.h"
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
    
protected:

    static const string EDITOR_ICONS_FOLDER_NAME;
    
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
    
    lafi::frame*  frm_picker;
    lafi::gui*    gui;
    int           gui_x;
    bool          holding_m1;
    bool          holding_m2;
    bool          holding_m3;
    bmp_manager   icons;
    bool          made_changes;
    unsigned char mode;
    size_t        picker_type;
    //Secondary/sub mode.
    unsigned char sec_mode;
    int           status_bar_y;
    
    void close_changes_warning();
    void create_changes_warning_frame();
    void create_picker_frame();
    void generate_and_open_picker(
        const vector<pair<string, string> > &elements, const string &title,
        const bool can_make_new
    );
    void hide_bottom_frame();
    bool is_mouse_in_gui(const point &mouse_coords);
    void leave();
    void populate_picker(const string &filter);
    void show_bottom_frame();
    void show_changes_warning();
    void update_gui_coordinates();
    
    virtual void hide_all_frames() = 0;
    virtual void change_to_right_frame() = 0;
    virtual void create_new_from_picker(const string &name) = 0;
    virtual void pick(const string &name, const string &category) = 0;
    
public:

    editor();
    
    virtual void do_logic() = 0;
    virtual void do_drawing() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev) = 0;
    virtual void load() = 0;
    virtual void unload() = 0;
};

#endif //ifndef EDITOR_INCLUDED
