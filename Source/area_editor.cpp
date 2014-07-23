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

#include <algorithm>
#include <iomanip>
#include <unordered_set>

#include <allegro5/allegro_primitives.h>

#include "area_editor.h"
#include "drawing.h"
#include "functions.h"
#include "LAFI/angle_picker.h"
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
    
    ((lafi_textbox*) f->widgets["txt_x"])->text =  f2s(ed_cur_sector->trans_x);
    ((lafi_textbox*) f->widgets["txt_y"])->text =  f2s(ed_cur_sector->trans_y);
    ((lafi_textbox*) f->widgets["txt_sx"])->text = f2s(ed_cur_sector->scale_x);
    ((lafi_textbox*) f->widgets["txt_sy"])->text = f2s(ed_cur_sector->scale_y);
    ((lafi_angle_picker*) f->widgets["ang_a"])->set_angle_rads(ed_cur_sector->rot);
}

/* ----------------------------------------------------------------------------
 * Loads the background's data from the memory to the gui.
 */
void area_editor::bg_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_bg"];
    ((lafi_textbox*) f->widgets["txt_file"])->text = ed_bg_file_name;
    ((lafi_textbox*) f->widgets["txt_x"])->text = f2s(ed_bg_x);
    ((lafi_textbox*) f->widgets["txt_y"])->text = f2s(ed_bg_y);
    ((lafi_textbox*) f->widgets["txt_w"])->text = f2s(ed_bg_w);
    ((lafi_textbox*) f->widgets["txt_h"])->text = f2s(ed_bg_h);
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
void area_editor::center_camera(float min_x, float min_y, float max_x, float max_y) {
    float width = max_x - min_x;
    float height = max_y - min_y;
    
    cam_x = -floor(min_x + width  / 2);
    cam_y = -floor(min_y + height / 2);
    
    if(width > height) cam_zoom = (scr_w - 208) / width;
    else cam_zoom = (scr_h - 16) / height;
    
    cam_zoom -= cam_zoom * 0.1;
    
    cam_zoom = max(cam_zoom, ZOOM_MIN_LEVEL_EDITOR);
    cam_zoom = min(cam_zoom, ZOOM_MAX_LEVEL_EDITOR);
    
}

/* ----------------------------------------------------------------------------
 * Changes the background image.
 */
