/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include <algorithm>

#include "onion.hpp"

#include "../../core/drawing.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/geometry_utils.hpp"
#include "../../util/string_utils.hpp"


using std::size_t;
using std::string;


namespace ONION {

//Delay before the Onion starts the generation process.
const float GENERATION_DELAY = 2.0f;

//An Onion-spat seed starts with this Z offset from the Onion.
const float NEW_SEED_Z_OFFSET = 320.0f;

//Interval between each individual Pikmin generation.
const float NEXT_GENERATION_INTERVAL = 0.10f;

//After spitting a seed, the next seed's angle shifts by this much.
const Angle SPIT_ANGLE_SHIFT = TAU * 0.12345;

//An Onion-spat seed is this quick, horizontally.
const float SPIT_H_SPEED = 80.0f;

//Deviate the seed's horizontal speed by this much, more or less.
const float SPIT_H_SPEED_DEVIATION = 10.0f;

//An Onion-spat seed is this quick, vertically.
const float SPIT_V_SPEED = 600.0f;
}




/**
 * @brief Constructs a new Onion object.
 *
 * @param center Starting center coordinates.
 * @param type Onion type this mob belongs to.
 * @param angle Starting angle.
 */
Onion::Onion(const Point& center, OnionType* type, Angle angle) :
    Mob(center, type, angle),
    oniType(type) {
    
    nest = new PikminNest(this, oniType->nest);
    nest->nSpitsPtr = &nSpits;
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravityMult = 0.0f;
    bottomZ++;
    
    generationDelayTimer.onEnd =
    [this] () { startGenerating(); };
    nextGenerationTimer.onEnd =
    [this] () {
        forIdx(t, oniType->nest->pikTypes) {
            if(generationQueue[t] > 0) {
                nextGenerationTimer.start();
                generate();
                return;
            }
        }
        stopGenerating();
    };
    
    forIdx(t, oniType->nest->pikTypes) {
        generationQueue.push_back(0);
        nutrients.push_back(0);
    }
}


/**
 * @brief Destroys the Onion object.
 */
Onion::~Onion() {
    delete nest;
}


/**
 * @brief Spit a Pikmin seed in the queue or add it to the Onion's storage.
 */
void Onion::generate() {
    forIdx(t, generationQueue) {
        if(generationQueue[t] == 0) continue;
        
        generationQueue[t]--;
        
        game.statistics.pikminBirths++;
        game.states.gameplay->pikminBorn++;
        game.states.gameplay->pikminBornPerType[oniType->nest->pikTypes[t]]++;
        game.states.gameplay->lastPikminBornPos = center;
        
        size_t totalAfter = game.states.gameplay->mobs.pikmin.size() + 1;
        
        if(totalAfter > game.curArea->getMaxPikminInField()) {
            //Can't eject this Pikmin because we're at the limit.
            //Generate inside.
            nest->pikminInside[t][0]++;
            
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parOnionGenInside, this
                );
            pg.baseParticle.priority = PARTICLE_PRIORITY_LOW;
            particleGenerators.push_back(pg);
            
        } else if(oniType->autoEject || game.curArea->onionsAutoEject) {
            //Generate inside, and let the Onion's tick logic automatically
            //spit a seed or call it out.
            nest->pikminInside[t][0]++;
            
        } else if(
            oniType->ejectGrownPikmin ||
            game.curArea->onionsEjectGrownPikmin
        ) {
            //Generate inside and then immediately eject it out.
            nest->pikminInside[t][0]++;
            nest->requestPikmin(t, 1, nullptr);
            
        } else {
            //Don't generate inside, just spit the seed out directly.
            spit(t);
            
        }
        
        //Only one Pikmin is allowed to generate per function call, so quit
        //out once we generate one.
        return;
    }
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param varsMgr Script var manager to use.
 */
void Onion::readScriptVars(const ScriptVarManager& varsMgr) {
    Mob::readScriptVars(varsMgr);
    
    nest->readScriptVars(varsMgr);
}


/**
 * @brief Handles some nutrients being received, typically from
 * a delivered mob.
 *
 * @param pikType Type of Pikmin that will benefit from these nutrients.
 * @param nutrientAmount Amount of nutrients.
 */
void Onion::receiveNutrients(PikminType* pikType, size_t nutrientAmount) {
    size_t pikTypeIdx = INVALID;
    forIdx(t, oniType->nest->pikTypes) {
        if(oniType->nest->pikTypes[t] == pikType) {
            pikTypeIdx = t;
            break;
        }
    }
    
    if(pikTypeIdx == INVALID) {
        //This Onion doesn't manage this Pikmin type. Too bad.
        return;
    }
    
    stopGenerating();
    nutrients[pikTypeIdx] += nutrientAmount;
    bool willGenerate = false;
    
    while(nutrients[pikTypeIdx] >= oniType->nutrientsPerSeed) {
        generationQueue[pikTypeIdx]++;
        nutrients[pikTypeIdx] -= oniType->nutrientsPerSeed;
        willGenerate = true;
    }
    
    if(willGenerate) {
        generationDelayTimer.start();
    }
}


/**
 * @brief Spits a Pikmin seed right now.
 *
 * @param typeIdx Index of the Pikmin type in the nest's data.
 */
void Onion::spit(size_t typeIdx) {
    if(
        game.states.gameplay->mobs.pikmin.size() >=
        game.curArea->getMaxPikminInField()
    ) {
        return;
    }
    spitPikminSeed(
        center, bottomZ + ONION::NEW_SEED_Z_OFFSET,
        oniType->nest->pikTypes[typeIdx],
        nSpits, ONION::SPIT_H_SPEED, ONION::SPIT_H_SPEED_DEVIATION,
        ONION::SPIT_V_SPEED
    );
    nSpits++;
    playSound(oniType->soundPopIdx);
}


/**
 * @brief Starts generating Pikmin.
 */
void Onion::startGenerating() {
    generationDelayTimer.stop();
    nextGenerationTimer.start();
    string msg = "started_generation";
    game.states.gameplay->sendScriptMessage(this, this, msg);
}


/**
 * @brief Stops generating Pikmin.
 */
void Onion::stopGenerating() {
    generationDelayTimer.stop();
    nextGenerationTimer.stop();
    string msg = "stopped_generation";
    game.states.gameplay->sendScriptMessage(this, this, msg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Onion::tickClassSpecifics(float deltaT) {
    //Timers.
    generationDelayTimer.tick(deltaT);
    nextGenerationTimer.tick(deltaT);
    
    //Auto-ejection.
    if(
        (oniType->autoEject || game.curArea->onionsAutoEject) &&
        game.states.gameplay->mobs.pikmin.size() <
        game.curArea->getMaxPikminInField()
    ) {
        forIdx(t, oniType->nest->pikTypes) {
            if(!nest->hasPikminInside(t)) continue;
            if(
                game.states.gameplay->mobs.pikmin.size() >=
                game.curArea->getMaxPikminInField()
            ) {
                break;
            }
            
            if(
                oniType->ejectGrownPikmin ||
                game.curArea->onionsEjectGrownPikmin
            ) {
                if(nest->callQueue[t] == 0) {
                    nest->requestPikmin(t, 1, nullptr);
                }
            } else {
                spit(t);
            }
        }
    }
    
    //Nest.
    nest->tick(deltaT);
}
