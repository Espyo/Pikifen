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
#include "LAFI/const.h"
#include "LAFI/frame.h"
#include "LAFI/gui.h"
#include "LAFI/image.h"
#include "LAFI/minor.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "vars.h"


const float  area_editor::DEF_GRID_INTERVAL = 32.0f;
const float  area_editor::MAX_GRID_INTERVAL = 4096;
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
const float  area_editor::MIN_GRID_INTERVAL = 2;
const float  area_editor::PATH_LINK_THICKNESS = 2.0f;
const float  area_editor::PATH_PREVIEW_CHECKPOINT_RADIUS = 8.0f;
const float  area_editor::PATH_PREVIEW_TIMEOUT_DUR = 0.1f;
const float  area_editor::STOP_RADIUS = 16.0f;
const float  area_editor::VERTEX_MERGE_RADIUS = 10.0f;


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
    backup_timer(editor_backup_interval),
    cur_mob(NULL),
    cur_sector(NULL),
    cur_shadow(NULL),
    debug_edge_nrs(false),
    debug_sector_nrs(false),
    debug_triangulation(false),
    debug_vertex_nrs(false),
    double_click_time(0),
    error_mob_ptr(NULL),
    error_path_stop_ptr(NULL),
    error_sector_ptr(NULL),
    error_shadow_ptr(NULL),
    error_type(area_editor::EET_NONE_YET),
    error_vertex_ptr(NULL),
    grid_interval(DEF_GRID_INTERVAL),
    gui(NULL),
    holding_m1(false),
    holding_m2(false),
    made_changes(false),
    mode(EDITOR_MODE_MAIN),
    mode_before_options(EDITOR_MODE_MAIN),
    moving_path_preview_checkpoint(-1),
    moving_thing(INVALID),
    moving_thing_x(0),
    moving_thing_y(0),
    new_link_first_stop(NULL),
    new_sector_valid_line(false),
    on_sector(NULL),
    path_preview_timeout(0),
    sec_mode(ESM_NONE),
    shift_pressed(false),
    show_closest_stop(false),
    show_guide(false),
    show_path_preview(false),
    show_shadows(true),
    wum(NULL) {
    
    path_preview_checkpoints_x[0] = -DEF_GRID_INTERVAL;
    path_preview_checkpoints_y[0] = 0;
    path_preview_checkpoints_x[1] = DEF_GRID_INTERVAL;
    path_preview_checkpoints_y[1] = 0;
    path_preview_timeout =
        timer(
            PATH_PREVIEW_TIMEOUT_DUR,
    [this] () {calculate_preview_path();}
        );
    if(editor_backup_interval > 0) {
        backup_timer =
            timer(
                editor_backup_interval,
        [this] () {save_backup();}
            );
    }
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
    
    lafi::frame* f =
        (lafi::frame*) area_editor::gui->widgets["frm_adv_textures"];
        
    ((lafi::textbox*) f->widgets["txt_x"])->text =
        f2s(cur_sector->texture_info.trans_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text =
        f2s(cur_sector->texture_info.trans_y);
    ((lafi::textbox*) f->widgets["txt_sx"])->text =
        f2s(cur_sector->texture_info.scale_x);
    ((lafi::textbox*) f->widgets["txt_sy"])->text =
        f2s(cur_sector->texture_info.scale_y);
    ((lafi::angle_picker*) f->widgets["ang_a"])->set_angle_rads(
        cur_sector->texture_info.rot
    );
    ((lafi::textbox*) f->widgets["txt_tint"])->text =
        c2s(cur_sector->texture_info.tint);
        
}


/* ----------------------------------------------------------------------------
 * Calculates the preview path.
 */
void area_editor::calculate_preview_path() {
    if(!show_path_preview) return;
    
    float d = 0;
    path_preview =
        get_path(
            path_preview_checkpoints_x[0],
            path_preview_checkpoints_y[0],
            path_preview_checkpoints_x[1],
            path_preview_checkpoints_y[1],
            NULL, NULL, &d
        );
        
    if(path_preview.empty() && d == 0) {
        d =
            dist(
                path_preview_checkpoints_x[0],
                path_preview_checkpoints_y[0],
                path_preview_checkpoints_x[1],
                path_preview_checkpoints_y[1]
            ).to_float();
    }
    
    ((lafi::label*) gui->widgets["frm_paths"]->widgets["lbl_path_dist"])->text =
        "  Total dist.: " + f2s(d);
}


/* ----------------------------------------------------------------------------
 * Cancels the creation of a new sector.
 * Only use this to cancel, not to finish successfully.
 */
void area_editor::cancel_new_sector() {
    for(size_t v = 0; v < new_sector_vertexes.size(); ++v) {
        if(new_sector_vertexes[v]->edges.empty()) {
            delete new_sector_vertexes[v];
        }
    }
    new_sector_vertexes.clear();
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void area_editor::center_camera(
    float min_x, float min_y, float max_x, float max_y
) {
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
    if(guide_bitmap && guide_bitmap != bmp_error) {
        al_destroy_bitmap(guide_bitmap);
    }
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
    hide_widget(gui->widgets["frm_texture"]);
    hide_widget(gui->widgets["frm_objects"]);
    hide_widget(gui->widgets["frm_shadows"]);
    hide_widget(gui->widgets["frm_guide"]);
    hide_widget(gui->widgets["frm_review"]);
    hide_widget(gui->widgets["frm_tools"]);
    hide_widget(gui->widgets["frm_options"]);
    
    if(!hide_all) {
        if(mode == EDITOR_MODE_MAIN) {
            show_widget(gui->widgets["frm_main"]);
        } else if(mode == EDITOR_MODE_SECTORS) {
            show_widget(gui->widgets["frm_sectors"]);
        } else if(mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS) {
            show_widget(gui->widgets["frm_adv_textures"]);
        } else if(mode == EDITOR_MODE_TEXTURE) {
            show_widget(gui->widgets["frm_texture"]);
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
        } else if(mode == EDITOR_MODE_TOOLS) {
            show_widget(gui->widgets["frm_tools"]);
        } else if(mode == EDITOR_MODE_OPTIONS) {
            show_widget(gui->widgets["frm_options"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Closes the change warning box.
 */
void area_editor::close_changes_warning() {
    hide_widget(gui->widgets["frm_changes"]);
    show_widget(gui->widgets["frm_bottom"]);
}


/* ----------------------------------------------------------------------------
 * Creates a new sector using the previously-drawn vertexes.
 */
void area_editor::create_sector() {
    if(new_sector_vertexes.size() < 3) {
        cancel_new_sector();
        return;
    }
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    //Then, try to merge the new vertexes with existing ones.
    
    //Some of the new vertexes may have to be merged. Let's check now,
    //as this will be important in just a bit.
    vector<vertex*> merge_dest_vertexes;
    vector<size_t> merge_dest_vertex_nrs;
    for(size_t v = 0; v < new_sector_vertexes.size(); ++v) {
        size_t merge_nr = INVALID;
        vertex* merge_v =
            get_merge_vertex(
                new_sector_vertexes[v]->x, new_sector_vertexes[v]->y, &merge_nr
            );
            
        merge_dest_vertexes.push_back(merge_v);
        merge_dest_vertex_nrs.push_back(merge_nr);
    }
    
    //Get the outer sector, so we can know where to start working in.
    size_t outer_sector_nr = INVALID;
    sector* outer_sector = NULL;
    
    bool has_common =
        get_common_sector(
            new_sector_vertexes, merge_dest_vertexes, &outer_sector
        );
        
    if(!has_common) {
        //What? The user is trying to create a sector that goes through
        //different sectors! We know, because at least one vertex
        //doesn't share sectors with the other vertexes. Abort!
        cancel_new_sector();
        return;
    }
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        if(cur_area_data.sectors[s] != outer_sector) continue;
        outer_sector_nr = s;
        break;
    }
    
    //Start creating the new sector.
    sector* new_sector = new sector();
    if(outer_sector) outer_sector->clone(new_sector);
    vector<edge*> new_sector_edges;
    
    vector<size_t> new_sector_vertex_nrs;
    for(size_t v = 0; v < new_sector_vertexes.size(); ++v) {
        new_sector_vertex_nrs.push_back(cur_area_data.vertexes.size());
        cur_area_data.vertexes.push_back(new_sector_vertexes[v]);
    }
    cur_area_data.sectors.push_back(new_sector);
    size_t new_sector_nr = cur_area_data.sectors.size() - 1;
    
    //Create the edges.
    vector<size_t> new_sector_edge_nrs;
    for(size_t v = 1; v < new_sector_vertexes.size(); ++v) {
        size_t v_nr = new_sector_vertex_nrs[v];
        edge* e_ptr = new edge(v_nr - 1, v_nr);
        new_sector_edges.push_back(e_ptr);
    }
    new_sector_edges.push_back(
        new edge(
            new_sector_vertex_nrs[new_sector_vertex_nrs.size() - 1],
            new_sector_vertex_nrs[0]
        )
    );
    
    //Populate the edges with the sector's data.
    bool is_clockwise = is_polygon_clockwise(new_sector_vertexes);
    for(size_t e = 0; e < new_sector_edges.size(); ++e) {
        edge* e_ptr = new_sector_edges[e];
        if(is_clockwise) {
            e_ptr->sector_nrs[0] = outer_sector_nr;
            e_ptr->sector_nrs[1] = new_sector_nr;
        } else {
            e_ptr->sector_nrs[0] = new_sector_nr;
            e_ptr->sector_nrs[1] = outer_sector_nr;
        }
        new_sector_edge_nrs.push_back(cur_area_data.edges.size());
        cur_area_data.edges.push_back(e_ptr);
        new_sector->edge_nrs.push_back(
            cur_area_data.edges.size() - 1
        );
    }
    
    //Connect the vertexes and edges.
    for(size_t e = 0; e < new_sector_edges.size(); ++e) {
        new_sector_edges[e]->fix_pointers(cur_area_data);
    }
    
    for(size_t v = 0; v < new_sector_vertexes.size(); ++v) {
        new_sector_vertexes[v]->connect_edges(
            cur_area_data, new_sector_vertex_nrs[v]
        );
    }
    
    new_sector->connect_edges(
        cur_area_data, cur_area_data.sectors.size() - 1
    );
    
    //Add the edges to the outer sector's list.
    if(outer_sector) {
        for(size_t e = 0; e < new_sector_edges.size(); ++e) {
            outer_sector->edges.push_back(new_sector_edges[e]);
            outer_sector->edge_nrs.push_back(
                new_sector_edge_nrs[e]
            );
        }
    }
    
    //Triangulate new sector so we can check what's inside.
    triangulate(new_sector);
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    unordered_set<edge*> inner_edges;
    for(
        size_t v = 0;
        v < cur_area_data.vertexes.size() -
        new_sector_vertexes.size();
        ++v
    ) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        
        //If we're going to merge one of our vertexes with this one,
        //just never mind.
        if(
            find(merge_dest_vertexes.begin(), merge_dest_vertexes.end(), v_ptr)
            != merge_dest_vertexes.end()
        ) {
            continue;
        }
        
        if(
            is_point_in_sector(v_ptr->x, v_ptr->y, new_sector)
        ) {
            inner_edges.insert(
                v_ptr->edges.begin(),
                v_ptr->edges.end()
            );
        }
    }
    for(
        auto i = inner_edges.begin();
        i != inner_edges.end(); ++i
    ) {
        if((*i)->sector_nrs[0] == outer_sector_nr) {
            (*i)->sector_nrs[0] = new_sector_nr;
        } else if((*i)->sector_nrs[1] == outer_sector_nr) {
            (*i)->sector_nrs[1] = new_sector_nr;
        }
        (*i)->fix_pointers(cur_area_data);
    }
    new_sector->connect_edges(
        cur_area_data,
        cur_area_data.sectors.size() - 1
    );
    if(outer_sector) {
        outer_sector->connect_edges(
            cur_area_data, outer_sector_nr
        );
    }
    
    //Merge vertexes that share a spot.
    unordered_set<sector*> merge_affected_sectors;
    for(size_t v = 0; v < new_sector_vertexes.size(); ++v) {
        if(!merge_dest_vertexes[v]) continue;
        merge_vertex(
            new_sector_vertexes[v],
            merge_dest_vertexes[v],
            &merge_affected_sectors
        );
    }
    
    //Final triangulations.
    triangulate(new_sector);
    if(outer_sector) triangulate(outer_sector);
    for(
        auto s = merge_affected_sectors.begin();
        s != merge_affected_sectors.end(); ++s
    ) {
        if(*s) triangulate(*s);
    }
    
    //Check for intersections, so they can get reported.
    for(size_t e = 0; e < new_sector->edges.size(); ++e) {
        check_edge_intersections(
            new_sector->edges[e]->vertexes[0]
        );
    }
    
    cur_sector = new_sector;
    new_sector_vertexes.clear();
}


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {

    gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(
        &transform, cam_x + ((scr_w - 208) / 2 / cam_zoom),
        cam_y + (scr_h / 2 / cam_zoom)
    );
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        
        //Grid.
        if(sec_mode != ESM_TEXTURE_VIEW) {
            float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
            float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
            float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
            float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
            
            float x = floor(cam_leftmost / grid_interval) * grid_interval;
            while(x < cam_rightmost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(48, 48, 48);
                bool draw_line = true;
                
                if(fmod(x, grid_interval * 2) == 0) {
                    c = al_map_rgb(64, 64, 64);
                    if((grid_interval * 2) * cam_zoom <= 6) draw_line = false;
                } else {
                    if(grid_interval * cam_zoom <= 6) draw_line = false;
                }
                
                if(draw_line) {
                    al_draw_line(
                        x, cam_topmost, x, cam_bottommost + grid_interval,
                        c, 1.0 / cam_zoom
                    );
                }
                x += grid_interval;
            }
            
            float y = floor(cam_topmost / grid_interval) * grid_interval;
            while(y < cam_bottommost + grid_interval) {
                ALLEGRO_COLOR c = al_map_rgb(48, 48, 48);
                bool draw_line = true;
                
                if(fmod(y, grid_interval * 2) == 0) {
                    c = al_map_rgb(64, 64, 64);
                    if((grid_interval * 2) * cam_zoom <= 6) draw_line = false;
                } else {
                    if(grid_interval * cam_zoom <= 6) draw_line = false;
                }
                
                if(draw_line) {
                    al_draw_line(
                        cam_leftmost, y, cam_rightmost + grid_interval, y,
                        c, 1.0 / cam_zoom
                    );
                }
                y += grid_interval;
            }
            
            //0,0 marker.
            al_draw_line(
                -(DEF_GRID_INTERVAL * 2), 0, DEF_GRID_INTERVAL * 2, 0,
                al_map_rgb(128, 128, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                0, -(DEF_GRID_INTERVAL * 2), 0, DEF_GRID_INTERVAL * 2,
                al_map_rgb(128, 128, 255), 1.0 / cam_zoom
            );
        }
        
        //Edges.
        if(sec_mode != ESM_TEXTURE_VIEW) {
        
            unsigned char sector_opacity = 255;
            bool show_vertices = true;
            if(
                mode == EDITOR_MODE_OBJECTS ||
                mode == EDITOR_MODE_PATHS ||
                mode == EDITOR_MODE_SHADOWS
            ) {
                sector_opacity = 128;
                show_vertices = false;
            }
            
            size_t n_edges = cur_area_data.edges.size();
            for(size_t e = 0; e < n_edges; ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                
                if(!is_edge_valid(e_ptr)) continue;
                
                bool one_sided = true;
                bool same_z = false;
                bool error_highlight = false;
                bool valid = true;
                bool mouse_on = false;
                bool selected = false;
                
                if(error_sector_ptr) {
                    if(
                        e_ptr->sectors[0] == error_sector_ptr ||
                        e_ptr->sectors[1] == error_sector_ptr
                    ) {
                        error_highlight = true;
                    }
                    
                } else {
                    for(size_t ie = 0; ie < intersecting_edges.size(); ++ie) {
                        if(intersecting_edges[ie].contains(e_ptr)) {
                            valid = false;
                            break;
                        }
                    }
                    
                    if(
                        non_simples.find(e_ptr->sectors[0]) !=
                        non_simples.end()
                    ) {
                        valid = false;
                    }
                    if(
                        non_simples.find(e_ptr->sectors[1]) !=
                        non_simples.end()
                    ) {
                        valid = false;
                    }
                    if(lone_edges.find(e_ptr) != lone_edges.end()) {
                        valid = false;
                    }
                }
                
                if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
                
                if(
                    !one_sided &&
                    e_ptr->sectors[0]->z == e_ptr->sectors[1]->z &&
                    e_ptr->sectors[0]->type == e_ptr->sectors[1]->type
                ) {
                    same_z = true;
                }
                
                if(on_sector && mode == EDITOR_MODE_SECTORS) {
                    if(e_ptr->sectors[0] == on_sector) mouse_on = true;
                    if(e_ptr->sectors[1] == on_sector) mouse_on = true;
                }
                
                if(
                    cur_sector &&
                    (mode == EDITOR_MODE_SECTORS || mode == EDITOR_MODE_TEXTURE)
                ) {
                    if(e_ptr->sectors[0] == cur_sector) selected = true;
                    if(e_ptr->sectors[1] == cur_sector) selected = true;
                }
                
                
                al_draw_line(
                    e_ptr->vertexes[0]->x,
                    e_ptr->vertexes[0]->y,
                    e_ptr->vertexes[1]->x,
                    e_ptr->vertexes[1]->y,
                    (
                        selected ?
                        al_map_rgba(224, 224, 64,  sector_opacity) :
                        error_highlight ?
                        al_map_rgba(192, 80,  0,   sector_opacity) :
                        !valid ?
                        al_map_rgba(192, 32,  32,  sector_opacity) :
                        one_sided ?
                        al_map_rgba(255, 255, 255, sector_opacity) :
                        same_z ?
                        al_map_rgba(128, 128, 128, sector_opacity) :
                        al_map_rgba(192, 192, 192, sector_opacity)
                    ),
                    (mouse_on || selected ? 3.0 : 2.0) / cam_zoom
                );
                
                if(debug_sector_nrs) {
                    float mid_x =
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f;
                    float mid_y =
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f;
                    float angle =
                        atan2(
                            e_ptr->vertexes[0]->y - e_ptr->vertexes[1]->y,
                            e_ptr->vertexes[0]->x - e_ptr->vertexes[1]->x
                        );
                    draw_scaled_text(
                        font_main, al_map_rgb(192, 255, 192),
                        mid_x + cos(angle + M_PI_2) * 4,
                        mid_y + sin(angle + M_PI_2) * 4,
                        0.5 / cam_zoom, 0.5 / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        (
                            e_ptr->sector_nrs[0] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[0])
                        )
                    );
                    draw_scaled_text(
                        font_main, al_map_rgb(192, 255, 192),
                        mid_x + cos(angle - M_PI_2) * 4,
                        mid_y + sin(angle - M_PI_2) * 4,
                        0.5 / cam_zoom, 0.5 / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        (
                            e_ptr->sector_nrs[1] == INVALID ?
                            "--" :
                            i2s(e_ptr->sector_nrs[1])
                        )
                    );
                }
                
                if(debug_edge_nrs) {
                    float mid_x =
                        (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f;
                    float mid_y =
                        (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f;
                    draw_scaled_text(
                        font_main, al_map_rgb(255, 192, 192),
                        mid_x, mid_y,
                        0.5 / cam_zoom, 0.5 / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        i2s(e)
                    );
                }
            }
            
            //Vertexes.
            if(show_vertices) {
                size_t n_vertexes = cur_area_data.vertexes.size();
                for(size_t v = 0; v < n_vertexes; ++v) {
                    vertex* v_ptr = cur_area_data.vertexes[v];
                    al_draw_filled_circle(
                        v_ptr->x,
                        v_ptr->y,
                        3.0 / cam_zoom,
                        al_map_rgba(80, 160, 255, sector_opacity)
                    );
                    
                    if(debug_vertex_nrs) {
                        draw_scaled_text(
                            font_main, al_map_rgb(192, 192, 255),
                            v_ptr->x, v_ptr->y,
                            0.5 / cam_zoom, 0.5 / cam_zoom,
                            ALLEGRO_ALIGN_CENTER, 1,
                            i2s(v)
                        );
                    }
                }
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
            mode == EDITOR_MODE_SECTORS ||
            mode == EDITOR_MODE_ADV_TEXTURE_SETTINGS ||
            mode == EDITOR_MODE_TEXTURE ||
            mode == EDITOR_MODE_PATHS ||
            mode == EDITOR_MODE_SHADOWS
        ) {
            mob_opacity = 32;
        }
        if(sec_mode == ESM_TEXTURE_VIEW) mob_opacity = 0;
        
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            bool valid = m_ptr->type != NULL;
            
            float radius =
                m_ptr->type ?
                m_ptr->type->radius == 0 ? 16 :
                m_ptr->type->radius : 16;
            ALLEGRO_COLOR c = mob_categories.get_editor_color(m_ptr->category);
            
            al_draw_filled_circle(
                m_ptr->x, m_ptr->y,
                radius,
                (
                    valid ? change_alpha(c, mob_opacity) :
                    al_map_rgba(255, 0, 0, mob_opacity)
                )
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
            float tx2 =
                tx1 + cos(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty2 =
                ty1 + sin(m_ptr->angle - (M_PI_2 + M_PI_4)) * radius * 0.5;
            float tx3 =
                tx1 + cos(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
            float ty3 =
                ty1 + sin(m_ptr->angle + (M_PI_2 + M_PI_4)) * radius * 0.5;
                
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
                    al_map_rgba(255, 255, 255, mob_opacity), 2 / cam_zoom
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
                    bool one_way = !(s_ptr->links[l].end_ptr->has_link(s_ptr));
                    
                    al_draw_line(
                        s_ptr->x, s_ptr->y,
                        s2_ptr->x, s2_ptr->y,
                        (
                            one_way ? al_map_rgb(255, 160, 160) :
                            al_map_rgb(255, 255, 160)
                        ),
                        PATH_LINK_THICKNESS / cam_zoom
                    );
                    
                    if(one_way) {
                        //Draw a triangle down the middle.
                        float mid_x =
                            (s_ptr->x + s2_ptr->x) / 2.0f;
                        float mid_y =
                            (s_ptr->y + s2_ptr->y) / 2.0f;
                        float angle =
                            atan2(s2_ptr->y - s_ptr->y, s2_ptr->x - s_ptr->x);
                        const float delta =
                            (PATH_LINK_THICKNESS * 4) / cam_zoom;
                            
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
            
            if(show_path_preview) {
                //Draw the checkpoints.
                for(unsigned char c = 0; c < 2; ++c) {
                    string letter = (c == 0 ? "A" : "B");
                    
                    al_draw_filled_rectangle(
                        path_preview_checkpoints_x[c] -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_y[c] -
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_x[c] +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        path_preview_checkpoints_y[c] +
                        (PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom),
                        al_map_rgb(255, 255, 32)
                    );
                    draw_scaled_text(
                        allegro_font, al_map_rgb(0, 64, 64),
                        path_preview_checkpoints_x[c],
                        path_preview_checkpoints_y[c],
                        1.0 / cam_zoom, 1.0 / cam_zoom,
                        ALLEGRO_ALIGN_CENTER, 1,
                        letter
                    );
                }
                
                //Draw the lines of the path.
                if(path_preview.empty()) {
                    al_draw_line(
                        path_preview_checkpoints_x[0],
                        path_preview_checkpoints_y[0],
                        path_preview_checkpoints_x[1],
                        path_preview_checkpoints_y[1],
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                } else {
                    al_draw_line(
                        path_preview_checkpoints_x[0],
                        path_preview_checkpoints_y[0],
                        path_preview[0]->x,
                        path_preview[0]->y,
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                    for(size_t s = 0; s < path_preview.size() - 1; ++s) {
                        al_draw_line(
                            path_preview[s]->x,
                            path_preview[s]->y,
                            path_preview[s + 1]->x,
                            path_preview[s + 1]->y,
                            al_map_rgb(255, 0, 0), 3 / cam_zoom
                        );
                    }
                    
                    al_draw_line(
                        path_preview.back()->x,
                        path_preview.back()->y,
                        path_preview_checkpoints_x[1],
                        path_preview_checkpoints_y[1],
                        al_map_rgb(255, 0, 0), 3 / cam_zoom
                    );
                }
            }
        }
        
        //Shadows.
        if(
            mode == EDITOR_MODE_SHADOWS ||
            (sec_mode == ESM_TEXTURE_VIEW && show_shadows)
        ) {
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                draw_sprite(
                    s_ptr->bitmap, s_ptr->x, s_ptr->y, s_ptr->w, s_ptr->h,
                    s_ptr->angle, map_alpha(s_ptr->alpha)
                );
                
                if(mode == EDITOR_MODE_SHADOWS) {
                    float min_x, min_y, max_x, max_y;
                    get_shadow_bounding_box(
                        s_ptr, &min_x, &min_y, &max_x, &max_y
                    );
                    
                    al_draw_rectangle(
                        min_x, min_y, max_x, max_y,
                        (
                            s_ptr == cur_shadow ?
                            al_map_rgb(224, 224, 64) :
                            al_map_rgb(128, 128, 64)
                        ),
                        2 / cam_zoom
                    );
                }
            }
        }
        
        //New sector preview.
        if(sec_mode == ESM_NEW_SECTOR) {
            for(size_t v = 1; v < new_sector_vertexes.size(); ++v) {
                al_draw_line(
                    new_sector_vertexes[v - 1]->x,
                    new_sector_vertexes[v - 1]->y,
                    new_sector_vertexes[v]->x,
                    new_sector_vertexes[v]->y,
                    al_map_rgb(128, 255, 128),
                    3 / cam_zoom
                );
            }
            if(!new_sector_vertexes.empty()) {
                al_draw_line(
                    new_sector_vertexes.back()->x,
                    new_sector_vertexes.back()->y,
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y),
                    (new_sector_valid_line ?
                     al_map_rgb(64, 255, 64) :
                     al_map_rgb(255, 0, 0)),
                    3 / cam_zoom
                );
            }
        }
        
        //New thing marker.
        if(
            sec_mode == ESM_NEW_SECTOR || sec_mode == ESM_NEW_OBJECT ||
            sec_mode == ESM_DUPLICATE_OBJECT || sec_mode == ESM_NEW_SHADOW ||
            sec_mode == ESM_NEW_STOP || sec_mode == ESM_NEW_LINK1 ||
            sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK1 ||
            sec_mode == ESM_NEW_1WLINK2
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
            al_draw_line(
                x - 16, y, x + 16, y,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
            al_draw_line(
                x, y - 16, x, y + 16,
                al_map_rgb(255, 255, 255), 1.0 / cam_zoom
            );
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
            if(on_sector && moving_thing == INVALID) {
                for(size_t t = 0; t < on_sector->triangles.size(); ++t) {
                    triangle* t_ptr = &on_sector->triangles[t];
                    
                    if(debug_triangulation) {
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
                    }
                    
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
                al_get_bitmap_width(guide_bitmap),
                al_get_bitmap_height(guide_bitmap),
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
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {

    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    path_preview_timeout.tick(delta_t);
    
    if(!area_name.empty() && editor_backup_interval > 0) {
        backup_timer.tick(delta_t);
    }
    
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Finds errors with the area.
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
    
    //Check for the existence of a leader object.
    if(error_type == EET_NONE) {
        bool has_leader = false;
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            if(
                cur_area_data.mob_generators[m]->category ==
                MOB_CATEGORY_LEADERS &&
                cur_area_data.mob_generators[m]->type != NULL
            ) {
                has_leader = true;
                break;
            }
        }
        if(!has_leader) {
            error_type = EET_MISSING_LEADER;
        }
    }
    
    //Check for missing textures.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        
            sector* s_ptr = cur_area_data.sectors[s];
            if(s_ptr->edges.empty()) continue;
            if(
                s_ptr->texture_info.file_name.empty() &&
                s_ptr->type != SECTOR_TYPE_BOTTOMLESS_PIT && !s_ptr->fade
            ) {
                error_type = EET_MISSING_TEXTURE;
                error_sector_ptr = s_ptr;
                break;
            }
        }
    }
    
    //Check for unknown textures.
    if(error_type == EET_NONE) {
        vector<string> texture_file_names =
            folder_to_vector(TEXTURES_FOLDER, false);
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        
            sector* s_ptr = cur_area_data.sectors[s];
            if(s_ptr->edges.empty()) continue;
            
            if(s_ptr->texture_info.file_name.empty()) continue;
            
            if(
                find(
                    texture_file_names.begin(), texture_file_names.end(),
                    s_ptr->texture_info.file_name
                ) == texture_file_names.end()
            ) {
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
            depth_first_search(
                cur_area_data.path_stops, visited, cur_area_data.path_stops[0]
            );
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
                        if(
                            e_ptr->sectors[0]->z >
                            e_ptr->sectors[1]->z + SECTOR_STEP
                        ) {
                            in_wall = true;
                        }
                        if(
                            e_ptr->sectors[1]->z >
                            e_ptr->sectors[0]->z + SECTOR_STEP
                        ) {
                            in_wall = true;
                        }
                        if(
                            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            in_wall = true;
                        }
                        if(
                            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            in_wall = true;
                        }
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
 * Returns a sector common to all vertexes.
 * A sector is considered this if a vertex has it as on a neighboring edge,
 * or if a vertex is inside it.
 * Use the former for vertexes that will be merged, and the latter
 * for vertexes that won't.
 * vertexes: List of vertexes to check.
 * merges:   For every vertex to check, which vertex it will be merged with.
 * result:   Returns the common sector here.
 * Returns false if there is no common sector. True otherwise.
 */
bool area_editor::get_common_sector(
    vector<vertex*> &vertexes, vector<vertex*> &merges, sector** result
) {
    vector<unordered_set<sector*> > related_sectors;
    //Get the related sectors of each vertex.
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        related_sectors.push_back(unordered_set<sector*>());
        
        if(merges[v]) {
            //If this sector is going to be merged, check
            //the edges of the destination vertex.
            vertex* mv_ptr = merges[v];
            for(size_t e = 0; e < mv_ptr->edges.size(); ++e) {
                for(size_t s = 0; s < 2; ++s) {
                    related_sectors[v].insert(mv_ptr->edges[e]->sectors[s]);
                }
            }
        } else {
            //If this is a lone vertex, check which sector it's in.
            related_sectors[v].insert(
                get_sector(
                    v_ptr->x,
                    v_ptr->y,
                    NULL, false
                )
            );
        }
    }
    
    unordered_set<sector*> all_related_sectors;
    for(size_t v = 0; v < vertexes.size(); ++v) {
        all_related_sectors.insert(
            related_sectors[v].begin(),
            related_sectors[v].end()
        );
    }
    
    unordered_set<sector*> all_common_sectors;
    for(
        auto s = all_related_sectors.begin();
        s != all_related_sectors.end(); ++s
    ) {
        //Check if this sector is common to ALL vertexes.
        bool is_common = true;
        for(size_t v = 0; v < vertexes.size(); ++v) {
            if(related_sectors[v].find(*s) == related_sectors[v].end()) {
                is_common = false;
                break;
            }
        }
        if(!is_common) continue;
        
        all_common_sectors.insert(*s);
    }
    
    if(all_common_sectors.size() == 0) {
        *result = NULL;
        return false;
    } else if(all_common_sectors.size() == 1) {
        *result = *all_common_sectors.begin();
        return true;
    }
    
    //Uh-oh...there's no clear answer. We'll have to decide between the
    //involved sectors. Get the rightmost vertexes of all involved sectors.
    //The one most to the left wins.
    //Why? Imagine you're making a triangle inside a square, which is in turn
    //inside another square. The triangle's points share both the inner and
    //outer square sectors. The triangle "belongs" to the inner sector,
    //and we can easily find out which is the inner one with this method.
    float best_rightmost_x = 0;
    sector* best_rightmost_sector = NULL;
    for(
        auto s = all_common_sectors.begin(); s != all_common_sectors.end(); ++s
    ) {
        if(*s == NULL) continue;
        vertex* v_ptr = get_rightmost_vertex(*s);
        if(!best_rightmost_sector || v_ptr->x < best_rightmost_x) {
            best_rightmost_sector = *s;
            best_rightmost_x = v_ptr->x;
        }
    }
    
    *result = best_rightmost_sector;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the closest vertex that can be merged with the specified point.
 * Returns NULL if there is no vertex close enough to merge.
 * x, y: Coordinates of the point.
 * v_nr: If not NULL, the vertex's number is returned here.
 */
vertex* area_editor::get_merge_vertex(
    const float x, const float y, size_t* v_nr
) {
    dist closest_dist = 0;
    vertex* closest_v = NULL;
    size_t closest_nr = INVALID;
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        dist d(x, y, v_ptr->x, v_ptr->y);
        if(
            d <= VERTEX_MERGE_RADIUS / cam_zoom &&
            (d < closest_dist || !closest_v)
        ) {
            closest_dist = d;
            closest_v = v_ptr;
            closest_nr = v;
        }
    }
    
    if(v_nr) *v_nr = closest_nr;
    return closest_v;
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
        
    } else if(error_type == EET_MISSING_LEADER) {
    
        return;
        
    } else if(
        error_type == EET_MISSING_TEXTURE ||
        error_type == EET_UNKNOWN_TEXTURE
    ) {
    
        if(!error_sector_ptr) {
            find_errors(); return;
        }
        
        float min_x, min_y, max_x, max_y;
        get_sector_bounding_box(
            error_sector_ptr, &min_x, &min_y, &max_x, &max_y
        );
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
        get_shadow_bounding_box(
            error_shadow_ptr, &min_x, &min_y, &max_x, &max_y
        );
        center_camera(min_x, min_y, max_x, max_y);
    }
}


/* ----------------------------------------------------------------------------
 * Saves the advanced texture settings from the gui.
 */
void area_editor::gui_to_adv_textures() {
    if(!cur_sector) return;
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_adv_textures"];
    
    cur_sector->texture_info.trans_x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sector->texture_info.trans_y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sector->texture_info.scale_x =
        s2f(((lafi::textbox*) f->widgets["txt_sx"])->text);
    cur_sector->texture_info.scale_y =
        s2f(((lafi::textbox*) f->widgets["txt_sy"])->text);
    cur_sector->texture_info.rot =
        ((lafi::angle_picker*) f->widgets["ang_a"])->get_angle_rads();
    cur_sector->texture_info.tint =
        s2c(((lafi::textbox*) f->widgets["txt_tint"])->text);
        
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
    
    sec_mode =
        ((lafi::checkbox*) f->widgets["chk_mouse"])->checked ?
        ESM_GUIDE_MOUSE : ESM_NONE;
    guide_a = ((lafi::scrollbar*) f->widgets["bar_alpha"])->low_value;
    
    guide_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves a mob's data using info on the gui.
 */
void area_editor::gui_to_mob() {
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_objects"]->widgets["frm_object"];
        
    if(!cur_mob) return;
    
    cur_mob->angle =
        ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
    cur_mob->vars =
        ((lafi::textbox*) f->widgets["txt_vars"])->text;
        
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the current tree shadow using the info on the gui.
 */
void area_editor::gui_to_shadow() {
    if(!cur_shadow) return;
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_shadows"]->widgets["frm_shadow"];
        
    cur_shadow->x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_shadow->y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_shadow->w = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_shadow->h = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_shadow->angle =
        ((lafi::angle_picker*) f->widgets["ang_an"])->get_angle_rads();
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
void area_editor::gui_to_sector(bool called_by_brightness_bar) {
    if(!cur_sector) return;
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_sectors"]->widgets["frm_sector"];
        
    cur_sector->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    cur_sector->fade = ((lafi::checkbox*) f->widgets["chk_fade"])->checked;
    cur_sector->always_cast_shadow =
        ((lafi::checkbox*) f->widgets["chk_shadow"])->checked;
        
    if(cur_sector->fade) {
        cur_sector->texture_info.file_name.clear();
    } else {
        cur_sector->texture_info.file_name =
            ((lafi::button*) f->widgets["but_texture"])->text;
    }
    
    if(called_by_brightness_bar) {
        cur_sector->brightness =
            ((lafi::scrollbar*) f->widgets["bar_brightness"])->low_value;
    } else {
        cur_sector->brightness =
            s2i(((lafi::textbox*) f->widgets["txt_brightness"])->text);
    }
    
    cur_sector->tag = ((lafi::textbox*) f->widgets["txt_tag"])->text;
    cur_sector->hazards_str =
        ((lafi::textbox*) f->widgets["txt_hazards"])->text;
    cur_sector->hazard_floor =
        ((lafi::checkbox*) f->widgets["chk_hazards_floor"])->checked;
        
    (
        (lafi::textbox*) gui->widgets["frm_texture"]->widgets["txt_name"]
    )->text.clear();
    
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
    ((lafi::checkbox*) f->widgets["chk_mouse"])->set(
        sec_mode == ESM_GUIDE_MOUSE
    );
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
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x =
            ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y =
            ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* wum;
        if(
            ev.mouse.x < scr_w - 208 &&
            ev.mouse.y < scr_h - 16
        ) {
            wum = NULL;
        } else {
            wum =
                gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y);
        }
        ((lafi::label*) gui->widgets["lbl_status_bar"])->text =
            (
                wum ?
                wum->description :
                "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")"
            );
    }
    
    
    //Moving vertexes, camera, etc.
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
    
        if(
            ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16
            && moving_thing == INVALID && sec_mode != ESM_TEXTURE_VIEW &&
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
                        bool most_is_width =
                            fabs((double) ev.mouse.dx) >
                            fabs((double) ev.mouse.dy);
                            
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
        if(moving_thing != INVALID) {
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
                s_ptr->calculate_dists();
                path_preview_timeout.start(false);
            } else if(mode == EDITOR_MODE_SHADOWS) {
                tree_shadow* s_ptr = cur_area_data.tree_shadows[moving_thing];
                s_ptr->x = snap_to_grid(mouse_cursor_x - moving_thing_x);
                s_ptr->y = snap_to_grid(mouse_cursor_y - moving_thing_y);
                shadow_to_gui();
            }
            
            made_changes = true;
        }
        
        //Move path checkpoints.
        if(moving_path_preview_checkpoint != -1) {
            path_preview_checkpoints_x[moving_path_preview_checkpoint] =
                snap_to_grid(mouse_cursor_x);
            path_preview_checkpoints_y[moving_path_preview_checkpoint] =
                snap_to_grid(mouse_cursor_y);
            path_preview_timeout.start(false);
        }
        
        
        if(
            ev.mouse.dz != 0 && ev.mouse.x <= scr_w - 208 &&
            ev.mouse.y < scr_h - 16
        ) {
            //Zoom.
            float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
            new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
            new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
            float new_mc_x =
                ev.mouse.x / new_zoom - cam_x - ((scr_w - 208) / 2 / new_zoom);
            float new_mc_y =
                ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
                
            cam_x -= (mouse_cursor_x - new_mc_x);
            cam_y -= (mouse_cursor_y - new_mc_y);
            mouse_cursor_x = new_mc_x;
            mouse_cursor_y = new_mc_y;
            cam_zoom = new_zoom;
        }
        
        if(sec_mode == ESM_NEW_SECTOR) {
            new_sector_valid_line =
                is_new_sector_line_valid(
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y)
                );
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16
    ) {
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
        
            moving_thing = INVALID;
            
            edge* clicked_edge_ptr = NULL;
            size_t clicked_edge_nr = INVALID;
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
                    new_e_ptr->sectors[0]->edge_nrs.push_back(
                        cur_area_data.edges.size() - 1
                    );
                    new_e_ptr->sectors[0]->edges.push_back(new_e_ptr);
                }
                if(new_e_ptr->sectors[1]) {
                    new_e_ptr->sectors[1]->edge_nrs.push_back(
                        cur_area_data.edges.size() - 1
                    );
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
                        end_v_ptr->edges[ve] =
                            new_e_ptr;
                        end_v_ptr->edge_nrs[ve] =
                            cur_area_data.edges.size() - 1;
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
                            cur_area_data.vertexes[v]->x,
                            cur_area_data.vertexes[v]->y
                        ) <= 6.0 / cam_zoom
                    ) {
                        moving_thing = v;
                        break;
                    }
                }
            }
            
            //Find a sector to select.
            if(moving_thing == INVALID) {
                cur_sector =
                    get_sector(mouse_cursor_x, mouse_cursor_y, NULL, false);
                sector_to_gui();
            }
            
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_OBJECTS) {
            //Object-related clicking.
            
            cur_mob = NULL;
            moving_thing = INVALID;
            for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
                mob_gen* m_ptr = cur_area_data.mob_generators[m];
                float radius =
                    m_ptr->type ? m_ptr->type->radius == 0 ? 16 :
                    m_ptr->type->radius : 16;
                if(
                    dist(m_ptr->x, m_ptr->y, mouse_cursor_x, mouse_cursor_y) <=
                    radius
                ) {
                
                    cur_mob = m_ptr;
                    moving_thing = m;
                    break;
                }
            }
            mob_to_gui();
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_PATHS) {
            //Path-related clicking.
            
            cur_stop = NULL;
            moving_thing = INVALID;
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                if(
                    dist(s_ptr->x, s_ptr->y, mouse_cursor_x, mouse_cursor_y)
                    <= STOP_RADIUS
                ) {
                
                    cur_stop = s_ptr;
                    moving_thing = s;
                    break;
                }
            }
            
            moving_path_preview_checkpoint = -1;
            if(show_path_preview) {
                for(unsigned char c = 0; c < 2; ++c) {
                    if(
                        bbox_check(
                            path_preview_checkpoints_x[c],
                            path_preview_checkpoints_y[c],
                            mouse_cursor_x, mouse_cursor_y,
                            PATH_PREVIEW_CHECKPOINT_RADIUS / cam_zoom
                        )
                    ) {
                        moving_path_preview_checkpoint = c;
                        break;
                    }
                }
            }
            
        } else if(sec_mode == ESM_NONE && mode == EDITOR_MODE_SHADOWS) {
            //Shadow-related clicking.
            
            cur_shadow = NULL;
            moving_thing = INVALID;
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
            //Next vertex in a new sector.
            float hotspot_x = snap_to_grid(mouse_cursor_x);
            float hotspot_y = snap_to_grid(mouse_cursor_y);
            new_sector_valid_line =
                is_new_sector_line_valid(
                    snap_to_grid(mouse_cursor_x),
                    snap_to_grid(mouse_cursor_y)
                );
                
            if(new_sector_valid_line) {
                if(
                    !new_sector_vertexes.empty() &&
                    dist(
                        hotspot_x, hotspot_y,
                        new_sector_vertexes[0]->x,
                        new_sector_vertexes[0]->y
                    ) <= VERTEX_MERGE_RADIUS
                ) {
                    //Back to the first vertex.
                    sec_mode = ESM_NONE;
                    create_sector();
                    sector_to_gui();
                    made_changes = true;
                } else {
                    //Add a new vertex.
                    vertex* merge = get_merge_vertex(hotspot_x, hotspot_y);
                    if(merge) {
                        new_sector_vertexes.push_back(
                            new vertex(merge->x, merge->y)
                        );
                    } else {
                        new_sector_vertexes.push_back(
                            new vertex(hotspot_x, hotspot_y)
                        );
                    }
                }
            }
            
            
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
            
        } else if(sec_mode == ESM_DUPLICATE_OBJECT) {
            //Duplicate the current mob to where the cursor is.
            
            sec_mode = ESM_NONE;
            
            if(cur_mob) {
                float hotspot_x = snap_to_grid(mouse_cursor_x);
                float hotspot_y = snap_to_grid(mouse_cursor_y);
                
                mob_gen* new_mg = new mob_gen(*cur_mob);
                new_mg->x = hotspot_x;
                new_mg->y = hotspot_y;
                cur_area_data.mob_generators.push_back(
                    new_mg
                );
                
                cur_mob = new_mg;
                mob_to_gui();
                made_changes = true;
            }
            
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
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                    new_link_first_stop = s_ptr;
                    sec_mode =
                        sec_mode == ESM_NEW_LINK1 ? ESM_NEW_LINK2 :
                        ESM_NEW_1WLINK2;
                    break;
                }
            }
            
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if (sec_mode == ESM_NEW_LINK2 || sec_mode == ESM_NEW_1WLINK2) {
            //Pick a stop to end the link on.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                
                    if(new_link_first_stop == s_ptr) continue;
                    
                    //Check if these two stops already have a link.
                    //Delete it if so.
                    for(
                        size_t l = 0; l < new_link_first_stop->links.size();
                        ++l
                    ) {
                        if(new_link_first_stop->links[l].end_ptr == s_ptr) {
                            new_link_first_stop->links.erase(
                                new_link_first_stop->links.begin() + l
                            );
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
                        path_link(s_ptr, s)
                    );
                    
                    if(sec_mode == ESM_NEW_LINK2) {
                        s_ptr->links.push_back(
                            path_link(new_link_first_stop, INVALID)
                        );
                        s_ptr->fix_nrs(cur_area_data);
                    }
                    
                    new_link_first_stop->calculate_dists();
                    
                    sec_mode =
                        sec_mode == ESM_NEW_LINK2 ? ESM_NEW_LINK1 :
                        ESM_NEW_1WLINK1;
                    break;
                }
            }
            
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_STOP) {
            //Pick a stop to delete.
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                if(
                    dist(mouse_cursor_x, mouse_cursor_y, s_ptr->x, s_ptr->y) <=
                    STOP_RADIUS
                ) {
                
                    //Check all links to this stop.
                    for(
                        size_t s2 = 0; s2 < cur_area_data.path_stops.size();
                        ++s2
                    ) {
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
                    cur_area_data.path_stops.erase(
                        cur_area_data.path_stops.begin() + s
                    );
                    break;
                }
            }
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                cur_area_data.path_stops[s]->fix_nrs(cur_area_data);
            }
            
            path_preview.clear();
            path_preview_timeout.start(false);
            made_changes = true;
            
        } else if(sec_mode == ESM_DEL_LINK) {
            //Pick a link to delete.
            
            bool deleted = false;
            
            for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
                path_stop* s_ptr = cur_area_data.path_stops[s];
                
                for(size_t s2 = 0; s2 < s_ptr->links.size(); ++s2) {
                    path_stop* s2_ptr = s_ptr->links[s2].end_ptr;
                    if(
                        circle_intersects_line(
                            mouse_cursor_x, mouse_cursor_y, 8 / cam_zoom,
                            s_ptr->x, s_ptr->y,
                            s2_ptr->x, s2_ptr->y
                        )
                    ) {
                    
                        s_ptr->links.erase(s_ptr->links.begin() + s2);
                        
                        for(size_t s3 = 0; s3 < s2_ptr->links.size(); ++s3) {
                            if(s2_ptr->links[s3].end_ptr == s_ptr) {
                                s2_ptr->links.erase(
                                    s2_ptr->links.begin() + s3
                                );
                                break;
                            }
                        }
                        
                        deleted = true;
                        break;
                    }
                }
                
                if(deleted) break;
            }
            
            path_preview.clear();
            path_preview_timeout.start(false);
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
        
        if(
            ev.mouse.button == 1 &&
            mode == EDITOR_MODE_SECTORS && sec_mode == ESM_NONE &&
            moving_thing != INVALID
        ) {
            //Release the vertex.
            
            vertex* moved_v_ptr = cur_area_data.vertexes[moving_thing];
            vertex* final_vertex = moved_v_ptr;
            
            unordered_set<sector*> affected_sectors;
            
            //Check if we should merge.
            for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
                vertex* dest_v_ptr = cur_area_data.vertexes[v];
                if(dest_v_ptr == moved_v_ptr) continue;
                
                if(
                    dist(
                        moved_v_ptr->x, moved_v_ptr->y,
                        dest_v_ptr->x, dest_v_ptr->y
                    ) <= (VERTEX_MERGE_RADIUS / cam_zoom)
                ) {
                    merge_vertex(
                        moved_v_ptr, dest_v_ptr, &affected_sectors
                    );
                    final_vertex = dest_v_ptr;
                    break;
                }
            }
            
            //Finally, re-triangulate the affected sectors.
            for(size_t e = 0; e < final_vertex->edges.size(); ++e) {
                edge* e_ptr = final_vertex->edges[e];
                for(size_t s = 0; s < 2; ++s) {
                    if(e_ptr->sectors[s]) {
                        affected_sectors.insert(e_ptr->sectors[s]);
                    }
                }
            }
            for(
                auto s = affected_sectors.begin();
                s != affected_sectors.end(); ++s
            ) {
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
            check_edge_intersections(moved_v_ptr);
            
            moving_thing = INVALID;
            
            
            
        } else if(
            ev.mouse.button == 1 && sec_mode == ESM_NONE &&
            moving_thing != INVALID
        ) {
            //Release thing.
            
            moving_thing = INVALID;
            
        }
        
        moving_path_preview_checkpoint = -1;
        
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key press.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            shift_pressed = true;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F1) {
            debug_edge_nrs = !debug_edge_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F2) {
            debug_sector_nrs = !debug_sector_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F3) {
            debug_vertex_nrs = !debug_vertex_nrs;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F4) {
            debug_triangulation = !debug_triangulation;
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
 * Returns whether the next line for a sector's creation is valid.
 * i.e. it does not cross against other lines.
 * This is the line between the last chosen vertex of the new sector
 * and the provided coordinates.
 */
bool area_editor::is_new_sector_line_valid(const float x, const float y) {
    if(new_sector_vertexes.empty()) return true;
    
    //Given the last vertex of the new sector,
    //check if it'll be merged with an existing one.
    vertex* last_vertex = new_sector_vertexes.back();
    vertex* merge_vertex_1 = get_merge_vertex(last_vertex->x, last_vertex->y);
    vertex* merge_vertex_2 = get_merge_vertex(x, y);
    
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        //If this edge is on the same vertex as the last vertex
        //of the new sector, never mind.
        edge* e_ptr = cur_area_data.edges[e];
        if(!e_ptr->vertexes[0]) continue;
        if(
            e_ptr->vertexes[0] == merge_vertex_1 ||
            e_ptr->vertexes[1] == merge_vertex_1 ||
            e_ptr->vertexes[0] == merge_vertex_2 ||
            e_ptr->vertexes[1] == merge_vertex_2
        ) {
            continue;
        }
        
        if(
            lines_intersect(
                last_vertex->x, last_vertex->y, x, y,
                e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y,
                NULL, NULL
            )
        ) {
            return false;
        }
    }
    
    //Check if the line intersects with the new sector's edges.
    if(new_sector_vertexes.size() >= 2) {
        for(size_t v = 0; v < new_sector_vertexes.size() - 2; ++v) {
            vertex* v1_ptr = new_sector_vertexes[v];
            vertex* v2_ptr = new_sector_vertexes[v + 1];
            if(
                lines_intersect(
                    new_sector_vertexes.back()->x,
                    new_sector_vertexes.back()->y,
                    x, y, v1_ptr->x, v1_ptr->y, v2_ptr->x, v2_ptr->y,
                    NULL, NULL
                )
            ) {
                if(
                    v == 0 &&
                    dist(x, y, v1_ptr->x, v1_ptr->y) <= VERTEX_MERGE_RADIUS
                ) {
                    //We're trying to close the sector. Never mind this one.
                    continue;
                }
                return false;
            }
        }
    }
    
    return true;
}

/* ----------------------------------------------------------------------------
 * Returns whether a polygon was created clockwise or anti-clockwise,
 * given the order of its vertexes.
 */
bool area_editor::is_polygon_clockwise(vector<vertex*> &vertexes) {
    //Solution by http://stackoverflow.com/a/1165943
    float sum = 0;
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        vertex* v2_ptr = get_next_in_vector(vertexes, v);
        sum += (v2_ptr->x - v_ptr->x) * (v2_ptr->y + v_ptr->y);
    }
    return sum < 0;
}


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {

    fade_mgr.start_fade(true, nullptr);
    
    load_custom_particle_generators();
    load_liquids();
    load_status_types();
    load_hazards();
    load_mob_types(false);
    
    mode = EDITOR_MODE_MAIN;
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32),
        al_map_rgb(96, 128, 160)
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi::frame* frm_main =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_area",
        new lafi::label(0, 0, 0, 0, "Area:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_area",
        new lafi::button(0, 0, 0, 0), 100, 32
    );
    int y = frm_main->easy_row();
    
    lafi::frame* frm_area =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    hide_widget(frm_area);
    frm_area->easy_row();
    frm_area->easy_add(
        "but_sectors",
        new lafi::button(0, 0, 0, 0, "Edit sectors"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_objects",
        new lafi::button(0, 0, 0, 0, "Edit objects"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_paths",
        new lafi::button(0, 0, 0, 0, "Edit paths"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_shadows",
        new lafi::button(0, 0, 0, 0, "Edit shadows"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_guide",
        new lafi::button(0, 0, 0, 0, "Edit guide"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_review",
        new lafi::button(0, 0, 0, 0, "Review"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_tools",
        new lafi::button(0, 0, 0, 0, "Special tools"), 100, 32
    );
    frm_area->easy_row();
    
    
    //Properties -- main.
    frm_main->widgets["but_area"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_AREA);
    };
    frm_main->widgets["but_area"]->description =
        "Pick the area to edit.";
        
    frm_area->widgets["but_sectors"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_area->widgets["but_sectors"]->description =
        "Change sectors (polygons) and their settings.";
        
    frm_area->widgets["but_objects"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_OBJECTS;
        change_to_right_frame();
    };
    frm_area->widgets["but_objects"]->description =
        "Change object settings and placements.";
        
    frm_area->widgets["but_paths"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_PATHS;
        change_to_right_frame();
    };
    frm_area->widgets["but_paths"]->description =
        "Change movement paths and stops.";
        
    frm_area->widgets["but_shadows"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SHADOWS;
        change_to_right_frame();
    };
    frm_area->widgets["but_shadows"]->description =
        "Change the shadows of trees and leaves.";
        
    frm_area->widgets["but_guide"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_GUIDE;
        change_to_right_frame();
    };
    frm_area->widgets["but_guide"]->description =
        "Add an image, like a sketch, to guide you.";
        
    frm_area->widgets["but_review"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_REVIEW;
        change_to_right_frame();
        update_review_frame();
    };
    frm_area->widgets["but_review"]->description =
        "Tools to make sure everything is fine in the area.";
        
    frm_area->widgets["but_tools"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_TOOLS;
        change_to_right_frame();
    };
    frm_area->widgets["but_tools"]->description =
        "Special tools to help with specific tasks.";
        
        
    //Sectors frame.
    lafi::frame* frm_sectors =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_sectors);
    gui->add("frm_sectors", frm_sectors);
    
    frm_sectors->easy_row();
    frm_sectors->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_sectors->easy_row();
    frm_sectors->easy_add(
        "but_new",
        new lafi::button(0, 0, 0, 0, "+"), 20, 32
    );
    frm_sectors->easy_add(
        "but_sel_none",
        new lafi::button(0, 0, 0, 0, "None"), 20, 32
    );
    y = frm_sectors->easy_row();
    
    lafi::frame* frm_sector =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_sector);
    frm_sectors->add("frm_sector", frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_type",
        new lafi::label(0, 0, 0, 0, "Type:"), 30, 24
    );
    frm_sector->easy_add(
        "but_type",
        new lafi::button(0, 0, 0, 0), 70, 24
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_z",
        new lafi::label(0, 0, 0, 0, "Height:"), 50, 16
    );
    frm_sector->easy_add(
        "txt_z",
        new lafi::textbox(0, 0, 0, 0), 50, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_hazards",
        new lafi::label(0, 0, 0, 0, "Hazards:"), 65, 16
    );
    frm_sector->easy_add(
        "chk_hazards_floor",
        new lafi::checkbox(0, 0, 0, 0, "Floor"), 35, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "txt_hazards",
        new lafi::textbox(0, 0, 0, 0), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_texture",
        new lafi::label(0, 0, 0, 0, "Texture:"), 70, 16
    );
    frm_sector->easy_add(
        "chk_fade",
        new lafi::checkbox(0, 0, 0, 0, "Fade"), 30, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_texture",
        new lafi::button(0, 0, 0, 0), 100, 24
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_adv",
        new lafi::button(0, 0, 0, 0, "Adv. texture settings"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_brightness",
        new lafi::label(0, 0, 0, 0, "Brightness:"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "bar_brightness",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 80, 16
    );
    frm_sector->easy_add(
        "txt_brightness",
        new lafi::textbox(0, 0, 0, 0), 20, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "chk_shadow",
        new lafi::checkbox(0, 0, 0, 0, "Always cast shadow"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_2",
        new lafi::line(0, 0, 0, 0), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_tag",
        new lafi::label(0, 0, 0, 0, "Tags:"), 25, 16
    );
    frm_sector->easy_add(
        "txt_tag",
        new lafi::textbox(0, 0, 0, 0), 75, 16
    );
    frm_sector->easy_row();
    
    
    //Properties -- sectors.
    auto lambda_gui_to_sector =
    [this] (lafi::widget*) { gui_to_sector(); };
    auto lambda_gui_to_sector_click =
    [this] (lafi::widget*, int, int) { gui_to_sector(); };
    
    frm_sectors->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_sectors->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_sectors->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cancel_new_sector();
        new_sector_valid_line =
            is_new_sector_line_valid(
                snap_to_grid(mouse_cursor_x),
                snap_to_grid(mouse_cursor_y)
            );
        if(sec_mode == ESM_NEW_SECTOR) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_SECTOR;
    };
    frm_sectors->widgets["but_new"]->description =
        "Trace a new sector where you click.";
        
    frm_sectors->widgets["but_sel_none"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_sector = NULL;
        sector_to_gui();
    };
    frm_sectors->widgets["but_sel_none"]->description =
        "Deselect the current sector.";
        
    frm_sector->widgets["but_type"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_SECTOR_TYPE);
    };
    frm_sector->widgets["but_type"]->description =
        "Change the type of sector.";
        
    frm_sector->widgets["txt_z"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["txt_z"]->description =
        "Height of the floor.";
        
    frm_sector->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["txt_hazards"]->description =
        "Hazards the sector has. (e.g. \"fire; poison\")";
        
    frm_sector->widgets["chk_hazards_floor"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["chk_hazards_floor"]->description =
        "Trigger hazard on the floor only or in the air too?";
        
    frm_sector->widgets["but_texture"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sector) return;
        mode = EDITOR_MODE_TEXTURE;
        populate_texture_suggestions();
        change_to_right_frame();
    };
    frm_sector->widgets["but_texture"]->description =
        "Pick a texture (image) to use for the floor.";
        
    frm_sector->widgets["chk_fade"]->left_mouse_click_handler =
        lambda_gui_to_sector_click;
    frm_sector->widgets["chk_fade"]->description =
        "Makes the surrounding textures fade into each other.";
        
    frm_sector->widgets["but_adv"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sector) return;
        
        cur_sector->texture_info.bitmap =
            bitmaps.get("Textures/" + cur_sector->texture_info.file_name, NULL);
            
        mode = EDITOR_MODE_ADV_TEXTURE_SETTINGS;
        change_to_right_frame();
        adv_textures_to_gui();
    };
    frm_sector->widgets["but_adv"]->description =
        "Advanced settings for the sector's texture.";
        
    ((lafi::scrollbar*) frm_sector->widgets["bar_brightness"])->change_handler =
    [this] (lafi::widget*) { gui_to_sector(true); };
    frm_sector->widgets["bar_brightness"]->description =
        "0 = pitch black sector. 255 = normal lighting.";
        
    frm_sector->widgets["txt_brightness"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["txt_brightness"]->description =
        "0 = pitch black sector. 255 = normal lighting.";
        
    frm_sector->widgets["chk_shadow"]->left_mouse_click_handler =
        lambda_gui_to_sector_click;
    frm_sector->widgets["chk_shadow"]->description =
        "Makes it always cast a shadow onto lower sectors.";
        
    frm_sector->widgets["txt_tag"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["txt_tag"]->description =
        "Special values you may want the sector to know.";
        
        
    //Advanced texture settings frame.
    lafi::frame* frm_adv_textures =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_adv_textures);
    gui->add("frm_adv_textures", frm_adv_textures);
    
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 20, 16
    );
    frm_adv_textures->easy_add(
        "lbl_main",
        new lafi::label(0, 0, 0, 0, "Main texture"), 60, 16
    );
    frm_adv_textures->easy_add(
        "lin_2",
        new lafi::line(0, 0, 0, 0), 20, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X&Y:"), 40, 16
    );
    frm_adv_textures->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_adv_textures->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_sxy",
        new lafi::label(0, 0, 0, 0, "Scale:"), 40, 16
    );
    frm_adv_textures->easy_add(
        "txt_sx",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_adv_textures->easy_add(
        "txt_sy",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_a",
        new lafi::label(0, 0, 0, 0, "Angle:"), 50, 16
    );
    frm_adv_textures->easy_add(
        "ang_a",
        new lafi::angle_picker(0, 0, 0, 0), 50, 24
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_tint",
        new lafi::label(0, 0, 0, 0, "Tint color:"), 100, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "txt_tint",
        new lafi::textbox(0, 0, 0, 0), 100, 16
    );
    frm_adv_textures->easy_row();
    
    
    //Properties -- advanced texture settings.
    auto lambda_gui_to_adv_textures =
    [this] (lafi::widget*) { gui_to_adv_textures(); };
    
    frm_adv_textures->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        //Clears the texture set when we entered this menu.
        clear_area_textures();
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_adv_textures->widgets["but_back"]->description =
        "Go back.";
        
    frm_adv_textures->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_x"]->description =
        "Scroll the texture horizontally by this much.";
        
    frm_adv_textures->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_y"]->description =
        "Scroll the texture vertically by this much.";
        
    frm_adv_textures->widgets["txt_sx"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sx"]->description =
        "Zoom the texture horizontally by this much.";
        
    frm_adv_textures->widgets["txt_sy"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_sy"]->description =
        "Zoom the texture vertically by this much.";
        
    frm_adv_textures->widgets["ang_a"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["ang_a"]->description =
        "Rotate the texture by this much.";
        
    frm_adv_textures->widgets["txt_tint"]->lose_focus_handler =
        lambda_gui_to_adv_textures;
    frm_adv_textures->widgets["txt_tint"]->description =
        "Texture tint color, in the format \"r g b a\".";
        
        
    //Texture picker frame.
    lafi::frame* frm_texture =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_texture);
    gui->add("frm_texture", frm_texture);
    
    frm_texture->add(
        "but_back",
        new lafi::button(scr_w - 200, 8, scr_w - 104, 24, "Back")
    );
    frm_texture->add(
        "txt_name",
        new lafi::textbox(scr_w - 200, 40, scr_w - 48, 56)
    );
    frm_texture->add(
        "but_ok",
        new lafi::button(scr_w - 40, 32, scr_w - 8, 64, "Ok")
    );
    frm_texture->add(
        "lbl_suggestions",
        new lafi::label(scr_w - 200, 72, scr_w - 8, 88, "Suggestions:")
    );
    frm_texture->add(
        "frm_list",
        new lafi::frame(scr_w - 200, 96, scr_w - 32, scr_h - 56)
    );
    frm_texture->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24, 96, scr_w - 8, scr_h - 56)
    );
    
    
    //Properties -- texture picker.
    frm_texture->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
    };
    frm_texture->widgets["but_back"]->description =
        "Cancel.";
        
    frm_texture->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string n =
            (
                (lafi::textbox*)
                this->gui->widgets["frm_texture"]->widgets["txt_name"]
            )->text;
        if(n.empty()) return;
        lafi::widget* frm_sector =
            this->gui->widgets["frm_sectors"]->widgets["frm_sector"];
        (
            (lafi::button*)
            frm_sector->widgets["but_texture"]
        )->text = n;
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
        update_texture_suggestions(n);
        gui_to_sector();
    };
    
    ((lafi::textbox*) frm_texture->widgets["txt_name"])->enter_key_widget =
        frm_texture->widgets["but_ok"];
        
    frm_texture->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->gui->widgets["frm_texture"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(
                0,
                (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) /
                2 - 30 * dy
            );
        }
    };
    
    
    //Objects frame.
    lafi::frame* frm_objects =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_objects);
    gui->add("frm_objects", frm_objects);
    
    frm_objects->easy_row();
    frm_objects->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_objects->easy_row();
    frm_objects->easy_add(
        "but_new",
        new lafi::button(0, 0, 0, 0, "+"), 20, 32
    );
    frm_objects->easy_add(
        "but_sel_none",
        new lafi::button(0, 0, 0, 0, "None"), 20, 32
    );
    y = frm_objects->easy_row();
    
    lafi::frame* frm_object =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_object);
    frm_objects->add("frm_object", frm_object);
    
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_category",
        new lafi::label(0, 0, 0, 0, "Category:"), 70, 16
    );
    frm_object->easy_add(
        "but_rem",
        new lafi::button(0, 0, 0, 0, "-"), 15, 16
    );
    frm_object->easy_add(
        "but_duplicate",
        new lafi::button(0, 0, 0, 0, "x2"), 15, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_category",
        new lafi::button(0, 0, 0, 0), 100, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_type",
        new lafi::label(0, 0, 0, 0, "Type:"), 100, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_type",
        new lafi::button(0, 0, 0, 0), 100, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_angle",
        new lafi::label(0, 0, 0, 0, "Angle:"), 50, 16
    );
    frm_object->easy_add(
        "ang_angle",
        new lafi::angle_picker(0, 0, 0, 0), 50, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_vars",
        new lafi::label(0, 0, 0, 0, "Script variables:"), 100, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "txt_vars",
        new lafi::textbox(0, 0, 0, 0), 100, 16
    );
    frm_object->easy_row();
    
    
    //Properties -- objects.
    auto lambda_gui_to_mob = [this] (lafi::widget*) { gui_to_mob(); };
    
    frm_objects->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_objects->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_objects->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_OBJECT) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_OBJECT;
    };
    frm_objects->widgets["but_new"]->description =
        "Create a new object wherever you click.";
        
    frm_objects->widgets["but_sel_none"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_mob = NULL;
        mob_to_gui();
        if(sec_mode == ESM_DUPLICATE_OBJECT) sec_mode = ESM_NONE;
    };
    frm_objects->widgets["but_sel_none"]->description =
        "Deselect the current sector.";
        
    frm_object->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            if(cur_area_data.mob_generators[m] == cur_mob) {
                cur_area_data.mob_generators.erase(
                    cur_area_data.mob_generators.begin() + m
                );
                delete cur_mob;
                cur_mob = NULL;
                mob_to_gui();
                break;
            }
        }
    };
    frm_object->widgets["but_rem"]->description =
        "Delete the current object.";
        
    frm_object->widgets["but_duplicate"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        toggle_duplicate_mob_mode();
    };
    frm_object->widgets["but_duplicate"]->description =
        "Duplicate the current object (Ctrl+D).";
        
    frm_object->widgets["but_category"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_CATEGORY);
    };
    frm_object->widgets["but_category"]->description =
        "Choose the category of types of object.";
        
    frm_object->widgets["but_type"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_TYPE);
    };
    frm_object->widgets["but_type"]->description =
        "Choose the type this object is.";
        
    frm_object->widgets["ang_angle"]->lose_focus_handler =
        lambda_gui_to_mob;
    frm_object->widgets["ang_angle"]->description =
        "Angle the object is facing.";
        
    frm_object->widgets["txt_vars"]->lose_focus_handler =
        lambda_gui_to_mob;
    frm_object->widgets["txt_vars"]->description =
        "Extra variables (e.g.: sleep=y;jumping=n).";
        
    frm_object->register_accelerator(
        ALLEGRO_KEY_D, ALLEGRO_KEYMOD_CTRL,
        frm_object->widgets["but_duplicate"]
    );
    
    
    //Paths frame.
    lafi::frame* frm_paths =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_paths);
    gui->add("frm_paths", frm_paths);
    
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_create",
        new lafi::label(0, 0, 0, 0, "Create:"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_new_stop",
        new lafi::button(0, 0, 0, 0, "Stop"), 33, 32
    );
    frm_paths->easy_add(
        "but_new_link",
        new lafi::button(0, 0, 0, 0, "Link"), 33, 32
    );
    frm_paths->easy_add(
        "but_new_1wlink",
        new lafi::button(0, 0, 0, 0, "1WLink"), 33, 32
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_delete",
        new lafi::label(0, 0, 0, 0, "Delete:"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_del_stop",
        new lafi::button(0, 0, 0, 0, "Stop"), 33, 32
    );
    frm_paths->easy_add(
        "but_del_link",
        new lafi::button(0, 0, 0, 0, "Link"), 33, 32
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "chk_show_closest",
        new lafi::checkbox(0, 0, 0, 0, "Show closest stop"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "chk_show_path",
        new lafi::checkbox(0, 0, 0, 0, "Show calculated path"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_path_dist",
        new lafi::label(0, 0, 0, 0, "  Total dist.: 0"), 100, 16
    );
    frm_paths->easy_row();
    
    
    //Properties -- paths.
    frm_paths->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_paths->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_paths->widgets["but_new_stop"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_STOP) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_STOP;
    };
    frm_paths->widgets["but_new_stop"]->description =
        "Create new stops wherever you click.";
        
    frm_paths->widgets["but_new_link"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(
            sec_mode == ESM_NEW_LINK1 ||
            sec_mode == ESM_NEW_LINK2
        ) {
            sec_mode = ESM_NONE;
        } else {
            sec_mode = ESM_NEW_LINK1;
        }
    };
    frm_paths->widgets["but_new_link"]->description =
        "Click on two stops to connect them with a link.";
        
    frm_paths->widgets["but_new_1wlink"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(
            sec_mode == ESM_NEW_1WLINK1 ||
            sec_mode == ESM_NEW_1WLINK2
        ) {
            sec_mode = ESM_NONE;
        } else {
            sec_mode = ESM_NEW_1WLINK1;
        }
    };
    frm_paths->widgets["but_new_1wlink"]->description =
        "Click stop #1 then #2 for a one-way path link.";
        
    frm_paths->widgets["but_del_stop"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_DEL_STOP) sec_mode = ESM_NONE;
        else sec_mode = ESM_DEL_STOP;
    };
    frm_paths->widgets["but_del_stop"]->description =
        "Click stops to delete them.";
        
    frm_paths->widgets["but_del_link"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_DEL_LINK) sec_mode = ESM_NONE;
        else sec_mode = ESM_DEL_LINK;
    };
    frm_paths->widgets["but_del_link"]->description =
        "Click links to delete them.";
        
    frm_paths->widgets["chk_show_closest"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_closest_stop = !show_closest_stop;
    };
    frm_paths->widgets["chk_show_closest"]->description =
        "Show the closest stop to the cursor.";
        
    frm_paths->widgets["chk_show_path"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_path_preview = !show_path_preview;
        if(show_path_preview) {
            calculate_preview_path();
            show_widget(
                this->gui->widgets["frm_paths"]->widgets["lbl_path_dist"]
            );
        } else {
            hide_widget(
                this->gui->widgets["frm_paths"]->widgets["lbl_path_dist"]
            );
        }
    };
    frm_paths->widgets["chk_show_path"]->description =
        "Show path between draggable points A and B.";
        
    frm_paths->widgets["lbl_path_dist"]->description =
        "Total travel distance between A and B.";
        
    hide_widget(gui->widgets["frm_paths"]->widgets["lbl_path_dist"]);
    
    
    //Shadows frame.
    lafi::frame* frm_shadows =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_shadows);
    gui->add("frm_shadows", frm_shadows);
    
    frm_shadows->easy_row();
    frm_shadows->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_shadows->easy_row();
    frm_shadows->easy_add(
        "but_new",
        new lafi::button(0, 0, 0, 0, "+"), 20, 32
    );
    frm_shadows->easy_add(
        "but_sel_none",
        new lafi::button(0, 0, 0, 0, "None"), 20, 32
    );
    y = frm_shadows->easy_row();
    
    lafi::frame* frm_shadow =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_shadow);
    frm_shadows->add("frm_shadow", frm_shadow);
    
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "dum_1",
        new lafi::dummy(0, 0, 0, 0), 90, 16
    );
    frm_shadow->easy_add(
        "but_rem",
        new lafi::button(0, 0, 0, 0, "-"), 10, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_file",
        new lafi::label(0, 0, 0, 0, "File:"), 25, 16
    );
    frm_shadow->easy_add(
        "txt_file",
        new lafi::textbox(0, 0, 0, 0), 75, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X&Y:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_wh",
        new lafi::label(0, 0, 0, 0, "W&H:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_w",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_add(
        "txt_h",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_an",
        new lafi::label(0, 0, 0, 0, "Angle:"), 40, 16
    );
    frm_shadow->easy_add(
        "ang_an",
        new lafi::angle_picker(0, 0, 0, 0), 60, 24
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_al",
        new lafi::label(0, 0, 0, 0, "Opacity:"), 40, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "bar_al",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_sway",
        new lafi::label(0, 0, 0, 0, "Sway X&Y:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_sx",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_add(
        "txt_sy",
        new lafi::textbox(0, 0, 0, 0), 30, 16
    );
    frm_shadow->easy_row();
    
    
    //Properties -- shadows.
    auto lambda_gui_to_shadow = [this] (lafi::widget*) { gui_to_shadow(); };
    
    frm_shadows->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        sec_mode = ESM_NONE;
        shadow_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_shadows->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_shadows->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sec_mode == ESM_NEW_SHADOW) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_SHADOW;
    };
    frm_shadows->widgets["but_new"]->description =
        "Create a new tree shadow wherever you click.";
        
    frm_shadows->widgets["but_sel_none"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_shadow = NULL;
        shadow_to_gui();
    };
    frm_shadows->widgets["but_sel_none"]->description =
        "Deselect the current tree shadow.";
        
    frm_shadow->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            if(cur_area_data.tree_shadows[s] == cur_shadow) {
                cur_area_data.tree_shadows.erase(
                    cur_area_data.tree_shadows.begin() + s
                );
                delete cur_shadow;
                cur_shadow = NULL;
                shadow_to_gui();
                break;
            }
        }
    };
    frm_shadow->widgets["but_rem"]->description =
        "Delete the current tree shadow.";
        
    frm_shadow->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_file"]->description =
        "File name for the shadow's texture.";
        
    frm_shadow->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_x"]->description =
        "X position of the shadow's center.";
        
    frm_shadow->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_y"]->description =
        "Y position of the shadow's center.";
        
    frm_shadow->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_w"]->description =
        "Width of the shadow's image.";
        
    frm_shadow->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_h"]->description =
        "Height of the shadow's image.";
        
    frm_shadow->widgets["ang_an"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["ang_an"]->description =
        "Angle of the shadow's image.";
        
    frm_shadow->widgets["bar_al"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["bar_al"]->description =
        "How opaque the shadow's image is.";
        
    frm_shadow->widgets["txt_sx"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_sx"]->description =
        "Horizontal sway amount multiplier (0 = no sway).";
        
    frm_shadow->widgets["txt_sy"]->lose_focus_handler =
        lambda_gui_to_shadow;
    frm_shadow->widgets["txt_sy"]->description =
        "Vertical sway amount multiplier (0 = no sway).";
        
        
    //Guide frame.
    lafi::frame* frm_guide =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_guide);
    gui->add("frm_guide", frm_guide);
    
    frm_guide->easy_row();
    frm_guide->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "lbl_file",
        new lafi::label(0, 0, 0, 0, "File:"), 30, 16
    );
    frm_guide->easy_add(
        "txt_file",
        new lafi::textbox(0, 0, 0, 0), 70, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X&Y:"), 30, 16
    );
    frm_guide->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0), 35, 16
    );
    frm_guide->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0), 35, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "lbl_wh",
        new lafi::label(0, 0, 0, 0, "W&H:"), 30, 16
    );
    frm_guide->easy_add(
        "txt_w",
        new lafi::textbox(0, 0, 0, 0), 35, 16
    );
    frm_guide->easy_add(
        "txt_h",
        new lafi::textbox(0, 0, 0, 0), 35, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "chk_ratio",
        new lafi::checkbox(0, 0, 0, 0, "Keep aspect ratio"), 100, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "chk_mouse",
        new lafi::checkbox(0, 0, 0, 0, "Transform with mouse"), 100, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "lbl_alpha",
        new lafi::label(0, 0, 0, 0, "Opacity:"), 100, 16
    );
    frm_guide->easy_row();
    frm_guide->easy_add(
        "bar_alpha",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24
    );
    frm_guide->easy_row();
    
    
    //Properties -- guide.
    auto lambda_gui_to_guide =
    [this] (lafi::widget*) { gui_to_guide(); };
    auto lambda_gui_to_guide_click =
    [this] (lafi::widget*, int, int) { gui_to_guide(); };
    
    frm_guide->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        sec_mode = ESM_NONE;
        guide_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_guide->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_guide->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["txt_file"]->description =
        "Image file (on the Images folder) for the guide.";
        
    frm_guide->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["txt_x"]->description =
        "X of the top-left corner for the guide.";
        
    frm_guide->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["txt_y"]->description =
        "Y of the top-left corner for the guide.";
        
    frm_guide->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["txt_w"]->description =
        "Guide total width.";
        
    frm_guide->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["txt_h"]->description =
        "Guide total height.";
        
    frm_guide->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_gui_to_guide_click;
    frm_guide->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    frm_guide->widgets["chk_mouse"]->left_mouse_click_handler =
        lambda_gui_to_guide_click;
    frm_guide->widgets["chk_mouse"]->description =
        "If checked, use mouse buttons to move/stretch.";
        
    ((lafi::scrollbar*) frm_guide->widgets["bar_alpha"])->change_handler =
        lambda_gui_to_guide;
    frm_guide->widgets["bar_alpha"]->description =
        "How see-through the guide is.";
        
    guide_to_gui();
    
    
    //Review frame.
    lafi::frame* frm_review =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_review);
    gui->add("frm_review", frm_review);
    
    frm_review->easy_row();
    frm_review->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_find_errors",
        new lafi::button(0, 0, 0, 0, "Find errors"), 100, 24
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_lbl",
        new lafi::label(0, 0, 0, 0, "Error found:", ALLEGRO_ALIGN_CENTER),
        100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_1",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_2",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_3",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_4",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_goto_error",
        new lafi::button(0, 0, 0, 0, "Go to error"), 100, 24
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "chk_see_textures",
        new lafi::checkbox(0, 0, 0, 0, "See textures"), 100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "dum_1",
        new lafi::dummy(0, 0, 0, 0), 10, 16
    );
    frm_review->easy_add(
        "chk_shadows",
        new lafi::checkbox(0, 0, 0, 0, "See tree shadows"), 90, 16
    );
    frm_review->easy_row();
    update_review_frame();
    
    
    //Properties -- review.
    frm_review->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        sec_mode = ESM_NONE;
        error_type = EET_NONE_YET;
        update_review_frame();
        change_to_right_frame();
    };
    frm_review->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_review->widgets["but_find_errors"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        find_errors();
    };
    frm_review->widgets["but_find_errors"]->description =
        "Search for problems with the area.";
        
    frm_review->widgets["but_goto_error"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        goto_error();
    };
    frm_review->widgets["but_goto_error"]->description =
        "Focus the camera on the problem found, if applicable.";
        
    frm_review->widgets["chk_see_textures"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
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
    frm_review->widgets["chk_see_textures"]->description =
        "Preview how the textures will look like.";
        
    frm_review->widgets["chk_shadows"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_shadows = ((lafi::checkbox*) c)->checked;
        update_review_frame();
    };
    frm_review->widgets["chk_shadows"]->description =
        "Show tree shadows?";
        
        
    //Tools frame.
    lafi::frame* frm_tools =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_tools);
    gui->add("frm_tools", frm_tools);
    
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_resize",
        new lafi::label(0, 0, 0, 0, "Resize everything:"), 100, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_resize",
        new lafi::textbox(0, 0, 0, 0), 80, 16
    );
    frm_tools->easy_add(
        "but_resize",
        new lafi::button(0, 0, 0, 0, "Ok"), 20, 24
    );
    frm_tools->easy_row();
    
    
    //Properties -- tools.
    frm_tools->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_tools->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_tools->widgets["txt_resize"]->description =
        "Resize multiplier. (0.5 = half, 2 = double)";
        
    frm_tools->widgets["but_resize"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        resize_everything();
    };
    frm_tools->widgets["but_resize"]->description =
        "Resize all X/Y coordinates by the given amount.";
        
        
    //Options frame.
    lafi::frame* frm_options =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_options);
    gui->add("frm_options", frm_options);
    
    frm_options->easy_row();
    frm_options->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "but_load",
        new lafi::button(0, 0, 0, 0, "Reload area"), 100, 24
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "but_backup",
        new lafi::button(0, 0, 0, 0, "Load auto-backup"), 100, 24
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "lbl_grid",
        new lafi::label(0, 0, 0, 0, "Grid spacing: "), 70, 24
    );
    frm_options->easy_add(
        "but_grid_plus",
        new lafi::button(0, 0, 0, 0, "+"), 15, 24
    );
    frm_options->easy_add(
        "but_grid_minus",
        new lafi::button(0, 0, 0, 0, "-"), 15, 24
    );
    frm_options->easy_row();
    update_options_frame();
    
    
    //Properties -- options.
    frm_options->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = mode_before_options;
        change_to_right_frame();
    };
    frm_options->widgets["but_back"]->description =
        "Close the options.";
        
    frm_options->widgets["but_load"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->load_area(false);
    };
    frm_options->widgets["but_load"]->description =
        "Discard all changes made and load the area again.";
        
    frm_options->widgets["but_backup"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->load_backup();
    };
    frm_options->widgets["but_backup"]->description =
        "Discard all changes made and load the auto-backup.";
        
    frm_options->widgets["but_grid_plus"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(grid_interval == MAX_GRID_INTERVAL) return;
        grid_interval *= 2;
        update_options_frame();
    };
    frm_options->widgets["but_grid_plus"]->description =
        "Increase the spacing on the grid.";
        
    frm_options->widgets["but_grid_minus"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(grid_interval == MIN_GRID_INTERVAL) return;
        grid_interval *= 0.5;
        update_options_frame();
    };
    frm_options->widgets["but_grid_minus"]->description =
        "Decrease the spacing on the grid.";
        
        
    //Picker frame.
    lafi::frame* frm_picker =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add(
        "but_back",
        new lafi::button(scr_w - 200, 8, scr_w - 104, 24, "Back")
    );
    frm_picker->add(
        "frm_list",
        new lafi::frame(scr_w - 200, 40, scr_w - 32, scr_h - 56)
    );
    frm_picker->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24, 40, scr_w - 8, scr_h - 56)
    );
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_bottom"]);
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description =
        "Cancel.";
        
    frm_picker->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->gui->widgets["frm_picker"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(
                0,
                (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) /
                2 - 30 * dy
            );
        }
    };
    
    
    //Bottom bar.
    lafi::frame* frm_bottom =
        new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    
    frm_bottom->easy_row();
    frm_bottom->easy_add(
        "but_options",
        new lafi::button(0, 0, 0, 0, "Opt"), 25, 32
    );
    frm_bottom->easy_add(
        "but_guide",
        new lafi::button(0, 0, 0, 0, "G"), 25, 32
    );
    frm_bottom->easy_add(
        "but_save",
        new lafi::button(0, 0, 0, 0, "Save"), 25, 32
    );
    frm_bottom->easy_add(
        "but_quit",
        new lafi::button(0, 0, 0, 0, "Quit"), 25, 32
    );
    frm_bottom->easy_row();
    
    lafi::label* gui_status_bar =
        new lafi::label(0, scr_h - 16, scr_w - 208, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Properties -- bottom.
    frm_bottom->widgets["but_options"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(mode == EDITOR_MODE_OPTIONS) return;
        mode_before_options = mode;
        mode = EDITOR_MODE_OPTIONS;
        update_backup_status();
        change_to_right_frame();
        update_options_frame();
    };
    frm_bottom->widgets["but_options"]->description =
        "Options and misc. tools.";
        
    frm_bottom->widgets["but_guide"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_guide = !show_guide;
    };
    frm_bottom->widgets["but_guide"]->description =
        "Toggle the visibility of the guide.";
        
    frm_bottom->widgets["but_save"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        save_area(false);
        cur_sector = NULL;
        cur_mob = NULL;
        sector_to_gui();
        mob_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
        made_changes = false;
    };
    frm_bottom->widgets["but_save"]->description =
        "Save the area onto the files.";
        
    frm_bottom->widgets["but_quit"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            leave();
        }
    };
    frm_bottom->widgets["but_quit"]->description =
        "Quit the area editor.";
        
    disable_widget(frm_bottom->widgets["but_save"]);
    
    
    //Changes warning.
    lafi::frame* frm_changes =
        new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    hide_widget(frm_changes);
    gui->add("frm_changes", frm_changes);
    
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text1",
        new lafi::label(0, 0, 0, 0, "Warning: you have", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text2",
        new lafi::label(0, 0, 0, 0, "unsaved changes!", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->add(
        "but_ok",
        new lafi::button(scr_w - 40, scr_h - 40, scr_w - 8, scr_h - 8, "Ok")
    );
    
    
    //Properties -- changes warning.
    frm_changes->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) { close_changes_warning(); };
    
    
    cam_zoom = 1.0;
    cam_x = cam_y = 0.0;
    grid_interval = DEF_GRID_INTERVAL;
    show_closest_stop = false;
    area_name.clear();
    
    if(!auto_load_area.empty()) {
        area_name = auto_load_area;
        load_area(false);
    }
    
}


