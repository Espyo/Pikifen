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
#include <iomanip>
#include <unordered_set>

#include <allegro5/allegro_primitives.h>

#include "area_editor.h"
#include "../drawing.h"
#include "../functions.h"
#include "../geometry_utils.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/const.h"
#include "../LAFI/frame.h"
#include "../LAFI/gui.h"
#include "../LAFI/image.h"
#include "../LAFI/minor.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../vars.h"


//Radius to use when drawing a cross-section point.
const float  area_editor::CROSS_SECTION_POINT_RADIUS = 8.0f;
//Scale the debug text by this much.
const float  area_editor::DEBUG_TEXT_SCALE = 1.5f;
//Default grid interval.
const float  area_editor::DEF_GRID_INTERVAL = 32.0f;
//Maximum grid interval.
const float  area_editor::MAX_GRID_INTERVAL = 4096;
//Maximum number of points that a circle sector can be created with.
const size_t area_editor::MAX_CIRCLE_SECTOR_POINTS = 32;
//Maximum number of texture suggestions.
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
//Minimum number of points that a circle sector can be created with.
const size_t area_editor::MIN_CIRCLE_SECTOR_POINTS = 3;
//Minimum grid interval.
const float  area_editor::MIN_GRID_INTERVAL = 2;
//Thickness to use when drawing a path link line.
const float  area_editor::PATH_LINK_THICKNESS = 2.0f;
//Radius to use when drawing a path preview checkpoint.
const float  area_editor::PATH_PREVIEW_CHECKPOINT_RADIUS = 8.0f;
//Only fetch the path these many seconds after the player stops the checkpoints.
const float  area_editor::PATH_PREVIEW_TIMEOUT_DUR = 0.1f;
//Scale the letters on the "points" of various features by this much.
const float  area_editor::POINT_LETTER_TEXT_SCALE = 1.5f;
//Radius to use when drawing a path stop circle.
const float  area_editor::STOP_RADIUS = 16.0f;
//Minimum distance between two sectors for them to merge.
const float  area_editor::VERTEX_MERGE_RADIUS = 10.0f;
//Maximum zoom level possible in the editor.
const float  area_editor::ZOOM_MAX_LEVEL_EDITOR = 8.0f;
//Minimum zoom level possible in the editor.
const float  area_editor::ZOOM_MIN_LEVEL_EDITOR = 0.05f;

const string area_editor::EDITOR_ICONS_FOLDER_NAME = "Editor_icons";
const string area_editor::DELETE_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Delete.png";
const string area_editor::DELETE_LINK_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_link.png";
const string area_editor::DELETE_STOP_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_stop.png";
const string area_editor::DUPLICATE_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Duplicate.png";
const string area_editor::EXIT_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Exit.png";
const string area_editor::NEW_CIRCLE_SECTOR_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/New_circle_sector.png";
const string area_editor::NEW_1WLINK_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/New_1wlink.png";
const string area_editor::NEW_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/New.png";
const string area_editor::NEW_LINK_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/New_link.png";
const string area_editor::REFERENCE_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Reference.png";
const string area_editor::NEW_STOP_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/New_stop.png";
const string area_editor::NEXT_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Next.png";
const string area_editor::OPTIONS_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Options.png";
const string area_editor::PREVIOUS_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Previous.png";
const string area_editor::SAVE_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Save.png";
const string area_editor::SELECT_NONE_ICON =
    EDITOR_ICONS_FOLDER_NAME + "/Select_none.png";


/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    reference_aspect_ratio(true),
    reference_bitmap(NULL),
    reference_size(1000, 1000),
    reference_a(255),
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
    mode_before_options(EDITOR_MODE_MAIN),
    moving_cross_section_point(-1),
    moving_path_preview_checkpoint(-1),
    moving_thing(INVALID),
    new_link_first_stop(NULL),
    new_circle_sector_step(0),
    new_sector_valid_line(false),
    on_sector(NULL),
    path_preview_timeout(0),
    shift_pressed(false),
    show_closest_stop(false),
    show_cross_section(false),
    show_cross_section_grid(false),
    show_reference(false),
    show_path_preview(false),
    show_shadows(true),
    wum(NULL) {
    
    path_preview_checkpoints[0].x = -DEF_GRID_INTERVAL;
    path_preview_checkpoints[1].x = DEF_GRID_INTERVAL;
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
    
    cross_section_points[0].x = -DEF_GRID_INTERVAL;
    cross_section_points[1].x = DEF_GRID_INTERVAL;
}

/* ----------------------------------------------------------------------------
 * Stores the data from the advanced texture settings onto the gui.
 */
