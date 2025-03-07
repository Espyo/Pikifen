
/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pack management menu struct and functions.
 */

#include "packs_menu.h"

#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../util/os_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace PACKS_MENU {

//Name of the pack management menu GUI information file.
const string GUI_FILE_NAME = "packs_menu";

}


/**
 * @brief Changes the info that's being shown about the currently-selected
 * pack.
 *
 * @param idx Index of the pack. -1 for the base pack, -2 for nothing.
 */
void PacksMenu::change_info(int idx) {
    //Figure out what pack this is.
    Pack* pack_ptr = nullptr;
    string new_pack_name;
    if(idx == -1) {
        new_pack_name = FOLDER_NAMES::BASE_PACK;
        pack_ptr = &game.content.packs.list[new_pack_name];
    } else if(idx >= 0 && idx < (int) pack_order.size()) {
        new_pack_name = pack_order[idx];
        pack_ptr = &game.content.packs.list[new_pack_name];
    }
    
    if(cur_pack_name == new_pack_name) {
        return;
    }
    
    cur_pack_name = new_pack_name;
    if(!pack_ptr) {
        pack_name_text->text.clear();
        pack_description_text->text.clear();
        pack_tags_text->text.clear();
        pack_maker_text->text.clear();
        pack_version_text->text.clear();
        return;
    }
    
    //Fill the GUI items.
    pack_name_text->text =
        pack_ptr->name;
    pack_name_text->start_juice_animation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    pack_description_text->text =
        pack_ptr->description;
    pack_description_text->start_juice_animation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
    );
    
    pack_tags_text->text =
        (pack_ptr->tags.empty() ? "" : "Tags: " + pack_ptr->tags);
    pack_tags_text->start_juice_animation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    pack_maker_text->text =
        (pack_ptr->maker.empty() ? "" : "Maker: " + pack_ptr->maker);
    pack_maker_text->start_juice_animation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    pack_version_text->text =
        (pack_ptr->version.empty() ? "" : "Version: " + pack_ptr->version);
    pack_version_text->start_juice_animation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
}


/**
 * @brief Initializes the main GUI.
 */
