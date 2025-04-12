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

#include "../core/misc_functions.h"
#include "../core/game.h"
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
 * @param max_size Maximum width or height the text can occupy.
 * A value of zero in one of these coordinates makes it not have a
 * limit in that dimension.
 * @param text Text to draw.
 */
void HelpMenu::drawTidbit(
    const ALLEGRO_FONT* const font, const Point &where,
    const Point &max_size, const string &text
) {
    //Get the tokens that make up the tidbit.
    vector<StringToken> tokens = tokenizeString(text);
    if(tokens.empty()) return;
    
    int line_height = al_get_font_line_height(font);
    
    setStringTokenWidths(tokens, font, game.sysContent.fntSlim, line_height, true);
    
    //Split long lines.
    vector<vector<StringToken> > tokens_per_line =
        splitLongStringWithTokens(tokens, max_size.x);
        
    if(tokens_per_line.empty()) return;
    
    //Figure out if we need to scale things vertically.
    //Control bind icons that are bitmaps will have their width unchanged,
    //otherwise this would turn into a cat-and-mouse game of the Y scale
    //shrinking causing a token width to shrink, which could cause the
    //Y scale to grow, ad infinitum.
    float y_scale = 1.0f;
    if(tokens_per_line.size() * line_height > max_size.y) {
        y_scale = max_size.y / (tokens_per_line.size() * (line_height + 4));
    }
    
    //Draw!
    for(size_t l = 0; l < tokens_per_line.size(); l++) {
        drawStringTokens(
            tokens_per_line[l], game.sysContent.fntStandard, game.sysContent.fntSlim,
            true,
            Point(
                where.x,
                where.y + l * (line_height + 4) * y_scale -
                (tokens_per_line.size() * line_height * y_scale / 2.0f)
            ),
            ALLEGRO_ALIGN_CENTER, Point(max_size.x, line_height * y_scale)
        );
    }
}


/**
 * @brief Loads the menu.
 */
void HelpMenu::load() {
    //Initial setup.
    const vector<string> category_node_names {
        "gameplay_basics", "advanced_gameplay", "controls", "", "objects"
    };
    DataNode* gui_file = &game.content.guiDefs.list[HELP_MENU::GUI_FILE_NAME];
    
    //Load the tidbits.
    DataNode* tidbits_node = gui_file->getChildByName("tidbits");
    
    for(size_t c = 0; c < N_HELP_CATEGORIES; c++) {
        if(category_node_names[c].empty()) continue;
        DataNode* category_node =
            tidbits_node->getChildByName(category_node_names[c]);
        size_t n_tidbits = category_node->getNrOfChildren();
        vector<Tidbit> &category_tidbits = tidbits[(HELP_CATEGORY) c];
        category_tidbits.reserve(n_tidbits);
        for(size_t t = 0; t < n_tidbits; t++) {
            vector<string> parts =
                semicolonListToVector(category_node->getChild(t)->name);
            Tidbit new_t;
            new_t.name = parts.size() > 0 ? parts[0] : "";
            new_t.description = parts.size() > 1 ? parts[1] : "";
            new_t.image = parts.size() > 2 ? game.content.bitmaps.list.get(parts[2]) : nullptr;
            category_tidbits.push_back(new_t);
        }
    }
    for(size_t p = 0; p < game.config.pikmin.order.size(); p++) {
        Tidbit new_t;
        new_t.name = game.config.pikmin.order[p]->name;
        new_t.description = game.config.pikmin.order[p]->description;
        new_t.image = game.config.pikmin.order[p]->bmpIcon;
        tidbits[HELP_CATEGORY_PIKMIN].push_back(new_t);
    }
    
    //Initialize the GUIs.
    initGuiMain(gui_file);
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}


/**
 * @brief Initializes the main GUI.
 */
