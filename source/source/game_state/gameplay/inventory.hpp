/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the in-game inventory class and
 * in-game inventory-related functions.
 */

#pragma once

#include <functional>

#include "../../content/other/gui.hpp"
#include "../../util/drawing_utils.hpp"


namespace INVENTORY {
extern const size_t COLUMNS;
extern const float FADE_DURATION;
extern const string GUI_FILE_NAME;
extern const size_t ROWS;
extern const float SLOT_PADDING;
}


class Player;


/**
 * @brief Represents an instance of an item in the inventory.
 */
class InventoryItemInstance {

public:

    //--- Public members ---
    
    //Index of the item in the database of inventory items.
    size_t dbIdx = INVALID;
    
    //GUI button.
    ButtonGuiItem* button = nullptr;
    
};


/**
 * @brief Holds information about the player's in-game inventory GUI.
 */
class Inventory {

public:

    //--- Public members ---
    
    //GUI manager.
    GuiManager gui;
    
    //Whose player this inventory belongs to.
    Player* player = nullptr;
    
    //List of items, in order.
    vector<InventoryItemInstance> items;
    
    //Inventory list GUI item.
    ListGuiItem* itemList = nullptr;
    
    //Item information GUI item.
    GuiItem* itemInfoItem = nullptr;
    
    //Index of the currently focused item. INVALID for none.
    size_t focusedItemIdx = INVALID;
    
    //Is it currently open?
    bool isOpen = false;
    
    
    //--- Public function declarations ---
    
    Inventory(Player* player);
    ~Inventory();
    void close();
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    bool handlePlayerAction(const Inpution::Action& action);
    void open();
    void requestClose();
    void tick(float deltaT);
    bool useShortcut(const string& itemInternalName);
    
    private:
    
    //--- Private function declarations ---
    
    bool canUseItem(InventoryItemInstance* iPtr);
    void initGui();
    void populateInventoryListGui();
    bool tryUseItem(size_t itemIdx);
    void update();
    
};
