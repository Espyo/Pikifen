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

#include "../../gui.h"

using std::map;
using std::string;


namespace PACKS_MENU {
extern const string GUI_FILE_PATH;
}


/**
 * @brief Info about the pack management menu currently being presented to
 * the player.
 */
struct packs_menu_t {
    public:
    
    //--- Members ---
    
    //GUI manager.
    gui_manager gui;
    
    //Callback for when the "Back" button is pressed to leave the menu.
    std::function<void()> back_callback;
    
    //Is the struct meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    packs_menu_t();
    ~packs_menu_t();
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

    //Working copy of the order of the packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> pack_order;
    
    //Working copy of the list of disabled packs. This is a list of internal
    //names and excludes the base pack.
    vector<string> packs_disabled;
    
    //Pack list item.
    list_gui_item* packs_list = nullptr;

    //Pack bullet items, in order.
    vector<bullet_gui_item*> pack_bullets;

    //Pack check items, in order.
    vector<check_gui_item*> pack_checks;

    //Pack name text item.
    text_gui_item* pack_name_text = nullptr;
    
    //Pack description text item.
    text_gui_item* pack_description_text = nullptr;

    //Pack tags text item.
    text_gui_item* pack_tags_text = nullptr;

    //Pack maker text item.
    text_gui_item* pack_maker_text = nullptr;

    //Pack version text item.
    text_gui_item* pack_version_text = nullptr;

    //Restart warning text item.
    text_gui_item* warning_text = nullptr;

    //Internal name of the currently-selected pack, if any.
    string cur_pack_name;

    //Bitmaps for each pack's thumbnail.
    std::map<string, ALLEGRO_BITMAP*> pack_thumbs;
    
    //--- Function declarations ---
    
    void change_info(int idx);
    void populate_packs_list();
    void start_closing();
    void trigger_restart_warning();
    
};
