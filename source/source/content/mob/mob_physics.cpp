/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Logic about mob movement, gravity, wall collision, etc.
 */

#include <algorithm>

#include "mob.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"


using std::set;


/**
 * @brief Returns which walkable mob this mob should be considered to be on
 * top of.
 *
 * @return The mob to walk on, or nullptr if none is found.
 */
Mob* Mob::getMobToWalkOn() const {
    //Can't walk on anything if it's moving upwards.
    if(speedZ > 0.0f) return nullptr;
    
    Mob* bestCandidate = nullptr;
    for(size_t m = 0; m < game.states.gameplay->mobs.walkables.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.walkables[m];
        if(mPtr == this) {
            continue;
        }
        if(fabs(z - (mPtr->z + mPtr->height)) > GEOMETRY::STEP_HEIGHT) {
            continue;
        }
        if(bestCandidate && mPtr->z <= bestCandidate->z) {
            continue;
        }
        
        //Check if they collide on X+Y.
        if(
            rectangularDim.x != 0 &&
            mPtr->rectangularDim.x != 0
        ) {
            //Rectangle vs rectangle.
            if(
                !rectanglesIntersect(
                    pos, rectangularDim, angle,
                    mPtr->pos, mPtr->rectangularDim, mPtr->angle
                )
            ) {
                continue;
            }
        } else if(rectangularDim.x != 0) {
            //Rectangle vs circle.
            if(
                !circleIntersectsRectangle(
                    mPtr->pos, mPtr->radius,
                    pos, rectangularDim,
                    angle
                )
            ) {
                continue;
            }
        } else if(mPtr->rectangularDim.x != 0) {
            //Circle vs rectangle.
            if(
                !circleIntersectsRectangle(
                    pos, radius,
                    mPtr->pos, mPtr->rectangularDim,
                    mPtr->angle
                )
            ) {
                continue;
            }
        } else {
            //Circle vs circle.
            if(
                Distance(pos, mPtr->pos) >
                (radius + mPtr->radius)
            ) {
                continue;
            }
        }
        bestCandidate = mPtr;
    }
    return bestCandidate;
}


/**
 * @brief Calculates which edges the mob is intersecting with for
 * horizontal movement physics logic.
 *
 * @param newPos Position to check.
 * @param intersectingEdges List of edges it is intersecting with.
 * @return H_MOVE_RESULT_OK if everything is okay, H_MOVE_RESULT_FAIL if
 * movement is impossible.
 */
H_MOVE_RESULT Mob::getMovementEdgeIntersections(
    const Point &newPos, vector<Edge*>* intersectingEdges
) const {
    //Before checking the edges, let's consult the blockmap and look at
    //the edges in the same blocks the mob is on.
    //This way, we won't check for edges that are really far away.
    //Use the bounding box to know which blockmap blocks the mob will be on.
    set<Edge*> candidateEdges;
    //Use the terrain radius if the mob is moving about and alive.
    //Otherwise if it's a corpse, it can use the regular radius.
    float radiusToUse =
        (type->terrainRadius < 0 || health <= 0) ?
        radius :
        type->terrainRadius;
        
    if(
        !game.curAreaData->bmap.getEdgesInRegion(
            newPos - radiusToUse,
            newPos + radiusToUse,
            candidateEdges
        )
    ) {
        //Somehow out of bounds. No movement.
        return H_MOVE_RESULT_FAIL;
    }
    
    //Go through each edge, and figure out if it is a valid wall for our mob.
    for(auto &ePtr : candidateEdges) {
    
        bool isEdgeBlocking = false;
        
        if(
            !circleIntersectsLineSeg(
                newPos, radiusToUse,
                v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
                nullptr, nullptr
            )
        ) {
            //No intersection? Well, obviously this one doesn't count.
            continue;
        }
        
        if(!ePtr->sectors[0] || !ePtr->sectors[1]) {
            //If we're on the edge of out-of-bounds geometry,
            //block entirely.
            return H_MOVE_RESULT_FAIL;
        }
        
        for(unsigned char s = 0; s < 2; s++) {
            if(ePtr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                isEdgeBlocking = true;
                break;
            }
        }
        
        if(!isEdgeBlocking) {
            if(ePtr->sectors[0]->z == ePtr->sectors[1]->z) {
                //No difference in floor height = no wall.
                //Ignore this.
                continue;
            }
            if(
                ePtr->sectors[0]->z < z &&
                ePtr->sectors[1]->z < z
            ) {
                //An edge whose sectors are below the mob?
                //No collision here.
                continue;
            }
        }
        
        if(
            ePtr->sectors[0]->z > z &&
            ePtr->sectors[1]->z > z
        ) {
            //If both floors of this edge are above the mob...
            //then what does that mean? That the mob is under the ground?
            //Nonsense! Throw this edge away!
            //It's a false positive, and it's likely behind a more logical
            //edge that we actually did collide against.
            continue;
        }
        
        if(
            ePtr->sectors[0]->type == SECTOR_TYPE_BLOCKING &&
            ePtr->sectors[1]->type == SECTOR_TYPE_BLOCKING
        ) {
            //Same logic as the previous check.
            continue;
        }
        
        //Add this edge to the list of intersections, then.
        intersectingEdges->push_back(ePtr);
    }
    
    return H_MOVE_RESULT_OK;
}


