/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor drawing function.
 */

#include "editor.h"

#include <algorithm>

#include "../../game.h"
#include "../../utils/allegro_utils.h"
#include "../../mobs/mob_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the GUI editor.
 */
void particle_editor::do_drawing() {
    //Render what is needed for the (Dear ImGui) GUI.
    //This will also render the canvas in due time.
    ImGui::Render();
    
    //Actually draw the GUI + canvas on-screen.
    al_clear_to_color(COLOR_BLACK);
    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    
    draw_op_error_cursor();
    
    //And the fade manager atop it all.
    game.fade_mgr.draw();
    
    //Finally, swap buffers.
    al_flip_display();
}


/**
 * @brief Draw the canvas. This is called as a callback inside the
 * Dear ImGui rendering process.
 */
void particle_editor::draw_canvas() {
    al_use_transform(&game.world_to_screen_transform);
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    //Background.
    al_clear_to_color(al_map_rgb(96, 128, 96));
    
    //Screen dimensions.
    al_draw_filled_rectangle(
        0.0f, 0.0f, 100.0f, 100.0f, al_map_rgb(96, 128, 96)
    );
    
    //Grid.
    draw_grid(
        game.options.particle_editor_grid_interval,
        al_map_rgba(64, 64, 64, 84),
        al_map_rgba(64, 64, 64, 40)
    );
    
    //Center grid line.
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

    if (emission_offset_visible) {
        switch (loaded_gen.emission.shape)
        {
        case(PARTICLE_EMISSION_SHAPE_CIRCLE):

            if (loaded_gen.emission.circular_arc == TAU) {
                al_draw_circle(
                    0, 0, loaded_gen.emission.max_circular_radius,
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_circle(
                    0, 0, loaded_gen.emission.min_circular_radius,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );
            } else {
                al_draw_arc(
                    0, 0, loaded_gen.emission.max_circular_radius, 
                    -loaded_gen.emission.circular_arc / 2 + loaded_gen.emission.circular_arc_rotation, loaded_gen.emission.circular_arc,
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_arc(
                    0, 0, loaded_gen.emission.min_circular_radius, 
                    -loaded_gen.emission.circular_arc / 2 + loaded_gen.emission.circular_arc_rotation, loaded_gen.emission.circular_arc,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );

            }
            break;
        case(PARTICLE_EMISSION_SHAPE_RECTANGLE):
            al_draw_rectangle(
                -loaded_gen.emission.max_rectangular_offset.x, -loaded_gen.emission.max_rectangular_offset.y,
                loaded_gen.emission.max_rectangular_offset.x, loaded_gen.emission.max_rectangular_offset.y,
                al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
            );
            al_draw_rectangle(
                -loaded_gen.emission.min_rectangular_offset.x, -loaded_gen.emission.min_rectangular_offset.y,
                loaded_gen.emission.min_rectangular_offset.x, loaded_gen.emission.min_rectangular_offset.y,
                al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
            );
            break;
        }
    }

    if(leader_silhouette_visible) {
        float x_offset = 32;

        draw_bitmap(
            game.sys_assets.bmp_leader_silhouette_top, point(x_offset, 0),
            point(-1, game.config.standard_leader_radius * 2.0f),
            0, al_map_rgba(240, 240, 240, 160)
        );
    }

    vector<world_component> components;
    components.reserve(part_manager.get_count());
    //Particles.
    part_manager.fill_component_list(components, game.cam.box[0], game.cam.box[1]);

    //Time to draw!
    for (size_t c = 0; c < components.size(); ++c) {
        components[c].nr = c;
    }

    sort(
        components.begin(), components.end(),
        [](const world_component& c1, const world_component& c2) -> bool {
            if (c1.z == c2.z) {
                return c1.nr < c2.nr;
            }
            return c1.z < c2.z;
        }
    );
    for (size_t c = 0; c < components.size(); ++c) {
        world_component* c_ptr = &components[c];
        if (c_ptr->particle_ptr) {
            c_ptr->particle_ptr->draw();
        }
    }

    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
}
