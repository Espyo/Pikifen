/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
#include "menu_widgets.h"

using namespace std;

class main_menu : public game_state {
private:
    ALLEGRO_BITMAP* bmp_menu_bg;
    size_t new_game_state;
    float time_spent;
    animation_pool logo;
    animation_instance logo_anim;
    
public:
    main_menu();
    virtual void load();
    virtual void unload();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void do_logic();
    virtual void do_drawing();
};


class options_menu : public game_state {
private:
    ALLEGRO_BITMAP* bmp_menu_bg;
    float time_spent;
    
    size_t cur_player_nr;
    size_t cur_page_nr;
    
    menu_text* cur_player_nr_widget;
    menu_text* cur_page_nr_widget;
    menu_text* input_capture_msg_widget;
    vector<menu_widget*> control_widgets;
    //Widgets to hide during the "press something" message.
    vector<menu_widget*> bottom_widgets;
    
    bool capturing_input;
    size_t input_capture_control_nr;
    
    void update();
    void leave();
    
public:
    options_menu();
    virtual void load();
    virtual void unload();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void do_logic();
    virtual void do_drawing();
};


class area_menu : public game_state {
private:
    ALLEGRO_BITMAP* bmp_menu_bg;
    float time_spent;
    size_t cur_page_nr;
    vector<string> areas_to_pick;
    
    vector<menu_widget*> area_buttons;
    menu_text* cur_page_nr_widget;
    
    void leave();
    void update();
    
public:
    area_menu();
    virtual void load();
    virtual void unload();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void do_logic();
    virtual void do_drawing();
};


#endif //ifndef MENUS_INCLUDED
