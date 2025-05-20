/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the statistics menu struct and related functions.
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include "../content/other/gui.h"
#include "menu.h"


using std::map;
using std::string;


namespace STATS_MENU {
extern const string GUI_FILE_PATH;
}


/**
 * @brief Info about the statistics menu currently being presented to
 * the player.
 */
class StatsMenu : public Menu {
public:

    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    
    //--- Function declarations ---
    void load() override;
    void tick(float deltaT) override;
    
    
private:

    //--- Members ---
    
    //Statistics list item.
    ListGuiItem* statsList = nullptr;
    
    //Runtime stat text item.
    TextGuiItem* runtimeValueText = nullptr;
    
    
    //--- Function declarations ---
    
    void addHeader(const string& label);
    TextGuiItem* addStat(
        const string& label, const string& value, const string& description
    );
    void initGuiMain();
    void populateStatsList();
    void updateRuntimeValueText();
    
};
