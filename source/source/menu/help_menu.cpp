/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Help menu structs and functions.
 */

#include "help_menu.h"

#include "../core/game.h"
#include "../core/misc_functions.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace HELP_MENU {

//Name of the help menu GUI information file.
const string GUI_FILE_NAME = "help";

}


/**
 * @brief Draws some help tidbit's text.
 *
 * @param font Font to use.
 * @param where Coordinates to draw the text on.
 * @param maxSize Maximum width or height the text can occupy.
 * A value of zero in one of these coordinates makes it not have a
 * limit in that dimension.
 * @param text Text to draw.
 */
void HelpMenu::drawTidbit(
    const ALLEGRO_FONT* const font, const Point& where,
    const Point& maxSize, const string& text
) {
    //Get the tokens that make up the tidbit.
    vector<StringToken> tokens = tokenizeString(text);
    if(tokens.empty()) return;
    
    int lineHeight = al_get_font_line_height(font);
    
    setStringTokenWidths(
        tokens, font, game.sysContent.fntSlim, lineHeight, true
    );
    
    //Split long lines.
    vector<vector<StringToken> > tokensPerLine =
        splitLongStringWithTokens(tokens, maxSize.x);
        
    if(tokensPerLine.empty()) return;
    
    //Figure out if we need to scale things vertically.
    //Control bind icons that are bitmaps will have their width unchanged,
    //otherwise this would turn into a cat-and-mouse game of the Y scale
    //shrinking causing a token width to shrink, which could cause the
    //Y scale to grow, ad infinitum.
    float yScale = 1.0f;
    if(tokensPerLine.size() * lineHeight > maxSize.y) {
        yScale = maxSize.y / (tokensPerLine.size() * (lineHeight + 4));
    }
    
    //Draw!
    for(size_t l = 0; l < tokensPerLine.size(); l++) {
        drawStringTokens(
            tokensPerLine[l],
            game.sysContent.fntStandard, game.sysContent.fntSlim,
            true,
            Point(
                where.x,
                where.y + l * (lineHeight + 4) * yScale -
                (tokensPerLine.size() * lineHeight * yScale / 2.0f)
            ),
            ALLEGRO_ALIGN_CENTER, Point(maxSize.x, lineHeight * yScale)
        );
    }
}


/**
 * @brief Initializes the main GUI.
 *
 * @param guiFile The GUI definition's file.
 */
