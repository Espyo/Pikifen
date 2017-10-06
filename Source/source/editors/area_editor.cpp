/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General area editor-related functions.
 */

#include <algorithm>

#include "area_editor.h"
#include "../functions.h"
#include "../load.h"
#include "../vars.h"

using namespace std;

//Scale the debug text by this much.
const float area_editor::DEBUG_TEXT_SCALE = 1.5f;
//Default grid interval.
const float area_editor::DEF_GRID_INTERVAL = 32.0f;
//Time until the next click is no longer considered a double-click.
const float area_editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How much to zoom in/out with the keyboard keys.
const float area_editor::KEYBOARD_CAM_ZOOM = 0.25f;
//Maximum number of texture suggestions.
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
//If the mouse is dragged outside of this range, that's a real drag.
const float area_editor::MOUSE_DRAG_CONFIRM_RANGE = 4.0f;
//Thickness to use when drawing a path link line.
const float area_editor::PATH_LINK_THICKNESS = 2.0f;
//Radius to use when drawing a path stop circle.
const float area_editor::PATH_STOP_RADIUS = 16.0f;
//Color of a selected element, or the selection box.
const unsigned char area_editor::SELECTION_COLOR[3] = {255, 215, 0};
//Speed at which the selection effect's "wheel" spins, in radians per second.
const float area_editor::SELECTION_EFFECT_SPEED = M_PI * 4;
//Maximum zoom level possible in the editor.
const float area_editor::ZOOM_MAX_LEVEL_EDITOR = 8.0f;
//Minimum zoom level possible in the editor.
const float area_editor::ZOOM_MIN_LEVEL_EDITOR = 0.01f;

const string area_editor::EDITOR_ICONS_FOLDER_NAME = "Editor_icons";
const string area_editor::ICON_DELETE =
    EDITOR_ICONS_FOLDER_NAME + "/Delete.png";
const string area_editor::ICON_DELETE_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_link.png";
const string area_editor::ICON_DELETE_STOP =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_stop.png";
const string area_editor::ICON_DUPLICATE =
    EDITOR_ICONS_FOLDER_NAME + "/Duplicate.png";
const string area_editor::ICON_EXIT =
    EDITOR_ICONS_FOLDER_NAME + "/Exit.png";
const string area_editor::ICON_NEW =
    EDITOR_ICONS_FOLDER_NAME + "/New.png";
const string area_editor::ICON_NEW_1WAY_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/New_1wlink.png";
const string area_editor::ICON_NEW_CIRCLE_SECTOR =
    EDITOR_ICONS_FOLDER_NAME + "/New_circle_sector.png";
const string area_editor::ICON_NEW_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/New_link.png";
const string area_editor::ICON_NEW_STOP =
    EDITOR_ICONS_FOLDER_NAME + "/New_stop.png";
const string area_editor::ICON_NEXT =
    EDITOR_ICONS_FOLDER_NAME + "/Next.png";
const string area_editor::ICON_OPTIONS =
    EDITOR_ICONS_FOLDER_NAME + "/Options.png";
const string area_editor::ICON_PREVIOUS =
    EDITOR_ICONS_FOLDER_NAME + "/Previous.png";
const string area_editor::ICON_REFERENCE =
    EDITOR_ICONS_FOLDER_NAME + "/Reference.png";
const string area_editor::ICON_SAVE =
    EDITOR_ICONS_FOLDER_NAME + "/Save.png";


/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    state(EDITOR_STATE_MAIN),
    backup_timer(editor_backup_interval),
    debug_edge_nrs(false),
    debug_sector_nrs(false),
    debug_triangulation(false),
    debug_vertex_nrs(false),
    double_click_time(0),
    grid_interval(DEF_GRID_INTERVAL),
    is_ctrl_pressed(false),
    is_shift_pressed(false),
    is_gui_focused(false),
    last_mouse_click(INVALID),
    mouse_drag_confirmed(false),
    path_preview_timer(0),
    selecting(false),
    selection_effect(0),
    show_reference(false) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given its name.
 */
