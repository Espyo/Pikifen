/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor event handler function.
 */

#include <algorithm>
#include <allegro5/allegro.h>

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


using std::set;


/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed anywhere.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F1)) {
        debug_edge_nrs = !debug_edge_nrs;
        if(debug_edge_nrs) {
            set_status("Enabled debug edge number display.");
        } else {
            set_status("Disabled debug edge number display.");
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F2)) {
        debug_sector_nrs = !debug_sector_nrs;
        if(debug_sector_nrs) {
            set_status("Enabled debug sector number display.");
        } else {
            set_status("Disabled debug sector number display.");
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F3)) {
        debug_vertex_nrs = !debug_vertex_nrs;
        if(debug_vertex_nrs) {
            set_status("Enabled debug vertex number display.");
        } else {
            set_status("Disabled debug vertex number display.");
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F4)) {
        debug_triangulation = !debug_triangulation;
        if(debug_triangulation) {
            set_status("Enabled debug triangulation display.");
        } else {
            set_status("Disabled debug triangulation display.");
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F5)) {
        debug_path_nrs = !debug_path_nrs;
        if(debug_path_nrs) {
            set_status("Enabled debug path number display.");
        } else {
            set_status("Disabled debug path number display.");
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Y, true)) {
        press_redo_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Z, true)) {
        press_undo_button();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.cam.target_pos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.cam.target_pos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.cam.target_pos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.cam.target_pos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_MINUS)) {
        press_zoom_out_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        press_zoom_in_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        press_grid_interval_decrease_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        press_grid_interval_increase_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        press_zoom_and_pos_reset_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_R)) {
        if(state == EDITOR_STATE_MOBS && sub_state == EDITOR_SUB_STATE_NONE) {
            rotate_mob_gens_to_point(game.mouse_cursor.w_pos);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_X)) {
        press_snap_mode_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_X, false, true)) {
        //Toggles the snap modes backwards.
        press_snap_mode_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_BACKSPACE)) {
        undo_layout_drawing_node();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        press_load_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        press_quick_play_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        press_quit_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        press_reference_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        press_save_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        escape_was_pressed = true;
        
        if(!dialogs.empty()) {
            close_top_dialog();
            
        } else if(state == EDITOR_STATE_LAYOUT) {
            if(sub_state == EDITOR_SUB_STATE_DRAWING) {
                cancel_layout_drawing();
            } else if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
                cancel_circle_sector();
            } else if(sub_state == EDITOR_SUB_STATE_NONE && moving) {
                cancel_layout_moving();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_MOBS) {
            if(
                sub_state == EDITOR_SUB_STATE_NEW_MOB ||
                sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
                sub_state == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
                sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
                sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
            ) {
                sub_state = EDITOR_SUB_STATE_NONE;
                set_status();
            } else if(sub_state == EDITOR_SUB_STATE_MISSION_MOBS) {
                change_state(EDITOR_STATE_GAMEPLAY);
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_PATHS) {
            if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
                sub_state = EDITOR_SUB_STATE_NONE;
                set_status();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_DETAILS) {
            if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
                sub_state = EDITOR_SUB_STATE_NONE;
                set_status();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
            }
            
        } else if(state == EDITOR_STATE_MAIN) {
            press_quit_button();
            
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_1)) {
        if(state == EDITOR_STATE_PATHS) {
            path_drawing_normals = false;
        } else if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_OFFSET;
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_2)) {
        if(state == EDITOR_STATE_PATHS) {
            path_drawing_normals = true;
        } else if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_SCALE;
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_3)) {
        if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_ANGLE;
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_A, true)) {
        press_select_all_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_C)) {
        if(
            state == EDITOR_STATE_LAYOUT &&
            sub_state == EDITOR_SUB_STATE_NONE &&
            !moving && !selecting
        ) {
            press_circle_sector_button();
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        press_copy_properties_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        if(
            !moving && !selecting &&
            game.options.area_editor_advanced_mode
        ) {
            change_state(EDITOR_STATE_DETAILS);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_D, true)) {
        if(state == EDITOR_STATE_MOBS && !moving && !selecting) {
            press_duplicate_mobs_button();
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F)) {
        press_selection_filter_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_F, false, true)) {
        //Toggles the filter modes backwards.
        press_selection_filter_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_H)) {
        if(state == EDITOR_STATE_LAYOUT && sub_state == EDITOR_SUB_STATE_NONE) {
            if(selected_sectors.empty()) {
                set_status(
                    "To set a sector's height, you must first select a sector!",
                    true
                );
            } else if(selected_sectors.size() > 1) {
                set_status(
                    "To set a sector's height, you can only select 1 sector!",
                    true
                );
            } else {
                sub_state = EDITOR_SUB_STATE_QUICK_HEIGHT_SET;
                quick_height_set_start_pos = game.mouse_cursor.s_pos;
                quick_height_set_start_height = (*selected_sectors.begin())->z;
                set_status(
                    "Move the cursor up or down to change the sector's height."
                );
            }
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L)) {
        if(
            !moving && !selecting &&
            game.options.area_editor_advanced_mode
        ) {
            change_state(EDITOR_STATE_LAYOUT);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, false, true)) {
        switch(state) {
        case EDITOR_STATE_MOBS: {
            if(selected_mobs.size() == 1 || selection_homogenized) {
                if(sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK) {
                    sub_state = EDITOR_SUB_STATE_NONE;
                } else {
                    sub_state = EDITOR_SUB_STATE_ADD_MOB_LINK;
                }
            }
            break;
        }
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_N)) {
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
            press_layout_drawing_button();
            break;
        } case EDITOR_STATE_MOBS: {
            press_new_mob_button();
            break;
        } case EDITOR_STATE_PATHS: {
            press_new_path_button();
            break;
        } case EDITOR_STATE_DETAILS: {
            press_new_tree_shadow_button();
            break;
        }
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_O)) {
        if(
            !moving && !selecting &&
            game.options.area_editor_advanced_mode
        ) {
            change_state(EDITOR_STATE_MOBS);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P)) {
        if(
            !moving && !selecting &&
            game.options.area_editor_advanced_mode
        ) {
            change_state(EDITOR_STATE_PATHS);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, false, true)) {
        preview_mode = !preview_mode;
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_T, true)) {
        press_paste_texture_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_V, true)) {
        press_paste_properties_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_DELETE)) {
        press_delete_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        press_zoom_everything_button();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a keyboard key being released anywhere.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {
    if(ev.keyboard.keycode == ALLEGRO_KEY_H) {
        if(
            state == EDITOR_STATE_LAYOUT &&
            sub_state == EDITOR_SUB_STATE_QUICK_HEIGHT_SET
        ) {
            sub_state = EDITOR_SUB_STATE_NONE;
            set_status();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(is_ctrl_pressed) {
        handle_lmb_down(ev);
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            vertex* clicked_vertex =
                get_vertex_under_point(game.mouse_cursor.w_pos);
            if(!clicked_vertex) {
                edge* clicked_edge =
                    get_edge_under_point(game.mouse_cursor.w_pos);
                if(clicked_edge) {
                    register_change("edge split");
                    vertex* new_vertex =
                        split_edge(clicked_edge, game.mouse_cursor.w_pos);
                    clear_selection();
                    selected_vertexes.insert(new_vertex);
                    update_vertex_selection();
                }
            }
        }
        break;
        
    }
    case EDITOR_STATE_MOBS: {
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            mob_gen* clicked_mob =
                get_mob_under_point(game.mouse_cursor.w_pos);
            if(!clicked_mob) {
                create_mob_under_cursor();
                //Quit now, otherwise the code after this will simulate a
                //regular click, and if the mob is on the grid and the cursor
                //isn't, this will deselect the mob.
                return;
            }
        }
        break;
        
    }
    case EDITOR_STATE_PATHS: {
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            bool clicked_stop =
                get_path_stop_under_point(game.mouse_cursor.w_pos);
            if(!clicked_stop) {
                path_link* clicked_link_1;
                path_link* clicked_link_2;
                bool clicked_link =
                    get_path_link_under_point(
                        game.mouse_cursor.w_pos,
                        &clicked_link_1, &clicked_link_2
                    );
                if(clicked_link) {
                    register_change("path link split");
                    path_stop* new_stop =
                        split_path_link(
                            clicked_link_1, clicked_link_2,
                            game.mouse_cursor.w_pos
                        );
                    clear_selection();
                    selected_path_stops.insert(new_stop);
                }
            }
        }
        break;
        
    }
    default: {
        break;
        
    }
    }
    
    handle_lmb_down(ev);
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_GAMEPLAY: {

        if(sub_state == EDITOR_SUB_STATE_MISSION_EXIT) {
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor.w_pos,
                &game.cur_area_data.mission.goal_exit_center,
                &game.cur_area_data.mission.goal_exit_size,
                NULL,
                1.0f / game.cam.zoom
            );
        }
        break;
        
    }
    case EDITOR_STATE_LAYOUT: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_DRAWING: {
    
            //Drawing the layout.
            point hotspot = snap_point(game.mouse_cursor.w_pos);
            
            //First, check if the user is trying to undo the previous node.
            if(
                !drawing_nodes.empty() &&
                dist(
                    hotspot,
                    point(
                        drawing_nodes.back().snapped_spot.x,
                        drawing_nodes.back().snapped_spot.y
                    )
                ) <= AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            ) {
                undo_layout_drawing_node();
                return;
            }
            
            if(drawing_nodes.empty()) {
                //First node.
                drawing_nodes.push_back(layout_drawing_node(this, hotspot));
                
            } else {
            
                check_drawing_line(hotspot);
                
                bool needs_reverse = false;
                if(drawing_line_result == DRAWING_LINE_HIT_EDGE_OR_VERTEX) {
                    //Instead of throwing an error, let's swap the order around.
                    needs_reverse = true;
                    drawing_line_result = DRAWING_LINE_OK;
                }
                
                if(drawing_line_result != DRAWING_LINE_OK) {
                    handle_line_error();
                    
                } else if(
                    dist(hotspot, drawing_nodes.begin()->snapped_spot) <=
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
                ) {
                    //Back to the first vertex. Finish the drawing.
                    finish_new_sector_drawing();
                    
                } else {
                    //Add a new node.
                    drawing_nodes.push_back(layout_drawing_node(this, hotspot));
                    
                    if(needs_reverse) {
                        //This is now a sector split drawing.
                        std::reverse(
                            drawing_nodes.begin(), drawing_nodes.end()
                        );
                    }
                    
                    if(
                        drawing_nodes.back().on_edge ||
                        drawing_nodes.back().on_vertex
                    ) {
                        //Split the sector.
                        setup_sector_split();
                        SECTOR_SPLIT_RESULTS result =
                            get_sector_split_evaluation();
                        switch(result) {
                        case SECTOR_SPLIT_OK: {
                            do_sector_split();
                            break;
                            
                        } case SECTOR_SPLIT_INVALID: {
                            rollback_to_prepared_state(
                                sector_split_info.pre_split_area_data
                            );
                            forget_prepared_state(
                                sector_split_info.pre_split_area_data
                            );
                            clear_selection();
                            clear_layout_drawing();
                            sub_state = EDITOR_SUB_STATE_NONE;
                            set_status(
                                "That's not a valid split!",
                                true
                            );
                            break;
                            
                        } case SECTOR_SPLIT_USELESS: {
                            rollback_to_prepared_state(
                                sector_split_info.pre_split_area_data
                            );
                            forget_prepared_state(
                                sector_split_info.pre_split_area_data
                            );
                            recreate_drawing_nodes();
                            sector_split_info.useless_split_part_2_checkpoint =
                                drawing_nodes.size();
                            update_layout_drawing_status_text();
                            break;
                        }
                        }
                    }
                }
            }
            
            break;
            
        } case EDITOR_SUB_STATE_CIRCLE_SECTOR: {
    
            //Create a new circular sector.
            point hotspot = snap_point(game.mouse_cursor.w_pos);
            
            if(new_circle_sector_step == 0) {
                new_circle_sector_center = hotspot;
                new_circle_sector_anchor = new_circle_sector_center;
                new_circle_sector_step++;
                
            } else if(new_circle_sector_step == 1) {
                new_circle_sector_anchor = hotspot;
                set_new_circle_sector_points();
                new_circle_sector_step++;
                
            } else {
                set_new_circle_sector_points();
                
                bool all_valid = true;
                for(
                    size_t e = 0; e < new_circle_sector_valid_edges.size(); ++e
                ) {
                    if(!new_circle_sector_valid_edges[e]) {
                        all_valid = false;
                        break;
                    }
                }
                if(!all_valid) {
                    set_status("Some lines touch existing edges!", true);
                } else {
                    finish_circle_sector();
                }
                
            }
            
            break;
            
        } case EDITOR_SUB_STATE_OCTEE: {
    
            moving = true;
            octee_drag_start = game.mouse_cursor.w_pos;
            sector* s_ptr = *selected_sectors.begin();
            octee_orig_angle = s_ptr->texture_info.rot;
            octee_orig_offset = s_ptr->texture_info.translation;
            octee_orig_scale = s_ptr->texture_info.scale;
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool tw_handled = false;
            if(
                game.options.area_editor_sel_trans &&
                selected_vertexes.size() >= 2
            ) {
                tw_handled =
                    cur_transformation_widget.handle_mouse_down(
                        game.mouse_cursor.w_pos,
                        &selection_center,
                        &selection_size,
                        &selection_angle,
                        1.0f / game.cam.zoom
                    );
            }
            
            if(!tw_handled) {
            
                //Start a new layout selection or select something.
                bool start_new_selection = true;
                
                vertex* clicked_vertex = NULL;
                edge* clicked_edge = NULL;
                sector* clicked_sector = NULL;
                get_hovered_layout_element(
                    &clicked_vertex, &clicked_edge, &clicked_sector
                );
                
                if(!is_shift_pressed) {
                    if(clicked_vertex || clicked_edge || clicked_sector) {
                        start_new_selection = false;
                    }
                    
                }
                
                if(start_new_selection) {
                    if(!is_ctrl_pressed) clear_selection();
                    selecting = true;
                    selection_start = game.mouse_cursor.w_pos;
                    selection_end = game.mouse_cursor.w_pos;
                    
                } else {
                
                    if(clicked_vertex) {
                        if(
                            selected_vertexes.find(clicked_vertex) ==
                            selected_vertexes.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clear_selection();
                            }
                            select_vertex(clicked_vertex);
                        }
                    } else if(clicked_edge) {
                        if(
                            selected_edges.find(clicked_edge) ==
                            selected_edges.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clear_selection();
                            }
                            select_edge(clicked_edge);
                        }
                    } else {
                        if(
                            selected_sectors.find(clicked_sector) ==
                            selected_sectors.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clear_selection();
                            }
                            select_sector(clicked_sector);
                        }
                    }
                    
                }
                
                selection_homogenized = false;
                set_selection_status_text();
                
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_MOBS: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_NEW_MOB: {
    
            //Create a mob where the cursor is.
            create_mob_under_cursor();
            
            break;
            
        } case EDITOR_SUB_STATE_DUPLICATE_MOB: {
    
            //Duplicate the current mobs to where the cursor is.
            register_change("object duplication");
            sub_state = EDITOR_SUB_STATE_NONE;
            point hotspot = snap_point(game.mouse_cursor.w_pos);
            
            point selection_tl = (*selected_mobs.begin())->pos;
            point selection_br = selection_tl;
            for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
                if(m == selected_mobs.begin()) continue;
                if((*m)->pos.x < selection_tl.x) {
                    selection_tl.x = (*m)->pos.x;
                }
                if((*m)->pos.x > selection_br.x) {
                    selection_br.x = (*m)->pos.x;
                }
                if((*m)->pos.y < selection_tl.y) {
                    selection_tl.y = (*m)->pos.y;
                }
                if((*m)->pos.y > selection_br.y) {
                    selection_br.y = (*m)->pos.y;
                }
            }
            point selection_center = (selection_br + selection_tl) / 2.0;
            set<mob_gen*> mobs_to_select;
            
            for(auto &m : selected_mobs) {
                mob_gen* new_mg = new mob_gen(*m);
                new_mg->pos = point(hotspot + (m->pos) - selection_center);
                game.cur_area_data.mob_generators.push_back(new_mg);
                mobs_to_select.insert(new_mg);
            }
            
            clear_selection();
            selected_mobs = mobs_to_select;
            
            set_status(
                "Duplicated " +
                amount_str((int) selected_mobs.size(), "object") + "."
            );
            
            break;
            
        } case EDITOR_SUB_STATE_STORE_MOB_INSIDE: {
    
            //Store the mob inside another.
            size_t target_idx;
            mob_gen* target =
                get_mob_under_point(game.mouse_cursor.w_pos, &target_idx);
            if(!target) return;
            
            for(const auto &m : selected_mobs) {
                if(m == target) {
                    set_status(
                        "You can't store to an object inside itself!",
                        true
                    );
                    return;
                }
            }
            mob_gen* m_ptr = *(selected_mobs.begin());
            if(m_ptr->stored_inside == target_idx) {
                set_status(
                    "The object is already stored inside that object!",
                    true
                );
                return;
            }
            
            register_change("Object in object storing");
            
            m_ptr->stored_inside = target_idx;
            
            homogenize_selected_mobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            set_status("Stored the object inside another.");
            
            break;
            
        } case EDITOR_SUB_STATE_ADD_MOB_LINK: {
    
            //Link two mobs.
            mob_gen* target = get_mob_under_point(game.mouse_cursor.w_pos);
            if(!target) return;
            
            for(const auto &m : selected_mobs) {
                if(m == target) {
                    set_status(
                        "You can't link to an object to itself!",
                        true
                    );
                    return;
                }
            }
            mob_gen* m_ptr = *(selected_mobs.begin());
            for(size_t l = 0; l < m_ptr->links.size(); ++l) {
                if(m_ptr->links[l] == target) {
                    set_status(
                        "The object already links to that object!",
                        true
                    );
                    return;
                }
            }
            
            register_change("Object link creation");
            
            m_ptr->links.push_back(target);
            m_ptr->link_nrs.push_back(
                game.cur_area_data.find_mob_gen_nr(target)
            );
            
            homogenize_selected_mobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            set_status("Linked the two objects.");
            
            break;
            
        } case EDITOR_SUB_STATE_DEL_MOB_LINK: {
    
            //Delete a mob link.
            mob_gen* target = get_mob_under_point(game.mouse_cursor.w_pos);
            mob_gen* m_ptr = *(selected_mobs.begin());
            
            if(!target) {
                std::pair<mob_gen*, mob_gen*> data1;
                std::pair<mob_gen*, mob_gen*> data2;
                if(
                    !get_mob_link_under_point(
                        game.mouse_cursor.w_pos, &data1, &data2
                    )
                ) {
                    return;
                }
                
                if(
                    data1.first != m_ptr &&
                    data1.second != m_ptr &&
                    data2.first != m_ptr &&
                    data2.second != m_ptr
                ) {
                    set_status(
                        "That link does not belong to the current object!",
                        true
                    );
                    return;
                }
                
                if(data1.first == m_ptr) {
                    target = data1.second;
                } else if(data2.first == m_ptr) {
                    target = data2.second;
                }
            }
            
            size_t link_i = 0;
            for(; link_i < m_ptr->links.size(); ++link_i) {
                if(m_ptr->links[link_i] == target) {
                    break;
                }
            }
            
            if(link_i == m_ptr->links.size()) {
                set_status(
                    "That object is not linked by the current one!",
                    true
                );
                return;
            } else {
                register_change("Object link deletion");
                m_ptr->links.erase(m_ptr->links.begin() + link_i);
                m_ptr->link_nrs.erase(m_ptr->link_nrs.begin() + link_i);
            }
            
            homogenize_selected_mobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            set_status("Deleted object link.");
            
            break;
            
        } case EDITOR_SUB_STATE_MISSION_MOBS: {
    
            size_t clicked_mob_idx;
            mob_gen* clicked_mob =
                get_mob_under_point(game.mouse_cursor.w_pos, &clicked_mob_idx);
                
            if(
                clicked_mob_idx != INVALID &&
                game.mission_goals[game.cur_area_data.mission.goal]->
                is_mob_applicable(clicked_mob->type)
            ) {
                register_change("mission object requirements change");
                auto it =
                    game.cur_area_data.mission.goal_mob_idxs.find(
                        clicked_mob_idx
                    );
                if(it == game.cur_area_data.mission.goal_mob_idxs.end()) {
                    game.cur_area_data.mission.goal_mob_idxs.insert(
                        clicked_mob_idx
                    );
                } else {
                    game.cur_area_data.mission.goal_mob_idxs.erase(it);
                }
            }
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            //Start a new mob selection or select something.
            bool start_new_selection = true;
            mob_gen* clicked_mob = get_mob_under_point(game.mouse_cursor.w_pos);
            
            if(!is_shift_pressed) {
                if(clicked_mob) {
                    start_new_selection = false;
                }
            }
            
            if(start_new_selection) {
                if(!is_ctrl_pressed) clear_selection();
                selecting = true;
                selection_start = game.mouse_cursor.w_pos;
                selection_end = game.mouse_cursor.w_pos;
                
            } else {
                if(selected_mobs.find(clicked_mob) == selected_mobs.end()) {
                    if(!is_ctrl_pressed) {
                        clear_selection();
                    }
                    selected_mobs.insert(clicked_mob);
                }
                
            }
            
            selection_homogenized = false;
            set_selection_status_text();
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_PATHS: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_PATH_DRAWING: {
    
            //Drawing a path.
            point hotspot = snap_point(game.mouse_cursor.w_pos);
            path_stop* clicked_stop = get_path_stop_under_point(hotspot);
            
            if(path_drawing_stop_1) {
                //A starting stop already exists, so now we create a link.
                path_stop* next_stop = NULL;
                if(clicked_stop) {
                    if(clicked_stop == path_drawing_stop_1) {
                        path_drawing_stop_1 = NULL;
                    } else {
                        next_stop = clicked_stop;
                    }
                } else {
                    register_change("path stop creation");
                    next_stop = new path_stop(hotspot);
                    game.cur_area_data.path_stops.push_back(next_stop);
                    set_status("Created path stop.");
                }
                
                if(next_stop) {
                    register_change("path stop link");
                    path_drawing_stop_1->add_link(
                        next_stop, path_drawing_normals
                    );
                    path_link* l1 = path_drawing_stop_1->get_link(next_stop);
                    path_link* l2 = next_stop->get_link(path_drawing_stop_1);
                    l1->type = path_drawing_type;
                    l1->label = path_drawing_label;
                    if(l2) {
                        l2->type = path_drawing_type;
                        l2->label = path_drawing_label;
                    }
                    game.cur_area_data.fix_path_stop_nrs(path_drawing_stop_1);
                    game.cur_area_data.fix_path_stop_nrs(next_stop);
                    next_stop->calculate_dists_plus_neighbors();
                    set_status("Created path link.");
                    
                    if(clicked_stop) {
                        path_drawing_stop_1 = NULL;
                    } else {
                        path_drawing_stop_1 = next_stop;
                    }
                }
                
            } else {
                //We need to create or assign a starting stop.
                if(clicked_stop) {
                    path_drawing_stop_1 = clicked_stop;
                } else {
                    register_change("path stop creation");
                    path_drawing_stop_1 = new path_stop(hotspot);
                    game.cur_area_data.path_stops.push_back(
                        path_drawing_stop_1
                    );
                    set_status("Created path stop.");
                }
                
            }
            
            path_preview.clear(); //Clear so it doesn't reference deleted stops.
            path_preview_timer.start(false);
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            //First, check if the user clicked on a path preview checkpoint.
            if(show_path_preview) {
                for(unsigned char c = 0; c < 2; ++c) {
                    if(
                        bbox_check(
                            path_preview_checkpoints[c],
                            game.mouse_cursor.w_pos,
                            AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS /
                            game.cam.zoom
                        )
                    ) {
                        clear_selection();
                        moving_path_preview_checkpoint = c;
                        return;
                    }
                }
            }
            
            //Start a new path selection or select something.
            bool start_new_selection = true;
            
            path_stop* clicked_stop =
                get_path_stop_under_point(game.mouse_cursor.w_pos);
            path_link* clicked_link_1;
            path_link* clicked_link_2;
            bool clicked_link =
                get_path_link_under_point(
                    game.mouse_cursor.w_pos,
                    &clicked_link_1, &clicked_link_2
                );
            if(!is_shift_pressed) {
                if(clicked_stop || clicked_link) {
                    start_new_selection = false;
                }
                
            }
            
            if(start_new_selection) {
                if(!is_ctrl_pressed) clear_selection();
                selecting = true;
                selection_start = game.mouse_cursor.w_pos;
                selection_end = game.mouse_cursor.w_pos;
                
            } else {
            
                if(clicked_stop) {
                    if(
                        selected_path_stops.find(clicked_stop) ==
                        selected_path_stops.end()
                    ) {
                        if(!is_ctrl_pressed) {
                            clear_selection();
                        }
                        selected_path_stops.insert(clicked_stop);
                    }
                } else {
                    if(
                        selected_path_links.find(clicked_link_1) ==
                        selected_path_links.end()
                    ) {
                        if(!is_ctrl_pressed) {
                            clear_selection();
                        }
                        selected_path_links.insert(clicked_link_1);
                        if(clicked_link_2 != NULL) {
                            selected_path_links.insert(clicked_link_2);
                        }
                    }
                }
                
                set_selection_status_text();
                
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_DETAILS: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_NEW_SHADOW: {
    
            //Create a new shadow where the cursor is.
            register_change("tree shadow creation");
            sub_state = EDITOR_SUB_STATE_NONE;
            point hotspot = snap_point(game.mouse_cursor.w_pos);
            
            tree_shadow* new_shadow = new tree_shadow(hotspot);
            new_shadow->bitmap = game.bmp_error;
            
            game.cur_area_data.tree_shadows.push_back(new_shadow);
            
            select_tree_shadow(new_shadow);
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool transformation_handled = false;
            if(selected_shadow) {
                transformation_handled =
                    cur_transformation_widget.handle_mouse_down(
                        game.mouse_cursor.w_pos,
                        &selected_shadow->center,
                        &selected_shadow->size,
                        &selected_shadow->angle,
                        1.0f / game.cam.zoom
                    );
            }
            
            if(!transformation_handled) {
                //Select a tree shadow.
                selected_shadow = NULL;
                for(
                    size_t s = 0;
                    s < game.cur_area_data.tree_shadows.size(); ++s
                ) {
                
                    tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
                    point min_coords, max_coords;
                    get_transformed_rectangle_bounding_box(
                        s_ptr->center, s_ptr->size, s_ptr->angle,
                        &min_coords, &max_coords
                    );
                    
                    if(
                        game.mouse_cursor.w_pos.x >= min_coords.x &&
                        game.mouse_cursor.w_pos.x <= max_coords.x &&
                        game.mouse_cursor.w_pos.y >= min_coords.y &&
                        game.mouse_cursor.w_pos.y <= max_coords.y
                    ) {
                        select_tree_shadow(s_ptr);
                        break;
                    }
                }
                
                set_selection_status_text();
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_TOOLS: {

        if(reference_bitmap) {
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor.w_pos,
                &reference_center,
                &reference_size,
                NULL,
                1.0f / game.cam.zoom
            );
        }
        
        break;
        
    } case EDITOR_STATE_REVIEW: {

        if(show_cross_section) {
            moving_cross_section_point = -1;
            for(unsigned char p = 0; p < 2; ++p) {
                if(
                    bbox_check(
                        cross_section_checkpoints[p], game.mouse_cursor.w_pos,
                        AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom
                    )
                ) {
                    moving_cross_section_point = p;
                    break;
                }
            }
        }
        
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(selecting) {
    
        float selection_x1 = std::min(selection_start.x, selection_end.x);
        float selection_x2 = std::max(selection_start.x, selection_end.x);
        float selection_y1 = std::min(selection_start.y, selection_end.y);
        float selection_y2 = std::max(selection_start.y, selection_end.y);
        selection_end = game.mouse_cursor.w_pos;
        
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
    
            //Selection box around the layout.
            if(!is_ctrl_pressed) clear_selection();
            
            for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
                vertex* v_ptr = game.cur_area_data.vertexes[v];
                
                if(
                    v_ptr->x >= selection_x1 &&
                    v_ptr->x <= selection_x2 &&
                    v_ptr->y >= selection_y1 &&
                    v_ptr->y <= selection_y2
                ) {
                    selected_vertexes.insert(v_ptr);
                }
            }
            update_vertex_selection();
            
            if(selection_filter != SELECTION_FILTER_VERTEXES) {
                for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
                    edge* e_ptr = game.cur_area_data.edges[e];
                    
                    if(
                        e_ptr->vertexes[0]->x >= selection_x1 &&
                        e_ptr->vertexes[0]->x <= selection_x2 &&
                        e_ptr->vertexes[0]->y >= selection_y1 &&
                        e_ptr->vertexes[0]->y <= selection_y2 &&
                        e_ptr->vertexes[1]->x >= selection_x1 &&
                        e_ptr->vertexes[1]->x <= selection_x2 &&
                        e_ptr->vertexes[1]->y >= selection_y1 &&
                        e_ptr->vertexes[1]->y <= selection_y2
                    ) {
                        selected_edges.insert(e_ptr);
                    }
                }
            }
            
            if(selection_filter == SELECTION_FILTER_SECTORS) {
                for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
                    sector* s_ptr = game.cur_area_data.sectors[s];
                    bool valid_sector = true;
                    
                    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
                        edge* e_ptr = s_ptr->edges[e];
                        
                        if(
                            e_ptr->vertexes[0]->x < selection_x1 ||
                            e_ptr->vertexes[0]->x > selection_x2 ||
                            e_ptr->vertexes[0]->y < selection_y1 ||
                            e_ptr->vertexes[0]->y > selection_y2 ||
                            e_ptr->vertexes[1]->x < selection_x1 ||
                            e_ptr->vertexes[1]->x > selection_x2 ||
                            e_ptr->vertexes[1]->y < selection_y1 ||
                            e_ptr->vertexes[1]->y > selection_y2
                        ) {
                            valid_sector = false;
                            break;
                        }
                    }
                    
                    if(valid_sector) {
                        selected_sectors.insert(s_ptr);
                    }
                }
            }
            
            selection_homogenized = false;
            set_selection_status_text();
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            //Selection box around mobs.
            if(!is_ctrl_pressed) clear_selection();
            
            for(
                size_t m = 0;
                m < game.cur_area_data.mob_generators.size(); ++m
            ) {
                mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
                float radius = get_mob_gen_radius(m_ptr);
                
                if(
                    m_ptr->pos.x - radius >= selection_x1 &&
                    m_ptr->pos.x + radius <= selection_x2 &&
                    m_ptr->pos.y - radius >= selection_y1 &&
                    m_ptr->pos.y + radius <= selection_y2
                ) {
                    selected_mobs.insert(m_ptr);
                }
            }
            
            selection_homogenized = false;
            set_selection_status_text();
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            //Selection box around path stops.
            if(!is_ctrl_pressed) clear_selection();
            
            for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = game.cur_area_data.path_stops[s];
                
                if(
                    s_ptr->pos.x -
                    AREA_EDITOR::PATH_STOP_RADIUS >= selection_x1 &&
                    s_ptr->pos.x +
                    AREA_EDITOR::PATH_STOP_RADIUS <= selection_x2 &&
                    s_ptr->pos.y -
                    AREA_EDITOR::PATH_STOP_RADIUS >= selection_y1 &&
                    s_ptr->pos.y +
                    AREA_EDITOR::PATH_STOP_RADIUS <= selection_y2
                ) {
                    selected_path_stops.insert(s_ptr);
                }
            }
            
            for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = game.cur_area_data.path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); ++l) {
                    path_stop* s2_ptr = s_ptr->links[l]->end_ptr;
                    
                    if(
                        s_ptr->pos.x >= selection_x1 &&
                        s_ptr->pos.x <= selection_x2 &&
                        s_ptr->pos.y >= selection_y1 &&
                        s_ptr->pos.y <= selection_y2 &&
                        s2_ptr->pos.x >= selection_x1 &&
                        s2_ptr->pos.x <= selection_x2 &&
                        s2_ptr->pos.y >= selection_y1 &&
                        s2_ptr->pos.y <= selection_y2
                    ) {
                        selected_path_links.insert(s_ptr->links[l]);
                    }
                }
            }
            
            set_selection_status_text();
            
            break;
            
        }
        }
        
    } else {
    
        switch(state) {
        case EDITOR_STATE_GAMEPLAY: {
    
            if(sub_state == EDITOR_SUB_STATE_MISSION_EXIT) {
                cur_transformation_widget.handle_mouse_move(
                    snap_point(game.mouse_cursor.w_pos, true),
                    &game.cur_area_data.mission.goal_exit_center,
                    &game.cur_area_data.mission.goal_exit_size,
                    NULL,
                    1.0f / game.cam.zoom,
                    false,
                    AREA_EDITOR::MISSION_EXIT_MIN_SIZE,
                    is_alt_pressed
                );
            }
            break;
            
        }
        case EDITOR_STATE_LAYOUT: {
    
            bool tw_handled = false;
            if(
                game.options.area_editor_sel_trans &&
                selected_vertexes.size() >= 2
            ) {
                tw_handled =
                    cur_transformation_widget.handle_mouse_move(
                        snap_point(game.mouse_cursor.w_pos, true),
                        &selection_center,
                        &selection_size,
                        &selection_angle,
                        1.0f / game.cam.zoom,
                        false,
                        AREA_EDITOR::SELECTION_TW_PADDING * 2.0f,
                        is_alt_pressed
                    );
                if(tw_handled) {
                    if(!moving) {
                        start_vertex_move();
                    }
                    
                    ALLEGRO_TRANSFORM t;
                    al_identity_transform(&t);
                    al_scale_transform(
                        &t,
                        selection_size.x / selection_orig_size.x,
                        selection_size.y / selection_orig_size.y
                    );
                    al_translate_transform(
                        &t,
                        selection_center.x - selection_orig_center.x,
                        selection_center.y - selection_orig_center.y
                    );
                    al_rotate_transform(
                        &t,
                        selection_angle - selection_orig_angle
                    );
                    
                    for(vertex* v : selected_vertexes) {
                        point p = pre_move_vertex_coords[v];
                        p -= selection_orig_center;
                        al_transform_coordinates(&t, &p.x, &p.y);
                        p += selection_orig_center;
                        v->x = p.x;
                        v->y = p.y;
                    }
                }
            }
            
            if(
                !tw_handled &&
                !selected_vertexes.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move vertexes.
                if(!moving) {
                    start_vertex_move();
                }
                
                point mouse_offset = game.mouse_cursor.w_pos - move_mouse_start_pos;
                point closest_vertex_new_p =
                    snap_point(
                        move_start_pos + mouse_offset, true
                    );
                point offset =
                    closest_vertex_new_p - move_start_pos;
                for(vertex* v : selected_vertexes) {
                    point orig = pre_move_vertex_coords[v];
                    v->x = orig.x + offset.x;
                    v->y = orig.y + offset.y;
                }
                
            } else if(
                sub_state == EDITOR_SUB_STATE_OCTEE && moving
            ) {
                //Move sector texture transformation property.
                sector* s_ptr = *selected_sectors.begin();
                
                switch(octee_mode) {
                case OCTEE_MODE_OFFSET: {
                    register_change("sector texture offset change");
                    point diff = (game.mouse_cursor.w_pos - octee_drag_start);
                    diff = rotate_point(diff, -s_ptr->texture_info.rot);
                    diff = diff / s_ptr->texture_info.scale;
                    s_ptr->texture_info.translation = octee_orig_offset + diff;
                    break;
                } case OCTEE_MODE_SCALE: {
                    register_change("sector texture scale change");
                    point diff = (game.mouse_cursor.w_pos - octee_drag_start);
                    diff = rotate_point(diff, -s_ptr->texture_info.rot);
                    point drag_start_rot =
                        rotate_point(
                            octee_drag_start, -s_ptr->texture_info.rot
                        );
                    diff = diff / drag_start_rot * octee_orig_scale;
                    s_ptr->texture_info.scale = octee_orig_scale + diff;
                    break;
                } case OCTEE_MODE_ANGLE: {
                    register_change("sector texture angle change");
                    float drag_start_a = get_angle(octee_drag_start);
                    float cursor_a = get_angle(game.mouse_cursor.w_pos);
                    s_ptr->texture_info.rot =
                        octee_orig_angle + (cursor_a - drag_start_a);
                    break;
                }
                };
                
                homogenize_selected_sectors();
            }
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            if(
                !selected_mobs.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move mobs.
                if(!moving) {
                    start_mob_move();
                }
                
                point mouse_offset = game.mouse_cursor.w_pos - move_mouse_start_pos;
                point closest_mob_new_p =
                    snap_point(move_start_pos + mouse_offset);
                point offset = closest_mob_new_p - move_start_pos;
                for(
                    auto m = selected_mobs.begin();
                    m != selected_mobs.end(); ++m
                ) {
                    point orig = pre_move_mob_coords[*m];
                    (*m)->pos = orig + offset;
                }
            }
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            if(
                !selected_path_stops.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move path stops.
                if(!moving) {
                    start_path_stop_move();
                }
                
                point mouse_offset = game.mouse_cursor.w_pos - move_mouse_start_pos;
                point closest_stop_new_p =
                    snap_point(move_start_pos + mouse_offset);
                point offset = closest_stop_new_p - move_start_pos;
                for(
                    auto s = selected_path_stops.begin();
                    s != selected_path_stops.end(); ++s
                ) {
                    point orig = pre_move_stop_coords[*s];
                    (*s)->pos.x = orig.x + offset.x;
                    (*s)->pos.y = orig.y + offset.y;
                }
                
                for(
                    auto s = selected_path_stops.begin();
                    s != selected_path_stops.end(); ++s
                ) {
                    (*s)->calculate_dists_plus_neighbors();
                }
                
                path_preview_timer.start(false);
                
            } else if(
                moving_path_preview_checkpoint != -1 &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move path preview checkpoints.
                path_preview_checkpoints[moving_path_preview_checkpoint] =
                    snap_point(game.mouse_cursor.w_pos);
                path_preview_timer.start(false);
            }
            
            break;
            
        } case EDITOR_STATE_DETAILS: {
    
            if(
                selected_shadow &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move tree shadow.
                point shadow_center = selected_shadow->center;
                point shadow_size = selected_shadow->size;
                float shadow_angle = selected_shadow->angle;
                if(
                    cur_transformation_widget.handle_mouse_move(
                        snap_point(game.mouse_cursor.w_pos),
                        &shadow_center,
                        &shadow_size,
                        &shadow_angle,
                        1.0f / game.cam.zoom,
                        selected_shadow_keep_aspect_ratio,
                        -FLT_MAX,
                        is_alt_pressed
                    )
                ) {
                    register_change("tree shadow transformation");
                    selected_shadow->center = shadow_center;
                    selected_shadow->size = shadow_size;
                    selected_shadow->angle = shadow_angle;
                }
            }
            
            break;
            
        } case EDITOR_STATE_TOOLS: {
    
            //Move reference handle.
            cur_transformation_widget.handle_mouse_move(
                snap_point(game.mouse_cursor.w_pos),
                &reference_center,
                &reference_size,
                NULL,
                1.0f / game.cam.zoom,
                reference_keep_aspect_ratio,
                5.0f,
                is_alt_pressed
            );
            
            break;
            
        } case EDITOR_STATE_REVIEW: {
    
            //Move cross-section points.
            if(moving_cross_section_point != -1) {
                cross_section_checkpoints[moving_cross_section_point] =
                    snap_point(game.mouse_cursor.w_pos);
            }
            
            break;
            
        }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    selecting = false;
    
    if(moving) {
        if(
            state == EDITOR_STATE_LAYOUT && sub_state != EDITOR_SUB_STATE_OCTEE
        ) {
            finish_layout_moving();
        }
        moving = false;
    }
    
    cur_transformation_widget.handle_mouse_up();
    
    moving_path_preview_checkpoint = -1;
    moving_cross_section_point = -1;
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    game.mouse_cursor.s_pos.x = ev.mouse.x;
    game.mouse_cursor.s_pos.y = ev.mouse.y;
    game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
    );
    
    //Update highlighted elements.
    highlighted_vertex = NULL;
    highlighted_edge = NULL;
    highlighted_sector = NULL;
    highlighted_mob = NULL;
    highlighted_path_stop = NULL;
    highlighted_path_link = NULL;
    if(!is_mouse_in_gui) {
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
            get_hovered_layout_element(
                &highlighted_vertex, &highlighted_edge, &highlighted_sector
            );
            break;
        } case EDITOR_STATE_MOBS: {
            highlighted_mob =
                get_mob_under_point(game.mouse_cursor.w_pos);
            break;
        } case EDITOR_STATE_PATHS: {
            path_link* hovered_link_1;
            
            highlighted_path_stop =
                get_path_stop_under_point(game.mouse_cursor.w_pos);
                
            if (highlighted_path_stop == NULL) {
                //Selecting the stop takes priority,
                //so keep the link null if there's a stop.
                get_path_link_under_point(
                    game.mouse_cursor.w_pos,
                    &hovered_link_1, &highlighted_path_link
                );
                if(highlighted_path_link == NULL) {
                    highlighted_path_link = hovered_link_1;
                }
            }
            break;
        }
        }
    }
    
    if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        point hotspot = snap_point(game.mouse_cursor.w_pos, true);
        if(new_circle_sector_step == 1) {
            new_circle_sector_anchor = hotspot;
        } else {
            set_new_circle_sector_points();
        }
    }
    
    if(sub_state == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        float offset = quick_height_set_start_pos.y - game.mouse_cursor.s_pos.y;
        offset = floor(offset / 2.0f);
        offset = floor(offset / 10.0f);
        offset *= 10.0f;
        register_change("quick sector height set");
        (*selected_sectors.begin())->z = quick_height_set_start_height + offset;
        update_all_edge_offset_caches();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void area_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}
