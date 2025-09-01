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

#include "../../util/drawing_utils.h"
#include "../animation/animation.h"
#include "../content.h"
#include "particle.h"


using std::string;


//Flags that control what sorts of mob a status effect affects.
enum STATUS_AFFECTS_FLAG {

    //Affects Pikmin.
    STATUS_AFFECTS_FLAG_PIKMIN = 1 << 0,
    
    //Affects enemies.
    STATUS_AFFECTS_FLAG_ENEMIES = 1 << 1,
    
    //Affects leaders.
    STATUS_AFFECTS_FLAG_LEADERS = 1 << 2,
    
    //Affects other mobs.
    STATUS_AFFECTS_FLAG_OTHERS = 1 << 4,
    
};


//What mob script state the status effect changes to.
enum STATUS_STATE_CHANGE {

    //None.
    STATUS_STATE_CHANGE_NONE,
    
    //Pikmin flailing state.
    STATUS_STATE_CHANGE_FLAILING,
    
    //Pikmin helpless state.
    STATUS_STATE_CHANGE_HELPLESS,
    
    //Pikmin panic state.
    STATUS_STATE_CHANGE_PANIC,
    
    //A custom state.
    STATUS_STATE_CHANGE_CUSTOM,
    
};


//Rule to follow when re-applying a status effect.
enum STATUS_REAPPLY_RULE {

    //Keep the same auto-remove time as before.
    STATUS_REAPPLY_RULE_KEEP_TIME,
    
    //Reset the auto-remove time.
    STATUS_REAPPLY_RULE_RESET_TIME,
    
    //Add more time to the auto-remove time.
    STATUS_REAPPLY_RULE_ADD_TIME,
    
};


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

    //--- Members ---
    
    //Flags indicating what sorts of mobs it affects.
    unsigned char affects = 0;
    
    //Color that best represents this status type.
    ALLEGRO_COLOR color = COLOR_EMPTY;
    
    //Tint affected mobs with this color.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //Make affected mobs glow with this color.
    ALLEGRO_COLOR glow = COLOR_EMPTY;
    
    //Can the status effect be removed if the affected mob is whistled?
    bool removableWithWhistle = false;
    
    //Remove the status when the affected mob leaves the hazard causing it?
    bool removeOnHazardLeave = false;
    
    //Remove the status automatically after these many seconds. 0 for never.
    float autoRemoveTime = 0.0f;
    
    //Rule to follow when re-applying the status effect.
    STATUS_REAPPLY_RULE reapplyRule = STATUS_REAPPLY_RULE_KEEP_TIME;
    
    //Health addition/subtraction per second.
    float healthChange = 0.0f;
    
    //Health addition/subtraction percentage per second.
    float healthChangeRatio = 0.0f;
    
    //Increase/decrease in maturity when the status is gained.
    int maturityChangeAmount = 0;
    
    //How the affected mob's state changes, if it does at all.
    STATUS_STATE_CHANGE stateChangeType = STATUS_STATE_CHANGE_NONE;
    
    //Name of the mob state to change to, if any.
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
    
    //Does this status effect disable the affected mob's attacking ability?
    bool disablesAttack = false;
    
    //Does this status effect make the mob inedible?
    bool turnsInedible = false;
    
    //Does this status effect make the mob invisible?
    bool turnsInvisible = false;
    
    //Does this status effect freeze the mob's animation?
    bool freezesAnimation = false;
    
    //Generates particles? We need to know so we can remove the generator later.
    bool generatesParticles = false;
    
    //Particle generator, if any.
    ParticleGenerator* particleGen = nullptr;
    
    //Horizontal offset of the particle generator.
    Point particleOffsetPos;
    
    //Vertical offset of the particle generator.
    float particleOffsetZ = 0.0f;
    
    //How much the affected mob should shake by, if at all.
    float shakingEffect = 0.0f;
    
    //Name of the animation to overlay on top of affected mobs.
    string overlayAnimation;
    
    //Scale the overlay animation by this much, related to the mob's size.
    float overlayAnimMobScale = 1.0f;
    
    //Animation instance for the overlay animation.
    AnimationInstance overlayAnim;
    
    //Replace with this other status effect, when its time is over.
    StatusType* replacementOnTimeout = nullptr;
    
    //Replacement name. Used during loading.
    string replacementOnTimeoutStr;
    
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level);
    
};


/**
 * @brief Instance of an active status effect on a mob.
 */
struct Status {

    //--- Members ---
    
    //Status type.
    StatusType* type = nullptr;
    
    //Time left, if this status effect auto-removes itself.
    float timeLeft = 0.0f;
    
    //Was this status inflicted by a hazard?
    bool fromHazard = false;
    
    //Should this status be deleted from the mob's statuses?
    bool toDelete = false;
    
    
    //--- Function declarations ---
    
    explicit Status(StatusType* type);
    bool isActive() const;
    void tick(float deltaT);
    
};