void HelpMenu::initGuiMain(DataNode* guiFile) {
    //Menu items.
    gui.registerCoords("back",        12,  5, 20,  6);
    gui.registerCoords("back_input",   3,  7,  4,  4);
    gui.registerCoords("gameplay1",   22, 15, 36,  6);
    gui.registerCoords("gameplay2",   22, 23, 36,  6);
    gui.registerCoords("controls",    22, 31, 36,  6);
    gui.registerCoords("pikmin",      22, 39, 36,  6);
    gui.registerCoords("objects",     22, 47, 36,  6);
    gui.registerCoords("manual",      22, 54, 36,  4);
    gui.registerCoords("category",    71,  5, 54,  6);
    gui.registerCoords("list",        69, 39, 50, 54);
    gui.registerCoords("list_scroll", 96, 39,  2, 54);
    gui.registerCoords("image",       16, 83, 28, 30);
    gui.registerCoords("tooltip",     65, 83, 66, 30);
    gui.readCoords(guiFile->getChildByName("positions"));
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Gameplay basics button.
    ButtonGuiItem* gameplay1Button =
        new ButtonGuiItem("Gameplay basics", game.sysContent.fntStandard);
    gameplay1Button->onActivate =
    [this] (const Point&) {
        populateTidbits(HELP_CATEGORY_GAMEPLAY1);
    };
    gameplay1Button->onGetTooltip =
    [] () {
        return "Show help about basic gameplay features.";
    };
    gui.addItem(gameplay1Button, "gameplay1");
    
    //Gameplay advanced button.
    ButtonGuiItem* gameplay2Button =
        new ButtonGuiItem("Advanced gameplay", game.sysContent.fntStandard);
    gameplay2Button->onActivate =
    [this] (const Point&) {
        populateTidbits(HELP_CATEGORY_GAMEPLAY2);
    };
    gameplay2Button->onGetTooltip =
    [] () {
        return "Show advanced gameplay tips.";
    };
    gui.addItem(gameplay2Button, "gameplay2");
    
    //Controls button.
    ButtonGuiItem* controlsButton =
        new ButtonGuiItem("Controls", game.sysContent.fntStandard);
    controlsButton->onActivate =
    [this] (const Point&) {
        populateTidbits(HELP_CATEGORY_CONTROLS);
    };
    controlsButton->onGetTooltip =
    [] () {
        return "Show game controls and certain actions you can perform.";
    };
    gui.addItem(controlsButton, "controls");
    
    //Pikmin button.
    ButtonGuiItem* pikminButton =
        new ButtonGuiItem("Pikmin types", game.sysContent.fntStandard);
    pikminButton->onActivate =
    [this] (const Point&) {
        populateTidbits(HELP_CATEGORY_PIKMIN);
    };
    pikminButton->onGetTooltip =
    [] () {
        return "Show a description of each Pikmin type.";
    };
    gui.addItem(pikminButton, "pikmin");
    
    //Objects button.
    ButtonGuiItem* objectsButton =
        new ButtonGuiItem("Objects", game.sysContent.fntStandard);
    objectsButton->onActivate =
    [this] (const Point&) {
        populateTidbits(HELP_CATEGORY_OBJECTS);
    };
    objectsButton->onGetTooltip =
    [] () {
        return "Show help about some noteworthy objects you'll find.";
    };
    gui.addItem(objectsButton, "objects");
    
    //Manual text.
    BulletGuiItem* manualBullet =
        new BulletGuiItem("More help...", game.sysContent.fntStandard);
    manualBullet->onActivate =
    [] (const Point&) {
        openManual("home.html");
    };
    manualBullet->onGetTooltip = [] () {
        return
            "Click to open the manual (in the game's folder) for more help.";
    };
    gui.addItem(manualBullet, "manual");
    
    //Category text.
    categoryText = new TextGuiItem("Help", game.sysContent.fntStandard);
    gui.addItem(categoryText, "category");
    
    //Tidbit list box.
    tidbitList = new ListGuiItem();
    gui.addItem(tidbitList, "list");
    
    //Tidbit list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->listItem = tidbitList;
    gui.addItem(listScroll, "list_scroll");
    
    //Image item.
    GuiItem* imageItem = new GuiItem();
    imageItem->onDraw =
    [this] (const DrawInfo & draw) {
        if(curTidbit == nullptr) return;
        if(curTidbit->image == nullptr) return;
        drawBitmapInBox(
            curTidbit->image,
            draw.center, draw.size, false
        );
    };
    gui.addItem(imageItem, "image");
    
    //Tooltip text.
    TextGuiItem* tooltipText =
        new TextGuiItem("", game.sysContent.fntStandard);
    tooltipText->onDraw =
        [this]
    (const DrawInfo & draw) {
        drawTidbit(
            game.sysContent.fntStandard, draw.center, draw.size,
            gui.getCurrentTooltip()
        );
    };
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    gui.setSelectedItem(gui.backItem, true);
    gui.onSelectionChanged =
    [this] () {
        curTidbit = nullptr;
    };
}


/**
 * @brief Loads the menu.
 */
