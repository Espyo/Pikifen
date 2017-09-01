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

#include <allegro5/allegro_font.h>
#include <algorithm>

#include "area_editor_old.h"
#include "../drawing.h"
#include "../functions.h"
#include "../geometry_utils.h"
#include "../vars.h"

using namespace std;


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor_old::do_drawing() {

    gui->draw();
    
    al_use_transform(&world_to_screen_transform);
    
    al_set_clipping_rectangle(0, 0, gui_x, status_bar_y); {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        
        //Grid.
        if(sec_mode != ESM_TEXTURE_VIEW) {
            point cam_top_left_corner(0, 0);
            point cam_bottom_right_corner(gui_x, status_bar_y);
            al_transform_coordinates(
                &screen_to_world_transform,
                &cam_top_left_corner.x, &cam_top_left_corner.y
            );
            al_transform_coordinates(
                &screen_to_world_transform,
                &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
            );
            
            float x =
                floor(cam_top_left_corner.x / grid_interval) * grid_interval;
            while(x < cam_bottom_right_corner.x + grid_interval) {
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
                        x, cam_top_left_corner.y,
                        x, cam_bottom_right_corner.y + grid_interval,
                        c, 1.0 / cam_zoom
                    );
                }
                x += grid_interval;
            }
            
            float y =
                floor(cam_top_left_corner.y / grid_interval) * grid_interval;
            while(y < cam_bottom_right_corner.y + grid_interval) {
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
                        cam_top_left_corner.x, y,
                        cam_bottom_right_corner.x + grid_interval, y,
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
                    point middle(
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f,
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f
                    );
                    float angle =
                        get_angle(
                            point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
                            point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y)
                        );
                    draw_debug_text(
                        al_map_rgb(192, 255, 192),
                        point(
                            middle.x + cos(angle + M_PI_2) * 4,
                            middle.y + sin(angle + M_PI_2) * 4
                        ),
                        (
                            e_ptr->sector_nrs[0] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[0])
                        )
                    );
                    
                    draw_debug_text(
                        al_map_rgb(192, 255, 192),
                        point(
                            middle.x + cos(angle - M_PI_2) * 4,
                            middle.y + sin(angle - M_PI_2) * 4
                        ),
                        (
                            e_ptr->sector_nrs[1] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[1])
                        )
                    );
                }
                
                if(debug_edge_nrs) {
                    point middle(
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f,
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f
                    );
                    draw_debug_text(al_map_rgb(255, 192, 192), middle, i2s(e));
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
                        draw_debug_text(
                            al_map_rgb(192, 192, 255),
                            point(v_ptr->x, v_ptr->y), i2s(v)
                        );
                    }
                }
            }
            
            if(mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS && cur_sector) {
                draw_sector_texture(cur_sector, point(), 1.0f, 1.0f);
            }
            
        } else {
        
            //Draw textures.
            for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
                draw_sector_texture(
                    cur_area_data.sectors[s], point(), 1.0f, 1.0f
                );
                draw_sector_shadows(cur_area_data.sectors[s], point(), 1.0);
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
            ALLEGRO_COLOR c = m_ptr->category->editor_color;
            
            al_draw_filled_circle(
                m_ptr->pos.x, m_ptr->pos.y,
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
                m_ptr->pos.x - lrw * 0.8, m_ptr->pos.y - lrh * 0.8,
                m_ptr->pos.x + lrw * 0.8, m_ptr->pos.y + lrh * 0.8,
                al_map_rgba(0, 0, 0, mob_opacity), lt
            );
            
            float tx1 = m_ptr->pos.x + lrw;
            float ty1 = m_ptr->pos.y + lrh;
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
                    m_ptr->pos.x, m_ptr->pos.y,
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
                    s_ptr->pos.x, s_ptr->pos.y,
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
                        s_ptr->pos.x, s_ptr->pos.y,
                        s2_ptr->pos.x, s2_ptr->pos.y,
                        (
                            one_way ? al_map_rgb(255, 160, 160) :
                            al_map_rgb(255, 255, 160)
                        ),
                        PATH_LINK_THICKNESS / cam_zoom
                    );
                    
                    if(one_way) {
                        //Draw a triangle down the middle.
                        float mid_x =
                            (s_ptr->pos.x + s2_ptr->pos.x) / 2.0f;
                        float mid_y =
                            (s_ptr->pos.y + s2_ptr->pos.y) / 2.0f;
                        float angle =
                            get_angle(s_ptr->pos, s2_ptr->pos);
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
                    new_link_first_stop->pos.x, new_link_first_stop->pos.y,
                    mouse_cursor_w.x, mouse_cursor_w.y,
                    al_map_rgb(255, 255, 255), 2 / cam_zoom
                );
            }
            
            if(show_closest_stop) {
                path_stop* closest = NULL;
                dist closest_dist;
                for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                    path_stop* s_ptr = cur_area_data.path_stops[s];
                    dist d(mouse_cursor_w, s_ptr->pos);
                    
                    if(!closest || d < closest_dist) {
                        closest = s_ptr;
                        closest_dist = d;
                    }
                }
                
                al_draw_line(
                    mouse_cursor_w.x, mouse_cursor_w.y,
                    closest->pos.x, closest->pos.y,
                    al_map_rgb(96, 224, 32), 2 / cam_zoom
                );
            }
            
            if(show_path_preview) {
                //Draw the checkpoints.
                for(unsigned char c = 0; c < 2; ++c) {
                    string letter = (c == 0 ? "A" : "B");
                    
                    al_draw_filled_rectangle(
                        path_preview_checkpoints[c].x -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints[c].y -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints[c].x +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints[c].y +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        al_map_rgb(255, 255, 32)
                    );
                    draw_scaled_text(
                        font_builtin, al_map_rgb(0, 64, 64),
                        path_preview_checkpoints[c],
                        point(
                            POINT_LETTER_TEXT_SCALE / cam_zoom,
                            POINT_LETTER_TEXT_SCALE / cam_zoom
                        ),
                        ALLEGRO_ALIGN_CENTER, 1,
                        letter
                    );
                }
                
                //Draw the lines of the path.
                if(path_preview.empty()) {
                    al_draw_line(
                        path_preview_checkpoints[0].x,
                        path_preview_checkpoints[0].y,
                        path_preview_checkpoints[1].x,
                        path_preview_checkpoints[1].y,
                        al_map_rgb(255, 0, 0), 3.0 / cam_zoom
                    );
                } else {
                    al_draw_line(
                        path_preview_checkpoints[0].x,
                        path_preview_checkpoints[0].y,
                        path_preview[0]->pos.x,
                        path_preview[0]->pos.y,
                        al_map_rgb(255, 0, 0), 3.0 / cam_zoom
                    );
                    for(size_t s = 0; s < path_preview.size() - 1; ++s) {
                        al_draw_line(
                            path_preview[s]->pos.x,
                            path_preview[s]->pos.y,
                            path_preview[s + 1]->pos.x,
                            path_preview[s + 1]->pos.y,
                            al_map_rgb(255, 0, 0), 3.0 / cam_zoom
                        );
                    }
                    
                    al_draw_line(
                        path_preview.back()->pos.x,
                        path_preview.back()->pos.y,
                        path_preview_checkpoints[1].x,
                        path_preview_checkpoints[1].y,
                        al_map_rgb(255, 0, 0), 3.0 / cam_zoom
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
                    s_ptr->bitmap, s_ptr->center, s_ptr->size,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(mode == EDITOR_MODE_SHADOWS) {
                    point min_coords, max_coords;
                    get_shadow_bounding_box(
                        s_ptr, &min_coords, &max_coords
                    );
                    
                    al_draw_rectangle(
                        min_coords.x, min_coords.y, max_coords.x, max_coords.y,
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
                point hotspot = snap_to_grid(mouse_cursor_w);
                al_draw_line(
                    new_sector_vertexes.back()->x,
                    new_sector_vertexes.back()->y,
                    hotspot.x,
                    hotspot.y,
                    (new_sector_valid_line ?
                     al_map_rgb(64, 255, 64) :
                     al_map_rgb(255, 0, 0)),
                    3 / cam_zoom
                );
            }
        }
        
        //New circular sector preview.
        if(sec_mode == ESM_NEW_CIRCLE_SECTOR) {
            if(new_circle_sector_step == 1) {
                float circle_radius =
                    dist(
                        new_circle_sector_center, new_circle_sector_anchor
                    ).to_float();
                al_draw_circle(
                    new_circle_sector_center.x,
                    new_circle_sector_center.y,
                    circle_radius,
                    al_map_rgb(64, 255, 64),
                    3 / cam_zoom
                );
                
            } else if(new_circle_sector_step == 2) {
                for(size_t p = 0; p < new_circle_sector_points.size(); ++p) {
                    point cur_point = new_circle_sector_points[p];
                    point next_point =
                        get_next_in_vector(new_circle_sector_points, p);
                    ALLEGRO_COLOR color =
                        new_circle_sector_valid_edges[p] ?
                        al_map_rgb(64, 255, 64) :
                        al_map_rgb(255, 0, 0);
                        
                    al_draw_line(
                        cur_point.x, cur_point.y,
                        next_point.x, next_point.y,
                        color, 3.0 / cam_zoom
                    );
                }
                
                for(size_t p = 0; p < new_circle_sector_points.size(); ++p) {
                    al_draw_filled_circle(
                        new_circle_sector_points[p].x,
                        new_circle_sector_points[p].y,
                        3.0 / cam_zoom, al_map_rgb(192, 255, 192)
                    );
                }
                
            }
        }
        
        //New thing marker.
        if(
            sec_mode == ESM_NEW_SECTOR || sec_mode == ESM_NEW_CIRCLE_SECTOR ||
            sec_mode == ESM_NEW_OBJECT || sec_mode == ESM_DUPLICATE_OBJECT ||
            sec_mode == ESM_NEW_SHADOW ||
            sec_mode == ESM_NEW_STOP || sec_mode == ESM_NEW_LINK1 ||
            sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK1 ||
            sec_mode == ESM_NEW_1WLINK2
        ) {
            point marker = mouse_cursor_w;
            if(
                sec_mode != ESM_NEW_1WLINK1 && sec_mode != ESM_NEW_1WLINK2 &&
                sec_mode != ESM_NEW_LINK1 && sec_mode != ESM_NEW_LINK2 &&
                new_circle_sector_step != 2
            ) {
                marker = snap_to_grid(marker);
            }
            al_draw_line(
                marker.x - 16, marker.y, marker.x + 16, marker.y,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                marker.x, marker.y - 16, marker.x, marker.y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
        
        //Delete thing marker.
        if(
            sec_mode == ESM_DEL_STOP || sec_mode == ESM_DEL_LINK
        ) {
            al_draw_line(
                mouse_cursor_w.x - 16, mouse_cursor_w.y - 16,
                mouse_cursor_w.x + 16, mouse_cursor_w.y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                mouse_cursor_w.x + 16, mouse_cursor_w.y - 16,
                mouse_cursor_w.x - 16, mouse_cursor_w.y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
        
        //Cross-section points and line.
        if(mode == EDITOR_MODE_REVIEW && show_cross_section) {
            for(unsigned char p = 0; p < 2; ++p) {
                string letter = (p == 0 ? "A" : "B");
                
                al_draw_filled_rectangle(
                    cross_section_points[p].x -
                    (CROSS_SECTION_POINT_RADIUS / cam_zoom),
                    cross_section_points[p].y -
                    (CROSS_SECTION_POINT_RADIUS / cam_zoom),
                    cross_section_points[p].x +
                    (CROSS_SECTION_POINT_RADIUS / cam_zoom),
                    cross_section_points[p].y +
                    (CROSS_SECTION_POINT_RADIUS / cam_zoom),
                    al_map_rgb(255, 255, 32)
                );
                draw_scaled_text(
                    font_builtin, al_map_rgb(0, 64, 64),
                    cross_section_points[p],
                    point(
                        POINT_LETTER_TEXT_SCALE / cam_zoom,
                        POINT_LETTER_TEXT_SCALE / cam_zoom
                    ),
                    ALLEGRO_ALIGN_CENTER, 1,
                    letter
                );
            }
            al_draw_line(
                cross_section_points[0].x,
                cross_section_points[0].y,
                cross_section_points[1].x,
                cross_section_points[1].y,
                al_map_rgb(255, 0, 0), 3.0 / cam_zoom
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
        
        //Reference.
        if(
            reference_bitmap &&
            (show_reference || mode == EDITOR_MODE_REFERENCE)
        ) {
            al_draw_tinted_scaled_bitmap(
                reference_bitmap,
                map_alpha(reference_a),
                0, 0,
                al_get_bitmap_width(reference_bitmap),
                al_get_bitmap_height(reference_bitmap),
                reference_pos.x, reference_pos.y,
                reference_size.x, reference_size.y,
                0
            );
        }
        
    } al_reset_clipping_rectangle();
    
    al_use_transform(&identity_transform);
    
    //Cross-section graph.
    if(mode == EDITOR_MODE_REVIEW && show_cross_section) {
    
        dist cross_section_world_length(
            cross_section_points[0], cross_section_points[1]
        );
        float proportion =
            (cross_section_window_end.x - cross_section_window_start.x) /
            cross_section_world_length.to_float();
            
        al_draw_filled_rectangle(
            cross_section_window_start.x, cross_section_window_start.y,
            cross_section_window_end.x, cross_section_window_end.y,
            al_map_rgb(0, 0, 64)
        );
        
        if(show_cross_section_grid) {
            al_draw_filled_rectangle(
                cross_section_z_window_start.x, cross_section_z_window_start.y,
                cross_section_z_window_end.x, cross_section_z_window_end.y,
                al_map_rgb(0, 0, 0)
            );
        }
        
        sector* cs_left_sector =
            get_sector(cross_section_points[0], NULL, false);
        sector* cs_right_sector =
            get_sector(cross_section_points[1], NULL, false);
        struct split_info {
            sector* sector_ptrs[2];
            float ur;
            float ul;
            split_info(
                sector* s1, sector* s2, const float ur, const float ul
            ) {
                sector_ptrs[0] = s1;
                sector_ptrs[1] = s2;
                this->ur = ur;
                this->ul = ul;
            }
        };
        vector<split_info> splits;
        for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
            edge* e_ptr = cur_area_data.edges[e];
            float ur = 0;
            float ul = 0;
            if(
                lines_intersect(
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    point(
                        cross_section_points[0].x, cross_section_points[0].y
                    ),
                    point(
                        cross_section_points[1].x, cross_section_points[1].y
                    ),
                    &ur, &ul
                )
            ) {
                splits.push_back(
                    split_info(e_ptr->sectors[0], e_ptr->sectors[1], ur, ul)
                );
            }
        }
        
        if(!splits.empty()) {
            sort(
                splits.begin(), splits.end(),
            [] (split_info i1, split_info i2) -> bool {
                return i1.ur < i2.ur;
            }
            );
            
            splits.insert(
                splits.begin(),
                split_info(cs_left_sector, cs_left_sector, 0, 0)
            );
            splits.push_back(
                split_info(cs_right_sector, cs_right_sector, 1, 1)
            );
            
            for(size_t s = 1; s < splits.size(); ++s) {
                if(splits[s].sector_ptrs[0] != splits[s - 1].sector_ptrs[1]) {
                    swap(splits[s].sector_ptrs[0], splits[s].sector_ptrs[1]);
                }
            }
            
            float lowest_z = 0;
            bool got_lowest_z = false;
            for(size_t sp = 1; sp < splits.size(); ++sp) {
                for(size_t se = 0; se < 2; ++se) {
                    if(
                        splits[sp].sector_ptrs[se] &&
                        (
                            splits[sp].sector_ptrs[se]->z < lowest_z ||
                            !got_lowest_z
                        )
                    ) {
                        lowest_z = splits[sp].sector_ptrs[se]->z;
                        got_lowest_z = true;
                    }
                }
            }
            
            int ocr_x, ocr_y, ocr_w, ocr_h;
            al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
            al_set_clipping_rectangle(
                cross_section_window_start.x, cross_section_window_start.y,
                cross_section_window_end.x - cross_section_window_start.x,
                cross_section_window_end.y - cross_section_window_start.y
            );
            
            for(size_t s = 1; s < splits.size(); ++s) {
                if(!splits[s].sector_ptrs[0]) continue;
                draw_cross_section_sector(
                    splits[s - 1].ur, splits[s].ur, proportion,
                    lowest_z, splits[s].sector_ptrs[0]
                );
            }
            
            sector* central_sector = NULL;
            for(size_t s = 1; s < splits.size(); ++s) {
                if(splits[s].ur > 0.5) {
                    central_sector = splits[s].sector_ptrs[0];
                    break;
                }
            }
            
            if(central_sector) {
                float pikmin_silhouette_w =
                    standard_pikmin_radius * 2.0 * proportion;
                float pikmin_silhouette_h =
                    standard_pikmin_height * proportion;
                float pikmin_silhouette_pivot_x =
                    (
                        cross_section_window_start.x +
                        cross_section_window_end.x
                    ) / 2.0;
                float pikmin_silhouette_pivot_y =
                    cross_section_window_end.y - 8 -
                    ((central_sector->z - lowest_z) * proportion);
                al_draw_tinted_scaled_bitmap(
                    bmp_pikmin_silhouette,
                    al_map_rgba(255, 255, 255, 128),
                    0, 0,
                    al_get_bitmap_width(bmp_pikmin_silhouette),
                    al_get_bitmap_height(bmp_pikmin_silhouette),
                    pikmin_silhouette_pivot_x - pikmin_silhouette_w / 2.0,
                    pikmin_silhouette_pivot_y - pikmin_silhouette_h,
                    pikmin_silhouette_w, pikmin_silhouette_h,
                    0
                );
            }
            
            al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
            
            float highest_z =
                lowest_z + cross_section_window_end.y / proportion;
                
            if(show_cross_section_grid) {
                for(float z = lowest_z; z <= highest_z; z += 50) {
                    float line_y =
                        cross_section_window_end.y - 8 -
                        ((z - lowest_z) * proportion);
                    al_draw_line(
                        cross_section_window_start.x, line_y,
                        cross_section_z_window_start.x + 6, line_y,
                        al_map_rgb(255, 255, 255), 1
                    );
                    
                    draw_scaled_text(
                        font_builtin, al_map_rgb(255, 255, 255),
                        point(
                            (cross_section_z_window_start.x + 8),
                            line_y
                        ),
                        point(1, 1),
                        ALLEGRO_ALIGN_LEFT, 1, i2s(z)
                    );
                }
            }
            
        } else {
        
            draw_scaled_text(
                font_builtin, al_map_rgb(255, 255, 255),
                point(
                    (
                        cross_section_window_start.x +
                        cross_section_window_end.x
                    ) * 0.5,
                    (
                        cross_section_window_start.y +
                        cross_section_window_end.y
                    ) * 0.5
                ),
                point(1, 1), ALLEGRO_ALIGN_CENTER, 1,
                "Please cross\nsome edges."
            );
            
        }
        
        float cursor_segment_ratio = 0;
        point cursor_line_point =
            get_closest_point_in_line(
                cross_section_points[0], cross_section_points[1],
                point(mouse_cursor_w.x, mouse_cursor_w.y),
                &cursor_segment_ratio
            );
        if(cursor_segment_ratio >= 0 && cursor_segment_ratio <= 1) {
            al_draw_line(
                cross_section_window_start.x +
                (cross_section_window_end.x - cross_section_window_start.x) *
                cursor_segment_ratio,
                cross_section_window_start.y,
                cross_section_window_start.x +
                (cross_section_window_end.x - cross_section_window_start.x) *
                cursor_segment_ratio,
                cross_section_window_end.y,
                al_map_rgba(255, 255, 255, 128), 1
            );
        }
        
        float cross_section_x2 =
            show_cross_section_grid ? cross_section_z_window_end.x :
            cross_section_window_end.x;
        al_draw_line(
            cross_section_window_start.x, cross_section_window_end.y + 1,
            cross_section_x2 + 2, cross_section_window_end.y + 1,
            al_map_rgb(160, 96, 96), 2
        );
        al_draw_line(
            cross_section_x2 + 1, cross_section_window_start.y,
            cross_section_x2 + 1, cross_section_window_end.y + 2,
            al_map_rgb(160, 96, 96), 2
        );
    }
    
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draws a sector on the cross-section view.
 * *_ratio:    Where the sector starts/ends on the graph ([0, 1]).
 * proportion: Ratio of how much to resize the heights.
 * lowest_z:   What z coordinate represents the bottom of the graph.
 * sector_ptr: Pointer to the sector to draw.
 */
void area_editor_old::draw_cross_section_sector(
    const float start_ratio, const float end_ratio, const float proportion,
    const float lowest_z, sector* sector_ptr
) {
    float rectangle_x1 =
        cross_section_window_start.x +
        (cross_section_window_end.x - cross_section_window_start.x) *
        start_ratio;
    float rectangle_x2 =
        cross_section_window_start.x +
        (cross_section_window_end.x - cross_section_window_start.x) *
        end_ratio;
    float rectangle_y =
        cross_section_window_end.y - 8 -
        ((sector_ptr->z - lowest_z) * proportion);
        
    al_draw_filled_rectangle(
        rectangle_x1, rectangle_y,
        rectangle_x2 + 1, cross_section_window_end.y + 1,
        al_map_rgb(0, 64, 0)
    );
    al_draw_line(
        rectangle_x1 + 0.5, rectangle_y,
        rectangle_x1 + 0.5, cross_section_window_end.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangle_x2 + 0.5, rectangle_y,
        rectangle_x2 + 0.5, cross_section_window_end.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangle_x1, rectangle_y + 0.5,
        rectangle_x2, rectangle_y + 0.5,
        al_map_rgb(192, 192, 192), 1
    );
    
}


/* ----------------------------------------------------------------------------
 * Draws debug text, used to identify edges, sectors, or vertexes.
 * color: Text color.
 * where: Where to draw, in world coordinates.
 * text:  Text to show.
 */
void area_editor_old::draw_debug_text(
    const ALLEGRO_COLOR color, const point &where, const string &text
) {
    int dw = 0;
    int dh = 0;
    al_get_text_dimensions(
        font_builtin, text.c_str(),
        NULL, NULL, &dw, &dh
    );
    
    float bbox_w = (dw * DEBUG_TEXT_SCALE) / cam_zoom;
    float bbox_h = (dh * DEBUG_TEXT_SCALE) / cam_zoom;
    
    al_draw_filled_rectangle(
        where.x - bbox_w * 0.5, where.y - bbox_h * 0.5,
        where.x + bbox_w * 0.5, where.y + bbox_h * 0.5,
        al_map_rgba(0, 0, 0, 128)
    );
    
    draw_scaled_text(
        font_builtin, color,
        where,
        point(
            DEBUG_TEXT_SCALE / cam_zoom,
            DEBUG_TEXT_SCALE / cam_zoom
        ),
        ALLEGRO_ALIGN_CENTER, 1,
        text
    );
}
