/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the Dumb GUI middleware.
 * Please read the included readme file.
 */

#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>


using std::function;
using std::map;
using std::string;
using std::vector;


namespace DumbGui {


class Manager;


#pragma region Constants




#pragma endregion
#pragma region GUI item


/**
 * @brief An item in the GUI. This can be a HUD element, a button,
 * some text, etc.
 */
class Item {

public:

    //--- Public members ---
    
    //What GUI manager it belongs to, if any.
    Manager* manager = nullptr;
    
    //Raw center X coordinate, in parent ratio.
    float centerRatioX = 0.0f;
    
    //Raw center Y coordinate, in parent ratio.
    float centerRatioY = 0.0f;
    
    //Raw width, in parent ratio.
    float widthRatio = 0.0f;
    
    //Raw height, in parent ratio.
    float heightRatio = 0.0f;
    
};


#pragma endregion
#pragma region Manager


/**
 * @brief GUI manager.
 *
 * It is responsible for holding information about all GUI
 * items present on the game window, managing their coordinates, which one is
 * focused, ordering them to be rendered or to handle being activated, etc.
 * Due to the system's flexibility, it can be used both to manage the game's
 * heads-up display (HUD) during gameplay, as well as the interactable elements
 * of menus.
 */
class Manager {

public:

    //--- Public members ---
    
    //List of items.
    vector<Item*> items;
    
    
    //--- Public function declarations ---
    
    virtual bool addItem(Item* item);
    virtual bool removeItem(Item* item);
    
};


#pragma endregion


};
