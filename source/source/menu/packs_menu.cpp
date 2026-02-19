
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

#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../util/os_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace PACKS_MENU {

//Name of the pack management menu GUI definition file.
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
    Pack* packPtr = nullptr;
    string newPackName;
    if(idx == -1) {
        newPackName = FOLDER_NAMES::BASE_PACK;
        packPtr = &game.content.packs.list[newPackName];
    } else if(idx >= 0 && idx < (int) packOrder.size()) {
        newPackName = packOrder[idx];
        packPtr = &game.content.packs.list[newPackName];
    }
    
    if(curPackName == newPackName) {
        return;
    }
    
    curPackName = newPackName;
    if(!packPtr) {
        packNameText->text.clear();
        packDescriptionText->text.clear();
        packTagsText->text.clear();
        packMakerText->text.clear();
        packVersionText->text.clear();
        return;
    }
    
    //Fill the GUI items.
    packNameText->text =
        packPtr->name;
    packNameText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packDescriptionText->text =
        packPtr->description;
    packDescriptionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
    );
    
    packTagsText->text =
        (packPtr->tags.empty() ? "" : "Tags: " + packPtr->tags);
    packTagsText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packMakerText->text =
        (packPtr->maker.empty() ? "" : "Maker: " + packPtr->maker);
    packMakerText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    
    packVersionText->text =
        (packPtr->version.empty() ? "" : "Version: " + packPtr->version);
    packVersionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
}


/**
 * @brief Initializes the main GUI.
 */
