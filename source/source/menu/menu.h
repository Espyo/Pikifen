/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menu class and related functions.
 */

#pragma once

#include <string>
#include <vector>

#include "../content/other/gui.h"

using std::string;
using std::vector;


/**
 * @brief Data about a menu, which can be used in any game state, and can
 * link to other menus or even contain a network of menus itself.
 */
class menu_t {

public:

    //--- Members ---
    
    //List of GUI managers it has.
    vector<gui_manager*> guis;
    
    //Callback for when the player enters this menu.
    std::function<void()> enter_callback;
    
    //Callback for when the player chooses to leave this menu.
    std::function<void()> leave_callback;
    
    //Callback for when the menu object finishes loading.
    std::function<void()> load_callback;
    
    //Callback for when the menu object finishes unloading.
    std::function<void()> unload_callback;
    
    //If not LARGE_FLOAT, unloading will automatically occur after this time.
    float unload_timer = LARGE_FLOAT;
    
    //Is the menu loaded?
    bool loaded = false;
    
    //Is it active? Can the player interact with it?
    bool active = true;
    
    
    //--- Function declarations ---
    
    virtual ~menu_t() = default;
    virtual void load();
    virtual void unload();
    virtual void draw();
    virtual void enter();
    virtual void handle_event(const ALLEGRO_EVENT &ev);
    virtual void handle_player_action(const player_action &action);
    virtual void leave();
    virtual void tick(float delta_t);
    
};
