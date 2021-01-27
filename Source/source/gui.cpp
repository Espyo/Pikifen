/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General GUI manager and GUI item classes.
 */

#include <algorithm>

#include "gui.h"

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "utils/string_utils.h"


//When an item does a "juicy grow", change the size by this much.
const float gui_item::JUICY_GROW_DELTA = 0.05f;
//When an item does a "juicy grow", this is the full effect duration.
const float gui_item::JUICY_GROW_DURATION = 0.3f;
//Interval between auto-repeat activations, at the slowest speed.
const float gui_manager::AUTO_REPEAT_MAX_INTERVAL = 0.3f;
//Interval between auto-repeat activations, at the fastest speed.
const float gui_manager::AUTO_REPEAT_MIN_INTERVAL = 0.011f;
//How long it takes for the auto-repeat activations to reach max speed.
const float gui_manager::AUTO_REPEAT_RAMP_TIME = 0.9f;


/* ----------------------------------------------------------------------------
 * Creates a new button GUI item.
 */
button_gui_item::button_gui_item(
    const string &text, ALLEGRO_FONT* font, const ALLEGRO_COLOR &color
) :
    gui_item(true),
    text(text),
    font(font),
    color(color) {
    
    on_draw =
    [this, text, font, color] (const point & center, const point & size) {
        draw_button(
            center, size, text, font, color, selected, get_juicy_grow_amount()
        );
    };
}


/* ----------------------------------------------------------------------------
 * Creates a new checkbox GUI item.
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
        [this, text, font, value, color]
    (const point & center, const point & size) {
        draw_compressed_text(
            font, color,
            point(center.x - size.x * 0.45, center.y),
            ALLEGRO_ALIGN_LEFT, 1,
            point(size.x * 0.90, size.y),
            text
        );
        
        draw_bitmap(
            game.sys_assets.bmp_checkbox_check,
            point((center.x + size.x * 0.5) - 40, center.y),
            point(32, -1),
            0,
            (*value) ? map_gray(255) : al_map_rgba(32, 32, 32, 80)
        );
        
        ALLEGRO_COLOR box_tint =
            selected ? al_map_rgb(87, 200, 208) : map_gray(255);
            
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
    [this, value] (const point &) {
        (*value) = !(*value);
    };
}


/* ----------------------------------------------------------------------------
 * Creates a new GUI item.
 */
