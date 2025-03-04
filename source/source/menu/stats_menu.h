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

using std::map;
using std::string;


namespace STATS_MENU {
extern const string GUI_FILE_PATH;
}


/**
 * @brief Info about the statistics menu currently being presented to
 * the player.
 */
struct stats_menu_t {
    public:
    
    //--- Members ---
    
    //GUI manager.
    gui_manager gui;
    
    //Callback for when the "Back" button is pressed to leave the menu.
    std::function<void()> back_callback;
    
    //Is the struct meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    stats_menu_t();
    ~stats_menu_t();
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
    
    //Statistics list item.
    list_gui_item* stats_list = nullptr;
    
    //Runtime stat text item.
    text_gui_item* runtime_value_text = nullptr;
    
    
    //--- Function declarations ---
    
    void add_header(const string &label);
    text_gui_item* add_stat(
        const string &label, const string &value, const string &description
    );
    void populate_stats_list();
    void start_closing();
    void update_runtime_value_text();
    
};
