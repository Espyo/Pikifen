/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menu class and related functions.
 */

#include "menu.h"

#include "../game_state/gameplay/gameplay.h"


/**
 * @brief Draws the menu.
 */
void Menu::draw() {
    if(!loaded) return;
    
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->draw();
        }
    }
}


/**
 * @brief Enters the menu.
 */
void Menu::enter() {
    if(!loaded) return;
    
    if(enter_callback) enter_callback();
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 */
void Menu::handleAllegroEvent(const ALLEGRO_EVENT &ev) {
    if(!loaded || !active) return;
    
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->handleAllegroEvent(ev);
        }
    }
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void Menu::handlePlayerAction(const PlayerAction &action) {
    if(!loaded || !active) return;
    
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->handlePlayerAction(action);
        }
    }
}


/**
 * @brief Leaves the menu.
 */
void Menu::leave() {
    if(!loaded) return;
    active = false;
    if(leave_callback) leave_callback();
}


/**
 * @brief Loads the menu.
 */
void Menu::load() {
    if(loaded) return;
    
    loaded = true;
    if(load_callback) load_callback();
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Menu::tick(float delta_t) {
    if(!loaded) return;
    
    //Tick the GUIs.
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->tick(delta_t);
        }
    }
    
    //Tick the unload timer.
    if(unload_timer != LARGE_FLOAT) {
        unload_timer -= delta_t;
        if(unload_timer <= 0.0f) {
            unload();
            unload_timer = LARGE_FLOAT;
        }
    }
}


/**
 * @brief Unloads the menu.
 */
void Menu::unload() {
    if(!loaded) return;
    
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->destroy();
        }
    }
    guis.clear();
    
    loaded = false;
    if(unload_callback) unload_callback();
}
