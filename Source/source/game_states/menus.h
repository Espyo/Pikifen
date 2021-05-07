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


using std::map;
using std::size_t;
using std::vector;


class main_menu_state : public game_state {
public:
    main_menu_state();
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual string get_name() const;
    
private:
    struct logo_pik {
        point pos;
        float angle;
        float speed;
        point destination;
        float sway_speed;
        float sway_var;
        ALLEGRO_BITMAP* top;
        bool reached_destination;
    };
    
    ALLEGRO_BITMAP* bmp_menu_bg;
    vector<logo_pik> logo_pikmin;
    gui_manager gui;
    
    point logo_min_screen_limit;
    point logo_max_screen_limit;
    float logo_pikmin_max_speed;
    float logo_pikmin_min_speed;
    float logo_pikmin_speed_smoothness;
    float logo_pikmin_sway_amount;
    float logo_pikmin_sway_max_speed;
    float logo_pikmin_sway_min_speed;
    point logo_pikmin_size;
    map<unsigned char, ALLEGRO_BITMAP*> logo_type_bitmaps;
    
    static const string GUI_FILE_PATH;
};


class options_menu_state : public game_state {
public:
    options_menu_state();
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual string get_name() const;
    
private:
    vector<std::pair<int, int> > resolution_presets;
    
    ALLEGRO_BITMAP* bmp_menu_bg;
    gui_manager gui;
    
    picker_gui_item* resolution_picker;
    picker_gui_item* cursor_speed_picker;
    text_gui_item* warning_text;
    
    void change_cursor_speed(const signed int step);
    void change_resolution(const signed int step);
    void go_to_controls();
    void update();
    void leave();
    void trigger_restart_warning();
    
    static const float CURSOR_SPEED_PRESETS[];
    static const string CURSOR_SPEED_PRESET_NAMES[];
    static const string GUI_FILE_PATH;
    static const unsigned char N_CURSOR_SPEED_PRESETS;
    
};


class controls_menu_state : public game_state {
public:
    controls_menu_state();
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual string get_name() const;
    
private:
    ALLEGRO_BITMAP* bmp_menu_bg;
    gui_manager gui;
    
    list_gui_item* list_box;
    
    bool capturing_input;
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


class area_menu_state : public game_state {
public:
    area_menu_state();
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual string get_name() const;
    
private:
    ALLEGRO_BITMAP* bmp_menu_bg;
    vector<string> areas_to_pick;
    vector<string> area_names;
    gui_manager gui;
    
    void leave();
    
    static const string GUI_FILE_PATH;
    
};


#endif //ifndef MENUS_INCLUDED