/**
 * @brief Calculates how much the mob is going to move horizontally,
 * for the purposes of movement physics calculation.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param moveSpeedMult Movement speed is multiplied by this.
 * @param moveSpeed The calculated move speed is placed in this struct.
 * @return H_MOVE_RESULT_OK on normal movement, H_MOVE_RESULT_TELEPORTED if
 * the mob's X and Y have been set and movement logic can be skipped,
 * and H_MOVE_RESULT_FAIL if movement is entirely impossible this frame.
 */
H_MOVE_RESULT Mob::getPhysicsHorizontalMovement(
    float deltaT, float moveSpeedMult, Point* moveSpeed
) {
    //Held by another mob.
    if(holder.m) {
        Point finalPos = holder.getFinalPos(&z);
        speedZ = 0;
        chase(finalPos, z, CHASE_FLAG_TELEPORT);
    }
    
    //Chasing.
    if(chaseInfo.state == CHASE_STATE_CHASING) {
        Point finalTargetPos = getChaseTarget();
        
        if(hasFlag(chaseInfo.flags, CHASE_FLAG_TELEPORT)) {
        
            Sector* sec =
                getSector(finalTargetPos, nullptr, true);
                
            if(!sec) {
                //No sector, invalid teleport. No move.
                return H_MOVE_RESULT_FAIL;
            }
            
            z = chaseInfo.offsetZ;
            if(chaseInfo.origZ) {
                z += *chaseInfo.origZ;
            }
            
            groundSector = sec;
            centerSector = sec;
            speed.x = speed.y = 0;
            pos = finalTargetPos;
            
            if(!hasFlag(chaseInfo.flags, CHASE_FLAG_TELEPORTS_CONSTANTLY)) {
                chaseInfo.state = CHASE_STATE_FINISHED;
            }
            return H_MOVE_RESULT_TELEPORTED;
            
        } else {
        
            //Make it go to the direction it wants.
            float d = Distance(pos, finalTargetPos).toFloat();
            
            chaseInfo.curSpeed +=
                chaseInfo.acceleration * deltaT;
            chaseInfo.curSpeed =
                std::min(chaseInfo.curSpeed, chaseInfo.maxSpeed);
                
            float moveAmount =
                std::min(
                    (double) (d / deltaT),
                    (double) chaseInfo.curSpeed * moveSpeedMult
                );
                
            bool canFreeMove =
                hasFlag(chaseInfo.flags, CHASE_FLAG_ANY_ANGLE) ||
                d <= MOB::FREE_MOVE_THRESHOLD;
                
            float movementAngle =
                canFreeMove ?
                getAngle(pos, finalTargetPos) :
                angle;
                
            moveSpeed->x = cos(movementAngle) * moveAmount;
            moveSpeed->y = sin(movementAngle) * moveAmount;
        }
        
    } else {
        chaseInfo.acceleration = 0.0f;
        chaseInfo.curSpeed = 0.0f;
        chaseInfo.maxSpeed = 0.0f;
        
    }
    
    //If another mob is pushing it.
    if(pushAmount != 0.0f) {
        //Overly-aggressive pushing results in going through walls.
        //Let's place a cap.
        pushAmount =
            std::min(pushAmount, (float) (radius / deltaT) * 4);
            
        moveSpeed->x +=
            cos(pushAngle) * (pushAmount + MOB::PUSH_EXTRA_AMOUNT);
        moveSpeed->y +=
            sin(pushAngle) * (pushAmount + MOB::PUSH_EXTRA_AMOUNT);
    }
    
    //Scrolling floors.
    if(
        (groundSector->scroll.x != 0 || groundSector->scroll.y != 0) &&
        z <= groundSector->z
    ) {
        (*moveSpeed) += groundSector->scroll;
    }
    
    //On top of a mob.
    if(standingOnMob) {
        (*moveSpeed) += standingOnMob->walkableMoved;
    }
    
    return H_MOVE_RESULT_OK;
}


