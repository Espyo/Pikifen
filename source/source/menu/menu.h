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
class Menu {

public:

    //--- Members ---
    
    //List of GUI managers it has.
    vector<GuiManager*> guis;
    
    //Callback for when the player enters this menu.
    std::function<void()> enterCallback;
    
    //Callback for when the player chooses to leave this menu.
    std::function<void()> leaveCallback;
    
    //Callback for when the menu object finishes loading.
    std::function<void()> loadCallback;
    
    //Callback for when the menu object finishes unloading.
    std::function<void()> unloadCallback;
    
    //If not LARGE_FLOAT, unloading will automatically occur after this time.
    float unloadTimer = LARGE_FLOAT;
    
    //Is the menu loaded?
    bool loaded = false;
    
    //Is it active? Can the player interact with it?
    bool active = true;
    
    
    //--- Function declarations ---
    
    virtual ~Menu() = default;
    virtual void load();
    virtual void unload();
    virtual void draw();
    virtual void enter();
    virtual void handleAllegroEvent(const ALLEGRO_EVENT& ev);
    virtual bool handlePlayerAction(const Inpution::Action& action);
    virtual void leave();
    virtual void tick(float deltaT);
    
};
