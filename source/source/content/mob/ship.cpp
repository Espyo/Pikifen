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
#include "../../core/misc_functions.h"
#include "../../core/game.h"
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
Ship::Ship(const Point &pos, ShipType* type, float angle) :
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
        float ring_idx_ratio = b / (float) SHIP::CONTROL_POINT_RING_AMOUNT;
        
        float ring_hue = 360 * ring_idx_ratio;
        ALLEGRO_COLOR ring_color = al_color_hsl(ring_hue, 1.0f, 0.8f);
        
        float ring_anim_ratio =
            fmod(
                game.states.gameplay->areaTimePassed +
                SHIP::CONTROL_POINT_ANIM_DUR * ring_idx_ratio,
                SHIP::CONTROL_POINT_ANIM_DUR
            );
        ring_anim_ratio /= SHIP::CONTROL_POINT_ANIM_DUR;
        
        unsigned char ring_alpha = 120;
        
        if(ring_anim_ratio <= 0.3f) {
            //Fading into existence.
            ring_alpha =
                interpolateNumber(
                    ring_anim_ratio,
                    0.0f, 0.3f,
                    0, ring_alpha
                );
        } else if(ring_anim_ratio >= 0.7f) {
            //Shrinking down.
            ring_alpha =
                interpolateNumber(
                    ring_anim_ratio,
                    0.7f, 1.0f,
                    ring_alpha, 0
                );
        }
        
        float ring_scale =
            interpolateNumber(
                ease(EASE_METHOD_IN, ring_anim_ratio),
                0.0f, 1.0f,
                1.0f, 0.3f
            );
        float ring_diameter =
            shiType->controlPointRadius * 2.0f * ring_scale;
            
        drawBitmap(
            game.sysContent.bmpBrightRing,
            controlPointFinalPos, Point(ring_diameter),
            0.0f,
            changeAlpha(ring_color, ring_alpha)
        );
    }
    
    //Drawing the tractor beam rings.
    //Go in reverse to ensure the most recent rings are drawn underneath.
    for(char r = (char) tractorBeamRings.size() - 1; r > 0; r--) {
    
        float ring_anim_ratio =
            tractorBeamRings[r] / SHIP::TRACTOR_BEAM_RING_ANIM_DUR;
            
        unsigned char ring_alpha = 80;
        if(ring_anim_ratio <= 0.3f) {
            //Fading into existence.
            ring_alpha =
                interpolateNumber(
                    ring_anim_ratio,
                    0.0f, 0.3f,
                    0, ring_alpha
                );
        } else if(ring_anim_ratio >= 0.5f) {
            //Shrinking down.
            ring_alpha =
                interpolateNumber(
                    ring_anim_ratio,
                    0.5f, 1.0f,
                    ring_alpha, 0
                );
        }
        
        float ring_brightness =
            interpolateNumber(
                ring_anim_ratio,
                0.0f, 1.0f,
                0.4f, 0.6f
            );
            
        ALLEGRO_COLOR ring_color =
            al_color_hsl(tractorBeamRingColors[r], 1.0f, ring_brightness);
        ring_color = changeAlpha(ring_color, ring_alpha);
        
        float ring_scale =
            interpolateNumber(
                ring_anim_ratio,
                0.0f, 1.0f,
                shiType->controlPointRadius * 2.5f, 1.0f
            );
            
        float distance = controlPointToReceptacleDist * ring_anim_ratio;
        float angle = getAngle(controlPointFinalPos, receptacleFinalPos);
        Point ring_pos(
            controlPointFinalPos.x + cos(angle) * distance,
            controlPointFinalPos.y + sin(angle) * distance
        );
        
        drawBitmap(
            game.sysContent.bmpBrightRing,
            ring_pos,
            Point(ring_scale),
            0.0f,
            ring_color
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
void Ship::readScriptVars(const ScriptVarReader &svr) {
    Mob::readScriptVars(svr);
    
    nest->readScriptVars(svr);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Ship::tickClassSpecifics(float delta_t) {
    nest->tick(delta_t);
    
    if(mobsBeingBeamed > 0) {
        nextTractorBeamRingTimer.tick(delta_t);
    }
    
    for(size_t r = 0; r < tractorBeamRings.size(); ) {
        //Erase rings that have reached the end of their animation.
        tractorBeamRings[r] += delta_t;
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
