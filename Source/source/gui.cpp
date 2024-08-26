/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General GUI manager and GUI item classes.
 * These are used during gameplay and menus, and are not related to Dear ImGui,
 * which is the GUI library used for the editors.
 */

#include <algorithm>

#include "gui.h"

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "utils/string_utils.h"


namespace GUI {

//Interval between auto-repeat activations, at the slowest speed.
const float AUTO_REPEAT_MAX_INTERVAL = 0.3f;

//Interval between auto-repeat activations, at the fastest speed.
const float AUTO_REPEAT_MIN_INTERVAL = 0.011f;

//How long it takes for the auto-repeat activations to reach max speed.
const float AUTO_REPEAT_RAMP_TIME = 0.9f;

//Padding before/after the circle in a bullet point item.
const float BULLET_PADDING = 6.0f;

//Radius of the circle that represents the bullet in a bullet point item.
const float BULLET_RADIUS = 4.0f;

//When an item does a juicy grow, this is the full effect duration.
const float JUICY_GROW_DURATION = 0.3f;

//When an item does a juicy elastic grow, this is the full effect duration.
const float JUICY_GROW_ELASTIC_DURATION = 0.4f;

//Grow scale multiplier for a juicy icon grow animation.
const float JUICY_GROW_ICON_MULT = 5.0f;

//Grow scale multiplier for a juicy text high grow animation.
const float JUICY_GROW_TEXT_HIGH_MULT = 0.15f;

//Grow scale multiplier for a juicy text low grow animation.
const float JUICY_GROW_TEXT_LOW_MULT = 0.02f;

//Grow scale multiplier for a juicy text medium grow animation.
const float JUICY_GROW_TEXT_MEDIUM_MULT = 0.05f;

}


/**
 * @brief Constructs a new bullet point gui item object.
 *
 * @param text Text to display on the bullet point.
 * @param font Font for the button's text.
 * @param color Color of the button's text.
 */
bullet_point_gui_item::bullet_point_gui_item(
    const string &text, ALLEGRO_FONT* font, const ALLEGRO_COLOR &color
) :
    gui_item(true),
    text(text),
    font(font),
    color(color) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        float item_x_start = center.x - size.x * 0.5;
        float text_x_offset =
            GUI::BULLET_RADIUS * 2 +
            GUI::BULLET_PADDING * 2;
        point text_space(
            std::max(1.0f, size.x - text_x_offset),
            size.y
        );
        
        al_draw_filled_circle(
            item_x_start +
            GUI::BULLET_RADIUS +
            GUI::BULLET_PADDING,
            center.y,
            GUI::BULLET_RADIUS,
            this->color
        );
        float juicy_grow_amount = get_juice_value();
        draw_compressed_scaled_text(
            this->font, this->color,
            point(item_x_start + text_x_offset, center.y),
            point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER, text_space, true,
            this->text
        );
        if(selected) {
            draw_textured_box(
                center,
                size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
                game.sys_assets.bmp_focus_box
            );
        }
    };
}


/**
 * @brief Constructs a new button gui item object.
 *
 * @param text Text to display on the button.
 * @param font Font for the button's text.
 * @param color Color of the button's text.
 */
button_gui_item::button_gui_item(
    const string &text, ALLEGRO_FONT* font, const ALLEGRO_COLOR &color
) :
    gui_item(true),
    text(text),
    font(font),
    color(color) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        draw_button(
            center, size, this->text, this->font, this->color, selected,
            get_juice_value()
        );
    };
}


/**
 * @brief Constructs a new check gui item object.
 *
 * @param value Pointer to the boolean that stores the current checkmark value.
 * @param text Text to display on the checkbox.
 * @param font Font for the checkbox's text.
 * @param color Color of the checkbox's text.
 */
check_gui_item::check_gui_item(
    bool* value, const string &text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR &color
) :
    gui_item(true),
    value(value),
    text(text),
    font(font),
    color(color) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        float juicy_grow_amount = get_juice_value();
        draw_compressed_scaled_text(
            this->font, this->color,
            point(center.x - size.x * 0.45, center.y),
            point(1.0f + juicy_grow_amount, 1.0f + juicy_grow_amount),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
            point(size.x * 0.90, size.y), true, this->text
        );
        
        draw_bitmap(
            (*this->value) ?
            game.sys_assets.bmp_checkbox_check :
            game.sys_assets.bmp_checkbox_no_check,
            point((center.x + size.x * 0.5) - 40, center.y),
            point(32, -1)
        );
        
        ALLEGRO_COLOR box_tint =
            selected ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
            
        draw_textured_box(
            center, size, game.sys_assets.bmp_bubble_box, box_tint
        );
        
        if(selected) {
            draw_textured_box(
                center,
                size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
                game.sys_assets.bmp_focus_box
            );
        }
    };
    
    on_activate =
    [this] (const point &) {
        (*this->value) = !(*this->value);
        this->start_juice_animation(JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM);
    };
}


/**
 * @brief Constructs a new gui item object.
 *
 * @param selectable Can the item be selected by the player?
 */
gui_item::gui_item(const bool selectable) :
    selectable(selectable) {
    
}


/**
 * @brief Activates the item.
 *
 * @param cursor_pos Cursor coordinates, if applicable.
 * @return Whether it could activate it.
 */
