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
 * @brief Returns the reference center coordinates, in pixels.
 * These are used, for instance, when not animating.
 *
 * @param outPxX The X coordinate is returned here.
 * @param outPxY The Y coordinate is returned here.
 */
void DumbGui::Item::getReferencePxCenter(float* outPxX, float* outPxY) const {
    if(parent) {
        float parentPxWidth, parentPxHeight;
        parent->getReferencePxSize(&parentPxWidth, &parentPxHeight);
        parentPxWidth -= parent->pxPadding * 2.0f;
        parentPxHeight -= parent->pxPadding * 2.0f;
        
        float parentCenterPxX, parentCenterPxY;
        parent->getReferencePxCenter(&parentCenterPxX, &parentCenterPxY);
        
        float resultPxX = rawCenterRatioX * parentPxWidth;
        resultPxX += parentCenterPxX - parentPxWidth / 2.0f;
        resultPxX -= parentPxWidth * parent->offsetRatioX;
        
        float resultPxY = rawCenterRatioY * parentPxHeight;
        resultPxY += parentCenterPxY - parentPxHeight / 2.0f;
        resultPxY -= parentPxHeight * parent->offsetRatioY;
        
        *outPxX = resultPxX;
        *outPxY = resultPxY;
        
    } else {
        float resultPxX = rawCenterRatioX * manager->getWindowPxWidth();
        float resultPxY = rawCenterRatioY * manager->getWindowPxHeight();
        
        *outPxX = resultPxX;
        *outPxY = resultPxY;
        
    }
}


/**
 * @brief Returns the reference width and height, in pixels.
 * These are used, for instance, when not animating.
 *
 * @param outPxWidth The width is returned here.
 * @param outPxHeight The height is returned here.
 */
void DumbGui::Item::getReferencePxSize(
    float* outPxWidth, float* outPxHeight
) const {
    float multX = 0.0f;
    float multY = 0.0f;
    
    if(parent) {
        float parentPxWidth, parentPxHeight;
        parent->getReferencePxSize(&parentPxWidth, &parentPxHeight);
        multX = parentPxWidth - (parent->pxPadding * 2.0f);
        multY = parentPxHeight - (parent->pxPadding * 2.0f);
        
    } else {
        multX = manager->getWindowPxWidth();
        multY = manager->getWindowPxHeight();
        
    }
    
    float finalPxWidth = rawRatioWidth * multX;
    float finalPxHeight = rawRatioHeight * multY;
    
    if(forceSquare) {
        finalPxWidth =
            finalPxWidth < finalPxHeight ? finalPxWidth : finalPxHeight;
        finalPxHeight = finalPxWidth;
    }
    
    *outPxWidth = finalPxWidth;
    *outPxHeight = finalPxHeight;
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
 * @brief Returns the current height of the program window, in pixels.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The height.
 */
int DumbGui::Manager::getWindowPxHeight() const {
    return 100;
}


/**
 * @brief Returns the current width of the program window, in pixels.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The width.
 */
int DumbGui::Manager::getWindowPxWidth() const {
    return 100;
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


#pragma endregion