void area_editor::adv_textures_to_gui() {
    if(!cur_sector) {
        mode = EDITOR_MODE_SECTORS;
        change_to_right_frame();
        return;
    }
    
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_adv_textures"];
        
    ((lafi::textbox*) f->widgets["txt_x"])->text =
        f2s(cur_sector->texture_info.translation.x);
    ((lafi::textbox*) f->widgets["txt_y"])->text =
        f2s(cur_sector->texture_info.translation.y);
    ((lafi::textbox*) f->widgets["txt_sx"])->text =
        f2s(cur_sector->texture_info.scale.x);
    ((lafi::textbox*) f->widgets["txt_sy"])->text =
        f2s(cur_sector->texture_info.scale.y);
    ((lafi::angle_picker*) f->widgets["ang_a"])->set_angle_rads(
        cur_sector->texture_info.rot
    );
    ((lafi::textbox*) f->widgets["txt_tint"])->text =
        c2s(cur_sector->texture_info.tint);
        
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
            cur_mob->category->plural_name;
            
        lafi::button* but_type = (lafi::button*) f->widgets["but_type"];
        if(cur_mob->category->id == MOB_CATEGORY_NONE) {
            disable_widget(but_type);
        } else {
            enable_widget(but_type);
        }
        but_type->text = cur_mob->type ? cur_mob->type->name : "";
    }
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
 * Snaps a point to the nearest grid space.
 */
point area_editor::snap_to_grid(const point p) {
    if(shift_pressed) return p;
    return
        point(
            round(p.x / grid_interval) * grid_interval,
            round(p.y / grid_interval) * grid_interval
        );
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
            f2s(cur_shadow->center.x);
        ((lafi::textbox*) f->widgets["txt_y"])->text =
            f2s(cur_shadow->center.y);
        ((lafi::textbox*) f->widgets["txt_w"])->text =
            f2s(cur_shadow->size.x);
        ((lafi::textbox*) f->widgets["txt_h"])->text =
            f2s(cur_shadow->size.y);
        ((lafi::angle_picker*) f->widgets["ang_an"])->set_angle_rads(
            cur_shadow->angle
        );
        ((lafi::scrollbar*) f->widgets["bar_al"])->set_value(
            cur_shadow->alpha, false
        );
        ((lafi::textbox*) f->widgets["txt_file"])->text =
            cur_shadow->file_name;
        ((lafi::textbox*) f->widgets["txt_sx"])->text =
            f2s(cur_shadow->sway.x);
        ((lafi::textbox*) f->widgets["txt_sy"])->text =
            f2s(cur_shadow->sway.y);
            
    } else {
        hide_widget(f);
    }
}


/* ----------------------------------------------------------------------------
 * Saves the advanced texture settings to memory using info on the gui.
 */
