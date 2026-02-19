/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-game inventory classes and functions.
 */

#include <algorithm>

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"
#include "gameplay.h"

#include "inventory.h"


using DrawInfo = GuiItem::DrawInfo;


namespace INVENTORY {

//How many columns are visible by default.
const size_t COLUMNS = 3;

//How long to fade when opening/closing for.
const float FADE_DURATION = 0.2f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "inventory";

//How many rows exist.
const size_t ROWS = 2;

//Padding between item slots, in GUI width ratio.
const float SLOT_PADDING = 0.1f;

}


/**
 * @brief Constructs a new Inventory struct object.
 *
 * @param player Whose player this inventory belongs to.
 */
Inventory::Inventory(Player* player) :
    player(player) {
    
    for(size_t i = 0; i < game.inventoryItems.getAmount(); i++) {
        InventoryItemInstance item;
        item.dbIndex = i;
        items.push_back(item);
    }
    
    gui.ignoreInputOnAnimation = false;
    initGui();
    populateInventoryListGui();
    update();
}


/**
 * @brief Destroys the Inventory struct object.
 */
Inventory::~Inventory() {
}


/**
 * @brief Returns whether or not a given item can be used.
 *
 * @param iiPtr The item.
 * @return Whether it can be used.
 */
bool Inventory::canUseItem(InventoryItemInstance* iiPtr) {
    if(!player->leaderPtr) return false;
    
    InventoryItem* iPtr = game.inventoryItems.getByIndex(iiPtr->dbIndex);
    if(!iPtr->onUse) return false;
    if(iPtr->onGetAmount) {
        if(iPtr->onGetAmount(player) == 0) return false;
    }
    return true;
}


/**
 * @brief Closes the inventory.
 *
 */
