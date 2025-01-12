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

#pragma once

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro_font.h>

#include "const.h"
#include "controls.h"
#include "misc_structs.h"
#include "libs/data_file.h"
#include "utils/drawing_utils.h"
#include "utils/general_utils.h"
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
extern const float STANDARD_CONTENT_SIZE;
}


//Animations for the GUI manager to animate its items with.
enum GUI_MANAGER_ANIM {

    //None.
    GUI_MANAGER_ANIM_NONE,
    
    //Items are outward out of view, and slide inward into view.
    GUI_MANAGER_ANIM_OUT_TO_IN,
    
    //Items are in view, and slide outward out of view.
    GUI_MANAGER_ANIM_IN_TO_OUT,
    
    //Items are above out of view, and slide downward into view.
    GUI_MANAGER_ANIM_UP_TO_CENTER,
    
    //Items are in view, and slide up out of view.
    GUI_MANAGER_ANIM_CENTER_TO_UP,
    
    //Items are below out of view, and slide upward into view.
    GUI_MANAGER_ANIM_DOWN_TO_CENTER,
    
    //Items are in view, and slide down out of view.
    GUI_MANAGER_ANIM_CENTER_TO_DOWN,
    
    //Items are to the left out of view, and slide right into view.
    GUI_MANAGER_ANIM_LEFT_TO_CENTER,
    
    //Items are in view, and slide left out of view.
    GUI_MANAGER_ANIM_CENTER_TO_LEFT,
    
    //Items are to the right out of view, and slide left into view.
    GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
    
    //Items are in view, and slide right out of view.
    GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
    
};


class gui_manager;


/**
 * @brief An item in the GUI. This can be a HUD element, a button,
 * some text, etc.
 */
class gui_item {

public:

    //--- Misc. declarations ---
    
    //Juicy animation types for GUI items.
    enum JUICE_TYPE {
    
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
    
    
    //--- Members ---
    
    //What GUI manager it belongs to, if any.
    gui_manager* manager = nullptr;
    
    //Its raw on-screen position, in screen ratio (or parent ratio).
    point center;
    
    //Its raw width and height, in screen ratio (or parent ratio).
    point size;
    
    //Is it currently visible?
    bool visible = true;
    
    //Is it currentl responsive?
    bool responsive = true;
    
    //Can it be selected?
    bool selectable = false;
    
    //Is it currently the selected item?
    bool selected = false;
    
    //If it is placed inside of another item, specify it here.
    gui_item* parent = nullptr;
    
    //List of children items, that are placed inside this one.
    vector<gui_item*> children;
    
    //Vertical offset (height percentage) of the items inside of it, if any.
    float offset = 0.0f;
    
    //Padding amount, if it has items inside of it.
    float padding = 0.0f;
    
    //Can this item's activation be auto-repeated by holding the button down?
    bool can_auto_repeat = false;
    
    //Type of the current juice animation.
    JUICE_TYPE juice_type = JUICE_TYPE_NONE;
    
    //Timer that controls the current juice animation.
    float juice_timer = 0.0f;
    
    //What to do when it's time to draw it.
    std::function<void(const point &center, const point &size)> on_draw = nullptr;
    
    //What to do when it's time to tick one frame.
    std::function<void(float time)> on_tick = nullptr;
    
    //What to do when it receives any Allegro event.
    std::function<void(const ALLEGRO_EVENT &ev)> on_event = nullptr;
    
    //What to do when the item is activated.
    std::function<void(const point &cursor_pos)> on_activate = nullptr;
    
    //What to do when the mouse cursor is on top of it this frame.
    std::function<void(const point &cursor_pos)> on_mouse_over = nullptr;
    
    //What to do when a directional button's pressed with the item selected.
    std::function<bool(size_t button_id)> on_menu_dir_button = nullptr;
    
    //What to do when it gets selected.
    std::function<void()> on_selected = nullptr;
    
