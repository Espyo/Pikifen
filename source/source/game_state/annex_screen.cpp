/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Annex screen state class and related functions.
 */

#include <algorithm>

#include "annex_screen.h"

#include "../core/game.h"
#include "../util/allegro_utils.h"


/**
 * @brief Draws the annex screen state.
 */
void AnnexScreen::doDrawing() {
    al_clear_to_color(COLOR_BLACK);
    
    drawBitmap(
        bmpBg, Point(game.winW * 0.5, game.winH * 0.5),
        Point(game.winW, game.winH), 0, mapGray(64)
    );
    
    if(curMenu) curMenu->draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void AnnexScreen::doLogic() {
    if(!game.fadeMgr.isFading()) {
        for(size_t a = 0; a < game.controls.actionQueue.size(); a++) {
            if(curMenu) {
                curMenu->handlePlayerAction(game.controls.actionQueue[a]);
            }
        }
    }
    
    if(curMenu) {
        if(curMenu->loaded) {
            curMenu->tick(game.deltaT);
        }
        if(!curMenu->loaded) {
            delete curMenu;
            curMenu = nullptr;
        }
    }
    
    game.fadeMgr.tick(game.deltaT);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string AnnexScreen::getName() const {
    return "annex screen";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void AnnexScreen::handleAllegroEvent(ALLEGRO_EVENT& ev) {
    if(game.fadeMgr.isFading()) return;
    
    if(curMenu) curMenu->handleAllegroEvent(ev);
}


/**
 * @brief Leaves the annex screen state and goes to the title screen.
 */
void AnnexScreen::leave() {
    game.fadeMgr.startFade(false, [] () {
        game.changeState(game.states.titleScreen);
    });
}


/**
 * @brief Loads the annex screen state into memory.
 */
void AnnexScreen::load() {
    //Resources.
    bmpBg =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpTitleScreenBg
        );
        
    //Game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Load the intended concrete menu.
    switch(menuToLoad) {
    case ANNEX_SCREEN_MENU_AREA_SELECTION: {
        AreaMenu* areaMenu = new AreaMenu();
        areaMenu->areaType = areaMenuAreaType;
        areaMenu->leaveCallback =
        [this] () {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_PLAY;
            leave();
        };
        curMenu = areaMenu;
        break;
        
    } case ANNEX_SCREEN_MENU_HELP: {
        game.content.loadAll(
        vector<CONTENT_TYPE> {
            CONTENT_TYPE_PARTICLE_GEN,
            CONTENT_TYPE_GLOBAL_ANIMATION,
            CONTENT_TYPE_STATUS_TYPE,
            CONTENT_TYPE_LIQUID,
            CONTENT_TYPE_SPRAY_TYPE,
            CONTENT_TYPE_HAZARD,
            CONTENT_TYPE_WEATHER_CONDITION,
            CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        },
        CONTENT_LOAD_LEVEL_BASIC
        );
        game.content.loadAll(
        vector<CONTENT_TYPE> {
            CONTENT_TYPE_MOB_ANIMATION,
            CONTENT_TYPE_MOB_TYPE,
        },
        CONTENT_LOAD_LEVEL_FULL
        );
        HelpMenu* helpMenu = new HelpMenu();
        helpMenu->unloadCallback =
        [this] () {
            game.content.unloadAll(
            vector<CONTENT_TYPE> {
                CONTENT_TYPE_MOB_ANIMATION,
                CONTENT_TYPE_MOB_TYPE,
                CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
                CONTENT_TYPE_WEATHER_CONDITION,
                CONTENT_TYPE_HAZARD,
                CONTENT_TYPE_SPRAY_TYPE,
                CONTENT_TYPE_LIQUID,
                CONTENT_TYPE_STATUS_TYPE,
                CONTENT_TYPE_GLOBAL_ANIMATION,
                CONTENT_TYPE_PARTICLE_GEN,
            }
            );
        };
        helpMenu->leaveCallback =
        [this] () {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        curMenu = helpMenu;
        break;
        
    } case ANNEX_SCREEN_MENU_OPTIONS: {
        OptionsMenu* optionsMenu = new OptionsMenu();
        optionsMenu->leaveCallback = [this] () {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        curMenu = optionsMenu;
        break;
        
    } case ANNEX_SCREEN_MENU_STATS: {
        StatsMenu* statsMenu = new StatsMenu();
        statsMenu->leaveCallback = [this] () {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        curMenu = statsMenu;
        break;
        
    }
    }
    
    curMenu->load();
    curMenu->enter();
    menuToLoad = ANNEX_SCREEN_MENU_HELP;
    
    //Finishing touches.
    game.audio.setCurrentSong(game.sysContentNames.sngMenus, false);
    game.fadeMgr.startFade(true, nullptr);
}


/**
 * @brief Unloads the annex screen state from memory.
 */
void AnnexScreen::unload() {
    //Resources.
    game.content.bitmaps.list.free(bmpBg);
    
    //Menus.
    if(curMenu) {
        curMenu->unload();
        delete curMenu;
        curMenu = nullptr;
    }
    
    //Game content.
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_GUI,
    }
    );
}