bool gui_item::activate(const point &cursor_pos) {
    if(!on_activate) return false;
    on_activate(cursor_pos);
    
    ALLEGRO_SAMPLE* sample =
        this == manager->back_item ?
        game.sys_assets.sfx_menu_back :
        game.sys_assets.sfx_menu_activate;
    sfx_source_config_t activate_sfx_config;
    activate_sfx_config.gain = 0.75f;
    game.audio.create_ui_sfx_source(sample, activate_sfx_config);
    
    return true;
}


/**
 * @brief Adds a child item.
 *
 * @param item Item to add as a child item.
 */
void gui_item::add_child(gui_item* item) {
    children.push_back(item);
    item->parent = this;
}


/**
 * @brief Removes and deletes all children items.
 */
void gui_item::delete_all_children() {
    while(!children.empty()) {
        gui_item* i_ptr = children[0];
        remove_child(i_ptr);
        manager->remove_item(i_ptr);
        delete i_ptr;
    }
}


/**
 * @brief Returns the bottommost Y coordinate, in height ratio,
 * of the item's children items.
 *
 * @return The Y coordinate.
 */
float gui_item::get_child_bottom() {
    float bottommost = 0.0f;
    for(size_t c = 0; c < children.size(); ++c) {
        gui_item* c_ptr = children[c];
        bottommost =
            std::max(
                bottommost,
                c_ptr->center.y + (c_ptr->size.y / 2.0f)
            );
    }
    return bottommost;
}


/**
 * @brief Returns the value related to the current juice animation.
 *
 * @return The juice value, or 0 if there's no animation.
 */
float gui_item::get_juice_value() {
    switch(juice_type) {
    case JUICE_TYPE_GROW_TEXT_LOW: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, anim_ratio) *
            GUI::JUICY_GROW_TEXT_LOW_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_MEDIUM: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, anim_ratio) *
            GUI::JUICY_GROW_TEXT_MEDIUM_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_HIGH: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, anim_ratio) *
            GUI::JUICY_GROW_TEXT_HIGH_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_LOW: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, anim_ratio) *
            GUI::JUICY_GROW_TEXT_LOW_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, anim_ratio) *
            GUI::JUICY_GROW_TEXT_MEDIUM_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, anim_ratio) *
            GUI::JUICY_GROW_TEXT_HIGH_MULT;
    }
    case JUICE_TYPE_GROW_ICON: {
        float anim_ratio =
            1.0f - (juice_timer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, anim_ratio) *
            GUI::JUICY_GROW_ICON_MULT;
    }
    default: {
        return 0.0f;
    }
    }
}


/**
 * @brief Returns the reference center coordinates,
 * i.e. used when not animating.
 *
 * @return The center.
 */
point gui_item::get_reference_center() {
    if(parent) {
        point parent_s =
            parent->get_reference_size() - (parent->padding * 2.0f);
        point parent_c =
            parent->get_reference_center();
        point result = center * parent_s;
        result.x += parent_c.x - parent_s.x / 2.0f;
        result.y += parent_c.y - parent_s.y / 2.0f;
        result.y -= parent_s.y * parent->offset;
        return result;
    } else {
        return point(center.x * game.win_w, center.y * game.win_h);
    }
}


/**
 * @brief Returns the reference width and height, i.e. used when not animating.
 *
 * @return The size.
 */
point gui_item::get_reference_size() {
    point mult;
    if(parent) {
        mult = parent->get_reference_size() - (parent->padding * 2.0f);
    } else {
        mult.x = game.win_w;
        mult.y = game.win_h;
    }
    return size * mult;
}


/**
 * @brief Returns whether the mouse cursor is on top of it.
 *
 * @param cursor_pos Position of the mouse cursor, in screen coordinates.
 * @return Whether the cursor is on top.
 */
bool gui_item::is_mouse_on(const point &cursor_pos) {
    if(parent && !parent->is_mouse_on(cursor_pos)) {
        return false;
    }
    
    point c = get_reference_center();
    point s = get_reference_size();
    return
        (
            cursor_pos.x >= c.x - s.x * 0.5 &&
            cursor_pos.x <= c.x + s.x * 0.5 &&
            cursor_pos.y >= c.y - s.y * 0.5 &&
            cursor_pos.y <= c.y + s.y * 0.5
        );
}


/**
 * @brief Returns whether or not it is responsive, and also checks the parents.
 *
 * @return Whether it is responsive.
 */
bool gui_item::is_responsive() {
    if(parent) return parent->is_responsive();
    return responsive;
}


/**
 * @brief Returns whether or not it is visible, and also checks the parents.
 *
 * @return Whether it is visible.
 */
bool gui_item::is_visible() {
    if(parent) return parent->is_visible();
    return visible;
}


/**
 * @brief Removes an item from the list of children, without deleting it.
 *
 * @param item Child item to remove.
 */
void gui_item::remove_child(gui_item* item) {
    for(size_t c = 0; c < children.size(); ++c) {
        if(children[c] == item) {
            children.erase(children.begin() + c);
        }
    }
    
    item->parent = nullptr;
}


/**
 * @brief Starts some juice animation.
 *
 * @param type Type of juice animation.
 */
