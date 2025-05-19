/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob class and mob-related functions.
 */

#include <algorithm>

#include "mob.h"

#include "../../core/const.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"
#include "../other/mob_script_action.h"
#include "pikmin.h"
#include "ship.h"
#include "tool.h"
#include "track.h"


namespace MOB {

//Acceleration for a mob that's being carried.
const float CARRIED_MOB_ACCELERATION = 100.0f;

//Radius around a spot that a stuck carried object should circle.
const float CARRY_STUCK_CIRCLING_RADIUS = 8.0f;

//When a carried object is stuck, multiply the carrying speed by this.
const float CARRY_STUCK_SPEED_MULTIPLIER = 0.4f;

//When a carried mob sways around, rotate it by this much.
const float CARRY_SWAY_ROTATION_AMOUNT = TAU * 0.01f;

//When a carried mob sways around, multiply time by this.
const float CARRY_SWAY_TIME_MULT = 4.5f;

//When a carried mob sways around, offset X by this much.
const float CARRY_SWAY_X_TRANSLATION_AMOUNT = 2.0f;

//When a carried mob sways around, offset Y by this much.
const float CARRY_SWAY_Y_TRANSLATION_AMOUNT =
    CARRY_SWAY_X_TRANSLATION_AMOUNT / 2.0f;
    
//How much to change the scale by during a damage squash and stretch animation.
const float DAMAGE_SQUASH_AMOUNT = 0.04f;

//Duration of the damage squash and stretch animation.
const float DAMAGE_SQUASH_DURATION = 0.25f;

//When a mob shakes during delivery, this is the shake multiplier.
const float DELIVERY_SUCK_SHAKING_MULT = 4.0f;

//When a mob shakes during delivery, multiply time by this.
const float DELIVERY_SUCK_SHAKING_TIME_MULT = 60.0f;

//How long to suck a mob in for, when being delivered to an Onion/ship.
const float DELIVERY_SUCK_TIME = 2.0f;

//Multiply the offset by this much, when doing a delivery toss.
const float DELIVERY_TOSS_MULT = 40.0f;

//How long to toss a mob in the air for, when being delivered to a mob.
const float DELIVERY_TOSS_TIME = 1.0f;

//Multiply the offset by this much, when winding up for a delivery toss.
const float DELIVERY_TOSS_WINDUP_MULT = 5.0f;

//Randomly vary X by this much, when doing a delivery toss.
const float DELIVERY_TOSS_X_OFFSET = 20.0f;

//If a mob is this close to the destination, it can move without tank controls.
const float FREE_MOVE_THRESHOLD = 10.0f;

//Accelerate the Z speed of mobs affected by gravity by this amount per second.
const float GRAVITY_ADDER = -2600.0f;

//If there's less than this much gap between the leader and group,
//then the group's Pikmin should shuffle a bit to keep up with the leader.
const float GROUP_SHUFFLE_DIST = 40.0f;

//Pikmin must be at least these many units away from one another;
//used when calculating group spots.
const float GROUP_SPOT_INTERVAL = 5.0f;

//Group spots can randomly deviate in X or Y up to this much.
const float GROUP_SPOT_MAX_DEVIATION = MOB::GROUP_SPOT_INTERVAL * 0.60f;

//When using the height effect, scale the mob by this factor.
const float HEIGHT_EFFECT_FACTOR = 0.002;

//Base horizontal speed at which mobs move due to attacks with knockback.
const float KNOCKBACK_H_POWER = 64.0f;

//Base vertical speed at which mobs move due to attacks with knockback.
const float KNOCKBACK_V_POWER = 800.0f;

//Maximum speed multiplier for animations whose speed depend on the mob's.
const float MOB_SPEED_ANIM_MAX_MULT = 3.0f;

//Minimum speed multiplier for animations whose speed depend on the mob's.
const float MOB_SPEED_ANIM_MIN_MULT = 0.3f;

//When an opponent is hit, it takes this long to be possible to hit it again.
const float OPPONENT_HIT_REGISTER_TIMEOUT = 0.5f;

//Wait these many seconds before allowing another Pikmin to be called out.
const float PIKMIN_NEST_CALL_INTERVAL = 0.01f;

//A little extra push amount when mobs intersect. Can't be throttled.
const float PUSH_EXTRA_AMOUNT = 50.0f;

//Amount to push when a mob pushes softly.
const float PUSH_SOFTLY_AMOUNT = 60.0f;

//During push throttling, multiply the push by this.
const float PUSH_THROTTLE_FACTOR = 0.1f;

//Before this much time, a mob can't push others as effectively.
const float PUSH_THROTTLE_TIMEOUT = 1.0f;

//Multiply the stretch of the shadow by this much.
const float SHADOW_STRETCH_MULT = 0.5f;

//For every unit above the ground that the mob is on,
//the shadow goes these many units to the side.
const float SHADOW_Y_MULT = 0.2f;

//Duration of the "smack" particle.
const float SMACK_PARTICLE_DUR = 0.1f;

//With a status effect that causes shaking, multiply time by this.
const float STATUS_SHAKING_TIME_MULT = 60.0f;

//Put this space between the leader and the "main" member of the group,
//when using swarming.
const float SWARM_MARGIN = 8.0f;

//When swarming, the group can scale this much vertically.
//Basically, the tube shape's girth can reach this scale.
const float SWARM_VERTICAL_SCALE = 0.5f;

}


/**
 * @brief Constructs a new mob object.
 *
 * @param pos Starting coordinates.
 * @param type Mob type this mob belongs to.
 * @param angle Starting angle.
 */
Mob::Mob(const Point &pos, MobType* type, float angle) :
    type(type),
    pos(pos),
    angle(angle),
    radius(type->radius),
    height(type->height),
    rectangularDim(type->rectangularDim),
    fsm(this),
    intendedTurnAngle(angle),
    home(pos),
    id(game.states.gameplay->nextMobId),
    health(type->maxHealth),
    maxHealth(type->maxHealth),
    itchTime(type->itchTime),
    anim(type->animDb),
    physicalSpan(type->physicalSpan) {
    
    game.states.gameplay->nextMobId++;
    
    Sector* sec = getSector(pos, nullptr, true);
    if(sec) {
        z = sec->z;
    } else {
        toDelete = true;
    }
    groundSector = sec;
    centerSector = sec;
    
    team = type->startingTeam;
    
    if(type->canBlockPaths) {
        setCanBlockPaths(true);
    }
    
    if(type->hasGroup) {
        group = new Group(this);
    }
    
    updateInteractionSpan();
}


/**
 * @brief Destroys the mob object.
 */
Mob::~Mob() {
    if(pathInfo) delete pathInfo;
    if(circlingInfo) delete circlingInfo;
    if(carryInfo) delete carryInfo;
    if(deliveryInfo) delete deliveryInfo;
    if(trackInfo) delete trackInfo;
    if(healthWheel) delete healthWheel;
    if(fraction) delete fraction;
    if(group) delete group;
    if(parent) delete parent;
}



/**
 * @brief Adds a mob to this mob's group.
 *
 * @param newMember The new member to add.
 */
void Mob::addToGroup(Mob* newMember) {
    //If it's already following, never mind.
    if(newMember->followingGroup == this) return;
    if(!group) return;
    
    newMember->followingGroup = this;
    group->members.push_back(newMember);
    
    //Find a spot.
    group->initSpots(newMember);
    newMember->groupSpotIdx = group->spots.size() - 1;
    
    if(!group->curStandbyType) {
        if(
            newMember->type->category->id != MOB_CATEGORY_LEADERS ||
            game.config.rules.canThrowLeaders
        ) {
            group->curStandbyType =
                newMember->subgroupTypePtr;
        }
    }
    
    if(group->members.size() == 1) {
        //If this is the first member, update the anchor position.
        group->anchor = pos;
        group->anchorAngle = TAU / 2.0f;
    }
}

/**
 * @brief counts the number of non identified links
 */
void Mob::push_anonymous_link(Mob* linkPtr) {
    links[i2s(link_anon_size)]= linkPtr;
    link_anon_size +=1;
}

/**
 * @brief Applies the damage caused by an attack from another mob to this one.
 *
 * @param attacker The mob that caused the attack.
 * @param attackH Hitbox used for the attack.
 * @param victimH Victim's hitbox that got hit.
 * @param damage Total damage the attack caused.
 */
void Mob::applyAttackDamage(
    Mob* attacker, Hitbox* attackH, Hitbox* victimH, float damage
) {
    //Register this hit, so the next frame doesn't hit it too.
    attacker->hitOpponents.push_back(
        std::make_pair(MOB::OPPONENT_HIT_REGISTER_TIMEOUT, this)
    );
    
    //Will the parent mob be handling the damage?
    if(parent && parent->relayDamage) {
        parent->m->applyAttackDamage(attacker, attackH, victimH, damage);
        if(!parent->handleDamage) {
            return;
        }
    }
    
    //Perform the damage and script-related events.
    if(damage > 0) {
        setHealth(true, false, -damage);
        
        HitboxInteraction evInfo(this, victimH, attackH);
        fsm.runEvent(MOB_EV_DAMAGE, (void*) &evInfo);
        
        attacker->causeSpikeDamage(this, false);
    }
    
    //Final setup.
    itchDamage += damage;
}


/**
 * @brief Applies the knockback values to a mob, caused by an attack.
 *
 * @param knockback Total knockback value.
 * @param knockbackAngle Angle to knockback towards.
 */
void Mob::applyKnockback(float knockback, float knockbackAngle) {
    if(knockback != 0) {
        stopChasing();
        speed.x = cos(knockbackAngle) * knockback * MOB::KNOCKBACK_H_POWER;
        speed.y = sin(knockbackAngle) * knockback * MOB::KNOCKBACK_H_POWER;
        speedZ = MOB::KNOCKBACK_V_POWER;
        face(getAngle(speed) + TAU / 2, nullptr, true);
        startHeightEffect();
    }
}


/**
 * @brief Applies a status effect's effects.
 *
 * @param s Status effect to use.
 * @param givenByParent If true, this status effect was given to the mob
 * by its parent mob.
 * @param fromHazard If true, this status effect was given from a hazard.
 */
void Mob::applyStatusEffect(
    StatusType* s, bool givenByParent, bool fromHazard
) {
    if(parent && parent->relayStatuses && !givenByParent) {
        parent->m->applyStatusEffect(s, false, fromHazard);
        if(!parent->handleStatuses) return;
    }
    
    if(!givenByParent && !canReceiveStatus(s)) {
        return;
    }
    
    //Let's start by sending the status to the child mobs.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* m2Ptr = game.states.gameplay->mobs.all[m];
        if(m2Ptr->parent && m2Ptr->parent->m == this) {
            m2Ptr->applyStatusEffect(s, true, fromHazard);
        }
    }
    
    //Get the vulnerabilities to this status.
    auto vulnIt = type->statusVulnerabilities.find(s);
    if(vulnIt != type->statusVulnerabilities.end()) {
        if(vulnIt->second.statusToApply) {
            //It must instead receive this status.
            applyStatusEffect(
                vulnIt->second.statusToApply, givenByParent, fromHazard
            );
            return;
        }
    }
    
    //Check if the mob is already under this status.
    for(size_t ms = 0; ms < this->statuses.size(); ms++) {
        if(this->statuses[ms].type == s) {
            //Already exists. What do we do with the time left?
            
            switch(s->reapplyRule) {
            case STATUS_REAPPLY_RULE_KEEP_TIME: {
                break;
            }
            case STATUS_REAPPLY_RULE_RESET_TIME: {
                this->statuses[ms].timeLeft = s->autoRemoveTime;
                break;
            }
            case STATUS_REAPPLY_RULE_ADD_TIME: {
                this->statuses[ms].timeLeft += s->autoRemoveTime;
                break;
            }
            }
            
            return;
        }
    }
    
    //This status is not already inflicted. Let's do so.
    Status newStatus(s);
    newStatus.fromHazard = fromHazard;
    this->statuses.push_back(newStatus);
    handleStatusEffectGain(s);
    
    if(!s->animationChange.empty()) {
        setAnimation(s->animationChange);
    }
    
    if(s->turnsInvisible) {
        hasInvisibilityStatus = true;
    }
    
    if(s->generatesParticles) {
        ParticleGenerator pg = *s->particleGen;
        pg.restartTimer();
        pg.followMob = this;
        pg.followAngle = &this->angle;
        pg.followPosOffset = s->particleOffsetPos;
        pg.followZOffset = s->particleOffsetZ;
        particleGenerators.push_back(pg);
    }
    
    if(s->freezesAnimation) {
        getSpriteData(&forcedSprite, nullptr, nullptr);
    }
}


/**
 * @brief Does the logic that arachnorb feet need to move to their next
 * spot, based on variables set by the parent mob (the arachnorb head).
 */
void Mob::arachnorbFootMoveLogic() {
    if(!parent) {
        return;
    }
    if(parent->limbParentBodyPart == INVALID) {
        return;
    }
    
    float feetNormalDist = s2f(parent->m->vars["feet_normal_distance"]);
    if(feetNormalDist == 0) {
        feetNormalDist = 175;
    }
    
    float defaultAngle =
        getAngle(
            Point(),
            parent->m->getHitbox(
                parent->limbParentBodyPart
            )->pos
        );
        
    Point finalPos = s2p(parent->m->vars["_destination_pos"]);
    float finalAngle = s2f(parent->m->vars["_destination_angle"]);
    
    Point offset = Point(feetNormalDist, 0);
    offset = rotatePoint(offset, defaultAngle);
    offset = rotatePoint(offset, finalAngle);
    
    finalPos += offset;
    
    chase(finalPos, z);
}


/**
 * @brief Does the logic that arachnorb heads need to turn, based on their
 * feet's positions.
 */
void Mob::arachnorbHeadTurnLogic() {
    if(links.empty()) return;
    
    float angleDeviationAvg = 0;
    size_t nFeet = 0;
    
    for (const auto& [identifier, link] : links) {
        if (!isNumber(identifier)){
            continue;
        }
        if(!link) {
            continue;
        }
        
        if(!link->parent) {
            continue;
        }
        if(link->parent->m != this) {
            continue;
        }
        if(link->parent->limbParentBodyPart == INVALID) {
            continue;
        }
        
        nFeet++;
        
        float defaultAngle =
            getAngle(
                Point(),
                getHitbox(
                    link->parent->limbParentBodyPart
                )->pos
            );
        float curAngle =
            getAngle(pos, link->pos) - angle;
        float angleDeviation =
            getAngleCwDiff(defaultAngle, curAngle);
        if(angleDeviation > M_PI) {
            angleDeviation -= TAU;
        }
        angleDeviationAvg += angleDeviation;
    }
    
    face(angle + (angleDeviationAvg / nFeet), nullptr);
}


/**
 * @brief Does the logic that arachnorb heads need to plan out how to move
 * their feet for the next set of steps.
 *
 * @param goal What its goal is.
 */
