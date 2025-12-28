/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Liquid class and liquid-related functions.
 */

#include "liquid.h"

#include "../../content/other/status.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../core/misc_structs.h"
#include "../../util/string_utils.h"


using std::string;
using std::vector;


namespace LIQUID {

//Liquids drain for this long.
const float DRAIN_DURATION = 2.0f;

//How long the just-frozen flash effect lasts for.
const float FREEZING_EFFECT_DURATION = 0.3f;

//How opaque the sheet of ice is [0 - 1].
const float FREEZING_OPACITY = 0.8f;

//Multiply the liquid's surface area by this to get the freezing point.
const float FREEZING_POINT_AREA_MULT = 0.0003f;

//Sectors can have a var with this name to control the freezing point.
const string FREEZING_POINT_SECTOR_VAR = "freezing_point";

//Liquids thaw from being frozen for this long.
const float THAW_DURATION = 8.0f;

//How long the thawing transparency effect lasts for.
const float THAW_EFFECT_DURATION = 0.1f;

}


/**
 * @brief Constructs a new liquid object.
 *
 * @param hazard Hazard that brought this liquid about.
 * @param sectors List of sectors that contain this liquid.
 */
Liquid::Liquid(Hazard* hazard, const vector<Sector*>& sectors) :
    hazard(hazard),
    sectors(sectors) {
    
    if(!hazard->associatedLiquid) {
        hazard = nullptr;
    }
    
    if(hazard->associatedLiquid->canFreeze) {
        bool freezingPointFromVars = false;
        size_t highestVarValue = 0;
        float totalSurfaceArea = 0.0f;
        
        for(size_t s = 0; s < sectors.size(); s++) {
            totalSurfaceArea += sectors[s]->surfaceArea;
            if(!sectors[s]->vars.empty()) {
                map<string, string> vars = getVarMap(sectors[s]->vars);
                auto var = vars.find(LIQUID::FREEZING_POINT_SECTOR_VAR);
                if(var != vars.end()) {
                    freezingPointFromVars = true;
                    size_t value = s2i(var->second);
                    highestVarValue = std::max(highestVarValue, value);
                }
            }
        }
        
        if(freezingPointFromVars && highestVarValue > 0) {
            freezingPoint = highestVarValue;
        } else {
            freezingPoint = totalSurfaceArea * LIQUID::FREEZING_POINT_AREA_MULT;
        }
    }
}


/**
 * @brief Destroys the liquid object.
 */
Liquid::~Liquid() {
    if(chillFraction) delete chillFraction;
}


/**
 * @brief Changes the hazard of all its sectors.
 *
 * @param hPtr New hazard, or nullptr to remove.
 */
void Liquid::changeSectorsHazard(Hazard* hPtr) {
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* sPtr = sectors[s];
        sPtr->hazard = hPtr;
        game.states.gameplay->pathMgr.handleSectorHazardChange(sPtr);
        
        unordered_set<Vertex*> sectorVertexes;
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
            sectorVertexes.insert(sPtr->edges[e]->vertexes[0]);
            sectorVertexes.insert(sPtr->edges[e]->vertexes[1]);
        }
        updateOffsetEffectCaches(
            game.liquidLimitEffectCaches,
            sectorVertexes,
            doesEdgeHaveLiquidLimit,
            getLiquidLimitLength,
            getLiquidLimitColor
        );
    }
}


/**
 * @brief Returns a nice default position for the chill fraction.
 */
Point Liquid::getDefaultChillFractionPos() const {
    Point tl(FLT_MAX);
    Point br(-FLT_MAX);
    
    for(size_t s = 0; s < sectors.size(); s++) {
        updateMinCoords(tl, sectors[s]->bbox[0]);
        updateMaxCoords(br, sectors[s]->bbox[1]);
    }
    
    return
        Point(
            (tl.x + br.x) / 2.0f,
            (tl.y + br.y) / 2.0f
        );
}


/**
 * @brief Returns a list of all mobs currently on the liquid.
 *
 * @return The list.
 */
vector<Mob*> Liquid::getMobsOn() const {
    vector<Mob*> result;
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        for(size_t s = 0; s < sectors.size(); s++) {
            if(
                mPtr->groundSector == sectors[s] &&
                mPtr->z <= mPtr->groundSector->z
            ) {
                result.push_back(mPtr);
                break;
            }
        }
    }
    return result;
}


/**
 * @brief Returns whether the liquid is currently frozen.
 *
 * @param thawOpacity If it's just about to thaw, the opacity of the ice
 * is returned here [0 - 1]. Otherwise, 0 is returned.
 * @param flashOpacity If it's just been frozen, the opacity of the white flash
 * is returned here [0 - 1]. Otherwise, 0 is returned.
 * @return Whether it is frozen.
 */
