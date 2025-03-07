/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the annex screen class and related functions.
 */

#pragma once


#include "../content/area/area.h"
#include "../menu/area_menu.h"
#include "../menu/help_menu.h"
#include "../menu/options_menu.h"
#include "../menu/stats_menu.h"
#include "game_state.h"


//Specific menus of the annex screen.
enum ANNEX_SCREEN_MENU {

    //Area selection.
    ANNEX_SCREEN_MENU_AREA_SELECTION,
    
    //Help.
    ANNEX_SCREEN_MENU_HELP,
    
    //Options.
    ANNEX_SCREEN_MENU_OPTIONS,
    
    //Statistics.
    ANNEX_SCREEN_MENU_STATS,
    
};


/**
 * @brief Info about the annex screen used for misc. menus.
 */
class AnnexScreen : public GameState {

public:

    //--- Members ---
    
    //What specific menu to load when it is created.
    ANNEX_SCREEN_MENU menu_to_load = ANNEX_SCREEN_MENU_HELP;
    
    //Information about the current menu, if any.
    Menu* cur_menu = nullptr;
    
    //Type of area that the area menu is dealing with.
    AREA_TYPE area_menu_area_type = AREA_TYPE_SIMPLE;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
    
private:

    //--- Members ---
    
    //Bitmap of the background.
    ALLEGRO_BITMAP* bmp_bg = nullptr;
    
    
    //--- Function declarations ---
    
    void leave();
    
};

