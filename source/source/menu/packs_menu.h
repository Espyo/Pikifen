/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pack management menu struct and related functions.
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include <allegro5/allegro.h>

#include "../content/other/gui.h"
#include "menu.h"

using std::map;
using std::string;


namespace PACKS_MENU {
extern const string GUI_FILE_PATH;
}


/**
 * @brief Info about the pack management menu currently being presented to
 * the player.
 */
class PacksMenu : public Menu {

public:

    //--- Public members ---
    
    //GUI manager.
    GuiManager gui;
    
    
    //--- Public function declarations ---
    
    void load() override;
    void unload() override;
    
    
private:

    //--- Private members ---
    
    //Working copy of the order of the packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> packOrder;
    
    //Working copy of the list of disabled packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> packsDisabled;
    
    //Pack list item.
    ListGuiItem* packsList = nullptr;
    
    //Pack bullet items, in order.
    vector<BulletGuiItem*> packBullets;
    
    //Pack check items, in order.
    vector<CheckGuiItem*> packChecks;
    
    //Pack name text item.
    TextGuiItem* packNameText = nullptr;
    
    //Pack description text item.
    TextGuiItem* packDescriptionText = nullptr;
    
    //Pack tags text item.
    TextGuiItem* packTagsText = nullptr;
    
    //Pack maker text item.
    TextGuiItem* packMakerText = nullptr;
    
    //Pack version text item.
    TextGuiItem* packVersionText = nullptr;
    
    //Restart warning text item.
    TextGuiItem* warningText = nullptr;
    
    //Internal name of the currently-selected pack, if any.
    string curPackName;
    
    //Bitmaps for each pack's thumbnail.
    std::map<string, ALLEGRO_BITMAP*> packThumbs;
    
    
    //--- Private function declarations ---
    
    void changeInfo(int idx);
    void initGuiMain();
    void populatePacksList();
    void triggerRestartWarning();
    
};
