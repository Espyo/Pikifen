/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Source code for the Dumb GUI middleware.
 * Please read the included readme file.
 */

#include "dumb_gui.hpp"


#pragma region GUI item


/**
 * @brief Constructs a new GUI item object.
 *
 * @param focusable Can the item be focused by the user?
 */
DumbGui::Item::Item(bool focusable) :
    focusable(focusable) {
    
}


/**
 * @brief Activates the item.
 *
 * @param cursorPxX Mouse cursor X coordinate, if applicable.
 * @param cursorPxY Mouse cursor Y coordinate, if applicable.
 * @return Whether it could activate it.
 */
bool DumbGui::Item::activate(float cursorPxX, float cursorPxY) {
    if(!isResponsive()) return false;
    if(!onActivate) return false;
    
    onActivate(cursorPxX, cursorPxY);
    
    return true;
}


/**
 * @brief Adds an existing item to the list of children items.
 *
 * @param item Item to add as a child item.
 * @return Whether it succeeded.
 */
bool DumbGui::Item::addChild(Item* item) {
    if(!item) return false;
    children.push_back(item);
    item->parent = this;
    return true;
}


/**
 * @brief Removes and deletes all children items.
 *
 * @return Whether all of the deletions succeeded.
 */
bool DumbGui::Item::deleteAllChildren() {
    bool success = true;
    while(!children.empty()) {
        Item* iPtr = children[0];
        removeChild(iPtr);
        success &= manager->removeItem(iPtr);
        delete iPtr;
    }
    return success;
}


/**
 * @brief Draws the item, by calling its onDraw function.
 *
 * @param draw Data to draw with.
 * @return Whether it succeeded.
 */
bool DumbGui::Item::draw(const DrawInfo& draw) {
    if(!onDraw) return false;
    
    onDraw(draw);
    
    return true;
}


/**
 * @brief Returns the bottommost Y coordinate (in height ratio),
 * or the rightmost X coordinate (in width ratio),
 * of the item's children items.
 *
 * @param horizontal If true, check the horizontal reach (width). If false,
 * check the vertical reach (height).
 * @return The X or Y coordinate.
 */
float DumbGui::Item::getChildrenRatioSpan(bool horizontal) const {
    float span = 0.0f;
    for(size_t c = 0; c < children.size(); c++) {
        Item* cPtr = children[c];
        float cCoord =
            horizontal ?
            cPtr->rawCenterRatioX + (cPtr->rawSizeRatioW / 2.0f) :
            cPtr->rawCenterRatioY + (cPtr->rawSizeRatioH / 2.0f);
        span = std::max(span, cCoord);
    }
    return span;
}


/**
 * @brief Returns the reference center coordinates, in pixels.
 * These are used, for instance, when not animating.
 *
 * @param outCenterPxX The X coordinate is returned here.
 * @param outCenterPxY The Y coordinate is returned here.
 */
void DumbGui::Item::getReferencePxCenter(
    float* outCenterPxX, float* outCenterPxY
) const {
    if(parent) {
        float parentSizePxW, parentSizePxH;
        parent->getReferencePxSize(&parentSizePxW, &parentSizePxH);
        parentSizePxW -= parent->pxPadding * 2.0f;
        parentSizePxH -= parent->pxPadding * 2.0f;
        
        float parentCenterPxX, parentCenterPxY;
        parent->getReferencePxCenter(&parentCenterPxX, &parentCenterPxY);
        
        float resultPxX = rawCenterRatioX * parentSizePxW;
        resultPxX += parentCenterPxX - parentSizePxW / 2.0f;
        resultPxX -= parentSizePxW * parent->offsetRatioX;
        
        float resultPxY = rawCenterRatioY * parentSizePxH;
        resultPxY += parentCenterPxY - parentSizePxH / 2.0f;
        resultPxY -= parentSizePxH * parent->offsetRatioY;
        
        *outCenterPxX = resultPxX;
        *outCenterPxY = resultPxY;
        
    } else {
        float resultPxX = rawCenterRatioX * manager->getWindowSizePxW();
        float resultPxY = rawCenterRatioY * manager->getWindowSizePxH();
        
        *outCenterPxX = resultPxX;
        *outCenterPxY = resultPxY;
        
    }
}


/**
 * @brief Returns the reference width and height, in pixels.
 * These are used, for instance, when not animating.
 *
 * @param outSizePxW The width is returned here.
 * @param outSizePxH The height is returned here.
 */