void Mob::arachnorbPlanLogic(
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE goal
) {
    float maxStepDistance = s2f(vars["max_step_distance"]);
    float maxTurnAngle = degToRad(s2f(vars["max_turn_angle"]));
    float minTurnAngle = degToRad(s2f(vars["min_turn_angle"]));
    if(maxStepDistance == 0) {
        maxStepDistance = 100;
    }
    if(maxTurnAngle == 0) {
        maxTurnAngle = TAU * 0.2;
    }
    
    float amountToMove = 0;
    float amountToTurn = 0;
    
    switch(goal) {
    case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME: {
        amountToTurn = getAngleCwDiff(angle, getAngle(pos, home));
        if(amountToTurn > TAU / 2)  amountToTurn -= TAU;
        if(amountToTurn < -TAU / 2) amountToTurn += TAU;
        
        if(fabs(amountToTurn) < TAU * 0.05) {
            //We can also start moving towards home now.
            amountToMove = Distance(pos, home).toFloat();
        }
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD: {
        amountToMove = maxStepDistance;
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN: {
        amountToTurn = game.rng.f(minTurnAngle, TAU * 0.25);
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN: {
        amountToTurn = game.rng.f(-TAU * 0.25, -minTurnAngle);
        break;
        
    }
    }
    
    amountToMove = std::min(amountToMove, maxStepDistance);
    amountToTurn =
        sign(amountToTurn) *
        std::min((double) fabs(amountToTurn), (double) maxTurnAngle);
        
    Point destinationPos = pos;
    float destinationAngle = angle + amountToTurn;
    normalizeAngle(destinationAngle);
    
    Point offset = Point(amountToMove, 0);
    offset = rotatePoint(offset, destinationAngle);
    
    destinationPos += offset;
    
    vars["_destination_pos"] = p2s(destinationPos);
    vars["_destination_angle"] = f2s(destinationAngle);
}


/**
 * @brief Sets up data for a mob to become carriable.
 *
 * @param destination Where to carry it.
 */
void Mob::becomeCarriable(const CARRY_DESTINATION destination) {
    carryInfo = new CarryInfo(this, destination);
}


/**
 * @brief Sets up data for a mob to stop being carriable.
 */
void Mob::becomeUncarriable() {
    if(!carryInfo) return;
    
    for(size_t p = 0; p < carryInfo->spotInfo.size(); p++) {
        if(carryInfo->spotInfo[p].state != CARRY_SPOT_STATE_FREE) {
            carryInfo->spotInfo[p].pikPtr->fsm.runEvent(
                MOB_EV_FOCUSED_MOB_UNAVAILABLE
            );
        }
    }
    
    stopChasing();
    
    delete carryInfo;
    carryInfo = nullptr;
}


/**
 * @brief Calculates the final carrying target, and the final carrying position,
 * given the sort of carry destination, what Pikmin are holding on, and what
 * Pikmin got added or removed.
 *
 * @param added The Pikmin that got added, if any.
 * @param removed The Pikmin that got removed, if any.
 * @param targetType Return the target Pikmin type (if any) here.
 * @param targetMob Return the target mob (if any) here.
 * @param targetPoint Return the target point here.
 * @return Whether it succeeded.
 * Returns false if there are no available targets or if
 * something went wrong.
 */
bool Mob::calculateCarryingDestination(
    Mob* added, Mob* removed,
    PikminType** targetType, Mob** targetMob, Point* targetPoint
) const {
    *targetMob = nullptr;
    *targetPoint = pos;
    if(!carryInfo) return false;
    
    switch(carryInfo->destination) {
    case CARRY_DESTINATION_SHIP: {

        //Go to the nearest ship.
        Ship* target = calculateCarryingShip();
        
        if(target) {
            *targetMob = target;
            *targetPoint = target->controlPointFinalPos;
            return true;
            
        } else {
            return false;
        }
        
        break;
        
    } case CARRY_DESTINATION_ONION: {

        Onion* target = calculateCarryingOnion(added, removed, targetType);
        
        if(!target) {
            return false;
        }
        *targetMob = target;
        *targetPoint = (*targetMob)->pos;
        return true;
        
        break;
        
    } case CARRY_DESTINATION_SHIP_NO_ONION: {

        Onion* oniTarget = calculateCarryingOnion(added, removed, targetType);
        
        if(oniTarget) {
            *targetMob = oniTarget;
            *targetPoint = (*targetMob)->pos;
            return true;
        }
        
        //No onion, find a ship instead.
        Ship* shiTarget = calculateCarryingShip();
        if(shiTarget) {
            *targetMob = shiTarget;
            *targetPoint = shiTarget->controlPointFinalPos;
            return true;
        }
        return false;
        
    } case CARRY_DESTINATION_LINKED_MOB: {

        //If it's towards a linked mob, just go to the closest one.
        Mob* closestLink = nullptr;
        Distance closestLinkDist;
        for (const auto& [identifier, link] : links) {
            if (!isNumber(identifier)){
                continue;
            }
            Distance d(pos, link->pos);
            
            if(!closestLink || d < closestLinkDist) {
                closestLink = link;
                closestLinkDist = d;
            }
        }
        
        if(closestLink) {
            *targetMob = closestLink;
            *targetPoint = closestLink->pos;
            return true;
        } else {
            return false;
        }
        
        break;
        
    } case CARRY_DESTINATION_LINKED_MOB_MATCHING_TYPE: {

        //Towards one of the linked mobs that matches the decided Pikmin type.
        if(links.empty()) {
            return false;
        }
        
        unordered_set<PikminType*> availableTypes;
        vector<std::pair<string, PikminType*> > mobsPerType;
        
        for (const auto& [identifier, link] : links) {
            if (!isNumber(identifier)){
                continue;
            }
            if(!link) continue;
            string typeName =
                link->vars["carry_destination_type"];
            MobType* pikType =
                game.mobCategories.get(MOB_CATEGORY_PIKMIN)->
                getType(typeName);
            if(!pikType) continue;
            
            availableTypes.insert(
                (PikminType*) pikType
            );
            mobsPerType.push_back(
                std::make_pair(identifier,(PikminType*) pikType)
            );
        }
        
        if(availableTypes.empty()) {
            //No available types?! Well...make the Pikmin stuck.
            return false;
        }
        
        PikminType* decidedType =
            decideCarryPikminType(availableTypes, added, removed);
            
        //Figure out which linked mob matches the decided type.
        string closestTargetKey = "";
        Distance closestTargetDist;
        for(size_t m = 0; m < mobsPerType.size(); m++) {
            if(mobsPerType[m].second != decidedType) continue;
            
            Distance d(pos, links.at(mobsPerType[m].first)->pos);
            if(closestTargetKey.empty() || d < closestTargetDist) {
                closestTargetDist = d;
                closestTargetKey = mobsPerType[m].first;
            }
        }
        
        //Finally, set the destination data.
        *targetType = decidedType;
        *targetMob = links.at(closestTargetKey);
        *targetPoint = (*targetMob)->pos;
        
        return true;
        
        break;
        
    }
    }
    
    return false;
}


/**
 * @brief Calculates to which Onion Pikmin should carry something.
 *
 * @param added Newly added Pikmin, if any.
 * @param removed Newly removed Pikmin, if any.
 * @param targettype If not nullptr, the target Pikmin type is returned here.
 * @return The Onion.
 */
Onion* Mob::calculateCarryingOnion(
    Mob* added, Mob* removed, PikminType** targetType
) const {
    //If it's meant for an Onion, we need to decide which Onion, based on
    //the Pikmin. First, check which Onion Pikmin types are even available.
    unordered_set<PikminType*> availableTypes;
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        Onion* oPtr = game.states.gameplay->mobs.onions[o];
        if(oPtr->activated) {
            for(
                size_t t = 0;
                t < oPtr->oniType->nest->pikTypes.size();
                t++
            ) {
                availableTypes.insert(
                    oPtr->oniType->nest->pikTypes[t]
                );
            }
        }
    }
    
    if(availableTypes.empty()) {
        //No available types?! Well...make the Pikmin stuck.
        return nullptr;
    }
    
    PikminType* decidedType =
        decideCarryPikminType(availableTypes, added, removed);
        
    //Figure out where that type's Onion is.
    size_t closestOnionIdx = INVALID;
    Distance closestOnionDist;
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        Onion* oPtr = game.states.gameplay->mobs.onions[o];
        if(!oPtr->activated) continue;
        bool hasType = false;
        for(
            size_t t = 0;
            t < oPtr->oniType->nest->pikTypes.size();
            t++
        ) {
            if(oPtr->oniType->nest->pikTypes[t] == decidedType) {
                hasType = true;
                break;
            }
        }
        if(!hasType) continue;
        
        Distance d(pos, oPtr->pos);
        if(closestOnionIdx == INVALID || d < closestOnionDist) {
            closestOnionDist = d;
            closestOnionIdx = o;
        }
    }
    
    *targetType = decidedType;
    return game.states.gameplay->mobs.onions[closestOnionIdx];
}


/**
 * @brief Calculates to which ship Pikmin should carry something.
 *
 * @return The ship.
 */
Ship* Mob::calculateCarryingShip() const {
    //Go to the nearest ship.
    Ship* closestShip = nullptr;
    Distance closestShipDist;
    
    for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); s++) {
        Ship* sPtr = game.states.gameplay->mobs.ships[s];
        Distance d(pos, sPtr->controlPointFinalPos);
        
        if(!closestShip || d < closestShipDist) {
            closestShip = sPtr;
            closestShipDist = d;
        }
    }
    return closestShip;
}


/**
 * @brief Calculates how much damage an attack will cause.
 *
 * @param victim The mob that'll take the damage.
 * @param attackH Hitbox used for the attack.
 * @param victimH Victim's hitbox that got hit.
 * @param damage Return the calculated damage here.
 * @return Whether the attack will hit.
 * Returns true even if it will end up causing zero damage.
 * Returns false if it cannot hit (e.g. the victim hitbox is not valid).
 */
bool Mob::calculateDamage(
    Mob* victim, Hitbox* attackH, const Hitbox* victimH, float* damage
) const {
    float attackerOffense = 0;
    float defenseMultiplier = 1;
    
    //First, check if this hitbox cannot be damaged.
    if(victimH->type != HITBOX_TYPE_NORMAL) {
        //This hitbox can't be damaged! Abort!
        return false;
    }
    
    //Calculate the damage.
    if(attackH) {
        attackerOffense = attackH->value;
        
        if(attackH->hazard) {
            MobType::Vulnerability vuln =
                victim->getHazardVulnerability(attackH->hazard);
                
            if(vuln.effectMult == 0.0f) {
                //The victim is immune to this hazard!
                *damage = 0;
                return true;
            } else {
                defenseMultiplier = 1.0f / vuln.effectMult;
            }
            
        } else {
        
            if(victim->type->defaultVulnerability == 0.0f) {
                //The victim is invulnerable to everything about this attack!
                *damage = 0;
                return true;
            } else {
                defenseMultiplier = 1.0f / victim->type->defaultVulnerability;
            }
        }
        
    } else {
        attackerOffense = 1;
    }
    
    if(victimH->value == 0.0f) {
        //Hah, this hitbox is invulnerable!
        *damage = 0;
        return true;
    }
    
    defenseMultiplier *= victimH->value;
    
    for(size_t s = 0; s < statuses.size(); s++) {
        attackerOffense *= statuses[s].type->attackMultiplier;
    }
    for(size_t s = 0; s < victim->statuses.size(); s++) {
        float vulnMult = victim->statuses[s].type->defenseMultiplier - 1.0f;
        auto vulnIt = type->statusVulnerabilities.find(statuses[s].type);
        if(vulnIt != type->statusVulnerabilities.end()) {
            vulnMult *= vulnIt->second.effectMult;
        }
        defenseMultiplier *= (vulnMult + 1.0f);
    }
    
    if(this->type->category->id == MOB_CATEGORY_PIKMIN) {
        //It's easier to calculate the maturity attack boost here.
        Pikmin* pikPtr = (Pikmin*) this;
        attackerOffense *=
            1 + (game.config.pikmin.maturityPowerMult * pikPtr->maturity);
    }
    
    *damage = attackerOffense * (1.0f / defenseMultiplier);
    return true;
}


/**
 * @brief Calculates how much knockback an attack will cause.
 *
 * @param victim The mob that'll take the damage.
 * @param attackh The hitbox of the attacker mob, if any.
 * @param victimH The hitbox of the victim mob, if any.
 * @param kbStrength The variable to return the knockback amount to.
 * @param kbAngle The variable to return the angle of the knockback to.
 */
void Mob::calculateKnockback(
    const Mob* victim, const Hitbox* attackH,
    Hitbox* victimH, float* kbStrength, float* kbAngle
) const {
    if(attackH) {
        *kbStrength = attackH->knockback;
        if(attackH->knockbackOutward) {
            *kbAngle =
                getAngle(attackH->getCurPos(pos, angle), victim->pos);
        } else {
            *kbAngle =
                angle + attackH->knockbackAngle;
        }
    } else {
        *kbStrength = 0;
        *kbAngle = 0;
    }
}


/**
 * @brief Does this mob want to attack mob v? Teams and other factors are
 * used to decide this.
 *
 * @param v The victim to check.
 * @return Whether it can hunt.
 */
bool Mob::canHunt(Mob* v) const {
    //Teammates cannot hunt each other down.
    if(team == v->team && team != MOB_TEAM_NONE) return false;
    
    //Mobs that do not participate in combat whatsoever cannot be hunted down.
    if(v->type->targetType == MOB_TARGET_FLAG_NONE) return false;
    
    //Invisible mobs cannot be seen, so they can't be hunted down.
    if(v->hasInvisibilityStatus) return false;
    
    //Mobs that don't want to be hunted right now cannot be hunted down.
    if(hasFlag(v->flags, MOB_FLAG_NON_HUNTABLE)) return false;
    
    //Return whether or not this mob wants to hunt v.
    return (type->huntableTargets & v->type->targetType);
}


/**
 * @brief Can this mob damage v? Teams and other factors are used to
 * decide this.
 *
 * @param v The victim to check.
 * @return Whether it can hurt.
 */
bool Mob::canHurt(Mob* v) const {
    //Teammates cannot hurt each other.
    if(team == v->team && team != MOB_TEAM_NONE) return false;
    
    //Mobs that do not participate in combat whatsoever cannot be hurt.
    if(v->type->targetType == MOB_TARGET_FLAG_NONE) return false;
    
    //Mobs that are invulnerable cannot be hurt.
    if(v->invulnPeriod.timeLeft > 0) return false;
    
    //Mobs that don't want to be hurt right now cannot be hurt.
    if(hasFlag(v->flags, MOB_FLAG_NON_HURTABLE)) return false;
    
    //Check if this mob has already hit v recently.
    for(size_t h = 0; h < hitOpponents.size(); h++) {
        if(hitOpponents[h].second == v) {
            //v was hit by this mob recently, so don't let it attack again.
            //This stops the same attack from hitting every single frame.
            return false;
        }
    }
    
    //Return whether or not this mob can damage v.
    return (type->hurtableTargets & v->type->targetType);
}


/**
 * @brief Returns whether or not a mob can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive the status.
 */
bool Mob::canReceiveStatus(StatusType* s) const {
    return hasFlag(s->affects, STATUS_AFFECTS_FLAG_OTHERS);
}


/**
 * @brief Makes the mob cause spike damage to another mob.
 *
 * @param victim The mob that will be damaged.
 * @param isIngestion If true, the attacker just got eaten.
 * If false, it merely got hurt.
 */