/**
 * @brief Calculates the angle at which the mob should slide against this wall,
 * for the purposes of movement physics calculations.
 *
 * @param ePtr Pointer to the edge in question.
 * @param wallSector Side index of the sector that actually makes a wall
 * (i.e. the highest).
 * @param moveAngle Angle at which the mob is going to move.
 * @param slideAngle Holds the calculated slide angle.
 * @return H_MOVE_RESULT_OK on success, H_MOVE_RESULT_FAIL if the mob can't
 * slide against this wall.
 */
H_MOVE_RESULT Mob::getWallSlideAngle(
    const Edge* ePtr, unsigned char wallSector, float moveAngle,
    float* slideAngle
) const {
    //The wall's normal is the direction the wall is facing.
    //i.e. the direction from the top floor to the bottom floor.
    //We know which side of an edge is which sector because of
    //the vertexes. Imagine you're in first person view,
    //following the edge as a line on the ground.
    //You start on vertex 0 and face vertex 1.
    //Sector 0 will always be on your left.
    
    float wallNormal;
    float wallAngle =
        getAngle(v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]));
        
    if(wallSector == 0) {
        wallNormal = normalizeAngle(wallAngle + TAU / 4);
    } else {
        wallNormal = normalizeAngle(wallAngle - TAU / 4);
    }
    
    float nd = getAngleCwDiff(wallNormal, moveAngle);
    if(nd < TAU * 0.25 || nd > TAU * 0.75) {
        //If the difference between the movement and the wall's
        //normal is this, that means we came FROM the wall.
        //No way! There has to be an edge that makes more sense.
        return H_MOVE_RESULT_FAIL;
    }
    
    //If we were to slide on this edge, this would be
    //the slide angle.
    if(nd < TAU / 2) {
        //Coming in from the "left" of the normal. Slide right.
        *slideAngle = wallNormal + TAU / 4;
    } else {
        //Coming in from the "right" of the normal. Slide left.
        *slideAngle = wallNormal - TAU / 4;
    }
    
    return H_MOVE_RESULT_OK;
}


/**
 * @brief Ticks physics logic regarding the mob's horizontal movement.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param attemptedMoveSpeed Movement speed to calculate with.
 * @param touchedWall Holds whether or not the mob touched a wall in this move.
 */
