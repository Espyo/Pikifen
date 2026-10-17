/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the status effect classes and status effect-related functions.
 */

#pragma once

#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "../../core/audio.hpp"
#include "../../util/drawing_utils.hpp"
#include "../animation/animation.hpp"
#include "../content.hpp"
#include "particle.hpp"


using std::string;


#pragma region Constants


//Flags that control what sorts of mob a status effect affects.
enum STATUS_AFFECTS_FLAG {

    //Affects Pikmin.
    STATUS_AFFECTS_FLAG_PIKMIN = 1 << 0,
    
    //Affects enemies.
    STATUS_AFFECTS_FLAG_ENEMIES = 1 << 1,
    
    //Affects leaders.
    STATUS_AFFECTS_FLAG_LEADERS = 1 << 2,
    
    //Affects other mobs.
    STATUS_AFFECTS_FLAG_OTHERS = 1 << 3,
    
};


//Status affects flag enum naming (internal names).
buildEnumNames(statusAffectsFlagINames, STATUS_AFFECTS_FLAG)({
    { STATUS_AFFECTS_FLAG_PIKMIN, "pikmin" },
    { STATUS_AFFECTS_FLAG_ENEMIES, "enemies" },
    { STATUS_AFFECTS_FLAG_LEADERS, "leaders" },
    { STATUS_AFFECTS_FLAG_OTHERS, "others" },
});


//What mob FSM state the status effect changes to.
enum STATUS_STATE_CHANGE {

    //None.
    STATUS_STATE_CHANGE_NONE,
    
    //Pikmin flailing state.
    STATUS_STATE_CHANGE_FLAILING,
    
    //Pikmin helpless state.
    STATUS_STATE_CHANGE_HELPLESS,
    
    //Pikmin panic state.
    STATUS_STATE_CHANGE_PANIC,
    
    //Pikmin wading state.
    STATUS_STATE_CHANGE_WADING,
    
    //A custom state.
    STATUS_STATE_CHANGE_CUSTOM,
    
};


//Status state change enum naming (internal names).
buildEnumNames(statusStateChangeINames, STATUS_STATE_CHANGE)({
    { STATUS_STATE_CHANGE_NONE, "none" },
    { STATUS_STATE_CHANGE_FLAILING, "flailing" },
    { STATUS_STATE_CHANGE_HELPLESS, "helpless" },
    { STATUS_STATE_CHANGE_PANIC, "panic" },
    { STATUS_STATE_CHANGE_WADING, "wading" },
    { STATUS_STATE_CHANGE_CUSTOM, "custom" },
});


//Rule to follow when re-applying a status effect.
enum STATUS_REAPPLY_RULE {

    //Keep the same auto-remove time as before.
    STATUS_REAPPLY_RULE_KEEP_TIME,
    
    //Reset the auto-remove time.
    STATUS_REAPPLY_RULE_RESET_TIME,
    
    //Add more time to the auto-remove time.
    STATUS_REAPPLY_RULE_ADD_TIME,
    
};


//Status reapply rule enum naming (internal names).
buildEnumNames(statusReapplyRuleINames, STATUS_REAPPLY_RULE)({
    { STATUS_REAPPLY_RULE_KEEP_TIME, "keep_time" },
    { STATUS_REAPPLY_RULE_RESET_TIME, "reset_time" },
    { STATUS_REAPPLY_RULE_ADD_TIME, "add_time" },
});


#pragma endregion
#pragma region Status type


/**
 * @brief A status effect type, like "burning", "spicy", "petrified", etc.
 *
 * Any mob under the influence of a status effect will suffer or
 * benefit from changes in some of its values. Some effects can
 * increase the speed, others can decrease attack power. Others
 * can even slowly kill the mob unless they're cleared out, like
 * Pikmin on fire or drowning.
 */
class StatusType : public Content {

public:

    //--- Public members ---
    
    //Flags indicating what sorts of mobs it affects.
    Bitmask8 affects = 0;
    