void Mob::causeSpikeDamage(Mob* victim, bool isIngestion) {
    if(!type->spikeDamage) return;
    
    if(type->spikeDamage->ingestionOnly != isIngestion) return;
    
    float damage;
    if(type->spikeDamage->isDamageRatio) {
        damage = victim->maxHealth * type->spikeDamage->damage;
    } else {
        damage = type->spikeDamage->damage;
    }
    
    auto v =
        victim->type->spikeDamageVulnerabilities.find(type->spikeDamage);
    if(v != victim->type->spikeDamageVulnerabilities.end()) {
        damage *= v->second.effectMult;
    }
    
    if(type->spikeDamage->statusToApply) {
        victim->applyStatusEffect(
            type->spikeDamage->statusToApply, false, false
        );
    }
    
    victim->setHealth(true, false, -damage);
    
    if(type->spikeDamage->particleGen) {
        ParticleGenerator pg = *(type->spikeDamage->particleGen);
        pg.restartTimer();
        pg.followMob = victim;
        pg.followAngle = &victim->angle;
        pg.followPosOffset = type->spikeDamage->particleOffsetPos;
        pg.followZOffset = type->spikeDamage->particleOffsetZ;
        victim->particleGenerators.push_back(pg);
    }
    
    if(
        v != victim->type->spikeDamageVulnerabilities.end() &&
        v->second.statusToApply
    ) {
        victim->applyStatusEffect(
            v->second.statusToApply, false, false
        );
    }
}


/**
 * @brief Sets a target for the mob to follow.
 *
 * @param origcoords Pointer to changing coordinates. If nullptr, it is
 * the world origin. Use this to make the mob follow another mob
 * wherever they go, for instance.
 * @param origZ Same as origCoords, but for the Z coordinate.
 * @param offset Offset from origCoords.
 * @param offsetZ Z offset from origZ.
 * @param flags Flags that control how to chase. Use CHASE_FLAG.
 * @param targetDistance Distance at which the mob considers the
 * chase finished.
 * @param speed Speed at which to go to the target.
 * LARGE_FLOAT makes it use the mob's standard speed.
 * @param acceleration Speed acceleration.
 * LARGE_FLOAT makes it use the mob's standard acceleration.
 */
void Mob::chase(
    Point* origCoords, float* origZ,
    const Point &offset, float offsetZ,
    Bitmask8 flags,
    float targetDistance, float speed, float acceleration
) {
    chaseInfo.origCoords = origCoords;
    chaseInfo.origZ = origZ;
    chaseInfo.offset = offset;
    chaseInfo.offsetZ = offsetZ;
    
    chaseInfo.flags = flags;
    if(type->canFreeMove) {
        enableFlag(chaseInfo.flags, CHASE_FLAG_ANY_ANGLE);
    }
    
    chaseInfo.targetDist = targetDistance;
    chaseInfo.maxSpeed =
        (speed == LARGE_FLOAT ? getBaseSpeed() : speed);
    chaseInfo.acceleration =
        (acceleration == LARGE_FLOAT ? type->acceleration : acceleration);
        
    chaseInfo.state = CHASE_STATE_CHASING;
}


/**
 * @brief Sets a target for the mob to follow.
 *
 * @param coords Coordinates of the target.
 * @param coordsZ Z coordinates of the target.
 * @param flags Flags that control how to chase. Use CHASE_FLAG.
 * @param targetDistance Distance at which the mob considers the
 * chase finished.
 * @param speed Speed at which to go to the target.
 * LARGE_FLOAT makes it use the mob's standard speed.
 * @param acceleration Speed acceleration.
 * LARGE_FLOAT makes it use the mob's standard acceleration.
 */
void Mob::chase(
    const Point &coords, float coordsZ,
    unsigned char flags,
    float targetDistance, float speed, float acceleration
) {
    chase(
        nullptr, nullptr, coords, coordsZ,
        flags, targetDistance, speed, acceleration
    );
}


/**
 * @brief Makes a mob chomp another mob. Mostly applicable for enemies chomping
 * on Pikmin.
 *
 * @param m The mob to be chomped.
 * @param hitboxInfo Information about the hitbox that caused the chomp.
 */
void Mob::chomp(Mob* m, const Hitbox* hitboxInfo) {
    if(m->type->category->id == MOB_CATEGORY_TOOLS) {
        Tool* tooPtr = (Tool*) m;
        if(!hasFlag(tooPtr->holdabilityFlags, HOLDABILITY_FLAG_ENEMIES)) {
            //Enemies can't chomp this tool right now.
            return;
        }
    }
    
    for(size_t c = 0; c < chompingMobs.size(); c++) {
        if(chompingMobs[c] == m) {
            //It's already chomping the mob.
            return;
        }
    }
    
    float hOffsetDist;
    float hOffsetAngle;
    float vOffsetDist;
    getHitboxHoldPoint(
        m, hitboxInfo, &hOffsetDist, &hOffsetAngle, &vOffsetDist
    );
    hold(
        m, hitboxInfo->bodyPartIdx,
        hOffsetDist, hOffsetAngle, vOffsetDist,
        true, HOLD_ROTATION_METHOD_NEVER
    );
    
    m->focusOnMob(this);
    chompingMobs.push_back(m);
}


/**
 * @brief Makes the mob start circling around a point or another mob.
 *
 * @param m The mob to circle around. If nullptr, circle around a point instead.
 * @param p The point to circle around, if any.
 * @param radius Circle these many units around the target.
 * @param clockwise Circle clockwise or counterclockwise?
 * @param speed Speed at which to move.
 * @param canFreeMove Can the mob move freely, or only forward?
 */
void Mob::circleAround(
    Mob* m, const Point &p, float radius, bool clockwise,
    float speed, bool canFreeMove
) {
    if(!circlingInfo) {
        circlingInfo = new CirclingInfo(this);
    }
    circlingInfo->circlingMob = m;
    circlingInfo->circlingPoint = p;
    circlingInfo->radius = radius;
    circlingInfo->clockwise = clockwise;
    circlingInfo->speed = speed;
    circlingInfo->canFreeMove = canFreeMove;
    circlingInfo->curAngle =
        getAngle((m ? m->pos : p), pos);
}


/**
 * @brief Returns what Pikmin type is decided when carrying something.
 *
 * @param availableTypes List of Pikmin types that are currently
 * available in the area.
 * @param added If a Pikmin got added to the carriers, specify it here.
 * @param removed If a Pikmin got removed from the carriers, specify it here.
 * @return The Pikmin type.
 */
PikminType* Mob::decideCarryPikminType(
    const unordered_set<PikminType*> &availableTypes,
    Mob* added, Mob* removed
) const {
    //How many of each Pikmin type are carrying.
    map<PikminType*, unsigned> typeQuantity;
    //The Pikmin type with the most carriers.
    vector<PikminType*> majorityTypes;
    
    //Count how many of each type there are carrying.
    for(size_t p = 0; p < type->maxCarriers; p++) {
        Pikmin* pikPtr = nullptr;
        
        if(carryInfo->spotInfo[p].state != CARRY_SPOT_STATE_USED) continue;
        
        pikPtr = (Pikmin*) carryInfo->spotInfo[p].pikPtr;
        
        //If it doesn't have an Onion to carry to, it won't even count.
        if(!isInContainer(availableTypes, pikPtr->pikType)) {
            continue;
        }
        
        typeQuantity[pikPtr->pikType]++;
    }
    
    //Then figure out what are the majority types.
    unsigned most = 0;
    for(auto &t : typeQuantity) {
        if(t.second > most) {
            most = t.second;
            majorityTypes.clear();
        }
        if(t.second == most) majorityTypes.push_back(t.first);
    }
    
    //If we ended up with no candidates, pick a type at random,
    //out of all possible types.
    bool forceRandom = false;
    if(majorityTypes.empty()) {
        forceRandom = true;
        for(auto t = availableTypes.begin(); t != availableTypes.end(); ++t) {
            majorityTypes.push_back(*t);
        }
    }
    
    PikminType* decidedType = nullptr;
    
    //Now let's pick an Pikmin type from the candidates.
    if(majorityTypes.size() == 1) {
        //If there's only one possible type to pick, pick it.
        decidedType = *majorityTypes.begin();
        
    } else {
        //If the current type is a majority, it takes priority.
        //Otherwise, pick a majority at random.
        if(
            carryInfo->intendedPikType &&
            !forceRandom &&
            isInContainer(majorityTypes, carryInfo->intendedPikType)
        ) {
            decidedType = carryInfo->intendedPikType;
        } else {
            decidedType =
                majorityTypes[
                    game.rng.i(0, (int) majorityTypes.size() - 1)
                ];
        }
    }
    
    return decidedType;
}


/**
 * @brief Deletes all status effects asking to be deleted.
 */
void Mob::deleteOldStatusEffects() {
    vector<std::pair<StatusType*, bool> > newStatusesToApply;
    bool removedForcedSprite = false;
    
    for(size_t s = 0; s < statuses.size(); ) {
        Status &sPtr = statuses[s];
        if(sPtr.toDelete) {
            handleStatusEffectLoss(sPtr.type);
            
            if(sPtr.type->generatesParticles) {
                removeParticleGenerator(sPtr.type->particleGen->id);
            }
            
            if(sPtr.type->freezesAnimation) {
                removedForcedSprite = true;
            }
            
            if(sPtr.type->replacementOnTimeout && sPtr.timeLeft <= 0.0f) {
                newStatusesToApply.push_back(
                    std::make_pair(
                        sPtr.type->replacementOnTimeout,
                        sPtr.fromHazard
                    )
                );
                if(sPtr.type->replacementOnTimeout->freezesAnimation) {
                    //Actually, never mind, let's keep the current forced
                    //sprite so that the next status effect can use it too.
                    removedForcedSprite = false;
                }
            }
            
            statuses.erase(statuses.begin() + s);
        } else {
            s++;
        }
    }
    
    //Apply new status effects.
    for(size_t s = 0; s < newStatusesToApply.size(); s++) {
        applyStatusEffect(
            newStatusesToApply[s].first,
            false, newStatusesToApply[s].second
        );
    }
    
    if(removedForcedSprite) {
        forcedSprite = nullptr;
    }
    
    //Update some flags.
    hasInvisibilityStatus = false;
    for(size_t s = 0; s < statuses.size(); s++) {
        if(statuses[s].type->turnsInvisible) {
            hasInvisibilityStatus = true;
            break;
        }
    }
}


/**
 * @brief Starts the particle effect and sound for an attack,
 * which could either be a meaty whack, or a harmless ding.
 *
 * @param attacker Mob that caused the attack.
 * @param attackH Hitbox that caused the attack.
 * @param victimH Hitbox that suffered the attack.
 * @param damage Total damage caused.
 * @param knockback Total knockback strength.
 */
void Mob::doAttackEffects(
    const Mob* attacker, const Hitbox* attackH, const Hitbox* victimH,
    float damage, float knockback
) {
    if(attackH->value == 0.0f) {
        //Attack hitboxes that cause 0 damage don't need to smack or ding.
        //This way, objects can "attack" other objects at 0 damage for the
        //purposes of triggering events (like hazard touching), without
        //having to constantly display the dings.
        //The ding effect should only be used when an attack that really WANTED
        //to cause damage failed to do so, thus highlighting the uselessness.
        return;
    }
    
    //Calculate the particle's final position.
    Point attackHPos = attackH->getCurPos(attacker->pos, attacker->angle);
    Point victimHPos = victimH->getCurPos(pos, angle);
    
    float edgesD;
    float aToVAngle;
    coordinatesToAngle(
        victimHPos - attackHPos,
        &aToVAngle, &edgesD
    );
    
    edgesD -= attackH->radius;
    edgesD -= victimH->radius;
    float offset = attackH->radius + edgesD / 2.0;
    
    Point particlePos =
        attackHPos +
        Point(cos(aToVAngle) * offset, sin(aToVAngle) * offset);
    float particleZ =
        std::max(
            z + getDrawingHeight() + 1.0f,
            attacker->z + attacker->getDrawingHeight() + 1.0f
        );
        
    bool useless = (damage <= 0 && knockback == 0.0f);
    
    //Create the particle.
    string particleInternalName =
        useless ?
        game.sysContentNames.parDing :
        game.sysContentNames.parSmack;
    ParticleGenerator pg =
        standardParticleGenSetup(
            particleInternalName, nullptr
        );
    pg.baseParticle.pos = particlePos;
    pg.baseParticle.z = particleZ;
    pg.emit(game.states.gameplay->particles);
    
    if(!useless) {
        //Play the sound.
        
        SoundSourceConfig attackSoundConfig;
        attackSoundConfig.gain = 0.6f;
        game.audio.createPosSoundSource(
            game.sysContent.sndAttack,
            pos, false, attackSoundConfig
        );
        
        //Damage squash and stretch animation.
        if(damageSquashTime == 0.0f) {
            damageSquashTime = MOB::DAMAGE_SQUASH_DURATION;
        }
    }
}


/**
 * @brief Draws the limb that connects this mob to its parent.
 */
void Mob::drawLimb() {
    if(!parent) return;
    if(!parent->limbAnim.animDb) return;
    Sprite* limbCurSPtr;
    Sprite* limbNextSPtr;
    float limbInterpolationFactor;
    parent->limbAnim.getSpriteData(
        &limbCurSPtr, &limbNextSPtr, &limbInterpolationFactor
    );
    if(!limbCurSPtr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        limbCurSPtr, limbNextSPtr, limbInterpolationFactor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    Point parentEnd;
    if(parent->limbParentBodyPart == INVALID) {
        parentEnd = parent->m->pos;
    } else {
        parentEnd =
            parent->m->getHitbox(
                parent->limbParentBodyPart
            )->getCurPos(
                parent->m->pos, parent->m->angleCos, parent->m->angleSin
            );
    }
    
    Point childEnd;
    if(parent->limbChildBodyPart == INVALID) {
        childEnd = pos;
    } else {
        childEnd =
            getHitbox(
                parent->limbChildBodyPart
            )->getCurPos(pos, angleCos, angleSin);
    }
    
    float p2cAngle = getAngle(parentEnd, childEnd);
    
    if(parent->limbParentOffset) {
        parentEnd +=
            rotatePoint(
                Point(parent->limbParentOffset, 0), p2cAngle
            );
    }
    if(parent->limbChildOffset) {
        childEnd -=
            rotatePoint(
                Point(parent->limbChildOffset, 0), p2cAngle
            );
    }
    
    float length = Distance(parentEnd, childEnd).toFloat();
    Point limbBmpSize = getBitmapDimensions(limbCurSPtr->bitmap);
    
    eff.translation = (parentEnd + childEnd) / 2.0;
    eff.scale.x = length / limbBmpSize.x;
    eff.scale.y = parent->limbThickness / limbBmpSize.y;
    eff.rotation = p2cAngle;
    
    drawBitmapWithEffects(limbCurSPtr->bitmap, eff);
}


/**
 * @brief Draws just the mob.
 * This is a generic function, and can be overwritten by child classes.
 */
void Mob::drawMob() {
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
        SPRITE_BMP_EFFECT_CARRY
    );
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);
}


/**
 * @brief Makes a mob intend to face a new angle, or face there right away.
 *
 * @param newAngle Face this angle.
 * @param newPos If this is not nullptr, turn towards this point every frame,
 * instead.
 * @param instantly If true, the mob faces that angle instantly instead
 * of rotating towards that direction over time.
 */
void Mob::face(float newAngle, Point* newPos, bool instantly) {
    if(carryInfo) return; //If it's being carried, it shouldn't rotate.
    intendedTurnAngle = newAngle;
    intendedTurnPos = newPos;
    if(instantly) {
        angle = newAngle;
        angleCos = cos(angle);
        angleSin = sin(angle);
    }
}


