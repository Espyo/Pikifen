/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile class and pile-related functions.
 */

#include "pile.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "resource.h"


/**
 * @brief Constructs a new pile object.
 *
 * @param pos Starting coordinates.
 * @param type Pile type this mob belongs to.
 * @param angle Starting angle.
 */
Pile::Pile(const Point& pos, PileType* type, float angle) :
    Mob(pos, type, angle),
    pilType(type),
    amount(type->maxAmount) {
    
    team = MOB_TEAM_OBSTACLE;
    
    rechargeTimer =
    Timer(pilType->rechargeInterval, [this] () { this->recharge(); });
}


/**
 * @brief Changes the amount in the pile, and updates the appropriate variables.
 *
 * @param change Amount to increase by.
 */
void Pile::changeAmount(int change) {
    if(change < 0 && amount == 0) return;
    if(change > 0 && amount == pilType->maxAmount) return;
    
    amount += change;
    setHealth(true, false, change * pilType->healthPerResource);
    
    update();
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
bool Pile::getFractionNumbersInfo(
    float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
) const {
    if(amount == 0 || !pilType->showAmount) return false;
    *outValueNr = amount;
    *outReqNr = 0;
    *outColor = game.config.aestheticGen.carryingColorStop;
    return true;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Pile::readScriptVars(const ScriptVarReader& svr) {
    Mob::readScriptVars(svr);
    
    size_t amountVar;
    
    if(svr.get("amount", amountVar)) {
        amount = amountVar;
        amount = std::clamp(amount, (size_t) 0, pilType->maxAmount);
    }
    
    health = pilType->healthPerResource * amount;
    maxHealth = health;
    update();
}


/**
 * @brief Adds some more to the pile from a periodic recharge.
 */
void Pile::recharge() {
    rechargeTimer.start();
    changeAmount(pilType->rechargeAmount);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Pile::tickClassSpecifics(float deltaT) {
    rechargeTimer.tick(deltaT);
    
    if(amount == 0 && pilType->deleteWhenFinished) {
        //Ready to delete. Unless it's being used, that is.
        
        for(
            size_t r = 0;
            r < game.states.gameplay->mobs.resources.size(); r++
        ) {
            Resource* rPtr = game.states.gameplay->mobs.resources[r];
            if(rPtr->originPile == this) {
                return;
            }
        }
        
        toDelete = true;
    }
}


/**
 * @brief Updates the animation to the right one, the recharge timer, and
 * some other things.
 */
void Pile::update() {
    amount = std::clamp(amount, (size_t) 0, pilType->maxAmount);
    
    if(amount == pilType->maxAmount) {
        rechargeTimer.stop();
    }
    
    size_t animAmountNr = 0;
    size_t nGroups = pilType->animationGroupSuffixes.size();
    if(nGroups > 1 && amount > 0) {
        animAmountNr =
            ceil(
                (nGroups - 1) *
                ((float) amount / (float) pilType->maxAmount)
            );
        animAmountNr = std::clamp(animAmountNr, (size_t) 0, nGroups - 1);
    }
    setAnimation(
        getAnimationIdxFromBaseAndGroup(
            PILE_ANIM_IDLING, N_PILE_ANIMS, animAmountNr
        ),
        START_ANIM_OPTION_NO_RESTART, true
    );
    
    if(pilType->autoShrinkSmallestRadius != 0.0f) {
        setRadius(
            interpolateNumber(
                amount, 1, pilType->maxAmount,
                pilType->autoShrinkSmallestRadius, pilType->radius
            )
        );
    }
    
    if(pilType->hideWhenEmpty) {
        if(amount == 0) {
            enableFlag(flags, MOB_FLAG_HIDDEN);
            enableFlag(flags, MOB_FLAG_INTANGIBLE);
        } else {
            disableFlag(flags, MOB_FLAG_HIDDEN);
            disableFlag(flags, MOB_FLAG_INTANGIBLE);
        }
    }
}
