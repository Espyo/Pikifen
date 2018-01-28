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
    GAME_STATE_CONTROLS_MENU,
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
    
    bool right_pressed;
    bool up_pressed;
    bool left_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool back_pressed;
    
public:
    vector<menu_widget*> menu_widgets;
    menu_widget* back_widget;
    menu_widget* selected_widget;
    
    void handle_menu_button(
        const size_t action, const float pos, const size_t player
    );
    
    game_state();
    virtual void load() = 0;
    virtual void unload() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev) = 0;
    virtual void do_logic() = 0;
    virtual void do_drawing() = 0;
    virtual void update_transformations();
};

#endif //ifndef GAME_STATE_INCLUDED
