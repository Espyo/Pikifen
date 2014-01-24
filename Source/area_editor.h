#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "functions.h"
#include "vars.h"

void do_area_editor_logic() {

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
    
    size_t n_sectors = sectors.size();
    for(size_t s = 0; s < n_sectors; s++) {
        sector* sector_ptr = &sectors[s];
        
        size_t n_linedefs = sector_ptr->linedefs.size();
        
        for(size_t l = 0; l < n_linedefs; l++) {
            al_draw_line(
                sector_ptr->linedefs[l]->x1,
                sector_ptr->linedefs[l]->y1,
                sector_ptr->linedefs[l]->x2,
                sector_ptr->linedefs[l]->y2,
                al_map_rgb(192, 192, 192), 2.0 / cam_zoom);
                
            al_draw_filled_circle(
                sector_ptr->linedefs[l]->x1,
                sector_ptr->linedefs[l]->y1,
                3.0 / cam_zoom, al_map_rgb(224, 224, 224));
                
            al_draw_filled_circle(
                sector_ptr->linedefs[l]->x2,
                sector_ptr->linedefs[l]->y2,
                3.0 / cam_zoom, al_map_rgb(224, 224, 224));
        }
    }
    
    al_draw_filled_circle(0, 0, 5, al_map_rgb(0, 128, 0));
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0,
                 (to_string((long long) (mouse_cursor_x - cam_x / cam_zoom)) + "," +
                  to_string((long long) (mouse_cursor_y - cam_y / cam_zoom))).c_str());;
                  
    al_flip_display();
}

void handle_area_editor_controls(ALLEGRO_EVENT ev) {
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(editor_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        mouse_cursor_x = ev.mouse.x;
        mouse_cursor_y = ev.mouse.y;
        
        cam_zoom += cam_zoom * ev.mouse.dz * 0.1;
        if(cam_zoom <= ZOOM_MIN_LEVEL_EDITOR) cam_zoom = ZOOM_MIN_LEVEL_EDITOR;
        if(cam_zoom >= ZOOM_MAX_LEVEL_EDITOR) cam_zoom = ZOOM_MAX_LEVEL_EDITOR;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) editor_holding_m2 = true;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) editor_holding_m2 = false;
    }
}

#endif //ifndef AREA_EDITOR_INCLUDED