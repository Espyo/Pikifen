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

#include "onion.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"


using std::size_t;
using std::string;


namespace ONION {

//How quickly an Onion fades to and from see-through, in values per second.
const float FADE_SPEED = 255.0f;

//Delay before the Onion starts the generation process.
const float GENERATION_DELAY = 2.0f;

//An Onion-spat seed starts with this Z offset from the Onion.
const float NEW_SEED_Z_OFFSET = 320.0f;

//Interval between each individual Pikmin generation.
const float NEXT_GENERATION_INTERVAL = 0.10f;

//Onion opacity when it goes see-through.
const unsigned char SEETHROUGH_ALPHA = 128;

//After spitting a seed, the next seed's angle shifts by this much.
const float SPIT_ANGLE_SHIFT = TAU * 0.12345;

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
 * @param pos Starting coordinates.
 * @param type Onion type this mob belongs to.
 * @param angle Starting angle.
 */
Onion::Onion(const Point& pos, OnionType* type, float angle) :
    Mob(pos, type, angle),
    oniType(type) {
    
    nest = new PikminNest(this, oniType->nest);
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravityMult = 0.0f;
    z++;
    
    generationDelayTimer.onEnd =
    [this] () { startGenerating(); };
    nextGenerationTimer.onEnd =
    [this] () {
        for(size_t t = 0; t < oniType->nest->pikTypes.size(); t++) {
            if(generationQueue[t] > 0) {
                nextGenerationTimer.start();
                generate();
                return;
            }
        }
        stopGenerating();
    };
    
    for(size_t t = 0; t < oniType->nest->pikTypes.size(); t++) {
        generationQueue.push_back(0);
    }
}


/**
 * @brief Destroys the Onion object.
 */
Onion::~Onion() {
    delete nest;
}


/**
 * @brief Draws an Onion.
 */
void Onion::drawMob() {
    Sprite* curSPtr;
    Sprite* nextSPtr;
    float interpolationFactor;
    getSpriteData(&curSPtr, &nextSPtr, &interpolationFactor);
    if(!curSPtr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    eff.tintColor.a *= (seethrough / 255.0f);
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);
}


/**
 * @brief Spit a Pikmin seed in the queue or add it to the Onion's storage.
 */
void Onion::generate() {
    for(size_t t = 0; t < generationQueue.size(); t++) {
        if(generationQueue[t] == 0) continue;
        
        generationQueue[t]--;
        
        game.statistics.pikminBirths++;
        game.states.gameplay->pikminBorn++;
        game.states.gameplay->pikminBornPerType[
            oniType->nest->pikTypes[t]
        ]++;
        game.states.gameplay->lastPikminBornPos = pos;
        
        size_t totalAfter =
            game.states.gameplay->mobs.pikmin.size() + 1;
            
        if(totalAfter > game.config.rules.maxPikminInField) {
            nest->pikminInside[t][0]++;
            
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parOnionGenInside, this
                );
            pg.baseParticle.priority = PARTICLE_PRIORITY_LOW;
            particleGenerators.push_back(pg);
            
            return;
        }
        
        float horizontalStrength =
            ONION::SPIT_H_SPEED +
            game.rng.f(
                -ONION::SPIT_H_SPEED_DEVIATION,
                ONION::SPIT_H_SPEED_DEVIATION
            );
        spitPikminSeed(
            pos, z + ONION::NEW_SEED_Z_OFFSET, oniType->nest->pikTypes[t],
            nextSpitAngle, horizontalStrength, ONION::SPIT_V_SPEED
        );
        
        nextSpitAngle += ONION::SPIT_ANGLE_SHIFT;
        nextSpitAngle = normalizeAngle(nextSpitAngle);
        
        playSound(oniType->soundPopIdx);
        
        return;
    }
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Onion::readScriptVars(const ScriptVarReader& svr) {
    Mob::readScriptVars(svr);
    
    nest->readScriptVars(svr);
}


/**
 * @brief Starts generating Pikmin.
 */
void Onion::startGenerating() {
    generationDelayTimer.stop();
    nextGenerationTimer.start();
    string msg = "started_generation";
    sendScriptMessage(this, msg);
}


/**
 * @brief Stops generating Pikmin.
 */
void Onion::stopGenerating() {
    generationDelayTimer.stop();
    nextGenerationTimer.stop();
    string msg = "stopped_generation";
    sendScriptMessage(this, msg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Onion::tickClassSpecifics(float deltaT) {
    generationDelayTimer.tick(deltaT);
    nextGenerationTimer.tick(deltaT);
    
    unsigned char finalAlpha = 255;
    
    for(const Player& player : game.states.gameplay->players) {
        if(!player.leaderPtr) continue;
        if(
            bBoxCheck(
                player.leaderPtr->pos, pos,
                player.leaderPtr->radius + radius * 3
            )
        ) {
            finalAlpha = ONION::SEETHROUGH_ALPHA;
        }
        
        if(
            bBoxCheck(
                player.leaderCursorWorld, pos,
                player.leaderPtr->radius + radius * 3
            )
        ) {
            finalAlpha = ONION::SEETHROUGH_ALPHA;
        }
    }
    
    if(seethrough != finalAlpha) {
        if(finalAlpha < seethrough) {
            seethrough =
                std::max(
                    (double) finalAlpha,
                    (double) seethrough - ONION::FADE_SPEED * deltaT
                );
        } else {
            seethrough =
                std::min(
                    (double) finalAlpha,
                    (double) seethrough + ONION::FADE_SPEED * deltaT
                );
        }
    }
    
    nest->tick(deltaT);
}
