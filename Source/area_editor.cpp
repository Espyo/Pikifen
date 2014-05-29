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

#include <iomanip>
#include <unordered_set>

#include <allegro5/allegro_primitives.h>

#include "area_editor.h"
#include "functions.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/gui.h"
#include "LAFI/minor.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Stores the data from the advanced texture settings onto the gui.
 */
void area_editor::adv_textures_to_gui() {
    if(!ed_cur_sector) {
        ed_mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
        return;
    }
    
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_adv_textures"];
    
    ((lafi_textbox*) f->widgets["txt_x"])->text =  ftos(ed_cur_sector->textures[0].trans_x);
    ((lafi_textbox*) f->widgets["txt_y"])->text =  ftos(ed_cur_sector->textures[0].trans_y);
    ((lafi_textbox*) f->widgets["txt_sx"])->text = ftos(ed_cur_sector->textures[0].scale_x);
    ((lafi_textbox*) f->widgets["txt_sy"])->text = ftos(ed_cur_sector->textures[0].scale_y);
    ((lafi_textbox*) f->widgets["txt_a"])->text =  ftos(ed_cur_sector->textures[0].rot);
    ((lafi_checkbox*) f->widgets["chk_fade"])->set(ed_cur_sector->fade);
    
    f = (lafi_frame*) f->widgets["frm_texture_2"];
    if(ed_cur_sector->fade) {
        show_widget(f);
    } else {
        hide_widget(f);
    }
    
    ((lafi_textbox*) f->widgets["txt_fade_a"])->text = ftos(ed_cur_sector->fade_angle);
    ((lafi_textbox*) f->widgets["txt_2texture"])->text = ed_cur_sector->textures[1].filename;
    ((lafi_textbox*) f->widgets["txt_2x"])->text =  ftos(ed_cur_sector->textures[1].trans_x);
    ((lafi_textbox*) f->widgets["txt_2y"])->text =  ftos(ed_cur_sector->textures[1].trans_y);
    ((lafi_textbox*) f->widgets["txt_2sx"])->text = ftos(ed_cur_sector->textures[1].scale_x);
    ((lafi_textbox*) f->widgets["txt_2sy"])->text = ftos(ed_cur_sector->textures[1].scale_y);
    ((lafi_textbox*) f->widgets["txt_2a"])->text =  ftos(ed_cur_sector->textures[1].rot);
}

/* ----------------------------------------------------------------------------
 * Loads the background's data from the memory to the gui.
 */
void area_editor::bg_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_bg"];
    ((lafi_textbox*) f->widgets["txt_file"])->text = ed_bg_file_name;
    ((lafi_textbox*) f->widgets["txt_x"])->text = ftos(ed_bg_x);
    ((lafi_textbox*) f->widgets["txt_y"])->text = ftos(ed_bg_y);
    ((lafi_textbox*) f->widgets["txt_w"])->text = ftos(ed_bg_w);
    ((lafi_textbox*) f->widgets["txt_h"])->text = ftos(ed_bg_h);
    ((lafi_checkbox*) f->widgets["chk_ratio"])->set(ed_bg_aspect_ratio);
    ((lafi_checkbox*) f->widgets["chk_mouse"])->set(ed_sec_mode == ESM_BG_MOUSE);
    ((lafi_scrollbar*) f->widgets["bar_alpha"])->set_value(ed_bg_a);
}

/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void area_editor::center_camera(float min_x, float max_x, float min_y, float max_y) {
    float width = max_x - min_x;
    float height = max_y - min_y;
    
    cam_x = -floor(min_x + width  / 2);
    cam_y = -floor(min_y + height / 2);
    
    if(width > height) cam_zoom = (scr_w - 208) / width;
    else cam_zoom = (scr_h - 16) / height;
    
    cam_zoom -= cam_zoom * 0.05;
    
    cam_zoom = max(cam_zoom, ZOOM_MIN_LEVEL_EDITOR);
    cam_zoom = min(cam_zoom, ZOOM_MAX_LEVEL_EDITOR);
    
}