void PacksMenu::initGuiMain() {
    //Menu items.
    DataNode* guiFile = &game.content.guiDefs.list[PACKS_MENU::GUI_FILE_NAME];
    gui.registerCoords("back",               12,    5, 20,  6);
    gui.registerCoords("back_input",          3,    7,  4,  4);
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
    gui.readDataFile(guiFile);
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard, game.config.guiColors.back
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        game.options.packs.order = packOrder;
        game.options.packs.disabled = packsDisabled;
        saveOptions();
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiCreateBackInputIcon(&gui);
    
    //Packs list.
    packsList = new ListGuiItem();
    gui.addItem(packsList, "list");
    
    const float ITEM_HEIGHT = 0.08f;
    const float ITEM_PADDING = 0.02f;
    const float ITEMS_OFFSET = 0.01f;
    
    //Base pack's bullet.
    BulletGuiItem* baseBullet =
        new BulletGuiItem(
        "Base", game.sysContent.fntStandard, game.config.guiColors.gold
    );
    baseBullet->ratioCenter =
        Point(0.37f, ITEMS_OFFSET + ITEM_HEIGHT / 2.0f);
    baseBullet->ratioSize =
        Point(0.70f, ITEM_HEIGHT);
    baseBullet->onFocused =
    [this] () { changeInfo(-1); };
    packsList->addChild(baseBullet);
    gui.addItem(baseBullet);
    
    for(size_t p = 0; p < packOrder.size(); p++) {
        float listBottomY = packsList->getChildrenSpan();
        float rowCenterY = listBottomY + ITEM_PADDING + ITEM_HEIGHT / 2.0f;
        
        //Pack bullet.
        BulletGuiItem* bullet =
            new BulletGuiItem(
            "",
            game.sysContent.fntStandard
        );
        bullet->ratioCenter = Point(0.37f, rowCenterY);
        bullet->ratioSize = Point(0.70f, ITEM_HEIGHT);
        bullet->onFocused = [p, this] () { changeInfo((int) p); };
        packsList->addChild(bullet);
        gui.addItem(bullet);
        packBullets.push_back(bullet);
        
        //Enable/disable checkbox.
        CheckGuiItem* check =
            new CheckGuiItem(
            false, "", game.sysContent.fntStandard
        );
        check->forceSquare = true;
        check->ratioCenter = Point(0.78f, rowCenterY);
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
        check->onFocused = [p, this] () { changeInfo((int) p); };
        check->onGetTooltip =
        [] () {
            return "Enable or disable this pack.";
        };
        packsList->addChild(check);
        gui.addItem(check);
        packChecks.push_back(check);
        
        //Move up button.
        if(p > 0) {
            ButtonGuiItem* upButton =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            upButton->forceSquare = true;
            upButton->ratioCenter = Point(0.87f, rowCenterY);
            upButton->ratioSize = Point(0.08f, ITEM_HEIGHT) * 0.80f;
            upButton->onDraw =
            [upButton] (const DrawInfo & draw) {
                upButton->defDrawCode(draw);
                drawBitmapInBox(
                    game.sysContent.bmpArrowUp, draw.center,
                    draw.size * 0.80f, true, 0.0f, draw.tint
                );
            };
            upButton->onActivate =
            [p, this] (const Point&) {
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
            upButton->onFocused = [p, this] () { changeInfo((int) p); };
            upButton->onGetTooltip =
            [] () {
                return "Move up on the list (make it be loaded earlier).";
            };
            packsList->addChild(upButton);
            gui.addItem(upButton);
        }
        
        //Move down button.
        if(p < packOrder.size() - 1) {
            ButtonGuiItem* downButton =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            downButton->forceSquare = true;
            downButton->ratioCenter = Point(0.95f, rowCenterY);
            downButton->ratioSize = Point(0.08f, ITEM_HEIGHT) * 0.80f;
            downButton->onDraw =
            [downButton] (const DrawInfo & draw) {
                downButton->defDrawCode(draw);
                drawBitmapInBox(
                    game.sysContent.bmpArrowDown, draw.center,
                    draw.size * 0.80f, true, 0.0f, draw.tint
                );
            };
            downButton->onActivate =
            [p, this] (const Point&) {
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
            downButton->onFocused = [p, this] () { changeInfo((int) p); };
            downButton->onGetTooltip =
            [] () {
                return "Move down on the list (make it be loaded later).";
            };
            packsList->addChild(downButton);
            gui.addItem(downButton);
        }
    }
    
    //Packs list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->listItem = packsList;
    gui.addItem(listScroll, "list_scroll");
    
    //Info box item.
    GuiItem* infoBox = new GuiItem();
    infoBox->onDraw =
    [] (const DrawInfo & draw) {
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
        );
    };
    gui.addItem(infoBox, "info_box");
    
    //Pack name text.
    packNameText =
        new TextGuiItem(
        "", game.sysContent.fntAreaName,
        game.config.guiColors.gold, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(packNameText, "pack_name");
    
    //Pack thumbnail.
    GuiItem* packThumbItem = new GuiItem();
    packThumbItem->forceSquare = true;
    packThumbItem->onDraw =
    [this] (const DrawInfo & draw) {
        //Align it to the top-right corner.
        Point finalCenter(
            (draw.center.x + draw.size.x / 2.0f) - draw.size.x / 2.0f,
            (draw.center.y - draw.size.y / 2.0f) + draw.size.y / 2.0f
        );
        if(!curPackName.empty() && packThumbs[curPackName]) {
            drawBitmap(
                packThumbs[curPackName],
                finalCenter, draw.size - 4.0f, 0.0f, draw.tint
            );
        }
        drawTexturedBox(
            finalCenter, draw.size, game.sysContent.bmpFrameBox,
            tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
        );
    };
    gui.addItem(packThumbItem, "pack_thumbnail");
    
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
        "You may need to leave this menu and then restart for some of "
        "the changes to take effect.",
        game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    warningText->visible = false;
    gui.addItem(warningText, "restart_warning");
    
    //Open folder button.
    ButtonGuiItem* openFolderButton =
        new ButtonGuiItem("Open folder", game.sysContent.fntStandard);
    openFolderButton->onActivate =
    [this] (const Point&) {
        openFileExplorer(FOLDER_PATHS_FROM_ROOT::GAME_DATA);
    };
    openFolderButton->onGetTooltip =
    [] () {
        return
            "Opens the packs folder on your operative system. "
            "Place downloaded pack folders here!";
    };
    gui.addItem(openFolderButton, "open_folder");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    populatePacksList();
    
    //Finishing touches.
    gui.setFocusedItem(gui.backItem, true);
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
        string thumbPath =
            FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + pack + "/thumbnail.png";
        ALLEGRO_BITMAP* thumbBmp =
            loadBmp(thumbPath, nullptr, false, false, false);
        packThumbs[pack] = thumbBmp;
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
 * @brief Triggers the restart warning.
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
    for(auto& t : packThumbs) {
        al_destroy_bitmap(t.second);
    }
    packThumbs.clear();
    
    Menu::unload();
}