void PacksMenu::init_gui_main() {
    //Menu items.
    gui.register_coords("back",               12,    5, 20,  6);
    gui.register_coords("back_input",          3,    7,  4,  4);
    gui.register_coords("header",             61,    5, 74,  6);
    gui.register_coords("list",               26,   47, 48, 74);
    gui.register_coords("list_scroll",        52,   47,  2, 74);
    gui.register_coords("info_box",           76,   47, 44, 74);
    gui.register_coords("pack_name",        67.5,   19, 25, 16);
    gui.register_coords("pack_thumbnail",     89,   19, 16, 16);
    gui.register_coords("pack_description",   76, 48.5, 42, 41);
    gui.register_coords("pack_tags",          76,   73, 42,  6);
    gui.register_coords("pack_maker",         65,   80, 20,  6);
    gui.register_coords("pack_version",       87,   80, 20,  6);
    gui.register_coords("restart_warning",  35.5, 88.5, 67,  5);
    gui.register_coords("open_folder",        84, 88.5, 28,  5);
    gui.register_coords("tooltip",            50,   96, 96,  4);
    gui.read_coords(
        game.content.gui_defs.list[PACKS_MENU::GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    gui.back_item->on_activate =
    [this] (const Point &) {
        game.options.pack_order = pack_order;
        game.options.packs_disabled = packs_disabled;
        save_options();
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    gui.add_item(gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "PACKS",
        game.sys_content.fnt_area_name,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.add_item(header_text, "header");
    
    //Packs list.
    packs_list = new ListGuiItem();
    gui.add_item(packs_list, "list");
    
    const float ITEM_HEIGHT = 0.08f;
    const float ITEM_PADDING = 0.02f;
    const float ITEMS_OFFSET = 0.01f;
    
    //Base pack's bullet.
    BulletGuiItem* base_bullet =
        new BulletGuiItem(
        "Base", game.sys_content.fnt_standard, COLOR_GOLD
    );
    base_bullet->ratio_center =
        Point(0.37f, ITEMS_OFFSET + ITEM_HEIGHT / 2.0f);
    base_bullet->ratio_size =
        Point(0.70f, ITEM_HEIGHT);
    base_bullet->on_selected =
    [this] () { change_info(-1); };
    packs_list->add_child(base_bullet);
    gui.add_item(base_bullet);
    
    for(size_t p = 0; p < pack_order.size(); p++) {
        float list_bottom_y = packs_list->get_child_bottom();
        float row_center_y = list_bottom_y + ITEM_PADDING + ITEM_HEIGHT / 2.0f;
        
        //Pack bullet.
        BulletGuiItem* bullet =
            new BulletGuiItem(
            "",
            game.sys_content.fnt_standard
        );
        bullet->ratio_center = Point(0.37f, row_center_y);
        bullet->ratio_size = Point(0.70f, ITEM_HEIGHT);
        bullet->on_selected = [p, this] () { change_info((int) p); };
        packs_list->add_child(bullet);
        gui.add_item(bullet);
        pack_bullets.push_back(bullet);
        
        //Enable/disable checkbox.
        CheckGuiItem* check =
            new CheckGuiItem(
            false, "", game.sys_content.fnt_standard
        );
        check->ratio_center = Point(0.78f, row_center_y);
        check->ratio_size = Point(0.08f, ITEM_HEIGHT);
        check->on_activate =
        [p, check, this] (const Point & pos) {
            check->def_activate_code();
            if(check->value) {
                packs_disabled.erase(
                    std::find(
                        packs_disabled.begin(), packs_disabled.end(),
                        pack_order[p]
                    )
                );
            } else {
                packs_disabled.push_back(pack_order[p]);
            }
            trigger_restart_warning();
        };
        check->on_selected = [p, this] () { change_info((int) p); };
        check->on_get_tooltip =
        [] () {
            return "Enable or disable this pack.";
        };
        packs_list->add_child(check);
        gui.add_item(check);
        pack_checks.push_back(check);
        
        //Move up button.
        if(p > 0) {
            ButtonGuiItem* up_button =
                new ButtonGuiItem(
                "U", game.sys_content.fnt_standard
            );
            up_button->ratio_center = Point(0.87f, row_center_y);
            up_button->ratio_size = Point(0.08f, ITEM_HEIGHT);
            up_button->on_activate =
            [p, this] (const Point &) {
                std::iter_swap(
                    pack_order.begin() + p, pack_order.begin() + (p - 1)
                );
                pack_bullets[p]->start_juice_animation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                pack_bullets[p - 1]->start_juice_animation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                trigger_restart_warning();
                populate_packs_list();
            };
            up_button->on_selected = [p, this] () { change_info((int) p); };
            up_button->on_get_tooltip =
            [] () {
                return "Move up on the list (make it be loaded earlier).";
            };
            packs_list->add_child(up_button);
            gui.add_item(up_button);
        }
        
        //Move down button.
        if(p < pack_order.size() - 1) {
            ButtonGuiItem* down_button =
                new ButtonGuiItem(
                "D", game.sys_content.fnt_standard
            );
            down_button->ratio_center = Point(0.95f, row_center_y);
            down_button->ratio_size = Point(0.08f, ITEM_HEIGHT);
            down_button->on_activate =
            [p, this] (const Point &) {
                std::iter_swap(
                    pack_order.begin() + p, pack_order.begin() + (p + 1)
                );
                pack_bullets[p]->start_juice_animation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                pack_bullets[p + 1]->start_juice_animation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                trigger_restart_warning();
                populate_packs_list();
            };
            down_button->on_selected = [p, this] () { change_info((int) p); };
            down_button->on_get_tooltip =
            [] () {
                return "Move down on the list (make it be loaded later).";
            };
            packs_list->add_child(down_button);
            gui.add_item(down_button);
        }
    }
    
    //Packs list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->list_item = packs_list;
    gui.add_item(list_scroll, "list_scroll");
    
    //Info box item.
    GuiItem* info_box = new GuiItem();
    info_box->on_draw =
    [] (const DrawInfo & draw) {
        draw_textured_box(
            draw.center, draw.size, game.sys_content.bmp_frame_box,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.add_item(info_box, "info_box");
    
    //Pack name text.
    pack_name_text =
        new TextGuiItem(
        "", game.sys_content.fnt_area_name, COLOR_GOLD, ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pack_name_text, "pack_name");
    
    //Pack thumbnail.
    GuiItem* pack_thumb_item = new GuiItem();
    pack_thumb_item->on_draw =
    [this] (const DrawInfo & draw) {
        //Make it a square.
        Point final_size(
            std::min(draw.size.x, draw.size.y),
            std::min(draw.size.x, draw.size.y)
        );
        //Align it to the top-right corner.
        Point final_center(
            (draw.center.x + draw.size.x / 2.0f) - final_size.x / 2.0f,
            (draw.center.y - draw.size.y / 2.0f) + final_size.y / 2.0f
        );
        if(!cur_pack_name.empty() && pack_thumbs[cur_pack_name]) {
            draw_bitmap(
                pack_thumbs[cur_pack_name],
                final_center, final_size - 4.0f
            );
        }
        draw_textured_box(
            final_center, final_size, game.sys_content.bmp_frame_box,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.add_item(pack_thumb_item, "pack_thumbnail");
    
    //Pack description text.
    pack_description_text =
        new TextGuiItem(
        "", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    pack_description_text->line_wrap = true;
    gui.add_item(pack_description_text, "pack_description");
    
    //Pack tags text.
    pack_tags_text =
        new TextGuiItem(
        "", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pack_tags_text, "pack_tags");
    
    //Pack maker text.
    pack_maker_text =
        new TextGuiItem(
        "", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pack_maker_text, "pack_maker");
    
    //Pack version text.
    pack_version_text =
        new TextGuiItem(
        "", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    gui.add_item(pack_version_text, "pack_version");
    
    //Restart warning text.
    warning_text =
        new TextGuiItem(
        "You may need to restart for some of the changes to take effect.",
        game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    warning_text->visible = false;
    gui.add_item(warning_text, "restart_warning");
    
    //Open folder button.
    ButtonGuiItem* open_folder_button =
        new ButtonGuiItem("Open folder", game.sys_content.fnt_standard);
    open_folder_button->on_activate =
    [this] (const Point &) {
        open_file_explorer(FOLDER_PATHS_FROM_ROOT::GAME_DATA);
    };
    open_folder_button->on_get_tooltip =
    [] () {
        return
            "Opens the packs folder on your operative system. "
            "Place downloaded packs here!";
    };
    gui.add_item(open_folder_button, "open_folder");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    populate_packs_list();
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item, true);
    change_info(-1);
}


/**
 * @brief Loads the menu.
 */
void PacksMenu::load() {
    //Fill the menu's lists of packs.
    pack_order =
        sort_vector_with_preference_list(
            game.content.packs.manifests_sans_base_raw,
            game.options.pack_order
        );
    packs_disabled = game.options.packs_disabled;
    
    //Get the thumbnails.
    for(
        size_t p = 0; p < game.content.packs.manifests_with_base_raw.size(); p++
    ) {
        string pack = game.content.packs.manifests_with_base_raw[p];
        string thumb_path =
            FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + pack + "/thumbnail.png";
        ALLEGRO_BITMAP* thumb_bmp =
            load_bmp(thumb_path, nullptr, true, false, false);
        pack_thumbs[pack] = thumb_bmp;
    }
    
    //Initialize the GUIs.
    init_gui_main();
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}


/**
 * @brief Populates the packs list with rows for each pack.
 */
void PacksMenu::populate_packs_list() {
    for(size_t p = 0; p < pack_order.size(); p++) {
        pack_bullets[p]->text =
            game.content.packs.list[pack_order[p]].name;
        pack_checks[p]->value =
            std::find(
                packs_disabled.begin(), packs_disabled.end(), pack_order[p]
            ) == packs_disabled.end();
    }
}


/**
 * @brief Triggers the restart warning at the bottom of the screen.
 */
void PacksMenu::trigger_restart_warning() {
    if(!warning_text->visible) {
        warning_text->visible = true;
        warning_text->start_juice_animation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the menu.
 */
void PacksMenu::unload() {
    for(auto &t : pack_thumbs) {
        al_destroy_bitmap(t.second);
    }
    pack_thumbs.clear();
    
    Menu::unload();
}
