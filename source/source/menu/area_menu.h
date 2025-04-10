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
#include "menu.h"

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
class AreaMenu : public Menu {

public:

    //--- Members ---
    
    //Type of area that the menu is dealing with.
    AREA_TYPE area_type = AREA_TYPE_SIMPLE;
    
    //Main GUI.
    GuiManager gui;
    
    
    //--- Function declarations ---
    
    void load() override;
    
private:

    //--- Members ---
    
    //Button for each area available.
    vector<GuiItem*> area_buttons;
    
    //Records of each area available.
    vector<MissionRecord> area_records;
    
    //Area info GUI item.
    GuiItem* info_box = nullptr;
    
    //Mission specs GUI item.
    GuiItem* specs_box = nullptr;
    
    //Currently selected area, or INVALID for none.
    size_t cur_area_idx = INVALID;
    
    //Area list box item.
    ListGuiItem* list_box = nullptr;
    
    //Button of the first area available, if any.
    ButtonGuiItem* first_area_button = nullptr;
    
    //Name text item, in the info page.
    TextGuiItem* info_name_text = nullptr;
    
    //Name text item, in the specs page.
    TextGuiItem* specs_name_text = nullptr;
    
    //Subtitle text item.
    TextGuiItem* subtitle_text = nullptr;
    
    //Thumbnail of the currently selected area.
    ALLEGRO_BITMAP* cur_thumb = nullptr;
    
    //Description text item.
    TextGuiItem* description_text = nullptr;
    
    //Difficulty text item.
    TextGuiItem* difficulty_text = nullptr;
    
    //Tags text item.
    TextGuiItem* tags_text = nullptr;
    
    //Maker text item.
    TextGuiItem* maker_text = nullptr;
    
    //Version text item.
    TextGuiItem* version_text = nullptr;
    
    //Record info text item.
    TextGuiItem* record_info_text = nullptr;
    
    //Record stamp of the currently selected area.
    ALLEGRO_BITMAP* cur_stamp = nullptr;
    
    //Record medal of the currently selected area.
    ALLEGRO_BITMAP* cur_medal = nullptr;
    
    //Record date text item.
    TextGuiItem* record_date_text = nullptr;
    
    //Goal text item.
    TextGuiItem* goal_text = nullptr;
    
    //Fail explanation list item.
    ListGuiItem* fail_list = nullptr;
    
    //Grading explanation list item.
    ListGuiItem* grading_list = nullptr;
    
    //Show the mission specs?
    bool show_mission_specs = false;
    
    
    //--- Function declarations ---
    
    void addBullet(ListGuiItem* list, const string &text);
    void animateInfoAndSpecs();
    void changeInfo(size_t area_idx);
    void initGuiMain();
    void initGuiInfoPage();
    void initGuiSpecsPage();
    
};