bool Liquid::isFrozen(float* thawOpacity, float* flashOpacity) const {
    *thawOpacity = 0.0f;
    *flashOpacity = 0.0f;
    if(freezingPoint == 0) return false;
    if(thawing) {
        float timeLeft = LIQUID::THAW_DURATION - stateTime;
        if(timeLeft < LIQUID::THAW_EFFECT_DURATION) {
            *thawOpacity = timeLeft / LIQUID::THAW_EFFECT_DURATION;
        }
        return true;
    }
    if(chillAmount >= freezingPoint) {
        if(stateTime < LIQUID::FREEZING_EFFECT_DURATION) {
            *flashOpacity =
                1.0f - (stateTime / LIQUID::FREEZING_EFFECT_DURATION);
        }
        return true;
    }
    return false;
}


/**
 * @brief Starts the draining process.
 *
 * @return Whether it could start draining.
 */
bool Liquid::startDraining() {
    if(draining) return false;
    draining = true;
    stateTime = 0.0f;
    return true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Liquid::tick(float deltaT) {
    stateTime += deltaT;
    if(draining) {
        if(stateTime >= LIQUID::DRAIN_DURATION) {
            draining = false;
            stateTime = 0.0f;
            
            changeSectorsHazard(nullptr);
        }
    }
    
    if(thawing) {
        if(stateTime >= LIQUID::THAW_DURATION) {
            thawing = false;
            stateTime = 0.0f;
            
            changeSectorsHazard(hazard);
        }
    }
    
    if(freezingPoint != 0) {
        vector<Mob*> mobsOn = getMobsOn();
        size_t chillingMobs = 0;
        Point chillingMobPos;
        for(size_t m = 0; m < mobsOn.size(); m++) {
            if(mobsOn[m]->type->category->id == MOB_CATEGORY_PIKMIN) {
                Pikmin* pikPtr = (Pikmin*) mobsOn[m];
                if(pikPtr->pikType->chillsLiquids) {
                    chillingMobs++;
                    chillingMobPos = pikPtr->pos;
                }
            }
        }
        
        updateChill(chillingMobs, &chillingMobPos, mobsOn);
        
        if(chillFraction) {
            chillFraction->tick(deltaT);
            if(chillFraction->toDelete) {
                delete chillFraction;
                chillFraction = nullptr;
            }
        }
    }
}


/**
 * @brief Updates the chill amount to the new amount, and starts freezing or
 * thawing if necessary.
 *
 * @param newAmount The new amount.
 * @param where Source location. Used to know where to show the
 * fraction numbers. nullptr for none.
 * @param mobsOn Mobs currently on the liquid.
 */
void Liquid::updateChill(
    size_t newAmount, Point* where, const vector<Mob*>& mobsOn
) {
    if(!hazard) return;
    if(draining) return;
    if(!hazard->associatedLiquid->canFreeze) return;
    if(freezingPoint == 0) return;
    if(chillAmount == newAmount) return;
    
    bool wasFrozen = chillAmount >= freezingPoint;
    bool willFreeze = newAmount >= freezingPoint;
    
    if(!wasFrozen && willFreeze) {
        thawing = false;
        stateTime = 0.0f;
        changeSectorsHazard(nullptr);
    } else if(wasFrozen && !willFreeze) {
        thawing = true;
        stateTime = 0.0f;
    }
    
    if(chillAmount == 0 && newAmount > 0) {
        if(chillFraction) delete chillFraction;
        chillFraction = new InWorldFraction();
        chillFraction->setNoMobPos(getDefaultChillFractionPos());
        if(where) chillFraction->setNoMobPos(*where);
    }
    
    if(chillFraction) {
        chillFraction->setColor(game.config.aestheticGen.carryingColorMove);
        chillFraction->setRequirementNumber(freezingPoint);
        chillFraction->setValueNumber(newAmount);
        if(newAmount == 0) {
            chillFraction->startFading();
        }
    }
    
    chillAmount = newAmount;
}


/**
 * @brief Loads liquid data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void LiquidType::loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter lRS(node);
    
    lRS.set("animation_speed", animSpeed);
    lRS.set("body_color", bodyColor);
    lRS.set("can_freeze", canFreeze);
    lRS.set("distortion_amount", distortionAmount);
    lRS.set("radar_color", radarColor);
    lRS.set("shine_color", shineColor);
    lRS.set("shine_max_threshold", shineMaxThreshold);
    lRS.set("shine_min_threshold", shineMinThreshold);
}
