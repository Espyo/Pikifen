/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menus.
 */

#ifndef MENUS_INCLUDED
#define MENUS_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "game_state.h"
#include "../gui.h"
#include "../options.h"


using std::map;
using std::size_t;
using std::vector;


namespace AREA_MENU {
extern const string GUI_FILE_PATH;
}


namespace CONTROLS_MENU {
extern const string GUI_FILE_PATH;
}


namespace OPTIONS_MENU {
extern const string AUTO_THROW_PRESET_NAMES[];
extern const AUTO_THROW_MODES AUTO_THROW_PRESETS[];
extern const string CURSOR_SPEED_PRESET_NAMES[];
extern const float CURSOR_SPEED_PRESETS[];
extern const string GUI_FILE_PATH;
extern const unsigned char N_AUTO_THROW_PRESETS;
extern const unsigned char N_CURSOR_SPEED_PRESETS;
}


namespace RESULTS {
extern const string GUI_FILE_PATH;
}


/* ----------------------------------------------------------------------------
 * Information about the main menu.
 */
class main_menu_state : public game_state {
public:
    main_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    struct logo_pik {
        //Position.
        point pos;
        //Current angle.
        float angle;
        //Forward movement speed.
        float speed;
        //Its destination.
        point destination;
        //Speed at which it sways.
        float sway_speed;
        //Variable that controls its swaying.
        float sway_var;
        //Image that represents this Pikmin's top.
        ALLEGRO_BITMAP* top;
        //Has it reached its destination?
        bool reached_destination;
    };
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //List of Pikmin that make up the logo.
    vector<logo_pik> logo_pikmin;
    //GUI.
    gui_manager gui;
    //Top-left coordinates of the logo, in screen percentage.
    point logo_min_screen_limit;
    //Bottom-right coordinates of the logo, in screen percentage.
    point logo_max_screen_limit;
    //Maximum speed a logo Pikmin can move at.
    float logo_pikmin_max_speed;
    //Minimum speed a logo Pikmin can move at.
    float logo_pikmin_min_speed;
    //How much to smooth a logo Pikmin's speed by.
    float logo_pikmin_speed_smoothness;
    //How much to sway a logo Pikmin by.
    float logo_pikmin_sway_amount;
    //Maximum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_max_speed;
    //Minimum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_min_speed;
    //Width and height of a logo Pikmin.
    point logo_pikmin_size;
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> logo_type_bitmaps;
    
    static const string GUI_FILE_PATH;
};


/* ----------------------------------------------------------------------------
 * Information about the options menu.
 */
class options_menu_state : public game_state {
public:
    options_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Known good resolution presets.
    vector<std::pair<int, int> > resolution_presets;
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //GUI.
    gui_manager gui;
    //Auto-throw picker widget.
    picker_gui_item* auto_throw_picker;
    //Resolution picker widget.
    picker_gui_item* resolution_picker;
    //Cursor speed picker widget.
    picker_gui_item* cursor_speed_picker;
    //Restart warning text widget.
    text_gui_item* warning_text;
    
    void change_auto_throw(const signed int step);
    void change_cursor_speed(const signed int step);
    void change_resolution(const signed int step);
    size_t get_auto_throw_idx() const;
    size_t get_cursor_speed_idx() const;
    size_t get_resolution_idx() const;
    void go_to_controls();
    void update();
    void leave();
    void trigger_restart_warning();
};


/* ----------------------------------------------------------------------------
 * Information about the controls menu.
 */
class controls_menu_state : public game_state {
public:
    controls_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Bitmap of the menu's background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //GUI.
    gui_manager gui;
    //Control list widget.
    list_gui_item* list_box;
    //Is it currently capturing input?
    bool capturing_input;
    //If it's capturing input, this is the index of the control to capture for.
    size_t input_capture_control_nr;
    
    void add_control();
    void add_control_gui_items(const size_t index, const bool focus);
    void choose_button(const size_t index);
    void choose_next_action(const size_t index);
    void choose_prev_action(const size_t index);
    void delete_control(const size_t index);
    void delete_control_gui_items();
    void leave();
    
    static const string GUI_FILE_PATH;
    
};


/* ----------------------------------------------------------------------------
 * Information about the area selection menu.
 */
class area_menu_state : public game_state {
public:
    area_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Bitmap of the menu background,
    ALLEGRO_BITMAP* bmp_menu_bg;
    //Folder name of each area available.
    vector<string> areas_to_pick;
    //Display name of each area available.
    vector<string> area_names;
    //GUI.
    gui_manager gui;
    
    void leave();
    
    static const string GUI_FILE_PATH;
    
};


#endif //ifndef MENUS_INCLUDED
