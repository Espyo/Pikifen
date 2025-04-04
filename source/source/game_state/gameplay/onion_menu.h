/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion menu class and related functions.
 */

#pragma once

#include <string>

#include "../../content/mob/mob_utils.h"
#include "../../content/other/gui.h"


using std::size_t;
using std::string;


class PikminType;

namespace ONION_MENU {
extern const string GUI_FILE_PATH;
extern const float RED_TEXT_DURATION;
extern const size_t TYPES_PER_PAGE;
}


/**
 * @brief Info about a given Pikmin type in an Onion menu.
 */
struct OnionMenuPikminType {

    //--- Members ---
    
    //The player wants to add/subtract these many from the group.
    int delta = 0;
    
    //Index of this type in the Onion's list. Cache for convenience.
    size_t type_idx = INVALID;
    
    //Index in the on-screen list, or INVALID. Cache for convenience.
    size_t on_screen_idx = INVALID;
    
    //Pikmin type associated. Cache for convenience.
    PikminType* pik_type = nullptr;
    
    
    //--- Function declarations ---
    
    OnionMenuPikminType(size_t idx, PikminType* pik_type);
    
};


/**
 * @brief Info about the Onion menu currently being presented to
 * the player.
 */
struct OnionMenu {

    public:
    
    //--- Members ---
    
    //Pointer to the struct with nest information.
    PikminNest* n_ptr = nullptr;
    
    //Pointer to the leader responsible.
    Leader* l_ptr = nullptr;
    
    //Information on every type's management.
    vector<OnionMenuPikminType> types;
    
    //GUI manager.
    GuiManager gui;
    
    //Is "select all" currently on?
    bool select_all = false;
    
    //If it manages more than 5, this is the Pikmin type page index.
    size_t page = 0;
    
    //Which GUI items are in red right now, if any, and how much time left.
    map<GuiItem*, float> red_items;
    
    //Total page amount. Cache for convenience.
    size_t nr_pages = 0;
    
    //Pikmin types currently on-screen. Cache for convenience.
    vector<OnionMenuPikminType*> on_screen_types;
    
    //List of GUI items for the Onion icons. Cache for convenience.
    vector<GuiItem*> onion_icon_items;
    
    //List of GUI items for the Onion buttons. Cache for convenience.
    vector<GuiItem*> onion_button_items;
    
    //List of GUI items for the Onion amounts. Cache for convenience.
    vector<GuiItem*> onion_amount_items;
    
    //List of GUI items for the group icons. Cache for convenience.
    vector<GuiItem*> group_icon_items;
    
    //List of GUI items for the group buttons. Cache for convenience.
    vector<GuiItem*> group_button_items;
    
    //List of GUI items for the group amounts. Cache for convenience.
    vector<GuiItem*> group_amount_items;
    
    //The button that controls all Onions. Cache for convenience.
    GuiItem* onion_all_button = nullptr;
    
    //The button that controls all groups. Cache for convenience.
    GuiItem* group_all_button = nullptr;
    
    //Left Onion "more..." icon. Cache for convenience.
    GuiItem* onion_more_l_icon = nullptr;
    
    //Right Onion "more..." icon. Cache for convenience.
    GuiItem* onion_more_r_icon = nullptr;
    
    //Left group "more..." icon. Cache for convenience.
    GuiItem* group_more_l_icon = nullptr;
    
    //Right group "more..." icon. Cache for convenience.
    GuiItem* group_more_r_icon = nullptr;
    
    //Previous page button. Cache for convenience.
    GuiItem* prev_page_button = nullptr;
    
    //Next page button. Cache for convenience.
    GuiItem* next_page_button = nullptr;
    
    //Field amount text. Cache for convenience.
    GuiItem* field_amount_text = nullptr;
    
    //Multiply the background alpha by this much.
    float bg_alpha_mult = 0.0f;
    
    //Time left until the menu finishes closing.
    float closing_timer = 0.0f;
    
    //Is the struct meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    
    OnionMenu(PikminNest* n_ptr, Leader* l_ptr);
    ~OnionMenu();
    void add_all_to_group();
    void add_all_to_onion();
    void add_to_group(size_t type_idx);
    void add_to_onion(size_t type_idx);
    void confirm();
    void go_to_page(size_t page);
    void grow_buttons();
    void handle_allegro_event(const ALLEGRO_EVENT &ev);
    void handle_player_action(const PlayerAction &action);
    void start_closing();
    void tick(float delta_t);
    void toggle_select_all();
    
    private:
    
    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    
    //--- Function declarations ---
    
    void make_gui_item_red(GuiItem* item);
    void update();
    
};
