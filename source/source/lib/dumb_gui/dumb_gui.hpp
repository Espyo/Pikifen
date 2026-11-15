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
    float rawCenterRatioX = 0.0f;
    
    //Raw center Y coordinate, in parent ratio.
    float rawCenterRatioY = 0.0f;
    
    //Raw width, in parent ratio.
    float rawRatioWidth = 0.0f;
    
    //Raw height, in parent ratio.
    float rawRatioHeight = 0.0f;
    
    
    //What item it is placed inside of, if any.
    Item* parent = nullptr;
    
    //List of children items, that are placed inside this one, if any.
    vector<Item*> children;
    
    //Horizontal offset of the children items, in size ratio, if any.
    float offsetRatioX = 0.0f;
    
    //Vertical offset of the children items, in size ratio, if any.
    float offsetRatioY = 0.0f;
    
    //Padding amount around the children items, in pixels.
    float pxPadding = 0.0f;
    
    //Whether to clip the children items' drawing to the confines of the item.
    bool clipChildren = true;
    
    
    //Force its final dimensions to be square, such that they fit inside
    //the reference dimensions.
    bool forceSquare = false;
    
    
    //--- Public function declarations ---
    
    virtual ~Item() = default;
    
    virtual void getReferencePxCenter(float* outPxX, float* outPxY) const;
    virtual void getReferencePxSize(
        float* outPxWidth, float* outPxHeight
    ) const;
    
    virtual bool addChild(Item* item);
    virtual bool deleteAllChildren();
    virtual float getChildrenRatioSpan(bool horizontal = false) const;
    virtual bool removeChild(Item* item);
    
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
    
    virtual ~Manager() = default;
    
    virtual bool addItem(Item* item);
    virtual bool removeItem(Item* item);
    
    virtual int getWindowPxWidth() const;
    virtual int getWindowPxHeight() const;
    
};


#pragma endregion


};
