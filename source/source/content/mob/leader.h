/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#pragma once

#include <vector>

#include "../../core/player.h"
#include "../../util/general_utils.h"
#include "../mob_type/leader_type.h"
#include "mob.h"


class Pikmin;

using std::size_t;


namespace LEADER {
extern const float AUTO_THROW_FASTEST_INTERVAL;
extern const float AUTO_THROW_RAMP_TIME;
extern const float AUTO_THROW_SLOWEST_INTERVAL;
extern const float BORED_ANIM_MAX_DELAY;
extern const float BORED_ANIM_MIN_DELAY;
extern const float DISMISS_ANGLE_RANGE;
extern const float DISMISS_MEMBER_SIZE_MULTIPLIER;
extern const size_t DISMISS_PARTICLE_AMOUNT;
extern const float DISMISS_PARTICLE_ALPHA;
extern const float DISMISS_PARTICLE_FRICTION;
extern const float DISMISS_PARTICLE_MIN_DURATION;
extern const float DISMISS_PARTICLE_MIN_SPEED;
extern const float DISMISS_PARTICLE_MAX_DURATION;
extern const float DISMISS_PARTICLE_MAX_SPEED;
extern const float DISMISS_PARTICLE_SIZE;
extern const float DISMISS_SUBGROUP_DISTANCE;
extern const float HEALTH_CAUTION_RATIO;
extern const float HEALTH_CAUTION_RING_DURATION;
extern const float HELD_GROUP_MEMBER_ANGLE;
extern const float HELD_GROUP_MEMBER_H_DIST;
extern const float HELD_GROUP_MEMBER_V_DIST;
extern const float INVULN_PERIOD_KB;
extern const float INVULN_PERIOD_NORMAL;
extern const float SWARM_ARROW_INTERVAL;
extern const float SWARM_PARTICLE_ALPHA;
extern const float SWARM_PARTICLE_ANGLE_DEVIATION;
extern const float SWARM_PARTICLE_FRICTION;
extern const float SWARM_PARTICLE_MAX_DURATION;
extern const float SWARM_PARTICLE_MIN_DURATION;
extern const float SWARM_PARTICLE_SIZE;
extern const float SWARM_PARTICLE_SPEED_DEVIATION;
extern const float SWARM_PARTICLE_SPEED_MULT;
extern const float THROW_COOLDOWN_DURATION;
extern const float THROW_PREVIEW_FADE_IN_RATIO;
extern const float THROW_PREVIEW_FADE_OUT_RATIO;
extern const float THROW_PREVIEW_MIN_THICKNESS;
extern const float THROW_PREVIEW_DEF_MAX_THICKNESS;
}


/**
 * @brief A leader controls Pikmin, and
 * is controlled by the player.
 */
class Leader : public Mob {

public:

    //--- Members ---
    
    //Player that is currently controlling it, if any. Cache for performance.
    Player* player = nullptr;
    
    //What type of leader it is.
    LeaderType* leaType = nullptr;
    
    //Is it currently auto-plucking?
    bool autoPlucking = false;
    
    //Pikmin it wants to pluck.
    Pikmin* pluckTarget = nullptr;
    
    //Has the player asked for the auto-plucking to stop?
    bool queuedPluckCancel = false;
    
    //Mid Go Here.
    bool midGoHere = false;
    
    //Is the leader currently in the walking animation?
    bool isInWalkingAnim = false;
    
    //Is the leader currently turning in place, in the active state?
    bool isActiveTurning = false;
    
    //Is the leader currently walking, in the active state?
    bool isActiveWalking = false;
    
    //Time until the next arrow in the list of swarm arrows appears.
    Timer swarmNextArrowTimer = Timer(LEADER::SWARM_ARROW_INTERVAL);
    
    //List of swarm mode arrows.
    vector<float> swarmArrows;
    
    //Time left before the leader can throw again.
    float throwCooldown = 0.0f;
    
    //Whether or not a throw has been queued to be pulled off.
    bool throwQueued = false;
    
    //Auto-repeater for auto-throwing.
    AutoRepeater autoThrowRepeater;
    
    //Provided there's a throw, this is the mob to throw.
    Mob* throwee = nullptr;
    
    //Provided there's a throw, this is the angle.
    float throweeAngle = 0.0f;
    
    //Provided there's a throw, this is the max Z.
    float throweeMaxZ = 0.0f;
    
    //Provided there's a throw, this is the horizontal speed.
    Point throweeSpeed;
    
    //Provided there's a throw, this is the vertical speed.
    float throweeSpeedZ = 0.0f;
    
    //Provided there's a throw, this indicates whether it's low enough to reach.
    bool throweeCanReach = false;
    
    //How much the health wheel is filled. Gradually moves to the target amount.
    float healthWheelVisibleRatio = 1.0f;
    
    //Timer for the animation of the health wheel's caution ring.
    float healthWheelCautionTimer = 0.0f;
    
    //Is it currently doing some boredom-related animation?
    bool inBoredAnimation = false;
    
    //Temporary variable. Hacky, but effective. Only use within the same state!
    size_t tempI = 0;
    
    
    //--- Function declarations ---
    
    Leader(const Point& pos, LeaderType* type, float angle);
    bool checkThrowOk() const;
    bool canGrabGroupMember(Mob* m) const;
    void dismiss();
    size_t getAmountOfGroupPikmin(const PikminType* filter);
    bool orderPikminToOnion(
        const PikminType* type, PikminNest* nPtr, size_t amount
    );
    void queueThrow();
    void signalSwarmStart() const;
    void signalSwarmEnd() const;
    void startAutoThrowing();
    void startThrowTrail();
    void startWhistling();
    void stopAutoThrowing();
    void stopWhistling();
    void swapHeldPikmin(Mob* newPik);
    void updateThrowVariables();
    bool canReceiveStatus(StatusType* s) const override;
    void getGroupSpotInfo(
        Point* outSpot, float* outDist
    ) const override;
    void drawMob() override;
    
    
protected:

    //--- Function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
private:

    //--- Members ---
    
    //Sound effect source ID of the whistle, or 0 for none.
    size_t whistleSoundSourceId = 0;
    
    //Returns how many rows are needed for all members' dismissal.
    size_t getDismissRows(size_t nMembers) const;
    
    
    //--- Function declarations ---
    
    void dismissDetails();
    void dismissLogic();
    
};


void changeToNextLeader(
    Player* player, bool forward, bool forceSuccess, bool keepIdx
);
bool grabClosestGroupMember(Player* player);
