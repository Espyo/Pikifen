/*
 * Copyright (c) Andre 'Espyo' Silva 2025.
 *
 * === FILE DESCRIPTION ===
 * Header for the Easy Spatial Navigation library.
 * Please read the included readme file.
 */

#pragma once

#define _USE_MATH_DEFINES
#include <map>
#include <math.h>
#include <vector>


//#define EASY_SPAT_NAV_DEBUG


namespace EasySpatNav {

//Full circle, in radians.
constexpr const float TAU = (float) M_PI * 2.0f;


//Cardinal directions.
enum DIRECTION {

    //Right (East).
    DIRECTION_RIGHT,
    
    //Down (South).
    DIRECTION_DOWN,
    
    //Left (West).
    DIRECTION_LEFT,
    
    //Up (North).
    DIRECTION_UP,
    
};


//Ways to calculate the distance between two points, when scoring which item
//is better.
enum DIST_CALC_METHOD {

    //Normal Euclidean distance.
    DIST_CALC_METHOD_EUCLIDEAN,
    
    //Taxicab distance, i.e. dx + dy.
    DIST_CALC_METHOD_TAXICAB,
    
    //Taxicab distance, but the axis that's not in the direction of navigation
    //receives double the score.
    DIST_CALC_METHOD_TAXICAB_2,
    
};


/**
 * @brief Manager for the spatial navigation algorithm.
 */
class Interface {

public:

    //--- Members ---
    
    //Settings for how it works.
    struct Settings {
    
        //--- Members ---
        
        //Top-left corner's X coordinate.
        //If not specified, i.e. left at the default values, the limits will
        //be automatically calculated based on the existing items, with no
        //padding.
        float limitX1 = 0.0f;
        
        //Same as limitX1, but for the top-left corner's Y coordinate.
        float limitY1 = 0.0f;
        
        //Same as limitX1, but for the bottom-right corner's X coordinate.
        float limitX2 = 0.0f;
        
        //Same as limitX1, but for the bottom-right corner's Y coordinate.
        float limitY2 = 0.0f;
        
        //Whether it loops around when it reaches a horizontal limit.
        bool loopX = true;
        
        //Whether it loops around when it reaches a vertical limit.
        bool loopY = true;
        
    } settings;
    
    //Heuristics for how it decides on items.
    struct Heuristics {
    
        //If true, only use the center coordinates of items when comparing them.
        //If false, use the closest point along the limits, which takes the
        //item's width and height into account.
        bool centerOnly = false;
        
        //Distance calculation method.
        DIST_CALC_METHOD distCalcMethod = DIST_CALC_METHOD_TAXICAB;
        
        //If false, do two passes: try once without looping anything,
        //and only if that doesn't return anything do we try with the
        //looped items. This is useful if you have, say, a few items in one
        //corner and an item in the other corner. If you're in the first corner
        //and when hitting a direction you hit a limit, you'll probably end up
        //in the corner again, making it difficult to reach the lone item.
        //If true, we try all items at once.
        bool singleLoopPass = false;
        
    } heuristics;
    
    
#ifdef EASY_SPAT_NAV_DEBUG
    
    /**
     * @brief Represents an item when it was checked for the latest navigation.
     */
    struct DebugItem {
    
        //--- Members ---
        
        //X of the point on the focus that was checked.
        double focusX = 0.0f;
        
        //Y of the point on the focus that was checked.
        double focusY = 0.0f;
        
        //X of the point on the item that was checked.
        double itemX = 0.0f;
        
        //Y of the point on the item that was checked.
        double itemY = 0.0f;
        
        //Score that this item received.
        double score = 0.0f;
        
        //Whether it got calculated or discarded.
        bool accepted = false;
        
        //Whether it looped around or not.
        bool looped = false;
        
    };
    
    //Information about how each item fared in the latest navigation.
    std::map<void*, DebugItem> lastNavInfo;
    
#endif
    
    
    //--- Function declarations ---
    
