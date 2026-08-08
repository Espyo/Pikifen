/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include <allegro5/allegro_color.h>

#include "ship.hpp"

#include "../../core/drawing.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/allegro_utils.hpp"
#include "../../util/general_utils.hpp"
#include "../../util/geometry_utils.hpp"
#include "leader.hpp"


namespace SHIP {

//How often the beam generates a ring.
const float BEAM_EMIT_RATE = 0.15f;

//Animate each beam ring for this long.
const float BEAM_RING_ANIM_DUR = 0.8f;

//Animate the control point's ring for this long.
const float CONTROL_POINT_ANIM_DUR = 10.0f;

//The amount of rings the ship's control point has.
const unsigned char CONTROL_POINT_RING_AMOUNT = 4;

}


/**
 * @brief Constructs a new ship object.
 *
 * @param center Starting center coordinates.
 * @param type Ship type this mob belongs to.
 * @param angle Starting angle.
 */
Ship::Ship(const Point& center, ShipType* type, float angle) :
    Mob(center, type, angle),
    shiType(type),
    controlPointFinalPos(
        rotatePoint(type->controlPointOffset, angle)
    ),
    receptacleFinalPos(
        rotatePoint(type->receptacleOffset, angle)
    ),
    controlPointToReceptacleDist(
        Distance(controlPointFinalPos, receptacleFinalPos).toFloat()
    ) {
    
    nextBeamRingTimer.onEnd = [this] () {
        nextBeamRingTimer.start();
        beamRings.push_back(0);
        float hue =
            fmod(
                game.states.gameplay->areaTimePassed * 360, 360
            );
            
        beamRingColors.push_back(hue);
    };
    nextBeamRingTimer.start();
    
    nest = new PikminNest(this, shiType->nest);
    
    controlPointFinalPos += center;
    receptacleFinalPos += center;
}


/**
 * @brief Destroys the ship object.
 */
Ship::~Ship() {
    delete nest;
}


/**
 * @brief Draws a ship.
 */
void Ship::drawMob() {
    //Draw the rings on the control point.
    for(unsigned char b = 0; b < SHIP::CONTROL_POINT_RING_AMOUNT; b++) {
        float ringIdxRatio = b / (float) SHIP::CONTROL_POINT_RING_AMOUNT;
        
        float ringHue = 360 * ringIdxRatio;
        ALLEGRO_COLOR ringColor = al_color_hsl(ringHue, 1.0f, 0.8f);
        
        float ringAnimRatio =
            fmod(
                game.states.gameplay->areaTimePassed +
                SHIP::CONTROL_POINT_ANIM_DUR * ringIdxRatio,
                SHIP::CONTROL_POINT_ANIM_DUR
            );
        ringAnimRatio /= SHIP::CONTROL_POINT_ANIM_DUR;
        
        float ringAlpha = 0.45f;
        
        if(ringAnimRatio <= 0.3f) {
            //Fading into existence.
            ringAlpha =
                interpolateNumber(
                    ringAnimRatio,
                    0.0f, 0.3f,
                    0, ringAlpha
                );
        } else if(ringAnimRatio >= 0.7f) {
            //Shrinking down.
            ringAlpha =
                interpolateNumber(
                    ringAnimRatio,
                    0.7f, 1.0f,
                    ringAlpha, 0
                );
        }
        
        float ringScale =
            interpolateNumber(
                ease(ringAnimRatio, EASE_METHOD_IN),
                0.0f, 1.0f,
                1.0f, 0.3f
            );
        float ringDiameter =
            shiType->controlPointRadius * 2.0f * ringScale;
            
        drawBitmap(
            game.sysContent.bmpBrightRing,
            controlPointFinalPos, Point(ringDiameter),
            0.0f,
            changeAlpha(ringColor, ringAlpha * 255)
        );
    }
    
    //Drawing the beam rings.
    //Go in reverse to ensure the most recent rings are drawn underneath.
    for(char r = (char) beamRings.size() - 1; r > 0; r--) {
        float ringAnimRatio =
            beamRings[r] / SHIP::BEAM_RING_ANIM_DUR;
            
        float ringAlpha = 0.30f;
        if(ringAnimRatio <= 0.3f) {
            //Fading into existence.
            ringAlpha =
                interpolateNumber(
                    ringAnimRatio,
                    0.0f, 0.3f,
                    0, ringAlpha
                );
        } else if(ringAnimRatio >= 0.5f) {
            //Shrinking down.
            ringAlpha =
                interpolateNumber(
                    ringAnimRatio,
                    0.5f, 1.0f,
                    ringAlpha, 0
                );
        }
        
        float ringBrightness =
            interpolateNumber(
                ringAnimRatio,
                0.0f, 1.0f,
                0.4f, 0.6f
            );
            
        ALLEGRO_COLOR ringColor =
            al_color_hsl(beamRingColors[r], 1.0f, ringBrightness);
        ringColor = changeAlpha(ringColor, ringAlpha * 255);
        
        float ringScale =
            interpolateNumber(
                ringAnimRatio,
                0.0f, 1.0f,
                shiType->controlPointRadius * 2.5f, 1.0f
            );
            
        float distance = controlPointToReceptacleDist * ringAnimRatio;
        float angle = getAngle(controlPointFinalPos, receptacleFinalPos);
        Point ringPos(
            controlPointFinalPos.x + cos(angle) * distance,
            controlPointFinalPos.y + sin(angle) * distance
        );
        
        drawBitmap(
            game.sysContent.bmpBrightRing,
            ringPos,
            Point(ringScale),
            0.0f,
            ringColor
        );
    }
    
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
        SPRITE_BMP_EFFECT_DELIVERY |
        (type->useDamageSquashAndStretch ? SPRITE_BMP_EFFECT_DAMAGE : 0)
    );
    
    eff.tintColor.a *= seeThrough;
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);
}


