/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin class and Pikmin-related functions.
 */

#include <algorithm>

#include "pikmin.h"

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"
#include "../mob_script/pikmin_fsm.h"
#include "mob_enums.h"
#include "mob.h"


namespace PIKMIN {

//Maximum amount of time for the random boredom animation delay.
const float BORED_ANIM_MAX_DELAY = 5.0f;

//Minimum amount of time for hte random boredom animation delay.
const float BORED_ANIM_MIN_DELAY = 1.0f;

//Chance of circling the opponent instead of striking, when grounded.
const float CIRCLE_OPPONENT_CHANCE_GROUNDED = 0.2f;

//Chance of circling the opponent instead of latching, if it can latch.
const float CIRCLE_OPPONENT_CHANCE_PRE_LATCH = 0.5f;

//Time until moving Pikmin timeout and stay in place, after being dismissed.
const float DISMISS_TIMEOUT = 4.0f;

//Height above the floor that a flying Pikmin prefers to stay at.
const float FLIER_ABOVE_FLOOR_HEIGHT = 55.0f;

//Timeout before a Pikmin gives up, when ordered to go to something.
const float GOTO_TIMEOUT = 5.0f;

//If the Pikmin is within this distance of the mob, it can ground attack.
const float GROUNDED_ATTACK_DIST = 5.0f;

//The idle glow spins these many radians per second.
const float IDLE_GLOW_SPIN_SPEED = TAU / 4;

//Invulnerability period after getting hit.
const float INVULN_PERIOD = 0.7f;

//How long to remember a missed incoming attack for.
const float MISSED_ATTACK_DURATION = 1.5f;

//Interval for when a Pikmin decides a new chase spot, when panicking.
const float PANIC_CHASE_INTERVAL = 0.2f;

//A plucked Pikmin is thrown behind the leader at this speed, horizontally.
const float THROW_HOR_SPEED = 80.0f;

//A plucked Pikmin is thrown behind the leader at this speed, vertically.
const float THROW_VER_SPEED = 900.0f;

}


/**
 * @brief Constructs a new Pikmin object.
 *
 * @param pos Starting coordinates.
 * @param type Pikmin type this mob belongs to.
 * @param angle Starting angle.
 */