    //Color that best represents this status type.
    ALLEGRO_COLOR color = COLOR_EMPTY;
    
    //Tint affected mobs with this color.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //Make affected mobs get colorized with this color.
    ALLEGRO_COLOR colorize = COLOR_EMPTY;
    
    //Remove the status when the affected mob leaves the hazard causing it?
    bool removeOnHazardLeave = false;
    
    //Remove the status when the affected mob is knocked down?
    bool removeOnKnockDown = false;
    
    //Remove the status when the affected mob is whistled?
    bool removeOnWhistle = false;
    
    //Remove the status automatically after these many seconds. 0 for never.
    float autoRemoveTime = 0.0f;
    
    //Rule to follow when re-applying the status effect.
    STATUS_REAPPLY_RULE reapplyRule = STATUS_REAPPLY_RULE_KEEP_TIME;
    
    //If 0, activate the status instantly.
    //Otherwise, apply this much buildup [0 - 1].
    float buildup = 0.0f;
    
    //If not 0, apply a cooldown of these many seconds, during which
    //the status effect cannot be re-activated.
    float cooldown = 0.0f;
    
    //If it uses buildup, all of it is gone after these many seconds
    //without any bumps in buildup.
    float buildupRemovalDuration = 0.0f;
    
    //Health addition/subtraction per second.
    float healthChange = 0.0f;
    
    //Health addition/subtraction percentage per second.
    float healthChangeRatio = 0.0f;
    
    //Increase/decrease in maturity when the status is gained.
    int maturityChangeAmount = 0;
    
    //How the affected mob's state changes, if it does at all.
    STATUS_STATE_CHANGE stateChangeType = STATUS_STATE_CHANGE_NONE;
    
    //Name of the FSM state to change to, if any.
    string stateChangeName;
    
    //Name of the mob animation to change to, if any.
    string animationChange;
    
    //Multiply the affected mob's speed by this much.
    float speedMultiplier = 1.0f;
    
    //Multiply the affected mob's attack power by this much.
    float attackMultiplier = 1.0f;
    
    //Multiply the affected mob's defense by this much.
    float defenseMultiplier = 1.0f;
    
    //Multiply the affected mob's animation speed by this much.
    float animSpeedMultiplier = 1.0f;
    
    //Does this status effect cause affected mobs to change to the
    //attacker's team, if applicable?
    bool causesBetrayal = false;
    
    //Does this status effect disable the affected mob's attacking ability?
    bool disablesAttack = false;
    
    //Does this status effect make the mob inedible?
    bool turnsInedible = false;
    
    //Does this status effect make the mob invisible?
    bool turnsInvisible = false;
    
    //Does this status effect freeze the mob's animation?
    bool freezesAnimation = false;
    
    //Particles to emit constantly, if any.
    ParticleGenerator* particleGen = nullptr;
    
    //Particles to emit when the status is activated, if any.
    ParticleGenerator* particleGenStart = nullptr;
    
    //Particles to emit when the status is deactivated, if any.
    ParticleGenerator* particleGenEnd = nullptr;
    
    //Horizontal offset of the particle generators.
    Point particleOffsetPos;
    
    //Vertical offset of the particle generators.
    float particleOffsetZ = 0.0f;
    
    //Whether the particle sizes scale with the mob.
    bool particleScaleSizes = false;
    
    //Whether the particle emission reaches scale with the mob.
    bool particleScaleReaches = false;
    
    //Sound to play when the status is activated, if any.
    DataNodeSound soundStart;
    
    //Sound to play when the status is deactivated, if any.
    DataNodeSound soundEnd;
    
    //How much the affected mob should shake by, if at all.
    float shakingEffect = 0.0f;
    
    //Whether the shaking should only occur on the final X seconds.
    //0 means it occurs the entire status duration.
    float shakingEffectOnEnd = 0.0f;
    
    //Name of the animation to overlay on top of affected mobs.
    string overlayAnimation;
    
    //Scale the overlay animation by this much, related to the mob's size.
    float overlayAnimMobScale = 1.0f;
    
