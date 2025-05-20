/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion menu classes and functions.
 */

#include <algorithm>

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"
#include "gameplay.h"


using DrawInfo = GuiItem::DrawInfo;


namespace ONION_MENU {

//Name of the GUI information file.
const string GUI_FILE_NAME = "onion_menu";

//How long to let text turn red for.
const float RED_TEXT_DURATION = 1.0f;

//The Onion menu can only show, at most, these many Pikmin types per page.
const size_t TYPES_PER_PAGE = 5;

}


/**
 * @brief Constructs a new Onion menu struct object.
 *
 * @param nPtr Pointer to the nest information struct.
 * @param lPtr Leader responsible.
 */
OnionMenu::OnionMenu(
    PikminNest* nPtr, Leader* lPtr
) :
    nestPtr(nPtr),
    leaderPtr(lPtr) {
    
    for(size_t t = 0; t < nPtr->nestType->pikTypes.size(); t++) {
        types.push_back(
            OnionMenuPikminType(t, nPtr->nestType->pikTypes[t])
        );
    }
    
    nrPages = ceil(types.size() / (float) ONION_MENU::TYPES_PER_PAGE);
    
    gui.registerCoords("instructions",     50,  7, 90, 20);
    gui.registerCoords("cancel",           16, 85, 18, 11);
    gui.registerCoords("cancel_input",      8, 89,  4,  4);
    gui.registerCoords("ok",               84, 85, 18, 11);
    gui.registerCoords("field",            50, 75, 18,  4);
    gui.registerCoords("select_all",       50, 87, 24,  6);
    gui.registerCoords("onion_1_button",   50, 20,  9, 12);
    gui.registerCoords("onion_2_button",   50, 20,  9, 12);
    gui.registerCoords("onion_3_button",   50, 20,  9, 12);
    gui.registerCoords("onion_4_button",   50, 20,  9, 12);
    gui.registerCoords("onion_5_button",   50, 20,  9, 12);
    gui.registerCoords("onion_1_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_2_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_3_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_4_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_5_amount",   50, 29, 12,  4);
    gui.registerCoords("group_1_button",   50, 60,  9, 12);
    gui.registerCoords("group_2_button",   50, 60,  9, 12);
    gui.registerCoords("group_3_button",   50, 60,  9, 12);
    gui.registerCoords("group_4_button",   50, 60,  9, 12);
    gui.registerCoords("group_5_button",   50, 60,  9, 12);
    gui.registerCoords("group_1_amount",   50, 51, 12,  4);
    gui.registerCoords("group_2_amount",   50, 51, 12,  4);
    gui.registerCoords("group_3_amount",   50, 51, 12,  4);
    gui.registerCoords("group_4_amount",   50, 51, 12,  4);
    gui.registerCoords("group_5_amount",   50, 51, 12,  4);
    gui.registerCoords("onion_all",        50, 20,  9, 12);
    gui.registerCoords("group_all",        50, 60,  9, 12);
    gui.registerCoords("prev_page",         5, 40,  8, 10);
    gui.registerCoords("next_page",        95, 40,  8, 11);
    gui.registerCoords("onion_left_more",   5, 20,  3,  4);
    gui.registerCoords("onion_right_more", 95, 20,  3,  4);
    gui.registerCoords("group_left_more",   5, 60,  3,  4);
    gui.registerCoords("group_right_more", 95, 60,  3,  4);
    gui.registerCoords("tooltip",          50, 95, 95,  8);
    gui.readCoords(
        game.content.guiDefs.list[ONION_MENU::GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Instructions text.
    TextGuiItem* instructionsText =
        new TextGuiItem(
        "Call or store Pikmin", game.sysContent.fntStandard,
        al_map_rgb(188, 230, 230)
    );
    gui.addItem(instructionsText, "instructions");
    
    //Cancel button.
    gui.backItem =
        new ButtonGuiItem(
        "Cancel", game.sysContent.fntStandard, al_map_rgb(226, 112, 112)
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        startClosing();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Forget all changes and leave the Onion menu."; };
    gui.addItem(gui.backItem, "cancel");
    
    //Cancel input icon.
    guiAddBackInputIcon(&gui, "cancel_input");
    
    //Ok button.
    ButtonGuiItem* okButton =
        new ButtonGuiItem(
        "Ok", game.sysContent.fntStandard, al_map_rgb(96, 226, 80)
    );
    okButton->onActivate =
    [this] (const Point&) {
        confirm();
        startClosing();
    };
    okButton->onGetTooltip =
    [] () { return "Confirm changes."; };
    gui.addItem(okButton, "ok");
    
    //Field amount text.
    fieldAmountText =
        new TextGuiItem("", game.sysContent.fntStandard);
    fieldAmountText->onDraw =
    [this] (const DrawInfo & draw) {
        int totalDelta = 0;
        for(size_t t = 0; t < this->types.size(); t++) {
            totalDelta += this->types[t].delta;
        }
        
        drawFilledRoundedRectangle(
            draw.center, draw.size, game.winW * 0.01,
            al_map_rgba(188, 230, 230, 128)
        );
        
        ALLEGRO_COLOR color = al_map_rgb(188, 230, 230);
        const auto& redIt = this->redItems.find(fieldAmountText);
        if(redIt != this->redItems.end()) {
            color =
                interpolateColor(
                    redIt->second,
                    0, ONION_MENU::RED_TEXT_DURATION,
                    color, al_map_rgb(224, 0, 0)
                );
        }
        
        float juicyGrowAmount = fieldAmountText->getJuiceValue();
        drawText(
            "Field: " +
            i2s(game.states.gameplay->mobs.pikmin.size() + totalDelta),
            game.sysContent.fntStandard, draw.center,
            draw.size * GUI::STANDARD_CONTENT_SIZE, color,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + juicyGrowAmount)
        );
    };
    gui.addItem(fieldAmountText, "field");
    
    //Select all checkbox.
    CheckGuiItem* selectAllCheck =
        new CheckGuiItem(
        &selectAll,
        "Select all", game.sysContent.fntStandard, al_map_rgb(188, 230, 230)
    );
    selectAllCheck->onActivate =
    [this, selectAllCheck] (const Point&) {
        growButtons();
        selectAllCheck->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        toggleSelectAll();
    };
    selectAllCheck->visible = types.size() > 1;
    selectAllCheck->selectable = types.size() > 1;
    selectAllCheck->onGetTooltip =
    [] () { return "Control all Pikmin numbers at once?"; };
    gui.addItem(selectAllCheck, "select_all");
    
    //Onion icons and buttons.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        string id = "onion_" + i2s(t + 1) + "_button";
        
        GuiItem* onionIcon = new GuiItem(false);
        onionIcon->onDraw =
        [this, t, onionIcon] (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            if(tPtr->pikType->bmpOnionIcon) {
                float juicyGrowAmount = onionIcon->getJuiceValue();
                drawBitmapInBox(
                    tPtr->pikType->bmpOnionIcon, draw.center,
                    (draw.size * 0.8f) + juicyGrowAmount, true
                );
            }
        };
        gui.addItem(onionIcon, id);
        onionIconItems.push_back(onionIcon);
        
        ButtonGuiItem* onionButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        onionButton->onDraw =
        [this, onionButton] (const DrawInfo & draw) {
            float juicyGrowAmount = onionButton->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicyGrowAmount,
                "", game.sysContent.fntStandard, COLOR_WHITE,
                onionButton->selected
            );
        };
        onionButton->onActivate =
        [this, t] (const Point&) {
            addToOnion(onWindowTypes[t]->typeIdx);
        };
        onionButton->canAutoRepeat = true;
        onionButton->onGetTooltip =
        [this, t] () {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            return "Store one " + tPtr->pikType->name + " inside.";
        };
        gui.addItem(onionButton, id);
        onionButtonItems.push_back(onionButton);
    }
    
    //Onion's all button.
    onionAllButton =
        new ButtonGuiItem("", game.sysContent.fntStandard);
    onionAllButton->onDraw =
    [this] (const DrawInfo & draw) {
        float juicyGrowAmount = onionAllButton->getJuiceValue();
        drawButton(
            draw.center,
            draw.size + juicyGrowAmount,
            "", game.sysContent.fntStandard, COLOR_WHITE,
            onionAllButton->selected
        );
    };
    onionAllButton->onActivate =
    [this] (const Point&) {
        addAllToOnion();
    };
    onionAllButton->canAutoRepeat = true;
    onionAllButton->onGetTooltip =
    [] () { return "Store one Pikmin of each type inside."; };
    gui.addItem(onionAllButton, "onion_all");
    
    //Onion amounts.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        GuiItem* onionAmountText = new GuiItem(false);
        onionAmountText->onDraw =
            [this, t, onionAmountText]
        (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            
            size_t realOnionAmount =
                this->nestPtr->getAmountByType(tPtr->pikType);
                
            drawFilledRoundedRectangle(
                draw.center, draw.size, game.winW * 0.01,
                al_map_rgba(188, 230, 230, 128)
            );
            
            ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
            const auto& redIt = this->redItems.find(onionAmountText);
            if(redIt != this->redItems.end()) {
                color =
                    interpolateColor(
                        redIt->second,
                        0, ONION_MENU::RED_TEXT_DURATION,
                        color, al_map_rgb(224, 0, 0)
                    );
            }
            
            float juicyGrowAmount = onionAmountText->getJuiceValue();
            drawText(
                i2s(realOnionAmount - tPtr->delta),
                game.sysContent.fntAreaName, draw.center,
                draw.size * GUI::STANDARD_CONTENT_SIZE, color,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0f + juicyGrowAmount)
            );
        };
        gui.addItem(onionAmountText, "onion_" + i2s(t + 1) + "_amount");
        onionAmountItems.push_back(onionAmountText);
    }
    
    //Group icons and buttons.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        string id = "group_" + i2s(t + 1) + "_button";
        
        GuiItem* groupIcon = new GuiItem(false);
        groupIcon->onDraw =
        [this, t, groupIcon] (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            float juicyGrowAmount = groupIcon->getJuiceValue();
            if(tPtr->pikType->bmpIcon) {
                drawBitmapInBox(
                    tPtr->pikType->bmpIcon, draw.center,
                    (draw.size * 0.8f) + juicyGrowAmount, true
                );
            }
        };
        gui.addItem(groupIcon, id);
        groupIconItems.push_back(groupIcon);
        
        ButtonGuiItem* groupButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        groupButton->onDraw =
        [this, groupButton] (const DrawInfo & draw) {
            float juicyGrowAmount = groupButton->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicyGrowAmount,
                "", game.sysContent.fntStandard, COLOR_WHITE,
                groupButton->selected
            );
        };
        groupButton->onActivate =
        [this, t] (const Point&) {
            addToGroup(onWindowTypes[t]->typeIdx);
        };
        groupButton->canAutoRepeat = true;
        groupButton->onGetTooltip =
        [this, t] () {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            return "Call one " + tPtr->pikType->name + " to the group.";
        };
        gui.addItem(groupButton, id);
        groupButtonItems.push_back(groupButton);
    }
    
    //Group's all button.
    groupAllButton =
        new ButtonGuiItem("", game.sysContent.fntStandard);
    groupAllButton->onDraw =
    [this] (const DrawInfo & draw) {
        float juicyGrowAmount = groupAllButton->getJuiceValue();
        drawButton(
            draw.center,
            draw.size + juicyGrowAmount,
            "", game.sysContent.fntStandard, COLOR_WHITE,
            groupAllButton->selected
        );
    };
    groupAllButton->onActivate =
    [this] (const Point&) {
        addAllToGroup();
    };
    groupAllButton->canAutoRepeat = true;
    groupAllButton->onGetTooltip =
    [] () { return "Call one Pikmin of each type to the group."; };
    gui.addItem(groupAllButton, "group_all");
    
    //Group amounts.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        GuiItem* groupAmountText = new GuiItem(false);
        groupAmountText->onDraw =
            [this, t, groupAmountText]
        (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = this->onWindowTypes[t];
            
            size_t realGroupAmount =
                this->leaderPtr->group->getAmountByType(tPtr->pikType);
                
            drawFilledRoundedRectangle(
                draw.center, draw.size, game.winW * 0.01,
                al_map_rgba(188, 230, 230, 128)
            );
            
            ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
            const auto& redIt = this->redItems.find(groupAmountText);
            if(redIt != this->redItems.end()) {
                color =
                    interpolateColor(
                        redIt->second,
                        0, ONION_MENU::RED_TEXT_DURATION,
                        color, al_map_rgb(224, 0, 0)
                    );
            }
            
            float juicyGrowAmount = groupAmountText->getJuiceValue();
            drawText(
                i2s(realGroupAmount + tPtr->delta),
                game.sysContent.fntAreaName, draw.center,
                draw.size * GUI::STANDARD_CONTENT_SIZE, color,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0f + juicyGrowAmount)
            );
        };
        gui.addItem(groupAmountText, "group_" + i2s(t + 1) + "_amount");
        groupAmountItems.push_back(groupAmountText);
    }
    
    //Onion left "more" indicator.
    onionMoreLIcon = new GuiItem(false);
    onionMoreLIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center,
            Point(-draw.size.x, draw.size.y) * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(onionMoreLIcon, "onion_left_more");
    
    //Onion right "more" indicator.
    onionMoreRIcon = new GuiItem(false);
    onionMoreRIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center,
            draw.size * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(onionMoreRIcon, "onion_right_more");
    
    //Group left "more" indicator.
    groupMoreLIcon = new GuiItem(false);
    groupMoreLIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center,
            Point(-draw.size.x, draw.size.y) * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(groupMoreLIcon, "group_left_more");
    
    //Group right "more" indicator.
    groupMoreRIcon = new GuiItem(false);
    groupMoreRIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center,
            draw.size * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(groupMoreRIcon, "group_right_more");
    
    //Previous page button.
    prevPageButton = new GuiItem(true);
    prevPageButton->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center, Point(-draw.size.x, draw.size.y) * 0.5f
        );
        
        drawButton(
            draw.center, draw.size, "",
            game.sysContent.fntStandard, COLOR_WHITE,
            prevPageButton->selected,
            prevPageButton->getJuiceValue()
        );
    };
    prevPageButton->onActivate =
    [this] (const Point&) {
        goToPage(sumAndWrap((int) page, -1, (int) nrPages));
    };
    prevPageButton->visible = nrPages > 1;
    prevPageButton->selectable = nrPages > 1;
    prevPageButton->onGetTooltip =
    [] () { return "Go to the previous page of Pikmin types."; };
    gui.addItem(prevPageButton, "prev_page");
    
    //Next page button.
    nextPageButton = new GuiItem(true);
    nextPageButton->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sysContent.bmpMore,
            draw.center, draw.size * 0.5f
        );
        
        drawButton(
            draw.center, draw.size, "",
            game.sysContent.fntStandard, COLOR_WHITE,
            nextPageButton->selected,
            nextPageButton->getJuiceValue()
        );
    };
    nextPageButton->onActivate =
    [this] (const Point&) {
        goToPage(sumAndWrap((int) page, 1, (int) nrPages));
    };
    nextPageButton->visible = nrPages > 1;
    nextPageButton->selectable = nrPages > 1;
    nextPageButton->onGetTooltip =
    [] () { return "Go to the next page of Pikmin types."; };
    gui.addItem(nextPageButton, "next_page");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    update();
    gui.startAnimation(
        GUI_MANAGER_ANIM_UP_TO_CENTER,
        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/**
 * @brief Destroys the Onion menu struct object.
 */
OnionMenu::~OnionMenu() {
    gui.destroy();
}


/**
 * @brief Adds one Pikmin of each type from Onion to the group, if possible.
 */
void OnionMenu::addAllToGroup() {
    for(size_t t = 0; t < types.size(); t++) {
        addToGroup(t);
    }
}


/**
 * @brief Adds one Pikmin of each type from the group to the Onion, if possible.
 */
void OnionMenu::addAllToOnion() {
    for(size_t t = 0; t < types.size(); t++) {
        addToOnion(t);
    }
}


/**
 * @brief Adds one Pikmin from the Onion to the group, if possible.
 *
 * @param typeIdx Index of the Onion's Pikmin type.
 */
void OnionMenu::addToGroup(size_t typeIdx) {
    size_t realOnionAmount =
        nestPtr->getAmountByType(nestPtr->nestType->pikTypes[typeIdx]);
        
    //First, check if there are enough in the Onion to take out.
    if((signed int) (realOnionAmount - types[typeIdx].delta) <= 0) {
        size_t windowIdx = types[typeIdx].onWindowIdx;
        if(windowIdx != INVALID) {
            makeGuiItemRed(onionAmountItems[windowIdx]);
        }
        return;
    }
    
    //Next, check if the addition won't make the field amount hit the limit.
    int totalDelta = 0;
    for(size_t t = 0; t < types.size(); t++) {
        totalDelta += types[t].delta;
    }
    if(
        game.states.gameplay->mobs.pikmin.size() + totalDelta >=
        game.config.rules.maxPikminInField
    ) {
        makeGuiItemRed(fieldAmountText);
        return;
    }
    
    types[typeIdx].delta++;
    
    size_t onWindowIdx = types[typeIdx].onWindowIdx;
    if(onWindowIdx != INVALID) {
        onionAmountItems[onWindowIdx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
        groupAmountItems[onWindowIdx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
    }
    fieldAmountText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
    );
    
}


/**
 * @brief Adds one Pikmin from the group to the Onion, if possible.
 *
 * @param typeIdx Index of the Onion's Pikmin type.
 */
void OnionMenu::addToOnion(size_t typeIdx) {
    size_t realGroupAmount =
        leaderPtr->group->getAmountByType(nestPtr->nestType->pikTypes[typeIdx]);
        
    if((signed int) (realGroupAmount + types[typeIdx].delta) <= 0) {
        size_t windowIdx = types[typeIdx].onWindowIdx;
        if(windowIdx != INVALID) {
            makeGuiItemRed(groupAmountItems[windowIdx]);
        }
        return;
    }
    
    types[typeIdx].delta--;
    
    size_t onWindowIdx = types[typeIdx].onWindowIdx;
    if(onWindowIdx != INVALID) {
        onionAmountItems[onWindowIdx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
        groupAmountItems[onWindowIdx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
    }
    fieldAmountText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
    );
}


/**
 * @brief Confirms the player's changes, and sets up the Pikmin to climb up the
 * Onion, if any, and sets up the Onion to spit out Pikmin, if any.
 */
void OnionMenu::confirm() {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t].delta > 0) {
            nestPtr->requestPikmin(t, types[t].delta, leaderPtr);
        } else if(types[t].delta < 0) {
            leaderPtr->orderPikminToOnion(
                types[t].pikType, nestPtr, -types[t].delta
            );
        }
    }
}


