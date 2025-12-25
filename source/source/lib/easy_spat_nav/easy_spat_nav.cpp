/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Source code for the Easy Spatial Navigation library.
 * Please read the included readme file.
 */

#include <algorithm>
#include <cmath>
#include <float.h>
#include <limits.h>

#include "easy_spat_nav.h"


namespace EasySpatNav {


/**
 * @brief Destroys the interface object.
 */
Interface::~Interface() {
    reset();
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
bool Interface::addItem(ItemId id, float x, float y, float w, float h) {
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
 * @param loopEvenIfInFront If true, the item is looped even if it's already
 * in front of the focus.
 * @return Whether it had looped.
 */
bool Interface::checkLoopRelativeCoordinates(
    DIRECTION direction, double* itemRelX,
    double limitX1, double limitY1, double limitX2, double limitY2,
    bool loopEvenIfInFront
) {
    bool horizontalDir =
        direction == DIRECTION_RIGHT || direction == DIRECTION_LEFT;
    bool loopAllowed = horizontalDir ? settings.loopX : settings.loopY;
    bool needsLoop = loopEvenIfInFront || *itemRelX < 0.0f;
    
    if(!loopAllowed || !needsLoop) return false;
    
    if(!horizontalDir) {
        *itemRelX += limitY2 - limitY1;
    } else {
        *itemRelX += limitX2 - limitX1;
    }
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
ItemId Interface::doNavigation(
    DIRECTION direction, ItemId focusedItemId,
    float focusX, float focusY, float focusW, float focusH
) {
    //Setup.
    
#ifdef EASY_SPAT_NAV_DEBUG
    lastNavInfo.clear();
#endif
    
    flattenItems();
    
    std::map<ItemId, ItemWithRelUnits> itemsWithRelUnits =
        getItemsWithRelativeUnits(
            direction, focusX, focusY, focusW, focusH
        );
        
    double limitX1 = settings.limitX1;
    double limitY1 = settings.limitY1;
    double limitX2 = settings.limitX2;
    double limitY2 = settings.limitY2;
    getItemLimitsFlattened(&limitX1, &limitY1, &limitX2, &limitY2);
    
    //Loop any items that need looping.
    std::map<ItemId, ItemWithRelUnits> nonLoopedItems;
    std::map<ItemId, ItemWithRelUnits> loopedItems;
    loopItems(
        itemsWithRelUnits, direction, focusedItemId,
        limitX1, limitY1, limitX2, limitY2,
        &nonLoopedItems, &loopedItems
    );
    
    //Score them.
    std::vector<double> bestScores;
    std::vector<ItemId> bestItemIds;
    bool checkLoopedItems = true;
    getBestItems(nonLoopedItems, &bestScores, &bestItemIds, false);
    
    if(!heuristics.singleLoopPass) {
        //If in two loop passes mode, only check the looped items if
        //the non-looped items gave us nothing.
        if(!bestItemIds.empty()) checkLoopedItems = false;
    }
    
    if(checkLoopedItems) {
        getBestItems(loopedItems, &bestScores, &bestItemIds, true);
    }
    
    //Break any ties.
    bool updateHistory = true;
    bool usedHistory = false;
    ItemId bestItemId =
        getBestItem(bestScores, bestItemIds, direction, &usedHistory);
    if(usedHistory) {
        history.pop_back();
        updateHistory = false;
    }
    
    if(bestItemId == focusedItemId) {
        //This can only happen if after looping the best item was the initial
        //one. Trying to focus on a different item would result in a nonsense
        //focus, so consider it as no target instead.
        bestItemId = nullptr;
    }
    
    //Finished!
    if(updateHistory && heuristics.historyScoreThreshold >= 0.0f) {
        if(direction != historyDirection || focusedItemId == nullptr) {
            history.clear();
        }
        if(focusedItemId != nullptr) {
            history.push_back(focusedItemId);
            historyDirection = direction;
        }
    }
    
    return bestItemId;
}


/**
 * @brief Flattens any children items that go outside their parents' limits.
 * This only affects children items that are completely outside, not partially.
 */
void Interface::flattenItems() {
    double limitX1 = settings.limitX1;
    double limitY1 = settings.limitY1;
    double limitX2 = settings.limitX2;
    double limitY2 = settings.limitY2;
    
    if(limitX1 == limitX2 || limitY1 == limitY2) {
        //No specified limits.
        getItemLimitsNonFlattened(&limitX1, &limitY1, &limitX2, &limitY2);
    }
    
    //Start with the top-level items.
    std::vector<Item*> list;
    for(const auto& i : items) {
        if(getItemParent(i.first)) continue;
        list.push_back(i.second);
    }
    flattenItemsInList(list, limitX1, limitY1, limitX2, limitY2);
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
 * @brief Returns which item is the best one in the given list, using
 * heuristics or in the case of ties, the item order.
 *
 * @param bestScores Vector of each best item's score.
 * @param bestItemIds Vector of each best item's identifier.
 * @param direction Direction of navigation.
 * @param usedHistory Whether or not the history was used is returned here.
 * @return The best item's identifier, or nullptr if none.
 */
ItemId Interface::getBestItem(
    const std::vector<double>& bestScores,
    const std::vector<ItemId> bestItemIds,
    DIRECTION direction, bool* usedHistory
) const {
    *usedHistory = false;
    if(bestItemIds.empty()) return nullptr;
    if(bestItemIds.size() == 1) return bestItemIds[0];
    
    //We got multiple good items to navigate to. Figure out the best one.
    ItemId bestItemId = nullptr;
    if(
        heuristics.historyScoreThreshold >= 0.0f &&
        !history.empty() &&
        isOppositeDirection(direction, historyDirection)
    ) {
        //Using the history, figure out where the user came from,
        //and prefer that item, if possible.
        auto it =
            std::find(
                bestItemIds.begin(), bestItemIds.end(),
                history.back()
            );
        if(it != bestItemIds.end()) {
            //Ok, go back in the user's history!
            bestItemId = *it;
            *usedHistory = true;
        }
    }
    
    if(bestItemId == nullptr) {
        //Pick the one with the absolute best score.
        //Tie-breakers are resolved by the item order
        //(first added to the interface wins).
        double bestScore = FLT_MAX;
        for(size_t i = 0; i < bestItemIds.size(); i++) {
            if(bestScores[i] < bestScore) {
                bestScore = bestScores[i];
                bestItemId = bestItemIds[i];
            }
        }
    }
    
    return bestItemId;
}


/**
 * @brief Returns the best item in a list, by scoring them.
 * Returns nullptr if no good item is found.
 *
 * @param list The list.
 * @param bestScores Pointer to the vector of each best item's score so far.
 * @param bestItemIds Pointer to the vector of each best item's identifier
 * so far.
 * @param loopedItems Whether the list provided is of items that got looped.
 */
void Interface::getBestItems(
    const std::map<ItemId, ItemWithRelUnits>& list,
    std::vector<double>* bestScores, std::vector<ItemId>* bestItemIds,
    bool loopedItems
) const {
    for(auto& i : list) {
        if(i.second.relX <= 0.0f) {
            //Wrong direction!
            continue;
        }
        
        double itemScore =
            getItemScore(
                i.second.relX, i.second.relY, i.second.relW, i.second.relH
            );
        double bestScore = FLT_MAX;
        if(!bestScores->empty()) {
            bestScore =
                *(std::max_element(bestScores->begin(), bestScores->end()));
        }
        if(
            itemScore <=
            bestScore + std::max(heuristics.historyScoreThreshold, 0.0f)
        ) {
            bestScores->push_back(itemScore);
            bestItemIds->push_back(i.first);
        }
        
#ifdef EASY_SPAT_NAV_DEBUG
        lastNavInfo[i.first].score = itemScore;
        lastNavInfo[i.first].accepted = true;
        lastNavInfo[i.first].looped = loopedItems;
#endif
        
    }
    
    //Delete any items whose score is below the threshold.
    double bestScore = FLT_MAX;
    if(!bestScores->empty()) {
        bestScore =
            *(std::max_element(bestScores->begin(), bestScores->end()));
    }
    for(size_t i = 0; i < bestItemIds->size();) {
        if(
            (*bestScores)[i] >
            bestScore + std::max(heuristics.historyScoreThreshold, 0.0f)
        ) {
            bestItemIds->erase(bestItemIds->begin() + i);
            bestScores->erase(bestScores->begin() + i);
        } else {
            i++;
        }
    }
}


/**
 * @brief Returns an item's children items, if any.
 *
 * @param id Identifier of the item whose children to check.
 * @return The children, or an empty vector if none.
 */
std::vector<Interface::Item*> Interface::getItemChildren(ItemId id) const {
    std::vector<Item*> result;
    const auto& it = children.find(id);
    if(it != children.end()) {
        for(size_t c = 0; c < it->second.size(); c++) {
            result.push_back(items.at(it->second[c]));
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
) const {
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
    
#ifdef EASY_SPAT_NAV_DEBUG
    lastNavInfo[iPtr->id].focusX = workingX;
    lastNavInfo[iPtr->id].focusY = workingY;
    lastNavInfo[iPtr->id].itemX = itemX;
    lastNavInfo[iPtr->id].itemY = itemY;
#endif
}


/**
 * @brief Returns the limits of all items, using their already-flattened
 * coordinates.
 *
 * @param limitX1 The top-left corner's X coordinate is returned here.
 * @param limitY1 The top-left corner's Y coordinate is returned here.
 * @param limitX2 The bottom-right corner's X coordinate is returned here.
 * @param limitY2 The bottom-right corner's Y coordinate is returned here.
 */
void Interface::getItemLimitsFlattened(
    double* limitX1, double* limitY1, double* limitX2, double* limitY2
) const {
    for(const auto& i : items) {
        Item* iPtr = i.second;
        *limitX1 = std::min(*limitX1, iPtr->flatX - iPtr->flatW / 2.0f);
        *limitY1 = std::min(*limitY1, iPtr->flatY - iPtr->flatH / 2.0f);
        *limitX2 = std::max(*limitX2, iPtr->flatX + iPtr->flatW / 2.0f);
        *limitY2 = std::max(*limitY2, iPtr->flatY + iPtr->flatH / 2.0f);
    }
}


/**
 * @brief Returns the limits of all items, using their normal,
 * non-flattened coordinates.
 *
 * @param limitX1 The top-left corner's X coordinate is returned here.
 * @param limitY1 The top-left corner's Y coordinate is returned here.
 * @param limitX2 The bottom-right corner's X coordinate is returned here.
 * @param limitY2 The bottom-right corner's Y coordinate is returned here.
 */
void Interface::getItemLimitsNonFlattened(
    double* limitX1, double* limitY1, double* limitX2, double* limitY2
) const {
    for(const auto& i : items) {
        Item* iPtr = i.second;
        *limitX1 = std::min(*limitX1, (double) (iPtr->x - iPtr->w / 2.0f));
        *limitY1 = std::min(*limitY1, (double) (iPtr->y - iPtr->h / 2.0f));
        *limitX2 = std::max(*limitX2, (double) (iPtr->x + iPtr->w / 2.0f));
        *limitY2 = std::max(*limitY2, (double) (iPtr->y + iPtr->h / 2.0f));
    }
}


/**
 * @brief Returns an item's parent item, if any.
 *
 * @param id Identifier of the item whose parent to check.
 * @return The parent, or nullptr if none.
 */
Interface::Item* Interface::getItemParent(ItemId id) const {
    const auto& it = parents.find(id);
    if(it == parents.end()) return nullptr;
    return items.at(it->second);
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
) const {
    double resultX = 0.0f;
    double resultY = 0.0f;
    double resultW = 0.0f;
    double resultH = 0.0f;;
    double diffX = 0.0f;
    double diffY = 0.0f;
    if(heuristics.centerOnly) {
        diffX = iPtr->flatX - focusX;
        diffY = iPtr->flatY - focusY;
#ifdef EASY_SPAT_NAV_DEBUG
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
) const {
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
 * @param focusX X coordinate of the current focus.
 * @param focusY Y coordinate of the current focus.
 * @param focusW Width of the current focus.
 * @param focusH Height of the current focus.
 * @return The list.
 */
std::map<ItemId, Interface::ItemWithRelUnits>
Interface::getItemsWithRelativeUnits(
    DIRECTION direction,
    float focusX, float focusY, float focusW, float focusH
) const {
    std::map<ItemId, Interface::ItemWithRelUnits> result;
    
    for(auto& i : items) {
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
 * @brief Returns whether two directions are opposites.
 *
 * @param dir1 First direction.
 * @param dir2 Second direction.
 * @return Whether they are opposites.
 */
bool Interface::isOppositeDirection(DIRECTION dir1, DIRECTION dir2) const {
    if(dir1 == DIRECTION_RIGHT && dir2 == DIRECTION_LEFT) return true;
    if(dir1 == DIRECTION_DOWN && dir2 == DIRECTION_UP) return true;
    if(dir1 == DIRECTION_LEFT && dir2 == DIRECTION_RIGHT) return true;
    if(dir1 == DIRECTION_UP && dir2 == DIRECTION_DOWN) return true;
    return false;
}


/**
 * @brief Returns whether an item has children.
 *
 * @param id Identifier of the item to check.
 * @return Whether it has children.
 */
bool Interface::itemHasChildren(ItemId id) const {
    return children.find(id) != children.end();
}


/**
 * @brief Loops any items that need looping, and splits all items
 * between a list of items that got looped and those that didn't.
 *
 * @param itemsWithRelUnits List of items with their relative units already
 * calculated.
 * @param direction Navigation direction.
 * @param focusedItemId Identifier of the currently-focused item.
 * @param limitX1 Top-left X coordinate of the limit they must abide.
 * @param limitY1 Top-left Y coordinate of the limit they must abide.
 * @param limitX2 Bottom-right X coordinate of the limit they must abide.
 * @param limitY2 Bottom-right Y coordinate of the limit they must abide.
 * @param outNonLoopedItems List of items which did not get looped.
 * @param outLoopedItems List of items which did get looped.
 */
void Interface::loopItems(
    const std::map<ItemId, Interface::ItemWithRelUnits>& itemsWithRelUnits,
    DIRECTION direction, ItemId focusedItemId,
    double limitX1, double limitY1, double limitX2, double limitY2,
    std::map<ItemId, Interface::ItemWithRelUnits>* outNonLoopedItems,
    std::map<ItemId, Interface::ItemWithRelUnits>* outLoopedItems
) {
    for(const auto& i : itemsWithRelUnits) {
        double relX = i.second.relX;
        bool looped =
            checkLoopRelativeCoordinates(
                direction, &relX,
                limitX1, limitY1, limitX2, limitY2,
                i.first == focusedItemId
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
ItemId Interface::navigate(DIRECTION direction, ItemId focusedItemId) {
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
ItemId Interface::navigate(
    DIRECTION direction, float focusX, float focusY, float focusW, float focusH
) {
    return
        doNavigation(
            direction, nullptr, focusX, focusY, focusW, focusH
        );
}


/**
 * @brief Deletes and clears all items, and resets some other states.
 *
 * @param resetHistory Whether the navigation history gets reset too.
 * @return Whether it succeeded.
 */
bool Interface::reset(bool resetHistory) {
    for(auto i : items) {
        delete i.second;
    }
    items.clear();
    
    parents.clear();
    children.clear();
    
    if(resetHistory) {
        history.clear();
        historyDirection = DIRECTION_RIGHT;
    }
    
    return true;
}


/**
 * @brief Sets a child item's parent.
 *
 * @param childId Identifier of the child item.
 * @param parentId Identifier of the parent item.
 * @return Whether it succeeded.
 */
bool Interface::setParentItem(ItemId childId, ItemId parentId) {
    parents[childId] = parentId;
    children[parentId].push_back(childId);
    return true;
}


}
