/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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


const float area_editor::GRID_INTERVAL = 32.0f;
const float area_editor::STOP_RADIUS = 16.0f;
const float area_editor::LINK_THICKNESS = 2.0f;


/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    guide_aspect_ratio(true),
    guide_bitmap(NULL),
    guide_x(0),
    guide_y(0),
    guide_w(1000),
    guide_h(1000),
    guide_a(255),
    cur_mob(NULL),
    cur_sector(NULL),
    cur_shadow(NULL),
    double_click_time(0),
    error_mob_ptr(NULL),
    error_path_stop_ptr(NULL),
    error_sector_ptr(NULL),
    error_shadow_ptr(NULL),
    error_type(area_editor::EET_NONE_YET),
    error_vertex_ptr(NULL),
    gui(NULL),
    holding_m1(false),
    holding_m2(false),
    made_changes(false),
    mode(EDITOR_MODE_MAIN),
    moving_thing(string::npos),
    moving_thing_x(0),
    moving_thing_y(0),
    new_link_first_stop(NULL),
    on_sector(NULL),
    sec_mode(ESM_NONE),
    shift_pressed(false),
    show_closest_stop(false),
    show_guide(false),
    show_shadows(true),
    wum(NULL) {
    
}

/* ----------------------------------------------------------------------------
 * Stores the data from the advanced texture settings onto the gui.
 */
