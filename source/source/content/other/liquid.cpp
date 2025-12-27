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
 * @brief Starts the draining process.
 *
 * @return Whether it could start draining.
 */
bool Liquid::startDraining() {
    if(draining) return false;
    if(drainTimeLeft > 0.0f) return false;
    
    draining = true;
    drainTimeLeft = LIQUID::DRAIN_DURATION;
    
    return true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Liquid::tick(float deltaT) {
    if(draining) {
        drainTimeLeft -= deltaT;
        
        if(drainTimeLeft <= 0.0f) {
            drainTimeLeft = 0.0f;
            draining = false;
            
            changeSectorsHazard(nullptr);
        }
    }
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
    
    lRS.set("body_color", bodyColor);
    lRS.set("shine_color", shineColor);
    lRS.set("radar_color", radarColor);
    lRS.set("shine_min_threshold", shineMinThreshold);
    lRS.set("shine_max_threshold", shineMaxThreshold);
    lRS.set("distortion_amount", distortionAmount);
    lRS.set("animation_speed", animSpeed);
}
