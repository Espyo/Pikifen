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

#include "../../content/other/gui.h"
#include "../../util/drawing_utils.h"


namespace INVENTORY {
extern const size_t COLUMNS;
extern const float FADE_DURATION;
extern const string GUI_FILE_NAME;
extern const float SLOT_PADDING;
extern const size_t ROWS;
}


struct Player;


/**
 * @brief Represents an instance of an item in the inventory.
 */
struct InventoryItemInstance {

    //--- Members ---
    
    //Index of the item in the database of inventory items.
    size_t dbIndex = INVALID;
    
    //GUI button.
    ButtonGuiItem* button = nullptr;
    
};


/**
 * @brief Holds information about the player's in-game inventory GUI.
 */
struct Inventory {

    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    //Whose player this inventory belongs to.
    Player* player = nullptr;
    
    //List of items, in order.
    vector<InventoryItemInstance> items;
    
    //Inventory list GUI item.
    ListGuiItem* itemList = nullptr;
    
    //Is it currently open?
    bool isOpen = false;
    
    
    //--- Function declarations ---
    
    Inventory(Player* player);
    ~Inventory();
    void close();
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    bool handlePlayerAction(const Inpution::Action& action);
    void open();
    void requestClose();
    void tick(float deltaT);
    
    private:
    
    //--- Function declarations ---
    
    bool canUseItem(InventoryItemInstance* iPtr);
    void initGui();
    void populateInventoryListGui();
    bool tryUseItem(size_t itemIdx);
    void update();
    
};