void Inventory::close() {
    if(!isOpen) return;
    gui.responsive = false;
    gui.startAnimation(
        GUI_MANAGER_ANIM_FADE_OUT, INVENTORY::FADE_DURATION
    );
    isOpen = false;
    
    game.controls.ignoreMenuCloseActions();
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 * @return Whether it got handled.
 */
bool Inventory::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    return gui.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 * @return Whether it got handled.
 */
bool Inventory::handlePlayerAction(const Inpution::Action& action) {
    if(
        action.actionTypeId == PLAYER_ACTION_TYPE_INVENTORY &&
        action.value < 0.5f &&
        game.options.controls.fastInventory
    ) {
        GuiItem* focusedItem = gui.getFocusedItem();
        if(focusedItem) {
            focusedItem->activate();
        } else {
            requestClose();
        }
        return true;
    }
    return gui.handlePlayerAction(action);
}


/**
 * @brief Initializes the base inventory GUI.
 */
void Inventory::initGui() {
    DataNode* guiFile =
        &game.content.guiDefs.list[INVENTORY::GUI_FILE_NAME];
        
    gui.registerCoords("list",        50, 50, 32, 28);
    gui.registerCoords("list_scroll", 50, 67, 32,  2);
    gui.registerCoords("info",        57, 31, 18,  6);
    gui.registerCoords("close",       40, 31, 12,  6);
    gui.registerCoords("close_input", 34, 34,  4,  4);
    gui.registerCoords("extra_info",  57, 32, 18,  4);
    gui.readDataFile(guiFile);
    
    //Item list box.
    itemList = new ListGuiItem();
    itemList->horizontal = true;
    gui.addItem(itemList, "list");
    
    //Item list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->horizontal = true;
    listScroll->listItem = itemList;
    gui.addItem(listScroll, "list_scroll");
    
    //Item info text.
    itemInfoItem = new GuiItem();
    itemInfoItem->onDraw =
    [this] (const DrawInfo & draw) {
        if(focusedItemIdx == INVALID) return;
        InventoryItemInstance* iiPtr = &items[focusedItemIdx];
        InventoryItem* iPtr = game.inventoryItems.getByIndex(iiPtr->dbIndex);
        string progressText;
        if(iPtr->onGetExtraInfo) {
            progressText = iPtr->onGetExtraInfo(player);
        }
        drawText(
            iPtr->name, game.sysContent.fntStandard,
            progressText.empty() ?
            draw.center :
            Point(draw.center.x, draw.center.y - draw.size.y / 4.0f),
            Point(draw.size.x, draw.size.y * 0.50f),
            game.config.guiColors.smallHeader
        );
        if(!progressText.empty()) {
            drawText(
                progressText, game.sysContent.fntStandard,
                Point(draw.center.x, draw.center.y + draw.size.y / 4.0f),
                Point(draw.size.x, draw.size.y * 0.40f)
            );
        }
    };
    gui.addItem(itemInfoItem, "info");
    
    //Close button.
    gui.backItem =
        new ButtonGuiItem(
        "Close", game.sysContent.fntStandard, game.config.guiColors.back
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        requestClose();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Close inventory"; };
    gui.addItem(gui.backItem, "close");
    
    //Close input icon.
    guiCreateBackInputIcon(&gui, "close_input");
    
    //Finishing touches.
    gui.onFocusChanged =
    [this] () {
        focusedItemIdx = INVALID;
    };
    gui.responsive = false;
    gui.hideItems();
}


/**
 * @brief Opens the inventory.
 */
void Inventory::open() {
    if(isOpen) return;
    gui.responsive = true;
    gui.startAnimation(
        GUI_MANAGER_ANIM_FADE_IN, INVENTORY::FADE_DURATION
    );
    isOpen = true;
}


/**
 * @brief Populates the inventory's list GUI item with items for each item.
 */
void Inventory::populateInventoryListGui() {
    const float SLOT_WIDTH =
        (
            1.0f -
            INVENTORY::SLOT_PADDING * (INVENTORY::COLUMNS - 1)
        ) / (float) INVENTORY::COLUMNS;
    const float SLOT_HEIGHT =
        (
            1.0f -
            INVENTORY::SLOT_PADDING * (INVENTORY::ROWS - 1)
        ) / (float) INVENTORY::ROWS;
    int rowIdx = 0;
    int columnIdx = 0;
    Point slotCenter(SLOT_WIDTH / 2.0f, SLOT_HEIGHT / 2.0f);
    
    auto nextSlot =
    [&columnIdx, &rowIdx, &slotCenter, SLOT_WIDTH, SLOT_HEIGHT] () {
        rowIdx++;
        slotCenter.y += SLOT_HEIGHT + INVENTORY::SLOT_PADDING;
        if(rowIdx >= (int) INVENTORY::ROWS) {
            rowIdx = 0;
            slotCenter.y = SLOT_HEIGHT / 2.0f;
            columnIdx++;
            slotCenter.x += SLOT_WIDTH + INVENTORY::SLOT_PADDING;
        }
    };
    
    for(size_t i = 0; i < items.size(); i++) {
        InventoryItemInstance* iiPtr = &items[i];
        InventoryItem* iPtr = game.inventoryItems.getByIndex(iiPtr->dbIndex);
        
        //Item button.
        ButtonGuiItem* button =
            new ButtonGuiItem(
            "", game.sysContent.fntStandard
        );
        button->ratioCenter = slotCenter;
        button->ratioSize = Point(SLOT_WIDTH, SLOT_HEIGHT);
        button->forceSquare = true;
        button->onDraw =
        [this, button, iPtr] (const DrawInfo & draw) {
            button->defDrawCode(draw);
            ALLEGRO_COLOR bmpTint = draw.tint;
            if(!button->responsive) {
                bmpTint = tintColor(bmpTint, al_map_rgba(128, 128, 128, 128));
            }
            if(iPtr->icon) {
                drawBitmapInBox(
                    iPtr->icon, draw.center, draw.size * 0.8f,
                    true, 0.0f, bmpTint
                );
            }
            if(iPtr->onGetAmount) {
                drawText(
                    "x" + i2s(iPtr->onGetAmount(player)),
                    game.sysContent.fntCounter,
                    draw.center + draw.size / 2.0f,
                    Point(0.80f, 0.50f) * draw.size,
                    draw.tint, ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_BOTTOM
                );
            }
        };
        button->onActivate =
        [this, i] (const Point&) {
            tryUseItem(i);
        };
        button->onGetTooltip = [iPtr] () { return iPtr->name; };
        button->onFocused = [this, i] () { focusedItemIdx = i; };
        itemList->addChild(button);
        gui.addItem(button);
        
        iiPtr->button = button;
        
        nextSlot();
    }
    
    //Create any missing placeholders.
    int colPlaceholdersNeeded = (int) (INVENTORY::ROWS) - rowIdx;
    int placeholderColsNeeded = (int) (INVENTORY::COLUMNS) - (columnIdx + 1);
    int placeholdersNeeded =
        colPlaceholdersNeeded + placeholderColsNeeded * INVENTORY::ROWS;
        
    for(int p = 0; p < placeholdersNeeded; p++) {
        //Item placeholder item.
        GuiItem* placeholder = new GuiItem();
        placeholder->ratioCenter = slotCenter;
        placeholder->ratioSize = Point(SLOT_WIDTH, SLOT_HEIGHT);
        placeholder->forceSquare = true;
        placeholder->onDraw =
        [this] (const DrawInfo & draw) {
            drawTexturedBox(
                draw.center, draw.size, game.sysContent.bmpFrameBox,
                tintColor(mapAlpha(48), draw.tint)
            );
        };
        itemList->addChild(placeholder);
        gui.addItem(placeholder);
        
        nextSlot();
    }
    
    if(!itemList->children.empty()) {
        gui.setFocusedItem(itemList->children[0], true);
    }
}


/**
 * @brief Request to the leader that the inventory gets closed.
 */
void Inventory::requestClose() {
    if(!player->leaderPtr) return;
    if(!isOpen) return;
    player->leaderPtr->fsm.runEvent(LEADER_EV_CANCEL);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Inventory::tick(float deltaT) {
    if(!player) return;
    
    update();
    
    //Tick the GUI items proper.
    gui.tick(game.deltaT);
}


/**
 * @brief Tries to use an item.
 *
 * @param itemIdx Index of the item in the inventory.
 * @return Whether it was possible to use it.
 */
bool Inventory::tryUseItem(size_t itemIdx) {
    InventoryItemInstance* iiPtr = &items[itemIdx];
    InventoryItem* iPtr = game.inventoryItems.getByIndex(iiPtr->dbIndex);
    if(canUseItem(iiPtr)) {
        iPtr->onUse(player);
        return true;
    }
    return false;
}


/**
 * @brief Updates the state of the inventory items.
 */
void Inventory::update() {
    for(size_t i = 0; i < items.size(); i++) {
        InventoryItemInstance* iiPtr = &items[i];
        iiPtr->button->responsive = canUseItem(iiPtr);
    }
}


/**
 * @brief Use an item via a shortcut.
 *
 * @param itemInternalName Internal name of the item to use.
 * @return Whether it succeeded.
 */
bool Inventory::useShortcut(const string& itemInternalName) {
    for(size_t i = 0; i < items.size(); i++) {
        InventoryItemInstance* iiPtr = &items[i];
        InventoryItem* iPtr = game.inventoryItems.getByIndex(iiPtr->dbIndex);
        if(iPtr->iName == itemInternalName) {
            return tryUseItem(i);
        }
    }
    return false;
}
