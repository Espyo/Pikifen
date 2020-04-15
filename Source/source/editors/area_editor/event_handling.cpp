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
#include "../../vars.h"

using std::set;

/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed anywhere.
 */
void area_editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_F1) {
        debug_edge_nrs = !debug_edge_nrs;
        if(debug_edge_nrs) {
            emit_status_bar_message(
                "Enabled debug edge number display.", false
            );
        } else {
            emit_status_bar_message(
                "Disabled debug edge number display.", false
            );
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_F2) {
        debug_sector_nrs = !debug_sector_nrs;
        if(debug_sector_nrs) {
            emit_status_bar_message(
                "Enabled debug sector number display.", false
            );
        } else {
            emit_status_bar_message(
                "Disabled debug sector number display.", false
            );
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_F3) {
        debug_vertex_nrs = !debug_vertex_nrs;
        if(debug_vertex_nrs) {
            emit_status_bar_message(
                "Enabled debug vertex number display.", false
            );
        } else {
            emit_status_bar_message(
                "Disabled debug vertex number display.", false
            );
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_F4) {
        debug_triangulation = !debug_triangulation;
        if(debug_triangulation) {
            emit_status_bar_message(
                "Enabled debug triangulation display.", false
            );
        } else {
            emit_status_bar_message(
                "Disabled debug triangulation display.", false
            );
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_F5) {
        debug_path_nrs = !debug_path_nrs;
        if(debug_path_nrs) {
            emit_status_bar_message(
                "Enabled debug path number display.", false
            );
        } else {
            emit_status_bar_message(
                "Disabled debug path number display.", false
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed on the canvas exclusively.
 */
void area_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
        game.cam.pos.x -= KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
        game.cam.pos.x += KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
        game.cam.pos.y -= KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
        game.cam.pos.y += KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_MINUS) {
        zoom(game.cam.zoom - (game.cam.zoom * KEYBOARD_CAM_ZOOM), false);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_EQUALS) {
        zoom(game.cam.zoom + (game.cam.zoom * KEYBOARD_CAM_ZOOM), false);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_0) {
        if(game.cam.zoom == 1.0f) {
            game.cam.pos.x = 0.0f;
            game.cam.pos.y = 0.0f;
        } else {
            zoom(1.0f, false);
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_R) {
        rotate_mob_gens_to_cursor();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_X) {
        frm_toolbar->widgets["but_snap"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
        undo_layout_drawing_node();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 */
void area_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_L && is_ctrl_pressed) {
        frm_toolbar->widgets["but_reload"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_P && is_ctrl_pressed) {
        frm_toolbar->widgets["but_play"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_Q && is_ctrl_pressed) {
        frm_toolbar->widgets["but_quit"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_R && is_ctrl_pressed) {
        frm_toolbar->widgets["but_reference"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_S && is_ctrl_pressed) {
        frm_toolbar->widgets["but_save"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_Z && is_ctrl_pressed) {
        if(sub_state == EDITOR_SUB_STATE_NONE && !selecting && !moving) {
            frm_toolbar->widgets["but_undo"]->simulate_click();
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down on the canvas exclusively.
 */
void area_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_1) {
        frm_paths->widgets["rad_one_way"]->simulate_click();
        frm_stt->widgets["rad_offset"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_2) {
        frm_paths->widgets["rad_normal"]->simulate_click();
        frm_stt->widgets["rad_scale"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_3) {
        frm_stt->widgets["rad_angle"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_A && is_ctrl_pressed) {
        if(sub_state == EDITOR_SUB_STATE_NONE && !selecting && !moving) {
            if(state == EDITOR_STATE_LAYOUT) {
                selected_edges.insert(
                    game.cur_area_data.edges.begin(),
                    game.cur_area_data.edges.end()
                );
                selected_sectors.insert(
                    game.cur_area_data.sectors.begin(),
                    game.cur_area_data.sectors.end()
                );
                selected_vertexes.insert(
                    game.cur_area_data.vertexes.begin(),
                    game.cur_area_data.vertexes.end()
                );
                sector_to_gui();
                
            } else if(state == EDITOR_STATE_MOBS) {
                selected_mobs.insert(
                    game.cur_area_data.mob_generators.begin(),
                    game.cur_area_data.mob_generators.end()
                );
                mob_to_gui();
                
            } else if(state == EDITOR_STATE_PATHS) {
                selected_path_stops.insert(
                    game.cur_area_data.path_stops.begin(),
                    game.cur_area_data.path_stops.end()
                );
                path_to_gui();
            }
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_C) {
        if(!moving && !selecting) {
            frm_layout->widgets["but_circle"]->simulate_click();
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_D) {
        if(!moving && !selecting) {
            frm_mobs->widgets["but_duplicate"]->simulate_click();
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_F) {
        frm_layout->widgets["but_sel_filter"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_N) {
        if(!moving && !selecting) {
            frm_layout->widgets["but_new"]->simulate_click();
            frm_mobs->widgets["but_new"]->simulate_click();
            frm_paths->widgets["but_draw"]->simulate_click();
            frm_details->widgets["but_new"]->simulate_click();
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_DELETE) {
        if(!moving && !selecting) {
            frm_layout->widgets["but_rem"]->simulate_click();
            frm_mobs->widgets["but_del"]->simulate_click();
            frm_paths->widgets["but_del"]->simulate_click();
            frm_details->widgets["but_del"]->simulate_click();
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_HOME) {
        bool got_something = false;
        point min_coords, max_coords;
        
        for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
            vertex* v_ptr = game.cur_area_data.vertexes[v];
            if(v_ptr->x < min_coords.x || !got_something) {
                min_coords.x = v_ptr->x;
            }
            if(v_ptr->y < min_coords.y || !got_something) {
                min_coords.y = v_ptr->y;
            }
            if(v_ptr->x > max_coords.x || !got_something) {
                max_coords.x = v_ptr->x;
            }
            if(v_ptr->y > max_coords.y || !got_something) {
                max_coords.y = v_ptr->y;
            }
            got_something = true;
        }
        
        for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
            if(m_ptr->pos.x < min_coords.x || !got_something) {
                min_coords.x = m_ptr->pos.x;
            }
            if(m_ptr->pos.y < min_coords.y || !got_something) {
                min_coords.y = m_ptr->pos.y;
            }
            if(m_ptr->pos.x > max_coords.x || !got_something) {
                max_coords.x = m_ptr->pos.x;
            }
            if(m_ptr->pos.y > max_coords.y || !got_something) {
                max_coords.y = m_ptr->pos.y;
            }
            got_something = true;
        }
        
        for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = game.cur_area_data.path_stops[s];
            if(s_ptr->pos.x < min_coords.x || !got_something) {
                min_coords.x = s_ptr->pos.x;
            }
            if(s_ptr->pos.y < min_coords.y || !got_something) {
                min_coords.y = s_ptr->pos.y;
            }
            if(s_ptr->pos.x > max_coords.x || !got_something) {
                max_coords.x = s_ptr->pos.x;
            }
            if(s_ptr->pos.y > max_coords.y || !got_something) {
                max_coords.y = s_ptr->pos.y;
            }
            got_something = true;
        }
        
        if(!got_something) return;
        
        center_camera(min_coords, max_coords);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        if(
            state == EDITOR_STATE_LAYOUT ||
            state == EDITOR_STATE_ASA ||
            state == EDITOR_STATE_ASB
        ) {
            if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
                cancel_circle_sector();
            } else if(sub_state == EDITOR_SUB_STATE_DRAWING) {
                cancel_layout_drawing();
            }
            if(sub_state == EDITOR_SUB_STATE_NONE && moving) {
                cancel_layout_moving();
            }
            if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_MOBS) {
            if(
                sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
                sub_state == EDITOR_SUB_STATE_NEW_MOB ||
                sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
                sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
            ) {
                sub_state = EDITOR_SUB_STATE_NONE;
            }
            if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_PATHS) {
            if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
                sub_state = EDITOR_SUB_STATE_NONE;
            }
            if(sub_state == EDITOR_SUB_STATE_NONE) {
                clear_selection();
                selecting = false;
            }
        } else if(state == EDITOR_STATE_DETAILS) {
            if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
                sub_state = EDITOR_SUB_STATE_NONE;
            }
            if(sub_state == EDITOR_SUB_STATE_NONE) {
                selected_shadow = NULL;
            }
        } else if(state == EDITOR_STATE_MAIN) {
            frm_toolbar->widgets["but_quit"]->simulate_click();
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked.
 */
void area_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_NONE && state == EDITOR_STATE_LAYOUT) {
        vertex* clicked_vertex = get_vertex_under_point(game.mouse_cursor_w);
        if(!clicked_vertex) {
            edge* clicked_edge = get_edge_under_point(game.mouse_cursor_w);
            if(clicked_edge) {
                register_change("edge split");
                vertex* new_vertex =
                    split_edge(clicked_edge, game.mouse_cursor_w);
                clear_selection();
                selected_vertexes.insert(new_vertex);
            }
        }
        
    } else if(
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_PATHS
    ) {
        bool clicked_stop =
            get_path_stop_under_point(game.mouse_cursor_w);
        if(!clicked_stop) {
            std::pair<path_stop*, path_stop*> clicked_link_data_1;
            std::pair<path_stop*, path_stop*> clicked_link_data_2;
            bool clicked_link =
                get_path_link_under_point(
                    game.mouse_cursor_w,
                    &clicked_link_data_1, &clicked_link_data_2
                );
            if(clicked_link) {
                register_change("path link split");
                path_stop* new_stop =
                    split_path_link(
                        clicked_link_data_1,
                        clicked_link_data_2,
                        game.mouse_cursor_w
                    );
                clear_selection();
                selected_path_stops.insert(new_stop);
            }
        }
    }
    
    handle_lmb_down(ev);
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down.
 */
void area_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_DRAWING) {
    
        //Drawing the layout.
        point hotspot = snap_point(game.mouse_cursor_w);
        
        //First, check if the user is trying to undo the previous node.
        if(
            !drawing_nodes.empty() &&
            dist(
                hotspot,
                point(
                    drawing_nodes.back().snapped_spot.x,
                    drawing_nodes.back().snapped_spot.y
                )
            ) <= VERTEX_MERGE_RADIUS / game.cam.zoom
        ) {
            undo_layout_drawing_node();
            return;
        }
        
        if(drawing_nodes.empty()) {
            //First node.
            drawing_nodes.push_back(layout_drawing_node(this, hotspot));
            
        } else {
        
            check_drawing_line(hotspot);
            
            if(drawing_line_error != DRAWING_LINE_NO_ERROR) {
                handle_line_error();
                
            } else if(
                dist(hotspot, drawing_nodes.begin()->snapped_spot) <=
                VERTEX_MERGE_RADIUS / game.cam.zoom
            ) {
                //Back to the first vertex. Finish the drawing.
                finish_layout_drawing();
                
            } else {
                //Add a new node.
                drawing_nodes.push_back(layout_drawing_node(this, hotspot));
                
            }
        }
        
        
    } else if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
    
        //Create a new circular sector.
        point hotspot = snap_point(game.mouse_cursor_w);
        
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
            for(size_t e = 0; e < new_circle_sector_valid_edges.size(); ++e) {
                if(!new_circle_sector_valid_edges[e]) {
                    all_valid = false;
                    break;
                }
            }
            if(!all_valid) {
                emit_status_bar_message(
                    "Some lines touch existing edges!", true
                );
            } else {
                finish_circle_sector();
            }
            
        }
        
    } else if(sub_state == EDITOR_SUB_STATE_NEW_MOB) {
    
        //Create a mob where the cursor is.
        register_change("object creation");
        sub_state = EDITOR_SUB_STATE_NONE;
        point hotspot = snap_point(game.mouse_cursor_w);
        
        mob_category* category_to_use = last_mob_category;
        if(!category_to_use) {
            category_to_use = mob_categories.get(MOB_CATEGORY_PIKMIN);
        }
        mob_type* type_to_use = last_mob_type;
        if(!type_to_use) {
            type_to_use = game.pikmin_order[0];
        }
        
        game.cur_area_data.mob_generators.push_back(
            new mob_gen(category_to_use, hotspot, type_to_use)
        );
        
        last_mob_category = category_to_use;
        last_mob_type = type_to_use;
        
        selected_mobs.insert(game.cur_area_data.mob_generators.back());
        mob_to_gui();
        
    } else if(sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB) {
    
        //Duplicate the current mobs to where the cursor is.
        register_change("object duplication");
        sub_state = EDITOR_SUB_STATE_NONE;
        point hotspot = snap_point(game.mouse_cursor_w);
        
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
        
        for(auto m : selected_mobs) {
            mob_gen* new_mg = new mob_gen(*m);
            new_mg->pos = point(hotspot + (m->pos) - selection_center);
            game.cur_area_data.mob_generators.push_back(new_mg);
            mobs_to_select.insert(new_mg);
        }
        
        clear_selection();
        selected_mobs = mobs_to_select;
        mob_to_gui();
        
    } else if(sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK) {
    
        //Link two mobs.
        mob_gen* target = get_mob_under_point(game.mouse_cursor_w);
        if(!target) return;
        
        for(auto m : selected_mobs) {
            if(m == target) {
                emit_status_bar_message(
                    "You can't link to an object to itself!", false
                );
                return;
            }
        }
        mob_gen* m_ptr = *(selected_mobs.begin());
        for(size_t l = 0; l < m_ptr->links.size(); ++l) {
            if(m_ptr->links[l] == target) {
                emit_status_bar_message(
                    "The object already links to that object!", false
                );
                return;
            }
        }
        
        register_change("Object link creation");
        
        m_ptr->links.push_back(target);
        m_ptr->link_nrs.push_back(game.cur_area_data.find_mob_gen_nr(target));
        
        homogenize_selected_mobs();
        
        sub_state = EDITOR_SUB_STATE_NONE;
        mob_to_gui();
        
        
    } else if(sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK) {
    
        //Delete a mob link.
        mob_gen* target = get_mob_under_point(game.mouse_cursor_w);
        mob_gen* m_ptr = *(selected_mobs.begin());
        
        if(!target) {
            std::pair<mob_gen*, mob_gen*> data1;
            std::pair<mob_gen*, mob_gen*> data2;
            if(!get_mob_link_under_point(game.mouse_cursor_w, &data1, &data2)) {
                return;
            }
            
            if(
                data1.first != m_ptr &&
                data1.second != m_ptr &&
                data2.first != m_ptr &&
                data2.second != m_ptr
            ) {
                emit_status_bar_message(
                    "That link does not belong to the current object!", false
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
            emit_status_bar_message(
                "That object is not linked by the current one!", false
            );
            return;
        } else {
            register_change("Object link deletion");
            m_ptr->links.erase(m_ptr->links.begin() + link_i);
            m_ptr->link_nrs.erase(m_ptr->link_nrs.begin() + link_i);
        }
        
        homogenize_selected_mobs();
        
        sub_state = EDITOR_SUB_STATE_NONE;
        mob_to_gui();
        
    } else if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
    
        //Drawing a path.
        point hotspot = snap_point(game.mouse_cursor_w);
        path_stop* clicked_stop = get_path_stop_under_point(hotspot);
        
        if(path_drawing_stop_1) {
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
            }
            
            if(next_stop) {
                register_change("path stop link");
                path_drawing_stop_1->add_link(
                    next_stop, path_drawing_normals
                );
                game.cur_area_data.fix_path_stop_nrs(path_drawing_stop_1);
                game.cur_area_data.fix_path_stop_nrs(next_stop);
                path_drawing_stop_1 = next_stop;
                next_stop->calculate_dists_plus_neighbors();
            }
            
        } else {
            if(clicked_stop) {
                path_drawing_stop_1 = clicked_stop;
            } else {
                register_change("path stop creation");
                path_drawing_stop_1 = new path_stop(hotspot);
                game.cur_area_data.path_stops.push_back(path_drawing_stop_1);
            }
            
        }
        
        path_preview.clear(); //Clear so it doesn't reference deleted stops.
        path_preview_timer.start(false);
        
    } else if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
    
        //Create a new shadow where the cursor is.
        register_change("tree shadow creation");
        sub_state = EDITOR_SUB_STATE_NONE;
        point hotspot = snap_point(game.mouse_cursor_w);
        
        tree_shadow* new_shadow = new tree_shadow(hotspot);
        new_shadow->bitmap = game.bmp_error;
        
        game.cur_area_data.tree_shadows.push_back(new_shadow);
        
        select_tree_shadow(new_shadow);
        details_to_gui();
        
    } else if(
        state == EDITOR_STATE_LAYOUT &&
        sub_state == EDITOR_SUB_STATE_NONE
    ) {
    
        //Start a new layout selection or select something.
        bool start_new_selection = true;
        
        vertex* clicked_vertex = NULL;
        edge* clicked_edge = NULL;
        sector* clicked_sector = NULL;
        get_clicked_layout_element(
            &clicked_vertex, &clicked_edge, &clicked_sector
        );
        
        if(!is_shift_pressed) {
            if(clicked_vertex || clicked_edge || clicked_sector) {
                start_new_selection = false;
            }
            
        }
        
        if(start_new_selection) {
            clear_selection();
            selecting = true;
            selection_start = game.mouse_cursor_w;
            selection_end = game.mouse_cursor_w;
            
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
        sector_to_gui();
        asa_to_gui();
        asb_to_gui();
        
    } else if(
        state == EDITOR_STATE_MOBS &&
        sub_state == EDITOR_SUB_STATE_NONE
    ) {
    
        //Start a new mob selection or select something.
        bool start_new_selection = true;
        mob_gen* clicked_mob = get_mob_under_point(game.mouse_cursor_w);
        
        if(!is_shift_pressed) {
            if(clicked_mob) {
                start_new_selection = false;
            }
        }
        
        if(start_new_selection) {
            clear_selection();
            selecting = true;
            selection_start = game.mouse_cursor_w;
            selection_end = game.mouse_cursor_w;
            
        } else {
            if(selected_mobs.find(clicked_mob) == selected_mobs.end()) {
                if(!is_ctrl_pressed) {
                    clear_selection();
                }
                selected_mobs.insert(clicked_mob);
            }
            
        }
        
        selection_homogenized = false;
        mob_to_gui();
        
    } else if(
        state == EDITOR_STATE_PATHS &&
        sub_state == EDITOR_SUB_STATE_NONE
    ) {
    
        //First, check if the user clicked on a path preview checkpoint.
        if(show_path_preview) {
            for(unsigned char c = 0; c < 2; ++c) {
                if(
                    bbox_check(
                        path_preview_checkpoints[c],
                        game.mouse_cursor_w,
                        PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom
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
            get_path_stop_under_point(game.mouse_cursor_w);
        std::pair<path_stop*, path_stop*> clicked_link_data_1;
        std::pair<path_stop*, path_stop*> clicked_link_data_2;
        bool clicked_link =
            get_path_link_under_point(
                game.mouse_cursor_w, &clicked_link_data_1, &clicked_link_data_2
            );
            
        if(!is_shift_pressed) {
            if(clicked_stop || clicked_link) {
                start_new_selection = false;
            }
            
        }
        
        if(start_new_selection) {
            clear_selection();
            selecting = true;
            selection_start = game.mouse_cursor_w;
            selection_end = game.mouse_cursor_w;
            
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
                    selected_path_links.find(clicked_link_data_1) ==
                    selected_path_links.end()
                ) {
                    if(!is_ctrl_pressed) {
                        clear_selection();
                    }
                    selected_path_links.insert(clicked_link_data_1);
                    if(clicked_link_data_2.first != NULL) {
                        selected_path_links.insert(clicked_link_data_2);
                    }
                }
            }
            
        }
        
        path_to_gui();
        
    } else if(
        state == EDITOR_STATE_DETAILS &&
        sub_state == EDITOR_SUB_STATE_NONE
    ) {
    
        bool transformation_handled = false;
        if(selected_shadow) {
            transformation_handled =
                selected_shadow_transformation.handle_mouse_down(
                    game.mouse_cursor_w
                );
            if(transformation_handled) {
                selected_shadow->angle =
                    selected_shadow_transformation.get_angle();
                selected_shadow->center =
                    selected_shadow_transformation.get_center();
                selected_shadow->size =
                    selected_shadow_transformation.get_size();
                details_to_gui();
            }
        }
        
        if(!transformation_handled) {
            //Select a tree shadow.
            selected_shadow = NULL;
            for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
                point min_coords, max_coords;
                get_transformed_rectangle_bounding_box(
                    s_ptr->center, s_ptr->size, s_ptr->angle,
                    &min_coords, &max_coords
                );
                
                if(
                    game.mouse_cursor_w.x >= min_coords.x &&
                    game.mouse_cursor_w.x <= max_coords.x &&
                    game.mouse_cursor_w.y >= min_coords.y &&
                    game.mouse_cursor_w.y <= max_coords.y
                ) {
                    select_tree_shadow(s_ptr);
                    break;
                }
            }
        }
        
        details_to_gui();
        
    } else if(state == EDITOR_STATE_TOOLS) {
        if(reference_bitmap) {
            reference_transformation.handle_mouse_down(game.mouse_cursor_w);
            tools_to_gui();
        }
        
    } else if(state == EDITOR_STATE_STT) {
        moving = false;
        stt_drag_start = game.mouse_cursor_w;
        stt_sector = get_sector(game.mouse_cursor_w, NULL, false);
        if(stt_sector) {
            moving = true;
            stt_orig_angle = stt_sector->texture_info.rot;
            stt_orig_offset = stt_sector->texture_info.translation;
            stt_orig_scale = stt_sector->texture_info.scale;
        }
        
    } else if(state == EDITOR_STATE_REVIEW && show_cross_section) {
        moving_cross_section_point = -1;
        for(unsigned char p = 0; p < 2; ++p) {
            if(
                bbox_check(
                    cross_section_checkpoints[p], game.mouse_cursor_w,
                    CROSS_SECTION_POINT_RADIUS / game.cam.zoom
                )
            ) {
                moving_cross_section_point = p;
                break;
            }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged.
 */
void area_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(selecting) {
    
        float selection_x1 = std::min(selection_start.x, selection_end.x);
        float selection_x2 = std::max(selection_start.x, selection_end.x);
        float selection_y1 = std::min(selection_start.y, selection_end.y);
        float selection_y2 = std::max(selection_start.y, selection_end.y);
        selection_end = game.mouse_cursor_w;
        
        if(state == EDITOR_STATE_LAYOUT) {
        
            //Selection box around the layout.
            clear_selection();
            
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
            sector_to_gui();
            asa_to_gui();
            asb_to_gui();
            
        } else if(state == EDITOR_STATE_MOBS) {
        
            //Selection box around mobs.
            clear_selection();
            
            for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
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
            mob_to_gui();
            
        } else if(state == EDITOR_STATE_PATHS) {
        
            //Selection box around path stops.
            clear_selection();
            
            for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = game.cur_area_data.path_stops[s];
                
                if(
                    s_ptr->pos.x - PATH_STOP_RADIUS >= selection_x1 &&
                    s_ptr->pos.x + PATH_STOP_RADIUS <= selection_x2 &&
                    s_ptr->pos.y - PATH_STOP_RADIUS >= selection_y1 &&
                    s_ptr->pos.y + PATH_STOP_RADIUS <= selection_y2
                ) {
                    selected_path_stops.insert(s_ptr);
                }
            }
            
            for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = game.cur_area_data.path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); ++l) {
                    path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                    
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
                        selected_path_links.insert(
                            std::make_pair(s_ptr, s2_ptr)
                        );
                    }
                }
            }
            
            path_to_gui();
            
        }
        
    } else if(
        !selected_vertexes.empty() &&
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_LAYOUT
    ) {
    
        //Move vertexes.
        if(!moving) {
            start_vertex_move();
        }
        
        point mouse_offset = game.mouse_cursor_w - move_mouse_start_pos;
        point closest_vertex_new_p =
            snap_point(move_closest_vertex_start_pos + mouse_offset);
        point offset = closest_vertex_new_p - move_closest_vertex_start_pos;
        for(
            auto v = selected_vertexes.begin();
            v != selected_vertexes.end(); ++v
        ) {
            point orig = pre_move_vertex_coords[*v];
            (*v)->x = orig.x + offset.x;
            (*v)->y = orig.y + offset.y;
        }
        
    } else if(
        !selected_mobs.empty() &&
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_MOBS
    ) {
    
        //Move mobs.
        if(!moving) {
            start_mob_move();
        }
        
        point mouse_offset = game.mouse_cursor_w - move_mouse_start_pos;
        point closest_mob_new_p =
            snap_point(move_closest_mob_start_pos + mouse_offset);
        point offset = closest_mob_new_p - move_closest_mob_start_pos;
        for(
            auto m = selected_mobs.begin();
            m != selected_mobs.end(); ++m
        ) {
            point orig = pre_move_mob_coords[*m];
            (*m)->pos = orig + offset;
        }
        
    } else if(
        !selected_path_stops.empty() &&
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_PATHS
    ) {
    
        //Move path stops.
        if(!moving) {
            start_path_stop_move();
        }
        
        point mouse_offset = game.mouse_cursor_w - move_mouse_start_pos;
        point closest_stop_new_p =
            snap_point(move_closest_stop_start_pos + mouse_offset);
        point offset = closest_stop_new_p - move_closest_stop_start_pos;
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
        selected_shadow &&
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_DETAILS
    ) {
    
        if(selected_shadow) {
            area_data* prepared_state = prepare_state();
            if(
                !selected_shadow_transformation.handle_mouse_move(
                    snap_point(game.mouse_cursor_w)
                )
            ) {
                forget_prepared_state(prepared_state);
            } else {
                register_change("tree shadow transformation", prepared_state);
            }
            selected_shadow->angle =
                selected_shadow_transformation.get_angle();
            selected_shadow->center =
                selected_shadow_transformation.get_center();
            selected_shadow->size =
                selected_shadow_transformation.get_size();
            details_to_gui();
            
        }
        
    } else if(
        moving_path_preview_checkpoint != -1 &&
        sub_state == EDITOR_SUB_STATE_NONE &&
        state == EDITOR_STATE_PATHS
    ) {
    
        //Move path preview checkpoints.
        path_preview_checkpoints[moving_path_preview_checkpoint] =
            snap_point(game.mouse_cursor_w);
        path_preview_timer.start(false);
        
    } else if(state == EDITOR_STATE_TOOLS) {
        //Move reference handle.
        reference_transformation.handle_mouse_move(
            snap_point(game.mouse_cursor_w)
        );
        tools_to_gui();
        
    } else if(state == EDITOR_STATE_STT) {
        //Move sector texture transformation property.
        if(stt_sector && moving) {
            if(stt_mode == 0) {
                register_change("texture offset change");
                point diff = (game.mouse_cursor_w - stt_drag_start);
                diff = rotate_point(diff, -stt_sector->texture_info.rot);
                diff = diff / stt_sector->texture_info.scale;
                stt_sector->texture_info.translation = stt_orig_offset + diff;
            } else if(stt_mode == 1) {
                register_change("texture scale change");
                point diff = (game.mouse_cursor_w - stt_drag_start);
                diff = rotate_point(diff, -stt_sector->texture_info.rot);
                point drag_start_rot =
                    rotate_point(stt_drag_start, -stt_sector->texture_info.rot);
                diff = diff / drag_start_rot * stt_orig_scale;
                stt_sector->texture_info.scale = stt_orig_scale + diff;
            } else {
                register_change("texture angle change");
                float drag_start_a = get_angle(point(), stt_drag_start);
                float cursor_a = get_angle(point(), game.mouse_cursor_w);
                stt_sector->texture_info.rot =
                    stt_orig_angle + (cursor_a - drag_start_a);
            }
        }
        
    } else if(state == EDITOR_STATE_REVIEW) {
        //Move cross-section points.
        if(moving_cross_section_point != -1) {
            cross_section_checkpoints[moving_cross_section_point] =
                snap_point(game.mouse_cursor_w);
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 */
void area_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    selecting = false;
    
    if(moving) {
        if(state == EDITOR_STATE_LAYOUT) {
            finish_layout_moving();
        }
        moving = false;
    }
    
    if(state == EDITOR_STATE_DETAILS && selected_shadow) {
        selected_shadow_transformation.handle_mouse_up();
        
    } else if(state == EDITOR_STATE_TOOLS) {
        reference_transformation.handle_mouse_up();
        tools_to_gui();
    }
    
    moving_path_preview_checkpoint = -1;
    moving_cross_section_point = -1;
    stt_sector = NULL;
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked.
 */
void area_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down.
 */
void area_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged.
 */
void area_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
 */
void area_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    game.mouse_cursor_s.x = ev.mouse.x;
    game.mouse_cursor_s.y = ev.mouse.y;
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
    
    update_status_bar();
    
    if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        point hotspot = snap_point(game.mouse_cursor_w);
        if(new_circle_sector_step == 1) {
            new_circle_sector_anchor = hotspot;
        } else {
            set_new_circle_sector_points();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved.
 */
void area_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being double-clicked.
 */
void area_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void area_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void area_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 */
void area_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.pos.x -= ev.mouse.dx / game.cam.zoom;
    game.cam.pos.y -= ev.mouse.dy / game.cam.zoom;
}


/* ----------------------------------------------------------------------------
 * Resets the camera's X and Y coordinates.
 */
void area_editor::reset_cam_xy(const ALLEGRO_EVENT &ev) {
    game.cam.pos.x = 0;
    game.cam.pos.y = 0;
}


/* ----------------------------------------------------------------------------
 * Resets the camera's zoom.
 */
void area_editor::reset_cam_zoom(const ALLEGRO_EVENT &ev) {
    zoom(1.0f);
}
