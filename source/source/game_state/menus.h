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
#include "other_menus/help_menu.h"
#include "other_menus/options_menu.h"
#include "other_menus/stats_menu.h"


using std::map;
using std::size_t;
using std::vector;


namespace AREA_MENU {
extern const string GUI_FILE_NAME;
extern const string INFO_GUI_FILE_NAME;
extern const float PAGE_SWAP_DURATION;
extern const string SONG_NAME;
extern const string SPECS_GUI_FILE_NAME;
}


namespace MAIN_MENU {
extern const string GUI_FILE_NAME;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_NAME;
extern const string PLAY_GUI_FILE_NAME;
extern const string SONG_NAME;
extern const string TUTORIAL_GUI_FILE_NAME;
}


namespace RESULTS {
extern const string GUI_FILE_NAME;
extern const string SONG_NAME;
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


//Specific menus of the "dark" main menu.
enum DARK_MAIN_MENU_MENU {

    //Help.
    DARK_MAIN_MENU_MENU_HELP,
    
    //Options.
    DARK_MAIN_MENU_MENU_OPTIONS,
    
    //Statistics.
    DARK_MAIN_MENU_MENU_STATS,
    
};


/**
 * @brief Info about the "dark", full-screen main menu.
 */
class dark_main_menu_state : public game_state {

public:

    //--- Members ---
    
    //What specific menu to load when it is created.
    DARK_MAIN_MENU_MENU menu_to_load = DARK_MAIN_MENU_MENU_HELP;
    
    //Information about the current help menu, if any+.
    help_menu_t* help_menu = nullptr;
    
    //Information about the current options menu, if any+.
    options_menu_t* options_menu = nullptr;
    
    //Information about the current statistics menu, if any+.
    stats_menu_t* stats_menu = nullptr;
    
    
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
 * @brief Info about the main menu.
 */
class main_menu_state : public game_state {

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


/**
 * @brief Info about the area selection menu.
 */
class area_menu_state : public game_state {

public:

    //--- Members ---
    
    //Type of area that the menu is dealing with.
    AREA_TYPE area_type = AREA_TYPE_SIMPLE;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //Bitmap of the menu background,
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //Button for each area available.
    vector<gui_item*> area_buttons;
    
    //Records of each area available.
    vector<mission_record> area_records;
    
    //Main GUI.
    gui_manager gui;
    
    //Area info GUI item.
    gui_item* info_box = nullptr;
    
    //Mission specs GUI item.
    gui_item* specs_box = nullptr;
    
    //Currently selected area, or INVALID for none.
    size_t cur_area_idx = INVALID;
    
    //Area list box item.
    list_gui_item* list_box = nullptr;
    
    //Button of the first area available, if any.
    button_gui_item* first_area_button = nullptr;
    
    //Name text item, in the info page.
    text_gui_item* info_name_text = nullptr;
    
    //Name text item, in the specs page.
    text_gui_item* specs_name_text = nullptr;
    
    //Subtitle text item.
    text_gui_item* subtitle_text = nullptr;
    
    //Thumbnail of the currently selected area.
    ALLEGRO_BITMAP* cur_thumb = nullptr;
    
    //Description text item.
    text_gui_item* description_text = nullptr;
    
    //Difficulty text item.
    text_gui_item* difficulty_text = nullptr;
    
    //Tags text item.
    text_gui_item* tags_text = nullptr;
    
    //Maker text item.
    text_gui_item* maker_text = nullptr;
    
    //Version text item.
    text_gui_item* version_text = nullptr;
    
    //Record info text item.
    text_gui_item* record_info_text = nullptr;
    
    //Record stamp of the currently selected area.
    ALLEGRO_BITMAP* cur_stamp = nullptr;
    
    //Record medal of the currently selected area.
    ALLEGRO_BITMAP* cur_medal = nullptr;
    
    //Record date text item.
    text_gui_item* record_date_text = nullptr;
    
    //Goal text item.
    text_gui_item* goal_text = nullptr;
    
    //Fail explanation list item.
    list_gui_item* fail_list = nullptr;
    
    //Grading explanation list item.
    list_gui_item* grading_list = nullptr;
    
    //Show the mission specs?
    bool show_mission_specs = false;
    
    
    //--- Function declarations ---
    
    void add_bullet(list_gui_item* list, const string &text);
    void animate_info_and_specs();
    void change_info(size_t area_idx);
    void init_gui_main();
    void init_gui_info_page();
    void init_gui_specs_page();
    void leave();
    
};
