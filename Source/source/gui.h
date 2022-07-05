/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general GUI manager and GUI item classes.
 * These are used during gameplay and menus, and are not related to Dear ImGui,
 * which is the GUI library used for the editors.
 */

#ifndef GUI_INCLUDED
#define GUI_INCLUDED

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro_font.h>

#include "const.h"
#include "controls.h"
#include "misc_structs.h"
#include "utils/data_file.h"
#include "utils/geometry_utils.h"

using std::map;
using std::vector;


namespace GUI {
extern const float AUTO_REPEAT_MAX_INTERVAL;
extern const float AUTO_REPEAT_MIN_INTERVAL;
extern const float AUTO_REPEAT_RAMP_TIME;
extern const float BULLET_PADDING;
extern const float BULLET_RADIUS;
extern const float JUICY_GROW_DURATION;
extern const float JUICY_GROW_ELASTIC_DURATION;
extern const float JUICY_GROW_ICON_MULT;
extern const float JUICY_GROW_TEXT_HIGH_MULT;
extern const float JUICY_GROW_TEXT_LOW_MULT;
extern const float JUICY_GROW_TEXT_MEDIUM_MULT;
}


//Animations for the GUI manager to animate its items with.
enum GUI_MANAGER_ANIMS {
    //None.
    GUI_MANAGER_ANIM_NONE,
    //Items are outward out of view, and slide inward into view.
    GUI_MANAGER_ANIM_OUT_TO_IN,
    //Items are in view, and slide outward out of view.
    GUI_MANAGER_ANIM_IN_TO_OUT,
    //Items are above out of view, and slide downard into view.
    GUI_MANAGER_ANIM_UP_TO_CENTER,
    //Items are in view, and slide up out of view.
    GUI_MANAGER_ANIM_CENTER_TO_UP
};


class gui_manager;


/* ----------------------------------------------------------------------------
 * An item in the GUI. This can be a HUD element, a button, some text, etc.
 */
class gui_item {
public:

    //Juicy animation types for GUI items.
    enum JUICE_TYPES {
        //None.
        JUICE_TYPE_NONE,
        //Text grow effect, low impact.
        JUICE_TYPE_GROW_TEXT_LOW,
        //Text grow effect, medium impact.
        JUICE_TYPE_GROW_TEXT_MEDIUM,
        //Text grow effect, high impact.
        JUICE_TYPE_GROW_TEXT_HIGH,
        //Elastic text grow effect, low impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_LOW,
        //Elastic text grow effect, medium impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM,
        //Elastic text grow effect, high impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH,
        //Icon grow effect.
        JUICE_TYPE_GROW_ICON,
    };
    
    //What GUI manager it belongs to, if any.
    gui_manager* manager;
    //Its raw on-screen position, in screen ratio (or parent ratio).
    point center;
    //Its raw width and height, in screen ratio (or parent ratio).
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
    //Type of the current juice animation.
    JUICE_TYPES juice_type;
    //Timer that controls the current juice animation.
    float juice_timer;
    
    //What to do when it's time to draw it.
    std::function<void(const point &center, const point &size)> on_draw;
    //What to do when it's time to tick one frame.
    std::function<void(const float time)> on_tick;
    //What to do when it receives any Allegro event.
    std::function<void(const ALLEGRO_EVENT &ev)> on_event;
    //What to do when the item is activated.
    std::function<void(const point &cursor_pos)> on_activate;
    //What to do when the mouse cursor is on top of it this frame.
    std::function<void(const point &cursor_pos)> on_mouse_over;
    //What to do when a directional button's pressed with the item selected.
    std::function<bool(const size_t button_id)> on_menu_dir_button;
    //What to do when one of its children became the selected item.
    std::function<void(const gui_item* child)> on_child_selected;
    //What to do when its tooltip needs to be retrieved.
    std::function<string()> on_get_tooltip;
    
    //Adds a child item.
    void add_child(gui_item* item);
    //Returns the bottommost Y coordinate of the item's children items.
    float get_child_bottom();
    //Returns the value related to the current juice animation.
    float get_juice_value();
    //Returns the reference center coordinates.
    point get_reference_center();
    //Returns the reference size.
    point get_reference_size();
    //Returns whether the mouse cursor is on top of it.
    bool is_mouse_on(const point &cursor_pos);
    //Removes an item from the list of children.
    void remove_child(gui_item* item);
    //Starts some juice animation.
    void start_juice_animation(JUICE_TYPES type);
    
    gui_item(const bool selectable = false);
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a bullet point in a list.
 */
class bullet_point_gui_item : public gui_item {
public:
    //Text to display on the bullet point.
    string text;
    //Font to display the text with.
    ALLEGRO_FONT* font;
    //Color to tint the text with.
    ALLEGRO_COLOR color;
    
    bullet_point_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
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
        const ALLEGRO_COLOR &color = COLOR_WHITE
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
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
};


class scroll_gui_item;

/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it behave like a list.
 */
class list_gui_item : public gui_item {
public:
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
    //Total amount of options. Optional.
    size_t nr_options;
    //Index of the currently selected option. Only used if nr_options > 0.
    size_t cur_option_idx;
    
    //What to do when the user picks the previous option.
    std::function<void()> on_previous;
    //What to do when the user picks the next option.
    std::function<void()> on_next;
    
    picker_gui_item(
        const string &base_text, const string &option,
        const size_t nr_options = 0, const size_t cur_option_idx = INVALID
    );
    
private:
    //Highlight one of the arrows due to mouse-over. 255 = none.
    unsigned char arrow_highlight;
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
        const ALLEGRO_COLOR &color = COLOR_WHITE,
        const int flags = ALLEGRO_ALIGN_CENTER
    );
};


/* ----------------------------------------------------------------------------
 * A GUI item with fields ready to make it specialize in showing another
 * item's tooltip.
 */
class tooltip_gui_item : public gui_item {
public:
    //The GUI it belongs to.
    gui_manager* gui;
    
    tooltip_gui_item(gui_manager* gui);
private:
    string prev_text;
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
    //Is it currently responding to input?
    bool responsive;
    //Should it ignore input while animating?
    bool ignore_input_on_animation;
    
    //Add an item to the list.
    void add_item(gui_item* item, const string &id = "");
    //Draw the items on-screen.
    void draw();
    //Tick one frame of logic.
    void tick(const float delta_t);
    //Returns the current item's tooltip, if any.
    string get_current_tooltip();
    //Get an item's draw information.
    bool get_item_draw_info(
        gui_item* item, point* draw_center, point* draw_size
    );
    //Handle an Allegro event.
    void handle_event(const ALLEGRO_EVENT &ev);
    //Handle a button press or release.
    bool handle_menu_button(
        const BUTTONS action, const float pos, const size_t player
    );
    //Hides items until an animation shows them again.
    void hide_items();
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
    //Was the last input a mouse input?
    bool was_last_input_mouse();
    
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
    //Was the last input given a mouse movement?
    bool last_input_was_mouse;
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
    //Are the items currently visible?
    bool visible;
};


#endif //ifndef GUI_MANAGER_INCLUDED