void Mob::tickHorizontalMovementPhysics(
    float deltaT, const Point &attemptedMoveSpeed,
    bool* touchedWall
) {
    if(attemptedMoveSpeed.x == 0 && attemptedMoveSpeed.y == 0) {
        //No movement. Nothing to do here.
        return;
    }
    
    //Setup.
    bool finishedMoving = false;
    bool doingSlide = false;
    
    Point newPos = pos;
    Point moveSpeed = attemptedMoveSpeed;
    
    //Try placing it in the place it should be at, judging
    //from the movement speed.
    while(!finishedMoving) {
    
        //Start by checking sector collisions.
        //For this, we will only check if the mob is intersecting
        //with any edge. With this, we trust that mobs can't go so fast
        //that they're fully on one side of an edge in one frame,
        //and the other side on the next frame.
        //It's pretty naive...but it works!
        bool successfulMove = true;
        
        newPos.x = pos.x + deltaT* moveSpeed.x;
        newPos.y = pos.y + deltaT* moveSpeed.y;
        float newZ = z;
        
        //Get the sector the mob will be on.
        Sector* newCenterSector = getSector(newPos, nullptr, true);
        Sector* newGroundSector = newCenterSector;
        Sector* stepSector = newCenterSector;
        
        if(!newCenterSector) {
            //Out of bounds. No movement.
            return;
        }
        if(z + GEOMETRY::STEP_HEIGHT < newCenterSector->z) {
            //We can't walk onto this sector. Refuse the move.
            return;
        }
        //Get all edges it collides against in this new position.
        vector<Edge*> intersectingEdges;
        if(
            getMovementEdgeIntersections(newPos, &intersectingEdges) ==
            H_MOVE_RESULT_FAIL
        ) {
            return;
        }
        
        //For every sector in the new position, let's figure out
        //the ground sector, and also a stepping sector, if possible.
        for(size_t e = 0; e < intersectingEdges.size(); e++) {
            Edge* ePtr = intersectingEdges[e];
            Sector* tallestSector = groundSector; //Tallest of the two.
            if(
                ePtr->sectors[0]->type != SECTOR_TYPE_BLOCKING &&
                ePtr->sectors[1]->type != SECTOR_TYPE_BLOCKING
            ) {
                if(ePtr->sectors[0]->z > ePtr->sectors[1]->z) {
                    tallestSector = ePtr->sectors[0];
                } else {
                    tallestSector = ePtr->sectors[1];
                }
            }
            
            if(
                tallestSector->z > newGroundSector->z &&
                tallestSector->z <= z
            ) {
                newGroundSector = tallestSector;
            }
            
            //Check if it can go up this step.
            //It can go up this step if the floor is within
            //stepping distance of the mob's current Z,
            //and if this step is larger than any step
            //encountered of all edges crossed.
            if(
                !hasFlag(flags, MOB_FLAG_WAS_THROWN) &&
                tallestSector->z <= z + GEOMETRY::STEP_HEIGHT &&
                tallestSector->z > stepSector->z
            ) {
                stepSector = tallestSector;
            }
        }
        
        //Mosey on up to the step sector, if any.
        if(stepSector->z > newGroundSector->z) {
            newGroundSector = stepSector;
        }
        if(z < stepSector->z) newZ = stepSector->z;
        
        //Figure out sliding logic now, if needed.
        float moveAngle;
        float totalMoveSpeed;
        coordinatesToAngle(
            moveSpeed, &moveAngle, &totalMoveSpeed
        );
        
        //Angle to slide towards.
        float slideAngle = moveAngle;
        //Difference between the movement angle and the slide.
        float slideAngleDiff = 0;
        
        //Check the sector heights of the intersecting edges to figure out
        //which are really walls, and how to slide against them.
        for(size_t e = 0; e < intersectingEdges.size(); e++) {
            Edge* ePtr = intersectingEdges[e];
            bool isEdgeWall = false;
            unsigned char wallSector = 0;
            
            for(unsigned char s = 0; s < 2; s++) {
                if(ePtr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                    isEdgeWall = true;
                    wallSector = s;
                }
            }
            
            if(!isEdgeWall) {
                for(unsigned char s = 0; s < 2; s++) {
                    if(ePtr->sectors[s]->z > newZ) {
                        isEdgeWall = true;
                        wallSector = s;
                    }
                }
            }
            
            //This isn't a wall... Get out of here, faker.
            if(!isEdgeWall) continue;
            
            //Ok, there's obviously been a collision, so let's work out what
            //wall the mob will slide on.
            
            if(!doingSlide) {
                float tentativeSlideAngle;
                if(
                    getWallSlideAngle(
                        ePtr, wallSector, moveAngle, &tentativeSlideAngle
                    ) == H_MOVE_RESULT_FAIL
                ) {
                    continue;
                }
                
                float sd =
                    getAngleSmallestDiff(moveAngle, tentativeSlideAngle);
                if(sd > slideAngleDiff) {
                    slideAngleDiff = sd;
                    slideAngle = tentativeSlideAngle;
                }
            }
            
            //By the way, if we got to this point, that means there are real
            //collisions happening. Let's mark this move as unsuccessful.
            successfulMove = false;
            (*touchedWall) = true;
        }
        
        //If the mob is just slamming against the wall head-on, perpendicularly,
        //then forget any idea about sliding.
        //It'd just be awkwardly walking in place.
        //Reset its horizontal position, but keep calculations for
        //everything else.
        if(!successfulMove && slideAngleDiff > TAU / 4 - 0.05) {
            newPos = pos;
            successfulMove = true;
        }
        
        
        //We're done checking. If the move was unobstructed, good, go there.
        //If not, we'll use the info we gathered before to calculate sliding,
        //and try again.
        
        if(successfulMove) {
            //Good news, the mob can be placed in this new spot freely.
            pos = newPos;
            z = newZ;
            groundSector = newGroundSector;
            centerSector = newCenterSector;
            finishedMoving = true;
            
        } else {
        
            //Try sliding.
            if(doingSlide) {
                //We already tried sliding, and we still hit something...
                //Let's just stop completely. This mob can't go forward.
                finishedMoving = true;
                
            } else {
                doingSlide = true;
                //To limit the speed, we should use a cross-product of the
                //movement and slide vectors.
                //But nuts to that, this is just as nice, and a lot simpler!
                totalMoveSpeed *= 1 - (slideAngleDiff / TAU / 2);
                moveSpeed =
                    angleToCoordinates(slideAngle, totalMoveSpeed);
                    
            }
        }
    }
}