/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 * from_backup: If false, load it normally. If true, load from a backup, if any.
 */
void area_editor::load_area(const bool from_backup) {
    intersecting_edges.clear();
    non_simples.clear();
    lone_edges.clear();
    
    ::load_area(area_name, true, from_backup);
    ((lafi::button*) gui->widgets["frm_main"]->widgets["but_area"])->text =
        area_name;
    show_widget(gui->widgets["frm_main"]->widgets["frm_area"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_save"]);
    
    clear_area_textures();
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        check_edge_intersections(cur_area_data.vertexes[v]);
    }
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        string n = cur_area_data.sectors[s]->texture_info.file_name;
        if(n.empty()) continue;
        texture_uses_map[n]++;
    }
    for(auto u = texture_uses_map.begin(); u != texture_uses_map.end(); ++u) {
        texture_uses_vector.push_back(make_pair(u->first, u->second));
    }
    sort(
        texture_uses_vector.begin(), texture_uses_vector.end(),
    [] (pair<string, size_t> u1, pair<string, size_t> u2) -> bool {
        return u1.second > u2.second;
    }
    );
    texture_suggestions.clear();
    for(
        size_t u = 0;
        u < texture_uses_vector.size() && u < MAX_TEXTURE_SUGGESTIONS;
        ++u
    ) {
        texture_suggestions.push_back(
            texture_suggestion(texture_uses_vector[u].first)
        );
    }
    
    change_guide(guide_file_name);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
    
    error_type = EET_NONE_YET;
    error_sector_ptr = NULL;
    error_string.clear();
    error_vertex_ptr = NULL;
    
    backup_timer.start(editor_backup_interval);
    show_path_preview = false;
    (
        (lafi::checkbox*) gui->widgets["frm_paths"]->widgets["chk_show_path"]
    )->uncheck();
    hide_widget(gui->widgets["frm_paths"]->widgets["lbl_path_dist"]);
    path_preview.clear();
    path_preview_checkpoints_x[0] = -DEF_GRID_INTERVAL;
    path_preview_checkpoints_y[0] = 0;
    path_preview_checkpoints_x[1] = DEF_GRID_INTERVAL;
    path_preview_checkpoints_y[1] = 0;
    
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
 * Loads a backup file.
 */
void area_editor::load_backup() {
    if(!update_backup_status()) return;
    
    load_area(true);
    backup_timer.start(editor_backup_interval);
}


/* ----------------------------------------------------------------------------
 * Merges vertex 1 into vertex 2.
 * v1:               Vertex that is being moved and will be merged.
 * v2:               Vertex that is going to absorb v1.
 * affected_sectors: List of sectors that will be affected with this merge.
 */
void area_editor::merge_vertex(
    vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
) {
    //Find out what to do with every edge of the dragged vertex.
    for(size_t e = 0; e < v1->edges.size();) {
    
        bool was_deleted = false;
        edge* e_ptr = v1->edges[e];
        vertex* other_vertex =
            e_ptr->vertexes[0] == v1 ?
            e_ptr->vertexes[1] : e_ptr->vertexes[0];
            
        //Check if it's being squashed into non-existence.
        if(other_vertex == v2) {
        
            affected_sectors->insert(e_ptr->sectors[0]);
            affected_sectors->insert(e_ptr->sectors[1]);
            
            e_ptr->remove_from_vertexes();
            e_ptr->remove_from_sectors();
            
            //Clear it from the list of lone edges, if there.
            auto it = lone_edges.find(e_ptr);
            if(it != lone_edges.end()) lone_edges.erase(it);
            
            //Delete it.
            cur_area_data.remove_edge(e_ptr);
            was_deleted = true;
            
        } else {
        
            bool has_merged = false;
            //Check if the edge will be merged with another one.
            //These are edges that share a common vertex,
            //plus the moved/destination vertex.
            for(
                size_t de = 0; de < v2->edges.size();
                ++de
            ) {
            
                edge* de_ptr = v2->edges[de];
                vertex* d_other_vertex =
                    de_ptr->vertexes[0] == v2 ?
                    de_ptr->vertexes[1] : de_ptr->vertexes[0];
                    
                if(d_other_vertex == other_vertex) {
                    //The edge will be merged with this one.
                    has_merged = true;
                    affected_sectors->insert(e_ptr->sectors[0]);
                    affected_sectors->insert(e_ptr->sectors[1]);
                    affected_sectors->insert(de_ptr->sectors[0]);
                    affected_sectors->insert(de_ptr->sectors[1]);
                    
                    //Tell the destination edge's sectors
                    //to forget it; they'll be re-added later.
                    size_t old_de_nr =
                        de_ptr->remove_from_sectors();
                        
                    //Set the new sectors.
                    if(e_ptr->sector_nrs[0] == de_ptr->sector_nrs[0])
                        de_ptr->sector_nrs[0] = e_ptr->sector_nrs[1];
                    else if(e_ptr->sector_nrs[0] == de_ptr->sector_nrs[1])
                        de_ptr->sector_nrs[1] = e_ptr->sector_nrs[1];
                    else if(e_ptr->sector_nrs[1] == de_ptr->sector_nrs[0])
                        de_ptr->sector_nrs[0] = e_ptr->sector_nrs[0];
                    else if(e_ptr->sector_nrs[1] == de_ptr->sector_nrs[1])
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
                    
                    //Delete it.
                    cur_area_data.remove_edge(e_ptr);
                    was_deleted = true;
                    
                    break;
                }
            }
            
            //If it's matchless, that means it'll just be joined to
            //the group of edges on the destination vertex.
            if(!has_merged) {
                v2->edge_nrs.push_back(v1->edge_nrs[e]);
                v2->edges.push_back(v1->edges[e]);
                unsigned char n = (e_ptr->vertexes[0] == v1 ? 0 : 1);
                e_ptr->vertexes[n] = v2;
                e_ptr->vertex_nrs[n] =
                    cur_area_data.find_vertex_nr(e_ptr->vertexes[n]);
            }
        }
        
        if(!was_deleted) ++e;
        
    }
    
    v2->fix_pointers(cur_area_data);
    
    //Check if any of the final edges have the same sector
    //on both sides. If so, delete them.
    for(size_t ve = 0; ve < v2->edges.size(); ) {
        edge* ve_ptr = v2->edges[ve];
        if(ve_ptr->sectors[0] == ve_ptr->sectors[1]) {
            ve_ptr->remove_from_sectors();
            ve_ptr->remove_from_vertexes();
            cur_area_data.remove_edge(v2->edge_nrs[ve]);
        } else {
            ++ve;
        }
    }
    
    //Delete the old vertex.
    cur_area_data.remove_vertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < cur_area_data.vertexes.size();) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        if(v_ptr->edges.empty()) {
            cur_area_data.remove_vertex(v);
        } else {
            ++v;
        }
    }
    for(size_t s = 0; s < cur_area_data.sectors.size();) {
        sector* s_ptr = cur_area_data.sectors[s];
        if(s_ptr->edges.empty()) {
            cur_area_data.remove_sector(s);
        } else {
            ++s;
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Loads the current mob's data onto the gui.
 */
void area_editor::mob_to_gui() {
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_objects"]->widgets["frm_object"];
        
    if(!cur_mob) {
        hide_widget(f);
    } else {
        show_widget(f);
        
        (
            (lafi::angle_picker*) f->widgets["ang_angle"]
        )->set_angle_rads(cur_mob->angle);
        ((lafi::textbox*) f->widgets["txt_vars"])->text = cur_mob->vars;
        
        ((lafi::button*) f->widgets["but_category"])->text =
            mob_categories.get_pname(cur_mob->category);
            
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
    
        for(
            unsigned char f = 0; f < mob_categories.get_nr_of_categories();
            ++f
        ) {
            //0 is none.
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
        b->left_mouse_click_handler =
        [name, type, this] (lafi::widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    (
        (lafi::scrollbar*) gui->widgets["frm_picker"]->widgets["bar_scroll"]
    )->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void area_editor::pick(string name, unsigned char type) {
    change_to_right_frame();
    show_widget(gui->widgets["frm_bottom"]);
    
    if(type == AREA_EDITOR_PICKER_AREA) {
    
        area_name = name;
        load_area(false);
        
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
 * Adds texture suggestions to the GUI frame.
 */
void area_editor::populate_texture_suggestions() {
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_texture"]->widgets["frm_list"];
        
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    if(texture_suggestions.empty()) return;
    
    f->easy_reset();
    f->easy_row();
    
    for(size_t s = 0; s < texture_suggestions.size(); ++s) {
    
        string name = texture_suggestions[s].name;
        lafi::image* i =
            new lafi::image(0, 0, 0, 0, texture_suggestions[s].bmp);
        lafi::label* l =
            new lafi::label(0, 0, 0, 0, name);
            
        auto lambda = [name, this] (lafi::widget*, int, int) {
            lafi::widget* frm_sector =
                this->gui->widgets["frm_sectors"]->widgets["frm_sector"];
            (
                (lafi::button*)
                frm_sector->widgets["but_texture"]
            )->text = name;
            mode = EDITOR_MODE_SECTORS;
            change_to_right_frame();
            update_texture_suggestions(name);
            gui_to_sector();
        };
        i->left_mouse_click_handler = lambda;
        l->left_mouse_click_handler = lambda;
        f->easy_add("img_" + i2s(s), i, 48, 48, lafi::EASY_FLAG_WIDTH_PX);
        f->easy_add("lbl_" + i2s(s), l, 96, 48, lafi::EASY_FLAG_WIDTH_PX);
        f->easy_row(0);
    }
    
    (
        (lafi::scrollbar*)
        gui->widgets["frm_texture"]->widgets["bar_scroll"]
    )->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Updates the list of texture suggestions, adding a new one or bumping it up.
 */
void area_editor::update_texture_suggestions(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t s = 0; s < texture_suggestions.size(); ++s) {
        if(texture_suggestions[s].name == n) {
            pos = s;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        texture_suggestions.insert(
            texture_suggestions.begin(),
            texture_suggestion(n)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        texture_suggestion s = texture_suggestions[pos];
        texture_suggestions.erase(texture_suggestions.begin() + pos);
        texture_suggestions.insert(texture_suggestions.begin(), s);
    }
    
    if(texture_suggestions.size() > MAX_TEXTURE_SUGGESTIONS) {
        texture_suggestions.erase(
            texture_suggestions.begin() + texture_suggestions.size() - 1
        );
    }
}


/* ----------------------------------------------------------------------------
 * Resizes all X and Y coordinates etc. by a multiplier.
 */
void area_editor::resize_everything() {
    lafi::textbox* txt_resize =
        (lafi::textbox*) gui->widgets["frm_tools"]->widgets["txt_resize"];
    float mult = s2f(txt_resize->text);
    
    if(mult == 0) return;
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        v_ptr->x *= mult;
        v_ptr->y *= mult;
    }
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        s_ptr->texture_info.scale_x *= mult;
        s_ptr->texture_info.scale_y *= mult;
        s_ptr->texture_info.trans_x *= mult;
        s_ptr->texture_info.trans_y *= mult;
        s_ptr->triangles.clear();
        triangulate(s_ptr);
    }
    
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        m_ptr->x *= mult;
        m_ptr->y *= mult;
    }
    
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        s_ptr->x *= mult;
        s_ptr->y *= mult;
    }
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        cur_area_data.path_stops[s]->calculate_dists();
    }
    
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
        s_ptr->x      *= mult;
        s_ptr->y      *= mult;
        s_ptr->w      *= mult;
        s_ptr->h      *= mult;
        s_ptr->sway_x *= mult;
        s_ptr->sway_y *= mult;
    }
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the area onto the disk.
 * to_backup: If false, save normally. If true, save to an auto-backup file.
 */
void area_editor::save_area(const bool to_backup) {
    data_node geometry_file = data_node("", "");
    
    //Vertexes.
    data_node* vertexes_node = new data_node("vertexes", "");
    geometry_file.add(vertexes_node);
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        data_node* vertex_node =
            new data_node("v", f2s(v_ptr->x) + " " + f2s(v_ptr->y));
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
            if(e_ptr->sector_nrs[s] == INVALID) s_str += "-1";
            else s_str += i2s(e_ptr->sector_nrs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        edge_node->add(new data_node("s", s_str));
        edge_node->add(
            new data_node(
                "v",
                i2s(e_ptr->vertex_nrs[0]) + " " + i2s(e_ptr->vertex_nrs[1])
            )
        );
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    geometry_file.add(sectors_node);
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        data_node* sector_node = new data_node("s", "");
        sectors_node->add(sector_node);
        
        if(s_ptr->type != SECTOR_TYPE_NORMAL) {
            sector_node->add(
                new data_node("type", sector_types.get_name(s_ptr->type))
            );
        }
        sector_node->add(new data_node("z", f2s(s_ptr->z)));
        if(s_ptr->brightness != DEF_SECTOR_BRIGHTNESS) {
            sector_node->add(
                new data_node("brightness", i2s(s_ptr->brightness))
            );
        }
        if(!s_ptr->tag.empty()) {
            sector_node->add(new data_node("tag", s_ptr->tag));
        }
        if(s_ptr->fade) {
            sector_node->add(new data_node("fade", b2s(s_ptr->fade)));
        }
        if(s_ptr->always_cast_shadow) {
            sector_node->add(
                new data_node(
                    "always_cast_shadow",
                    b2s(s_ptr->always_cast_shadow)
                )
            );
        }
        if(!s_ptr->hazards_str.empty()) {
            sector_node->add(new data_node("hazards", s_ptr->hazards_str));
            sector_node->add(
                new data_node(
                    "hazards_floor",
                    b2s(s_ptr->hazard_floor)
                )
            );
        }
        
        if(!s_ptr->texture_info.file_name.empty()) {
            sector_node->add(
                new data_node(
                    "texture",
                    s_ptr->texture_info.file_name
                )
            );
        }
        
        if(s_ptr->texture_info.rot != 0) {
            sector_node->add(
                new data_node(
                    "texture_rotate",
                    f2s(s_ptr->texture_info.rot)
                )
            );
        }
        if(
            s_ptr->texture_info.scale_x != 1 ||
            s_ptr->texture_info.scale_y != 1
        ) {
            sector_node->add(
                new data_node(
                    "texture_scale",
                    f2s(s_ptr->texture_info.scale_x) + " " +
                    f2s(s_ptr->texture_info.scale_y)
                )
            );
        }
        if(
            s_ptr->texture_info.trans_x != 0 ||
            s_ptr->texture_info.trans_y != 0
        ) {
            sector_node->add(
                new data_node(
                    "texture_trans",
                    f2s(s_ptr->texture_info.trans_x) + " " +
                    f2s(s_ptr->texture_info.trans_y)
                )
            );
        }
        if(
            s_ptr->texture_info.tint.r != 1.0 ||
            s_ptr->texture_info.tint.g != 1.0 ||
            s_ptr->texture_info.tint.b != 1.0 ||
            s_ptr->texture_info.tint.a != 1.0
        ) {
            sector_node->add(
                new data_node("texture_tint", c2s(s_ptr->texture_info.tint))
            );
        }
        
    }
    
    //Mobs.
    data_node* mobs_node = new data_node("mobs", "");
    geometry_file.add(mobs_node);
    
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        data_node* mob_node =
            new data_node(mob_categories.get_sname(m_ptr->category), "");
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
        
        path_stop_node->add(
            new data_node("pos", f2s(s_ptr->x) + " " + f2s(s_ptr->y))
        );
        
        data_node* links_node = new data_node("links", "");
        path_stop_node->add(links_node);
        
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            path_link* l_ptr = &s_ptr->links[l];
            data_node* link_node = new data_node("nr", i2s(l_ptr->end_nr));
            links_node->add(link_node);
        }
        
    }
    
    //Tree shadows.
    data_node* shadows_node = new data_node("tree_shadows", "");
    geometry_file.add(shadows_node);
    
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
        data_node* shadow_node = new data_node("shadow", "");
        shadows_node->add(shadow_node);
        
        shadow_node->add(
            new data_node("pos", f2s(s_ptr->x) + " " + f2s(s_ptr->y))
        );
        shadow_node->add(
            new data_node("size", f2s(s_ptr->w) + " " + f2s(s_ptr->h))
        );
        if(s_ptr->angle != 0) {
            shadow_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        if(s_ptr->alpha != 255) {
            shadow_node->add(new data_node("alpha", i2s(s_ptr->alpha)));
        }
        shadow_node->add(new data_node("file", s_ptr->file_name));
        shadow_node->add(
            new data_node("sway", f2s(s_ptr->sway_x) + " " + f2s(s_ptr->sway_y))
        );
        
    }
    
    //Editor guide.
    geometry_file.add(new data_node("guide_file_name", guide_file_name));
    geometry_file.add(new data_node("guide_x",         f2s(guide_x)));
    geometry_file.add(new data_node("guide_y",         f2s(guide_y)));
    geometry_file.add(new data_node("guide_w",         f2s(guide_w)));
    geometry_file.add(new data_node("guide_h",         f2s(guide_h)));
    geometry_file.add(new data_node("guide_alpha",     i2s(guide_a)));
    
    
    geometry_file.save_file(
        AREA_FOLDER + "/" + area_name +
        (to_backup ? "/Geometry_backup.txt" : "/Geometry.txt")
    );
    
    backup_timer.start(editor_backup_interval);
}


/* ----------------------------------------------------------------------------
 * Saves the area onto a backup file.
 */
void area_editor::save_backup() {
    save_area(true);
    update_backup_status();
}


/* ----------------------------------------------------------------------------
 * Loads the current sector's data onto the gui.
 */
void area_editor::sector_to_gui() {
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_sectors"]->widgets["frm_sector"];
    if(cur_sector) {
        show_widget(f);
        
        ((lafi::textbox*) f->widgets["txt_z"])->text =
            f2s(cur_sector->z);
        ((lafi::checkbox*) f->widgets["chk_fade"])->set(
            cur_sector->fade
        );
        ((lafi::checkbox*) f->widgets["chk_shadow"])->set(
            cur_sector->always_cast_shadow
        );
        ((lafi::button*) f->widgets["but_texture"])->text =
            cur_sector->texture_info.file_name;
        ((lafi::scrollbar*) f->widgets["bar_brightness"])->set_value(
            cur_sector->brightness, false
        );
        ((lafi::textbox*) f->widgets["txt_brightness"])->text =
            i2s(cur_sector->brightness);
        ((lafi::textbox*) f->widgets["txt_tag"])->text =
            cur_sector->tag;
        ((lafi::button*) f->widgets["but_type"])->text =
            sector_types.get_name(cur_sector->type);
        ((lafi::textbox*) f->widgets["txt_hazards"])->text =
            cur_sector->hazards_str;
        ((lafi::checkbox*) f->widgets["chk_hazards_floor"])->set(
            cur_sector->hazard_floor
        );
        
        if(cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            disable_widget(f->widgets["chk_fade"]);
        } else {
            enable_widget(f->widgets["chk_fade"]);
        }
        
        if(cur_sector->fade || cur_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            disable_widget(f->widgets["but_texture"]);
            disable_widget(f->widgets["but_adv"]);
            disable_widget(f->widgets["bar_brightness"]);
            disable_widget(f->widgets["txt_brightness"]);
        } else {
            enable_widget(f->widgets["but_texture"]);
            enable_widget(f->widgets["but_adv"]);
            enable_widget(f->widgets["bar_brightness"]);
            enable_widget(f->widgets["txt_brightness"]);
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
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_shadows"]->widgets["frm_shadow"];
    if(cur_shadow) {
    
        show_widget(f);
        ((lafi::textbox*) f->widgets["txt_x"])->text =
            f2s(cur_shadow->x);
        ((lafi::textbox*) f->widgets["txt_y"])->text =
            f2s(cur_shadow->y);
        ((lafi::textbox*) f->widgets["txt_w"])->text =
            f2s(cur_shadow->w);
        ((lafi::textbox*) f->widgets["txt_h"])->text =
            f2s(cur_shadow->h);
        ((lafi::angle_picker*) f->widgets["ang_an"])->set_angle_rads(
            cur_shadow->angle
        );
        ((lafi::scrollbar*) f->widgets["bar_al"])->set_value(
            cur_shadow->alpha, false
        );
        ((lafi::textbox*) f->widgets["txt_file"])->text =
            cur_shadow->file_name;
        ((lafi::textbox*) f->widgets["txt_sx"])->text =
            f2s(cur_shadow->sway_x);
        ((lafi::textbox*) f->widgets["txt_sy"])->text =
            f2s(cur_shadow->sway_y);
            
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
    return round(c / grid_interval) * grid_interval;
}

/* ----------------------------------------------------------------------------
 * Toggles between normal mode and mob duplication mode.
 */
void area_editor::toggle_duplicate_mob_mode() {
    if(sec_mode == ESM_DUPLICATE_OBJECT) sec_mode = ESM_NONE;
    else sec_mode = ESM_DUPLICATE_OBJECT;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void area_editor::unload() {
    //TODO
    cur_mob = NULL;
    cur_area_data.clear();
    delete(gui->style);
    delete(gui);
    
    unload_hazards();
    unload_status_types();
}


/* ----------------------------------------------------------------------------
 * Reads the area's backup file, and sets the "load backup" button's
 * availability accordingly.
 * Returns true if it exists, false if not.
 */
bool area_editor::update_backup_status() {
    disable_widget(gui->widgets["frm_options"]->widgets["but_backup"]);
    
    if(area_name.empty()) return false;
    
    data_node file(AREA_FOLDER + "/" + area_name + "/Geometry_backup.txt");
    if(!file.file_was_opened) return false;
    
    enable_widget(gui->widgets["frm_options"]->widgets["but_backup"]);
    return true;
}


/* ----------------------------------------------------------------------------
 * Updates the widgets on the options frame.
 */
void area_editor::update_options_frame() {
    ((lafi::label*) gui->widgets["frm_options"]->widgets["lbl_grid"])->text =
        "Grid: " + i2s(grid_interval);
    lafi::widget* but_load = gui->widgets["frm_options"]->widgets["but_load"];
    if(area_name.empty()) {
        disable_widget(but_load);
    } else {
        enable_widget(but_load);
    }
}


/* ----------------------------------------------------------------------------
 * Updates the widgets on the review frame.
 */
void area_editor::update_review_frame() {
    lafi::button* but_goto_error =
        (lafi::button*) gui->widgets["frm_review"]->widgets["but_goto_error"];
    lafi::label* lbl_error_1 =
        (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_1"];
    lafi::label* lbl_error_2 =
        (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_2"];
    lafi::label* lbl_error_3 =
        (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_3"];
    lafi::label* lbl_error_4 =
        (lafi::label*) gui->widgets["frm_review"]->widgets["lbl_error_4"];
        
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
        
    } else if(error_type == EET_INTERSECTING_EDGES) {
    
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
        
        float a =
            atan2(
                ei_ptr->e1->vertexes[1]->y - ei_ptr->e1->vertexes[0]->y,
                ei_ptr->e1->vertexes[1]->x - ei_ptr->e1->vertexes[0]->x
            );
        dist d(
            ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y,
            ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y
        );
        
        lbl_error_3->text =
            "(" + f2s(
                floor(ei_ptr->e1->vertexes[0]->x + cos(a) * u *
                      d.to_float())
            ) + "," + f2s(
                floor(ei_ptr->e1->vertexes[0]->y + sin(a) * u *
                      d.to_float())
            ) + ")!";
            
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
        
    } else if(error_type == EET_MISSING_LEADER) {
    
        lbl_error_1->text = "No leader found!";
        lbl_error_2->text = "You need at least";
        lbl_error_3->text = "one to play.";
        disable_widget(gui->widgets["frm_review"]->widgets["but_goto_error"]);
        
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
    
    (
        (lafi::checkbox*)
        gui->widgets["frm_review"]->widgets["chk_see_textures"]
    )->set(sec_mode == ESM_TEXTURE_VIEW);
    (
        (lafi::checkbox*)
        gui->widgets["frm_review"]->widgets["chk_shadows"]
    )->set(show_shadows);
}


/* ----------------------------------------------------------------------------
 * Leaves the area editor, with a fade out.
 */
void area_editor::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


/* ----------------------------------------------------------------------------
 * Sets the guide's file name.
 */
void area_editor::set_guide_file_name(string n) {
    guide_file_name = n;
}


/* ----------------------------------------------------------------------------
 * Sets the guide's X coordinate.
 */
void area_editor::set_guide_x(float x) {
    guide_x = x;
}


/* ----------------------------------------------------------------------------
 * Sets the guide's Y coordinate.
 */
void area_editor::set_guide_y(float y) {
    guide_y = y;
}


/* ----------------------------------------------------------------------------
 * Sets the guide's width.
 */
void area_editor::set_guide_w(float w) {
    guide_w = w;
}


/* ----------------------------------------------------------------------------
 * Sets the guide's height.
 */
void area_editor::set_guide_h(float h) {
    guide_h = h;
}


/* ----------------------------------------------------------------------------
 * Sets the guide's alpha.
 */
void area_editor::set_guide_a(unsigned char a) {
    guide_a = a;
}


/* ----------------------------------------------------------------------------
 * Creates a texture suggestion.
 */
texture_suggestion::texture_suggestion(const string &n) :
    bmp(NULL),
    name(n) {
    
    bmp = bitmaps.get("Textures/" + name, NULL);
}


/* ----------------------------------------------------------------------------
 * Destroys a texture suggestion.
 */
texture_suggestion::~texture_suggestion() {
    bitmaps.detach(name);
}