void area_editor::adv_textures_to_gui() {
    if(!cur_sector) {
        area_editor::mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
        return;
    }
    
    lafi::frame* f = (lafi::frame*) area_editor::gui->widgets["frm_adv_textures"];
    
    ((lafi::textbox*) f->widgets["txt_x"])->text =  f2s(cur_sector->texture_info.trans_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text =  f2s(cur_sector->texture_info.trans_y);
    ((lafi::textbox*) f->widgets["txt_sx"])->text = f2s(cur_sector->texture_info.scale_x);
    ((lafi::textbox*) f->widgets["txt_sy"])->text = f2s(cur_sector->texture_info.scale_y);
    ((lafi::angle_picker*) f->widgets["ang_a"])->set_angle_rads(cur_sector->texture_info.rot);
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
 * Changes the guide image.
 */
void area_editor::change_guide(string new_file_name) {
    if(guide_bitmap && guide_bitmap != bmp_error) al_destroy_bitmap(guide_bitmap);
    guide_bitmap = NULL;
    
    if(new_file_name.size()) {
        guide_bitmap = load_bmp(new_file_name, NULL, false);
    }
    guide_file_name = new_file_name;
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame(bool hide_all) {
    sec_mode = ESM_NONE;
    
    hide_widget(gui->widgets["frm_main"]);
    hide_widget(gui->widgets["frm_picker"]);
    hide_widget(gui->widgets["frm_sectors"]);
    hide_widget(gui->widgets["frm_paths"]);
    hide_widget(gui->widgets["frm_adv_textures"]);
    hide_widget(gui->widgets["frm_objects"]);
    hide_widget(gui->widgets["frm_shadows"]);
    hide_widget(gui->widgets["frm_guide"]);
    hide_widget(gui->widgets["frm_review"]);
    
    if(!hide_all) {
        if(mode == EDITOR_MODE_MAIN) {
            show_widget(gui->widgets["frm_main"]);
        } else if(mode == EDITOR_MODE_SECTORS) {
            show_widget(gui->widgets["frm_sectors"]);
        } else if(mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS) {
            show_widget(gui->widgets["frm_adv_textures"]);
        } else if(mode == EDITOR_MODE_OBJECTS) {
            show_widget(gui->widgets["frm_objects"]);
        } else if(mode == EDITOR_MODE_PATHS) {
            show_widget(gui->widgets["frm_paths"]);
        } else if(mode == EDITOR_MODE_SHADOWS) {
            show_widget(gui->widgets["frm_shadows"]);
        } else if(mode == EDITOR_MODE_GUIDE) {
            show_widget(gui->widgets["frm_guide"]);
        } else if(mode == EDITOR_MODE_REVIEW) {
            show_widget(gui->widgets["frm_review"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {

    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Closes the change warning box.
 */
void area_editor::close_changes_warning() {
    hide_widget(gui->widgets["frm_changes"]);
    show_widget(gui->widgets["frm_bottom"]);
}


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {

    gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
        al_clear_to_color(al_map_rgb(0, 0, 16));
        
        //Grid.
        if(sec_mode != ESM_TEXTURE_VIEW) {
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
                    
                    if(draw_line) al_draw_line(x, cam_topmost, x, cam_bottommost + GRID_INTERVAL, c, 1.0 / cam_zoom);
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
        }
        
        //Edges.
        if(sec_mode != ESM_TEXTURE_VIEW) {
        
            unsigned char sector_opacity = 224;
            if(mode == EDITOR_MODE_OBJECTS || mode == EDITOR_MODE_PATHS || mode == EDITOR_MODE_SHADOWS) sector_opacity = 128;
            
            size_t n_edges = cur_area_data.edges.size();
            for(size_t e = 0; e < n_edges; ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                
                if(!is_edge_valid(e_ptr)) continue;
                
                bool one_sided = true;
                bool error_highlight = false;
                bool valid = true;
                bool mouse_on = false;
                bool selected = false;
                
                if(error_sector_ptr) {
                    if(e_ptr->sectors[0] == error_sector_ptr) error_highlight = true;
                    if(e_ptr->sectors[1] == error_sector_ptr) error_highlight = true;
                    
                } else {
                    for(size_t ie = 0; ie < intersecting_edges.size(); ++ie) {
                        if(intersecting_edges[ie].contains(e_ptr)) {
                            valid = false;
                            break;
                        }
                    }
                    
                    if(non_simples.find(e_ptr->sectors[0]) != non_simples.end()) valid = false;
                    if(non_simples.find(e_ptr->sectors[1]) != non_simples.end()) valid = false;
                    if(lone_edges.find(e_ptr) != lone_edges.end()) valid = false;
                }
                
                if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
                
                if(on_sector && mode == EDITOR_MODE_SECTORS) {
                    if(e_ptr->sectors[0] == on_sector) mouse_on = true;
                    if(e_ptr->sectors[1] == on_sector) mouse_on = true;
                }
                
                if(cur_sector && mode == EDITOR_MODE_SECTORS) {
                    if(e_ptr->sectors[0] == cur_sector) selected = true;
                    if(e_ptr->sectors[1] == cur_sector) selected = true;
                }
                
                
                al_draw_line(
                    e_ptr->vertexes[0]->x,
                    e_ptr->vertexes[0]->y,
                    e_ptr->vertexes[1]->x,
                    e_ptr->vertexes[1]->y,
                    (selected ?        al_map_rgba(224, 224, 64,  sector_opacity) :
                     error_highlight ? al_map_rgba(192, 80,  0,   sector_opacity) :
                     !valid ?          al_map_rgba(192, 32,  32,  sector_opacity) :
                     one_sided ?       al_map_rgba(240, 240, 240, sector_opacity) :
                     al_map_rgba(160, 160, 160, sector_opacity)
                    ),
                    (mouse_on || selected ? 3.0 : 2.0) / cam_zoom
                );
                
                //Debug: uncomment this to show the sector numbers on each side.
                /*float mid_x = (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2;
                float mid_y = (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2;
                float angle = atan2(e_ptr->vertexes[0]->y - e_ptr->vertexes[1]->y, e_ptr->vertexes[0]->x - e_ptr->vertexes[1]->x);
                draw_scaled_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle + M_PI_2) * 4,
                    mid_x + sin(angle + M_PI_2) * 4,
                    0.5 / cam_zoom, 0.5 / cam_zoom,
                    ALLEGRO_ALIGN_CENTER, e_ptr->sector_nrs[0] == string::npos ? "--" : i2s(e_ptr->sector_nrs[0]).c_str());
                draw_scaled_text(
                    font, al_map_rgb(192, 255, 192),
                    mid_x + cos(angle - M_PI_2) * 4,
                    mid_y + sin(angle - M_PI_2) * 4,
                    0.5 / cam_zoom, 0.5 / cam_zoom,
                    ALLEGRO_ALIGN_CENTER, e_ptr->sector_nrs[1] == string::npos ? "--" : i2s(e_ptr->sector_nrs[1]).c_str());*/
            }
            
            //Vertexes.
            size_t n_vertexes = cur_area_data.vertexes.size();
            for(size_t v = 0; v < n_vertexes; ++v) {
                vertex* v_ptr = cur_area_data.vertexes[v];
                al_draw_filled_circle(
                    v_ptr->x,
                    v_ptr->y,
                    3.0 / cam_zoom,
                    al_map_rgba(224, 224, 224, sector_opacity)
                );
            }
            
            if(mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS && cur_sector) {
                draw_sector_texture(cur_sector, 0, 0, 1);
            }
            
        } else {
        
            //Draw textures.
            for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
                draw_sector(cur_area_data.sectors[s], 0, 0, 1.0);
            }
        }
        
        //Mobs.
        unsigned char mob_opacity = 224;
        if(
            mode == EDITOR_MODE_SECTORS || mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS ||
            mode == EDITOR_MODE_PATHS || mode == EDITOR_MODE_SHADOWS
        ) {
            mob_opacity = 64;
        }
        if(sec_mode == ESM_TEXTURE_VIEW) mob_opacity = 0;
        
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            bool valid = m_ptr->type != NULL;
            
            float radius = m_ptr->type ? m_ptr->type->radius == 0 ? 16 : m_ptr->type->radius : 16;
            
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
            
            if(m_ptr == cur_mob && mode == EDITOR_MODE_OBJECTS) {
                al_draw_circle(
                    m_ptr->x, m_ptr->y,
                    radius,
                    al_map_rgba(192, 192, 192, mob_opacity), 2 / cam_zoom
                );
            }
            
        }
        
        //Paths.
        if(mode == EDITOR_MODE_PATHS) {
        
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                al_draw_filled_circle(
                    s_ptr->x, s_ptr->y,
                    STOP_RADIUS,
                    al_map_rgb(224, 192, 160)
                );
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); l++) {
                    path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                    bool one_way = s_ptr->links[l].one_way;
                    
                    al_draw_line(
                        s_ptr->x, s_ptr->y,
                        s2_ptr->x, s2_ptr->y,
                        (one_way ? al_map_rgb(255, 160, 160) : al_map_rgb(255, 255, 160)),
                        LINK_THICKNESS / cam_zoom
                    );
                    
                    if(one_way) {
                        //Draw a triangle down the middle.
                        float mid_x = (s_ptr->x + s2_ptr->x) / 2.0f;
                        float mid_y = (s_ptr->y + s2_ptr->y) / 2.0f;
                        float angle = atan2(s2_ptr->y - s_ptr->y, s2_ptr->x - s_ptr->x);
                        const float delta = (LINK_THICKNESS * 4) / cam_zoom;
                        
                        al_draw_filled_triangle(
                            mid_x + cos(angle) * delta,
                            mid_y + sin(angle) * delta,
                            mid_x + cos(angle + M_PI_2) * delta,
                            mid_y + sin(angle + M_PI_2) * delta,
                            mid_x + cos(angle - M_PI_2) * delta,
                            mid_y + sin(angle - M_PI_2) * delta,
                            al_map_rgb(255, 160, 160)
                        );
                    }
                }
            }
            
            if(sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK2) {
                al_draw_line(
                    new_link_first_stop->x, new_link_first_stop->y,
                    mouse_cursor_x, mouse_cursor_y,
                    al_map_rgb(255, 255, 255), 2 / cam_zoom
                );
            }
            
            if(show_closest_stop) {
                path_stop* closest = NULL;
                dist closest_dist;
                for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                    path_stop* s_ptr = cur_area_data.path_stops[s];
                    dist d(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y);
                    
                    if(!closest || d < closest_dist) {
                        closest = s_ptr;
                        closest_dist = d;
                    }
                }
                
                al_draw_line(
                    mouse_cursor_x, mouse_cursor_y,
                    closest->x, closest->y,
                    al_map_rgb(96, 224, 32), 2 / cam_zoom
                );
            }
        }
        
        //Shadows.
        if(mode == EDITOR_MODE_SHADOWS || (sec_mode == ESM_TEXTURE_VIEW && show_shadows)) {
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                draw_sprite(
                    s_ptr->bitmap, s_ptr->x, s_ptr->y, s_ptr->w, s_ptr->h,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(mode == EDITOR_MODE_SHADOWS) {
                    float min_x, min_y, max_x, max_y;
                    get_shadow_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
                    
                    al_draw_rectangle(
                        min_x, min_y, max_x, max_y,
                        (s_ptr == cur_shadow ? al_map_rgb(224, 224, 64) : al_map_rgb(128, 128, 64)),
                        2 / cam_zoom
                    );
                }
            }
        }
        
        //New thing marker.
        if(
            sec_mode == ESM_NEW_SECTOR || sec_mode == ESM_NEW_OBJECT || sec_mode == ESM_NEW_SHADOW ||
            sec_mode == ESM_NEW_STOP || sec_mode == ESM_NEW_LINK1 || sec_mode == ESM_NEW_LINK2 ||
            sec_mode == ESM_NEW_1WLINK1 || sec_mode == ESM_NEW_1WLINK2
        ) {
            float x = mouse_cursor_x;
            float y = mouse_cursor_y;
            if(
                sec_mode != ESM_NEW_1WLINK1 && sec_mode != ESM_NEW_1WLINK2 &&
                sec_mode != ESM_NEW_LINK1 && sec_mode != ESM_NEW_LINK2
            ) {
                x = snap_to_grid(mouse_cursor_x);
                y = snap_to_grid(mouse_cursor_y);
            }
            al_draw_line(x - 16, y,      x + 16, y,      al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
            al_draw_line(x,      y - 16, x,      y + 16, al_map_rgb(255, 255, 255), 1.0 / cam_zoom);
        }
        
        //Delete thing marker.
        if(
            sec_mode == ESM_DEL_STOP || sec_mode == ESM_DEL_LINK
        ) {
            al_draw_line(
                mouse_cursor_x - 16, mouse_cursor_y - 16,
                mouse_cursor_x + 16, mouse_cursor_y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                mouse_cursor_x + 16, mouse_cursor_y - 16,
                mouse_cursor_x - 16, mouse_cursor_y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
        }
        
        //Lightly glow the sector under the mouse.
        if(mode == EDITOR_MODE_SECTORS) {
            if(on_sector && moving_thing == string::npos) {
                for(size_t t = 0; t < on_sector->triangles.size(); ++t) {
                    triangle* t_ptr = &on_sector->triangles[t];
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
        
        //Guide.
        if(guide_bitmap && show_guide) {
            al_draw_tinted_scaled_bitmap(
                guide_bitmap,
                map_alpha(guide_a),
                0, 0,
                al_get_bitmap_width(guide_bitmap), al_get_bitmap_height(guide_bitmap),
                guide_x, guide_y,
                guide_w, guide_h,
                0
            );
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Finds errors with the map.
 * On the first error, it adds it to error_type and stops.
 */
void area_editor::find_errors() {
    error_type = EET_NONE;
    error_sector_ptr = NULL;
    error_vertex_ptr = NULL;
    error_string.clear();
    
    //Check intersecting edges.
    if(!intersecting_edges.empty()) {
        error_type = EET_INTERSECTING_EDGES;
    }
    
    //Check overlapping vertexes.
    if(error_type == EET_NONE) {
        error_vertex_ptr = NULL;
        
        for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
            vertex* v1_ptr = cur_area_data.vertexes[v];
            if(v1_ptr->x == FLT_MAX) continue;
            
            for(size_t v2 = v + 1; v2 < cur_area_data.vertexes.size(); ++v2) {
                vertex* v2_ptr = cur_area_data.vertexes[v2];
                
                if(v1_ptr->x == v2_ptr->x && v1_ptr->y == v2_ptr->y) {
                    error_type = EET_OVERLAPPING_VERTEXES;
                    error_vertex_ptr = v1_ptr;
                    break;
                }
            }
            if(error_vertex_ptr) break;
        }
    }
    
    //Check non-simple sectors.
    if(error_type == EET_NONE) {
        if(!non_simples.empty()) {
            error_type = EET_BAD_SECTOR;
        }
    }
    
    //Check lone edges.
    if(error_type == EET_NONE) {
        if(!lone_edges.empty()) {
            error_type = EET_LONE_EDGE;
        }
    }
    
    //Check for missing textures.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        
            sector* s_ptr = cur_area_data.sectors[s];
            if(s_ptr->texture_info.file_name.empty() && s_ptr->type != SECTOR_TYPE_BOTTOMLESS_PIT && !s_ptr->fade) {
                error_type = EET_MISSING_TEXTURE;
                error_sector_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Check for unknown textures.
    if(error_type == EET_NONE) {
        vector<string> texture_file_names = folder_to_vector(TEXTURES_FOLDER, false);
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        
            sector* s_ptr = cur_area_data.sectors[s];
            
            if(s_ptr->texture_info.file_name.empty()) continue;
            
            if(find(texture_file_names.begin(), texture_file_names.end(), s_ptr->texture_info.file_name) == texture_file_names.end()) {
                error_type = EET_UNKNOWN_TEXTURE;
                error_string = s_ptr->texture_info.file_name;
                error_sector_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Objects with no type.
    if(error_type == EET_NONE) {
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            if(!cur_area_data.mob_generators[m]->type) {
                error_type = EET_TYPELESS_MOB;
                error_mob_ptr = cur_area_data.mob_generators[m];
                break;
            }
        }
    }
    
    //Objects out of bounds.
    if(error_type == EET_NONE) {
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            if(!get_sector(m_ptr->x, m_ptr->y, NULL, false)) {
                error_type = EET_MOB_OOB;
                error_mob_ptr = m_ptr;
                break;
            }
        }
    }
    
    //Lone path stops.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = cur_area_data.path_stops[s];
            bool has_link = false;
            
            if(!s_ptr->links.empty()) continue; //Duh, this means it has links.
            
            for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
                path_stop* s2_ptr = cur_area_data.path_stops[s2];
                if(s2_ptr == s_ptr) continue;
                
                if(s2_ptr->has_link(s_ptr)) {
                    has_link = true;
                    break;
                }
                
                if(has_link) break;
            }
            
            if(!has_link) {
                error_type = EET_LONE_PATH_STOP;
                error_path_stop_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Path stops out of bounds.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = cur_area_data.path_stops[s];
            if(!get_sector(s_ptr->x, s_ptr->y, NULL, false)) {
                error_type = EET_PATH_STOP_OOB;
                error_path_stop_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Two stops intersecting.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = cur_area_data.path_stops[s];
            for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
                path_stop* s2_ptr = cur_area_data.path_stops[s2];
                if(s2_ptr == s_ptr) continue;
                
                if(dist(s_ptr->x, s_ptr->y, s2_ptr->x, s2_ptr->y) <= 3.0) {
                    error_type = EET_PATH_STOPS_TOGETHER;
                    error_path_stop_ptr = s_ptr;
                    break;
                }
            }
        }
    }
    
    //Path graph is not connected.
    if(error_type == EET_NONE) {
        if(!cur_area_data.path_stops.empty()) {
            unordered_set<path_stop*> visited;
            depth_first_search(cur_area_data.path_stops, visited, cur_area_data.path_stops[0]);
            if(visited.size() != cur_area_data.path_stops.size()) {
                error_type = EET_PATHS_UNCONNECTED;
            }
        }
    }
    
    //Objects inside walls.
    if(error_type == EET_NONE) {
        error_mob_ptr = NULL;
        
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            
            if(error_mob_ptr) break;
            
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                if(!is_edge_valid(e_ptr)) continue;
                
                if(
                    circle_intersects_line(
                        m_ptr->x, m_ptr->y,
                        m_ptr->type->radius,
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y,
                        NULL, NULL
                    )
                ) {
                
                    bool in_wall = false;
                    
                    if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) in_wall = true;
                    else {
                        if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z + SECTOR_STEP) in_wall = true;
                        if(e_ptr->sectors[1]->z > e_ptr->sectors[0]->z + SECTOR_STEP) in_wall = true;
                        if(e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING) in_wall = true;
                        if(e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING) in_wall = true;
                    }
                    
                    if(in_wall) {
                        error_type = EET_MOB_IN_WALL;
                        error_mob_ptr = m_ptr;
                    }
                    break;
                    
                }
            }
        }
    }
    
    //Check if there are tree shadows with invalid images.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            if(cur_area_data.tree_shadows[s]->bitmap == bmp_error) {
                error_type = EET_INVALID_SHADOW;
                error_shadow_ptr = cur_area_data.tree_shadows[s];
            }
        }
    }
    
    
    update_review_frame();
}


/* ----------------------------------------------------------------------------
 * Focuses the camera on the error found, if any.
 */
void area_editor::goto_error() {
    if(error_type == EET_NONE || error_type == EET_NONE_YET) return;
    
    if(error_type == EET_INTERSECTING_EDGES) {
    
        if(intersecting_edges.empty()) {
            find_errors(); return;
        }
        
        edge_intersection* li_ptr = &intersecting_edges[0];
        float min_x, max_x, min_y, max_y;
        min_x = max_x = li_ptr->e1->vertexes[0]->x;
        min_y = max_y = li_ptr->e1->vertexes[0]->y;
        
        min_x = min(min_x, li_ptr->e1->vertexes[0]->x);
        min_x = min(min_x, li_ptr->e1->vertexes[1]->x);
        min_x = min(min_x, li_ptr->e2->vertexes[0]->x);
        min_x = min(min_x, li_ptr->e2->vertexes[1]->x);
        max_x = max(max_x, li_ptr->e1->vertexes[0]->x);
        max_x = max(max_x, li_ptr->e1->vertexes[1]->x);
        max_x = max(max_x, li_ptr->e2->vertexes[0]->x);
        max_x = max(max_x, li_ptr->e2->vertexes[1]->x);
        min_y = min(min_y, li_ptr->e1->vertexes[0]->y);
        min_y = min(min_y, li_ptr->e1->vertexes[1]->y);
        min_y = min(min_y, li_ptr->e2->vertexes[0]->y);
        min_y = min(min_y, li_ptr->e2->vertexes[1]->y);
        max_y = max(max_y, li_ptr->e1->vertexes[0]->y);
        max_y = max(max_y, li_ptr->e1->vertexes[1]->y);
        max_y = max(max_y, li_ptr->e2->vertexes[0]->y);
        max_y = max(max_y, li_ptr->e2->vertexes[1]->y);
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(error_type == EET_BAD_SECTOR) {
    
        if(non_simples.empty()) {
            find_errors(); return;
        }
        
        sector* s_ptr = *non_simples.begin();
        float min_x, min_y, max_x, max_y;
        get_sector_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(error_type == EET_LONE_EDGE) {
    
        if(lone_edges.empty()) {
            find_errors(); return;
        }
        
        edge* e_ptr = *lone_edges.begin();
        float min_x, min_y, max_x, max_y;
        min_x = e_ptr->vertexes[0]->x;
        max_x = min_x;
        min_y = e_ptr->vertexes[0]->y;
        max_y = min_y;
        
        min_x = min(min_x, e_ptr->vertexes[0]->x);
        min_x = min(min_x, e_ptr->vertexes[1]->x);
        max_x = max(max_x, e_ptr->vertexes[0]->x);
        max_x = max(max_x, e_ptr->vertexes[1]->x);
        min_y = min(min_y, e_ptr->vertexes[0]->y);
        min_y = min(min_y, e_ptr->vertexes[1]->y);
        max_y = max(max_y, e_ptr->vertexes[0]->y);
        max_y = max(max_y, e_ptr->vertexes[1]->y);
        
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(error_type == EET_OVERLAPPING_VERTEXES) {
    
        if(!error_vertex_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            error_vertex_ptr->x - 64,
            error_vertex_ptr->y - 64,
            error_vertex_ptr->x + 64,
            error_vertex_ptr->y + 64
        );
        
    } else if(
        error_type == EET_MISSING_TEXTURE ||
        error_type == EET_UNKNOWN_TEXTURE
    ) {
    
        if(!error_sector_ptr) {
            find_errors(); return;
        }
        
        float min_x, min_y, max_x, max_y;
        get_sector_bounding_box(error_sector_ptr, &min_x, &min_y, &max_x, &max_y);
        center_camera(min_x, min_y, max_x, max_y);
        
    } else if(
        error_type == EET_TYPELESS_MOB ||
        error_type == EET_MOB_OOB ||
        error_type == EET_MOB_IN_WALL
    ) {
    
        if(!error_mob_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            error_mob_ptr->x - 64,
            error_mob_ptr->y - 64,
            error_mob_ptr->x + 64,
            error_mob_ptr->y + 64
        );
        
    } else if(
        error_type == EET_LONE_PATH_STOP ||
        error_type == EET_PATH_STOPS_TOGETHER ||
        error_type == EET_PATH_STOP_OOB
    ) {
    
        if(!error_path_stop_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            error_path_stop_ptr->x - 64,
            error_path_stop_ptr->y - 64,
            error_path_stop_ptr->x + 64,
            error_path_stop_ptr->y + 64
        );
        
    } else if(error_type == EET_INVALID_SHADOW) {
    
        float min_x, min_y, max_x, max_y;
        get_shadow_bounding_box(error_shadow_ptr, &min_x, &min_y, &max_x, &max_y);
        center_camera(min_x, min_y, max_x, max_y);
    }
}


/* ----------------------------------------------------------------------------
 * Saves the advanced texture settings from the gui.
 */
void area_editor::gui_to_adv_textures() {
    if(!cur_sector) return;
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_adv_textures"];
    
    cur_sector->texture_info.trans_x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sector->texture_info.trans_y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sector->texture_info.scale_x = s2f(((lafi::textbox*) f->widgets["txt_sx"])->text);
    cur_sector->texture_info.scale_y = s2f(((lafi::textbox*) f->widgets["txt_sy"])->text);
    cur_sector->texture_info.rot = ((lafi::angle_picker*) f->widgets["ang_a"])->get_angle_rads();
    
    adv_textures_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the guide's data from the fields in the gui.
 */
void area_editor::gui_to_guide() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_guide"];
    
    string new_file_name = ((lafi::textbox*) f->widgets["txt_file"])->text;
    bool is_file_new = false;
    
    if(new_file_name != guide_file_name) {
        //New guide image, delete the old one.
        change_guide(new_file_name);
        is_file_new = true;
        if(guide_bitmap) {
            guide_w = al_get_bitmap_width(guide_bitmap);
            guide_h = al_get_bitmap_height(guide_bitmap);
        } else {
            guide_w = 0;
            guide_h = 0;
        }
    }
    
    guide_x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    guide_y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    
    guide_aspect_ratio = ((lafi::checkbox*) f->widgets["chk_ratio"])->checked;
    float new_w = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    float new_h = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    
    if(new_w != 0 && new_h != 0 && !is_file_new) {
        if(guide_aspect_ratio) {
            if(new_w == guide_w && new_h != guide_h) {
                float ratio = guide_w / guide_h;
                guide_h = new_h;
                guide_w = new_h * ratio;
            } else if(new_w != guide_w && new_h == guide_h) {
                float ratio = guide_h / guide_w;
                guide_w = new_w;
                guide_h = new_w * ratio;
            } else {
                guide_w = new_w;
                guide_h = new_h;
            }
        } else {
            guide_w = new_w;
            guide_h = new_h;
        }
    }
    
    sec_mode = ((lafi::checkbox*) f->widgets["chk_mouse"])->checked ? ESM_GUIDE_MOUSE : ESM_NONE;
    guide_a = ((lafi::scrollbar*) f->widgets["bar_alpha"])->low_value;
    
    guide_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves a mob's data using info on the gui.
 */
void area_editor::gui_to_mob() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_objects"]->widgets["frm_object"];
    
    if(!cur_mob) return;
    
    cur_mob->angle = ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
    cur_mob->vars = ((lafi::textbox*) f->widgets["txt_vars"])->text;
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the current tree shadow using the info on the gui.
 */
void area_editor::gui_to_shadow() {
    if(!cur_shadow) return;
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_shadows"]->widgets["frm_shadow"];
    
    cur_shadow->x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_shadow->y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_shadow->w = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_shadow->h = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_shadow->angle = ((lafi::angle_picker*) f->widgets["ang_an"])->get_angle_rads();
    cur_shadow->alpha = ((lafi::scrollbar*) f->widgets["bar_al"])->low_value;
    cur_shadow->sway_x = s2f(((lafi::textbox*) f->widgets["txt_sx"])->text);
    cur_shadow->sway_y = s2f(((lafi::textbox*) f->widgets["txt_sy"])->text);
    
    string new_file_name = ((lafi::textbox*) f->widgets["txt_file"])->text;
    
    if(new_file_name != cur_shadow->file_name) {
        //New image, delete the old one.
        if(cur_shadow->bitmap != bmp_error) {
            bitmaps.detach(cur_shadow->file_name);
        }
        cur_shadow->bitmap = bitmaps.get("Textures/" + new_file_name, NULL);
        cur_shadow->file_name = new_file_name;
    }
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sector using the info on the gui.
 */
void area_editor::gui_to_sector() {
    if(!cur_sector) return;
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_sectors"]->widgets["frm_sector"];
    
    cur_sector->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    cur_sector->fade = ((lafi::checkbox*) f->widgets["chk_fade"])->checked;
    cur_sector->always_cast_shadow = ((lafi::checkbox*) f->widgets["chk_shadow"])->checked;
    cur_sector->texture_info.file_name = ((lafi::textbox*) f->widgets["txt_texture"])->text;
    cur_sector->brightness = s2i(((lafi::textbox*) f->widgets["txt_brightness"])->text);
    cur_sector->tag = ((lafi::textbox*) f->widgets["txt_tag"])->text;
    //TODO hazards.
    
    sector_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Loads the guide's data from the memory to the gui.
 */
void area_editor::guide_to_gui() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_guide"];
    ((lafi::textbox*) f->widgets["txt_file"])->text = guide_file_name;
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(guide_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(guide_y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(guide_w);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(guide_h);
    ((lafi::checkbox*) f->widgets["chk_ratio"])->set(guide_aspect_ratio);
    ((lafi::checkbox*) f->widgets["chk_mouse"])->set(sec_mode == ESM_GUIDE_MOUSE);
    ((lafi::scrollbar*) f->widgets["bar_alpha"])->set_value(guide_a, false);
}


/* ----------------------------------------------------------------------------
 * Handles the events for the area editor.
 */
void area_editor::handle_controls(ALLEGRO_EVENT ev) {

    if(fade_mgr.is_fading()) return;
    
    gui->handle_event(ev);
    
    //Update mouse cursor in world coordinates.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* wum;
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) wum = NULL;
        else wum = gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y); //Widget under mouse.
        ((lafi::label*) gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")");
    }
    
    
    //Moving vertexes, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        if(
            ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16
            && moving_thing == string::npos && sec_mode != ESM_TEXTURE_VIEW &&
            mode != EDITOR_MODE_OBJECTS
        ) {
            on_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL, false);
        } else {
            on_sector = NULL;
        }
        
        //Move guide.
        if(sec_mode == ESM_GUIDE_MOUSE) {
        
            if(holding_m1) {
                guide_x += ev.mouse.dx / cam_zoom;
                guide_y += ev.mouse.dy / cam_zoom;
                
            } else if(holding_m2) {
            
                float new_w = guide_w + ev.mouse.dx / cam_zoom;
                float new_h = guide_h + ev.mouse.dy / cam_zoom;
                
                if(guide_aspect_ratio) {
                    //Find the most significant change.
                    if(ev.mouse.dx != 0 || ev.mouse.dy != 0) {
                        bool most_is_width = fabs((double) ev.mouse.dx) > fabs((double) ev.mouse.dy);
                        
                        if(most_is_width) {
                            float ratio = guide_h / guide_w;
                            guide_w = new_w;
                            guide_h = new_w * ratio;
                        } else {
                            float ratio = guide_w / guide_h;
                            guide_h = new_h;
                            guide_w = new_h * ratio;
                        }
                    }
                } else {
                    guide_w = new_w;
                    guide_h = new_h;
                }
                
            }
            
            guide_to_gui();
            
        } else if(holding_m2) {
            //Move camera.
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        //Move thing.
        if(moving_thing != string::npos) {
            if(mode == EDITOR_MODE_SECTORS) {
                vertex* v_ptr = cur_area_data.vertexes[moving_thing];
                v_ptr->x = snap_to_grid(mouse_cursor_x);
                v_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(mode == EDITOR_MODE_OBJECTS) {
                mob_gen* m_ptr = cur_area_data.mob_generators[moving_thing];
                m_ptr->x = snap_to_grid(mouse_cursor_x);
                m_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(mode == EDITOR_MODE_PATHS) {
                path_stop* s_ptr = cur_area_data.path_stops[moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y);
            } else if(mode == EDITOR_MODE_SHADOWS) {
                tree_shadow* s_ptr = cur_area_data.tree_shadows[moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x - moving_thing_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y - moving_thing_y);
                shadow_to_gui();
            }
            
            made_changes = true;
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
        
        if(ev.mouse.button == 1) holding_m1 = true;
        else if(ev.mouse.button == 2) holding_m2 = true;
        else if(ev.mouse.button == 3) cam_zoom = 1.0;
        
        if(ev.mouse.button != 1) return;
        if(ev.mouse.x > scr_w - 208) return;
        
        //If the user was editing, save it.
        if(mode == EDITOR_MODE_SECTORS) {
            gui_to_sector();
        } else if(mode == EDITOR_MODE_OBJECTS) {
            gui_to_mob();
        } else if(mode == EDITOR_MODE_SHADOWS) {
            gui_to_shadow();
        }
        
        //Sector-related clicking.
        if(sec_mode == ESM_NONE && mode == EDITOR_MODE_SECTORS) {
        
            moving_thing = string::npos;
            
            edge* clicked_edge_ptr = NULL;
            size_t clicked_edge_nr = string::npos;
            bool created_vertex = false;
            
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                
                if(!is_edge_valid(e_ptr)) continue;
                
                if(
                    circle_intersects_line(
                        mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    )
                ) {
                    clicked_edge_ptr = e_ptr;
                    clicked_edge_nr = e;
                    break;
                }
            }
            
            if(double_click_time == 0) double_click_time = 0.5;
            else if(clicked_edge_ptr) {
                //Create a new vertex.
                double_click_time = 0;
                
                //New vertex, on the split point.
                //TODO create it on the edge, not on the cursor.
                vertex* new_v_ptr = new vertex(mouse_cursor_x, mouse_cursor_y);
                cur_area_data.vertexes.push_back(new_v_ptr);
                
                //New edge, copied from the original one.
                edge* new_e_ptr = new edge(*clicked_edge_ptr);
                cur_area_data.edges.push_back(new_e_ptr);
                
                //Save the original end vertex for later.
                vertex* end_v_ptr = clicked_edge_ptr->vertexes[1];
                
                //Set vertexes on the new and original edges.
                new_e_ptr->vertex_nrs[0] = cur_area_data.vertexes.size() - 1;
                new_e_ptr->vertexes[0] = new_v_ptr;
                clicked_edge_ptr->vertex_nrs[1] = new_e_ptr->vertex_nrs[0];
                clicked_edge_ptr->vertexes[1] = new_v_ptr;
                
                //Set sectors on the new edge.
                if(new_e_ptr->sectors[0]) {
                    new_e_ptr->sectors[0]->edge_nrs.push_back(cur_area_data.edges.size() - 1);
                    new_e_ptr->sectors[0]->edges.push_back(new_e_ptr);
                }
                if(new_e_ptr->sectors[1]) {
                    new_e_ptr->sectors[1]->edge_nrs.push_back(cur_area_data.edges.size() - 1);
                    new_e_ptr->sectors[1]->edges.push_back(new_e_ptr);
                }
                
                //Set edges of the new vertex.
                new_v_ptr->edge_nrs.push_back(cur_area_data.edges.size() - 1);
                new_v_ptr->edge_nrs.push_back(clicked_edge_nr);
                new_v_ptr->edges.push_back(new_e_ptr);
                new_v_ptr->edges.push_back(clicked_edge_ptr);
                
                //Update edge data on the end vertex of the original edge
                //(it now links to the new edge, not the old).
                for(size_t ve = 0; ve < end_v_ptr->edges.size(); ++ve) {
                    if(end_v_ptr->edges[ve] == clicked_edge_ptr) {
                        end_v_ptr->edges[ve] = new_e_ptr;
                        end_v_ptr->edge_nrs[ve] = cur_area_data.edges.size() - 1;
                        break;
                    }
                }
                
                //Start dragging the new vertex.
                moving_thing = cur_area_data.vertexes.size() - 1;
                
                created_vertex = true;
                made_changes = true;
            }
            
            //Find a vertex to drag.
            if(!created_vertex) {
                for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                    if(
                        dist(
                            mouse_cursor_x, mouse_cursor_y,
                            cur_area_data.vertexes[v]->x, cur_area_data.vertexes[v]->y
                        ) <= 6.0 / cam_zoom
                    ) {
                        moving_thing = v;
                        break;
                    }
                }
            }
            
            //Find a sector to select.
            if(moving_thing == string::npos && !clicked_edge_ptr) {
                cur_sector = get_sector(mouse_cursor_x, mouse_cursor_y, NULL, false);
                sector_to_gui();
            }
            
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_OBJECTS) {
            //Object-related clicking.
            
            cur_mob = NULL;
            moving_thing = string::npos;
            for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
                mob_gen* m_ptr = cur_area_data.mob_generators[m];
                float radius = m_ptr->type ? m_ptr->type->radius == 0 ? 16 : m_ptr->type->radius : 16;
                if(dist(m_ptr->x, m_ptr->y, mouse_cursor_x, mouse_cursor_y) <= radius) {
                
                    cur_mob = m_ptr;
                    moving_thing = m;
                    break;
                }
            }
            mob_to_gui();
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_PATHS) {
            //Path-related clicking.
            
            cur_stop = NULL;
            moving_thing = string::npos;
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                if(dist(s_ptr->x, s_ptr->y, mouse_cursor_x, mouse_cursor_y) <= STOP_RADIUS) {
                
                    cur_stop = s_ptr;
                    moving_thing = s;
                    break;
                }
            }
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_SHADOWS) {
            //Shadow-related clicking.
            
            cur_shadow = NULL;
            moving_thing = string::npos;
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                float min_x, min_y, max_x, max_y;
                get_shadow_bounding_box(s_ptr, &min_x, &min_y, &max_x, &max_y);
                
                if(
                    mouse_cursor_x >= min_x && mouse_cursor_x <= max_x &&
                    mouse_cursor_y >= min_y && mouse_cursor_y <= max_y
                ) {
                    cur_shadow = s_ptr;
                    moving_thing = s;
                    moving_thing_x = mouse_cursor_x - s_ptr->x;
                    moving_thing_y = mouse_cursor_y - s_ptr->y;
                    break;
                }
            }
            shadow_to_gui();
            
        }
        
        if(sec_mode == ESM_NEW_SECTOR) {
            //Place a new sector where the cursor is.
            
            sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            size_t outer_sector_nr;
            sector* outer_sector = get_sector(hotspot_x, hotspot_y, &outer_sector_nr, false);
            
            sector* new_sector = new sector();
            if(outer_sector) outer_sector->clone(new_sector);
            
            //Create the vertexes.
            vertex* new_vertexes[4];
            for(size_t v = 0; v < 4; ++v) new_vertexes[v] = new vertex(0, 0);
            new_vertexes[0]->x = hotspot_x - GRID_INTERVAL / 2;
            new_vertexes[0]->y = hotspot_y - GRID_INTERVAL / 2;
            new_vertexes[1]->x = hotspot_x + GRID_INTERVAL / 2;
            new_vertexes[1]->y = hotspot_y - GRID_INTERVAL / 2;
            new_vertexes[2]->x = hotspot_x + GRID_INTERVAL / 2;
            new_vertexes[2]->y = hotspot_y + GRID_INTERVAL / 2;
            new_vertexes[3]->x = hotspot_x - GRID_INTERVAL / 2;
            new_vertexes[3]->y = hotspot_y + GRID_INTERVAL / 2;
            for(size_t v = 0; v < 4; ++v)cur_area_data.vertexes.push_back(new_vertexes[v]);
            
            //Create the edges.
            edge* new_edges[4];
            for(size_t l = 0; l < 4; ++l) {
                new_edges[l] = new edge(
                    cur_area_data.vertexes.size() - (4 - l),
                    cur_area_data.vertexes.size() - (4 - ((l + 1) % 4))
                );
                new_edges[l]->sector_nrs[0] = outer_sector_nr;
                new_edges[l]->sector_nrs[1] = cur_area_data.sectors.size();
                cur_area_data.edges.push_back(new_edges[l]);
            }
            
            //Add them to the area map.
            for(size_t e = 0; e < 4; ++e) new_sector->edge_nrs.push_back(cur_area_data.edges.size() - (4 - e));
            cur_area_data.sectors.push_back(new_sector);
            
            for(size_t e = 0; e < 4; ++e) new_edges[e]->fix_pointers(cur_area_data);
            for(size_t v = 0; v < 4; ++v) new_vertexes[v]->connect_edges(cur_area_data, cur_area_data.vertexes.size() - (4 - v));
            new_sector->connect_edges(cur_area_data, cur_area_data.sectors.size() - 1);
            
            //Add the edges to the outer sector's list.
            if(outer_sector) {
                for(size_t e = 0; e < 4; ++e) {
                    outer_sector->edges.push_back(new_edges[e]);
                    outer_sector->edge_nrs.push_back(cur_area_data.edges.size() - (4 - e));
                }
            }
            
            //Check for intersections.
            for(size_t v = 0; v < 4; v += 2) check_edge_intersections(new_vertexes[v]);
            
            //Triangulate new sector and the parent one.
            triangulate(new_sector);
            if(outer_sector) triangulate(outer_sector);
            
            cur_sector = new_sector;
            sector_to_gui();
            made_changes = true;
            
            
        } else if(sec_mode == ESM_NEW_OBJECT) {
            //Create a mob where the cursor is.
            
            sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            cur_area_data.mob_generators.push_back(
                new mob_gen(hotspot_x, hotspot_y)
            );
            
            cur_mob = cur_area_data.mob_generators.back();
            mob_to_gui();
            made_changes = true;
            
        } else if(sec_mode == ESM_NEW_STOP) {
            //Create a new stop where the cursor is.
            
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            cur_area_data.path_stops.push_back(
                new path_stop(hotspot_x, hotspot_y, vector<path_link>())
            );
            
            cur_stop = cur_area_data.path_stops.back();
            made_changes = true;
            
            
        } else if (sec_mode == ESM_NEW_LINK1 || sec_mode == ESM_NEW_1WLINK1) {
            //Pick a stop to start the link on.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <= STOP_RADIUS) {
                    new_link_first_stop = s_ptr;
                    sec_mode = (sec_mode == ESM_NEW_LINK1 ? ESM_NEW_LINK2 : ESM_NEW_1WLINK2);
                    break;
                }
            }
            
            made_changes = true;
            
        } else if (sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK2) {
            //Pick a stop to end the link on.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <= STOP_RADIUS) {
                
                    if(new_link_first_stop == s_ptr) continue;
                    
                    //Check if these two stops already have a link. Delete it if so.
                    for(size_t l = 0; l < new_link_first_stop->links.size(); ++l) {
                        if(new_link_first_stop->links[l].end_ptr == s_ptr) {
                            new_link_first_stop->links.erase(new_link_first_stop->links.begin() + l);
                            break;
                        }
                    }
                    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
                        if(s_ptr->links[l].end_ptr == new_link_first_stop) {
                            s_ptr->links.erase(s_ptr->links.begin() + l);
                            break;
                        }
                    }
                    
                    
                    new_link_first_stop->links.push_back(
                        path_link(
                            s_ptr, s, (sec_mode == ESM_NEW_1WLINK2)
                        )
                    );
                    sec_mode = (sec_mode == ESM_NEW_LINK2 ? ESM_NEW_LINK1 : ESM_NEW_1WLINK1);
                    break;
                }
            }
            
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_STOP) {
            //Pick a stop to delete.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <= STOP_RADIUS) {
                
                    //Check all links to this stop.
                    for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
                        path_stop* s2_ptr = cur_area_data.path_stops[s2];
                        for(size_t l = 0; l < s2_ptr->links.size(); ++l) {
                            if(s2_ptr->links[l].end_ptr == s_ptr) {
                                s2_ptr->links.erase(s2_ptr->links.begin() + l);
                                break;
                            }
                        }
                    }
                    
                    //Finally, delete the stop.
                    delete s_ptr;
                    cur_area_data.path_stops.erase(cur_area_data.path_stops.begin() + s);
                    break;
                }
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                cur_area_data.path_stops[s]->fix_nrs(cur_area_data);
            }
            
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_LINK) {
            //Pick a link to delete.
            
            bool deleted = false;
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                for(size_t l = 0; l < s_ptr->links.size(); ++l) {
                    path_stop* s2_ptr = s_ptr->links[l].end_ptr;
                    if(
                        circle_intersects_line(
                            mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                            s_ptr->x, s_ptr->y,
                            s2_ptr->x, s2_ptr->y
                        )
                    ) {
                    
                        s_ptr->links.erase(s_ptr->links.begin() + l);
                        deleted = true;
                        break;
                    }
                }
                
                if(deleted) break;
            }
            
            made_changes = true;
            
        } else if(sec_mode == ESM_NEW_SHADOW) {
            //Create a new shadow where the cursor is.
            
            sec_mode = ESM_NONE;
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            
            tree_shadow* new_shadow = new tree_shadow(hotspot_x, hotspot_y);
            new_shadow->bitmap = bmp_error;
            
            cur_area_data.tree_shadows.push_back(new_shadow);
            
            cur_shadow = new_shadow;
            shadow_to_gui();
            made_changes = true;
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        //Mouse button release.
        
        if(ev.mouse.button == 1) holding_m1 = false;
        else if(ev.mouse.button == 2) holding_m2 = false;
        
        if(ev.mouse.button == 1 && mode == EDITOR_MODE_SECTORS && sec_mode == ESM_NONE && moving_thing != string::npos) {
            //Release the vertex.
            
            vertex* moved_v_ptr = cur_area_data.vertexes[moving_thing];
            vertex* final_vertex = moved_v_ptr;
            
            unordered_set<sector*> affected_sectors;
            
            //Check if we should merge.
            for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                vertex* dest_v_ptr = cur_area_data.vertexes[v];
                if(dest_v_ptr == moved_v_ptr) continue;
                
                if(dist(moved_v_ptr->x, moved_v_ptr->y, dest_v_ptr->x, dest_v_ptr->y) <= (10 / cam_zoom)) {
                    //Merge vertexes.
                    
                    //Find out what to do with every edge of the dragged vertex.
                    for(size_t e = 0; e < moved_v_ptr->edges.size();) {
                    
                        bool was_deleted = false;
                        edge* e_ptr = moved_v_ptr->edges[e];
                        vertex* other_vertex = e_ptr->vertexes[0] == moved_v_ptr ? e_ptr->vertexes[1] : e_ptr->vertexes[0];
                        
                        //Check if it's being squashed into non-existence.
                        if(other_vertex == dest_v_ptr) {
                        
                            affected_sectors.insert(e_ptr->sectors[0]);
                            affected_sectors.insert(e_ptr->sectors[1]);
                            
                            //Clear it from its vertexes' lists.
                            for(size_t ve = 0; ve < other_vertex->edges.size(); ++ve) {
                                if(other_vertex->edges[ve] == e_ptr) {
                                    other_vertex->edges.erase(other_vertex->edges.begin() + ve);
                                    other_vertex->edge_nrs.erase(other_vertex->edge_nrs.begin() + ve);
                                    break;
                                }
                            }
                            
                            //Clear it from the sector lists.
                            for(size_t s = 0; s < 2; ++s) {
                                if(!e_ptr->sectors[s]) continue;
                                for(size_t se = 0; se < e_ptr->sectors[s]->edges.size(); ++se) {
                                    if(e_ptr->sectors[s]->edges[se] == e_ptr) {
                                        e_ptr->sectors[s]->edges.erase(e_ptr->sectors[s]->edges.begin() + se);
                                        e_ptr->sectors[s]->edge_nrs.erase(e_ptr->sectors[s]->edge_nrs.begin() + se);
                                        break;
                                    }
                                }
                            }
                            
                            //Clear it from the list of lone edges, if there.
                            auto it = lone_edges.find(e_ptr);
                            if(it != lone_edges.end()) lone_edges.erase(it);
                            
                            //Clear its info, so it gets marked for deletion.
                            e_ptr->vertex_nrs[0] = e_ptr->vertex_nrs[1] = string::npos;
                            e_ptr->fix_pointers(cur_area_data);
                            
                        } else {
                        
                            bool has_merged = false;
                            //Check if the edge will be merged with another one.
                            //These are edges that share a common vertex,
                            //plus the moved/destination vertex.
                            for(size_t de = 0; de < dest_v_ptr->edges.size(); ++de) {
                            
                                edge* de_ptr = dest_v_ptr->edges[de];
                                vertex* d_other_vertex = de_ptr->vertexes[0] == dest_v_ptr ? de_ptr->vertexes[1] : de_ptr->vertexes[0];
                                
                                if(d_other_vertex == other_vertex) {
                                    //The edge will be merged with this one.
                                    has_merged = true;
                                    affected_sectors.insert(e_ptr->sectors[0]);
                                    affected_sectors.insert(e_ptr->sectors[1]);
                                    affected_sectors.insert(de_ptr->sectors[0]);
                                    affected_sectors.insert(de_ptr->sectors[1]);
                                    
                                    //Tell the destination edge's sectors
                                    //to forget it; they'll be re-added later.
                                    size_t old_de_nr = de_ptr->remove_from_sectors();
                                    
                                    //Set the new sectors.
                                    //TODO if one of the central sectors is null.
                                    if(e_ptr->sector_nrs[0] == de_ptr->sector_nrs[0])
                                        de_ptr->sector_nrs[0] = e_ptr->sector_nrs[1];
                                    else if(e_ptr->sector_nrs[0] == de_ptr->sector_nrs[1])
                                        de_ptr->sector_nrs[1] = e_ptr->sector_nrs[1];
                                    else if(e_ptr->sector_nrs[1] == de_ptr->sector_nrs[0] || !e_ptr->sectors[0])
                                        de_ptr->sector_nrs[0] = e_ptr->sector_nrs[0];
                                    else if(e_ptr->sector_nrs[1] == de_ptr->sector_nrs[1] || !e_ptr->sectors[1])
                                        de_ptr->sector_nrs[1] = e_ptr->sector_nrs[0];
                                    de_ptr->fix_pointers(cur_area_data);
                                    
                                    //Go to the edge's old vertexes,
                                    //and tell them that it no longer exists.
                                    e_ptr->remove_from_vertexes();
                                    
                                    //Now tell the edge's old sectors.
                                    e_ptr->remove_from_sectors();
                                    
                                    //Add the edges to the sectors' lists.
                                    for(size_t s = 0; s < 2; ++s) {
                                        if(!de_ptr->sectors[s]) continue;
                                        de_ptr->sectors[s]->edges.push_back(de_ptr);
                                        de_ptr->sectors[s]->edge_nrs.push_back(old_de_nr);
                                    }
                                    
                                    //Remove the deleted edge's info.
                                    //This'll mark it for deletion.
                                    e_ptr->sector_nrs[0] = e_ptr->sector_nrs[1] = string::npos;
                                    e_ptr->vertex_nrs[0] = e_ptr->vertex_nrs[1] = string::npos;
                                    e_ptr->fix_pointers(cur_area_data);
                                    was_deleted = true;
                                    
                                    break;
                                }
                            }
                            
                            //If it's matchless, that means it'll just be joined to
                            //the group of edges on the destination vertex.
                            if(!has_merged) {
                                dest_v_ptr->edge_nrs.push_back(moved_v_ptr->edge_nrs[e]);
                                dest_v_ptr->edges.push_back(moved_v_ptr->edges[e]);
                                unsigned char n = (e_ptr->vertexes[0] == moved_v_ptr ? 0 : 1);
                                e_ptr->vertexes[n] = dest_v_ptr;
                                e_ptr->vertex_nrs[n] = v;
                            }
                        }
                        
                        if(!was_deleted) ++e;
                        
                    }
                    
                    dest_v_ptr->fix_pointers(cur_area_data);
                    
                    //Check if any of the final edges have the same sector
                    //on both sides. If so, delete them.
                    for(size_t ve = 0; ve < dest_v_ptr->edges.size(); ) {
                        edge* ve_ptr = dest_v_ptr->edges[ve];
                        if(ve_ptr->sectors[0] == ve_ptr->sectors[1]) {
                            ve_ptr->remove_from_sectors();
                            ve_ptr->remove_from_vertexes();
                            for(size_t v = 0; v < 2; ++v) {
                                if(ve_ptr->vertexes[v]->edges.empty()) {
                                    ve_ptr->vertexes[v]->x = ve_ptr->vertexes[v]->y = FLT_MAX;
                                }
                            }
                            ve_ptr->sector_nrs[0] = ve_ptr->sector_nrs[1] = string::npos;
                            ve_ptr->vertex_nrs[0] = ve_ptr->vertex_nrs[1] = string::npos;
                            ve_ptr->fix_pointers(cur_area_data);
                        } else {
                            ++ve;
                        }
                    }
                    
                    //If this vertex is out of edges, it'll be
                    //deleted eventually. Move it out of the way.
                    if(dest_v_ptr->edges.empty()) {
                        dest_v_ptr->x = dest_v_ptr->y = FLT_MAX;
                    }
                    
                    //Remove the old vertex' info.
                    //This'll mark it for deletion.
                    moved_v_ptr->edge_nrs.clear();
                    moved_v_ptr->edges.clear();
                    moved_v_ptr->x = moved_v_ptr->y = FLT_MAX; //So it's out of the way.
                    
                    final_vertex = dest_v_ptr;
                    
                    break;
                }
            }
            
            //Finally, re-triangulate the affected sectors.
            for(size_t e = 0; e < final_vertex->edges.size(); ++e) {
                edge* e_ptr = final_vertex->edges[e];
                for(size_t s = 0; s < 2; ++s) {
                    if(e_ptr->sectors[s]) affected_sectors.insert(e_ptr->sectors[s]);
                }
            }
            for(auto s = affected_sectors.begin(); s != affected_sectors.end(); ++s) {
                if(!(*s)) continue;
                triangulate(*s);
            }
            
            //If somewhere along the line, the current sector
            //got marked for deletion, unselect it.
            if(cur_sector) {
                if(cur_sector->edges.empty()) {
                    cur_sector = NULL;
                    sector_to_gui();
                }
            }
            
            //Check if the edge's vertexes intersect with any other edges.
            //If so, they're marked with red.
            if(moved_v_ptr->x != FLT_MAX) //If it didn't get marked for deletion in the meantime.
                check_edge_intersections(moved_v_ptr);
                
            moving_thing = string::npos;
            
            
            
        } else if(ev.mouse.button == 1 && sec_mode == ESM_NONE && moving_thing != string::npos) {
            //Release thing.
            
            moving_thing = string::npos;
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key press.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            shift_pressed = true;
            
        }
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //Key release.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            shift_pressed = false;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an edge is valid.
 * An edge is valid if it has non-NULL vertexes.
 */
