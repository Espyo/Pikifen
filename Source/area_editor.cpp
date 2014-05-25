/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor-related functions.
 */

#include <iomanip>
#include <unordered_set>

#include <allegro5/allegro_primitives.h>

#include "area_editor.h"
#include "functions.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/gui.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "vars.h"

void area_editor::change_to_right_frame() {
    hide_widget(ed_gui->widgets["frm_main"]);
    hide_widget(ed_gui->widgets["frm_picker"]);
    hide_widget(ed_gui->widgets["frm_sectors"]);
    hide_widget(ed_gui->widgets["frm_bg"]);
    
    if(ed_mode == EDITOR_MODE_MAIN) {
        show_widget(ed_gui->widgets["frm_main"]);
    } else if(ed_mode == EDITOR_MODE_SECTORS) {
        show_widget(ed_gui->widgets["frm_sectors"]);
    } else if(ed_mode == EDITOR_MODE_BG) {
        show_widget(ed_gui->widgets["frm_bg"]);
    }
}

/* ----------------------------------------------------------------------------
 * Handles the main loop, both logic and drawing.
 */
void area_editor::do_logic() {

    //---Logic---
    
    if(ed_double_click_time > 0) {
        ed_double_click_time -= delta_t;
        if(ed_double_click_time < 0) ed_double_click_time = 0;
    }
    
    lafi_widget* wum = NULL; //Widget under mouse.
    wum = ed_gui->widgets["frm_bottom"]->mouse_over_widget;
    if(!wum) {
        if(!(ed_gui->widgets["frm_picker"]->flags & LAFI_FLAG_DISABLED)) {
            wum = ed_gui->widgets["frm_picker"]->mouse_over_widget;
        } else if(ed_mode == EDITOR_MODE_MAIN) {
            wum = ed_gui->widgets["frm_main"]->mouse_over_widget;
        }
    }
    
    ((lafi_label*) ed_gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + itos(mouse_cursor_x) + "," + itos(mouse_cursor_y) + ")");
    
    //---Drawing---
    
    ed_gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
        al_clear_to_color(al_map_rgb(0, 0, 16));
        
        //Grid.
        float grid_interval = 32;
        float grid_interval_2 = 64;
        
        float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
        float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
        float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
        float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
        
        if(cam_zoom >= ZOOM_MIN_LEVEL_EDITOR * 1.5) {
            float x = floor(cam_leftmost / grid_interval) * grid_interval;
            while(x < cam_rightmost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(255, 255, 255);
                bool draw_line = true;
                
                if(fmod(x, grid_interval_2) == 0) {
                    c = al_map_rgb(0, 96, 160);
                } else {
                    if(cam_zoom > ZOOM_MIN_LEVEL_EDITOR * 4) {
                        c = al_map_rgb(0, 64, 128);
                    } else {
                        draw_line = false;
                    }
                }
                
                if(draw_line) al_draw_line(x, cam_topmost, x, cam_bottommost + grid_interval, al_map_rgb(0, 64, 128), 1.0 / cam_zoom);
                x += grid_interval;
            }
            
            float y = floor(cam_topmost / grid_interval) * grid_interval;
            while(y < cam_bottommost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(255, 255, 255);
                bool draw_line = true;
                
                if(fmod(y, grid_interval_2) == 0) {
                    c = al_map_rgb(0, 96, 160);
                } else {
                    if(cam_zoom > ZOOM_MIN_LEVEL_EDITOR * 4) {
                        c = al_map_rgb(0, 64, 128);
                    } else {
                        draw_line = false;
                    }
                }
                
                if(draw_line) al_draw_line(cam_leftmost, y, cam_rightmost + grid_interval, y, c, 1.0 / cam_zoom);
                y += grid_interval;
            }
        }
        
        //0,0 marker.
        al_draw_line(-(grid_interval * 2), 0, grid_interval * 2, 0, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        al_draw_line(0, -(grid_interval * 2), 0, grid_interval * 2, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        
        //Linedefs.
        size_t n_linedefs = cur_area_map.linedefs.size();
        for(size_t l = 0; l < n_linedefs; l++) {
            linedef* l_ptr = cur_area_map.linedefs[l];
            
            if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
            
            bool one_sided = true;
            if(l_ptr->sectors[0] && l_ptr->sectors[1]) one_sided = false;
            
            bool valid = true;
            for(size_t il = 0; il < ed_intersecting_lines.size(); il++) {
                if(ed_intersecting_lines[il].contains(l_ptr)) {
                    valid = false;
                    break;
                }
            }
            
            al_draw_line(
                l_ptr->vertices[0]->x,
                l_ptr->vertices[0]->y,
                l_ptr->vertices[1]->x,
                l_ptr->vertices[1]->y,
                (one_sided ? al_map_rgb(240, 240, 240) :
                 valid ? al_map_rgb(160, 160, 160) :
                 al_map_rgb(192, 64, 64)
                ),
                2.0 / cam_zoom
            );
            
            //Debug: uncomment this to show the sector numbers on each side.
            //Orientantion could be wrong, as there is no concept of front/back sector.
            float mid_x = (l_ptr->vertices[0]->x + l_ptr->vertices[1]->x) / 2;
            float mid_y = (l_ptr->vertices[0]->y + l_ptr->vertices[1]->y) / 2;
            float angle = atan2(l_ptr->vertices[0]->y - l_ptr->vertices[1]->y, l_ptr->vertices[0]->x - l_ptr->vertices[1]->x);
            al_draw_text(
                font, al_map_rgb(192, 255, 192),
                mid_x + cos(angle - M_PI_2) * 15,
                mid_y + sin(angle - M_PI_2) * 15 - font_h / 2,
                ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[0] == string::npos ? "--" : itos(l_ptr->sector_nrs[0]).c_str());
            al_draw_text(
                font, al_map_rgb(192, 255, 192),
                mid_x + cos(angle + M_PI_2) * 15,
                mid_y + sin(angle + M_PI_2) * 15 - font_h / 2,
                ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[1] == string::npos ? "--" : itos(l_ptr->sector_nrs[1]).c_str());
        }
        
        //Vertices.
        size_t n_vertices = cur_area_map.vertices.size();
        for(size_t v = 0; v < n_vertices; v++) {
            vertex* v_ptr = cur_area_map.vertices[v];
            al_draw_filled_circle(
                v_ptr->x,
                v_ptr->y,
                3.0 / cam_zoom,
                al_map_rgb(224, 224, 224)
            );
        }
        
        //New sector marker.
        if(ed_new_sector_mode) {
            float x = snap_to_grid(mouse_cursor_x);
            float y = snap_to_grid(mouse_cursor_y);
            al_draw_line(x - 16, y,      x + 16, y,      al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(x,      y - 16, x,      y + 16, al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
        }
        
        //ToDo temp stuff.
        /*for(size_t v = 0; v < ed_temp_o.size(); v++) {
            al_draw_text(font, al_map_rgb(255, 255, 255), ed_temp_o[v]->x, ed_temp_o[v]->y - font_h, ALLEGRO_ALIGN_CENTER, ("O" + to_string((long long) v)).c_str());
        }
        for(size_t i = 0; i < ed_temp_i.size(); i++) {
            for(size_t v = 0; v < ed_temp_i[i].size(); v++) {
                al_draw_text(font, al_map_rgb(255, 255, 255), ed_temp_i[i][v]->x, ed_temp_i[i][v]->y - font_h * 2, ALLEGRO_ALIGN_CENTER, ("I" + to_string((long long) v)).c_str());
            }
        }*/
        for(size_t t = 0; t < cur_area_map.sectors[0]->triangles.size(); t++) {
            al_draw_triangle(
                cur_area_map.sectors[0]->triangles[t].points[0]->x,
                cur_area_map.sectors[0]->triangles[t].points[0]->y,
                cur_area_map.sectors[0]->triangles[t].points[1]->x,
                cur_area_map.sectors[0]->triangles[t].points[1]->y,
                cur_area_map.sectors[0]->triangles[t].points[2]->x,
                cur_area_map.sectors[0]->triangles[t].points[2]->y,
                al_map_rgb(192, 0, 0), 1 / cam_zoom
            );
            al_draw_filled_triangle(
                cur_area_map.sectors[0]->triangles[t].points[0]->x,
                cur_area_map.sectors[0]->triangles[t].points[0]->y,
                cur_area_map.sectors[0]->triangles[t].points[1]->x,
                cur_area_map.sectors[0]->triangles[t].points[1]->y,
                cur_area_map.sectors[0]->triangles[t].points[2]->x,
                cur_area_map.sectors[0]->triangles[t].points[2]->y,
                al_map_rgba(0, (t % 3 == 0 ? 85 : t % 3 == 1 ? 170 : 255), 0, 128)
            );
        }
        
        if(ed_bg_bitmap) {
            al_draw_tinted_scaled_bitmap(
                ed_bg_bitmap,
                al_map_rgba(255, 255, 255, ed_bg_a),
                0, 0,
                al_get_bitmap_width(ed_bg_bitmap), al_get_bitmap_height(ed_bg_bitmap),
                ed_bg_x, ed_bg_y,
                ed_bg_w, ed_bg_h,
                0
            );
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_flip_display();
}

/* ----------------------------------------------------------------------------
 * Handles the events for the area editor.
 */
void area_editor::handle_controls(ALLEGRO_EVENT ev) {

    ed_gui->handle_event(ev);
    
    //Update mouse cursor in world coordinates.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
    }
    
    
    //Moving vertices, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        //Move background.
        if(ed_sec_mode == EDITOR_SEC_MODE_BG_MOUSE) {
        
            if(ed_holding_m1) {
                ed_bg_x += ev.mouse.dx / cam_zoom;
                ed_bg_y += ev.mouse.dy / cam_zoom;
                
            } else if(ed_holding_m2) {
            
                float new_w = ed_bg_w + ev.mouse.dx / cam_zoom;
                float new_h = ed_bg_h + ev.mouse.dy / cam_zoom;
                
                if(ed_bg_aspect_ratio) {
                    //Find the most significant change.
                    if(ev.mouse.dx != 0 || ev.mouse.dy != 0) {
                        bool most_is_width = fabs((double) ev.mouse.dx) > fabs((double) ev.mouse.dy);
                        
                        
                        if(most_is_width) {
                            float ratio = ed_bg_h / ed_bg_w;
                            ed_bg_w = new_w;
                            ed_bg_h = new_w * ratio;
                        } else {
                            float ratio = ed_bg_w / ed_bg_h;
                            ed_bg_h = new_h;
                            ed_bg_w = new_h * ratio;
                        }
                    }
                } else {
                    ed_bg_w = new_w;
                    ed_bg_h = new_h;
                }
                
            }
            
            load_bg_to_gui();
            
            //Move camera.
        } else if(ed_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        //Move vertex.
        if(ed_moving_vertex != string::npos) {
            cur_area_map.vertices[ed_moving_vertex]->x = snap_to_grid(mouse_cursor_x);
            cur_area_map.vertices[ed_moving_vertex]->y = snap_to_grid(mouse_cursor_y);
        }
        
        if(ev.mouse.dz != 0) {
            //Zoom.
            float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
            new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
            new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
            float new_mc_x = ev.mouse.x / new_zoom - cam_x - ((scr_w - 208) / 2 / new_zoom);
            float new_mc_y = ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
            
            cam_x -= (mouse_cursor_x - new_mc_x);
            cam_y -= (mouse_cursor_y - new_mc_y);
            mouse_cursor_x = new_mc_x;
            mouse_cursor_y = new_mc_y;
            cam_zoom = new_zoom;
        }
        
        
        //Clicking.
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16) {
    
        if(ev.mouse.button == 1) ed_holding_m1 = true;
        else if(ev.mouse.button == 2) ed_holding_m2 = true;
        
        if(ev.mouse.button != 1) return;
        
        //Drag vertex.
        if(ev.mouse.button == 1 && ed_sec_mode == EDITOR_SEC_MODE_NONE) {
            if(ev.mouse.x < scr_w - 208) {
            
                //Find a vertex to drag.
                for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                    if(
                        check_dist(
                            mouse_cursor_x, mouse_cursor_y,
                            cur_area_map.vertices[v]->x, cur_area_map.vertices[v]->y,
                            6.0 / cam_zoom
                        )
                    ) {
                        ed_moving_vertex = v;
                        break;
                    }
                }
                
            }
        }
        
        //Place a new sector where the cursor is.
        if(ed_new_sector_mode) {
        
            ed_new_sector_mode = false;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            size_t outer_sector_nr;
            sector* outer_sector = get_sector(hotspot_x, hotspot_y, &outer_sector_nr);
            
            sector* new_sector = new sector();
            if(outer_sector) {
                //ToDo missing attributes.
                new_sector->brightness = outer_sector->brightness;
                new_sector->textures[0] = outer_sector->textures[0];
                new_sector->textures[1] = outer_sector->textures[1];
                new_sector->type = outer_sector->type;
                new_sector->z = outer_sector->z;
            }
            
            //Create the vertices.
            vertex* new_vertices[4];
            for(size_t v = 0; v < 4; v++) new_vertices[v] = new vertex(0, 0);
            new_vertices[0]->x = hotspot_x - 32;
            new_vertices[0]->y = hotspot_y - 32;
            new_vertices[1]->x = hotspot_x + 32;
            new_vertices[1]->y = hotspot_y - 32;
            new_vertices[2]->x = hotspot_x + 32;
            new_vertices[2]->y = hotspot_y + 32;
            new_vertices[3]->x = hotspot_x - 32;
            new_vertices[3]->y = hotspot_y + 32;
            for(size_t v = 0; v < 4; v++)cur_area_map.vertices.push_back(new_vertices[v]);
            
            //Create the linedefs.
            linedef* new_linedefs[4];
            for(size_t l = 0; l < 4; l++) {
                new_linedefs[l] = new linedef(
                    cur_area_map.vertices.size() - (4 - l),
                    cur_area_map.vertices.size() - (4 - ((l + 1) % 4))
                );
                new_linedefs[l]->sector_nrs[0] = cur_area_map.sectors.size();
                new_linedefs[l]->sector_nrs[1] = outer_sector_nr;
                cur_area_map.linedefs.push_back(new_linedefs[l]);
            }
            
            //Add them to the area map.
            for(size_t l = 0; l < 4; l++) new_sector->linedef_nrs.push_back(cur_area_map.linedefs.size() - (4 - l));
            cur_area_map.sectors.push_back(new_sector);
            
            for(size_t l = 0; l < 4; l++) new_linedefs[l]->fix_pointers(cur_area_map);
            for(size_t v = 0; v < 4; v++) new_vertices[v]->connect_linedefs(cur_area_map, cur_area_map.vertices.size() - (4 - v));
            new_sector->connect_linedefs(cur_area_map, cur_area_map.sectors.size() - 1);
            
            //Add the linedefs to the outer sector's list.
            if(outer_sector) {
                for(size_t l = 0; l < 4; l++) {
                    outer_sector->linedefs.push_back(new_linedefs[l]);
                    outer_sector->linedef_nrs.push_back(cur_area_map.linedefs.size() - (4 - l));
                }
            }
            
            //Check for intersections.
            for(size_t v = 0; v < 4; v += 2) check_linedef_intersections(new_vertices[v]);
            
            //Triangulate new sector and the parent one.
            triangulate(new_sector);
            if(outer_sector) triangulate(outer_sector);
            
            
            //Create a new vertex in a linedef.
        } else if(ed_moving_vertex == string::npos && ed_sec_mode == EDITOR_SEC_MODE_NONE) {
        
            if(ed_double_click_time == 0) ed_double_click_time = 0.5;
            else {
                ed_double_click_time = 0;
                //Create a new vertex.
                
                for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
                    linedef* l_ptr = cur_area_map.linedefs[l];
                    
                    if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
                    
                    if(
                        circle_intersects_line(
                            mouse_cursor_x, mouse_cursor_y, 6,
                            l_ptr->vertices[0]->x, l_ptr->vertices[0]->y,
                            l_ptr->vertices[1]->x, l_ptr->vertices[1]->y
                        )
                    ) {
                    
                        //New vertex, on the split point.
                        vertex* new_v_ptr = new vertex(mouse_cursor_x, mouse_cursor_y);
                        cur_area_map.vertices.push_back(new_v_ptr);
                        
                        //New linedef, copied from the original one.
                        linedef* new_l_ptr = new linedef(*l_ptr);
                        cur_area_map.linedefs.push_back(new_l_ptr);
                        
                        //Save the original end vertex for later.
                        vertex* end_v_ptr = l_ptr->vertices[1];
                        
                        //Set vertices on the new and original linedefs.
                        new_l_ptr->vertex_nrs[0] = cur_area_map.vertices.size() - 1;
                        new_l_ptr->vertices[0] = new_v_ptr;
                        l_ptr->vertex_nrs[1] = new_l_ptr->vertex_nrs[0];
                        l_ptr->vertices[1] = new_v_ptr;
                        
                        //Set sectors on the new linedef.
                        if(new_l_ptr->sectors[0]) {
                            new_l_ptr->sectors[0]->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                            new_l_ptr->sectors[0]->linedefs.push_back(new_l_ptr);
                        }
                        if(new_l_ptr->sectors[1]) {
                            new_l_ptr->sectors[1]->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                            new_l_ptr->sectors[1]->linedefs.push_back(new_l_ptr);
                        }
                        
                        //Set linedefs of the new vertex.
                        new_v_ptr->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                        new_v_ptr->linedef_nrs.push_back(l);
                        new_v_ptr->linedefs.push_back(new_l_ptr);
                        new_v_ptr->linedefs.push_back(l_ptr);
                        
                        //Update linedef data on the end vertex of the original line
                        //(it now links to the new line, not the old).
                        for(size_t vl = 0; vl < end_v_ptr->linedefs.size(); vl++) {
                            if(end_v_ptr->linedefs[vl] == l_ptr) {
                                end_v_ptr->linedefs[vl] = new_l_ptr;
                                end_v_ptr->linedef_nrs[vl] = cur_area_map.linedefs.size() - 1;
                                break;
                            }
                        }
                        
                        break;
                    }
                }
            }
            
        }
        
        //Mouse button release.
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 1) ed_holding_m1 = false;
        else if(ev.mouse.button == 2) ed_holding_m2 = false;
        
        if(ev.mouse.button == 1 && ed_sec_mode == EDITOR_SEC_MODE_NONE) {
            //Release the vertex.
            
            if(ed_moving_vertex != string::npos) {
                vertex* moved_v_ptr = cur_area_map.vertices[ed_moving_vertex];
                vertex* final_vertex = moved_v_ptr;
                
                //Check if the line's vertices intersect with any other lines.
                //If so, they're marked with red.
                check_linedef_intersections(moved_v_ptr);
                
                
                //Check if we should merge.
                for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                    vertex* dest_v_ptr = cur_area_map.vertices[v];
                    if(dest_v_ptr == moved_v_ptr) continue;
                    
                    if(check_dist(moved_v_ptr->x, moved_v_ptr->y, dest_v_ptr->x, dest_v_ptr->y, 10)) {
                        //Merge vertices.
                        
                        //Find out what to do with every linedef of the dragged vertex.
                        for(size_t l = 0; l < moved_v_ptr->linedefs.size();) {
                        
                            bool was_deleted = false;
                            linedef* l_ptr = moved_v_ptr->linedefs[l];
                            vertex* other_vertex = l_ptr->vertices[0] == moved_v_ptr ? l_ptr->vertices[1] : l_ptr->vertices[0];
                            
                            //Check if it's being squashed into non-existence.
                            if(other_vertex == dest_v_ptr) {
                                //Clear it from the vertex lists.
                                for(size_t vl = 0; vl < other_vertex->linedefs.size(); vl++) {
                                    if(other_vertex->linedefs[vl] == l_ptr) {
                                        other_vertex->linedefs.erase(other_vertex->linedefs.begin() + vl);
                                        other_vertex->linedef_nrs.erase(other_vertex->linedef_nrs.begin() + vl);
                                        break;
                                    }
                                }
                                
                                //Clear it from the sector lists.
                                for(size_t s = 0; s < 2; s++) {
                                    for(size_t sl = 0; sl < l_ptr->sectors[s]->linedefs.size(); sl++) {
                                        if(l_ptr->sectors[s]->linedefs[sl] == l_ptr) {
                                            l_ptr->sectors[s]->linedefs.erase(l_ptr->sectors[s]->linedefs.begin() + sl);
                                            l_ptr->sectors[s]->linedef_nrs.erase(l_ptr->sectors[s]->linedef_nrs.begin() + sl);
                                            break;
                                        }
                                    }
                                }
                                
                                //Clear its info, so it gets marked for deletion.
                                l_ptr->vertex_nrs[0] = l_ptr->vertex_nrs[1] = string::npos;
                                l_ptr->fix_pointers(cur_area_map);
                                
                            } else {
                            
                                bool has_merged = false;
                                //Check if the linedef will be merged with another one.
                                //These are linedefs that share a common vertex,
                                //plus the moved/destination vertex.
                                for(size_t dl = 0; dl < dest_v_ptr->linedefs.size(); dl++) {
                                
                                    linedef* dl_ptr = dest_v_ptr->linedefs[dl];
                                    vertex* d_other_vertex = dl_ptr->vertices[0] == dest_v_ptr ? dl_ptr->vertices[1] : dl_ptr->vertices[0];
                                    
                                    if(d_other_vertex == other_vertex) {
                                        has_merged = true;
                                        //The linedef will be merged with this one.
                                        if(l_ptr->sector_nrs[0] == dl_ptr->sector_nrs[0])
                                            dl_ptr->sector_nrs[0] = l_ptr->sector_nrs[1];
                                        else if(l_ptr->sector_nrs[0] == dl_ptr->sector_nrs[1])
                                            dl_ptr->sector_nrs[1] = l_ptr->sector_nrs[1];
                                        else if(l_ptr->sector_nrs[1] == dl_ptr->sector_nrs[0])
                                            dl_ptr->sector_nrs[0] = l_ptr->sector_nrs[0];
                                        else if(l_ptr->sector_nrs[1] == dl_ptr->sector_nrs[1])
                                            dl_ptr->sector_nrs[1] = l_ptr->sector_nrs[0];
                                        dl_ptr->fix_pointers(cur_area_map);
                                        
                                        //Go to the linedef's old vertices,
                                        //and tell them that it no longer exists.
                                        for(size_t v = 0; v < 2; v++) {
                                        
                                            for(size_t vl = 0; vl < l_ptr->vertices[v]->linedefs.size(); vl++) {
                                                linedef* vl_ptr = l_ptr->vertices[v]->linedefs[vl];
                                                
                                                if(vl_ptr == l_ptr) {
                                                    l_ptr->vertices[v]->linedefs.erase(
                                                        l_ptr->vertices[v]->linedefs.begin() + vl);
                                                    l_ptr->vertices[v]->linedef_nrs.erase(
                                                        l_ptr->vertices[v]->linedef_nrs.begin() + vl);
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        //Now tell the linedef's old sectors.
                                        for(size_t s = 0; s < 2; s++) {
                                            for(size_t sl = 0; sl < l_ptr->sectors[s]->linedefs.size(); sl++) {
                                                if(l_ptr->sectors[s]->linedefs[sl] == l_ptr) {
                                                    l_ptr->sectors[s]->linedefs.erase(l_ptr->sectors[s]->linedefs.begin() + sl);
                                                    l_ptr->sectors[s]->linedef_nrs.erase(l_ptr->sectors[s]->linedef_nrs.begin() + sl);
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        
                                        //Remove the deleted linedef's info.
                                        //This'll mark it for deletion.
                                        l_ptr->sector_nrs[0] = l_ptr->sector_nrs[1] = string::npos;
                                        l_ptr->vertex_nrs[0] = l_ptr->vertex_nrs[1] = string::npos;
                                        l_ptr->fix_pointers(cur_area_map);
                                        was_deleted = true;
                                        
                                        break;
                                    }
                                }
                                
                                //If it's matchless, that means it'll just be joined to
                                //the group of linedefs on the destination vertex.
                                if(!has_merged) {
                                    dest_v_ptr->linedef_nrs.push_back(moved_v_ptr->linedef_nrs[l]);
                                    unsigned char n = (l_ptr->vertices[0] == moved_v_ptr ? 0 : 1);
                                    l_ptr->vertices[n] = dest_v_ptr;
                                    l_ptr->vertex_nrs[n] = v;
                                }
                            }
                            
                            if(!was_deleted) l++;
                            
                        }
                        
                        dest_v_ptr->fix_pointers(cur_area_map);
                        
                        //If this vertex is out of linedefs, it'll be
                        //deleted eventually. Move it out of the way.
                        if(dest_v_ptr->linedefs.size() == 0) {
                            dest_v_ptr->x = dest_v_ptr->y = FLT_MAX;
                        }
                        
                        //Remove the old vertex' info.
                        //This'll mark it for deletion.
                        moved_v_ptr->linedef_nrs.clear();
                        moved_v_ptr->linedefs.clear();
                        moved_v_ptr->x = moved_v_ptr->y = FLT_MAX; //So it's out of the way.
                        
                        final_vertex = dest_v_ptr;
                        
                        break;
                    }
                }
                
                //Finally, re-triangulate the affected sectors.
                unordered_set<sector*> affected_sectors;
                for(size_t l = 0; l < final_vertex->linedefs.size(); l++) {
                    linedef* l_ptr = final_vertex->linedefs[l];
                    for(size_t s = 0; s < 2; s++) {
                        if(l_ptr->sectors[s]) affected_sectors.insert(l_ptr->sectors[s]);
                    }
                }
                for(auto s = affected_sectors.begin(); s != affected_sectors.end(); s++) {
                    triangulate(*s);
                }
                
                ed_moving_vertex = string::npos;
                
            }
        }
        
        //Key press.
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            ed_shift_pressed = true;
        }
        
        //Key release.
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            ed_shift_pressed = false;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {
    ed_mode = EDITOR_MODE_MAIN;
    
    load_area("test"); //ToDo non-fixed name, duh.
    
    lafi_style* s = new lafi_style(al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32), al_map_rgb(96, 128, 160));
    ed_gui = new lafi_gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi_frame* frm_main = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    ed_gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add("lbl_area", new lafi_label(0, 0, 0, 0, "Area:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_area", new lafi_button(0, 0, 0, 0), 100, 32);
    int y = frm_main->easy_row();
    
    lafi_frame* frm_area = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    frm_area->easy_row();
    frm_area->easy_add("but_sectors", new lafi_button(0, 0, 0, 0, "Edit sectors"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_objects", new lafi_button(0, 0, 0, 0, "Edit objects"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_bg", new lafi_button(0, 0, 0, 0, "Edit background"), 100, 32);
    frm_area->easy_row();
    
    
    //Bottom bar.
    lafi_frame* frm_bottom = new lafi_frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    ed_gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_toggle_hitboxes", new lafi_button(0, 0, 0, 0, "Hit"), 25, 32);
    frm_bottom->easy_add("but_load", new lafi_button(           0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save", new lafi_button(           0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit", new lafi_button(           0, 0, 0, 0, "X"), 25, 32);
    frm_bottom->easy_row();
    
    
    //Picker frame.
    lafi_frame* frm_picker = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    ed_gui->add("frm_picker", frm_picker);
    
    frm_picker->add("but_back", new lafi_button(     scr_w - 200, 8, scr_w - 104, 24, "Back"));
    frm_picker->add("txt_new", new lafi_textbox(     scr_w - 200, 40, scr_w - 48, 56));
    frm_picker->add("but_new", new lafi_button(      scr_w - 40,  32, scr_w - 8,  64, "+"));
    frm_picker->add("frm_list", new lafi_frame(      scr_w - 200, 72, scr_w - 32, scr_h - 56));
    frm_picker->add("bar_scroll", new lafi_scrollbar(scr_w - 24,  72, scr_w - 8,  scr_h - 56));
    
    
    //Sectors frame.
    lafi_frame* frm_sectors = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_sectors);
    ed_gui->add("frm_sectors", frm_sectors);
    
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_new", new lafi_button(0, 0, 0, 0, "+"), 20, 32);
    frm_sectors->easy_row();
    
    
    //Background frame.
    lafi_frame* frm_bg = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_bg);
    ed_gui->add("frm_bg", frm_bg);
    
    frm_bg->easy_row();
    frm_bg->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_file", new lafi_label(0, 0, 0, 0, "File:"), 30, 16);
    frm_bg->easy_add("txt_file", new lafi_textbox(0, 0, 0, 0), 70, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_xy", new lafi_label(0, 0, 0, 0, "X&Y:"), 30, 16);
    frm_bg->easy_add("txt_x", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_add("txt_y", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_wh", new lafi_label(0, 0, 0, 0, "W&H:"), 30, 16);
    frm_bg->easy_add("txt_w", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_add("txt_h", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("chk_ratio", new lafi_checkbox(0, 0, 0, 0, "Keep aspect ratio"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("chk_mouse", new lafi_checkbox(0, 0, 0, 0, "Transform with mouse"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_alpha", new lafi_label(0, 0, 0, 0, "Transparency:"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("bar_alpha", new lafi_scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24);
    frm_bg->easy_row();
    
    
    //Status bar.
    lafi_label* ed_gui_status_bar = new lafi_label(0, scr_h - 16, scr_w - 208, scr_h);
    ed_gui->add("lbl_status_bar", ed_gui_status_bar);
    
    
    //Properties -- main.
    frm_main->widgets["frm_area"]->widgets["but_sectors"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_main->widgets["frm_area"]->widgets["but_bg"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_BG;
        change_to_right_frame();
    };
    
    
    //Properties -- sectors.
    frm_sectors->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_sectors->widgets["but_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_new_sector_mode = true;
    };
    
    
    //Properties -- background.
    auto lambda_save_bg_from_gui = [] (lafi_widget*) { save_bg_from_gui(); };
    auto lambda_save_bg_from_gui_click = [] (lafi_widget*, int, int) { save_bg_from_gui(); };
    frm_bg->widgets["txt_file"]->lose_focus_handler = lambda_save_bg_from_gui;
    frm_bg->widgets["txt_x"]->lose_focus_handler = lambda_save_bg_from_gui;
    frm_bg->widgets["txt_y"]->lose_focus_handler = lambda_save_bg_from_gui;
    frm_bg->widgets["txt_w"]->lose_focus_handler = lambda_save_bg_from_gui;
    frm_bg->widgets["txt_h"]->lose_focus_handler = lambda_save_bg_from_gui;
    ((lafi_scrollbar*) frm_bg->widgets["bar_alpha"])->change_handler = lambda_save_bg_from_gui;
    frm_bg->widgets["chk_ratio"]->left_mouse_click_handler = lambda_save_bg_from_gui_click;
    frm_bg->widgets["chk_mouse"]->left_mouse_click_handler = lambda_save_bg_from_gui_click;
    frm_bg->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    
    
    load_bg_to_gui();
}

/* ----------------------------------------------------------------------------
 * Loads the background's data from the memory to the gui.
 */
void area_editor::load_bg_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_bg"];
    ((lafi_textbox*) f->widgets["txt_file"])->text = ed_bg_file_name;
    ((lafi_textbox*) f->widgets["txt_x"])->text = ftos(ed_bg_x);
    ((lafi_textbox*) f->widgets["txt_y"])->text = ftos(ed_bg_y);
    ((lafi_textbox*) f->widgets["txt_w"])->text = ftos(ed_bg_w);
    ((lafi_textbox*) f->widgets["txt_h"])->text = ftos(ed_bg_h);
    ((lafi_checkbox*) f->widgets["chk_ratio"])->set(ed_bg_aspect_ratio);
    ((lafi_checkbox*) f->widgets["chk_mouse"])->set(ed_sec_mode == EDITOR_SEC_MODE_BG_MOUSE);
    ((lafi_scrollbar*) f->widgets["bar_alpha"])->set_value(ed_bg_a);
}

/* ----------------------------------------------------------------------------
 * Saves the background's data from the fields in the gui.
 */
void area_editor::save_bg_from_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_bg"];
    
    string new_file_name = ((lafi_textbox*) f->widgets["txt_file"])->text;
    bool is_file_new = false;
    
    if(new_file_name != ed_bg_file_name) {
        //New background image, delete the old one.
        is_file_new = true;
        if(ed_bg_bitmap && ed_bg_bitmap != bmp_error) al_destroy_bitmap(ed_bg_bitmap);
        ed_bg_bitmap = load_bmp(new_file_name);
        ed_bg_file_name = new_file_name;
        if(ed_bg_bitmap) {
            ed_bg_w = al_get_bitmap_width(ed_bg_bitmap);
            ed_bg_h = al_get_bitmap_height(ed_bg_bitmap);
        } else {
            ed_bg_w = 0;
            ed_bg_h = 0;
        }
    }
    
    ed_bg_x = tof(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_bg_y = tof(((lafi_textbox*) f->widgets["txt_y"])->text);
    
    ed_bg_aspect_ratio = ((lafi_checkbox*) f->widgets["chk_ratio"])->checked;
    float new_w = tof(((lafi_textbox*) f->widgets["txt_w"])->text);
    float new_h = tof(((lafi_textbox*) f->widgets["txt_h"])->text);
    
    if(new_w != 0 && new_h != 0 && !is_file_new) {
        if(ed_bg_aspect_ratio) {
            if(new_w == ed_bg_w && new_h != ed_bg_h) {
                float ratio = ed_bg_w / ed_bg_h;
                ed_bg_h = new_h;
                ed_bg_w = new_h * ratio;
            } else if(new_w != ed_bg_w && new_h == ed_bg_h) {
                float ratio = ed_bg_h / ed_bg_w;
                ed_bg_w = new_w;
                ed_bg_h = new_w * ratio;
            } else {
                ed_bg_w = new_w;
                ed_bg_h = new_h;
            }
        } else {
            ed_bg_w = new_w;
            ed_bg_h = new_h;
        }
    }
    
    ed_sec_mode = ((lafi_checkbox*) f->widgets["chk_mouse"])->checked ? EDITOR_SEC_MODE_BG_MOUSE : EDITOR_SEC_MODE_NONE;
    ed_bg_a = ((lafi_scrollbar*) f->widgets["bar_alpha"])->low_value;
    
    load_bg_to_gui();
}

/* ----------------------------------------------------------------------------
 * Snaps a coordinate to the nearest grid space.
 */
float area_editor::snap_to_grid(const float c) {
    if(ed_shift_pressed) return c;
    return round(c / 32) * 32;
}