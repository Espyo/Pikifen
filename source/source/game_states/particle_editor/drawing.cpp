/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor drawing function.
 */

#include "editor.h"

#include <algorithm>

#include "../../game.h"
#include "../../utils/allegro_utils.h"
#include "../../mobs/mob_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the particle editor.
 */
void particle_editor::do_drawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    draw_op_error_cursor();
}


/**
 * @brief Draw the canvas. This is called as a callback inside the
 * Dear ImGui rendering process.
 */
void particle_editor::draw_canvas() {
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    //Background.
    if(use_bg && bg) {
        point texture_tl = canvas_tl;
        point texture_br = canvas_br;
        al_transform_coordinates(
            &game.screen_to_world_transform, &texture_tl.x, &texture_tl.y
        );
        al_transform_coordinates(
            &game.screen_to_world_transform, &texture_br.x, &texture_br.y
        );
        ALLEGRO_VERTEX bg_vertexes[4];
        for(size_t v = 0; v < 4; ++v) {
            bg_vertexes[v].z = 0;
            bg_vertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bg_vertexes[0].x = canvas_tl.x;
        bg_vertexes[0].y = canvas_tl.y;
        bg_vertexes[0].u = texture_tl.x;
        bg_vertexes[0].v = texture_tl.y;
        //Top-right vertex.
        bg_vertexes[1].x = canvas_br.x;
        bg_vertexes[1].y = canvas_tl.y;
        bg_vertexes[1].u = texture_br.x;
        bg_vertexes[1].v = texture_tl.y;
        //Bottom-right vertex.
        bg_vertexes[2].x = canvas_br.x;
        bg_vertexes[2].y = canvas_br.y;
        bg_vertexes[2].u = texture_br.x;
        bg_vertexes[2].v = texture_br.y;
        //Bottom-left vertex.
        bg_vertexes[3].x = canvas_tl.x;
        bg_vertexes[3].y = canvas_br.y;
        bg_vertexes[3].u = texture_tl.x;
        bg_vertexes[3].v = texture_br.y;
        
        al_draw_prim(
            bg_vertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.world_to_screen_transform);
    
    //Particles.
    vector<world_component> components;
    components.reserve(part_mgr.get_count());
    part_mgr.fill_component_list(components, game.cam.box[0], game.cam.box[1]);
    
    for(size_t c = 0; c < components.size(); ++c) {
        components[c].idx = c;
    }
    
    sort(
        components.begin(), components.end(),
    [](const world_component & c1, const world_component & c2) -> bool {
        if(c1.z == c2.z) {
            return c1.idx < c2.idx;
        }
        return c1.z < c2.z;
    }
    );
    for(size_t c = 0; c < components.size(); ++c) {
        world_component* c_ptr = &components[c];
        if(c_ptr->particle_ptr) {
            c_ptr->particle_ptr->draw();
        }
    }
    
    //Grid.
    if(grid_visible) {
        point cam_top_left_corner(0, 0);
        point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &cam_top_left_corner.x, &cam_top_left_corner.y
        );
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
        );
        
        al_draw_line(
            0, cam_top_left_corner.y, 0, cam_bottom_right_corner.y,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
        al_draw_line(
            cam_top_left_corner.x, 0, cam_bottom_right_corner.x, 0,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
    }
    
    //Emission shapes.
    if(emission_shape_visible) {
        switch (loaded_gen.emission.shape) {
        case PARTICLE_EMISSION_SHAPE_CIRCLE: {
    
            if(loaded_gen.emission.circle_arc == TAU) {
                al_draw_circle(
                    generator_pos_offset.x, generator_pos_offset.y,
                    loaded_gen.emission.circle_outer_dist,
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_circle(
                    generator_pos_offset.x, generator_pos_offset.y,
                    loaded_gen.emission.circle_inner_dist,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );
            } else {
                al_draw_arc(
                    generator_pos_offset.x, generator_pos_offset.y,
                    loaded_gen.emission.circle_outer_dist,
                    -loaded_gen.emission.circle_arc / 2.0f +
                    loaded_gen.emission.circle_arc_rot +
                    generator_angle_offset,
                    loaded_gen.emission.circle_arc,
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_arc(
                    generator_pos_offset.x, generator_pos_offset.y,
                    loaded_gen.emission.circle_inner_dist,
                    -loaded_gen.emission.circle_arc / 2.0f +
                    loaded_gen.emission.circle_arc_rot +
                    generator_angle_offset,
                    loaded_gen.emission.circle_arc,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );
            }
            break;
            
        } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
    
            draw_rotated_rectangle(
                generator_pos_offset,
                loaded_gen.emission.rect_outer_dist * 2.0f,
                generator_angle_offset,
                al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
            );
            draw_rotated_rectangle(
                generator_pos_offset,
                loaded_gen.emission.rect_inner_dist * 2.0f,
                generator_angle_offset,
                al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
            );
            break;
            
        }
        }
    }
    
    //Leader silhouette.
    if(leader_silhouette_visible) {
        float x_offset = 32.0f;
        
        draw_bitmap(
            game.sys_assets.bmp_leader_silhouette_top, point(x_offset, 0),
            point(-1, game.config.standard_leader_radius * 2.0f),
            0, al_map_rgba(240, 240, 240, 160)
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
}