/**
 * @brief Flips to the specified page of Pikmin types.
 *
 * @param page Index of the new page.
 */
void OnionMenu::goToPage(size_t page) {
    this->page = page;
    growButtons();
    update();
}


/**
 * @brief Makes the Onion and group buttons juicy grow.
 */
void OnionMenu::growButtons() {
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        onionIconItems[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        onionButtonItems[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        groupIconItems[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        groupButtonItems[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
    }
    onionAllButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
    groupAllButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void OnionMenu::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!closing) gui.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void OnionMenu::handlePlayerAction(const PlayerAction& action) {
    gui.handlePlayerAction(action);
}


/**
 * @brief Makes a given GUI item turn red.
 *
 * @param item The item.
 */
void OnionMenu::makeGuiItemRed(GuiItem* item) {
    redItems[item] = ONION_MENU::RED_TEXT_DURATION;
}


/**
 * @brief Starts the closing process.
 */
void OnionMenu::startClosing() {
    closing = true;
    closingTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
    gui.startAnimation(
        GUI_MANAGER_ANIM_CENTER_TO_UP, GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    for(Player& player : game.states.gameplay->players) {
        player.hud->gui.startAnimation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void OnionMenu::tick(float deltaT) {

    //Correct the amount of wanted group members, if they are invalid.
    int totalDelta = 0;
    
    for(size_t t = 0; t < nestPtr->nestType->pikTypes.size(); t++) {
        //Get how many the player really has with them.
        int realGroupAmount =
            (int) leaderPtr->group->getAmountByType(
                nestPtr->nestType->pikTypes[t]
            );
            
        //Make sure the player can't request to store more than what they have.
        types[t].delta = std::max(-realGroupAmount, (int) types[t].delta);
        
        //Get how many are really in the Onion.
        int realOnionAmount =
            (int) nestPtr->getAmountByType(nestPtr->nestType->pikTypes[t]);
            
        //Make sure the player can't request to call more than the Onion has.
        types[t].delta = std::min(realOnionAmount, (int) types[t].delta);
        
        //Calculate the total delta.
        totalDelta += types[t].delta;
    }
    
    //Make sure the player can't request to have more than the field limit.
    int deltaOverLimit =
        (int) game.states.gameplay->mobs.pikmin.size() +
        totalDelta -
        (int) game.config.rules.maxPikminInField;
        
    while(deltaOverLimit > 0) {
        vector<size_t> candidateTypes;
        
        for(size_t t = 0; t < nestPtr->nestType->pikTypes.size(); t++) {
            int realGroupAmount =
                (int) leaderPtr->group->getAmountByType(
                    nestPtr->nestType->pikTypes[t]
                );
                
            if((-types[t].delta) < realGroupAmount) {
                //It's possible to take away from this type's delta request.
                candidateTypes.push_back(t);
            }
        }
        
        //Figure out the type with the largest delta.
        size_t bestType = 0;
        int bestTypeDelta = types[candidateTypes[0]].delta;
        for(size_t t = 1; t < candidateTypes.size(); t++) {
            if(types[candidateTypes[t]].delta > bestTypeDelta) {
                bestType = candidateTypes[t];
                bestTypeDelta = types[candidateTypes[t]].delta;
            }
        }
        
        //Finally, remove one request from this type.
        types[candidateTypes[bestType]].delta--;
        deltaOverLimit--;
    }
    
    //Animate red text, if any.
    for(auto w = redItems.begin(); w != redItems.end();) {
        w->second -= deltaT;
        if(w->second <= 0.0f) {
            w = redItems.erase(w);
        } else {
            ++w;
        }
    }
    
    //Tick the GUI.
    gui.tick(deltaT);
    
    //Tick the background.
    const float bgAlphaMultSpeed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bgAlphaMultSpeed : bgAlphaMultSpeed;
    bgAlphaMult = std::clamp(bgAlphaMult + diff * deltaT, 0.0f, 1.0f);
    
    //Tick the menu closing.
    if(closing) {
        closingTimer -= deltaT;
        if(closingTimer <= 0.0f) {
            toDelete = true;
        }
    }
}


/**
 * @brief Toggles the "select all" mode.
 */
void OnionMenu::toggleSelectAll() {
    selectAll = !selectAll;
    
    update();
}


/**
 * @brief Updates some things about the Onion's state, especially caches.
 */
void OnionMenu::update() {
    //Reset the on-window types.
    onWindowTypes.clear();
    
    for(size_t t = 0; t < types.size(); t++) {
        types[t].onWindowIdx = INVALID;
    }
    
    //Reset the button and amount states.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        onionIconItems[t]->visible = false;
        onionButtonItems[t]->visible = false;
        onionButtonItems[t]->selectable = false;
        onionAmountItems[t]->visible = false;
        groupIconItems[t]->visible = false;
        groupButtonItems[t]->visible = false;
        groupButtonItems[t]->selectable = false;
        groupAmountItems[t]->visible = false;
    }
    
    //Assign the on-window types.
    for(
        size_t t = page * ONION_MENU::TYPES_PER_PAGE;
        t < (page + 1) * ONION_MENU::TYPES_PER_PAGE &&
        t < nestPtr->nestType->pikTypes.size();
        t++
    ) {
        types[t].onWindowIdx = onWindowTypes.size();
        onWindowTypes.push_back(&types[t]);
    }
    
    //Assign the coordinates of the on-window-type-related GUI items.
    float splits = onWindowTypes.size() + 1;
    float leftmost = 0.50f;
    float rightmost = 0.50f;
    
    for(size_t t = 0; t < onWindowTypes.size(); t++) {
        float x = 1.0f / splits * (t + 1);
        onionIconItems[t]->ratioCenter.x = x;
        onionButtonItems[t]->ratioCenter.x = x;
        onionAmountItems[t]->ratioCenter.x = x;
        groupIconItems[t]->ratioCenter.x = x;
        groupButtonItems[t]->ratioCenter.x = x;
        groupAmountItems[t]->ratioCenter.x = x;
        
        leftmost =
            std::min(
                leftmost,
                x - onionButtonItems[t]->ratioSize.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                x + onionButtonItems[t]->ratioSize.x / 2.0f
            );
    }
    
    //Make all relevant GUI items active.
    for(size_t t = 0; t < onWindowTypes.size(); t++) {
        onionIconItems[t]->visible = true;
        onionAmountItems[t]->visible = true;
        groupIconItems[t]->visible = true;
        groupAmountItems[t]->visible = true;
        if(!selectAll) {
            onionButtonItems[t]->visible = true;
            onionButtonItems[t]->selectable = true;
            groupButtonItems[t]->visible = true;
            groupButtonItems[t]->selectable = true;
        }
    }
    
    if(nrPages > 1) {
        leftmost =
            std::min(
                leftmost,
                onionMoreLIcon->ratioCenter.x -
                onionMoreLIcon->ratioSize.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                onionMoreRIcon->ratioCenter.x +
                onionMoreRIcon->ratioSize.x / 2.0f
            );
    }
    
    onionMoreLIcon->visible = nrPages > 1 && selectAll;
    onionMoreRIcon->visible = nrPages > 1 && selectAll;
    groupMoreLIcon->visible = nrPages > 1 && selectAll;
    groupMoreRIcon->visible = nrPages > 1 && selectAll;
    
    onionAllButton->ratioSize.x = rightmost - leftmost;
    groupAllButton->ratioSize.x = rightmost - leftmost;
    
    onionAllButton->visible = selectAll;
    onionAllButton->selectable = selectAll;
    groupAllButton->visible = selectAll;
    groupAllButton->selectable = selectAll;
}


/**
 * @brief Constructs a new Onion menu type struct object.
 *
 * @param idx Index of the Pikmin type in the nest object.
 * @param pikType The Pikmin type.
 */
OnionMenuPikminType::OnionMenuPikminType(
    size_t idx, PikminType* pikType
) :
    typeIdx(idx),
    pikType(pikType) {
    
}
