/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor loading function.
 */

#include "area_editor_old.h"
#include "../functions.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/frame.h"
#include "../LAFI/minor.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../load.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor_old::load() {
    fade_mgr.start_fade(true, nullptr);
    
    update_gui_coordinates();
    mode = EDITOR_MODE_MAIN;
    
    load_custom_particle_generators(false);
    load_liquids(false);
    load_status_types(false);
    load_hazards();
    load_mob_types(false);
    
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(32, 32, 64),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main -- declarations.
    lafi::frame* frm_main =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_area",
        new lafi::label("Area:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_area",
        new lafi::button(), 100, 32
    );
    int y = frm_main->easy_row();
    
    lafi::frame* frm_area =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    frm_area->hide();
    frm_area->easy_row();
    frm_area->easy_add(
        "but_sectors",
        new lafi::button("Edit sectors"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_objects",
        new lafi::button("Edit objects"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_paths",
        new lafi::button("Edit paths"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_shadows",
        new lafi::button("Edit shadows"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_reference",
        new lafi::button("Edit reference"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_review",
        new lafi::button("Review"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_tools",
        new lafi::button("Special tools"), 100, 32
    );
    frm_area->easy_row();
    
    
    //Main -- properties.
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
        mode = EDITOR_MODE_FOLDER_PATHS;
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
        
    frm_area->widgets["but_reference"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_REFERENCE;
        change_to_right_frame();
    };
    frm_area->widgets["but_reference"]->description =
        "Add a reference image, like a sketch, to guide you.";
        
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
        
        
    //Sectors -- declarations.
    lafi::frame* frm_sectors =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_sectors->hide();
    gui->add("frm_sectors", frm_sectors);
    
    frm_sectors->easy_row();
    frm_sectors->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sectors->easy_row();
    frm_sectors->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(NEW_ICON)), 20, 32
    );
    frm_sectors->easy_add(
        "but_circle",
        new lafi::button("", "", icons.get(NEW_CIRCLE_SECTOR_ICON)), 20, 32
    );
    frm_sectors->easy_add(
        "but_sel_none",
        new lafi::button("", "", icons.get(SELECT_NONE_ICON)), 20, 32
    );
    frm_sectors->easy_add(
        "but_rem",
        new lafi::button("", "", icons.get(DELETE_ICON)), 20, 32
    );
    y = frm_sectors->easy_row();
    
    lafi::frame* frm_sector =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_sector->hide();
    frm_sectors->add("frm_sector", frm_sector);
    
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_type",
        new lafi::label("Type:"), 30, 24
    );
    frm_sector->easy_add(
        "but_type",
        new lafi::button(), 70, 24
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
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 65, 16
    );
    frm_sector->easy_add(
        "chk_hazards_floor",
        new lafi::checkbox("Floor"), 35, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "txt_hazards",
        new lafi::textbox(), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_1",
        new lafi::line(), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_texture",
        new lafi::label("Texture:"), 70, 16
    );
    frm_sector->easy_add(
        "chk_fade",
        new lafi::checkbox("Fade"), 30, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_texture",
        new lafi::button(), 100, 24
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "but_adv",
        new lafi::button("Adv. texture settings"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_brightness",
        new lafi::label("Brightness:"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "bar_brightness",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 80, 16
    );
    frm_sector->easy_add(
        "txt_brightness",
        new lafi::textbox(), 20, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "chk_shadow",
        new lafi::checkbox("Always cast shadow"), 100, 16
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lin_2",
        new lafi::line(), 100, 8
    );
    frm_sector->easy_row();
    frm_sector->easy_add(
        "lbl_tag",
        new lafi::label("Tags:"), 25, 16
    );
    frm_sector->easy_add(
        "txt_tag",
        new lafi::textbox(), 75, 16
    );
    frm_sector->easy_row();
    
    
    //Sectors -- properties.
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
            is_new_sector_line_valid(snap_to_grid(mouse_cursor_w));
        if(sec_mode == ESM_NEW_SECTOR) sec_mode = ESM_NONE;
        else sec_mode = ESM_NEW_SECTOR;
    };
    frm_sectors->widgets["but_new"]->description =
        "Trace a new sector where you click.";
        
    frm_sectors->widgets["but_circle"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cancel_new_sector();
        if(sec_mode == ESM_NEW_CIRCLE_SECTOR) {
            sec_mode = ESM_NONE;
        } else {
            sec_mode = ESM_NEW_CIRCLE_SECTOR;
        }
    };
    frm_sectors->widgets["but_circle"]->description =
        "Create a new circular sector in three steps.";
        
    frm_sectors->widgets["but_sel_none"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_sector = NULL;
        sector_to_gui();
    };
    frm_sectors->widgets["but_sel_none"]->description =
        "Deselect the current sector.";
        
    frm_sectors->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sector) return;
        if(!remove_isolated_sector(cur_sector)) return;
        cur_sector = NULL;
        sector_to_gui();
    };
    frm_sectors->widgets["but_rem"]->description =
        "Removes the selected sector, if it's isolated.";
        
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
            bitmaps.get(
                TEXTURES_FOLDER_NAME + "/" +
                cur_sector->texture_info.file_name, NULL
            );
            
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
        
        
    //Advanced texture settings -- declarations.
    lafi::frame* frm_adv_textures =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_adv_textures->hide();
    gui->add("frm_adv_textures", frm_adv_textures);
    
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lin_1",
        new lafi::line(), 20, 16
    );
    frm_adv_textures->easy_add(
        "lbl_main",
        new lafi::label("Main texture"), 60, 16
    );
    frm_adv_textures->easy_add(
        "lin_2",
        new lafi::line(), 20, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 40, 16
    );
    frm_adv_textures->easy_add(
        "txt_x",
        new lafi::textbox(), 30, 16
    );
    frm_adv_textures->easy_add(
        "txt_y",
        new lafi::textbox(), 30, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_sxy",
        new lafi::label("Scale:"), 40, 16
    );
    frm_adv_textures->easy_add(
        "txt_sx",
        new lafi::textbox(), 30, 16
    );
    frm_adv_textures->easy_add(
        "txt_sy",
        new lafi::textbox(), 30, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_a",
        new lafi::label("Angle:"), 50, 16
    );
    frm_adv_textures->easy_add(
        "ang_a",
        new lafi::angle_picker(), 50, 24
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "lbl_tint",
        new lafi::label("Tint color:"), 100, 16
    );
    frm_adv_textures->easy_row();
    frm_adv_textures->easy_add(
        "txt_tint",
        new lafi::textbox(), 100, 16
    );
    frm_adv_textures->easy_row();
    
    
    //Advanced texture settings -- properties.
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
        
        
    //Texture picker -- declarations.
    lafi::frame* frm_texture =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_texture->hide();
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
    
    
    //Objects -- declarations.
    lafi::frame* frm_objects =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_objects->hide();
    gui->add("frm_objects", frm_objects);
    
    frm_objects->easy_row();
    frm_objects->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_objects->easy_row();
    frm_objects->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(NEW_ICON)), 20, 32
    );
    frm_objects->easy_add(
        "but_sel_none",
        new lafi::button("", "", icons.get(SELECT_NONE_ICON)), 20, 32
    );
    frm_objects->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(DELETE_ICON)), 20, 32
    );
    frm_objects->easy_add(
        "but_duplicate",
        new lafi::button("", "", icons.get(DUPLICATE_ICON)), 20, 32
    );
    y = frm_objects->easy_row();
    
    lafi::frame* frm_object =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_object->hide();
    frm_objects->add("frm_object", frm_object);
    
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_category",
        new lafi::label("Category:"), 100, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_category",
        new lafi::button(), 100, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_type",
        new lafi::label("Type:"), 100, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_type",
        new lafi::button(), 100, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_angle",
        new lafi::label("Angle:"), 50, 16
    );
    frm_object->easy_add(
        "ang_angle",
        new lafi::angle_picker(), 50, 24
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_vars",
        new lafi::label("Script variables:"), 100, 16
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "txt_vars",
        new lafi::textbox(), 100, 16
    );
    frm_object->easy_row();
    
    
    //Objects -- properties.
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
        "Deselect the current object.";
        
    frm_objects->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_mob) return;
        for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
            if(cur_area_data.mob_generators[m] == cur_mob) {
                cur_area_data.mob_generators.erase(
                    cur_area_data.mob_generators.begin() + m
                );
                delete cur_mob;
                cur_mob = NULL;
                mob_to_gui();
                made_changes = true;
                break;
            }
        }
    };
    frm_objects->widgets["but_del"]->description =
        "Delete the current object (Ctrl+Minus).";
        
    frm_objects->widgets["but_duplicate"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_mob) return;
        toggle_duplicate_mob_mode();
    };
    frm_objects->widgets["but_duplicate"]->description =
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
        frm_objects->widgets["but_duplicate"]
    );
    frm_object->register_accelerator(
        ALLEGRO_KEY_MINUS, ALLEGRO_KEYMOD_CTRL,
        frm_objects->widgets["but_del"]
    );
    
    
    //Paths -- declarations.
    lafi::frame* frm_paths =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_paths->hide();
    gui->add("frm_paths", frm_paths);
    
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_create",
        new lafi::label("Create:"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_new_stop",
        new lafi::button("", "", icons.get(NEW_STOP_ICON)), 33, 32
    );
    frm_paths->easy_add(
        "but_new_link",
        new lafi::button("", "", icons.get(NEW_LINK_ICON)), 33, 32
    );
    frm_paths->easy_add(
        "but_new_1wlink",
        new lafi::button("", "", icons.get(NEW_1WLINK_ICON)), 33, 32
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "lbl_delete",
        new lafi::label("Delete:"), 100, 16
    );
    frm_paths->easy_row();
    frm_paths->easy_add(
        "but_del_stop",
        new lafi::button("", "", icons.get(DELETE_STOP_ICON)), 33, 32
    );
    frm_paths->easy_add(
        "but_del_link",
        new lafi::button("", "", icons.get(DELETE_LINK_ICON)), 33, 32
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
            this->gui->widgets["frm_paths"]->widgets["lbl_path_dist"]->show();
        } else {
            this->gui->widgets["frm_paths"]->widgets["lbl_path_dist"]->hide();
        }
    };
    frm_paths->widgets["chk_show_path"]->description =
        "Show path between draggable points A and B.";
        
    frm_paths->widgets["lbl_path_dist"]->description =
        "Total travel distance between A and B.";
        
    gui->widgets["frm_paths"]->widgets["lbl_path_dist"]->hide();
    
    
    //Shadows -- declarations.
    lafi::frame* frm_shadows =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_shadows->hide();
    gui->add("frm_shadows", frm_shadows);
    
    frm_shadows->easy_row();
    frm_shadows->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_shadows->easy_row();
    frm_shadows->easy_add(
        "but_new",
        new lafi::button("", "", icons.get(NEW_ICON)), 20, 32
    );
    frm_shadows->easy_add(
        "but_sel_none",
        new lafi::button("", "", icons.get(SELECT_NONE_ICON)), 20, 32
    );
    frm_shadows->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(DELETE_ICON)), 20, 32
    );
    y = frm_shadows->easy_row();
    
    lafi::frame* frm_shadow =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_shadow->hide();
    frm_shadows->add("frm_shadow", frm_shadow);
    
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
    
    
    //Shadows -- properties.
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
        
    frm_shadows->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_shadow) return;
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
    frm_shadows->widgets["but_del"]->description =
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
        
        
    //Reference -- declarations.
    lafi::frame* frm_reference =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_reference->hide();
    gui->add("frm_reference", frm_reference);
    
    frm_reference->easy_row();
    frm_reference->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "lbl_file",
        new lafi::label("File:"), 30, 16
    );
    frm_reference->easy_add(
        "txt_file",
        new lafi::textbox(), 70, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 30, 16
    );
    frm_reference->easy_add(
        "txt_x",
        new lafi::textbox(), 35, 16
    );
    frm_reference->easy_add(
        "txt_y",
        new lafi::textbox(), 35, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "lbl_wh",
        new lafi::label("W&H:"), 30, 16
    );
    frm_reference->easy_add(
        "txt_w",
        new lafi::textbox(), 35, 16
    );
    frm_reference->easy_add(
        "txt_h",
        new lafi::textbox(), 35, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "chk_ratio",
        new lafi::checkbox("Keep aspect ratio"), 100, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "chk_mouse",
        new lafi::checkbox("Transform with mouse"), 100, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "lbl_alpha",
        new lafi::label("Opacity:"), 100, 16
    );
    frm_reference->easy_row();
    frm_reference->easy_add(
        "bar_alpha",
        new lafi::scrollbar(0, 0, 0, 0, 0, 285, 0, 30, false), 100, 24
    );
    frm_reference->easy_row();
    
    
    //Reference -- properties.
    auto lambda_gui_to_reference =
    [this] (lafi::widget*) { gui_to_reference(); };
    auto lambda_gui_to_reference_click =
    [this] (lafi::widget*, int, int) { gui_to_reference(); };
    
    frm_reference->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        sec_mode = ESM_NONE;
        reference_to_gui();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_reference->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_reference->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["txt_file"]->description =
        "Image file (on the Images folder) for the reference.";
        
    frm_reference->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["txt_x"]->description =
        "X of the top-left corner for the reference.";
        
    frm_reference->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["txt_y"]->description =
        "Y of the top-left corner for the reference.";
        
    frm_reference->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["txt_w"]->description =
        "Reference total width.";
        
    frm_reference->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["txt_h"]->description =
        "Reference total height.";
        
    frm_reference->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_gui_to_reference_click;
    frm_reference->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    frm_reference->widgets["chk_mouse"]->left_mouse_click_handler =
        lambda_gui_to_reference_click;
    frm_reference->widgets["chk_mouse"]->description =
        "If checked, use mouse buttons to move/stretch.";
        
    ((lafi::scrollbar*) frm_reference->widgets["bar_alpha"])->change_handler =
        lambda_gui_to_reference;
    frm_reference->widgets["bar_alpha"]->description =
        "How see-through the reference is.";
        
    reference_to_gui();
    
    
    //Review -- declarations.
    lafi::frame* frm_review =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_review->hide();
    gui->add("frm_review", frm_review);
    
    frm_review->easy_row();
    frm_review->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_find_errors",
        new lafi::button("Find errors"), 100, 24
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_lbl",
        new lafi::label("Error found:", ALLEGRO_ALIGN_CENTER),
        100, 16
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_1",
        new lafi::label(), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_2",
        new lafi::label(), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_3",
        new lafi::label(), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "lbl_error_4",
        new lafi::label(), 100, 12
    );
    frm_review->easy_row();
    frm_review->easy_add(
        "but_goto_error",
        new lafi::button("Go to error"), 100, 24
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
        "dum_1",
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
        "dum_2",
        new lafi::dummy(), 10, 16
    );
    frm_review->easy_add(
        "chk_cross_section_grid",
        new lafi::checkbox("See height grid"), 90, 16
    );
    frm_review->easy_row();
    update_review_frame();
    
    
    //Review -- properties.
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
    frm_review->widgets["chk_cross_section"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_cross_section = ((lafi::checkbox*) c)->checked;
        update_review_frame();
    };
    frm_review->widgets["chk_cross_section"]->description =
        "Show a 2D cross section between points A and B.";
        
    frm_review->widgets["chk_cross_section_grid"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        show_cross_section_grid = ((lafi::checkbox*) c)->checked;
        update_review_frame();
    };
    frm_review->widgets["chk_cross_section_grid"]->description =
        "Show a height grid in the cross-section window.";
        
        
    //Tools -- declarations.
    lafi::frame* frm_tools =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_tools->hide();
    gui->add("frm_tools", frm_tools);
    
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
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
        
        
    //Options -- declarations.
    lafi::frame* frm_options =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_options->hide();
    gui->add("frm_options", frm_options);
    
    frm_options->easy_row();
    frm_options->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "but_load",
        new lafi::button("Reload area"), 100, 24
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "but_backup",
        new lafi::button("Load auto-backup"), 100, 24
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
    update_options_frame();
    
    
    //Options -- properties.
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
        
        
    //Bottom bar -- declarations.
    lafi::frame* frm_bottom =
        new lafi::frame(gui_x, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    
    frm_bottom->easy_row();
    frm_bottom->easy_add(
        "but_options",
        new lafi::button("", "", icons.get(OPTIONS_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_reference",
        new lafi::button("", "", icons.get(REFERENCE_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_save",
        new lafi::button("", "", icons.get(SAVE_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_quit",
        new lafi::button("", "", icons.get(EXIT_ICON)), 25, 32
    );
    frm_bottom->easy_row();
    
    lafi::label* gui_status_bar =
        new lafi::label(0, status_bar_y, gui_x, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Bottom bar -- properties.
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
        
    frm_bottom->widgets["but_reference"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_reference = !show_reference;
    };
    frm_bottom->widgets["but_reference"]->description =
        "Toggle the visibility of the reference.";
        
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
        
    create_changes_warning_frame();
    create_picker_frame(true);
    
    disable_widget(frm_options->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
    
    cam_zoom = 1.0;
    cam_pos.x = cam_pos.y = 0.0;
    grid_interval = DEF_GRID_INTERVAL;
    show_closest_stop = false;
    area_name.clear();
    
    if(!auto_load_area.empty()) {
        area_name = auto_load_area;
        load_area(false);
    }
    
    cross_section_window_start = point(0.0f, 0.0f);
    cross_section_window_end = point(gui_x * 0.5, status_bar_y * 0.5);
    cross_section_z_window_start =
        point(cross_section_window_end.x, cross_section_window_start.y);
    cross_section_z_window_end =
        point(cross_section_window_end.x + 48, cross_section_window_end.y);
        
}
