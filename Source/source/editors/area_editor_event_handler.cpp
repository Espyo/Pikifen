/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor event handler function.
 */

#include "area_editor.h"
#include "../functions.h"
#include "../geometry_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Handles the events for the area editor.
 */
void area_editor::handle_controls(const ALLEGRO_EVENT &ev) {

    if(fade_mgr.is_fading()) return;
    
    gui->handle_event(ev);
    
    //Update mouse cursor in world coordinates.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x =
            ev.mouse.x / cam_zoom - cam_x - (gui_x / 2 / cam_zoom);
        mouse_cursor_y =
            ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* wum;
        if(!is_mouse_in_gui(ev.mouse.x, ev.mouse.y)) {
            wum = NULL;
        } else {
            wum = gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y);
        }
        ((lafi::label*) gui->widgets["lbl_status_bar"])->text =
            (
                wum ?
                wum->description :
                "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")"
            );
    }
    
    
    //Moving vertexes, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        if(
            !is_mouse_in_gui(ev.mouse.x, ev.mouse.y)
            && moving_thing == INVALID && sec_mode != ESM_TEXTURE_VIEW &&
            mode != EDITOR_MODE_OBJECTS
        ) {
            on_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL, false);
        } else {
            on_sector = NULL;
        }
        
        //Move guide.
        if(sec_mode == ESM_GUIDE_MOUSE) {
        
            if(holding_m1) {
                guide_x += ev.mouse.dx / cam_zoom;
                guide_y += ev.mouse.dy / cam_zoom;
                
            } else if(holding_m2) {
            
                float new_w = guide_w + ev.mouse.dx / cam_zoom;
                float new_h = guide_h + ev.mouse.dy / cam_zoom;
                
                if(guide_aspect_ratio) {
                    //Find the most significant change.
                    if(ev.mouse.dx != 0 || ev.mouse.dy != 0) {
                        bool most_is_width =
                            fabs((double) ev.mouse.dx) >
                            fabs((double) ev.mouse.dy);
                            
                        if(most_is_width) {
                            float ratio = guide_h / guide_w;
                            guide_w = new_w;
                            guide_h = new_w * ratio;
                        } else {
                            float ratio = guide_w / guide_h;
                            guide_h = new_h;
                            guide_w = new_h * ratio;
                        }
                    }
                } else {
                    guide_w = new_w;
                    guide_h = new_h;
                }
                
            }
            
            guide_to_gui();
            
        } else if(holding_m2) {
            //Move camera.
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        //Move thing.
        if(moving_thing != INVALID) {
            if(mode == EDITOR_MODE_SECTORS) {
                vertex* v_ptr = cur_area_data.vertexes[moving_thing];
                v_ptr->x = snap_to_grid(mouse_cursor_x);
                v_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(mode == EDITOR_MODE_OBJECTS) {
                mob_gen* m_ptr = cur_area_data.mob_generators[moving_thing];
                m_ptr->x = snap_to_grid(mouse_cursor_x);
                m_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(mode == EDITOR_MODE_FOLDER_PATHS) {
                path_stop* s_ptr = cur_area_data.path_stops[moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y);
                s_ptr->calculate_dists();
                path_preview_timeout.start(false);
            } else if(mode == EDITOR_MODE_SHADOWS) {
                tree_shadow* s_ptr = cur_area_data.tree_shadows[moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x - moving_thing_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y - moving_thing_y);
                shadow_to_gui();
            }
            
            made_changes = true;
        }
        
        //Move path checkpoints.
        if(moving_path_preview_checkpoint != -1) {
            path_preview_checkpoints_x[moving_path_preview_checkpoint] =
                snap_to_grid(mouse_cursor_x);
            path_preview_checkpoints_y[moving_path_preview_checkpoint] =
                snap_to_grid(mouse_cursor_y);
            path_preview_timeout.start(false);
        }
        
        //Move cross-section points.
        if(moving_cross_section_point != -1) {
            cross_section_points[moving_cross_section_point].x =
                snap_to_grid(mouse_cursor_x);
            cross_section_points[moving_cross_section_point].y =
                snap_to_grid(mouse_cursor_y);
        }
        
        
        if(ev.mouse.dz != 0 && !is_mouse_in_gui(ev.mouse.x, ev.mouse.y)) {
            //Zoom.
            float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
            new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
            new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
            float new_mc_x =
                ev.mouse.x / new_zoom - cam_x - (gui_x / 2 / new_zoom);
            float new_mc_y =
                ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
                
            cam_x -= (mouse_cursor_x - new_mc_x);
            cam_y -= (mouse_cursor_y - new_mc_y);
            mouse_cursor_x = new_mc_x;
            mouse_cursor_y = new_mc_y;
            cam_zoom = new_zoom;
        }
        
        if(sec_mode == ESM_NEW_SECTOR) {
            new_sector_valid_line =
                is_new_sector_line_valid(
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y)
                );
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui(ev.mouse.x, ev.mouse.y)
    ) {
        //Clicking.
        
        if(ev.mouse.button == 1) holding_m1 = true;
        else if(ev.mouse.button == 2) holding_m2 = true;
        else if(ev.mouse.button == 3) cam_zoom = 1.0;
        
        if(ev.mouse.button != 1) return;
        
        //If the user was editing, save it.
        if(mode == EDITOR_MODE_SECTORS) {
            gui_to_sector();
        } else if(mode == EDITOR_MODE_OBJECTS) {
            gui_to_mob();
        } else if(mode == EDITOR_MODE_SHADOWS) {
            gui_to_shadow();
        }
        
        //Sector-related clicking.
        if(sec_mode == ESM_NONE && mode == EDITOR_MODE_SECTORS) {
        
            moving_thing = INVALID;
            
            edge* clicked_edge_ptr = NULL;
            size_t clicked_edge_nr = INVALID;
            bool created_vertex = false;
            
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                
                if(!is_edge_valid(e_ptr)) continue;
                
                if(
                    circle_intersects_line(
                        mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    )
                ) {
                    clicked_edge_ptr = e_ptr;
                    clicked_edge_nr = e;
                    break;
                }
            }
            
            if(double_click_time == 0) double_click_time = 0.5;
            else if(clicked_edge_ptr) {
                //Create a new vertex.
                double_click_time = 0;
                
                //New vertex, on the split point.
                //TODO create it on the edge, not on the cursor.
                vertex* new_v_ptr = new vertex(mouse_cursor_x, mouse_cursor_y);
                cur_area_data.vertexes.push_back(new_v_ptr);
                
                //New edge, copied from the original one.
                edge* new_e_ptr = new edge(*clicked_edge_ptr);
                cur_area_data.edges.push_back(new_e_ptr);
                
                //Save the original end vertex for later.
                vertex* end_v_ptr = clicked_edge_ptr->vertexes[1];
                
                //Set vertexes on the new and original edges.
                new_e_ptr->vertex_nrs[0] = cur_area_data.vertexes.size() - 1;
                new_e_ptr->vertexes[0] = new_v_ptr;
                clicked_edge_ptr->vertex_nrs[1] = new_e_ptr->vertex_nrs[0];
                clicked_edge_ptr->vertexes[1] = new_v_ptr;
                
                //Set sectors on the new edge.
                if(new_e_ptr->sectors[0]) {
                    new_e_ptr->sectors[0]->edge_nrs.push_back(
                        cur_area_data.edges.size() - 1
                    );
                    new_e_ptr->sectors[0]->edges.push_back(new_e_ptr);
                }
                if(new_e_ptr->sectors[1]) {
                    new_e_ptr->sectors[1]->edge_nrs.push_back(
                        cur_area_data.edges.size() - 1
                    );
                    new_e_ptr->sectors[1]->edges.push_back(new_e_ptr);
                }
                
                //Set edges of the new vertex.
                new_v_ptr->edge_nrs.push_back(cur_area_data.edges.size() - 1);
                new_v_ptr->edge_nrs.push_back(clicked_edge_nr);
                new_v_ptr->edges.push_back(new_e_ptr);
                new_v_ptr->edges.push_back(clicked_edge_ptr);
                
                //Update edge data on the end vertex of the original edge
                //(it now links to the new edge, not the old).
                for(size_t ve = 0; ve < end_v_ptr->edges.size(); ++ve) {
                    if(end_v_ptr->edges[ve] == clicked_edge_ptr) {
                        end_v_ptr->edges[ve] =
                            new_e_ptr;
                        end_v_ptr->edge_nrs[ve] =
                            cur_area_data.edges.size() - 1;
                        break;
                    }
                }
                
                //Start dragging the new vertex.
                moving_thing = cur_area_data.vertexes.size() - 1;
                
                created_vertex = true;
                made_changes = true;
            }
            
            //Find a vertex to drag.
            if(!created_vertex) {
                for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                    if(
                        dist(
                            mouse_cursor_x, mouse_cursor_y,
                            cur_area_data.vertexes[v]->x,
                            cur_area_data.vertexes[v]->y
                        ) <= 6.0 / cam_zoom
                    ) {
                        moving_thing = v;
                        break;
                    }
                }
            }
            
            //Find a sector to select.
            if(moving_thing == INVALID) {
                cur_sector =
                    get_sector(mouse_cursor_x, mouse_cursor_y, NULL, false);
                sector_to_gui();
            }
            
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_OBJECTS) {
            //Object-related clicking.
            
            cur_mob = NULL;
            moving_thing = INVALID;
            for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
                mob_gen* m_ptr = cur_area_data.mob_generators[m];
                float radius =
                    m_ptr->type ? m_ptr->type->radius == 0 ? 16 :
                    m_ptr->type->radius : 16;
                if(
                    dist(m_ptr->x, m_ptr->y, mouse_cursor_x, mouse_cursor_y) <=
                    radius
                ) {
                
                    cur_mob = m_ptr;
                    moving_thing = m;
                    break;
                }
            }
            mob_to_gui();
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_FOLDER_PATHS) {
            //Path-related clicking.
            
            cur_stop = NULL;
            moving_thing = INVALID;
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                if(
                    dist(s_ptr->x, s_ptr->y, mouse_cursor_x, mouse_cursor_y)
                    <= STOP_RADIUS
                ) {
                
                    cur_stop = s_ptr;
                    moving_thing = s;
                    break;
                }
            }
            
            moving_path_preview_checkpoint = -1;
            if(show_path_preview) {
                for(unsigned char c = 0; c < 2; ++c) {
                    if(
                        bbox_check(
                            path_preview_checkpoints_x[c],
                            path_preview_checkpoints_y[c],
                            mouse_cursor_x, mouse_cursor_y,
                            PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom
                        )
                    ) {
                        moving_path_preview_checkpoint = c;
                        break;
                    }
                }
            }
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_SHADOWS) {
            //Shadow-related clicking.
            
            cur_shadow = NULL;
            moving_thing = INVALID;
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                float min_x, min_y, max_x, max_y;
                get_shadow_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
                
                if(
                    mouse_cursor_x >= min_x && mouse_cursor_x <= max_x &&
                    mouse_cursor_y >= min_y && mouse_cursor_y <= max_y
                ) {
                    cur_shadow = s_ptr;
                    moving_thing = s;
                    moving_thing_x = mouse_cursor_x - s_ptr->x;
                    moving_thing_y = mouse_cursor_y - s_ptr->y;
                    break;
                }
            }
            shadow_to_gui();
            
        }
        
        if(sec_mode == ESM_NEW_SECTOR) {
            //Next vertex in a new sector.
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            //First, check if the user is trying to undo the previous vertex.
            if(
                !new_sector_vertexes.empty() &&
                dist(
                    hotspot_x, hotspot_y,
                    new_sector_vertexes.back()->x,
                    new_sector_vertexes.back()->y
                ) <= cam_zoom / VERTEX_MERGE_RADIUS
            ) {
                new_sector_vertexes.erase(
                    new_sector_vertexes.begin() + new_sector_vertexes.size() - 1
                );
                return;
            }
            
            new_sector_valid_line =
                is_new_sector_line_valid(
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y)
                );
                
            if(new_sector_valid_line) {
                if(
                    !new_sector_vertexes.empty() &&
                    dist(
                        hotspot_x, hotspot_y,
                        new_sector_vertexes[0]->x,
                        new_sector_vertexes[0]->y
                    ) <= cam_zoom / VERTEX_MERGE_RADIUS
                ) {
                    //Back to the first vertex.
                    sec_mode = ESM_NONE;
                    create_sector();
                    sector_to_gui();
                    made_changes = true;
                } else {
                    //Add a new vertex.
                    vertex* merge =
                        get_merge_vertex(
                            hotspot_x, hotspot_y,
                            cur_area_data.vertexes,
                            VERTEX_MERGE_RADIUS / cam_zoom
                        );
                    if(merge) {
                        new_sector_vertexes.push_back(
                            new vertex(merge->x, merge->y)
                        );
                    } else {
                        new_sector_vertexes.push_back(
                            new vertex(hotspot_x, hotspot_y)
                        );
                    }
                }
            }
            
            
        } else if(sec_mode == ESM_NEW_OBJECT) {
            //Create a mob where the cursor is.
            
            sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            cur_area_data.mob_generators.push_back(
                new mob_gen(hotspot_x, hotspot_y)
            );
            
            cur_mob = cur_area_data.mob_generators.back();
            mob_to_gui();
            made_changes = true;
            
        } else if(sec_mode == ESM_DUPLICATE_OBJECT) {
            //Duplicate the current mob to where the cursor is.
            
            sec_mode = ESM_NONE;
            
            if(cur_mob) {
                float hotspot_x = snap_to_grid(mouse_cursor_x);
                float hotspot_y = snap_to_grid(mouse_cursor_y);
                
                mob_gen* new_mg = new mob_gen(*cur_mob);
                new_mg->x = hotspot_x;
                new_mg->y = hotspot_y;
                cur_area_data.mob_generators.push_back(
                    new_mg
                );
                
                cur_mob = new_mg;
                mob_to_gui();
                made_changes = true;
            }
            
        } else if(sec_mode == ESM_NEW_STOP) {
            //Create a new stop where the cursor is.
            
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            cur_area_data.path_stops.push_back(
                new path_stop(hotspot_x, hotspot_y, vector<path_link>())
            );
            
            cur_stop = cur_area_data.path_stops.back();
            made_changes = true;
            
            
        } else if (sec_mode == ESM_NEW_LINK1 || sec_mode == ESM_NEW_1WLINK1) {
            //Pick a stop to start the link on.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                    new_link_first_stop = s_ptr;
                    sec_mode =
                        sec_mode == ESM_NEW_LINK1 ? ESM_NEW_LINK2 :
                        ESM_NEW_1WLINK2;
                    break;
                }
            }
            
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if (sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK2) {
            //Pick a stop to end the link on.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                
                    if(new_link_first_stop == s_ptr) continue;
                    
                    //Check if these two stops already have a link.
                    //Delete it if so.
                    for(
                        size_t l = 0; l < new_link_first_stop->links.size();
                        ++l
                    ) {
                        if(new_link_first_stop->links[l].end_ptr == s_ptr) {
                            new_link_first_stop->links.erase(
                                new_link_first_stop->links.begin() + l
                            );
                            break;
                        }
                    }
                    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
                        if(s_ptr->links[l].end_ptr == new_link_first_stop) {
                            s_ptr->links.erase(s_ptr->links.begin() + l);
                            break;
                        }
                    }
                    
                    
                    new_link_first_stop->links.push_back(
                        path_link(s_ptr, s)
                    );
                    
                    if(sec_mode == ESM_NEW_LINK2) {
                        s_ptr->links.push_back(
                            path_link(new_link_first_stop, INVALID)
                        );
                        s_ptr->fix_nrs(cur_area_data);
                    }
                    
                    new_link_first_stop->calculate_dists();
                    
                    sec_mode =
                        sec_mode == ESM_NEW_LINK2 ? ESM_NEW_LINK1 :
                        ESM_NEW_1WLINK1;
                    break;
                }
            }
            
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_STOP) {
            //Pick a stop to delete.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                
                    //Check all links to this stop.
                    for(
                        size_t s2 = 0; s2 < cur_area_data.path_stops.size();
                        ++s2
                    ) {
                        path_stop* s2_ptr = cur_area_data.path_stops[s2];
                        for(size_t l = 0; l < s2_ptr->links.size(); ++l) {
                            if(s2_ptr->links[l].end_ptr == s_ptr) {
                                s2_ptr->links.erase(s2_ptr->links.begin() + l);
                                break;
                            }
                        }
                    }
                    
                    //Finally, delete the stop.
                    delete s_ptr;
                    cur_area_data.path_stops.erase(
                        cur_area_data.path_stops.begin() + s
                    );
                    break;
                }
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                cur_area_data.path_stops[s]->fix_nrs(cur_area_data);
            }
            
            path_preview.clear();
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_LINK) {
            //Pick a link to delete.
            
            bool deleted = false;
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                for(size_t s2 = 0; s2 < s_ptr->links.size(); ++s2) {
                    path_stop* s2_ptr = s_ptr->links[s2].end_ptr;
                    if(
                        circle_intersects_line(
                            mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                            s_ptr->x, s_ptr->y,
                            s2_ptr->x, s2_ptr->y
                        )
                    ) {
                    
                        s_ptr->links.erase(s_ptr->links.begin() + s2);
                        
                        for(size_t s3 = 0; s3 < s2_ptr->links.size(); ++s3) {
                            if(s2_ptr->links[s3].end_ptr == s_ptr) {
                                s2_ptr->links.erase(
                                    s2_ptr->links.begin() + s3
                                );
                                break;
                            }
                        }
                        
                        deleted = true;
                        break;
                    }
                }
                
                if(deleted) break;
            }
            
            path_preview.clear();
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if(sec_mode == ESM_NEW_SHADOW) {
            //Create a new shadow where the cursor is.
            
            sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            tree_shadow* new_shadow = new tree_shadow(hotspot_x, hotspot_y);
            new_shadow->bitmap = bmp_error;
            
            cur_area_data.tree_shadows.push_back(new_shadow);
            
            cur_shadow = new_shadow;
            shadow_to_gui();
            made_changes = true;
            
        } else if(mode == EDITOR_MODE_REVIEW && show_cross_section) {
            moving_cross_section_point = -1;
            for(unsigned char p = 0; p < 2; ++p) {
                if(
                    bbox_check(
                        cross_section_points[p].x,
                        cross_section_points[p].y,
                        mouse_cursor_x, mouse_cursor_y,
                        CROSS_SECTION_POINT_RADIUS / cam_zoom
                    )
                ) {
                    moving_cross_section_point = p;
                    break;
                }
            }
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        //Mouse button release.
        
        if(ev.mouse.button == 1) holding_m1 = false;
        else if(ev.mouse.button == 2) holding_m2 = false;
        
        if(
            ev.mouse.button == 1 &&
            mode == EDITOR_MODE_SECTORS && sec_mode == ESM_NONE &&
            moving_thing != INVALID
        ) {
            //Release the vertex.
            
            vertex* moved_v_ptr = cur_area_data.vertexes[moving_thing];
            vertex* final_vertex = moved_v_ptr;
            
            unordered_set<sector*> affected_sectors;
            
            //Check if we should merge.
            for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                vertex* dest_v_ptr = cur_area_data.vertexes[v];
                if(dest_v_ptr == moved_v_ptr) continue;
                
                if(
                    dist(
                        moved_v_ptr->x, moved_v_ptr->y,
                        dest_v_ptr->x, dest_v_ptr->y
                    ) <= (VERTEX_MERGE_RADIUS / cam_zoom)
                ) {
                    merge_vertex(
                        moved_v_ptr, dest_v_ptr, &affected_sectors
                    );
                    final_vertex = dest_v_ptr;
                    break;
                }
            }
            
            //Finally, re-triangulate the affected sectors.
            for(size_t e = 0; e < final_vertex->edges.size(); ++e) {
                edge* e_ptr = final_vertex->edges[e];
                for(size_t s = 0; s < 2; ++s) {
                    if(e_ptr->sectors[s]) {
                        affected_sectors.insert(e_ptr->sectors[s]);
                    }
                }
            }
            for(
                auto s = affected_sectors.begin();
                s != affected_sectors.end(); ++s
            ) {
                if(!(*s)) continue;
                triangulate(*s);
            }
            
            //If somewhere along the line, the current sector
            //got marked for deletion, unselect it.
            if(cur_sector) {
                if(cur_sector->edges.empty()) {
                    cur_sector = NULL;
                    sector_to_gui();
                }
            }
            
            //Check if the edge's vertexes intersect with any other edges.
            //If so, they're marked with red.
            check_edge_intersections(moved_v_ptr);
            
            moving_thing = INVALID;
            
            
            
        } else if(
            ev.mouse.button == 1 && sec_mode == ESM_NONE &&
            moving_thing != INVALID
        ) {
            //Release thing.
            
            moving_thing = INVALID;
            
        }
        
        moving_path_preview_checkpoint = -1;
        moving_cross_section_point = -1;
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key press.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            shift_pressed = true;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F1) {
            debug_edge_nrs = !debug_edge_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F2) {
            debug_sector_nrs = !debug_sector_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F3) {
            debug_vertex_nrs = !debug_vertex_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F4) {
            debug_triangulation = !debug_triangulation;
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //Key release.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            shift_pressed = false;
        }
    }
}
