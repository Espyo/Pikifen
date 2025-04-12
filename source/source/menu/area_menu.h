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
    AREA_TYPE areaType = AREA_TYPE_SIMPLE;
    
    //Main GUI.
    GuiManager gui;
    
    
    //--- Function declarations ---
    
    void load() override;
    
private:

    //--- Members ---
    
    //Button for each area available.
    vector<GuiItem*> areaButtons;
    
    //Records of each area available.
    vector<MissionRecord> areaRecords;
    
    //Area info GUI item.
    GuiItem* infoBox = nullptr;
    
    //Mission specs GUI item.
    GuiItem* specsBox = nullptr;
    
    //Currently selected area, or INVALID for none.
    size_t curAreaIdx = INVALID;
    
    //Area list box item.
    ListGuiItem* listBox = nullptr;
    
    //Button of the first area available, if any.
    ButtonGuiItem* firstAreaButton = nullptr;
    
    //Name text item, in the info page.
    TextGuiItem* infoNameText = nullptr;
    
    //Name text item, in the specs page.
    TextGuiItem* specsNameText = nullptr;
    
    //Subtitle text item.
    TextGuiItem* subtitleText = nullptr;
    
    //Thumbnail of the currently selected area.
    ALLEGRO_BITMAP* curThumb = nullptr;
    
    //Description text item.
    TextGuiItem* descriptionText = nullptr;
    
    //Difficulty text item.
    TextGuiItem* difficultyText = nullptr;
    
    //Tags text item.
    TextGuiItem* tagsText = nullptr;
    
    //Maker text item.
    TextGuiItem* makerText = nullptr;
    
    //Version text item.
    TextGuiItem* versionText = nullptr;
    
    //Record info text item.
    TextGuiItem* recordInfoText = nullptr;
    
    //Record stamp of the currently selected area.
    ALLEGRO_BITMAP* curStamp = nullptr;
    
    //Record medal of the currently selected area.
    ALLEGRO_BITMAP* curMedal = nullptr;
    
    //Record date text item.
    TextGuiItem* recordDateText = nullptr;
    
    //Goal text item.
    TextGuiItem* goalText = nullptr;
    
    //Fail explanation list item.
    ListGuiItem* failList = nullptr;
    
    //Grading explanation list item.
    ListGuiItem* gradingList = nullptr;
    
    //Show the mission specs?
    bool showMissionSpecs = false;
    
    
    //--- Function declarations ---
    
    void addBullet(ListGuiItem* list, const string &text);
    void animateInfoAndSpecs();
    void changeInfo(size_t area_idx);
    void initGuiMain();
    void initGuiInfoPage();
    void initGuiSpecsPage();
    
};