void gui_item::start_juice_animation(JUICE_TYPE type) {
    juice_type = type;
    switch(type) {
    case JUICE_TYPE_GROW_TEXT_LOW:
    case JUICE_TYPE_GROW_TEXT_MEDIUM:
    case JUICE_TYPE_GROW_TEXT_HIGH:
    case JUICE_TYPE_GROW_ICON: {
        juice_timer = GUI::JUICY_GROW_DURATION;
        break;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_LOW:
    case JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM:
    case JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH: {
        juice_timer = GUI::JUICY_GROW_ELASTIC_DURATION;
        break;
    }
    default: {
        break;
    }
    }
}


/**
 * @brief Constructs a new gui manager object.
 */
gui_manager::gui_manager() {

    anim_timer =
        timer(
    0.0f, [this] () {
        switch(anim_type) {
        case GUI_MANAGER_ANIM_IN_TO_OUT:
        case GUI_MANAGER_ANIM_CENTER_TO_UP:
        case GUI_MANAGER_ANIM_CENTER_TO_DOWN:
        case GUI_MANAGER_ANIM_CENTER_TO_LEFT:
        case GUI_MANAGER_ANIM_CENTER_TO_RIGHT: {
            visible = false;
            break;
        }
        default: {
            visible = true;
            break;
        }
        }
    }
        );
}


/**
 * @brief Add an item to the list.
 *
 * @param item Pointer to the new item.
 * @param id If this item has an associated ID, specify it here.
 * Empty string if none.
 */
void gui_manager::add_item(gui_item* item, const string &id) {
    auto c = registered_centers.find(id);
    if(c != registered_centers.end()) {
        item->center = c->second;
    }
    auto s = registered_sizes.find(id);
    if(s != registered_sizes.end()) {
        item->size = s->second;
    }
    
    items.push_back(item);
    item->manager = this;
}


/**
 * @brief Destroys and deletes all items and information.
 */
void gui_manager::destroy() {
    set_selected_item(nullptr);
    back_item = nullptr;
    for(size_t i = 0; i < items.size(); i++) {
        delete items[i];
    }
    items.clear();
    registered_centers.clear();
    registered_sizes.clear();
}


/**
 * @brief Draws all items on-screen.
 */
void gui_manager::draw() {
    if(!visible) return;
    
    int ocr_x = 0;
    int ocr_y = 0;
    int ocr_w = 0;
    int ocr_h = 0;
    
    for(size_t i = 0; i < items.size(); ++i) {
    
        gui_item* i_ptr = items[i];
        
        if(!i_ptr->on_draw) continue;
        
        point draw_center = i_ptr->get_reference_center();
        point draw_size = i_ptr->get_reference_size();
        
        if(!get_item_draw_info(i_ptr, &draw_center, &draw_size)) continue;
        
        if(i_ptr->parent) {
            point parent_c;
            point parent_s;
            if(!get_item_draw_info(i_ptr->parent, &parent_c, &parent_s)) {
                continue;
            }
            al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
            al_set_clipping_rectangle(
                (parent_c.x - parent_s.x / 2.0f) + 1,
                (parent_c.y - parent_s.y / 2.0f) + 1,
                parent_s.x - 2,
                parent_s.y - 2
            );
        }
        
        i_ptr->on_draw(draw_center, draw_size);
        
        if(i_ptr->parent) {
            al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
        }
    }
}


/**
 * @brief Returns the currently selected item's tooltip, if any.
 *
 * @return The tooltip.
 */
string gui_manager::get_current_tooltip() {
    if(!selected_item) return string();
    if(!selected_item->on_get_tooltip) return string();
    return selected_item->on_get_tooltip();
}


/**
 * @brief Returns a given item's drawing information.
 *
 * @param item What item to check.
 * @param draw_center The drawing center coordinates to use.
 * @param draw_size The drawing width and height to use.
 * @return True if the item exists and is meant to be drawn, false otherwise.
 */
bool gui_manager::get_item_draw_info(
    gui_item* item, point* draw_center, point* draw_size
) {
    if(!item->is_visible()) return false;
    if(item->size.x == 0.0f) return false;
    
    point final_center = item->get_reference_center();
    point final_size = item->get_reference_size();
    
    if(anim_timer.time_left > 0.0f) {
        switch(anim_type) {
        case GUI_MANAGER_ANIM_OUT_TO_IN: {
            point start_center;
            float angle =
                get_angle(
                    point(game.win_w, game.win_h) / 2.0f,
                    final_center
                );
            start_center.x = final_center.x + cos(angle) * game.win_w;
            start_center.y = final_center.y + sin(angle) * game.win_h;
            
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, start_center.x, final_center.x
                );
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, start_center.y, final_center.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_IN_TO_OUT: {
            point end_center;
            float angle =
                get_angle(
                    point(game.win_w, game.win_h) / 2.0f,
                    final_center
                );
            end_center.x = final_center.x + cos(angle) * game.win_w;
            end_center.y = final_center.y + sin(angle) * game.win_h;
            
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_IN, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.x, end_center.x
                );
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_IN, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.y, end_center.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_UP_TO_CENTER: {
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.y - game.win_h, final_center.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_UP: {
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.y, final_center.y - game.win_h
                );
            break;
            
        } case GUI_MANAGER_ANIM_DOWN_TO_CENTER: {
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.y + game.win_h, final_center.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_DOWN: {
            final_center.y =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.y, final_center.y + game.win_h
                );
            break;
            
        } case GUI_MANAGER_ANIM_LEFT_TO_CENTER: {
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.x - game.win_w, final_center.x
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_LEFT: {
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.x, final_center.x - game.win_w
                );
            break;
            
        } case GUI_MANAGER_ANIM_RIGHT_TO_CENTER: {
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.x + game.win_w, final_center.x
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_RIGHT: {
            final_center.x =
                interpolate_number(
                    ease(EASE_METHOD_OUT, 1.0f - anim_timer.get_ratio_left()),
                    0.0f, 1.0f, final_center.x, final_center.x + game.win_w
                );
            break;
            
        } default: {
            break;
            
        }
        }
    }
    
    *draw_center = final_center;
    *draw_size = final_size;
    return true;
}


/**
 * @brief Handle an Allegro event.
 * Controls are handled in handle_player_action.
 *
 * @param ev Event.
 */
void gui_manager::handle_event(const ALLEGRO_EVENT &ev) {
    if(!responsive) return;
    if(anim_timer.get_ratio_left() > 0.0f && ignore_input_on_animation) return;
    
    bool mouse_moved = false;
    
    //Mousing over an item and clicking.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        gui_item* selection_result = nullptr;
        for(size_t i = 0; i < items.size(); ++i) {
            gui_item* i_ptr = items[i];
            if(
                i_ptr->is_mouse_on(point(ev.mouse.x, ev.mouse.y)) &&
                i_ptr->is_responsive() &&
                i_ptr->selectable
            ) {
                selection_result = i_ptr;
                if(i_ptr->on_mouse_over) {
                    i_ptr->on_mouse_over(point(ev.mouse.x, ev.mouse.y));
                }
                break;
            }
        }
        set_selected_item(selection_result);
        mouse_moved = true;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
        if(
            selected_item &&
            selected_item->is_responsive() &&
            selected_item->on_activate
        ) {
            selected_item->activate(point(ev.mouse.x, ev.mouse.y));
            auto_repeat_on = true;
            auto_repeat_duration = 0.0f;
            auto_repeat_next_activation = GUI::AUTO_REPEAT_MAX_INTERVAL;
        }
        mouse_moved = true;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1) {
        auto_repeat_on = false;
        mouse_moved = true;
    }
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i]->is_responsive() && items[i]->on_event) {
            items[i]->on_event(ev);
        }
    }
    
    if(mouse_moved) last_input_was_mouse = true;
}


