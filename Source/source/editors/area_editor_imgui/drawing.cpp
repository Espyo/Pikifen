/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include "editor.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../drawing.h"
#include "../../vars.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor_imgui::do_drawing() {
    //Draw the GUI first.
    ImGui::Render();
    al_clear_to_color(al_map_rgb(0, 0, 0));
    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

    //And now, draw the canvas.
    al_use_transform(&world_to_screen_transform);
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );



    
    float grid_opacity = 1.0f;

    //Grid.
    point cam_top_left_corner(0, 0);
    point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
    al_transform_coordinates(
        &screen_to_world_transform,
        &cam_top_left_corner.x, &cam_top_left_corner.y
    );
    al_transform_coordinates(
        &screen_to_world_transform,
        &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
    );
    
    float x =
        floor(cam_top_left_corner.x / area_editor_grid_interval) *
        area_editor_grid_interval;
    while(x < cam_bottom_right_corner.x + area_editor_grid_interval) {
        ALLEGRO_COLOR c = al_map_rgba(48, 48, 48, grid_opacity * 255);
        bool draw_line = true;
        
        if(fmod(x, area_editor_grid_interval * 2) == 0) {
            c = al_map_rgba(64, 64, 64, grid_opacity * 255);
            if((area_editor_grid_interval * 2) * cam_zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(area_editor_grid_interval * cam_zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                x, cam_top_left_corner.y,
                x, cam_bottom_right_corner.y + area_editor_grid_interval,
                c, 1.0 / cam_zoom
            );
        }
        x += area_editor_grid_interval;
    }
    
    float y =
        floor(cam_top_left_corner.y / area_editor_grid_interval) *
        area_editor_grid_interval;
    while(y < cam_bottom_right_corner.y + area_editor_grid_interval) {
        ALLEGRO_COLOR c = al_map_rgba(48, 48, 48, grid_opacity * 255);
        bool draw_line = true;
        
        if(fmod(y, area_editor_grid_interval * 2) == 0) {
            c = al_map_rgba(64, 64, 64, grid_opacity * 255);
            if((area_editor_grid_interval * 2) * cam_zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(area_editor_grid_interval * cam_zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                cam_top_left_corner.x, y,
                cam_bottom_right_corner.x + area_editor_grid_interval, y,
                c, 1.0 / cam_zoom
            );
        }
        y += area_editor_grid_interval;
    }
    
    
    
    al_reset_clipping_rectangle();
    al_use_transform(&identity_transform);
    
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
void area_editor_imgui::draw_cross_section_sector(
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
 * dots:  How many dots to draw above the text. 0, 1, or 2.
 */
void area_editor_imgui::draw_debug_text(
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


/* ----------------------------------------------------------------------------
 * Draws a number signifying the distance between two points next to the
 * main one.
 */
void area_editor_imgui::draw_line_dist(const point &focus, const point &other) {
    float d = dist(other, focus).to_float();
    if(d < 64) return;
    
    float angle = get_angle(focus, other);
    point length_nr_pos;
    length_nr_pos.x = focus.x + cos(angle) * 64.0;
    length_nr_pos.y = focus.y + sin(angle) * 64.0;
    length_nr_pos.y -= 12;
    
    draw_debug_text(al_map_rgb(64, 255, 64), length_nr_pos, i2s(d));
}
