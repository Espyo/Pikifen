/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the help menu struct and related functions.
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include "../content/other/gui.h"
#include "menu.h"

using std::map;
using std::string;


namespace HELP_MENU {
extern const string GUI_FILE_PATH;
}


//Categories of help page tidbits.
enum HELP_CATEGORY {

    //Gameplay basics tidbits.
    HELP_CATEGORY_GAMEPLAY1,
    
    //Gameplay advanced tidbits.
    HELP_CATEGORY_GAMEPLAY2,
    
    //Control tidbits.
    HELP_CATEGORY_CONTROLS,
    
    //Player type tidbits.
    HELP_CATEGORY_PIKMIN,
    
    //Noteworthy object tidbits.
    HELP_CATEGORY_OBJECTS,
    
    //Total amount of help page tidbit categories.
    N_HELP_CATEGORIES
    
};


/**
 * @brief Info about the help menu currently being presented to
 * the player.
 */
class HelpMenu : public Menu {
public:

    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    
    //--- Function declarations ---
    void load() override;
    void unload() override;
    
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief One of the help menu's tidbits.
     */
    struct Tidbit {
    
        //--- Members ---
        
        //Name.
        string name;
        
        //Description.
        string description;
        
        //Image.
        ALLEGRO_BITMAP* image = nullptr;
        
    };
    
    
    //--- Members ---
    
    //All tidbits.
    map<HELP_CATEGORY, vector<Tidbit> > tidbits;
    
    //Currently shown tidbit, if any.
    Tidbit* curTidbit = nullptr;
    
    //Category text GUI item.
    TextGuiItem* categoryText = nullptr;
    
    //Tidbit list.
    ListGuiItem* tidbitList = nullptr;
    
    
    //--- Function declarations ---
    
    void drawTidbit(
        const ALLEGRO_FONT* const font, const Point &where,
        const Point &maxSize, const string &text
    );
    void initGuiMain(DataNode* guiFile);
    void populateTidbits(const HELP_CATEGORY category);
    
};
