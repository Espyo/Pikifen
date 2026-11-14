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
 * @brief Returns the reference center coordinates,
 * i.e. used when not animating.
 *
 * @return The center.
 */
void DumbGui::Item::getReferenceCenter(float* outX, float* outY) const {
    if(parent) {
        float parentWidth, parentHeight;
        parent->getReferenceSize(&parentWidth, &parentHeight);
        parentWidth -= parent->paddingPx * 2.0f;
        parentHeight -= parent->paddingPx * 2.0f;
        float parentCenterX, parentCenterY;
        parent->getReferenceCenter(&parentCenterX, &parentCenterY);
        
        float resultX = rawCenterRatioX * parentWidth;
        resultX += parentCenterX - parentWidth / 2.0f;
        resultX -= parentWidth * parent->offsetRatioX;
        float resultY = rawCenterRatioY * parentHeight;
        resultY += parentCenterY - parentHeight / 2.0f;
        resultY -= parentHeight * parent->offsetRatioY;
        
        *outX = resultX;
        *outY = resultY;
        
    } else {
        float resultX = rawCenterRatioX * manager->getWindowWidth();
        float resultY = rawCenterRatioY * manager->getWindowHeight();
        
        *outX = resultX;
        *outY = resultY;
        
    }
}


/**
 * @brief Returns the reference width and height, i.e. used when not animating.
 *
 * @return The size.
 */
void DumbGui::Item::getReferenceSize(float* outWidth, float* outHeight) const {
    float multX = 0.0f;
    float multY = 0.0f;
    
    if(parent) {
        float parentWidth, parentHeight;
        parent->getReferenceSize(&parentWidth, &parentHeight);
        multX = parentWidth - (parent->paddingPx * 2.0f);
        multY = parentHeight - (parent->paddingPx * 2.0f);
    } else {
        multX = manager->getWindowWidth();
        multY = manager->getWindowHeight();
    }
    
    float finalWidth = rawWidthRatio * multX;
    float finalHeight = rawHeightRatio * multY;
    
    if(forceSquare) {
        finalWidth = finalWidth < finalHeight ? finalWidth : finalHeight;
        finalHeight = finalWidth;
    }
    
    *outWidth = finalWidth;
    *outHeight = finalHeight;
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
 * @brief Returns the current height of the program window.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The height.
 */
int DumbGui::Manager::getWindowHeight() const {
    return 100;
}


/**
 * @brief Returns the current width of the program window.
 * This should be overridden by a derived class since it does not do anything.
 *
 * @return The width.
 */
int DumbGui::Manager::getWindowWidth() const {
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

