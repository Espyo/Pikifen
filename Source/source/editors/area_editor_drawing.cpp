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
    //TODO
    
    gui->draw();
    
    al_use_transform(&world_to_screen_transform);
    al_set_clipping_rectangle(0, 0, gui_x, status_bar_y);
    
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    if(sub_state != EDITOR_SUB_STATE_TEXTURE_VIEW) {
    
        float layout_opacity = 1.0f;
        float selection_opacity = 0.25 + (sin(selection_effect) + 1) * 0.25;
        
        //Sectors.
        size_t n_sectors = cur_area_data.sectors.size();
        for(size_t s = 0; s < n_sectors; ++s) {
            sector* s_ptr = cur_area_data.sectors[s];
            draw_sector_texture(
                s_ptr, point(),
                1.0, 0.5 * layout_opacity
            );
            
            if(selected_sectors.find(s_ptr) != selected_sectors.end()) {
                for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
                
                    ALLEGRO_VERTEX av[3];
                    for(size_t v = 0; v < 3; ++v) {
                        av[v].color =
                            al_map_rgba(
                                192, 192, 32,
                                selection_opacity * 0.5 * 255
                            );
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
        
        //Edges.
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
            
            //TODO
            /*if(error_sector_ptr) {
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
            }*/
            
            if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
            
            if(
                !one_sided &&
                e_ptr->sectors[0]->z == e_ptr->sectors[1]->z &&
                e_ptr->sectors[0]->type == e_ptr->sectors[1]->type
            ) {
                same_z = true;
            }
            
            //TODO
            /*
            if(on_sector && mode == EDITOR_MODE_SECTORS) {
                if(e_ptr->sectors[0] == on_sector) mouse_on = true;
                if(e_ptr->sectors[1] == on_sector) mouse_on = true;
            }*/
            
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
                    al_map_rgba(192, 192, 32,  selection_opacity * 255) :
                    error_highlight ?
                    al_map_rgba(192, 80,  0,   layout_opacity * 255) :
                    !valid ?
                    al_map_rgba(192, 32,  32,  layout_opacity * 255) :
                    one_sided ?
                    al_map_rgba(255, 255, 255, layout_opacity * 255) :
                    same_z ?
                    al_map_rgba(128, 128, 128, layout_opacity * 255) :
                    al_map_rgba(192, 192, 192, layout_opacity * 255)
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
        size_t n_vertexes = cur_area_data.vertexes.size();
        for(size_t v = 0; v < n_vertexes; ++v) {
            vertex* v_ptr = cur_area_data.vertexes[v];
            bool selected =
                (selected_vertexes.find(v_ptr) != selected_vertexes.end());
            al_draw_filled_circle(
                v_ptr->x,
                v_ptr->y,
                3.0 / cam_zoom,
                selected ?
                al_map_rgba(192, 192, 32, selection_opacity * 255) :
                al_map_rgba(80, 160, 255, layout_opacity * 255)
            );
            
            if(debug_vertex_nrs) {
                draw_debug_text(
                    al_map_rgb(192, 192, 255),
                    point(v_ptr->x, v_ptr->y), i2s(v)
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
                al_map_rgb(224, 224, 96),
                2.0 / cam_zoom
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
 */
void area_editor::draw_debug_text(
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