void HelpMenu::load() {
    //Initial setup.
    const vector<string> categoryNodeNames {
        "gameplay_basics", "advanced_gameplay", "controls", "", "objects"
    };
    DataNode* guiFile = &game.content.guiDefs.list[HELP_MENU::GUI_FILE_NAME];
    
    //Load the tidbits.
    DataNode* tidbitsNode = guiFile->getChildByName("tidbits");
    
    for(size_t c = 0; c < N_HELP_CATEGORIES; c++) {
        if(categoryNodeNames[c].empty()) continue;
        DataNode* categoryNode =
            tidbitsNode->getChildByName(categoryNodeNames[c]);
        size_t nTidbits = categoryNode->getNrOfChildren();
        vector<Tidbit>& categoryTidbits = tidbits[(HELP_CATEGORY) c];
        categoryTidbits.reserve(nTidbits);
        for(size_t t = 0; t < nTidbits; t++) {
            vector<string> parts =
                semicolonListToVector(categoryNode->getChild(t)->name);
            Tidbit newT;
            newT.name = parts.size() > 0 ? parts[0] : "";
            newT.description = parts.size() > 1 ? parts[1] : "";
            newT.image =
                parts.size() > 2 ?
                game.content.bitmaps.list.get(parts[2]) :
                nullptr;
            categoryTidbits.push_back(newT);
        }
    }
    for(size_t p = 0; p < game.config.pikmin.order.size(); p++) {
        Tidbit newT;
        newT.name = game.config.pikmin.order[p]->name;
        newT.description = game.config.pikmin.order[p]->description;
        newT.image = game.config.pikmin.order[p]->bmpIcon;
        tidbits[HELP_CATEGORY_PIKMIN].push_back(newT);
    }
    
    //Initialize the GUIs.
    initGuiMain(guiFile);
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}


/**
 * @brief Populates the help menu's list of tidbits.
 *
 * @param category Category of tidbits to use.
 */
void HelpMenu::populateTidbits(const HELP_CATEGORY category) {
    vector<Tidbit>& categoryTidbits = tidbits[category];
    
    switch(category) {
    case HELP_CATEGORY_GAMEPLAY1: {
        categoryText->text = "Gameplay basics";
        break;
    } case HELP_CATEGORY_GAMEPLAY2: {
        categoryText->text = "Advanced gameplay";
        break;
    } case HELP_CATEGORY_CONTROLS: {
        categoryText->text = "Controls";
        break;
    } case HELP_CATEGORY_PIKMIN: {
        categoryText->text = "Pikmin";
        break;
    } case HELP_CATEGORY_OBJECTS: {
        categoryText->text = "Objects";
        break;
    } default: {
        categoryText->text = "Help";
        break;
    }
    }
    
    tidbitList->deleteAllChildren();
    
    for(size_t t = 0; t < categoryTidbits.size(); t++) {
        Tidbit* tPtr = &categoryTidbits[t];
        BulletGuiItem* tidbitBullet =
            new BulletGuiItem(
            tPtr->name,
            game.sysContent.fntStandard
        );
        tidbitBullet->ratioCenter = Point(0.50f, 0.045f + t * 0.10f);
        tidbitBullet->ratioSize = Point(1.0f, 0.09f);
        tidbitBullet->onGetTooltip = [this, tPtr] () {
            return tPtr->description;
        };
        tidbitBullet->onSelected = [this, tPtr] () {
            curTidbit = tPtr;
        };
        tidbitBullet->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
        );
        tidbitList->addChild(tidbitBullet);
        gui.addItem(tidbitBullet);
    }
    
    categoryText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
    );
}


/**
 * @brief Unloads the menu.
 */
void HelpMenu::unload() {
    for(size_t c = 0; c < N_HELP_CATEGORIES; c++) {
        if(c == HELP_CATEGORY_PIKMIN) continue;
        for(size_t t = 0; t < tidbits[(HELP_CATEGORY) c].size(); t++) {
            if(tidbits[(HELP_CATEGORY) c][t].image) {
                game.content.bitmaps.list.free(
                    tidbits[(HELP_CATEGORY) c][t].image
                );
            }
        }
    }
    tidbits.clear();
    
    Menu::unload();
}
