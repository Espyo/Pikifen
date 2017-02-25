/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
    bool picker_allows_new;
    
protected:

    lafi::gui*    gui;
    int           gui_x;
    bool          holding_m1;
    bool          holding_m2;
    bmp_manager   icons;
    bool          made_changes;
    unsigned char mode;
    //Secondary/sub mode.
    unsigned char sec_mode;
    int           status_bar_y;
    
    void close_changes_warning();
    void create_changes_warning_frame();
    void create_picker_frame(const bool can_create_new);
    void generate_and_open_picker(
        const vector<string> &elements, const unsigned char type,
        const bool can_make_new = false
    );
    void hide_bottom_frame();
    bool is_mouse_in_gui(const int x, const int y);
    void leave();
    void show_bottom_frame();
    void show_changes_warning();
    void update_gui_coordinates();
    
    virtual void hide_all_frames() = 0;
    virtual void change_to_right_frame() = 0;
    virtual void create_new_from_picker(const string &name) = 0;
    virtual void pick(const string &name, const unsigned char type) = 0;
    
public:

    editor();
    
    virtual void do_logic() = 0;
    virtual void do_drawing() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev) = 0;
    virtual void load() = 0;
    virtual void unload() = 0;
};

#endif //ifndef EDITOR_INCLUDED
