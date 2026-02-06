/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin class and Pikmin-related functions.
 */

#pragma once

class Leader;

#include "../mob_type/pikmin_type.h"
#include "enemy.h"
#include "leader.h"
#include "onion.h"


namespace PIKMIN {
extern const float BORED_ANIM_MAX_DELAY;
extern const float BORED_ANIM_MIN_DELAY;
extern const float CIRCLE_OPPONENT_CHANCE_GROUNDED;
extern const float CIRCLE_OPPONENT_CHANCE_PRE_LATCH;
extern const float DISMISS_TIMEOUT;
extern const float FLIER_ABOVE_FLOOR_HEIGHT;
extern const float GOTO_TIMEOUT;
extern const float GROUNDED_ATTACK_DIST;
extern const float IDLE_GLOW_SPIN_SPEED;
extern const float INVULN_PERIOD;
extern const float MISSED_ATTACK_DURATION;
extern const float PANIC_CHASE_INTERVAL;
extern const float THROW_HOR_SPEED;
extern const float THROW_VER_SPEED;
}


/**
 * @brief The eponymous Pikmin.
 */
class Pikmin : public Mob {

public:

    //--- Public members ---
    
    //What type of Pikmin it is.
    PikminType* pikType = nullptr;
    
    //Mob that it is carrying.
    Mob* carryingMob = nullptr;
    
    //The Pikmin is considering this attack animation as having "missed".
    Animation* missedAttackPtr = nullptr;
    
    //The Pikmin will consider the miss for this long.
    Timer missedAttackTimer;
    
    //The Pikmin will automatically maturate one stage after this long.
    Timer autoMaturateTimer;
    
    //Did the Pikmin's last attack cause zero damage?
    bool wasLastHitDing = false;
    
    //How many hits in a row have done no damage.
    unsigned char consecutiveDings = 0;
    
    //Maturity. 0: leaf. 1: bud. 2: flower.
    unsigned char maturity = 2;
    
    //Is this Pikmin currently a seed or a sprout?
    bool isSeedOrSprout = false;
    
    //Is this Pikmin currently grabbed by an enemy?
    bool isGrabbedByEnemy = false;
    
    //If true, someone's already coming to pluck this Pikmin.
    //This is to let other leaders know that they should pick a different one.
    bool pluckReserved = false;
    
    //Leader it is meant to return to after what it is doing, if any.
    Mob* leaderToReturnTo = nullptr;
    
    //Is this Pikmin latched on to a mob?
    bool latched = false;
    
    //Is the Pikmin holding a tool and ready to drop it on whistle?
    bool isToolPrimedForWhistle = false;
    
    //Does this Pikmin have to follow its linked mob as its leader?
    bool mustFollowLinkAsLeader = false;
    
    //Leader bump lock. Leaders close and timer running = timer resets.
    float bumpLock = 0.0f;
    
    //Is it currently doing some boredom-related animation?
    bool inBoredAnimation = false;
    
    //Is it currently doing its shaking animation?
    bool inShakingAnimation = false;
    
    //Is it currently in the carrying struggling animation?
    bool inCarryStruggleAnimation = false;
    
    //Temporary variable. Hacky, but effective. Only use within the same state!
    size_t tempI = 0;
    
    
    //--- Public function declarations ---
    
    Pikmin(const Point& pos, PikminType* type, float angle);
    void forceCarry(Mob* m);
    bool processAttackMiss(HitboxInteraction* info);
    bool increaseMaturity(int amount);
    void latch(Mob* m, const Hitbox* h);
    void startThrowTrail();
    bool canReceiveStatus(StatusType* s) const override;
    void drawMob() override;
    float getBaseSpeed() const override;
    void getGroupSpotInfo(Point* outSpot, float* outDist) const override;
    float getTaskRange() const;
    void handleStatusEffectGain(StatusType* s) override;
    void handleStatusEffectLoss(StatusType* s) override;
    void readScriptVars(const ScriptVarReader& svr) override;
    void finishDyingClassSpecifics() override;
    void startDyingClassSpecifics() override;
    
protected:

    //--- Protected function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
};


Pikmin* getClosestSprout(
    const Point& pos, Distance* d, bool ignoreReserved
    
);
