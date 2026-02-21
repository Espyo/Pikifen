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

//Name of the GUI definition file.
const string GUI_FILE_NAME = "onion_menu";

//Maximum number of Pikmin types visible without scrolling.
const size_t NR_TYPES_VISIBLE = 6;

//How long to let text turn red for.
const float RED_TEXT_DURATION = 1.0f;

//Name of the Pikmin type GUI definition file.
const string TYPE_GUI_FILE_NAME = "onion_menu_pikmin_type";

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
    
    DataNode* guiFile = &game.content.guiDefs.list[ONION_MENU::GUI_FILE_NAME];
    gui.registerCoords("cancel",           8.75, 16.25, 12.5, 12.5);
    gui.registerCoords("cancel_input",      2.5,  22.5,    4,    4);
    gui.registerCoords("ok",               8.75, 71.25, 12.5, 12.5);
    gui.registerCoords("ok_input",          2.5,  77.5,    4,    4);
    gui.registerCoords("field",              50,  87.5,   30,    5);
    gui.registerCoords("change_ten",          5,    37,    5,    8);
    gui.registerCoords("change_ten_input",  2.5,    41,    4,    4);
    gui.registerCoords("select_all",          5,    50,    5,    8);
    gui.registerCoords("select_all_input",  2.5,    54,    4,    4);
    gui.registerCoords("list",             57.5, 43.75,   80, 67.5);
    gui.registerCoords("list_scroll",      57.5, 81.25,   80,  2.5);
    gui.registerCoords("tooltip",            50,    95,   95,    8);
    gui.readDataFile(guiFile);
    
    DataNode* typeGuiFile =
        &game.content.guiDefs.list[ONION_MENU::TYPE_GUI_FILE_NAME];
    gui.registerCoords("onion_button",     11.25,    15, 12.5,  20);
    gui.registerCoords("onion_amount",     11.25, 33.75, 12.5, 7.5);
    gui.registerCoords("group_button",     11.25,    85, 12.5,  20);
    gui.registerCoords("group_amount",     11.25, 66.25, 12.5, 7.5);
    gui.registerCoords("full_type",         77.5,    50, 12.5,  90);
    gui.registerCoords("onion_all_button",    40,    15, 12.5,  20);
    gui.registerCoords("group_all_button",    40,    85, 12.5,  20);
    gui.registerCoords("full_type_all",     57.5,    50, 12.5,  90);
    gui.readDataFile(typeGuiFile);
    
    //Cancel button.
    gui.backItem =
        new ButtonGuiItem(
        "Cancel", game.sysContent.fntStandard, game.config.guiColors.bad
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        startClosing();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Forget all changes and leave the Onion menu."; };
    gui.addItem(gui.backItem, "cancel");
    
    //Cancel input icon.
    guiCreateBackInputIcon(&gui, "cancel_input");
    
    //Ok button.
    okButton =
        new ButtonGuiItem(
        "Ok", game.sysContent.fntStandard, game.config.guiColors.good
    );
    okButton->onActivate =
    [this] (const Point&) {
        confirm();
    };
    okButton->onGetTooltip =
    [] () { return "Confirm changes."; };
    gui.addItem(okButton, "ok");
    
    //Ok input.
    GuiItem* okInput = new GuiItem();
    okInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_MENU_OK, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    gui.addItem(okInput, "ok_input");
    
    //Field amount text.
    fieldAmountText =
        new TextGuiItem("", game.sysContent.fntStandard);
    fieldAmountText->onDraw =
    [this] (const DrawInfo & draw) {
        int totalDelta = 0;
        for(size_t t = 0; t < this->types.size(); t++) {
            totalDelta += this->types[t].delta;
        }
        
        ALLEGRO_COLOR color = al_map_rgb(188, 230, 230);
        const auto& redIt = this->redItems.find(fieldAmountText);
        if(redIt != this->redItems.end()) {
            color =
                interpolateColor(
                    redIt->second,
                    0, ONION_MENU::RED_TEXT_DURATION,
                    tintColor(color, draw.tint),
                    tintColor(al_map_rgb(224, 0, 0), draw.tint)
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
    
    //Change ten at a time button.
    changeTenButton =
        new ButtonGuiItem(
        "", game.sysContent.fntStandard, al_map_rgb(188, 230, 230)
    );
    changeTenButton->forceSquare = true;
    changeTenButton->onActivate =
    [this] (const Point&) {
        toggleChangeTen();
    };
    changeTenButton->onDraw =
    [this] (const DrawInfo & draw) {
        float juicyGrowAmount = changeTenButton->getJuiceValue();
        drawBitmapInBox(
            changeTen ?
            game.sysContent.bmpOnionMenu10 :
            game.sysContent.bmpOnionMenu1,
            draw.center, (draw.size * (0.8f + juicyGrowAmount)), true,
            0.0f, draw.tint
        );
        changeTenButton->defDrawCode(draw);
    };
    changeTenButton->onGetTooltip =
    [this] () {
        if(changeTen) {
            return
                "Changing the numbers by ten at a time. "
                "Press to change by one.";
        } else {
            return
                "Changing the numbers by one at a time. "
                "Press to change by ten.";
        }
    };
    gui.addItem(changeTenButton, "change_ten");
    
    //Change ten at a time input.
    GuiItem* changeTenInput = new GuiItem();
    changeTenInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_ONION_CHANGE_10, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    gui.addItem(changeTenInput, "change_ten_input");
    
    //Select all button.
    selectAllButton =
        new ButtonGuiItem(
        "", game.sysContent.fntStandard, al_map_rgb(188, 230, 230)
    );
    selectAllButton->forceSquare = true;
    selectAllButton->onActivate =
    [this] (const Point&) {
        toggleSelectAll();
    };
    selectAllButton->onDraw =
    [this] (const DrawInfo & draw) {
        float juicyGrowAmount = selectAllButton->getJuiceValue();
        drawBitmapInBox(
            selectAll ?
            game.sysContent.bmpOnionMenuAll :
            game.sysContent.bmpOnionMenuSingle,
            draw.center, (draw.size * (0.8f + juicyGrowAmount)), true,
            0.0f, draw.tint
        );
        selectAllButton->defDrawCode(draw);
    };
    selectAllButton->visible = types.size() > 1;
    selectAllButton->focusable = types.size() > 1;
    selectAllButton->onGetTooltip =
    [this] () {
        if(selectAll) {
            return
                "Controlling all Pikmin types at once. "
                "Press to control one at a time.";
        } else {
            return
                "Controlling one Pikmin type at a time. "
                "Press to control all at once.";
        }
    };
    gui.addItem(selectAllButton, "select_all");
    
    //Select all input.
    GuiItem* selectAllInput = new GuiItem();
    selectAllInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_ONION_SELECT_ALL, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    selectAllInput->visible = types.size() > 1;
    gui.addItem(selectAllInput, "select_all_input");
    
    //List box.
    listItem = new ListGuiItem();
    listItem->horizontal = true;
    gui.addItem(listItem, "list");
    
    //List scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->horizontal = true;
    listScroll->listItem = listItem;
    gui.addItem(listScroll, "list_scroll");
    
    //Items for each Pikmin type.
    for(size_t t = 0; t < types.size(); t++) {
    
        //Onion icon.
        GuiItem* onionIcon = new GuiItem(false);
        onionIcon->forceSquare = true;
        onionIcon->onDraw =
        [this, t, onionIcon] (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = &this->types[t];
            if(tPtr->pikType->bmpOnionIcon) {
                float juicyGrowAmount = onionIcon->getJuiceValue();
                drawBitmapInBox(
                    tPtr->pikType->bmpOnionIcon, draw.center,
                    (draw.size * 0.8f) + juicyGrowAmount, true,
                    0.0f, draw.tint
                );
            }
        };
        listItem->addChild(onionIcon);
        gui.addItem(onionIcon, "onion_button");
        onionIconItems.push_back(onionIcon);
        
        //Onion button.
        ButtonGuiItem* onionButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        onionButton->forceSquare = true;
        onionButton->onDraw =
        [this, onionButton] (const DrawInfo & draw) {
            float juicyGrowAmount = onionButton->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicyGrowAmount,
                "", game.sysContent.fntStandard, COLOR_WHITE,
                onionButton->focused, 0.0f, draw.tint
            );
        };
        onionButton->onActivate =
        [this, t, onionButton] (const Point&) {
            doButtonLogic(false, t, false);
        };
        onionButton->onMenuSNAction =
        [this, t] (PLAYER_ACTION_TYPE playerActionId) {
            return doButtonSNLogic(playerActionId, t);
        };
        onionButton->canAutoRepeat = true;
        onionButton->focusableFromSN = false;
        onionButton->onGetTooltip =
        [this, t] () {
            OnionMenuPikminType* tPtr = &this->types[t];
            return
                "Store " + getTransferAmountStr() + " " +
                tPtr->pikType->name + " inside.";
        };
        listItem->addChild(onionButton);
        gui.addItem(onionButton, "onion_button");
        onionButtonItems.push_back(onionButton);
        
        //Onion amount text.
        GuiItem* onionAmountText = new GuiItem(false);
        onionAmountText->onDraw =
            [this, t, onionAmountText]
        (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = &this->types[t];
            
            size_t realOnionAmount =
                this->nestPtr->getAmountByType(tPtr->pikType);
                
            drawFilledRoundedRatioRectangle(
                draw.center, draw.size, 0.30f,
                tintColor(al_map_rgba(188, 230, 230, 128), draw.tint)
            );
            
            ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
            const auto& redIt = this->redItems.find(onionAmountText);
            if(redIt != this->redItems.end()) {
                color =
                    interpolateColor(
                        redIt->second,
                        0, ONION_MENU::RED_TEXT_DURATION,
                        color, tintColor(al_map_rgb(224, 0, 0), draw.tint)
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
        listItem->addChild(onionAmountText);
        gui.addItem(onionAmountText, "onion_amount");
        onionAmountItems.push_back(onionAmountText);
        
        //Group icon.
        GuiItem* groupIcon = new GuiItem(false);
        groupIcon->forceSquare = true;
        groupIcon->onDraw =
        [this, t, groupIcon] (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = &this->types[t];
            float juicyGrowAmount = groupIcon->getJuiceValue();
            if(tPtr->pikType->bmpIcon) {
                drawBitmapInBox(
                    tPtr->pikType->bmpIcon, draw.center,
                    (draw.size * 0.8f) + juicyGrowAmount, true, 0.0f, draw.tint
                );
            }
        };
        listItem->addChild(groupIcon);
        gui.addItem(groupIcon, "group_button");
        groupIconItems.push_back(groupIcon);
        
        //Group button.
        ButtonGuiItem* groupButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        groupButton->forceSquare = true;
        groupButton->onDraw =
        [this, groupButton] (const DrawInfo & draw) {
            float juicyGrowAmount = groupButton->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicyGrowAmount,
                "", game.sysContent.fntStandard, COLOR_WHITE,
                groupButton->focused, 0.0f, draw.tint
            );
        };
        groupButton->onActivate =
        [this, t, groupButton] (const Point&) {
            doButtonLogic(true, t, false);
        };
        groupButton->onMenuSNAction =
        [this, t] (PLAYER_ACTION_TYPE playerActionId) {
            return doButtonSNLogic(playerActionId, t);
        };
        groupButton->canAutoRepeat = true;
        groupButton->focusableFromSN = false;
        groupButton->onGetTooltip =
        [this, t] () {
            OnionMenuPikminType* tPtr = &this->types[t];
            return
                "Call " + getTransferAmountStr() + " " +
                tPtr->pikType->name + " to the group.";
        };
        listItem->addChild(groupButton);
        gui.addItem(groupButton, "group_button");
        groupButtonItems.push_back(groupButton);
        
        //Group amount text.
        GuiItem* groupAmountText = new GuiItem(false);
        groupAmountText->onDraw =
            [this, t, groupAmountText]
        (const DrawInfo & draw) {
            OnionMenuPikminType* tPtr = &this->types[t];
            
            size_t realGroupAmount =
                this->leaderPtr->group->getAmountByType(tPtr->pikType);
                
            drawFilledRoundedRatioRectangle(
                draw.center, draw.size, 0.30f,
                tintColor(al_map_rgba(188, 230, 230, 128), draw.tint)
            );
            
            ALLEGRO_COLOR color = draw.tint;
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
        listItem->addChild(groupAmountText);
        gui.addItem(groupAmountText, "group_amount");
        groupAmountItems.push_back(groupAmountText);
        
        //Full type item.
        GuiItem* fullTypeItem = new GuiItem();
        fullTypeItem->onMenuSNAction =
        [this, t] (PLAYER_ACTION_TYPE playerActionId) {
            return doButtonSNLogic(playerActionId, t);
        };
        fullTypeItem->focusableFromMouse = false;
        fullTypeItem->onGetTooltip =
        [this, t] () {
            OnionMenuPikminType* tPtr = &this->types[t];
            return
                "Call or store " + getTransferAmountStr() + " " +
                tPtr->pikType->name + ".";
        };
        listItem->addChild(fullTypeItem);
        gui.addItem(fullTypeItem, "full_type");
        fullTypeItems.push_back(fullTypeItem);
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
            onionAllButton->focused, 0.0f, draw.tint
        );
    };
    onionAllButton->onActivate =
    [this] (const Point&) {
        doButtonLogic(false, 0, false);
    };
    onionAllButton->onMenuSNAction =
    [this] (PLAYER_ACTION_TYPE playerActionId) {
        return doButtonSNLogic(playerActionId, 0);
    };
    onionAllButton->canAutoRepeat = true;
    onionAllButton->focusableFromSN = false;
    onionAllButton->onGetTooltip =
    [this] () {
        return
            "Store " + getTransferAmountStr() +
            " Pikmin of each type inside.";
    };
    listItem->addChild(onionAllButton);
    gui.addItem(onionAllButton, "onion_all_button");
    
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
            groupAllButton->focused, 0.0f, draw.tint
        );
    };
    groupAllButton->onActivate =
    [this] (const Point&) {
        doButtonLogic(true, 0, false);
    };
    groupAllButton->onMenuSNAction =
    [this] (PLAYER_ACTION_TYPE playerActionId) {
        return doButtonSNLogic(playerActionId, 0);
    };
    groupAllButton->canAutoRepeat = true;
    groupAllButton->focusableFromSN = false;
    groupAllButton->onGetTooltip =
    [this] () {
        return
            "Call " + getTransferAmountStr() +
            " Pikmin of each type to the group.";
    };
    listItem->addChild(groupAllButton);
    gui.addItem(groupAllButton, "group_all_button");
    
    //All types item.
    fullTypeAllItem = new GuiItem();
    fullTypeAllItem->onMenuSNAction =
    [this] (PLAYER_ACTION_TYPE playerActionId) {
        return doButtonSNLogic(playerActionId, 0);
    };
    fullTypeAllItem->focusableFromMouse = false;
    fullTypeAllItem->onGetTooltip =
    [this] () {
        return
            "Call or store " + getTransferAmountStr() + " " +
            "Pikmin of each type.";
    };
    listItem->addChild(fullTypeAllItem);
    gui.addItem(fullTypeAllItem, "full_type_all");
    fullTypeItems.push_back(fullTypeAllItem);
    
    //List padding dummy item.
    listPaddingDummyItem = new GuiItem();
    listItem->addChild(listPaddingDummyItem);
    gui.addItem(listPaddingDummyItem);
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    update();
    gui.setFocusedItem(okButton, true);
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
 * @brief Returns whether it's possible to add one Pikmin from the
 * Onion to the group.
 *
 * @param typeIdx Index of the Onion's Pikmin type.
 * @return The transfer's result.
 */
ONION_TRANSFER_RESULT OnionMenu::canAddToGroup(size_t typeIdx) {
    size_t realOnionAmount =
        nestPtr->getAmountByType(nestPtr->nestType->pikTypes[typeIdx]);
        
    //First, check if there are enough in the Onion to take out.
    if((signed int) (realOnionAmount - types[typeIdx].delta) <= 0) {
        return ONION_TRANSFER_RESULT_NONE_IN_ONION;
    }
    
    //Next, check if the addition won't make the field amount hit the limit.
    int totalDelta = 0;
    for(size_t t = 0; t < types.size(); t++) {
        totalDelta += types[t].delta;
    }
    if(
        game.states.gameplay->mobs.pikmin.size() + totalDelta >=
        game.curArea->getMaxPikminInField()
    ) {
        return ONION_TRANSFER_RESULT_FIELD_FULL;
    }
    
    //All good!
    return ONION_TRANSFER_RESULT_OK;
}


/**
 * @brief Returns whether it's possible to add one Pikmin from the
 * group to the Onion.
 *
 * @param typeIdx Index of the Onion's Pikmin type.
 * @return The transfer's result.
 */
ONION_TRANSFER_RESULT OnionMenu::canAddToOnion(size_t typeIdx) {
    size_t realGroupAmount =
        leaderPtr->group->getAmountByType(nestPtr->nestType->pikTypes[typeIdx]);
        
    //First, check if there are enough in the group to put in.
    if((signed int) (realGroupAmount + types[typeIdx].delta) <= 0) {
    
        return ONION_TRANSFER_RESULT_NONE_IN_GROUP;
    }
    
    //All good!
    return ONION_TRANSFER_RESULT_OK;
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
    
    startClosing();
}


/**
 * @brief Does all of the logic for either an Onion or a group button
 * having been pressed, no matter which way that happened.
 *
 * @param toGroup Whether the transfer is to the group or to the Onion.
 * @param typeIdx Index of the Onion's Pikmin type, if applicable.
 * @param fromSN Whether it came from spatial navigation.
 */
void OnionMenu::doButtonLogic(
    bool toGroup, size_t typeIdx, bool fromSN
) {
    GuiItem* relevantItem = nullptr;
    if(toGroup) {
        if(selectAll) {
            relevantItem = groupAllButton;
        } else {
            relevantItem = groupButtonItems[typeIdx];
        }
    } else {
        if(selectAll) {
            relevantItem = onionAllButton;
        } else {
            relevantItem = onionButtonItems[typeIdx];
        }
    }
    
    ONION_TRANSFER_RESULT result = transfer(toGroup, typeIdx);
    
    if(!fromSN) {
        if(result != ONION_TRANSFER_RESULT_OK) {
            relevantItem->playFailSound = true;
        }
    } else {
        if(result == ONION_TRANSFER_RESULT_OK) {
            game.audio.addNewUiSoundSource(
                game.sysContent.sndMenuActivate, { .volume = 0.75f }
            );
        } else {
            game.audio.addNewUiSoundSource(
                game.sysContent.sndMenuFail, { .volume = 0.75f }
            );
        }
    }
}


/**
 * @brief Does all of the logic for the player having chosen to store or
 * call a Pikmin via directional menu inputs.
 *
 * @param playerActionId Player action.
 * @param typeIdx Index of the Onion's Pikmin type, if applicable.
 * @return Whether the action was consumed.
 */
bool OnionMenu::doButtonSNLogic(
    PLAYER_ACTION_TYPE playerActionId, size_t typeIdx
) {
    if(playerActionId == PLAYER_ACTION_TYPE_MENU_UP) {
        doButtonLogic(false, typeIdx, true);
        return true;
    } else if(playerActionId == PLAYER_ACTION_TYPE_MENU_DOWN) {
        doButtonLogic(true, typeIdx, true);
        return true;
    }
    return false;
}


/**
 * @brief Returns "ten" or "one", depending on the amount of Pikmin slated to
 * be transferred each transfer. Used in tooltips.
 *
 * @return "ten" or "one".
 */
string OnionMenu::getTransferAmountStr() {
    return changeTen ? "ten" : "one";
}


/**
 * @brief Makes the Onion and group buttons juicy grow.
 */
void OnionMenu::growButtons() {
    for(size_t t = 0; t < types.size(); t++) {
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
        fullTypeItems[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
    }
    onionAllButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
    groupAllButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
    fullTypeAllItem->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void OnionMenu::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!gui.shouldHandleEvents()) return;
    if(closing) return;
    gui.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void OnionMenu::handlePlayerAction(const Inpution::Action& action) {
    if(!gui.shouldHandleEvents()) return;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_ONION_CHANGE_10: {
        if(action.value >= 0.5f) {
            changeTenButton->activate();
        }
        break;
    } case PLAYER_ACTION_TYPE_ONION_SELECT_ALL: {
        if(action.value >= 0.5f) {
            selectAllButton->activate();
        }
        break;
    } case PLAYER_ACTION_TYPE_MENU_OK: {
        if(action.value >= 0.5f) {
            okButton->activate();
        }
        break;
    }
    }
    
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
    gui.responsive = false;
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
        totalDelta - (int) game.curArea->getMaxPikminInField();
        
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
 * @brief Toggles the "change 10" mode.
 *
 * @return Whether it succeeded.
 */
bool OnionMenu::toggleChangeTen() {
    changeTenButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
    );
    changeTen = !changeTen;
    return true;
}


/**
 * @brief Toggles the "select all" mode.
 *
 * @return Whether it succeeded.
 */
bool OnionMenu::toggleSelectAll() {
    if(types.size() <= 1) return false;
    
    selectAll = !selectAll;
    growButtons();
    selectAllButton->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
    );
    
    update();
    
    return true;
}


/**
 * @brief Transfers some Pikmin, if possible. This moves either one or ten
 * depending on changeTen, moves from either one type or from each type
 * depending on selectAll, and moves to either direction.
 *
 * @param toGroup Whether the transfer is to the group or to the Onion.
 * @param typeIdx Index of the Onion's Pikmin type, if applicable.
 * @return Success if any transfer succeeded, otherwise the failure reason.
 */
ONION_TRANSFER_RESULT OnionMenu::transfer(bool toGroup, size_t typeIdx) {
    bool success = false;
    ONION_TRANSFER_RESULT latestError = ONION_TRANSFER_RESULT_OK;
    const size_t amountToTransfer = changeTen ? 10 : 1;
    const size_t firstTypeIdx = selectAll ? 0 : typeIdx;
    const size_t lastTypeIdx = selectAll ? types.size() - 1 : typeIdx;
    
    for(size_t p = 0; p < amountToTransfer; p++) {
        for(size_t t = firstTypeIdx; t <= lastTypeIdx; t++) {
        
            ONION_TRANSFER_RESULT oneResult =
                toGroup ? canAddToGroup(t) : canAddToOnion(t);
                
            switch(oneResult) {
            case ONION_TRANSFER_RESULT_OK: {
                if(toGroup) {
                    types[t].delta++;
                    onionAmountItems[t]->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    groupAmountItems[t]->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    fieldAmountText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                    );
                } else {
                    types[t].delta--;
                    onionAmountItems[t]->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    groupAmountItems[t]->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    fieldAmountText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                    );
                }
                success = true;
                break;
                
            } case ONION_TRANSFER_RESULT_NONE_IN_ONION: {
                makeGuiItemRed(onionAmountItems[t]);
                latestError = oneResult;
                break;
                
            } case ONION_TRANSFER_RESULT_NONE_IN_GROUP: {
                makeGuiItemRed(groupAmountItems[t]);
                latestError = oneResult;
                break;
                
            } case ONION_TRANSFER_RESULT_FIELD_FULL: {
                makeGuiItemRed(fieldAmountText);
                latestError = oneResult;
                break;
                
            }
            }
        }
    }
    
    return success ? ONION_TRANSFER_RESULT_OK : latestError;
}