void DumbGui::Item::getReferencePxSize(
    float* outSizePxW, float* outSizePxH
) const {
    float multX = 0.0f;
    float multY = 0.0f;
    
    if(parent) {
        float parentSizePxW, parentSizePxH;
        parent->getReferencePxSize(&parentSizePxW, &parentSizePxH);
        multX = parentSizePxW - (parent->pxPadding * 2.0f);
        multY = parentSizePxH - (parent->pxPadding * 2.0f);
        
    } else {
        multX = manager->getWindowSizePxW();
        multY = manager->getWindowSizePxH();
        
    }
    
    float finalSizePxW = rawSizeRatioW * multX;
    float finalSizePxH = rawSizeRatioH * multY;
    
    if(forceSquare) {
        finalSizePxW =
            finalSizePxW < finalSizePxH ? finalSizePxW : finalSizePxH;
        finalSizePxH = finalSizePxW;
    }
    
    *outSizePxW = finalSizePxW;
    *outSizePxH = finalSizePxH;
}


/**
 * @brief Returns whether the mouse cursor is on top of it.
 *
 * @param cursorPxX Mouse cursor X coordinate.
 * @param cursorPxY Mouse cursor Y coordinate.
 * @return Whether the cursor is on top.
 */
bool DumbGui::Item::isMouseOn(float cursorPxX, float cursorPxY) const {
    if(parent && !parent->isMouseOn(cursorPxX, cursorPxY)) {
        return false;
    }
    
    float refCenterPxX, refCenterPxY;
    float refSizePxW, refSizePxH;
    getReferencePxCenter(&refCenterPxX, &refCenterPxY);
    getReferencePxSize(&refSizePxW, &refSizePxH);
    return
        (
            cursorPxX >= refCenterPxX - refSizePxW * 0.5f &&
            cursorPxX <= refCenterPxX + refSizePxW * 0.5f &&
            cursorPxY >= refCenterPxY - refSizePxH * 0.5f &&
            cursorPxY <= refCenterPxY + refSizePxH * 0.5f
        );
}


/**
 * @brief Returns whether or not it is responsive, and also checks the parents.
 *
 * @return Whether it is responsive.
 */
bool DumbGui::Item::isResponsive() const {
    if(!responsive) return false;
    if(parent) return parent->isResponsive();
    return responsive;
}


/**
 * @brief Returns whether or not it is visible, and also checks the parents.
 *
 * @return Whether it is visible.
 */
bool DumbGui::Item::isVisible() const {
    if(!visible) return false;
    if(parent) return parent->isVisible();
    return true;
}


/**
 * @brief Removes an item from the list of children, without deleting it.
 *
 * @param item Child item to remove.
 * @return Whether it succeeded.
 */
bool DumbGui::Item::removeChild(Item* item) {
    for(size_t c = 0; c < children.size(); c++) {
        if(children[c] == item) {
            children.erase(children.begin() + c);
            item->parent = nullptr;
            return true;
        }
    }
    
    return false;
}


#pragma endregion
#pragma region GUI manager


/**
 * @brief Adds an existing item to the manager's list of items.
 *
 * @param item The new item.
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::addItem(Item* item) {
    if(!item) return false;
    
    items.push_back(item);
    item->manager = this;
    
    return true;
}


/**
 * @brief Destroys and deletes all items and information.
 *
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::destroy() {
    setFocusedItem(nullptr);
    for(size_t i = 0; i < items.size(); i++) {
        delete items[i];
    }
    items.clear();
    
    return true;
}


/**
 * @brief Draws all items.
 *
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::draw() {
    if(!visible) return false;
    
    int ocrX = 0;
    int ocrY = 0;
    int ocrW = 0;
    int ocrH = 0;
    
    //Sort them by layer.
    vector<DumbGui::Item*> drawingSortedItems = items;
    std::stable_sort(
        drawingSortedItems.begin(), drawingSortedItems.end(),
    [] (DumbGui::Item * i1, DumbGui::Item * i2) {
        return i1->drawingLayer < i2->drawingLayer;
    }
    );
    
    //Draw them.
    for(size_t i = 0; i < drawingSortedItems.size(); i++) {
        Item* iPtr = drawingSortedItems[i];
        
        DrawInfo draw;
        iPtr->getReferencePxCenter(&draw.centerPxX, &draw.centerPxY);
        iPtr->getReferencePxSize(&draw.sizePxW, &draw.sizePxH);
        
        if(!getItemDrawInfo(iPtr, &draw)) continue;
        
        if(iPtr->parent) {
            DrawInfo parentDraw;
            if(!getItemDrawInfo(iPtr->parent, &parentDraw)) {
                continue;
            }
            if(iPtr->parent->clipChildren) {
                getClippingRectangle(&ocrX, &ocrY, &ocrW, &ocrH);
                setClippingRectangle(
                    (parentDraw.centerPxX - parentDraw.sizePxW / 2.0f) + 1.0f,
                    (parentDraw.centerPxY - parentDraw.sizePxH / 2.0f) + 1.0f,
                    parentDraw.sizePxW - 2.0f,
                    parentDraw.sizePxH - 2.0f
                );
            }
        }
        
        iPtr->draw(draw);
        
        if(iPtr->parent) {
            if(iPtr->parent->clipChildren) {
                setClippingRectangle(ocrX, ocrY, ocrW, ocrH);
            }
        }
    }
    
    return true;
}


/**
 * @brief Returns the current clipping rectangle.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @param outX1 The top-left corner's X coordinate is returned here.
 * @param outY1 The top-left corner's Y coordinate is returned here.
 * @param outW The width is returned here.
 * @param outH The height is returned here.
 */