Pikmin::Pikmin(const Point &pos, PikminType* type, float angle) :
    Mob(pos, type, angle),
    pikType(type) {
    
    invulnPeriod = Timer(PIKMIN::INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1;
    subgroupTypePtr =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pikType
        );
    nearReach = 0;
    farReach = 2;
    updateInteractionSpan();
    missedAttackTimer =
        Timer(
            PIKMIN::MISSED_ATTACK_DURATION,
    [this] () { this->missedAttackPtr = nullptr; }
        );
        
    if(pikType->canFly) {
        enableFlag(flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Returns whether or not a Pikmin can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive it.
 */
bool Pikmin::canReceiveStatus(StatusType* s) const {
    return hasFlag(s->affects, STATUS_AFFECTS_FLAG_PIKMIN);
}


/**
 * @brief Draws a Pikmin, including its leaf/bud/flower, idle glow, etc.
 */
void Pikmin::drawMob() {
    Sprite* curSPtr;
    Sprite* nextSPtr;
    float interpolationFactor;
    getSpriteData(&curSPtr, &nextSPtr, &interpolationFactor);
    if(!curSPtr) return;
    
    //The Pikmin itself.
    BitmapEffect mobEff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &mobEff,
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    BitmapEffect pikSpriteEff = mobEff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &pikSpriteEff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD
    );
    
    bool isIdle =
        fsm.curState->id == PIKMIN_STATE_IDLING ||
        fsm.curState->id == PIKMIN_STATE_IDLING_H ||
        fsm.curState->id == PIKMIN_STATE_SPROUT;
        
    if(isIdle) {
        mobEff.glowColor = COLOR_WHITE;
    }
    
    drawBitmapWithEffects(curSPtr->bitmap, pikSpriteEff);
    
    //Top.
    if(curSPtr->topVisible) {
        Point topCoords;
        float topAngle;
        Point topSize;
        BitmapEffect topEff = mobEff;
        ALLEGRO_BITMAP* topBmp = pikType->bmpTop[maturity];
        getSpriteBasicTopEffects(
            curSPtr, nextSPtr, interpolationFactor,
            &topCoords, &topAngle, &topSize
        );
        //To get the height effect to work, we'll need to scale the translation
        //too, otherwise the top will detach from the Pikmin visually as
        //the Pikmin falls into a pit. The "right" scale is a bit of a guess
        //at this point in the code, but honestly, either X scale or Y scale
        //will work. In the off-chance they are different, using an average
        //will be more than enough.
        float avgScale = (topEff.scale.x + topEff.scale.y) / 2.0f;
        Point topBmpSize = getBitmapDimensions(topBmp);
        topEff.translation +=
            pos + rotatePoint(topCoords, angle) * avgScale;
        topEff.scale *= topSize / topBmpSize;
        topEff.rotation +=
            angle + topAngle;
        topEff.glowColor =
            mapGray(0);
            
        drawBitmapWithEffects(topBmp, topEff);
    }
    
    //Idle glow.
    if(isIdle) {
        BitmapEffect idleEff = pikSpriteEff;
        Point glowBmpSize =
            getBitmapDimensions(game.sysContent.bmpIdleGlow);
        idleEff.translation = pos;
        idleEff.scale =
            (game.config.pikmin.standardRadius * 8) / glowBmpSize;
        idleEff.rotation =
            game.states.gameplay->areaTimePassed *
            PIKMIN::IDLE_GLOW_SPIN_SPEED;
        idleEff.tintColor = type->mainColor;
        idleEff.glowColor = mapGray(0);
        
        drawBitmapWithEffects(game.sysContent.bmpIdleGlow, idleEff);
    }
    
    drawStatusEffectBmp(this, pikSpriteEff);
}


/**
 * @brief Logic specific to Pikmin for when they finish dying.
 */
void Pikmin::finishDyingClassSpecifics() {
    //Essentials.
    toDelete = true;
    
    //Soul.
    Particle par(
        pos, LARGE_FLOAT,
        radius * 2, 2.0f
    );
    par.bitmap = game.sysContent.bmpPikminSpirit;
    par.friction = 0.8;
    Point baseSpeed = Point(game.rng.f(-20, 20), game.rng.f(-70, -30));
    par.linearSpeed = KeyframeInterpolator<Point>(baseSpeed);
    par.linearSpeed.add(1, Point(Point(baseSpeed.x, baseSpeed.y - 20)));
    par.color.setKeyframeValue(0, changeAlpha(pikType->mainColor, 0));
    par.color.add(0.1f, pikType->mainColor);
    par.color.add(1, changeAlpha(pikType->mainColor, 0));
    game.states.gameplay->particles.add(par);
    
    //Sound. Create a positional sound source instead of a mob sound source,
    //since the Pikmin object is now practically deleted.
    size_t dyingSoundIdx =
        pikType->soundDataIdxs[PIKMIN_SOUND_DYING];
    if(dyingSoundIdx != INVALID) {
        MobType::Sound* dyingSound =
            &type->sounds[dyingSoundIdx];
        game.audio.createPosSoundSource(
            dyingSound->sample,
            pos, false, dyingSound->config
        );
    }
}


/**
 * @brief Forces the Pikmin to start carrying the given mob.
 * This quickly runs over several steps in the usual FSM logic, just to
 * instantly get to the end result.
 * As such, be careful when using it.
 *
 * @param m The mob to carry.
 */
void Pikmin::forceCarry(Mob* m) {
    fsm.setState(PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT, (void*) m, nullptr);
    fsm.runEvent(MOB_EV_REACHED_DESTINATION);
}


/**
 * @brief Returns a Pikmin's base speed, without status effects and the like.
 * This depends on the maturity.
 *
 * @return The base speed.
 */
float Pikmin::getBaseSpeed() const {
    float base = Mob::getBaseSpeed();
    return base + (base * this->maturity * game.config.pikmin.maturitySpeedMult);
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param outSpot The final coordinates are returned here.
 * @param outDist The final distance to those coordinates is returned here.
 */
void Pikmin::getGroupSpotInfo(Point* outSpot, float* outDist) const {
    outSpot->x = 0.0f;
    outSpot->y = 0.0f;
    *outDist = 0.0f;
    
    if(!followingGroup || !followingGroup->group) {
        return;
    }
    
    *outSpot =
        followingGroup->group->anchor +
        followingGroup->group->getSpotOffset(groupSpotIdx);
    *outDist = 5.0f;
}    


/**
 * @brief Returns the task range for whether the Pikmin is idling or
 * swarming.
 * 
 * @return The range.
 */
float Pikmin::getTaskRange() const {
    float result = game.config.pikmin.idleTaskRange;
    if(!followingGroup) return result;
    if(followingGroup->type->category->id != MOB_CATEGORY_LEADERS) {
        return result;
    }
    Leader* leaPtr = (Leader*) followingGroup;
    if(!leaPtr->player) return result;
    if(leaPtr->player->swarmMagnitude == 0.0f) return result;

    result = game.config.pikmin.swarmTaskRange;
    return result;
}

/**
 * @brief Handles a status effect being applied.
 *
 * @param staType Status effect to handle.
 */
void Pikmin::handleStatusEffectGain(StatusType* staType) {
    Mob::handleStatusEffectGain(staType);
    
    switch(staType->stateChangeType) {
    case STATUS_STATE_CHANGE_FLAILING: {
        fsm.setState(PIKMIN_STATE_FLAILING);
        break;
    }
    case STATUS_STATE_CHANGE_HELPLESS: {
        fsm.setState(PIKMIN_STATE_HELPLESS);
        break;
    }
    case STATUS_STATE_CHANGE_PANIC: {
        fsm.setState(PIKMIN_STATE_PANICKING);
        break;
    }
    default: {
        break;
    }
    }
    
    increaseMaturity(staType->maturityChangeAmount);
    
    if(carryingMob) {
        carryingMob->chaseInfo.maxSpeed =
            carryingMob->carryInfo->getSpeed();
    }
}


/**
 * @brief Handles a status effect being removed.
 *
 * @param staType Status effect to handle.
 */
void Pikmin::handleStatusEffectLoss(StatusType* staType) {
    bool stillHasFlailing = false;
    bool stillHasHelplessness = false;
    bool stillHasPanic = false;
    for(size_t s = 0; s < statuses.size(); s++) {
        if(statuses[s].type == staType) continue;
        
        switch(statuses[s].type->stateChangeType) {
        case STATUS_STATE_CHANGE_FLAILING: {
            stillHasFlailing = true;
            break;
        }
        case STATUS_STATE_CHANGE_HELPLESS: {
            stillHasHelplessness = true;
            break;
        }
        case STATUS_STATE_CHANGE_PANIC: {
            stillHasPanic = true;
            break;
        }
        default: {
            break;
        }
        }
    }
    
    if(
        staType->stateChangeType == STATUS_STATE_CHANGE_FLAILING &&
        !stillHasFlailing &&
        fsm.curState->id == PIKMIN_STATE_FLAILING
    ) {
        fsm.setState(PIKMIN_STATE_IDLING);
        setAnimation(PIKMIN_ANIM_SHAKING);
        inShakingAnimation = true;
        setTimer(0); //The boredom animation timeout.
        PikminFsm::standStill(this, nullptr, nullptr);
        invulnPeriod.start();
    }
    
    if(
        staType->stateChangeType == STATUS_STATE_CHANGE_HELPLESS &&
        !stillHasHelplessness &&
        fsm.curState->id == PIKMIN_STATE_HELPLESS
    ) {
        fsm.setState(PIKMIN_STATE_IDLING);
        PikminFsm::standStill(this, nullptr, nullptr);
        invulnPeriod.start();
        
    } else if(
        staType->stateChangeType == STATUS_STATE_CHANGE_PANIC &&
        !stillHasPanic &&
        fsm.curState->id == PIKMIN_STATE_PANICKING
    ) {
        fsm.setState(PIKMIN_STATE_IDLING);
        PikminFsm::standStill(this, nullptr, nullptr);
        invulnPeriod.start();
        
    }
    
    if(carryingMob) {
        carryingMob->chaseInfo.maxSpeed =
            carryingMob->carryInfo->getSpeed();
    }
}


/**
 * @brief Increases (or decreases) the Pikmin's maturity by the given amount.
 * This makes sure that the maturity doesn't overflow.
 *
 * @param amount Amount to increase by.
 * @return Whether it changed the maturity.
 */
bool Pikmin::increaseMaturity(int amount) {
    int oldMaturity = maturity;
    int newMaturity = maturity + amount;
    maturity = std::clamp(newMaturity, 0, (int) (N_MATURITIES - 1));
    
    if(maturity > oldMaturity) {
        game.statistics.pikminBlooms++;
    }
    return maturity != oldMaturity;
}


/**
 * @brief Latches on to the specified mob.
 *
 * @param m Mob to latch on to.
 * @param h Hitbox to latch on to.
 */
void Pikmin::latch(Mob* m, const Hitbox* h) {
    speed.x = speed.y = speedZ = 0;
    
    //Shuffle it slightly, randomly, so that multiple Pikmin thrown
    //at the exact same spot aren't perfectly overlapping each other.
    pos.x += game.rng.f(-2.0f, 2.0f);
    pos.y += game.rng.f(-2.0f, 2.0f);
    
    float hOffsetDist;
    float hOffsetAngle;
    float vOffsetDist;
    m->getHitboxHoldPoint(
        this, h, &hOffsetDist, &hOffsetAngle, &vOffsetDist
    );
    m->hold(
        this, h->bodyPartIdx, hOffsetDist, hOffsetAngle, vOffsetDist,
        true,
        HOLD_ROTATION_METHOD_NEVER //PikminFsm::prepareToAttack handles it.
    );
    
    latched = true;
}


/**
 * @brief Checks if an incoming attack should miss, and returns the result.
 *
 * If it was already decided that it missed in a previous frame, that's a
 * straight no. If not, it will roll with the hit rate to check.
 * If the attack is a miss, it also registers the miss, so that we can keep
 * memory of it for the next frames.
 *
 * @param info Info about the hitboxes involved.
 * @return Whether it hit.
 */
bool Pikmin::processAttackMiss(HitboxInteraction* info) {
    if(info->mob2->anim.curAnim == missedAttackPtr) {
        //In a previous frame, we had already considered this animation a miss.
        return false;
    }
    
    unsigned char hitRate = info->mob2->anim.curAnim->hitRate;
    if(hitRate == 0) return false;
    
    unsigned char hitRoll = game.rng.i(0, 100);
    if(hitRoll > hitRate) {
        //This attack was randomly decided to be a miss.
        //Record this animation so it won't be considered a hit next frame.
        missedAttackPtr = info->mob2->anim.curAnim;
        missedAttackTimer.start();
        return false;
    }
    
    return true;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Pikmin::readScriptVars(const ScriptVarReader &svr) {
    Mob::readScriptVars(svr);
    
    int maturityVar;
    bool sproutVar;
    bool followLinkVar;
    
    if(svr.get("maturity", maturityVar)) {
        maturity = std::clamp(maturityVar, 0, (int) (N_MATURITIES - 1));
    }
    if(svr.get("sprout", sproutVar)) {
        if(sproutVar) {
            fsm.firstStateOverride = PIKMIN_STATE_SPROUT;
        }
    }
    if(svr.get("follow_link_as_leader", followLinkVar)) {
        if(followLinkVar) {
            mustFollowLinkAsLeader = true;
        }
    }
}


/**
 * @brief Sets up stuff for the beginning of the Pikmin's death process.
 */
void Pikmin::startDyingClassSpecifics() {
    game.states.gameplay->pikminDeaths++;
    game.states.gameplay->pikminDeathsPerType[pikType]++;
    game.states.gameplay->lastPikminDeathPos = pos;
    game.statistics.pikminDeaths++;
    
    enableFlag(flags, MOB_FLAG_INTANGIBLE);
}


/**
 * @brief Starts the particle generator that leaves a trail behind
 * a thrown Pikmin.
 */
void Pikmin::startThrowTrail() {
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parThrowTrail, this
        );
    pg.followZOffset = 0.0f;
    adjustKeyframeInterpolatorValues<float>(
        pg.baseParticle.size,
    [ = ] (const float & f) { return f * radius; }
    );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        pg.baseParticle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR newColor = c;
        newColor.r *= type->mainColor.r;
        newColor.g *= type->mainColor.g;
        newColor.b *= type->mainColor.b;
        newColor.a *= type->mainColor.a;
        return newColor;
    }
    );
    pg.id = MOB_PARTICLE_GENERATOR_ID_THROW;
    particleGenerators.push_back(pg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Pikmin::tickClassSpecifics(float deltaT) {
    //Carrying object.
    if(carryingMob) {
        if(!carryingMob->carryInfo) {
            fsm.runEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
        }
    }
    
    //Tick some timers.
    missedAttackTimer.tick(deltaT);
    bumpLock = std::max(bumpLock - deltaT, 0.0f);
    
    //Forcefully follow another mob as a leader.
    if(mustFollowLinkAsLeader && !link_anon_size == 0 && links["0"]) {
        Mob* leader = links["0"];
        fsm.runEvent(
            MOB_EV_TOUCHED_ACTIVE_LEADER,
            (void*) (leader),
            (void*) 1 //Be silent.
        );
        
        //If the leader is an enemy, let's keep these Pikmin safe.
        if(leader->type->category->id == MOB_CATEGORY_ENEMIES) {
            enableFlag(flags, MOB_FLAG_NON_HUNTABLE);
            enableFlag(flags, MOB_FLAG_NON_HURTABLE);
        }
        mustFollowLinkAsLeader = false;
    }
    
}


/**
 * @brief Returns the sprout closest to a leader. Used when auto-plucking.
 *
 * @param pos Coordinates of the leader.
 * @param d Variable to return the distance to. nullptr for none.
 * @param ignoreReserved If true, ignore any sprouts that are "reserved"
 * (i.e. already chosen to be plucked by another leader).
 * @return The sprout.
 */
Pikmin* getClosestSprout(
    const Point &pos, Distance* d, bool ignoreReserved
) {
    Distance closestDist;
    Pikmin* closestPikmin = nullptr;
    
    size_t nPikmin = game.states.gameplay->mobs.pikmin.size();
    for(size_t p = 0; p < nPikmin; p++) {
        if(
            game.states.gameplay->mobs.pikmin[p]->fsm.curState->id !=
            PIKMIN_STATE_SPROUT
        ) {
            continue;
        }
        
        Distance dis(pos, game.states.gameplay->mobs.pikmin[p]->pos);
        if(closestPikmin == nullptr || dis < closestDist) {
        
            if(
                !(
                    ignoreReserved ||
                    game.states.gameplay->mobs.pikmin[p]->pluckReserved
                )
            ) {
                closestDist = dis;
                closestPikmin = game.states.gameplay->mobs.pikmin[p];
            }
        }
    }
    
    if(d) *d = closestDist;
    return closestPikmin;
}