/**
 * @brief Updates some things about the Onion's state, especially caches.
 */
void OnionMenu::update() {
    //Calculate size and position things.
    float columnWidth = onionButtonItems[0]->ratioSize.x;
    columnWidth = std::max(columnWidth, onionAmountItems[0]->ratioSize.x);
    columnWidth = std::max(columnWidth, groupButtonItems[0]->ratioSize.x);
    columnWidth = std::max(columnWidth, groupAmountItems[0]->ratioSize.x);
    columnWidth = std::max(columnWidth, fullTypeItems[0]->ratioSize.x);
    float visibleColWidthSums = columnWidth * ONION_MENU::NR_TYPES_VISIBLE;
    float visibleColPaddingSums = 1.0f - visibleColWidthSums;
    float columnPadding =
        visibleColPaddingSums / (ONION_MENU::NR_TYPES_VISIBLE + 1);
        
    float listWidth =
        columnWidth * types.size() + columnPadding * (types.size() + 1);
    float listStartX = 0.0f;
    if(listWidth < 1.0f) {
        listStartX = (1.0f - listWidth) / 2.0f;
    }
    
    //Assign the coordinates of each type GUI item.
    float curX = listStartX;
    for(size_t t = 0; t < types.size(); t++) {
        curX += columnPadding + columnWidth / 2.0f;
        onionIconItems[t]->ratioCenter.x = curX;
        onionButtonItems[t]->ratioCenter.x = curX;
        onionAmountItems[t]->ratioCenter.x = curX;
        groupIconItems[t]->ratioCenter.x = curX;
        groupButtonItems[t]->ratioCenter.x = curX;
        groupAmountItems[t]->ratioCenter.x = curX;
        fullTypeItems[t]->ratioCenter.x = curX;
        curX += columnWidth / 2.0f;
    }
    
    //Make all relevant GUI items in/active.
    for(size_t t = 0; t < types.size(); t++) {
        onionButtonItems[t]->visible = !selectAll;
        onionButtonItems[t]->focusable = !selectAll;
        groupButtonItems[t]->visible = !selectAll;
        groupButtonItems[t]->focusable = !selectAll;
        fullTypeItems[t]->focusable = !selectAll;
    }
    onionAllButton->visible = selectAll;
    onionAllButton->focusable = selectAll;
    groupAllButton->visible = selectAll;
    groupAllButton->focusable = selectAll;
    fullTypeAllItem->visible = selectAll;
    fullTypeAllItem->focusable = selectAll;
    
    //Make the "all" buttons fit.
    float onionAllButtonX1 = FLT_MAX;
    float onionAllButtonX2 = -FLT_MAX;
    float groupAllButtonX1 = FLT_MAX;
    float groupAllButtonX2 = -FLT_MAX;
    for(size_t t = 0; t < types.size(); t++) {
        onionAllButtonX1 =
            std::min(
                onionAllButtonX1,
                onionButtonItems[t]->ratioCenter.x -
                onionButtonItems[t]->ratioSize.x / 2.0f
            );
        onionAllButtonX2 =
            std::max(
                onionAllButtonX2,
                onionButtonItems[t]->ratioCenter.x +
                onionButtonItems[t]->ratioSize.x / 2.0f
            );
        groupAllButtonX1 =
            std::min(
                groupAllButtonX1,
                groupButtonItems[t]->ratioCenter.x -
                groupButtonItems[t]->ratioSize.x / 2.0f
            );
        groupAllButtonX2 =
            std::max(
                groupAllButtonX2,
                groupButtonItems[t]->ratioCenter.x +
                groupButtonItems[t]->ratioSize.x / 2.0f
            );
    }
    float fullTypeAllButtonX1 = std::min(onionAllButtonX1, groupAllButtonX1);
    float fullTypeAllButtonX2 = std::max(onionAllButtonX2, groupAllButtonX2);
    onionAllButton->ratioCenter.x =
        (onionAllButtonX1 + onionAllButtonX2) / 2.0f;
    onionAllButton->ratioSize.x =
        (onionAllButtonX2 - onionAllButtonX1);
    groupAllButton->ratioCenter.x =
        (groupAllButtonX1 + groupAllButtonX2) / 2.0f;
    groupAllButton->ratioSize.x =
        (groupAllButtonX2 - groupAllButtonX1);
    fullTypeAllItem->ratioCenter.x =
        (fullTypeAllButtonX1 + fullTypeAllButtonX2) / 2.0f;
    fullTypeAllItem->ratioSize.x =
        (fullTypeAllButtonX2 - fullTypeAllButtonX1);
        
    //If the list has more than ONION_MENU::NR_TYPES_VISIBLE type, the final
    //type won't have padding to the right since the scrollbar calculations
    //only take into account actually used space. Let's adjust the dummy
    //padding GUI item to fix that.
    listPaddingDummyItem->ratioCenter.x = listStartX + listWidth / 2.0f;
    listPaddingDummyItem->ratioSize.x = listWidth;
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
