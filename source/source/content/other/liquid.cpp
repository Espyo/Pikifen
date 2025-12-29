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

//Frozen liquids should show up as cracked for this long.
const float THAW_CRACKED_DURATION = 4.0f;

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
            freezingPoint =
                roundToNearestMultipleOf(
                    totalSurfaceArea * LIQUID::FREEZING_POINT_AREA_MULT, 5
                );
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
 * @brief Sets the liquid's state.
 *
 * @param newState The new state.
 */
void Liquid::setState(LIQUID_STATE newState) {
    state = newState;
    stateTime = 0.0f;
}


/**
 * @brief Returns whether the liquid is currently frozen.
 *
 * @param thawOpacity If it's just about to thaw, the opacity of the ice
 * is returned here [0 - 1]. Otherwise, 0 is returned.
 * @param flashOpacity If it's just been frozen, the opacity of the white flash
 * is returned here [0 - 1]. Otherwise, 0 is returned.
 * @param cracked If it's getting close to thawing, true is returned here so
 * the ice texture can be drawn cracked. Otherwise, false is returned.
 * @return Whether it is frozen.
 */
bool Liquid::isFrozen(
    float* thawOpacity, float* flashOpacity, bool* cracked
) const {
    *thawOpacity = 0.0f;
    *flashOpacity = 0.0f;
    *cracked = false;
    
    if(freezingPoint == 0) return false;
    
    if(state == LIQUID_STATE_THAWING) {
        float timeLeft = LIQUID::THAW_DURATION - stateTime;
        if(timeLeft < LIQUID::THAW_EFFECT_DURATION) {
            *thawOpacity = timeLeft / LIQUID::THAW_EFFECT_DURATION;
        }
        if(timeLeft < LIQUID::THAW_CRACKED_DURATION) {
            *cracked = true;
        }
        return true;
    }
    if(state == LIQUID_STATE_FROZEN) {
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
    if(state == LIQUID_STATE_GONE) return false;
    if(state == LIQUID_STATE_DRAINING) return false;
    setState(LIQUID_STATE_DRAINING);
    return true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Liquid::tick(float deltaT) {
    stateTime += deltaT;
    
    //Process timer-based states.
    switch(state) {
    case LIQUID_STATE_DRAINING: {
        if(stateTime >= LIQUID::DRAIN_DURATION) {
            setState(LIQUID_STATE_GONE);
            changeSectorsHazard(nullptr);
        }
        break;
        
    } case LIQUID_STATE_THAWING: {
        if(stateTime >= LIQUID::THAW_DURATION) {
            setState(LIQUID_STATE_NORMAL);
            changeSectorsHazard(hazard);
            freezeCaughtMobs.clear();
        }
        break;
    } default: {
        break;
    }
    }
    
    //Apply a status to any mobs that got caught.
    for(size_t m = 0; m < freezeCaughtMobs.size(); m++) {
        Mob* mPtr = freezeCaughtMobs[m];
        
        const auto it =
            std::find(
                game.states.gameplay->mobs.all.begin(),
                game.states.gameplay->mobs.all.end(),
                mPtr
            );
        if(it == game.states.gameplay->mobs.all.end()) {
            freezeCaughtMobs.erase(freezeCaughtMobs.begin() + m);
            m--;
            continue;
        }
        
        if(mPtr->health <= 0.0f) continue;
        mPtr->applyStatus(
            hazard->associatedLiquid->freezeMobStatus,
            false, true, true, true
        );
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
    if(!hazard->associatedLiquid->canFreeze) return;
    if(freezingPoint == 0) return;
    if(chillAmount == newAmount) return;
    
    switch(state) {
    case LIQUID_STATE_NORMAL: {
        if(newAmount >= freezingPoint) {
            setState(LIQUID_STATE_FROZEN);
            changeSectorsHazard(nullptr);
            if(hazard->associatedLiquid->freezeMobStatus) {
                freezeCaughtMobs = mobsOn;
            }
        }
        break;
    } case LIQUID_STATE_GONE:
    case LIQUID_STATE_DRAINING: {
        return;
        break;
    } case LIQUID_STATE_FROZEN: {
        if(newAmount < freezingPoint) {
            setState(LIQUID_STATE_THAWING);
        }
        break;
    } case LIQUID_STATE_THAWING: {
        if(newAmount >= freezingPoint) {
            setState(LIQUID_STATE_FROZEN);
        }
        break;
    }
    }
    
    if(chillAmount == 0 && newAmount > 0) {
        if(chillFraction) delete chillFraction;
        chillFraction = new InWorldFraction();
        chillFraction->setNoMobPos(getDefaultChillFractionPos());
        if(where) {
            chillFraction->setNoMobPos(
                Point(
                    where->x,
                    where->y - game.config.pikmin.standardRadius *  2.0f
                )
            );
        }
    }
    
    if(chillFraction) {
        chillFraction->setColor(
            newAmount >= freezingPoint ?
            game.config.aestheticGen.carryingColorMove :
            game.config.aestheticGen.carryingColorStop
        );
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
    string freezeMobStatusStr;
    DataNode* freezeMobStatusNode = nullptr;
    
    lRS.set("animation_speed", animSpeed);
    lRS.set("body_color", bodyColor);
    lRS.set("can_freeze", canFreeze);
    lRS.set("freeze_mob_status", freezeMobStatusStr, &freezeMobStatusNode);
    lRS.set("distortion_amount", distortionAmount);
    lRS.set("radar_color", radarColor);
    lRS.set("shine_color", shineColor);
    lRS.set("shine_max_threshold", shineMaxThreshold);
    lRS.set("shine_min_threshold", shineMinThreshold);
    
    if(freezeMobStatusNode) {
        auto s = game.content.statusTypes.list.find(freezeMobStatusStr);
        if(s != game.content.statusTypes.list.end()) {
            freezeMobStatus = s->second;
        } else {
            game.errors.report(
                "Unknown status type \"" + freezeMobStatusStr + "\"!",
                freezeMobStatusNode
            );
        }
    }
}
