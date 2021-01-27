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

#include <allegro5/allegro_font.h>

#include "const.h"
#include "misc_structs.h"
#include "utils/data_file.h"
#include "utils/geometry_utils.h"

using std::map;
using std::vector;


enum GUI_MANAGER_ANIMS {
    GUI_MANAGER_ANIM_NONE,
    GUI_MANAGER_ANIM_OUT_TO_IN,
    GUI_MANAGER_ANIM_IN_TO_OUT,
};


/* ----------------------------------------------------------------------------
 * An item in the GUI. This can be a HUD element, a button, some text, etc.
 */
class gui_item {
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
    //List of children items, that are placed inside this one.
    vector<gui_item*> children;
    //Vertical offset (height percentage) of the items inside of it, if any.
    float offset;
    //Padding amount, if it has items inside of it.
    float padding;
    //Can this item's activation be auto-repeated by holding the button down?
    bool can_auto_repeat;
    //Timer that controls it growing in size. Used for juice.
    float juice_timer;
    
    //What to do when it's time to draw it.
    std::function<void(const point &center, const point &size)> on_draw;
    //What to do when it's time to tick one frame.
    std::function<void(const float time)> on_tick;
    //What to do when it receives any Allegro event.
    std::function<void(const ALLEGRO_EVENT &ev)> on_event;
    //What to do when the item is activated.
    std::function<void(const point &cursor_pos)> on_activate;
    //What to do when a directional button's pressed with the item selected.
    std::function<bool(const size_t button_id)> on_menu_dir_button;
    //What to do when one of its children became the selected item.
    std::function<void(const gui_item* child)> on_child_selected;
    
    //Adds a child item.
    void add_child(gui_item* item);
    //Returns the bottommost Y coordinate of the item's children items.
    float get_child_bottom();
    //Returns the juicy grow amount for the current juicy grow animation.
    float get_juicy_grow_amount();
    //Returns the real center coordinates.
    point get_real_center();
    //Returns the real size coordinates.
    point get_real_size();
    //Returns whether the mouse cursor is on top of it.
    bool is_mouse_on(const point &cursor_pos);
    //Removes an item from the list of children.
    void remove_child(gui_item* item);
    //Starts the process of animation a juicy grow effect.
    void start_juicy_grow();
    
    gui_item(const bool selectable = false);
    
    static const float JUICY_GROW_DURATION;
    static const float JUICY_GROW_DELTA;
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a button.
 */
class button_gui_item : public gui_item {
public:
    //Text to display on the button.
    string text;
    //Font to display the text with.
    ALLEGRO_FONT* font;
    //Color to tint the text with.
    ALLEGRO_COLOR color;
    
    button_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = al_map_rgb(255, 255, 255)
    );
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a checkbox.
 */
class check_gui_item : public gui_item {
public:
    //Variable that controls the checkmark value.
    bool* value;
    //Text to display on the button.
    string text;
    //Font to display the text with.
    ALLEGRO_FONT* font;
    //Color to tint the text with.
    ALLEGRO_COLOR color;
    
    check_gui_item(
        bool* value, const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = al_map_rgb(255, 255, 255)
    );
};


class scroll_gui_item;

/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a list.
 */
class list_gui_item : public gui_item {
public:
    //What scrollbar item controls it, if any.
    scroll_gui_item* scroll_item;
    //What the offset is supposed to be, after it finishes animating.
    float target_offset;
    
    list_gui_item();
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a previous/next
 * option picker.
 */
class picker_gui_item : public gui_item {
public:
    //The text to show before the currently selected option.
    string base_text;
    //The currently selected option.
    string option;
    
    //What to do when the user picks the previous option.
    std::function<void()> on_previous;
    //What to do when the user picks the next option.
    std::function<void()> on_next;
    
    picker_gui_item(const string &base_text, const string &option);
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a scrollbar.
 */
class scroll_gui_item : public gui_item {
public:
    //What item this scrollbar is in charge of controlling.
    list_gui_item* list_item;
    
    scroll_gui_item();
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a simple text display.
 */
class text_gui_item : public gui_item {
public:
    //Text to display.
    string text;
    //Font to display the text with.
    ALLEGRO_FONT* font;
    //Color to tint the text with.
    ALLEGRO_COLOR color;
    //Allegro flags.
    int flags;
    
    text_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = al_map_rgb(255, 255, 255),
        const int flags = ALLEGRO_ALIGN_CENTER
    );
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
    //Reads item coordinates from a data node.
    void read_coords(data_node* node);
    //Registers an item's default centers and size.
    void register_coords(
        const string &id,
        const float cx, const float cy, const float w, const float h
    );
    //Removes an item from the list.
    void remove_item(gui_item* item);
    //Sets the currently selected item.
    void set_selected_item(gui_item* item);
    //Starts an animation tha affects all items.
    void start_animation(
        const GUI_MANAGER_ANIMS type, const float duration
    );
    
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
    //Is the current item's activation auto-repeat mode on?
    bool auto_repeat_on;
    //How long the activation button has been held for.
    float auto_repeat_duration;
    //How long until the item's next activation, from the button being held.
    float auto_repeat_next_activation;
    //Type of the current animation, if any.
    GUI_MANAGER_ANIMS anim_type;
    //Timer for the current animation.
    timer anim_timer;
    
    static const float AUTO_REPEAT_MAX_INTERVAL;
    static const float AUTO_REPEAT_MIN_INTERVAL;
    static const float AUTO_REPEAT_RAMP_TIME;
};


#endif //ifndef GUI_MANAGER_INCLUDED
