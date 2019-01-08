/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Functions about the area editor's GUI.
 */

#include <algorithm>

#include "editor.h"
#include "../../LAFI/angle_picker.h"
#include "../../LAFI/button.h"
#include "../../LAFI/checkbox.h"
#include "../../LAFI/image.h"
#include "../../LAFI/radio_button.h"
#include "../../LAFI/scrollbar.h"
#include "../../LAFI/textbox.h"
#include "../../functions.h"
#include "../../utils/string_utils.h"
#include "../../vars.h"

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
    
    set_textbox_text(frm_asa, "txt_x", f2s(s_ptr->texture_info.translation.x));
    set_textbox_text(frm_asa, "txt_y", f2s(s_ptr->texture_info.translation.y));
    set_textbox_text(frm_asa, "txt_sx", f2s(s_ptr->texture_info.scale.x));
    set_textbox_text(frm_asa, "txt_sy", f2s(s_ptr->texture_info.scale.y));
    set_angle_picker_angle(frm_asa, "ang_a", s_ptr->texture_info.rot);
    set_textbox_text(frm_asa, "txt_tint", c2s(s_ptr->texture_info.tint));
    set_textbox_text(frm_asa, "txt_brightness", i2s(s_ptr->brightness));
    ((lafi::scrollbar*) frm_asa->widgets["bar_brightness"])->set_value(
        s_ptr->brightness, false
    );
    set_checkbox_check(frm_asa, "chk_shadow", s_ptr->always_cast_shadow);
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
    
    if(
        s_ptr->type == SECTOR_TYPE_BRIDGE ||
        s_ptr->type == SECTOR_TYPE_BRIDGE_RAIL
    ) {
        frm_asb->widgets["lbl_tag"]->show();
        frm_asb->widgets["txt_tag"]->show();
        set_label_text(frm_asb, "lbl_tag", "Bridge height:");
        ((lafi::label*) frm_asb->widgets["txt_tag"])->description =
            "Height to set the sector to, when the bridge opens.";
    } else {
        s_ptr->tag.clear();
        
        frm_asb->widgets["lbl_tag"]->hide();
        frm_asb->widgets["txt_tag"]->hide();
    }
    
    set_button_text(
        frm_asb, "but_sector_type", sector_types.get_name(s_ptr->type)
    );
    
    if(s_ptr->hazards_str.empty()) {
        set_label_text(frm_asb, "lbl_hazard", "(No hazards)");
        disable_widget(frm_asb->widgets["but_h_del"]);
        disable_widget(frm_asb->widgets["but_h_prev"]);
        disable_widget(frm_asb->widgets["but_h_next"]);
        
    } else {
        vector<string> list = semicolon_list_to_vector(s_ptr->hazards_str);
        if(cur_hazard_nr >= list.size()) {
            cur_hazard_nr = list.size() - 1;
        }
        set_label_text(
            frm_asb, "lbl_hazard",
            i2s(cur_hazard_nr + 1) + "/" + i2s(list.size()) + ": " +
            list[cur_hazard_nr]
        );
        
        enable_widget(frm_asb->widgets["but_h_del"]);
        enable_widget(frm_asb->widgets["but_h_prev"]);
        enable_widget(frm_asb->widgets["but_h_next"]);
    }
    
    set_checkbox_check(frm_asb, "chk_h_air", !s_ptr->hazard_floor);
    set_checkbox_check(frm_asb, "chk_pit", s_ptr->is_bottomless_pit);
    set_textbox_text(frm_asb, "txt_tag", s_ptr->tag);
    
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame() {
    sub_state = EDITOR_SUB_STATE_NONE;
    
    frm_toolbar->show();
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
        frm_toolbar->hide();
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
    } else if(state == EDITOR_STATE_STT) {
        frm_stt->show();
        stt_to_gui();
    } else if(state == EDITOR_STATE_OPTIONS) {
        frm_options->show();
        options_to_gui();
    }
}


/* ----------------------------------------------------------------------------
 * GUI functions for clearing the data for the current area.
 */
void area_editor::clear_current_area_gui() {
    frm_area->show();
    enable_widget(frm_toolbar->widgets["but_save"]);
    frm_paths->widgets["lbl_path_dist"]->hide();
    set_checkbox_check(frm_paths, "chk_show_path", false);
    update_main_frame();
}