/**
 * @brief Sets up stuff for the end of the mob's dying process.
 */
void Mob::finishDying() {
    releaseChompedPikmin();
    
    finishDyingClassSpecifics();
}


/**
 * @brief Sets up stuff for the end of the mob's dying process.
 * This function is meant to be overridden by child classes.
 */
void Mob::finishDyingClassSpecifics() {
}


/**
 * @brief Makes the mob focus on m2.
 *
 * @param m2 The mob to focus on.
 */
void Mob::focusOnMob(Mob* m2) {
    unfocusFromMob();
    focusedMob = m2;
}



/**
 * @brief Makes the mob start following a path. This populates the pathInfo
 * class member, and calculates a path to take.
 * Returns whether or not there is a path available.
 *
 * @param settings Settings about how the path should be followed.
 * @param speed Speed at which to travel.
 * @param acceleration Speed acceleration.
 * @return Whether there is a path available.
 */
bool Mob::followPath(
    const PathFollowSettings &settings,
    float speed, float acceleration
) {
    bool wasBlocked = false;
    PathStop* oldNextStop = nullptr;
    
    //Some setup before we begin.
    if(hasFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE) && pathInfo) {
        wasBlocked = pathInfo->blockReason != PATH_BLOCK_REASON_NONE;
        if(pathInfo->curPathStopIdx < pathInfo->path.size()) {
            oldNextStop = pathInfo->path[pathInfo->curPathStopIdx];
        }
    }
    
    if(pathInfo) {
        delete pathInfo;
    }
    
    PathFollowSettings finalSettings = settings;
    
    if(carryInfo) {
        //Check if this carriable is considered light load.
        if(type->weight == 1) {
            enableFlag(finalSettings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD);
        }
        //The object will only be airborne if all its carriers can fly.
        if(carryInfo->canFly()) {
            enableFlag(finalSettings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        }
    } else {
        if(
            type->category->id == MOB_CATEGORY_PIKMIN ||
            type->category->id == MOB_CATEGORY_LEADERS
        ) {
            //Simple mobs are empty-handed, so that's considered light load.
            enableFlag(finalSettings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD);
        }
        //Check if the object can fly directly.
        if(hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            enableFlag(finalSettings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        }
    }
    
    if(carryInfo) {
        //The object is only as invulnerable as the Pikmin carrying it.
        finalSettings.invulnerabilities =
            carryInfo->getCarrierInvulnerabilities();
    } if(group) {
        //The object is only as invulnerable as the members of its group.
        finalSettings.invulnerabilities =
            group->getGroupInvulnerabilities(this);
    } else {
        //Use the object's standard invulnerabilities.
        for(auto &v : type->hazardVulnerabilities) {
            if(v.second.effectMult == 0.0f) {
                finalSettings.invulnerabilities.push_back(v.first);
            }
        }
    }
    
    //Establish the mob's path-following information.
    //This also generates the path to take.
    pathInfo = new Path(this, finalSettings);
    
    if(
        hasFlag(pathInfo->settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE) &&
        oldNextStop &&
        !wasBlocked &&
        pathInfo->path.size() >= 2
    ) {
        for(size_t s = 1; s < pathInfo->path.size(); s++) {
            if(pathInfo->path[s] == oldNextStop) {
                //If before, the mob was already heading towards this stop,
                //then just continue the new journey from there.
                pathInfo->curPathStopIdx = s;
                break;
            }
        }
    }
    
    if(pathInfo->path.size() >= 2 && pathInfo->curPathStopIdx > 0) {
        if(pathInfo->checkBlockage(&pathInfo->blockReason)) {
            fsm.runEvent(MOB_EV_PATH_BLOCKED);
        }
    }
    
    bool direct =
        pathInfo->result == PATH_RESULT_DIRECT ||
        pathInfo->result == PATH_RESULT_DIRECT_NO_STOPS;
    //Now, let's figure out how the mob should start its journey.
    if(direct) {
        //The path info is telling us to just go to the destination directly.
        moveToPathEnd(speed, acceleration);
        
    } else if(!pathInfo->path.empty()) {
        //Head to the first stop.
        PathStop* nextStop =
            pathInfo->path[pathInfo->curPathStopIdx];
        float nextStopZ = z;
        if(
            hasFlag(pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) &&
            nextStop->sectorPtr
        ) {
            nextStopZ =
                nextStop->sectorPtr->z +
                PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT;
        }
        
        chase(
            nextStop->pos, nextStopZ,
            CHASE_FLAG_ANY_ANGLE,
            PATHS::DEF_CHASE_TARGET_DISTANCE,
            speed, acceleration
        );
        
    } else {
        //No valid path.
        return false;
        
    }
    
    return true;
}


/**
 * @brief Returns the base speed for this mob.
 * This is overwritten by some child classes.
 *
 * @return The base speed.
 */
float Mob::getBaseSpeed() const {
    return this->type->moveSpeed;
}


/**
 * @brief Returns the actual location of the movement target.
 *
 * @param outZ If not nullptr, the Z coordinate is returned here.
 * @return The (X and Y) coordinates of the target.
 */
Point Mob::getChaseTarget(float* outZ) const {
    Point p = chaseInfo.offset;
    if(chaseInfo.origCoords) p += (*chaseInfo.origCoords);
    if(outZ) {
        *outZ = chaseInfo.offsetZ;
        if(chaseInfo.origZ) (*outZ) += (*chaseInfo.origZ);
    }
    return p;
}


/**
 * @brief Returns the closest hitbox to a point,
 * belonging to a mob's current frame of animation and position.
 *
 * @param p The point.
 * @param hType Type of hitbox. INVALID means any.
 * @param d Return the distance here, optionally.
 * @return The hitbox.
 */
Hitbox* Mob::getClosestHitbox(
    const Point &p, size_t hType, Distance* d
) const {
    Sprite* s;
    getSpriteData(&s, nullptr, nullptr);
    if(!s) return nullptr;
    Hitbox* closestHitbox = nullptr;
    float closestHitboxDist = 0;
    
    for(size_t h = 0; h < s->hitboxes.size(); h++) {
        Hitbox* hPtr = &s->hitboxes[h];
        if(hType != INVALID && hPtr->type != hType) continue;
        
        float thisD =
            Distance(
                hPtr->getCurPos(pos, angleCos, angleSin), p
            ).toFloat() - hPtr->radius;
        if(closestHitbox == nullptr || thisD < closestHitboxDist) {
            closestHitboxDist = thisD;
            closestHitbox = hPtr;
        }
    }
    
    if(d) *d = closestHitboxDist;
    
    return closestHitbox;
}


/**
 * @brief Returns data for figuring out the state of the current sprite
 * of animation.
 *
 * Normally, this returns the current animation's current sprite,
 * but it can return a forced sprite (e.g. from a status effect that
 * freezes animations).
 *
 * @param outCurSpritePtr If not nullptr, the current frame's sprite is
 * returned here.
 * @param outNextSpritePtr If not nullptr, the next frame's sprite is
 * returned here.
 * @param outInterpolationFactor If not nullptr, the interpolation factor
 * (0 to 1) between the two is returned here.
 */
void Mob::getSpriteData(
    Sprite** outCurSpritePtr, Sprite** outNextSpritePtr,
    float* outInterpolationFactor
) const {
    if(forcedSprite) {
        if(outCurSpritePtr) *outCurSpritePtr = forcedSprite;
        if(outNextSpritePtr) *outNextSpritePtr = forcedSprite;
        if(outInterpolationFactor) *outInterpolationFactor = 0.0f;
    } else {
        anim.getSpriteData(
            outCurSpritePtr, outNextSpritePtr, outInterpolationFactor
        );
    }
}


/**
 * @brief Returns the distance between the limits of this mob and
 * the limits of another.
 *
 * @param m2Ptr Pointer to the mob to check.
 * @param regularDistanceCache If the regular distance had already been
 * calculated, specify it here. This should help with performance.
 * Otherwise, use nullptr.
 * @return The distance.
 */
Distance Mob::getDistanceBetween(
    const Mob* m2Ptr, const Distance* regularDistanceCache
) const {
    Distance mobToHotspotDist;
    float distPadding;
    if(m2Ptr->rectangularDim.x != 0.0f) {
        bool isInside = false;
        Point hotspot =
            getClosestPointInRotatedRectangle(
                pos,
                m2Ptr->pos, m2Ptr->rectangularDim,
                m2Ptr->angle,
                &isInside
            );
        if(isInside) {
            mobToHotspotDist = Distance(0.0f);
        } else {
            mobToHotspotDist = Distance(pos, hotspot);
        }
        distPadding = radius;
    } else {
        if(regularDistanceCache) {
            mobToHotspotDist = *regularDistanceCache;
        } else {
            mobToHotspotDist = Distance(pos, m2Ptr->pos);
        }
        distPadding = radius + m2Ptr->radius;
    }
    mobToHotspotDist -= distPadding;
    return mobToHotspotDist;
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
bool Mob::getFractionNumbersInfo(
    float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
) const {
    if(!carryInfo || carryInfo->curCarryingStrength <= 0) return false;
    bool destinationHasPikminType =
        carryInfo->intendedMob &&
        carryInfo->intendedPikType;
    if(type->weight <= 1 && !destinationHasPikminType) return false;
    
    *outValueNr = carryInfo->curCarryingStrength;
    *outReqNr = type->weight;
    if(carryInfo->isMoving) {
        if(
            carryInfo->destination ==
            CARRY_DESTINATION_SHIP
        ) {
            *outColor = game.config.aestheticGen.carryingColorMove;
            
        } else if(destinationHasPikminType) {
            *outColor =
                carryInfo->intendedPikType->mainColor;
        } else {
            *outColor = game.config.aestheticGen.carryingColorMove;
        }
    } else {
        *outColor = game.config.aestheticGen.carryingColorStop;
    }
    return true;
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param outSpot The final coordinates are returned here.
 * @param outDist The final distance to those coordinates is returned here.
 */
void Mob::getGroupSpotInfo(Point* outSpot, float* outDist) const {
    outSpot->x = 0.0f;
    outSpot->y = 0.0f;
    *outDist = 0.0f;
}


/**
 * @brief Returns how vulnerable the mob is to that specific hazard,
 * or the mob type's default if there is no vulnerability data for that hazard.
 *
 * @param hPtr The hazard to check.
 * @return The vulnerability info.
 */
MobType::Vulnerability Mob::getHazardVulnerability(Hazard* hPtr) const {
    MobType::Vulnerability vuln;
    vuln.effectMult = type->defaultVulnerability;
    
    auto v = type->hazardVulnerabilities.find(hPtr);
    if(v != type->hazardVulnerabilities.end()) {
        vuln = v->second;
    }
    
    return vuln;
}


/**
 * @brief Returns the hitbox in the current animation with the specified number.
 *
 * @param idx The hitbox's index.
 * @return The hitbox.
 */
Hitbox* Mob::getHitbox(size_t idx) const {
    Sprite* s;
    getSpriteData(&s, nullptr, nullptr);
    if(!s) return nullptr;
    if(s->hitboxes.empty()) return nullptr;
    return &s->hitboxes[idx];
}


/**
 * @brief When a mob is meant to be held by a hitbox, this function
 * returns where in the hitbox the mob currently is.
 *
 * @param mobToHold The mob that will be held.
 * @param hPtr Pointer to the hitbox to check.
 * @param offsetDist The distance from the center of the hitbox is
 * returned here. 1 means the full radius.
 * @param offsetAngle The angle the mob to hold makes with the hitbox's
 * center is returned here.
 * @param verticalDist Ratio of distance from the hitbox/body's bottom.
 * 1 is the very top.
 */
void Mob::getHitboxHoldPoint(
    const Mob* mobToHold, const Hitbox* hPtr,
    float* offsetDist, float* offsetAngle, float* verticalDist
) const {
    Point actualHPos = hPtr->getCurPos(pos, angleCos, angleSin);
    float actualHZ = z + hPtr->z;
    
    Point posDif = mobToHold->pos - actualHPos;
    coordinatesToAngle(posDif, offsetAngle, offsetDist);
    
    //Relative to 0 degrees.
    *offsetAngle -= angle;
    //Distance in units to distance in percentage.
    *offsetDist /= hPtr->radius;
    
    if(hPtr->height <= 0.0f) {
        *verticalDist = 0.0f;
    } else {
        *verticalDist = mobToHold->z - actualHZ;
        *verticalDist /= hPtr->height;
    }
}


/**
 * @brief Returns how many Pikmin are currently latched on to this mob.
 *
 * @return The amount.
 */
size_t Mob::getLatchedPikminAmount() const {
    size_t total = 0;
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin.size(); p++
    ) {
        Pikmin* pPtr = game.states.gameplay->mobs.pikmin[p];
        if(pPtr->focusedMob != this) continue;
        if(pPtr->holder.m != this) continue;
        if(!pPtr->latched) continue;
        total++;
    }
    return total;
}


/**
 * @brief Returns the total weight of the Pikmin that are currently
 * latched on to this mob.
 *
 * @return The weight.
 */
float Mob::getLatchedPikminWeight() const {
    float total = 0;
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin.size(); p++
    ) {
        Pikmin* pPtr = game.states.gameplay->mobs.pikmin[p];
        if(pPtr->focusedMob != this) continue;
        if(pPtr->holder.m != this) continue;
        if(!pPtr->latched) continue;
        total += pPtr->type->weight;
    }
    return total;
}


/**
 * @brief If this mob belongs to a player's team, this returns the player team
 * index number (0 for team 1, 1 for team 2, etc.).
 * Otherwise, it returns INVALID.
 *
 * @return The player team index, or INVALID.
 */
size_t Mob::getPlayerTeamIdx() const {
    if(team >= MOB_TEAM_PLAYER_1 && team <= MOB_TEAM_PLAYER_4) {
        return (team - MOB_TEAM_PLAYER_1);
    }
    return INVALID;
}


/**
 * @brief Recalculates the max distance a mob can interact with another mob.
 */
void Mob::updateInteractionSpan() {
    interactionSpan = physicalSpan;
    
    if(farReach != INVALID) {
        interactionSpan =
            std::max(
                std::max(
                    type->reaches[farReach].radius1,
                    type->reaches[farReach].radius2
                ),
                physicalSpan
            );
    }
    if(nearReach != INVALID) {
        interactionSpan =
            std::max(
                std::max(
                    type->reaches[nearReach].radius1,
                    type->reaches[nearReach].radius2
                ),
                physicalSpan
            );
    }
}


/**
 * @brief Returns the speed multiplier for this mob.
 *
 * @return The multiplier.
 */
float Mob::getSpeedMultiplier() const {
    float moveSpeedMult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); s++) {
        if(!statuses[s].toDelete) {
            float vulnMult = this->statuses[s].type->speedMultiplier - 1.0f;
            auto vulnIt = type->statusVulnerabilities.find(statuses[s].type);
            if(vulnIt != type->statusVulnerabilities.end()) {
                vulnMult *= vulnIt->second.effectMult;
            }
            moveSpeedMult *= (vulnMult + 1.0f);
        }
    }
    return moveSpeedMult;
}


