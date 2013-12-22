#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "vars.h"

void do_area_editor_logic() {
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + scr_w / 2, cam_y + scr_h / 2);
    al_use_transform(&transform);
    
    al_clear_to_color(al_map_rgb(0, 0, 64));
    
    //Grid.
    int x = -(cam_x + scr_h / 2);
    while(x < -(cam_x - scr_w / 2)) {
        ALLEGRO_COLOR c;
        if(x % 20) c = al_map_rgb(0, 96, 160); else c = al_map_rgb(0, 64, 128);
        al_draw_line(x, 0, x, scr_h, al_map_rgb(0, 64, 128), 1);
        x += 20;
    }
    
    int y = 0;
    while(y < scr_h) {
        ALLEGRO_COLOR c;
        if(y % 20) c = al_map_rgb(0, 96, 160); else c = al_map_rgb(0, 64, 128);
        al_draw_line(0, y, scr_w, y, c, 1);
        y += 20;
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
                al_map_rgb(192, 192, 192), 2);
                
            al_draw_filled_circle(
                sector_ptr->linedefs[l]->x1,
                sector_ptr->linedefs[l]->y1,
                3, al_map_rgb(224, 224, 224));
                
            al_draw_filled_circle(
                sector_ptr->linedefs[l]->x2,
                sector_ptr->linedefs[l]->y2,
                3, al_map_rgb(224, 224, 224));
        }
    }
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0,
                 (to_string((long long) (mouse_cursor_x - cam_x)) + "," + to_string((long long) (mouse_cursor_y - cam_y))).c_str());;
                 
    al_flip_display();
}

void handle_editor_controls(ALLEGRO_EVENT ev) {
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(editor_holding_m2) {
            cam_x += ev.mouse.dx;
            cam_y += ev.mouse.dy;
        }
        
        mouse_cursor_x = ev.mouse.x;
        mouse_cursor_y = ev.mouse.y;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) editor_holding_m2 = true;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) editor_holding_m2 = false;
    }
}

#endif //ifndef AREA_EDITOR_INCLUDED