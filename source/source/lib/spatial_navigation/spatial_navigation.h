/*
 * Copyright (c) Andre 'Espyo' Silva 2025.
 *
 * === FILE DESCRIPTION ===
 * Header for the spatial navigation library.
 * Please read the included readme file.
 */

#pragma once

#define _USE_MATH_DEFINES
#include <map>
#include <math.h>
#include <vector>


namespace SpatNav {

//Full circle, in radians.
constexpr float TAU = (float) M_PI * 2.0f;


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



/**
 * @brief Manager for the spatial navigation algorithm.
 */
class Interface {

protected:

    //--- Misc. definitions ---
    
    
    //How much to flatten the coordinates of children outside their parents'
    //limits by.
    static constexpr float FLATTEN_FACTOR = 0.0001f;


    /**
     * @brief Represents an item in the interface. It can be inside of a parent
     * item.
     */
    class Item {
    
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
    
    
public:

    //--- Members ---
    
    //Settings for how it works.
    struct Settings {
    
        //--- Members ---
        
        //Top-left corner's X coordinate.
        float limitX1 = 0.0f;
        
        //Top-left corner's Y coordinate.
        float limitY1 = 0.0f;
        
        //Bottom-right corner's X coordinate.
        float limitX2 = 0.0f;
        
        //Bottom-right corner's Y coordinate.
        float limitY2 = 0.0f;
        
        //Whether it loops around when it reaches a limit.
        bool loop = true;
        
    } settings;
    
    //Heuristics for how it decides on items.
    struct Heuristics {
    
        //If not zero, if the item's relative angle with the current focus
        //position is within this reach, the item gets ignored. This is
        //useful to stop a horizontal list of items with no vertical variance
        //from picking another item when the navigation direction is up,
        //for instance.
        float minBlindspotAngle = (float) (TAU * 0.17f);
        
        //See minBlindspotAngle.
        float maxBlindspotAngle = (float) (TAU * 0.33f);
        
        //Only use the center coordinates of items, sans dimensions.
        bool centerOnly = true; //TODO
        
    } heuristics;
    
    
    //--- Function declarations ---
    
    ~Interface();
    bool addItem(void* id, float x, float y, float w, float h);
    bool setParentItem(void* childId, void* parentId);
    bool clearItems();
    void* navigate(DIRECTION direction, void* focusedItemId);
    void* navigate(DIRECTION direction, float focusX, float focusY);
    
    
protected:

    //--- Members ---
    
    //All registered items.
    std::map<void*, Item*> items;
    
    //Parent associations.
    std::map<void*, void*> parents;
    
    //Children associations.
    std::map<void*, std::vector<void*> > children;
    
    
    //--- Function declarations ---
    
    bool checkHeuristicsPass(
        double itemRelX, double itemRelY, double itemRelW, double itemRelH
    );
    void checkLoopRelativeCoordinates(
        DIRECTION direction, double* itemRelX,
        double limitX1, double limitY1, double limitX2, double limitY2
    );
    void* doNavigation(
        DIRECTION direction, void* focusedItemId, float focusX, float focusY,
        bool focusAtDirectionStart
    );
    void flattenItems();
    void flattenItemsInList(
        std::vector<Item*> list,
        float limitX1, float limitY1, float limitX2, float limitY2
    );
    void getItemRelativeUnits(
        Item* iPtr, DIRECTION direction, float focusX, float focusY,
        double* outRelX, double* outRelY, double* outRelW, double* outRelH
    );
    std::vector<Item*> getItemChildren(void* id);
    Item* getItemParent(void* id);
    double getItemScore(
        double itemRelX, double itemRelY, double itemRelW, double itemRelH
    );
    void getLimits(
        double* limitX1, double* limitY1, double* limitX2, double* limitY2
    ) const;
    
};

}