void area_editor::create_new_from_picker(const string &name) {
    //TODO;
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {

    gui->tick(delta_t);
    
    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    path_preview_timer.tick(delta_t);
    
    if(!cur_area_name.empty() && editor_backup_interval > 0) {
        backup_timer.tick(delta_t);
    }
    
    fade_mgr.tick(delta_t);
    
    selection_effect += SELECTION_EFFECT_SPEED * delta_t;
    
}


/* ----------------------------------------------------------------------------
 * Returns the edge currently under the mouse, or NULL if none.
 */
edge* area_editor::get_edge_under_mouse() {
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        
        if(!is_edge_valid(e_ptr)) continue;
        
        if(
            circle_intersects_line(
                mouse_cursor_w, 8 / cam_zoom,
                point(
                    e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                ),
                point(
                    e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                )
            )
        ) {
            return e_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the radius of the specific mob generator. Normally, this returns the
 * type's radius, but if the type/radius is invalid, it returns a default.
 */
float area_editor::get_mob_gen_radius(mob_gen* m) {
    return m->type ? m->type->radius == 0 ? 16 : m->type->radius : 16;
}


/* ----------------------------------------------------------------------------
 * Returns the mob currently under the mouse, or NULL if none.
 */
mob_gen* area_editor::get_mob_under_mouse() {
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        
        if(
            dist(m_ptr->pos, mouse_cursor_w) <= get_mob_gen_radius(m_ptr)
        ) {
            return m_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the path stop currently under the mouse, or NULL if none.
 */
path_stop* area_editor::get_path_stop_under_mouse() {
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        
        if(dist(s_ptr->pos, mouse_cursor_w) <= PATH_STOP_RADIUS) {
            return s_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the sector currently under the mouse, or NULL if none.
 */
sector* area_editor::get_sector_under_mouse() {
    return get_sector(mouse_cursor_w, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Returns the vertex currently under the mouse, or NULL if none.
 */
vertex* area_editor::get_vertex_under_mouse() {
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        
        if(
            rectangles_intersect(
                mouse_cursor_w - (4 / cam_zoom),
                mouse_cursor_w + (4 / cam_zoom),
                point(
                    v_ptr->x - (4 / cam_zoom),
                    v_ptr->y - (4 / cam_zoom)
                ),
                point(
                    v_ptr->x + (4 / cam_zoom),
                    v_ptr->y + (4 / cam_zoom)
                )
            )
        ) {
            return v_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Homogenizes all selected mobs,
 * based on the one at the head of the selection.
 */
void area_editor::homogenize_selected_mobs() {
    mob_gen* base = *selected_mobs.begin();
    for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
        if(m == selected_mobs.begin()) continue;
        mob_gen* m_ptr = *m;
        m_ptr->category = base->category;
        m_ptr->type = base->type;
        m_ptr->angle = base->angle;
        m_ptr->vars = base->vars;
    }
}


/* ----------------------------------------------------------------------------
 * Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void area_editor::homogenize_selected_sectors() {
    sector* base = *selected_sectors.begin();
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        if(s == selected_sectors.begin()) continue;
        sector* s_ptr = *s;
        s_ptr->type = base->type;
        s_ptr->z = base->z;
        s_ptr->tag = base->tag;
        s_ptr->hazard_floor = base->hazard_floor;
        s_ptr->hazards_str = base->hazards_str;
        s_ptr->brightness = base->brightness;
        update_sector_texture(s_ptr, base->texture_info.file_name);
        s_ptr->texture_info.rot = base->texture_info.rot;
        s_ptr->texture_info.scale = base->texture_info.scale;
        s_ptr->texture_info.tint = base->texture_info.tint;
        s_ptr->texture_info.translation = base->texture_info.translation;
        s_ptr->always_cast_shadow = base->always_cast_shadow;
        s_ptr->fade = base->fade;
    }
}


/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 * from_backup: If false, load it normally. If true, load from a backup, if any.
 */
void area_editor::load_area(const bool from_backup) {
    clear_current_area();
    
    ::load_area(cur_area_name, true, from_backup);
    
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
    
    for(
        size_t u = 0;
        u < texture_uses_vector.size() && u < MAX_TEXTURE_SUGGESTIONS;
        ++u
    ) {
        texture_suggestions.push_back(
            texture_suggestion(texture_uses_vector[u].first)
        );
    }
    
    //TODO change_reference(reference_file_name);
    
    enable_widget(gui->widgets["frm_options"]->widgets["but_load"]);
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void area_editor::unload() {
    //TODO
    clear_current_area();
    
    delete(gui->style);
    delete(gui);
    
    unload_hazards();
    unload_mob_types(false);
    unload_status_types(false);
    
    icons.clear();
}


/* ----------------------------------------------------------------------------
 * Updates a sector's texture.
 */
void area_editor::update_sector_texture(sector* s_ptr, const string file_name) {
    bitmaps.detach(
        TEXTURES_FOLDER_NAME + "/" + s_ptr->texture_info.file_name
    );
    s_ptr->texture_info.file_name = file_name;
    s_ptr->texture_info.bitmap =
        bitmaps.get(TEXTURES_FOLDER_NAME + "/" + file_name);
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
        texture_suggestions[texture_suggestions.size() - 1].destroy();
        texture_suggestions.erase(
            texture_suggestions.begin() + texture_suggestions.size() - 1
        );
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void area_editor::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + gui_x / 2.0 / cam_zoom,
        -cam_pos.y + status_bar_y / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Zooms in or out to a specific amount, optionally keeping the mouse cursor
 * in the same spot.
 */
void area_editor::zoom(const float new_zoom, const bool anchor_cursor) {
    cam_zoom =
        clamp(new_zoom, ZOOM_MIN_LEVEL_EDITOR, ZOOM_MAX_LEVEL_EDITOR);
        
    if(anchor_cursor) {
        //Keep a backup of the old mouse coordinates.
        point old_mouse_pos = mouse_cursor_w;
        
        //Figure out where the mouse will be after the zoom.
        update_transformations();
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        
        //Readjust the transformation by shifting the camera
        //so that the cursor ends up where it was before.
        cam_pos.x += (old_mouse_pos.x - mouse_cursor_w.x);
        cam_pos.y += (old_mouse_pos.y - mouse_cursor_w.y);
    }
    
    update_transformations();
}


/* ----------------------------------------------------------------------------
 * Creates a texture suggestion.
 */
area_editor::texture_suggestion::texture_suggestion(const string &n) :
    bmp(NULL),
    name(n) {
    
    bmp = bitmaps.get(TEXTURES_FOLDER_NAME + "/" + name, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Destroys a texture suggestion.
 */
void area_editor::texture_suggestion::destroy() {
    bitmaps.detach(TEXTURES_FOLDER_NAME + "/" + name);
}
