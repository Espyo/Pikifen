/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Functions about the area editor's GUI.
 */

#include "area_editor.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/image.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../functions.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Loads the current advanced sector appearance data onto the GUI.
 */
void area_editor::asa_to_gui() {
    if(selected_sectors.empty()) {
        if(state == EDITOR_STATE_ASA) {
            sector_to_gui();
            state = EDITOR_STATE_LAYOUT;
            change_to_right_frame();
        }
        return;
    }
    
    sector* s_ptr = *selected_sectors.begin();
    
    ((lafi::textbox*) frm_asa->widgets["txt_x"])->text =
        f2s(s_ptr->texture_info.translation.x);
    ((lafi::textbox*) frm_asa->widgets["txt_y"])->text =
        f2s(s_ptr->texture_info.translation.y);
    ((lafi::textbox*) frm_asa->widgets["txt_sx"])->text =
        f2s(s_ptr->texture_info.scale.x);
    ((lafi::textbox*) frm_asa->widgets["txt_sy"])->text =
        f2s(s_ptr->texture_info.scale.y);
    ((lafi::angle_picker*) frm_asa->widgets["ang_a"])->set_angle_rads(
        s_ptr->texture_info.rot
    );
    ((lafi::textbox*) frm_asa->widgets["txt_tint"])->text =
        c2s(s_ptr->texture_info.tint);
        
    ((lafi::textbox*) frm_asa->widgets["txt_brightness"])->text =
        i2s(s_ptr->brightness);
    ((lafi::scrollbar*) frm_asa->widgets["bar_brightness"])->set_value(
        s_ptr->brightness, false
    );
    ((lafi::checkbox*) frm_asa->widgets["chk_shadow"])->set(
        s_ptr->always_cast_shadow
    );
}


/* ----------------------------------------------------------------------------
 * Loads the current advanced sector behavior data onto the GUI.
 */
