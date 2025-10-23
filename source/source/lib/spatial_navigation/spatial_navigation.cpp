/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Source code for the spatial navigation library.
 * Please read the included readme file.
 */

#include <algorithm>
#include <cmath>
#include <float.h>
#include <limits.h>

#include "spatial_navigation.h"


namespace SpatNav {


/**
 * @brief Destroys the interface object.
 */
Interface::~Interface() {
    clearItems();
}


/**
 * @brief Adds an item to the interface, or to the currently opened parent item.
 *
 * @param id Identifier. This can be anything you want, but ensure every item
 * has a unique identifier, and that the value nullptr (0) is not used. You
 * can use a pointer to your own data type, or an index (starting at 1).
 * @param x X coordinate of its center.
 * @param y Y coordinate of its center.
 * @param w Width.
 * @param h Height.
 * @return Whether it succeeded.
 */
bool Interface::addItem(void* id, float x, float y, float w, float h) {
    if(items.find(id) != items.end()) return false;
    
    Item* newItem = new Item();
    newItem->id = id;
    newItem->x = x;
    newItem->y = y;
    newItem->w = w;
    newItem->h = h;
    items[id] = newItem;
    return true;
}


/**
 * @brief Checks if this item passes the heuristics tests.
 *
 * @param itemRelX X coordinate of its center, relative to the focus.
 * @param itemRelY Y coordinate of its center, relative to the focus.
 * @param itemRelW Width, relative to the focus.
 * @param itemRelH Height, relative to the focus.
 * @return Whether it passed.
 */
bool Interface::checkHeuristicsPass(
    double itemRelX, double itemRelY, double itemRelW, double itemRelH
) {
    if(
        heuristics.minBlindspotAngle >= 0.0f ||
        heuristics.maxBlindspotAngle >= 0.0f
    ) {
        //Check if it's between the blind spot angles.
        //We get the same result whether the Y is positive or negative,
        //so let's simplify things and make it positive.
        float itemRelAngle = (float) atan2((float) fabs(itemRelY), itemRelX);
        
        if(
            itemRelAngle >= heuristics.minBlindspotAngle &&
            itemRelAngle <= heuristics.maxBlindspotAngle
        ) {
            //If so, never let this item be chosen, no matter what. This is
            //useful to stop a list of items with no vertical variance from
            //picking another item when the direction is up, for instance.
            return false;
        }
    }
    
    return true;
}


/**
 * @brief Checks if an item that's behind the focus needs to be placed
 * in front of the focus. This behavior is what allows looping from the edges
 * of the interface.
 *
 * @param direction Current navigation direction.
 * @param itemRelX Relative X coordinate. It gets updated if needed.
 * @param limitX1 Top-left corner's X coordinate.
 * @param limitY1 Top-left corner's Y coordinate.
 * @param limitX2 Bottom-right corner's X coordinate.
 * @param limitY2 Bottom-right corner's Y coordinate.
 * @return Whether it had looped.
 */
bool Interface::checkLoopRelativeCoordinates(
    DIRECTION direction, double* itemRelX,
    double limitX1, double limitY1, double limitX2, double limitY2
) {
    if(*itemRelX >= 0.0f || !settings.loop) return false;
    
    if(direction == DIRECTION_DOWN || direction == DIRECTION_UP) {
        *itemRelX += limitY2 - limitY1;
    } else {
        *itemRelX += limitX2 - limitX1;
    }
    return true;
}


/**
 * @brief Deletes and clears all items.
 *
 * @return Whether it succeeded.
 */
bool Interface::clearItems() {
    if(items.empty()) return false;
    
    for(auto i : items) {
        delete i.second;
    }
    items.clear();
    
    parents.clear();
    children.clear();
    
    return true;
}


/**
 * @brief Navigates in a given direction.
 *
 * @param direction Navigation direction.
 * @param focusedItemId Identifier of the currently-focused item.
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @return Identifier of the newly-focused item.
 */
void* Interface::doNavigation(
    DIRECTION direction, void* focusedItemId,
    float focusX, float focusY, float focusW, float focusH
) {
    //Setup.
    
#ifdef SPAT_NAV_DEBUG
    lastNavInfo.clear();
#endif
    
    flattenItems();
    
    std::map<void*, ItemWithRelUnits> itemsWithRelUnits =
        getItemsWithRelativeUnits(
            direction, focusedItemId, focusX, focusY, focusW, focusH
        );
        
    double limitX1, limitY1, limitX2, limitY2;
    getLimits(&limitX1, &limitY1, &limitX2, &limitY2);
    
    std::map<void*, ItemWithRelUnits> nonLoopedItems;
    std::map<void*, ItemWithRelUnits> loopedItems;
    loopItems(
        itemsWithRelUnits, direction, limitX1, limitY1, limitX2, limitY2,
        &nonLoopedItems, &loopedItems
    );
    
    //Score them.
    double bestScore = FLT_MAX;
    void* bestItemId = nullptr;
    bool checkLoopedItems = true;
    getBestItem(nonLoopedItems, &bestScore, &bestItemId, false);
    
    if(!heuristics.singleLoopPass) {
        //If in two loop passes mode, only check the looped items if
        //the non-looped items gave us nothing.
        if(bestItemId != nullptr) checkLoopedItems = false;
    }
    
    if(checkLoopedItems) {
        getBestItem(loopedItems, &bestScore, &bestItemId, true);
    }
    
    //Finished!
    return bestItemId;
}


/**
 * @brief Flattens any children items that go outside their parents' limits.
 * This only affects children items that are completely outside, not partially.
 */
void Interface::flattenItems() {
    //Start with the top-level items.
    std::vector<Item*> list;
    for(const auto& i : items) {
        if(getItemParent(i.first)) continue;
        list.push_back(i.second);
    }
    flattenItemsInList(
        list,
        settings.limitX1, settings.limitY1, settings.limitX2, settings.limitY2
    );
}


/**
 * @brief Recursively flattens items in the given list.
 *
 * @param list List of items.
 * @param limitX1 Top-left X coordinate of the limit they must abide.
 * @param limitY1 Top-left Y coordinate of the limit they must abide.
 * @param limitX2 Bottom-right X coordinate of the limit they must abide.
 * @param limitY2 Bottom-right Y coordinate of the limit they must abide.
 */
void Interface::flattenItemsInList(
    std::vector<Item*> list,
    float limitX1, float limitY1, float limitX2, float limitY2
) {
    for(size_t i = 0; i < list.size(); i++) {
        //Flatten the item proper first.
        Item* iPtr = list[i];
        
        iPtr->flatX = iPtr->x;
        iPtr->flatY = iPtr->y;
        iPtr->flatW = iPtr->w;
        iPtr->flatH = iPtr->h;
        
        const float iX1 = iPtr->x - iPtr->w / 2.0f;
        const float iX2 = iPtr->x + iPtr->w / 2.0f;
        const float iY1 = iPtr->y - iPtr->h / 2.0f;
        const float iY2 = iPtr->y + iPtr->h / 2.0f;
        const float diffX1 = limitX1 - iX1;
        const float diffY1 = limitY1 - iY1;
        const float diffX2 = iX2 - limitX2;
        const float diffY2 = iY2 - limitY2;
        
        if(diffX1 > 0.0f) {
            iPtr->flatX = limitX1 - diffX1 * FLATTEN_FACTOR;
            iPtr->flatW = iPtr->w * FLATTEN_FACTOR;
        }
        if(diffY1 > 0.0f) {
            iPtr->flatY = limitY1 - diffY1 * FLATTEN_FACTOR;
            iPtr->flatH = iPtr->h * FLATTEN_FACTOR;
        }
        if(diffX2 > 0.0f) {
            iPtr->flatX = limitX2 + diffX2 * FLATTEN_FACTOR;
            iPtr->flatW = iPtr->w * FLATTEN_FACTOR;
        }
        if(diffY2 > 0.0f) {
            iPtr->flatY = limitY2 + diffY2 * FLATTEN_FACTOR;
            iPtr->flatH = iPtr->h * FLATTEN_FACTOR;
        }
        
        //Now, flatten the children.
        flattenItemsInList(
            getItemChildren(iPtr->id),
            iPtr->flatX - iPtr->flatW / 2.0f,
            iPtr->flatY - iPtr->flatH / 2.0f,
            iPtr->flatX + iPtr->flatW / 2.0f,
            iPtr->flatY + iPtr->flatH / 2.0f
        );
    }
}


/**
 * @brief Returns the best item in a list, by scoring them.
 *
 * @param list The list.
 * @param bestScore Pointer to the best score so far.
 * @param bestItemId Pointer to the best item's identifier so far.
 */
void Interface::getBestItem(
    const std::map<void*, ItemWithRelUnits>& list,
    double* bestScore, void** bestItemId, bool loopedItems
) {
    for(auto& i : list) {
        if(
            !checkHeuristicsPass(
                i.second.relX, i.second.relY, i.second.relW, i.second.relH
            )
        ) {
            //Doesn't pass some of the heuristics checks.
            continue;
        }
        
        if(i.second.relX <= 0.0f) {
            //Wrong direction!
            continue;
        }
        
        double score =
            getItemScore(
                i.second.relX, i.second.relY, i.second.relW, i.second.relH
            );
        if(score < *bestScore) {
            *bestScore = score;
            *bestItemId = i.first;
        }
        
#ifdef SPAT_NAV_DEBUG
        lastNavInfo[i.first].score = score;
        lastNavInfo[i.first].accepted = true;
        lastNavInfo[i.first].looped = loopedItems;
#endif
        
    }
}

/**
 * @brief Returns an item's children items, if any.
 *
 * @param id Identifier of the item whose children to check.
 * @return The children, or an empty vector if none.
 */
std::vector<Interface::Item*> Interface::getItemChildren(void* id) {
    std::vector<Item*> result;
    const auto& it = children.find(id);
    if(it != children.end()) {
        for(size_t c = 0; c < it->second.size(); c++) {
            result.push_back(items[it->second[c]]);
        }
    }
    return result;
}


/**
 * @brief Returns the X and Y difference between the focus and the given item.
 *
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @param iPtr Item to use.
 * @param direction Direction of navigation.
 * @param outDiffX The X coordinate of the difference is returned here.
 * @param outDiffY The Y coordinate of the difference  is returned here.
 */
void Interface::getItemDiffs(
    float focusX, float focusY, float focusW, float focusH,
    Item* iPtr, DIRECTION direction, double* outDiffX, double* outDiffY
) {
    double focusX1 = focusX - focusW / 2.0f;
    double focusY1 = focusY - focusH / 2.0f;
    double focusX2 = focusX + focusW / 2.0f;
    double focusY2 = focusY + focusH / 2.0f;
    double iX1 = iPtr->flatX - iPtr->flatW / 2.0f;
    double iY1 = iPtr->flatY - iPtr->flatH / 2.0f;
    double iX2 = iPtr->flatX + iPtr->flatW / 2.0f;
    double iY2 = iPtr->flatY + iPtr->flatH / 2.0f;
    
    double workingX =
        direction == DIRECTION_LEFT ?
        focusX1 :
        direction == DIRECTION_RIGHT ?
        focusX2 :
        focusX;
    double workingY =
        direction == DIRECTION_UP ?
        focusY1 :
        direction == DIRECTION_DOWN ?
        focusY2 :
        focusY;
    double itemX =
        direction == DIRECTION_LEFT ?
        iX2 :
        direction == DIRECTION_RIGHT ?
        iX1 :
        workingX;
    double itemY =
        direction == DIRECTION_UP ?
        iY2 :
        direction == DIRECTION_DOWN ?
        iY1 :
        workingY;
        
    itemX = std::clamp(itemX, iX1, iX2);
    itemY = std::clamp(itemY, iY1, iY2);
    *outDiffX = itemX - workingX;
    *outDiffY = itemY - workingY;
    
#ifdef SPAT_NAV_DEBUG
    lastNavInfo[iPtr->id].focusX = workingX;
    lastNavInfo[iPtr->id].focusY = workingY;
    lastNavInfo[iPtr->id].itemX = itemX;
    lastNavInfo[iPtr->id].itemY = itemY;
#endif
}


/**
 * @brief Converts the standard coordinates of an item to ones relative
 * to the current focus coordinates, and rotated so they're to its right.
 *
 * @param iPtr Item to check.
 * @param direction Navigation direction.
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @param outRelX The relative X coordinate is returned here.
 * @param outRelY The relative Y coordinate is returned here.
 * @param outRelW The relative width is returned here.
 * @param outRelH The relative height is returned here.
 */
void Interface::getItemRelativeUnits(
    Item* iPtr, DIRECTION direction,
    float focusX, float focusY, float focusW, float focusH,
    double* outRelX, double* outRelY, double* outRelW, double* outRelH
) {
    double resultX, resultY, resultW, resultH;
    double diffX, diffY;
    if(heuristics.centerOnly) {
        diffX = iPtr->flatX - focusX;
        diffY = iPtr->flatY - focusY;
#ifdef SPAT_NAV_DEBUG
        lastNavInfo[iPtr->id].focusX = focusX;
        lastNavInfo[iPtr->id].focusY = focusY;
        lastNavInfo[iPtr->id].itemX = iPtr->flatX;
        lastNavInfo[iPtr->id].itemY = iPtr->flatY;
#endif
        
    } else {
        getItemDiffs(
            focusX, focusY, focusW, focusH, iPtr, direction, &diffX, &diffY
        );
    }
    
    //Rotate the position, and the size if needed.
    switch(direction) {
    case DIRECTION_RIGHT: {
        resultX = diffX;
        resultY = diffY;
        resultW = iPtr->flatW;
        resultH = iPtr->flatH;
        break;
    } case DIRECTION_DOWN: {
        resultX = diffY;
        resultY = -diffX;
        resultW = iPtr->flatH;
        resultH = iPtr->flatW;
        break;
    } case DIRECTION_LEFT: {
        resultX = -diffX;
        resultY = -diffY;
        resultW = iPtr->flatW;
        resultH = iPtr->flatH;
        break;
    } case DIRECTION_UP: {
        resultX = -diffY;
        resultY = diffX;
        resultW = iPtr->flatH;
        resultH = iPtr->flatW;
        break;
    }
    }
    
    *outRelX = resultX;
    *outRelY = resultY;
    *outRelW = resultW;
    *outRelH = resultH;
}


/**
 * @brief Returns an item's parent item, if any.
 *
 * @param id Identifier of the item whose parent to check.
 * @return The parent, or nullptr if none.
 */
Interface::Item* Interface::getItemParent(void* id) {
    const auto& it = parents.find(id);
    if(it == parents.end()) return nullptr;
    return items[it->second];
}


/**
 * @brief Returns an item's score. Lower is better.
 *
 * @param itemRelX X coordinate of its center, relative to the focus.
 * @param itemRelY Y coordinate of its center, relative to the focus.
 * @param itemRelW Width, relative to the focus.
 * @param itemRelH Height, relative to the focus.
 * @return Its score.
 */
double Interface::getItemScore(
    double itemRelX, double itemRelY, double itemRelW, double itemRelH
) {
    switch(heuristics.distCalcMethod) {
    case DIST_CALC_METHOD_EUCLIDEAN: {
        return itemRelX * itemRelX + itemRelY * itemRelY;
        break;
    } case DIST_CALC_METHOD_TAXICAB: {
        return itemRelX + fabs(itemRelY);
        break;
    } case DIST_CALC_METHOD_TAXICAB_2: {
        return itemRelX + fabs(itemRelY * 2.0f);
        break;
    }
    }
    
    return 0.0f;
}


/**
 * @brief Returns the list of items, but with their units made relative to
 * the focus.
 *
 * @param direction Navigation direction.
 * @param focusedItemId Identifier of the currently-focused item.
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @return The list.
 */
std::map<void*, Interface::ItemWithRelUnits>
Interface::getItemsWithRelativeUnits(
    DIRECTION direction, void* focusedItemId,
    float focusX, float focusY, float focusW, float focusH
) {
    std::map<void*, Interface::ItemWithRelUnits> result;
    
    for(auto& i : items) {
        if(i.first == focusedItemId) continue;
        if(itemHasChildren(i.first)) continue;
        
        double itemRelX, itemRelY, itemRelW, itemRelH;
        getItemRelativeUnits(
            i.second, direction, focusX, focusY, focusW, focusH,
            &itemRelX, &itemRelY, &itemRelW, &itemRelH
        );
        
        result[i.first] = {
            .item = i.second,
            .relX = itemRelX,
            .relY = itemRelY,
            .relW = itemRelW,
            .relH = itemRelH,
        };
    }
    
    return result;
}


/**
 * @brief Returns the limits of the given items.
 *
 * @param limitX1 The top-left corner's X coordinate is returned here.
 * @param limitY1 The top-left corner's Y coordinate is returned here.
 * @param limitX2 The bottom-right corner's X coordinate is returned here.
 * @param limitY2 The bottom-right corner's Y coordinate is returned here.
 */
void Interface::getLimits(
    double* limitX1, double* limitY1, double* limitX2, double* limitY2
) const {
    *limitX1 = settings.limitX1;
    *limitY1 = settings.limitY1;
    *limitX2 = settings.limitX2;
    *limitY2 = settings.limitY2;
    
    for(const auto& i : items) {
        Item* iPtr = i.second;
        *limitX1 = std::min(*limitX1, iPtr->flatX - iPtr->flatW / 2.0f);
        *limitY1 = std::min(*limitY1, iPtr->flatY - iPtr->flatH / 2.0f);
        *limitX2 = std::max(*limitX2, iPtr->flatX + iPtr->flatW / 2.0f);
        *limitY2 = std::max(*limitY2, iPtr->flatY + iPtr->flatH / 2.0f);
    }
}


/**
 * @brief Returns whether an item has children.
 *
 * @param id Identifier of the item to check.
 * @return Whether it has children.
 */
bool Interface::itemHasChildren(void* id) const {
    return children.find(id) != children.end();
}


/**
 * @brief Loops any items that need looping, and splits all items
 * between a list of items that got looped and those that didn't.
 *
 * @param itemsWithRelUnits List of items with their relative units already
 * calculated.
 * @param direction Navigation direction.
 * @param limitX1 Top-left X coordinate of the limit they must abide.
 * @param limitY1 Top-left Y coordinate of the limit they must abide.
 * @param limitX2 Bottom-right X coordinate of the limit they must abide.
 * @param limitY2 Bottom-right Y coordinate of the limit they must abide.
 * @param outNonLoopedItems List of items which did not get looped.
 * @param outLoopedItems List of items which did get looped.
 */
void Interface::loopItems(
    const std::map<void*, Interface::ItemWithRelUnits>& itemsWithRelUnits,
    DIRECTION direction,
    double limitX1, double limitY1, double limitX2, double limitY2,
    std::map<void*, Interface::ItemWithRelUnits>* outNonLoopedItems,
    std::map<void*, Interface::ItemWithRelUnits>* outLoopedItems
) {
    for(const auto& i : itemsWithRelUnits) {
        double relX = i.second.relX;
        bool looped =
            checkLoopRelativeCoordinates(
                direction, &relX,
                limitX1, limitY1, limitX2, limitY2
            );
            
        if(looped) {
            (*outLoopedItems)[i.first] = i.second;
            (*outLoopedItems)[i.first].relX = relX;
        } else {
            (*outNonLoopedItems)[i.first] = i.second;
            (*outNonLoopedItems)[i.first].relX = relX;
        }
    }
}


/**
 * @brief Navigates in a given direction.
 *
 * @param direction Navigation direction.
 * @param focusedItemId Identifier of the currently-focused item.
 * @return Identifier of the newly-focused item.
 */
void* Interface::navigate(DIRECTION direction, void* focusedItemId) {
    if(items.find(focusedItemId) == items.end()) focusedItemId = nullptr;
    
    return
        doNavigation(
            direction, focusedItemId,
            focusedItemId == nullptr ? 0.0f : items[focusedItemId]->x,
            focusedItemId == nullptr ? 0.0f : items[focusedItemId]->y,
            focusedItemId == nullptr ? 0.0f : items[focusedItemId]->w,
            focusedItemId == nullptr ? 0.0f : items[focusedItemId]->h
        );
}


/**
 * @brief Navigates in a given direction.
 *
 * @param direction Navigation direction.
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @return Identifier of the newly-focused item.
 */
void* Interface::navigate(
    DIRECTION direction, float focusX, float focusY, float focusW, float focusH
) {
    return
        doNavigation(
            direction, nullptr, focusX, focusY, focusW, focusH
        );
}


/**
 * @brief Sets a child item's parent.
 *
 * @param childId Identifier of the child item.
 * @param parentId Identifier of the parent item.
 * @return Whether it succeeded.
 */
bool Interface::setParentItem(void* childId, void* parentId) {
    parents[childId] = parentId;
    children[parentId].push_back(childId);
    return true;
}

}