/* ----------------------------------------------------------------------------
 * Deletes the currently selected hazard from the list.
 */
void area_editor::delete_current_hazard() {
    register_change("hazard removal");
    
    sector* s_ptr = *selected_sectors.begin();
    
    vector<string> list = semicolon_list_to_vector(s_ptr->hazards_str);
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
        set_textbox_text(frm_shadow, "txt_x", f2s(selected_shadow->center.x));
        set_textbox_text(frm_shadow, "txt_y", f2s(selected_shadow->center.y));
        set_textbox_text(frm_shadow, "txt_w", f2s(selected_shadow->size.x));
        set_textbox_text(frm_shadow, "txt_h", f2s(selected_shadow->size.y));
        set_checkbox_check(
            frm_shadow, "chk_ratio",
            selected_shadow_transformation.keep_aspect_ratio
        );
        set_angle_picker_angle(frm_shadow, "ang_an", selected_shadow->angle);
        ((lafi::scrollbar*) frm_shadow->widgets["bar_al"])->set_value(
            selected_shadow->alpha, false
        );
        set_textbox_text(frm_shadow, "txt_file", selected_shadow->file_name);
        set_textbox_text(frm_shadow, "txt_sx", f2s(selected_shadow->sway.x));
        set_textbox_text(frm_shadow, "txt_sy", f2s(selected_shadow->sway.y));
        
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
            s2f(get_textbox_text(frm_asa, "txt_x")),
            s2f(get_textbox_text(frm_asa, "txt_y"))
        )
    );
    h.register_point(
        &s_ptr->texture_info.scale,
        point(
            s2f(get_textbox_text(frm_asa, "txt_sx")),
            s2f(get_textbox_text(frm_asa, "txt_sy"))
        )
    );
    h.register_float(
        &s_ptr->texture_info.rot,
        get_angle_picker_angle(frm_asa, "ang_a")
    );
    h.register_color(
        &s_ptr->texture_info.tint,
        s2c(get_textbox_text(frm_asa, "txt_tint"))
    );
    h.register_uchar(
        &s_ptr->brightness,
        s2i(get_textbox_text(frm_asa, "txt_brightness"))
    );
    h.register_bool(
        &s_ptr->always_cast_shadow,
        get_checkbox_check(frm_asa, "chk_shadow")
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
        !get_checkbox_check(frm_asb, "chk_h_air")
    );
    h.register_bool(
        &s_ptr->is_bottomless_pit,
        get_checkbox_check(frm_asb, "chk_pit")
    );
    h.register_string(
        &s_ptr->tag,
        get_textbox_text(frm_asb, "txt_tag")
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
            s2f(get_textbox_text(frm_shadow, "txt_x")),
            s2f(get_textbox_text(frm_shadow, "txt_y"))
        )
    );
    h.register_bool(
        &selected_shadow_transformation.keep_aspect_ratio,
        get_checkbox_check(frm_shadow, "chk_ratio")
    );
    point new_size(
        s2f(get_textbox_text(frm_shadow, "txt_w")),
        s2f(get_textbox_text(frm_shadow, "txt_h"))
    );
    if(get_checkbox_check(frm_shadow, "chk_ratio")) {
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
        get_angle_picker_angle(frm_shadow, "ang_an")
    );
    h.register_uchar(
        &selected_shadow->alpha,
        ((lafi::scrollbar*) frm_shadow->widgets["bar_al"])->low_value
    );
    h.register_point(
        &selected_shadow->sway,
        point(
            s2f(get_textbox_text(frm_shadow, "txt_sx")),
            s2f(get_textbox_text(frm_shadow, "txt_sy"))
        )
    );
    
    string new_file_name =
        get_textbox_text(frm_shadow, "txt_file");
        
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
        get_textbox_text(frm_info, "txt_name")
    );
    h.register_string(
        &cur_area_data.subtitle,
        get_textbox_text(frm_info, "txt_subtitle")
    );
    h.register_string(
        &cur_area_data.weather_name,
        get_button_text(frm_info, "but_weather")
    );
    h.register_string(
        &cur_area_data.bg_bmp_file_name,
        get_textbox_text(frm_info, "txt_bg_bitmap")
    );
    h.register_color(
        &cur_area_data.bg_color,
        s2c(get_textbox_text(frm_info, "txt_bg_color"))
    );
    h.register_float(
        &cur_area_data.bg_dist,
        s2f(get_textbox_text(frm_info, "txt_bg_dist"))
    );
    h.register_float(
        &cur_area_data.bg_bmp_zoom,
        s2f(get_textbox_text(frm_info, "txt_bg_zoom"))
    );
    h.register_string(
        &cur_area_data.creator,
        get_textbox_text(frm_info, "txt_creator")
    );
    h.register_string(
        &cur_area_data.version,
        get_textbox_text(frm_info, "txt_version")
    );
    h.register_string(
        &cur_area_data.notes,
        get_textbox_text(frm_info, "txt_notes")
    );
    h.register_string(
        &cur_area_data.spray_amounts,
        get_textbox_text(frm_info, "txt_sprays")
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
        get_angle_picker_angle(frm_mob, "ang_angle")
    );
    h.register_string(
        &m_ptr->vars,
        get_textbox_text(frm_mob, "txt_vars")
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
        get_checkbox_check(frm_options, "chk_edge_length");
        
    if(get_radio_selection(frm_options, "rad_view_textures")) {
        area_editor_view_mode = VIEW_MODE_TEXTURES;
        
    } else if(get_radio_selection(frm_options, "rad_view_wireframe")) {
        area_editor_view_mode = VIEW_MODE_WIREFRAME;
        
    } else if(get_radio_selection(frm_options, "rad_view_heightmap")) {
        area_editor_view_mode = VIEW_MODE_HEIGHTMAP;
        
    } else if(get_radio_selection(frm_options, "rad_view_brightness")) {
        area_editor_view_mode = VIEW_MODE_BRIGHTNESS;
        
    }
    
    area_editor_backup_interval =
        s2i(get_textbox_text(frm_options, "txt_backup"));
    area_editor_undo_limit =
        s2i(get_textbox_text(frm_options, "txt_undo_limit"));
        
    editor_mmb_pan =
        get_checkbox_check(frm_options, "chk_mmb_pan");
    editor_mouse_drag_threshold =
        s2i(get_textbox_text(frm_options, "txt_drag_threshold"));
        
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
        s2f(get_textbox_text(frm_sector, "txt_z"))
    );
    h.register_bool(
        &s_ptr->fade,
        get_radio_selection(frm_sector, "rad_fade")
    );
    string new_texture = s_ptr->texture_info.file_name;
    h.register_string(
        &new_texture,
        get_button_text(frm_sector, "but_texture")
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
    string new_file_name = get_textbox_text(frm_tools, "txt_ref_file");
    
    reference_transformation.set_center(
        point(
            s2f(get_textbox_text(frm_tools, "txt_ref_x")),
            s2f(get_textbox_text(frm_tools, "txt_ref_y"))
        )
    );
    
    reference_transformation.keep_aspect_ratio =
        get_checkbox_check(frm_tools, "chk_ref_ratio");
        
    point new_size(
        s2f(get_textbox_text(frm_tools, "txt_ref_w")),
        s2f(get_textbox_text(frm_tools, "txt_ref_h"))
    );
    
    reference_alpha =
        ((lafi::scrollbar*) frm_tools->widgets["bar_ref_alpha"])->low_value;
        
    if(reference_transformation.keep_aspect_ratio) {
        if(
            new_size.x == reference_transformation.get_size().x &&
            new_size.y != reference_transformation.get_size().y
        ) {
            if(reference_transformation.get_size().y != 0.0f) {
                float ratio =
                    reference_transformation.get_size().x /
                    reference_transformation.get_size().y;
                new_size.x = new_size.y * ratio;
            }
            
        } else if(
            new_size.x != reference_transformation.get_size().x &&
            new_size.y == reference_transformation.get_size().y
        ) {
            if(reference_transformation.get_size().x != 0.0f) {
                float ratio =
                    reference_transformation.get_size().y /
                    reference_transformation.get_size().x;
                new_size.y = new_size.x * ratio;
            }
            
        }
    }
    
    reference_transformation.set_size(new_size);
    
    update_reference(new_file_name);
    
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
    frm_stt->hide();
    frm_tools->hide();
    frm_options->hide();
}


