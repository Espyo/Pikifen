/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include <algorithm>

#include "editor.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {
    //Render what is needed for the GUI.
    //This will also render the canvas in due time.
    ImGui::Render();
    
    //Actually draw the GUI + canvas on-screen.
    al_clear_to_color(al_map_rgb(0, 0, 0));
    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    
    draw_unsaved_changes_warning();
    
    //And the fade manager atop it all.
    game.fade_mgr.draw();
    
    //Finally, swap buffers.
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draw the canvas. This is called as a callback inside the
 * ImGui rendering process.
 */
void area_editor::draw_canvas() {
    al_use_transform(&game.world_to_screen_transform);
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    float lowest_sector_z = 0.0f;
    float highest_sector_z = 0.0f;
    if(
        game.options.area_editor_view_mode == VIEW_MODE_HEIGHTMAP &&
        !game.cur_area_data.sectors.empty()
    ) {
        lowest_sector_z = game.cur_area_data.sectors[0]->z;
        highest_sector_z = lowest_sector_z;
        
        for(size_t s = 1; s < game.cur_area_data.sectors.size(); ++s) {
            lowest_sector_z =
                std::min(lowest_sector_z, game.cur_area_data.sectors[s]->z);
            highest_sector_z =
                std::max(highest_sector_z, game.cur_area_data.sectors[s]->z);
        }
    }
    
    float selection_min_opacity = 0.25f;
    float selection_max_opacity = 0.75f;
    float textures_opacity = 0.4f;
    float edges_opacity = 0.25f;
    float grid_opacity = 1.0f;
    float mob_opacity = 0.15f;
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        textures_opacity = 0.5f;
        edges_opacity = 1.0f;
        break;
        
    } case EDITOR_STATE_MOBS: {
        mob_opacity = 1.0f;
        break;
        
    } case EDITOR_STATE_MAIN:
    case EDITOR_STATE_REVIEW: {
        textures_opacity = 0.6f;
        edges_opacity = 0.5f;
        grid_opacity = 0.3f;
        mob_opacity = 0.75f;
        break;
        
    }
    }
    
    if(sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW) {
        textures_opacity = 1.0f;
        edges_opacity = 0.0f;
        grid_opacity = 0.0f;
        mob_opacity = 0.0f;
    } else if(sub_state == EDITOR_SUB_STATE_OCTEE) {
        quick_preview_timer.start();
    }
    
    if(quick_preview_timer.time_left > 0) {
        float t =
            std::min(
                quick_preview_timer.time_left,
                quick_preview_timer.duration / 2.0f
            );
        selection_min_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                selection_min_opacity, 0.0f
            );
        selection_max_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                selection_max_opacity, 0.0f
            );
        textures_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                textures_opacity, 1.0f
            );
        edges_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                edges_opacity, 0.5f
            );
        grid_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                grid_opacity, 0.0f
            );
        mob_opacity =
            interpolate_number(
                t, 0.0f, quick_preview_timer.duration / 2.0f,
                mob_opacity, 0.0f
            );
    }
    
    float selection_opacity =
        selection_min_opacity +
        (sin(selection_effect) + 1) *
        (selection_max_opacity - selection_min_opacity) / 2.0;
        
    //Sectors.
    if(sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW) {
        update_wall_shadow_buffer(
            game.cam.box[0], game.cam.box[1],
            game.wall_shadow_buffer
        );
    }
    size_t n_sectors = game.cur_area_data.sectors.size();
    for(size_t s = 0; s < n_sectors; ++s) {
        sector* s_ptr;
        if(
            pre_move_area_data &&
            moving &&
            (
                state == EDITOR_STATE_LAYOUT
            )
        ) {
            s_ptr = pre_move_area_data->sectors[s];
        } else {
            s_ptr = game.cur_area_data.sectors[s];
        }
        
        bool view_heightmap = false;
        bool view_brightness = false;
        
        if(
            game.options.area_editor_view_mode == VIEW_MODE_TEXTURES ||
            sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW
        ) {
            draw_sector_texture(s_ptr, point(), 1.0, textures_opacity);
            
            if(sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW) {
                draw_sector_wall_shadows(s_ptr, game.wall_shadow_buffer);
            }
            
        } else if(game.options.area_editor_view_mode == VIEW_MODE_HEIGHTMAP) {
            view_heightmap = true;
            
        } else if(game.options.area_editor_view_mode == VIEW_MODE_BRIGHTNESS) {
            view_brightness = true;
            
        }
        
        bool selected =
            selected_sectors.find(s_ptr) != selected_sectors.end();
        bool valid = true;
        
        if(
            game.cur_area_data.problems.non_simples.find(s_ptr) !=
            game.cur_area_data.problems.non_simples.end()
        ) {
            valid = false;
        }
        if(s_ptr == problem_sector_ptr) {
            valid = false;
        }
        
        if(selected || !valid || view_heightmap || view_brightness) {
            for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
            
                ALLEGRO_VERTEX av[3];
                for(size_t v = 0; v < 3; ++v) {
                    if(!valid) {
                        av[v].color = al_map_rgba(160, 16, 16, 224);
                    } else if(view_brightness) {
                        av[v].color =
                            al_map_rgba(
                                s_ptr->brightness * 0.7,
                                s_ptr->brightness * 0.8,
                                s_ptr->brightness * 0.7,
                                255
                            );
                    } else if(view_heightmap) {
                        unsigned char g =
                            interpolate_number(
                                s_ptr->z, lowest_sector_z, highest_sector_z,
                                0, 224
                            );
                        av[v].color =
                            al_map_rgba(g, g + 31, g, 255);
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
    draw_grid(
        game.options.area_editor_grid_interval,
        al_map_rgba(64, 64, 64, grid_opacity * 255),
        al_map_rgba(48, 48, 48, grid_opacity * 255)
    );
    
    //0,0 marker.
    al_draw_line(
        -(COMFY_DIST * 2), 0,
        COMFY_DIST * 2, 0,
        al_map_rgba(192, 192, 224, grid_opacity * 255),
        1.0f / game.cam.zoom
    );
    al_draw_line(
        0, -(COMFY_DIST * 2), 0,
        COMFY_DIST * 2,
        al_map_rgba(192, 192, 224, grid_opacity * 255),
        1.0f / game.cam.zoom
    );
    
    //Edges.
    size_t n_edges = game.cur_area_data.edges.size();
    for(size_t e = 0; e < n_edges; ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        
        if(!e_ptr->is_valid()) continue;
        
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
        
        if(
            game.cur_area_data.problems.lone_edges.find(e_ptr) !=
            game.cur_area_data.problems.lone_edges.end()
        ) {
            valid = false;
        }
        
        if(
            game.cur_area_data.problems.non_simples.find(e_ptr->sectors[0]) !=
            game.cur_area_data.problems.non_simples.end() ||
            game.cur_area_data.problems.non_simples.find(e_ptr->sectors[1]) !=
            game.cur_area_data.problems.non_simples.end()
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
            (selected ? 3.0 : 2.0) / game.cam.zoom
        );
        
        if(
            state == EDITOR_STATE_LAYOUT &&
            moving &&
            game.options.area_editor_show_edge_length
        ) {
            bool draw_dist = false;
            point other_point;
            if(
                e_ptr->vertexes[0] == move_closest_vertex &&
                selected_vertexes.find(e_ptr->vertexes[1]) ==
                selected_vertexes.end()
            ) {
                other_point.x = e_ptr->vertexes[1]->x;
                other_point.y = e_ptr->vertexes[1]->y;
                draw_dist = true;
            } else if(
                e_ptr->vertexes[1] == move_closest_vertex &&
                selected_vertexes.find(e_ptr->vertexes[0]) ==
                selected_vertexes.end()
            ) {
                other_point.x = e_ptr->vertexes[0]->x;
                other_point.y = e_ptr->vertexes[0]->y;
                draw_dist = true;
            }
            
            if(draw_dist) {
                draw_line_dist(
                    point(move_closest_vertex->x, move_closest_vertex->y),
                    other_point
                );
            }
        }
        
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
                    1.0f / game.cam.zoom
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
                    middle.x + cos(angle + TAU / 4) * 4,
                    middle.y + sin(angle + TAU / 4) * 4
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
                    middle.x + cos(angle - TAU / 4) * 4,
                    middle.y + sin(angle - TAU / 4) * 4
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
        size_t n_vertexes = game.cur_area_data.vertexes.size();
        for(size_t v = 0; v < n_vertexes; ++v) {
            vertex* v_ptr = game.cur_area_data.vertexes[v];
            bool selected =
                (selected_vertexes.find(v_ptr) != selected_vertexes.end());
            bool valid =
                v_ptr != problem_vertex_ptr;
            draw_filled_diamond(
                point(v_ptr->x, v_ptr->y),
                3.0 / game.cam.zoom,
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
    
    //Selection transformation widget.
    if(
        game.options.area_editor_sel_trans &&
        selected_vertexes.size() >= 2 &&
        (!moving || cur_transformation_widget.is_moving_handle())
    ) {
        cur_transformation_widget.draw(
            &selection_center,
            &selection_size,
            &selection_angle,
            1.0f / game.cam.zoom
        );
    }
    
    //Mobs.
    if(state == EDITOR_STATE_MOBS) {
        for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
            mob_gen* m2_ptr = NULL;
            
            for(size_t l = 0; l < m_ptr->links.size(); ++l) {
                m2_ptr = m_ptr->links[l];
                if(!m_ptr->type) continue;
                if(!m2_ptr->type) continue;
                
                al_draw_line(
                    m_ptr->pos.x, m_ptr->pos.y,
                    m2_ptr->pos.x, m2_ptr->pos.y,
                    al_map_rgb(160, 224, 64),
                    MOB_LINK_THICKNESS / game.cam.zoom
                );
                
                if(game.cam.zoom >= 0.25) {
                    float angle =
                        get_angle(m_ptr->pos, m2_ptr->pos);
                    point start = point(m_ptr->type->radius, 0);
                    start = rotate_point(start, angle);
                    start += m_ptr->pos;
                    point end = point(m2_ptr->type->radius, 0);
                    end = rotate_point(end, angle + TAU / 2.0);
                    end += m2_ptr->pos;
                    
                    point pivot(
                        start.x + (end.x - start.x) * 0.55,
                        start.y + (end.y - start.y) * 0.55
                    );
                    const float delta =
                        (MOB_LINK_THICKNESS * 4) / game.cam.zoom;
                        
                    al_draw_filled_triangle(
                        pivot.x + cos(angle) * delta,
                        pivot.y + sin(angle) * delta,
                        pivot.x + cos(angle + TAU / 4) * delta,
                        pivot.y + sin(angle + TAU / 4) * delta,
                        pivot.x + cos(angle - TAU / 4) * delta,
                        pivot.y + sin(angle - TAU / 4) * delta,
                        al_map_rgb(160, 224, 64)
                    );
                }
            }
        }
    }
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        float radius = get_mob_gen_radius(m_ptr);
        ALLEGRO_COLOR c =
            change_alpha(m_ptr->category->editor_color, mob_opacity * 255);
        if(m_ptr == problem_mob_ptr) {
            c = al_map_rgb(255, 0, 0);
        }
        
        if(m_ptr->type && m_ptr->type->rectangular_dim.x != 0) {
            draw_rotated_rectangle(
                m_ptr->pos, m_ptr->type->rectangular_dim,
                m_ptr->angle, c, 1.0f / game.cam.zoom
            );
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
            tx1 + cos(m_ptr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty2 =
            ty1 + sin(m_ptr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float tx3 =
            tx1 + cos(m_ptr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty3 =
            ty1 + sin(m_ptr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
            
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
            
            if(
                game.options.area_editor_show_territory &&
                m_ptr->type->territory_radius > 0
            ) {
                al_draw_circle(
                    m_ptr->pos.x, m_ptr->pos.y, m_ptr->type->territory_radius,
                    al_map_rgb(240, 240, 192), 1.0f / game.cam.zoom
                );
            }
        }
        
    }
    
    //Paths.
    if(state == EDITOR_STATE_PATHS) {
        for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = game.cur_area_data.path_stops[s];
            for(size_t l = 0; l < s_ptr->links.size(); l++) {
                path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                bool one_way =
                    s_ptr->links[l].end_ptr->get_link(s_ptr) == INVALID;
                bool selected =
                    selected_path_links.find(std::make_pair(s_ptr, s2_ptr)) !=
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
                    PATH_LINK_THICKNESS / game.cam.zoom
                );
                
                if(debug_path_nrs && (one_way || s < s_ptr->links[l].end_nr)) {
                    point middle = (s_ptr->pos + s2_ptr->pos) / 2.0f;
                    float angle = get_angle(s_ptr->pos, s2_ptr->pos);
                    draw_debug_text(
                        al_map_rgb(96, 104, 224),
                        point(
                            middle.x + cos(angle + TAU / 4) * 4,
                            middle.y + sin(angle + TAU / 4) * 4
                        ),
                        f2s(s_ptr->links[l].distance)
                    );
                }
                
                if(one_way) {
                    //Draw a triangle down the middle.
                    float mid_x =
                        (s_ptr->pos.x + s2_ptr->pos.x) / 2.0f;
                    float mid_y =
                        (s_ptr->pos.y + s2_ptr->pos.y) / 2.0f;
                    float angle =
                        get_angle(s_ptr->pos, s2_ptr->pos);
                    const float delta =
                        (PATH_LINK_THICKNESS * 4) / game.cam.zoom;
                        
                    al_draw_filled_triangle(
                        mid_x + cos(angle) * delta,
                        mid_y + sin(angle) * delta,
                        mid_x + cos(angle + TAU / 4) * delta,
                        mid_y + sin(angle + TAU / 4) * delta,
                        mid_x + cos(angle - TAU / 4) * delta,
                        mid_y + sin(angle - TAU / 4) * delta,
                        al_map_rgb(192, 128, 224)
                    );
                }
            }
        }
        
        for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = game.cur_area_data.path_stops[s];
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
            
            if(debug_path_nrs) {
                draw_debug_text(
                    al_map_rgb(80, 192, 192), s_ptr->pos, i2s(s)
                );
            }
        }
        
        if(show_closest_stop) {
            path_stop* closest = NULL;
            dist closest_dist;
            for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = game.cur_area_data.path_stops[s];
                dist d(game.mouse_cursor_w, s_ptr->pos);
                
                if(!closest || d < closest_dist) {
                    closest = s_ptr;
                    closest_dist = d;
                }
            }
            
            if(closest) {
                al_draw_line(
                    game.mouse_cursor_w.x, game.mouse_cursor_w.y,
                    closest->pos.x, closest->pos.y,
                    al_map_rgb(192, 128, 32), 2.0 / game.cam.zoom
                );
            }
        }
        
        if(show_path_preview) {
            //Draw the lines of the path.
            if(path_preview.empty()) {
                al_draw_line(
                    path_preview_checkpoints[0].x,
                    path_preview_checkpoints[0].y,
                    path_preview_checkpoints[1].x,
                    path_preview_checkpoints[1].y,
                    al_map_rgb(240, 128, 128), 3.0 / game.cam.zoom
                );
            } else {
                al_draw_line(
                    path_preview_checkpoints[0].x,
                    path_preview_checkpoints[0].y,
                    path_preview[0]->pos.x,
                    path_preview[0]->pos.y,
                    al_map_rgb(240, 128, 128), 3.0 / game.cam.zoom
                );
                for(size_t s = 0; s < path_preview.size() - 1; ++s) {
                    al_draw_line(
                        path_preview[s]->pos.x,
                        path_preview[s]->pos.y,
                        path_preview[s + 1]->pos.x,
                        path_preview[s + 1]->pos.y,
                        al_map_rgb(240, 128, 128), 3.0 / game.cam.zoom
                    );
                }
                
                al_draw_line(
                    path_preview.back()->pos.x,
                    path_preview.back()->pos.y,
                    path_preview_checkpoints[1].x,
                    path_preview_checkpoints[1].y,
                    al_map_rgb(240, 128, 128), 3.0 / game.cam.zoom
                );
            }
            
            //Draw the checkpoints.
            for(unsigned char c = 0; c < 2; ++c) {
                string letter = (c == 0 ? "A" : "B");
                
                al_draw_filled_rectangle(
                    path_preview_checkpoints[c].x -
                    (PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom),
                    path_preview_checkpoints[c].y -
                    (PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom),
                    path_preview_checkpoints[c].x +
                    (PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom),
                    path_preview_checkpoints[c].y +
                    (PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom),
                    al_map_rgb(240, 224, 160)
                );
                draw_scaled_text(
                    game.fonts.builtin, al_map_rgb(0, 64, 64),
                    path_preview_checkpoints[c],
                    point(
                        POINT_LETTER_TEXT_SCALE / game.cam.zoom,
                        POINT_LETTER_TEXT_SCALE / game.cam.zoom
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
        for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        
            tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
            if(
                sub_state != EDITOR_SUB_STATE_TEXTURE_VIEW &&
                s_ptr == selected_shadow
            ) {
                //Draw a white rectangle to contrast the shadow better.
                ALLEGRO_TRANSFORM tra, current;
                al_identity_transform(&tra);
                al_rotate_transform(&tra, s_ptr->angle);
                al_translate_transform(
                    &tra, s_ptr->center.x, s_ptr->center.y
                );
                al_copy_transform(&current, al_get_current_transform());
                al_compose_transform(&tra, &current);
                al_use_transform(&tra);
                
                al_draw_filled_rectangle(
                    -s_ptr->size.x / 2.0,
                    -s_ptr->size.y / 2.0,
                    s_ptr->size.x / 2.0,
                    s_ptr->size.y / 2.0,
                    al_map_rgba(255, 255, 255, 96 * (s_ptr->alpha / 255.0))
                );
                
                al_use_transform(&current);
            }
            
            draw_bitmap(
                s_ptr->bitmap, s_ptr->center, s_ptr->size,
                s_ptr->angle, map_alpha(s_ptr->alpha)
            );
            
            if(state == EDITOR_STATE_DETAILS) {
                point min_coords, max_coords;
                get_transformed_rectangle_bounding_box(
                    s_ptr->center, s_ptr->size, s_ptr->angle,
                    &min_coords, &max_coords
                );
                
                if(selected_shadow != s_ptr) {
                    al_draw_rectangle(
                        min_coords.x, min_coords.y, max_coords.x, max_coords.y,
                        (
                            s_ptr == selected_shadow ?
                            al_map_rgb(224, 224, 64) :
                            al_map_rgb(128, 128, 64)
                        ),
                        2.0 / game.cam.zoom
                    );
                }
            }
        }
        if(selected_shadow) {
            cur_transformation_widget.draw(
                &selected_shadow->center,
                &selected_shadow->size,
                &selected_shadow->angle,
                1.0f / game.cam.zoom
            );
        }
    }
    
    //Cross-section points and line.
    if(state == EDITOR_STATE_REVIEW && show_cross_section) {
        for(unsigned char p = 0; p < 2; ++p) {
            string letter = (p == 0 ? "A" : "B");
            
            al_draw_filled_rectangle(
                cross_section_checkpoints[p].x -
                (CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                cross_section_checkpoints[p].y -
                (CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                cross_section_checkpoints[p].x +
                (CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                cross_section_checkpoints[p].y +
                (CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                al_map_rgb(255, 255, 32)
            );
            draw_scaled_text(
                game.fonts.builtin, al_map_rgb(0, 64, 64),
                cross_section_checkpoints[p],
                point(
                    POINT_LETTER_TEXT_SCALE / game.cam.zoom,
                    POINT_LETTER_TEXT_SCALE / game.cam.zoom
                ),
                ALLEGRO_ALIGN_CENTER, 1,
                letter
            );
        }
        al_draw_line(
            cross_section_checkpoints[0].x,
            cross_section_checkpoints[0].y,
            cross_section_checkpoints[1].x,
            cross_section_checkpoints[1].y,
            al_map_rgb(255, 0, 0), 3.0 / game.cam.zoom
        );
    }
    
    //Reference image.
    if(
        reference_bitmap &&
        (show_reference || state == EDITOR_STATE_TOOLS)
    ) {
        draw_bitmap(
            reference_bitmap,
            reference_center,
            reference_size,
            0,
            map_alpha(reference_alpha)
        );
        
        if(state == EDITOR_STATE_TOOLS) {
            cur_transformation_widget.draw(
                &reference_center,
                &reference_size,
                NULL,
                1.0f / game.cam.zoom
            );
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
                3.0 / game.cam.zoom
            );
        }
        if(!drawing_nodes.empty()) {
            ALLEGRO_COLOR new_line_color =
                interpolate_color(
                    new_sector_error_tint_timer.get_ratio_left(),
                    1, 0,
                    al_map_rgb(255, 0, 0),
                    al_map_rgb(64, 255, 64)
                );
            point hotspot = snap_point(game.mouse_cursor_w);
            
            al_draw_line(
                drawing_nodes.back().snapped_spot.x,
                drawing_nodes.back().snapped_spot.y,
                hotspot.x,
                hotspot.y,
                new_line_color,
                3.0 / game.cam.zoom
            );
            
            if(game.options.area_editor_show_edge_length) {
                draw_line_dist(hotspot, drawing_nodes.back().snapped_spot);
            }
        }
    }
    
    //New circular sector drawing.
    if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        switch(new_circle_sector_step) {
        case 1: {
            float circle_radius =
                dist(
                    new_circle_sector_center, new_circle_sector_anchor
                ).to_float();
            al_draw_circle(
                new_circle_sector_center.x,
                new_circle_sector_center.y,
                circle_radius,
                al_map_rgb(64, 255, 64),
                3.0 / game.cam.zoom
            );
            break;
            
        } case 2: {
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
                    color, 3.0 / game.cam.zoom
                );
            }
            
            for(size_t p = 0; p < new_circle_sector_points.size(); ++p) {
                al_draw_filled_circle(
                    new_circle_sector_points[p].x,
                    new_circle_sector_points[p].y,
                    3.0 / game.cam.zoom, al_map_rgb(192, 255, 192)
                );
            }
            break;
        }
        }
    }
    
    //Path drawing.
    if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
        if(path_drawing_stop_1) {
            point hotspot = snap_point(game.mouse_cursor_w);
            al_draw_line(
                path_drawing_stop_1->pos.x,
                path_drawing_stop_1->pos.y,
                hotspot.x,
                hotspot.y,
                al_map_rgb(64, 255, 64),
                3.0 / game.cam.zoom
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
            2.0 / game.cam.zoom
            
        );
    }
    
    //New thing marker.
    if(
        sub_state == EDITOR_SUB_STATE_DRAWING ||
        sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR ||
        sub_state == EDITOR_SUB_STATE_NEW_MOB ||
        sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        sub_state == EDITOR_SUB_STATE_PATH_DRAWING ||
        sub_state == EDITOR_SUB_STATE_NEW_SHADOW
    ) {
        point marker = game.mouse_cursor_w;
        
        if(sub_state != EDITOR_SUB_STATE_ADD_MOB_LINK) {
            marker = snap_point(marker);
        }
        
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y,
            marker.x + 10 / game.cam.zoom,
            marker.y,
            al_map_rgb(255, 255, 255), 2.0 / game.cam.zoom
        );
        al_draw_line(
            marker.x,
            marker.y - 10 / game.cam.zoom,
            marker.x,
            marker.y + 10 / game.cam.zoom,
            al_map_rgb(255, 255, 255), 2.0 / game.cam.zoom
        );
    }
    
    //Delete thing marker.
    if(
        sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        point marker = game.mouse_cursor_w;
        
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y - 10 / game.cam.zoom,
            marker.x + 10 / game.cam.zoom,
            marker.y + 10 / game.cam.zoom,
            al_map_rgb(255, 255, 255), 2.0 / game.cam.zoom
        );
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y + 10 / game.cam.zoom,
            marker.x + 10 / game.cam.zoom,
            marker.y - 10 / game.cam.zoom,
            al_map_rgb(255, 255, 255), 2.0 / game.cam.zoom
        );
    }
    
    al_use_transform(&game.identity_transform);
    
    //Cross-section graph.
    if(state == EDITOR_STATE_REVIEW && show_cross_section) {
    
        dist cross_section_world_length(
            cross_section_checkpoints[0], cross_section_checkpoints[1]
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
            get_sector(cross_section_checkpoints[0], NULL, false);
        sector* cs_right_sector =
            get_sector(cross_section_checkpoints[1], NULL, false);
        struct split_info {
            sector* sector_ptrs[2];
            float l1r;
            float l2r;
            split_info(
                sector* s1, sector* s2, const float l1r, const float l2r
            ) {
                sector_ptrs[0] = s1;
                sector_ptrs[1] = s2;
                this->l1r = l1r;
                this->l2r = l2r;
            }
        };
        vector<split_info> splits;
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            float l1r = 0;
            float l2r = 0;
            if(
                line_segments_intersect(
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    point(
                        cross_section_checkpoints[0].x,
                        cross_section_checkpoints[0].y
                    ),
                    point(
                        cross_section_checkpoints[1].x,
                        cross_section_checkpoints[1].y
                    ),
                    &l1r, &l2r
                )
            ) {
                splits.push_back(
                    split_info(e_ptr->sectors[0], e_ptr->sectors[1], l1r, l2r)
                );
            }
        }
        
        if(!splits.empty()) {
            sort(
                splits.begin(), splits.end(),
            [] (split_info i1, split_info i2) -> bool {
                return i1.l2r < i2.l2r;
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
                    std::swap(
                        splits[s].sector_ptrs[0], splits[s].sector_ptrs[1]
                    );
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
                    splits[s - 1].l2r, splits[s].l2r, proportion,
                    lowest_z, splits[s].sector_ptrs[0]
                );
            }
            
            sector* central_sector = NULL;
            for(size_t s = 1; s < splits.size(); ++s) {
                if(splits[s].l2r > 0.5) {
                    central_sector = splits[s].sector_ptrs[0];
                    break;
                }
            }
            
            if(central_sector) {
                float pikmin_silhouette_w =
                    game.config.standard_pikmin_radius * 2.0 * proportion;
                float pikmin_silhouette_h =
                    game.config.standard_pikmin_height * proportion;
                float pikmin_silhouette_pivot_x =
                    (
                        cross_section_window_start.x +
                        cross_section_window_end.x
                    ) / 2.0;
                float pikmin_silhouette_pivot_y =
                    cross_section_window_end.y - 8 -
                    ((central_sector->z - lowest_z) * proportion);
                al_draw_tinted_scaled_bitmap(
                    game.sys_assets.bmp_pikmin_silhouette,
                    al_map_rgba(255, 255, 255, 128),
                    0, 0,
                    al_get_bitmap_width(game.sys_assets.bmp_pikmin_silhouette),
                    al_get_bitmap_height(game.sys_assets.bmp_pikmin_silhouette),
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
                        game.fonts.builtin, al_map_rgb(255, 255, 255),
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
                game.fonts.builtin, al_map_rgb(255, 255, 255),
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
        get_closest_point_in_line(
            cross_section_checkpoints[0], cross_section_checkpoints[1],
            point(game.mouse_cursor_w.x, game.mouse_cursor_w.y),
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
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
}


/* ----------------------------------------------------------------------------
 * Draws a sector on the cross-section view.
 * start_ratio:
 *   Where the sector starts on the graph ([0, 1]).
 * end_ratio:
 *   Where the sector end on the graph ([0, 1]).
 * proportion:
 *   Ratio of how much to resize the heights.
 * lowest_z:
 *   What z coordinate represents the bottom of the graph.
 * sector_ptr:
 *   Pointer to the sector to draw.
 */
void area_editor::draw_cross_section_sector(
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
 * color:
 *   Text color.
 * where:
 *   Where to draw, in world coordinates.
 * text:
 *   Text to show.
 * dots:
 *   How many dots to draw above the text. 0, 1, or 2.
 */
void area_editor::draw_debug_text(
    const ALLEGRO_COLOR color, const point &where, const string &text,
    const unsigned char dots
) {
    int dw = 0;
    int dh = 0;
    al_get_text_dimensions(
        game.fonts.builtin, text.c_str(),
        NULL, NULL, &dw, &dh
    );
    
    float bbox_w = (dw * DEBUG_TEXT_SCALE) / game.cam.zoom;
    float bbox_h = (dh * DEBUG_TEXT_SCALE) / game.cam.zoom;
    
    al_draw_filled_rectangle(
        where.x - bbox_w * 0.5, where.y - bbox_h * 0.5,
        where.x + bbox_w * 0.5, where.y + bbox_h * 0.5,
        al_map_rgba(0, 0, 0, 128)
    );
    
    draw_scaled_text(
        game.fonts.builtin, color,
        where,
        point(
            DEBUG_TEXT_SCALE / game.cam.zoom,
            DEBUG_TEXT_SCALE / game.cam.zoom
        ),
        ALLEGRO_ALIGN_CENTER, 1,
        text
    );
    
    if(dots > 0) {
        al_draw_filled_rectangle(
            where.x - 3.0f / game.cam.zoom,
            where.y + bbox_h * 0.5f,
            where.x + 3.0f / game.cam.zoom,
            where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
            al_map_rgba(0, 0, 0, 128)
        );
        
        if(dots == 1) {
            al_draw_filled_rectangle(
                where.x - 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x + 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
        } else {
            al_draw_filled_rectangle(
                where.x - 3.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x - 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
            al_draw_filled_rectangle(
                where.x + 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x + 3.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws a number signifying the distance between two points.
 * The number is drawn next to the main point.
 * focus:
 *   The main point.
 * other:
 *   The point to measure against.
 */
void area_editor::draw_line_dist(const point &focus, const point &other) {
    float d = dist(other, focus).to_float();
    if(d < 64) return;
    
    float angle = get_angle(focus, other);
    point length_nr_pos;
    length_nr_pos.x = focus.x + cos(angle) * 64.0;
    length_nr_pos.y = focus.y + sin(angle) * 64.0;
    length_nr_pos.y -= 12;
    
    draw_debug_text(al_map_rgb(64, 255, 64), length_nr_pos, i2s(d));
}
