/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the liquid class and liquid-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include <string>
#include <vector>

#include "../../util/drawing_utils.h"
#include "../animation/animation.h"
#include "../content.h"


using std::string;
using std::vector;


struct Hazard;
class InWorldFraction;
class Mob;
class Sector;
class StatusType;


namespace LIQUID {
extern const float DRAIN_DURATION;
extern const float FREEZING_EFFECT_DURATION;
extern const float FREEZING_OPACITY;
extern const float FREEZING_POINT_AREA_MULT;
extern const string FREEZING_POINT_SECTOR_VAR;
extern const float THAW_CRACKED_DURATION;
extern const float THAW_DURATION;
extern const float THAW_EFFECT_DURATION;
}


//Possible states for a liquid.
enum LIQUID_STATE {

    //Normal.
    LIQUID_STATE_NORMAL,
    
    //Gone, like after being drained.
    LIQUID_STATE_GONE,
    
    //Draining.
    LIQUID_STATE_DRAINING,
    
    //Frozen, and staying frozen.
    LIQUID_STATE_FROZEN,
    
    //Frozen, but thawing.
    LIQUID_STATE_THAWING,
    
};


/**
 * @brief Defines a body of water, or another liquid, within the area.
 */
struct Liquid {

    //--- Members ---
    
    //Hazard that brought this liquid about.
    Hazard* hazard = nullptr;
    
    //List of sectors that contain this liquid.
    vector<Sector*> sectors;
    
    //Time passed in the current state.
    float stateTime = 0.0f;
    
    //Current state.
    LIQUID_STATE state = LIQUID_STATE_NORMAL;
    
    //How chilled it is.
    size_t chillAmount = 0;
    
    //How chilled it needs to be to freeze. 0 to disable freezing.
    size_t freezingPoint = 0;
    
    //Whether it is currently thawing from frozen, as it's no longer chilled.
    bool thawing = false;
    
    //Data about the in-world chill fraction numbers, if any.
    InWorldFraction* chillFraction = nullptr;
    
    
    //--- Function declarations ---
    
    Liquid(Hazard* hazard, const vector<Sector*>& sectors);
    ~Liquid();
    bool isFrozen(float* thawOpacity, float* flashOpacity, bool* cracked) const;
    bool startDraining();
    void tick(float deltaT);
    
    
    private:
    
    //--- Function declarations ---
    
    void changeSectorsHazard(Hazard* hPtr);
    Point getDefaultChillFractionPos() const;
    vector<Mob*> getMobsOn() const;
    void setState(LIQUID_STATE newState);
    void updateChill(size_t amount, Point* where, const vector<Mob*>& mobsOn);
    
};


/**
 * @brief A liquid type defines how a sector should look to make it look
 * like water.
 *
 * This is considered a "liquid" and not specifically "water" because the
 * engine allows creating other water-like things, like acid, lava, etc.
 * Each have their own color, reflectivity, etc.
 * A hazard can be associated with a liquid. It's the way the
 * engine has to know if a sector is to be shown as a liquid or not.
 */
struct LiquidType : public Content {

    //--- Members ---
    
    //Color the body of liquid is.
    ALLEGRO_COLOR bodyColor = COLOR_BLACK;
    
    //Color the shine of liquid is.
    ALLEGRO_COLOR shineColor = COLOR_WHITE;
    
    //Color used for this liquid in the radar.
    ALLEGRO_COLOR radarColor = COLOR_EMPTY;
    
    //Maximum displacement amount.
    Point distortionAmount = Point(14.0f, 4.0f);
    
    //Noise threshold for how much of the liquid will have no shines.
    float shineMinThreshold = 0.5f;
    
    //Noise threshold for how much of the liquid fully covered in shines.
    float shineMaxThreshold = 1.0f;
    
    //How fast the water animates.
    float animSpeed = 1;
    
    //Whether it can be chilled and frozen.
    bool canFreeze = false;
    
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level);
    
};