void area_editor::gui_to_adv_textures() {
    if(!cur_sector) return;
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_adv_textures"];
    
    cur_sector->texture_info.translation.x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sector->texture_info.translation.y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sector->texture_info.scale.x =
        s2f(((lafi::textbox*) f->widgets["txt_sx"])->text);
    cur_sector->texture_info.scale.y =
        s2f(((lafi::textbox*) f->widgets["txt_sy"])->text);
    cur_sector->texture_info.rot =
        ((lafi::angle_picker*) f->widgets["ang_a"])->get_angle_rads();
    cur_sector->texture_info.tint =
        s2c(((lafi::textbox*) f->widgets["txt_tint"])->text);
        
    adv_textures_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the reference's data to memory using info on the gui.
 */
void area_editor::gui_to_reference() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_reference"];
    
    string new_file_name = ((lafi::textbox*) f->widgets["txt_file"])->text;
    bool is_file_new = false;
    
    if(new_file_name != reference_file_name) {
        //New reference image, delete the old one.
        change_reference(new_file_name);
        is_file_new = true;
        if(reference_bitmap) {
            reference_size.x = al_get_bitmap_width(reference_bitmap);
            reference_size.y = al_get_bitmap_height(reference_bitmap);
        } else {
            reference_pos.x = 0;
            reference_pos.y = 0;
        }
    }
    
    reference_pos.x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    reference_pos.y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    
    reference_aspect_ratio =
        ((lafi::checkbox*) f->widgets["chk_ratio"])->checked;
    point new_size(
        s2f(((lafi::textbox*) f->widgets["txt_w"])->text),
        s2f(((lafi::textbox*) f->widgets["txt_h"])->text)
    );
    
    if(new_size.x != 0 && new_size.y != 0 && !is_file_new) {
        if(reference_aspect_ratio) {
            if(new_size.x == reference_size.x && new_size.y != reference_size.y) {
                float ratio = reference_size.x / reference_size.y;
                reference_size.y = new_size.y;
                reference_size.x = new_size.y * ratio;
            } else if(new_size.x != reference_size.x && new_size.y == reference_size.y) {
                float ratio = reference_size.y / reference_size.x;
                reference_size.x = new_size.x;
                reference_size.y = new_size.x * ratio;
            } else {
                reference_size = new_size;
            }
        } else {
            reference_size = new_size;
        }
    }
    
    sec_mode =
        ((lafi::checkbox*) f->widgets["chk_mouse"])->checked ?
        ESM_REFERENCE_MOUSE : ESM_NONE;
    reference_a = ((lafi::scrollbar*) f->widgets["bar_alpha"])->low_value;
    
    reference_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves a mob's data to memory using info on the gui.
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
 * Saves the sector data to memory using info on the gui.
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
 * Saves the current tree shadow to memory using info on the gui.
 */
void area_editor::gui_to_shadow() {
    if(!cur_shadow) return;
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_shadows"]->widgets["frm_shadow"];
        
    cur_shadow->center.x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_shadow->center.y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_shadow->size.x = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_shadow->size.y = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_shadow->angle =
        ((lafi::angle_picker*) f->widgets["ang_an"])->get_angle_rads();
    cur_shadow->alpha = ((lafi::scrollbar*) f->widgets["bar_al"])->low_value;
    cur_shadow->sway.x = s2f(((lafi::textbox*) f->widgets["txt_sx"])->text);
    cur_shadow->sway.y = s2f(((lafi::textbox*) f->widgets["txt_sy"])->text);
    
    string new_file_name = ((lafi::textbox*) f->widgets["txt_file"])->text;
    
    if(new_file_name != cur_shadow->file_name) {
        //New image, delete the old one.
        if(cur_shadow->bitmap != bmp_error) {
            bitmaps.detach(cur_shadow->file_name);
        }
        cur_shadow->bitmap =
            bitmaps.get(TEXTURES_FOLDER_NAME + "/" + new_file_name, NULL);
        cur_shadow->file_name = new_file_name;
    }
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Calculates the preview path.
 */
void area_editor::calculate_preview_path() {
    if(!show_path_preview) return;
    
    float d = 0;
    path_preview =
        get_path(
            path_preview_checkpoints[0],
            path_preview_checkpoints[1],
            NULL, NULL, &d
        );
        
    if(path_preview.empty() && d == 0) {
        d =
            dist(
                path_preview_checkpoints[0],
                path_preview_checkpoints[1]
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
    
    new_circle_sector_step = 0;
    new_circle_sector_points.clear();
    new_circle_sector_valid_edges.clear();
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void area_editor::center_camera(
    const point min_coords, const point max_coords
) {
    float width = max_coords.x - min_coords.x;
    float height = max_coords.y - min_coords.y;
    
    cam_pos.x = floor(min_coords.x + width  / 2);
    cam_pos.y = floor(min_coords.y + height / 2);
    
    if(width > height) cam_zoom = gui_x / width;
    else cam_zoom = status_bar_y / height;
    
    cam_zoom -= cam_zoom * 0.1;
    
    cam_zoom = max(cam_zoom, ZOOM_MIN_LEVEL_EDITOR);
    cam_zoom = min(cam_zoom, ZOOM_MAX_LEVEL_EDITOR);
    
}


/* ----------------------------------------------------------------------------
 * Changes the reference image.
 */
void area_editor::change_reference(string new_file_name) {
    if(reference_bitmap && reference_bitmap != bmp_error) {
        al_destroy_bitmap(reference_bitmap);
    }
    reference_bitmap = NULL;
    
    if(new_file_name.size()) {
        reference_bitmap = load_bmp(new_file_name, NULL, false);
    }
    reference_file_name = new_file_name;
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Hides all menu frames.
 */
void area_editor::hide_all_frames() {
    hide_widget(gui->widgets["frm_main"]);
    hide_widget(gui->widgets["frm_picker"]);
    hide_widget(gui->widgets["frm_sectors"]);
    hide_widget(gui->widgets["frm_paths"]);
    hide_widget(gui->widgets["frm_adv_textures"]);
    hide_widget(gui->widgets["frm_texture"]);
    hide_widget(gui->widgets["frm_objects"]);
    hide_widget(gui->widgets["frm_shadows"]);
    hide_widget(gui->widgets["frm_reference"]);
    hide_widget(gui->widgets["frm_review"]);
    hide_widget(gui->widgets["frm_tools"]);
    hide_widget(gui->widgets["frm_options"]);
    
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame() {
    sec_mode = ESM_NONE;
    
    hide_all_frames();
    
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
    } else if(mode == EDITOR_MODE_FOLDER_PATHS) {
        show_widget(gui->widgets["frm_paths"]);
    } else if(mode == EDITOR_MODE_SHADOWS) {
        show_widget(gui->widgets["frm_shadows"]);
    } else if(mode == EDITOR_MODE_REFERENCE) {
        show_widget(gui->widgets["frm_reference"]);
    } else if(mode == EDITOR_MODE_REVIEW) {
        show_widget(gui->widgets["frm_review"]);
    } else if(mode == EDITOR_MODE_TOOLS) {
        show_widget(gui->widgets["frm_tools"]);
    } else if(mode == EDITOR_MODE_OPTIONS) {
        show_widget(gui->widgets["frm_options"]);
    }
}


/* ----------------------------------------------------------------------------
 * Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    intersecting_edges.clear();
    non_simples.clear();
    lone_edges.clear();
    
    error_type = EET_NONE_YET;
    error_sector_ptr = NULL;
    error_string.clear();
    error_vertex_ptr = NULL;
    
    ((lafi::button*) gui->widgets["frm_main"]->widgets["but_area"])->text =
        area_name;
    show_widget(gui->widgets["frm_main"]->widgets["frm_area"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_save"]);
    hide_widget(gui->widgets["frm_paths"]->widgets["lbl_path_dist"]);
    (
        (lafi::checkbox*) gui->widgets["frm_paths"]->widgets["chk_show_path"]
    )->uncheck();
    
    
    change_reference("");
    reference_aspect_ratio = true;
    reference_size = point(1000, 1000);
    reference_a = 255;
    cur_sector = NULL;
    cur_mob = NULL;
    cur_shadow = NULL;
    clear_area_textures();
    sector_to_gui();
    mob_to_gui();
    reference_to_gui();
    
    cam_pos.x = cam_pos.y = 0;
    cam_zoom = 1;
    show_cross_section = false;
    show_cross_section_grid = false;
    show_path_preview = false;
    path_preview.clear();
    path_preview_checkpoints[0] = point(-DEF_GRID_INTERVAL, 0);
    path_preview_checkpoints[1] = point(DEF_GRID_INTERVAL, 0);
    cross_section_points[0] = point(-DEF_GRID_INTERVAL, 0);
    cross_section_points[1] = point(DEF_GRID_INTERVAL, 0);
    
    texture_suggestions.clear();
    
    cur_area_data.clear();
    
    made_changes = false;
    backup_timer.start(editor_backup_interval);
    
    mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given its name.
 */
void area_editor::create_new_from_picker(const string &name) {
    string new_area_path =
        AREAS_FOLDER_PATH + "/" + name;
    ALLEGRO_FS_ENTRY* new_area_folder_entry =
        al_create_fs_entry(new_area_path.c_str());
        
    if(al_fs_entry_exists(new_area_folder_entry)) {
        //Already exists, just load it.
        area_name = name;
        area_editor::load_area(false);
    } else {
        //Create a new area.
        area_name = name;
        clear_current_area();
        disable_widget(gui->widgets["frm_options"]->widgets["but_load"]);
        
        //Create a sector for it.
        new_sector_valid_line = true;
        new_sector_vertexes.push_back(new vertex(-500, -500));
        new_sector_vertexes.push_back(new vertex(500,  -500));
        new_sector_vertexes.push_back(new vertex(500,  500));
        new_sector_vertexes.push_back(new vertex(-500, 500));
        create_sector();
        cur_sector = NULL;
        sector_to_gui();
        
        //Find a texture to give to this sector.
        vector<string> textures = folder_to_vector(TEXTURES_FOLDER_PATH, false);
        size_t texture_to_use = INVALID;
        //First, if there's any "grass" texture, use that.
        for(size_t t = 0; t < textures.size(); ++t) {
            string lc_name = str_to_lower(textures[t]);
            if(lc_name.find("grass") != string::npos) {
                texture_to_use = t;
                break;
            }
        }
        //No grass texture? Try one with "dirt".
        if(texture_to_use == INVALID) {
            for(size_t t = 0; t < textures.size(); ++t) {
                string lc_name = str_to_lower(textures[t]);
                if(lc_name.find("dirt") != string::npos) {
                    texture_to_use = t;
                    break;
                }
            }
        }
        //If there's no good texture, just pick the first one.
        if(texture_to_use == INVALID) {
            if(!textures.empty()) texture_to_use = 0;
        }
        //Apply the texture.
        if(texture_to_use != INVALID) {
            cur_area_data.sectors[0]->texture_info.file_name =
                textures[texture_to_use];
        }
        
        //Now add a leader. The first available.
        cur_area_data.mob_generators.push_back(
            new mob_gen(
                mob_categories.get(MOB_CATEGORY_LEADERS), point(),
                leader_types.begin()->second, 0, ""
            )
        );
    }
    
    al_destroy_fs_entry(new_area_folder_entry);
    
    show_bottom_frame();
    change_to_right_frame();
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
                point(
                    new_sector_vertexes[v]->x,
                    new_sector_vertexes[v]->y
                ),
                cur_area_data.vertexes, VERTEX_MERGE_RADIUS / cam_zoom,
                &merge_nr
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
            is_point_in_sector(point(v_ptr->x, v_ptr->y), new_sector)
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
    sector_to_gui();
    new_sector_vertexes.clear();
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {

    update_transformations();
    
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
                cur_area_data.mob_generators[m]->category->id ==
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
            if(!get_sector(m_ptr->pos, NULL, false)) {
                error_type = EET_MOB_OOB;
                error_mob_ptr = m_ptr;
                break;
            }
        }
    }
    
    //Objects inside walls.
    if(error_type == EET_NONE) {
        error_mob_ptr = NULL;
        
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            mob_gen* m_ptr = cur_area_data.mob_generators[m];
            
            if(
                m_ptr->category->id == MOB_CATEGORY_GATES ||
                m_ptr->category->id == MOB_CATEGORY_BRIDGES
            ) {
                continue;
            }
            
            for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
                edge* e_ptr = cur_area_data.edges[e];
                if(!is_edge_valid(e_ptr)) continue;
                
                if(
                    circle_intersects_line(
                        m_ptr->pos,
                        m_ptr->type->radius,
                        point(
                            e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                        ),
                        point(
                            e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                        ),
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
            
            if(error_mob_ptr) break;
            
        }
    }
    
    //Path stops out of bounds.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = cur_area_data.path_stops[s];
            if(!get_sector(s_ptr->pos, NULL, false)) {
                error_type = EET_PATH_STOP_OOB;
                error_path_stop_ptr = s_ptr;
                break;
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
            folder_to_vector(TEXTURES_FOLDER_PATH, false);
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
    
    //Two stops intersecting.
    if(error_type == EET_NONE) {
        for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
            path_stop* s_ptr = cur_area_data.path_stops[s];
            for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
                path_stop* s2_ptr = cur_area_data.path_stops[s2];
                if(s2_ptr == s_ptr) continue;
                
                if(dist(s_ptr->pos, s2_ptr->pos) <= 3.0) {
                    error_type = EET_PATH_STOPS_TOGETHER;
                    error_path_stop_ptr = s_ptr;
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
 * Sets the vector of points that make up a new circle sector.
 */
void area_editor::set_new_circle_sector_points() {
    float anchor_angle =
        get_angle(new_circle_sector_center, new_circle_sector_anchor);
    float cursor_angle =
        get_angle(new_circle_sector_center, mouse_cursor_w);
    float radius =
        dist(
            new_circle_sector_center, new_circle_sector_anchor
        ).to_float();
    float angle_dif =
        get_angle_smallest_dif(cursor_angle, anchor_angle);
        
    size_t n_points = MAX_CIRCLE_SECTOR_POINTS;
    if(angle_dif > 0) {
        n_points = round((M_PI * 2) / angle_dif);
    }
    n_points =
        min(n_points, MAX_CIRCLE_SECTOR_POINTS);
    n_points =
        max(MIN_CIRCLE_SECTOR_POINTS, n_points);
        
    new_circle_sector_points.clear();
    for(size_t p = 0; p < n_points; ++p) {
        float delta_a = ((M_PI * 2) / n_points) * p;
        new_circle_sector_points.push_back(
            point(
                new_circle_sector_center.x +
                radius * cos(anchor_angle + delta_a),
                new_circle_sector_center.y +
                radius * sin(anchor_angle + delta_a)
            )
        );
    }
    
    new_circle_sector_valid_edges.clear();
    for(size_t p = 0; p < n_points; ++p) {
        point next = get_next_in_vector(new_circle_sector_points, p);
        bool valid = true;
        
        for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
            edge* e_ptr = cur_area_data.edges[e];
            
            if(
                lines_intersect(
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    new_circle_sector_points[p], next,
                    NULL, NULL
                )
            ) {
                valid = false;
                break;
            }
        }
        
        new_circle_sector_valid_edges.push_back(valid);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a sector common to all vertexes.
 * A sector is considered this if a vertex has it as a sector of
 * a neighboring edge, or if a vertex is inside it.
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
                get_sector(point(v_ptr->x, v_ptr->y), NULL, false)
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
 * Focuses the camera on the error found, if any.
 */
void area_editor::goto_error() {
    if(error_type == EET_NONE || error_type == EET_NONE_YET) return;
    
    if(error_type == EET_INTERSECTING_EDGES) {
    
        if(intersecting_edges.empty()) {
            find_errors(); return;
        }
        
        edge_intersection* li_ptr = &intersecting_edges[0];
        point min_coords, max_coords;
        min_coords.x = max_coords.x = li_ptr->e1->vertexes[0]->x;
        min_coords.y = max_coords.y = li_ptr->e1->vertexes[0]->y;
        
        min_coords.x = min(min_coords.x, li_ptr->e1->vertexes[0]->x);
        min_coords.x = min(min_coords.x, li_ptr->e1->vertexes[1]->x);
        min_coords.x = min(min_coords.x, li_ptr->e2->vertexes[0]->x);
        min_coords.x = min(min_coords.x, li_ptr->e2->vertexes[1]->x);
        max_coords.x = max(max_coords.x, li_ptr->e1->vertexes[0]->x);
        max_coords.x = max(max_coords.x, li_ptr->e1->vertexes[1]->x);
        max_coords.x = max(max_coords.x, li_ptr->e2->vertexes[0]->x);
        max_coords.x = max(max_coords.x, li_ptr->e2->vertexes[1]->x);
        min_coords.y = min(min_coords.y, li_ptr->e1->vertexes[0]->y);
        min_coords.y = min(min_coords.y, li_ptr->e1->vertexes[1]->y);
        min_coords.y = min(min_coords.y, li_ptr->e2->vertexes[0]->y);
        min_coords.y = min(min_coords.y, li_ptr->e2->vertexes[1]->y);
        max_coords.y = max(max_coords.y, li_ptr->e1->vertexes[0]->y);
        max_coords.y = max(max_coords.y, li_ptr->e1->vertexes[1]->y);
        max_coords.y = max(max_coords.y, li_ptr->e2->vertexes[0]->y);
        max_coords.y = max(max_coords.y, li_ptr->e2->vertexes[1]->y);
        
        center_camera(min_coords, max_coords);
        
    } else if(error_type == EET_BAD_SECTOR) {
    
        if(non_simples.empty()) {
            find_errors(); return;
        }
        
        sector* s_ptr = *non_simples.begin();
        point min_coords, max_coords;
        get_sector_bounding_box(s_ptr, &min_coords, &max_coords);
        
        center_camera(min_coords, max_coords);
        
    } else if(error_type == EET_LONE_EDGE) {
    
        if(lone_edges.empty()) {
            find_errors(); return;
        }
        
        edge* e_ptr = *lone_edges.begin();
        point min_coords, max_coords;
        min_coords.x = e_ptr->vertexes[0]->x;
        max_coords.x = min_coords.x;
        min_coords.y = e_ptr->vertexes[0]->y;
        max_coords.y = min_coords.y;
        
        min_coords.x = min(min_coords.x, e_ptr->vertexes[0]->x);
        min_coords.x = min(min_coords.x, e_ptr->vertexes[1]->x);
        max_coords.x = max(max_coords.x, e_ptr->vertexes[0]->x);
        max_coords.x = max(max_coords.x, e_ptr->vertexes[1]->x);
        min_coords.y = min(min_coords.y, e_ptr->vertexes[0]->y);
        min_coords.y = min(min_coords.y, e_ptr->vertexes[1]->y);
        max_coords.y = max(max_coords.y, e_ptr->vertexes[0]->y);
        max_coords.y = max(max_coords.y, e_ptr->vertexes[1]->y);
        
        center_camera(min_coords, max_coords);
        
    } else if(error_type == EET_OVERLAPPING_VERTEXES) {
    
        if(!error_vertex_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            point(
                error_vertex_ptr->x - 64,
                error_vertex_ptr->y - 64
            ),
            point(
                error_vertex_ptr->x + 64,
                error_vertex_ptr->y + 64
            )
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
        
        point min_coords, max_coords;
        get_sector_bounding_box(error_sector_ptr, &min_coords, &max_coords);
        center_camera(min_coords, max_coords);
        
    } else if(
        error_type == EET_TYPELESS_MOB ||
        error_type == EET_MOB_OOB ||
        error_type == EET_MOB_IN_WALL
    ) {
    
        if(!error_mob_ptr) {
            find_errors(); return;
        }
        
        center_camera(error_mob_ptr->pos - 64, error_mob_ptr->pos + 64);
        
    } else if(
        error_type == EET_LONE_PATH_STOP ||
        error_type == EET_PATH_STOPS_TOGETHER ||
        error_type == EET_PATH_STOP_OOB
    ) {
    
        if(!error_path_stop_ptr) {
            find_errors(); return;
        }
        
        center_camera(
            error_path_stop_ptr->pos - 64,
            error_path_stop_ptr->pos + 64
        );
        
    } else if(error_type == EET_INVALID_SHADOW) {
    
        point min_coords, max_coords;
        get_shadow_bounding_box(
            error_shadow_ptr, &min_coords, &max_coords
        );
        center_camera(min_coords, max_coords);
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether the next line for a sector's creation is valid.
 * i.e. it does not cross against other lines.
 * This is the line between the last chosen vertex of the new sector
 * and the provided coordinates.
 */
bool area_editor::is_new_sector_line_valid(const point pos) {
    if(new_sector_vertexes.empty()) return true;
    
    //Given the last vertex of the new sector,
    //check if it'll be merged with an existing one.
    vertex* last_vertex = new_sector_vertexes.back();
    vertex* merge_vertex_1 =
        get_merge_vertex(
            point(last_vertex->x, last_vertex->y),
            cur_area_data.vertexes, VERTEX_MERGE_RADIUS / cam_zoom
        );
    vertex* merge_vertex_2 =
        get_merge_vertex(
            pos, cur_area_data.vertexes, VERTEX_MERGE_RADIUS / cam_zoom
        );
        
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
                point(last_vertex->x, last_vertex->y),
                pos,
                point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
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
                    point(
                        new_sector_vertexes.back()->x,
                        new_sector_vertexes.back()->y
                    ),
                    pos,
                    point(v1_ptr->x, v1_ptr->y),
                    point(v2_ptr->x, v2_ptr->y),
                    NULL, NULL
                )
            ) {
                if(
                    v == 0 &&
                    dist(pos, point(v1_ptr->x, v1_ptr->y)) <=
                    VERTEX_MERGE_RADIUS
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
 * Load the area from the disk.
 * from_backup: If false, load it normally. If true, load from a backup, if any.
 */
void area_editor::load_area(const bool from_backup) {
    clear_current_area();
    
    ::load_area(area_name, true, from_backup);
    
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
    
    change_reference(reference_file_name);
    enable_widget(gui->widgets["frm_options"]->widgets["but_load"]);
    made_changes = false;
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
 * affected_sectors: List of sectors that will be affected by this merge.
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
        size_t ve_nr = v2->edge_nrs[ve];
        if(ve_ptr->sectors[0] == ve_ptr->sectors[1]) {
            ve_ptr->remove_from_sectors();
            ve_ptr->remove_from_vertexes();
            cur_area_data.remove_edge(ve_nr);
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
 * Opens the frame where you pick from a list.
 * For the type of content, use area_editor::AREA_EDITOR_PICKER_*.
 */
void area_editor::open_picker(const unsigned char type) {
    vector<string> elements;
    if(type == AREA_EDITOR_PICKER_AREA) {
        elements = folder_to_vector(AREAS_FOLDER_PATH, true);
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
    
        for(unsigned char f = 0; f < N_MOB_CATEGORIES; ++f) {
            //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            elements.push_back(mob_categories.get(f)->plural_name);
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        if(cur_mob->category->id != MOB_CATEGORY_NONE) {
            cur_mob->category->get_type_names(elements);
        }
        
    }
    
    generate_and_open_picker(elements, type, type == AREA_EDITOR_PICKER_AREA);
}


/* ----------------------------------------------------------------------------
 * Picks an item and closes the list picker frame.
 */
void area_editor::pick(const string &name, const unsigned char type) {
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
            cur_mob->category = mob_categories.get_from_pname(name);
            cur_mob->type = NULL;
            mob_to_gui();
        }
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
        if(cur_mob) {
            cur_mob->type = cur_mob->category->get_type(name);
        }
        
        mob_to_gui();
        
    }
    
    show_bottom_frame();
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Adds texture suggestions to the gui frame.
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
            new lafi::image(texture_suggestions[s].bmp);
        lafi::label* l =
            new lafi::label(name);
            
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
 * Loads the reference's data from the memory to the gui.
 */
void area_editor::reference_to_gui() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_reference"];
    ((lafi::textbox*) f->widgets["txt_file"])->text = reference_file_name;
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(reference_pos.x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(reference_pos.y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(reference_size.x);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(reference_size.y);
    ((lafi::checkbox*) f->widgets["chk_ratio"])->set(reference_aspect_ratio);
    ((lafi::checkbox*) f->widgets["chk_mouse"])->set(
        sec_mode == ESM_REFERENCE_MOUSE
    );
    ((lafi::scrollbar*) f->widgets["bar_alpha"])->set_value(reference_a, false);
}


/* ----------------------------------------------------------------------------
 * Removes a sector, if it's isolated.
 * Returns true on success.
 */
bool area_editor::remove_isolated_sector(sector* s_ptr) {
    //If around the sector there are two different sectors, then
    //it's definitely connected.
    sector* alt_sector = NULL;
    bool got_an_alt_sector = false;
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edge* e_ptr = s_ptr->edges[e];
        
        for(size_t s = 0; s < 2; ++s) {
            if(e_ptr->sectors[s] == s_ptr) {
                //The main sector; never mind.
                continue;
            }
            
            if(!got_an_alt_sector) {
                alt_sector = e_ptr->sectors[s];
                got_an_alt_sector = true;
            } else if(e_ptr->sectors[s] != alt_sector) {
                //Different alternative sector found! No good.
                return false;
            }
        }
    }
    
    //Remove the sector now.
    vector<edge*> main_sector_edges = s_ptr->edges;
    unordered_set<vertex*> main_vertexes;
    for(size_t e = 0; e < main_sector_edges.size(); ++e) {
        edge* e_ptr = main_sector_edges[e];
        main_vertexes.insert(e_ptr->vertexes[0]);
        main_vertexes.insert(e_ptr->vertexes[1]);
        e_ptr->remove_from_sectors();
        e_ptr->remove_from_vertexes();
        cur_area_data.remove_edge(e_ptr);
    }
    
    for(auto v = main_vertexes.begin(); v != main_vertexes.end(); ++v) {
        cur_area_data.remove_vertex(*v);
    }
    
    cur_area_data.remove_sector(s_ptr);
    
    //Re-triangulate the outer sector.
    if(alt_sector) triangulate(alt_sector);
    
    return true;
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
        s_ptr->texture_info.scale *= mult;
        s_ptr->texture_info.translation *= mult;
        s_ptr->triangles.clear();
        triangulate(s_ptr);
    }
    
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        m_ptr->pos *= mult;
    }
    
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        s_ptr->pos *= mult;
    }
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        cur_area_data.path_stops[s]->calculate_dists();
    }
    
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
        s_ptr->center *= mult;
        s_ptr->size   *= mult;
        s_ptr->sway   *= mult;
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
            s_ptr->texture_info.scale.x != 1 ||
            s_ptr->texture_info.scale.y != 1
        ) {
            sector_node->add(
                new data_node(
                    "texture_scale",
                    f2s(s_ptr->texture_info.scale.x) + " " +
                    f2s(s_ptr->texture_info.scale.y)
                )
            );
        }
        if(
            s_ptr->texture_info.translation.x != 0 ||
            s_ptr->texture_info.translation.y != 0
        ) {
            sector_node->add(
                new data_node(
                    "texture_trans",
                    f2s(s_ptr->texture_info.translation.x) + " " +
                    f2s(s_ptr->texture_info.translation.y)
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
            new data_node(m_ptr->category->name, "");
        mobs_node->add(mob_node);
        
        if(m_ptr->type) {
            mob_node->add(
                new data_node("type", m_ptr->type->name)
            );
        }
        mob_node->add(
            new data_node(
                "p",
                f2s(m_ptr->pos.x) + " " + f2s(m_ptr->pos.y)
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
            new data_node("pos", f2s(s_ptr->pos.x) + " " + f2s(s_ptr->pos.y))
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
            new data_node(
                "pos", f2s(s_ptr->center.x) + " " + f2s(s_ptr->center.y)
            )
        );
        shadow_node->add(
            new data_node(
                "size", f2s(s_ptr->size.x) + " " + f2s(s_ptr->size.y)
            )
        );
        if(s_ptr->angle != 0) {
            shadow_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        if(s_ptr->alpha != 255) {
            shadow_node->add(new data_node("alpha", i2s(s_ptr->alpha)));
        }
        shadow_node->add(new data_node("file", s_ptr->file_name));
        shadow_node->add(
            new data_node("sway", f2s(s_ptr->sway.x) + " " + f2s(s_ptr->sway.y))
        );
        
    }
    
    //Editor reference.
    geometry_file.add(new data_node("reference_file_name", reference_file_name));
    geometry_file.add(new data_node("reference_pos",       p2s(reference_pos)));
    geometry_file.add(new data_node("reference_size",      p2s(reference_size)));
    geometry_file.add(new data_node("reference_alpha",     i2s(reference_a)));
    
    
    //Check if the folder exists before saving. If not, create it.
    ALLEGRO_FS_ENTRY* folder_fs_entry =
        al_create_fs_entry((AREAS_FOLDER_PATH + "/" + area_name).c_str());
    if(!al_open_directory(folder_fs_entry)) {
        al_make_directory((AREAS_FOLDER_PATH + "/" + area_name).c_str());
    }
    al_close_directory(folder_fs_entry);
    al_destroy_fs_entry(folder_fs_entry);
    
    //Also, check if the data file exists. Create it if not.
    if(
        !al_filename_exists(
            (AREAS_FOLDER_PATH + "/" + area_name + "/Data.txt").c_str()
        )
    ) {
        data_node data_file;
        data_file.save_file(
            (AREAS_FOLDER_PATH + "/" + area_name + "/Data.txt").c_str()
        );
    }
    
    
    //Finally, save.
    geometry_file.save_file(
        AREAS_FOLDER_PATH + "/" + area_name +
        (to_backup ? "/Geometry_backup.txt" : "/Geometry.txt")
    );
    
    backup_timer.start(editor_backup_interval);
    enable_widget(gui->widgets["frm_options"]->widgets["but_load"]);
}


/* ----------------------------------------------------------------------------
 * Saves the area onto a backup file.
 */
void area_editor::save_backup() {

    backup_timer.start(editor_backup_interval);
    
    //First, check if the folder even exists.
    //If not, chances are this is a new area.
    //We should probably create a backup anyway, but if the area is
    //just for testing, the backups are pointless.
    //Plus, creating the backup will create the area's folder on the disk,
    //which will basically mean the area exists, even though this might not be
    //what the user wants, since they haven't saved proper yet.
    
    ALLEGRO_FS_ENTRY* folder_fs_entry =
        al_create_fs_entry((AREAS_FOLDER_PATH + "/" + area_name).c_str());
    bool folder_exists = al_open_directory(folder_fs_entry);
    al_close_directory(folder_fs_entry);
    al_destroy_fs_entry(folder_fs_entry);
    
    if(!folder_exists) return;
    
    save_area(true);
    update_backup_status();
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
    
    data_node file(AREAS_FOLDER_PATH + "/" + area_name + "/Geometry_backup.txt");
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
            point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
            point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y),
            point(ei_ptr->e2->vertexes[0]->x, ei_ptr->e2->vertexes[0]->y),
            point(ei_ptr->e2->vertexes[1]->x, ei_ptr->e2->vertexes[1]->y),
            NULL, &u
        );
        
        float a =
            get_angle(
                point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
                point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y)
            );
        dist d(
            point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
            point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y)
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
    (
        (lafi::checkbox*)
        gui->widgets["frm_review"]->widgets["chk_cross_section"]
    )->set(show_cross_section);
    (
        (lafi::checkbox*)
        gui->widgets["frm_review"]->widgets["chk_cross_section_grid"]
    )->set(show_cross_section_grid);
    
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
 * Sets the reference's file name.
 */
void area_editor::set_reference_file_name(const string &n) {
    reference_file_name = n;
}


/* ----------------------------------------------------------------------------
 * Sets the reference's coordinates.
 */
void area_editor::set_reference_pos(const point &p) {
    reference_pos = p;
}


/* ----------------------------------------------------------------------------
 * Sets the reference's dimensions.
 */
void area_editor::set_reference_size(const point &p) {
    reference_size = p;
}


/* ----------------------------------------------------------------------------
 * Sets the reference's alpha.
 */
void area_editor::set_reference_a(const unsigned char a) {
    reference_a = a;
}


/* ----------------------------------------------------------------------------
 * Creates a texture suggestion.
 */
texture_suggestion::texture_suggestion(const string &n) :
    bmp(NULL),
    name(n) {
    
    bmp = bitmaps.get(TEXTURES_FOLDER_NAME + "/" + name, NULL);
}


/* ----------------------------------------------------------------------------
 * Destroys a texture suggestion.
 */
texture_suggestion::~texture_suggestion() {
    bitmaps.detach(name);
}