/**
 * @brief Handles a player input.
 *
 * @param action Data about the player action.
 * @return Whether the input was used.
 */
bool gui_manager::handle_player_action(const player_action &action) {
    if(!responsive) {
        return false;
    }
    if(
        anim_timer.get_ratio_left() > 0.0f &&
        ignore_input_on_animation
    ) {
        return false;
    }
    
    bool is_down = (action.value >= 0.5);
    bool button_recognized = true;
    
    switch(action.action_type_id) {
    case PLAYER_ACTION_TYPE_MENU_RIGHT:
    case PLAYER_ACTION_TYPE_MENU_UP:
    case PLAYER_ACTION_TYPE_MENU_LEFT:
    case PLAYER_ACTION_TYPE_MENU_DOWN: {

        //Selecting a different item with the arrow keys.
        size_t pressed = PLAYER_ACTION_TYPE_NONE;
        
        switch(action.action_type_id) {
        case PLAYER_ACTION_TYPE_MENU_RIGHT: {
            if(!right_pressed && is_down) {
                pressed = PLAYER_ACTION_TYPE_MENU_RIGHT;
            }
            right_pressed = is_down;
            break;
        } case PLAYER_ACTION_TYPE_MENU_UP: {
            if(!up_pressed && is_down) {
                pressed = PLAYER_ACTION_TYPE_MENU_UP;
            }
            up_pressed = is_down;
            break;
        } case PLAYER_ACTION_TYPE_MENU_LEFT: {
            if(!left_pressed && is_down) {
                pressed = PLAYER_ACTION_TYPE_MENU_LEFT;
            }
            left_pressed = is_down;
            break;
        } case PLAYER_ACTION_TYPE_MENU_DOWN: {
            if(!down_pressed && is_down) {
                pressed = PLAYER_ACTION_TYPE_MENU_DOWN;
            }
            down_pressed = is_down;
            break;
        } default: {
            break;
        }
        }
        
        if(pressed == PLAYER_ACTION_TYPE_NONE) break;
        
        if(!selected_item) {
            for(size_t i = 0; i < items.size(); ++i) {
                if(items[i]->is_responsive() && items[i]->selectable) {
                    set_selected_item(items[i]);
                    break;
                }
            }
            if(selected_item) {
                break;
            }
        }
        if(!selected_item) {
            //No item can be selected.
            break;
        }
        
        vector<point> selectables;
        vector<gui_item*> selectable_ptrs;
        size_t selectable_idx = INVALID;
        float direction = 0.0f;
        
        switch(pressed) {
        case PLAYER_ACTION_TYPE_MENU_DOWN: {
            direction = TAU * 0.25f;
            break;
        }
        case PLAYER_ACTION_TYPE_MENU_LEFT: {
            direction = TAU * 0.50f;
            break;
        }
        case PLAYER_ACTION_TYPE_MENU_UP: {
            direction = TAU * 0.75f;
            break;
        }
        }
        
        if(
            selected_item &&
            selected_item->is_responsive() &&
            selected_item->on_menu_dir_button
        ) {
            if(selected_item->on_menu_dir_button(pressed)) {
                //If it returned true, that means the following logic about
                //changing the current item needs to be skipped.
                break;
            }
        }
        
        float min_y = 0;
        float max_y = game.win_h;
        
        for(size_t i = 0; i < items.size(); ++i) {
            gui_item* i_ptr = items[i];
            if(i_ptr->is_responsive() && i_ptr->selectable) {
                point i_center = i_ptr->get_reference_center();
                if(i_ptr == selected_item) {
                    selectable_idx = selectables.size();
                }
                
                min_y = std::min(min_y, i_center.y);
                max_y = std::max(max_y, i_center.y);
                
                selectable_ptrs.push_back(i_ptr);
                selectables.push_back(i_ptr->get_reference_center());
            }
        }
        
        size_t new_selectable_idx =
            select_next_item_directionally(
                selectables,
                selectable_idx,
                direction,
                point(game.win_w, max_y - min_y)
            );
            
        if(new_selectable_idx != selectable_idx) {
            set_selected_item(selectable_ptrs[new_selectable_idx]);
            if(
                selected_item->parent &&
                selected_item->parent->on_child_selected
            ) {
                selected_item->parent->on_child_selected(
                    selected_item
                );
            }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_MENU_OK: {
        if(
            is_down &&
            selected_item &&
            selected_item->on_activate &&
            selected_item->is_responsive()
        ) {
            selected_item->activate(point(LARGE_FLOAT, LARGE_FLOAT));
            auto_repeat_on = true;
            auto_repeat_duration = 0.0f;
            auto_repeat_next_activation = GUI::AUTO_REPEAT_MAX_INTERVAL;
        } else if(!is_down) {
            auto_repeat_on = false;
        }
        break;
        
    } case PLAYER_ACTION_TYPE_MENU_BACK: {
        if(is_down && back_item && back_item->is_responsive()) {
            back_item->activate(point(LARGE_FLOAT, LARGE_FLOAT));
        }
        break;
        
    } default: {
        button_recognized = false;
        break;
        
    }
    }
    
    if(button_recognized) {
        last_input_was_mouse = false;
    }
    return button_recognized;
}


/**
 * @brief Hides all items until an animation shows them again.
 */
void gui_manager::hide_items() {
    visible = false;
}


/**
 * @brief Reads item default centers and sizes from a data node.
 *
 * @param node Data node to read from.
 */
void gui_manager::read_coords(data_node* node) {
    size_t n_items = node->get_nr_of_children();
    for(size_t i = 0; i < n_items; ++i) {
        data_node* item_node = node->get_child(i);
        vector<string> words = split(item_node->value);
        if(words.size() < 4) {
            continue;
        }
        register_coords(
            item_node->name,
            s2f(words[0]), s2f(words[1]), s2f(words[2]), s2f(words[3])
        );
    }
}


/**
 * @brief Registers an item's default center and size.
 *
 * @param id String ID of the item.
 * @param cx Center X, in screen percentage.
 * @param cy Center Y, in screen percentage.
 * @param w Width, in screen percentage.
 * @param h Height, in screen percentage.
 */
void gui_manager::register_coords(
    const string &id,
    const float cx, const float cy, const float w, const float h
) {
    registered_centers[id] =
        point(cx / 100.0f, cy / 100.0f);
    registered_sizes[id] =
        point(w / 100.0f, h / 100.0f);
}


/**
 * @brief Removes an item from the list.
 *
 * @param item Item to remove.
 */
void gui_manager::remove_item(gui_item* item) {
    if(selected_item == item) {
        set_selected_item(nullptr);
    }
    if(back_item == item) {
        back_item = nullptr;
    }
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i] == item) {
            items.erase(items.begin() + i);
        }
    }
    item->manager = nullptr;
}


