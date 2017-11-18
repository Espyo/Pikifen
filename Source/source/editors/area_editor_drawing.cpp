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
#include "../vars.h"

void area_editor::do_drawing() {
    gui->draw();
    
    al_use_transform(&world_to_screen_transform);
    al_set_clipping_rectangle(0, 0, gui_x, status_bar_y);
    
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    if(sub_state != EDITOR_SUB_STATE_TEXTURE_VIEW) {
    
        float selection_min_opacity = 0.25f;
        float selection_max_opacity = 0.75f;
        float textures_opacity = 0.4f;
        float edges_opacity = 0.25f;
        float grid_opacity = 1.0f;
        float mob_opacity = 0.15f;
        if(
            state == EDITOR_STATE_LAYOUT ||
            state == EDITOR_STATE_ASB ||
            state == EDITOR_STATE_ASA
        ) {
            textures_opacity = 0.5f;
            edges_opacity = 1.0f;
            
        } else if(state == EDITOR_STATE_MOBS) {
            mob_opacity = 1.0f;
            
        } else if(state == EDITOR_STATE_MAIN || state == EDITOR_STATE_REVIEW) {
            textures_opacity = 0.6f;
            edges_opacity = 0.5f;
            grid_opacity = 0.3f;
            mob_opacity = 0.75f;
            
        }
        if(state == EDITOR_STATE_ASA) {
            selection_min_opacity = 0.0f;
            selection_max_opacity = 0.0f;
            textures_opacity = 1.0f;
        }
        
        float selection_opacity =
            selection_min_opacity +
            (sin(selection_effect) + 1) *
            (selection_max_opacity - selection_min_opacity) / 2.0;
            
        //Sectors.
        size_t n_sectors = cur_area_data.sectors.size();
        for(size_t s = 0; s < n_sectors; ++s) {
            sector* s_ptr;
            if(
                moving &&
                (
                    state == EDITOR_STATE_ASA ||
                    state == EDITOR_STATE_ASB ||
                    state == EDITOR_STATE_LAYOUT
                )
            ) {
                s_ptr = pre_move_area_data.sectors[s];
            } else {
                s_ptr = cur_area_data.sectors[s];
            }
            
            draw_sector_texture(s_ptr, point(), 1.0, textures_opacity);
            
            bool selected =
                selected_sectors.find(s_ptr) != selected_sectors.end();
            bool valid = true;
            
            if(non_simples.find(s_ptr) != non_simples.end()) {
                valid = false;
            }
            if(s_ptr == problem_sector_ptr) {
                valid = false;
            }
            
            if(selected || !valid) {
                for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
                
                    ALLEGRO_VERTEX av[3];
                    for(size_t v = 0; v < 3; ++v) {
                        if(!valid) {
                            av[v].color = al_map_rgba(160, 16, 16, 224);
                        } else {
                            av[v].color =
                                al_map_rgba(
                                    SELECTION_COLOR[0],
                                    SELECTION_COLOR[1],
                                    SELECTION_COLOR[2],
                                    selection_opacity * 0.5 * 255
                                );
                        }
                        av[v].u = 0;
                        av[v].v = 0;
                        av[v].x = s_ptr->triangles[t].points[v]->x;
                        av[v].y = s_ptr->triangles[t].points[v]->y;
                        av[v].z = 0;
                    }
                    
                    al_draw_prim(
                        av, NULL, NULL,
                        0, 3, ALLEGRO_PRIM_TRIANGLE_LIST
                    );
                    
                }
            }
        }
        
        //Grid.
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
            ALLEGRO_COLOR c = al_map_rgba(48, 48, 48, grid_opacity * 255);
            bool draw_line = true;
            
            if(fmod(x, grid_interval * 2) == 0) {
                c = al_map_rgba(64, 64, 64, grid_opacity * 255);
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
            ALLEGRO_COLOR c = al_map_rgba(48, 48, 48, grid_opacity * 255);
            bool draw_line = true;
            
            if(fmod(y, grid_interval * 2) == 0) {
                c = al_map_rgba(64, 64, 64, grid_opacity * 255);
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
            al_map_rgba(192, 192, 224, grid_opacity * 255),
            1.0 / cam_zoom
        );
        al_draw_line(
            0, -(DEF_GRID_INTERVAL * 2), 0, DEF_GRID_INTERVAL * 2,
            al_map_rgba(192, 192, 224, grid_opacity * 255),
            1.0 / cam_zoom
        );
        
        //Edges.
        size_t n_edges = cur_area_data.edges.size();
        for(size_t e = 0; e < n_edges; ++e) {
            edge* e_ptr = cur_area_data.edges[e];
            
            if(!is_edge_valid(e_ptr)) continue;
            
            bool one_sided = true;
            bool same_z = false;
            bool valid = true;
            bool selected = false;
            
            if(problem_sector_ptr) {
                if(
                    e_ptr->sectors[0] == problem_sector_ptr ||
                    e_ptr->sectors[1] == problem_sector_ptr
                ) {
                    valid = false;
                }
                
            }
            if(
                problem_edge_intersection.e1 == e_ptr ||
                problem_edge_intersection.e2 == e_ptr
            ) {
                valid = false;
            }
            
            if(lone_edges.find(e_ptr) != lone_edges.end()) {
                valid = false;
            }
            
            if(
                non_simples.find(e_ptr->sectors[0]) != non_simples.end() ||
                non_simples.find(e_ptr->sectors[1]) != non_simples.end()
            ) {
                valid = false;
            }
            
            if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
            
            if(
                !one_sided &&
                e_ptr->sectors[0]->z == e_ptr->sectors[1]->z &&
                e_ptr->sectors[0]->type == e_ptr->sectors[1]->type
            ) {
                same_z = true;
            }
            
            if(selected_edges.find(e_ptr) != selected_edges.end()) {
                selected = true;
            }
            
            al_draw_line(
                e_ptr->vertexes[0]->x,
                e_ptr->vertexes[0]->y,
                e_ptr->vertexes[1]->x,
                e_ptr->vertexes[1]->y,
                (
                    selected ?
                    al_map_rgba(
                        SELECTION_COLOR[0],
                        SELECTION_COLOR[1],
                        SELECTION_COLOR[2],
                        selection_opacity * 255
                    ) :
                    !valid ?
                    al_map_rgba(192, 32,  32,  edges_opacity * 255) :
                    one_sided ?
                    al_map_rgba(255, 255, 255, edges_opacity * 255) :
                    same_z ?
                    al_map_rgba(128, 128, 128, edges_opacity * 255) :
                    al_map_rgba(192, 192, 192, edges_opacity * 255)
                ),
                (selected ? 3.0 : 2.0) / cam_zoom
            );
            
            if(debug_triangulation && !selected_sectors.empty()) {
                sector* s_ptr = *selected_sectors.begin();
                for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
                    triangle* t_ptr = &s_ptr->triangles[t];
                    al_draw_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        al_map_rgb(192, 0, 160),
                        1.0 / cam_zoom
                    );
                }
            }
            
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
                        "-" :
                        i2s(e_ptr->sector_nrs[0])
                    ),
                    1
                );
                
                draw_debug_text(
                    al_map_rgb(192, 255, 192),
                    point(
                        middle.x + cos(angle - M_PI_2) * 4,
                        middle.y + sin(angle - M_PI_2) * 4
                    ),
                    (
                        e_ptr->sector_nrs[1] == INVALID ?
                        "-" :
                        i2s(e_ptr->sector_nrs[1])
                    ),
                    2
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
        if(state == EDITOR_STATE_LAYOUT) {
            size_t n_vertexes = cur_area_data.vertexes.size();
            for(size_t v = 0; v < n_vertexes; ++v) {
                vertex* v_ptr = cur_area_data.vertexes[v];
                bool selected =
                    (selected_vertexes.find(v_ptr) != selected_vertexes.end());
                bool valid =
                    v_ptr != problem_vertex_ptr;
                al_draw_filled_circle(
                    v_ptr->x,
                    v_ptr->y,
                    3.0 / cam_zoom,
                    selected ?
                    al_map_rgba(
                        SELECTION_COLOR[0],
                        SELECTION_COLOR[1],
                        SELECTION_COLOR[2],
                        selection_opacity * 255
                    ) :
                    !valid ?
                    al_map_rgb(192, 32, 32) :
                    al_map_rgba(80, 160, 255, edges_opacity * 255)
                );
                
                if(debug_vertex_nrs) {
                    draw_debug_text(
                        al_map_rgb(192, 192, 255),
                        point(v_ptr->x, v_ptr->y), i2s(v)
                    );
                }
            }
        }
        
        //Mobs.
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            bool valid = m_ptr->type != NULL;
            
            float radius = get_mob_gen_radius(m_ptr);
            ALLEGRO_COLOR c =
                change_alpha(m_ptr->category->editor_color, mob_opacity * 255);
            if(m_ptr == problem_mob_ptr) {
                c = al_map_rgb(192, 32, 32);
            }
            
            al_draw_filled_circle(
                m_ptr->pos.x, m_ptr->pos.y,
                radius, c
            );
            
            float lrw = cos(m_ptr->angle) * radius;
            float lrh = sin(m_ptr->angle) * radius;
            float lt = radius / 8.0;
            
            al_draw_line(
                m_ptr->pos.x - lrw * 0.8, m_ptr->pos.y - lrh * 0.8,
                m_ptr->pos.x + lrw * 0.8, m_ptr->pos.y + lrh * 0.8,
                al_map_rgba(0, 0, 0, mob_opacity * 255), lt
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
                al_map_rgba(0, 0, 0, mob_opacity * 255)
            );
            
            if(selected_mobs.find(m_ptr) != selected_mobs.end()) {
                al_draw_filled_circle(
                    m_ptr->pos.x, m_ptr->pos.y, radius,
                    al_map_rgba(
                        SELECTION_COLOR[0],
                        SELECTION_COLOR[1],
                        SELECTION_COLOR[2],
                        selection_opacity * 255
                    )
                );
            }
            
        }
        
        //Paths.
        if(state == EDITOR_STATE_PATHS) {
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); l++) {
                    path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                    bool one_way = !(s_ptr->links[l].end_ptr->has_link(s_ptr));
                    bool selected =
                        selected_path_links.find(make_pair(s_ptr, s2_ptr)) !=
                        selected_path_links.end();
                        
                    al_draw_line(
                        s_ptr->pos.x, s_ptr->pos.y,
                        s2_ptr->pos.x, s2_ptr->pos.y,
                        (
                            selected ?
                            al_map_rgba(
                                SELECTION_COLOR[0],
                                SELECTION_COLOR[1],
                                SELECTION_COLOR[2],
                                selection_opacity * 255
                            ) :
                            one_way ? al_map_rgb(192, 128, 224) :
                            al_map_rgb(0, 80, 224)
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
                            al_map_rgb(192, 128, 224)
                        );
                    }
                }
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                al_draw_filled_circle(
                    s_ptr->pos.x, s_ptr->pos.y,
                    PATH_STOP_RADIUS,
                    al_map_rgb(80, 192, 192)
                );
                
                if(
                    selected_path_stops.find(s_ptr) !=
                    selected_path_stops.end()
                ) {
                    al_draw_filled_circle(
                        s_ptr->pos.x, s_ptr->pos.y, PATH_STOP_RADIUS,
                        al_map_rgba(
                            SELECTION_COLOR[0],
                            SELECTION_COLOR[1],
                            SELECTION_COLOR[2],
                            selection_opacity * 255
                        )
                    );
                }
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
                    al_map_rgb(192, 128, 32), 2.0 / cam_zoom
                );
            }
            
            if(show_path_preview) {
                //Draw the lines of the path.
                if(path_preview.empty()) {
                    al_draw_line(
                        path_preview_checkpoints[0].x,
                        path_preview_checkpoints[0].y,
                        path_preview_checkpoints[1].x,
                        path_preview_checkpoints[1].y,
                        al_map_rgb(240, 128, 128), 3.0 / cam_zoom
                    );
                } else {
                    al_draw_line(
                        path_preview_checkpoints[0].x,
                        path_preview_checkpoints[0].y,
                        path_preview[0]->pos.x,
                        path_preview[0]->pos.y,
                        al_map_rgb(240, 128, 128), 3.0 / cam_zoom
                    );
                    for(size_t s = 0; s < path_preview.size() - 1; ++s) {
                        al_draw_line(
                            path_preview[s]->pos.x,
                            path_preview[s]->pos.y,
                            path_preview[s + 1]->pos.x,
                            path_preview[s + 1]->pos.y,
                            al_map_rgb(240, 128, 128), 3.0 / cam_zoom
                        );
                    }
                    
                    al_draw_line(
                        path_preview.back()->pos.x,
                        path_preview.back()->pos.y,
                        path_preview_checkpoints[1].x,
                        path_preview_checkpoints[1].y,
                        al_map_rgb(240, 128, 128), 3.0 / cam_zoom
                    );
                }
                
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
                        al_map_rgb(240, 224, 160)
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
            }
        }
        
        //Tree shadows.
        if(
            state == EDITOR_STATE_DETAILS ||
            (sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW && show_shadows)
        ) {
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                al_draw_filled_rectangle(
                    s_ptr->center.x - s_ptr->size.x * 0.5,
                    s_ptr->center.y - s_ptr->size.y * 0.5,
                    s_ptr->center.x + s_ptr->size.x * 0.5,
                    s_ptr->center.y + s_ptr->size.y * 0.5,
                    al_map_rgba(255, 255, 255, 96 * (s_ptr->alpha / 255.0))
                );
                draw_sprite(
                    s_ptr->bitmap, s_ptr->center, s_ptr->size,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(state == EDITOR_STATE_DETAILS) {
                    point min_coords, max_coords;
                    get_shadow_bounding_box(
                        s_ptr, &min_coords, &max_coords
                    );
                    
                    al_draw_rectangle(
                        min_coords.x, min_coords.y, max_coords.x, max_coords.y,
                        (
                            s_ptr == selected_shadow ?
                            al_map_rgb(224, 224, 64) :
                            al_map_rgb(128, 128, 64)
                        ),
                        2.0 / cam_zoom
                    );
                }
            }
        }
        
        //Reference image.
        if(
            reference_bitmap &&
            (show_reference || state == EDITOR_STATE_TOOLS)
        ) {
            al_draw_tinted_scaled_bitmap(
                reference_bitmap,
                map_alpha(reference_a),
                0, 0,
                al_get_bitmap_width(reference_bitmap),
                al_get_bitmap_height(reference_bitmap),
                reference_transformation.center.x -
                reference_transformation.size.x / 2.0,
                reference_transformation.center.y -
                reference_transformation.size.y / 2.0,
                reference_transformation.size.x,
                reference_transformation.size.y,
                0
            );
            
            if(state == EDITOR_STATE_TOOLS) {
                reference_transformation.draw_handles();
            }
        }
        
        //Sector drawing.
        if(sub_state == EDITOR_SUB_STATE_DRAWING) {
            for(size_t n = 1; n < drawing_nodes.size(); ++n) {
                al_draw_line(
                    drawing_nodes[n - 1].snapped_spot.x,
                    drawing_nodes[n - 1].snapped_spot.y,
                    drawing_nodes[n].snapped_spot.x,
                    drawing_nodes[n].snapped_spot.y,
                    al_map_rgb(128, 255, 128),
                    3.0 / cam_zoom
                );
            }
            ALLEGRO_COLOR new_line_color =
                interpolate_color(
                    new_sector_error_tint_timer.get_ratio_left(),
                    1, 0,
                    al_map_rgb(255, 0, 0),
                    al_map_rgb(64, 255, 64)
                );
            if(!drawing_nodes.empty()) {
                point hotspot = snap_to_grid(mouse_cursor_w);
                al_draw_line(
                    drawing_nodes.back().snapped_spot.x,
                    drawing_nodes.back().snapped_spot.y,
                    hotspot.x,
                    hotspot.y,
                    new_line_color,
                    3.0 / cam_zoom
                );
            }
        }
        
        //New circular sector drawing.
        if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
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
                    3.0 / cam_zoom
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
        
        //Path drawing.
        if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
            if(path_drawing_stop_1) {
                point hotspot = snap_to_grid(mouse_cursor_w);
                al_draw_line(
                    path_drawing_stop_1->pos.x,
                    path_drawing_stop_1->pos.y,
                    hotspot.x,
                    hotspot.y,
                    al_map_rgb(64, 255, 64),
                    3.0 / cam_zoom
                );
            }
        }
        
        //Selection box.
        if(selecting) {
            al_draw_rectangle(
                selection_start.x,
                selection_start.y,
                selection_end.x,
                selection_end.y,
                al_map_rgb(
                    SELECTION_COLOR[0],
                    SELECTION_COLOR[1],
                    SELECTION_COLOR[2]
                ),
                2.0 / cam_zoom
                
            );
        }
        
        //New thing marker.
        if(
            sub_state == EDITOR_SUB_STATE_DRAWING ||
            sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR ||
            sub_state == EDITOR_SUB_STATE_NEW_MOB ||
            sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
            sub_state == EDITOR_SUB_STATE_PATH_DRAWING ||
            sub_state == EDITOR_SUB_STATE_NEW_SHADOW
        ) {
            point marker = mouse_cursor_w;
            marker = snap_to_grid(marker);
            al_draw_line(
                marker.x - 16, marker.y, marker.x + 16, marker.y,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                marker.x, marker.y - 16, marker.x, marker.y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
    }
    
    al_reset_clipping_rectangle();
    al_use_transform(&identity_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draws debug text, used to identify edges, sectors, or vertexes.
 * color: Text color.
 * where: Where to draw, in world coordinates.
 * text:  Text to show.
 * dots:  How many dots to draw above the text. 0, 1, or 2.
 */
void area_editor::draw_debug_text(
    const ALLEGRO_COLOR color, const point &where, const string &text,
    const unsigned char dots
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
    
    if(dots > 0) {
        al_draw_filled_rectangle(
            where.x - 3.0 / cam_zoom,
            where.y + bbox_h * 0.5,
            where.x + 3.0 / cam_zoom,
            where.y + bbox_h * 0.5 + 3.0 / cam_zoom,
            al_map_rgba(0, 0, 0, 128)
        );
        
        if(dots == 1) {
            al_draw_filled_rectangle(
                where.x - 1.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 1.0 / cam_zoom,
                where.x + 1.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 3.0 / cam_zoom,
                color
            );
        } else {
            al_draw_filled_rectangle(
                where.x - 3.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 1.0 / cam_zoom,
                where.x - 1.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 3.0 / cam_zoom,
                color
            );
            al_draw_filled_rectangle(
                where.x + 1.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 1.0 / cam_zoom,
                where.x + 3.0 / cam_zoom,
                where.y + bbox_h * 0.5 + 3.0 / cam_zoom,
                color
            );
        }
    }
}
