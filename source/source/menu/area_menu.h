/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area selection menu struct and related functions.
 */

#pragma once

#include <string>
#include <vector>

#include "../content/area/area.h"
#include "../content/other/gui.h"

using std::string;
using std::vector;


namespace AREA_MENU {
extern const string GUI_FILE_NAME;
extern const string INFO_GUI_FILE_NAME;
extern const float PAGE_SWAP_DURATION;
extern const string SPECS_GUI_FILE_NAME;
}


/**
 * @brief Info about the area selection currently being presented to
 * the player.
 */
struct area_menu_t {

    public:
    
    //--- Members ---
    
    //Type of area that the menu is dealing with.
    AREA_TYPE area_type = AREA_TYPE_SIMPLE;
    
    //Main GUI.
    gui_manager gui;
    
    //Callback for when the "Back" button is pressed to leave the menu.
    std::function<void()> back_callback;
    
    //Is the struct meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    
    area_menu_t(AREA_TYPE area_type);
    ~area_menu_t();
    void draw();
    void handle_event(const ALLEGRO_EVENT &ev);
    void handle_player_action(const player_action &action);
    void tick(float delta_t);
    
    private:
    
    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    //Time left until the menu finishes closing.
    float closing_timer = 0.0f;
    
    //Button for each area available.
    vector<gui_item*> area_buttons;
    
    //Records of each area available.
    vector<mission_record> area_records;
    
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
    void start_closing();
    
};