/**
* @brief Heals a leader, causes particle effects, etc.
*
* @param l Leader to heal.
*/
void Ship::healLeader(Leader* l) const {
    l->setHealth(false, true, 1.0);
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parLeaderHeal, l
        );
    l->particleGenerators.push_back(pg);
}


/**
 * @brief Checks whether the specified leader is currently on the ship's
 * control point or not.
 *
 * @param l Leader to check.
 * @return Whether the leader is on the control point.
 */
bool Ship::isLeaderOnCp(const Leader* l) const {
    return
        Distance(l->center, controlPointFinalPos) <=
        shiType->controlPointRadius;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param varsMgr Script var manager to use.
 */
void Ship::readScriptVars(const ScriptVarManager& varsMgr) {
    Mob::readScriptVars(varsMgr);
    
    nest->readScriptVars(varsMgr);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Ship::tickClassSpecifics(float deltaT) {
    nest->tick(deltaT);
    
    if(mobsBeingBeamed > 0) {
        nextBeamRingTimer.tick(deltaT);
    }
    
    for(size_t r = 0; r < beamRings.size(); ) {
        //Delete rings that have reached the end of their animation.
        beamRings[r] += deltaT;
        if(beamRings[r] > SHIP::BEAM_RING_ANIM_DUR) {
            beamRings.erase(
                beamRings.begin() + r
            );
            beamRingColors.erase(
                beamRingColors.begin() + r
            );
        } else {
            r++;
        }
    }
    
    //See-through effect.
    if(shiType->canTurnSeeThrough) {
        float finalAlpha = 1.0f;
        
        forIdx(p, game.states.gameplay->players) {
            Player& player = game.states.gameplay->players[p];
            if(!player.leaderPtr) continue;
            if(
                bBoxCheck(
                    player.leaderPtr->center, center,
                    player.leaderPtr->radius + radius
                )
            ) {
                finalAlpha = ONION::SEE_THROUGH_ALPHA;
            }
            
            if(
                bBoxCheck(
                    player.leaderCursorWorld, center,
                    player.leaderPtr->radius + radius
                )
            ) {
                finalAlpha = ONION::SEE_THROUGH_ALPHA;
            }
        }
        
        if(seeThrough != finalAlpha) {
            if(finalAlpha < seeThrough) {
                seeThrough =
                    std::max(
                        finalAlpha, seeThrough - ONION::FADE_SPEED * deltaT
                    );
            } else {
                seeThrough =
                    std::min(
                        finalAlpha, seeThrough + ONION::FADE_SPEED * deltaT
                    );
            }
        }
    }
}