void DumbGui::Manager::getClippingRectangle(
    int* outX1, int* outY1, int* outW, int* outH
) const {
    *outX1 = 0;
    *outY1 = 0;
    *outW = 0;
    *outH = 0;
}


/**
 * @brief Returns the currently focused item's tooltip, if any.
 *
 * @return The tooltip.
 */
string DumbGui::Manager::getCurrentTooltip() const {
    if(!focusedItem) return string();
    if(!focusedItem->onGetTooltip) return string();
    return focusedItem->onGetTooltip();
}


/**
 * @brief Returns which item is currently focused.
 *
 * @return The item, or nullptr for none.
 */
DumbGui::Item* DumbGui::Manager::getFocusedItem() const {
    return focusedItem;
}


/**
 * @brief Returns a given item's drawing information.
 *
 * @param item What item to check.
 * @param draw The information on how to draw is returned here.
 * @return True if the item exists and is meant to be drawn, false otherwise.
 */
bool DumbGui::Manager::getItemDrawInfo(Item* item, DrawInfo* draw) const {
    if(!item->isVisible()) return false;
    if(item->rawSizeRatioW == 0.0f) return false;
    
    float finalCenterPxX, finalCenterPxY;
    float finalSizePxW, finalSizePxH;
    float finalAngle = 0.0f;
    float finalAlpha = 1.0f;
    
    item->getReferencePxCenter(&finalCenterPxX, &finalCenterPxY);
    item->getReferencePxSize(&finalSizePxW, &finalSizePxH);
    
    draw->centerPxX = finalCenterPxX;
    draw->centerPxY = finalCenterPxY;
    draw->sizePxW = finalSizePxW;
    draw->sizePxH = finalSizePxH;
    draw->angle = finalAngle;
    draw->tint[0] = 255;
    draw->tint[1] = 255;
    draw->tint[2] = 255;
    draw->tint[3] = finalAlpha * 255;
    
    return true;
}


/**
 * @brief Returns the current height of the program window, in pixels.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The height.
 */
int DumbGui::Manager::getWindowSizePxH() const {
    return 100;
}


/**
 * @brief Returns the current width of the program window, in pixels.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The width.
 */
int DumbGui::Manager::getWindowSizePxW() const {
    return 100;
}


/**
 * @brief Hides all items until an animation shows them again.
 *
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::hideItems() {
    if(!visible) return false;
    visible = false;
    return true;
}


/**
 * @brief Removes an item from the list, without deleting it.
 *
 * @param item Item to remove.
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::removeItem(Item* item) {
    if(!item) return false;
    bool success = false;
    
    for(size_t i = 0; i < items.size(); i++) {
        if(items[i] == item) {
            items.erase(items.begin() + i);
            item->manager = nullptr;
            success = true;
            break;
        }
    }
    
    return success;
}


/**
 * @brief Sets the current clipping rectangle.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @param x1 The top-left corner's X coordinate.
 * @param y1 The top-left corner's Y coordinate.
 * @param width The width.
 * @param height The height.
 */
void DumbGui::Manager::setClippingRectangle(
    int x1, int y1, int w, int h
) {
}


/**
 * @brief Sets the given item as the one that is focused, or none.
 *
 * @param item Item to focus, or nullptr for none.
 * @param silent If true, no sound effect will play.
 * Useful if you want the item to be focused not because of user input,
 * but because it's the default focused item when the GUI loads.
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::setFocusedItem(Item* item, bool silent) {
    if(focusedItem == item) return false;
    
    if(focusedItem) {
        focusedItem->focused = false;
    }
    focusedItem = item;
    if(focusedItem) {
        focusedItem->focused = true;
    }
    
    if(onFocusChanged) onFocusChanged();
    if(focusedItem) {
        if(focusedItem->onFocused) {
            focusedItem->onFocused();
        }
    }
    
    return true;
}


/**
 * @brief Shows all items, if they were hidden.
 *
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::showItems() {
    if(visible) return false;
    visible = true;
    return true;
}


/**
 * @brief Ticks the time of all items by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @return Whether it succeeded.
 */
bool DumbGui::Manager::tick(float deltaT) {
    //Tick all items.
    for(size_t i = 0; i < items.size(); i++) {
        Item* iPtr = items[i];
        if(iPtr->onTick) {
            iPtr->onTick(deltaT);
        }
    }
    
    return true;
}


#pragma endregion