/**
 * @brief Sets the given item as the one that is selected, or none.
 *
 * @param item Item to select, or nullptr for none.
 * @param silent If true, no sound effect will play.
 * Useful if you want the item to be selected not because of user input,
 * but because it's the default selected item when the GUI loads.
 */
void gui_manager::set_selected_item(gui_item* item, bool silent) {
    if(selected_item == item) {
        return;
    }
    
    auto_repeat_on = false;
    
    if(selected_item) {
        selected_item->selected = false;
    }
    selected_item = item;
    if(selected_item) {
        selected_item->selected = true;
    }
    
    if(on_selection_changed) on_selection_changed();
    if(selected_item) {
        if(selected_item->on_selected) {
            selected_item->on_selected();
        }
    }
    
    if(selected_item && !silent) {
        sfx_source_config_t select_sfx_config;
        select_sfx_config.gain = 0.5f;
        select_sfx_config.speed_deviation = 0.1f;
        select_sfx_config.stack_min_pos = 0.01f;
        game.audio.create_ui_sfx_source(
            game.sys_assets.sfx_menu_select,
            select_sfx_config
        );
    }
}


/**
 * @brief Shows all items, if they were hidden.
 */
void gui_manager::show_items() {
    visible = true;
}


/**
 * @brief Starts an animation that affects all items.
 *
 * @param type Type of aniimation to start.
 * @param duration Total duration of the animation.
 */
void gui_manager::start_animation(
    const GUI_MANAGER_ANIM type, const float duration
) {
    anim_type = type;
    anim_timer.start(duration);
    visible = true;
}