/* ----------------------------------------------------------------------------
 * Loads the current area metadata onto the GUI.
 */
void area_editor::info_to_gui() {
    set_textbox_text(frm_info, "txt_name", cur_area_data.name);
    set_textbox_text(frm_info, "txt_subtitle", cur_area_data.subtitle);
    set_button_text(frm_info, "but_weather", cur_area_data.weather_name);
    set_textbox_text(frm_info, "txt_bg_bitmap", cur_area_data.bg_bmp_file_name);
    set_textbox_text(frm_info, "txt_bg_color", c2s(cur_area_data.bg_color));
    set_textbox_text(frm_info, "txt_bg_dist", f2s(cur_area_data.bg_dist));
    set_textbox_text(frm_info, "txt_bg_zoom", f2s(cur_area_data.bg_bmp_zoom));
    set_textbox_text(frm_info, "txt_creator", cur_area_data.creator);
    set_textbox_text(frm_info, "txt_version", cur_area_data.version);
    set_textbox_text(frm_info, "txt_notes", cur_area_data.notes);
    set_textbox_text(frm_info, "txt_sprays", cur_area_data.spray_amounts);
    
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
        
        set_angle_picker_angle(frm_mob, "ang_angle", m_ptr->angle);
        set_textbox_text(frm_mob, "txt_vars", m_ptr->vars);
        
        set_label_text(
            frm_mob, "lbl_cat",
            "Category: " +
            (m_ptr->category ? m_ptr->category->plural_name : "")
        );
        set_button_text(
            frm_mob, "but_type", m_ptr->type ? m_ptr->type->name : ""
        );
        
        set_label_text(
            frm_mob, "lbl_links",
            i2s(m_ptr->links.size()) + " " +
            (m_ptr->links.size() == 1 ? "link" : "links")
        );
        if(m_ptr->links.empty()) {
            disable_widget(frm_mob->widgets["but_del_link"]);
        } else {
            enable_widget(frm_mob->widgets["but_del_link"]);
        }
        
    } else if(selected_mobs.size() > 1 && !selection_homogenized) {
        frm_mob_multi->show();
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the ID of the picker, use area_editor::PICKER_*.
 * The content to use is decided from that.
 */
void area_editor::open_picker(const unsigned char id) {
    vector<pair<string, string> > elements;
    bool can_create_new = false;
    string title;
    
    if(
        id == PICKER_LOAD_AREA
    ) {
        vector<string> folders = folder_to_vector(AREAS_FOLDER_PATH, true);
        for(size_t f = 0; f < folders.size(); ++f) {
            elements.push_back(make_pair("", folders[f]));
        }
        title = "Create/load an area.";
        can_create_new = true;
        
    } else if(
        id == PICKER_SET_SECTOR_TYPE
    ) {
    
        for(size_t t = 0; t < sector_types.get_nr_of_types(); ++t) {
            elements.push_back(make_pair("", sector_types.get_name(t)));
        }
        title = "Choose a sector type.";
        
    } else if(
        id == PICKER_ADD_SECTOR_HAZARD
    ) {
    
        for(auto h = hazards.begin(); h != hazards.end(); ++h) {
            elements.push_back(make_pair("", h->first));
        }
        title = "Choose a hazard.";
        
    } else if(
        id == PICKER_SET_MOB_TYPE
    ) {
    
        for(unsigned char f = 0; f < N_MOB_CATEGORIES; ++f) {
            //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            
            vector<string> names;
            mob_category* cat = mob_categories.get(f);
            cat->get_type_names(names);
            string cat_name = mob_categories.get(f)->plural_name;
            
            for(size_t n = 0; n < names.size(); ++n) {
                if(!cat->get_type(names[n])->appears_in_area_editor) {
                    continue;
                }
                elements.push_back(make_pair(cat_name, names[n]));
            }
        }
        
        title = "Choose a mob type.";
        
    } else if(
        id == PICKER_SET_WEATHER
    ) {
    
        for(
            auto w = weather_conditions.begin();
            w != weather_conditions.end(); ++w
        ) {
            elements.push_back(make_pair("", w->first));
        }
        title = "Choose a weather type.";
        
    }
    
    generate_and_open_picker(id, elements, title, can_create_new);
}


/* ----------------------------------------------------------------------------
 * Loads the options data onto the GUI.
 */
void area_editor::options_to_gui() {
    set_label_text(
        frm_options, "lbl_grid", "Grid: " + i2s(area_editor_grid_interval)
    );
    set_checkbox_check(
        frm_options, "chk_edge_length", area_editor_show_edge_length
    );
    
    if(area_editor_view_mode == VIEW_MODE_TEXTURES) {
        set_radio_selection(frm_options, "rad_view_textures", true);
        
    } else if(area_editor_view_mode == VIEW_MODE_WIREFRAME) {
        set_radio_selection(frm_options, "rad_view_wireframe", true);
    } else if(area_editor_view_mode == VIEW_MODE_HEIGHTMAP) {
        set_radio_selection(frm_options, "rad_view_heightmap", true);
    } else if(area_editor_view_mode == VIEW_MODE_BRIGHTNESS) {
        set_radio_selection(frm_options, "rad_view_brightness", true);
    }
    
    set_textbox_text(
        frm_options, "txt_backup", i2s(area_editor_backup_interval)
    );
    set_textbox_text(
        frm_options, "txt_undo_limit", i2s(area_editor_undo_limit)
    );
    set_checkbox_check(frm_options, "chk_mmb_pan", editor_mmb_pan);
    set_textbox_text(
        frm_options, "txt_drag_threshold", i2s(editor_mouse_drag_threshold)
    );
}


/* ----------------------------------------------------------------------------
 * Loads the current path data onto the GUI.
 */
void area_editor::path_to_gui() {
    if(path_drawing_normals) {
        set_radio_selection(frm_paths, "rad_normal", true);
    } else {
        set_radio_selection(frm_paths, "rad_one_way", true);
    }
}


/* ----------------------------------------------------------------------------
 * Picks an item and closes the list picker frame.
 */
void area_editor::pick(
    const size_t picker_id, const string &name, const string &category
) {
    if(picker_id == PICKER_LOAD_AREA) {
        cur_area_name = name;
        area_editor::load_area(false);
        update_main_frame();
        
    } else if(picker_id == PICKER_ADD_SECTOR_HAZARD) {
        register_change("hazard addition");
        sector* s_ptr = *selected_sectors.begin();
        vector<string> list = semicolon_list_to_vector(s_ptr->hazards_str);
        if(!s_ptr->hazards_str.empty()) {
            s_ptr->hazards_str += ";";
        }
        s_ptr->hazards_str += name;
        homogenize_selected_sectors();
        asb_to_gui();
        cur_hazard_nr = list.size();
        
    } else if(picker_id == PICKER_SET_SECTOR_TYPE) {
        register_change("sector type change");
        sector* s_ptr = *selected_sectors.begin();
        s_ptr->type = sector_types.get_nr(name);
        homogenize_selected_sectors();
        asb_to_gui();
        
    } else if(picker_id == PICKER_SET_MOB_TYPE) {
        register_change("object type change");
        mob_gen* m_ptr = *selected_mobs.begin();
        m_ptr->category = mob_categories.get_from_pname(category);
        m_ptr->type = m_ptr->category->get_type(name);
        last_mob_category = m_ptr->category;
        last_mob_type = m_ptr->type;
        homogenize_selected_mobs();
        mob_to_gui();
        
    } else if(picker_id == PICKER_SET_WEATHER) {
        register_change("weather change");
        cur_area_data.weather_name = name;
        info_to_gui();
        
    }
    
    frm_toolbar->show();
    change_to_right_frame();
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
            set_button_text(this->frm_sector, "but_texture", name);
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
    
    set_checkbox_check(
        frm_review, "chk_see_textures",
        sub_state == EDITOR_SUB_STATE_TEXTURE_VIEW
    );
    set_checkbox_check(
        frm_review, "chk_shadows", show_shadows
    );
    set_checkbox_check(
        frm_review, "chk_cross_section", show_cross_section
    );
    set_checkbox_check(
        frm_review, "chk_cross_section_grid", show_cross_section_grid
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
        
        
    } else if(problem_type == EPT_SECTORLESS_BRIDGE) {
    
        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        lbl_prob_title_1->text = "Bridge mob on";
        lbl_prob_title_2->text = "wrong sector!";
        lbl_prob_desc->text =
            "This bridge mob should be on a sector of the "
            "\"Bridge\" type.";
            
            
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
        (lafi::button*) frm_layout->widgets["but_sel_filter"];
    if(selection_filter == SELECTION_FILTER_SECTORS) {
        but_sel_filter->icon = editor_icons[ICON_SECTORS];
        but_sel_filter->description =
            "Current selection filter: Sectors + edges + vertexes. (F)";
    } else if(selection_filter == SELECTION_FILTER_EDGES) {
        but_sel_filter->icon = editor_icons[ICON_EDGES];
        but_sel_filter->description =
            "Current selection filter: Edges + vertexes. (F)";
    } else {
        but_sel_filter->icon = editor_icons[ICON_VERTEXES];
        but_sel_filter->description =
            "Current selection filter: Vertexes only. (F)";
    }
    
    frm_sector->hide();
    frm_sector_multi->hide();
    
    if(selected_sectors.size() == 1 || selection_homogenized) {
        frm_sector->show();
        
        sector* s_ptr = *selected_sectors.begin();
        
        set_textbox_text(frm_sector, "txt_z", f2s(s_ptr->z));
        
        if(s_ptr->fade) {
            set_radio_selection(frm_sector, "rad_fade", true);
            set_button_text(frm_sector, "but_texture", "");
            disable_widget(frm_sector->widgets["but_texture"]);
            
        } else {
            set_radio_selection(frm_sector, "rad_texture", true);
            set_button_text(
                frm_sector, "but_texture", s_ptr->texture_info.file_name
            );
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
    vector<string> list = semicolon_list_to_vector(s_ptr->hazards_str);
    cur_hazard_nr = min(cur_hazard_nr, list.size() - 1);
    cur_hazard_nr = sum_and_wrap(cur_hazard_nr, next ? 1 : -1, list.size());
    asb_to_gui();
}


/* ----------------------------------------------------------------------------
 * Loads the current sector texture transformer data onto the GUI.
 */
void area_editor::stt_to_gui() {
    if(stt_mode == 0) {
        set_radio_selection(frm_stt, "rad_offset", true);
    } else if(stt_mode == 1) {
        set_radio_selection(frm_stt, "rad_scale", true);
    } else {
        set_radio_selection(frm_stt, "rad_angle", true);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the current tools data onto the GUI.
 */
void area_editor::tools_to_gui() {
    set_textbox_text(
        frm_tools, "txt_ref_file", reference_file_name
    );
    set_textbox_text(
        frm_tools, "txt_ref_x", f2s(reference_transformation.get_center().x)
    );
    set_textbox_text(
        frm_tools, "txt_ref_y", f2s(reference_transformation.get_center().y)
    );
    set_textbox_text(
        frm_tools, "txt_ref_w", f2s(reference_transformation.get_size().x)
    );
    set_textbox_text(
        frm_tools, "txt_ref_h", f2s(reference_transformation.get_size().y)
    );
    set_checkbox_check(
        frm_tools, "chk_ref_ratio", reference_transformation.keep_aspect_ratio
    );
    ((lafi::scrollbar*) frm_tools->widgets["bar_ref_alpha"])->set_value(
        reference_alpha, false
    );
    update_backup_status();
}


/* ----------------------------------------------------------------------------
 * Updates the main frame.
 */
void area_editor::update_main_frame() {
    if(cur_area_name.empty()) {
        frm_area->hide();
    } else {
        loaded_content_yet = true;
        frm_area->show();
    }
    set_button_text(frm_main, "but_area", cur_area_name);
    
    set_label_text(
        frm_area, "lbl_n_sectors",
        "Sectors: " + i2s(cur_area_data.sectors.size())
    );
    set_label_text(
        frm_area, "lbl_n_vertexes",
        "Vertexes: " + i2s(cur_area_data.vertexes.size())
    );
    set_label_text(
        frm_area, "lbl_n_mobs",
        "Objects: " + i2s(cur_area_data.mob_generators.size())
    );
    set_label_text(
        frm_area, "lbl_n_path_stops",
        "Path stops: " + i2s(cur_area_data.path_stops.size())
    );
}
