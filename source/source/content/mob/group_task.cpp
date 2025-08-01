/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Group task class and group task-related functions.
 */

#include "group_task.h"

#include "../../core/game.h"


/**
 * @brief Constructs a new group task object.
 *
 * @param pos Starting coordinates.
 * @param type Group task type this mob belongs to.
 * @param angle Starting angle.
 */
GroupTask::GroupTask(
    const Point& pos, GroupTaskType* type, float angle
):
    Mob(pos, type, angle),
    tasType(type),
    powerGoal(type->powerGoal) {
    
    //Initialize spots.
    float rowAngle = getAngle(tasType->firstRowP1, tasType->firstRowP2);
    size_t neededRows =
        ceil(tasType->maxPikmin / (float) tasType->pikminPerRow);
    float pointDist =
        Distance(tasType->firstRowP1, tasType->firstRowP2).toFloat();
    float spaceBetweenNeighbors =
        pointDist / (float) (tasType->pikminPerRow - 1);
        
    //Create a transformation based on the anchor -- p1.
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_rotate_transform(&trans, rowAngle);
    al_translate_transform(
        &trans, tasType->firstRowP1.x, tasType->firstRowP1.y
    );
    
    for(size_t r = 0; r < neededRows; r++) {
    
        for(size_t s = 0; s < tasType->pikminPerRow; s++) {
        
            float x;
            if(tasType->pikminPerRow % 2 == 0) {
                x =
                    spaceBetweenNeighbors / 2.0 +
                    spaceBetweenNeighbors * ceil((s - 1.0f) / 2.0);
                x *= (s % 2 == 0) ? 1 : -1;
            } else {
                if(s == 0) {
                    x = 0;
                } else {
                    x = spaceBetweenNeighbors * ceil(s / 2.0);
                    x *= (s % 2 == 0) ? 1 : -1;
                }
            }
            x += pointDist / 2.0f;
            
            Point sPos(x, r * tasType->intervalBetweenRows);
            al_transform_coordinates(&trans, &sPos.x, &sPos.y);
            
            spots.push_back(GroupTaskSpot(sPos));
        }
    }
    
    updateSpotAbsolutePositions();
}


/**
 * @brief Adds a Pikmin to the task as an actual worker.
 *
 * @param who Pikmin to add.
 */
void GroupTask::addWorker(Pikmin* who) {
    for(size_t s = 0; s < spots.size(); s++) {
        if(spots[s].pikminHere == who) {
            spots[s].state = 2;
            break;
        }
    }
    
    bool hadGoal = power >= powerGoal;
    
    switch(tasType->contributionMethod) {
    case GROUP_TASK_CONTRIBUTION_NORMAL: {
        power++;
        break;
    } case GROUP_TASK_CONTRIBUTION_WEIGHT: {
        power += who->pikType->weight;
        break;
    } case GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH: {
        power += who->pikType->carryStrength;
        break;
    } case GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH: {
        power += who->pikType->pushStrength;
        break;
    }
    }
    
    if(!hadGoal && power >= powerGoal) {
        string msg = "goal_reached";
        who->sendScriptMessage(this, msg);
    }
}


/**
 * @brief Code to run when the task is finished.
 */
void GroupTask::finishTask() {
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin.size(); p++
    ) {
        Pikmin* pikPtr = game.states.gameplay->mobs.pikmin[p];
        if(pikPtr->focusedMob && pikPtr->focusedMob == this) {
            pikPtr->fsm.runEvent(MOB_EV_FINISHED_TASK);
            pikPtr->fsm.runEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
        }
    }
}


/**
 * @brief Frees up a previously-reserved spot.
 *
 * @param whose Who had the reservation?
 */
void GroupTask::freeUpSpot(Pikmin* whose) {
    bool wasContributing = false;
    
    for(size_t s = 0; s < spots.size(); s++) {
        if(spots[s].pikminHere == whose) {
            if(spots[s].state == 2) {
                wasContributing = true;
            }
            spots[s].state = 0;
            spots[s].pikminHere = nullptr;
            break;
        }
    }
    
    if(wasContributing) {
        bool hadGoal = power >= powerGoal;
        
        switch(tasType->contributionMethod) {
        case GROUP_TASK_CONTRIBUTION_NORMAL: {
            power--;
            break;
        } case GROUP_TASK_CONTRIBUTION_WEIGHT: {
            power -= whose->pikType->weight;
            break;
        } case GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH: {
            power -= whose->pikType->carryStrength;
            break;
        } case GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH: {
            power -= whose->pikType->pushStrength;
            break;
        }
        }
        
        if(hadGoal && power < powerGoal) {
            string msg = "goal_lost";
            whose->sendScriptMessage(this, msg);
        }
    }
}