void HelpMenu::initGuiMain(DataNode* gui_file) {
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
    gui.readCoords(gui_file->getChildByName("positions"));
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point &) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Gameplay basics button.
    ButtonGuiItem* gameplay1_button =
        new ButtonGuiItem("Gameplay basics", game.sysContent.fntStandard);
    gameplay1_button->onActivate =
    [this] (const Point &) {
        populateTidbits(HELP_CATEGORY_GAMEPLAY1);
    };
    gameplay1_button->onGetTooltip =
    [] () {
        return "Show help about basic gameplay features.";
    };
    gui.addItem(gameplay1_button, "gameplay1");
    
    //Gameplay advanced button.
    ButtonGuiItem* gameplay2_button =
        new ButtonGuiItem("Advanced gameplay", game.sysContent.fntStandard);
    gameplay2_button->onActivate =
    [this] (const Point &) {
        populateTidbits(HELP_CATEGORY_GAMEPLAY2);
    };
    gameplay2_button->onGetTooltip =
    [] () {
        return "Show advanced gameplay tips.";
    };
    gui.addItem(gameplay2_button, "gameplay2");
    
    //Controls button.
    ButtonGuiItem* controls_button =
        new ButtonGuiItem("Controls", game.sysContent.fntStandard);
    controls_button->onActivate =
    [this] (const Point &) {
        populateTidbits(HELP_CATEGORY_CONTROLS);
    };
    controls_button->onGetTooltip =
    [] () {
        return "Show game controls and certain actions you can perform.";
    };
    gui.addItem(controls_button, "controls");
    
    //Pikmin button.
    ButtonGuiItem* pikmin_button =
        new ButtonGuiItem("Pikmin types", game.sysContent.fntStandard);
    pikmin_button->onActivate =
    [this] (const Point &) {
        populateTidbits(HELP_CATEGORY_PIKMIN);
    };
    pikmin_button->onGetTooltip =
    [] () {
        return "Show a description of each Pikmin type.";
    };
    gui.addItem(pikmin_button, "pikmin");
    
    //Objects button.
    ButtonGuiItem* objects_button =
        new ButtonGuiItem("Objects", game.sysContent.fntStandard);
    objects_button->onActivate =
    [this] (const Point &) {
        populateTidbits(HELP_CATEGORY_OBJECTS);
    };
    objects_button->onGetTooltip =
    [] () {
        return "Show help about some noteworthy objects you'll find.";
    };
    gui.addItem(objects_button, "objects");
    
    //Manual text.
    BulletGuiItem* manual_bullet =
        new BulletGuiItem("More help...", game.sysContent.fntStandard);
    manual_bullet->onActivate =
    [] (const Point &) {
        openManual("home.html");
    };
    manual_bullet->onGetTooltip = [] () {
        return
            "Click to open the manual (in the game's folder) for more help.";
    };
    gui.addItem(manual_bullet, "manual");
    
    //Category text.
    categoryText = new TextGuiItem("Help", game.sysContent.fntStandard);
    gui.addItem(categoryText, "category");
    
    //Tidbit list box.
    tidbitList = new ListGuiItem();
    gui.addItem(tidbitList, "list");
    
    //Tidbit list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->listItem = tidbitList;
    gui.addItem(list_scroll, "list_scroll");
    
    //Image item.
    GuiItem* image_item = new GuiItem();
    image_item->onDraw =
    [this] (const DrawInfo & draw) {
        if(curTidbit == nullptr) return;
        if(curTidbit->image == nullptr) return;
        drawBitmapInBox(
            curTidbit->image,
            draw.center, draw.size, false
        );
    };
    gui.addItem(image_item, "image");
    
    //Tooltip text.
    TextGuiItem* tooltip_text =
        new TextGuiItem("", game.sysContent.fntStandard);
    tooltip_text->onDraw =
        [this]
    (const DrawInfo & draw) {
        drawTidbit(
            game.sysContent.fntStandard, draw.center, draw.size,
            gui.getCurrentTooltip()
        );
    };
    gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    gui.setSelectedItem(gui.backItem, true);
    gui.onSelectionChanged =
    [this] () {
        curTidbit = nullptr;
    };
}


/**
 * @brief Populates the help menu's list of tidbits.
 *
 * @param category Category of tidbits to use.
 */
void HelpMenu::populateTidbits(const HELP_CATEGORY category) {
    vector<Tidbit> &category_tidbits = tidbits[category];
    
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
    
    for(size_t t = 0; t < category_tidbits.size(); t++) {
        Tidbit* t_ptr = &category_tidbits[t];
        BulletGuiItem* tidbit_bullet =
            new BulletGuiItem(
            t_ptr->name,
            game.sysContent.fntStandard
        );
        tidbit_bullet->ratioCenter = Point(0.50f, 0.045f + t * 0.10f);
        tidbit_bullet->ratioSize = Point(1.0f, 0.09f);
        tidbit_bullet->onGetTooltip = [this, t_ptr] () {
            return t_ptr->description;
        };
        tidbit_bullet->onSelected = [this, t_ptr] () {
            curTidbit = t_ptr;
        };
        tidbit_bullet->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
        );
        tidbitList->addChild(tidbit_bullet);
        gui.addItem(tidbit_bullet);
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
                game.content.bitmaps.list.free(tidbits[(HELP_CATEGORY) c][t].image);
            }
        }
    }
    tidbits.clear();
    
    Menu::unload();
}