bool area_editor::is_edge_valid(edge* l) {
    if(!l->vertexes[0]) return false;
    if(!l->vertexes[1]) return false;
    return true;
}

/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {

    fade_mgr.start_fade(true, nullptr);
    
    load_mob_types(false);
    
    mode = EDITOR_MODE_MAIN;
    
    lafi::style* s = new lafi::style(al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32), al_map_rgb(96, 128, 160));
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi::frame* frm_main = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add("lbl_area", new lafi::label(0, 0, 0, 0, "Area:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_area", new lafi::button(0, 0, 0, 0), 100, 32);
    int y = frm_main->easy_row();
    
    lafi::frame* frm_area = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    hide_widget(frm_area);
    frm_area->easy_row();
    frm_area->easy_add("but_sectors", new lafi::button(0, 0, 0, 0, "Edit sectors"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_objects", new lafi::button(0, 0, 0, 0, "Edit objects"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_paths", new lafi::button(0, 0, 0, 0, "Edit paths"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_shadows", new lafi::button(0, 0, 0, 0, "Edit shadows"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_guide", new lafi::button(0, 0, 0, 0, "Edit guide"), 100, 32);
    frm_area->easy_row();
    frm_area->easy_add("but_review", new lafi::button(0, 0, 0, 0, "Review"), 100, 32);
    frm_area->easy_row();
    
    
    //Bottom bar.
    lafi::frame* frm_bottom = new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_guide", new lafi::button(0, 0, 0, 0, "G"), 25, 32);
    frm_bottom->easy_add("but_load",  new lafi::button(0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save",  new lafi::button(0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit",  new lafi::button(0, 0, 0, 0, "X"), 25, 32);
    frm_bottom->easy_row();
    
    
    //Changes warning.
    lafi::frame* frm_changes = new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    gui->add("frm_changes", frm_changes);
    hide_widget(frm_changes);
    frm_changes->easy_row();
    frm_changes->easy_add("lbl_text1", new lafi::label(0, 0, 0, 0, "Warning: you have", ALLEGRO_ALIGN_LEFT), 80, 8);
    frm_changes->easy_row();
    frm_changes->easy_add("lbl_text2", new lafi::label(0, 0, 0, 0, "unsaved changes!", ALLEGRO_ALIGN_LEFT), 80, 8);
    frm_changes->easy_row();
    frm_changes->add("but_ok", new lafi::button(scr_w - 40, scr_h - 40, scr_w - 8, scr_h - 8, "Ok"));
    
    
    //Picker frame.
    lafi::frame* frm_picker = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add("but_back", new lafi::button(     scr_w - 200, 8, scr_w - 104, 24, "Back"));
    frm_picker->add("frm_list", new lafi::frame(      scr_w - 200, 40, scr_w - 32, scr_h - 56));
    frm_picker->add("bar_scroll", new lafi::scrollbar(scr_w - 24,  40, scr_w - 8,  scr_h - 56));
    
    
    //Sectors frame.
    lafi::frame* frm_sectors = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_sectors);
    gui->add("frm_sectors", frm_sectors);
    
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_sectors->easy_row();
    frm_sectors->easy_add("but_new", new lafi::button(0, 0, 0, 0, "+"), 20, 32);
    frm_sectors->easy_add("but_sel_none", new lafi::button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_sectors->easy_row();
    
    lafi::frame* frm_sector = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_sectors->add("frm_sector", frm_sector);
    hide_widget(frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_type", new lafi::label(0, 0, 0, 0, "Type:"), 30, 24);
    frm_sector->easy_add("but_type", new lafi::button(0, 0, 0, 0), 70, 24);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_z", new lafi::label(0, 0, 0, 0, "Height:"), 50, 16);
    frm_sector->easy_add("txt_z", new lafi::textbox(0, 0, 0, 0), 50, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("chk_fade", new lafi::checkbox(0, 0, 0, 0, "Fade textures"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_texture", new lafi::label(0, 0, 0, 0, "Texture:"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("txt_texture", new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("but_adv", new lafi::button(0, 0, 0, 0, "Adv. texture settings"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("chk_shadow", new lafi::checkbox(0, 0, 0, 0, "Always cast shadow"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lin_1", new lafi::line(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_brightness", new lafi::label(0, 0, 0, 0, "Brightness:"), 50, 16);
    frm_sector->easy_add("txt_brightness", new lafi::textbox(0, 0, 0, 0), 50, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_tag", new lafi::label(0, 0, 0, 0, "Tag:"), 20, 16);
    frm_sector->easy_add("txt_tag", new lafi::textbox(0, 0, 0, 0), 80, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("lbl_hazards", new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 16);
    frm_sector->easy_row();
    frm_sector->easy_add("txt_hazards", new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_sector->easy_row();
    
    
    //Advanced sector texture settings frame.
    lafi::frame* frm_adv_textures = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_adv_textures);
    gui->add("frm_adv_textures", frm_adv_textures);
    
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lin_1", new lafi::line(0, 0, 0, 0), 20, 16);
    frm_adv_textures->easy_add("lbl_main", new lafi::label(0, 0, 0, 0, "Main texture"), 60, 16);
    frm_adv_textures->easy_add("lin_2", new lafi::line(0, 0, 0, 0), 20, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_xy", new lafi::label(0, 0, 0, 0, "X&Y:"), 40, 16);
    frm_adv_textures->easy_add("txt_x", new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_add("txt_y", new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_sxy", new lafi::label(0, 0, 0, 0, "Scale:"), 40, 16);
    frm_adv_textures->easy_add("txt_sx", new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_add("txt_sy", new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add("lbl_a", new lafi::label(0, 0, 0, 0, "Angle:"), 50, 16);
    frm_adv_textures->easy_add("ang_a", new lafi::angle_picker(0, 0, 0, 0), 50, 24);
    frm_adv_textures->easy_row();
    
    
    //Objects frame.
    lafi::frame* frm_objects = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_objects);
    gui->add("frm_objects", frm_objects);
    
    frm_objects->easy_row();
    frm_objects->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_objects->easy_row();
    frm_objects->easy_add("but_new", new lafi::button(0, 0, 0, 0, "+"), 20, 32);
    frm_objects->easy_add("but_sel_none", new lafi::button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_objects->easy_row();
    
    lafi::frame* frm_object = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_objects->add("frm_object", frm_object);
    hide_widget(frm_object);
    
    frm_object->easy_row();
    frm_object->easy_add("lbl_category", new lafi::label(0, 0, 0, 0, "Category:"), 90, 16);
    frm_object->easy_add("but_rem", new lafi::button(0, 0, 0, 0, "-"), 10, 16);
    frm_object->easy_row();
    frm_object->easy_add("but_category", new lafi::button(0, 0, 0, 0), 100, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_type", new lafi::label(0, 0, 0, 0, "Type:"), 100, 16);
    frm_object->easy_row();
    frm_object->easy_add("but_type", new lafi::button(0, 0, 0, 0), 100, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_angle", new lafi::label(0, 0, 0, 0, "Angle:"), 50, 16);
    frm_object->easy_add("ang_angle", new lafi::angle_picker(0, 0, 0, 0), 50, 24);
    frm_object->easy_row();
    frm_object->easy_add("lbl_vars", new lafi::label(0, 0, 0, 0, "Script variables:"), 100, 16);
    frm_object->easy_row();
    frm_object->easy_add("txt_vars", new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_object->easy_row();
    
    
    //Paths frame.
    lafi::frame* frm_paths = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_paths);
    gui->add("frm_paths", frm_paths);
    
    frm_paths->easy_row();
    frm_paths->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_paths->easy_row();
    frm_paths->easy_add("lbl_create", new lafi::label(0, 0, 0, 0, "Create:"), 100, 16);
    frm_paths->easy_row();
    frm_paths->easy_add("but_new_stop", new lafi::button(0, 0, 0, 0, "Stop"), 33, 32);
    frm_paths->easy_add("but_new_link", new lafi::button(0, 0, 0, 0, "Link"), 33, 32);
    frm_paths->easy_add("but_new_1wlink", new lafi::button(0, 0, 0, 0, "1WLink"), 33, 32);
    frm_paths->easy_row();
    frm_paths->easy_add("lbl_delete", new lafi::label(0, 0, 0, 0, "Delete:"), 100, 16);
    frm_paths->easy_row();
    frm_paths->easy_add("but_del_stop", new lafi::button(0, 0, 0, 0, "Stop"), 33, 32);
    frm_paths->easy_add("but_del_link", new lafi::button(0, 0, 0, 0, "Link"), 33, 32);
    frm_paths->easy_row();
    frm_paths->easy_add("chk_show_closest", new lafi::checkbox(0, 0, 0, 0, "Show closest stop"), 100, 16);
    frm_paths->easy_row();
    
    
    //Shadows frame.
    lafi::frame* frm_shadows = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_shadows);
    gui->add("frm_shadows", frm_shadows);
    
    frm_shadows->easy_row();
    frm_shadows->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_shadows->easy_row();
    frm_shadows->easy_add("but_new", new lafi::button(0, 0, 0, 0, "+"), 20, 32);
    frm_shadows->easy_add("but_sel_none", new lafi::button(0, 0, 0, 0, "None"), 20, 32);
    y = frm_shadows->easy_row();
    
    lafi::frame* frm_shadow = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_shadows->add("frm_shadow", frm_shadow);
    hide_widget(frm_shadow);
    
    frm_shadow->easy_row();
    frm_shadow->easy_add("dum_1", new lafi::dummy(0, 0, 0, 0), 90, 16);
    frm_shadow->easy_add("but_rem", new lafi::button(0, 0, 0, 0, "-"), 10, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_file", new lafi::label(0, 0, 0, 0, "File:"), 20, 16);
    frm_shadow->easy_add("txt_file", new lafi::textbox(0, 0, 0, 0), 80, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_xy", new lafi::label(0, 0, 0, 0, "X&Y:"), 40, 16);
    frm_shadow->easy_add("txt_x",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_y",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_wh", new lafi::label(0, 0, 0, 0, "W&H:"), 40, 16);
    frm_shadow->easy_add("txt_w",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_h",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_an", new lafi::label(0, 0, 0, 0, "Angle:"), 40, 16);
    frm_shadow->easy_add("ang_an", new lafi::angle_picker(0, 0, 0, 0), 60, 24);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_al", new lafi::label(0, 0, 0, 0, "Opacity:"), 40, 16);
    frm_shadow->easy_row();
    frm_shadow->easy_add("bar_al", new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24);
    frm_shadow->easy_row();
    frm_shadow->easy_add("lbl_sway", new lafi::label(0, 0, 0, 0, "Sway X&Y:"), 40, 16);
    frm_shadow->easy_add("txt_sx",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_add("txt_sy",  new lafi::textbox(0, 0, 0, 0), 30, 16);
    frm_shadow->easy_row();
    
    
    //Guide frame.
    lafi::frame* frm_guide = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_guide);
    gui->add("frm_guide", frm_guide);
    
    frm_guide->easy_row();
    frm_guide->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("lbl_file", new lafi::label(0, 0, 0, 0, "File:"), 30, 16);
    frm_guide->easy_add("txt_file", new lafi::textbox(0, 0, 0, 0), 70, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("lbl_xy", new lafi::label(0, 0, 0, 0, "X&Y:"), 30, 16);
    frm_guide->easy_add("txt_x", new lafi::textbox(0, 0, 0, 0), 35, 16);
    frm_guide->easy_add("txt_y", new lafi::textbox(0, 0, 0, 0), 35, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("lbl_wh", new lafi::label(0, 0, 0, 0, "W&H:"), 30, 16);
    frm_guide->easy_add("txt_w", new lafi::textbox(0, 0, 0, 0), 35, 16);
    frm_guide->easy_add("txt_h", new lafi::textbox(0, 0, 0, 0), 35, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("chk_ratio", new lafi::checkbox(0, 0, 0, 0, "Keep aspect ratio"), 100, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("chk_mouse", new lafi::checkbox(0, 0, 0, 0, "Transform with mouse"), 100, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("lbl_alpha", new lafi::label(0, 0, 0, 0, "Opacity:"), 100, 16);
    frm_guide->easy_row();
    frm_guide->easy_add("bar_alpha", new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24);
    frm_guide->easy_row();
    
    
    //Review frame.
    lafi::frame* frm_review = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_review);
    gui->add("frm_review", frm_review);
    
    frm_review->easy_row();
    frm_review->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_review->easy_row();
    frm_review->easy_add("but_find_errors", new lafi::button(0, 0, 0, 0, "Find errors"), 100, 24);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_lbl", new lafi::label(0, 0, 0, 0, "Error found:", ALLEGRO_ALIGN_CENTER), 100, 16);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_1", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_2", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_3", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("lbl_error_4", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_review->easy_row();
    frm_review->easy_add("but_goto_error", new lafi::button(0, 0, 0, 0, "Go to error"), 100, 24);
    frm_review->easy_row();
    frm_review->easy_add("lin_1", new lafi::line(0, 0, 0, 0), 100, 16);
    frm_review->easy_row();
    frm_review->easy_add("chk_see_textures", new lafi::checkbox(0, 0, 0, 0, "See textures"), 100, 16);
    frm_review->easy_row();
    frm_review->easy_add("dum_1", new lafi::dummy(0, 0, 0, 0), 10, 16);
    frm_review->easy_add("chk_shadows", new lafi::checkbox(0, 0, 0, 0, "See tree shadows"), 90, 16);
    frm_review->easy_row();
    update_review_frame();
    
    
    //Status bar.
    lafi::label* gui_status_bar = new lafi::label(0, scr_h - 16, scr_w - 208, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Properties -- main.
    frm_main->widgets["but_area"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_AREA);
    };
    frm_area->widgets["but_sectors"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_area->widgets["but_objects"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_OBJECTS;
        change_to_right_frame();
    };
    frm_area->widgets["but_paths"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_PATHS;
        change_to_right_frame();
    };
    frm_area->widgets["but_shadows"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SHADOWS;
        change_to_right_frame();
    };
    frm_area->widgets["but_guide"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_GUIDE;
        change_to_right_frame();
    };
    frm_area->widgets["but_review"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_REVIEW;
        change_to_right_frame();
        update_review_frame();
    };
    frm_main->widgets["but_area"]->description =    "Pick the area to edit.";
    frm_area->widgets["but_sectors"]->description = "Change sector (polygon) settings.";
    frm_area->widgets["but_objects"]->description = "Change object settings and placements.";
    frm_area->widgets["but_paths"]->description =   "Change movement paths and stops.";
    frm_area->widgets["but_shadows"]->description = "Change the shadows of trees and leaves.";
    frm_area->widgets["but_guide"]->description =   "Manage the guide image.";
    frm_area->widgets["but_review"]->description =  "Tools to make sure everything is fine in the area.";
    
    
    //Properties -- bottom.
    frm_bottom->widgets["but_guide"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_guide = !show_guide;
    };
    frm_bottom->widgets["but_load"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            this->load_area();
        }
    };
    frm_bottom->widgets["but_save"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        save_area();
    };
    frm_bottom->widgets["but_quit"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            leave();
        }
    };
    disable_widget(frm_bottom->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
    frm_bottom->widgets["but_guide"]->description = "Toggle the visibility of the guide.";
    frm_bottom->widgets["but_load"]->description =  "Load the area from the files.";
    frm_bottom->widgets["but_save"]->description =  "Save the area onto the files.";
    frm_bottom->widgets["but_quit"]->description =  "Quit the area editor.";
    
    
    //Properties -- changes warning.
    frm_changes->widgets["but_ok"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) { close_changes_warning(); };
    
    
    //Properties -- sectors.
    auto lambda_gui_to_sector = [this] (lafi::widget*) { gui_to_sector(); };
    auto lambda_gui_to_sector_click = [this] (lafi::widget*, int, int) { gui_to_sector(); };
    frm_sectors->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_sectors->widgets["but_new"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_SECTOR) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_SECTOR;
    };
    frm_sectors->widgets["but_sel_none"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        cur_sector = NULL;
        sector_to_gui();
    };
    frm_sector->widgets["but_type"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_SECTOR_TYPE);
    };
    frm_sector->widgets["but_adv"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(!cur_sector) return;
        
        cur_sector->texture_info.bitmap = bitmaps.get("Textures/" + cur_sector->texture_info.file_name, NULL);
        
        mode = EDITOR_MODE_ADV_TEXTURE_SETTINGS;
        change_to_right_frame();
        adv_textures_to_gui();
    };
    frm_sector->widgets["txt_z"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["chk_fade"]->left_mouse_click_handler = lambda_gui_to_sector_click;
    frm_sector->widgets["txt_texture"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_brightness"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_tag"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_to_sector;
    frm_sector->widgets["chk_shadow"]->left_mouse_click_handler = lambda_gui_to_sector_click;
    frm_sectors->widgets["but_back"]->description =      "Go back to the main menu.";
    frm_sectors->widgets["but_new"]->description =       "Create a new sector where you click.";
    frm_sectors->widgets["but_sel_none"]->description =  "Deselect the current sector.";
    frm_sector->widgets["but_type"]->description =       "Change the type of sector.";
    frm_sector->widgets["chk_fade"]->description =       "Makes the surrounding textures fade into each other.";
    frm_sector->widgets["txt_z"]->description =          "Height of the floor.";
    frm_sector->widgets["txt_texture"]->description =    "File name of the Texture (image) of the floor.";
    frm_sector->widgets["txt_brightness"]->description = "0 = pitch black sector. 255 = normal lighting.";
    frm_sector->widgets["txt_tag"]->description =        "Special values you may want the sector to know.";
    frm_sector->widgets["txt_hazards"]->description =    "Hazards the sector has.";
    frm_sector->widgets["but_adv"]->description =        "Advanced settings for the sector's texture.";
    frm_sector->widgets["chk_shadow"]->description =     "Makes this sector always cast a shadow onto lower sectors.";
    
    
    //Properties -- advanced textures.
    auto lambda_gui_to_adv_textures = [this] (lafi::widget*) { gui_to_adv_textures(); };
    frm_adv_textures->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        clear_area_textures(); //Clears the texture set when we entered this menu.
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_adv_textures->widgets["txt_x"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_y"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sx"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sy"]->lose_focus_handler = lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_x"]->description =  "Scroll the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_y"]->description =  "Scroll the texture vertically by this much.";
    frm_adv_textures->widgets["txt_sx"]->description = "Zoom the texture horizontally by this much.";
    frm_adv_textures->widgets["txt_sy"]->description = "Zoom the texture vertically by this much.";
    frm_adv_textures->widgets["ang_a"]->description =  "Rotate the texture by this much.";
    
    
    //Properties -- objects.
    auto lambda_gui_to_mob = [this] (lafi::widget*) { gui_to_mob(); };
    frm_objects->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_objects->widgets["but_new"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_OBJECT) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_OBJECT;
    };
    frm_objects->widgets["but_sel_none"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        cur_mob = NULL;
        mob_to_gui();
    };
    frm_object->widgets["but_rem"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            if(cur_area_data.mob_generators[m] == cur_mob) {
                cur_area_data.mob_generators.erase(cur_area_data.mob_generators.begin() + m);
                delete cur_mob;
                cur_mob = NULL;
                mob_to_gui();
                break;
            }
        }
    };
    frm_object->widgets["but_category"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_CATEGORY);
    };
    frm_object->widgets["but_type"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
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
    
    
    //Properties -- paths.
    frm_paths->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_paths->widgets["but_new_stop"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_STOP) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_STOP;
    };
    frm_paths->widgets["but_new_link"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_LINK1 || sec_mode == ESM_NEW_LINK2) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_LINK1;
    };
    frm_paths->widgets["but_new_1wlink"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_1WLINK1 || sec_mode == ESM_NEW_1WLINK2) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_1WLINK1;
    };
    frm_paths->widgets["but_del_stop"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_DEL_STOP) sec_mode = ESM_NONE;
        else sec_mode = ESM_DEL_STOP;
    };
    frm_paths->widgets["but_del_link"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_DEL_LINK) sec_mode = ESM_NONE;
        else sec_mode = ESM_DEL_LINK;
    };
    frm_paths->widgets["chk_show_closest"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_closest_stop = !show_closest_stop;
    };
    frm_paths->widgets["but_back"]->description = "Go back to the main menu.";
    frm_paths->widgets["but_new_stop"]->description = "Create new stops wherever you click.";
    frm_paths->widgets["but_new_link"]->description = "Click on two stops to connect them with a link.";
    frm_paths->widgets["but_new_1wlink"]->description = "Click stop #1 then #2 for a one-way path link.";
    frm_paths->widgets["but_del_stop"]->description = "Click stops to delete them.";
    frm_paths->widgets["but_del_link"]->description = "Click links to delete them.";
    frm_paths->widgets["chk_show_closest"]->description = "Show the closest stop to the cursor.";
    
    
    //Properties -- shadows.
    auto lambda_gui_to_shadow = [this] (lafi::widget*) { gui_to_shadow(); };
    frm_shadows->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        sec_mode = ESM_NONE;
        shadow_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_shadows->widgets["but_new"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_SHADOW) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_SHADOW;
    };
    frm_shadows->widgets["but_sel_none"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        cur_shadow = NULL;
        shadow_to_gui();
    };
    frm_shadow->widgets["but_rem"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            if(cur_area_data.tree_shadows[s] == cur_shadow) {
                cur_area_data.tree_shadows.erase(cur_area_data.tree_shadows.begin() + s);
                delete cur_shadow;
                cur_shadow = NULL;
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
    
    
    //Properties -- guide.
    auto lambda_gui_to_guide = [this] (lafi::widget*) { gui_to_guide(); };
    auto lambda_gui_to_guide_click = [this] (lafi::widget*, int, int) { gui_to_guide(); };
    frm_guide->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        sec_mode = ESM_NONE;
        guide_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_guide->widgets["txt_file"]->lose_focus_handler = lambda_gui_to_guide;
    frm_guide->widgets["txt_x"]->lose_focus_handler = lambda_gui_to_guide;
    frm_guide->widgets["txt_y"]->lose_focus_handler = lambda_gui_to_guide;
    frm_guide->widgets["txt_w"]->lose_focus_handler = lambda_gui_to_guide;
    frm_guide->widgets["txt_h"]->lose_focus_handler = lambda_gui_to_guide;
    ((lafi::scrollbar*) frm_guide->widgets["bar_alpha"])->change_handler = lambda_gui_to_guide;
    frm_guide->widgets["chk_ratio"]->left_mouse_click_handler = lambda_gui_to_guide_click;
    frm_guide->widgets["chk_mouse"]->left_mouse_click_handler = lambda_gui_to_guide_click;
    frm_guide->widgets["but_back"]->description = "Go back to the main menu.";
    frm_guide->widgets["txt_file"]->description = "Image file (on the Images folder) for the guide.";
    frm_guide->widgets["txt_x"]->description = "X of the top-left corner for the guide.";
    frm_guide->widgets["txt_y"]->description = "Y of the top-left corner for the guide.";
    frm_guide->widgets["txt_w"]->description = "Guide total width.";
    frm_guide->widgets["txt_h"]->description = "Guide total height.";
    frm_guide->widgets["bar_alpha"]->description = "How see-through the guide is.";
    frm_guide->widgets["chk_ratio"]->description = "Lock the width/height proportions when changing either one.";
    frm_guide->widgets["chk_mouse"]->description = "If checked, use left/right mouse button to move/stretch.";
    guide_to_gui();
    
    
    //Properties -- review.
    frm_review->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        sec_mode = ESM_NONE;
        error_type = EET_NONE_YET;
        update_review_frame();
        change_to_right_frame();
    };
    frm_review->widgets["but_find_errors"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        find_errors();
    };
    frm_review->widgets["but_goto_error"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        goto_error();
    };
    frm_review->widgets["chk_see_textures"]->left_mouse_click_handler = [this] (lafi::widget * c, int, int) {
        error_type = EET_NONE_YET;
        if(((lafi::checkbox*) c)->checked) {
            sec_mode = ESM_TEXTURE_VIEW;
            clear_area_textures();
            load_area_textures();
            update_review_frame();
            
        } else {
            sec_mode = ESM_NONE;
            update_review_frame();
        }
    };
    frm_review->widgets["chk_shadows"]->left_mouse_click_handler = [this] (lafi::widget * c, int, int) {
        show_shadows = ((lafi::checkbox*) c)->checked;
        update_review_frame();
    };
    frm_review->widgets["but_back"]->description =         "Go back to the main menu.";
    frm_review->widgets["but_find_errors"]->description =  "Search for problems with the area.";
    frm_review->widgets["but_goto_error"]->description =   "Focus the camera on the problem found, if applicable.";
    frm_review->widgets["chk_see_textures"]->description = "Preview how the textures will look like.";
    frm_review->widgets["chk_shadows"]->description =      "Show tree shadows?";
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_bottom"]);
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description = "Cancel.";
    
    
    cam_zoom = 1.0;
    cam_x = cam_y = 0.0;
    show_closest_stop = false;
    area_name.clear();
    
}


/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 */
void area_editor::load_area() {
    intersecting_edges.clear();
    non_simples.clear();
    lone_edges.clear();
    
    ::load_area(area_name, true);
    ((lafi::button*) gui->widgets["frm_main"]->widgets["but_area"])->text = area_name;
    show_widget(gui->widgets["frm_main"]->widgets["frm_area"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_load"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_save"]);
    
    clear_area_textures();
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        check_edge_intersections(cur_area_data.vertexes[v]);
    }
    
    change_guide(guide_file_name);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
    
    error_type = EET_NONE_YET;
    error_sector_ptr = NULL;
    error_string.clear();
    error_vertex_ptr = NULL;
    
    cur_sector = NULL;
    cur_mob = NULL;
    cur_shadow = NULL;
    sector_to_gui();
    mob_to_gui();
    guide_to_gui();
    
    made_changes = false;
    
    mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Loads the current mob's data onto the gui.
 */
void area_editor::mob_to_gui() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_objects"]->widgets["frm_object"];
    
    if(!cur_mob) {
        hide_widget(f);
    } else {
        show_widget(f);
        
        ((lafi::angle_picker*) f->widgets["ang_angle"])->set_angle_rads(cur_mob->angle);
        ((lafi::textbox*) f->widgets["txt_vars"])->text = cur_mob->vars;
        
        ((lafi::button*) f->widgets["but_category"])->text = mob_categories.get_pname(cur_mob->category);
        
        lafi::button* but_type = (lafi::button*) f->widgets["but_type"];
        if(cur_mob->category == MOB_CATEGORY_NONE) {
            disable_widget(but_type);
        } else {
            enable_widget(but_type);
        }
        but_type->text = cur_mob->type ? cur_mob->type->name : "";
    }
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type, use area_editor::AREA_EDITOR_PICKER_*.
 */
void area_editor::open_picker(unsigned char type) {
    change_to_right_frame(true);
    show_widget(gui->widgets["frm_picker"]);
    hide_widget(gui->widgets["frm_bottom"]);
    
    lafi::widget* f = gui->widgets["frm_picker"]->widgets["frm_list"];
    
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    vector<string> elements;
    if(type == AREA_EDITOR_PICKER_AREA) {
        elements = folder_to_vector(AREA_FOLDER, true);
        for(size_t e = 0; e < elements.size(); ++e) {
            size_t pos = elements[e].find(".txt");
            if(pos != string::npos) {
                elements[e].erase(pos, 4);
            }
        }
        
    } else if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
    
        for(size_t t = 0; t < sector_types.get_nr_of_types(); ++t) {
            elements.push_back(sector_types.get_name(t));
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_CATEGORY) {
    
        for(unsigned char f = 0; f < mob_categories.get_nr_of_categories(); ++f) { //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            elements.push_back(mob_categories.get_pname(f));
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        if(cur_mob->category != MOB_CATEGORY_NONE) {
            mob_categories.get_list(elements, cur_mob->category);
        }
        
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); ++e) {
        lafi::button* b = new lafi::button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler = [name, type, this] (lafi::widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    ((lafi::scrollbar*) gui->widgets["frm_picker"]->widgets["bar_scroll"])->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void area_editor::pick(string name, unsigned char type) {
    change_to_right_frame();
    show_widget(gui->widgets["frm_bottom"]);
    
    if(type == AREA_EDITOR_PICKER_AREA) {
    
        area_name = name;
        load_area();
        
    } else if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
    
        if(cur_sector) {
            cur_sector->type = sector_types.get_nr(name);
            sector_to_gui();
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_CATEGORY) {
    
        if(cur_mob) {
            cur_mob->category = mob_categories.get_nr_from_pname(name);
            cur_mob->type = NULL;
            mob_to_gui();
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        if(cur_mob) {
            mob_categories.set_mob_type_ptr(cur_mob, name);
        }
        
        mob_to_gui();
        
    }
}


/* ----------------------------------------------------------------------------
 * Saves the area onto the disk.
 */
void area_editor::save_area() {
    data_node geometry_file = data_node("", "");
    
    //Start by cleaning unused vertex, sector, etc. ids.
    //Unused vertex ids.
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ) {
    
        vertex* v_ptr = cur_area_data.vertexes[v];
        if(v_ptr->edge_nrs.empty()) {
        
            cur_area_data.vertexes.erase(cur_area_data.vertexes.begin() + v);
            
            //Fix numbers in edge lists.
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                for(unsigned char ev = 0; ev < 2; ++ev) {
                    if(e_ptr->vertex_nrs[ev] >= v && e_ptr->vertex_nrs[ev] != string::npos) {
                        e_ptr->vertex_nrs[ev]--;
                    }
                }
            }
            
        } else {
            ++v;
        }
    }
    
    //Unused sector ids.
    for(size_t s = 0; s < cur_area_data.sectors.size(); ) {
    
        sector* s_ptr = cur_area_data.sectors[s];
        if(s_ptr->edge_nrs.empty()) {
        
            cur_area_data.sectors.erase(cur_area_data.sectors.begin() + s);
            
            //Fix numbers in edge lists.
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                for(unsigned char es = 0; es < 2; ++es) {
                    if(e_ptr->sector_nrs[es] >= s && e_ptr->sector_nrs[es] != string::npos) {
                        e_ptr->sector_nrs[es]--;
                    }
                }
            }
            
        } else {
            ++s;
        }
    }
    
    //Unused edge ids.
    for(size_t e = 0; e < cur_area_data.edges.size(); ) {
    
        edge* e_ptr = cur_area_data.edges[e];
        if(e_ptr->vertex_nrs[0] == string::npos) {
        
            cur_area_data.edges.erase(cur_area_data.edges.begin() + e);
            
            //Fix numbers in vertex lists.
            for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                vertex* v_ptr = cur_area_data.vertexes[v];
                for(size_t ve = 0; ve < v_ptr->edge_nrs.size(); ++ve) {
                    if(v_ptr->edge_nrs[ve] >= e && v_ptr->edge_nrs[ve] != string::npos) {
                        --v_ptr->edge_nrs[ve];
                    }
                }
            }
            
            //Fix numbers in sector lists.
            for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
                sector* s_ptr = cur_area_data.sectors[s];
                for(size_t se = 0; se < s_ptr->edge_nrs.size(); ++se) {
                    if(s_ptr->edge_nrs[se] >= e && s_ptr->edge_nrs[se] != string::npos) {
                        s_ptr->edge_nrs[se]--;
                    }
                }
            }
            
        } else {
            ++e;
        }
    }
    
    
    //Save the content now.
    //Vertexes.
    data_node* vertexes_node = new data_node("vertexes", "");
    geometry_file.add(vertexes_node);
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        data_node* vertex_node = new data_node("v", f2s(v_ptr->x) + " " + f2s(v_ptr->y));
        vertexes_node->add(vertex_node);
    }
    
    //Edges.
    data_node* edges_node = new data_node("edges", "");
    geometry_file.add(edges_node);
    
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        data_node* edge_node = new data_node("e", "");
        edges_node->add(edge_node);
        string s_str;
        for(size_t s = 0; s < 2; ++s) {
            if(e_ptr->sector_nrs[s] == string::npos) s_str += "-1";
            else s_str += i2s(e_ptr->sector_nrs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        edge_node->add(new data_node("s", s_str));
        edge_node->add(new data_node("v", i2s(e_ptr->vertex_nrs[0]) + " " + i2s(e_ptr->vertex_nrs[1])));
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    geometry_file.add(sectors_node);
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        data_node* sector_node = new data_node("s", "");
        sectors_node->add(sector_node);
        
        if(s_ptr->type != SECTOR_TYPE_NORMAL) sector_node->add(new data_node("type", sector_types.get_name(s_ptr->type)));
        sector_node->add(new data_node("z", f2s(s_ptr->z)));
        if(s_ptr->brightness != DEF_SECTOR_BRIGHTNESS) sector_node->add(new data_node("brightness", i2s(s_ptr->brightness)));
        if(!s_ptr->tag.empty()) sector_node->add(new data_node("tag", s_ptr->tag));
        if(s_ptr->fade) sector_node->add(new data_node("fade", b2s(s_ptr->fade)));
        if(s_ptr->always_cast_shadow) sector_node->add(new data_node("always_cast_shadow", b2s(s_ptr->always_cast_shadow)));
        
        
        sector_node->add(new data_node("texture", s_ptr->texture_info.file_name));
        if(s_ptr->texture_info.rot != 0) {
            sector_node->add(new data_node("texture_rotate", f2s(s_ptr->texture_info.rot)));
        }
        if(s_ptr->texture_info.scale_x != 1 || s_ptr->texture_info.scale_y != 1) {
            sector_node->add(new data_node("texture_scale",
                                           f2s(s_ptr->texture_info.scale_x) + " " + f2s(s_ptr->texture_info.scale_y)));
        }
        if(s_ptr->texture_info.trans_x != 0 || s_ptr->texture_info.trans_y != 0) {
            sector_node->add(new data_node("texture_trans",
                                           f2s(s_ptr->texture_info.trans_x) + " " + f2s(s_ptr->texture_info.trans_y)));
        }
    }
    
    //Mobs.
    data_node* mobs_node = new data_node("mobs", "");
    geometry_file.add(mobs_node);
    
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        data_node* mob_node = new data_node(mob_categories.get_sname(m_ptr->category), "");
        mobs_node->add(mob_node);
        
        if(m_ptr->type) {
            mob_node->add(
                new data_node("type", m_ptr->type->name)
            );
        }
        mob_node->add(
            new data_node(
                "p",
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
    
    //Path stops.
    data_node* path_stops_node = new data_node("path_stops", "");
    geometry_file.add(path_stops_node);
    
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        data_node* path_stop_node = new data_node("s", "");
        path_stops_node->add(path_stop_node);
        
        path_stop_node->add(new data_node("pos", f2s(s_ptr->x) + " " + f2s(s_ptr->y)));
        
        data_node* links_node = new data_node("links", "");
        path_stop_node->add(links_node);
        
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            path_link* l_ptr = &s_ptr->links[l];
            data_node* link_node = new data_node("l", "");
            links_node->add(link_node);
            
            link_node->add(new data_node("nr", i2s(l_ptr->end_nr)));
            link_node->add(new data_node("1w", b2s(l_ptr->one_way)));
        }
        
    }
    
    //Tree shadows.
    data_node* shadows_node = new data_node("tree_shadows", "");
    geometry_file.add(shadows_node);
    
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
        data_node* shadow_node = new data_node("shadow", "");
        shadows_node->add(shadow_node);
        
        shadow_node->add(new data_node("pos", f2s(s_ptr->x) + " " + f2s(s_ptr->y)));
        shadow_node->add(new data_node("size", f2s(s_ptr->w) + " " + f2s(s_ptr->h)));
        if(s_ptr->angle != 0) shadow_node->add(new data_node("angle", f2s(s_ptr->angle)));
        if(s_ptr->alpha != 255) shadow_node->add(new data_node("alpha", i2s(s_ptr->alpha)));
        shadow_node->add(new data_node("file", s_ptr->file_name));
        shadow_node->add(new data_node("sway", f2s(s_ptr->sway_x) + " " + f2s(s_ptr->sway_y)));
        
    }
    
    //Editor guide.
    geometry_file.add(new data_node("guide_file_name", guide_file_name));
    geometry_file.add(new data_node("guide_x",         f2s(guide_x)));
    geometry_file.add(new data_node("guide_y",         f2s(guide_y)));
    geometry_file.add(new data_node("guide_w",         f2s(guide_w)));
    geometry_file.add(new data_node("guide_h",         f2s(guide_h)));
    geometry_file.add(new data_node("guide_alpha",     i2s(guide_a)));
    
    
    geometry_file.save_file(AREA_FOLDER + "/" + area_name + "/Geometry.txt");
    
    cur_sector = NULL;
    cur_mob = NULL;
    sector_to_gui();
    mob_to_gui();
    mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
    
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Loads the current sector's data onto the gui.
 */
void area_editor::sector_to_gui() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_sectors"]->widgets["frm_sector"];
    if(cur_sector) {
        show_widget(f);
        
        ((lafi::textbox*) f->widgets["txt_z"])->text = f2s(cur_sector->z);
        ((lafi::checkbox*) f->widgets["chk_fade"])->set(cur_sector->fade);
        ((lafi::checkbox*) f->widgets["chk_shadow"])->set(cur_sector->always_cast_shadow);
        ((lafi::textbox*) f->widgets["txt_texture"])->text = cur_sector->texture_info.file_name;
        ((lafi::textbox*) f->widgets["txt_brightness"])->text = i2s(cur_sector->brightness);
        ((lafi::textbox*) f->widgets["txt_tag"])->text = cur_sector->tag;
        ((lafi::button*) f->widgets["but_type"])->text = sector_types.get_name(cur_sector->type);
        //TODO hazards.
        
        if(cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            disable_widget(f->widgets["chk_fade"]);
        } else {
            enable_widget(f->widgets["chk_fade"]);
        }
        
        if(cur_sector->fade || cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
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
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_shadows"]->widgets["frm_shadow"];
    if(cur_shadow) {
    
        show_widget(f);
        ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_shadow->x);
        ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_shadow->y);
        ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(cur_shadow->w);
        ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_shadow->h);
        ((lafi::angle_picker*) f->widgets["ang_an"])->set_angle_rads(cur_shadow->angle);
        ((lafi::scrollbar*) f->widgets["bar_al"])->set_value(cur_shadow->alpha, false);
        ((lafi::textbox*) f->widgets["txt_file"])->text = cur_shadow->file_name;
        ((lafi::textbox*) f->widgets["txt_sx"])->text = f2s(cur_shadow->sway_x);
        ((lafi::textbox*) f->widgets["txt_sy"])->text = f2s(cur_shadow->sway_y);
        
    } else {
        hide_widget(f);
    }
}


/* ----------------------------------------------------------------------------
 * Shows the "unsaved changes" warning.
 */
void area_editor::show_changes_warning() {
    show_widget(gui->widgets["frm_changes"]);
    hide_widget(gui->widgets["frm_bottom"]);
    
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Snaps a coordinate to the nearest grid space.
 */
float area_editor::snap_to_grid(const float c) {
    if(shift_pressed) return c;
    return round(c / GRID_INTERVAL) * GRID_INTERVAL;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void area_editor::unload() {
    //TODO
    cur_area_data.clear();
    delete(gui->style);
    delete(gui);
}


/* ----------------------------------------------------------------------------
 * Updates the widgets on the review frame.
 */
void area_editor::update_review_frame() {
    lafi::button* but_goto_error = (lafi::button*) gui->widgets["frm_review"]->widgets["but_goto_error"];
    lafi::label* lbl_error_1 = (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_1"];
    lafi::label* lbl_error_2 = (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_2"];
    lafi::label* lbl_error_3 = (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_3"];
    lafi::label* lbl_error_4 = (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_4"];
    
    lbl_error_2->text.clear();
    lbl_error_3->text.clear();
    lbl_error_4->text.clear();
    
    if(sec_mode == ESM_TEXTURE_VIEW) {
        disable_widget(gui->widgets["frm_review"]->widgets["but_find_errors"]);
        disable_widget(gui->widgets["frm_review"]->widgets["but_goto_error"]);
    } else {
        enable_widget(gui->widgets["frm_review"]->widgets["but_find_errors"]);
        enable_widget(gui->widgets["frm_review"]->widgets["but_goto_error"]);
    }
    
    if(error_type == EET_NONE_YET || error_type == EET_NONE) {
        disable_widget(but_goto_error);
        if(error_type == EET_NONE_YET) {
            lbl_error_1->text = "---";
        } else {
            lbl_error_1->text = "No errors found.";
        }
        
    } else {
        if(error_type == EET_INTERSECTING_EDGES) {
        
            if(intersecting_edges.empty()) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Two edges cross";
            lbl_error_2->text = "each other, at";
            float u;
            edge_intersection* ei_ptr = &intersecting_edges[0];
            lines_intersect(
                ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y,
                ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y,
                ei_ptr->e2->vertexes[0]->x, ei_ptr->e2->vertexes[0]->y,
                ei_ptr->e2->vertexes[1]->x, ei_ptr->e2->vertexes[1]->y,
                NULL, &u
            );
            
            float a = atan2(
                          ei_ptr->e1->vertexes[1]->y - ei_ptr->e1->vertexes[0]->y,
                          ei_ptr->e1->vertexes[1]->x - ei_ptr->e1->vertexes[0]->x
                      );
            dist d(
                ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y,
                ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y
            );
            
            lbl_error_3->text = "(" + f2s(floor(ei_ptr->e1->vertexes[0]->x + cos(a) * u * d.to_float())) +
                                "," + f2s(floor(ei_ptr->e1->vertexes[0]->y + sin(a) * u * d.to_float())) + ")!";
                                
        } else if(error_type == EET_BAD_SECTOR) {
        
            if(non_simples.empty()) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Non-simple sector";
            lbl_error_2->text = "found! (Does the";
            lbl_error_3->text = "sector contain";
            lbl_error_4->text = "itself?)";
            
        } else if(error_type == EET_LONE_EDGE) {
        
            if(lone_edges.empty()) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Lone edge found!";
            lbl_error_2->text = "You probably want";
            lbl_error_3->text = "to drag one vertex";
            lbl_error_4->text = "to the other.";
            
        } else if(error_type == EET_OVERLAPPING_VERTEXES) {
        
            if(!error_vertex_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Overlapping vertexes";
            lbl_error_2->text =
                "at (" + f2s(error_vertex_ptr->x) + "," +
                f2s(error_vertex_ptr->y) + ")!";
            lbl_error_3->text = "(Drag one of them";
            lbl_error_3->text = "into the other)";
            
        } else if(error_type == EET_MISSING_TEXTURE) {
        
            if(!error_sector_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Sector without";
            lbl_error_2->text = "texture found!";
            
        } else if(error_type == EET_UNKNOWN_TEXTURE) {
        
            if(!error_sector_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Sector with unknown";
            lbl_error_2->text = "texture found!";
            lbl_error_3->text = "(" + error_string + ")";
            
        } else if(error_type == EET_TYPELESS_MOB) {
        
            if(!error_mob_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Mob with no";
            lbl_error_2->text = "type found!";
            
            
        } else if(error_type == EET_MOB_OOB) {
        
            if(!error_mob_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Mob that is not";
            lbl_error_2->text = "on any sector";
            lbl_error_3->text = "found! It's probably";
            lbl_error_4->text = "out of bounds.";
            
            
        } else if(error_type == EET_MOB_IN_WALL) {
        
            if(!error_mob_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Mob stuck";
            lbl_error_2->text = "in wall found!";
            
            
        } else if(error_type == EET_LONE_PATH_STOP) {
        
            if(!error_path_stop_ptr) {
                find_errors(); return;
            }
            
            lbl_error_1->text = "Lone path stop";
            lbl_error_2->text = "found!";
            
        } else if(error_type == EET_PATHS_UNCONNECTED) {
        
            disable_widget(but_goto_error);
            lbl_error_1->text = "The path is";
            lbl_error_2->text = "split into two";
            lbl_error_3->text = "or more parts!";
            lbl_error_4->text = "Connect them.";
            
        } else if(error_type == EET_PATH_STOPS_TOGETHER) {
        
            lbl_error_1->text = "Two path stops";
            lbl_error_2->text = "found close";
            lbl_error_3->text = "together!";
            lbl_error_4->text = "Separate them.";
            
        } else if(error_type == EET_PATH_STOP_OOB) {
        
            lbl_error_1->text = "Path stop out";
            lbl_error_2->text = "of bounds found!";
            
        } else if(error_type == EET_INVALID_SHADOW) {
        
            lbl_error_1->text = "Tree shadow with";
            lbl_error_2->text = "invalid image found!";
            
        }
    }
    
    ((lafi::checkbox*) gui->widgets["frm_review"]->widgets["chk_see_textures"])->set(sec_mode == ESM_TEXTURE_VIEW);
    ((lafi::checkbox*) gui->widgets["frm_review"]->widgets["chk_shadows"])->set(show_shadows);
}


void area_editor::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


void area_editor::set_guide_file_name(string n) {
    guide_file_name = n;
}

void area_editor::set_guide_x(float x) {
    guide_x = x;
}

void area_editor::set_guide_y(float y) {
    guide_y = y;
}

void area_editor::set_guide_w(float w) {
    guide_w = w;
}

void area_editor::set_guide_h(float h) {
    guide_h = h;
}

void area_editor::set_guide_a(unsigned char a) {
    guide_a = a;
}
