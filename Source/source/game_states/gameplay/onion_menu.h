/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion menu class and related functions.
 */

#ifndef ONION_MENU_INCLUDED
#define ONION_MENU_INCLUDED

#include <string>

#include "../../mobs/mob_utils.h"
#include "../../gui.h"

using std::size_t;
using std::string;

class pikmin_type;

namespace ONION_MENU {
extern const string GUI_FILE_PATH;
extern const float RED_TEXT_DURATION;
extern const size_t TYPES_PER_PAGE;
}


/* ----------------------------------------------------------------------------
 * Contains information about a given Pikmin type in an Onion menu.
 */
struct onion_menu_type_struct {
    //The player wants to add/subtract these many from the group.
    int delta;
    //Index of this type in the Onion's list. Cache for convenience.
    size_t type_idx;
    //Index in the on-screen list, or INVALID. Cache for convenience.
    size_t on_screen_idx;
    //Pikmin type associated. Cache for convenience.
    pikmin_type* pik_type;
    
    onion_menu_type_struct(const size_t idx, pikmin_type* pik_type);
};


/* ----------------------------------------------------------------------------
 * Contains information about the Onion menu currently being presented to
 * the player.
 */
struct onion_menu_struct {
public:
    //Pointer to the struct with nest information.
    pikmin_nest_struct* n_ptr;
    //Pointer to the leader responsible.
    leader* l_ptr;
    //Information on every type's management.
    vector<onion_menu_type_struct> types;
    //GUI manager.
    gui_manager gui;
    //Is "select all" currently on?
    bool select_all;
    //If it manages more than 5, this is the Pikmin type page index.
    size_t page;
    //Which GUI items are in red right now, if any, and how much time left.
    map<gui_item*, float> red_items;
    //Total page amount. Cache for convenience.
    size_t nr_pages;
    //Pikmin types currently on-screen. Cache for convenience.
    vector<onion_menu_type_struct*> on_screen_types;
    //List of GUI items for the Onion icons. Cache for convenience.
    vector<gui_item*> onion_icon_items;
    //List of GUI items for the Onion buttons. Cache for convenience.
    vector<gui_item*> onion_button_items;
    //List of GUI items for the Onion amounts. Cache for convenience.
    vector<gui_item*> onion_amount_items;
    //List of GUI items for the group icons. Cache for convenience.
    vector<gui_item*> group_icon_items;
    //List of GUI items for the group buttons. Cache for convenience.
    vector<gui_item*> group_button_items;
    //List of GUI items for the group amounts. Cache for convenience.
    vector<gui_item*> group_amount_items;
    //The button that controls all Onions. Cache for convenience.
    gui_item* onion_all_button;
    //The button that controls all groups. Cache for convenience.
    gui_item* group_all_button;
    //Left Onion "more..." icon. Cache for convenience.
    gui_item* onion_more_l_icon;
    //Right Onion "more..." icon. Cache for convenience.
    gui_item* onion_more_r_icon;
    //Left group "more..." icon. Cache for convenience.
    gui_item* group_more_l_icon;
    //Right group "more..." icon. Cache for convenience.
    gui_item* group_more_r_icon;
    //Previous page button. Cache for convenience.
    gui_item* prev_page_button;
    //Next page button. Cache for convenience.
    gui_item* next_page_button;
    //Field amount text. Cache for convenience.
    gui_item* field_amount_text;
    //Multiply the background alpha by this much.
    float bg_alpha_mult;
    //Time left until the menu finishes closing.
    float closing_timer;
    //Is the struct meant to be deleted?
    bool to_delete;
    
    onion_menu_struct(pikmin_nest_struct* n_ptr, leader* l_ptr);
    ~onion_menu_struct();
    void add_all_to_group();
    void add_all_to_onion();
    void add_to_group(const size_t type_idx);
    void add_to_onion(const size_t type_idx);
    void confirm();
    void go_to_page(const size_t page);
    void grow_buttons();
    void handle_event(const ALLEGRO_EVENT &ev);
    void start_closing();
    void tick(const float delta_t);
    void toggle_select_all();
    
private:
    //Is it currently closing?
    bool closing;
    
    void make_gui_item_red(gui_item* item);
    void update();
};


#endif //ifndef ONION_MENU_INCLUDED
