/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menus.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "../content/other/gui.h"
#include "../core/misc_structs.h"
#include "../core/options.h"
#include "game_state.h"
#include "other_menus/area_menu.h"
#include "other_menus/help_menu.h"
#include "other_menus/options_menu.h"
#include "other_menus/stats_menu.h"


using std::map;
using std::size_t;
using std::vector;


namespace TITLE_SCREEN {
extern const string GUI_FILE_NAME;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_NAME;
extern const string PLAY_GUI_FILE_NAME;
extern const string TUTORIAL_GUI_FILE_NAME;
}


namespace RESULTS {
extern const string GUI_FILE_NAME;
}


//Pages of the main menu.
enum MAIN_MENU_PAGE {

    //Main page.
    MAIN_MENU_PAGE_MAIN,
    
    //Play page.
    MAIN_MENU_PAGE_PLAY,
    
    //Make page.
    MAIN_MENU_PAGE_MAKE,
    
};


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
class annex_screen_state : public game_state {

public:

    //--- Members ---
    
    //What specific menu to load when it is created.
    ANNEX_SCREEN_MENU menu_to_load = ANNEX_SCREEN_MENU_HELP;
    
    //Information about the current area selection menu, if any.
    area_menu_t* area_menu = nullptr;
    
    //Information about the current help menu, if any.
    help_menu_t* help_menu = nullptr;
    
    //Information about the current options menu, if any.
    options_menu_t* options_menu = nullptr;
    
    //Information about the current statistics menu, if any.
    stats_menu_t* stats_menu = nullptr;
    
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
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    
    //--- Function declarations ---
    
    void leave();
    
};


/**
 * @brief Info about the title screen.
 */
class title_screen_state : public game_state {

public:

    //--- Members ---
    
    //What page to load when it is created.
    MAIN_MENU_PAGE page_to_load = MAIN_MENU_PAGE_MAIN;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a Pikmin in the logo.
     */
    struct logo_pik {
    
        //--- Members ---
        
        //Position.
        point pos;
        
        //Current angle.
        float angle = 0.0f;
        
        //Forward movement speed.
        float speed = 0.0f;
        
        //Its destination.
        point destination;
        
        //Speed at which it sways.
        float sway_speed = 0.0f;
        
        //Variable that controls its swaying.
        float sway_var = 0.0f;
        
        //Image that represents this Pikmin's top.
        ALLEGRO_BITMAP* top = nullptr;
        
        //Has it reached its destination?
        bool reached_destination = false;
        
    };
    
    
    //--- Members ---
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //List of Pikmin that make up the logo.
    vector<logo_pik> logo_pikmin;
    
    //GUI for the main page.
    gui_manager main_gui;
    
    //GUI for the play page.
    gui_manager play_gui;
    
    //GUI for the make page.
    gui_manager make_gui;
    
    //GUI for the tutorial question page.
    gui_manager tutorial_gui;
    
    //Top-left coordinates of the logo, in screen percentage.
    point logo_min_screen_limit = point(10.0f);
    
    //Bottom-right coordinates of the logo, in screen percentage.
    point logo_max_screen_limit = point(90.0f, 50.0f);
    
    //Maximum speed a logo Pikmin can move at.
    float logo_pikmin_max_speed = 800.0f;
    
    //Minimum speed a logo Pikmin can move at.
    float logo_pikmin_min_speed = 600.0f;
    
    //How much to smooth a logo Pikmin's speed by.
    float logo_pikmin_speed_smoothness = 0.08f;
    
    //How much to sway a logo Pikmin by.
    float logo_pikmin_sway_amount = 3.0f;
    
    //Maximum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_max_speed = 5.5f;
    
    //Minimum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_min_speed = 2.5f;
    
    //Width and height of a logo Pikmin.
    point logo_pikmin_size = point(3.5f);
    
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> logo_type_bitmaps;
    
    
    //--- Function declarations ---
    
    void init_gui_main_page();
    void init_gui_make_page();
    void init_gui_play_page();
    void init_gui_tutorial_page();
    
};
