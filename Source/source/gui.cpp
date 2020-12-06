/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General GUI manager and GUI item classes.
 */

#include "gui.h"

#include "game.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new GUI item.
 */
gui_item::gui_item() :
    visible(true),
    selectable(false),
    selected(false),
    parent(nullptr),
    juicy_timer(0.0f),
    on_draw(nullptr),
    on_tick(nullptr),
    on_activate(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is on top of it.
 */
bool gui_item::is_mouse_on(const point &cursor_pos) {
    point c = cursor_pos;
    c.x /= game.win_w;
    c.y /= game.win_h;
    return
        (
            c.x >= center.x - size.x * 0.5 &&
            c.x <= center.x + size.x * 0.5 &&
            c.y >= center.y - size.y * 0.5 &&
            c.y <= center.y + size.y * 0.5
        );
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
    back_pressed(false) {
    
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
    for(size_t i = 0; i < items.size(); ++i) {
        point center = items[i]->center;
        center.x *= game.win_w;
        center.y *= game.win_h;
        point size = items[i]->size;
        size.x *= game.win_w;
        size.y *= game.win_h;
        items[i]->on_draw(center, size);
    }
}


/* ----------------------------------------------------------------------------
 * Handle an Allegro event.
 * ev:
 *   Event.
 */
void gui_manager::handle_event(const ALLEGRO_EVENT &ev) {
    //Mousing over a widget and clicking.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        set_selected_item(NULL);
        for(size_t i = 0; i < items.size(); ++i) {
            gui_item* i_ptr = items[i];
            if(
                i_ptr->is_mouse_on(point(ev.mouse.x, ev.mouse.y)) &&
                i_ptr->selectable
            ) {
                set_selected_item(i_ptr);
                break;
            }
        }
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
        if(selected_item) selected_item->on_activate();
    }
    
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_menu_button(
            actions[a].button, actions[a].pos, actions[a].player
        );
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

        //Selecting a different widget with the arrow keys.
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
                    break;
                }
            }
        }
        if(!selected_item) {
            //No item can be selected.
            return;
        }
        
        gui_item* closest_item = NULL;
        dist closest_item_dist;
        point cur_pivot;
        point i2_pivot;
        
        for(size_t i = 0; i < items.size(); i++) {
            gui_item* i_ptr = items[i];
            if(
                i_ptr == selected_item ||
                !i_ptr->selectable
            ) {
                continue;
            }
            
            switch(pressed) {
            case BUTTON_MENU_RIGHT: {
                cur_pivot.x =
                    selected_item->center.x + selected_item->size.x * 0.25;
                cur_pivot.y =
                    selected_item->center.y;
                i2_pivot.x = i_ptr->center.x - i_ptr->size.x * 0.25;
                i2_pivot.y = i_ptr->center.y;
                
                if(selected_item->center.x == i_ptr->center.x) continue;
                if(cur_pivot.x > i2_pivot.x) i2_pivot.x += game.win_w;
                break;
            } case BUTTON_MENU_UP: {
                cur_pivot.x =
                    selected_item->center.x;
                cur_pivot.y =
                    selected_item->center.y - selected_item->size.y * 0.25;
                i2_pivot.x = i_ptr->center.x;
                i2_pivot.y = i_ptr->center.y + i_ptr->size.y * 0.25;
                
                if(selected_item->center.y == i_ptr->center.y) continue;
                if(cur_pivot.y < i2_pivot.y) i2_pivot.y -= game.win_h;
                break;
            } case BUTTON_MENU_LEFT: {
                cur_pivot.x =
                    selected_item->center.x - selected_item->size.x * 0.25;
                cur_pivot.y =
                    selected_item->center.y;
                i2_pivot.x = i_ptr->center.x + i_ptr->size.x * 0.25;
                i2_pivot.y = i_ptr->center.y;
                
                if(selected_item->center.x == i_ptr->center.x) continue;
                if(cur_pivot.x < i2_pivot.x) i2_pivot.x -= game.win_w;
                break;
            } case BUTTON_MENU_DOWN: {
                cur_pivot.x =
                    selected_item->center.x;
                cur_pivot.y =
                    selected_item->center.y + selected_item->size.y * 0.25;
                i2_pivot.x = i_ptr->center.x;
                i2_pivot.y = i_ptr->center.y - i_ptr->size.y * 0.25;
                
                if(selected_item->center.y == i_ptr->center.y) continue;
                if(cur_pivot.y > i2_pivot.y) i2_pivot.y += game.win_h;
                break;
            }
            }
            
            dist d(cur_pivot, i2_pivot);
            
            if(!closest_item || d <= closest_item_dist) {
                closest_item = i_ptr;
                closest_item_dist = d;
            }
        }
        
        if(closest_item) {
            set_selected_item(closest_item);
        }
        
        break;
        
    } case BUTTON_MENU_OK: {
        if(is_down && selected_item) {
            selected_item->on_activate();
        }
        break;
        
    } case BUTTON_MENU_BACK: {
        if(is_down && back_item) {
            back_item->on_activate();
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
 * Sets the given item as the one that is selected, or none.
 * item:
 *   Item to select, or NULL for none.
 */
void gui_manager::set_selected_item(gui_item* item) {
    if(selected_item) {
        selected_item->selected = false;
    }
    selected_item = item;
    if(selected_item) {
        selected_item->selected = true;
    }
}


/* ----------------------------------------------------------------------------
 * Ticks all items on-screen by one frame of logic.
 */
void gui_manager::tick(const float delta_t) {
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i]->on_tick) {
            items[i]->on_tick(delta_t);
        }
    }
}