void area_editor::asb_to_gui() {
    if(selected_sectors.empty()) {
        if(state == EDITOR_STATE_ASB) {
            sector_to_gui();
            state = EDITOR_STATE_LAYOUT;
            change_to_right_frame();
        }
        return;
    }
    
    sector* s_ptr = *selected_sectors.begin();
    
    ((lafi::button*) frm_asb->widgets["but_sector_type"])->text =
        sector_types.get_name(s_ptr->type);
        
    if(s_ptr->hazards_str.empty()) {
        ((lafi::label*) frm_asb->widgets["lbl_hazard"])->text = "(No hazards)";
        disable_widget(frm_asb->widgets["but_h_del"]);
        disable_widget(frm_asb->widgets["but_h_prev"]);
        disable_widget(frm_asb->widgets["but_h_next"]);
        
    } else {
        vector<string> list = split(s_ptr->hazards_str, ";");
        if(cur_hazard_nr >= list.size()) {
            cur_hazard_nr = list.size() - 1;
        }
        ((lafi::label*) frm_asb->widgets["lbl_hazard"])->text =
            i2s(cur_hazard_nr + 1) + "/" + i2s(list.size()) + ": " +
            list[cur_hazard_nr];
            
        enable_widget(frm_asb->widgets["but_h_del"]);
        enable_widget(frm_asb->widgets["but_h_prev"]);
        enable_widget(frm_asb->widgets["but_h_next"]);
    }
    
    ((lafi::checkbox*) frm_asb->widgets["chk_h_air"])->set(
        !s_ptr->hazard_floor
    );
    
    ((lafi::textbox*) frm_asb->widgets["txt_tag"])->text = s_ptr->tag;
    
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame() {
    sub_state = EDITOR_SUB_STATE_NONE;
    
    hide_all_frames();
    
    if(state == EDITOR_STATE_MAIN) {
        frm_main->show();
        update_main_frame();
    } else if(state == EDITOR_STATE_LAYOUT) {
        frm_layout->show();
        sector_to_gui();
    } else if(state == EDITOR_STATE_ASB) {
        frm_asb->show();
        asb_to_gui();
    } else if(state == EDITOR_STATE_TEXTURE) {
        frm_texture->show();
    } else if(state == EDITOR_STATE_ASA) {
        frm_asa->show();
        asa_to_gui();
    } else if(state == EDITOR_STATE_MOBS) {
        frm_mobs->show();
        mob_to_gui();
    } else if(state == EDITOR_STATE_PATHS) {
        frm_paths->show();
        path_to_gui();
    } else if(state == EDITOR_STATE_DETAILS) {
        frm_details->show();
        details_to_gui();
    } else if(state == EDITOR_STATE_REVIEW) {
        frm_review->show();
        review_to_gui();
    } else if(state == EDITOR_STATE_INFO) {
        frm_info->show();
        info_to_gui();
    } else if(state == EDITOR_STATE_TOOLS) {
        frm_tools->show();
        tools_to_gui();
    } else if(state == EDITOR_STATE_OPTIONS) {
        frm_options->show();
        options_to_gui();
    }
}


/* ----------------------------------------------------------------------------
 * GUI functions for clearing the data for the current area.
 */
void area_editor::clear_current_area_gui() {
    ((lafi::button*) frm_main->widgets["but_area"])->text = cur_area_name;
    frm_area->show();
    enable_widget(frm_bottom->widgets["but_save"]);
    frm_paths->widgets["lbl_path_dist"]->hide();
    ((lafi::checkbox*) frm_paths->widgets["chk_show_path"])->uncheck();
}


/* ----------------------------------------------------------------------------
 * Deletes the currently selected hazard from the list.
 */
void area_editor::delete_current_hazard() {
    register_change("hazard removal");
    
    sector* s_ptr = *selected_sectors.begin();
    
    vector<string> list = split(s_ptr->hazards_str, ";");
    s_ptr->hazards_str.clear();
    for(size_t h = 0; h < list.size(); ++h) {
        if(h == cur_hazard_nr) continue;
        s_ptr->hazards_str += list[h] + ";";
    }
    if(!s_ptr->hazards_str.empty()) {
        s_ptr->hazards_str.pop_back(); //Remove the trailing semicolon.
    }
    cur_hazard_nr = min(cur_hazard_nr, list.size() - 1);
    
    homogenize_selected_sectors();
    asb_to_gui();
}


/* ----------------------------------------------------------------------------
 * Loads the current details data onto the GUI.
 */
void area_editor::details_to_gui() {
    if(selected_shadow) {
    
        frm_shadow->show();
        ((lafi::textbox*) frm_shadow->widgets["txt_x"])->text =
            f2s(selected_shadow->center.x);
        ((lafi::textbox*) frm_shadow->widgets["txt_y"])->text =
            f2s(selected_shadow->center.y);
        ((lafi::textbox*) frm_shadow->widgets["txt_w"])->text =
            f2s(selected_shadow->size.x);
        ((lafi::textbox*) frm_shadow->widgets["txt_h"])->text =
            f2s(selected_shadow->size.y);
        ((lafi::checkbox*) frm_shadow->widgets["chk_ratio"])->set(
            selected_shadow_transformation.keep_aspect_ratio
        );
        ((lafi::angle_picker*) frm_shadow->widgets["ang_an"])->set_angle_rads(
            selected_shadow->angle
        );
        ((lafi::scrollbar*) frm_shadow->widgets["bar_al"])->set_value(
            selected_shadow->alpha, false
        );
        ((lafi::textbox*) frm_shadow->widgets["txt_file"])->text =
            selected_shadow->file_name;
        ((lafi::textbox*) frm_shadow->widgets["txt_sx"])->text =
            f2s(selected_shadow->sway.x);
        ((lafi::textbox*) frm_shadow->widgets["txt_sy"])->text =
            f2s(selected_shadow->sway.y);
            
    } else {
        frm_shadow->hide();
    }
}


/* ----------------------------------------------------------------------------
 * Saves the advanced sector appearance data to memory using info on the gui.
 */
void area_editor::gui_to_asa() {
    sector* s_ptr = *selected_sectors.begin();
    gui_to_var_helper h;
    
    h.register_point(
        &s_ptr->texture_info.translation,
        point(
            s2f(((lafi::textbox*) frm_asa->widgets["txt_x"])->text),
            s2f(((lafi::textbox*) frm_asa->widgets["txt_y"])->text)
        )
    );
    h.register_point(
        &s_ptr->texture_info.scale,
        point(
            s2f(((lafi::textbox*) frm_asa->widgets["txt_sx"])->text),
            s2f(((lafi::textbox*) frm_asa->widgets["txt_sy"])->text)
        )
    );
    h.register_float(
        &s_ptr->texture_info.rot,
        ((lafi::angle_picker*) frm_asa->widgets["ang_a"])->get_angle_rads()
    );
    h.register_color(
        &s_ptr->texture_info.tint,
        s2c(((lafi::textbox*) frm_asa->widgets["txt_tint"])->text)
    );
    h.register_uchar(
        &s_ptr->brightness,
        s2i(((lafi::textbox*) frm_asa->widgets["txt_brightness"])->text)
    );
    h.register_bool(
        &s_ptr->always_cast_shadow,
        ((lafi::checkbox*) frm_asa->widgets["chk_shadow"])->checked
    );
    
    if(!h.all_equal()) {
        register_change("advanced sector appearance change");
    }
    
    h.set_all();
    
    homogenize_selected_sectors();
    asa_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the advanced sector behavior data to memory using info on the gui.
 */
void area_editor::gui_to_asb() {
    sector* s_ptr = *selected_sectors.begin();
    gui_to_var_helper h;
    
    h.register_bool(
        &s_ptr->hazard_floor,
        !((lafi::checkbox*) frm_asb->widgets["chk_h_air"])->checked
    );
    h.register_string(
        &s_ptr->tag,
        ((lafi::textbox*) frm_asb->widgets["txt_tag"])->text
    );
    
    if(!h.all_equal()) {
        register_change("advanced sector behavior change");
    }
    
    h.set_all();
    
    homogenize_selected_sectors();
    asb_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the details data to memory using info on the gui.
 */
void area_editor::gui_to_details() {
    if(!selected_shadow) return;
    
    gui_to_var_helper h;
    
    h.register_point(
        &selected_shadow->center,
        point(
            s2f(((lafi::textbox*) frm_shadow->widgets["txt_x"])->text),
            s2f(((lafi::textbox*) frm_shadow->widgets["txt_y"])->text)
        )
    );
    h.register_bool(
        &selected_shadow_transformation.keep_aspect_ratio,
        ((lafi::checkbox*) frm_shadow->widgets["chk_ratio"])->checked
    );
    point new_size(
        s2f(((lafi::textbox*) frm_shadow->widgets["txt_w"])->text),
        s2f(((lafi::textbox*) frm_shadow->widgets["txt_h"])->text)
    );
    if(((lafi::checkbox*) frm_shadow->widgets["chk_ratio"])->checked) {
        if(
            new_size.x == selected_shadow->size.x &&
            new_size.y != selected_shadow->size.y
        ) {
            if(selected_shadow->size.y != 0.0f) {
                float ratio =
                    selected_shadow->size.x / selected_shadow->size.y;
                new_size.x = new_size.y * ratio;
            }
            
        } else if(
            new_size.x != selected_shadow->size.x &&
            new_size.y == selected_shadow->size.y
        ) {
            if(selected_shadow->size.x != 0.0f) {
                float ratio =
                    selected_shadow->size.y / selected_shadow->size.x;
                new_size.y = new_size.x * ratio;
            }
            
        }
    }
    h.register_point(
        &selected_shadow->size,
        new_size
    );
    h.register_float(
        &selected_shadow->angle,
        ((lafi::angle_picker*) frm_shadow->widgets["ang_an"])->get_angle_rads()
    );
    h.register_uchar(
        &selected_shadow->alpha,
        ((lafi::scrollbar*) frm_shadow->widgets["bar_al"])->low_value
    );
    h.register_point(
        &selected_shadow->sway,
        point(
            s2f(((lafi::textbox*) frm_shadow->widgets["txt_sx"])->text),
            s2f(((lafi::textbox*) frm_shadow->widgets["txt_sy"])->text)
        )
    );
    
    string new_file_name =
        ((lafi::textbox*) frm_shadow->widgets["txt_file"])->text;
        
    if(!h.all_equal() || new_file_name != selected_shadow->file_name) {
        register_change("area details change");
    }
    
    h.set_all();
    
    selected_shadow_transformation.set_size(new_size);
    
    if(new_file_name != selected_shadow->file_name) {
        //New image, delete the old one.
        if(selected_shadow->bitmap != bmp_error) {
            textures.detach(selected_shadow->file_name);
        }
        selected_shadow->bitmap =
            textures.get(new_file_name, NULL);
        selected_shadow->file_name = new_file_name;
    }
    
    select_tree_shadow(selected_shadow); //Update transformation controller.
    details_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the area info data to memory using info on the gui.
 */
void area_editor::gui_to_info() {
    gui_to_var_helper h;
    
    h.register_string(
        &cur_area_data.name,
        ((lafi::textbox*) frm_info->widgets["txt_name"])->text
    );
    h.register_string(
        &cur_area_data.subtitle,
        ((lafi::textbox*) frm_info->widgets["txt_subtitle"])->text
    );
    h.register_string(
        &cur_area_data.weather_name,
        ((lafi::button*) frm_info->widgets["but_weather"])->text
    );
    h.register_string(
        &cur_area_data.bg_bmp_file_name,
        ((lafi::textbox*) frm_info->widgets["txt_bg_bitmap"])->text
    );
    h.register_color(
        &cur_area_data.bg_color,
        s2c(((lafi::textbox*) frm_info->widgets["txt_bg_color"])->text)
    );
    h.register_float(
        &cur_area_data.bg_dist,
        s2f(((lafi::textbox*) frm_info->widgets["txt_bg_dist"])->text)
    );
    h.register_float(
        &cur_area_data.bg_bmp_zoom,
        s2f(((lafi::textbox*) frm_info->widgets["txt_bg_zoom"])->text)
    );
    
    if(!h.all_equal()) {
        register_change("area info change");
    }
    
    h.set_all();
    
    info_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the mob data to memory using info on the gui.
 */
void area_editor::gui_to_mob() {
    mob_gen* m_ptr = *selected_mobs.begin();
    gui_to_var_helper h;
    
    h.register_float(
        &m_ptr->angle,
        (
            (lafi::angle_picker*) frm_mob->widgets["ang_angle"]
        )->get_angle_rads()
    );
    h.register_string(
        &m_ptr->vars,
        ((lafi::textbox*) frm_mob->widgets["txt_vars"])->text
    );
    
    if(!h.all_equal()) {
        register_change("object data change");
    }
    
    h.set_all();
    
    homogenize_selected_mobs();
}


/* ----------------------------------------------------------------------------
 * Saves the options data to memory using info on the gui.
 */
void area_editor::gui_to_options() {
    area_editor_show_edge_length =
        ((lafi::checkbox*) frm_options->widgets["chk_edge_length"])->checked;
        
    if(
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_textures"]
        )->selected
    ) {
        area_editor_view_mode = VIEW_MODE_TEXTURES;
        
    } else if(
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_wireframe"]
        )->selected
    ) {
        area_editor_view_mode = VIEW_MODE_WIREFRAME;
        
    } else if(
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_heightmap"]
        )->selected
    ) {
        area_editor_view_mode = VIEW_MODE_HEIGHTMAP;
        
    } else if(
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_brightness"]
        )->selected
    ) {
        area_editor_view_mode = VIEW_MODE_BRIGHTNESS;
        
    }
    
    area_editor_backup_interval =
        s2i(((lafi::textbox*) frm_options->widgets["txt_backup"])->text);
    area_editor_undo_limit =
        s2i(((lafi::textbox*) frm_options->widgets["txt_undo_limit"])->text);
        
    update_undo_history();
    
    save_options();
    options_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the sector data to memory using info on the gui.
 */
void area_editor::gui_to_sector() {
    sector* s_ptr = *selected_sectors.begin();
    gui_to_var_helper h;
    
    h.register_float(
        &s_ptr->z,
        s2f(((lafi::textbox*) frm_sector->widgets["txt_z"])->text)
    );
    h.register_bool(
        &s_ptr->fade,
        ((lafi::radio_button*) frm_sector->widgets["rad_fade"])->selected
    );
    string new_texture = s_ptr->texture_info.file_name;
    h.register_string(
        &new_texture,
        ((lafi::button*) frm_sector->widgets["but_texture"])->text
    );
    
    if(!h.all_equal()) {
        register_change("sector data change");
    }
    
    h.set_all();
    
    update_sector_texture(s_ptr, new_texture);
    
    homogenize_selected_sectors();
    sector_to_gui();
}


/* ----------------------------------------------------------------------------
 * Saves the tool data to memory using info on the gui.
 */
void area_editor::gui_to_tools() {
    gui_to_var_helper h;
    
    string new_file_name =
        ((lafi::textbox*) frm_tools->widgets["txt_file"])->text;
    h.register_string(
        &cur_area_data.reference_file_name,
        new_file_name
    );
    
    point new_center(
        s2f(((lafi::textbox*) frm_tools->widgets["txt_x"])->text),
        s2f(((lafi::textbox*) frm_tools->widgets["txt_y"])->text)
    );
    h.register_point(
        &cur_area_data.reference_center,
        new_center
    );
    
    bool new_aspect_ratio =
        ((lafi::checkbox*) frm_tools->widgets["chk_ratio"])->checked;
    h.register_bool(
        &reference_transformation.keep_aspect_ratio,
        new_aspect_ratio
    );
    
    point new_size(
        s2f(((lafi::textbox*) frm_tools->widgets["txt_w"])->text),
        s2f(((lafi::textbox*) frm_tools->widgets["txt_h"])->text)
    );
    h.register_point(
        &cur_area_data.reference_size,
        new_size
    );
    
    unsigned char new_alpha =
        ((lafi::scrollbar*) frm_tools->widgets["bar_alpha"])->low_value;
    h.register_uchar(&cur_area_data.reference_alpha, new_alpha);
    
    if(!h.all_equal()) {
        register_change("tools change");
    }
    
    bool is_file_new = false;
    
    if(new_file_name != cur_area_data.reference_file_name) {
        //New reference image, reset size.
        change_reference(new_file_name);
        is_file_new = true;
        if(reference_bitmap) {
            cur_area_data.reference_size.x =
                al_get_bitmap_width(reference_bitmap);
            cur_area_data.reference_size.y =
                al_get_bitmap_height(reference_bitmap);
        }
    }
    
    if(!is_file_new) {
        if(reference_transformation.keep_aspect_ratio) {
            if(
                new_size.x == cur_area_data.reference_size.x &&
                new_size.y != cur_area_data.reference_size.y
            ) {
                if(cur_area_data.reference_size.y == 0.0f) {
                    cur_area_data.reference_size.y = new_size.y;
                } else {
                    float ratio =
                        cur_area_data.reference_size.x /
                        cur_area_data.reference_size.y;
                    cur_area_data.reference_size.x = new_size.y * ratio;
                    cur_area_data.reference_size.y = new_size.y;
                }
                
            } else if(
                new_size.x != cur_area_data.reference_size.x &&
                new_size.y == cur_area_data.reference_size.y
            ) {
                if(cur_area_data.reference_size.x == 0.0f) {
                    cur_area_data.reference_size.x = new_size.x;
                } else {
                    float ratio =
                        cur_area_data.reference_size.y /
                        cur_area_data.reference_size.x;
                    cur_area_data.reference_size.x =
                        new_size.x;
                    cur_area_data.reference_size.y = new_size.x * ratio;
                }
                
            } else {
                cur_area_data.reference_size = new_size;
                
            }
        } else {
            cur_area_data.reference_size = new_size;
        }
    }
    
    cur_area_data.reference_center = new_center;
    cur_area_data.reference_alpha = new_alpha;
    reference_transformation.set_center(cur_area_data.reference_center);
    reference_transformation.set_size(cur_area_data.reference_size);
    reference_transformation.keep_aspect_ratio = new_aspect_ratio;
    
    tools_to_gui();
}


/* ----------------------------------------------------------------------------
 * Hides all menu frames.
 */
void area_editor::hide_all_frames() {
    frm_picker->hide();
    
    frm_main->hide();
    frm_layout->hide();
    frm_asb->hide();
    frm_texture->hide();
    frm_asa->hide();
    frm_mobs->hide();
    frm_paths->hide();
    frm_details->hide();
    frm_review->hide();
    frm_info->hide();
    frm_tools->hide();
    frm_options->hide();
}


/* ----------------------------------------------------------------------------
 * Loads the current area metadata onto the GUI.
 */
void area_editor::info_to_gui() {
    ((lafi::textbox*) frm_info->widgets["txt_name"])->text =
        cur_area_data.name;
    ((lafi::textbox*) frm_info->widgets["txt_subtitle"])->text =
        cur_area_data.subtitle;
    ((lafi::button*) frm_info->widgets["but_weather"])->text =
        cur_area_data.weather_name;
    ((lafi::textbox*) frm_info->widgets["txt_bg_bitmap"])->text =
        cur_area_data.bg_bmp_file_name;
    ((lafi::textbox*) frm_info->widgets["txt_bg_color"])->text =
        c2s(cur_area_data.bg_color);
    ((lafi::textbox*) frm_info->widgets["txt_bg_dist"])->text =
        f2s(cur_area_data.bg_dist);
    ((lafi::textbox*) frm_info->widgets["txt_bg_zoom"])->text =
        f2s(cur_area_data.bg_bmp_zoom);
        
}


/* ----------------------------------------------------------------------------
 * Loads the current mob data onto the GUI.
 */
void area_editor::mob_to_gui() {
    frm_mob->hide();
    frm_mob_multi->hide();
    
    if(selected_mobs.size() == 1 || selection_homogenized) {
        frm_mob->show();
        
        mob_gen* m_ptr = *selected_mobs.begin();
        
        (
            (lafi::angle_picker*) frm_mob->widgets["ang_angle"]
        )->set_angle_rads(m_ptr->angle);
        ((lafi::textbox*) frm_mob->widgets["txt_vars"])->text = m_ptr->vars;
        
        ((lafi::label*) frm_mob->widgets["lbl_cat"])->text =
            "Category: " +
            (m_ptr->category ? m_ptr->category->plural_name : "");
        ((lafi::button*) frm_mob->widgets["but_type"])->text =
            m_ptr->type ? m_ptr->type->name : "";
            
    } else if(selected_mobs.size() > 1 && !selection_homogenized) {
        frm_mob_multi->show();
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type of content, use area_editor_old::AREA_EDITOR_PICKER_*.
 */
void area_editor::open_picker(const unsigned char type) {
    vector<pair<string, string> > elements;
    bool can_create_new = false;
    string title;
    picker_type = type;
    
    if(type == AREA_EDITOR_PICKER_AREA) {
        vector<string> folders = folder_to_vector(AREAS_FOLDER_PATH, true);
        for(size_t f = 0; f < folders.size(); ++f) {
            elements.push_back(make_pair("", folders[f]));
        }
        title = "Create/load an area.";
        can_create_new = true;
        
    } else if(type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
    
        for(size_t t = 0; t < sector_types.get_nr_of_types(); ++t) {
            elements.push_back(make_pair("", sector_types.get_name(t)));
        }
        title = "Choose a sector type.";
        
    } else if(type == AREA_EDITOR_PICKER_HAZARD) {
    
        for(auto h = hazards.begin(); h != hazards.end(); ++h) {
            elements.push_back(make_pair("", h->first));
        }
        title = "Choose a hazard.";
        
    } else if(type == AREA_EDITOR_PICKER_MOB_TYPE) {
    
        mob_gen* m_ptr = *selected_mobs.begin();
        
        for(unsigned char f = 0; f < N_MOB_CATEGORIES; ++f) {
            //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            
            vector<string> names;
            mob_categories.get(f)->get_type_names(names);
            string cat_name = mob_categories.get(f)->plural_name;
            
            for(size_t n = 0; n < names.size(); ++n) {
                elements.push_back(make_pair(cat_name, names[n]));
            }
        }
        
        title = "Choose a mob type.";
        
    } else if(type == AREA_EDITOR_PICKER_WEATHER) {
    
        for(
            auto w = weather_conditions.begin();
            w != weather_conditions.end(); ++w
        ) {
            elements.push_back(make_pair("", w->first));
        }
        title = "Choose a weather type.";
        
    }
    
    generate_and_open_picker(elements, title, can_create_new);
}


/* ----------------------------------------------------------------------------
 * Loads the options data onto the GUI.
 */
void area_editor::options_to_gui() {
    ((lafi::label*) frm_options->widgets["lbl_grid"])->text =
        "Grid: " + i2s(area_editor_grid_interval);
    ((lafi::checkbox*) frm_options->widgets["chk_edge_length"])->set(
        area_editor_show_edge_length
    );
    
    if(area_editor_view_mode == VIEW_MODE_TEXTURES) {
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_textures"]
        )->select();
        
    } else if(area_editor_view_mode == VIEW_MODE_WIREFRAME) {
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_wireframe"]
        )->select();
    } else if(area_editor_view_mode == VIEW_MODE_HEIGHTMAP) {
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_heightmap"]
        )->select();
    } else if(area_editor_view_mode == VIEW_MODE_BRIGHTNESS) {
        (
            (lafi::radio_button*) frm_options->widgets["rad_view_brightness"]
        )->select();
    }
    
    ((lafi::textbox*) frm_options->widgets["txt_backup"])->text =
        i2s(area_editor_backup_interval);
    ((lafi::textbox*) frm_options->widgets["txt_undo_limit"])->text =
        i2s(area_editor_undo_limit);
}


/* ----------------------------------------------------------------------------
 * Loads the current path data onto the GUI.
 */
void area_editor::path_to_gui() {
    if(path_drawing_normals) {
        ((lafi::radio_button*) frm_paths->widgets["rad_normal"])->select();
    } else {
        ((lafi::radio_button*) frm_paths->widgets["rad_one_way"])->select();
    }
}


/* ----------------------------------------------------------------------------
 * Adds texture suggestions to the gui frame.
 */
void area_editor::populate_texture_suggestions() {
    lafi::frame* f = (lafi::frame*) frm_texture->widgets["frm_list"];
    
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
            ((lafi::button*) this->frm_sector->widgets["but_texture"])->text =
                name;
            update_texture_suggestions(name);
            gui_to_sector();
            state = EDITOR_STATE_LAYOUT;
            change_to_right_frame();
        };
        i->left_mouse_click_handler = lambda;
        l->left_mouse_click_handler = lambda;
        f->easy_add("img_" + i2s(s), i, 48, 48, lafi::EASY_FLAG_WIDTH_PX);
        f->easy_add("lbl_" + i2s(s), l, 96, 48, lafi::EASY_FLAG_WIDTH_PX);
        f->easy_row(0);
    }
    
    (
        (lafi::scrollbar*) frm_texture->widgets["bar_scroll"]
    )->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Picks an item and closes the list picker frame.
 */