/**
 * @brief Returns what the given sprite's center, rotation, tint, etc. should be
 * at the present moment, for normal mob drawing routines.
 *
 * @param sPtr Sprite to get info about.
 * @param nextSPtr Next sprite in the animation, if any.
 * @param interpolationFactor If we're meant to interpolate from the current
 * sprite to the next, specify the interpolation factor (0 to 1) here.
 * @param info Struct to fill the info with.
 * @param effects What effects to use. Use SPRITE_BMP_EFFECT_FLAG for this.
 */
void Mob::getSpriteBitmapEffects(
    Sprite* sPtr, Sprite* nextSPtr, float interpolationFactor,
    BitmapEffect* info, Bitmask16 effects
) const {

    //Animation, position, angle, etc.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_STANDARD)) {
        Point effTrans;
        float effAngle;
        Point effScale;
        ALLEGRO_COLOR effTint;
        
        getSpriteBasicEffects(
            pos, angle, angleCos, angleSin,
            sPtr, nextSPtr, interpolationFactor,
            &effTrans, &effAngle, &effScale, &effTint
        );
        
        info->translation += effTrans;
        info->rotation += effAngle;
        info->scale.x *= effScale.x;
        info->scale.y *= effScale.y;
        info->tintColor.r *= effTint.r;
        info->tintColor.g *= effTint.g;
        info->tintColor.b *= effTint.b;
        info->tintColor.a *= effTint.a;
    }
    
    //Status effects.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_STATUS)) {
        size_t nGlowColors = 0;
        ALLEGRO_COLOR glowColorSum = COLOR_EMPTY;
        
        for(size_t s = 0; s < statuses.size(); s++) {
            StatusType* t = this->statuses[s].type;
            if(
                t->tint.r == 1.0f &&
                t->tint.g == 1.0f &&
                t->tint.b == 1.0f &&
                t->tint.a == 1.0f &&
                t->glow.a == 0.0f
            ) {
                continue;
            }
            
            info->tintColor.r *= t->tint.r;
            info->tintColor.g *= t->tint.g;
            info->tintColor.b *= t->tint.b;
            info->tintColor.a *= t->tint.a;
            
            if(t->glow.a > 0) {
                glowColorSum.r += t->glow.r;
                glowColorSum.g += t->glow.g;
                glowColorSum.b += t->glow.b;
                glowColorSum.a += t->glow.a;
                nGlowColors++;
            }
            
            if(nGlowColors > 0) {
                t->glow.r = glowColorSum.r / nGlowColors;
                t->glow.g = glowColorSum.g / nGlowColors;
                t->glow.b = glowColorSum.b / nGlowColors;
                t->glow.a = glowColorSum.a / nGlowColors;
            }
            
            if(t->shakingEffect != 0.0f) {
                info->translation.x +=
                    sin(
                        game.states.gameplay->areaTimePassed *
                        MOB::STATUS_SHAKING_TIME_MULT
                    ) * t->shakingEffect;
            }
        }
    }
    
    //Sector brightness tint.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS)) {
        Sector* sectorPtr = centerSector;
        float brightness = centerSector->brightness / 255.0;
        if(sectorPtr->fade) {
            Sector* textureSector[2] = {nullptr, nullptr};
            sectorPtr->getTextureMergeSectors(
                &textureSector[0], &textureSector[1]
            );
            vector<Edge*> fadeEdges[2];
            size_t nEdges = sectorPtr->edges.size();
            for(size_t e = 0; e < nEdges; e++) {
                Edge* ePtr = sectorPtr->edges[e];
                Sector* oSector = ePtr->getOtherSector(sectorPtr);
                if(oSector == textureSector[0]) {
                    fadeEdges[0].push_back(ePtr);
                }
                if(oSector == textureSector[1]) {
                    fadeEdges[1].push_back(ePtr);
                }
            }
            
            Distance closestDist[2] = {Distance(FLT_MAX), Distance(FLT_MAX)};
            for(size_t n = 0; n < 2; n++) {
                for(size_t e = 0; e < fadeEdges[n].size(); e++) {
                    Point v1 = v2p(fadeEdges[n][e]->vertexes[0]);
                    Point v2 = v2p(fadeEdges[n][e]->vertexes[1]);
                    float segmentRatio;
                    Point closestPos = getClosestPointInLineSeg(v1, v2, pos, &segmentRatio);
                    if(segmentRatio < 0) {
                        Point v2ToV1 = v2 - v1;
                        closestPos -= v2ToV1 * abs(segmentRatio);
                    }
                    if(segmentRatio > 1) {
                        Point v2ToV1 = v2 - v1;
                        closestPos -= v2ToV1 * (segmentRatio - 1);
                    }
                    
                    Distance d(closestPos, pos);
                    closestDist[n] = closestDist[n] <= d ? closestDist[n] : d;
                }
            }
            float totalBrightness = 0;
            if(textureSector[0]) {
                totalBrightness +=
                    textureSector[0]->brightness *
                    (
                        closestDist[1].toFloat() /
                        (
                            closestDist[0].toFloat() +
                            closestDist[1].toFloat()
                        )
                    );
            }
            if(textureSector[1]) {
                totalBrightness +=
                    textureSector[1]->brightness *
                    (
                        closestDist[0].toFloat() /
                        (
                            closestDist[0].toFloat() +
                            closestDist[1].toFloat()
                        )
                    );
            }
            brightness = totalBrightness / 255.0;
        }
        
        info->tintColor.r *= brightness;
        info->tintColor.g *= brightness;
        info->tintColor.b *= brightness;
    }
    
    //Height effect.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_HEIGHT)) {
        if(heightEffectPivot != LARGE_FLOAT) {
            float heightEffectScale = 1.0;
            //First, check for the mob being in the air.
            heightEffectScale +=
                (z - heightEffectPivot) * MOB::HEIGHT_EFFECT_FACTOR;
            heightEffectScale = std::max(heightEffectScale, 1.0f);
            if(
                groundSector->isBottomlessPit &&
                heightEffectScale == 1.0f
            ) {
                //When atop a pit, heightEffectPivot holds what height
                //the mob fell from.
                heightEffectScale =
                    (z - groundSector->z) /
                    (heightEffectPivot - groundSector->z);
            }
            info->scale *= heightEffectScale;
        }
    }
    
    //Being delivered.
    if(
        hasFlag(effects, SPRITE_BMP_EFFECT_DELIVERY) &&
        deliveryInfo &&
        focusedMob
    ) {
        switch(deliveryInfo->animType) {
        case DELIVERY_ANIM_SUCK: {
            ALLEGRO_COLOR newGlow;
            float newScale;
            Point newOffset;
            
            float shakeScale =
                (1 - deliveryInfo->animTimeRatioLeft) *
                MOB::DELIVERY_SUCK_SHAKING_MULT;
                
            if(deliveryInfo->animTimeRatioLeft < 0.4) {
                shakeScale =
                    std::max(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft, 0.2, 0.4,
                            0.0f, shakeScale),
                        0.0f);
            }
            
            newOffset.x =
                sin(
                    game.states.gameplay->areaTimePassed *
                    MOB::DELIVERY_SUCK_SHAKING_TIME_MULT
                ) * shakeScale;
                
                
            if(deliveryInfo->animTimeRatioLeft > 0.6) {
                //Changing color.
                newGlow =
                    interpolateColor(
                        deliveryInfo->animTimeRatioLeft, 0.6, 1.0,
                        deliveryInfo->color, mapGray(0)
                    );
                newScale = 1.0f;
            } else if(deliveryInfo->animTimeRatioLeft > 0.4) {
                //Fixed in color.
                newGlow = deliveryInfo->color;
                newScale = 1.0f;
            } else {
                //Shrinking.
                newGlow = deliveryInfo->color;
                newScale =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft, 0.0, 0.4,
                        0.0f, 1.0f
                    );
                newScale = ease(EASE_METHOD_OUT, newScale);
                
                Point targetPos = focusedMob->pos;
                
                if(focusedMob->type->category->id == MOB_CATEGORY_SHIPS) {
                    Ship* shiPtr = (Ship*) focusedMob;
                    targetPos = shiPtr->receptacleFinalPos;
                }
                
                Point endOffset = targetPos - pos;
                
                float absorbRatio =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft, 0.0, 0.4,
                        1.0f, 0.0f
                    );
                absorbRatio = ease(EASE_METHOD_IN, absorbRatio);
                newOffset += endOffset * absorbRatio;
            }
            
            info->glowColor.r =
                std::clamp(info->glowColor.r + newGlow.r, 0.0f, 1.0f);
            info->glowColor.g =
                std::clamp(info->glowColor.g + newGlow.g, 0.0f, 1.0f);
            info->glowColor.b =
                std::clamp(info->glowColor.b + newGlow.b, 0.0f, 1.0f);
            info->glowColor.a =
                std::clamp(info->glowColor.a + newGlow.a, 0.0f, 1.0f);
                
            info->scale *= newScale;
            info->translation += newOffset;
            break;
        }
        case DELIVERY_ANIM_TOSS: {
            Point newOffset;
            float newScale = 1.0f;
            
            if(deliveryInfo->animTimeRatioLeft > 0.85) {
                //Wind-up.
                newOffset.y =
                    sin(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft,
                            0.85f, 1.0f,
                            0.0f, TAU / 2.0f
                        )
                    );
                newOffset.y *= MOB::DELIVERY_TOSS_WINDUP_MULT;
            } else {
                //Toss.
                newOffset.y =
                    sin(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft,
                            0.0f, 0.85f,
                            TAU / 2.0f, TAU
                        )
                    );
                newOffset.y *= MOB::DELIVERY_TOSS_MULT;
                //Randomly deviate left or right, slightly.
                float deviationMult =
                    hashNr((unsigned int) id) / (float) UINT32_MAX;
                deviationMult = deviationMult * 2.0f - 1.0f;
                deviationMult *= MOB::DELIVERY_TOSS_X_OFFSET;
                newOffset.x =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft,
                        0.0f, 0.85f,
                        1.0f, 0.0f
                    ) * deviationMult;
                newScale =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft,
                        0.0f, 0.85f,
                        0.1f, 1.0f
                    );
            }
            
            info->translation += newOffset;
            info->scale *= newScale;
            break;
        }
        }
        
    }
    
    //Damage squash and stretch.
    if(
        hasFlag(effects, SPRITE_BMP_EFFECT_DAMAGE) &&
        damageSquashTime > 0.0f
    ) {
        float damageSquashTimeRatio =
            damageSquashTime / MOB::DAMAGE_SQUASH_DURATION;
        float damageScaleY;
        if(damageSquashTimeRatio > 0.5) {
            damageScaleY =
                interpolateNumber(
                    damageSquashTimeRatio,
                    0.5f, 1.0f, 0.0f, 1.0f
                );
            damageScaleY =
                ease(
                    EASE_METHOD_UP_AND_DOWN,
                    damageScaleY
                );
            damageScaleY *= MOB::DAMAGE_SQUASH_AMOUNT;
        } else {
            damageScaleY =
                interpolateNumber(
                    damageSquashTimeRatio,
                    0.0f, 0.5f, 1.0f, 0.0f
                );
            damageScaleY =
                ease(
                    EASE_METHOD_UP_AND_DOWN,
                    damageScaleY
                );
            damageScaleY *= -MOB::DAMAGE_SQUASH_AMOUNT;
        }
        damageScaleY += 1.0f;
        info->scale.y *= damageScaleY;
        info->scale.x *= 1.0f / damageScaleY;
    }
    
    //Carry sway.
    if(
        hasFlag(effects, SPRITE_BMP_EFFECT_CARRY) &&
        carryInfo
    ) {
        if(carryInfo->isMoving) {
            float factor1 =
                sin(
                    game.states.gameplay->areaTimePassed *
                    MOB::CARRY_SWAY_TIME_MULT
                );
            float factor2 =
                sin(
                    game.states.gameplay->areaTimePassed *
                    MOB::CARRY_SWAY_TIME_MULT * 2.0f
                );
            info->translation.x -=
                factor1 * MOB::CARRY_SWAY_X_TRANSLATION_AMOUNT;
            info->translation.y -=
                factor2 * MOB::CARRY_SWAY_Y_TRANSLATION_AMOUNT;
            info->rotation -=
                factor1 * MOB::CARRY_SWAY_ROTATION_AMOUNT;
        }
    }
}


/**
 * @brief Returns the current sprite of one of the status effects
 * that the mob is under.
 *
 * @param bmpScale Returns the mob size's scale to apply to the image.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* Mob::getStatusBitmap(float* bmpScale) const {
    *bmpScale = 0.0f;
    for(size_t st = 0; st < this->statuses.size(); st++) {
        StatusType* t = this->statuses[st].type;
        if(t->overlayAnimation.empty()) continue;
        Sprite* sp;
        t->overlayAnim.getSpriteData(&sp, nullptr, nullptr);
        if(!sp) return nullptr;
        *bmpScale = t->overlayAnimMobScale;
        return sp->bitmap;
    }
    return nullptr;
}


/**
 * @brief Handles a status effect being applied.
 *
 * @param staType Status type to check.
 */
void Mob::handleStatusEffectGain(StatusType* staType) {
    if(staType->stateChangeType == STATUS_STATE_CHANGE_CUSTOM) {
        size_t nr = fsm.getStateIdx(staType->stateChangeName);
        if(nr != INVALID) {
            fsm.setState(nr);
        }
    }
}


/**
 * @brief Handles a status effect being removed.
 *
 * @param staType Status type to check.
 */
void Mob::handleStatusEffectLoss(StatusType* staType) {
}


/**
 * @brief Returns whether or not this mob has a clear line towards another mob.
 * In other words, if a straight line is drawn between both,
 * is this line clear, or is it interrupted by a wall or pushing mob?
 *
 * @param targetMob The mob to check against.
 * @return Whether it has a clear line.
 */
bool Mob::hasClearLine(const Mob* targetMob) const {
    //First, get a bounding box of the line to check.
    //This will help with performance later.
    Point bbTL = pos;
    Point bbBR = pos;
    updateMinMaxCoords(bbTL, bbBR, targetMob->pos);
    
    const float selfMaxZ = z + height;
    const float targetMobMaxZ = targetMob->z + targetMob->height;
    
    //Check against other mobs.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        
        if(!mPtr->type->pushes) continue;
        if(mPtr == this || mPtr == targetMob) continue;
        if(hasFlag(mPtr->flags, MOB_FLAG_INTANGIBLE)) continue;
        
        const float mPtrMaxZ = mPtr->z + mPtr->height;
        if(mPtrMaxZ < selfMaxZ || mPtrMaxZ < targetMobMaxZ) continue;
        if(
            mPtr->z > z + height &&
            mPtr->z > targetMob->z + targetMob->height
        ) {
            continue;
        }
        if(
            targetMob->standingOnMob == mPtr &&
            fabs(z - targetMob->z) <= GEOMETRY::STEP_HEIGHT
        ) {
            continue;
        }
        if(
            !rectanglesIntersect(
                bbTL, bbBR,
                mPtr->pos - mPtr->physicalSpan,
                mPtr->pos + mPtr->physicalSpan
            )
        ) {
            continue;
        }
        
        if(mPtr->rectangularDim.x != 0.0f) {
            if(
                lineSegIntersectsRotatedRectangle(
                    pos, targetMob->pos,
                    mPtr->pos, mPtr->rectangularDim, mPtr->angle
                )
            ) {
                return false;
            }
        } else {
            if(
                circleIntersectsLineSeg(
                    mPtr->pos, mPtr->radius,
                    pos, targetMob->pos,
                    nullptr, nullptr
                )
            ) {
                return false;
            }
        }
    }
    
    //Check against walls.
    //We can ignore walls that are below or within stepping distance of
    //both mobs, so use the lowest of the two Zs as a cut-off point.
    if(
        areaWallsBetween(
            pos, targetMob->pos,
            std::min(z + height, targetMob->z + targetMob->height) +
            GEOMETRY::STEP_HEIGHT
        )
    ) {
        return false;
    }
    
    //Seems good!
    return true;
}