/**
 * @brief Ticks the mob's actual physics procedures:
 * falling because of gravity, moving forward, etc.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickPhysics(float deltaT) {
    if(!groundSector) {
        //Object is placed out of bounds.
        return;
    }
    
    //Initial setup.
    float moveSpeedMult = getSpeedMultiplier();
    
    Point preMovePos = pos;
    Point moveSpeed = speed;
    bool touchedWall = false;
    float preMoveGroundZ = groundSector->z;
    
    //Rotation logic.
    tickRotationPhysics(deltaT, moveSpeedMult);
    
    //What type of horizontal movement is this?
    H_MOVE_RESULT hMoveType =
        getPhysicsHorizontalMovement(deltaT, moveSpeedMult, &moveSpeed);
        
    switch (hMoveType) {
    case H_MOVE_RESULT_FAIL: {
        return;
    } case H_MOVE_RESULT_TELEPORTED: {
        break;
    } case H_MOVE_RESULT_OK: {
        //Horizontal movement time!
        tickHorizontalMovementPhysics(
            deltaT, moveSpeed, &touchedWall
        );
        break;
    }
    }
    
    //Vertical movement.
    tickVerticalMovementPhysics(
        deltaT, preMoveGroundZ, hMoveType == H_MOVE_RESULT_TELEPORTED
    );
    
    //Walk on top of another mob, if possible.
    if(type->canWalkOnOthers) tickWalkableRidingPhysics(deltaT);
    
    //Final setup.
    pushAmount = 0;
    
    if(touchedWall) {
        fsm.runEvent(MOB_EV_TOUCHED_WALL);
    }
    
    if(type->walkable) {
        walkableMoved = (pos - preMovePos) / deltaT;
    }
}


/**
 * @brief Ticks physics logic regarding the mob rotating.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param moveSpeedMult Movement speed is multiplied by this.
 */
