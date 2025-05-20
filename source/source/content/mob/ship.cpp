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

#include "ship.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "leader.h"


namespace SHIP {

//Animate the control point's ring for this long.
const float CONTROL_POINT_ANIM_DUR = 10.0f;

//The amount of rings the ship's control point has.
const unsigned char CONTROL_POINT_RING_AMOUNT = 4;

//How often the tractor beam generates a ring.
const float TRACTOR_BEAM_EMIT_RATE = 0.15f;

//Animate each tractor beam ring for this long.
const float TRACTOR_BEAM_RING_ANIM_DUR = 0.8f;

}


/**
 * @brief Constructs a new ship object.
 *
 * @param pos Starting coordinates.
 * @param type Ship type this mob belongs to.
 * @param angle Starting angle.
 */
Ship::Ship(const Point& pos, ShipType* type, float angle) :
    Mob(pos, type, angle),
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
    
    nextTractorBeamRingTimer.onEnd = [this] () {
        nextTractorBeamRingTimer.start();
        tractorBeamRings.push_back(0);
        float hue =
            fmod(
                game.states.gameplay->areaTimePassed * 360, 360
            );
            
        tractorBeamRingColors.push_back(hue);
    };
    nextTractorBeamRingTimer.start();
    
    nest = new PikminNest(this, shiType->nest);
    
    controlPointFinalPos += pos;
    receptacleFinalPos += pos;
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
        
        unsigned char ringAlpha = 120;
        
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
                ease(EASE_METHOD_IN, ringAnimRatio),
                0.0f, 1.0f,
                1.0f, 0.3f
            );
        float ringDiameter =
            shiType->controlPointRadius * 2.0f * ringScale;
            
        drawBitmap(
            game.sysContent.bmpBrightRing,
            controlPointFinalPos, Point(ringDiameter),
            0.0f,
            changeAlpha(ringColor, ringAlpha)
        );
    }
    
    //Drawing the tractor beam rings.
    //Go in reverse to ensure the most recent rings are drawn underneath.
    for(char r = (char) tractorBeamRings.size() - 1; r > 0; r--) {
    
        float ringAnimRatio =
            tractorBeamRings[r] / SHIP::TRACTOR_BEAM_RING_ANIM_DUR;
            
        unsigned char ringAlpha = 80;
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
            al_color_hsl(tractorBeamRingColors[r], 1.0f, ringBrightness);
        ringColor = changeAlpha(ringColor, ringAlpha);
        
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
    
    Mob::drawMob();
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
        Distance(l->pos, controlPointFinalPos) <=
        shiType->controlPointRadius;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Ship::readScriptVars(const ScriptVarReader& svr) {
    Mob::readScriptVars(svr);
    
    nest->readScriptVars(svr);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Ship::tickClassSpecifics(float deltaT) {
    nest->tick(deltaT);
    
    if(mobsBeingBeamed > 0) {
        nextTractorBeamRingTimer.tick(deltaT);
    }
    
    for(size_t r = 0; r < tractorBeamRings.size(); ) {
        //Erase rings that have reached the end of their animation.
        tractorBeamRings[r] += deltaT;
        if(tractorBeamRings[r] > SHIP::TRACTOR_BEAM_RING_ANIM_DUR) {
            tractorBeamRings.erase(
                tractorBeamRings.begin() + r
            );
            tractorBeamRingColors.erase(
                tractorBeamRingColors.begin() + r
            );
        } else {
            r++;
        }
    }
    
}