/**
 * @brief Starts holding the specified mob.
 *
 * @param m  Mob to start holding.
 * @param hitboxIdx Index of the hitbox to hold on. INVALID for mob center.
 * @param offsetDist Distance from the hitbox/body center. 1 is full radius.
 * @param offsetAngle Hitbox/body angle from which the mob will be held.
 * @param verticalDist Ratio of distance from the hitbox/body's bottom.
 * 1 is the very top.
 * @param forceAboveHolder If true, force the mob to be drawn above the holder.
 * @param rotationMethod How should the held mob rotate?
 */
void Mob::hold(
    Mob* m, size_t hitboxIdx,
    float offsetDist, float offsetAngle,
    float verticalDist,
    bool forceAboveHolder, const HOLD_ROTATION_METHOD rotationMethod
) {
    if(m->holder.m != nullptr && m->holder.m != this) {
        //A different mob is already holding it.
        return;
    }
    for(size_t h = 0; h < holding.size(); h++) {
        if(holding[h] == m) {
            //It's already holding the mob.
            return;
        }
    }
    
    holding.push_back(m);
    m->holder.m = this;
    m->holder.hitboxIdx = hitboxIdx;
    m->holder.offsetDist = offsetDist;
    m->holder.offsetAngle = offsetAngle;
    m->holder.verticalDist = verticalDist;
    m->holder.forceAboveHolder = forceAboveHolder;
    m->holder.rotationMethod = rotationMethod;
    m->fsm.runEvent(MOB_EV_HELD, (void*) this);
    
    if(standingOnMob) {
        if(m->type->weight > 0) {
            //Better inform the mob below that extra weight has been added.
            standingOnMob->fsm.runEvent(MOB_EV_WEIGHT_ADDED, (void*) m);
        }
    }
}



/**
 * @brief Checks if a mob is completely off-camera.
 *
 * @param viewport What viewport to calculate with.
 * @return Whether it is off-camera.
 */
bool Mob::isOffCamera(const Viewport &viewport) const {
    if(parent) return false;
    
    float spriteBound = 0;
    Sprite* sPtr;
    anim.getSpriteData(&sPtr, nullptr, nullptr);
    if(sPtr) {
        Point spriteSize = sPtr->bmpSize;
        spriteBound =
            std::max(
                spriteSize.x / 2.0,
                spriteSize.y / 2.0
            );
    }
    
    float collisionBound = 0;
    if(rectangularDim.x == 0) {
        collisionBound = radius;
    } else {
        collisionBound =
            std::max(
                rectangularDim.x / 2.0,
                rectangularDim.y / 2.0
            );
    }
    
    float radiusToUse = std::max(spriteBound, collisionBound);
    return !BBoxCheck(viewport.box[0], viewport.box[1], pos, radiusToUse);
}


/**
 * @brief Checks if the given point is on top of the mob.
 *
 * @param p Point to check.
 * @return Whether it is on top.
 */
bool Mob::isPointOn(const Point &p) const {
    if(rectangularDim.x == 0) {
        return Distance(p, pos) <= radius;
        
    } else {
        Point pDelta = p - pos;
        pDelta = rotatePoint(pDelta, -angle);
        pDelta += rectangularDim / 2.0f;
        
        return
            pDelta.x > 0 && pDelta.x < rectangularDim.x &&
            pDelta.y > 0 && pDelta.y < rectangularDim.y;
    }
}


/**
 * @brief Checks if a mob is resistant to all of the hazards inside a
 * given list.
 *
 * @param hazards List of hazards to check.
 * @return Whether it is resitant.
 */
bool Mob::isResistantToHazards(const vector<Hazard*> &hazards) const {
    for(size_t h = 0; h < hazards.size(); h++) {
        if(getHazardVulnerability(hazards[h]).effectMult != 0.0f) {
            return false;
        }
    }
    return true;
}


/**
 * @brief Checks if a mob or its parent is stored inside another mob.
 *
 * @return Whether it is stored.
 */
bool Mob::isStoredInsideMob() const {
    if(storedInsideAnother) return true;
    if(parent && parent->m->storedInsideAnother) return true;
    return false;
}


/**
 * @brief Removes a mob from its leader's group.
 */
void Mob::leaveGroup() {
    if(!followingGroup) return;
    
    Mob* groupLeader = followingGroup;
    
    groupLeader->group->members.erase(
        find(
            groupLeader->group->members.begin(),
            groupLeader->group->members.end(),
            this
        )
    );
    
    groupLeader->group->initSpots(this);
    
    groupLeader->group->changeStandbyTypeIfNeeded();
    
    followingGroup = nullptr;
    
    if(groupLeader->type->category->id == MOB_CATEGORY_LEADERS) {
        Leader* leaPtr = (Leader*) groupLeader;
        if(leaPtr->player) {
            game.states.gameplay->updateClosestGroupMembers(leaPtr->player);
        }
    }
}


/**
 * @brief Makes the mob start going towards the final destination of its path.
 *
 * @param speed Speed to move at.
 * @param acceleration Speed acceleration.
 */
void Mob::moveToPathEnd(float speed, float acceleration) {
    if(!pathInfo) return;
    if(
        (
            pathInfo->settings.flags &
            PATH_FOLLOW_FLAG_FOLLOW_MOB
        ) &&
        pathInfo->settings.targetMob
    ) {
        chase(
            &(pathInfo->settings.targetMob->pos),
            &(pathInfo->settings.targetMob->z),
            Point(), 0.0f,
            CHASE_FLAG_ANY_ANGLE,
            pathInfo->settings.finalTargetDistance,
            speed, acceleration
        );
    } else {
        chase(
            pathInfo->settings.targetPoint,
            getSector(pathInfo->settings.targetPoint, nullptr, true)->z,
            CHASE_FLAG_ANY_ANGLE,
            pathInfo->settings.finalTargetDistance,
            speed, acceleration
        );
    }
}


/**
 * @brief Plays a sound from the list of sounds in the mob type's data.
 *
 * @param soundDataIdx Index of the sound data in the list.
 * @return The sound source ID.
 */
size_t Mob::playSound(size_t soundDataIdx) {
    if(soundDataIdx >= type->sounds.size()) return 0;
    
    if(
        game.states.gameplay->areaTimePassed == 0.0f ||
        game.states.gameplay->curInterlude != INTERLUDE_NONE
    ) {
        //During interludes or area load, don't play any mob sounds.
        //This allows stuff like obstacles being cleared upon starting and
        //not playing the obstacle clear jingle.
        return 0;
    }
    
    MobType::Sound* sound = &type->sounds[soundDataIdx];
    
    switch(sound->type) {
    case SOUND_TYPE_GAMEPLAY_GLOBAL: {
        return
            game.audio.createGlobalSoundSource(
                sound->sample, false, sound->config
            );
        break;
    } case SOUND_TYPE_GAMEPLAY_POS: {
        return
            game.audio.createMobSoundSource(
                sound->sample, this, false, sound->config
            );
        break;
    } case SOUND_TYPE_AMBIANCE_GLOBAL: {
        return
            game.audio.createGlobalSoundSource(
                sound->sample, true, sound->config
            );
        break;
    } case SOUND_TYPE_AMBIANCE_POS: {
        return
            game.audio.createMobSoundSource(
                sound->sample, this, true, sound->config
            );
        break;
    } case SOUND_TYPE_UI: {
        return
            game.audio.createUiSoundsource(
                sound->sample, sound->config
            );
    }
    }
    
    return 0;
}


/**
 * @brief Returns a string containing the FSM state history for this mob.
 * This is used for debugging crashes.
 *
 * @return The string.
 */
string Mob::printStateHistory() const {
    string str = "State history: ";
    
    if(fsm.curState) {
        str += fsm.curState->name;
    } else {
        str += "No current state!";
        return str;
    }
    
    for(size_t s = 0; s < STATE_HISTORY_SIZE; s++) {
        str += ", " + fsm.prevStateNames[s];
    }
    str += ".";
    
    return str;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Mob::readScriptVars(const ScriptVarReader &svr) {
    string teamVar;
    
    if(svr.get("team", teamVar)) {
        MOB_TEAM teamNr = stringToTeamNr(teamVar);
        if(teamNr == INVALID) {
            game.errors.report(
                "Unknown team name \"" + teamVar +
                "\", when trying to create mob (" +
                getErrorMessageMobInfo(this) + ")!", nullptr
            );
        } else {
            team = teamNr;
        }
    }
    
    if(svr.get("max_health", maxHealth)) {
        maxHealth = std::max(1.0f, maxHealth);
        health = maxHealth;
    }
    
    if(svr.get("health", health)) {
        health = std::min(health, maxHealth);
    }
}


/**
 * @brief Stop holding a mob.
 *
 * @param m Mob to release.
 */
void Mob::release(Mob* m) {
    size_t idx = INVALID;
    for(size_t h = 0; h < holding.size(); h++) {
        if(holding[h] == m) {
            idx = h;
            break;
        }
    }
    
    if(idx == INVALID) {
        //It's not holding the mob.
        return;
    }
    
    m->fsm.runEvent(MOB_EV_RELEASED, (void*) this);
    holding.erase(holding.begin() + idx);
    m->holder.clear();
    
    if(standingOnMob) {
        if(m->type->weight > 0) {
            //Better inform the mob below that weight has been removed.
            standingOnMob->fsm.runEvent(MOB_EV_WEIGHT_REMOVED, (void*) m);
        }
    }
}


/**
 * @brief Safely releases all chomped Pikmin.
 */
void Mob::releaseChompedPikmin() {
    for(size_t p = 0; p < chompingMobs.size(); p++) {
        if(!chompingMobs[p]) continue;
        release(chompingMobs[p]);
    }
    chompingMobs.clear();
}


/**
 * @brief Releases any mobs stored inside.
 */
void Mob::releaseStoredMobs() {
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        if(mPtr->storedInsideAnother == this) {
            release(mPtr);
            mPtr->storedInsideAnother = nullptr;
            mPtr->timeAlive = 0.0f;
            float a = game.rng.f(0, TAU);
            const float momentum = 100;
            mPtr->speed.x = cos(a) * momentum;
            mPtr->speed.y = sin(a) * momentum;
            mPtr->speedZ = momentum * 7;
        }
    }
}


/**
 * @brief Removes all particle generators with the given ID.
 *
 * @param id ID of particle generators to remove.
 */
void Mob::removeParticleGenerator(const MOB_PARTICLE_GENERATOR_ID id) {
    for(size_t g = 0; g < particleGenerators.size();) {
        if(particleGenerators[g].id == id) {
            particleGenerators.erase(particleGenerators.begin() + g);
        } else {
            g++;
        }
    }
}


/**
 * @brief Respawns an object back to its home.
 */
void Mob::respawn() {
    pos = home;
    centerSector = getSector(pos, nullptr, true);
    groundSector = centerSector;
    z = centerSector->z + 100;
}


/**
 * @brief Sends a script message to another mob. This calls the mob's
 * "message received" event, with the message as data.
 *
 * @param receiver Mob that will receive the message.
 * @param msg The message.
 */
void Mob::sendScriptMessage(Mob* receiver, string &msg) const {
    MobEvent* ev = receiver->fsm.getEvent(MOB_EV_RECEIVE_MESSAGE);
    if(!ev) return;
    ev->run(receiver, (void*) &msg, (void*) this);
}


/**
 * @brief Sets the mob's animation.
 *
 * @param idx Animation index.
 * It's the animation instance index from the database.
 * @param options Options to start the new animation with.
 * @param preNamed If true, the animation has already been named in-engine.
 * @param mobSpeedAnimBaseline If not 0, the animation's speed will depend on
 * the mob's speed, using this value as a baseline (for 1.0x speed).
 */
void Mob::setAnimation(
    size_t idx, const START_ANIM_OPTION options, bool preNamed,
    float mobSpeedAnimBaseline
) {
    if(idx >= type->animDb->animations.size()) return;
    
    size_t finalIdx;
    if(preNamed) {
        if(anim.animDb->preNamedConversions.size() <= idx) return;
        finalIdx = anim.animDb->preNamedConversions[idx];
    } else {
        finalIdx = idx;
    }
    
    if(finalIdx == INVALID) {
        game.errors.report(
            "Mob (" + getErrorMessageMobInfo(this) +
            ") tried to switch from " +
            (
                anim.curAnim ? "animation \"" + anim.curAnim->name + "\"" :
                "no animation"
            ) +
            " to a non-existent one (with the internal"
            " number of " + i2s(idx) + ")!"
        );
        return;
    }
    
    Animation* newAnim = anim.animDb->animations[finalIdx];
    anim.curAnim = newAnim;
    this->mobSpeedAnimBaseline = mobSpeedAnimBaseline;
    
    if(newAnim->frames.empty()) {
        anim.curFrameIdx = INVALID;
    } else {
        if(
            !hasFlag(options, START_ANIM_OPTION_NO_RESTART) ||
            anim.curFrameIdx >= anim.curAnim->frames.size()
        ) {
            anim.toStart();
        }
    }
    
    if(options == START_ANIM_OPTION_RANDOM_TIME) {
        anim.skipAheadRandomly();
    } else if(options == START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN) {
        if(timeAlive == 0.0f) {
            anim.skipAheadRandomly();
        }
    }
}


/**
 * @brief Sets the mob's animation, given its name.
 * If there is no animation with that name, nothing happens.
 *
 * @param name Name of the animation.
 * @param options Options to start the new animation with.
 * @param mobSpeedAnimBaseline If not 0, the animation's speed will depend on
 * the mob's speed, using this value as a baseline (for 1.0x speed).
 */
void Mob::setAnimation(
    const string &name, const START_ANIM_OPTION options,
    float mobSpeedAnimBaseline
) {
    size_t idx = anim.animDb->findAnimation(name);
    if(idx != INVALID) {
        setAnimation(idx, options, false, mobSpeedAnimBaseline);
    }
}


/**
 * @brief Sets whether the mob can block paths from here on.
 *
 * @param blocks Whether it can block paths or not.
 */
void Mob::setCanBlockPaths(bool blocks) {
    if(blocks) {
        if(!canBlockPaths) {
            game.states.gameplay->pathMgr.handleObstacleAdd(this);
            canBlockPaths = true;
        }
    } else {
        if(canBlockPaths) {
            game.states.gameplay->pathMgr.handleObstacleRemove(this);
            canBlockPaths = false;
        }
    }
}


/**
 * @brief Changes a mob's health, relatively or absolutely.
 *
 * @param add If true, change is relative to the current value
 * (i.e. add or subtract from current health).
 * If false, simply set to that number.
 * @param ratio If true, the specified value represents the max health ratio.
 * If false, it's the number in HP.
 * @param amount Health amount.
 */
