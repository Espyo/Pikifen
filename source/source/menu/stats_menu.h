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
class stats_menu_t : public menu_t {
public:

    //--- Members ---
    
    //GUI manager.
    gui_manager gui;
    
    
    //--- Function declarations ---
    void load() override;
    void tick(float delta_t) override;
    
    
private:

    //--- Members ---
    
    //Statistics list item.
    list_gui_item* stats_list = nullptr;
    
    //Runtime stat text item.
    text_gui_item* runtime_value_text = nullptr;
    
    
    //--- Function declarations ---
    
    void add_header(const string &label);
    text_gui_item* add_stat(
        const string &label, const string &value, const string &description
    );
    void init_gui_main();
    void populate_stats_list();
    void update_runtime_value_text();
    
};