void area_editor::change_background(string new_file_name) {
    if(ed_bg_bitmap && ed_bg_bitmap != bmp_error) al_destroy_bitmap(ed_bg_bitmap);
    ed_bg_bitmap = NULL;
    
    if(new_file_name.size()) {
        ed_bg_bitmap = load_bmp(new_file_name, false);
    }
    ed_bg_file_name = new_file_name;
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
    hide_widget(ed_gui->widgets["frm_objects"]);
    hide_widget(ed_gui->widgets["frm_shadows"]);
    hide_widget(ed_gui->widgets["frm_bg"]);
    hide_widget(ed_gui->widgets["frm_review"]);
    
    if(!hide_all) {
        if(ed_mode == EDITOR_MODE_MAIN) {
            show_widget(ed_gui->widgets["frm_main"]);
        } else if(ed_mode == EDITOR_MODE_SECTORS) {
            show_widget(ed_gui->widgets["frm_sectors"]);
        } else if(ed_mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS) {
            show_widget(ed_gui->widgets["frm_adv_textures"]);
        } else if(ed_mode == EDITOR_MODE_OBJECTS) {
            show_widget(ed_gui->widgets["frm_objects"]);
        } else if(ed_mode == EDITOR_MODE_SHADOWS) {
            show_widget(ed_gui->widgets["frm_shadows"]);
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
        float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
        float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
        float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
        float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
        
        if(cam_zoom >= ZOOM_MIN_LEVEL_EDITOR * 1.5) {
            float x = floor(cam_leftmost / GRID_INTERVAL) * GRID_INTERVAL;
            while(x < cam_rightmost + GRID_INTERVAL) {
                ALLEGRO_COLOR c = al_map_rgb(255, 255, 255);
                bool draw_line = true;
                
                if(fmod(x, GRID_INTERVAL * 2) == 0) {
                    c = al_map_rgb(0, 96, 160);
                } else {
                    if(cam_zoom > ZOOM_MIN_LEVEL_EDITOR * 4) {
                        c = al_map_rgb(0, 64, 128);
                    } else {
                        draw_line = false;
                    }
                }
                
                if(draw_line) al_draw_line(x, cam_topmost, x, cam_bottommost + GRID_INTERVAL, al_map_rgb(0, 64, 128), 1.0 / cam_zoom);
                x += GRID_INTERVAL;
            }
            
            float y = floor(cam_topmost / GRID_INTERVAL) * GRID_INTERVAL;
            while(y < cam_bottommost + GRID_INTERVAL) {
                ALLEGRO_COLOR c = al_map_rgb(255, 255, 255);
                bool draw_line = true;
                
                if(fmod(y, GRID_INTERVAL * 2) == 0) {
                    c = al_map_rgb(0, 96, 160);
                } else {
                    if(cam_zoom > ZOOM_MIN_LEVEL_EDITOR * 4) {
                        c = al_map_rgb(0, 64, 128);
                    } else {
                        draw_line = false;
                    }
                }
                
                if(draw_line) al_draw_line(cam_leftmost, y, cam_rightmost + GRID_INTERVAL, y, c, 1.0 / cam_zoom);
                y += GRID_INTERVAL;
            }
        }
        
        //0,0 marker.
        al_draw_line(-(GRID_INTERVAL * 2), 0, GRID_INTERVAL * 2, 0, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        al_draw_line(0, -(GRID_INTERVAL * 2), 0, GRID_INTERVAL * 2, al_map_rgb(128, 192, 255), 1.0 / cam_zoom);
        
        //Linedefs.
        if(ed_sec_mode != ESM_TEXTURE_VIEW) {
        
            unsigned char sector_opacity = 224;
            if(ed_mode == EDITOR_MODE_OBJECTS || ed_mode == EDITOR_MODE_SHADOWS) sector_opacity = 64;
            
            size_t n_linedefs = cur_area_map.linedefs.size();
            for(size_t l = 0; l < n_linedefs; l++) {
                linedef* l_ptr = cur_area_map.linedefs[l];
                
                if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
                
                bool one_sided = true;
                bool error_highlight = false;
                bool valid = true;
                bool mouse_on = false;
                bool selected = false;
                
                if(ed_error_sector_ptr) {
                    if(l_ptr->sectors[0] == ed_error_sector_ptr) error_highlight = true;
                    if(l_ptr->sectors[1] == ed_error_sector_ptr) error_highlight = true;
                    
                } else {
                    for(size_t il = 0; il < ed_intersecting_lines.size(); il++) {
                        if(ed_intersecting_lines[il].contains(l_ptr)) {
                            valid = false;
                            break;
                        }
                    }
                    
                    if(ed_non_simples.find(l_ptr->sectors[0]) != ed_non_simples.end()) valid = false;
                    if(ed_non_simples.find(l_ptr->sectors[1]) != ed_non_simples.end()) valid = false;
                    if(ed_lone_lines.find(l_ptr) != ed_lone_lines.end()) valid = false;
                }
                
                if(l_ptr->sectors[0] && l_ptr->sectors[1]) one_sided = false;
                
                if(ed_on_sector && ed_mode == EDITOR_MODE_SECTORS) {
                    if(l_ptr->sectors[0] == ed_on_sector) mouse_on = true;
                    if(l_ptr->sectors[1] == ed_on_sector) mouse_on = true;
                }
                
                if(ed_cur_sector && ed_mode == EDITOR_MODE_SECTORS) {
                    if(l_ptr->sectors[0] == ed_cur_sector) selected = true;
                    if(l_ptr->sectors[1] == ed_cur_sector) selected = true;
                }
                
                
                al_draw_line(
                    l_ptr->vertices[0]->x,
                    l_ptr->vertices[0]->y,
                    l_ptr->vertices[1]->x,
                    l_ptr->vertices[1]->y,
                    (selected ?        al_map_rgba(224, 224, 64,  sector_opacity) :
                     error_highlight ? al_map_rgba(192, 80,  0,   sector_opacity) :
                     !valid ?          al_map_rgba(192, 32,  32,  sector_opacity) :
                     one_sided ?       al_map_rgba(240, 240, 240, sector_opacity) :
                     al_map_rgba(160, 160, 160, sector_opacity)
                    ),
                    (mouse_on || selected ? 3.0 : 2.0) / cam_zoom
                );
                
                //Debug: uncomment this to show the sector numbers on each side.
                //Orientantion could be wrong, as there is no concept of front/back sector.
                /*float mid_x = (l_ptr->vertices[0]->x + l_ptr->vertices[1]->x) / 2;
                float mid_y = (l_ptr->vertices[0]->y + l_ptr->vertices[1]->y) / 2;
                float angle = atan2(l_ptr->vertices[0]->y - l_ptr->vertices[1]->y, l_ptr->vertices[0]->x - l_ptr->vertices[1]->x);
                al_draw_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle - M_PI_2) * 15,
                    mid_y + sin(angle - M_PI_2) * 15 - font_h / 2,
                    ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[0] == string::npos ? "--" : i2s(l_ptr->sector_nrs[0]).c_str());
                al_draw_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle + M_PI_2) * 15,
                    mid_y + sin(angle + M_PI_2) * 15 - font_h / 2,
                    ALLEGRO_ALIGN_CENTER, l_ptr->sector_nrs[1] == string::npos ? "--" : i2s(l_ptr->sector_nrs[1]).c_str());*/
            }
            
            //Vertices.
            size_t n_vertices = cur_area_map.vertices.size();
            for(size_t v = 0; v < n_vertices; v++) {
                vertex* v_ptr = cur_area_map.vertices[v];
                al_draw_filled_circle(
                    v_ptr->x,
                    v_ptr->y,
                    3.0 / cam_zoom,
                    al_map_rgba(224, 224, 224, sector_opacity)
                );
            }
            
        } else {
        
            //Draw textures.
            for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
                draw_sector(cur_area_map.sectors[s], 0, 0);
            }
        }
        
        //Mobs.
        unsigned char mob_opacity = 224;
        if(ed_mode == EDITOR_MODE_SECTORS || ed_mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS || ed_mode == EDITOR_MODE_SHADOWS) mob_opacity = 64;
        if(ed_sec_mode == ESM_TEXTURE_VIEW) mob_opacity = 0;
        
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            mob_gen* m_ptr = cur_area_map.mob_generators[m];
            bool valid = m_ptr->type != NULL;
            
            float radius = m_ptr->type ? m_ptr->type->size == 0 ? 16 : m_ptr->type->size * 0.5 : 16;
            
            al_draw_filled_circle(
                m_ptr->x, m_ptr->y,
                radius,
                (valid ? al_map_rgba(96, 224, 96, mob_opacity) : al_map_rgba(224, 96, 96, mob_opacity))
            );
            
            float lrw = cos(m_ptr->angle) * radius;
            float lrh = sin(m_ptr->angle) * radius;
            float lt = radius / 8.0;
            
            al_draw_line(
                m_ptr->x - lrw * 0.8, m_ptr->y - lrh * 0.8,
                m_ptr->x + lrw * 0.8, m_ptr->y + lrh * 0.8,
                al_map_rgba(0, 0, 0, mob_opacity), lt
            );
            
            float tx1 = m_ptr->x + lrw;
            float ty1 = m_ptr->y + lrh;
            float tx2 = tx1 + cos(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty2 = ty1 + sin(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float tx3 = tx1 + cos(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty3 = ty1 + sin(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
            
            al_draw_filled_triangle(
                tx1, ty1,
                tx2, ty2,
                tx3, ty3,
                al_map_rgba(0, 0, 0, mob_opacity)
            );
            
            if(m_ptr == ed_cur_mob && ed_mode == EDITOR_MODE_OBJECTS) {
                al_draw_circle(
                    m_ptr->x, m_ptr->y,
                    radius,
                    al_map_rgba(192, 192, 192, mob_opacity), 2 / cam_zoom
                );
            }
            
        }
        
        //Shadows.
        if(ed_mode == EDITOR_MODE_SHADOWS || (ed_sec_mode == ESM_TEXTURE_VIEW && ed_show_shadows)) {
            for(size_t s = 0; s < cur_area_map.tree_shadows.size(); s++) {
            
                tree_shadow* s_ptr = cur_area_map.tree_shadows[s];
                draw_sprite(
                    s_ptr->bitmap, s_ptr->x, s_ptr->y, s_ptr->w, s_ptr->h,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(ed_mode == EDITOR_MODE_SHADOWS) {
                    float min_x, min_y, max_x, max_y;
                    get_shadow_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
                    
                    al_draw_rectangle(
                        min_x, min_y, max_x, max_y,
                        (s_ptr == ed_cur_shadow ? al_map_rgb(224, 224, 64) : al_map_rgb(128, 128, 64)),
                        2 / cam_zoom
                    );
                }
            }
        }
        
        //New thing marker.
        if(ed_sec_mode == ESM_NEW_SECTOR || ed_sec_mode == ESM_NEW_OBJECT || ed_sec_mode == ESM_NEW_SHADOW) {
            float x = snap_to_grid(mouse_cursor_x);
            float y = snap_to_grid(mouse_cursor_y);
            al_draw_line(x - 16, y,      x + 16, y,      al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(x,      y - 16, x,      y + 16, al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
        }
        
        //Lightly glow the sector under the mouse.
        if(ed_mode == EDITOR_MODE_SECTORS) {
            if(ed_on_sector && ed_moving_thing == string::npos) {
                for(size_t t = 0; t < ed_on_sector->triangles.size(); t++) {
                    triangle* t_ptr = &ed_on_sector->triangles[t];
                    //Uncomment this to show the triangles.
                    /*al_draw_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        al_map_rgb(192, 0, 0),
                        1.0 / cam_zoom
                    );*/
                    al_draw_filled_triangle(
                        t_ptr->points[0]->x,
                        t_ptr->points[0]->y,
                        t_ptr->points[1]->x,
                        t_ptr->points[1]->y,
                        t_ptr->points[2]->x,
                        t_ptr->points[2]->y,
                        map_alpha(12)
                    );
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
        
        
        //Background.
        if(ed_bg_bitmap && ed_show_bg) {
            al_draw_tinted_scaled_bitmap(
                ed_bg_bitmap,
                map_alpha(ed_bg_a),
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
    ed_error_sector_ptr = NULL;
    ed_error_vertex_ptr = NULL;
    ed_error_string.clear();
    
    //Check intersecting lines.
    if(ed_intersecting_lines.size() > 0) {
        ed_error_type = EET_INTERSECTING_LINEDEFS;
    }
    
    //Check overlapping vertices.
    if(ed_error_type == EET_NONE) {
        ed_error_vertex_ptr = NULL;
        
        for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
            vertex* v1_ptr = cur_area_map.vertices[v];
            if(v1_ptr->x == FLT_MAX) continue;
            
            for(size_t v2 = v + 1; v2 < cur_area_map.vertices.size(); v2++) {
                vertex* v2_ptr = cur_area_map.vertices[v2];
                
                if(v1_ptr->x == v2_ptr->x && v1_ptr->y == v2_ptr->y) {
                    ed_error_type = EET_OVERLAPPING_VERTICES;
                    ed_error_vertex_ptr = v1_ptr;
                    break;
                }
            }
            if(ed_error_vertex_ptr) break;
        }
    }
    
    //Check non-simple sectors.
    if(ed_error_type == EET_NONE) {
        if(ed_non_simples.size() > 0) {
            ed_error_type = EET_BAD_SECTOR;
        }
    }
    
    //Check lone linedefs.
    if(ed_error_type == EET_NONE) {
        if(ed_lone_lines.size() > 0) {
            ed_error_type = EET_LONE_LINE;
        }
    }
    
    //Check for missing textures.
    if(ed_error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        
            sector* s_ptr = cur_area_map.sectors[s];
            if(s_ptr->file_name.size() == 0 && s_ptr->type != SECTOR_TYPE_BOTTOMLESS_PIT && !s_ptr->fade) {
                ed_error_type = EET_MISSING_TEXTURE;
                ed_error_sector_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Check for unknown textures.
    if(ed_error_type == EET_NONE) {
        vector<string> texture_file_names = folder_to_vector(TEXTURES_FOLDER, false);
        for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        
            sector* s_ptr = cur_area_map.sectors[s];
            
            if(s_ptr->file_name.size() == 0) continue;
            
            if(find(texture_file_names.begin(), texture_file_names.end(), s_ptr->file_name) == texture_file_names.end()) {
                ed_error_type = EET_UNKNOWN_TEXTURE;
                ed_error_string = s_ptr->file_name;
                ed_error_sector_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Objects with no type.
    if(ed_error_type == EET_NONE) {
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            if(!cur_area_map.mob_generators[m]->type) {
                ed_error_type = EET_TYPELESS_MOB;
                ed_error_mob_ptr = cur_area_map.mob_generators[m];
                break;
            }
        }
    }
    
    //Objects out of bounds.
    if(ed_error_type == EET_NONE) {
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            mob_gen* m_ptr = cur_area_map.mob_generators[m];
            if(!get_sector(m_ptr->x, m_ptr->y, NULL)) {
                ed_error_type = EET_MOB_OOB;
                ed_error_mob_ptr = m_ptr;
                break;
            }
        }
    }
    
    //Check if there are no landing site sectors.
    if(ed_error_type == EET_NONE) {
        bool landing_site_missing = true;
        for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
            sector* s_ptr = cur_area_map.sectors[s];
            if(s_ptr->linedefs.size() == 0) continue;
            
            if(s_ptr->type == SECTOR_TYPE_LANDING_SITE) {
                landing_site_missing = false;
                break;
            }
        }
        
        if(landing_site_missing) ed_error_type = EET_LANDING_SITE;
    }
    
    //Check if there are tree shadows with invalid images.
    if(ed_error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_map.tree_shadows.size(); s++) {
            if(cur_area_map.tree_shadows[s]->bitmap == bmp_error) {
                ed_error_type = EET_INVALID_SHADOW;
                ed_error_shadow_ptr = cur_area_map.tree_shadows[s];
            }
        }
    }
    
    
    update_review_frame();
}

/* ----------------------------------------------------------------------------
 * Focuses the camera on the error found, if any.
 */
void area_editor::goto_error() {
    if(ed_error_type == EET_NONE || ed_error_type == EET_NONE_YET) return;
    
    if(ed_error_type == EET_INTERSECTING_LINEDEFS) {
    
        if(ed_intersecting_lines.size() == 0) {
            find_errors(); return;
        }
        
        linedef_intersection* li_ptr = &ed_intersecting_lines[0];
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
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(ed_error_type == EET_BAD_SECTOR) {
    
        if(ed_non_simples.size() == 0) {
            find_errors(); return;
        }
        
        sector* s_ptr = *ed_non_simples.begin();
        float min_x, min_y, max_x, max_y;
        get_sector_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(ed_error_type == EET_LONE_LINE) {
    
        if(ed_lone_lines.size() == 0) {
            find_errors(); return;
        }
        
        linedef* l_ptr = *ed_lone_lines.begin();
        float min_x, min_y, max_x, max_y;
        min_x = l_ptr->vertices[0]->x;
        max_x = min_x;
        min_y = l_ptr->vertices[0]->y;
        max_y = min_y;
        
        min_x = min(min_x, l_ptr->vertices[0]->x);
        min_x = min(min_x, l_ptr->vertices[1]->x);
        max_x = max(max_x, l_ptr->vertices[0]->x);
        max_x = max(max_x, l_ptr->vertices[1]->x);
        min_y = min(min_y, l_ptr->vertices[0]->y);
        min_y = min(min_y, l_ptr->vertices[1]->y);
        max_y = max(max_y, l_ptr->vertices[0]->y);
        max_y = max(max_y, l_ptr->vertices[1]->y);
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(ed_error_type == EET_OVERLAPPING_VERTICES) {
    
        if(!ed_error_vertex_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            ed_error_vertex_ptr->x - 64,
            ed_error_vertex_ptr->y - 64,
            ed_error_vertex_ptr->x + 64,
            ed_error_vertex_ptr->y + 64
        );
        
    } else if(ed_error_type == EET_MISSING_TEXTURE || ed_error_type == EET_UNKNOWN_TEXTURE) {
    
        if(!ed_error_sector_ptr) {
            find_errors(); return;
        }
        
        float min_x, min_y, max_x, max_y;
        get_sector_bounding_box(ed_error_sector_ptr, &min_x, &min_y, &max_x, &max_y);
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(ed_error_type == EET_TYPELESS_MOB || ed_error_type == EET_MOB_OOB) {
    
        if(!ed_error_mob_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            ed_error_mob_ptr->x - 64,
            ed_error_mob_ptr->y - 64,
            ed_error_mob_ptr->x + 64,
            ed_error_mob_ptr->y + 64
        );
        
    } else if(ed_error_type == EET_LANDING_SITE) {
        //Nothing to focus on.
        return;
        
    } else if(ed_error_type == EET_INVALID_SHADOW) {
    
        float min_x, min_y, max_x, max_y;
        get_shadow_bounding_box(ed_error_shadow_ptr, &min_x, &min_y, &max_x, &max_y);
        center_camera(min_x, min_y, max_x, max_y);
    }
}

/* ----------------------------------------------------------------------------
 * Saves the advanced texture settings from the gui.
 */
void area_editor::gui_to_adv_textures() {
    if(!ed_cur_sector) return;
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_adv_textures"];
    
    ed_cur_sector->trans_x = s2f(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_cur_sector->trans_y = s2f(((lafi_textbox*) f->widgets["txt_y"])->text);
    ed_cur_sector->scale_x = s2f(((lafi_textbox*) f->widgets["txt_sx"])->text);
    ed_cur_sector->scale_y = s2f(((lafi_textbox*) f->widgets["txt_sy"])->text);
    ed_cur_sector->rot = ((lafi_angle_picker*) f->widgets["ang_a"])->get_angle_rads();
    
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
        change_background(new_file_name);
        is_file_new = true;
        if(ed_bg_bitmap) {
            ed_bg_w = al_get_bitmap_width(ed_bg_bitmap);
            ed_bg_h = al_get_bitmap_height(ed_bg_bitmap);
        } else {
            ed_bg_w = 0;
            ed_bg_h = 0;
        }
    }
    
    ed_bg_x = s2f(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_bg_y = s2f(((lafi_textbox*) f->widgets["txt_y"])->text);
    
    ed_bg_aspect_ratio = ((lafi_checkbox*) f->widgets["chk_ratio"])->checked;
    float new_w = s2f(((lafi_textbox*) f->widgets["txt_w"])->text);
    float new_h = s2f(((lafi_textbox*) f->widgets["txt_h"])->text);
    
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
 * Saves a mob's data using info on the gui.
 */
void area_editor::gui_to_mob() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_objects"]->widgets["frm_object"];
    
    if(!ed_cur_mob) return;
    
    ed_cur_mob->angle = ((lafi_angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
    ed_cur_mob->vars = ((lafi_textbox*) f->widgets["txt_vars"])->text;
}

/* ----------------------------------------------------------------------------
 * Saves the current tree shadow using the info on the gui.
 */
void area_editor::gui_to_shadow() {
    if(!ed_cur_shadow) return;
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_shadows"]->widgets["frm_shadow"];
    
    ed_cur_shadow->x = s2f(((lafi_textbox*) f->widgets["txt_x"])->text);
    ed_cur_shadow->y = s2f(((lafi_textbox*) f->widgets["txt_y"])->text);
    ed_cur_shadow->w = s2f(((lafi_textbox*) f->widgets["txt_w"])->text);
    ed_cur_shadow->h = s2f(((lafi_textbox*) f->widgets["txt_h"])->text);
    ed_cur_shadow->angle = ((lafi_angle_picker*) f->widgets["ang_an"])->get_angle_rads();
    ed_cur_shadow->alpha = ((lafi_scrollbar*) f->widgets["bar_al"])->low_value;
    ed_cur_shadow->sway_x = s2f(((lafi_textbox*) f->widgets["txt_sx"])->text);
    ed_cur_shadow->sway_y = s2f(((lafi_textbox*) f->widgets["txt_sy"])->text);
    
    string new_file_name = ((lafi_textbox*) f->widgets["txt_file"])->text;
    
    if(new_file_name != ed_cur_shadow->file_name) {
        //New image, delete the old one.
        if(ed_cur_shadow->bitmap != bmp_error) {
            bitmaps.detach(ed_cur_shadow->file_name);
        }
        ed_cur_shadow->bitmap = bitmaps.get("Textures/" + new_file_name, NULL);
        ed_cur_shadow->file_name = new_file_name;
    }
}

/* ----------------------------------------------------------------------------
 * Saves the sector using the info on the gui.
 */
void area_editor::gui_to_sector() {
    if(!ed_cur_sector) return;
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_sectors"]->widgets["frm_sector"];
    
    ed_cur_sector->z = s2f(((lafi_textbox*) f->widgets["txt_z"])->text);
    ed_cur_sector->fade = ((lafi_checkbox*) f->widgets["chk_fade"])->checked;
    ed_cur_sector->file_name = ((lafi_textbox*) f->widgets["txt_texture"])->text;
    ed_cur_sector->brightness = s2i(((lafi_textbox*) f->widgets["txt_brightness"])->text);
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
        ((lafi_label*) ed_gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")");
    }
    
    
    //Moving vertices, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        if(
            ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16
            && ed_moving_thing == string::npos && ed_sec_mode != ESM_TEXTURE_VIEW &&
            ed_mode != EDITOR_MODE_OBJECTS
        ) {
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
            
        } else if(ed_holding_m2) {
            //Move camera.
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        //Move vertex, mob or shadow.
        if(ed_moving_thing != string::npos) {
            if(ed_mode == EDITOR_MODE_SECTORS) {
                vertex* v_ptr = cur_area_map.vertices[ed_moving_thing];
                v_ptr->x = snap_to_grid(mouse_cursor_x);
                v_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(ed_mode == EDITOR_MODE_OBJECTS) {
                mob_gen* m_ptr = cur_area_map.mob_generators[ed_moving_thing];
                m_ptr->x = snap_to_grid(mouse_cursor_x);
                m_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(ed_mode == EDITOR_MODE_SHADOWS) {
                tree_shadow* s_ptr = cur_area_map.tree_shadows[ed_moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x - ed_moving_thing_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y - ed_moving_thing_y);
                shadow_to_gui();
            }
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
        
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16) {
        //Clicking.
        
        if(ev.mouse.button == 1) ed_holding_m1 = true;
        else if(ev.mouse.button == 2) ed_holding_m2 = true;
        else if(ev.mouse.button == 3) cam_zoom = 1.0;
        
        if(ev.mouse.button != 1) return;
        if(ev.mouse.x > scr_w - 208) return;
        
        //If the user was editing, save it.
        if(ed_mode == EDITOR_MODE_SECTORS) {
            gui_to_sector();
        } else if(ed_mode == EDITOR_MODE_OBJECTS) {
            gui_to_mob();
        } else if(ed_mode == EDITOR_MODE_SHADOWS) {
            gui_to_shadow();
        }
        
        //Sector-related clicking.
        if(ed_sec_mode == ESM_NONE && ed_mode == EDITOR_MODE_SECTORS) {
        
            ed_moving_thing = string::npos;
            
            linedef* clicked_linedef_ptr = NULL;
            size_t clicked_linedef_nr = string::npos;
            bool created_vertex = false;
            
            for(size_t l = 0; l < cur_area_map.linedefs.size(); l++) {
                linedef* l_ptr = cur_area_map.linedefs[l];
                
                if(!l_ptr->vertices[0] || !l_ptr->vertices[1]) continue;
                
                if(
                    circle_intersects_line(
                        mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                        l_ptr->vertices[0]->x, l_ptr->vertices[0]->y,
                        l_ptr->vertices[1]->x, l_ptr->vertices[1]->y
                    )
                ) {
                    clicked_linedef_ptr = l_ptr;
                    clicked_linedef_nr = l;
                    break;
                }
            }
            
            if(ed_double_click_time == 0) ed_double_click_time = 0.5;
            else if(clicked_linedef_ptr) {
                //Create a new vertex.
                ed_double_click_time = 0;
                
                //New vertex, on the split point.
                //ToDo create it on the line, not on the cursor.
                vertex* new_v_ptr = new vertex(mouse_cursor_x, mouse_cursor_y);
                cur_area_map.vertices.push_back(new_v_ptr);
                
                //New linedef, copied from the original one.
                linedef* new_l_ptr = new linedef(*clicked_linedef_ptr);
                cur_area_map.linedefs.push_back(new_l_ptr);
                
                //Save the original end vertex for later.
                vertex* end_v_ptr = clicked_linedef_ptr->vertices[1];
                
                //Set vertices on the new and original linedefs.
                new_l_ptr->vertex_nrs[0] = cur_area_map.vertices.size() - 1;
                new_l_ptr->vertices[0] = new_v_ptr;
                clicked_linedef_ptr->vertex_nrs[1] = new_l_ptr->vertex_nrs[0];
                clicked_linedef_ptr->vertices[1] = new_v_ptr;
                
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
                new_v_ptr->linedef_nrs.push_back(clicked_linedef_nr);
                new_v_ptr->linedefs.push_back(new_l_ptr);
                new_v_ptr->linedefs.push_back(clicked_linedef_ptr);
                
                //Update linedef data on the end vertex of the original line
                //(it now links to the new line, not the old).
                for(size_t vl = 0; vl < end_v_ptr->linedefs.size(); vl++) {
                    if(end_v_ptr->linedefs[vl] == clicked_linedef_ptr) {
                        end_v_ptr->linedefs[vl] = new_l_ptr;
                        end_v_ptr->linedef_nrs[vl] = cur_area_map.linedefs.size() - 1;
                        break;
                    }
                }
                
                //Start dragging the new vertex.
                ed_moving_thing = cur_area_map.vertices.size() - 1;
                
                created_vertex = true;
            }
            
            //Find a vertex to drag.
            if(!created_vertex) {
                for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                    if(
                        check_dist(
                            mouse_cursor_x, mouse_cursor_y,
                            cur_area_map.vertices[v]->x, cur_area_map.vertices[v]->y,
                            6.0 / cam_zoom
                        )
                    ) {
                        ed_moving_thing = v;
                        break;
                    }
                }
            }
            
            //Find a sector to select.
            if(ed_moving_thing == string::npos && !clicked_linedef_ptr) {
                ed_cur_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL);
                sector_to_gui();
            }
            
            
        } else if(ed_sec_mode == ESM_NONE && ed_mode == EDITOR_MODE_OBJECTS) {
            //Object-related clicking.
            
            ed_cur_mob = NULL;
            ed_moving_thing = string::npos;
            for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
                mob_gen* m_ptr = cur_area_map.mob_generators[m];
                float radius = m_ptr->type ? m_ptr->type->size == 0 ? 16 : m_ptr->type->size * 0.5 : 16;
                if(check_dist(m_ptr->x, m_ptr->y, mouse_cursor_x, mouse_cursor_y, radius)) {
                
                    ed_cur_mob = m_ptr;
                    ed_moving_thing = m;
                    break;
                }
            }
            mob_to_gui();
            
        } else if(ed_sec_mode == ESM_NONE && ed_mode == EDITOR_MODE_SHADOWS) {
            //Shadow-related clicking.
            
            ed_cur_shadow = NULL;
            ed_moving_thing = string::npos;
            for(size_t s = 0; s < cur_area_map.tree_shadows.size(); s++) {
            
                tree_shadow* s_ptr = cur_area_map.tree_shadows[s];
                float min_x, min_y, max_x, max_y;
                get_shadow_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
                
                if(
                    mouse_cursor_x >= min_x && mouse_cursor_x <= max_x &&
                    mouse_cursor_y >= min_y && mouse_cursor_y <= max_y
                ) {
                    ed_cur_shadow = s_ptr;
                    ed_moving_thing = s;
                    ed_moving_thing_x = mouse_cursor_x - s_ptr->x;
                    ed_moving_thing_y = mouse_cursor_y - s_ptr->y;
                    break;
                }
            }
            shadow_to_gui();
            
        }
        
        if(ed_sec_mode == ESM_NEW_SECTOR) {
            //Place a new sector where the cursor is.
            
            ed_sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            size_t outer_sector_nr;
            sector* outer_sector = get_sector(hotspot_x, hotspot_y, &outer_sector_nr);
            
            sector* new_sector = new sector();
            if(outer_sector) outer_sector->clone(new_sector);
            
            //Create the vertices.
            vertex* new_vertices[4];
            for(size_t v = 0; v < 4; v++) new_vertices[v] = new vertex(0, 0);
            new_vertices[0]->x = hotspot_x - GRID_INTERVAL / 2;
            new_vertices[0]->y = hotspot_y - GRID_INTERVAL / 2;
            new_vertices[1]->x = hotspot_x + GRID_INTERVAL / 2;
            new_vertices[1]->y = hotspot_y - GRID_INTERVAL / 2;
            new_vertices[2]->x = hotspot_x + GRID_INTERVAL / 2;
            new_vertices[2]->y = hotspot_y + GRID_INTERVAL / 2;
            new_vertices[3]->x = hotspot_x - GRID_INTERVAL / 2;
            new_vertices[3]->y = hotspot_y + GRID_INTERVAL / 2;
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
            
            ed_cur_sector = new_sector;
            sector_to_gui();
            
            
        } else if(ed_sec_mode == ESM_NEW_OBJECT) {
            //Create a mob where the cursor is.
            
            ed_sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            cur_area_map.mob_generators.push_back(
                new mob_gen(hotspot_x, hotspot_y)
            );
            
            ed_cur_mob = cur_area_map.mob_generators.back();
            mob_to_gui();
            
        } else if(ed_sec_mode == ESM_NEW_SHADOW) {
            //Create a new shadow where the cursor is.
            
            ed_sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            tree_shadow* new_shadow = new tree_shadow(hotspot_x, hotspot_y);
            new_shadow->bitmap = bmp_error;
            
            cur_area_map.tree_shadows.push_back(new_shadow);
            ed_cur_shadow = new_shadow;
            shadow_to_gui();
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        //Mouse button release.
        
        if(ev.mouse.button == 1) ed_holding_m1 = false;
        else if(ev.mouse.button == 2) ed_holding_m2 = false;
        
        if(ev.mouse.button == 1 && ed_sec_mode == ESM_NONE && ed_moving_thing != string::npos) {
            //Release the vertex.
            
            vertex* moved_v_ptr = cur_area_map.vertices[ed_moving_thing];
            vertex* final_vertex = moved_v_ptr;
            
            unordered_set<sector*> affected_sectors;
            
            //Check if the line's vertices intersect with any other lines.
            //If so, they're marked with red.
            check_linedef_intersections(moved_v_ptr);
            
            
            //Check if we should merge.
            for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
                vertex* dest_v_ptr = cur_area_map.vertices[v];
                if(dest_v_ptr == moved_v_ptr) continue;
                
                if(check_dist(moved_v_ptr->x, moved_v_ptr->y, dest_v_ptr->x, dest_v_ptr->y, 10 / cam_zoom)) {
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
                            
                            //Clear it from the list of lone lines, if there.
                            auto it = ed_lone_lines.find(l_ptr);
                            if(it != ed_lone_lines.end()) ed_lone_lines.erase(it);
                            
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
                                    l_ptr->remove_from_vertices();
                                    
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
                                dest_v_ptr->linedefs.push_back(moved_v_ptr->linedefs[l]);
                                unsigned char n = (l_ptr->vertices[0] == moved_v_ptr ? 0 : 1);
                                l_ptr->vertices[n] = dest_v_ptr;
                                l_ptr->vertex_nrs[n] = v;
                            }
                        }
                        
                        if(!was_deleted) l++;
                        
                    }
                    
                    dest_v_ptr->fix_pointers(cur_area_map);
                    
                    //Check if any of the final linedefs have the same sector
                    //on both sides. If so, delete them.
                    for(size_t vl = 0; vl < dest_v_ptr->linedefs.size(); ) {
                        linedef* vl_ptr = dest_v_ptr->linedefs[vl];
                        if(vl_ptr->sectors[0] == vl_ptr->sectors[1]) {
                            vl_ptr->remove_from_sectors();
                            vl_ptr->remove_from_vertices();
                            for(size_t v = 0; v < 2; v++) {
                                if(vl_ptr->vertices[v]->linedefs.size() == 0) {
                                    vl_ptr->vertices[v]->x = vl_ptr->vertices[v]->y = FLT_MAX;
                                }
                            }
                            vl_ptr->sector_nrs[0] = vl_ptr->sector_nrs[1] = string::npos;
                            vl_ptr->vertex_nrs[0] = vl_ptr->vertex_nrs[1] = string::npos;
                            vl_ptr->fix_pointers(cur_area_map);
                        } else {
                            vl++;
                        }
                    }
                    
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
            
            //If somewhere along the line, the current sector
            //got marked for deletion, unselect it.
            if(ed_cur_sector) {
                if(ed_cur_sector->linedefs.size() == 0) {
                    ed_cur_sector = NULL;
                    sector_to_gui();
                }
            }
            
            ed_moving_thing = string::npos;
            
            
            
        } else if(ev.mouse.button == 1 && ed_mode == EDITOR_MODE_OBJECTS && ed_moving_thing != string::npos) {
            //Release object.
            
            ed_moving_thing = string::npos;
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key press.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            ed_shift_pressed = true;
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //Key release.
        
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

    load_mob_types(false);
    
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
    hide_widget(frm_area);
    frm_area->easy_row();
    frm_area->easy_add("but_sectors", new lafi_button(0, 0, 0, 0, "Edit sectors"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_objects", new lafi_button(0, 0, 0, 0, "Edit objects"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_shadows", new lafi_button(0, 0, 0, 0, "Edit shadows"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_bg", new lafi_button(0, 0, 0, 0, "Edit background"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_review", new lafi_button(0, 0, 0, 0, "Review"), 100, 32);
    frm_area->easy_row();
    
    
    //Bottom bar.
    lafi_frame* frm_bottom = new lafi_frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    ed_gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_bg", new lafi_button(  0, 0, 0, 0, "Bg"), 25, 32);
    frm_bottom->easy_add("but_load", new lafi_button(0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save", new lafi_button(0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit", new lafi_button(0, 0, 0, 0, "X"), 25, 32);
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
    frm_sectors->easy_add("but_new", new lafi_button(0, 0, 0, 0, "+"), 20, 32);
    frm_sectors->easy_add("but_sel_none", new lafi_button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_sectors->easy_row();
    
    lafi_frame* frm_sector = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_sectors->add("frm_sector", frm_sector);
    hide_widget(frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_type", new lafi_label(0, 0, 0, 0, "Type:"), 30, 24);
    frm_sector->easy_add("but_type", new lafi_button(0, 0, 0, 0), 70, 24);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_z", new lafi_label(0, 0, 0, 0, "Height:"), 50, 16);
    frm_sector->easy_add("txt_z", new lafi_textbox(0, 0, 0, 0), 50, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("chk_fade", new lafi_checkbox(0, 0, 0, 0, "Fade textures"), 100, 16);
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
    frm_adv_textures->easy_add("ang_a", new lafi_angle_picker(0, 0, 0, 0), 50, 24);
    frm_adv_textures->easy_row();
    
    
    //Objects frame.
    lafi_frame* frm_objects = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_objects);
    ed_gui->add("frm_objects", frm_objects);
    
    frm_objects->easy_row();
    frm_objects->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_objects->easy_row();
    frm_objects->easy_add("but_new", new lafi_button(0, 0, 0, 0, "+"), 20, 32);
    frm_objects->easy_add("but_sel_none", new lafi_button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_objects->easy_row();
    
    lafi_frame* frm_object = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_objects->add("frm_object", frm_object);
    hide_widget(frm_object);
    
    frm_object->easy_row();
    frm_object->easy_add("lbl_category", new lafi_label(0, 0, 0, 0, "Category:"), 90, 16);
    frm_object->easy_add("but_rem", new lafi_button(0, 0, 0, 0, "-"), 10, 16);
    frm_object->easy_row();
    frm_object->easy_add("but_category", new lafi_button(0, 0, 0, 0), 100, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_type", new lafi_label(0, 0, 0, 0, "Type:"), 100, 16);
    frm_object->easy_row();
    frm_object->easy_add("but_type", new lafi_button(0, 0, 0, 0), 100, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_angle", new lafi_label(0, 0, 0, 0, "Angle:"), 50, 16);
    frm_object->easy_add("ang_angle", new lafi_angle_picker(0, 0, 0, 0), 50, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_vars", new lafi_label(0, 0, 0, 0, "Script variables:"), 100, 16);
    frm_object->easy_row();
    frm_object->easy_add("txt_vars", new lafi_textbox(0, 0, 0, 0), 100, 16);
    frm_object->easy_row();
    
    
    //Shadows frame.
    lafi_frame* frm_shadows = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_shadows);
    ed_gui->add("frm_shadows", frm_shadows);
    
    frm_shadows->easy_row();
    frm_shadows->easy_add("but_back", new lafi_button(0, 0, 0, 0, "Back"), 50, 16);
    frm_shadows->easy_row();
    frm_shadows->easy_add("but_new", new lafi_button(0, 0, 0, 0, "+"), 20, 32);
    frm_shadows->easy_add("but_sel_none", new lafi_button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_shadows->easy_row();
    
    lafi_frame* frm_shadow = new lafi_frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_shadows->add("frm_shadow", frm_shadow);
    hide_widget(frm_shadow);
    
    frm_shadow->easy_row();
    frm_shadow->easy_add("dum_1", new lafi_dummy(0, 0, 0, 0), 90, 16);
    frm_shadow->easy_add("but_rem", new lafi_button(0, 0, 0, 0, "-"), 10, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_file", new lafi_label(0, 0, 0, 0, "File:"), 20, 16);
    frm_shadow->easy_add("txt_file", new lafi_textbox(0, 0, 0, 0), 80, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_xy", new lafi_label(0, 0, 0, 0, "X&Y:"), 40, 16);
    frm_shadow->easy_add("txt_x",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_y",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_wh", new lafi_label(0, 0, 0, 0, "W&H:"), 40, 16);
    frm_shadow->easy_add("txt_w",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_h",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_an", new lafi_label(0, 0, 0, 0, "Angle:"), 40, 16);
    frm_shadow->easy_add("ang_an", new lafi_angle_picker(0, 0, 0, 0), 60, 24);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_al", new lafi_label(0, 0, 0, 0, "Opacity:"), 40, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("bar_al", new lafi_scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_sway", new lafi_label(0, 0, 0, 0, "Sway X&Y:"), 40, 16);
    frm_shadow->easy_add("txt_sx",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_sy",  new lafi_textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    
    
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
    frm_bg->easy_add("lbl_alpha", new lafi_label(0, 0, 0, 0, "Opacity:"), 100, 16);
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
    frm_review->easy_add("dum_1", new lafi_dummy(0, 0, 0, 0), 10, 16);
    frm_review->easy_add("chk_shadows", new lafi_checkbox(0, 0, 0, 0, "Enable shadows"), 90, 16);
    frm_review->easy_row();
    update_review_frame();
    
    
    //Status bar.
    lafi_label* ed_gui_status_bar = new lafi_label(0, scr_h - 16, scr_w - 208, scr_h);
    ed_gui->add("lbl_status_bar", ed_gui_status_bar);
    
    
    //Properties -- main.
    frm_main->widgets["but_area"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_AREA);
    };
    frm_area->widgets["but_sectors"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_area->widgets["but_objects"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_OBJECTS;
        change_to_right_frame();
    };
    frm_area->widgets["but_shadows"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_SHADOWS;
        change_to_right_frame();
    };
    frm_area->widgets["but_bg"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_BG;
        change_to_right_frame();
    };
    frm_area->widgets["but_review"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_REVIEW;
        change_to_right_frame();
        update_review_frame();
    };
    frm_main->widgets["but_area"]->description =    "Pick the area to edit.";
    frm_area->widgets["but_sectors"]->description = "Change sector (polygon) settings.";
    frm_area->widgets["but_objects"]->description = "Change object settings and placements.";
    frm_area->widgets["but_shadows"]->description = "Change the shadows of trees and leaves.";
    frm_area->widgets["but_bg"]->description =      "Add a background to guide you, like a blueprint.";
    frm_area->widgets["but_review"]->description =  "Tools to make sure everything is fine in the area.";
    
    
    //Properties -- bottom.
    frm_bottom->widgets["but_bg"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_show_bg = !ed_show_bg;
    };
    frm_bottom->widgets["but_load"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        load_area();
    };
    frm_bottom->widgets["but_save"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        save_area();
    };
    disable_widget(frm_bottom->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
    frm_bottom->widgets["but_bg"]->description =   "Toggle the visibility of the background.";
    frm_bottom->widgets["but_load"]->description = "Load the area from the files.";
    frm_bottom->widgets["but_save"]->description = "Save the area onto the disk.";
    frm_bottom->widgets["but_quit"]->description = "Quit the area editor.";
    
    //ToDo quit button.
    
    
    //Properties -- sectors.
    auto lambda_gui_to_sector = [] (lafi_widget*) { gui_to_sector(); };
    auto lambda_gui_to_sector_click = [] (lafi_widget*, int, int) { gui_to_sector(); };
    frm_sectors->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_sectors->widgets["but_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        if(ed_sec_mode == ESM_NEW_SECTOR) ed_sec_mode = ESM_NONE;
        else ed_sec_mode = ESM_NEW_SECTOR;
    };
    frm_sectors->widgets["but_sel_none"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_cur_sector = NULL;
        sector_to_gui();
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
    frm_sector->widgets["chk_fade"]->left_mouse_click_handler = lambda_gui_to_sector_click;
    frm_sector->widgets["txt_texture"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_brightness"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sectors->widgets["but_back"]->description =      "Go back to the main menu.";
    frm_sectors->widgets["but_new"]->description =       "Create a new sector where you click.";
    frm_sectors->widgets["but_sel_none"]->description =  "Deselect the current sector.";
    frm_sector->widgets["but_type"]->description =       "Change the type of sector.";
    frm_sector->widgets["chk_fade"]->description =       "Makes the surrounding textures fade into each other.";
    frm_sector->widgets["txt_z"]->description =          "Height of the floor.";
    frm_sector->widgets["txt_texture"]->description =    "File name of the Texture (image) of the floor.";
    frm_sector->widgets["txt_brightness"]->description = "0 = pitch black sector. 255 = normal lighting.";
    frm_sector->widgets["txt_hazards"]->description =    "Hazards the sector has.";
    frm_sector->widgets["but_adv"]->description =        "Advanced settings for the sector's texture.";
    
    
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
    frm_adv_textures->widgets["txt_x"]->description = "Scroll the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_y"]->description = "Scroll the texture vertically by this much.";
    frm_adv_textures->widgets["txt_sx"]->description = "Zoom the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_sy"]->description = "Zoom the texture vertically by this much.";
    frm_adv_textures->widgets["ang_a"]->description = "Rotate the texture by this much.";
    
    
    //Properties -- objects.
    auto lambda_gui_to_mob = [] (lafi_widget*) { gui_to_mob(); };
    frm_objects->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_objects->widgets["but_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        if(ed_sec_mode == ESM_NEW_OBJECT) ed_sec_mode = ESM_NONE;
        else ed_sec_mode = ESM_NEW_OBJECT;
    };
    frm_objects->widgets["but_sel_none"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_cur_mob = NULL;
        mob_to_gui();
    };
    frm_object->widgets["but_rem"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            if(cur_area_map.mob_generators[m] == ed_cur_mob) {
                cur_area_map.mob_generators.erase(cur_area_map.mob_generators.begin() + m);
                delete ed_cur_mob;
                ed_cur_mob = NULL;
                mob_to_gui();
                break;
            }
        }
    };
    frm_object->widgets["but_category"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_CATEGORY);
    };
    frm_object->widgets["but_type"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_TYPE);
    };
    frm_object->widgets["ang_angle"]->lose_focus_handler = lambda_gui_to_mob;
    frm_object->widgets["txt_vars"]->lose_focus_handler = lambda_gui_to_mob;
    frm_objects->widgets["but_back"]->description =     "Go back to the main menu.";
    frm_objects->widgets["but_new"]->description =      "Create a new object wherever you click.";
    frm_objects->widgets["but_sel_none"]->description = "Deselect the current sector.";
    frm_object->widgets["but_rem"]->description =       "Delete the current object.";
    frm_object->widgets["but_category"]->description =  "Choose the category of types of object.";
    frm_object->widgets["but_type"]->description =      "Choose the type this object is.";
    frm_object->widgets["ang_angle"]->description =     "Angle the object is facing.";
    frm_object->widgets["txt_vars"]->description =      "Extra variables (e.g.: sleep=y;jumping=n).";
    
    
    //Properties -- shadows.
    auto lambda_gui_to_shadow = [] (lafi_widget*) { gui_to_shadow(); };
    frm_shadows->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_sec_mode = ESM_NONE;
        shadow_to_gui();
        ed_mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_shadows->widgets["but_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        if(ed_sec_mode == ESM_NEW_SHADOW) ed_sec_mode = ESM_NONE;
        else ed_sec_mode = ESM_NEW_SHADOW;
    };
    frm_shadows->widgets["but_sel_none"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_cur_shadow = NULL;
        shadow_to_gui();
    };
    frm_shadow->widgets["but_rem"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        for(size_t s = 0; s < cur_area_map.tree_shadows.size(); s++) {
            if(cur_area_map.tree_shadows[s] == ed_cur_shadow) {
                cur_area_map.tree_shadows.erase(cur_area_map.tree_shadows.begin() + s);
                delete ed_cur_shadow;
                ed_cur_shadow = NULL;
                shadow_to_gui();
                break;
            }
        }
    };
    frm_shadow->widgets["txt_x"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_y"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_w"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_h"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["ang_an"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["bar_al"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_file"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_sx"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadow->widgets["txt_sy"]->lose_focus_handler = lambda_gui_to_shadow;
    frm_shadows->widgets["but_back"]->description =     "Go back to the main menu.";
    frm_shadows->widgets["but_new"]->description =      "Create a new tree shadow wherever you click.";
    frm_shadows->widgets["but_sel_none"]->description = "Deselect the current tree shadow.";
    frm_shadow->widgets["but_rem"]->description =       "Delete the current tree shadow.";
    frm_shadow->widgets["txt_file"]->description =      "File name for the shadow's texture.";
    frm_shadow->widgets["txt_x"]->description =         "X position of the shadow's center.";
    frm_shadow->widgets["txt_y"]->description =         "Y position of the shadow's center.";
    frm_shadow->widgets["txt_w"]->description =         "Width of the shadow's image.";
    frm_shadow->widgets["txt_h"]->description =         "Height of the shadow's image.";
    frm_shadow->widgets["ang_an"]->description =        "Angle of the shadow's image.";
    frm_shadow->widgets["bar_al"]->description =        "How opaque the shadow's image is.";
    frm_shadow->widgets["txt_sx"]->description =        "Horizontal sway amount multiplier (0 = no sway).";
    frm_shadow->widgets["txt_sy"]->description =        "Vertical sway amount multiplier (0 = no sway).";
    
    
    //Properties -- background.
    auto lambda_gui_to_bg = [] (lafi_widget*) { gui_to_bg(); };
    auto lambda_gui_to_bg_click = [] (lafi_widget*, int, int) { gui_to_bg(); };
    frm_bg->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        ed_sec_mode = ESM_NONE;
        bg_to_gui();
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
    frm_bg->widgets["chk_mouse"]->description = "If checked, use left/right mouse button to move/stretch.";
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
        ed_error_type = EET_NONE_YET;
        if(((lafi_checkbox*) c)->checked) {
            ed_sec_mode = ESM_TEXTURE_VIEW;
            clear_area_textures();
            load_area_textures();
            update_review_frame();
            
        } else {
            ed_sec_mode = ESM_NONE;
            update_review_frame();
        }
    };
    frm_review->widgets["chk_shadows"]->left_mouse_click_handler = [] (lafi_widget * c, int, int) {
        ed_show_shadows = ((lafi_checkbox*) c)->checked;
        update_review_frame();
    };
    frm_review->widgets["but_back"]->description =         "Go back to the main menu.";
    frm_review->widgets["but_find_errors"]->description =  "Search for problems with the area.";
    frm_review->widgets["but_goto_error"]->description =   "Focus the camera on the problem found, if applicable.";
    frm_review->widgets["chk_see_textures"]->description = "Preview how the textures will look like.";
    frm_review->widgets["chk_shadows"]->description =      "Show tree shadows?";
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
        show_widget(ed_gui->widgets["frm_bottom"]);
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description = "Cancel.";
    
    
    ed_file_name.clear();
    
}

/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 */
void area_editor::load_area() {
    ::load_area(ed_file_name, true);
    ((lafi_button*) ed_gui->widgets["frm_main"]->widgets["but_area"])->text = ed_file_name;
    show_widget(ed_gui->widgets["frm_main"]->widgets["frm_area"]);
    enable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_load"]);
    enable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_save"]);
    
    for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
        check_linedef_intersections(cur_area_map.vertices[v]);
    }
    
    change_background(ed_bg_file_name);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
    
    ed_error_type = EET_NONE_YET;
    ed_error_sector_ptr = NULL;
    ed_error_string.clear();
    ed_error_vertex_ptr = NULL;
    
    ed_intersecting_lines.clear();
    ed_non_simples.clear();
    ed_lone_lines.clear();
    
    ed_cur_sector = NULL;
    ed_cur_mob = NULL;
    ed_cur_shadow = NULL;
    sector_to_gui();
    mob_to_gui();
    bg_to_gui();
    
    ed_mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
}

/* ----------------------------------------------------------------------------
 * Loads the current mob's data onto the gui.
 */
void area_editor::mob_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_objects"]->widgets["frm_object"];
    
    if(!ed_cur_mob) {
        hide_widget(f);
    } else {
        show_widget(f);
        
        ((lafi_angle_picker*) f->widgets["ang_angle"])->set_angle_rads(ed_cur_mob->angle);
        ((lafi_textbox*) f->widgets["txt_vars"])->text = ed_cur_mob->vars;
        
        ((lafi_button*) f->widgets["but_category"])->text = mob_categories.get_pname(ed_cur_mob->category);
        
        lafi_button* but_type = (lafi_button*) f->widgets["but_type"];
        if(ed_cur_mob->category == MOB_CATEGORY_NONE) {
            disable_widget(but_type);
        } else {
            enable_widget(but_type);
        }
        but_type->text = ed_cur_mob->type ? ed_cur_mob->type->name : "";
    }
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
    if(type == AREA_EDITOR_PICKER_AREA) {
        elements = folder_to_vector(AREA_FOLDER, false);
        for(size_t e = 0; e < elements.size(); e++) {
            size_t pos = elements[e].find(".txt");
            if(pos != string::npos) {
                elements[e].erase(pos, 4);
            }
        }
        
    } else if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
    
        for(size_t t = 0; t < sector_types.get_nr_of_types(); t++) {
            elements.push_back(sector_types.get_name(t));
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_CATEGORY) {
    
        for(unsigned char f = 0; f < mob_categories.get_nr_of_categories(); f++) { //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            elements.push_back(mob_categories.get_pname(f));
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        if(ed_cur_mob->category != MOB_CATEGORY_NONE) {
            mob_categories.get_list(elements, ed_cur_mob->category);
        }
        
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); e++) {
        lafi_button* b = new lafi_button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler = [name, type] (lafi_widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    ((lafi_scrollbar*) ed_gui->widgets["frm_picker"]->widgets["bar_scroll"])->make_widget_scroll(f);
}

/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void area_editor::pick(string name, unsigned char type) {
    change_to_right_frame();
    show_widget(ed_gui->widgets["frm_bottom"]);
    
    if(type == AREA_EDITOR_PICKER_AREA) {
    
        ed_file_name = name;
        load_area();
        
    } else if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
    
        if(ed_cur_sector) {
            ed_cur_sector->type = sector_types.get_nr(name);
            sector_to_gui();
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_CATEGORY) {
    
        if(ed_cur_mob) {
            ed_cur_mob->category = mob_categories.get_nr_from_pname(name);
            ed_cur_mob->type = NULL;
            mob_to_gui();
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        if(ed_cur_mob) {
            mob_categories.set_mob_type_ptr(ed_cur_mob, name);
        }
        
        mob_to_gui();
        
    }
}

/* ----------------------------------------------------------------------------
 * Saves the area onto the disk.
 */
void area_editor::save_area() {
    data_node file_node = data_node("", "");
    
    //Point down the weather and background again.
    file_node.add(new data_node("weather", cur_area_map.weather_name));
    if(cur_area_map.bg_bmp_file_name.size())
        file_node.add(new data_node("bg_bmp", cur_area_map.bg_bmp_file_name));
    file_node.add(new data_node("bg_color", c2s(cur_area_map.bg_color)));
    file_node.add(new data_node("bg_dist", f2s(cur_area_map.bg_dist)));
    file_node.add(new data_node("bg_zoom", f2s(cur_area_map.bg_bmp_zoom)));
    
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
    //Mobs.
    data_node* mobs_node = new data_node("mobs", "");
    file_node.add(mobs_node);
    
    for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
        mob_gen* m_ptr = cur_area_map.mob_generators[m];
        data_node* mob_node = new data_node(mob_categories.get_sname(m_ptr->category), "");
        mobs_node->add(mob_node);
        
        if(m_ptr->type) {
            mob_node->add(
                new data_node("type", m_ptr->type->name)
            );
        }
        mob_node->add(
            new data_node(
                "pos",
                f2s(m_ptr->x) + " " + f2s(m_ptr->y)
            )
        );
        if(m_ptr->angle != 0) {
            mob_node->add(
                new data_node("angle", f2s(m_ptr->angle))
            );
        }
        if(m_ptr->vars.size()) {
            mob_node->add(
                new data_node("vars", m_ptr->vars)
            );
        }
        
    }
    
    //Vertices.
    data_node* vertices_node = new data_node("vertices", "");
    file_node.add(vertices_node);
    
    for(size_t v = 0; v < cur_area_map.vertices.size(); v++) {
        vertex* v_ptr = cur_area_map.vertices[v];
        data_node* vertex_node = new data_node("vertex", f2s(v_ptr->x) + " " + f2s(v_ptr->y));
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
            else s_str += i2s(l_ptr->sector_nrs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        linedef_node->add(new data_node("s", s_str));
        linedef_node->add(new data_node("v", i2s(l_ptr->vertex_nrs[0]) + " " + i2s(l_ptr->vertex_nrs[1])));
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    file_node.add(sectors_node);
    
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        data_node* sector_node = new data_node("sector", "");
        sectors_node->add(sector_node);
        
        if(s_ptr->type != SECTOR_TYPE_NORMAL) sector_node->add(new data_node("type", sector_types.get_name(s_ptr->type)));
        sector_node->add(new data_node("z", f2s(s_ptr->z)));
        if(s_ptr->brightness != DEF_SECTOR_BRIGHTNESS) sector_node->add(new data_node("brightness", i2s(s_ptr->brightness)));
        if(s_ptr->fade) sector_node->add(new data_node("fade", b2s(s_ptr->fade)));
        
        
        sector_node->add(new data_node("texture", s_ptr->file_name));
        if(s_ptr->rot != 0) {
            sector_node->add(new data_node("texture_rotate", f2s(s_ptr->rot)));
        }
        if(s_ptr->scale_x != 1 || s_ptr->scale_y != 1) {
            sector_node->add(new data_node("texture_scale",
                                           f2s(s_ptr->scale_x) + " " + f2s(s_ptr->scale_y)));
        }
        if(s_ptr->trans_x != 0 || s_ptr->trans_y != 0) {
            sector_node->add(new data_node("texture_trans",
                                           f2s(s_ptr->trans_x) + " " + f2s(s_ptr->trans_y)));
        }
    }
    
    //Tree shadows.
    data_node* shadows_node = new data_node("tree_shadows", "");
    file_node.add(shadows_node);
    
    for(size_t s = 0; s < cur_area_map.tree_shadows.size(); s++) {
        tree_shadow* s_ptr = cur_area_map.tree_shadows[s];
        data_node* shadow_node = new data_node("shadow", "");
        shadows_node->add(shadow_node);
        
        shadow_node->add(new data_node("pos", f2s(s_ptr->x) + " " + f2s(s_ptr->y)));
        shadow_node->add(new data_node("size", f2s(s_ptr->w) + " " + f2s(s_ptr->h)));
        if(s_ptr->angle != 0) shadow_node->add(new data_node("angle", f2s(s_ptr->angle)));
        if(s_ptr->alpha != 255) shadow_node->add(new data_node("alpha", i2s(s_ptr->alpha)));
        shadow_node->add(new data_node("file", s_ptr->file_name));
        shadow_node->add(new data_node("sway", f2s(s_ptr->sway_x) + " " + f2s(s_ptr->sway_y)));
        
    }
    
    //Editor background.
    file_node.add(new data_node("ed_bg_file_name", ed_bg_file_name));
    file_node.add(new data_node("ed_bg_x",         f2s(ed_bg_x)));
    file_node.add(new data_node("ed_bg_y",         f2s(ed_bg_y)));
    file_node.add(new data_node("ed_bg_w",         f2s(ed_bg_w)));
    file_node.add(new data_node("ed_bg_h",         f2s(ed_bg_h)));
    file_node.add(new data_node("ed_bg_alpha",     i2s(ed_bg_a)));
    
    
    file_node.save_file(AREA_FOLDER "/" + ed_file_name + ".txt");
    
    ed_cur_sector = NULL;
    ed_cur_mob = NULL;
    sector_to_gui();
    mob_to_gui();
    ed_mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
}

/* ----------------------------------------------------------------------------
 * Loads the current sector's data onto the gui.
 */
void area_editor::sector_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_sectors"]->widgets["frm_sector"];
    if(ed_cur_sector) {
        show_widget(f);
        
        ((lafi_textbox*) f->widgets["txt_z"])->text = f2s(ed_cur_sector->z);
        ((lafi_checkbox*) f->widgets["chk_fade"])->set(ed_cur_sector->fade);
        ((lafi_textbox*) f->widgets["txt_texture"])->text = ed_cur_sector->file_name;
        ((lafi_textbox*) f->widgets["txt_brightness"])->text = i2s(ed_cur_sector->brightness);
        ((lafi_button*) f->widgets["but_type"])->text = sector_types.get_name(ed_cur_sector->type);
        //ToDo hazards.
        
        if(ed_cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            disable_widget(f->widgets["chk_fade"]);
        } else {
            enable_widget(f->widgets["chk_fade"]);
        }
        
        if(ed_cur_sector->fade || ed_cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            disable_widget(f->widgets["txt_texture"]);
            disable_widget(f->widgets["but_adv"]);
        } else {
            enable_widget(f->widgets["txt_texture"]);
            enable_widget(f->widgets["but_adv"]);
        }
        
        adv_textures_to_gui();
        
    } else {
        hide_widget(f);
    }
}

/* ----------------------------------------------------------------------------
 * Loads a tree shadow's info onto the gui.
 */
void area_editor::shadow_to_gui() {
    lafi_frame* f = (lafi_frame*) ed_gui->widgets["frm_shadows"]->widgets["frm_shadow"];
    if(ed_cur_shadow) {
    
        show_widget(f);
        ((lafi_textbox*) f->widgets["txt_x"])->text = f2s(ed_cur_shadow->x);
        ((lafi_textbox*) f->widgets["txt_y"])->text = f2s(ed_cur_shadow->y);
        ((lafi_textbox*) f->widgets["txt_w"])->text = f2s(ed_cur_shadow->w);
        ((lafi_textbox*) f->widgets["txt_h"])->text = f2s(ed_cur_shadow->h);
        ((lafi_angle_picker*) f->widgets["ang_an"])->set_angle_rads(ed_cur_shadow->angle);
        ((lafi_scrollbar*) f->widgets["bar_al"])->set_value(ed_cur_shadow->alpha);
        ((lafi_textbox*) f->widgets["txt_file"])->text = ed_cur_shadow->file_name;
        ((lafi_textbox*) f->widgets["txt_sx"])->text = f2s(ed_cur_shadow->sway_x);
        ((lafi_textbox*) f->widgets["txt_sy"])->text = f2s(ed_cur_shadow->sway_y);
        
    } else {
        hide_widget(f);
    }
}

/* ----------------------------------------------------------------------------
 * Snaps a coordinate to the nearest grid space.
 */
float area_editor::snap_to_grid(const float c) {
    if(ed_shift_pressed) return c;
    return round(c / GRID_INTERVAL) * GRID_INTERVAL;
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
    
    lbl_error_2->text.clear();
    lbl_error_3->text.clear();
    lbl_error_4->text.clear();
    
    if(ed_sec_mode == ESM_TEXTURE_VIEW) {
        disable_widget(ed_gui->widgets["frm_review"]->widgets["but_find_errors"]);
        disable_widget(ed_gui->widgets["frm_review"]->widgets["but_goto_error"]);
    } else {
        enable_widget(ed_gui->widgets["frm_review"]->widgets["but_find_errors"]);
        enable_widget(ed_gui->widgets["frm_review"]->widgets["but_goto_error"]);
    }
    
    if(ed_error_type == EET_NONE_YET || ed_error_type == EET_NONE) {
        disable_widget(but_goto_error);
        if(ed_error_type == EET_NONE_YET) {
            lbl_error_1->text = "---";
        } else {
            lbl_error_1->text = "No errors found.";
        }
        
    } else {
        if(ed_error_type == EET_INTERSECTING_LINEDEFS) {
        
            if(ed_intersecting_lines.size() == 0) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Two lines cross";
            lbl_error_2->text = "each other, at";
            float u;
            linedef_intersection* li_ptr = &ed_intersecting_lines[0];
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
                      
            lbl_error_3->text = "(" + f2s(floor(li_ptr->l1->vertices[0]->x + cos(a) * u * d)) +
                                "," + f2s(floor(li_ptr->l1->vertices[0]->y + sin(a) * u * d)) + ")!";
                                
        } else if(ed_error_type == EET_BAD_SECTOR) {
        
            if(ed_non_simples.size() == 0) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Non-simple sector";
            lbl_error_2->text = "found! (Does the";
            lbl_error_3->text = "sector contain";
            lbl_error_4->text = "itself?)";
            
        } else if(ed_error_type == EET_LONE_LINE) {
        
            if(ed_lone_lines.size() == 0) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Lone line found!";
            lbl_error_2->text = "You probably want";
            lbl_error_3->text = "to drag one vertex";
            lbl_error_4->text = "to the other.";
            
        } else if(ed_error_type == EET_OVERLAPPING_VERTICES) {
        
            if(!ed_error_vertex_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Overlapping vertices";
            lbl_error_2->text =
                "at (" + f2s(ed_error_vertex_ptr->x) + "," +
                f2s(ed_error_vertex_ptr->y) + ")!";
            lbl_error_3->text = "(Drag one of them";
            lbl_error_3->text = "into the other)";
            
        } else if(ed_error_type == EET_MISSING_TEXTURE) {
        
            if(!ed_error_sector_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Sector without";
            lbl_error_2->text = "texture found!";
            
        } else if(ed_error_type == EET_UNKNOWN_TEXTURE) {
        
            if(!ed_error_sector_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Sector with unknown";
            lbl_error_2->text = "texture found!";
            lbl_error_3->text = "(" + ed_error_string + ")";
            
        } else if(ed_error_type == EET_TYPELESS_MOB) {
        
            if(!ed_error_mob_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Mob with no";
            lbl_error_2->text = "type found!";
            
            
        } else if(ed_error_type == EET_MOB_OOB) {
        
            if(!ed_error_mob_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Mob that is not";
            lbl_error_2->text = "on any sector";
            lbl_error_3->text = "found! It's probably";
            lbl_error_4->text = "out of bounds.";
            
            
        } else if(ed_error_type == EET_LANDING_SITE) {
        
            lbl_error_1->text = "There are no";
            lbl_error_2->text = "sectors of type";
            lbl_error_3->text = "\"landing site\"!";
            
        } else if(ed_error_type == EET_INVALID_SHADOW) {
        
            lbl_error_1->text = "Tree shadow with";
            lbl_error_2->text = "invalid image found!";
            
        }
    }
    
    ((lafi_checkbox*) ed_gui->widgets["frm_review"]->widgets["chk_see_textures"])->set(ed_sec_mode == ESM_TEXTURE_VIEW);
    ((lafi_checkbox*) ed_gui->widgets["frm_review"]->widgets["chk_shadows"])->set(ed_show_shadows);
}