    ~Interface();
    bool addItem(void* id, float x, float y, float w, float h);
    bool setParentItem(void* childId, void* parentId);
    bool clearItems();
    void* navigate(DIRECTION direction, void* focusedItemId);
    void* navigate(
        DIRECTION direction,
        float focusX, float focusY, float focusW, float focusH
    );
    
    
protected:

    //--- Misc. definitions ---
    
    
    //How much to flatten the coordinates of children outside their parents'
    //limits by.
    static constexpr float FLATTEN_FACTOR = 0.0001f;
    
    
    /**
     * @brief Represents an item in the interface. It can be inside of a parent
     * item.
     */
    struct Item {
    
    public:
    
        //--- Members ---
        
        //Identifier.
        void* id = nullptr;
        
        //Base X coordinate of its center.
        float x = 0.0f;
        
        //Base Y coordinate of its center.
        float y = 0.0f;
        
        //Base width.
        float w = 0.0f;
        
        //Base height.
        float h = 0.0f;
        
        //X coordinate of its center, after being flattened.
        double flatX = 0.0f;
        
        //Y coordinate of its center, after being flattened.
        double flatY = 0.0f;
        
        //Width, after being flattened.
        double flatW = 0.0f;
        
        //Height, after being flattened.
        double flatH = 0.0f;
        
    };
    
    
    /**
     * @brief An item with its units changed to be relative to the focus.
     */
    struct ItemWithRelUnits {
    
        //--- Members ---
        
        //The item.
        Item* item = nullptr;
        
        //Relative X coordinate.
        double relX = 0.0f;
        
        //Relative Y coordinate.
        double relY = 0.0f;
        
        //Relative width.
        double relW = 0.0f;
        
        //Relative height.
        double relH = 0.0f;
        
    };
    
    
    //--- Members ---
    
    //All registered items.
    std::map<void*, Item*> items;
    
    //Parent associations.
    std::map<void*, void*> parents;
    
    //Children associations.
    std::map<void*, std::vector<void*> > children;
    
    
    //--- Function declarations ---
    
    bool checkLoopRelativeCoordinates(
        DIRECTION direction, double* itemRelX,
        double limitX1, double limitY1, double limitX2, double limitY2,
        bool loopEvenIfInFront
    );
    void* doNavigation(
        DIRECTION direction, void* focusedItemId,
        float focusX, float focusY, float focusW, float focusH
    );
    void flattenItems();
    void flattenItemsInList(
        std::vector<Item*> list,
        float limitX1, float limitY1, float limitX2, float limitY2
    );
    void getBestItem(
        const std::map<void*, ItemWithRelUnits>& list,
        double* bestScore, void** bestItemId, bool loopedItems
    ) const;
    void getItemRelativeUnits(
        Item* iPtr, DIRECTION direction,
        float focusX, float focusY, float focusW, float focusH,
        double* outRelX, double* outRelY, double* outRelW, double* outRelH
    ) const;
    std::vector<Item*> getItemChildren(void* id) const;
    void getItemDiffs(
        float focusX, float focusY, float focusW, float focusH,
        Item* iPtr, DIRECTION direction, double* outDiffX, double* outDiffY
    ) const;
    Item* getItemParent(void* id) const;
    double getItemScore(
        double itemRelX, double itemRelY, double itemRelW, double itemRelH
    ) const;
    std::map<void*, ItemWithRelUnits> getItemsWithRelativeUnits(
        DIRECTION direction,
        float focusX, float focusY, float focusW, float focusH
    ) const;
    void getItemLimitsFlattened(
        double* limitX1, double* limitY1, double* limitX2, double* limitY2
    ) const;
    void getItemLimitsNonFlattened(
        double* limitX1, double* limitY1, double* limitX2, double* limitY2
    ) const;
    bool itemHasChildren(void* id) const;
    void loopItems(
        const std::map<void*, Interface::ItemWithRelUnits>& itemsWithRelUnits,
        DIRECTION direction, void* focusedItemId,
        double limitX1, double limitY1, double limitX2, double limitY2,
        std::map<void*, Interface::ItemWithRelUnits>* outNonLoopedItems,
        std::map<void*, Interface::ItemWithRelUnits>* outLoopedItems
    );
    
};

}