void Mob::setHealth(bool add, bool ratio, float amount) {
    float change = amount;
    if(ratio) change = maxHealth * amount;
    float baseNr = 0;
    if(add) baseNr = health;
    
    health = std::clamp(baseNr + change, 0.0f, maxHealth);
}


/**
 * @brief Sets the mob's radius to a different value.
 *
 * @param radius New radius.
 */
void Mob::setRadius(float radius) {
    this->radius = radius;
    physicalSpan =
        calculateMobPhysicalSpan(
            radius,
            type->animDb->hitboxSpan,
            rectangularDim
        );
    updateInteractionSpan();
}


/**
 * @brief Sets the mob's rectangular dimensions to a different value.
 *
 * @param rectangularDim New rectangular dimensions.
 */
void Mob::setRectangularDim(const Point &rectangularDim) {
    this->rectangularDim = rectangularDim;
    physicalSpan =
        calculateMobPhysicalSpan(
            radius,
            type->animDb ? type->animDb->hitboxSpan : 0.0f,
            rectangularDim
        );
    updateInteractionSpan();
}


/**
 * @brief Changes the timer's time and interval.
 *
 * @param time New time.
 */
void Mob::setTimer(float time) {
    scriptTimer.duration = time;
    scriptTimer.start();
}


/**
 * @brief Sets a script variable's value.
 *
 * @param name The variable's name.
 * @param value The variable's new value.
 */
void Mob::setVar(const string &name, const string &value) {
    vars[name] = value;
}


/**
 * @brief Makes the current mob spawn a new mob, given some spawn information.
 *
 * @param info Structure with information about how to spawn it.
 * @param typePtr If nullptr, the pointer to the mob type is obtained given its
 * name in the information structure. If not nullptr, uses this instead.
 * @return The new mob.
 */
Mob* Mob::spawn(const MobType::SpawnInfo* info, MobType* typePtr) {
    //First, find the mob.
    if(!typePtr) {
        typePtr = game.mobCategories.findMobType(info->mobTypeName);
    }
    
    if(!typePtr) {
        game.errors.report(
            "Mob (" + getErrorMessageMobInfo(this) +
            ") tried to spawn an object of the "
            "type \"" + info->mobTypeName + "\", but there is no such "
            "object type!"
        );
        return nullptr;
    }
    
    if(
        typePtr->category->id == MOB_CATEGORY_PIKMIN &&
        game.states.gameplay->mobs.pikmin.size() >=
        game.config.rules.maxPikminInField
    ) {
        return nullptr;
    }
    
    Point newXY;
    float newZ = 0;
    float newAngle = 0;
    
    if(info->relative) {
        newXY = pos + rotatePoint(info->coordsXY, angle);
        newZ = z + info->coordsZ;
        newAngle = angle + info->angle;
    } else {
        newXY = info->coordsXY;
        newZ = info->coordsZ;
        newAngle = info->angle;
    }
    
    if(!getSector(newXY, nullptr, true)) {
        //Spawn out of bounds? No way!
        return nullptr;
    }
    
    Mob* newMob =
        createMob(
            typePtr->category,
            newXY,
            typePtr,
            newAngle,
            info->vars
        );
        
    newMob->z = newZ;
    
    if(typePtr->category->id == MOB_CATEGORY_TREASURES) {
        //This way, treasures that fall into the abyss respawn at the
        //spawner mob's original spot.
        newMob->home = home;
    } else {
        newMob->home = newXY;
    }
    
    if(info->linkObjectToSpawn) {
        push_anonymous_link(newMob);
    }
    if(info->linkSpawnToObject) {
        newMob->push_anonymous_link(this);
    }
    if(info->momentum != 0) {
        float a = game.rng.f(0, TAU);
        newMob->speed.x = cos(a) * info->momentum;
        newMob->speed.y = sin(a) * info->momentum;
        newMob->speedZ = info->momentum * 7;
    }
    
    return newMob;
}


/**
 * @brief Sets up stuff for the beginning of the mob's death process.
 */
void Mob::startDying() {
    setHealth(false, false, 0.0f);
    
    stopChasing();
    stopTurning();
    gravityMult = 1.0;
    
    for(size_t s = 0; s < statuses.size(); s++) {
        statuses[s].toDelete = true;
    }
    
    if(group) {
        while(!group->members.empty()) {
            Mob* member = group->members[0];
            member->fsm.runEvent(
                MOB_EV_DISMISSED,
                (void*) & (member->pos)
            );
            if(type->category->id != MOB_CATEGORY_LEADERS) {
                //The Pikmin were likely following an enemy.
                //So they were likely invincible. Let's correct that.
                disableFlag(member->flags, MOB_FLAG_NON_HUNTABLE);
                disableFlag(member->flags, MOB_FLAG_NON_HURTABLE);
                member->team = MOB_TEAM_PLAYER_1;
            }
            member->leaveGroup();
        }
    }
    
    releaseStoredMobs();
    
    startDyingClassSpecifics();
}


/**
 * @brief Sets up stuff for the beginning of the mob's death process.
 * This function is meant to be overridden by child classes.
 */
void Mob::startDyingClassSpecifics() {
}


/**
 * @brief Returns the height that should be used in calculating
 * drawing order.
 */
float Mob::getDrawingHeight() const {
    //We can't use FLT_MAX since multiple mobs with max height can stack.
    return height == 0 ? 1000000 : height;
}

/**
 * @brief From here on out, the mob's Z changes will be reflected in the height
 * effect.
 */
void Mob::startHeightEffect() {
    heightEffectPivot = z;
}


/**
 * @brief Makes a mob not follow any target any more.
 */
void Mob::stopChasing() {
    chaseInfo.state = CHASE_STATE_STOPPED;
    chaseInfo.origZ = nullptr;
    
    speed.x = 0.0f;
    speed.y = 0.0f;
    if(hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
        speedZ = 0.0f;
    }
}


/**
 * @brief Makes the mob stop circling around a point or another mob.
 */
void Mob::stopCircling() {
    if(circlingInfo) {
        delete circlingInfo;
        circlingInfo = nullptr;
        stopChasing();
    }
}


/**
 * @brief Makes the mob stop following a path graph.
 */
void Mob::stopFollowingPath() {
    if(!pathInfo) return;
    
    stopChasing();
    
    delete pathInfo;
    pathInfo = nullptr;
}


/**
 * @brief From here on out, stop using the height effect.
 */
void Mob::stopHeightEffect() {
    if(
        type->category->id == MOB_CATEGORY_LEADERS &&
        highestMidairZ != FLT_MAX
    ) {
        float distanceFallen = highestMidairZ - z;
        if(distanceFallen > 0.0f) {
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parLeaderLand, this
                );
            adjustKeyframeInterpolatorValues<float>(
                pg.baseParticle.size,
            [ = ] (const float & s) {
                return
                    std::min(
                        GAMEPLAY::LEADER_LAND_PART_MAX_SIZE,
                        s * distanceFallen *
                        GAMEPLAY::LEADER_LAND_PART_SIZE_MULT
                    );
            }
            );
            pg.followZOffset = 1.0f;
            pg.baseParticle.priority = PARTICLE_PRIORITY_HIGH;
            particleGenerators.push_back(pg);
        }
    }
    
    heightEffectPivot = LARGE_FLOAT;
}


/**
 * @brief Makes a mob stop riding on a track mob.
 */
void Mob::stopTrackRide() {
    if(!trackInfo) return;
    
    delete trackInfo;
    trackInfo = nullptr;
    stopChasing();
    speedZ = 0;
    stopHeightEffect();
}


/**
 * @brief Makes a mob stop wanting to turn towards some direciton.
 */
void Mob::stopTurning() {
    face(angle, nullptr, true);
}


/**
 * @brief Stores a mob inside of this one, if possible.
 *
 * @param m The mob to store.
 */
void Mob::storeMobInside(Mob* m) {
    //First, go up the chain to make sure we're not trying to make a loop.
    Mob* temp = this;
    while(temp) {
        if(temp == m) return;
        temp = temp->storedInsideAnother;
    }
    
    hold(
        m, INVALID, 0.0f, 0.0f, 0.5f,
        false, HOLD_ROTATION_METHOD_NEVER
    );
    m->storedInsideAnother = this;
}


/**
 * @brief Makes the mob swallow some of the opponents it has chomped on.
 *
 * @param amount Number of captured opponents to swallow.
 */
void Mob::swallowChompedPikmin(size_t amount) {
    amount = std::min(amount, chompingMobs.size());
    
    vector<float> pickRandomFloats;
    for(size_t f = 0; f < chompingMobs.size(); f++) {
        pickRandomFloats.push_back(game.rng.f(0.0f, 1.0f));
    }
    vector<Mob*> shuffledList =
        shuffleVector(chompingMobs, pickRandomFloats);
        
    for(size_t p = 0; p < amount; p++) {
        swallowChompedPikmin(shuffledList[p]);
    }
}


/**
 * @brief Makes the mob swallow a specific opponent it has chomped on.
 *
 * @param mPtr Pointer to the chomped mob.
 */
void Mob::swallowChompedPikmin(Mob* mPtr) {
    if(!mPtr) return;
    
    size_t idx = INVALID;
    for(size_t m = 0; m < chompingMobs.size(); m++) {
        if(chompingMobs[m] == mPtr) {
            idx = m;
            break;
        }
    }
    
    if(idx == INVALID) {
        //It's not chomping the mob.
        return;
    }
    
    mPtr->fsm.runEvent(MOB_EV_SWALLOWED);
    mPtr->causeSpikeDamage(this, true);
    mPtr->setHealth(false, false, 0.0f);
    release(mPtr);
    if(mPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
        game.statistics.pikminEaten++;
    }
    
    chompingMobs.erase(chompingMobs.begin() + idx);
    
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * This basically calls sub-tickers.
 * Think of it this way: when you want to go somewhere,
 * you first think about rotating your body to face that
 * point, and then think about moving your legs.
 * Then, the actual physics go into place, your nerves
 * send signals to the muscles, and gravity, intertia, etc.
 * take over the rest, to make you move.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tick(float deltaT) {
    //Since the mob could be marked for deletion after any little
    //interaction with the world, and since doing logic on a mob that already
    //forgot some things due to deletion is dangerous... Let's constantly
    //check if the mob is scheduled for deletion, and bail if so.
    
    if(toDelete) return;
    
    //Brain.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Brain");
    }
    tickBrain(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Physics.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Physics");
    }
    tickPhysics(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Misc. logic.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Misc. logic");
    }
    tickMiscLogic(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Animation.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Animation");
    }
    tickAnimation(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Script.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Script");
    }
    tickScript(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Class specifics.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Misc. specifics");
    }
    tickClassSpecifics(deltaT);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
}


