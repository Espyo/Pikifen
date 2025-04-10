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

    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    
    //--- Function declarations ---
    void load() override;
    void unload() override;
    
    
private:

    //--- Members ---
    
    //Working copy of the order of the packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> pack_order;
    
    //Working copy of the list of disabled packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> packs_disabled;
    
    //Pack list item.
    ListGuiItem* packs_list = nullptr;
    
    //Pack bullet items, in order.
    vector<BulletGuiItem*> pack_bullets;
    
    //Pack check items, in order.
    vector<CheckGuiItem*> pack_checks;
    
    //Pack name text item.
    TextGuiItem* pack_name_text = nullptr;
    
    //Pack description text item.
    TextGuiItem* pack_description_text = nullptr;
    
    //Pack tags text item.
    TextGuiItem* pack_tags_text = nullptr;
    
    //Pack maker text item.
    TextGuiItem* pack_maker_text = nullptr;
    
    //Pack version text item.
    TextGuiItem* pack_version_text = nullptr;
    
    //Restart warning text item.
    TextGuiItem* warning_text = nullptr;
    
    //Internal name of the currently-selected pack, if any.
    string cur_pack_name;
    
    //Bitmaps for each pack's thumbnail.
    std::map<string, ALLEGRO_BITMAP*> pack_thumbs;
    
    //--- Function declarations ---
    
    void changeInfo(int idx);
    void initGuiMain();
    void populatePacksList();
    void triggerRestartWarning();
    
};
