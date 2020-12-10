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

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new button GUI item.
 */
button_gui_item::button_gui_item(const string &text, ALLEGRO_FONT* font) :
    gui_item(true),
    text(text),
    font(font) {
    
    on_draw =
    [this, text, font] (const point & center, const point & size) {
        draw_button(
            center, size, text, font,
            map_gray(255), selected
        );
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
    juicy_timer(0.0f),
    on_draw(nullptr),
    on_tick(nullptr),
    on_event(nullptr),
    on_activate(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a child item.
 */
void gui_item::add_child(gui_item* item) {
    children.push_back(item);
    item->parent = this;
}


/* ----------------------------------------------------------------------------
 * Returns the bottommost Y coordinate of the item's children items.
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
    int ocr_x, ocr_y, ocr_w, ocr_h;
    for(size_t i = 0; i < items.size(); ++i) {
        point center = items[i]->center;
        point size = items[i]->size;
        point multipliers;
        gui_item* parent = items[i]->parent;
        
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
        
        items[i]->on_draw(
            items[i]->get_real_center(),
            items[i]->get_real_size()
        );
        
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
        al_draw_rounded_rectangle(
            center.x - size.x * 0.5,
            center.y - size.y * 0.5,
            center.x + size.x * 0.5,
            center.y + size.y * 0.5,
            8.0f, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
        );
    };
    on_tick =
    [this] (const float delta_t) {
        offset += (target_offset - offset) * (10.0f * delta_t);
    };
    on_event =
    [this] (const ALLEGRO_EVENT & ev) {
        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dz != 0.0f) {
                float child_bottom = get_child_bottom();
                if(child_bottom <= 1.0f) {
                    return;
                }
                target_offset =
                    clamp(
                        target_offset + (-ev.mouse.dz) * 0.2f,
                        0.0f,
                        get_child_bottom() - 1.0f
                    );
            }
        }
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
            bar_y = list_item->offset / list_bottom;
            bar_h = 1.0f / list_bottom;
            alpha = 128;
        }
        
        al_draw_rounded_rectangle(
            center.x - size.x * 0.5,
            center.y - size.y * 0.5,
            center.x + size.x * 0.5,
            center.y + size.y * 0.5,
            8.0f, 8.0f, al_map_rgba(255, 255, 255, alpha), 1.0f
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
text_gui_item::text_gui_item(const string &text, ALLEGRO_FONT* font) :
    gui_item(),
    text(text),
    font(font) {
    
    on_draw =
    [this, text, font] (const point & center, const point & size) {
        draw_compressed_text(
            font, map_gray(255),
            center, ALLEGRO_ALIGN_CENTER, 1, size,
            text
        );
    };
}
