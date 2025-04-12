
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
void PacksMenu::changeInfo(int idx) {
    //Figure out what pack this is.
    Pack* pack_ptr = nullptr;
    string new_pack_name;
    if(idx == -1) {
        new_pack_name = FOLDER_NAMES::BASE_PACK;
        pack_ptr = &game.content.packs.list[new_pack_name];
    } else if(idx >= 0 && idx < (int) packOrder.size()) {
        new_pack_name = packOrder[idx];
        pack_ptr = &game.content.packs.list[new_pack_name];
    }
    
    if(curPackName == new_pack_name) {
        return;
    }
    
    curPackName = new_pack_name;
    if(!pack_ptr) {
        packNameText->text.clear();
        packDescriptionText->text.clear();
        packTagsText->text.clear();
        packMakerText->text.clear();
        packVersionText->text.clear();
        return;
    }
    
    //Fill the GUI items.
    packNameText->text =
        pack_ptr->name;
    packNameText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packDescriptionText->text =
        pack_ptr->description;
    packDescriptionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
    );
    
    packTagsText->text =
        (pack_ptr->tags.empty() ? "" : "Tags: " + pack_ptr->tags);
    packTagsText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packMakerText->text =
        (pack_ptr->maker.empty() ? "" : "Maker: " + pack_ptr->maker);
    packMakerText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packVersionText->text =
        (pack_ptr->version.empty() ? "" : "Version: " + pack_ptr->version);
    packVersionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
}


/**
 * @brief Initializes the main GUI.
 */
