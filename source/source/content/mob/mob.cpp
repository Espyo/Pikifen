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
 * @param new_member The new member to add.
 */
void Mob::addToGroup(Mob* new_member) {
    //If it's already following, never mind.
    if(new_member->followingGroup == this) return;
    if(!group) return;
    
    new_member->followingGroup = this;
    group->members.push_back(new_member);
    
    //Find a spot.
    group->initSpots(new_member);
    new_member->groupSpotIdx = group->spots.size() - 1;
    
    if(!group->curStandbyType) {
        if(
            new_member->type->category->id != MOB_CATEGORY_LEADERS ||
            game.config.rules.canThrowLeaders
        ) {
            group->curStandbyType =
                new_member->subgroupTypePtr;
        }
    }
    
    if(group->members.size() == 1) {
        //If this is the first member, update the anchor position.
        group->anchor = pos;
        group->anchorAngle = TAU / 2.0f;
    }
}


/**
 * @brief Applies the damage caused by an attack from another mob to this one.
 *
 * @param attacker The mob that caused the attack.
 * @param attack_h Hitbox used for the attack.
 * @param victim_h Victim's hitbox that got hit.
 * @param damage Total damage the attack caused.
 */
void Mob::applyAttackDamage(
    Mob* attacker, Hitbox* attack_h, Hitbox* victim_h, float damage
) {
    //Register this hit, so the next frame doesn't hit it too.
    attacker->hitOpponents.push_back(
        std::make_pair(MOB::OPPONENT_HIT_REGISTER_TIMEOUT, this)
    );
    
    //Will the parent mob be handling the damage?
    if(parent && parent->relay_damage) {
        parent->m->applyAttackDamage(attacker, attack_h, victim_h, damage);
        if(!parent->handle_damage) {
            return;
        }
    }
    
    //Perform the damage and script-related events.
    if(damage > 0) {
        setHealth(true, false, -damage);
        
        HitboxInteraction ev_info(this, victim_h, attack_h);
        fsm.runEvent(MOB_EV_DAMAGE, (void*) &ev_info);
        
        attacker->causeSpikeDamage(this, false);
    }
    
    //Final setup.
    itchDamage += damage;
}


/**
 * @brief Applies the knockback values to a mob, caused by an attack.
 *
 * @param knockback Total knockback value.
 * @param knockback_angle Angle to knockback towards.
 */
void Mob::applyKnockback(float knockback, float knockback_angle) {
    if(knockback != 0) {
        stopChasing();
        speed.x = cos(knockback_angle) * knockback * MOB::KNOCKBACK_H_POWER;
        speed.y = sin(knockback_angle) * knockback * MOB::KNOCKBACK_H_POWER;
        speedZ = MOB::KNOCKBACK_V_POWER;
        face(getAngle(speed) + TAU / 2, nullptr, true);
        startHeightEffect();
    }
}


/**
 * @brief Applies a status effect's effects.
 *
 * @param s Status effect to use.
 * @param given_by_parent If true, this status effect was given to the mob
 * by its parent mob.
 * @param from_hazard If true, this status effect was given from a hazard.
 */
