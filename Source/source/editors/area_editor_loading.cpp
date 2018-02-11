/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor loading function.
 */

#include "area_editor.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/gui.h"
#include "../LAFI/label.h"
#include "../LAFI/minor.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/style.h"
#include "../LAFI/textbox.h"
#include "../functions.h"
#include "../load.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {

    update_gui_coordinates();
    
    gui_style =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(32, 32, 64),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    faded_style =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(128, 128, 160),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    gui = new lafi::gui(scr_w, scr_h, gui_style);
    
    
    //Main -- declarations.
    frm_main =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_area",
        new lafi::label("Current area:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_area",
        new lafi::button(), 100, 32
    );
    int y = frm_main->easy_row();
    
    frm_area =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    
    frm_area->easy_row();
    frm_area->easy_add(
        "but_info",
        new lafi::button("Info", "", icons.get(ICON_INFO)), 50, 48
    );
    frm_area->easy_add(
        "but_layout",
        new lafi::button("Layout", "", icons.get(ICON_LAYOUT)), 50, 48
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_mobs",
        new lafi::button("Objects", "", icons.get(ICON_MOBS)), 50, 48
    );
    frm_area->easy_add(
        "but_paths",
        new lafi::button("Paths", "", icons.get(ICON_PATHS)), 50, 48
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_details",
        new lafi::button("Details", "", icons.get(ICON_DETAILS)), 50, 48
    );
    frm_area->easy_add(
        "but_review",
        new lafi::button("Review", "", icons.get(ICON_REVIEW)), 50, 48
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_tools",
        new lafi::button("Tools", "", icons.get(ICON_TOOLS)), 50, 48
    );
    frm_area->easy_add(
        "but_options",
        new lafi::button("Options", "", icons.get(ICON_OPTIONS)), 50, 48
    );
    frm_area->easy_row();
    
    
    //Main -- properties.
    frm_main->widgets["but_area"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            close_changes_warning();
            open_picker(AREA_EDITOR_PICKER_AREA);
        }
    };
    frm_main->widgets["but_area"]->description =
        "Pick which area you want to edit.";
        
    frm_area->widgets["but_info"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_INFO;
        change_to_right_frame();
    };
    frm_area->widgets["but_info"]->description =
        "Set the area's name, weather, etc.";
        
    frm_area->widgets["but_layout"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_LAYOUT;
        change_to_right_frame();
    };
    frm_area->widgets["but_layout"]->description =
        "Draw sectors (polygons) to create the layout.";
        
    frm_area->widgets["but_mobs"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MOBS;
        change_to_right_frame();
    };
    frm_area->widgets["but_mobs"]->description =
        "Change object settings and placements.";
        
    frm_area->widgets["but_paths"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_PATHS;
        change_to_right_frame();
    };
    frm_area->widgets["but_paths"]->description =
        "Draw movement paths and stops.";
        
    frm_area->widgets["but_details"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_DETAILS;
        change_to_right_frame();
    };
    frm_area->widgets["but_details"]->description =
        "Edit misc. details, like tree shadows.";
        
    frm_area->widgets["but_review"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_REVIEW;
        change_to_right_frame();
    };
    frm_area->widgets["but_review"]->description =
        "Use this to make sure everything is okay in the area.";
        
    frm_area->widgets["but_tools"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_TOOLS;
        change_to_right_frame();
    };
    frm_area->widgets["but_tools"]->description =
        "Special tools to help you develop the area.";
        
    frm_area->widgets["but_options"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_OPTIONS;
        change_to_right_frame();
    };
    frm_area->widgets["but_options"]->description =
        "Options for the area editor.";
        
        
    //Info -- declarations.
    frm_info =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_info", frm_info);
    
    frm_info->easy_row();
    frm_info->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_info->easy_add(
        "lbl_panel_name",
        new lafi::label("INFO", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lin_gen_1",
        new lafi::line(), 30, 16
    );
    frm_info->easy_add(
        "lbl_general",
        new lafi::label("General", ALLEGRO_ALIGN_CENTER), 40, 16
    );
    frm_info->easy_add(
        "lin_gen_2",
        new lafi::line(), 30, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_name",
        new lafi::label("Name:"), 30, 16
    );
    frm_info->easy_add(
        "txt_name",
        new lafi::textbox(), 70, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_subtitle",
        new lafi::label("Subtitle:"), 40, 16
    );
    frm_info->easy_add(
        "txt_subtitle",
        new lafi::textbox(), 60, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_weather",
        new lafi::label("Weather:"), 50, 16
    );
    frm_info->easy_add(
        "but_no_weather",
        new lafi::button("None"), 50, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "dum_1",
        new lafi::dummy(), 15, 24
    );
    frm_info->easy_add(
        "but_weather",
        new lafi::button(), 85, 24
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lin_bg_1",
        new lafi::line(), 20, 24
    );
    frm_info->easy_add(
        "lbl_bg",
        new lafi::label("Background", ALLEGRO_ALIGN_CENTER), 60, 16
    );
    frm_info->easy_add(
        "lin_bg_2",
        new lafi::line(), 20, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_bg_bitmap",
        new lafi::label("Bitmap:"), 40, 16
    );
    frm_info->easy_add(
        "txt_bg_bitmap",
        new lafi::textbox(), 60, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_bg_color",
        new lafi::label("Color:"), 40, 16
    );
    frm_info->easy_add(
        "txt_bg_color",
        new lafi::textbox(), 60, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_bg_dist",
        new lafi::label("Dist.:"), 30, 16
    );
    frm_info->easy_add(
        "txt_bg_dist",
        new lafi::textbox(), 20, 16
    );
    frm_info->easy_add(
        "lbl_bg_zoom",
        new lafi::label("Zoom:"), 30, 16
    );
    frm_info->easy_add(
        "txt_bg_zoom",
        new lafi::textbox(), 20, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lin_meta_1",
        new lafi::line(), 20, 24
    );
    frm_info->easy_add(
        "lbl_meta",
        new lafi::label("Metadata", ALLEGRO_ALIGN_CENTER), 60, 16
    );
    frm_info->easy_add(
        "lin_meta_2",
        new lafi::line(), 20, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_creator",
        new lafi::label("Creator:"), 30, 16
    );
    frm_info->easy_add(
        "txt_creator",
        new lafi::textbox(), 70, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_version",
        new lafi::label("Version:"), 30, 16
    );
    frm_info->easy_add(
        "txt_version",
        new lafi::textbox(), 70, 16
    );
    frm_info->easy_row();
    frm_info->easy_add(
        "lbl_notes",
        new lafi::label("Notes:"), 30, 16
    );
    frm_info->easy_add(
        "txt_notes",
        new lafi::textbox(), 70, 16
    );
    frm_info->easy_row();
    
    
    //Info -- properties.
    frm_info->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_info->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_info->widgets["lbl_panel_name"]->style = faded_style;
    
    auto lambda_gui_to_info =
    [this] (lafi::widget*) {
        gui_to_info();
    };
    frm_info->widgets["txt_name"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_name"]->description =
        "The area's name.";
        
    frm_info->widgets["txt_subtitle"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_subtitle"]->description =
        "Subtitle, if any. Appears on the loading screen.";
        
    frm_info->widgets["but_no_weather"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        register_change("weather removal");
        cur_area_data.weather_name.clear();
        info_to_gui();
    };
    frm_info->widgets["but_no_weather"]->description =
        "Sets the weather to none.";
        
    frm_info->widgets["but_weather"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_WEATHER);
    };
    frm_info->widgets["but_weather"]->description =
        "The weather condition to use.";
    ((lafi::button*) frm_info->widgets["but_weather"])->autoscroll =
        true;
        
    frm_info->widgets["txt_bg_bitmap"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_bg_bitmap"]->description =
        "File name of the texture to use as a background, extension included."
        " e.g. \"Kitchen_floor.jpg\"";
        
    frm_info->widgets["txt_bg_color"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_bg_color"]->description =
        "Color of the background, in the format \"r g b a\".";
        
    frm_info->widgets["txt_bg_dist"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_bg_dist"]->description =
        "How far away the background is. 2 is a good value.";
        
    frm_info->widgets["txt_bg_zoom"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_bg_zoom"]->description =
        "Scale the texture by this amount.";
        
    frm_info->widgets["txt_creator"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_creator"]->description =
        "Name (or nickname) of who created this area. (Optional)";
        
    frm_info->widgets["txt_version"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_version"]->description =
        "Version of the area, preferably in the \"X.Y.Z\" format. (Optional)";
        
    frm_info->widgets["txt_notes"]->lose_focus_handler = lambda_gui_to_info;
    frm_info->widgets["txt_notes"]->description =
        "Extra notes or comments about the area, if any.";
        
        
    //Layout -- declarations.
    frm_layout =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_layout", frm_layout);
    
    frm_layout->easy_row();
    frm_layout->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_layout->easy_add(
        "lbl_panel_name",
        new lafi::label("LAYOUT", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_layout->easy_row();
    frm_layout->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(ICON_NEW)), 20, 32
    );
    frm_layout->easy_add(
        "but_circle",
        new lafi::button("", "", icons.get(ICON_NEW_CIRCLE_SECTOR)), 20, 32
    );
    frm_layout->easy_add(
        "but_rem",
        new lafi::button("", "", icons.get(ICON_DELETE)), 20, 32
    );
    frm_layout->easy_add(
        "but_sel_filter",
        new lafi::button(), 20, 32
    );
    frm_layout->easy_add(
        "but_sel_none",
        new lafi::button("", "", icons.get(ICON_SELECT_NONE)), 20, 32
    );
    y = frm_layout->easy_row();
    
    frm_sector =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_layout->add("frm_sector", frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_1",
        new lafi::line(), 10, 16
    );
    frm_sector->easy_add(
        "lbl_behavior",
        new lafi::label("Sector behavior", ALLEGRO_ALIGN_CENTER), 80, 16
    );
    frm_sector->easy_add(
        "lin_2",
        new lafi::line(), 10, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_z",
        new lafi::label("Height:"), 50, 16
    );
    frm_sector->easy_add(
        "txt_z",
        new lafi::textbox(), 50, 16
    );
    frm_sector->easy_row(8, 8, 0);
    frm_sector->easy_add(
        "dum_z",
        new lafi::dummy(), 32, 12
    );
    frm_sector->easy_add(
        "but_z_m50",
        new lafi::button("-50"), 17, 14
    );
    frm_sector->easy_add(
        "but_z_m10",
        new lafi::button("-10"), 17, 14
    );
    frm_sector->easy_add(
        "but_z_p10",
        new lafi::button("+10"), 17, 14
    );
    frm_sector->easy_add(
        "but_z_p50",
        new lafi::button("+50"), 17, 14
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_adv_behavior",
        new lafi::button("Advanced..."), 100, 24
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "dum_2",
        new lafi::dummy(), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_3",
        new lafi::line(), 10, 16
    );
    frm_sector->easy_add(
        "lbl_appearance",
        new lafi::label("Sector appearance", ALLEGRO_ALIGN_CENTER), 80, 16
    );
    frm_sector->easy_add(
        "lin_4",
        new lafi::line(), 10, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "rad_fade",
        new lafi::radio_button("Texture fader"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "rad_texture",
        new lafi::radio_button("Regular texture"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_texture",
        new lafi::button(), 100, 24
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "dum_3",
        new lafi::dummy(), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_adv_appearance",
        new lafi::button("Advanced..."), 100, 24
    );
    frm_sector->easy_row();
    
    frm_sector_multi =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_layout->add("frm_sector_multi", frm_sector_multi);
    
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_1",
        new lafi::label("Multiple different", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_2",
        new lafi::label("sectors selected. To", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_3",
        new lafi::label("make all their", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_4",
        new lafi::label("properties the same", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_5",
        new lafi::label("and edit them all", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "lbl_multi_6",
        new lafi::label("together, click here:", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_sector_multi->easy_row();
    frm_sector_multi->easy_add(
        "but_ok",
        new lafi::button("Edit all together"), 100, 24
    );
    frm_sector_multi->easy_row();
    
    
    //Layout -- properties.
    frm_layout->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_selection();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_layout->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_layout->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_layout->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_layout_drawing();
        if(sub_state == EDITOR_SUB_STATE_DRAWING) {
            cancel_layout_drawing();
        } else {
            sub_state = EDITOR_SUB_STATE_DRAWING;
        }
    };
    frm_layout->widgets["but_new"]->description =
        "Trace a new sector where you click. (N)";
        
    frm_layout->widgets["but_circle"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_circle_sector();
        if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
            cancel_circle_sector();
        } else {
            sub_state = EDITOR_SUB_STATE_CIRCLE_SECTOR;
        }
    };
    frm_layout->widgets["but_circle"]->description =
        "Create a new circular sector in three steps. (C)";
        
    frm_layout->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(selected_sectors.empty()) {
            emit_status_bar_message(
                "You have to select sectors to delete!", false
            );
            return;
        }
        area_data* prepared_state = prepare_state();
        if(!remove_isolated_sectors()) {
            emit_status_bar_message(
                "Some of the sectors are not isolated!", false
            );
            forget_prepared_state(prepared_state);
        } else {
            emit_status_bar_message(
                "Deleted sectors.", false
            );
            clear_selection();
            register_change("sector removal", prepared_state);
        }
    };
    frm_layout->widgets["but_rem"]->description =
        "Removes the selected sectors, if they're isolated. (Delete)";
        
    frm_layout->widgets["but_sel_filter"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_selection();
        selection_filter =
            sum_and_wrap(selection_filter, 1, N_SELECTION_FILTERS);
        sector_to_gui();
    };
    
    frm_layout->widgets["but_sel_none"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_selection();
    };
    frm_layout->widgets["but_sel_none"]->description =
        "Clear the selection. (Esc)";
        
    auto lambda_gui_to_sector =
    [this] (lafi::widget*) { gui_to_sector(); };
    auto lambda_gui_to_sector_click =
    [this] (lafi::widget*, int, int) { gui_to_sector(); };
    
    frm_sector->widgets["txt_z"]->lose_focus_handler =
        lambda_gui_to_sector;
    frm_sector->widgets["txt_z"]->description =
        "Height of the floor.";
        
    frm_sector->widgets["but_z_m50"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        lafi::textbox* t = (lafi::textbox*) this->frm_sector->widgets["txt_z"];
        t->text = f2s(s2f(t->text) - 50);
        gui_to_sector();
    };
    frm_sector->widgets["but_z_m50"]->description =
        "Decrease the height number by 50.";
        
    frm_sector->widgets["but_z_m10"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        lafi::textbox* t = (lafi::textbox*) this->frm_sector->widgets["txt_z"];
        t->text = f2s(s2f(t->text) - 10);
        gui_to_sector();
    };
    frm_sector->widgets["but_z_m10"]->description =
        "Decrease the height number by 10.";
        
    frm_sector->widgets["but_z_p10"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        lafi::textbox* t = (lafi::textbox*) this->frm_sector->widgets["txt_z"];
        t->text = f2s(s2f(t->text) + 10);
        gui_to_sector();
    };
    frm_sector->widgets["but_z_p10"]->description =
        "Increase the height number by 10.";
        
    frm_sector->widgets["but_z_p50"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        lafi::textbox* t = (lafi::textbox*) this->frm_sector->widgets["txt_z"];
        t->text = f2s(s2f(t->text) + 50);
        gui_to_sector();
    };
    frm_sector->widgets["but_z_p50"]->description =
        "Increase the height number by 50.";
        
    frm_sector->widgets["but_adv_behavior"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_hazard_nr = 0;
        state = EDITOR_STATE_ASB;
        change_to_right_frame();
        asb_to_gui();
    };
    frm_sector->widgets["but_adv_behavior"]->description =
        "Open more advanced sector behavior settings.";
        
    frm_sector->widgets["rad_fade"]->left_mouse_click_handler =
        lambda_gui_to_sector_click;
    frm_sector->widgets["rad_fade"]->description =
        "Makes the surrounding textures fade into each other.";
        
    frm_sector->widgets["rad_texture"]->left_mouse_click_handler =
        lambda_gui_to_sector_click;
    frm_sector->widgets["rad_texture"]->description =
        "Makes the sector use a regular texture.";
        
    frm_sector->widgets["but_texture"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_TEXTURE;
        populate_texture_suggestions();
        change_to_right_frame();
    };
    frm_sector->widgets["but_texture"]->description =
        "Select a texture (image) for this sector.";
    ((lafi::button*) frm_sector->widgets["but_texture"])->autoscroll =
        true;
        
    frm_sector->widgets["but_adv_appearance"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_ASA;
        change_to_right_frame();
    };
    frm_sector->widgets["but_adv_appearance"]->description =
        "Open more advanced sector appearance settings.";
        
    frm_sector_multi->widgets["but_ok"]->description =
        "Confirm that you want all selected sectors to be similar.";
    frm_sector_multi->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        register_change("sector combining");
        selection_homogenized = true;
        homogenize_selected_sectors();
        sector_to_gui();
    };
    
    
    //Advanced sector behavior -- declarations.
    frm_asb =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_asb", frm_asb);
    
    frm_asb->easy_row();
    frm_asb->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "lbl_sector_type",
        new lafi::label("Sector type:"), 100, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "but_sector_type",
        new lafi::button(""), 100, 24
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 100, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "but_h_add",
        new lafi::button("", "", icons.get(ICON_NEW)), 25, 24
    );
    frm_asb->easy_add(
        "but_h_del",
        new lafi::button("", "", icons.get(ICON_DELETE)), 25, 24
    );
    frm_asb->easy_add(
        "but_h_prev",
        new lafi::button("", "", icons.get(ICON_PREVIOUS)), 25, 24
    );
    frm_asb->easy_add(
        "but_h_next",
        new lafi::button("", "", icons.get(ICON_NEXT)), 25, 24
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "dum_1",
        new lafi::dummy(), 10, 16
    );
    frm_asb->easy_add(
        "lbl_hazard",
        new lafi::label("", 0, true), 90, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "chk_h_air",
        new lafi::checkbox("Floor and air"), 100, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "lbl_tag",
        new lafi::label(), 100, 16
    );
    frm_asb->easy_row();
    frm_asb->easy_add(
        "txt_tag",
        new lafi::textbox(), 100, 16
    );
    frm_asb->easy_row();
    
    
    //Advanced sector behavior -- properties.
    frm_asb->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_LAYOUT;
        change_to_right_frame();
    };
    frm_asb->widgets["but_back"]->description =
        "Return to the layout menu.";
        
    frm_asb->widgets["but_sector_type"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_SECTOR_TYPE);
    };
    frm_asb->widgets["but_sector_type"]->description =
        "Change the type of sector.";
        
    frm_asb->widgets["but_h_add"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_HAZARD);
    };
    frm_asb->widgets["but_h_add"]->description =
        "Add a new hazard to the list.";
        
    frm_asb->widgets["but_h_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        delete_current_hazard();
    };
    frm_asb->widgets["but_h_del"]->description =
        "Remove the current hazard from the list.";
        
    frm_asb->widgets["but_h_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        select_different_hazard(false);
    };
    frm_asb->widgets["but_h_prev"]->description =
        "Show the previous hazard in the list.";
        
    frm_asb->widgets["but_h_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        select_different_hazard(true);
    };
    frm_asb->widgets["but_h_next"]->description =
        "Show the next hazard in the list.";
        
    frm_asb->widgets["chk_h_air"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        gui_to_asb();
    };
    frm_asb->widgets["chk_h_air"]->description =
        "Trigger hazards on the floor only or in the air too?";
        
    frm_asb->widgets["txt_tag"]->lose_focus_handler =
    [this] (lafi::widget*) {
        gui_to_asb();
    };
    
    
    //Texture picker -- declarations.
    frm_texture =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_texture", frm_texture);
    
    frm_texture->add(
        "but_back",
        new lafi::button(gui_x + 8, 8, gui_x + 96, 24, "Back")
    );
    frm_texture->add(
        "txt_name",
        new lafi::textbox(gui_x + 8, 40, scr_w - 48, 56)
    );
    frm_texture->add(
        "but_ok",
        new lafi::button(scr_w - 40, 32, scr_w - 8, 64, "Ok")
    );
    frm_texture->add(
        "lbl_suggestions",
        new lafi::label(gui_x + 8, 72, scr_w - 8, 88, "Suggestions:")
    );
    frm_texture->add(
        "frm_list",
        new lafi::frame(gui_x + 8, 96, scr_w - 32, scr_h - 56)
    );
    frm_texture->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24, 96, scr_w - 8, scr_h - 56)
    );
    
    
    //Texture picker -- properties.
    frm_texture->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_LAYOUT;
        change_to_right_frame();
    };
    frm_texture->widgets["but_back"]->description =
        "Cancel.";
        
    frm_texture->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string name =
            (
                (lafi::textbox*) this->frm_texture->widgets["txt_name"]
            )->text;
        if(name.empty()) return;
        ((lafi::button*) this->frm_sector->widgets["but_texture"])->text =
            name;
        ((lafi::textbox*) this->frm_texture->widgets["txt_name"])->text.clear();
        update_texture_suggestions(name);
        gui_to_sector();
        state = EDITOR_STATE_LAYOUT;
        change_to_right_frame();
    };
    
    ((lafi::textbox*) frm_texture->widgets["txt_name"])->enter_key_widget =
        frm_texture->widgets["but_ok"];
        
    frm_texture->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->frm_texture->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(
                0,
                (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) /
                2 - 30 * dy
            );
        }
    };
    
    
    //Advanced sector appearance -- declarations.
    frm_asa =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_asa", frm_asa);
    
    frm_asa->easy_row();
    frm_asa->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lin_1",
        new lafi::line(), 30, 16
    );
    frm_asa->easy_add(
        "lbl_texture",
        new lafi::label("Texture", ALLEGRO_ALIGN_CENTER), 40, 16
    );
    frm_asa->easy_add(
        "lin_2",
        new lafi::line(), 30, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 40, 16
    );
    frm_asa->easy_add(
        "txt_x",
        new lafi::textbox(), 30, 16
    );
    frm_asa->easy_add(
        "txt_y",
        new lafi::textbox(), 30, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lbl_sxy",
        new lafi::label("Scale:"), 40, 16
    );
    frm_asa->easy_add(
        "txt_sx",
        new lafi::textbox(), 30, 16
    );
    frm_asa->easy_add(
        "txt_sy",
        new lafi::textbox(), 30, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lbl_a",
        new lafi::label("Angle:"), 50, 16
    );
    frm_asa->easy_add(
        "ang_a",
        new lafi::angle_picker(), 50, 24
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lbl_tint",
        new lafi::label("Tint color:"), 100, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "txt_tint",
        new lafi::textbox(), 100, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lin_3",
        new lafi::line(), 30, 16
    );
    frm_asa->easy_add(
        "lbl_sector",
        new lafi::label("Sector", ALLEGRO_ALIGN_CENTER), 40, 16
    );
    frm_asa->easy_add(
        "lin_4",
        new lafi::line(), 30, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "lbl_brightness",
        new lafi::label("Brightness:"), 100, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "bar_brightness",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 80, 16
    );
    frm_asa->easy_add(
        "txt_brightness",
        new lafi::textbox(), 20, 16
    );
    frm_asa->easy_row();
    frm_asa->easy_add(
        "chk_shadow",
        new lafi::checkbox("Always cast shadow"), 100, 16
    );
    frm_asa->easy_row();
    
    
    //Advanced sector appearance -- properties.
    auto lambda_gui_to_asa =
    [this] (lafi::widget*) { gui_to_asa(); };
    
    frm_asa->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_LAYOUT;
        change_to_right_frame();
    };
    frm_asa->widgets["but_back"]->description =
        "Return to the layout menu.";
        
    frm_asa->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_x"]->description =
        "Offset the texture horizontally by this much.";
        
    frm_asa->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_y"]->description =
        "Offset the texture vertically by this much.";
        
    frm_asa->widgets["txt_sx"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_sx"]->description =
        "Zoom the texture horizontally by this much.";
        
    frm_asa->widgets["txt_sy"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_sy"]->description =
        "Zoom the texture vertically by this much.";
        
    frm_asa->widgets["ang_a"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["ang_a"]->description =
        "Rotate the texture by this much.";
        
    frm_asa->widgets["txt_tint"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_tint"]->description =
        "Texture tint color, in the format \"r g b a\".";
        
    ((lafi::scrollbar*) frm_asa->widgets["bar_brightness"])->change_handler =
    [this] (lafi::widget * w) {
        ((lafi::textbox*) frm_asa->widgets["txt_brightness"])->text =
            i2s(((lafi::scrollbar*) w)->low_value);
        gui_to_asa();
    };
    frm_asa->widgets["bar_brightness"]->description =
        "0 = pitch black sector. 255 = normal lighting.";
        
    frm_asa->widgets["txt_brightness"]->lose_focus_handler =
        lambda_gui_to_asa;
    frm_asa->widgets["txt_brightness"]->description =
        frm_asa->widgets["bar_brightness"]->description;
        
    frm_asa->widgets["chk_shadow"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        gui_to_asa();
    };
    frm_asa->widgets["chk_shadow"]->description =
        "Always cast a shadow onto lower sectors, "
        "even if they're just a step below.";
        
        
    //Mobs -- declarations.
    frm_mobs =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_mobs", frm_mobs);
    
    frm_mobs->easy_row();
    frm_mobs->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_mobs->easy_add(
        "lbl_panel_name",
        new lafi::label("OBJECTS", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_mobs->easy_row();
    frm_mobs->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(ICON_NEW)), 20, 32
    );
    frm_mobs->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(ICON_DELETE)), 20, 32
    );
    frm_mobs->easy_add(
        "but_duplicate",
        new lafi::button("", "", icons.get(ICON_DUPLICATE)), 20, 32
    );
    y = frm_mobs->easy_row();
    
    frm_mob =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_mobs->add("frm_mob", frm_mob);
    
    frm_mob->easy_row();
    frm_mob->easy_add(
        "lbl_cat",
        new lafi::label(""), 100, 16
    );
    frm_mob->easy_row();
    frm_mob->easy_add(
        "lbl_type",
        new lafi::label("Type:"), 100, 16
    );
    frm_mob->easy_row();
    frm_mob->easy_add(
        "but_type",
        new lafi::button(), 100, 24
    );
    frm_mob->easy_row();
    frm_mob->easy_add(
        "lbl_angle",
        new lafi::label("Angle:"), 50, 16
    );
    frm_mob->easy_add(
        "ang_angle",
        new lafi::angle_picker(), 50, 24
    );
    frm_mob->easy_row();
    frm_mob->easy_add(
        "lbl_vars",
        new lafi::label("Script variables:"), 100, 16
    );
    frm_mob->easy_row();
    frm_mob->easy_add(
        "txt_vars",
        new lafi::textbox(), 100, 16
    );
    frm_mob->easy_row();
    
    frm_mob_multi =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_mobs->add("frm_mob_multi", frm_mob_multi);
    
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_1",
        new lafi::label("Multiple different", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_2",
        new lafi::label("objects selected. To", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_3",
        new lafi::label("make all their", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_4",
        new lafi::label("properties the same", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_5",
        new lafi::label("and edit them all", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "lbl_multi_6",
        new lafi::label("together, click here:", ALLEGRO_ALIGN_CENTER), 100, 12
    );
    frm_mob_multi->easy_row();
    frm_mob_multi->easy_add(
        "but_ok",
        new lafi::button("Edit all together"), 100, 24
    );
    frm_mob_multi->easy_row();
    
    
    //Mobs -- properties.
    auto lambda_gui_to_mob =
    [this] (lafi::widget*) { gui_to_mob(); };
    
    frm_mobs->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_selection();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_mobs->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_mobs->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_mobs->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sub_state == EDITOR_SUB_STATE_NEW_MOB) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            clear_selection();
            sub_state = EDITOR_SUB_STATE_NEW_MOB;
        }
    };
    frm_mobs->widgets["but_new"]->description =
        "Create a new object wherever you click. (N)";
        
    frm_mobs->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(selected_mobs.empty()) {
            emit_status_bar_message(
                "You have to select mobs to delete!", false
            );
            return;
        }
        register_change("object deletion");
        for(auto sm = selected_mobs.begin(); sm != selected_mobs.end(); ++sm) {
            for(size_t mg = 0; mg < cur_area_data.mob_generators.size(); ++mg) {
                if(cur_area_data.mob_generators[mg] == *sm) {
                    cur_area_data.mob_generators.erase(
                        cur_area_data.mob_generators.begin() + mg
                    );
                    delete *sm;
                    break;
                }
            }
        }
        clear_selection();
    };
    frm_mobs->widgets["but_del"]->description =
        "Delete the selected objects. (Delete)";
        
    frm_mobs->widgets["but_duplicate"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(selected_mobs.empty()) {
            emit_status_bar_message(
                "You have to select mobs to duplicate!", false
            );
            return;
        }
        if(sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            sub_state = EDITOR_SUB_STATE_DUPLICATE_MOB;
        }
    };
    frm_mobs->widgets["but_duplicate"]->description =
        "Duplicate the current objects. (D)";
        
    frm_mob->widgets["but_type"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(AREA_EDITOR_PICKER_MOB_TYPE);
    };
    frm_mob->widgets["but_type"]->description =
        "Choose this object's type.";
        
    frm_mob->widgets["ang_angle"]->lose_focus_handler =
        lambda_gui_to_mob;
    frm_mob->widgets["ang_angle"]->description =
        "Angle the object is facing.";
        
    frm_mob->widgets["txt_vars"]->lose_focus_handler =
        lambda_gui_to_mob;
    frm_mob->widgets["txt_vars"]->description =
        "Extra variables (e.g.: \"sleep=y;jumping=n\").";
        
    frm_mob_multi->widgets["but_ok"]->description =
        "Confirm that you want all selected objects to be similar.";
    frm_mob_multi->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        register_change("object combining");
        selection_homogenized = true;
        homogenize_selected_mobs();
        mob_to_gui();
    };
    
    
    //Paths -- declarations.
    frm_paths =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_paths", frm_paths);
    
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_paths->easy_add(
        "lbl_panel_name",
        new lafi::label("PATHS", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_draw",
        new lafi::button("", "", icons.get(ICON_NEW)), 25, 32
    );
    frm_paths->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(ICON_DELETE)), 25, 32
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_drawing",
        new lafi::label("Drawing mode:"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "dum_drawing_1",
        new lafi::dummy(), 10, 16
    );
    frm_paths->easy_add(
        "rad_one_way",
        new lafi::radio_button("One-way links"), 90, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "dum_drawing_2",
        new lafi::dummy(), 10, 16
    );
    frm_paths->easy_add(
        "rad_normal",
        new lafi::radio_button("Normal links"), 90, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lin_tools_1",
        new lafi::line(), 35, 16
    );
    frm_paths->easy_add(
        "lbl_tools",
        new lafi::label("Tools", ALLEGRO_ALIGN_CENTER), 30, 16
    );
    frm_paths->easy_add(
        "lin_tools_2",
        new lafi::line(), 35, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "chk_show_closest",
        new lafi::checkbox("Show closest stop"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "chk_show_path",
        new lafi::checkbox("Show calculated path"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_path_dist",
        new lafi::label("  Total dist.: 0"), 100, 16
    );
    frm_paths->easy_row();
    
    
    //Paths -- properties.
    frm_paths->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_paths->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_paths->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_paths->widgets["but_draw"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            path_drawing_stop_1 = NULL;
            sub_state = EDITOR_SUB_STATE_PATH_DRAWING;
        }
    };
    frm_paths->widgets["but_draw"]->description =
        "Draw path stops and their links. (N)";
        
    frm_paths->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        delete_selected_path_elements();
    };
    frm_paths->widgets["but_del"]->description =
        "Delete the selected stops and/or links. (Delete)";
        
    frm_paths->widgets["rad_one_way"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        path_drawing_normals = false;
        path_to_gui();
    };
    frm_paths->widgets["rad_one_way"]->description =
        "New links drawn will be one-way links. (1)";
        
    frm_paths->widgets["rad_normal"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        path_drawing_normals = true;
        path_to_gui();
    };
    frm_paths->widgets["rad_normal"]->description =
        "New links drawn will be normal (two-way) links. (2)";
        
    frm_paths->widgets["chk_show_closest"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_closest_stop = !show_closest_stop;
    };
    frm_paths->widgets["chk_show_closest"]->description =
        "Show the closest stop to the cursor. Useful to know which stop "
        "Pikmin will go to when starting to carry.";
        
    frm_paths->widgets["chk_show_path"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_path_preview = !show_path_preview;
        if(show_path_preview) {
            calculate_preview_path();
            this->frm_paths->widgets["lbl_path_dist"]->show();
        } else {
            this->frm_paths->widgets["lbl_path_dist"]->hide();
        }
    };
    frm_paths->widgets["chk_show_path"]->description =
        "Show path between the draggable points A and B.";
        
    frm_paths->widgets["lbl_path_dist"]->description =
        "Total travel distance between A and B.";
        
        
    //Details -- declarations.
    frm_details =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_details", frm_details);
    
    frm_details->easy_row();
    frm_details->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_details->easy_add(
        "lbl_panel_name",
        new lafi::label("DETAILS", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_details->easy_row();
    frm_details->easy_add(
        "lin_1",
        new lafi::line(), 20, 16
    );
    frm_details->easy_add(
        "lbl_shadows",
        new lafi::label("Tree shadows", ALLEGRO_ALIGN_CENTER), 60, 16
    );
    frm_details->easy_add(
        "lin_2",
        new lafi::line(), 20, 16
    );
    frm_details->easy_row();
    frm_details->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(ICON_NEW)), 20, 32
    );
    frm_details->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(ICON_DELETE)), 20, 32
    );
    y = frm_details->easy_row();
    
    frm_shadow =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_details->add("frm_shadow", frm_shadow);
    
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_file",
        new lafi::label("File:"), 25, 16
    );
    frm_shadow->easy_add(
        "txt_file",
        new lafi::textbox(), 75, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_x",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_add(
        "txt_y",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_wh",
        new lafi::label("W&H:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_w",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_add(
        "txt_h",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "chk_ratio",
        new lafi::checkbox("Keep aspect ratio"), 100, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_an",
        new lafi::label("Angle:"), 40, 16
    );
    frm_shadow->easy_add(
        "ang_an",
        new lafi::angle_picker(), 60, 24
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_al",
        new lafi::label("Opacity:"), 40, 16
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "bar_al",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24
    );
    frm_shadow->easy_row();
    frm_shadow->easy_add(
        "lbl_sway",
        new lafi::label("Sway X&Y:"), 40, 16
    );
    frm_shadow->easy_add(
        "txt_sx",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_add(
        "txt_sy",
        new lafi::textbox(), 30, 16
    );
    frm_shadow->easy_row();
    
    
    //Details -- properties.
    frm_details->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_details->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_details->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_details->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            sub_state = EDITOR_SUB_STATE_NEW_SHADOW;
        }
    };
    frm_details->widgets["but_new"]->description =
        "Create a new tree shadow wherever you click. (N)";
        
    frm_details->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!selected_shadow) {
            emit_status_bar_message(
                "You have to select shadows to delete!", false
            );
            return;
        }
        register_change("tree shadow deletion");
        for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
            if(cur_area_data.tree_shadows[s] == selected_shadow) {
                cur_area_data.tree_shadows.erase(
                    cur_area_data.tree_shadows.begin() + s
                );
                delete selected_shadow;
                selected_shadow = NULL;
                details_to_gui();
                break;
            }
        }
    };
    frm_details->widgets["but_del"]->description =
        "Delete the current tree shadow. (Delete)";
        
    auto lambda_gui_to_details =
    [this] (lafi::widget*) {
        gui_to_details();
    };
    auto lambda_gui_to_details_click =
    [this] (lafi::widget*, int, int) {
        gui_to_details();
    };
    frm_shadow->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_file"]->description =
        "File name for the shadow's texture.";
        
    frm_shadow->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_x"]->description =
        "X position of the shadow's center.";
        
    frm_shadow->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_y"]->description =
        "Y position of the shadow's center.";
        
    frm_shadow->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_w"]->description =
        "Width of the shadow's image.";
        
    frm_shadow->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_h"]->description =
        "Height of the shadow's image.";
        
    frm_shadow->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_gui_to_details_click;
    frm_shadow->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    frm_shadow->widgets["ang_an"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["ang_an"]->description =
        "Angle of the shadow's image.";
        
    ((lafi::scrollbar*) frm_shadow->widgets["bar_al"])->change_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["bar_al"]->description =
        "How opaque the shadow's image is.";
        
    frm_shadow->widgets["txt_sx"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_sx"]->description =
        "Horizontal sway amount multiplier (0 = no sway).";
        
    frm_shadow->widgets["txt_sy"]->lose_focus_handler =
        lambda_gui_to_details;
    frm_shadow->widgets["txt_sy"]->description =
        "Vertical sway amount multiplier (0 = no sway).";
        
        
    //Review -- declarations.
    frm_review =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_review", frm_review);
    
    frm_review->easy_row();
    frm_review->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_review->easy_add(
        "lbl_panel_name",
        new lafi::label("REVIEW", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_find_prob",
        new lafi::button("Find problems"), 100, 24
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_prob_lbl",
        new lafi::label("Problem found:", ALLEGRO_ALIGN_CENTER),
        100, 8
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_prob_title_1",
        new lafi::label("", ALLEGRO_ALIGN_CENTER), 100, 8
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_prob_title_2",
        new lafi::label("", ALLEGRO_ALIGN_CENTER), 100, 8
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 2
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_prob_desc",
        new lafi::label("", 0, true), 100, 8
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_goto_prob",
        new lafi::button("Go to problem"), 100, 24
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lin_1",
        new lafi::line(), 100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "chk_see_textures",
        new lafi::checkbox("See textures"), 100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "dum_2",
        new lafi::dummy(), 10, 16
    );
    frm_review->easy_add(
        "chk_shadows",
        new lafi::checkbox("See tree shadows"), 90, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "chk_cross_section",
        new lafi::checkbox("Show cross-section"), 100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "dum_3",
        new lafi::dummy(), 10, 16
    );
    frm_review->easy_add(
        "chk_cross_section_grid",
        new lafi::checkbox("See height grid"), 90, 16
    );
    frm_review->easy_row();
    
    
    //Review -- properties.
    frm_review->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        clear_problems();
        review_to_gui();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_review->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_review->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_review->widgets["but_find_prob"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        problem_type = find_problems();
        review_to_gui();
    };
    frm_review->widgets["but_find_prob"]->description =
        "Search for problems with the area.";
        
    frm_review->widgets["but_goto_prob"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        goto_problem();
    };
    frm_review->widgets["but_goto_prob"]->description =
        "Focus the camera on the problem found, if applicable.";
        
    frm_review->widgets["chk_see_textures"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        problem_type = EPT_NONE_YET;
        if(((lafi::checkbox*) c)->checked) {
            sub_state = EDITOR_SUB_STATE_TEXTURE_VIEW;
            review_to_gui();
            
        } else {
            sub_state = EDITOR_SUB_STATE_NONE;
            review_to_gui();
        }
    };
    frm_review->widgets["chk_see_textures"]->description =
        "Preview how the textures and shadows will look like.";
        
    frm_review->widgets["chk_shadows"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_shadows = ((lafi::checkbox*) c)->checked;
        review_to_gui();
    };
    frm_review->widgets["chk_shadows"]->description =
        "Show tree shadows?";
    frm_review->widgets["chk_cross_section"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_cross_section = ((lafi::checkbox*) c)->checked;
        review_to_gui();
    };
    frm_review->widgets["chk_cross_section"]->description =
        "Show a 2D cross-section between points A and B.";
        
    frm_review->widgets["chk_cross_section_grid"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_cross_section_grid = ((lafi::checkbox*) c)->checked;
        review_to_gui();
    };
    frm_review->widgets["chk_cross_section_grid"]->description =
        "Show a height grid in the cross-section window.";
        
        
    //Tools -- declarations.
    frm_tools =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_tools", frm_tools);
    
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_tools->easy_add(
        "lbl_panel_name",
        new lafi::label("TOOLS", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lin_1",
        new lafi::line(), 10, 16
    );
    frm_tools->easy_add(
        "lbl_reference",
        new lafi::label("Reference image", ALLEGRO_ALIGN_CENTER), 80, 16
    );
    frm_tools->easy_add(
        "lin_2",
        new lafi::line(), 10, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_file",
        new lafi::label("", ALLEGRO_ALIGN_LEFT, true), 100, 12
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 30, 16
    );
    frm_tools->easy_add(
        "txt_x",
        new lafi::textbox(), 35, 16
    );
    frm_tools->easy_add(
        "txt_y",
        new lafi::textbox(), 35, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_wh",
        new lafi::label("W&H:"), 30, 16
    );
    frm_tools->easy_add(
        "txt_w",
        new lafi::textbox(), 35, 16
    );
    frm_tools->easy_add(
        "txt_h",
        new lafi::textbox(), 35, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "chk_ratio",
        new lafi::checkbox("Keep aspect ratio"), 100, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_alpha",
        new lafi::label("Opacity:"), 40, 16
    );
    frm_tools->easy_add(
        "bar_alpha",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 60, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lin_3",
        new lafi::line(), 20, 16
    );
    frm_tools->easy_add(
        "lbl_misc",
        new lafi::label("Misc. tools", ALLEGRO_ALIGN_CENTER), 60, 16
    );
    frm_tools->easy_add(
        "lin_4",
        new lafi::line(), 20, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_load",
        new lafi::button("Reload area"), 100, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_backup",
        new lafi::button("Load auto-backup"), 100, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_stt",
        new lafi::button("Texture transformer"), 100, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_resize",
        new lafi::label("Resize everything:"), 100, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_resize",
        new lafi::textbox(), 80, 16
    );
    frm_tools->easy_add(
        "but_resize",
        new lafi::button("Ok"), 20, 24
    );
    frm_tools->easy_row();
    
    
    //Tools -- properties.
    frm_tools->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        save_reference();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_tools->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_tools->widgets["lbl_panel_name"]->style = faded_style;
    
    auto lambda_gui_to_tools =
    [this] (lafi::widget*) {
        gui_to_tools();
    };
    auto lambda_gui_to_tools_click =
    [this] (lafi::widget*, int, int) {
        gui_to_tools();
    };
    frm_tools->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_tools;
    frm_tools->widgets["txt_x"]->description =
        "X of the top-left corner for the reference.";
        
    frm_tools->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_tools;
    frm_tools->widgets["txt_y"]->description =
        "Y of the top-left corner for the reference.";
        
    frm_tools->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_tools;
    frm_tools->widgets["txt_w"]->description =
        "Reference total width.";
        
    frm_tools->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_tools;
    frm_tools->widgets["txt_h"]->description =
        "Reference total height.";
        
    frm_tools->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_gui_to_tools_click;
    frm_tools->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    ((lafi::scrollbar*) frm_tools->widgets["bar_alpha"])->change_handler =
        lambda_gui_to_tools;
    frm_tools->widgets["bar_alpha"]->description =
        "How see-through the reference is.";
        
    frm_tools->widgets["but_load"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            close_changes_warning();
            this->load_area(false);
        }
    };
    frm_tools->widgets["but_load"]->description =
        "Discard all changes made and load the area again.";
        
    frm_tools->widgets["but_backup"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            this->close_changes_warning();
            this->load_backup();
        }
    };
    frm_tools->widgets["but_backup"]->description =
        "Discard all changes made and load the auto-backup.";
        
    frm_tools->widgets["but_stt"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_STT;
        change_to_right_frame();
    };
    frm_tools->widgets["but_stt"]->description =
        "Allows you to transform the sectors's textures with the mouse.";
        
    frm_tools->widgets["txt_resize"]->description =
        "Resize multiplier (0.5 = half, 2 = double).";
        
    frm_tools->widgets["but_resize"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        lafi::textbox* txt_resize =
            (lafi::textbox*) frm_tools->widgets["txt_resize"];
        float mult = s2f(txt_resize->text);
        resize_everything(mult);
    };
    frm_tools->widgets["but_resize"]->description =
        "Resize all X/Y coordinates by the given amount.";
        
        
    //Sector texture transformer -- declarations.
    frm_stt =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_stt", frm_stt);
    
    frm_stt->easy_row();
    frm_stt->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "lbl_1",
        new lafi::label("Drag the mouse on"), 100, 12
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "lbl_2",
        new lafi::label("a texture to change"), 100, 12
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "lbl_3",
        new lafi::label("its properties."), 100, 12
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "rad_offset",
        new lafi::radio_button("Offset", 0, true), 100, 16
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "rad_scale",
        new lafi::radio_button("Scale"), 100, 16
    );
    frm_stt->easy_row();
    frm_stt->easy_add(
        "rad_angle",
        new lafi::radio_button("Angle"), 100, 16
    );
    frm_stt->easy_row();
    
    
    //Sector texture transformer -- properties.
    frm_stt->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_TOOLS;
        change_to_right_frame();
    };
    frm_stt->widgets["but_back"]->description =
        "Go back to the tools menu.";
        
    frm_stt->widgets["rad_offset"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        stt_mode = 0;
        stt_to_gui();
    };
    frm_stt->widgets["rad_offset"]->description =
        "Mouse drags offset the texture. (1)";
        
    frm_stt->widgets["rad_scale"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        stt_mode = 1;
        stt_to_gui();
    };
    frm_stt->widgets["rad_scale"]->description =
        "Mouse drags change the texture's scale. (2)";
        
    frm_stt->widgets["rad_angle"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        stt_mode = 2;
        stt_to_gui();
    };
    frm_stt->widgets["rad_angle"]->description =
        "Mouse drags rotate the texture. (3)";
        
        
    //Options -- declarations.
    frm_options =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_options", frm_options);
    
    frm_options->easy_row();
    frm_options->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_options->easy_add(
        "lbl_panel_name",
        new lafi::label("OPTIONS", ALLEGRO_ALIGN_RIGHT), 50, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "lbl_grid",
        new lafi::label("Grid spacing: "), 70, 24
    );
    frm_options->easy_add(
        "but_grid_plus",
        new lafi::button("+"), 15, 24
    );
    frm_options->easy_add(
        "but_grid_minus",
        new lafi::button("-"), 15, 24
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "chk_edge_length",
        new lafi::checkbox("Show edge length"), 100, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "lbl_view_mode",
        new lafi::label("View mode:"), 100, 12
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "rad_view_textures",
        new lafi::radio_button("Textures"), 100, 12
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "rad_view_wireframe",
        new lafi::radio_button("Wireframe"), 100, 12
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "rad_view_heightmap",
        new lafi::radio_button("Heightmap"), 100, 12
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "rad_view_brightness",
        new lafi::radio_button("Brightness"), 100, 12
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "lbl_backup",
        new lafi::label("Auto-backup time:"), 80, 12
    );
    frm_options->easy_add(
        "txt_backup",
        new lafi::textbox(), 20, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "lbl_undo_limit",
        new lafi::label("Undo limit:"), 80, 12
    );
    frm_options->easy_add(
        "txt_undo_limit",
        new lafi::textbox(), 20, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "chk_mmb_pan",
        new lafi::checkbox("Use MMB to pan"), 100, 16
    );
    frm_options->easy_row();
    
    
    //Options -- properties.
    auto lambda_gui_to_options =
    [this] (lafi::widget*) {
        gui_to_options();
    };
    auto lambda_gui_to_options_click =
    [this] (lafi::widget*, int, int) {
        gui_to_options();
    };
    
    frm_options->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_options->widgets["but_back"]->description =
        "Close the options.";
        
    frm_options->widgets["lbl_panel_name"]->style = faded_style;
    
    frm_options->widgets["but_grid_plus"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        area_editor_grid_interval *= 2;
        area_editor_grid_interval =
            min(area_editor_grid_interval, MAX_GRID_INTERVAL);
        save_options();
        options_to_gui();
    };
    frm_options->widgets["but_grid_plus"]->description =
        "Increase the spacing on the grid.";
        
    frm_options->widgets["but_grid_minus"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        area_editor_grid_interval *= 0.5;
        area_editor_grid_interval =
            max(area_editor_grid_interval, MIN_GRID_INTERVAL);
        save_options();
        options_to_gui();
    };
    frm_options->widgets["but_grid_minus"]->description =
        "Decrease the spacing on the grid.";
        
    frm_options->widgets["chk_edge_length"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["chk_edge_length"]->description =
        "Show the length of nearby edges when drawing or moving vertexes.";
        
    frm_options->widgets["rad_view_textures"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["rad_view_textures"]->description =
        "Draw textures on the sectors.";
        
    frm_options->widgets["rad_view_wireframe"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["rad_view_wireframe"]->description =
        "Do not draw sectors, only edges and vertexes. Best for performance.";
        
    frm_options->widgets["rad_view_heightmap"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["rad_view_heightmap"]->description =
        "Draw sectors as heightmaps. Lighter = taller.";
        
    frm_options->widgets["rad_view_brightness"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["rad_view_brightness"]->description =
        "Draw sectors as solid grays based on their brightness.";
        
    frm_options->widgets["txt_backup"]->lose_focus_handler =
        lambda_gui_to_options;
    frm_options->widgets["txt_backup"]->description =
        "Interval between auto-backup saves, in seconds. 0 = off.";
        
    frm_options->widgets["txt_undo_limit"]->lose_focus_handler =
        lambda_gui_to_options;
    frm_options->widgets["txt_undo_limit"]->description =
        "Maximum number of operations that can be undone. 0 = off.";
        
        
    frm_options->widgets["chk_mmb_pan"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["chk_mmb_pan"]->description =
        "Use the middle mouse button to pan the camera "
        "(and RMB to reset camera/zoom).";
        
        
    //Bottom bar -- declarations.
    frm_bottom =
        new lafi::frame(gui_x, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    
    frm_bottom->easy_row();
    frm_bottom->easy_add(
        "but_undo",
        new lafi::button("", "", icons.get(ICON_UNDO)), 25, 32
    );
    frm_bottom->easy_add(
        "but_reference",
        new lafi::button("", "", icons.get(ICON_REFERENCE)), 25, 32
    );
    frm_bottom->easy_add(
        "but_save",
        new lafi::button("", "", icons.get(ICON_SAVE)), 25, 32
    );
    frm_bottom->easy_add(
        "but_quit",
        new lafi::button("", "", icons.get(ICON_EXIT)), 25, 32
    );
    frm_bottom->easy_row();
    
    
    //Bottom bar -- properties.
    frm_bottom->widgets["but_undo"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        undo();
    };
    frm_bottom->widgets["but_undo"]->description =
        "Undo the last move. (Ctrl+Z)";
        
    frm_bottom->widgets["but_reference"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_reference = !show_reference;
    };
    frm_bottom->widgets["but_reference"]->description =
        "Toggle the visibility of the reference. (Ctrl+R)";
        
    frm_bottom->widgets["but_save"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        save_area(false);
        save_reference();
        clear_selection();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
        made_changes = false;
    };
    frm_bottom->widgets["but_save"]->description =
        "Save the area onto the files. (Ctrl+S)";
        
    frm_bottom->widgets["but_quit"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            leave();
        }
    };
    frm_bottom->widgets["but_quit"]->description =
        "Quit the area editor. (Ctrl+Q)";
        
        
    //Status bar.
    lbl_status_bar =
        new lafi::label(0, status_bar_y, gui_x, scr_h, "", 0, true);
    gui->add("lbl_status_bar", lbl_status_bar);
    
    
    create_changes_warning_frame();
    create_picker_frame();
    
    fade_mgr.start_fade(true, nullptr);
    
    last_mob_category = NULL;
    last_mob_type = NULL;
    show_closest_stop = false;
    show_path_preview = false;
    path_preview_checkpoints[0] = point(-DEF_AREA_EDITOR_GRID_INTERVAL, 0);
    path_preview_checkpoints[1] = point(+DEF_AREA_EDITOR_GRID_INTERVAL, 0);
    clear_selection();
    selected_shadow = NULL;
    selection_homogenized = false;
    cam_zoom = 1.0;
    cam_pos.x = cam_pos.y = 0.0;
    selection_effect = 0.0;
    is_ctrl_pressed = false;
    is_shift_pressed = false;
    is_gui_focused = false;
    gui->lose_focus();
    cross_section_window_start = point(0.0f, 0.0f);
    cross_section_window_end = point(gui_x * 0.5, status_bar_y * 0.5);
    cross_section_z_window_start =
        point(cross_section_window_end.x, cross_section_window_start.y);
    cross_section_z_window_end =
        point(cross_section_window_end.x + 48, cross_section_window_end.y);
        
    state = EDITOR_STATE_MAIN;
    change_to_right_frame();
    open_picker(AREA_EDITOR_PICKER_AREA);
    update_status_bar();
    
    load_custom_particle_generators(false);
    load_spike_damage_types();
    load_liquids(false);
    load_status_types(false);
    load_hazards();
    load_mob_types(false);
    load_weather();
    
    if(!auto_load_area.empty()) {
        cur_area_name = auto_load_area;
        load_area(false);
    }
    
}
