/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include "editor.h"

#include "../game.h"


//Time until the next click is no longer considered a double-click.
const float editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How much to zoom in/out with the keyboard keys.
const float editor::KEYBOARD_CAM_ZOOM = 0.25f;


/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    canvas_separator_x(-1),
    double_click_time(0),
    is_ctrl_pressed(false),
    is_m1_pressed(false),
    is_m2_pressed(false),
    is_m3_pressed(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    loaded_content_yet(false),
    made_new_changes(false),
    mouse_drag_confirmed(false),
    state(0),
    sub_state(0),
    zoom_max_level(0),
    zoom_min_level(0) {
    
    
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void editor::center_camera(
    const point &min_coords, const point &max_coords
) {
    point min_c = min_coords;
    point max_c = max_coords;
    if(min_c == max_c) {
        min_c = min_c - 2.0;
        max_c = max_c + 2.0;
    }
    
    float width = max_c.x - min_c.x;
    float height = max_c.y - min_c.y;
    
    game.cam.pos.x = floor(min_c.x + width  / 2);
    game.cam.pos.y = floor(min_c.y + height / 2);
    
    float z;
    if(width > height) z = (canvas_br.x - canvas_tl.x) / width;
    else z = (canvas_br.y - canvas_tl.y) / height;
    z -= z * 0.1;
    
    zoom(z, false);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run after the editor's own logic code.
 */
void editor::do_logic_post() {
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run before the editor's own logic code.
 */
void editor::do_logic_pre() {
    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= game.delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event for control-related things.
 */
void editor::handle_controls(const ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        handle_mouse_update(ev);
    }
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui(game.mouse_cursor_s)
    ) {
    
        switch (ev.mouse.button) {
        case 1: {
            is_m1_pressed = true;
            break;
        } case 2: {
            is_m2_pressed = true;
            break;
        } case 3: {
            is_m3_pressed = true;
            break;
        }
        }
        
        mouse_drag_start = point(ev.mouse.x, ev.mouse.y);
        mouse_drag_confirmed = false;
        
        is_gui_focused = false;
        
        if(ev.mouse.button == last_mouse_click && double_click_time > 0) {
            switch(ev.mouse.button) {
            case 1: {
        
                handle_lmb_double_click(ev);
                break;
            } case 2: {
                handle_rmb_double_click(ev);
                break;
            } case 3: {
                handle_mmb_double_click(ev);
                break;
            }
            }
            
            double_click_time = 0;
            
        } else {
            switch(ev.mouse.button) {
            case 1: {
                handle_lmb_down(ev);
                break;
            } case 2: {
                handle_rmb_down(ev);
                break;
            } case 3: {
                handle_mmb_down(ev);
                break;
            }
            }
            
            last_mouse_click = ev.mouse.button;
            double_click_time = DOUBLE_CLICK_TIMEOUT;
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_gui(game.mouse_cursor_s)
    ) {
        is_gui_focused = true;
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        switch(ev.mouse.button) {
        case 1: {
            is_m1_pressed = false;
            handle_lmb_up(ev);
            break;
        } case 2: {
            is_m2_pressed = false;
            handle_rmb_up(ev);
            break;
        } case 3: {
            is_m3_pressed = false;
            handle_mmb_up(ev);
            break;
        }
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        if(
            fabs(ev.mouse.x - mouse_drag_start.x) >=
            game.options.editor_mouse_drag_threshold ||
            fabs(ev.mouse.y - mouse_drag_start.y) >=
            game.options.editor_mouse_drag_threshold
        ) {
            mouse_drag_confirmed = true;
        }
        
        if(mouse_drag_confirmed) {
            if(is_m1_pressed) {
                handle_lmb_drag(ev);
            }
            if(is_m2_pressed) {
                handle_rmb_drag(ev);
            }
            if(is_m3_pressed) {
                handle_mmb_drag(ev);
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            !is_mouse_in_gui(game.mouse_cursor_s)
        ) {
            handle_mouse_wheel(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = true;
            
        }
        
        handle_key_down_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_down_canvas(ev);
        }
        
        //TODO logic to go back when Escape is pressed.
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = false;
            
        }
        
        handle_key_up_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_up_canvas(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        handle_key_char_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_char_canvas(ev);
        }
        
    }
}


//Input handler functions.
void editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_up(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is inside the gui or not.
 * The status bar counts as the gui.
 */
bool editor::is_mouse_in_gui(const point &mouse_coords) const {
    return
        mouse_coords.x >= canvas_br.x || mouse_coords.y >= canvas_br.y ||
        mouse_coords.x <= canvas_tl.x || mouse_coords.y <= canvas_tl.y;
}


/* ----------------------------------------------------------------------------
 * Exits out of the editor, with a fade.
 */
void editor::leave() {
    game.fade_mgr.start_fade(false, [] () {
        if(game.states.area_editor_st->quick_play_area.empty()) {
            game.change_state(game.states.main_menu_st);
        } else {
            game.states.gameplay_st->area_to_load =
                game.states.area_editor_st->quick_play_area;
            game.change_state(game.states.gameplay_st);
        }
    });
}


/* ----------------------------------------------------------------------------
 * Loads content common for all editors.
 */
void editor::load() {
    update_canvas_coordinates();
    game.fade_mgr.start_fade(true, nullptr);
}


/* ----------------------------------------------------------------------------
 * Unloads loaded editor-related content.
 */
void editor::unload() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Updates the variables that hold the canvas's coordinates.
 */
void editor::update_canvas_coordinates() {
    if(
        canvas_separator_x < 1.0f ||
        canvas_separator_x > game.win_w - 1.0f
    ) {
        //Panic check: if the separator has crazy values, it's
        //likely not set properly.
        canvas_br.x = game.win_w * 0.675;
    } else {
        canvas_br.x = canvas_separator_x;
    }
    
    canvas_tl.x = 0;
    canvas_tl.y = 40;
    canvas_br.y = game.win_h - 16;
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void editor::update_transformations() {
    //World coordinates to screen coordinates.
    point canvas_center(
        (canvas_tl.x + canvas_br.x) / 2.0,
        (canvas_tl.y + canvas_br.y) / 2.0
    );
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + canvas_center.x / game.cam.zoom,
        -game.cam.pos.y + canvas_center.y / game.cam.zoom
    );
    al_scale_transform(&game.world_to_screen_transform, game.cam.zoom, game.cam.zoom);
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Zooms in or out to a specific amount, optionally keeping the mouse cursor
 * in the same spot.
 */
void editor::zoom(const float new_zoom, const bool anchor_cursor) {
    game.cam.zoom = clamp(new_zoom, zoom_min_level, zoom_max_level);
    
    if(anchor_cursor) {
        //Keep a backup of the old mouse coordinates.
        point old_mouse_pos = game.mouse_cursor_w;
        
        //Figure out where the mouse will be after the zoom.
        update_transformations();
        game.mouse_cursor_w = game.mouse_cursor_s;
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
        );
        
        //Readjust the transformation by shifting the camera
        //so that the cursor ends up where it was before.
        game.cam.pos.x += (old_mouse_pos.x - game.mouse_cursor_w.x);
        game.cam.pos.y += (old_mouse_pos.y - game.mouse_cursor_w.y);
    }
    
    update_transformations();
}


const float editor::transformation_controller::HANDLE_RADIUS = 6.0;
const float editor::transformation_controller::ROTATION_HANDLE_THICKNESS = 8.0;


/* ----------------------------------------------------------------------------
 * Creates a transformation controller.
 */
editor::transformation_controller::transformation_controller() :
    moving_handle(-1),
    size(point(16, 16)),
    angle(0),
    keep_aspect_ratio(true),
    allow_rotation(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the transformation (move, scale, rotate) handles.
 */
void editor::transformation_controller::draw_handles() {
    if(size.x == 0 || size.y == 0) return;
    
    //Rotation handle.
    if(allow_rotation) {
        al_draw_circle(
            center.x, center.y, radius,
            al_map_rgb(64, 64, 192), ROTATION_HANDLE_THICKNESS / game.cam.zoom
        );
    }
    
    //Outline.
    point corners[4];
    corners[0] = point(-size.x / 2.0, -size.y / 2.0);
    corners[1] = point(size.x / 2.0, -size.y / 2.0);
    corners[2] = point(size.x / 2.0, size.y / 2.0);
    corners[3] = point(-size.x / 2.0, size.y / 2.0);
    for(unsigned char c = 0; c < 4; ++c) {
        al_transform_coordinates(
            &disalign_transform, &corners[c].x, &corners[c].y
        );
    }
    for(unsigned char c = 0; c < 4; ++c) {
        size_t c2 = sum_and_wrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            al_map_rgb(32, 32, 160), 2.0 / game.cam.zoom
        );
    }
    
    //Translation and scale handles.
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        al_draw_filled_circle(
            handle_pos.x, handle_pos.y,
            HANDLE_RADIUS / game.cam.zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Returns the angle.
 */
float editor::transformation_controller::get_angle() const {
    return angle;
}


/* ----------------------------------------------------------------------------
 * Returns the center.
 */
point editor::transformation_controller::get_center() const {
    return center;
}


/* ----------------------------------------------------------------------------
 * Returns the position at which a handle is.
 */
point editor::transformation_controller::get_handle_pos(
    const unsigned char handle
) const {
    point result;
    switch(handle) {
    case 0:
    case 3:
    case 6: {
        result.x = -size.x / 2.0;
        break;
    } case 2:
    case 5:
    case 8: {
        result.x = size.x / 2.0;
        break;
    }
    }
    
    switch(handle) {
    case 0:
    case 1:
    case 2: {
        result.y = -size.y / 2.0;
        break;
    }
    case 6:
    case 7:
    case 8: {
        result.y = size.y / 2.0;
        break;
    }
    }
    
    al_transform_coordinates(&disalign_transform, &result.x, &result.y);
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the size.
 */
point editor::transformation_controller::get_size() const {
    return size;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse press, allowing a handle to be grabbed.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_down(const point pos) {
    if(size.x == 0 || size.y == 0) return false;
    
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        if(dist(handle_pos, pos) <= HANDLE_RADIUS / game.cam.zoom) {
            moving_handle = h;
            pre_move_size = size;
            return true;
        }
    }
    
    if(allow_rotation) {
        dist d(center, pos);
        if(
            d >= radius - ROTATION_HANDLE_THICKNESS / game.cam.zoom / 2.0 &&
            d <= radius + ROTATION_HANDLE_THICKNESS / game.cam.zoom / 2.0
        ) {
            moving_handle = 9;
            pre_rotation_angle = angle;
            pre_rotation_mouse_angle = ::get_angle(center, pos);
            return true;
        }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse move, allowing a handle to be moved.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_move(const point pos) {
    if(moving_handle == -1) {
        return false;
    }
    
    if(moving_handle == 4) {
        set_center(pos);
        return true;
    }
    
    if(moving_handle == 9) {
        set_angle(
            pre_rotation_angle +
            (::get_angle(center, pos) - pre_rotation_mouse_angle)
        );
        return true;
    }
    
    point aligned_cursor_pos = pos;
    al_transform_coordinates(
        &align_transform,
        &aligned_cursor_pos.x, &aligned_cursor_pos.y
    );
    point new_size = pre_move_size;
    point aligned_new_center = center;
    al_transform_coordinates(
        &align_transform,
        &aligned_new_center.x, &aligned_new_center.y
    );
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        new_size.x = size.x / 2.0 - aligned_cursor_pos.x;
        break;
    } case 2:
    case 5:
    case 8: {
        new_size.x = aligned_cursor_pos.x - (-size.x / 2.0);
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        new_size.y = (size.y / 2.0) - aligned_cursor_pos.y;
        break;
    } case 6:
    case 7:
    case 8: {
        new_size.y = aligned_cursor_pos.y - (-size.y / 2.0);
        break;
    }
    }
    
    if(keep_aspect_ratio) {
        if(
            fabs(pre_move_size.x - new_size.x) >
            fabs(pre_move_size.y - new_size.y)
        ) {
            //Most significant change is width.
            if(pre_move_size.x != 0) {
                float ratio = pre_move_size.y / pre_move_size.x;
                new_size.y = new_size.x * ratio;
            }
            
        } else {
            //Most significant change is height.
            if(pre_move_size.y != 0) {
                float ratio = pre_move_size.x / pre_move_size.y;
                new_size.x = new_size.y * ratio;
            }
            
        }
    }
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        aligned_new_center.x = (size.x / 2.0) - new_size.x / 2.0;
        break;
    } case 2:
    case 5:
    case 8: {
        aligned_new_center.x = (-size.x / 2.0) + new_size.x / 2.0;
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        aligned_new_center.y = (size.y / 2.0) - new_size.y / 2.0;
        break;
    } case 6:
    case 7:
    case 8: {
        aligned_new_center.y = (-size.y / 2.0) + new_size.y / 2.0;
        break;
    }
    }
    
    point new_center = aligned_new_center;
    al_transform_coordinates(
        &disalign_transform,
        &new_center.x, &new_center.y
    );
    
    set_center(new_center);
    set_size(new_size);
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse release, allowing a handle to be released.
 */
void editor::transformation_controller::handle_mouse_up() {
    moving_handle = -1;
}


/* ----------------------------------------------------------------------------
 * Sets the angle.
 */
void editor::transformation_controller::set_angle(const float angle) {
    this->angle = angle;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the center.
 */
void editor::transformation_controller::set_center(const point &center) {
    this->center = center;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the size.
 */
void editor::transformation_controller::set_size(const point &size) {
    this->size = size;
    update();
}


/* ----------------------------------------------------------------------------
 * Updates the transformations to match the new data, as well as
 * some caches.
 */
void editor::transformation_controller::update() {
    al_identity_transform(&align_transform);
    al_translate_transform(&align_transform, -center.x, -center.y);
    al_rotate_transform(&align_transform, -angle);
    
    al_copy_transform(&disalign_transform, &align_transform);
    al_invert_transform(&disalign_transform);
    
    radius = dist(center, center + (size / 2)).to_float();
}
