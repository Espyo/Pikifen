/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the game state class and
 * game state-related functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "../content/mob/mob.h"


using std::size_t;
using std::vector;


/**
 * @brief A game macro-state. It might be easier to think of this as a "screen".
 * You've got the title screen, the options screen, the gameplay screen, etc.
 */
class GameState {

public:

    //--- Members ---
    
    //Is it currently loaded?
    bool loaded = false;
    
    
    //--- Function declarations ---
    
    GameState();
    virtual ~GameState() = default;
    virtual void load() = 0;
    virtual void unload() = 0;
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev) = 0;
    virtual void do_logic() = 0;
    virtual void do_drawing() = 0;
    virtual void update_transformations();
    virtual string get_name() const = 0;
    
};