    //What to do when one of its children became the selected item.
    std::function<void(const gui_item* child)> on_child_selected = nullptr;
    
    //What to do when its tooltip needs to be retrieved.
    std::function<string()> on_get_tooltip = nullptr;
    
    
    //--- Function declarations ---
    
    explicit gui_item(bool selectable = false);
    bool activate(const point &cursor_pos);
    void add_child(gui_item* item);
    void delete_all_children();
    float get_child_bottom();
    float get_juice_value();
    point get_reference_center();
    point get_reference_size();
    bool is_mouse_on(const point &cursor_pos);
    bool is_responsive();
    bool is_visible();
    void remove_child(gui_item* item);
    void start_juice_animation(JUICE_TYPE type);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a bullet
 * point in a list.
 */
class bullet_gui_item : public gui_item {

public:

    //--- Members ---
    
    //Text to display on the bullet point.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    bullet_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );

    void def_draw_code(const point& center, const point& size);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a button.
 */
class button_gui_item : public gui_item {

public:

    //--- Members ---
    
    //Text to display on the button.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    button_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );

    void def_draw_code(const point& center, const point& size);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a checkbox.
 */
class check_gui_item : public gui_item {

public:

    //--- Members ---
    
    //Current value.
    bool value = false;
    
    //If not nullptr, the value is automatically adjusted to reflect this
    //variable and vice-versa.
    bool* value_ptr = nullptr;

    //Text to display on the button.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    explicit check_gui_item(
        bool value, const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    explicit check_gui_item(
        bool* value_ptr, const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );

    void def_activate_code();
    void def_draw_code(const point& center, const point& size);
    
};


class scroll_gui_item;

/**
 * @brief A GUI item with fields ready to make it behave like a list.
 */
class list_gui_item : public gui_item {

public:

    //--- Members ---
    
    //What the offset is supposed to be, after it finishes animating.
    float target_offset = 0.0f;
    
    
    //--- Function declarations ---
    
    list_gui_item();

    void def_child_selected_code(const gui_item* child);
    void def_draw_code(const point& center, const point& size);
    void def_event_code(const ALLEGRO_EVENT & ev);
    void def_tick_code(float delta_t);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a previous/next
 * option picker.
 */
class picker_gui_item : public gui_item {

public:

    //--- Members ---
    
    //The text to show before the currently selected option.
    string base_text;
    
    //The currently selected option.
    string option;
    
    //Total amount of options. Optional.
    size_t nr_options = 0;
    
    //Index of the currently selected option. Only used if nr_options > 0.
    size_t cur_option_idx = INVALID;
    
    //What to do when the user picks the previous option.
    std::function<void()> on_previous = nullptr;
    
    //What to do when the user picks the next option.
    std::function<void()> on_next = nullptr;
    
    
    //--- Function declarations ---
    
    picker_gui_item(
        const string &base_text, const string &option,
        size_t nr_options = 0, size_t cur_option_idx = INVALID
    );

    void def_activate_code(const point& cursor_pos);
    void def_draw_code(const point& center, const point& size);
    bool def_menu_dir_code(size_t button_id);
    void def_mouse_over_code(const point & cursor_pos);

    
private:

    //--- Members ---
    
    //Highlight one of the arrows due to mouse-over. 255 = none.
    unsigned char arrow_highlight = 255;
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a scrollbar.
 */
class scroll_gui_item : public gui_item {

public:

    //--- Members ---
    
    //What item this scrollbar is in charge of controlling.
    list_gui_item* list_item = nullptr;
    
    
    //--- Function declarations ---
    
    scroll_gui_item();

    void def_draw_code(const point& center, const point& size);
    void def_event_code(const ALLEGRO_EVENT & ev);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a simple
 * text display.
 */
class text_gui_item : public gui_item {

public:

    //--- Members ---
    
    //Text to display.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    //Allegro flags.
    int flags = ALLEGRO_ALIGN_CENTER;
    