gui_item::gui_item(const bool selectable) :
    visible(true),
    selectable(selectable),
    selected(false),
    parent(nullptr),
    offset(0.0f),
    padding(0.0f),
    can_auto_repeat(false),
    juice_timer(0.0f),
    on_draw(nullptr),
    on_tick(nullptr),
    on_event(nullptr),
    on_activate(nullptr),
    on_menu_dir_button(nullptr),
    on_child_selected(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a child item.
 */
void gui_item::add_child(gui_item* item) {
    children.push_back(item);
    item->parent = this;
}


/* ----------------------------------------------------------------------------
 * Returns the bottommost Y coordinate, in height ratio,
 * of the item's children items.
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


/* ----------------------------------------------------------------------------
 * Returns the juicy grow amount for the current juicy grow animation, if any.
 */
float gui_item::get_juicy_grow_amount() {
    if(juice_timer == 0.0f) {
        return 0.0f;
    }
    
    return
        ease(EASE_UP_AND_DOWN, juice_timer / JUICY_GROW_DURATION) *
        JUICY_GROW_DELTA;
}


/* ----------------------------------------------------------------------------
 * Returns the real center coordinates.
 */
point gui_item::get_real_center() {
    if(parent) {
        point parent_s = parent->get_real_size() - (parent->padding * 2.0f);
        point parent_c = parent->get_real_center();
        point result = center * parent_s;
        result.x += parent_c.x - parent_s.x / 2.0f;
        result.y += parent_c.y - parent_s.y / 2.0f;
        result.y -= parent_s.y * parent->offset;
        return result;
    } else {
        return point(center.x * game.win_w, center.y * game.win_h);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the real size coordinates.
 */
point gui_item::get_real_size() {
    point mult;
    if(parent) {
        mult = parent->get_real_size() - (parent->padding * 2.0f);
    } else {
        mult.x = game.win_w;
        mult.y = game.win_h;
    }
    return size * mult;
}


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is on top of it.
 */
bool gui_item::is_mouse_on(const point &cursor_pos) {
    if(parent && !parent->is_mouse_on(cursor_pos)) {
        return false;
    }
    
    point c = get_real_center();
    point s = get_real_size();
    return
        (
            cursor_pos.x >= c.x - s.x * 0.5 &&
            cursor_pos.x <= c.x + s.x * 0.5 &&
            cursor_pos.y >= c.y - s.y * 0.5 &&
            cursor_pos.y <= c.y + s.y * 0.5
        );
}


/* ----------------------------------------------------------------------------
 * Removes an item from the list of children.
 */
void gui_item::remove_child(gui_item* item) {
    for(size_t c = 0; c < children.size(); ++c) {
        if(children[c] == item) {
            children.erase(children.begin() + c);
        }
    }
    
    item->parent = NULL;
}


/* ----------------------------------------------------------------------------
 * Starts the process of animating a juicy grow effect.
 */
void gui_item::start_juicy_grow() {
    juice_timer = JUICY_GROW_DURATION;
}


/* ----------------------------------------------------------------------------
 * Creates a new GUI manager.
 */
gui_manager::gui_manager() :
    selected_item(nullptr),
    back_item(nullptr),
    right_pressed(false),
    up_pressed(false),
    left_pressed(false),
    down_pressed(false),
    ok_pressed(false),
    back_pressed(false),
    auto_repeat_on(false),
    auto_repeat_duration(0.0f),
    auto_repeat_next_activation(0.0f),
    anim_type(GUI_MANAGER_ANIM_NONE) {
    
}


/* ----------------------------------------------------------------------------
 * Add an item to the list.
 * item:
 *   Pointer to the new item.
 * id:
 *   If this item has an associated ID, specify it here. Empty string if none.
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
}


/* ----------------------------------------------------------------------------
 * Destroys all allocated items and information.
 */
void gui_manager::destroy() {
    set_selected_item(NULL);
    back_item = NULL;
    for(size_t i = 0; i < items.size(); i++) {
        delete items[i];
    }
    items.clear();
    registered_centers.clear();
    registered_sizes.clear();
}


/* ----------------------------------------------------------------------------
 * Draws all items on-screen.
 */
void gui_manager::draw() {
    int ocr_x, ocr_y, ocr_w, ocr_h;
    for(size_t i = 0; i < items.size(); ++i) {
        gui_item* i_ptr = items[i];
        
        if(!i_ptr->visible) continue;
        if(i_ptr->size.x == 0.0f) continue;
        
        point center = i_ptr->center;
        point size = i_ptr->size;
        point multipliers;
        gui_item* parent = i_ptr->parent;
        
        if(parent) {
            al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
            point parent_c = parent->get_real_center();
            point parent_s = parent->get_real_size();
            al_set_clipping_rectangle(
                parent_c.x - parent_s.x / 2.0f,
                parent_c.y - parent_s.y / 2.0f,
                parent_s.x,
                parent_s.y
            );
        }
        
        point final_center = i_ptr->get_real_center();
        point final_size = i_ptr->get_real_size();
        
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
                        ease(EASE_OUT, 1.0f - anim_timer.get_ratio_left()),
                        0.0f, 1.0f, start_center.x, final_center.x
                    );
                final_center.y =
                    interpolate_number(
                        ease(EASE_OUT, 1.0f - anim_timer.get_ratio_left()),
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
                        ease(EASE_IN, 1.0f - anim_timer.get_ratio_left()),
                        0.0f, 1.0f, final_center.x, end_center.x
                    );
                final_center.y =
                    interpolate_number(
                        ease(EASE_IN, 1.0f - anim_timer.get_ratio_left()),
                        0.0f, 1.0f, final_center.y, end_center.y
                    );
                break;
                
            } default: {
                break;
                
            }
            }
        }
        
        i_ptr->on_draw(final_center, final_size);
        
        if(parent) {
            al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handle an Allegro event.
 * ev:
 *   Event.
 */
void gui_manager::handle_event(const ALLEGRO_EVENT &ev) {
    //Mousing over an item and clicking.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        gui_item* selection_result = NULL;
        for(size_t i = 0; i < items.size(); ++i) {
            gui_item* i_ptr = items[i];
            if(
                i_ptr->is_mouse_on(point(ev.mouse.x, ev.mouse.y)) &&
                i_ptr->selectable
            ) {
                selection_result = i_ptr;
                break;
            }
        }
        set_selected_item(selection_result);
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
        if(selected_item && selected_item->on_activate) {
            selected_item->on_activate(point(ev.mouse.x, ev.mouse.y));
            auto_repeat_on = true;
            auto_repeat_duration = 0.0f;
            auto_repeat_next_activation = AUTO_REPEAT_MAX_INTERVAL;
        }
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1) {
        auto_repeat_on = false;
    }
    
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_menu_button(
            actions[a].button, actions[a].pos, actions[a].player
        );
    }
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i]->on_event) {
            items[i]->on_event(ev);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a button "press" in a GUI. Technically, it could also be
 * a button release.
 * action:
 *   The button's ID. Use BUTTON_*.
 * pos:
 *   The position of the button, i.e., how much it's "held".
 *   0 means it was released. 1 means it was fully pressed.
 *   For controls with more sensitivity, values between 0 and 1 are important.
 *   Like a 0.5 for swarming makes the group swarm at half distance.
 * player:
 *   Number of the player that pressed.
 */
void gui_manager::handle_menu_button(
    const size_t action, const float pos, const size_t player
) {

    bool is_down = (pos >= 0.5);
    
    switch(action) {
    case BUTTON_MENU_RIGHT:
    case BUTTON_MENU_UP:
    case BUTTON_MENU_LEFT:
    case BUTTON_MENU_DOWN: {

        //Selecting a different item with the arrow keys.
        size_t pressed = BUTTON_NONE;
        
        switch(action) {
        case BUTTON_MENU_RIGHT: {
            if(!right_pressed && is_down) {
                pressed = BUTTON_MENU_RIGHT;
            }
            right_pressed = is_down;
            break;
        } case BUTTON_MENU_UP: {
            if(!up_pressed && is_down) {
                pressed = BUTTON_MENU_UP;
            }
            up_pressed = is_down;
            break;
        } case BUTTON_MENU_LEFT: {
            if(!left_pressed && is_down) {
                pressed = BUTTON_MENU_LEFT;
            }
            left_pressed = is_down;
            break;
        } case BUTTON_MENU_DOWN: {
            if(!down_pressed && is_down) {
                pressed = BUTTON_MENU_DOWN;
            }
            down_pressed = is_down;
            break;
        }
        }
        
        if(pressed == BUTTON_NONE) return;
        
        if(!selected_item) {
            for(size_t i = 0; i < items.size(); ++i) {
                if(items[i]->selectable) {
                    set_selected_item(items[i]);
                    return;
                }
            }
        }
        if(!selected_item) {
            //No item can be selected.
            return;
        }
        
        vector<point> selectables;
        vector<gui_item*> selectable_ptrs;
        size_t selectable_idx = INVALID;
        float direction = 0.0f;
        
        switch(pressed) {
        case BUTTON_MENU_DOWN: {
            direction = TAU * 0.25f;
            break;
        }
        case BUTTON_MENU_LEFT: {
            direction = TAU * 0.50f;
            break;
        }
        case BUTTON_MENU_UP: {
            direction = TAU * 0.75f;
            break;
        }
        }
        
        if(selected_item && selected_item->on_menu_dir_button) {
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
            if(i_ptr->selectable) {
                point i_center = i_ptr->get_real_center();
                if(i_ptr == selected_item) {
                    selectable_idx = selectables.size();
                }
                
                min_y = std::min(min_y, i_center.y);
                max_y = std::max(max_y, i_center.y);
                
                selectable_ptrs.push_back(i_ptr);
                selectables.push_back(i_ptr->get_real_center());
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
        
    } case BUTTON_MENU_OK: {
        if(is_down && selected_item) {
            selected_item->on_activate(point(LARGE_FLOAT, LARGE_FLOAT));
            auto_repeat_on = true;
            auto_repeat_duration = 0.0f;
            auto_repeat_next_activation = AUTO_REPEAT_MAX_INTERVAL;
        } else if(!is_down) {
            auto_repeat_on = false;
        }
        break;
        
    } case BUTTON_MENU_BACK: {
        if(is_down && back_item) {
            back_item->on_activate(point(LARGE_FLOAT, LARGE_FLOAT));
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Reads item default centers and sizes from a data node.
 * node:
 *   Data node to read from.
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


/* ----------------------------------------------------------------------------
 * Registers an item's default center and size.
 * id:
 *   String ID of the item.
 * cx:
 *   Center X, in screen percentage.
 * cy:
 *   Center Y, in screen percentage.
 * w:
 *   Width, in screen percentage.
 * h:
 *   Height, in screen percentage.
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


/* ----------------------------------------------------------------------------
 * Removes an item from the list.
 * item:
 *   Item to remove.
 */
void gui_manager::remove_item(gui_item* item) {
    if(selected_item == item) {
        set_selected_item(NULL);
    }
    if(back_item == item) {
        back_item = NULL;
    }
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i] == item) {
            items.erase(items.begin() + i);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Sets the given item as the one that is selected, or none.
 * item:
 *   Item to select, or NULL for none.
 */
void gui_manager::set_selected_item(gui_item* item) {
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
}


/* ----------------------------------------------------------------------------
 * Starts an animation that affects all items.
 */
void gui_manager::start_animation(
    const GUI_MANAGER_ANIMS type, const float duration
) {
    anim_type = type;
    anim_timer.start(duration);
}


/* ----------------------------------------------------------------------------
 * Ticks all items on-screen by one frame of logic.
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
            selected_item->on_activate(point(LARGE_FLOAT, LARGE_FLOAT));
            auto_repeat_next_activation +=
                clamp(
                    interpolate_number(
                        auto_repeat_duration,
                        0, AUTO_REPEAT_RAMP_TIME,
                        AUTO_REPEAT_MAX_INTERVAL, AUTO_REPEAT_MIN_INTERVAL
                    ),
                    AUTO_REPEAT_MIN_INTERVAL,
                    AUTO_REPEAT_MAX_INTERVAL
                );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new list GUI item.
 */
list_gui_item::list_gui_item() :
    gui_item(),
    scroll_item(nullptr),
    target_offset(0.0f) {
    
    padding = 8.0f;
    on_draw =
    [this] (const point & center, const point & size) {
        draw_rounded_rectangle(
            center, size, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
        );
    };
    on_tick =
    [this] (const float delta_t) {
        offset += (target_offset - offset) * (10.0f * delta_t);
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


/* ----------------------------------------------------------------------------
 * Creates a new picker GUI item.
 */
picker_gui_item::picker_gui_item(
    const string &base_text, const string &option
) :
    gui_item(true),
    base_text(base_text),
    option(option),
    on_previous(nullptr),
    on_next(nullptr) {
    
    on_draw =
    [this] (const point & center, const point & size) {
        draw_text_lines(
            game.fonts.standard, map_gray(255),
            point(center.x - size.x * 0.45, center.y),
            ALLEGRO_ALIGN_CENTER, 1,
            "<"
        );
        draw_text_lines(
            game.fonts.standard, map_gray(255),
            point(center.x + size.x * 0.45, center.y),
            ALLEGRO_ALIGN_CENTER, 1,
            ">"
        );
        
        float juicy_grow_amount = this->get_juicy_grow_amount();
        
        draw_compressed_scaled_text(
            game.fonts.standard, map_gray(255),
            point(center.x - size.x * 0.40, center.y),
            point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
            ALLEGRO_ALIGN_LEFT, 1,
            point(size.x * 0.80, size.y),
            this->base_text + this->option
        );
        
        ALLEGRO_COLOR box_tint =
            selected ? al_map_rgb(87, 200, 208) : map_gray(255);
            
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
        if(cursor_pos.x >= get_real_center().x) {
            on_next();
        } else {
            on_previous();
        }
    };
    
    on_menu_dir_button =
    [this] (const size_t button_id) -> bool{
        if(button_id == BUTTON_MENU_RIGHT) {
            on_next();
            return true;
        } else if(button_id == BUTTON_MENU_LEFT) {
            on_previous();
            return true;
        }
        return false;
    };
}


/* ----------------------------------------------------------------------------
 * Creates a new scrollbar GUI item.
 */
scroll_gui_item::scroll_gui_item() :
    gui_item(),
    list_item(nullptr) {
    
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
            
            point c = get_real_center();
            point s = get_real_size();
            float bar_h = (1.0f / list_bottom) * s.y;
            float y1 = (c.y - s.y / 2.0f) + bar_h / 2.0f;
            float y2 = (c.y + s.y / 2.0f) - bar_h / 2.0f;
            float click = (ev.mouse.y - y1) / (y2 - y1);
            click = clamp(click, 0.0f, 1.0f);
            
            list_item->target_offset = click * (list_bottom - 1.0f);
        }
    };
}


/* ----------------------------------------------------------------------------
 * Creates a new text GUI item.
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
        [this, text, font, color, flags]
    (const point & center, const point & size) {
    
        int text_x = center.x;
        switch(flags) {
        case ALLEGRO_ALIGN_LEFT: {
            text_x = center.x - size.x * 0.5;
            break;
        } case ALLEGRO_ALIGN_RIGHT: {
            text_x = center.x + size.x * 0.5;
            break;
        }
        }
        
        float juicy_grow_amount = get_juicy_grow_amount();
        
        draw_compressed_scaled_text(
            font, color,
            point(text_x, center.y),
            point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
            flags, 1, size,
            text
        );
    };
}
