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

#include "../../game.h"
#include "../../utils/allegro_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the GUI editor.
 */
void gui_editor::do_drawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    draw_op_error_cursor();
}


/**
 * @brief Draw the canvas. This is called as a callback inside the
 * Dear ImGui rendering process.
 */
void gui_editor::draw_canvas() {
    al_use_transform(&game.world_to_screen_transform);
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    //Background.
    al_clear_to_color(COLOR_BLACK);
    
    //Screen dimensions.
    al_draw_filled_rectangle(
        0.0f, 0.0f, 100.0f, 100.0f, al_map_rgb(96, 128, 96)
    );
    
    //Grid.
    draw_grid(
        game.options.gui_editor_grid_interval,
        al_map_rgba(64, 64, 64, 84),
        al_map_rgba(64, 64, 64, 40)
    );
    
    //50%,50% marker.
    al_draw_line(
        0.0f, 50.0f, 100.0f, 50.0f,
        al_map_rgba(208, 208, 224, 84),
        1.0f / game.cam.zoom
    );
    al_draw_line(
        50.0f, 0.0f, 50.0f, 100.0f,
        al_map_rgba(208, 208, 224, 84),
        1.0f / game.cam.zoom
    );
    
    //Items.
    int orig_clip_x = 0;
    int orig_clip_y = 0;
    int orig_clip_w = 0;
    int orig_clip_h = 0;
    al_get_clipping_rectangle(
        &orig_clip_x, &orig_clip_y, &orig_clip_w, &orig_clip_h
    );
    for(size_t i = 0; i < items.size(); ++i) {
        if(items[i].size.x == 0.0f) continue;
        
        draw_filled_rounded_rectangle(
            items[i].center,
            items[i].size,
            8.0f / game.cam.zoom,
            al_map_rgba(224, 224, 224, 64)
        );
        
        float clip_x = items[i].center.x - items[i].size.x / 2.0f;
        float clip_y = items[i].center.y - items[i].size.y / 2.0f;
        al_transform_coordinates(
            &game.world_to_screen_transform, &clip_x, &clip_y
        );
        float clip_w = items[i].size.x * game.cam.zoom;
        float clip_h = items[i].size.y * game.cam.zoom;
        set_combined_clipping_rectangles(
            orig_clip_x, orig_clip_y, orig_clip_w, orig_clip_h,
            clip_x, clip_y, clip_w, clip_h
        );
        draw_text(
            items[i].name, game.sys_assets.fnt_builtin,
            point(
                (items[i].center.x - items[i].size.x / 2.0f) +
                (4.0f / game.cam.zoom),
                (items[i].center.y - items[i].size.y / 2.0f) +
                (4.0f / game.cam.zoom)
            ),
            point(LARGE_FLOAT, 8.0 / game.cam.zoom),
            al_map_rgb(40, 40, 96), ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
        al_set_clipping_rectangle(
            orig_clip_x, orig_clip_y, orig_clip_w, orig_clip_h
        );
        
        if(cur_item != i) {
            draw_rounded_rectangle(
                items[i].center,
                items[i].size,
                8.0f / game.cam.zoom,
                al_map_rgb(224, 224, 224),
                2.0f / game.cam.zoom
            );
        }
    }
    
    if(cur_item != INVALID && items[cur_item].size.x != 0.0f) {
        cur_transformation_widget.draw(
            &items[cur_item].center,
            &items[cur_item].size,
            nullptr,
            1.0f / game.cam.zoom
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
}
