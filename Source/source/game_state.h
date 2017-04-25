/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the game state class and
 * game state-related functions.
 */

#ifndef GAME_STATE_INCLUDED
#define GAME_STATE_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "mobs/mob.h"
#include "menu_widgets.h"

using namespace std;

enum GAME_STATES {
    GAME_STATE_MAIN_MENU,
    GAME_STATE_AREA_MENU,
    GAME_STATE_OPTIONS_MENU,
    GAME_STATE_GAME,
    GAME_STATE_AREA_EDITOR,
    GAME_STATE_ANIMATION_EDITOR,
    
    N_GAME_STATES,
};

/* ----------------------------------------------------------------------------
 * A game macro-state. It might be easier to think of this as a "screen".
 * You've got the title screen, the options screen, the gameplay screen, etc.
 */
class game_state {
protected:
    void set_selected_widget(menu_widget* widget);
    void handle_widget_events(ALLEGRO_EVENT ev);
    
public:
    vector<menu_widget*> menu_widgets;
    menu_widget* selected_widget;
    
    game_state();
    virtual void load() = 0;
    virtual void unload() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev) = 0;
    virtual void do_logic() = 0;
    virtual void do_drawing() = 0;
    virtual void update_transformations();
};


/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay : public game_state {
private:
    ALLEGRO_BITMAP* bmp_bubble;
    ALLEGRO_BITMAP* bmp_counter_bubble_group;
    ALLEGRO_BITMAP* bmp_counter_bubble_field;
    ALLEGRO_BITMAP* bmp_counter_bubble_standby;
    ALLEGRO_BITMAP* bmp_counter_bubble_total;
    ALLEGRO_BITMAP* bmp_day_bubble;
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker;
    ALLEGRO_BITMAP* bmp_hard_bubble;
    ALLEGRO_BITMAP* bmp_message_box;
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble;
    ALLEGRO_BITMAP* bmp_sun;
    
    void do_aesthetic_logic();
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_logic();
    ALLEGRO_BITMAP* draw_to_bitmap();
    void load_game_content();
    void load_hud_info();
    void load_hud_coordinates(const int item, string data);
    void process_mob(mob* m_ptr, size_t m);
    
public:
    gameplay();
    virtual void load();
    virtual void unload();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual void update_transformations();
};

#endif //ifndef GAME_STATE_INCLUDED
