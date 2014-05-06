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

#include <allegro5/allegro_primitives.h>

#include "area_editor.h"
#include "functions.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Handles the main loop, both logic and drawing.
 */
void area_editor::do_area_editor_logic() {

    //---Drawing.---
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + (scr_w / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_clear_to_color(al_map_rgb(0, 0, 64));
    
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
            
            if(mod(x, grid_interval_2) == 0) {
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
            
            if(mod(y, grid_interval_2) == 0) {
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
    
    size_t n_sectors = cur_area_map.sectors.size();
    for(size_t s = 0; s < n_sectors; s++) {
        sector* sector_ptr = &cur_area_map.sectors[s];
        
        size_t n_linedefs = sector_ptr->linedefs.size();
        
        for(size_t l = 0; l < n_linedefs; l++) {
            linedef* l_ptr = &cur_area_map.linedefs[sector_ptr->linedefs[l]];
            al_draw_line(
                l_ptr->vertex1->x,
                l_ptr->vertex1->y,
                l_ptr->vertex2->x,
                l_ptr->vertex2->y,
                al_map_rgb(192, 192, 192), 2.0 / cam_zoom);
                
            al_draw_filled_circle(
                l_ptr->vertex1->x,
                l_ptr->vertex1->y,
                3.0 / cam_zoom, al_map_rgb(224, 224, 224));
                
            al_draw_filled_circle(
                l_ptr->vertex2->x,
                l_ptr->vertex2->y,
                3.0 / cam_zoom, al_map_rgb(224, 224, 224));
        }
    }
    
    al_draw_filled_circle(0, 0, 5, al_map_rgb(0, 128, 0));
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0,
                 (itos((mouse_cursor_x - cam_x / cam_zoom)) + "," +
                  itos((mouse_cursor_y - cam_y / cam_zoom))).c_str());;
                  
    al_flip_display();
}

/* ----------------------------------------------------------------------------
 * Handles the events for the area editor.
 */
void area_editor::handle_area_editor_controls(ALLEGRO_EVENT ev) {
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(ed_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        mouse_cursor_x = ev.mouse.x;
        mouse_cursor_y = ev.mouse.y;
        
        cam_zoom += cam_zoom * ev.mouse.dz * 0.1;
        if(cam_zoom <= ZOOM_MIN_LEVEL_EDITOR) cam_zoom = ZOOM_MIN_LEVEL_EDITOR;
        if(cam_zoom >= ZOOM_MAX_LEVEL_EDITOR) cam_zoom = ZOOM_MAX_LEVEL_EDITOR;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) ed_holding_m2 = true;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) ed_holding_m2 = false;
    }
}