void Mob::tickRotationPhysics(
    float deltaT, float moveSpeedMult
) {
    //Change the facing angle to the angle the mob wants to face.
    if(angle > TAU / 2)  angle -= TAU;
    if(angle < -TAU / 2) angle += TAU;
    if(intendedTurnPos) {
        intendedTurnAngle = getAngle(pos, *intendedTurnPos);
    }
    if(intendedTurnAngle > TAU / 2)  intendedTurnAngle -= TAU;
    if(intendedTurnAngle < -TAU / 2) intendedTurnAngle += TAU;
    
    float angleDiff = intendedTurnAngle - angle;
    if(angleDiff > TAU / 2)  angleDiff -= TAU;
    if(angleDiff < -TAU / 2) angleDiff += TAU;
    
    angle +=
        sign(angleDiff) * std::min(
            (double) (type->rotationSpeed * moveSpeedMult * deltaT),
            (double) fabs(angleDiff)
        );
        
    if(holder.m) {
        switch(holder.rotationMethod) {
        case HOLD_ROTATION_METHOD_FACE_HOLDER: {
            float dummy;
            Point finalPos = holder.getFinalPos(&dummy);
            angle = getAngle(finalPos, holder.m->pos);
            stopTurning();
            break;
        } case HOLD_ROTATION_METHOD_COPY_HOLDER: {
            angle = holder.m->angle;
            stopTurning();
            break;
        } default: {
            break;
        }
        }
    }
    
    angleCos = cos(angle);
    angleSin = sin(angle);
}


/**
 * @brief Ticks physics logic regarding the mob's vertical movement.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param preMoveGroundZ Z of the floor before horizontal movement started.
 * @param wasTeleport Did the mob just teleport previously in the
 * movement logic?
 */