    //Wrap long lines. Also enables markup.
    bool line_wrap = false;
    
    //Whether to show a selection box when selected.
    bool show_selection_box = false;
    
    
    //--- Function declarations ---
    
    text_gui_item(
        const string &text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR &color = COLOR_WHITE,
        int flags = ALLEGRO_ALIGN_CENTER
    );

    void def_draw_code(const point& center, const point& size);
    
};


/**
 * @brief A GUI item with fields ready to make it specialize in showing another
 * item's tooltip.
 */
class tooltip_gui_item : public gui_item {

public:

    //--- Members ---
    
    //The GUI it belongs to.
    gui_manager* gui = nullptr;
    
    
    //--- Function declarations ---
    
    explicit tooltip_gui_item(gui_manager* gui);

    void def_draw_code(const point& center, const point& size);

    
private:

    //--- Members ---
    
    //Text it was showing the previous frame.
    string prev_text;
    
};


/**
 * @brief GUI manager.
 *
 * This manager is not used in the editors, since those work with
 * Dear ImGui. It is responsible for holding information about all GUI
 * elements present on-screen, managing their coordinates, which one is
 * selected, ordering them to be rendered or to handle being activated, etc.
 * Due to the system's flexibility, this is used both to manage the game's
 * heads-up display (HUD) during gameplay, as well as the interactable elements
 * of menus.
 */
class gui_manager {

public:

    //--- Members ---
    
    //List of items.
    vector<gui_item*> items;
    
    //Which item is currently selected.
    gui_item* selected_item = nullptr;
    
    //Item to activate when the user chooses to go back.
    gui_item* back_item = nullptr;
    
    //Is it currently responding to input?
    bool responsive = true;
    
    //Should it ignore input while animating?
    bool ignore_input_on_animation = true;
    
    //What to do when the currently selected item changes.
    std::function<void()> on_selection_changed = nullptr;
    
    
    //--- Function declarations ---
    
    gui_manager();
    void add_item(gui_item* item, const string &id = "");
    void draw();
    void tick(float delta_t);
    string get_current_tooltip();
    bool get_item_draw_info(
        gui_item* item, point* draw_center, point* draw_size
    );
    void handle_event(const ALLEGRO_EVENT &ev);
    bool handle_player_action(const player_action &action);
    void hide_items();
    void read_coords(data_node* node);
    void register_coords(
        const string &id,
        float cx, float cy, float w, float h
    );
    void remove_item(gui_item* item);
    void set_selected_item(gui_item* item, bool silent = false);
    void show_items();
    void start_animation(
        const GUI_MANAGER_ANIM type, float duration
    );
    bool was_last_input_mouse();
    void destroy();
    
private:

    //--- Members ---
    
    //Registered default centers.
    map<string, point> registered_centers;
    
    //Registered default sizes.
    map<string, point> registered_sizes;
    
    //Is the right button pressed?
    bool right_pressed = false;
    
    //Is the up button pressed?
    bool up_pressed = false;
    
    //Is the left button pressed?
    bool left_pressed = false;
    
    //Is the down button pressed?
    bool down_pressed = false;
    
    //Is the OK button pressed?
    bool ok_pressed = false;
    
    //Is the back button pressed?
    bool back_pressed = false;
    
    //Was the last input given a mouse movement?
    bool last_input_was_mouse = false;
    
    //Is the current item's activation auto-repeat mode on?
    bool auto_repeat_on = false;
    
    //How long the activation button has been held for.
    float auto_repeat_duration = 0.0f;
    
    //How long until the item's next activation, from the button being held.
    float auto_repeat_next_activation = 0.0f;
    
    //Type of the current animation, if any.
    GUI_MANAGER_ANIM anim_type = GUI_MANAGER_ANIM_NONE;
    
    //Timer for the current animation.
    timer anim_timer;
    
    //Are the items currently visible?
    bool visible = true;
    
};
