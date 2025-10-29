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
#include "../core/game.h"


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
    
    game.modal.draw();
}


/**
 * @brief Enters the menu.
 */
void Menu::enter() {
    if(!loaded) return;
    
    if(enterCallback) enterCallback();
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 */
void Menu::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!loaded || !active) return;
    
    if(!game.modal.isActive()) {
        for(size_t g = 0; g < guis.size(); g++) {
            if(guis[g]) {
                guis[g]->handleAllegroEvent(ev);
            }
        }
    }
    
    game.modal.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 * @return Whether the action was handled in any of the GUIs.
 */
bool Menu::handlePlayerAction(const Inpution::Action& action) {
    if(!loaded || !active) return false;
    
    bool handled = false;
    
    if(!game.modal.isActive()) {
        for(size_t g = 0; g < guis.size(); g++) {
            if(guis[g]) {
                handled |= guis[g]->handlePlayerAction(action);
            }
        }
    }
    
    handled |= game.modal.handlePlayerAction(action);
    
    return handled;
}


/**
 * @brief Leaves the menu.
 */
void Menu::leave() {
    if(!loaded) return;
    active = false;
    if(leaveCallback) leaveCallback();
}


/**
 * @brief Loads the menu.
 */
void Menu::load() {
    if(loaded) return;
    
    loaded = true;
    if(loadCallback) loadCallback();
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Menu::tick(float deltaT) {
    if(!loaded) return;
    
    //Tick the GUIs.
    for(size_t g = 0; g < guis.size(); g++) {
        if(guis[g]) {
            guis[g]->tick(deltaT);
        }
    }
    
    //Tick the modal.
    game.modal.tick(deltaT);
    
    //Tick the unload timer.
    if(unloadTimer != LARGE_FLOAT) {
        unloadTimer -= deltaT;
        if(unloadTimer <= 0.0f) {
            unload();
            unloadTimer = LARGE_FLOAT;
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
    if(unloadCallback) unloadCallback();
}