/**
 * @brief Ticks the time of all items by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void gui_manager::tick(const float delta_t) {
    //Tick the animation.
    anim_timer.tick(delta_t);
    
    //Tick all items.
    for(size_t i = 0; i < items.size(); ++i) {
        gui_item* i_ptr = items[i];
        if(i_ptr->on_tick) {
            i_ptr->on_tick(delta_t);
        }
        if(i_ptr->juice_timer > 0) {
            i_ptr->juice_timer =
                std::max(0.0f, i_ptr->juice_timer - delta_t);
        } else {
            i_ptr->juice_type = gui_item::JUICE_TYPE_NONE;
        }
    }
    
    //Auto-repeat activations of the selected item, if applicable.
    if(
        auto_repeat_on &&
        selected_item &&
        selected_item->can_auto_repeat &&
        selected_item->on_activate
    ) {
        auto_repeat_duration += delta_t;
        auto_repeat_next_activation -= delta_t;
        
        while(auto_repeat_next_activation <= 0.0f) {
            selected_item->activate(point(LARGE_FLOAT, LARGE_FLOAT));
            auto_repeat_next_activation +=
                clamp(
                    interpolate_number(
                        auto_repeat_duration,
                        0,
                        GUI::AUTO_REPEAT_RAMP_TIME,
                        GUI::AUTO_REPEAT_MAX_INTERVAL,
                        GUI::AUTO_REPEAT_MIN_INTERVAL
                    ),
                    GUI::AUTO_REPEAT_MIN_INTERVAL,
                    GUI::AUTO_REPEAT_MAX_INTERVAL
                );
        }
    }
}


/**
 * @brief Returns whether the last input was a mouse input.
 *
 * @return Whether it was a mouse input.
 */
bool gui_manager::was_last_input_mouse() {
    return last_input_was_mouse;
}


/**
 * @brief Constructs a new list gui item object.
 */
