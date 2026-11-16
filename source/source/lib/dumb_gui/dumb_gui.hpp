/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the Dumb GUI middleware.
 * Please read the included readme file.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>


using std::function;
using std::string;
using std::vector;


namespace DumbGui {


class Manager;


#pragma region Constants


extern const float INHERENT_PADDING_PX;
extern const int WINDOW_DEFAULT_PX_HEIGHT;
extern const int WINDOW_DEFAULT_PX_WIDTH;


#pragma endregion
#pragma region Misc. structs


//Information about how an item should be drawn.
struct DrawInfo {

    //Center X coordinate, in window pixels.
    float centerPxX = 0.0f;
    
    //Center Y coordinate, in window pixels.
    float centerPxY = 0.0f;
    
    //Width, in window pixels.
    float sizePxW = 0.0f;
    
    //Height, in window pixels.
    float sizePxH = 0.0f;
    
    //Angle, in radians.
    float angle = 0.0f;
    
    //Tint color, in RGBA.
    unsigned char tint[4] = { 255, 255, 255, 255};
    
};


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
    float rawSizeRatioW = 0.0f;
    
    //Raw height, in parent ratio.
    float rawSizeRatioH = 0.0f;
    
    
    //What to do when it's time to draw it.
    //First argument is the draw data.
    std::function<void(const DrawInfo& draw)> onDraw = nullptr;
    
    //What to do when it's time to tick one frame.
    //First argument is the time step between frames.
    function<void(float deltaT)> onTick = nullptr;
    
    //What to do when the item is activated.
    //First argument is the cursor's X coordinate, in window pixels.
    //Second argument is the cursor's Y coordinate, in window pixels.
    std::function<void(float cursorPxX, float cursorPxY)> onActivate = nullptr;
    
    //What to do when it gets focused.
    function<void()> onFocused = nullptr;
    
    //What to do when the mouse cursor is on top of it this frame.
    //First argument is the cursor's X coordinate, in window pixels.
    //Second argument is the cursor's Y coordinate, in window pixels.
    function<void(float cursorPxX, float cursorPxY)> onMouseOver = nullptr;
    
    //What to do when its tooltip needs to be retrieved.
    //Returns the tooltip's string.
    std::function<string()> onGetTooltip = nullptr;
    
    
    //Is it currently visible?
    bool visible = true;
    
    //Is it currently responsive?
    bool responsive = true;
    
    //Can it be focused?
    bool focusable = false;
    
    //Is it currently the focused item?
    bool focused = false;
    
    
    //What item it is placed inside of, if any.
    Item* parent = nullptr;
    
    //List of children items, that are placed inside this one, if any.
    vector<Item*> children;
    
    //Horizontal offset of the children items, in size ratio, if any.
    float offsetRatioX = 0.0f;
    
    //Vertical offset of the children items, in size ratio, if any.
    float offsetRatioY = 0.0f;
    
    //Padding amount around the children items, at the parent item's
    //edges, in pixels.
    float edgePaddingPx = 0.0f;
    
    //Whether to clip the children items' drawing to the confines of the item.
    bool clipChildren = true;
    
    
    //Is this item focusable from the mouse?
    bool focusableFromMouse = true;
    
    //Drawing layer. The lower the number, the sooner it'll be drawn.
    unsigned char drawingLayer = 127;
    
    //Force its final dimensions to be square, such that they fit inside
    //the reference dimensions.
    bool forceSquare = false;
    
    
    //--- Public function declarations ---
    
    explicit Item(bool focusable = false);
    virtual ~Item() = default;
    
    virtual void getReferencePxCenter(
        float* outCenterPxX, float* outCenterPxY
    ) const;
    virtual void getReferencePxSize(
        float* outSizePxW, float* outSizePxH
    ) const;
    
    virtual bool activate(float cursorPxX = 0.0f, float cursorPxY = 0.0f);
    virtual bool isMouseOn(
        float cursorPxX = 0.0f, float cursorPxY = 0.0f
    ) const;
    virtual bool isResponsive() const;
    virtual bool isVisible() const;
    
    virtual bool addChild(Item* item);
    virtual bool deleteAllChildren();
    virtual float getChildrenSpanRatio(bool horizontal = false) const;
    virtual bool removeChild(Item* item);
    
    virtual bool draw(const DrawInfo& draw);
    
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
    
    //Is it currently responding to input?
    bool responsive = true;
    
    
    //What to do when the currently focused item changes.
    std::function<void()> onFocusChanged = nullptr;
    
    //What to do when the game window's dimensions need to be obtained.
    //First argument is a pointer that receives the window's width in pixels.
    //Second argument is a pointer that receives the window's height in pixels.
    std::function<void(int*, int*)> onGetWindowSizePx = nullptr;
    
    //What to do when the current drawing clipping rectangle needs to
    //be obtained.
    //First argument is a pointer that receives the current X coordinate
    //of the top-left corner.
    //Second argument is a pointer that receives the current Y coordinate
    //of the top-left corner.
    //Third argument is a pointer that receives the current width.
    //Fourth argument is a pointer that receives the current height.
    std::function<void(int*, int*, int*, int*)> onGetClippingRectangle =
        nullptr;
        
    //What to do when the current drawing clipping rectangle needs to be set.
    //First argument is the X coordinate of the top-left corner.
    //Second argument is the Y coordinate of the top-left corner.
    //Third argument is the width.
    //Fourth argument is the height.
    std::function<void(int, int, int, int)> onSetClippingRectangle =
        nullptr;
        
        
    //--- Public function declarations ---
    
    virtual ~Manager() = default;
    
    virtual bool addItem(Item* item);
    virtual bool removeItem(Item* item);
    
    virtual bool draw();
    virtual bool tick(float deltaT);
    
    virtual bool handleMouseMovement(int mousePxX, int mousePxY);
    virtual bool handleMouseClick(int mousePxX, int mousePxY);
    virtual bool shouldHandleEvents() const;
    
    virtual Item* getFocusedItem() const;
    virtual bool setFocusedItem(Item* item);
    virtual string getCurrentTooltip() const;
    
    virtual bool hideItems();
    virtual bool showItems();
    
    virtual bool getItemDrawInfo(Item* item, DrawInfo* draw) const;
    
    virtual bool destroy();
    
    
protected:

    //--- Protected members ---
    
    //Are the items currently visible?
    bool visible = true;
    
    //Which item is currently focused.
    Item* focusedItem = nullptr;
    
};


#pragma endregion


};