/**
 * @brief Ticks animation time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickAnimation(float deltaT) {
    float mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); s++) {
        float vulnMult = this->statuses[s].type->animSpeedMultiplier - 1.0f;
        auto vulnIt = type->statusVulnerabilities.find(statuses[s].type);
        if(vulnIt != type->statusVulnerabilities.end()) {
            vulnMult *= vulnIt->second.effectMult;
        }
        mult *= (vulnMult + 1.0f);
    }
    
    if(mobSpeedAnimBaseline != 0.0f) {
        float mobSpeedMult = chaseInfo.curSpeed / mobSpeedAnimBaseline;
        mobSpeedMult =
            std::clamp(
                mobSpeedMult,
                MOB::MOB_SPEED_ANIM_MIN_MULT, MOB::MOB_SPEED_ANIM_MAX_MULT
            );
        mult *= mobSpeedMult;
    }
    
    vector<size_t> frameSignals;
    vector<size_t> frameSounds;
    bool finishedAnim =
        anim.tick(deltaT * mult, &frameSignals, &frameSounds);
        
    if(finishedAnim) {
        fsm.runEvent(MOB_EV_ANIMATION_END);
    }
    for(size_t s = 0; s < frameSignals.size(); s++) {
        fsm.runEvent(MOB_EV_FRAME_SIGNAL, &frameSignals[s]);
    }
    for(size_t s = 0; s < frameSounds.size(); s++) {
        playSound(frameSounds[s]);
    }
    
    for(size_t h = 0; h < hitOpponents.size();) {
        hitOpponents[h].first -= deltaT;
        if(hitOpponents[h].first <= 0.0f) {
            hitOpponents.erase(hitOpponents.begin() + h);
        } else {
            h++;
        }
    }
    
    if(parent && parent->limbAnim.animDb) {
        parent->limbAnim.tick(deltaT * mult);
    }
}


/**
 * @brief Ticks the mob's brain for the next frame.
 *
 * This has nothing to do with the mob's individual script.
 * This is related to mob-global things, like
 * thinking about where to move next and such.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickBrain(float deltaT) {
    //Circling around something.
    if(circlingInfo) {
        Point circlingCenter =
            circlingInfo->circlingMob ?
            circlingInfo->circlingMob->pos :
            circlingInfo->circlingPoint;
        float circlingZ =
            circlingInfo->circlingMob ?
            circlingInfo->circlingMob->z :
            z;
            
        circlingInfo->curAngle +=
            linearDistToAngular(
                circlingInfo->speed * deltaT, circlingInfo->radius
            ) *
            (circlingInfo->clockwise ? 1 : -1);
            
        chase(
            circlingCenter + angleToCoordinates(
                circlingInfo->curAngle, circlingInfo->radius
            ),
            circlingZ,
            (circlingInfo->canFreeMove ? CHASE_FLAG_ANY_ANGLE : 0),
            PATHS::DEF_CHASE_TARGET_DISTANCE,
            circlingInfo->speed
        );
    }
    
    //Chasing a target.
    if(
        chaseInfo.state == CHASE_STATE_CHASING &&
        !hasFlag(chaseInfo.flags, CHASE_FLAG_TELEPORT) &&
        (speedZ == 0 || hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR))
    ) {
    
        //Calculate where the target is.
        Point finalTargetPos = getChaseTarget();
        Distance horizDist = Distance(pos, finalTargetPos);
        float vertDist = 0.0f;
        if(hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            float finalTargetZ = chaseInfo.offsetZ;
            if(chaseInfo.origZ) finalTargetZ += *chaseInfo.origZ;
            vertDist = fabs(z - finalTargetZ);
        }
        
        if(
            horizDist > chaseInfo.targetDist ||
            vertDist > 1.0f
        ) {
            //If it still hasn't reached its target
            //(or close enough to the target),
            //time to make it think about how to get there.
            
            //Let the mob think about facing the actual target.
            if(!type->canFreeMove && horizDist > 0.0f) {
                face(getAngle(pos, finalTargetPos), nullptr);
            }
            
        } else {
            //Reached the chase location.
            bool direct =
                pathInfo &&
                (
                    pathInfo->result == PATH_RESULT_DIRECT ||
                    pathInfo->result == PATH_RESULT_DIRECT_NO_STOPS
                );
            if(
                pathInfo && !direct &&
                pathInfo->blockReason == PATH_BLOCK_REASON_NONE
            ) {
            
                pathInfo->curPathStopIdx++;
                
                if(pathInfo->curPathStopIdx < pathInfo->path.size()) {
                    //Reached a regular stop while traversing the path.
                    //Think about going to the next, if possible.
                    if(pathInfo->checkBlockage(&pathInfo->blockReason)) {
                        //Oop, there's an obstacle! Or some other blockage.
                        fsm.runEvent(MOB_EV_PATH_BLOCKED);
                    } else {
                        //All good. Head to the next stop.
                        PathStop* nextStop =
                            pathInfo->path[pathInfo->curPathStopIdx];
                        float nextStopZ = z;
                        if(
                            (
                                pathInfo->settings.flags &
                                PATH_FOLLOW_FLAG_AIRBORNE
                            ) &&
                            nextStop->sectorPtr
                        ) {
                            nextStopZ =
                                nextStop->sectorPtr->z +
                                PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT;
                        }
                        
                        chase(
                            nextStop->pos, nextStopZ,
                            CHASE_FLAG_ANY_ANGLE,
                            PATHS::DEF_CHASE_TARGET_DISTANCE,
                            chaseInfo.maxSpeed
                        );
                    }
                    
                } else if(
                    pathInfo->curPathStopIdx == pathInfo->path.size()
                ) {
                    //Reached the final stop of the path, but not the goal.
                    //Let's head there.
                    moveToPathEnd(
                        chaseInfo.maxSpeed, chaseInfo.acceleration
                    );
                    
                } else if(
                    pathInfo->curPathStopIdx == pathInfo->path.size() + 1
                ) {
                    //Reached the path's goal.
                    chaseInfo.state = CHASE_STATE_FINISHED;
                    
                }
                
            } else {
                chaseInfo.state = CHASE_STATE_FINISHED;
            }
            
            if(chaseInfo.state == CHASE_STATE_FINISHED) {
                //Reached the final destination.
                fsm.runEvent(MOB_EV_REACHED_DESTINATION);
            }
        }
        
    }
}


/**
 * @brief Code specific for each class.
 * Meant to be overwritten by the child classes.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickClassSpecifics(float deltaT) {
}


/**
 * @brief Performs some logic code for this game frame.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickMiscLogic(float deltaT) {
    if(timeAlive == 0.0f) {
        //This is a convenient spot to signal that the mob is ready.
        //This will only run once, and only after the mob is all set up.
        fsm.runEvent(MOB_EV_ON_READY);
    }
    timeAlive += deltaT;
    
    invulnPeriod.tick(deltaT);
    
    for(size_t s = 0; s < this->statuses.size(); s++) {
        statuses[s].tick(deltaT);
        
        float damageMult = 1.0f;
        auto vulnIt = type->statusVulnerabilities.find(statuses[s].type);
        if(vulnIt != type->statusVulnerabilities.end()) {
            damageMult = vulnIt->second.effectMult;
        }
        
        float healthBefore = health;
        
        if(statuses[s].type->healthChange != 0.0f) {
            setHealth(
                true, false,
                statuses[s].type->healthChange * damageMult * deltaT
            );
        }
        if(statuses[s].type->healthChangeRatio != 0.0f) {
            setHealth(
                true, true,
                statuses[s].type->healthChangeRatio * damageMult * deltaT
            );
        }
        
        if(health <= 0.0f && healthBefore > 0.0f) {
            if(
                type->category->id == MOB_CATEGORY_PIKMIN &&
                statuses[s].fromHazard
            ) {
                game.statistics.pikminHazardDeaths++;
            }
        }
    }
    deleteOldStatusEffects();
    
    for(size_t g = 0; g < particleGenerators.size();) {
        particleGenerators[g].tick(
            deltaT, game.states.gameplay->particles
        );
        if(particleGenerators[g].emission.interval == 0) {
            particleGenerators.erase(particleGenerators.begin() + g);
        } else {
            g++;
        }
    }
    
    if(groundSector->isBottomlessPit) {
        if(heightEffectPivot == LARGE_FLOAT) {
            heightEffectPivot = z;
        }
    }
    
    if(canBlockPaths && health <= 0) {
        setCanBlockPaths(false);
    }
    
    //Health wheel.
    bool shouldShowHealth =
        type->showHealth &&
        !hasFlag(flags, MOB_FLAG_HIDDEN) &&
        health > 0.0f &&
        health < maxHealth;
    if(!healthWheel && shouldShowHealth) {
        healthWheel = new InWorldHealthWheel(this);
    } else if(healthWheel && !shouldShowHealth) {
        healthWheel->startFading();
    }
    
    if(healthWheel) {
        healthWheel->tick(deltaT);
        if(healthWheel->toDelete) {
            delete healthWheel;
            healthWheel = nullptr;
        }
    }
    
    //Fraction numbers.
    float fractionValueNr = 0.0f;
    float fractionReqNr = 0.0f;
    ALLEGRO_COLOR fractionColor = COLOR_BLACK;
    bool shouldShowFraction =
        getFractionNumbersInfo(
            &fractionValueNr, &fractionReqNr, &fractionColor
        );
        
    if(!fraction && shouldShowFraction) {
        fraction = new InWorldFraction(this);
    } else if(fraction && !shouldShowFraction) {
        fraction->startFading();
    }
    
    if(fraction) {
        fraction->tick(deltaT);
        if(shouldShowFraction) {
            //Only update the numbers if we want to show a fraction, i.e.
            //if we actually KNOW the numbers. Otherwise, keep the old data.
            fraction->setColor(fractionColor);
            fraction->setValueNumber(fractionValueNr);
            fraction->setRequirementNumber(fractionReqNr);
        }
        if(fraction->toDelete) {
            delete fraction;
            fraction = nullptr;
        }
    }
    
    //Group stuff.
    if(group && group->members.size()) {
    
        Player* playerIfLeader = nullptr;
        if(type->category->id == MOB_CATEGORY_LEADERS) {
            playerIfLeader = ((Leader*) this)->player;
        }
        
        Group::MODE oldMode = group->mode;
        bool isHolding = !holding.empty();
        bool isFarFromGroup =
            Distance(group->getAverageMemberPos(), pos) >
            MOB::GROUP_SHUFFLE_DIST + (group->radius + radius);
        bool isSwarming =
            playerIfLeader && playerIfLeader->swarmMagnitude != 0.0f;
            
        //Find what mode we're in on this frame.
        if(isSwarming) {
            group->mode = Group::MODE_SWARM;
        } else if(isHolding || isFarFromGroup) {
            group->mode = Group::MODE_FOLLOW_BACK;
        } else {
            group->mode = Group::MODE_SHUFFLE;
        }
        
        //Change things depending on the mode.
        switch(group->mode) {
        case Group::MODE_FOLLOW_BACK: {
    
            //Follow the leader's back.
            group->anchorAngle = angle + TAU / 2.0f;
            Point newAnchorRelPos =
                rotatePoint(
                    Point(radius + MOB::GROUP_SPOT_INTERVAL * 2.0f, 0.0f),
                    group->anchorAngle
                );
            group->anchor = pos + newAnchorRelPos;
            
            al_identity_transform(&group->transform);
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
            
        } case Group::MODE_SHUFFLE: {
    
            //Casually shuffle with the leader, if needed.
            Point mov;
            Point groupMidPoint =
                group->anchor +
                rotatePoint(
                    Point(group->radius, 0.0f),
                    group->anchorAngle
                );
            movePoint(
                groupMidPoint,
                pos,
                type->moveSpeed,
                group->radius + radius + MOB::GROUP_SPOT_INTERVAL * 2.0f,
                &mov,
                nullptr, nullptr, deltaT
            );
            group->anchor += mov * deltaT;
            
            al_identity_transform(&group->transform);
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
            
        } case Group::MODE_SWARM: {
    
            //Swarming.
            group->anchorAngle = playerIfLeader->swarmAngle;
            Point newAnchorRelPos =
                rotatePoint(
                    Point(radius + MOB::GROUP_SPOT_INTERVAL * 2.0f, 0.0f),
                    group->anchorAngle
                );
            group->anchor = pos + newAnchorRelPos;
            
            float intensityDist =
                game.config.rules.cursorMaxDist *
                playerIfLeader->swarmMagnitude;
            al_identity_transform(&group->transform);
            al_translate_transform(
                &group->transform, -MOB::SWARM_MARGIN, 0
            );
            al_scale_transform(
                &group->transform,
                intensityDist / (group->radius * 2),
                1 -
                (
                    MOB::SWARM_VERTICAL_SCALE*
                    playerIfLeader->swarmMagnitude
                )
            );
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
        }
        }
        
        if(
            oldMode != Group::MODE_SHUFFLE &&
            group->mode == Group::MODE_SHUFFLE
        ) {
            //Started shuffling. Since it's a "casual" formation, we should
            //reassign the spots so Pikmin don't have to keep their order from
            //before.
            group->reassignSpots();
        }
    }
    
    //Damage squash stuff.
    if(damageSquashTime > 0.0f) {
        damageSquashTime -= deltaT;
        damageSquashTime = std::max(0.0f, damageSquashTime);
    }
    
    //Delivery stuff.
    if(
        deliveryInfo &&
        fsm.curState->id == ENEMY_EXTRA_STATE_BEING_DELIVERED
    ) {
        deliveryInfo->animTimeRatioLeft = scriptTimer.getRatioLeft();
    }
}


/**
 * @brief Checks general events in the mob's script for this frame.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Mob::tickScript(float deltaT) {
    if(!fsm.curState) return;
    
    //Timer events.
    MobEvent* timerEv = fsm.getEvent(MOB_EV_TIMER);
    if(scriptTimer.duration > 0) {
        if(scriptTimer.timeLeft > 0) {
            scriptTimer.tick(deltaT);
            if(scriptTimer.timeLeft == 0.0f && timerEv) {
                timerEv->run(this);
            }
        }
    }
    
    //Has it reached 0 health?
    if(health <= 0 && maxHealth != 0) {
        fsm.runEvent(MOB_EV_ZERO_HEALTH, this);
    }
    
    //Check the focused mob.
    if(focusedMob) {
    
        if(focusedMob->health <= 0) {
            fsm.runEvent(MOB_EV_FOCUS_DIED);
            fsm.runEvent(MOB_EV_FOCUS_OFF_REACH);
        }
        
        //We have to recheck if the focused mob is not nullptr, because
        //sending MOB_EV_FOCUS_DIED could've set this to nullptr.
        if(focusedMob) {
        
            Mob* focus = focusedMob;
            MobEvent* forEv = fsm.getEvent(MOB_EV_FOCUS_OFF_REACH);
            
            if(farReach != INVALID && forEv) {
                float angleToFocus = getAngle(pos, focus->pos);
                if(
                    !isMobInReach(
                        &type->reaches[farReach],
                        getDistanceBetween(focusedMob),
                        getAngleSmallestDiff(angle, angleToFocus)
                    )
                ) {
                    forEv->run(this);
                }
                
            }
        }
        
    }
    
    //Itch event.
    if(type->itchDamage > 0 || type->itchTime > 0) {
        itchTime += deltaT;
        MobEvent* itchEv = fsm.getEvent(MOB_EV_ITCH);
        if(
            itchEv &&
            itchDamage > type->itchDamage && itchTime > type->itchTime
        ) {
            itchEv->run(this);
            itchDamage = 0;
            itchTime = 0;
        }
    }
    
    //Health regeneration.
    if(health > 0) {
        setHealth(true, false, type->healthRegen * deltaT);
    }
    
    //Check if it got whistled.
    for(const Player &player : game.states.gameplay->players) {
        if(!player.leaderPtr) continue;
        if(!player.whistle.whistling) continue;
        if(Distance(pos, player.whistle.center) > player.whistle.radius) {
            continue;
        }
        
        fsm.runEvent(MOB_EV_WHISTLED, (void*) player.leaderPtr);
        
        bool savedByWhistle = false;
        for(size_t s = 0; s < statuses.size(); s++) {
            if(statuses[s].type->removableWithWhistle) {
                statuses[s].toDelete = true;
                if(
                    statuses[s].type->healthChange < 0.0f ||
                    statuses[s].type->healthChangeRatio < 0.0f
                ) {
                    savedByWhistle = true;
                }
            }
        }
        deleteOldStatusEffects();
        
        if(savedByWhistle && type->category->id == MOB_CATEGORY_PIKMIN) {
            game.statistics.pikminSaved++;
        }
    }
    
    //Following a leader.
    if(followingGroup) {
        MobEvent* spotFarEv =  fsm.getEvent(MOB_EV_SPOT_IS_FAR);
        
        if(spotFarEv) {
            Point targetPos;
            float targetDist;
            
            getGroupSpotInfo(&targetPos, &targetDist);
            
            Distance d(pos, targetPos);
            if(d > targetDist) {
                spotFarEv->run(this, (void*) &targetPos);
            }
        }
    }
    
    //Far away from home.
    MobEvent* farFromHomeEv = fsm.getEvent(MOB_EV_FAR_FROM_HOME);
    if(farFromHomeEv) {
        Distance d(pos, home);
        if(d >= type->territoryRadius) {
            farFromHomeEv->run(this);
        }
    }
    
    //Tick event.
    fsm.runEvent(MOB_EV_ON_TICK);
}


/**
 * @brief Ticks one frame's worth of time while the mob is riding on
 * a track mob. This updates the mob's position and riding progress.
 *
 * @return Whether the ride is over.
 */
bool Mob::tickTrackRide() {
    trackInfo->curCpProgress +=
        trackInfo->rideSpeed * game.deltaT;
        
    if(trackInfo->curCpProgress >= 1.0f) {
        //Next checkpoint.
        trackInfo->curCpIdx++;
        trackInfo->curCpProgress -= 1.0f;
        
        if(
            trackInfo->curCpIdx ==
            trackInfo->checkpoints.size() - 1
        ) {
            stopTrackRide();
            return true;
        }
    }
    
    //Teleport to the right spot.
    Hitbox* curCp =
        trackInfo->m->getHitbox(
            trackInfo->checkpoints[trackInfo->curCpIdx]
        );
    Hitbox* nextCp =
        trackInfo->m->getHitbox(
            trackInfo->checkpoints[trackInfo->curCpIdx + 1]
        );
    Point curCpPos =
        curCp->getCurPos(trackInfo->m->pos, trackInfo->m->angle);
    Point nextCpPos =
        nextCp->getCurPos(trackInfo->m->pos, trackInfo->m->angle);
        
    Point destXy(
        interpolateNumber(
            trackInfo->curCpProgress, 0.0f, 1.0f,
            curCpPos.x, nextCpPos.x
        ),
        interpolateNumber(
            trackInfo->curCpProgress, 0.0f, 1.0f,
            curCpPos.y, nextCpPos.y
        )
    );
    
    float destZ =
        interpolateNumber(
            trackInfo->curCpProgress, 0.0f, 1.0f,
            trackInfo->m->z + curCp->z,
            trackInfo->m->z + nextCp->z
        );
        
    float destAngle = getAngle(curCpPos, nextCpPos);
    
    chase(destXy, destZ, CHASE_FLAG_TELEPORT);
    face(destAngle, nullptr);
    
    return false;
}


/**
 * @brief Makes the mob lose focus on its currently focused mob.
 */
void Mob::unfocusFromMob() {
    focusedMob = nullptr;
}


/**
 * @brief Returns the index of an animation, given a base animation index and
 * group index.
 *
 * @param baseAnimIdx Base animation index.
 * @param groupIdx Group it belongs to.
 * @param baseAnimTotal Total index of base animations.
 * @return The index.
 */
size_t MobWithAnimGroups::getAnimationIdxFromBaseAndGroup(
    size_t baseAnimIdx, size_t groupIdx,
    size_t baseAnimTotal
) const {
    return groupIdx * baseAnimTotal + baseAnimIdx;
}
