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
class help_menu_t : public menu_t {
public:

    //--- Members ---
    
    //GUI manager.
    gui_manager gui;
    
    
    //--- Function declarations ---
    void load() override;
    void unload() override;
    
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief One of the help menu's tidbits.
     */
    struct tidbit {
    
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
    map<HELP_CATEGORY, vector<tidbit> > tidbits;
    
    //Currently shown tidbit, if any.
    tidbit* cur_tidbit = nullptr;
    
    //Category text GUI item.
    text_gui_item* category_text = nullptr;
    
    //Tidbit list.
    list_gui_item* tidbit_list = nullptr;
    
    
    //--- Function declarations ---
    
    void draw_tidbit(
        const ALLEGRO_FONT* const font, const point &where,
        const point &max_size, const string &text
    );
    void populate_tidbits(const HELP_CATEGORY category);
    
};