void area_editor::pick(const string &name, const string &category) {
    if(picker_type == AREA_EDITOR_PICKER_AREA) {
        cur_area_name = name;
        area_editor::load_area(false);
        update_main_frame();
        
    } else if(picker_type == AREA_EDITOR_PICKER_HAZARD) {
        register_change("hazard addition");
        sector* s_ptr = *selected_sectors.begin();
        vector<string> list = split(s_ptr->hazards_str, ";");
        if(!s_ptr->hazards_str.empty()) {
            s_ptr->hazards_str += ";";
        }
        s_ptr->hazards_str += name;
        homogenize_selected_sectors();
        asb_to_gui();
        cur_hazard_nr = list.size();
        
    } else if(picker_type == AREA_EDITOR_PICKER_SECTOR_TYPE) {
        register_change("sector type change");
        sector* s_ptr = *selected_sectors.begin();
        s_ptr->type = sector_types.get_nr(name);
        homogenize_selected_sectors();
        asb_to_gui();
        
    } else if(picker_type == AREA_EDITOR_PICKER_MOB_TYPE) {
        register_change("object type change");
        mob_gen* m_ptr = *selected_mobs.begin();
        m_ptr->category = mob_categories.get_from_pname(category);
        m_ptr->type = m_ptr->category->get_type(name);
        homogenize_selected_mobs();
        mob_to_gui();
        
    } else if(picker_type == AREA_EDITOR_PICKER_WEATHER) {
        register_change("weather change");
        cur_area_data.weather_name = name;
        info_to_gui();
        
    }
    
    show_bottom_frame();
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Loads the current review data onto the GUI.
 */
void area_editor::review_to_gui() {
    lafi::button* but_find_prob =
        (lafi::button*) frm_review->widgets["but_find_prob"];
    lafi::button* but_goto_prob =
        (lafi::button*) frm_review->widgets["but_goto_prob"];
    lafi::label* lbl_prob_title_1 =
        (lafi::label*) frm_review->widgets["lbl_prob_title_1"];
    lafi::label* lbl_prob_title_2 =
        (lafi::label*) frm_review->widgets["lbl_prob_title_2"];
    lafi::label* lbl_prob_desc =
        (lafi::label*) frm_review->widgets["lbl_prob_desc"];
        
    lbl_prob_title_1->text.clear();
    lbl_prob_title_2->text.clear();
    lbl_prob_desc->text.clear();
    
    ((lafi::checkbox*) frm_review->widgets["chk_see_textures"])->set(
        sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW
    );
    ((lafi::checkbox*) frm_review->widgets["chk_shadows"])->set(
        show_shadows
    );
    ((lafi::checkbox*) frm_review->widgets["chk_cross_section"])->set(
        show_cross_section
    );
    ((lafi::checkbox*) frm_review->widgets["chk_cross_section_grid"])->set(
        show_cross_section_grid
    );
    
    if(sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW) {
        disable_widget(but_find_prob);
        disable_widget(but_goto_prob);
    } else {
        enable_widget(but_find_prob);
        enable_widget(but_goto_prob);
    }
    
    if(problem_type == EPT_NONE_YET) {
    
        disable_widget(but_goto_prob);
        lbl_prob_title_1->text = "---";
        
    } else if(problem_type == EPT_NONE) {
    
        disable_widget(but_goto_prob);
        lbl_prob_title_1->text = "No problems found.";
        
    } else if(problem_type == EPT_INTERSECTING_EDGES) {
    
        if(
            !problem_edge_intersection.e1 || !problem_edge_intersection.e2
        ) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Two edges cross";
        lbl_prob_title_2->text = "each other!";
        float u;
        edge_intersection* ei_ptr = &problem_edge_intersection;
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
        
        lbl_prob_desc->text =
            "They cross at (" +
            f2s(
                floor(ei_ptr->e1->vertexes[0]->x + cos(a) * u *
                      d.to_float())
            ) + "," + f2s(
                floor(ei_ptr->e1->vertexes[0]->y + sin(a) * u *
                      d.to_float())
            ) + "). Edges should never cross each other.";
            
    } else if(problem_type == EPT_BAD_SECTOR) {
    
        if(non_simples.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        sector* s_ptr = non_simples.begin()->first;
        TRIANGULATION_ERRORS tri_error = non_simples.begin()->second;
        
        lbl_prob_title_1->text = "Non-simple sector!";
        
        if(tri_error == TRIANGULATION_ERROR_LONE_EDGES) {
            lbl_prob_desc->text =
                "It contains lone edges. Try clearing them up.";
        } else if(tri_error == TRIANGULATION_ERROR_NO_EARS) {
            lbl_prob_desc->text =
                "There's been a triangulation error. Try undoing or "
                "deleting the sector, and then rebuild it. Make sure there "
                "are no gaps, and keep it simple.";
        } else if(tri_error == TRIANGULATION_ERROR_VERTEXES_REUSED) {
            lbl_prob_desc->text =
                "Some vertexes are re-used. Make sure the sector "
                "has no loops or that the same vertex is not re-used "
                "by multiple edges of the sector. Split popular vertexes "
                "if you must.";
        }
        
    } else if(problem_type == EPT_LONE_EDGE) {
    
        if(lone_edges.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Lone edge!";
        lbl_prob_desc->text =
            "Likely leftover of something that went wrong. "
            "You probably want to drag one vertex into the other.";
            
    } else if(problem_type == EPT_OVERLAPPING_VERTEXES) {
    
        if(!problem_vertex_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Overlapping vertexes!";
        lbl_prob_desc->text =
            "They are very close together at (" +
            f2s(problem_vertex_ptr->x) + "," +
            f2s(problem_vertex_ptr->y) + "), and should likely be merged "
            "together.";
            
    } else if(problem_type == EPT_UNKNOWN_TEXTURE) {
    
        if(!problem_sector_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Sector with unknown";
        lbl_prob_title_2->text = "texture!";
        lbl_prob_desc->text = "Texture name: \"" + problem_string + "\".";
        
    } else if(problem_type == EPT_MISSING_LEADER) {
    
        disable_widget(gui->widgets["frm_review"]->widgets["but_goto_prob"]);
        lbl_prob_title_1->text = "No leader!";
        lbl_prob_desc->text =
            "You need at least one leader to play.";
            
    } else if(problem_type == EPT_TYPELESS_MOB) {
    
        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Mob with no";
        lbl_prob_title_2->text = "type!";
        
        
    } else if(problem_type == EPT_MOB_OOB) {
    
        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Mob out of";
        lbl_prob_title_2->text = "bounds!";
        
        
    } else if(problem_type == EPT_MOB_IN_WALL) {
    
        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Mob stuck";
        lbl_prob_title_2->text = "in wall!";
        
        
    } else if(problem_type == EPT_LONE_PATH_STOP) {
    
        if(!problem_path_stop_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Lone path stop!";
        
    } else if(problem_type == EPT_PATHS_UNCONNECTED) {
    
        disable_widget(but_goto_prob);
        lbl_prob_title_1->text = "Path split into";
        lbl_prob_title_2->text = "multiple parts!";
        lbl_prob_desc->text =
            "The path graph is split into two or more parts. Connect them.";
            
    } else if(problem_type == EPT_PATH_STOPS_TOGETHER) {
    
        lbl_prob_title_1->text = "Two close path";
        lbl_prob_title_2->text = "stops!";
        lbl_prob_desc->text =
            "These two are very close together. Separate them.";
            
    } else if(problem_type == EPT_PATH_STOP_OOB) {
    
        lbl_prob_title_1->text = "Path stop out";
        lbl_prob_title_2->text = "of bounds!";
        
    } else if(problem_type == EPT_INVALID_SHADOW) {
    
        lbl_prob_title_1->text = "Tree shadow with";
        lbl_prob_title_2->text = "invalid texture!";
        lbl_prob_desc->text = "Texture name: \"" + problem_string + "\".";
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Loads the current sector data onto the GUI.
 */
void area_editor::sector_to_gui() {
    lafi::button* but_sel_filter =
        ((lafi::button*) frm_layout->widgets["but_sel_filter"]);
    if(selection_filter == SELECTION_FILTER_SECTORS) {
        but_sel_filter->icon = icons.get(ICON_SELECT_SECTORS);
        but_sel_filter->description =
            "Current selection filter: Sectors + edges + vertexes. (F)";
    } else if(selection_filter == SELECTION_FILTER_EDGES) {
        but_sel_filter->icon = icons.get(ICON_SELECT_EDGES);
        but_sel_filter->description =
            "Current selection filter: Edges + vertexes. (F)";
    } else {
        but_sel_filter->icon = icons.get(ICON_SELECT_VERTEXES);
        but_sel_filter->description =
            "Current selection filter: Vertexes only. (F)";
    }
    
    frm_sector->hide();
    frm_sector_multi->hide();
    
    if(selected_sectors.size() == 1 || selection_homogenized) {
        frm_sector->show();
        
        sector* s_ptr = *selected_sectors.begin();
        
        ((lafi::textbox*) frm_sector->widgets["txt_z"])->text =
            f2s(s_ptr->z);
            
        if(s_ptr->fade) {
            ((lafi::radio_button*) frm_sector->widgets["rad_fade"])->select();
            ((lafi::button*) frm_sector->widgets["but_texture"])->text = "";
            disable_widget(frm_sector->widgets["but_texture"]);
            
        } else {
            (
                (lafi::radio_button*) frm_sector->widgets["rad_texture"]
            )->select();
            ((lafi::button*) frm_sector->widgets["but_texture"])->text =
                s_ptr->texture_info.file_name;
            enable_widget(frm_sector->widgets["but_texture"]);
            
        }
        
    } else if(selected_sectors.size() > 1 && !selection_homogenized) {
        frm_sector_multi->show();
        
    }
}


/* ----------------------------------------------------------------------------
 * Selects either the previous or the next hazard on the list.
 */
void area_editor::select_different_hazard(const bool next) {
    sector* s_ptr = *selected_sectors.begin();
    vector<string> list = split(s_ptr->hazards_str, ";");
    cur_hazard_nr = min(cur_hazard_nr, list.size() - 1);
    cur_hazard_nr = sum_and_wrap(cur_hazard_nr, next ? 1 : -1, list.size());
    asb_to_gui();
}


/* ----------------------------------------------------------------------------
 * Loads the current tools data onto the GUI.
 */
void area_editor::tools_to_gui() {
    ((lafi::textbox*) frm_tools->widgets["txt_file"])->text =
        cur_area_data.reference_file_name;
    ((lafi::textbox*) frm_tools->widgets["txt_x"])->text =
        f2s(cur_area_data.reference_center.x);
    ((lafi::textbox*) frm_tools->widgets["txt_y"])->text =
        f2s(cur_area_data.reference_center.y);
    ((lafi::textbox*) frm_tools->widgets["txt_w"])->text =
        f2s(cur_area_data.reference_size.x);
    ((lafi::textbox*) frm_tools->widgets["txt_h"])->text =
        f2s(cur_area_data.reference_size.y);
    ((lafi::checkbox*) frm_tools->widgets["chk_ratio"])->set(
        reference_transformation.keep_aspect_ratio
    );
    ((lafi::scrollbar*) frm_tools->widgets["bar_alpha"])->set_value(
        cur_area_data.reference_alpha, false
    );
    reference_transformation.set_center(cur_area_data.reference_center);
    reference_transformation.set_size(cur_area_data.reference_size);
    update_backup_status();
}


/* ----------------------------------------------------------------------------
 * Updates the main frame.
 */
void area_editor::update_main_frame() {
    if(cur_area_name.empty()) {
        frm_area->hide();
    } else {
        frm_area->show();
    }
    ((lafi::button*) frm_main->widgets["but_area"])->text = cur_area_name;
}
