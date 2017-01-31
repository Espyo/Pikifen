/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include "area_editor.h"
#include "../drawing.h"
#include "../functions.h"
#include "../geometry_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {

    gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(
        &transform, cam_x + (gui_x / 2 / cam_zoom),
        cam_y + (scr_h / 2 / cam_zoom)
    );
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, gui_x, status_bar_y); {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        
        //Grid.
        if(sec_mode != ESM_TEXTURE_VIEW) {
            float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
            float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
            float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
            float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
            
            float x = floor(cam_leftmost / grid_interval) * grid_interval;
            while(x < cam_rightmost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(48, 48, 48);
                bool draw_line = true;
                
                if(fmod(x, grid_interval * 2) == 0) {
                    c = al_map_rgb(64, 64, 64);
                    if((grid_interval * 2) * cam_zoom <= 6) draw_line = false;
                } else {
                    if(grid_interval * cam_zoom <= 6) draw_line = false;
                }
                
                if(draw_line) {
                    al_draw_line(
                        x, cam_topmost, x, cam_bottommost + grid_interval,
                        c, 1.0 / cam_zoom
                    );
                }
                x += grid_interval;
            }
            
            float y = floor(cam_topmost / grid_interval) * grid_interval;
            while(y < cam_bottommost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(48, 48, 48);
                bool draw_line = true;
                
                if(fmod(y, grid_interval * 2) == 0) {
                    c = al_map_rgb(64, 64, 64);
                    if((grid_interval * 2) * cam_zoom <= 6) draw_line = false;
                } else {
                    if(grid_interval * cam_zoom <= 6) draw_line = false;
                }
                
                if(draw_line) {
                    al_draw_line(
                        cam_leftmost, y, cam_rightmost + grid_interval, y,
                        c, 1.0 / cam_zoom
                    );
                }
                y += grid_interval;
            }
            
            //0,0 marker.
            al_draw_line(
                -(DEF_GRID_INTERVAL * 2), 0, DEF_GRID_INTERVAL * 2, 0,
                al_map_rgb(128, 128, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                0, -(DEF_GRID_INTERVAL * 2), 0, DEF_GRID_INTERVAL * 2,
                al_map_rgb(128, 128, 255), 1.0 / cam_zoom
            );
        }
        
        //Edges.
        if(sec_mode != ESM_TEXTURE_VIEW) {
        
            unsigned char sector_opacity = 255;
            bool show_vertices = true;
            if(
                mode == EDITOR_MODE_OBJECTS ||
                mode == EDITOR_MODE_FOLDER_PATHS ||
                mode == EDITOR_MODE_SHADOWS
            ) {
                sector_opacity = 128;
                show_vertices = false;
            }
            
            size_t n_edges = cur_area_data.edges.size();
            for(size_t e = 0; e < n_edges; ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                
                if(!is_edge_valid(e_ptr)) continue;
                
                bool one_sided = true;
                bool same_z = false;
                bool error_highlight = false;
                bool valid = true;
                bool mouse_on = false;
                bool selected = false;
                
                if(error_sector_ptr) {
                    if(
                        e_ptr->sectors[0] == error_sector_ptr ||
                        e_ptr->sectors[1] == error_sector_ptr
                    ) {
                        error_highlight = true;
                    }
                    
                } else {
                    for(size_t ie = 0; ie < intersecting_edges.size(); ++ie) {
                        if(intersecting_edges[ie].contains(e_ptr)) {
                            valid = false;
                            break;
                        }
                    }
                    
                    if(
                        non_simples.find(e_ptr->sectors[0]) !=
                        non_simples.end()
                    ) {
                        valid = false;
                    }
                    if(
                        non_simples.find(e_ptr->sectors[1]) !=
                        non_simples.end()
                    ) {
                        valid = false;
                    }
                    if(lone_edges.find(e_ptr) != lone_edges.end()) {
                        valid = false;
                    }
                }
                
                if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
                
                if(
                    !one_sided &&
                    e_ptr->sectors[0]->z == e_ptr->sectors[1]->z &&
                    e_ptr->sectors[0]->type == e_ptr->sectors[1]->type
                ) {
                    same_z = true;
                }
                
                if(on_sector && mode == EDITOR_MODE_SECTORS) {
                    if(e_ptr->sectors[0] == on_sector) mouse_on = true;
                    if(e_ptr->sectors[1] == on_sector) mouse_on = true;
                }
                
                if(
                    cur_sector &&
                    (mode == EDITOR_MODE_SECTORS || mode == EDITOR_MODE_TEXTURE)
                ) {
                    if(e_ptr->sectors[0] == cur_sector) selected = true;
                    if(e_ptr->sectors[1] == cur_sector) selected = true;
                }
                
                
                al_draw_line(
                    e_ptr->vertexes[0]->x,
                    e_ptr->vertexes[0]->y,
                    e_ptr->vertexes[1]->x,
                    e_ptr->vertexes[1]->y,
                    (
                        selected ?
                        al_map_rgba(224, 224, 64,  sector_opacity) :
                        error_highlight ?
                        al_map_rgba(192, 80,  0,   sector_opacity) :
                        !valid ?
                        al_map_rgba(192, 32,  32,  sector_opacity) :
                        one_sided ?
                        al_map_rgba(255, 255, 255, sector_opacity) :
                        same_z ?
                        al_map_rgba(128, 128, 128, sector_opacity) :
                        al_map_rgba(192, 192, 192, sector_opacity)
                    ),
                    (mouse_on || selected ? 3.0 : 2.0) / cam_zoom
                );
                
                if(debug_sector_nrs) {
                    float mid_x =
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f;
                    float mid_y =
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f;
                    float angle =
                        atan2(
                            e_ptr->vertexes[0]->y - e_ptr->vertexes[1]->y,
                            e_ptr->vertexes[0]->x - e_ptr->vertexes[1]->x
                        );
                    draw_scaled_text(
                        font_builtin, al_map_rgb(192, 255, 192),
                        mid_x + cos(angle + M_PI_2) * 4,
                        mid_y + sin(angle + M_PI_2) * 4,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        (
                            e_ptr->sector_nrs[0] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[0])
                        )
                    );
                    draw_scaled_text(
                        font_builtin, al_map_rgb(192, 255, 192),
                        mid_x + cos(angle - M_PI_2) * 4,
                        mid_y + sin(angle - M_PI_2) * 4,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        (
                            e_ptr->sector_nrs[1] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[1])
                        )
                    );
                }
                
                if(debug_edge_nrs) {
                    float mid_x =
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f;
                    float mid_y =
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f;
                    draw_scaled_text(
                        font_builtin, al_map_rgb(255, 192, 192),
                        mid_x, mid_y,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        i2s(e)
                    );
                }
            }
            
            //Vertexes.
            if(show_vertices) {
                size_t n_vertexes = cur_area_data.vertexes.size();
                for(size_t v = 0; v < n_vertexes; ++v) {
                    vertex* v_ptr = cur_area_data.vertexes[v];
                    al_draw_filled_circle(
                        v_ptr->x,
                        v_ptr->y,
                        3.0 / cam_zoom,
                        al_map_rgba(80, 160, 255, sector_opacity)
                    );
                    
                    if(debug_vertex_nrs) {
                        draw_scaled_text(
                            font_builtin, al_map_rgb(192, 192, 255),
                            v_ptr->x, v_ptr->y,
                            DEBUG_TEXT_SCALE / cam_zoom,
                            DEBUG_TEXT_SCALE / cam_zoom,
                            ALLEGRO_ALIGN_CENTER, 1,
                            i2s(v)
                        );
                    }
                }
            }
            
            if(mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS && cur_sector) {
                draw_sector_texture(cur_sector, 0, 0, 1);
            }
            
        } else {
        
            //Draw textures.
            for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
                draw_sector(cur_area_data.sectors[s], 0, 0, 1.0);
            }
        }
        
        //Mobs.
        unsigned char mob_opacity = 224;
        if(
            mode == EDITOR_MODE_SECTORS ||
            mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS ||
            mode == EDITOR_MODE_TEXTURE ||
            mode == EDITOR_MODE_FOLDER_PATHS ||
            mode == EDITOR_MODE_SHADOWS
        ) {
            mob_opacity = 32;
        }
        if(sec_mode == ESM_TEXTURE_VIEW) mob_opacity = 0;
        
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            bool valid = m_ptr->type != NULL;
            
            float radius =
                m_ptr->type ?
                m_ptr->type->radius == 0 ? 16 :
                m_ptr->type->radius : 16;
            ALLEGRO_COLOR c = mob_categories.get_editor_color(m_ptr->category);
            
            al_draw_filled_circle(
                m_ptr->x, m_ptr->y,
                radius,
                (
                    valid ? change_alpha(c, mob_opacity) :
                    al_map_rgba(255, 0, 0, mob_opacity)
                )
            );
            
            float lrw = cos(m_ptr->angle) * radius;
            float lrh = sin(m_ptr->angle) * radius;
            float lt = radius / 8.0;
            
            al_draw_line(
                m_ptr->x - lrw * 0.8, m_ptr->y - lrh * 0.8,
                m_ptr->x + lrw * 0.8, m_ptr->y + lrh * 0.8,
                al_map_rgba(0, 0, 0, mob_opacity), lt
            );
            
            float tx1 = m_ptr->x + lrw;
            float ty1 = m_ptr->y + lrh;
            float tx2 =
                tx1 + cos(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty2 =
                ty1 + sin(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float tx3 =
                tx1 + cos(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty3 =
                ty1 + sin(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
                
            al_draw_filled_triangle(
                tx1, ty1,
                tx2, ty2,
                tx3, ty3,
                al_map_rgba(0, 0, 0, mob_opacity)
            );
            
            if(m_ptr == cur_mob && mode == EDITOR_MODE_OBJECTS) {
                al_draw_circle(
                    m_ptr->x, m_ptr->y,
                    radius,
                    al_map_rgba(255, 255, 255, mob_opacity), 2 / cam_zoom
                );
            }
            
        }
        
        //Paths.
        if(mode == EDITOR_MODE_FOLDER_PATHS) {
        
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                al_draw_filled_circle(
                    s_ptr->x, s_ptr->y,
                    STOP_RADIUS,
                    al_map_rgb(224, 192, 160)
                );
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); l++) {
                    path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                    bool one_way = !(s_ptr->links[l].end_ptr->has_link(s_ptr));
                    
                    al_draw_line(
                        s_ptr->x, s_ptr->y,
                        s2_ptr->x, s2_ptr->y,
                        (
                            one_way ? al_map_rgb(255, 160, 160) :
                            al_map_rgb(255, 255, 160)
                        ),
                        PATH_LINK_THICKNESS / cam_zoom
                    );
                    
                    if(one_way) {
                        //Draw a triangle down the middle.
                        float mid_x =
                            (s_ptr->x + s2_ptr->x) / 2.0f;
                        float mid_y =
                            (s_ptr->y + s2_ptr->y) / 2.0f;
                        float angle =
                            atan2(s2_ptr->y - s_ptr->y, s2_ptr->x - s_ptr->x);
                        const float delta =
                            (PATH_LINK_THICKNESS * 4) / cam_zoom;
                            
                        al_draw_filled_triangle(
                            mid_x + cos(angle) * delta,
                            mid_y + sin(angle) * delta,
                            mid_x + cos(angle + M_PI_2) * delta,
                            mid_y + sin(angle + M_PI_2) * delta,
                            mid_x + cos(angle - M_PI_2) * delta,
                            mid_y + sin(angle - M_PI_2) * delta,
                            al_map_rgb(255, 160, 160)
                        );
                    }
                }
            }
            
            if(sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK2) {
                al_draw_line(
                    new_link_first_stop->x, new_link_first_stop->y,
                    mouse_cursor_x, mouse_cursor_y,
                    al_map_rgb(255, 255, 255), 2 / cam_zoom
                );
            }
            
            if(show_closest_stop) {
                path_stop* closest = NULL;
                dist closest_dist;
                for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                    path_stop* s_ptr = cur_area_data.path_stops[s];
                    dist d(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y);
                    
                    if(!closest || d < closest_dist) {
                        closest = s_ptr;
                        closest_dist = d;
                    }
                }
                
                al_draw_line(
                    mouse_cursor_x, mouse_cursor_y,
                    closest->x, closest->y,
                    al_map_rgb(96, 224, 32), 2 / cam_zoom
                );
            }
            
            if(show_path_preview) {
                //Draw the checkpoints.
                for(unsigned char c = 0; c < 2; ++c) {
                    string letter = (c == 0 ? "A" : "B");
                    
                    al_draw_filled_rectangle(
                        path_preview_checkpoints_x[c] -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_y[c] -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_x[c] +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_y[c] +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        al_map_rgb(255, 255, 32)
                    );
                    draw_scaled_text(
                        font_builtin, al_map_rgb(0, 64, 64),
                        path_preview_checkpoints_x[c],
                        path_preview_checkpoints_y[c],
                        DEBUG_TEXT_SCALE / cam_zoom,
                        DEBUG_TEXT_SCALE / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        letter
                    );
                }
                
                //Draw the lines of the path.
                if(path_preview.empty()) {
                    al_draw_line(
                        path_preview_checkpoints_x[0],
                        path_preview_checkpoints_y[0],
                        path_preview_checkpoints_x[1],
                        path_preview_checkpoints_y[1],
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                } else {
                    al_draw_line(
                        path_preview_checkpoints_x[0],
                        path_preview_checkpoints_y[0],
                        path_preview[0]->x,
                        path_preview[0]->y,
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                    for(size_t s = 0; s < path_preview.size() - 1; ++s) {
                        al_draw_line(
                            path_preview[s]->x,
                            path_preview[s]->y,
                            path_preview[s + 1]->x,
                            path_preview[s + 1]->y,
                            al_map_rgb(255, 0, 0), 3 / cam_zoom
                        );
                    }
                    
                    al_draw_line(
                        path_preview.back()->x,
                        path_preview.back()->y,
                        path_preview_checkpoints_x[1],
                        path_preview_checkpoints_y[1],
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                }
            }
        }
        
        //Shadows.
        if(
            mode == EDITOR_MODE_SHADOWS ||
            (sec_mode == ESM_TEXTURE_VIEW && show_shadows)
        ) {
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                draw_sprite(
                    s_ptr->bitmap, s_ptr->x, s_ptr->y, s_ptr->w, s_ptr->h,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(mode == EDITOR_MODE_SHADOWS) {
                    float min_x, min_y, max_x, max_y;
                    get_shadow_bounding_box(
                        s_ptr, &min_x, &min_y, &max_x, &max_y
                    );
                    
                    al_draw_rectangle(
                        min_x, min_y, max_x, max_y,
                        (
                            s_ptr == cur_shadow ?
                            al_map_rgb(224, 224, 64) :
                            al_map_rgb(128, 128, 64)
                        ),
                        2 / cam_zoom
                    );
                }
            }
        }
        
        //New sector preview.
        if(sec_mode == ESM_NEW_SECTOR) {
            for(size_t v = 1; v < new_sector_vertexes.size(); ++v) {
                al_draw_line(
                    new_sector_vertexes[v - 1]->x,
                    new_sector_vertexes[v - 1]->y,
                    new_sector_vertexes[v]->x,
                    new_sector_vertexes[v]->y,
                    al_map_rgb(128, 255, 128),
                    3 / cam_zoom
                );
            }
            if(!new_sector_vertexes.empty()) {
                al_draw_line(
                    new_sector_vertexes.back()->x,
                    new_sector_vertexes.back()->y,
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y),
                    (new_sector_valid_line ?
                     al_map_rgb(64, 255, 64) :
                     al_map_rgb(255, 0, 0)),
                    3 / cam_zoom
                );
            }
        }
        
        //New thing marker.
        if(
            sec_mode == ESM_NEW_SECTOR || sec_mode == ESM_NEW_OBJECT ||
            sec_mode == ESM_DUPLICATE_OBJECT || sec_mode == ESM_NEW_SHADOW ||
            sec_mode == ESM_NEW_STOP || sec_mode == ESM_NEW_LINK1 ||
            sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK1 ||
            sec_mode == ESM_NEW_1WLINK2
        ) {
            float x = mouse_cursor_x;
            float y = mouse_cursor_y;
            if(
                sec_mode != ESM_NEW_1WLINK1 && sec_mode != ESM_NEW_1WLINK2 &&
                sec_mode != ESM_NEW_LINK1 && sec_mode != ESM_NEW_LINK2
            ) {
                x = snap_to_grid(mouse_cursor_x);
                y = snap_to_grid(mouse_cursor_y);
            }
            al_draw_line(
                x - 16, y, x + 16, y,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                x, y - 16, x, y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
        
        //Delete thing marker.
        if(
            sec_mode == ESM_DEL_STOP || sec_mode == ESM_DEL_LINK
        ) {
            al_draw_line(
                mouse_cursor_x - 16, mouse_cursor_y - 16,
                mouse_cursor_x + 16, mouse_cursor_y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                mouse_cursor_x + 16, mouse_cursor_y - 16,
                mouse_cursor_x - 16, mouse_cursor_y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
        
        //Lightly glow the sector under the mouse.
        if(mode == EDITOR_MODE_SECTORS) {
            if(on_sector && moving_thing == INVALID) {
                for(size_t t = 0; t < on_sector->triangles.size(); ++t) {
                    triangle* t_ptr = &on_sector->triangles[t];
                    
                    if(debug_triangulation) {
                        al_draw_triangle(
                            t_ptr->points[0]->x,
                            t_ptr->points[0]->y,
                            t_ptr->points[1]->x,
                            t_ptr->points[1]->y,
                            t_ptr->points[2]->x,
                            t_ptr->points[2]->y,
                            al_map_rgb(192, 0, 0),
                            1.0 / cam_zoom
                        );
                    }
                    
                    al_draw_filled_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        map_alpha(12)
                    );
                }
            }
        }
        
        //Guide.
        if(guide_bitmap && show_guide) {
            al_draw_tinted_scaled_bitmap(
                guide_bitmap,
                map_alpha(guide_a),
                0, 0,
                al_get_bitmap_width(guide_bitmap),
                al_get_bitmap_height(guide_bitmap),
                guide_x, guide_y,
                guide_w, guide_h,
                0
            );
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}