    //Animation instance for the overlay animation.
    AnimationInstance overlayAnim;
    
    //If not nullptr, replace the Pikmin top/leader antenna with this graphic.
    ALLEGRO_BITMAP* topReplacementBmp = nullptr;
    
    //Replace with this other status effect, when its time is over.
    StatusType* replacementOnTimeout = nullptr;
    
    //Replacement name. Used during loading.
    string replacementOnTimeoutStr;
    
    
    //--- Public function declarations ---
    
    void loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level);
    
};


#pragma endregion
#pragma region Status


/**
 * @brief Instance of an active status effect on a mob.
 */
struct Status {

    //--- Public members ---
    
    //Status type.
    StatusType* type = nullptr;
    
    //Time left, if this status effect auto-removes itself.
    float timeLeft = 0.0f;
    
    //Was this status inflicted by a hazard?
    bool fromHazard = false;
    
    //If the status effect causes betrayal, return to this team once it's over.
    MOB_TEAM preBetrayalTeam = MOB_TEAM_NONE;
    
    
    //--- Public function declarations ---
    
    explicit Status(StatusType* type);
    
};


#pragma endregion
#pragma region Status buildup


/**
 * @brief Instance of the buildup of a status on a mob.
 */
struct StatusBuildup {

    //--- Public members ---
    
    //Status type.
    StatusType* type = nullptr;
    
    //Current buildup amount [0 - 1].
    float amount = 0.0f;
    
    //Time left until the buildup is removed, if applicable.
    float removalTimeLeft = 0.0f;
    
    
    //--- Public function declarations ---
    
    explicit StatusBuildup(StatusType* type);
    
};


#pragma endregion
#pragma region Status cooldown


/**
 * @brief Instance of the cooldown of a status on a mob.
 */
struct StatusCooldown {

    //--- Public members ---
    
    //Status type.
    StatusType* type = nullptr;
    
    //Time left until it is no longer in cooldown.
    float timeLeft = 0.0f;
    
    
    //--- Public function declarations ---
    
    explicit StatusCooldown(StatusType* type);
    
};


#pragma endregion
#pragma region Status manager


/**
 * @brief Manages everything about a mob's statuses. This only concerns itself
 * with what statuses should be activated and deactivated, buildups, and
 * cooldowns. Anything more mob-centric is not handled here.
 */
struct StatusManager {

    //--- Public function declarations ---
    
    explicit StatusManager(Mob* mob);
    void clearBuildupsAndCooldowns();
    void deactivate(StatusType* statusType);
    void deactivateAll();
    void deleteDeactivated();
    const vector<StatusCooldown>& getCooldowns() const;
    const vector<StatusBuildup>& getBuildups() const;
    const vector<Status>& getList() const;
    const vector<Status>& getDeactivated() const;
    void handleHazardLeave();
    void handleKnockdown();
    Status* handleStatusSource(
        StatusType* type,
        float overrideBuildup = FLT_MAX, bool forceReapplyResetTime = false
    );
    void handleWakingUp();
    bool handleWhistle();
    bool hasBuildups() const;
    bool hasCooldowns() const;
    void tick(float deltaT);
    
    
    //--- Private members ---
    
    //Mob it belongs to.
    Mob* mob = nullptr;
    
    //Active status effects currently inflicted on the mob.
    vector<Status> statuses;
    
    //Status effects that are inactive, but are being built-up.
    vector<StatusBuildup> buildups;
    
    //Status effects that are inactive, but are in cooldown.
    vector<StatusCooldown> cooldowns;
    
    //Status effects that were deactivated this frame, and not yet reported.
    vector<Status> deactivated;
    
    
    //--- Private function declarations ---
    
    Status* activate(StatusType* statusType);
    bool applyBuildup(StatusType* statusType, float overrideAmount);
    void deactivate(size_t idx);
    void deactivateIf(
        const std::function<bool(Status*)>& condition
    );
    
};


#pragma endregion
