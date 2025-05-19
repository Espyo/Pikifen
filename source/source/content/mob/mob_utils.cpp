/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob utility classes and functions.
 */

#include <algorithm>
#include <unordered_set>

#include "mob_utils.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../other/mob_script_action.h"
#include "mob.h"


using std::unordered_set;
using std::size_t;


/**
 * @brief Constructs a new carrier spot struct object.
 *
 * @param pos The spot's relative coordinates.
 */
CarrierSpot::CarrierSpot(const Point &pos) :
    pos(pos) {
    
}


/**
 * @brief Constructs a new carry info struct object.
 *
 * @param m The mob this info belongs to.
 * @param destination Where to deliver the mob.
 */
CarryInfo::CarryInfo(
    Mob* m, const CARRY_DESTINATION destination
) :
    m(m),
    destination(destination) {
    
    for(size_t c = 0; c < m->type->maxCarriers; c++) {
        Point p;
        if(m->type->customCarrySpots.empty()) {
            float angle = TAU / m->type->maxCarriers * c;
            p =
                Point(
                    cos(angle) *
                    (m->radius + game.config.pikmin.standardRadius),
                    sin(angle) *
                    (m->radius + game.config.pikmin.standardRadius)
                );
        } else {
            p = m->type->customCarrySpots[c];
        }
        spotInfo.push_back(CarrierSpot(p));
    }
}


/**
 * @brief Returns true if the carriers can all fly, and thus, the object can
 * be carried through the air.
 *
 * @return Whether it can fly.
 */