void Mob::applyStatusEffect(
    StatusType* s, bool given_by_parent, bool from_hazard
) {
    if(parent && parent->relay_statuses && !given_by_parent) {
        parent->m->applyStatusEffect(s, false, from_hazard);
        if(!parent->handle_statuses) return;
    }
    
    if(!given_by_parent && !canReceiveStatus(s)) {
        return;
    }
    
    //Let's start by sending the status to the child mobs.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* m2_ptr = game.states.gameplay->mobs.all[m];
        if(m2_ptr->parent && m2_ptr->parent->m == this) {
            m2_ptr->applyStatusEffect(s, true, from_hazard);
        }
    }
    
    //Get the vulnerabilities to this status.
    auto vuln_it = type->statusVulnerabilities.find(s);
    if(vuln_it != type->statusVulnerabilities.end()) {
        if(vuln_it->second.statusToApply) {
            //It must instead receive this status.
            applyStatusEffect(
                vuln_it->second.statusToApply, given_by_parent, from_hazard
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
    Status new_status(s);
    new_status.fromHazard = from_hazard;
    this->statuses.push_back(new_status);
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
    if(parent->limb_parent_body_part == INVALID) {
        return;
    }
    
    float feet_normal_distance = s2f(parent->m->vars["feet_normal_distance"]);
    if(feet_normal_distance == 0) {
        feet_normal_distance = 175;
    }
    
    float default_angle =
        getAngle(
            Point(),
            parent->m->getHitbox(
                parent->limb_parent_body_part
            )->pos
        );
        
    Point final_pos = s2p(parent->m->vars["_destination_pos"]);
    float final_angle = s2f(parent->m->vars["_destination_angle"]);
    
    Point offset = Point(feet_normal_distance, 0);
    offset = rotatePoint(offset, default_angle);
    offset = rotatePoint(offset, final_angle);
    
    final_pos += offset;
    
    chase(final_pos, z);
}


/**
 * @brief Does the logic that arachnorb heads need to turn, based on their
 * feet's positions.
 */
void Mob::arachnorbHeadTurnLogic() {
    if(links.empty()) return;
    
    float angle_deviation_avg = 0;
    size_t n_feet = 0;
    
    for(size_t l = 0; l < links.size(); l++) {
        if(!links[l]) {
            continue;
        }
        
        if(!links[l]->parent) {
            continue;
        }
        if(links[l]->parent->m != this) {
            continue;
        }
        if(links[l]->parent->limb_parent_body_part == INVALID) {
            continue;
        }
        
        n_feet++;
        
        float default_angle =
            getAngle(
                Point(),
                getHitbox(
                    links[l]->parent->limb_parent_body_part
                )->pos
            );
        float cur_angle =
            getAngle(pos, links[l]->pos) - angle;
        float angle_deviation =
            getAngleCwDiff(default_angle, cur_angle);
        if(angle_deviation > M_PI) {
            angle_deviation -= TAU;
        }
        angle_deviation_avg += angle_deviation;
    }
    
    face(angle + (angle_deviation_avg / n_feet), nullptr);
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
    float max_step_distance = s2f(vars["max_step_distance"]);
    float max_turn_angle = degToRad(s2f(vars["max_turn_angle"]));
    float min_turn_angle = degToRad(s2f(vars["min_turn_angle"]));
    if(max_step_distance == 0) {
        max_step_distance = 100;
    }
    if(max_turn_angle == 0) {
        max_turn_angle = TAU * 0.2;
    }
    
    float amount_to_move = 0;
    float amount_to_turn = 0;
    
    switch(goal) {
    case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME: {
        amount_to_turn = getAngleCwDiff(angle, getAngle(pos, home));
        if(amount_to_turn > TAU / 2)  amount_to_turn -= TAU;
        if(amount_to_turn < -TAU / 2) amount_to_turn += TAU;
        
        if(fabs(amount_to_turn) < TAU * 0.05) {
            //We can also start moving towards home now.
            amount_to_move = Distance(pos, home).toFloat();
        }
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD: {
        amount_to_move = max_step_distance;
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN: {
        amount_to_turn = game.rng.f(min_turn_angle, TAU * 0.25);
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN: {
        amount_to_turn = game.rng.f(-TAU * 0.25, -min_turn_angle);
        break;
        
    }
    }
    
    amount_to_move = std::min(amount_to_move, max_step_distance);
    amount_to_turn =
        sign(amount_to_turn) *
        std::min((double) fabs(amount_to_turn), (double) max_turn_angle);
        
    Point destination_pos = pos;
    float destination_angle = angle + amount_to_turn;
    normalizeAngle(destination_angle);
    
    Point offset = Point(amount_to_move, 0);
    offset = rotatePoint(offset, destination_angle);
    
    destination_pos += offset;
    
    vars["_destination_pos"] = p2s(destination_pos);
    vars["_destination_angle"] = f2s(destination_angle);
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
 * @param target_type Return the target Pikmin type (if any) here.
 * @param target_mob Return the target mob (if any) here.
 * @param target_point Return the target point here.
 * @return Whether it succeeded.
 * Returns false if there are no available targets or if
 * something went wrong.
 */
bool Mob::calculateCarryingDestination(
    Mob* added, Mob* removed,
    PikminType** target_type, Mob** target_mob, Point* target_point
) const {
    *target_mob = nullptr;
    *target_point = pos;
    if(!carryInfo) return false;
    
    switch(carryInfo->destination) {
    case CARRY_DESTINATION_SHIP: {

        //Go to the nearest ship.
        Ship* closest_ship = calculateCarryingShip();
        
        if(closest_ship) {
            *target_mob = closest_ship;
            *target_point = closest_ship->controlPointFinalPos;
            return true;
            
        } else {
            return false;
        }
        
        break;
        
    } case CARRY_DESTINATION_ONION: {

        Onion* target = calculateCarryingOnion(added, removed, target_type);
        
        if(!target) {
            return false;
        }
        *target_mob = target;
        *target_point = (*target_mob)->pos;
        return true;
        
        break;
        
    } case CARRY_DESTINATION_SHIP_NO_ONION: {

        Onion* oni_target = calculateCarryingOnion(added, removed, target_type);
        
        if(oni_target) {
            *target_mob = oni_target;
            *target_point = (*target_mob)->pos;
            return true;
        }
        
        //No onion, find a ship instead.
        Ship* shi_target = calculateCarryingShip();
        if(shi_target) {
            *target_mob = shi_target;
            *target_point = shi_target->controlPointFinalPos;
            return true;
        }
        return false;
        
    } case CARRY_DESTINATION_LINKED_MOB: {

        //If it's towards a linked mob, just go to the closest one.
        Mob* closest_link = nullptr;
        Distance closest_link_dist;
        
        for(size_t s = 0; s < links.size(); s++) {
            Distance d(pos, links[s]->pos);
            
            if(!closest_link || d < closest_link_dist) {
                closest_link = links[s];
                closest_link_dist = d;
            }
        }
        
        if(closest_link) {
            *target_mob = closest_link;
            *target_point = closest_link->pos;
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
        
        unordered_set<PikminType*> available_types;
        vector<std::pair<Mob*, PikminType*> > mobs_per_type;
        
        for(size_t l = 0; l < links.size(); l++) {
            if(!links[l]) continue;
            string type_name =
                links[l]->vars["carry_destination_type"];
            MobType* pik_type =
                game.mobCategories.get(MOB_CATEGORY_PIKMIN)->
                getType(type_name);
            if(!pik_type) continue;
            
            available_types.insert(
                (PikminType*) pik_type
            );
            mobs_per_type.push_back(
                std::make_pair(links[l], (PikminType*) pik_type)
            );
        }
        
        if(available_types.empty()) {
            //No available types?! Well...make the Pikmin stuck.
            return false;
        }
        
        PikminType* decided_type =
            decideCarryPikminType(available_types, added, removed);
            
        //Figure out which linked mob matches the decided type.
        size_t closest_target_idx = INVALID;
        Distance closest_target_dist;
        for(size_t m = 0; m < mobs_per_type.size(); m++) {
            if(mobs_per_type[m].second != decided_type) continue;
            
            Distance d(pos, mobs_per_type[m].first->pos);
            if(closest_target_idx == INVALID || d < closest_target_dist) {
                closest_target_dist = d;
                closest_target_idx = m;
            }
        }
        
        //Finally, set the destination data.
        *target_type = decided_type;
        *target_mob = links[closest_target_idx];
        *target_point = (*target_mob)->pos;
        
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
 * @param target_type If not nullptr, the target Pikmin type is returned here.
 * @return The Onion.
 */
Onion* Mob::calculateCarryingOnion(
    Mob* added, Mob* removed, PikminType** target_type
) const {
    //If it's meant for an Onion, we need to decide which Onion, based on
    //the Pikmin. First, check which Onion Pikmin types are even available.
    unordered_set<PikminType*> available_types;
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        Onion* o_ptr = game.states.gameplay->mobs.onions[o];
        if(o_ptr->activated) {
            for(
                size_t t = 0;
                t < o_ptr->oniType->nest->pik_types.size();
                t++
            ) {
                available_types.insert(
                    o_ptr->oniType->nest->pik_types[t]
                );
            }
        }
    }
    
    if(available_types.empty()) {
        //No available types?! Well...make the Pikmin stuck.
        return nullptr;
    }
    
    PikminType* decided_type =
        decideCarryPikminType(available_types, added, removed);
        
    //Figure out where that type's Onion is.
    size_t closest_onion_idx = INVALID;
    Distance closest_onion_dist;
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        Onion* o_ptr = game.states.gameplay->mobs.onions[o];
        if(!o_ptr->activated) continue;
        bool has_type = false;
        for(
            size_t t = 0;
            t < o_ptr->oniType->nest->pik_types.size();
            t++
        ) {
            if(o_ptr->oniType->nest->pik_types[t] == decided_type) {
                has_type = true;
                break;
            }
        }
        if(!has_type) continue;
        
        Distance d(pos, o_ptr->pos);
        if(closest_onion_idx == INVALID || d < closest_onion_dist) {
            closest_onion_dist = d;
            closest_onion_idx = o;
        }
    }
    
    *target_type = decided_type;
    return game.states.gameplay->mobs.onions[closest_onion_idx];
}


/**
 * @brief Calculates to which ship Pikmin should carry something.
 *
 * @return The ship.
 */
Ship* Mob::calculateCarryingShip() const {
    //Go to the nearest ship.
    Ship* closest_ship = nullptr;
    Distance closest_ship_dist;
    
    for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); s++) {
        Ship* s_ptr = game.states.gameplay->mobs.ships[s];
        Distance d(pos, s_ptr->controlPointFinalPos);
        
        if(!closest_ship || d < closest_ship_dist) {
            closest_ship = s_ptr;
            closest_ship_dist = d;
        }
    }
    return closest_ship;
}


/**
 * @brief Calculates how much damage an attack will cause.
 *
 * @param victim The mob that'll take the damage.
 * @param attack_h Hitbox used for the attack.
 * @param victim_h Victim's hitbox that got hit.
 * @param damage Return the calculated damage here.
 * @return Whether the attack will hit.
 * Returns true even if it will end up causing zero damage.
 * Returns false if it cannot hit (e.g. the victim hitbox is not valid).
 */
bool Mob::calculateDamage(
    Mob* victim, Hitbox* attack_h, const Hitbox* victim_h, float* damage
) const {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    
    //First, check if this hitbox cannot be damaged.
    if(victim_h->type != HITBOX_TYPE_NORMAL) {
        //This hitbox can't be damaged! Abort!
        return false;
    }
    
    //Calculate the damage.
    if(attack_h) {
        attacker_offense = attack_h->value;
        
        if(!attack_h->hazards.empty()) {
            float max_vulnerability = 0.0f;
            for(size_t h = 0; h < attack_h->hazards.size(); h++) {
                MobType::Vulnerability vuln =
                    victim->getHazardVulnerability(attack_h->hazards[h]);
                max_vulnerability =
                    std::max(vuln.effectMult, max_vulnerability);
            }
            
            if(max_vulnerability == 0.0f) {
                //The victim is immune to this hazard!
                *damage = 0;
                return true;
            } else {
                defense_multiplier = 1.0f / max_vulnerability;
            }
            
        } else {
        
            if(victim->type->defaultVulnerability == 0.0f) {
                //The victim is invulnerable to everything about this attack!
                *damage = 0;
                return true;
            } else {
                defense_multiplier = 1.0f / victim->type->defaultVulnerability;
            }
        }
        
    } else {
        attacker_offense = 1;
    }
    
    if(victim_h->value == 0.0f) {
        //Hah, this hitbox is invulnerable!
        *damage = 0;
        return true;
    }
    
    defense_multiplier *= victim_h->value;
    
    for(size_t s = 0; s < statuses.size(); s++) {
        attacker_offense *= statuses[s].type->attackMultiplier;
    }
    for(size_t s = 0; s < victim->statuses.size(); s++) {
        float vuln_mult = victim->statuses[s].type->defenseMultiplier - 1.0f;
        auto vuln_it = type->statusVulnerabilities.find(statuses[s].type);
        if(vuln_it != type->statusVulnerabilities.end()) {
            vuln_mult *= vuln_it->second.effectMult;
        }
        defense_multiplier *= (vuln_mult + 1.0f);
    }
    
    if(this->type->category->id == MOB_CATEGORY_PIKMIN) {
        //It's easier to calculate the maturity attack boost here.
        Pikmin* pik_ptr = (Pikmin*) this;
        attacker_offense *=
            1 + (game.config.pikmin.maturityPowerMult * pik_ptr->maturity);
    }
    
    *damage = attacker_offense * (1.0f / defense_multiplier);
    return true;
}


/**
 * @brief Calculates how much knockback an attack will cause.
 *
 * @param victim The mob that'll take the damage.
 * @param attack_h The hitbox of the attacker mob, if any.
 * @param victim_h The hitbox of the victim mob, if any.
 * @param kb_strength The variable to return the knockback amount to.
 * @param kb_angle The variable to return the angle of the knockback to.
 */
void Mob::calculateKnockback(
    const Mob* victim, const Hitbox* attack_h,
    Hitbox* victim_h, float* kb_strength, float* kb_angle
) const {
    if(attack_h) {
        *kb_strength = attack_h->knockback;
        if(attack_h->knockbackOutward) {
            *kb_angle =
                getAngle(attack_h->getCurPos(pos, angle), victim->pos);
        } else {
            *kb_angle =
                angle + attack_h->knockbackAngle;
        }
    } else {
        *kb_strength = 0;
        *kb_angle = 0;
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
 * @param is_ingestion If true, the attacker just got eaten.
 * If false, it merely got hurt.
 */
void Mob::causeSpikeDamage(Mob* victim, bool is_ingestion) {
    if(!type->spikeDamage) return;
    
    if(type->spikeDamage->ingestionOnly != is_ingestion) return;
    
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
 * @param orig_coords Pointer to changing coordinates. If nullptr, it is
 * the world origin. Use this to make the mob follow another mob
 * wherever they go, for instance.
 * @param orig_z Same as orig_coords, but for the Z coordinate.
 * @param offset Offset from orig_coords.
 * @param offset_z Z offset from orig_z.
 * @param flags Flags that control how to chase. Use CHASE_FLAG_*.
 * @param target_distance Distance at which the mob considers the
 * chase finished.
 * @param speed Speed at which to go to the target.
 * LARGE_FLOAT makes it use the mob's standard speed.
 * @param acceleration Speed acceleration.
 * LARGE_FLOAT makes it use the mob's standard acceleration.
 */
void Mob::chase(
    Point* orig_coords, float* orig_z,
    const Point &offset, float offset_z,
    bitmask_8_t flags,
    float target_distance, float speed, float acceleration
) {
    chaseInfo.origCoords = orig_coords;
    chaseInfo.origZ = orig_z;
    chaseInfo.offset = offset;
    chaseInfo.offsetZ = offset_z;
    
    chaseInfo.flags = flags;
    if(type->canFreeMove) {
        enableFlag(chaseInfo.flags, CHASE_FLAG_ANY_ANGLE);
    }
    
    chaseInfo.targetDist = target_distance;
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
 * @param coords_z Z coordinates of the target.
 * @param flags Flags that control how to chase. Use CHASE_FLAG_*.
 * @param target_distance Distance at which the mob considers the
 * chase finished.
 * @param speed Speed at which to go to the target.
 * LARGE_FLOAT makes it use the mob's standard speed.
 * @param acceleration Speed acceleration.
 * LARGE_FLOAT makes it use the mob's standard acceleration.
 */
void Mob::chase(
    const Point &coords, float coords_z,
    unsigned char flags,
    float target_distance, float speed, float acceleration
) {
    chase(
        nullptr, nullptr, coords, coords_z,
        flags, target_distance, speed, acceleration
    );
}


/**
 * @brief Makes a mob chomp another mob. Mostly applicable for enemies chomping
 * on Pikmin.
 *
 * @param m The mob to be chomped.
 * @param hitbox_info Information about the hitbox that caused the chomp.
 */
void Mob::chomp(Mob* m, const Hitbox* hitbox_info) {
    if(m->type->category->id == MOB_CATEGORY_TOOLS) {
        Tool* too_ptr = (Tool*) m;
        if(!hasFlag(too_ptr->holdabilityFlags, HOLDABILITY_FLAG_ENEMIES)) {
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
    
    float h_offset_dist;
    float h_offset_angle;
    float v_offset_dist;
    getHitboxHoldPoint(
        m, hitbox_info, &h_offset_dist, &h_offset_angle, &v_offset_dist
    );
    hold(
        m, hitbox_info->bodyPartIdx,
        h_offset_dist, h_offset_angle, v_offset_dist,
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
 * @param can_free_move Can the mob move freely, or only forward?
 */
void Mob::circleAround(
    Mob* m, const Point &p, float radius, bool clockwise,
    float speed, bool can_free_move
) {
    if(!circlingInfo) {
        circlingInfo = new CirclingInfo(this);
    }
    circlingInfo->circlingMob = m;
    circlingInfo->circlingPoint = p;
    circlingInfo->radius = radius;
    circlingInfo->clockwise = clockwise;
    circlingInfo->speed = speed;
    circlingInfo->canFreeMove = can_free_move;
    circlingInfo->curAngle =
        getAngle((m ? m->pos : p), pos);
}


/**
 * @brief Returns what Pikmin type is decided when carrying something.
 *
 * @param available_types List of Pikmin types that are currently
 * available in the area.
 * @param added If a Pikmin got added to the carriers, specify it here.
 * @param removed If a Pikmin got removed from the carriers, specify it here.
 * @return The Pikmin type.
 */
PikminType* Mob::decideCarryPikminType(
    const unordered_set<PikminType*> &available_types,
    Mob* added, Mob* removed
) const {
    //How many of each Pikmin type are carrying.
    map<PikminType*, unsigned> type_quantity;
    //The Pikmin type with the most carriers.
    vector<PikminType*> majority_types;
    
    //Count how many of each type there are carrying.
    for(size_t p = 0; p < type->maxCarriers; p++) {
        Pikmin* pik_ptr = nullptr;
        
        if(carryInfo->spotInfo[p].state != CARRY_SPOT_STATE_USED) continue;
        
        pik_ptr = (Pikmin*) carryInfo->spotInfo[p].pikPtr;
        
        //If it doesn't have an Onion to carry to, it won't even count.
        if(available_types.find(pik_ptr->pikType) == available_types.end()) {
            continue;
        }
        
        type_quantity[pik_ptr->pikType]++;
    }
    
    //Then figure out what are the majority types.
    unsigned most = 0;
    for(auto &t : type_quantity) {
        if(t.second > most) {
            most = t.second;
            majority_types.clear();
        }
        if(t.second == most) majority_types.push_back(t.first);
    }
    
    //If we ended up with no candidates, pick a type at random,
    //out of all possible types.
    bool force_random = false;
    if(majority_types.empty()) {
        force_random = true;
        for(
            auto t = available_types.begin();
            t != available_types.end(); ++t
        ) {
            majority_types.push_back(*t);
        }
    }
    
    PikminType* decided_type = nullptr;
    
    //Now let's pick an Pikmin type from the candidates.
    if(majority_types.size() == 1) {
        //If there's only one possible type to pick, pick it.
        decided_type = *majority_types.begin();
        
    } else {
        //If the current type is a majority, it takes priority.
        //Otherwise, pick a majority at random.
        if(
            carryInfo->intendedPikType &&
            !force_random &&
            find(
                majority_types.begin(),
                majority_types.end(),
                carryInfo->intendedPikType
            ) != majority_types.end()
        ) {
            decided_type = carryInfo->intendedPikType;
        } else {
            decided_type =
                majority_types[
                    game.rng.i(0, (int) majority_types.size() - 1)
                ];
        }
    }
    
    return decided_type;
}


/**
 * @brief Deletes all status effects asking to be deleted.
 */
void Mob::deleteOldStatusEffects() {
    vector<std::pair<StatusType*, bool> > new_statuses_to_apply;
    bool removed_forced_sprite = false;
    
    for(size_t s = 0; s < statuses.size(); ) {
        Status &s_ptr = statuses[s];
        if(s_ptr.toDelete) {
            handleStatusEffectLoss(s_ptr.type);
            
            if(s_ptr.type->generatesParticles) {
                removeParticleGenerator(s_ptr.type->particleGen->id);
            }
            
            if(s_ptr.type->freezesAnimation) {
                removed_forced_sprite = true;
            }
            
            if(s_ptr.type->replacementOnTimeout && s_ptr.timeLeft <= 0.0f) {
                new_statuses_to_apply.push_back(
                    std::make_pair(
                        s_ptr.type->replacementOnTimeout,
                        s_ptr.fromHazard
                    )
                );
                if(s_ptr.type->replacementOnTimeout->freezesAnimation) {
                    //Actually, never mind, let's keep the current forced
                    //sprite so that the next status effect can use it too.
                    removed_forced_sprite = false;
                }
            }
            
            statuses.erase(statuses.begin() + s);
        } else {
            s++;
        }
    }
    
    //Apply new status effects.
    for(size_t s = 0; s < new_statuses_to_apply.size(); s++) {
        applyStatusEffect(
            new_statuses_to_apply[s].first,
            false, new_statuses_to_apply[s].second
        );
    }
    
    if(removed_forced_sprite) {
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
 * @param attack_h Hitbox that caused the attack.
 * @param victim_h Hitbox that suffered the attack.
 * @param damage Total damage caused.
 * @param knockback Total knockback strength.
 */
void Mob::doAttackEffects(
    const Mob* attacker, const Hitbox* attack_h, const Hitbox* victim_h,
    float damage, float knockback
) {
    if(attack_h->value == 0.0f) {
        //Attack hitboxes that cause 0 damage don't need to smack or ding.
        //This way, objects can "attack" other objects at 0 damage for the
        //purposes of triggering events (like hazard touching), without
        //having to constantly display the dings.
        //The ding effect should only be used when an attack that really WANTED
        //to cause damage failed to do so, thus highlighting the uselessness.
        return;
    }
    
    //Calculate the particle's final position.
    Point attack_h_pos = attack_h->getCurPos(attacker->pos, attacker->angle);
    Point victim_h_pos = victim_h->getCurPos(pos, angle);
    
    float edges_d;
    float a_to_v_angle;
    coordinatesToAngle(
        victim_h_pos - attack_h_pos,
        &a_to_v_angle, &edges_d
    );
    
    edges_d -= attack_h->radius;
    edges_d -= victim_h->radius;
    float offset = attack_h->radius + edges_d / 2.0;
    
    Point particle_pos =
        attack_h_pos +
        Point(cos(a_to_v_angle) * offset, sin(a_to_v_angle) * offset);
    float particle_z =
        std::max(
            z + getDrawingHeight() + 1.0f,
            attacker->z + attacker->getDrawingHeight() + 1.0f
        );
        
    bool useless = (damage <= 0 && knockback == 0.0f);
    
    //Create the particle.
    string particle_internal_name =
        useless ?
        game.sysContentNames.parDing :
        game.sysContentNames.parSmack;
    ParticleGenerator pg =
        standardParticleGenSetup(
            particle_internal_name, nullptr
        );
    pg.baseParticle.pos = particle_pos;
    pg.baseParticle.z = particle_z;
    pg.emit(game.states.gameplay->particles);
    
    if(!useless) {
        //Play the sound.
        
        SoundSourceConfig attack_sound_config;
        attack_sound_config.gain = 0.6f;
        game.audio.createPosSoundSource(
            game.sysContent.sndAttack,
            pos, false, attack_sound_config
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
    if(!parent->limb_anim.animDb) return;
    Sprite* limb_cur_s_ptr;
    Sprite* limb_next_s_ptr;
    float limb_interpolation_factor;
    parent->limb_anim.getSpriteData(
        &limb_cur_s_ptr, &limb_next_s_ptr, &limb_interpolation_factor
    );
    if(!limb_cur_s_ptr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        limb_cur_s_ptr, limb_next_s_ptr, limb_interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    Point parent_end;
    if(parent->limb_parent_body_part == INVALID) {
        parent_end = parent->m->pos;
    } else {
        parent_end =
            parent->m->getHitbox(
                parent->limb_parent_body_part
            )->getCurPos(
                parent->m->pos, parent->m->angleCos, parent->m->angleSin
            );
    }
    
    Point child_end;
    if(parent->limb_child_body_part == INVALID) {
        child_end = pos;
    } else {
        child_end =
            getHitbox(
                parent->limb_child_body_part
            )->getCurPos(pos, angleCos, angleSin);
    }
    
    float p2c_angle = getAngle(parent_end, child_end);
    
    if(parent->limb_parent_offset) {
        parent_end +=
            rotatePoint(
                Point(parent->limb_parent_offset, 0), p2c_angle
            );
    }
    if(parent->limb_child_offset) {
        child_end -=
            rotatePoint(
                Point(parent->limb_child_offset, 0), p2c_angle
            );
    }
    
    float length = Distance(parent_end, child_end).toFloat();
    Point limb_bmp_size = getBitmapDimensions(limb_cur_s_ptr->bitmap);
    
    eff.translation = (parent_end + child_end) / 2.0;
    eff.scale.x = length / limb_bmp_size.x;
    eff.scale.y = parent->limb_thickness / limb_bmp_size.y;
    eff.rotation = p2c_angle;
    
    drawBitmapWithEffects(limb_cur_s_ptr->bitmap, eff);
}


/**
 * @brief Draws just the mob.
 * This is a generic function, and can be overwritten by child classes.
 */
void Mob::drawMob() {
    Sprite* cur_s_ptr;
    Sprite* next_s_ptr;
    float interpolation_factor;
    getSpriteData(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY
    );
    
    drawBitmapWithEffects(cur_s_ptr->bitmap, eff);
}


/**
 * @brief Makes a mob intend to face a new angle, or face there right away.
 *
 * @param new_angle Face this angle.
 * @param new_pos If this is not nullptr, turn towards this point every frame,
 * instead.
 * @param instantly If true, the mob faces that angle instantly instead
 * of rotating towards that direction over time.
 */
void Mob::face(float new_angle, Point* new_pos, bool instantly) {
    if(carryInfo) return; //If it's being carried, it shouldn't rotate.
    intendedTurnAngle = new_angle;
    intendedTurnPos = new_pos;
    if(instantly) {
        angle = new_angle;
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
    bool was_blocked = false;
    PathStop* old_next_stop = nullptr;
    
    //Some setup before we begin.
    if(hasFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE) && pathInfo) {
        was_blocked = pathInfo->block_reason != PATH_BLOCK_REASON_NONE;
        if(pathInfo->cur_path_stop_idx < pathInfo->path.size()) {
            old_next_stop = pathInfo->path[pathInfo->cur_path_stop_idx];
        }
    }
    
    if(pathInfo) {
        delete pathInfo;
    }
    
    PathFollowSettings final_settings = settings;
    
    if(carryInfo) {
        //Check if this carriable is considered light load.
        if(type->weight == 1) {
            enableFlag(final_settings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD);
        }
        //The object will only be airborne if all its carriers can fly.
        if(carryInfo->canFly()) {
            enableFlag(final_settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        }
    } else {
        if(
            type->category->id == MOB_CATEGORY_PIKMIN ||
            type->category->id == MOB_CATEGORY_LEADERS
        ) {
            //Simple mobs are empty-handed, so that's considered light load.
            enableFlag(final_settings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD);
        }
        //Check if the object can fly directly.
        if(hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            enableFlag(final_settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        }
    }
    
    if(carryInfo) {
        //The object is only as invulnerable as the Pikmin carrying it.
        final_settings.invulnerabilities =
            carryInfo->getCarrierInvulnerabilities();
    } if(group) {
        //The object is only as invulnerable as the members of its group.
        final_settings.invulnerabilities =
            group->getGroupInvulnerabilities(this);
    } else {
        //Use the object's standard invulnerabilities.
        for(auto &v : type->hazardVulnerabilities) {
            if(v.second.effectMult == 0.0f) {
                final_settings.invulnerabilities.push_back(v.first);
            }
        }
    }
    
    //Establish the mob's path-following information.
    //This also generates the path to take.
    pathInfo = new Path(this, final_settings);
    
    if(
        hasFlag(pathInfo->settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE) &&
        old_next_stop &&
        !was_blocked &&
        pathInfo->path.size() >= 2
    ) {
        for(size_t s = 1; s < pathInfo->path.size(); s++) {
            if(pathInfo->path[s] == old_next_stop) {
                //If before, the mob was already heading towards this stop,
                //then just continue the new journey from there.
                pathInfo->cur_path_stop_idx = s;
                break;
            }
        }
    }
    
    if(pathInfo->path.size() >= 2 && pathInfo->cur_path_stop_idx > 0) {
        if(pathInfo->checkBlockage(&pathInfo->block_reason)) {
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
        PathStop* next_stop =
            pathInfo->path[pathInfo->cur_path_stop_idx];
        float next_stop_z = z;
        if(
            hasFlag(pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) &&
            next_stop->sectorPtr
        ) {
            next_stop_z =
                next_stop->sectorPtr->z +
                PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT;
        }
        
        chase(
            next_stop->pos, next_stop_z,
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
 * @param out_z If not nullptr, the Z coordinate is returned here.
 * @return The (X and Y) coordinates of the target.
 */
Point Mob::getChaseTarget(float* out_z) const {
    Point p = chaseInfo.offset;
    if(chaseInfo.origCoords) p += (*chaseInfo.origCoords);
    if(out_z) {
        *out_z = chaseInfo.offsetZ;
        if(chaseInfo.origZ) (*out_z) += (*chaseInfo.origZ);
    }
    return p;
}


/**
 * @brief Returns the closest hitbox to a point,
 * belonging to a mob's current frame of animation and position.
 *
 * @param p The point.
 * @param h_type Type of hitbox. INVALID means any.
 * @param d Return the distance here, optionally.
 * @return The hitbox.
 */
Hitbox* Mob::getClosestHitbox(
    const Point &p, size_t h_type, Distance* d
) const {
    Sprite* s;
    getSpriteData(&s, nullptr, nullptr);
    if(!s) return nullptr;
    Hitbox* closest_hitbox = nullptr;
    float closest_hitbox_dist = 0;
    
    for(size_t h = 0; h < s->hitboxes.size(); h++) {
        Hitbox* h_ptr = &s->hitboxes[h];
        if(h_type != INVALID && h_ptr->type != h_type) continue;
        
        float this_d =
            Distance(
                h_ptr->getCurPos(pos, angleCos, angleSin), p
            ).toFloat() - h_ptr->radius;
        if(closest_hitbox == nullptr || this_d < closest_hitbox_dist) {
            closest_hitbox_dist = this_d;
            closest_hitbox = h_ptr;
        }
    }
    
    if(d) *d = closest_hitbox_dist;
    
    return closest_hitbox;
}


/**
 * @brief Returns data for figuring out the state of the current sprite
 * of animation.
 *
 * Normally, this returns the current animation's current sprite,
 * but it can return a forced sprite (e.g. from a status effect that
 * freezes animations).
 *
 * @param out_cur_sprite_ptr If not nullptr, the current frame's sprite is
 * returned here.
 * @param out_next_sprite_ptr If not nullptr, the next frame's sprite is
 * returned here.
 * @param out_interpolation_factor If not nullptr, the interpolation factor
 * (0 to 1) between the two is returned here.
 */
void Mob::getSpriteData(
    Sprite** out_cur_sprite_ptr, Sprite** out_next_sprite_ptr,
    float* out_interpolation_factor
) const {
    if(forcedSprite) {
        if(out_cur_sprite_ptr) *out_cur_sprite_ptr = forcedSprite;
        if(out_next_sprite_ptr) *out_next_sprite_ptr = forcedSprite;
        if(out_interpolation_factor) *out_interpolation_factor = 0.0f;
    } else {
        anim.getSpriteData(
            out_cur_sprite_ptr, out_next_sprite_ptr, out_interpolation_factor
        );
    }
}


/**
 * @brief Returns the distance between the limits of this mob and
 * the limits of another.
 *
 * @param m2_ptr Pointer to the mob to check.
 * @param regular_distance_cache If the regular distance had already been
 * calculated, specify it here. This should help with performance.
 * Otherwise, use nullptr.
 * @return The distance.
 */
Distance Mob::getDistanceBetween(
    const Mob* m2_ptr, const Distance* regular_distance_cache
) const {
    Distance mob_to_hotspot_dist;
    float dist_padding;
    if(m2_ptr->rectangularDim.x != 0.0f) {
        bool is_inside = false;
        Point hotspot =
            getClosestPointInRotatedRectangle(
                pos,
                m2_ptr->pos, m2_ptr->rectangularDim,
                m2_ptr->angle,
                &is_inside
            );
        if(is_inside) {
            mob_to_hotspot_dist = Distance(0.0f);
        } else {
            mob_to_hotspot_dist = Distance(pos, hotspot);
        }
        dist_padding = radius;
    } else {
        if(regular_distance_cache) {
            mob_to_hotspot_dist = *regular_distance_cache;
        } else {
            mob_to_hotspot_dist = Distance(pos, m2_ptr->pos);
        }
        dist_padding = radius + m2_ptr->radius;
    }
    mob_to_hotspot_dist -= dist_padding;
    return mob_to_hotspot_dist;
}


/**
 * @brief Returns information on how to show the fraction numbers.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 *
 * @param fraction_value_nr The fraction's value (upper) number gets set here.
 * @param fraction_req_nr The fraction's required (lower) number gets set here.
 * @param fraction_color The fraction's color gets set here.
 * @return Whether the numbers should be shown.
 */
bool Mob::getFractionNumbersInfo(
    float* fraction_value_nr, float* fraction_req_nr,
    ALLEGRO_COLOR* fraction_color
) const {
    if(!carryInfo || carryInfo->curCarryingStrength <= 0) return false;
    bool destination_has_pikmin_type =
        carryInfo->intendedMob &&
        carryInfo->intendedPikType;
    if(type->weight <= 1 && !destination_has_pikmin_type) return false;
    
    *fraction_value_nr = carryInfo->curCarryingStrength;
    *fraction_req_nr = type->weight;
    if(carryInfo->isMoving) {
        if(
            carryInfo->destination ==
            CARRY_DESTINATION_SHIP
        ) {
            *fraction_color = game.config.aestheticGen.carryingColorMove;
            
        } else if(destination_has_pikmin_type) {
            *fraction_color =
                carryInfo->intendedPikType->mainColor;
        } else {
            *fraction_color = game.config.aestheticGen.carryingColorMove;
        }
    } else {
        *fraction_color = game.config.aestheticGen.carryingColorStop;
    }
    return true;
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param out_spot The final coordinates are returned here.
 * @param out_dist The final distance to those coordinates is returned here.
 */
void Mob::getGroupSpotInfo(
    Point* out_spot, float* out_dist
) const {
    out_spot->x = 0.0f;
    out_spot->y = 0.0f;
    *out_dist = 0.0f;
}


/**
 * @brief Returns how vulnerable the mob is to that specific hazard,
 * or the mob type's default if there is no vulnerability data for that hazard.
 *
 * @param h_ptr The hazard to check.
 * @return The vulnerability info.
 */
MobType::Vulnerability Mob::getHazardVulnerability(
    Hazard* h_ptr
) const {
    MobType::Vulnerability vuln;
    vuln.effectMult = type->defaultVulnerability;
    
    auto v = type->hazardVulnerabilities.find(h_ptr);
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
 * @param mob_to_hold The mob that will be held.
 * @param h_ptr Pointer to the hitbox to check.
 * @param offset_dist The distance from the center of the hitbox is
 * returned here. 1 means the full radius.
 * @param offset_angle The angle the mob to hold makes with the hitbox's
 * center is returned here.
 * @param vertical_dist Ratio of distance from the hitbox/body's bottom.
 * 1 is the very top.
 */
void Mob::getHitboxHoldPoint(
    const Mob* mob_to_hold, const Hitbox* h_ptr,
    float* offset_dist, float* offset_angle, float* vertical_dist
) const {
    Point actual_h_pos = h_ptr->getCurPos(pos, angleCos, angleSin);
    float actual_h_z = z + h_ptr->z;
    
    Point pos_dif = mob_to_hold->pos - actual_h_pos;
    coordinatesToAngle(pos_dif, offset_angle, offset_dist);
    
    //Relative to 0 degrees.
    *offset_angle -= angle;
    //Distance in units to distance in percentage.
    *offset_dist /= h_ptr->radius;
    
    if(h_ptr->height <= 0.0f) {
        *vertical_dist = 0.0f;
    } else {
        *vertical_dist = mob_to_hold->z - actual_h_z;
        *vertical_dist /= h_ptr->height;
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
        Pikmin* p_ptr = game.states.gameplay->mobs.pikmin[p];
        if(p_ptr->focusedMob != this) continue;
        if(p_ptr->holder.m != this) continue;
        if(!p_ptr->latched) continue;
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
        Pikmin* p_ptr = game.states.gameplay->mobs.pikmin[p];
        if(p_ptr->focusedMob != this) continue;
        if(p_ptr->holder.m != this) continue;
        if(!p_ptr->latched) continue;
        total += p_ptr->type->weight;
    }
    return total;
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
    float move_speed_mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); s++) {
        if(!statuses[s].toDelete) {
            float vuln_mult = this->statuses[s].type->speedMultiplier - 1.0f;
            auto vuln_it = type->statusVulnerabilities.find(statuses[s].type);
            if(vuln_it != type->statusVulnerabilities.end()) {
                vuln_mult *= vuln_it->second.effectMult;
            }
            move_speed_mult *= (vuln_mult + 1.0f);
        }
    }
    return move_speed_mult;
}


/**
 * @brief Returns what the given sprite's center, rotation, tint, etc. should be
 * at the present moment, for normal mob drawing routines.
 *
 * @param s_ptr Sprite to get info about.
 * @param next_s_ptr Next sprite in the animation, if any.
 * @param interpolation_factor If we're meant to interpolate from the current
 * sprite to the next, specify the interpolation factor (0 to 1) here.
 * @param info Struct to fill the info with.
 * @param effects What effects to use. Use SPRITE_BMP_EFFECT_FLAG for this.
 */
void Mob::getSpriteBitmapEffects(
    Sprite* s_ptr, Sprite* next_s_ptr, float interpolation_factor,
    BitmapEffect* info, bitmask_16_t effects
) const {

    //Animation, position, angle, etc.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_STANDARD)) {
        Point eff_trans;
        float eff_angle;
        Point eff_scale;
        ALLEGRO_COLOR eff_tint;
        
        getSpriteBasicEffects(
            pos, angle, angleCos, angleSin,
            s_ptr, next_s_ptr, interpolation_factor,
            &eff_trans, &eff_angle, &eff_scale, &eff_tint
        );
        
        info->translation += eff_trans;
        info->rotation += eff_angle;
        info->scale.x *= eff_scale.x;
        info->scale.y *= eff_scale.y;
        info->tintColor.r *= eff_tint.r;
        info->tintColor.g *= eff_tint.g;
        info->tintColor.b *= eff_tint.b;
        info->tintColor.a *= eff_tint.a;
    }
    
    //Status effects.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_STATUS)) {
        size_t n_glow_colors = 0;
        ALLEGRO_COLOR glow_color_sum = COLOR_EMPTY;
        
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
                glow_color_sum.r += t->glow.r;
                glow_color_sum.g += t->glow.g;
                glow_color_sum.b += t->glow.b;
                glow_color_sum.a += t->glow.a;
                n_glow_colors++;
            }
            
            if(n_glow_colors > 0) {
                t->glow.r = glow_color_sum.r / n_glow_colors;
                t->glow.g = glow_color_sum.g / n_glow_colors;
                t->glow.b = glow_color_sum.b / n_glow_colors;
                t->glow.a = glow_color_sum.a / n_glow_colors;
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
        Sector* sector_ptr = centerSector;
        float brightness = centerSector->brightness / 255.0;
        if(sector_ptr->fade) {
            Sector* texture_sector[2] = {nullptr, nullptr};
            sector_ptr->getTextureMergeSectors(
                &texture_sector[0], &texture_sector[1]
            );
            vector<Edge*> fade_edges[2];
            size_t n_edges = sector_ptr->edges.size();
            for(size_t e = 0; e < n_edges; e++) {
                Edge* e_ptr = sector_ptr->edges[e];
                Sector* o_sector = e_ptr->getOtherSector(sector_ptr);
                if(o_sector == texture_sector[0]) {
                    fade_edges[0].push_back(e_ptr);
                }
                if(o_sector == texture_sector[1]) {
                    fade_edges[1].push_back(e_ptr);
                }
            }
            
            Distance closest_dist[2] = {Distance(FLT_MAX), Distance(FLT_MAX)};
            for(size_t n = 0; n < 2; n++) {
                for(size_t e = 0; e < fade_edges[n].size(); e++) {
                    Point v1 = v2p(fade_edges[n][e]->vertexes[0]);
                    Point v2 = v2p(fade_edges[n][e]->vertexes[1]);
                    float segment_ratio;
                    Point closest_pos = getClosestPointInLineSeg(v1, v2, pos, &segment_ratio);
                    if(segment_ratio < 0) {
                        Point v2_to_v1 = v2 - v1;
                        closest_pos -= v2_to_v1 * abs(segment_ratio);
                    }
                    if(segment_ratio > 1) {
                        Point v2_to_v1 = v2 - v1;
                        closest_pos -= v2_to_v1 * (segment_ratio - 1);
                    }
                    
                    Distance d(closest_pos, pos);
                    closest_dist[n] = closest_dist[n] <= d ? closest_dist[n] : d;
                }
            }
            float total_brightness = 0;
            if(texture_sector[0]) {
                total_brightness +=
                    texture_sector[0]->brightness *
                    (
                        closest_dist[1].toFloat() /
                        (
                            closest_dist[0].toFloat() +
                            closest_dist[1].toFloat()
                        )
                    );
            }
            if(texture_sector[1]) {
                total_brightness +=
                    texture_sector[1]->brightness *
                    (
                        closest_dist[0].toFloat() /
                        (
                            closest_dist[0].toFloat() +
                            closest_dist[1].toFloat()
                        )
                    );
            }
            brightness = total_brightness / 255.0;
        }
        
        info->tintColor.r *= brightness;
        info->tintColor.g *= brightness;
        info->tintColor.b *= brightness;
    }
    
    //Height effect.
    if(hasFlag(effects, SPRITE_BMP_EFFECT_FLAG_HEIGHT)) {
        if(heightEffectPivot != LARGE_FLOAT) {
            float height_effect_scale = 1.0;
            //First, check for the mob being in the air.
            height_effect_scale +=
                (z - heightEffectPivot) * MOB::HEIGHT_EFFECT_FACTOR;
            height_effect_scale = std::max(height_effect_scale, 1.0f);
            if(
                groundSector->isBottomlessPit &&
                height_effect_scale == 1.0f
            ) {
                //When atop a pit, heightEffectPivot holds what height
                //the mob fell from.
                height_effect_scale =
                    (z - groundSector->z) /
                    (heightEffectPivot - groundSector->z);
            }
            info->scale *= height_effect_scale;
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
            ALLEGRO_COLOR new_glow;
            float new_scale;
            Point new_offset;
            
            float shake_scale =
                (1 - deliveryInfo->animTimeRatioLeft) *
                MOB::DELIVERY_SUCK_SHAKING_MULT;
                
            if(deliveryInfo->animTimeRatioLeft < 0.4) {
                shake_scale =
                    std::max(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft, 0.2, 0.4,
                            0.0f, shake_scale),
                        0.0f);
            }
            
            new_offset.x =
                sin(
                    game.states.gameplay->areaTimePassed *
                    MOB::DELIVERY_SUCK_SHAKING_TIME_MULT
                ) * shake_scale;
                
                
            if(deliveryInfo->animTimeRatioLeft > 0.6) {
                //Changing color.
                new_glow =
                    interpolateColor(
                        deliveryInfo->animTimeRatioLeft, 0.6, 1.0,
                        deliveryInfo->color, mapGray(0)
                    );
                new_scale = 1.0f;
            } else if(deliveryInfo->animTimeRatioLeft > 0.4) {
                //Fixed in color.
                new_glow = deliveryInfo->color;
                new_scale = 1.0f;
            } else {
                //Shrinking.
                new_glow = deliveryInfo->color;
                new_scale =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft, 0.0, 0.4,
                        0.0f, 1.0f
                    );
                new_scale = ease(EASE_METHOD_OUT, new_scale);
                
                Point target_pos = focusedMob->pos;
                
                if(focusedMob->type->category->id == MOB_CATEGORY_SHIPS) {
                    Ship* shi_ptr = (Ship*) focusedMob;
                    target_pos = shi_ptr->receptacleFinalPos;
                }
                
                Point end_offset = target_pos - pos;
                
                float absorb_ratio =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft, 0.0, 0.4,
                        1.0f, 0.0f
                    );
                absorb_ratio = ease(EASE_METHOD_IN, absorb_ratio);
                new_offset += end_offset * absorb_ratio;
            }
            
            info->glowColor.r =
                std::clamp(info->glowColor.r + new_glow.r, 0.0f, 1.0f);
            info->glowColor.g =
                std::clamp(info->glowColor.g + new_glow.g, 0.0f, 1.0f);
            info->glowColor.b =
                std::clamp(info->glowColor.b + new_glow.b, 0.0f, 1.0f);
            info->glowColor.a =
                std::clamp(info->glowColor.a + new_glow.a, 0.0f, 1.0f);
                
            info->scale *= new_scale;
            info->translation += new_offset;
            break;
        }
        case DELIVERY_ANIM_TOSS: {
            Point new_offset;
            float new_scale = 1.0f;
            
            if(deliveryInfo->animTimeRatioLeft > 0.85) {
                //Wind-up.
                new_offset.y =
                    sin(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft,
                            0.85f, 1.0f,
                            0.0f, TAU / 2.0f
                        )
                    );
                new_offset.y *= MOB::DELIVERY_TOSS_WINDUP_MULT;
            } else {
                //Toss.
                new_offset.y =
                    sin(
                        interpolateNumber(
                            deliveryInfo->animTimeRatioLeft,
                            0.0f, 0.85f,
                            TAU / 2.0f, TAU
                        )
                    );
                new_offset.y *= MOB::DELIVERY_TOSS_MULT;
                //Randomly deviate left or right, slightly.
                float deviation_mult =
                    hashNr((unsigned int) id) / (float) UINT32_MAX;
                deviation_mult = deviation_mult * 2.0f - 1.0f;
                deviation_mult *= MOB::DELIVERY_TOSS_X_OFFSET;
                new_offset.x =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft,
                        0.0f, 0.85f,
                        1.0f, 0.0f
                    ) * deviation_mult;
                new_scale =
                    interpolateNumber(
                        deliveryInfo->animTimeRatioLeft,
                        0.0f, 0.85f,
                        0.1f, 1.0f
                    );
            }
            
            info->translation += new_offset;
            info->scale *= new_scale;
            break;
        }
        }
        
    }
    
    //Damage squash and stretch.
    if(
        hasFlag(effects, SPRITE_BMP_EFFECT_DAMAGE) &&
        damageSquashTime > 0.0f
    ) {
        float damage_squash_time_ratio =
            damageSquashTime / MOB::DAMAGE_SQUASH_DURATION;
        float damage_scale_y;
        if(damage_squash_time_ratio > 0.5) {
            damage_scale_y =
                interpolateNumber(
                    damage_squash_time_ratio,
                    0.5f, 1.0f, 0.0f, 1.0f
                );
            damage_scale_y =
                ease(
                    EASE_METHOD_UP_AND_DOWN,
                    damage_scale_y
                );
            damage_scale_y *= MOB::DAMAGE_SQUASH_AMOUNT;
        } else {
            damage_scale_y =
                interpolateNumber(
                    damage_squash_time_ratio,
                    0.0f, 0.5f, 1.0f, 0.0f
                );
            damage_scale_y =
                ease(
                    EASE_METHOD_UP_AND_DOWN,
                    damage_scale_y
                );
            damage_scale_y *= -MOB::DAMAGE_SQUASH_AMOUNT;
        }
        damage_scale_y += 1.0f;
        info->scale.y *= damage_scale_y;
        info->scale.x *= 1.0f / damage_scale_y;
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
 * @param bmp_scale Returns the mob size's scale to apply to the image.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* Mob::getStatusBitmap(float* bmp_scale) const {
    *bmp_scale = 0.0f;
    for(size_t st = 0; st < this->statuses.size(); st++) {
        StatusType* t = this->statuses[st].type;
        if(t->overlayAnimation.empty()) continue;
        Sprite* sp;
        t->overlayAnim.getSpriteData(&sp, nullptr, nullptr);
        if(!sp) return nullptr;
        *bmp_scale = t->overlayAnimMobScale;
        return sp->bitmap;
    }
    return nullptr;
}


/**
 * @brief Handles a status effect being applied.
 *
 * @param sta_type Status type to check.
 */
void Mob::handleStatusEffectGain(StatusType* sta_type) {
    if(sta_type->stateChangeType == STATUS_STATE_CHANGE_CUSTOM) {
        size_t nr = fsm.getStateIdx(sta_type->stateChangeName);
        if(nr != INVALID) {
            fsm.setState(nr);
        }
    }
}


/**
 * @brief Handles a status effect being removed.
 *
 * @param sta_type Status type to check.
 */
void Mob::handleStatusEffectLoss(StatusType* sta_type) {
}


/**
 * @brief Returns whether or not this mob has a clear line towards another mob.
 * In other words, if a straight line is drawn between both,
 * is this line clear, or is it interrupted by a wall or pushing mob?
 *
 * @param target_mob The mob to check against.
 * @return Whether it has a clear line.
 */
bool Mob::hasClearLine(const Mob* target_mob) const {
    //First, get a bounding box of the line to check.
    //This will help with performance later.
    Point bb_tl = pos;
    Point bb_br = pos;
    updateMinMaxCoords(bb_tl, bb_br, target_mob->pos);
    
    const float self_max_z = z + height;
    const float target_mob_max_z = target_mob->z + target_mob->height;
    
    //Check against other mobs.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* m_ptr = game.states.gameplay->mobs.all[m];
        
        if(!m_ptr->type->pushes) continue;
        if(m_ptr == this || m_ptr == target_mob) continue;
        if(hasFlag(m_ptr->flags, MOB_FLAG_INTANGIBLE)) continue;
        
        const float m_ptr_max_z = m_ptr->z + m_ptr->height;
        if(m_ptr_max_z < self_max_z || m_ptr_max_z < target_mob_max_z) continue;
        if(
            m_ptr->z > z + height &&
            m_ptr->z > target_mob->z + target_mob->height
        ) {
            continue;
        }
        if(
            target_mob->standingOnMob == m_ptr &&
            fabs(z - target_mob->z) <= GEOMETRY::STEP_HEIGHT
        ) {
            continue;
        }
        if(
            !rectanglesIntersect(
                bb_tl, bb_br,
                m_ptr->pos - m_ptr->physicalSpan,
                m_ptr->pos + m_ptr->physicalSpan
            )
        ) {
            continue;
        }
        
        if(m_ptr->rectangularDim.x != 0.0f) {
            if(
                lineSegIntersectsRotatedRectangle(
                    pos, target_mob->pos,
                    m_ptr->pos, m_ptr->rectangularDim, m_ptr->angle
                )
            ) {
                return false;
            }
        } else {
            if(
                circleIntersectsLineSeg(
                    m_ptr->pos, m_ptr->radius,
                    pos, target_mob->pos,
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
            pos, target_mob->pos,
            std::min(z + height, target_mob->z + target_mob->height) +
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
 * @param hitbox_idx Index of the hitbox to hold on. INVALID for mob center.
 * @param offset_dist Distance from the hitbox/body center. 1 is full radius.
 * @param offset_angle Hitbox/body angle from which the mob will be held.
 * @param vertical_dist Ratio of distance from the hitbox/body's bottom.
 * 1 is the very top.
 * @param force_above_holder If true, force the mob to be drawn above the holder.
 * @param rotation_method How should the held mob rotate?
 */
void Mob::hold(
    Mob* m, size_t hitbox_idx,
    float offset_dist, float offset_angle,
    float vertical_dist,
    bool force_above_holder, const HOLD_ROTATION_METHOD rotation_method
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
    m->holder.hitboxIdx = hitbox_idx;
    m->holder.offsetDist = offset_dist;
    m->holder.offsetAngle = offset_angle;
    m->holder.verticalDist = vertical_dist;
    m->holder.forceAboveHolder = force_above_holder;
    m->holder.rotationMethod = rotation_method;
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
 * @return Whether it is off-camera.
 */
bool Mob::isOffCamera() const {
    if(parent) return false;
    
    float sprite_bound = 0;
    Sprite* s_ptr;
    anim.getSpriteData(&s_ptr, nullptr, nullptr);
    if(s_ptr) {
        Point sprite_size = s_ptr->bmpSize;
        sprite_bound =
            std::max(
                sprite_size.x / 2.0,
                sprite_size.y / 2.0
            );
    }
    
    float collision_bound = 0;
    if(rectangularDim.x == 0) {
        collision_bound = radius;
    } else {
        collision_bound =
            std::max(
                rectangularDim.x / 2.0,
                rectangularDim.y / 2.0
            );
    }
    
    float radius_to_use = std::max(sprite_bound, collision_bound);
    return !BBoxCheck(game.cam.box[0], game.cam.box[1], pos, radius_to_use);
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
        Point p_delta = p - pos;
        p_delta = rotatePoint(p_delta, -angle);
        p_delta += rectangularDim / 2.0f;
        
        return
            p_delta.x > 0 && p_delta.x < rectangularDim.x &&
            p_delta.y > 0 && p_delta.y < rectangularDim.y;
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
    
    Mob* group_leader = followingGroup;
    
    group_leader->group->members.erase(
        find(
            group_leader->group->members.begin(),
            group_leader->group->members.end(),
            this
        )
    );
    
    group_leader->group->initSpots(this);
    
    group_leader->group->changeStandbyTypeIfNeeded();
    
    followingGroup = nullptr;
    
    game.states.gameplay->updateClosestGroupMembers();
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
 * @param sound_data_idx Index of the sound data in the list.
 * @return The sound source ID.
 */
size_t Mob::playSound(size_t sound_data_idx) {
    if(sound_data_idx >= type->sounds.size()) return 0;
    
    //If the area just loaded, don't play any mob sounds. This allows stuff
    //like obstacles being cleared upon area load and not playing the
    //obstacle clear jingle.
    if(game.states.gameplay->areaTimePassed <= 0.2f) return 0;
    
    MobType::Sound* sound = &type->sounds[sound_data_idx];
    
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
    string team_var;
    
    if(svr.get("team", team_var)) {
        MOB_TEAM team_nr = stringToTeamNr(team_var);
        if(team_nr == INVALID) {
            game.errors.report(
                "Unknown team name \"" + team_var +
                "\", when trying to create mob (" +
                getErrorMessageMobInfo(this) + ")!", nullptr
            );
        } else {
            team = team_nr;
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
        Mob* m_ptr = game.states.gameplay->mobs.all[m];
        if(m_ptr->storedInsideAnother == this) {
            release(m_ptr);
            m_ptr->storedInsideAnother = nullptr;
            m_ptr->timeAlive = 0.0f;
            float a = game.rng.f(0, TAU);
            const float momentum = 100;
            m_ptr->speed.x = cos(a) * momentum;
            m_ptr->speed.y = sin(a) * momentum;
            m_ptr->speedZ = momentum * 7;
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
 * @param pre_named If true, the animation has already been named in-engine.
 * @param mob_speed_anim_baseline If not 0, the animation's speed will depend on
 * the mob's speed, using this value as a baseline (for 1.0x speed).
 */
void Mob::setAnimation(
    size_t idx, const START_ANIM_OPTION options, bool pre_named,
    float mob_speed_anim_baseline
) {
    if(idx >= type->animDb->animations.size()) return;
    
    size_t final_idx;
    if(pre_named) {
        if(anim.animDb->preNamedConversions.size() <= idx) return;
        final_idx = anim.animDb->preNamedConversions[idx];
    } else {
        final_idx = idx;
    }
    
    if(final_idx == INVALID) {
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
    
    Animation* new_anim = anim.animDb->animations[final_idx];
    anim.curAnim = new_anim;
    this->mobSpeedAnimBaseline = mob_speed_anim_baseline;
    
    if(new_anim->frames.empty()) {
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
 * @param mob_speed_anim_baseline If not 0, the animation's speed will depend on
 * the mob's speed, using this value as a baseline (for 1.0x speed).
 */
void Mob::setAnimation(
    const string &name, const START_ANIM_OPTION options,
    float mob_speed_anim_baseline
) {
    size_t idx = anim.animDb->findAnimation(name);
    if(idx != INVALID) {
        setAnimation(idx, options, false, mob_speed_anim_baseline);
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
    float base_nr = 0;
    if(add) base_nr = health;
    
    health = std::clamp(base_nr + change, 0.0f, maxHealth);
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
 * @param rectangular_dim New rectangular dimensions.
 */
void Mob::setRectangularDim(const Point &rectangular_dim) {
    this->rectangularDim = rectangular_dim;
    physicalSpan =
        calculateMobPhysicalSpan(
            radius,
            type->animDb ? type->animDb->hitboxSpan : 0.0f,
            rectangular_dim
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
 * @param type_ptr If nullptr, the pointer to the mob type is obtained given its
 * name in the information structure. If not nullptr, uses this instead.
 * @return The new mob.
 */
Mob* Mob::spawn(const MobType::SpawnInfo* info, MobType* type_ptr) {
    //First, find the mob.
    if(!type_ptr) {
        type_ptr = game.mobCategories.findMobType(info->mobTypeName);
    }
    
    if(!type_ptr) {
        game.errors.report(
            "Mob (" + getErrorMessageMobInfo(this) +
            ") tried to spawn an object of the "
            "type \"" + info->mobTypeName + "\", but there is no such "
            "object type!"
        );
        return nullptr;
    }
    
    if(
        type_ptr->category->id == MOB_CATEGORY_PIKMIN &&
        game.states.gameplay->mobs.pikmin.size() >=
        game.config.rules.maxPikminInField
    ) {
        return nullptr;
    }
    
    Point new_xy;
    float new_z = 0;
    float new_angle = 0;
    
    if(info->relative) {
        new_xy = pos + rotatePoint(info->coordsXY, angle);
        new_z = z + info->coordsZ;
        new_angle = angle + info->angle;
    } else {
        new_xy = info->coordsXY;
        new_z = info->coordsZ;
        new_angle = info->angle;
    }
    
    if(!getSector(new_xy, nullptr, true)) {
        //Spawn out of bounds? No way!
        return nullptr;
    }
    
    Mob* new_mob =
        createMob(
            type_ptr->category,
            new_xy,
            type_ptr,
            new_angle,
            info->vars
        );
        
    new_mob->z = new_z;
    
    if(type_ptr->category->id == MOB_CATEGORY_TREASURES) {
        //This way, treasures that fall into the abyss respawn at the
        //spawner mob's original spot.
        new_mob->home = home;
    } else {
        new_mob->home = new_xy;
    }
    
    if(info->linkObjectToSpawn) {
        links.push_back(new_mob);
    }
    if(info->linkSpawnToObject) {
        new_mob->links.push_back(this);
    }
    if(info->momentum != 0) {
        float a = game.rng.f(0, TAU);
        new_mob->speed.x = cos(a) * info->momentum;
        new_mob->speed.y = sin(a) * info->momentum;
        new_mob->speedZ = info->momentum * 7;
    }
    
    return new_mob;
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
        float distance_fallen = highestMidairZ - z;
        if(distance_fallen > 0.0f) {
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
                        s * distance_fallen *
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
    
    vector<float> pick_random_floats;
    for(size_t f = 0; f < chompingMobs.size(); f++) {
        pick_random_floats.push_back(game.rng.f(0.0f, 1.0f));
    }
    vector<Mob*> shuffled_list =
        shuffleVector(chompingMobs, pick_random_floats);
        
    for(size_t p = 0; p < amount; p++) {
        swallowChompedPikmin(shuffled_list[p]);
    }
}


/**
 * @brief Makes the mob swallow a specific opponent it has chomped on.
 *
 * @param m_ptr Pointer to the chomped mob.
 */
void Mob::swallowChompedPikmin(Mob* m_ptr) {
    if(!m_ptr) return;
    
    size_t idx = INVALID;
    for(size_t m = 0; m < chompingMobs.size(); m++) {
        if(chompingMobs[m] == m_ptr) {
            idx = m;
            break;
        }
    }
    
    if(idx == INVALID) {
        //It's not chomping the mob.
        return;
    }
    
    m_ptr->fsm.runEvent(MOB_EV_SWALLOWED);
    m_ptr->causeSpikeDamage(this, true);
    m_ptr->setHealth(false, false, 0.0f);
    release(m_ptr);
    if(m_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tick(float delta_t) {
    //Since the mob could be marked for deletion after any little
    //interaction with the world, and since doing logic on a mob that already
    //forgot some things due to deletion is dangerous... Let's constantly
    //check if the mob is scheduled for deletion, and bail if so.
    
    if(toDelete) return;
    
    //Brain.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Brain");
    }
    tickBrain(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Physics.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Physics");
    }
    tickPhysics(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Misc. logic.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Misc. logic");
    }
    tickMiscLogic(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Animation.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Animation");
    }
    tickAnimation(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Script.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Script");
    }
    tickScript(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    if(toDelete) return;
    
    //Class specifics.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object -- Misc. specifics");
    }
    tickClassSpecifics(delta_t);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
}


/**
 * @brief Ticks animation time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tickAnimation(float delta_t) {
    float mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); s++) {
        float vuln_mult = this->statuses[s].type->animSpeedMultiplier - 1.0f;
        auto vuln_it = type->statusVulnerabilities.find(statuses[s].type);
        if(vuln_it != type->statusVulnerabilities.end()) {
            vuln_mult *= vuln_it->second.effectMult;
        }
        mult *= (vuln_mult + 1.0f);
    }
    
    if(mobSpeedAnimBaseline != 0.0f) {
        float mob_speed_mult = chaseInfo.curSpeed / mobSpeedAnimBaseline;
        mob_speed_mult =
            std::clamp(
                mob_speed_mult,
                MOB::MOB_SPEED_ANIM_MIN_MULT, MOB::MOB_SPEED_ANIM_MAX_MULT
            );
        mult *= mob_speed_mult;
    }
    
    vector<size_t> frame_signals;
    vector<size_t> frame_sounds;
    bool finished_anim =
        anim.tick(delta_t* mult, &frame_signals, &frame_sounds);
        
    if(finished_anim) {
        fsm.runEvent(MOB_EV_ANIMATION_END);
    }
    for(size_t s = 0; s < frame_signals.size(); s++) {
        fsm.runEvent(MOB_EV_FRAME_SIGNAL, &frame_signals[s]);
    }
    for(size_t s = 0; s < frame_sounds.size(); s++) {
        playSound(frame_sounds[s]);
    }
    
    for(size_t h = 0; h < hitOpponents.size();) {
        hitOpponents[h].first -= delta_t;
        if(hitOpponents[h].first <= 0.0f) {
            hitOpponents.erase(hitOpponents.begin() + h);
        } else {
            h++;
        }
    }
    
    if(parent && parent->limb_anim.animDb) {
        parent->limb_anim.tick(delta_t* mult);
    }
}


/**
 * @brief Ticks the mob's brain for the next frame.
 *
 * This has nothing to do with the mob's individual script.
 * This is related to mob-global things, like
 * thinking about where to move next and such.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tickBrain(float delta_t) {
    //Circling around something.
    if(circlingInfo) {
        Point circling_center =
            circlingInfo->circlingMob ?
            circlingInfo->circlingMob->pos :
            circlingInfo->circlingPoint;
        float circling_z =
            circlingInfo->circlingMob ?
            circlingInfo->circlingMob->z :
            z;
            
        circlingInfo->curAngle +=
            linearDistToAngular(
                circlingInfo->speed * delta_t, circlingInfo->radius
            ) *
            (circlingInfo->clockwise ? 1 : -1);
            
        chase(
            circling_center + angleToCoordinates(
                circlingInfo->curAngle, circlingInfo->radius
            ),
            circling_z,
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
        Point final_target_pos = getChaseTarget();
        Distance horiz_dist = Distance(pos, final_target_pos);
        float vert_dist = 0.0f;
        if(hasFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            float final_target_z = chaseInfo.offsetZ;
            if(chaseInfo.origZ) final_target_z += *chaseInfo.origZ;
            vert_dist = fabs(z - final_target_z);
        }
        
        if(
            horiz_dist > chaseInfo.targetDist ||
            vert_dist > 1.0f
        ) {
            //If it still hasn't reached its target
            //(or close enough to the target),
            //time to make it think about how to get there.
            
            //Let the mob think about facing the actual target.
            if(!type->canFreeMove && horiz_dist > 0.0f) {
                face(getAngle(pos, final_target_pos), nullptr);
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
                pathInfo->block_reason == PATH_BLOCK_REASON_NONE
            ) {
            
                pathInfo->cur_path_stop_idx++;
                
                if(pathInfo->cur_path_stop_idx < pathInfo->path.size()) {
                    //Reached a regular stop while traversing the path.
                    //Think about going to the next, if possible.
                    if(pathInfo->checkBlockage(&pathInfo->block_reason)) {
                        //Oop, there's an obstacle! Or some other blockage.
                        fsm.runEvent(MOB_EV_PATH_BLOCKED);
                    } else {
                        //All good. Head to the next stop.
                        PathStop* next_stop =
                            pathInfo->path[pathInfo->cur_path_stop_idx];
                        float next_stop_z = z;
                        if(
                            (
                                pathInfo->settings.flags &
                                PATH_FOLLOW_FLAG_AIRBORNE
                            ) &&
                            next_stop->sectorPtr
                        ) {
                            next_stop_z =
                                next_stop->sectorPtr->z +
                                PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT;
                        }
                        
                        chase(
                            next_stop->pos, next_stop_z,
                            CHASE_FLAG_ANY_ANGLE,
                            PATHS::DEF_CHASE_TARGET_DISTANCE,
                            chaseInfo.maxSpeed
                        );
                    }
                    
                } else if(
                    pathInfo->cur_path_stop_idx == pathInfo->path.size()
                ) {
                    //Reached the final stop of the path, but not the goal.
                    //Let's head there.
                    moveToPathEnd(
                        chaseInfo.maxSpeed, chaseInfo.acceleration
                    );
                    
                } else if(
                    pathInfo->cur_path_stop_idx == pathInfo->path.size() + 1
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tickClassSpecifics(float delta_t) {
}


/**
 * @brief Performs some logic code for this game frame.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tickMiscLogic(float delta_t) {
    if(timeAlive == 0.0f) {
        //This is a convenient spot to signal that the mob is ready.
        //This will only run once, and only after the mob is all set up.
        fsm.runEvent(MOB_EV_ON_READY);
    }
    timeAlive += delta_t;
    
    invulnPeriod.tick(delta_t);
    
    for(size_t s = 0; s < this->statuses.size(); s++) {
        statuses[s].tick(delta_t);
        
        float damage_mult = 1.0f;
        auto vuln_it = type->statusVulnerabilities.find(statuses[s].type);
        if(vuln_it != type->statusVulnerabilities.end()) {
            damage_mult = vuln_it->second.effectMult;
        }
        
        float health_before = health;
        
        if(statuses[s].type->healthChange != 0.0f) {
            setHealth(
                true, false,
                statuses[s].type->healthChange * damage_mult * delta_t
            );
        }
        if(statuses[s].type->healthChangeRatio != 0.0f) {
            setHealth(
                true, true,
                statuses[s].type->healthChangeRatio * damage_mult * delta_t
            );
        }
        
        if(health <= 0.0f && health_before > 0.0f) {
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
            delta_t, game.states.gameplay->particles
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
    bool should_show_health =
        type->showHealth &&
        !hasFlag(flags, MOB_FLAG_HIDDEN) &&
        health > 0.0f &&
        health < maxHealth;
    if(!healthWheel && should_show_health) {
        healthWheel = new InWorldHealthWheel(this);
    } else if(healthWheel && !should_show_health) {
        healthWheel->startFading();
    }
    
    if(healthWheel) {
        healthWheel->tick(delta_t);
        if(healthWheel->toDelete) {
            delete healthWheel;
            healthWheel = nullptr;
        }
    }
    
    //Fraction numbers.
    float fraction_value_nr = 0.0f;
    float fraction_req_nr = 0.0f;
    ALLEGRO_COLOR fraction_color = COLOR_BLACK;
    bool should_show_fraction =
        getFractionNumbersInfo(
            &fraction_value_nr, &fraction_req_nr, &fraction_color
        );
        
    if(!fraction && should_show_fraction) {
        fraction = new InWorldFraction(this);
    } else if(fraction && !should_show_fraction) {
        fraction->startFading();
    }
    
    if(fraction) {
        fraction->tick(delta_t);
        if(should_show_fraction) {
            //Only update the numbers if we want to show a fraction, i.e.
            //if we actually KNOW the numbers. Otherwise, keep the old data.
            fraction->setColor(fraction_color);
            fraction->setValueNumber(fraction_value_nr);
            fraction->setRequirementNumber(fraction_req_nr);
        }
        if(fraction->toDelete) {
            delete fraction;
            fraction = nullptr;
        }
    }
    
    //Group stuff.
    if(group && group->members.size()) {
    
        Group::MODE old_mode = group->mode;
        bool is_holding = !holding.empty();
        bool is_far_from_group =
            Distance(group->getAverageMemberPos(), pos) >
            MOB::GROUP_SHUFFLE_DIST + (group->radius + radius);
        bool is_swarming =
            game.states.gameplay->swarmMagnitude &&
            game.states.gameplay->curLeaderPtr == this;
            
        //Find what mode we're in on this frame.
        if(is_swarming) {
            group->mode = Group::MODE_SWARM;
        } else if(is_holding || is_far_from_group) {
            group->mode = Group::MODE_FOLLOW_BACK;
        } else {
            group->mode = Group::MODE_SHUFFLE;
        }
        
        //Change things depending on the mode.
        switch(group->mode) {
        case Group::MODE_FOLLOW_BACK: {
    
            //Follow the leader's back.
            group->anchorAngle = angle + TAU / 2.0f;
            Point new_anchor_rel_pos =
                rotatePoint(
                    Point(radius + MOB::GROUP_SPOT_INTERVAL * 2.0f, 0.0f),
                    group->anchorAngle
                );
            group->anchor = pos + new_anchor_rel_pos;
            
            al_identity_transform(&group->transform);
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
            
        } case Group::MODE_SHUFFLE: {
    
            //Casually shuffle with the leader, if needed.
            Point mov;
            Point group_mid_point =
                group->anchor +
                rotatePoint(
                    Point(group->radius, 0.0f),
                    group->anchorAngle
                );
            movePoint(
                group_mid_point,
                pos,
                type->moveSpeed,
                group->radius + radius + MOB::GROUP_SPOT_INTERVAL * 2.0f,
                &mov,
                nullptr, nullptr, delta_t
            );
            group->anchor += mov * delta_t;
            
            al_identity_transform(&group->transform);
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
            
        } case Group::MODE_SWARM: {
    
            //Swarming.
            group->anchorAngle = game.states.gameplay->swarmAngle;
            Point new_anchor_rel_pos =
                rotatePoint(
                    Point(radius + MOB::GROUP_SPOT_INTERVAL * 2.0f, 0.0f),
                    group->anchorAngle
                );
            group->anchor = pos + new_anchor_rel_pos;
            
            float intensity_dist =
                game.config.rules.cursorMaxDist *
                game.states.gameplay->swarmMagnitude;
            al_identity_transform(&group->transform);
            al_translate_transform(
                &group->transform, -MOB::SWARM_MARGIN, 0
            );
            al_scale_transform(
                &group->transform,
                intensity_dist / (group->radius * 2),
                1 -
                (
                    MOB::SWARM_VERTICAL_SCALE*
                    game.states.gameplay->swarmMagnitude
                )
            );
            al_rotate_transform(
                &group->transform, group->anchorAngle + TAU / 2.0f
            );
            break;
        }
        }
        
        if(
            old_mode != Group::MODE_SHUFFLE &&
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
        damageSquashTime -= delta_t;
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Mob::tickScript(float delta_t) {
    if(!fsm.curState) return;
    
    //Timer events.
    MobEvent* timer_ev = fsm.getEvent(MOB_EV_TIMER);
    if(scriptTimer.duration > 0) {
        if(scriptTimer.timeLeft > 0) {
            scriptTimer.tick(delta_t);
            if(scriptTimer.timeLeft == 0.0f && timer_ev) {
                timer_ev->run(this);
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
            MobEvent* for_ev = fsm.getEvent(MOB_EV_FOCUS_OFF_REACH);
            
            if(farReach != INVALID && for_ev) {
                float angle_to_focus = getAngle(pos, focus->pos);
                if(
                    !isMobInReach(
                        &type->reaches[farReach],
                        getDistanceBetween(focusedMob),
                        getAngleSmallestDiff(angle, angle_to_focus)
                    )
                ) {
                    for_ev->run(this);
                }
                
            }
        }
        
    }
    
    //Itch event.
    if(type->itchDamage > 0 || type->itchTime > 0) {
        itchTime += delta_t;
        MobEvent* itch_ev = fsm.getEvent(MOB_EV_ITCH);
        if(
            itch_ev &&
            itchDamage > type->itchDamage && itchTime > type->itchTime
        ) {
            itch_ev->run(this);
            itchDamage = 0;
            itchTime = 0;
        }
    }
    
    //Health regeneration.
    if(health > 0) {
        setHealth(true, false, type->healthRegen * delta_t);
    }
    
    //Check if it got whistled.
    if(
        game.states.gameplay->curLeaderPtr &&
        game.states.gameplay->whistle.whistling &&
        Distance(pos, game.states.gameplay->whistle.center) <=
        game.states.gameplay->whistle.radius
    ) {
        fsm.runEvent(
            MOB_EV_WHISTLED, (void*) game.states.gameplay->curLeaderPtr
        );
        
        bool saved_by_whistle = false;
        for(size_t s = 0; s < statuses.size(); s++) {
            if(statuses[s].type->removableWithWhistle) {
                statuses[s].toDelete = true;
                if(
                    statuses[s].type->healthChange < 0.0f ||
                    statuses[s].type->healthChangeRatio < 0.0f
                ) {
                    saved_by_whistle = true;
                }
            }
        }
        deleteOldStatusEffects();
        
        if(saved_by_whistle && type->category->id == MOB_CATEGORY_PIKMIN) {
            game.statistics.pikminSaved++;
        }
    }
    
    //Following a leader.
    if(followingGroup) {
        MobEvent* spot_far_ev =  fsm.getEvent(MOB_EV_SPOT_IS_FAR);
        
        if(spot_far_ev) {
            Point target_pos;
            float target_dist;
            
            getGroupSpotInfo(&target_pos, &target_dist);
            
            Distance d(pos, target_pos);
            if(d > target_dist) {
                spot_far_ev->run(this, (void*) &target_pos);
            }
        }
    }
    
    //Far away from home.
    MobEvent* far_from_home_ev = fsm.getEvent(MOB_EV_FAR_FROM_HOME);
    if(far_from_home_ev) {
        Distance d(pos, home);
        if(d >= type->territoryRadius) {
            far_from_home_ev->run(this);
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
    trackInfo->cur_cp_progress +=
        trackInfo->ride_speed * game.deltaT;
        
    if(trackInfo->cur_cp_progress >= 1.0f) {
        //Next checkpoint.
        trackInfo->cur_cp_idx++;
        trackInfo->cur_cp_progress -= 1.0f;
        
        if(
            trackInfo->cur_cp_idx ==
            trackInfo->checkpoints.size() - 1
        ) {
            stopTrackRide();
            return true;
        }
    }
    
    //Teleport to the right spot.
    Hitbox* cur_cp =
        trackInfo->m->getHitbox(
            trackInfo->checkpoints[trackInfo->cur_cp_idx]
        );
    Hitbox* next_cp =
        trackInfo->m->getHitbox(
            trackInfo->checkpoints[trackInfo->cur_cp_idx + 1]
        );
    Point cur_cp_pos =
        cur_cp->getCurPos(trackInfo->m->pos, trackInfo->m->angle);
    Point next_cp_pos =
        next_cp->getCurPos(trackInfo->m->pos, trackInfo->m->angle);
        
    Point dest_xy(
        interpolateNumber(
            trackInfo->cur_cp_progress, 0.0f, 1.0f,
            cur_cp_pos.x, next_cp_pos.x
        ),
        interpolateNumber(
            trackInfo->cur_cp_progress, 0.0f, 1.0f,
            cur_cp_pos.y, next_cp_pos.y
        )
    );
    
    float dest_z =
        interpolateNumber(
            trackInfo->cur_cp_progress, 0.0f, 1.0f,
            trackInfo->m->z + cur_cp->z,
            trackInfo->m->z + next_cp->z
        );
        
    float dest_angle = getAngle(cur_cp_pos, next_cp_pos);
    
    chase(dest_xy, dest_z, CHASE_FLAG_TELEPORT);
    face(dest_angle, nullptr);
    
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
 * @param base_anim_idx Base animation index.
 * @param group_idx Group it belongs to.
 * @param base_anim_total Total index of base animations.
 * @return The index.
 */
size_t MobWithAnimGroups::getAnimationIdxFromBaseAndGroup(
    size_t base_anim_idx, size_t group_idx,
    size_t base_anim_total
) const {
    return group_idx * base_anim_total + base_anim_idx;
}
