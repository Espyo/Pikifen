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


//Types of items in the inventory.
enum INVENTORY_ITEM_TYPE {

    //Spray.
    INVENTORY_ITEM_TYPE_SPRAY,
    
};


/**
 * @brief Represents an item in the inventory.
 *
 */
struct InventoryItem {

    //--- Members ---
    
    //Type.
    INVENTORY_ITEM_TYPE type = INVENTORY_ITEM_TYPE_SPRAY;
    
    //Icon.
    ALLEGRO_BITMAP* icon = nullptr;
    
    //Name.
    string name;
    
    //Callback for when we need its current amount. nullptr if it doesn't use
    //amounts.
    std::function<size_t(void)> onGetAmount = nullptr;
    
    //Callback for when we need to use the item.
    std::function<void(void)> onUse = nullptr;
    
    //GUI button.
    ButtonGuiItem* button = nullptr;
    
};


/**
 * @brief Holds information about the in-game inventory GUI.
 */
struct Inventory {

    //--- Members ---
    
    //GUI manager.
    GuiManager gui;
    
    //Whose player this inventory belongs to.
    Player* player = nullptr;
    
    //List of items, in order.
    vector<InventoryItem> items;
    
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
    
    bool canUseItem(InventoryItem* iPtr);
    void initGui();
    void populateInventory();
    void populateInventoryListGui();
    void tryUseItem(size_t itemIdx);
    void update();
    
};