void PacksMenu::initGuiMain() {
    //Menu items.
    gui.registerCoords("back",               12,    5, 20,  6);
    gui.registerCoords("back_input",          3,    7,  4,  4);
    gui.registerCoords("header",             61,    5, 74,  6);
    gui.registerCoords("list",               26,   47, 48, 74);
    gui.registerCoords("list_scroll",        52,   47,  2, 74);
    gui.registerCoords("info_box",           76,   47, 44, 74);
    gui.registerCoords("pack_name",        67.5,   19, 25, 16);
    gui.registerCoords("pack_thumbnail",     89,   19, 16, 16);
    gui.registerCoords("pack_description",   76, 48.5, 42, 41);
    gui.registerCoords("pack_tags",          76,   73, 42,  6);
    gui.registerCoords("pack_maker",         65,   80, 20,  6);
    gui.registerCoords("pack_version",       87,   80, 20,  6);
    gui.registerCoords("restart_warning",  35.5, 88.5, 67,  5);
    gui.registerCoords("open_folder",        84, 88.5, 28,  5);
    gui.registerCoords("tooltip",            50,   96, 96,  4);
    gui.readCoords(
        game.content.guiDefs.list[PACKS_MENU::GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    gui.backItem->onActivate =
    [this] (const Point &) {
        game.options.packs.order = packOrder;
        game.options.packs.disabled = packsDisabled;
        saveOptions();
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "PACKS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.addItem(header_text, "header");
    
    //Packs list.
    packsList = new ListGuiItem();
    gui.addItem(packsList, "list");
    
    const float ITEM_HEIGHT = 0.08f;
    const float ITEM_PADDING = 0.02f;
    const float ITEMS_OFFSET = 0.01f;
    
    //Base pack's bullet.
    BulletGuiItem* base_bullet =
        new BulletGuiItem(
        "Base", game.sysContent.fntStandard, COLOR_GOLD
    );
    base_bullet->ratioCenter =
        Point(0.37f, ITEMS_OFFSET + ITEM_HEIGHT / 2.0f);
    base_bullet->ratioSize =
        Point(0.70f, ITEM_HEIGHT);
    base_bullet->onSelected =
    [this] () { changeInfo(-1); };
    packsList->addChild(base_bullet);
    gui.addItem(base_bullet);
    
    for(size_t p = 0; p < packOrder.size(); p++) {
        float list_bottom_y = packsList->getChildBottom();
        float row_center_y = list_bottom_y + ITEM_PADDING + ITEM_HEIGHT / 2.0f;
        
        //Pack bullet.
        BulletGuiItem* bullet =
            new BulletGuiItem(
            "",
            game.sysContent.fntStandard
        );
        bullet->ratioCenter = Point(0.37f, row_center_y);
        bullet->ratioSize = Point(0.70f, ITEM_HEIGHT);
        bullet->onSelected = [p, this] () { changeInfo((int) p); };
        packsList->addChild(bullet);
        gui.addItem(bullet);
        packBullets.push_back(bullet);
        
        //Enable/disable checkbox.
        CheckGuiItem* check =
            new CheckGuiItem(
            false, "", game.sysContent.fntStandard
        );
        check->ratioCenter = Point(0.78f, row_center_y);
        check->ratioSize = Point(0.08f, ITEM_HEIGHT);
        check->onActivate =
        [p, check, this] (const Point & pos) {
            check->defActivateCode();
            if(check->value) {
                packsDisabled.erase(
                    std::find(
                        packsDisabled.begin(), packsDisabled.end(),
                        packOrder[p]
                    )
                );
            } else {
                packsDisabled.push_back(packOrder[p]);
            }
            triggerRestartWarning();
        };
        check->onSelected = [p, this] () { changeInfo((int) p); };
        check->onGetTooltip =
        [] () {
            return "Enable or disable this pack.";
        };
        packsList->addChild(check);
        gui.addItem(check);
        packChecks.push_back(check);
        
        //Move up button.
        if(p > 0) {
            ButtonGuiItem* up_button =
                new ButtonGuiItem(
                "U", game.sysContent.fntStandard
            );
            up_button->ratioCenter = Point(0.87f, row_center_y);
            up_button->ratioSize = Point(0.08f, ITEM_HEIGHT);
            up_button->onActivate =
            [p, this] (const Point &) {
                std::iter_swap(
                    packOrder.begin() + p, packOrder.begin() + (p - 1)
                );
                packBullets[p]->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                packBullets[p - 1]->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                triggerRestartWarning();
                populatePacksList();
            };
            up_button->onSelected = [p, this] () { changeInfo((int) p); };
            up_button->onGetTooltip =
            [] () {
                return "Move up on the list (make it be loaded earlier).";
            };
            packsList->addChild(up_button);
            gui.addItem(up_button);
        }
        
        //Move down button.
        if(p < packOrder.size() - 1) {
            ButtonGuiItem* down_button =
                new ButtonGuiItem(
                "D", game.sysContent.fntStandard
            );
            down_button->ratioCenter = Point(0.95f, row_center_y);
            down_button->ratioSize = Point(0.08f, ITEM_HEIGHT);
            down_button->onActivate =
            [p, this] (const Point &) {
                std::iter_swap(
                    packOrder.begin() + p, packOrder.begin() + (p + 1)
                );
                packBullets[p]->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                packBullets[p + 1]->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                triggerRestartWarning();
                populatePacksList();
            };
            down_button->onSelected = [p, this] () { changeInfo((int) p); };
            down_button->onGetTooltip =
            [] () {
                return "Move down on the list (make it be loaded later).";
            };
            packsList->addChild(down_button);
            gui.addItem(down_button);
        }
    }
    
    //Packs list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->listItem = packsList;
    gui.addItem(list_scroll, "list_scroll");
    
    //Info box item.
    GuiItem* info_box = new GuiItem();
    info_box->onDraw =
    [] (const DrawInfo & draw) {
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.addItem(info_box, "info_box");
    
    //Pack name text.
    packNameText =
        new TextGuiItem(
        "", game.sysContent.fntAreaName, COLOR_GOLD, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(packNameText, "pack_name");
    
    //Pack thumbnail.
    GuiItem* pack_thumb_item = new GuiItem();
    pack_thumb_item->onDraw =
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
        if(!curPackName.empty() && packThumbs[curPackName]) {
            drawBitmap(
                packThumbs[curPackName],
                final_center, final_size - 4.0f
            );
        }
        drawTexturedBox(
            final_center, final_size, game.sysContent.bmpFrameBox,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.addItem(pack_thumb_item, "pack_thumbnail");
    
    //Pack description text.
    packDescriptionText =
        new TextGuiItem(
        "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    packDescriptionText->lineWrap = true;
    gui.addItem(packDescriptionText, "pack_description");
    
    //Pack tags text.
    packTagsText =
        new TextGuiItem(
        "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(packTagsText, "pack_tags");
    
    //Pack maker text.
    packMakerText =
        new TextGuiItem(
        "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(packMakerText, "pack_maker");
    
    //Pack version text.
    packVersionText =
        new TextGuiItem(
        "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    gui.addItem(packVersionText, "pack_version");
    
    //Restart warning text.
    warningText =
        new TextGuiItem(
        "You may need to restart for some of the changes to take effect.",
        game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    warningText->visible = false;
    gui.addItem(warningText, "restart_warning");
    
    //Open folder button.
    ButtonGuiItem* open_folder_button =
        new ButtonGuiItem("Open folder", game.sysContent.fntStandard);
    open_folder_button->onActivate =
    [this] (const Point &) {
        openFileExplorer(FOLDER_PATHS_FROM_ROOT::GAME_DATA);
    };
    open_folder_button->onGetTooltip =
    [] () {
        return
            "Opens the packs folder on your operative system. "
            "Place downloaded packs here!";
    };
    gui.addItem(open_folder_button, "open_folder");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltip_text, "tooltip");
    
    populatePacksList();
    
    //Finishing touches.
    gui.setSelectedItem(gui.backItem, true);
    changeInfo(-1);
}


/**
 * @brief Loads the menu.
 */
void PacksMenu::load() {
    //Fill the menu's lists of packs.
    packOrder =
        sortVectorWithPreferenceList(
            game.content.packs.manifestsSansBaseRaw,
            game.options.packs.order
        );
    packsDisabled = game.options.packs.disabled;
    
    //Get the thumbnails.
    for(
        size_t p = 0; p < game.content.packs.manifestsWithBaseRaw.size(); p++
    ) {
        string pack = game.content.packs.manifestsWithBaseRaw[p];
        string thumb_path =
            FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + pack + "/thumbnail.png";
        ALLEGRO_BITMAP* thumb_bmp =
            loadBmp(thumb_path, nullptr, true, false, false);
        packThumbs[pack] = thumb_bmp;
    }
    
    //Initialize the GUIs.
    initGuiMain();
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}


/**
 * @brief Populates the packs list with rows for each pack.
 */
void PacksMenu::populatePacksList() {
    for(size_t p = 0; p < packOrder.size(); p++) {
        packBullets[p]->text =
            game.content.packs.list[packOrder[p]].name;
        packChecks[p]->value = !isInContainer(packsDisabled, packOrder[p]);
    }
}


/**
 * @brief Triggers the restart warning at the bottom of the screen.
 */
void PacksMenu::triggerRestartWarning() {
    if(!warningText->visible) {
        warningText->visible = true;
        warningText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the menu.
 */
void PacksMenu::unload() {
    for(auto &t : packThumbs) {
        al_destroy_bitmap(t.second);
    }
    packThumbs.clear();
    
    Menu::unload();
}