list_gui_item::list_gui_item() :
    gui_item() {
    
    padding = 8.0f;
    on_draw =
    [this] (const point & center, const point & size) {
        draw_rounded_rectangle(
            center, size, 8.0f, COLOR_TRANSPARENT_WHITE, 1.0f
        );
        if(offset > 0.0f) {
            //Shade effect at the top.
            ALLEGRO_VERTEX vertexes[8];
            for(size_t v = 0; v < 8; ++v) {
                vertexes[v].z = 0.0f;
            }
            float y1 = center.y - size.y / 2.0f;
            float y2 = y1 + 20.0f;
            ALLEGRO_COLOR c_opaque = al_map_rgba(255, 255, 255, 64);
            ALLEGRO_COLOR c_empty = al_map_rgba(255, 255, 255, 0);
            vertexes[0].x = center.x - size.x * 0.49;
            vertexes[0].y = y1;
            vertexes[0].color = c_empty;
            vertexes[1].x = center.x - size.x * 0.49;
            vertexes[1].y = y2;
            vertexes[1].color = c_empty;
            vertexes[2].x = center.x - size.x * 0.47;
            vertexes[2].y = y1;
            vertexes[2].color = c_opaque;
            vertexes[3].x = center.x - size.x * 0.47;
            vertexes[3].y = y2;
            vertexes[3].color = c_empty;
            vertexes[4].x = center.x + size.x * 0.47;
            vertexes[4].y = y1;
            vertexes[4].color = c_opaque;
            vertexes[5].x = center.x + size.x * 0.47;
            vertexes[5].y = y2;
            vertexes[5].color = c_empty;
            vertexes[6].x = center.x + size.x * 0.49;
            vertexes[6].y = y1;
            vertexes[6].color = c_empty;
            vertexes[7].x = center.x + size.x * 0.49;
            vertexes[7].y = y2;
            vertexes[7].color = c_empty;
            al_draw_prim(
                vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
        }
        float child_bottom = get_child_bottom();
        if(child_bottom > 1.0f && offset < child_bottom - 1.0f) {
            //Shade effect at the bottom.
            ALLEGRO_VERTEX vertexes[8];
            for(size_t v = 0; v < 8; ++v) {
                vertexes[v].z = 0.0f;
            }
            float y1 = center.y + size.y / 2.0f;
            float y2 = y1 - 20.0f;
            ALLEGRO_COLOR c_opaque = al_map_rgba(255, 255, 255, 64);
            ALLEGRO_COLOR c_empty = al_map_rgba(255, 255, 255, 0);
            vertexes[0].x = center.x - size.x * 0.49;
            vertexes[0].y = y1;
            vertexes[0].color = c_empty;
            vertexes[1].x = center.x - size.x * 0.49;
            vertexes[1].y = y2;
            vertexes[1].color = c_empty;
            vertexes[2].x = center.x - size.x * 0.47;
            vertexes[2].y = y1;
            vertexes[2].color = c_opaque;
            vertexes[3].x = center.x - size.x * 0.47;
            vertexes[3].y = y2;
            vertexes[3].color = c_empty;
            vertexes[4].x = center.x + size.x * 0.47;
            vertexes[4].y = y1;
            vertexes[4].color = c_opaque;
            vertexes[5].x = center.x + size.x * 0.47;
            vertexes[5].y = y2;
            vertexes[5].color = c_empty;
            vertexes[6].x = center.x + size.x * 0.49;
            vertexes[6].y = y1;
            vertexes[6].color = c_empty;
            vertexes[7].x = center.x + size.x * 0.49;
            vertexes[7].y = y2;
            vertexes[7].color = c_empty;
            al_draw_prim(
                vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
        }
    };
    on_tick =
    [this] (const float delta_t) {
        float child_bottom = get_child_bottom();
        if(child_bottom < 1.0f) {
            target_offset = 0.0f;
            offset = 0.0f;
        } else {
            target_offset = clamp(target_offset, 0.0f, child_bottom - 1.0f);
            offset += (target_offset - offset) * (10.0f * delta_t);
            offset = clamp(offset, 0.0f, child_bottom - 1.0f);
            if(offset <= 0.01f) offset = 0.0f;
            if(child_bottom > 1.0f) {
                if(child_bottom - offset - 1.0f <= 0.01f) {
                    offset = child_bottom - 1.0f;
                }
            }
        }
    };
    on_event =
    [this] (const ALLEGRO_EVENT & ev) {
        if(
            ev.type == ALLEGRO_EVENT_MOUSE_AXES &&
            is_mouse_on(point(ev.mouse.x, ev.mouse.y)) &&
            ev.mouse.dz != 0.0f
        ) {
            float child_bottom = get_child_bottom();
            if(child_bottom <= 1.0f && offset == 0.0f) {
                return;
            }
            target_offset =
                clamp(
                    target_offset + (-ev.mouse.dz) * 0.2f,
                    0.0f,
                    child_bottom - 1.0f
                );
        }
    };
    on_child_selected =
    [this] (const gui_item * child) {
        //Try to center the child.
        float child_bottom = get_child_bottom();
        if(child_bottom <= 1.0f && offset == 0.0f) {
            return;
        }
        target_offset =
            clamp(
                child->center.y - 0.5f,
                0.0f,
                child_bottom - 1.0f
            );
    };
}


/**
 * @brief Constructs a new picker gui item object.
 *
 * @param base_text Text to display before the current option's name.
 * @param option Text that matches the current option.
 * @param nr_options Total amount of options.
 * @param cur_option_idx Index of the currently selected option.
 */
picker_gui_item::picker_gui_item(
    const string &base_text, const string &option,
    const size_t nr_options, const size_t cur_option_idx
) :
    gui_item(true),
    base_text(base_text),
    option(option),
    nr_options(nr_options),
    cur_option_idx(cur_option_idx) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        if(this->nr_options != 0 && selected) {
            point option_boxes_start(
                center.x - size.x / 2.0f + 20.0f,
                center.y + size.y / 2.0f - 12.0f
            );
            float option_boxes_interval =
                (size.x - 40.0f) / (this->nr_options - 0.5f);
            for(size_t o = 0; o < this->nr_options; ++o) {
                float x1 = option_boxes_start.x + o * option_boxes_interval;
                float y1 = option_boxes_start.y;
                al_draw_filled_rectangle(
                    x1, y1,
                    x1 + option_boxes_interval * 0.5f, y1 + 4.0f,
                    this->cur_option_idx == o ?
                    al_map_rgba(255, 255, 255, 160) :
                    al_map_rgba(255, 255, 255, 64)
                );
            }
        }
        
        unsigned char real_arrow_highlight = 255;
        if(
            selected &&
            manager &&
            manager->was_last_input_mouse()
        ) {
            real_arrow_highlight = arrow_highlight;
        }
        ALLEGRO_COLOR arrow_highlight_color = al_map_rgb(87, 200, 208);
        ALLEGRO_COLOR arrow_regular_color = COLOR_WHITE;
        point arrow_highlight_size = point(1.4f, 1.4f);
        point arrow_regular_size = point(1.0f, 1.0f);
        
        draw_compressed_scaled_text(
            game.sys_assets.fnt_standard,
            real_arrow_highlight == 0 ?
            arrow_highlight_color :
            arrow_regular_color,
            point(center.x - size.x * 0.45, center.y),
            real_arrow_highlight == 0 ?
            arrow_highlight_size :
            arrow_regular_size,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
            size,
            false,
            "<"
        );
        draw_compressed_scaled_text(
            game.sys_assets.fnt_standard,
            real_arrow_highlight == 1 ?
            arrow_highlight_color :
            arrow_regular_color,
            point(center.x + size.x * 0.45, center.y),
            real_arrow_highlight == 1 ?
            arrow_highlight_size :
            arrow_regular_size,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
            size,
            false,
            ">"
        );
        
        float juicy_grow_amount = this->get_juice_value();
        
        draw_compressed_scaled_text(
            game.sys_assets.fnt_standard, COLOR_WHITE,
            point(center.x - size.x * 0.40, center.y),
            point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
            point(size.x * 0.80, size.y),
            true,
            this->base_text + this->option
        );
        
        ALLEGRO_COLOR box_tint =
            selected ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
            
        draw_textured_box(
            center, size, game.sys_assets.bmp_bubble_box, box_tint
        );
        
        if(selected) {
            draw_textured_box(
                center,
                size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
                game.sys_assets.bmp_focus_box
            );
        }
    };
    
    on_activate =
    [this] (const point & cursor_pos) {
        if(cursor_pos.x >= get_reference_center().x) {
            on_next();
        } else {
            on_previous();
        }
    };
    
    on_menu_dir_button =
    [this] (const size_t button_id) -> bool{
        if(button_id == PLAYER_ACTION_TYPE_MENU_RIGHT) {
            on_next();
            return true;
        } else if(button_id == PLAYER_ACTION_TYPE_MENU_LEFT) {
            on_previous();
            return true;
        }
        return false;
    };
    
    on_mouse_over =
    [this] (const point & cursor_pos) {
        arrow_highlight =
            cursor_pos.x >= get_reference_center().x ? 1 : 0;
    };
}