bool CarryInfo::canFly() const {
    for(size_t c = 0; c < spotInfo.size(); c++) {
        Mob* carrierPtr = spotInfo[c].pikPtr;
        if(!carrierPtr) continue;
        if(!hasFlag(spotInfo[c].pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            return false;
        }
    }
    return true;
}


/**
 * @brief Returns a list of hazards to which all carrier Pikmin
 * are invulnerable.
 *
 * @return The invulnerabilities.
 */
vector<Hazard*> CarryInfo::getCarrierInvulnerabilities() const {
    //Get all types to save on the amount of hazard checks.
    unordered_set<MobType*> carrierTypes;
    for(size_t c = 0; c < spotInfo.size(); c++) {
        Mob* carrierPtr = spotInfo[c].pikPtr;
        if(!carrierPtr) continue;
        carrierTypes.insert(carrierPtr->type);
    }
    
    return getMobTypeListInvulnerabilities(carrierTypes);
}


/**
 * @brief Returns the player team index number
 * (0 for team 1, 1 for team 2, etc.) of the Pikmin carrying this.
 * If none, it returns INVALID.
 *
 * @return The player team index, or INVALID.
 */
size_t CarryInfo::getPlayerTeamIdx() const {
    for(size_t s = 0; s < spotInfo.size(); s++) {
        if(!spotInfo[s].pikPtr) continue;
        return spotInfo[s].pikPtr->getPlayerTeamIdx();
    }
    return INVALID;
}


/**
 * @brief Returns the speed at which the object should move,
 * given the carrier Pikmin.
 *
 * @return The speed.
 */
float CarryInfo::getSpeed() const {
    if(curNCarriers == 0) {
        return 0;
    }
    
    float maxSpeed = 0;
    
    //Begin by obtaining the average walking speed of the carriers.
    for(size_t s = 0; s < spotInfo.size(); s++) {
        const CarrierSpot* sPtr = &spotInfo[s];
        
        if(sPtr->state != CARRY_SPOT_STATE_USED) continue;
        
        Pikmin* pPtr = (Pikmin*) sPtr->pikPtr;
        maxSpeed += pPtr->getBaseSpeed() * pPtr->getSpeedMultiplier();
    }
    maxSpeed /= curNCarriers;
    
    //If the object has all carriers, the Pikmin move as fast
    //as possible, which looks bad, since they're not jogging,
    //they're carrying. Let's add a penalty for the weight...
    maxSpeed *= (1 - game.config.carrying.speedWeightMult * m->type->weight);
    //...and a global carrying speed penalty.
    maxSpeed *= game.config.carrying.speedMaxMult;
    
    //The closer the mob is to having full carriers,
    //the closer to the max speed we get.
    //The speed goes from carrying.speedBaseMult (0 carriers)
    //to maxSpeed (all carriers).
    return
        maxSpeed * (
            game.config.carrying.speedBaseMult +
            (curNCarriers / (float) spotInfo.size()) *
            (1 - game.config.carrying.speedBaseMult)
        );
}


/**
 * @brief Returns true if no spot is reserved or used. False otherwise.
 *
 * @return Whether it is empty.
 */
bool CarryInfo::isEmpty() const {
    for(size_t s = 0; s < spotInfo.size(); s++) {
        if(spotInfo[s].state != CARRY_SPOT_STATE_FREE) return false;
    }
    return true;
}


/**
 * @brief Returns true if all spots are reserved. False otherwise.
 *
 * @return Whether it is full.
 */
bool CarryInfo::isFull() const {
    for(size_t s = 0; s < spotInfo.size(); s++) {
        if(spotInfo[s].state == CARRY_SPOT_STATE_FREE) return false;
    }
    return true;
}


/**
 * @brief Rotates all points in the struct, making it so spot 0 faces
 * the specified angle away from the mob.
 * This is useful when the first Pikmin is coming, to make the first carry
 * spot be closer to that Pikmin.
 *
 * @param angle Angle to rotate to.
 */
void CarryInfo::rotatePoints(float angle) {
    for(size_t s = 0; s < spotInfo.size(); s++) {
        float sAngle = angle + (TAU / m->type->maxCarriers * s);
        Point p(
            cos(sAngle) *
            (m->radius + game.config.pikmin.standardRadius),
            sin(sAngle) *
            (m->radius + game.config.pikmin.standardRadius)
        );
        spotInfo[s].pos = p;
    }
}


/**
 * @brief Constructs a new circling info struct object.
 *
 * @param m Mob this circling info struct belongs to.
 */
CirclingInfo::CirclingInfo(Mob* m) :
    m(m) {
    
}


/**
 * @brief Constructs a new delivery info struct object.
 */
DeliveryInfo::DeliveryInfo() :
    color(game.config.aestheticGen.carryingColorMove) {
}


/**
 * @brief Constructs a new group info struct object.
 *
 * @param leaderPtr Mob this group info struct belongs to.
 */
Group::Group(Mob* leaderPtr) :
    anchor(leaderPtr->pos),
    transform(game.identityTransform) {
}


/**
 * @brief Sets the standby group member type to the next available one,
 * or nullptr if none.
 *
 * @param moveBackwards If true, go through the list backwards.
 * @return Whether it succeeded.
 */
bool Group::changeStandbyType(bool moveBackwards) {
    return getNextStandbyType(moveBackwards, &curStandbyType);
}


/**
 * @brief Changes to a different standby subgroup type in case there are no more
 * Pikmin of the current one. Or to no type.
 */
void Group::changeStandbyTypeIfNeeded() {
    for(size_t m = 0; m < members.size(); m++) {
        if(members[m]->subgroupTypePtr == curStandbyType) {
            //Never mind, there is a member of this subgroup type.
            return;
        }
    }
    //No members of the current type? Switch to the next.
    changeStandbyType(false);
}


/**
 * @brief Returns how many members of the given type exist in the group.
 *
 * @param type Type to check.
 * @return The amount.
 */
size_t Group::getAmountByType(const MobType* type) const {
    size_t amount = 0;
    for(size_t m = 0; m < members.size(); m++) {
        if(members[m]->type == type) {
            amount++;
        }
    }
    return amount;
}


/**
 * @brief Returns the average position of the members.
 *
 * @return The average position.
 */
Point Group::getAverageMemberPos() const {
    Point avg;
    for(size_t m = 0; m < members.size(); m++) {
        avg += members[m]->pos;
    }
    return avg / members.size();
}


/**
 * @brief Returns a list of hazards to which all of a leader's group mobs
 * are invulnerable.
 *
 * @param includeLeader If not nullptr, include the group leader mob.
 * @return The list of invulnerabilities.
 */
vector<Hazard*> Group::getGroupInvulnerabilities(
    Mob* includeLeader
) const {
    //Get all types to save on the amount of hazard checks.
    unordered_set<MobType*> memberTypes;
    for(size_t m = 0; m < members.size(); m++) {
        Mob* memberPtr = members[m];
        if(!memberPtr) continue;
        memberTypes.insert(memberPtr->type);
    }
    
    if(includeLeader) memberTypes.insert(includeLeader->type);
    
    return getMobTypeListInvulnerabilities(memberTypes);
}


/**
 * @brief Returns the next available standby group member type, or nullptr if none.
 *
 * @param moveBackwards If true, go through the list backwards.
 * @param newType The new type is returned here.
 * @return Whether it succeeded.
 */
bool Group::getNextStandbyType(
    bool moveBackwards, SubgroupType** newType
) {

    if(members.empty()) {
        *newType = nullptr;
        return true;
    }
    
    bool success = false;
    SubgroupType* startingType = curStandbyType;
    SubgroupType* finalType = curStandbyType;
    if(!startingType) {
        startingType =
            game.states.gameplay->subgroupTypes.getFirstType();
    }
    SubgroupType* scanningType = startingType;
    SubgroupType* leaderSubgroupType =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
        
    if(moveBackwards) {
        scanningType =
            game.states.gameplay->subgroupTypes.getPrevType(
                scanningType
            );
    } else {
        scanningType =
            game.states.gameplay->subgroupTypes.getNextType(
                scanningType
            );
    }
    while(scanningType != startingType && !success) {
        //For each type, let's check if there's any group member that matches.
        if(
            scanningType == leaderSubgroupType &&
            !game.config.rules.canThrowLeaders
        ) {
            //If this is a leader, and leaders cannot be thrown, skip.
        } else {
            for(size_t m = 0; m < members.size(); m++) {
                if(members[m]->subgroupTypePtr == scanningType) {
                    finalType = scanningType;
                    success = true;
                    break;
                }
            }
        }
        
        if(moveBackwards) {
            scanningType =
                game.states.gameplay->subgroupTypes.getPrevType(
                    scanningType
                );
        } else {
            scanningType =
                game.states.gameplay->subgroupTypes.getNextType(
                    scanningType
                );
        }
    }
    
    *newType = finalType;
    return success;
}


/**
 * @brief Returns a point's offset from the anchor,
 * given the current group transformation.
 *
 * @param spotIdx Index of the spot to check.
 * @return The offset.
 */
Point Group::getSpotOffset(size_t spotIdx) const {
    Point res = spots[spotIdx].pos;
    al_transform_coordinates(&transform, &res.x, &res.y);
    return res;
}


/**
 * @brief (Re-)Initializes the group spots. This resizes it to the current
 * number of group members. Any old group members are moved to the appropriate
 * new spot.
 *
 * @param affectedMobPtr If this initialization is because a new mob entered
 * or left the group, this should point to said mob.
 */
void Group::initSpots(Mob* affectedMobPtr) {
    if(members.empty()) {
        spots.clear();
        radius = 0;
        return;
    }
    
    //First, backup the old mob indexes.
    vector<Mob*> oldMobs;
    oldMobs.resize(spots.size());
    for(size_t m = 0; m < spots.size(); m++) {
        oldMobs[m] = spots[m].mobPtr;
    }
    
    //Now, rebuild the spots. Let's draw wheels from the center, for now.
    
    /**
     * @brief Initial spot.
     */
    struct AlphaSpot {
    
        //--- Members ---
        
        //Position of the spot.
        Point pos;
        
        //How far away it is from the rightmost spot.
        Distance distanceToRightmost;
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new alpha spot object.
         *
         * @param p The position.
         */
        explicit AlphaSpot(const Point &p) :
            pos(p) { }
            
    };
    
    vector<AlphaSpot> alphaSpots;
    size_t currentWheel = 1;
    radius = game.config.pikmin.standardRadius;
    
    //Center spot first.
    alphaSpots.push_back(AlphaSpot(Point()));
    
    while(alphaSpots.size() < members.size()) {
    
        //First, calculate how far the center
        //of these spots are from the central spot.
        float distFromCenter =
            game.config.pikmin.standardRadius * currentWheel + //Spots.
            MOB::GROUP_SPOT_INTERVAL * currentWheel; //Interval between spots.
            
        /* Now we need to figure out what's the angular distance
         * between each spot. For that, we need the actual diameter
         * (distance from one point to the other),
         * and the central distance, which is distance between the center
         * and the middle of two spots.
         *
         * We can get the middle distance because we know the actual diameter,
         * which should be the size of a Pikmin and one interval unit,
         * and we know the distance from one spot to the center.
         */
        float actualDiameter =
            game.config.pikmin.standardRadius * 2.0 + MOB::GROUP_SPOT_INTERVAL;
            
        //Just calculate the remaining side of the triangle, now that we know
        //the hypotenuse and the actual diameter (one side of the triangle).
        float middleDistance =
            sqrt(
                (distFromCenter * distFromCenter) -
                (actualDiameter * 0.5 * actualDiameter * 0.5)
            );
            
        //Now, get the angular distance.
        float angularDist =
            atan2(actualDiameter, middleDistance * 2.0f) * 2.0;
            
        //Finally, we can calculate where the other spots are.
        size_t nSpotsOnWheel = floor(TAU / angularDist);
        //Get a better angle. One that can evenly distribute the spots.
        float angle = TAU / nSpotsOnWheel;
        
        for(unsigned s = 0; s < nSpotsOnWheel; s++) {
            alphaSpots.push_back(
                AlphaSpot(
                    Point(
                        distFromCenter * cos(angle * s) +
                        game.rng.f(
                            -MOB::GROUP_SPOT_MAX_DEVIATION,
                            MOB::GROUP_SPOT_MAX_DEVIATION
                        ),
                        distFromCenter * sin(angle * s) +
                        game.rng.f(
                            -MOB::GROUP_SPOT_MAX_DEVIATION,
                            MOB::GROUP_SPOT_MAX_DEVIATION
                        )
                    )
                )
            );
        }
        
        currentWheel++;
        radius = distFromCenter;
    }
    
    //Now, given all of these points, create our final spot vector,
    //with the rightmost points coming first.
    
    //Start by sorting the points.
    for(size_t a = 0; a < alphaSpots.size(); a++) {
        alphaSpots[a].distanceToRightmost =
            Distance(
                alphaSpots[a].pos,
                Point(radius, 0)
            );
    }
    
    std::sort(
        alphaSpots.begin(), alphaSpots.end(),
    [] (const AlphaSpot & a1, const AlphaSpot & a2) -> bool {
        return a1.distanceToRightmost < a2.distanceToRightmost;
    }
    );
    
    //Finally, create the group spots.
    spots.clear();
    spots.resize(members.size(), GroupSpot());
    for(size_t s = 0; s < members.size(); s++) {
        spots[s] =
            GroupSpot(
                Point(
                    alphaSpots[s].pos.x - radius,
                    alphaSpots[s].pos.y
                ),
                nullptr
            );
    }
    
    //Pass the old mobs over.
    if(oldMobs.size() < spots.size()) {
        for(size_t m = 0; m < oldMobs.size(); m++) {
            spots[m].mobPtr = oldMobs[m];
            spots[m].mobPtr->groupSpotIdx = m;
        }
        spots[oldMobs.size()].mobPtr = affectedMobPtr;
        affectedMobPtr->groupSpotIdx = oldMobs.size();
        
    } else if(oldMobs.size() > spots.size()) {
        for(size_t m = 0, s = 0; m < oldMobs.size(); m++) {
            if(oldMobs[m] == affectedMobPtr) {
                oldMobs[m]->groupSpotIdx = INVALID;
                continue;
            }
            spots[s].mobPtr = oldMobs[m];
            spots[s].mobPtr->groupSpotIdx = s;
            s++;
        }
        
    } else {
        for(size_t m = 0; m < oldMobs.size(); m++) {
            spots[m].mobPtr = oldMobs[m];
            spots[m].mobPtr->groupSpotIdx = m;
        }
    }
}


/**
 * @brief Assigns each mob a new spot, given how close each one of them is to
 * each spot.
 */
void Group::reassignSpots() {
    for(size_t m = 0; m < members.size(); m++) {
        members[m]->groupSpotIdx = INVALID;
    }
    
    for(size_t s = 0; s < spots.size(); s++) {
        Point spotPos = anchor + getSpotOffset(s);
        Mob* closestMob = nullptr;
        Distance closestDist;
        
        for(size_t m = 0; m < members.size(); m++) {
            Mob* mPtr = members[m];
            if(mPtr->groupSpotIdx != INVALID) continue;
            
            Distance d(mPtr->pos, spotPos);
            
            if(!closestMob || d < closestDist) {
                closestMob = mPtr;
                closestDist = d;
            }
        }
        
        if(closestMob) closestMob->groupSpotIdx = s;
    }
}


/**
 * @brief Sorts the group with the specified type at the front, and the
 * other types (in order) behind.
 *
 * @param leadingType The subgroup type that will be at the front of
 * the group.
 */
void Group::sort(SubgroupType* leadingType) {
    for(size_t m = 0; m < members.size(); m++) {
        members[m]->groupSpotIdx = INVALID;
    }
    
    SubgroupType* curType = leadingType;
    size_t curSpot = 0;
    
    while(curSpot != spots.size()) {
        Point spotPos = anchor + getSpotOffset(curSpot);
        
        //Find the member closest to this spot.
        Mob* closestMember = nullptr;
        Distance closestDist;
        for(size_t m = 0; m < members.size(); m++) {
            Mob* mPtr = members[m];
            if(mPtr->subgroupTypePtr != curType) continue;
            if(mPtr->groupSpotIdx != INVALID) continue;
            
            Distance d(mPtr->pos, spotPos);
            
            if(!closestMember || d < closestDist) {
                closestMember = mPtr;
                closestDist = d;
            }
            
        }
        
        if(!closestMember) {
            //There are no more members of the current type left!
            //Next type.
            curType =
                game.states.gameplay->subgroupTypes.getNextType(curType);
        } else {
            spots[curSpot].mobPtr = closestMember;
            closestMember->groupSpotIdx = curSpot;
            curSpot++;
        }
        
    }
    
}


/**
 * @brief Clears the information.
 */
void HoldInfo::clear() {
    m = nullptr;
    hitboxIdx = INVALID;
    offsetDist = 0;
    offsetAngle = 0;
    verticalDist = 0;
}


/**
 * @brief Returns the final coordinates this mob should be at.
 *
 * @param outZ The Z coordinate is returned here.
 * @return The (X and Y) coordinates.
 */
Point HoldInfo::getFinalPos(float* outZ) const {
    if(!m) return Point();
    
    Hitbox* hPtr = nullptr;
    if(hitboxIdx != INVALID) {
        hPtr = m->getHitbox(hitboxIdx);
    }
    
    Point finalPos;
    
    if(hPtr) {
        //Hitbox.
        finalPos = rotatePoint(hPtr->pos, m->angle);
        finalPos += m->pos;
        
        finalPos +=
            angleToCoordinates(
                offsetAngle + m->angle,
                offsetDist * hPtr->radius
            );
        *outZ = m->z + hPtr->z + (hPtr->height * verticalDist);
    } else {
        //Body center.
        finalPos = m->pos;
        
        finalPos +=
            angleToCoordinates(
                offsetAngle + m->angle,
                offsetDist * m->radius
            );
        *outZ = m->z + (m->height * verticalDist);
    }
    
    return finalPos;
}


/**
 * @brief Constructs a new parent info struct object.
 *
 * @param m The parent mob.
 */
Parent::Parent(Mob* m) :
    m(m) {
    
}


/**
 * @brief Constructs a new path info struct object.
 *
 * @param m Mob this path info struct belongs to.
 * @param settings Settings about how the path should be followed.
 */
Path::Path(
    Mob* m,
    const PathFollowSettings &settings
) :
    m(m),
    settings(settings) {
    
    result =
        getPath(
            m->pos, settings.targetPoint, settings,
            path, nullptr, nullptr, nullptr
        );
}


/**
 * @brief Calculates whether or not the way forward is currently blocked.
 *
 * @param outReason If not nullptr, the reason is returned here.
 * @return Whether there is a blockage.
 */
bool Path::checkBlockage(PATH_BLOCK_REASON* outReason) {
    if(
        path.size() >= 2 &&
        curPathStopIdx > 0 &&
        curPathStopIdx < path.size()
    ) {
        PathStop* curStop = path[curPathStopIdx - 1];
        PathStop* nextStop = path[curPathStopIdx];
        
        return
            !canTraversePathLink(
                curStop->getLink(nextStop),
                settings,
                outReason
            );
    }
    
    if(outReason) *outReason = PATH_BLOCK_REASON_NONE;
    return false;
}


/**
 * @brief Constructs a new Pikmin nest struct object.
 *
 * @param mPtr Nest mob responsible.
 * @param type Type of nest.
 */
PikminNest::PikminNest(
    Mob* mPtr, PikminNestType* type
) :
    mPtr(mPtr),
    nestType(type) {
    
    for(size_t t = 0; t < nestType->pikTypes.size(); t++) {
        pikminInside.push_back(vector<size_t>(N_MATURITIES, 0));
        callQueue.push_back(0);
    }
}


/**
 * @brief Calls out a Pikmin from inside the nest, if possible.
 * Gives priority to the higher maturities.
 *
 * @param mPtr Pointer to the nest mob.
 * @param typeIdx Index of the Pikmin type, from the types this nest manages.
 * @return Whether a Pikmin spawned.
 */
bool PikminNest::callPikmin(Mob* mPtr, size_t typeIdx) {
    if(
        game.states.gameplay->mobs.pikmin.size() >=
        game.config.rules.maxPikminInField
    ) {
        return false;
    }
    
    for(size_t m = 0; m < N_MATURITIES; m++) {
        //Let's check the maturities in reverse order.
        size_t curM = N_MATURITIES - m - 1;
        
        if(pikminInside[typeIdx][curM] == 0) continue;
        
        //Spawn the Pikmin!
        //Update the Pikmin count.
        pikminInside[typeIdx][curM]--;
        
        //Decide a leg to come out of.
        size_t legIdx =
            game.rng.i(0, (int) (nestType->legBodyParts.size() / 2) - 1);
        size_t legHoleBPIdx =
            mPtr->anim.animDb->findBodyPart(
                nestType->legBodyParts[legIdx * 2]
            );
        size_t legFootBPIdx =
            mPtr->anim.animDb->findBodyPart(
                nestType->legBodyParts[legIdx * 2 + 1]
            );
        Point spawnCoords =
            mPtr->getHitbox(legHoleBPIdx)->getCurPos(
                mPtr->pos, mPtr->angle
            );
        float spawnAngle =
            getAngle(mPtr->pos, spawnCoords);
            
        //Create the Pikmin.
        Pikmin* newPikmin =
            (Pikmin*)
            createMob(
                game.mobCategories.get(MOB_CATEGORY_PIKMIN),
                spawnCoords, nestType->pikTypes[typeIdx], spawnAngle,
                "maturity=" + i2s(curM)
            );
            
        //Set its data to start sliding.
        newPikmin->fsm.setState(PIKMIN_STATE_LEAVING_ONION, (void*) this);
        vector<size_t> checkpoints;
        checkpoints.push_back(legHoleBPIdx);
        checkpoints.push_back(legFootBPIdx);
        newPikmin->trackInfo =
            new TrackRideInfo(
            mPtr, checkpoints, nestType->pikminExitSpeed
        );
        newPikmin->leaderToReturnTo = callingLeader;
        
        return true;
    }
    
    return false;
}


/**
 * @brief Returns how many Pikmin of the given type exist inside.
 *
 * @param type Type to check.
 * @return The amount.
 */
size_t PikminNest::getAmountByType(const PikminType* type) {
    size_t amount = 0;
    for(size_t t = 0; t < nestType->pikTypes.size(); t++) {
        if(nestType->pikTypes[t] == type) {
            for(size_t m = 0; m < N_MATURITIES; m++) {
                amount += pikminInside[t][m];
            }
            break;
        }
    }
    return amount;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with
 * any that are related to nests.
 *
 * @param svr Script var reader to use.
 */
void PikminNest::readScriptVars(const ScriptVarReader &svr) {
    string pikminInsideVar;
    
    if(svr.get("pikmin_inside", pikminInsideVar)) {
        vector<string> pikminInsideVars = split(pikminInsideVar);
        size_t word = 0;
        
        for(size_t t = 0; t < nestType->pikTypes.size(); t++) {
            for(size_t m = 0; m < N_MATURITIES; m++) {
                if(word < pikminInsideVars.size()) {
                    pikminInside[t][m] = s2i(pikminInsideVars[word]);
                    word++;
                }
            }
        }
    }
}


/**
 * @brief Requests that Pikmin of the given type get called out.
 *
 * @param typeIdx Index of the type of Pikmin to call out, from the
 * nest's types.
 * @param amount How many to call out.
 * @param lPtr Leader responsible.
 */
void PikminNest::requestPikmin(
    size_t typeIdx, size_t amount, Leader* lPtr
) {
    callQueue[typeIdx] += amount;
    nextCallTime = MOB::PIKMIN_NEST_CALL_INTERVAL;
    callingLeader = lPtr;
}


/**
 * @brief Stores the given Pikmin inside the nest. This basically deletes the
 * Pikmin and updates the amount inside the nest.
 *
 * @param pPtr Pikmin to store.
 */
void PikminNest::storePikmin(Pikmin* pPtr) {
    for(size_t t = 0; t < nestType->pikTypes.size(); t++) {
        if(pPtr->type == nestType->pikTypes[t]) {
            pikminInside[t][pPtr->maturity]++;
            break;
        }
    }
    
    pPtr->toDelete = true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void PikminNest::tick(float deltaT) {
    if(callingLeader && callingLeader->toDelete) {
        callingLeader = nullptr;
    }
    
    //Call out Pikmin, if the timer agrees.
    if(nextCallTime > 0.0f) {
        nextCallTime -= deltaT;
    }
    
    while(nextCallTime < 0.0f) {
        size_t bestType = INVALID;
        size_t bestTypeAmount = 0;
        
        for(size_t t = 0; t < nestType->pikTypes.size(); t++) {
            if(callQueue[t] == 0) continue;
            if(callQueue[t] > bestTypeAmount) {
                bestType = t;
                bestTypeAmount = callQueue[t];
            }
        }
        
        if(bestType != INVALID) {
            //Try to call a Pikmin.
            if(callPikmin(mPtr, bestType)) {
                //Call successful! Update the queue.
                callQueue[bestType]--;
            } else {
                //Call failed. Forget the player's request.
                callQueue[bestType] = 0;
            }
        }
        
        nextCallTime += MOB::PIKMIN_NEST_CALL_INTERVAL;
    }
}


/**
 * @brief Loads nest-related properties from a data file.
 *
 * @param file File to read from.
 */
void PikminNestType::loadProperties(DataNode* file) {
    ReaderSetter nRS(file);
    
    string pikTypesStr;
    string legsStr;
    DataNode* pikTypesNode = nullptr;
    DataNode* legsNode = nullptr;
    
    nRS.set("leg_body_parts", legsStr, &legsNode);
    nRS.set("pikmin_types", pikTypesStr, &pikTypesNode);
    nRS.set("pikmin_enter_speed", pikminEnterSpeed);
    nRS.set("pikmin_exit_speed", pikminExitSpeed);
    
    legBodyParts = semicolonListToVector(legsStr);
    if(pikTypesNode && legBodyParts.empty()) {
        game.errors.report(
            "A nest-like object type needs a list of leg body parts!",
            file
        );
    } else if(legsNode && legBodyParts.size() % 2 == 1) {
        game.errors.report(
            "A nest-like object type needs an even number of leg body parts!",
            legsNode
        );
    }
    
    vector<string> pikTypesStrs = semicolonListToVector(pikTypesStr);
    for(size_t t = 0; t < pikTypesStrs.size(); t++) {
        string &str = pikTypesStrs[t];
        if(!isInMap(game.content.mobTypes.list.pikmin, str)) {
            game.errors.report(
                "Unknown Pikmin type \"" + str + "\"!",
                pikTypesNode
            );
        } else {
            pikTypes.push_back(game.content.mobTypes.list.pikmin[str]);
        }
    }
}


/**
 * @brief Constructs a new track ride info struct object.
 *
 * @param m Mob this track info struct belongs to.
 * @param checkpoints List of checkpoints (body part indexes) to cross.
 * @param rideSpeed Speed to ride at, in ratio per second.
 */
TrackRideInfo::TrackRideInfo(
    Mob* m, const vector<size_t> &checkpoints, float rideSpeed
) :
    m(m),
    checkpoints(checkpoints),
    rideSpeed(rideSpeed) {
    
}


/**
 * @brief Calculates the maximum physical span that a mob can ever reach
 * from its center.
 *
 * @param radius The mob's radius.
 * @param animHitboxSpan Maximum span of its hitboxes data.
 * @param rectangularDim Rectangular dimensions of the mob, if any.
 * @return The span.
 */
float calculateMobPhysicalSpan(
    float radius, float animHitboxSpan,
    const Point &rectangularDim
) {
    float finalSpan = std::max(radius, animHitboxSpan);
    
    if(rectangularDim.x != 0) {
        finalSpan =
            std::max(
                finalSpan, Distance(Point(0.0f), rectangularDim / 2.0).toFloat()
            );
    }
    
    return finalSpan;
}


/**
 * @brief Creates a mob, adding it to the corresponding vectors.
 *
 * @param category The category the new mob belongs to.
 * @param pos Initial position.
 * @param type Type of the new mob.
 * @param angle Initial facing angle.
 * @param vars Script variables.
 * @param codeAfterCreation Code to run right after the mob is created,
 * if any. This is run before any scripting takes place.
 * @param firstStateOverride If this is INVALID, use the first state
 * index defined in the mob's FSM struct, or the standard first state index.
 * Otherwise, use this.
 * @return The new mob.
 */
Mob* createMob(
    MobCategory* category, const Point &pos, MobType* type,
    float angle, const string &vars,
    std::function<void(Mob*)> codeAfterCreation,
    size_t firstStateOverride
) {
    Mob* mPtr = category->createMob(pos, type, angle);
    
    if(mPtr->type->walkable) {
        game.states.gameplay->mobs.walkables.push_back(mPtr);
    }
    
    if(codeAfterCreation) {
        codeAfterCreation(mPtr);
    }
    
    for(size_t a = 0; a < type->initActions.size(); a++) {
        type->initActions[a]->run(mPtr, nullptr, nullptr);
    }
    
    if(!vars.empty()) {
        map<string, string> varsMap = getVarMap(vars);
        ScriptVarReader svr(varsMap);
        
        mPtr->readScriptVars(svr);
        
        for(auto &v : varsMap) {
            mPtr->vars[v.first] = v.second;
        }
    }
    
    if(
        !mPtr->fsm.setState(
            firstStateOverride != INVALID ?
            firstStateOverride :
            mPtr->fsm.firstStateOverride != INVALID ?
            mPtr->fsm.firstStateOverride :
            type->firstStateIdx
        )
    ) {
        //If something went wrong, give it some dummy state.
        mPtr->fsm.curState = game.dummyMobState;
    };
    
    for(size_t c = 0; c < type->children.size(); c++) {
        MobType::Child* childInfo =
            &type->children[c];
        MobType::SpawnInfo* spawnInfo =
            getSpawnInfoFromChildInfo(mPtr->type, &type->children[c]);
            
        if(!spawnInfo) {
            game.errors.report(
                "Object \"" + type->name + "\" tried to spawn a child with the "
                "spawn name \"" + childInfo->spawnName + "\", but that name "
                "does not exist in the list of spawn data!"
            );
            continue;
        }
        
        Mob* newMob = mPtr->spawn(spawnInfo);
        
        if(!newMob) continue;
        
        Parent* pInfo = new Parent(mPtr);
        newMob->parent = pInfo;
        pInfo->handleDamage = childInfo->handleDamage;
        pInfo->relayDamage = childInfo->relayDamage;
        pInfo->handleEvents = childInfo->handleEvents;
        pInfo->relayEvents = childInfo->relayEvents;
        pInfo->handleStatuses = childInfo->handleStatuses;
        pInfo->relayStatuses = childInfo->relayStatuses;
        if(!childInfo->limbAnimName.empty()) {
            pInfo->limbAnim.animDb = mPtr->anim.animDb;
            Animation* animToUse = nullptr;
            for(size_t a = 0; a < mPtr->anim.animDb->animations.size(); a++) {
                if(
                    mPtr->anim.animDb->animations[a]->name ==
                    childInfo->limbAnimName
                ) {
                    animToUse = mPtr->anim.animDb->animations[a];
                }
            }
            
            if(animToUse) {
                pInfo->limbAnim.curAnim = animToUse;
                pInfo->limbAnim.toStart();
            } else {
                game.errors.report(
                    "Object \"" + newMob->type->name + "\", child object of "
                    "object \"" + type->name + "\", tried to use animation \"" +
                    childInfo->limbAnimName + "\" for a limb, but that "
                    "animation doesn't exist in the parent object's animations!"
                );
            }
        }
        pInfo->limbThickness = childInfo->limbThickness;
        pInfo->limbParentBodyPart =
            type->animDb->findBodyPart(childInfo->limbParentBodyPart);
        pInfo->limbParentOffset = childInfo->limbParentOffset;
        pInfo->limbChildBodyPart =
            newMob->type->animDb->findBodyPart(
                childInfo->limbChildBodyPart
            );
        pInfo->limbChildOffset = childInfo->limbChildOffset;
        pInfo->limbDrawMethod = childInfo->limbDrawMethod;
        
        if(childInfo->parentHolds) {
            mPtr->hold(
                newMob,
                type->animDb->findBodyPart(childInfo->holdBodyPart),
                childInfo->holdOffsetDist,
                childInfo->holdOffsetAngle,
                childInfo->holdOffsetVertDist,
                false,
                childInfo->holdRotationMethod
            );
        }
    }
    
    game.states.gameplay->mobs.all.push_back(mPtr);
    return mPtr;
}


/**
 * @brief Deletes a mob from the relevant vectors.
 *
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 *
 * @param mPtr The mob to delete.
 * @param completeDestruction If true, don't bother removing it from groups
 * and such, since everything is going to be destroyed.
 */
void deleteMob(Mob* mPtr, bool completeDestruction) {
    if(game.makerTools.infoLock == mPtr) game.makerTools.infoLock = nullptr;
    
    if(!completeDestruction) {
        mPtr->leaveGroup();
        
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m];
            if(m2Ptr->focusedMob == mPtr) {
                m2Ptr->fsm.runEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
                m2Ptr->fsm.runEvent(MOB_EV_FOCUS_OFF_REACH);
                m2Ptr->fsm.runEvent(MOB_EV_FOCUS_DIED);
                m2Ptr->focusedMob = nullptr;
            }
            if(m2Ptr->parent && m2Ptr->parent->m == mPtr) {
                delete m2Ptr->parent;
                m2Ptr->parent = nullptr;
                m2Ptr->toDelete = true;
            }
            for(size_t f = 0; f < m2Ptr->focusedMobMemory.size(); f++) {
                if(m2Ptr->focusedMobMemory[f] == mPtr) {
                    m2Ptr->focusedMobMemory[f] = nullptr;
                }
            }
            for(size_t c = 0; c < m2Ptr->chompingMobs.size(); c++) {
                if(m2Ptr->chompingMobs[c] == mPtr) {
                    m2Ptr->chompingMobs[c] = nullptr;
                }
            }
            for (const auto& [identifier, link] : m2Ptr->links) {
                if(link == mPtr) {
                    m2Ptr->links[identifier] = nullptr;
                }
            }
            if(m2Ptr->storedInsideAnother == mPtr) {
                mPtr->release(m2Ptr);
                m2Ptr->storedInsideAnother = nullptr;
            }
            if(m2Ptr->carryInfo) {
                for(
                    size_t c = 0; c < m2Ptr->carryInfo->spotInfo.size(); c++
                ) {
                    if(m2Ptr->carryInfo->spotInfo[c].pikPtr == mPtr) {
                        m2Ptr->carryInfo->spotInfo[c].pikPtr =
                            nullptr;
                        m2Ptr->carryInfo->spotInfo[c].state =
                            CARRY_SPOT_STATE_FREE;
                    }
                }
            }
        }
        
        if(mPtr->holder.m) {
            mPtr->holder.m->release(mPtr);
        }
        
        while(!mPtr->holding.empty()) {
            mPtr->release(mPtr->holding[0]);
        }
        
        mPtr->setCanBlockPaths(false);
        
        mPtr->fsm.setState(INVALID);
    }
    
    game.audio.handleMobDeletion(mPtr);
    
    mPtr->type->category->eraseMob(mPtr);
    game.states.gameplay->mobs.all.erase(
        find(
            game.states.gameplay->mobs.all.begin(),
            game.states.gameplay->mobs.all.end(),
            mPtr
        )
    );
    if(mPtr->type->walkable) {
        game.states.gameplay->mobs.walkables.erase(
            find(
                game.states.gameplay->mobs.walkables.begin(),
                game.states.gameplay->mobs.walkables.end(),
                mPtr
            )
        );
    }
    
    delete mPtr;
}


/**
 * @brief Returns a string that describes the given mob. Used in error messages
 * where you have to indicate a specific mob in the area.
 *
 * @param m The mob.
 * @return The string.
 */
string getErrorMessageMobInfo(Mob* m) {
    return
        "type \"" + m->type->name + "\", coordinates " +
        p2s(m->pos) + ", area \"" + game.curAreaData->name + "\"";
}


/**
 * @brief Returns a list of hazards to which all mob types given
 * are invulnerable.
 *
 * @param types Mob types to check.
 * @return The invulnerabilities.
 */
vector<Hazard*> getMobTypeListInvulnerabilities(
    const unordered_set<MobType*> &types
) {
    //Count how many types are invulnerable to each detected hazard.
    map<Hazard*, size_t> invInstances;
    for(auto &t : types) {
        for(auto &h : t->hazardVulnerabilities) {
            if(h.second.effectMult == 0.0f) {
                invInstances[h.first]++;
            }
        }
    }
    
    //Only accept those that ALL types are invulnerable to.
    vector<Hazard*> invulnerabilities;
    for(auto &i : invInstances) {
        if(i.second == types.size()) {
            invulnerabilities.push_back(i.first);
        }
    }
    
    return invulnerabilities;
}


/**
 * @brief Given a child info block, returns the spawn info block that matches.
 *
 * @param type Mob type that owns the children and spawn blocks.
 * @param childInfo Child info to check.
 * @return The spawn info, or nullptr if not found.
 */
MobType::SpawnInfo* getSpawnInfoFromChildInfo(
    MobType* type, const MobType::Child* childInfo
) {
    for(size_t s = 0; s < type->spawns.size(); s++) {
        if(type->spawns[s].name == childInfo->spawnName) {
            return &type->spawns[s];
        }
    }
    return nullptr;
}


/**
 * @brief Returns whether a given mob is in reach or out of reach of another,
 * given the positional and reach data.
 *
 * @param reachTPtr Pointer to the reach information.
 * @param distBetween Distance between the two mobs.
 * @param angleDiff Angle difference between the two mobs.
 * @return Whether it's in reach.
 */
bool isMobInReach(
    MobType::Reach* reachTPtr, const Distance &distBetween, float angleDiff
) {
    bool inReach =
        (
            distBetween <= reachTPtr->radius1 &&
            angleDiff <= reachTPtr->angle1 / 2.0
        );
    if(inReach) return true;
    inReach =
        (
            distBetween <= reachTPtr->radius2 &&
            angleDiff <= reachTPtr->angle2 / 2.0
        );
    return inReach;
}


/**
 * @brief Converts a string to the numeric representation of a mob target type.
 *
 * @param typeStr Text representation of the target type.
 * @return The type, or INVALID if invalid.
 */
MOB_TARGET_FLAG stringToMobTargetType(const string &typeStr) {
    if(typeStr == "none") {
        return MOB_TARGET_FLAG_NONE;
    } else if(typeStr == "player") {
        return MOB_TARGET_FLAG_PLAYER;
    } else if(typeStr == "enemy") {
        return MOB_TARGET_FLAG_ENEMY;
    } else if(typeStr == "weak_plain_obstacle") {
        return MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE;
    } else if(typeStr == "strong_plain_obstacle") {
        return MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE;
    } else if(typeStr == "pikmin_obstacle") {
        return MOB_TARGET_FLAG_PIKMIN_OBSTACLE;
    } else if(typeStr == "explodable") {
        return MOB_TARGET_FLAG_EXPLODABLE;
    } else if(typeStr == "explodable_pikmin_obstacle") {
        return MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE;
    } else if(typeStr == "fragile") {
        return MOB_TARGET_FLAG_FRAGILE;
    }
    return (MOB_TARGET_FLAG) INVALID;
}


/**
 * @brief Converts a string to the numeric representation of a team.
 *
 * @param teamStr Text representation of the team.
 * @return The team, or INVALID if invalid.
 */
MOB_TEAM stringToTeamNr(const string &teamStr) {
    for(size_t t = 0; t < N_MOB_TEAMS; t++) {
        if(teamStr == game.teamInternalNames[t]) {
            return (MOB_TEAM) t;
        }
    }
    return (MOB_TEAM) INVALID;
}
