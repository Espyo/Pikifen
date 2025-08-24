/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale class and scale-related functions.
 */

#include "scale.h"

#include "../../core/game.h"


/**
 * @brief Constructs a new scale object.
 *
 * @param pos Starting coordinates.
 * @param type Scale type this mob belongs to.
 * @param angle Starting angle.
 */
Scale::Scale(const Point& pos, ScaleType* type, float angle) :
    Mob(pos, type, angle),
    scaType(type),
    goalNumber(type->goalNumber) {
    
    
}


/**
 * @brief Calculates the total weight currently on top of the mob.
 *
 * @return The weight.
 */
float Scale::calculateCurWeight() const {

    //Start by figuring out which mobs are applying weight.
    set<Mob*> weighingMobs;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        
        if(mPtr->standingOnMob == this) {
            weighingMobs.insert(mPtr);
            for(size_t h = 0; h < mPtr->holding.size(); h++) {
                weighingMobs.insert(mPtr->holding[h]);
            }
        }
    }
    
    //Now, add up their weights.
    float w = 0;
    for(auto& m : weighingMobs) {
        w += m->type->weight;
    }
    
    return w;
}


/**
 * @brief Returns information on how to show the fraction numbers.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 *
 * @param outValueNr The fraction's value (upper) number is returned here.
 * @param outReqNr The fraction's required (lower) number is returned here.
 * @param outColor The fraction's color is returned here.
 * @return Whether the numbers should be shown.
 */
FRACTION_NR_VISIBILITY Scale::getFractionNumbersInfo(
    float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
) const {
    float weight = calculateCurWeight();
    if(health <= 0) return FRACTION_NR_VISIBILITY_NONE;
    
    *outValueNr = weight;
    *outReqNr = goalNumber;
    *outColor = game.config.aestheticGen.carryingColorStop;
    if(weight <= 0) return FRACTION_NR_VISIBILITY_CURSOR;
    return FRACTION_NR_VISIBILITY_ALWAYS;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Scale::readScriptVars(const ScriptVarReader& svr) {
    Mob::readScriptVars(svr);
    
    svr.get("goal_number", goalNumber);
}