/**
 * @brief Constructs a new scroll gui item object.
 */
scroll_gui_item::scroll_gui_item() :
    gui_item() {
    
    on_draw =
    [this] (const point & center, const point & size) {
        float bar_y = 0.0f; //Top, in height ratio.
        float bar_h = 0.0f; //In height ratio.
        float list_bottom = list_item->get_child_bottom();
        unsigned char alpha = 48;
        if(list_bottom > 1.0f) {
            float offset = std::min(list_item->offset, list_bottom - 1.0f);
            bar_y = offset / list_bottom;
            bar_h = 1.0f / list_bottom;
            alpha = 128;
        }
        
        draw_rounded_rectangle(
            center, size, 8.0f, al_map_rgba(255, 255, 255, alpha), 1.0f
        );
        
        if(bar_h != 0.0f) {
            draw_textured_box(
                point(
                    center.x,
                    (center.y - size.y * 0.5) +
                    (size.y * bar_y) +
                    (size.y * bar_h * 0.5f)
                ),
                point(size.x, (size.y * bar_h)),
                game.sys_assets.bmp_bubble_box
            );
        }
    };
    on_event =
    [this] (const ALLEGRO_EVENT & ev) {
        if(
            ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            ev.mouse.button == 1 &&
            is_mouse_on(point(ev.mouse.x, ev.mouse.y))
        ) {
            float list_bottom = list_item->get_child_bottom();
            if(list_bottom <= 1.0f) {
                return;
            }
            
            point c = get_reference_center();
            point s = get_reference_size();
            float bar_h = (1.0f / list_bottom) * s.y;
            float y1 = (c.y - s.y / 2.0f) + bar_h / 2.0f;
            float y2 = (c.y + s.y / 2.0f) - bar_h / 2.0f;
            float click = (ev.mouse.y - y1) / (y2 - y1);
            click = clamp(click, 0.0f, 1.0f);
            
            list_item->target_offset = click * (list_bottom - 1.0f);
        }
    };
}


/**
 * @brief Constructs a new text gui item object.
 *
 * @param text Text to display.
 * @param font Font to use for the text.
 * @param color Color to use for the text.
 * @param flags Allegro text flags to use.
 */
text_gui_item::text_gui_item(
    const string &text, ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    const int flags
) :
    gui_item(),
    text(text),
    font(font),
    color(color),
    flags(flags) {
    
    on_draw =
    [this] (const point & center, const point & size) {
    
        int text_x = center.x;
        switch(this->flags) {
        case ALLEGRO_ALIGN_LEFT: {
            text_x = center.x - size.x * 0.5;
            break;
        } case ALLEGRO_ALIGN_RIGHT: {
            text_x = center.x + size.x * 0.5;
            break;
        }
        }
        
        float juicy_grow_amount = get_juice_value();
        int text_y = center.y;
        
        if(line_wrap) {
        
            text_y = center.y - size.y / 2.0f;
            int line_height = al_get_font_line_height(this->font);
            vector<string_token> tokens =
                tokenize_string(this->text);
            set_string_token_widths(
                tokens, this->font, game.sys_assets.fnt_slim, line_height, false
            );
            vector<vector<string_token> > tokens_per_line =
                split_long_string_with_tokens(tokens, size.x);
                
            for(size_t l = 0; l < tokens_per_line.size(); ++l) {
                draw_string_tokens(
                    tokens_per_line[l], this->font, game.sys_assets.fnt_slim,
                    false,
                    point(
                        text_x,
                        text_y + l * line_height
                    ),
                    this->flags,
                    point(size.x, line_height),
                    point(1.0f + juicy_grow_amount, 1.0f + juicy_grow_amount)
                );
            }
            
        } else {
        
            draw_compressed_scaled_text(
                this->font, this->color,
                point(text_x, text_y),
                point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
                this->flags, V_ALIGN_MODE_CENTER, size, true,
                this->text
            );
            
        }
        
        if(selected && show_selection_box) {
            draw_textured_box(
                center,
                size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
                game.sys_assets.bmp_focus_box
            );
        }
    };
}


/**
 * @brief Constructs a new tooltip gui item object.
 *
 * @param gui Pointer to the GUI it belongs to.
 */
tooltip_gui_item::tooltip_gui_item(gui_manager* gui) :
    gui_item(),
    gui(gui) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        string cur_text = this->gui->get_current_tooltip();
        if(cur_text != this->prev_text) {
            this->start_juice_animation(JUICE_TYPE_GROW_TEXT_LOW);
            this->prev_text = cur_text;
        }
        float juicy_grow_amount = get_juice_value();
        draw_compressed_scaled_text(
            game.sys_assets.fnt_standard, COLOR_WHITE,
            center,
            point(0.7f + juicy_grow_amount, 0.7f + juicy_grow_amount),
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, size,
            false,
            cur_text
        );
    };
}