/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame(bool hide_all) {
    ed_sec_mode = ESM_NONE;
    
    hide_widget(ed_gui->widgets["frm_main"]);
    hide_widget(ed_gui->widgets["frm_picker"]);
    hide_widget(ed_gui->widgets["frm_sectors"]);
    hide_widget(ed_gui->widgets["frm_adv_textures"]);
    hide_widget(ed_gui->widgets["frm_bg"]);
    hide_widget(ed_gui->widgets["frm_review"]);
    
    if(!hide_all) {
        if(ed_mode == EDITOR_MODE_MAIN) {
            show_widget(ed_gui->widgets["frm_main"]);
        } else if(ed_mode == EDITOR_MODE_SECTORS) {
            show_widget(ed_gui->widgets["frm_sectors"]);
        } else if(ed_mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS) {
            show_widget(ed_gui->widgets["frm_adv_textures"]);
        } else if(ed_mode == EDITOR_MODE_BG) {
            show_widget(ed_gui->widgets["frm_bg"]);
        } else if(ed_mode == EDITOR_MODE_REVIEW) {
            show_widget(ed_gui->widgets["frm_review"]);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Handles the main loop, both logic and drawing.
 */
void area_editor::do_logic() {

    //---Logic---
    
    if(ed_double_click_time > 0) {
        ed_double_click_time -= delta_t;
        if(ed_double_click_time < 0) ed_double_click_time = 0;
    }
    
    //---Drawing---
    
    ed_gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
        al_clear_to_color(al_map_rgb(0, 0, 16));
        
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
                
                if(fmod(x, grid_interval_2) == 0) {
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
                
                if(fmod(y, grid_interval_2) == 0) {
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
        
        //0,0 marker.
        al_draw_line(-(grid_interval * 2), 0, grid_interval * 2, 0, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        al_draw_line(0, -(grid_interval * 2), 0, grid_interval * 2, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        
        //Linedefs.
        if(ed_sec_mode != ESM_TEXTURE_VIEW) {
            size_t n_linedefs = cur_area_map.linedefs.size();
            for(size_t l = 0; l < n_linedefs; l++) {
                linedef* l_ptr = cur_area_map.linedefs[l];
                
                if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
                
                bool one_sided = true;
                if(l_ptr->sectors[0] && l_ptr->sectors[1]) one_sided = false;
                
                bool valid = true;
                for(size_t il = 0; il < ed_intersecting_lines.size(); il++) {
                    if(ed_intersecting_lines[il].contains(l_ptr)) {
                        valid = false;
                        break;
                    }
                }
                
                bool sector_line = false;
                if(ed_on_sector) {
                    for(size_t sl = 0; sl < ed_on_sector->linedefs.size(); sl++) {
                        if(l_ptr == ed_on_sector->linedefs[sl]) {
                            sector_line = true;
                            break;
                        }
                    }
                }
                
                al_draw_line(
                    l_ptr->vertices[0]->x,
                    l_ptr->vertices[0]->y,
                    l_ptr->vertices[1]->x,
                    l_ptr->vertices[1]->y,
                    (valid       ? al_map_rgb(160, 160, 160) :
                     one_sided   ? al_map_rgb(240, 240, 240) :
                     al_map_rgb(192, 64, 64)
                    ),
                    (sector_line ? 3.0 : 2.0) / cam_zoom
                );
                
                //Debug: uncomment this to show the sector numbers on each side.
                //Orientantion could be wrong, as there is no concept of front/back sector.
                float mid_x = (l_ptr->vertices[0]->x + l_ptr->vertices[1]->x) / 2;
                float mid_y = (l_ptr->vertices[0]->y + l_ptr->vertices[1]->y) / 2;
                float angle = atan2(l_ptr->vertices[0]->y - l_ptr->vertices[1]->y, l_ptr->vertices[0]->x - l_ptr->vertices[1]->x);
                al_draw_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle - M_PI_2) * 15,
                    mid_y + sin(angle - M_PI_2) * 15 - font_h / 2,
                    ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[0] == string::npos ? "--" : itos(l_ptr->sector_nrs[0]).c_str());
                al_draw_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle + M_PI_2) * 15,
                    mid_y + sin(angle + M_PI_2) * 15 - font_h / 2,
                    ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[1] == string::npos ? "--" : itos(l_ptr->sector_nrs[1]).c_str());
            }
            
            //Vertices.
            size_t n_vertices = cur_area_map.vertices.size();
            for(size_t v = 0; v < n_vertices; v++) {
                vertex* v_ptr = cur_area_map.vertices[v];
                al_draw_filled_circle(
                    v_ptr->x,
                    v_ptr->y,
                    3.0 / cam_zoom,
                    al_map_rgb(224, 224, 224)
                );
            }
            
        }
        
        //New sector marker.
        if(ed_sec_mode == ESM_NEW_SECTOR) {
            float x = snap_to_grid(mouse_cursor_x);
            float y = snap_to_grid(mouse_cursor_y);
            al_draw_line(x - 16, y,      x + 16, y,      al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(x,      y - 16, x,      y + 16, al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
        }
        
        //Select sector marker.
        if(ed_sec_mode == ESM_SEL_SECTOR) {
            al_draw_circle(mouse_cursor_x, mouse_cursor_y, 12, al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(mouse_cursor_x - 16, mouse_cursor_y, mouse_cursor_x + 16, mouse_cursor_y,
                         al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(mouse_cursor_x, mouse_cursor_y - 16, mouse_cursor_x, mouse_cursor_y + 16,
                         al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
        }
        
        //Lightly glow the sector under the mouse.
        if(ed_sec_mode != ESM_TEXTURE_VIEW) {
            if(ed_on_sector && ed_moving_vertex == string::npos) {
                for(size_t t = 0; t < ed_on_sector->triangles.size(); t++) {
                    triangle* t_ptr = &ed_on_sector->triangles[t];
                    al_draw_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        al_map_rgb(192, 0, 0),
                        1.0 / cam_zoom
                    );
                    al_draw_filled_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        al_map_rgba(255, 255, 255, 12)
                    );
                }
            }
        }
        
        //Draw textures.
        if(ed_sec_mode == ESM_TEXTURE_VIEW) {
            for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
                sector* s_ptr = cur_area_map.sectors[s];
                
                for(size_t t = 0; t < s_ptr->triangles.size(); t++) {
                    ALLEGRO_VERTEX av[200]; //ToDo 200?
                    size_t n_vertices = s_ptr->triangles.size() * 3;
                    
                    //ToDo floors don't work like this.
                    
                    for(unsigned char t = 0; t < 1; t++) {
                    
                        for(size_t v = 0; v < n_vertices; v++) {
                            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
                            av[v].x = t_ptr->points[v % 3]->x;
                            av[v].y = t_ptr->points[v % 3]->y;
                            av[v].u = t_ptr->points[v % 3]->x;
                            av[v].v = t_ptr->points[v % 3]->y;
                            av[v].z = 0;
                            av[v].color = al_map_rgba_f(s_ptr->brightness, s_ptr->brightness, s_ptr->brightness, 1);
                        }
                        
                        al_draw_prim(av, NULL, s_ptr->textures[t].bitmap, 0, n_vertices, ALLEGRO_PRIM_TRIANGLE_LIST);
                        
                    }
                }
            }
        }
        
        //ToDo temp stuff.
        /*for(size_t v = 0; v < ed_temp_o.size(); v++) {
            al_draw_text(font, al_map_rgb(255, 255, 255), ed_temp_o[v]->x, ed_temp_o[v]->y - font_h, ALLEGRO_ALIGN_CENTER, ("O" + to_string((long long) v)).c_str());
        }
        for(size_t i = 0; i < ed_temp_i.size(); i++) {
            for(size_t v = 0; v < ed_temp_i[i].size(); v++) {
                al_draw_text(font, al_map_rgb(255, 255, 255), ed_temp_i[i][v]->x, ed_temp_i[i][v]->y - font_h * 2, ALLEGRO_ALIGN_CENTER, ("I" + to_string((long long) v)).c_str());
            }
        }*/
        
        if(ed_bg_bitmap) {
            al_draw_tinted_scaled_bitmap(
                ed_bg_bitmap,
                al_map_rgba(255, 255, 255, ed_bg_a),
                0, 0,
                al_get_bitmap_width(ed_bg_bitmap), al_get_bitmap_height(ed_bg_bitmap),
                ed_bg_x, ed_bg_y,
                ed_bg_w, ed_bg_h,
                0
            );
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_flip_display();
}

/* ----------------------------------------------------------------------------
 * Finds errors with the map.
 * On the first error, it adds it to ed_error_type and stops.
 */
void area_editor::find_errors() {
    ed_error_type = EET_NONE;
    
    //Check intersecting lines.
    if(ed_intersecting_lines.size() > 0) {
        ed_error_type = EET_INTERSECTING_LINEDEFS;
        ed_error_size_t_1 = 0;
    }
    //ToDo bad textures and sectors.
    
    update_review_frame();
}

/* ----------------------------------------------------------------------------
 * Focuses the camera on the error found, if any.
 */
void area_editor::goto_error() {
    if(ed_error_type == EET_NONE) return;
    
    if(ed_error_type == EET_INTERSECTING_LINEDEFS) {
    
        if(ed_error_size_t_1 >= ed_intersecting_lines.size()) {
            area_editor::find_errors();
            return;
        }
        
        linedef_intersection* li_ptr = &ed_intersecting_lines[ed_error_size_t_1];
        float min_x, max_x, min_y, max_y;
        min_x = max_x = li_ptr->l1->vertices[0]->x;
        min_y = max_y = li_ptr->l1->vertices[0]->y;
        
        min_x = min(min_x, li_ptr->l1->vertices[0]->x);
        min_x = min(min_x, li_ptr->l1->vertices[1]->x);
        min_x = min(min_x, li_ptr->l2->vertices[0]->x);
        min_x = min(min_x, li_ptr->l2->vertices[1]->x);
        max_x = max(max_x, li_ptr->l1->vertices[0]->x);
        max_x = max(max_x, li_ptr->l1->vertices[1]->x);
        max_x = max(max_x, li_ptr->l2->vertices[0]->x);
        max_x = max(max_x, li_ptr->l2->vertices[1]->x);
        min_y = min(min_y, li_ptr->l1->vertices[0]->y);
        min_y = min(min_y, li_ptr->l1->vertices[1]->y);
        min_y = min(min_y, li_ptr->l2->vertices[0]->y);
        min_y = min(min_y, li_ptr->l2->vertices[1]->y);
        max_y = max(max_y, li_ptr->l1->vertices[0]->y);
        max_y = max(max_y, li_ptr->l1->vertices[1]->y);
        max_y = max(max_y, li_ptr->l2->vertices[0]->y);
        max_y = max(max_y, li_ptr->l2->vertices[1]->y);
        
        center_camera(min_x, max_x, min_y, max_y);
    }
}

/* ----------------------------------------------------------------------------
 * Saves the advanced texture settings from the gui.
 */
void area_editor::gui_to_adv_textures() {
    if(!ed_cur_sector) return;
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_adv_textures"];
    
    ed_cur_sector->textures[0].trans_x = tof(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_cur_sector->textures[0].trans_y = tof(((lafi_textbox*) f->widgets["txt_y"])->text);
    ed_cur_sector->textures[0].scale_x = tof(((lafi_textbox*) f->widgets["txt_sx"])->text);
    ed_cur_sector->textures[0].scale_y = tof(((lafi_textbox*) f->widgets["txt_sy"])->text);
    ed_cur_sector->textures[0].rot =     tof(((lafi_textbox*) f->widgets["txt_a"])->text);
    ed_cur_sector->fade = ((lafi_checkbox*) f->widgets["chk_fade"])->checked;
    
    if(ed_cur_sector->fade) {
        f = (lafi_frame*) f->widgets["frm_texture_2"];
        ed_cur_sector->fade_angle = tof(((lafi_textbox*) f->widgets["txt_fade_a"])->text);
        ed_cur_sector->textures[1].filename = ((lafi_textbox*) f->widgets["txt_2texture"])->text;
        ed_cur_sector->textures[1].trans_x = tof(((lafi_textbox*) f->widgets["txt_2x"])->text);
        ed_cur_sector->textures[1].trans_y = tof(((lafi_textbox*) f->widgets["txt_2y"])->text);
        ed_cur_sector->textures[1].scale_x = tof(((lafi_textbox*) f->widgets["txt_2sx"])->text);
        ed_cur_sector->textures[1].scale_y = tof(((lafi_textbox*) f->widgets["txt_2sy"])->text);
        ed_cur_sector->textures[1].rot =     tof(((lafi_textbox*) f->widgets["txt_2a"])->text);
    }
    
    adv_textures_to_gui();
}

/* ----------------------------------------------------------------------------
 * Saves the background's data from the fields in the gui.
 */
void area_editor::gui_to_bg() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_bg"];
    
    string new_file_name = ((lafi_textbox*) f->widgets["txt_file"])->text;
    bool is_file_new = false;
    
    if(new_file_name != ed_bg_file_name) {
        //New background image, delete the old one.
        is_file_new = true;
        if(ed_bg_bitmap && ed_bg_bitmap != bmp_error) al_destroy_bitmap(ed_bg_bitmap);
        ed_bg_bitmap = load_bmp(new_file_name);
        ed_bg_file_name = new_file_name;
        if(ed_bg_bitmap) {
            ed_bg_w = al_get_bitmap_width(ed_bg_bitmap);
            ed_bg_h = al_get_bitmap_height(ed_bg_bitmap);
        } else {
            ed_bg_w = 0;
            ed_bg_h = 0;
        }
    }
    
    ed_bg_x = tof(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_bg_y = tof(((lafi_textbox*) f->widgets["txt_y"])->text);
    
    ed_bg_aspect_ratio = ((lafi_checkbox*) f->widgets["chk_ratio"])->checked;
    float new_w = tof(((lafi_textbox*) f->widgets["txt_w"])->text);
    float new_h = tof(((lafi_textbox*) f->widgets["txt_h"])->text);
    
    if(new_w != 0 && new_h != 0 && !is_file_new) {
        if(ed_bg_aspect_ratio) {
            if(new_w == ed_bg_w && new_h != ed_bg_h) {
                float ratio = ed_bg_w / ed_bg_h;
                ed_bg_h = new_h;
                ed_bg_w = new_h * ratio;
            } else if(new_w != ed_bg_w && new_h == ed_bg_h) {
                float ratio = ed_bg_h / ed_bg_w;
                ed_bg_w = new_w;
                ed_bg_h = new_w * ratio;
            } else {
                ed_bg_w = new_w;
                ed_bg_h = new_h;
            }
        } else {
            ed_bg_w = new_w;
            ed_bg_h = new_h;
        }
    }
    
    ed_sec_mode = ((lafi_checkbox*) f->widgets["chk_mouse"])->checked ? ESM_BG_MOUSE : ESM_NONE;
    ed_bg_a = ((lafi_scrollbar*) f->widgets["bar_alpha"])->low_value;
    
    bg_to_gui();
}

/* ----------------------------------------------------------------------------
 * Saves the sector using the info on the gui.
 */
void area_editor::gui_to_sector() {
    if(!ed_cur_sector) return;
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_sectors"]->widgets["frm_sector"];
    
    ed_cur_sector->z = tof(((lafi_textbox*) f->widgets["txt_z"])->text);
    ed_cur_sector->textures[0].filename = ((lafi_textbox*) f->widgets["txt_texture"])->text;
    ed_cur_sector->brightness = toi(((lafi_textbox*) f->widgets["txt_brightness"])->text);
    //ToDo hazards.
    
    sector_to_gui();
}

/* ----------------------------------------------------------------------------
 * Handles the events for the area editor.
 */
void area_editor::handle_controls(ALLEGRO_EVENT ev) {

    ed_gui->handle_event(ev);
    
    //Update mouse cursor in world coordinates.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi_widget* wum;
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) wum = NULL;
        else wum = ed_gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y); //Widget under mouse.
        ((lafi_label*) ed_gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + itos(mouse_cursor_x) + "," + itos(mouse_cursor_y) + ")");
    }
    
    
    //Moving vertices, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        if(ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16 && ed_moving_vertex == string::npos) {
            ed_on_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL);
        } else {
            ed_on_sector = NULL;
        }
        
        //Move background.
        if(ed_sec_mode == ESM_BG_MOUSE) {
        
            if(ed_holding_m1) {
                ed_bg_x += ev.mouse.dx / cam_zoom;
                ed_bg_y += ev.mouse.dy / cam_zoom;
                
            } else if(ed_holding_m2) {
            
                float new_w = ed_bg_w + ev.mouse.dx / cam_zoom;
                float new_h = ed_bg_h + ev.mouse.dy / cam_zoom;
                
                if(ed_bg_aspect_ratio) {
                    //Find the most significant change.
                    if(ev.mouse.dx != 0 || ev.mouse.dy != 0) {
                        bool most_is_width = fabs((double) ev.mouse.dx) > fabs((double) ev.mouse.dy);
                        
                        
                        if(most_is_width) {
                            float ratio = ed_bg_h / ed_bg_w;
                            ed_bg_w = new_w;
                            ed_bg_h = new_w * ratio;
                        } else {
                            float ratio = ed_bg_w / ed_bg_h;
                            ed_bg_h = new_h;
                            ed_bg_w = new_h * ratio;
                        }
                    }
                } else {
                    ed_bg_w = new_w;
                    ed_bg_h = new_h;
                }
                
            }
            
            bg_to_gui();
            
            //Move camera.
        } else if(ed_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        //Move vertex.
        if(ed_moving_vertex != string::npos) {
            cur_area_map.vertices[ed_moving_vertex]->x = snap_to_grid(mouse_cursor_x);
            cur_area_map.vertices[ed_moving_vertex]->y = snap_to_grid(mouse_cursor_y);
        }
        
        if(ev.mouse.dz != 0) {
            //Zoom.
            float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
            new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
            new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
            float new_mc_x = ev.mouse.x / new_zoom - cam_x - ((scr_w - 208) / 2 / new_zoom);
            float new_mc_y = ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
            
            cam_x -= (mouse_cursor_x - new_mc_x);
            cam_y -= (mouse_cursor_y - new_mc_y);
            mouse_cursor_x = new_mc_x;
            mouse_cursor_y = new_mc_y;
            cam_zoom = new_zoom;
        }
        
        
        //Clicking.
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16) {
    
        if(ev.mouse.button == 1) ed_holding_m1 = true;
        else if(ev.mouse.button == 2) ed_holding_m2 = true;
        
        if(ev.mouse.button != 1) return;
        
        //Drag vertex.
        if(ev.mouse.button == 1 && ed_sec_mode == ESM_NONE) {
            if(ev.mouse.x < scr_w - 208) {
            
                //Find a vertex to drag.
                for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                    if(
                        check_dist(
                            mouse_cursor_x, mouse_cursor_y,
                            cur_area_map.vertices[v]->x, cur_area_map.vertices[v]->y,
                            6.0 / cam_zoom
                        )
                    ) {
                        ed_moving_vertex = v;
                        break;
                    }
                }
                
            }
        }
        
        //Place a new sector where the cursor is.
        if(ed_sec_mode == ESM_NEW_SECTOR) {
        
            ed_sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            size_t outer_sector_nr;
            sector* outer_sector = get_sector(hotspot_x, hotspot_y, &outer_sector_nr);
            
            sector* new_sector = new sector();
            if(outer_sector) {
                //ToDo missing attributes.
                new_sector->brightness = outer_sector->brightness;
                new_sector->textures[0] = outer_sector->textures[0];
                new_sector->textures[1] = outer_sector->textures[1];
                new_sector->type = outer_sector->type;
                new_sector->z = outer_sector->z;
            }
            
            //Create the vertices.
            vertex* new_vertices[4];
            for(size_t v = 0; v < 4; v++) new_vertices[v] = new vertex(0, 0);
            new_vertices[0]->x = hotspot_x - 32;
            new_vertices[0]->y = hotspot_y - 32;
            new_vertices[1]->x = hotspot_x + 32;
            new_vertices[1]->y = hotspot_y - 32;
            new_vertices[2]->x = hotspot_x + 32;
            new_vertices[2]->y = hotspot_y + 32;
            new_vertices[3]->x = hotspot_x - 32;
            new_vertices[3]->y = hotspot_y + 32;
            for(size_t v = 0; v < 4; v++)cur_area_map.vertices.push_back(new_vertices[v]);
            
            //Create the linedefs.
            linedef* new_linedefs[4];
            for(size_t l = 0; l < 4; l++) {
                new_linedefs[l] = new linedef(
                    cur_area_map.vertices.size() - (4 - l),
                    cur_area_map.vertices.size() - (4 - ((l + 1) % 4))
                );
                new_linedefs[l]->sector_nrs[0] = cur_area_map.sectors.size();
                new_linedefs[l]->sector_nrs[1] = outer_sector_nr;
                cur_area_map.linedefs.push_back(new_linedefs[l]);
            }
            
            //Add them to the area map.
            for(size_t l = 0; l < 4; l++) new_sector->linedef_nrs.push_back(cur_area_map.linedefs.size() - (4 - l));
            cur_area_map.sectors.push_back(new_sector);
            
            for(size_t l = 0; l < 4; l++) new_linedefs[l]->fix_pointers(cur_area_map);
            for(size_t v = 0; v < 4; v++) new_vertices[v]->connect_linedefs(cur_area_map, cur_area_map.vertices.size() - (4 - v));
            new_sector->connect_linedefs(cur_area_map, cur_area_map.sectors.size() - 1);
            
            //Add the linedefs to the outer sector's list.
            if(outer_sector) {
                for(size_t l = 0; l < 4; l++) {
                    outer_sector->linedefs.push_back(new_linedefs[l]);
                    outer_sector->linedef_nrs.push_back(cur_area_map.linedefs.size() - (4 - l));
                }
            }
            
            //Check for intersections.
            for(size_t v = 0; v < 4; v += 2) check_linedef_intersections(new_vertices[v]);
            
            //Triangulate new sector and the parent one.
            triangulate(new_sector);
            if(outer_sector) triangulate(outer_sector);
            
            
            //Create a new vertex in a linedef.
        } else if(ed_moving_vertex == string::npos && ed_sec_mode == ESM_NONE) {
        
            if(ed_double_click_time == 0) ed_double_click_time = 0.5;
            else {
                ed_double_click_time = 0;
                //Create a new vertex.
                
                for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
                    linedef* l_ptr = cur_area_map.linedefs[l];
                    
                    if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
                    
                    if(
                        circle_intersects_line(
                            mouse_cursor_x, mouse_cursor_y, 6,
                            l_ptr->vertices[0]->x, l_ptr->vertices[0]->y,
                            l_ptr->vertices[1]->x, l_ptr->vertices[1]->y
                        )
                    ) {
                    
                        //New vertex, on the split point.
                        vertex* new_v_ptr = new vertex(mouse_cursor_x, mouse_cursor_y);
                        cur_area_map.vertices.push_back(new_v_ptr);
                        
                        //New linedef, copied from the original one.
                        linedef* new_l_ptr = new linedef(*l_ptr);
                        cur_area_map.linedefs.push_back(new_l_ptr);
                        
                        //Save the original end vertex for later.
                        vertex* end_v_ptr = l_ptr->vertices[1];
                        
                        //Set vertices on the new and original linedefs.
                        new_l_ptr->vertex_nrs[0] = cur_area_map.vertices.size() - 1;
                        new_l_ptr->vertices[0] = new_v_ptr;
                        l_ptr->vertex_nrs[1] = new_l_ptr->vertex_nrs[0];
                        l_ptr->vertices[1] = new_v_ptr;
                        
                        //Set sectors on the new linedef.
                        if(new_l_ptr->sectors[0]) {
                            new_l_ptr->sectors[0]->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                            new_l_ptr->sectors[0]->linedefs.push_back(new_l_ptr);
                        }
                        if(new_l_ptr->sectors[1]) {
                            new_l_ptr->sectors[1]->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                            new_l_ptr->sectors[1]->linedefs.push_back(new_l_ptr);
                        }
                        
                        //Set linedefs of the new vertex.
                        new_v_ptr->linedef_nrs.push_back(cur_area_map.linedefs.size() - 1);
                        new_v_ptr->linedef_nrs.push_back(l);
                        new_v_ptr->linedefs.push_back(new_l_ptr);
                        new_v_ptr->linedefs.push_back(l_ptr);
                        
                        //Update linedef data on the end vertex of the original line
                        //(it now links to the new line, not the old).
                        for(size_t vl = 0; vl < end_v_ptr->linedefs.size(); vl++) {
                            if(end_v_ptr->linedefs[vl] == l_ptr) {
                                end_v_ptr->linedefs[vl] = new_l_ptr;
                                end_v_ptr->linedef_nrs[vl] = cur_area_map.linedefs.size() - 1;
                                break;
                            }
                        }
                        
                        break;
                    }
                }
            }
            
        } else if(ed_sec_mode == ESM_SEL_SECTOR) {
            ed_sec_mode = ESM_NONE;
            
            ed_cur_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL);
            sector_to_gui();
            adv_textures_to_gui();
            
        }
        
        //Mouse button release.
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 1) ed_holding_m1 = false;
        else if(ev.mouse.button == 2) ed_holding_m2 = false;
        
        if(ev.mouse.button == 1 && ed_sec_mode == ESM_NONE) {
            //Release the vertex.
            
            if(ed_moving_vertex != string::npos) {
                vertex* moved_v_ptr = cur_area_map.vertices[ed_moving_vertex];
                vertex* final_vertex = moved_v_ptr;
                
                unordered_set<sector*> affected_sectors;
                
                //Check if the line's vertices intersect with any other lines.
                //If so, they're marked with red.
                check_linedef_intersections(moved_v_ptr);
                
                
                //Check if we should merge.
                for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                    vertex* dest_v_ptr = cur_area_map.vertices[v];
                    if(dest_v_ptr == moved_v_ptr) continue;
                    
                    if(check_dist(moved_v_ptr->x, moved_v_ptr->y, dest_v_ptr->x, dest_v_ptr->y, 10)) {
                        //Merge vertices.
                        
                        //Find out what to do with every linedef of the dragged vertex.
                        for(size_t l = 0; l < moved_v_ptr->linedefs.size();) {
                        
                            bool was_deleted = false;
                            linedef* l_ptr = moved_v_ptr->linedefs[l];
                            vertex* other_vertex = l_ptr->vertices[0] == moved_v_ptr ? l_ptr->vertices[1] : l_ptr->vertices[0];
                            
                            //Check if it's being squashed into non-existence.
                            if(other_vertex == dest_v_ptr) {
                            
                                affected_sectors.insert(l_ptr->sectors[0]);
                                affected_sectors.insert(l_ptr->sectors[1]);
                                
                                //Clear it from its vertices' lists.
                                for(size_t vl = 0; vl < other_vertex->linedefs.size(); vl++) {
                                    if(other_vertex->linedefs[vl] == l_ptr) {
                                        other_vertex->linedefs.erase(other_vertex->linedefs.begin() + vl);
                                        other_vertex->linedef_nrs.erase(other_vertex->linedef_nrs.begin() + vl);
                                        break;
                                    }
                                }
                                
                                //Clear it from the sector lists.
                                for(size_t s = 0; s < 2; s++) {
                                    if(!l_ptr->sectors[s]) continue;
                                    for(size_t sl = 0; sl < l_ptr->sectors[s]->linedefs.size(); sl++) {
                                        if(l_ptr->sectors[s]->linedefs[sl] == l_ptr) {
                                            l_ptr->sectors[s]->linedefs.erase(l_ptr->sectors[s]->linedefs.begin() + sl);
                                            l_ptr->sectors[s]->linedef_nrs.erase(l_ptr->sectors[s]->linedef_nrs.begin() + sl);
                                            break;
                                        }
                                    }
                                }
                                
                                //Clear its info, so it gets marked for deletion.
                                l_ptr->vertex_nrs[0] = l_ptr->vertex_nrs[1] = string::npos;
                                l_ptr->fix_pointers(cur_area_map);
                                
                            } else {
                            
                                bool has_merged = false;
                                //Check if the linedef will be merged with another one.
                                //These are linedefs that share a common vertex,
                                //plus the moved/destination vertex.
                                for(size_t dl = 0; dl < dest_v_ptr->linedefs.size(); dl++) {
                                
                                    linedef* dl_ptr = dest_v_ptr->linedefs[dl];
                                    vertex* d_other_vertex = dl_ptr->vertices[0] == dest_v_ptr ? dl_ptr->vertices[1] : dl_ptr->vertices[0];
                                    
                                    if(d_other_vertex == other_vertex) {
                                        //The linedef will be merged with this one.
                                        has_merged = true;
                                        affected_sectors.insert(l_ptr->sectors[0]);
                                        affected_sectors.insert(l_ptr->sectors[1]);
                                        affected_sectors.insert(dl_ptr->sectors[0]);
                                        affected_sectors.insert(dl_ptr->sectors[1]);
                                        
                                        //Tell the destination linedef's sectors
                                        //to forget it; they'll be re-added later.
                                        size_t old_dl_nr = dl_ptr->remove_from_sectors();
                                        
                                        //Set the new sectors.
                                        //ToDo if one of the central sectors is null.
                                        if(l_ptr->sector_nrs[0] == dl_ptr->sector_nrs[0])
                                            dl_ptr->sector_nrs[0] = l_ptr->sector_nrs[1];
                                        else if(l_ptr->sector_nrs[0] == dl_ptr->sector_nrs[1])
                                            dl_ptr->sector_nrs[1] = l_ptr->sector_nrs[1];
                                        else if(l_ptr->sector_nrs[1] == dl_ptr->sector_nrs[0] || !l_ptr->sectors[0])
                                            dl_ptr->sector_nrs[0] = l_ptr->sector_nrs[0];
                                        else if(l_ptr->sector_nrs[1] == dl_ptr->sector_nrs[1] || !l_ptr->sectors[1])
                                            dl_ptr->sector_nrs[1] = l_ptr->sector_nrs[0];
                                        dl_ptr->fix_pointers(cur_area_map);
                                        
                                        //Go to the linedef's old vertices,
                                        //and tell them that it no longer exists.
                                        for(size_t v = 0; v < 2; v++) {
                                        
                                            for(size_t vl = 0; vl < l_ptr->vertices[v]->linedefs.size(); vl++) {
                                                linedef* vl_ptr = l_ptr->vertices[v]->linedefs[vl];
                                                
                                                if(vl_ptr == l_ptr) {
                                                    l_ptr->vertices[v]->linedefs.erase(
                                                        l_ptr->vertices[v]->linedefs.begin() + vl);
                                                    l_ptr->vertices[v]->linedef_nrs.erase(
                                                        l_ptr->vertices[v]->linedef_nrs.begin() + vl);
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        //Now tell the linedef's old sectors.
                                        l_ptr->remove_from_sectors();
                                        
                                        //Add the linedefs to the sectors' lists.
                                        for(size_t s = 0; s < 2; s++) {
                                            if(!dl_ptr->sectors[s]) continue;
                                            dl_ptr->sectors[s]->linedefs.push_back(dl_ptr);
                                            dl_ptr->sectors[s]->linedef_nrs.push_back(old_dl_nr);
                                        }
                                        
                                        //Remove the deleted linedef's info.
                                        //This'll mark it for deletion.
                                        l_ptr->sector_nrs[0] = l_ptr->sector_nrs[1] = string::npos;
                                        l_ptr->vertex_nrs[0] = l_ptr->vertex_nrs[1] = string::npos;
                                        l_ptr->fix_pointers(cur_area_map);
                                        was_deleted = true;
                                        
                                        break;
                                    }
                                }
                                
                                //If it's matchless, that means it'll just be joined to
                                //the group of linedefs on the destination vertex.
                                if(!has_merged) {
                                    dest_v_ptr->linedef_nrs.push_back(moved_v_ptr->linedef_nrs[l]);
                                    unsigned char n = (l_ptr->vertices[0] == moved_v_ptr ? 0 : 1);
                                    l_ptr->vertices[n] = dest_v_ptr;
                                    l_ptr->vertex_nrs[n] = v;
                                }
                            }
                            
                            if(!was_deleted) l++;
                            
                        }
                        
                        dest_v_ptr->fix_pointers(cur_area_map);
                        
                        //If this vertex is out of linedefs, it'll be
                        //deleted eventually. Move it out of the way.
                        if(dest_v_ptr->linedefs.size() == 0) {
                            dest_v_ptr->x = dest_v_ptr->y = FLT_MAX;
                        }
                        
                        //Remove the old vertex' info.
                        //This'll mark it for deletion.
                        moved_v_ptr->linedef_nrs.clear();
                        moved_v_ptr->linedefs.clear();
                        moved_v_ptr->x = moved_v_ptr->y = FLT_MAX; //So it's out of the way.
                        
                        final_vertex = dest_v_ptr;
                        
                        break;
                    }
                }
                
                //Finally, re-triangulate the affected sectors.
                for(size_t l = 0; l < final_vertex->linedefs.size(); l++) {
                    linedef* l_ptr = final_vertex->linedefs[l];
                    for(size_t s = 0; s < 2; s++) {
                        if(l_ptr->sectors[s]) affected_sectors.insert(l_ptr->sectors[s]);
                    }
                }
                for(auto s = affected_sectors.begin(); s != affected_sectors.end(); s++) {
                    if(!(*s)) continue;
                    triangulate(*s);
                }
                
                ed_moving_vertex = string::npos;
                
            }
        }
        
        //Key press.
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            ed_shift_pressed = true;
        }
        
        //Key release.
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            ed_shift_pressed = false;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {
    ed_mode = EDITOR_MODE_MAIN;
    
    lafi_style* s = new lafi_style(al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32), al_map_rgb(96, 128, 160));
    ed_gui = new lafi_gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi_frame* frm_main = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    ed_gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add("lbl_area", new lafi_label(0, 0, 0, 0, "Area:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_area", new lafi_button(0, 0, 0, 0), 100, 32);
    int y = frm_main->easy_row();
    
    lafi_frame* frm_area = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    frm_area->easy_row();
    frm_area->easy_add("but_sectors", new lafi_button(0, 0, 0, 0, "Edit sectors"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_objects", new lafi_button(0, 0, 0, 0, "Edit objects"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_bg", new lafi_button(0, 0, 0, 0, "Edit background"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_review", new lafi_button(0, 0, 0, 0, "Review"), 100, 32);
    frm_area->easy_row();
    
    
    //Bottom bar.
    lafi_frame* frm_bottom = new lafi_frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    ed_gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_load", new lafi_button(           0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save", new lafi_button(           0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit", new lafi_button(           0, 0, 0, 0, "X"), 25, 32);
    frm_bottom->easy_row();
    
    
    //Picker frame.
    lafi_frame* frm_picker = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    ed_gui->add("frm_picker", frm_picker);
    
    frm_picker->add("but_back", new lafi_button(     scr_w - 200, 8, scr_w - 104, 24, "Back"));
    frm_picker->add("frm_list", new lafi_frame(      scr_w - 200, 40, scr_w - 32, scr_h - 56));
    frm_picker->add("bar_scroll", new lafi_scrollbar(scr_w - 24,  40, scr_w - 8,  scr_h - 56));
    
    
    //Sectors frame.
    lafi_frame* frm_sectors = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_sectors);
    ed_gui->add("frm_sectors", frm_sectors);
    
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_select", new lafi_button(0, 0, 0, 0, "Sel"), 20, 32);
    frm_sectors->easy_add("but_new", new lafi_button(0, 0, 0, 0, "+"), 20, 32);
    y = frm_sectors->easy_row();
    
    lafi_frame* frm_sector = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_sectors->add("frm_sector", frm_sector);
    hide_widget(frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_z", new lafi_label(0, 0, 0, 0, "Height:"), 50, 16);
    frm_sector->easy_add("txt_z", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_texture", new lafi_label(0, 0, 0, 0, "Texture:"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("txt_texture", new lafi_textbox(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("but_adv", new lafi_button(0, 0, 0, 0, "Adv. texture settings"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lin_1", new lafi_line(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_brightness", new lafi_label(0, 0, 0, 0, "Brightness:"), 50, 16);
    frm_sector->easy_add("txt_brightness", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_type", new lafi_label(0, 0, 0, 0, "Type:"), 30, 24);
    frm_sector->easy_add("but_type", new lafi_button(0, 0, 0, 0), 70, 24);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_hazards", new lafi_label(0, 0, 0, 0, "Hazards:"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("txt_hazards", new lafi_textbox(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    
    
    //Advanced sector texture settings frame.
    lafi_frame* frm_adv_textures = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_adv_textures);
    ed_gui->add("frm_adv_textures", frm_adv_textures);
    
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lin_1", new lafi_line(0, 0, 0, 0), 20, 16);
    frm_adv_textures->easy_add("lbl_main", new lafi_label(0, 0, 0, 0, "Main texture"), 60, 16);
    frm_adv_textures->easy_add("lin_2", new lafi_line(0, 0, 0, 0), 20, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_xy", new lafi_label(0, 0, 0, 0, "X&Y:"), 40, 16);
    frm_adv_textures->easy_add("txt_x", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_add("txt_y", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_sxy", new lafi_label(0, 0, 0, 0, "Scale:"), 40, 16);
    frm_adv_textures->easy_add("txt_sx", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_add("txt_sy", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_a", new lafi_label(0, 0, 0, 0, "Angle:"), 50, 16);
    frm_adv_textures->easy_add("txt_a", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("chk_fade", new lafi_checkbox(0, 0, 0, 0, "Fade into 2nd texture"), 100, 16);
    y = frm_adv_textures->easy_row();
    
    lafi_frame* frm_texture_2 = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_adv_textures->add("frm_texture_2", frm_texture_2);
    hide_widget(frm_texture_2);
    
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lin_3", new lafi_line(0, 0, 0, 0), 15, 16);
    frm_texture_2->easy_add("lbl_texture_2", new lafi_label(0, 0, 0, 0, "Second texture"), 70, 16);
    frm_texture_2->easy_add("lin_4", new lafi_line(0, 0, 0, 0), 15, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lbl_fade_a", new lafi_label(0, 0, 0, 0, "Fade angle:"), 50, 16);
    frm_texture_2->easy_add("txt_fade_a", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lbl_texture", new lafi_label(0, 0, 0, 0, "Texture:"), 100, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("txt_2texture", new lafi_textbox(0, 0, 0, 0), 100, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lbl_2xy", new lafi_label(0, 0, 0, 0, "X&Y:"), 40, 16);
    frm_texture_2->easy_add("txt_2x", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_texture_2->easy_add("txt_2y", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lbl_2sxy", new lafi_label(0, 0, 0, 0, "Scale:"), 40, 16);
    frm_texture_2->easy_add("txt_2sx", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_texture_2->easy_add("txt_2sy", new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_texture_2->easy_row();
    frm_texture_2->easy_add("lbl_2a", new lafi_label(0, 0, 0, 0, "Angle:"), 50, 16);
    frm_texture_2->easy_add("txt_2a", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_texture_2->easy_row();
    
    
    //Background frame.
    lafi_frame* frm_bg = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_bg);
    ed_gui->add("frm_bg", frm_bg);
    
    frm_bg->easy_row();
    frm_bg->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_file", new lafi_label(0, 0, 0, 0, "File:"), 30, 16);
    frm_bg->easy_add("txt_file", new lafi_textbox(0, 0, 0, 0), 70, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_xy", new lafi_label(0, 0, 0, 0, "X&Y:"), 30, 16);
    frm_bg->easy_add("txt_x", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_add("txt_y", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_wh", new lafi_label(0, 0, 0, 0, "W&H:"), 30, 16);
    frm_bg->easy_add("txt_w", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_add("txt_h", new lafi_textbox(0, 0, 0, 0), 35, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("chk_ratio", new lafi_checkbox(0, 0, 0, 0, "Keep aspect ratio"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("chk_mouse", new lafi_checkbox(0, 0, 0, 0, "Transform with mouse"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("lbl_alpha", new lafi_label(0, 0, 0, 0, "Transparency:"), 100, 16);
    frm_bg->easy_row();
    frm_bg->easy_add("bar_alpha", new lafi_scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24);
    frm_bg->easy_row();
    
    
    //Review frame.
    lafi_frame* frm_review = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_review);
    ed_gui->add("frm_review", frm_review);
    
    frm_review->easy_row();
    frm_review->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_review->easy_row();
    frm_review->easy_add("but_find_errors", new lafi_button(0, 0, 0, 0, "Find errors"), 100, 24);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_lbl", new lafi_label(0, 0, 0, 0, "Error found:", ALLEGRO_ALIGN_CENTER), 100, 16);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_1", new lafi_label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_2", new lafi_label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_3", new lafi_label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_4", new lafi_label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("but_goto_error", new lafi_button(0, 0, 0, 0, "Go to error"), 100, 24);
    frm_review->easy_row();
    frm_review->easy_add("lin_1", new lafi_line(0, 0, 0, 0), 100, 16);
    frm_review->easy_row();
    frm_review->easy_add("chk_see_textures", new lafi_checkbox(0, 0, 0, 0, "See textures"), 100, 16);
    frm_review->easy_row();
    update_review_frame();
    
    
    //Status bar.
    lafi_label* ed_gui_status_bar = new lafi_label(0, scr_h - 16, scr_w - 208, scr_h);
    ed_gui->add("lbl_status_bar", ed_gui_status_bar);
    
    
    //Properties -- main.
    frm_area->widgets["but_sectors"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_area->widgets["but_bg"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_BG;
        change_to_right_frame();
    };
    frm_area->widgets["but_review"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_REVIEW;
        change_to_right_frame();
    };
    frm_area->widgets["but_sectors"]->description = "Change sector (polygon) settings.";
    frm_area->widgets["but_objects"]->description = "Change object settings and placements.";
    frm_area->widgets["but_bg"]->description = "Add a background to guide you, like a blueprint.";
    frm_area->widgets["but_review"]->description = "Tools to make sure everything is fine in the area.";
    
    
    //Properties -- bottom.
    frm_bottom->widgets["but_load"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        load_area();
    };
    frm_bottom->widgets["but_save"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        save_area();
    };
    frm_bottom->widgets["but_load"]->description = "Load the area from the files.";
    frm_bottom->widgets["but_save"]->description = "Save the area onto the disk.";
    frm_bottom->widgets["but_quit"]->description = "Quit the area editor.";
    
    
    //Properties -- sectors.
    auto lambda_gui_to_sector = [] (lafi_widget*) { gui_to_sector(); };
    frm_sectors->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_sectors->widgets["but_select"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        if(ed_sec_mode == ESM_SEL_SECTOR) ed_sec_mode = ESM_NONE;
        else ed_sec_mode = ESM_SEL_SECTOR;
    };
    frm_sectors->widgets["but_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        if(ed_sec_mode == ESM_NEW_SECTOR) ed_sec_mode = ESM_NONE;
        else ed_sec_mode = ESM_NEW_SECTOR;
    };
    frm_sector->widgets["but_type"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_SECTOR_TYPE);
    };
    frm_sector->widgets["but_adv"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_ADV_TEXTURE_SETTINGS;
        change_to_right_frame();
        adv_textures_to_gui();
    };
    frm_sector->widgets["txt_z"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_texture"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_brightness"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sectors->widgets["but_back"]->description = "Go back to the main menu.";
    frm_sectors->widgets["but_select"]->description = "Select a sector to change the properties of.";
    frm_sectors->widgets["but_new"]->description = "Create a new sector where you click.";
    frm_sector->widgets["txt_z"]->description = "Height of the floor.";
    frm_sector->widgets["txt_texture"]->description = "File name of the Texture (image) of the floor.";
    frm_sector->widgets["txt_brightness"]->description = "0 = pitch black sector. 255 = normal lighting.";
    frm_sector->widgets["txt_hazards"]->description = "Hazards the sector has.";
    frm_sector->widgets["but_type"]->description = "Change the type of sector.";
    frm_sector->widgets["but_adv"]->description = "Advanced settings for the sector's texture.";
    
    
    //Properties -- advanced textures.
    auto lambda_gui_to_adv_textures = [] (lafi_widget*) { gui_to_adv_textures(); };
    auto lambda_gui_to_adv_textures_click = [] (lafi_widget*, int, int) { gui_to_adv_textures(); };
    frm_adv_textures->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_adv_textures->widgets["txt_x"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_y"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sx"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sy"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_a"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["chk_fade"]->left_mouse_click_handler = lambda_gui_to_adv_textures_click;
    frm_texture_2->widgets["txt_fade_a"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2texture"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2x"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2y"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2sx"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2sy"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_texture_2->widgets["txt_2a"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_x"]->description = "Scroll the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_y"]->description = "Scroll the texture vertically by this much.";
    frm_adv_textures->widgets["txt_sx"]->description = "Zoom the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_sy"]->description = "Zoom the texture vertically by this much.";
    frm_adv_textures->widgets["txt_a"]->description = "Rotate the texture by this much (radians).";
    frm_adv_textures->widgets["chk_fade"]->description = "Does the texture gradually fade into another?";
    frm_texture_2->widgets["txt_fade_a"]->description = "The texture fades in this angle towards the 2nd one.";
    frm_texture_2->widgets["txt_2texture"]->description = "File name of the texture to fade into.";
    frm_texture_2->widgets["txt_2x"]->description = "Scroll the texture horizontally by this much.";
    frm_texture_2->widgets["txt_2y"]->description = "Scroll the texture vertically by this much.";
    frm_texture_2->widgets["txt_2sx"]->description = "Zoom the texture horizontally by this much.";
    frm_texture_2->widgets["txt_2sy"]->description = "Zoom the texture vertically by this much.";
    frm_texture_2->widgets["txt_2a"]->description = "Rotate the texture by this much (radians).";
    
    
    //Properties -- background.
    auto lambda_gui_to_bg = [] (lafi_widget*) { gui_to_bg(); };
    auto lambda_gui_to_bg_click = [] (lafi_widget*, int, int) { gui_to_bg(); };
    frm_bg->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_bg->widgets["txt_file"]->lose_focus_handler = lambda_gui_to_bg;
    frm_bg->widgets["txt_x"]->lose_focus_handler = lambda_gui_to_bg;
    frm_bg->widgets["txt_y"]->lose_focus_handler = lambda_gui_to_bg;
    frm_bg->widgets["txt_w"]->lose_focus_handler = lambda_gui_to_bg;
    frm_bg->widgets["txt_h"]->lose_focus_handler = lambda_gui_to_bg;
    ((lafi_scrollbar*) frm_bg->widgets["bar_alpha"])->change_handler = lambda_gui_to_bg;
    frm_bg->widgets["chk_ratio"]->left_mouse_click_handler = lambda_gui_to_bg_click;
    frm_bg->widgets["chk_mouse"]->left_mouse_click_handler = lambda_gui_to_bg_click;
    frm_bg->widgets["but_back"]->description = "Go back to the main menu.";
    frm_bg->widgets["txt_file"]->description = "Image file (on the Images folder) for the background.";
    frm_bg->widgets["txt_x"]->description = "X of the top-left corner for the background.";
    frm_bg->widgets["txt_y"]->description = "Y of the top-left corner for the background.";
    frm_bg->widgets["txt_w"]->description = "Background total width.";
    frm_bg->widgets["txt_h"]->description = "Background total height.";
    frm_bg->widgets["bar_alpha"]->description = "How see-through the background is.";
    frm_bg->widgets["chk_ratio"]->description = "Lock the width/height proportions when changing either one.";
    frm_bg->widgets["chk_mouse"]->description = "If checked, drag left mouse to move, right mouse to stretch.";
    bg_to_gui();
    
    
    //Properties -- review.
    frm_review->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        ed_sec_mode = ESM_NONE;
        update_review_frame();
        change_to_right_frame();
    };
    frm_review->widgets["but_find_errors"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        find_errors();
    };
    frm_review->widgets["but_goto_error"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        goto_error();
    };
    frm_review->widgets["chk_see_textures"]->left_mouse_click_handler = [] (lafi_widget * c, int, int) {
        ed_error_type = EET_NONE;
        if(((lafi_checkbox*) c)->checked) {
            ed_sec_mode = ESM_TEXTURE_VIEW;
            clear_area_textures();
            load_area_textures();
            update_review_frame();
            disable_widget(ed_gui->widgets["frm_review"]->widgets["but_find_errors"]);
            disable_widget(ed_gui->widgets["frm_review"]->widgets["but_goto_error"]);
        } else {
            ed_sec_mode = ESM_NONE;
            enable_widget(ed_gui->widgets["frm_review"]->widgets["but_find_errors"]);
            update_review_frame();
        }
    };
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        show_widget(ed_gui->widgets["frm_bottom"]);
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description = "Cancel.";
    
    
    //Area loading.
    ed_filename = "test"; //ToDo non-fixed name, duh.
    load_area();
}

/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 */
void area_editor::load_area() {
    ::load_area(ed_filename);
    ed_mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
}

/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type, use area_editor::AREA_EDITOR_PICKER_*.
 */
void area_editor::open_picker(unsigned char type) {
    change_to_right_frame(true);
    show_widget(ed_gui->widgets["frm_picker"]);
    hide_widget(ed_gui->widgets["frm_bottom"]);
    
    lafi_widget* f = ed_gui->widgets["frm_picker"]->widgets["frm_list"];
    
    while(f->widgets.size()) {
        f->remove(f->widgets.begin()->first);
    }
    
    vector<string> elements;
    if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
        elements.push_back(SECTOR_TYPE_STR_NORMAL);
        elements.push_back(SECTOR_TYPE_STR_BOTTOMLESS_PIT);
        elements.push_back(SECTOR_TYPE_STR_LANDING_SITE);
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); e++) {
        lafi_button* b = new lafi_button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler = [name, type] (lafi_widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + itos(e), b, 100, 24);
        f->easy_row(0);
    }
    
    ((lafi_scrollbar*) ed_gui->widgets["frm_picker"]->widgets["bar_scroll"])->make_widget_scroll(f);
}

/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void area_editor::pick(string name, unsigned char type) {
    change_to_right_frame();
    
    if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
        if(ed_cur_sector) {
            if(name == SECTOR_TYPE_STR_NORMAL)              ed_cur_sector->type = SECTOR_TYPE_NORMAL;
            else if(name == SECTOR_TYPE_STR_BOTTOMLESS_PIT) ed_cur_sector->type = SECTOR_TYPE_BOTTOMLESS_PIT;
            else if(name == SECTOR_TYPE_STR_LANDING_SITE)   ed_cur_sector->type = SECTOR_TYPE_LANDING_SITE;
            sector_to_gui();
        }
        
    }
}

/* ----------------------------------------------------------------------------
 * Saves the area onto the disk.
 */
void area_editor::save_area() {
    data_node file_node = data_node("", "");
    
    //Start by cleaning unused vertices, sectors and linedefs.
    //Unused vertices.
    for(size_t v = 0; v < cur_area_map.vertices.size(); ) {
    
        vertex* v_ptr = cur_area_map.vertices[v];
        if(v_ptr->linedef_nrs.size() == 0) {
        
            cur_area_map.vertices.erase(cur_area_map.vertices.begin() + v);
            
            //Fix numbers in linedef lists.
            for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
                linedef* l_ptr = cur_area_map.linedefs[l];
                for(unsigned char lv = 0; lv < 2; lv++) {
                    if(l_ptr->vertex_nrs[lv] >= v && l_ptr->vertex_nrs[lv] != string::npos) {
                        l_ptr->vertex_nrs[lv]--;
                    }
                }
            }
            
        } else {
            v++;
        }
    }
    
    //Unused sectors.
    for(size_t s = 0; s < cur_area_map.sectors.size(); ) {
    
        sector* s_ptr = cur_area_map.sectors[s];
        if(s_ptr->linedef_nrs.size() == 0) {
        
            cur_area_map.sectors.erase(cur_area_map.sectors.begin() + s);
            
            //Fix numbers in linedef lists.
            for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
                linedef* l_ptr = cur_area_map.linedefs[l];
                for(unsigned char ls = 0; ls < 2; ls++) {
                    if(l_ptr->sector_nrs[ls] >= s && l_ptr->sector_nrs[ls] != string::npos) {
                        l_ptr->sector_nrs[ls]--;
                    }
                }
            }
            
        } else {
            s++;
        }
    }
    
    //Unused linedefs.
    for(size_t l = 0; l < cur_area_map.linedefs.size(); ) {
    
        linedef* l_ptr = cur_area_map.linedefs[l];
        if(l_ptr->vertex_nrs[0] == string::npos) {
        
            cur_area_map.linedefs.erase(cur_area_map.linedefs.begin() + l);
            
            //Fix numbers in vertex lists.
            for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                vertex* v_ptr = cur_area_map.vertices[v];
                for(size_t vl = 0; vl < v_ptr->linedef_nrs.size(); vl++) {
                    if(v_ptr->linedef_nrs[vl] >= l && v_ptr->linedef_nrs[vl] != string::npos) {
                        v_ptr->linedef_nrs[vl]--;
                    }
                }
            }
            
            //Fix numbers in sector lists.
            for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
                sector* s_ptr = cur_area_map.sectors[s];
                for(size_t sl = 0; sl < s_ptr->linedef_nrs.size(); sl++) {
                    if(s_ptr->linedef_nrs[sl] >= l && s_ptr->linedef_nrs[sl] != string::npos) {
                        s_ptr->linedef_nrs[sl]--;
                    }
                }
            }
            
        } else {
            l++;
        }
    }
    
    
    //Save the content now.
    //Vertices.
    data_node* vertices_node = new data_node("vertices", "");
    file_node.add(vertices_node);
    
    for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
        vertex* v_ptr = cur_area_map.vertices[v];
        data_node* vertex_node = new data_node("vertex", ftos(v_ptr->x) + " " + ftos(v_ptr->y));
        vertices_node->add(vertex_node);
    }
    
    //Linedefs.
    data_node* linedefs_node = new data_node("linedefs", "");
    file_node.add(linedefs_node);
    
    for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
        linedef* l_ptr = cur_area_map.linedefs[l];
        data_node* linedef_node = new data_node("linedef", "");
        linedefs_node->add(linedef_node);
        string s_str;
        for(size_t s = 0; s < 2; s++) {
            if(l_ptr->sector_nrs[s] == string::npos) s_str += "-1";
            else s_str += itos(l_ptr->sector_nrs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        linedef_node->add(new data_node("s", s_str));
        linedef_node->add(new data_node("v", itos(l_ptr->vertex_nrs[0]) + " " + itos(l_ptr->vertex_nrs[1])));
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    file_node.add(sectors_node);
    
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        data_node* sector_node = new data_node("sector", "");
        sectors_node->add(sector_node);
        sector_node->add(new data_node("z", ftos(s_ptr->z)));
        if(s_ptr->brightness != DEF_SECTOR_BRIGHTNESS) sector_node->add(new data_node("brightness", itos(s_ptr->brightness)));
        if(s_ptr->fade) sector_node->add(new data_node("fade", btos(s_ptr->fade)));
        if(s_ptr->fade) sector_node->add(new data_node("fade_angle", ftos(s_ptr->fade_angle)));
        
        for(unsigned char t = 0; t < (size_t) (s_ptr->fade ? 2 : 1); t++) {
            string n = itos(t);
            
            sector_node->add(new data_node("texture_" + n, s_ptr->textures[t].filename));
            if(s_ptr->textures[t].rot != 0) {
                sector_node->add(new data_node("texture_" + n + "rotate", ftos(s_ptr->textures[t].rot)));
            }
            if(s_ptr->textures[t].scale_x != 0 || s_ptr->textures[t].scale_y != 0) {
                sector_node->add(new data_node("texture_" + n + "scale",
                                               ftos(s_ptr->textures[t].scale_x) + " " + ftos(s_ptr->textures[t].scale_y)));
            }
            if(s_ptr->textures[t].trans_x != 0 || s_ptr->textures[t].trans_y != 0) {
                sector_node->add(new data_node("texture_" + n + "trans",
                                               ftos(s_ptr->textures[t].trans_x) + " " + ftos(s_ptr->textures[t].trans_y)));
            }
        }
    }
    
    file_node.save_file(AREA_FOLDER "/" + ed_filename + ".txt");
}

/* ----------------------------------------------------------------------------
 * Loads the current sector's data onto the gui.
 */
void area_editor::sector_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_sectors"]->widgets["frm_sector"];
    if(ed_cur_sector) {
        show_widget(f);
        
        ((lafi_textbox*) f->widgets["txt_z"])->text = ftos(ed_cur_sector->z);
        ((lafi_textbox*) f->widgets["txt_texture"])->text = ed_cur_sector->textures[0].filename;
        ((lafi_textbox*) f->widgets["txt_brightness"])->text = itos(ed_cur_sector->brightness);
        //ToDo hazards.
        
        lafi_button* but_type = ((lafi_button*) f->widgets["but_type"]);
        if(ed_cur_sector->type == SECTOR_TYPE_NORMAL) {
            but_type->text = SECTOR_TYPE_STR_NORMAL;
        } else if(ed_cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            but_type->text = SECTOR_TYPE_STR_BOTTOMLESS_PIT;
        } else if(ed_cur_sector->type == SECTOR_TYPE_LANDING_SITE) {
            but_type->text = SECTOR_TYPE_STR_LANDING_SITE;
        }
        
    } else {
        hide_widget(f);
    }
}

/* ----------------------------------------------------------------------------
 * Snaps a coordinate to the nearest grid space.
 */
float area_editor::snap_to_grid(const float c) {
    if(ed_shift_pressed) return c;
    return round(c / 32) * 32;
}

/* ----------------------------------------------------------------------------
 * Updates the widgets on the review frame.
 */
void area_editor::update_review_frame() {
    lafi_button* but_goto_error = (lafi_button*) ed_gui->widgets["frm_review"]->widgets["but_goto_error"];
    lafi_label* lbl_error_1 = (lafi_label*) ed_gui->widgets["frm_review"]->widgets["lbl_error_1"];
    lafi_label* lbl_error_2 = (lafi_label*) ed_gui->widgets["frm_review"]->widgets["lbl_error_2"];
    lafi_label* lbl_error_3 = (lafi_label*) ed_gui->widgets["frm_review"]->widgets["lbl_error_3"];
    lafi_label* lbl_error_4 = (lafi_label*) ed_gui->widgets["frm_review"]->widgets["lbl_error_4"];
    
    lbl_error_2->text = "";
    lbl_error_3->text = "";
    lbl_error_4->text = "";
    
    if(ed_error_type == EET_NONE) {
        disable_widget(but_goto_error);
        lbl_error_1->text = "---";
        
    } else {
        enable_widget(but_goto_error);
        
        if(ed_error_type == EET_INTERSECTING_LINEDEFS) {
            lbl_error_1->text = "Two lines cross";
            lbl_error_2->text = "each other, at";
            float u;
            linedef_intersection* li_ptr = &ed_intersecting_lines[ed_error_size_t_1];
            lines_intersect(
                li_ptr->l1->vertices[0]->x, li_ptr->l1->vertices[0]->y,
                li_ptr->l1->vertices[1]->x, li_ptr->l1->vertices[1]->y,
                li_ptr->l2->vertices[0]->x, li_ptr->l2->vertices[0]->y,
                li_ptr->l2->vertices[1]->x, li_ptr->l2->vertices[1]->y,
                NULL, &u
            );
            
            float a = atan2(
                          li_ptr->l1->vertices[1]->y - li_ptr->l1->vertices[0]->y,
                          li_ptr->l1->vertices[1]->x - li_ptr->l1->vertices[0]->x
                      );
            float d = dist(
                          li_ptr->l1->vertices[0]->x, li_ptr->l1->vertices[0]->y,
                          li_ptr->l1->vertices[1]->x, li_ptr->l1->vertices[1]->y
                      );
                      
            lbl_error_3->text = "(" + ftos(floor(li_ptr->l1->vertices[0]->x + cos(a) * u * d)) +
                                "," + ftos(floor(li_ptr->l1->vertices[0]->y + sin(a) * u * d)) + ")!";
                                
        } else if(ed_error_type == EET_TEXTURE_ERROR) {
            //ToDo.
            
        } else if(ed_error_type == EET_BAD_SECTOR) {
            //ToDo.
            
        }
    }
    
    ((lafi_checkbox*) ed_gui->widgets["frm_review"]->widgets["chk_see_textures"])->set(ed_sec_mode == ESM_TEXTURE_VIEW);
}