void Mob::tickVerticalMovementPhysics(
    float deltaT, float preMoveGroundZ,
    bool wasTeleport
) {
    bool applyGravity = true;
    
    if(!standingOnMob) {
        //If the current ground is one step (or less) below
        //the previous ground, just instantly go down the step.
        if(
            preMoveGroundZ - groundSector->z <= GEOMETRY::STEP_HEIGHT &&
            z == preMoveGroundZ
        ) {
            z = groundSector->z;
        }
    }
    
    //Vertical chasing.
    if(
        chaseInfo.state == CHASE_STATE_CHASING &&
        hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        !hasFlag(chaseInfo.flags, CHASE_FLAG_TELEPORT)
    ) {
        applyGravity = false;
        
        float targetZ = chaseInfo.offsetZ;
        if(chaseInfo.origZ) targetZ += *chaseInfo.origZ;
        float diffZ = fabs(targetZ - z);
        
        speedZ =
            std::min((float) (diffZ / deltaT), chaseInfo.curSpeed);
        if(targetZ < z) {
            speedZ = -speedZ;
        }
        
        z += speedZ * deltaT;
    }
    
    //Gravity.
    if(
        applyGravity && !hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        !holder.m && !wasTeleport
    ) {
        //Use Velocity Verlet for better results.
        //https://youtu.be/hG9SzQxaCm8
        z +=
            (speedZ * deltaT) +
            ((MOB::GRAVITY_ADDER * gravityMult / 2.0f) * deltaT* deltaT);
        speedZ += MOB::GRAVITY_ADDER * deltaT* gravityMult;
    }
    
    //Landing.
    Hazard* newOnHazard = nullptr;
    if(speedZ <= 0) {
        if(standingOnMob) {
            z = standingOnMob->z + standingOnMob->height;
            speedZ = 0;
            disableFlag(flags, MOB_FLAG_WAS_THROWN);
            fsm.runEvent(MOB_EV_LANDED);
            stopHeightEffect();
            highestMidairZ = FLT_MAX;
            
        } else if(z <= groundSector->z) {
            z = groundSector->z;
            speedZ = 0;
            disableFlag(flags, MOB_FLAG_WAS_THROWN);
            fsm.runEvent(MOB_EV_LANDED);
            stopHeightEffect();
            highestMidairZ = FLT_MAX;
            
            if(groundSector->isBottomlessPit) {
                fsm.runEvent(MOB_EV_BOTTOMLESS_PIT);
            }
            
            if(groundSector->hazard) {
                fsm.runEvent(
                    MOB_EV_TOUCHED_HAZARD,
                    (void*) groundSector->hazard
                );
                newOnHazard = groundSector->hazard;
            }
        }
    }
    
    if(z > groundSector->z) {
        if(highestMidairZ == FLT_MAX) highestMidairZ = z;
        else highestMidairZ = std::max(z, highestMidairZ);
    }
    
    //Held Pikmin are also touching the same hazards as the leader.
    if(holder.m && holder.m->type->category->id == MOB_CATEGORY_LEADERS) {
        Sector* leaderGround = holder.m->groundSector;
        if(leaderGround && holder.m->z <= leaderGround->z) {
            if(leaderGround->hazard) {
                fsm.runEvent(
                    MOB_EV_TOUCHED_HAZARD,
                    (void*) leaderGround->hazard
                );
                newOnHazard = leaderGround->hazard;
            }
        }
    }
    
    //Due to framerate imperfections, thrown Pikmin/leaders can reach higher
    //than intended. z_cap forces a cap. FLT_MAX = no cap.
    if(speedZ <= 0) {
        zCap = FLT_MAX;
    } else if(zCap < FLT_MAX) {
        z = std::min(z, zCap);
    }
    
    //On a sector that has a hazard that is not on the floor.
    if(
        groundSector->hazard &&
        !groundSector->hazardFloor &&
        z > groundSector->z
    ) {
        fsm.runEvent(
            MOB_EV_TOUCHED_HAZARD,
            (void*) groundSector->hazard
        );
        newOnHazard = groundSector->hazard;
    }
    
    //Check if any hazards have been left.
    if(newOnHazard != onHazard && onHazard != nullptr) {
        fsm.runEvent(
            MOB_EV_LEFT_HAZARD,
            (void*) onHazard
        );
        
        for(size_t s = 0; s < statuses.size(); s++) {
            if(statuses[s].type->removeOnHazardLeave) {
                statuses[s].toDelete = true;
            }
        }
        deleteOldStatusEffects();
    }
    onHazard = newOnHazard;
    
    //Quick panic check: if it's somehow inside the ground, pop it out.
    z = std::max(z, groundSector->z);
}


/**
 * @brief Ticks physics logic regarding landing on top of a walkable mob.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickWalkableRidingPhysics(float deltaT) {
    Mob* riderAddedEvMob = nullptr;
    Mob* riderRemovedEvMob = nullptr;
    Mob* newStandingOnMob = getMobToWalkOn();
    
    //Check which mob it is on top of, if any.
    if(newStandingOnMob) {
        z = newStandingOnMob->z + newStandingOnMob->height;
    }
    
    if(newStandingOnMob != standingOnMob) {
        if(standingOnMob) {
            riderRemovedEvMob = standingOnMob;
        }
        if(newStandingOnMob) {
            riderAddedEvMob = newStandingOnMob;
        }
    }
    
    standingOnMob = newStandingOnMob;
    
    if(riderRemovedEvMob) {
        riderRemovedEvMob->fsm.runEvent(
            MOB_EV_RIDER_REMOVED, (void*) this
        );
        if(type->weight != 0.0f) {
            riderRemovedEvMob->fsm.runEvent(
                MOB_EV_WEIGHT_REMOVED, (void*) this
            );
        }
    }
    if(riderAddedEvMob) {
        riderAddedEvMob->fsm.runEvent(
            MOB_EV_RIDER_ADDED, (void*) this
        );
        if(type->weight != 0.0f) {
            riderAddedEvMob->fsm.runEvent(
                MOB_EV_WEIGHT_ADDED, (void*) this
            );
        }
    }
}