/**
 * @brief Returns information on how to show the fraction numbers.
 *
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 *
 * @param outValueNr The fraction's value (upper) number is returned here.
 * @param outReqNr The fraction's required (lower) number is returned here.
 * @param outColor The fraction's color is returned here.
 * @return Whether the fraction numbers should be shown.
 */
bool GroupTask::getFractionNumbersInfo(
    float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
) const {
    if(getPower() <= 0) return false;
    *outValueNr = getPower();
    *outReqNr = powerGoal;
    *outColor = game.config.aestheticGen.carryingColorStop;
    return true;
}


/**
 * @brief Returns a free spot, closest to the center and to the frontmost row as
 * possible.
 *
 * @return The spot, or nullptr if there is none.
 */
GroupTask::GroupTaskSpot* GroupTask::getFreeSpot() {
    size_t spotsTaken = 0;
    
    for(size_t s = 0; s < spots.size(); s++) {
        if(spots[s].state != 0) {
            spotsTaken++;
            if(spotsTaken == tasType->maxPikmin) {
                //Max Pikmin reached! The Pikmin can't join,
                //regardless of there being free spots.
                return nullptr;
            }
        }
        if(spots[s].state == 0) return &(spots[s]);
    }
    
    return nullptr;
}


/**
 * @brief Returns the current power put into the task.
 *
 * @return The power.
 */
float GroupTask::getPower() const {
    return power;
}


/**
 * @brief Returns the current world coordinates of a spot, occupied by a Pikmin.
 *
 * @param whose Pikmin whose spot to check.
 * @return The coordinates, or (0,0) if that Pikmin doesn't have a spot.
 */
Point GroupTask::getSpotPos(const Pikmin* whose) const {
    for(size_t s = 0; s < spots.size(); s++) {
        if(spots[s].pikminHere == whose) {
            return spots[s].absolutePos;
        }
    }
    return Point();
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void GroupTask::readScriptVars(const ScriptVarReader& svr) {
    Mob::readScriptVars(svr);
    
    svr.get("power_goal", powerGoal);
}


/**
 * @brief Reserves a spot for a Pikmin.
 *
 * @param spot Pointer to the spot to reserve.
 * @param who Who will be reserving this spot?
 */
void GroupTask::reserveSpot(GroupTask::GroupTaskSpot* spot, Pikmin* who) {
    spot->state = 1;
    spot->pikminHere = who;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GroupTask::tickClassSpecifics(float deltaT) {
    if(health <= 0 && !ranTaskFinishedCode) {
        ranTaskFinishedCode = true;
        finishTask();
    }
    
    if(health > 0) {
        ranTaskFinishedCode = false;
    }
    
    if(
        chaseInfo.state == CHASE_STATE_CHASING &&
        power >= powerGoal &&
        tasType->speedBonus != 0.0f
    ) {
        //Being moved and movements can go through speed bonuses?
        //Let's update the speed.
        chaseInfo.maxSpeed =
            type->moveSpeed +
            (power - powerGoal) * tasType->speedBonus;
        chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    }
    
    updateSpotAbsolutePositions();
}


/**
 * @brief Updates the absolute position of all spots,
 * based on where the group task mob currently is and where it is
 * currently facing.
 */
void GroupTask::updateSpotAbsolutePositions() {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_rotate_transform(&t, angle);
    al_translate_transform(&t, pos.x, pos.y);
    
    for(size_t s = 0; s < spots.size(); s++) {
        Point* p = &(spots[s].absolutePos);
        *p = spots[s].relativePos;
        al_transform_coordinates(&t, &(p->x), &(p->y));
    }
}


/**
 * @brief Constructs a new group task spot object.
 *
 * @param pos Position of the spot, in relative coordinates.
 */
GroupTask::GroupTaskSpot::GroupTaskSpot(const Point& pos) :
    relativePos(pos),
    absolutePos(pos) {
    
}
