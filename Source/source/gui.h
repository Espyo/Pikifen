/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general GUI manager and GUI item classes.
 */

#ifndef GUI_INCLUDED
#define GUI_INCLUDED

#include <functional>
#include <map>
#include <vector>

#include "const.h"
#include "utils/data_file.h"
#include "utils/geometry_utils.h"

using std::map;
using std::vector;


/* ----------------------------------------------------------------------------
 * An item in the GUI. This can be a HUD element, a button, some text, etc.
 */
struct gui_item {
public:
    //On-screen position, in screen ratio.
    point center;
    //Width and height, in screen ratio.
    point size;
    //Is it currently visible?
    bool visible;
    //Can it be selected?
    bool selectable;
    //Is it currently the selected item?
    bool selected;
    //If it is placed inside of another item, specify it here.
    gui_item* parent;
    //Timer that controls it growing in size. Used for juice.
    float juicy_timer;
    
    //What to do when it's time to draw it.
    std::function<void(const point &center, const point &size)> on_draw;
    //What to do when it's time to tick one frame.
    std::function<void(const float time)> on_tick;
    //What to do when the item is activated.
    std::function<void()> on_activate;
    
    //Returns whether the mouse cursor is on top of it.
    bool is_mouse_on(const point &cursor_pos);
    
    gui_item();
};


/* ----------------------------------------------------------------------------
 * GUI manager. This manager is not used the editors, since those work with
 * Dear ImGui. It is responsible for holding information about all GUI
 * elements present on-screen, managing their coordinates, which one is
 * selected, ordering them to be rendered or to handle being activated, etc.
 * Due to the system's flexibility, this is used both to manage the game's
 * heads-up display (HUD) during gameplay, as well as the interactable elements
 * of menus.
 */
class gui_manager {
public:
    //List of items.
    vector<gui_item*> items;
    //Which item is currently selected.
    gui_item* selected_item;
    //Item to activate when the user chooses to go back.
    gui_item* back_item;
    
    //Add an item to the list.
    void add_item(gui_item* item, const string &id = "");
    //Draw the items on-screen.
    void draw();
    //Tick one frame of logic.
    void tick(const float delta_t);
    //Handle an Allegro event.
    void handle_event(const ALLEGRO_EVENT &ev);
    //Handle a button press or release.
    void handle_menu_button(
        const size_t action, const float pos, const size_t player
    );
    //Registers an item's default centers and size.
    void register_coords(
        const string &id,
        const float cx, const float cy, const float w, const float h
    );
    //Reads item coordinates from a data node.
    void read_coords(data_node* node);
    //Sets the currently selected item.
    void set_selected_item(gui_item* item);
    
    //Destroys all allocated items and information.
    void destroy();
    
    gui_manager();
    
private:
    //Registered default centers.
    map<string, point> registered_centers;
    //Registered default sizes.
    map<string, point> registered_sizes;
    //Is the right button pressed?
    bool right_pressed;
    //Is the up button pressed?
    bool up_pressed;
    //Is the left button pressed?
    bool left_pressed;
    //Is the down button pressed?
    bool down_pressed;
    //Is the OK button pressed?
    bool ok_pressed;
    //Is the back button pressed?
    bool back_pressed;
    
};


#endif //ifndef GUI_MANAGER_INCLUDED
