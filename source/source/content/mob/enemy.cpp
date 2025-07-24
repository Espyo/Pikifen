/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include <algorithm>
#include <unordered_set>

#include "enemy.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/math_utils.h"
#include "../../util/string_utils.h"
#include "../mob_type/mob_type.h"


namespace ENEMY {

//Maximum diameter an enemy's soul can be.
const float SOUL_MAX_SIZE = 128;

//Minimum diameter an enemy's soul can be.
const float SOUL_MIN_SIZE = 16;

//Maximum pitch an enemy's soul sound can have.
const float SOUL_MAX_PITCH = 1.1f;

//Minimum pitch an enemy's soul sound can have.
const float SOUL_MIN_PITCH = 0.85f;

//Normally, the soul's diameter is the enemy's. Multiply the soul by this.
const float SOUL_SIZE_MULT = 0.7;

}


/**
 * @brief Constructs a new enemy object.
 *
 * @param pos Starting coordinates.
 * @param type Enemy type this mob belongs to.
 * @param angle Starting angle.
 */
Enemy::Enemy(const Point& pos, EnemyType* type, float angle) :
    Mob(pos, type, angle),
    eneType(type),
    reviveTimer(type->reviveTime),
    isBoss(false) {
    
    reviveTimer.onEnd =
    [this] () { this->revive(); };
    
}


/**
 * @brief Returns whether or not an enemy can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive the status.
 */
bool Enemy::canReceiveStatus(StatusType* s) const {
    return hasFlag(s->affects, STATUS_AFFECTS_FLAG_ENEMIES);
}


/**
 * @brief Draws an enemy.
 */
void Enemy::drawMob() {
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
        SPRITE_BMP_EFFECT_DAMAGE |
        SPRITE_BMP_EFFECT_CARRY
    );
    drawBitmapWithEffects(curSPtr->bitmap, eff);
    drawStatusEffectBmp(this, eff);
}


/**
 * @brief Logic specific to enemies for when they finish dying.
 */
void Enemy::finishDyingClassSpecifics() {
    //Corpse.
    enableFlag(flags, MOB_FLAG_NON_HUNTABLE);
    becomeCarriable(CARRY_DESTINATION_SHIP_NO_ONION);
    fsm.setState(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
    
    if(reviveTimer.duration > 0.0f) {
        //Revival.
        reviveTimer.start();
        
    } else {
        //Soul particle, only if the enemy does not revive.
        float soulSize =
            std::clamp(
                radius * 2 * ENEMY::SOUL_SIZE_MULT,
                ENEMY::SOUL_MIN_SIZE, ENEMY::SOUL_MAX_SIZE
            );
        float soulPitch =
            interpolateNumber(
                soulSize, ENEMY::SOUL_MIN_SIZE, ENEMY::SOUL_MAX_SIZE,
                ENEMY::SOUL_MAX_PITCH, ENEMY::SOUL_MIN_PITCH
            );
            
        Particle par(pos, LARGE_FLOAT, soulSize, 2, PARTICLE_PRIORITY_MEDIUM);
        par.bitmap = game.sysContent.bmpEnemySoul;
        par.friction = 0.5f;
        par.linearSpeed = KeyframeInterpolator<Point>(Point(-50, -50));
        par.linearSpeed.add(0.5f, Point(50, -50));
        par.linearSpeed.add(1, Point(-50, -50));
        
        par.color =
            KeyframeInterpolator<ALLEGRO_COLOR>(al_map_rgba(255, 192, 255, 0));
        par.color.add(0.1f, al_map_rgb(255, 192, 255));
        par.color.add(0.6f, al_map_rgb(255, 192, 255));
        par.color.add(1, al_map_rgba(255, 192, 255, 0));
        game.states.gameplay->particles.add(par);
        
        game.audio.createPosSoundSource(
            game.sysContent.sndEnemySoul, pos, false,
        { .volume = 0.2f, .speed = soulPitch, .speedDeviation = 0.02f }
        );
    }
}


/**
 * @brief Brings the enemy back to life by taking it out of its death states.
 */
void Enemy::revive() {
    health = maxHealth;
    disableFlag(flags, MOB_FLAG_NON_HUNTABLE);
    becomeUncarriable();
    
    if(type->reviveStateIdx != INVALID) {
        fsm.setState(type->reviveStateIdx);
    } else {
        fsm.setState(type->firstStateIdx);
    }
}


/**
 * @brief Sets up stuff for the beginning of the enemy's death process.
 */
void Enemy::startDyingClassSpecifics() {
    //Numbers.
    game.states.gameplay->enemyDefeats++;
    if(!game.curAreaData->mission.enemyPointsOnCollection) {
        game.states.gameplay->enemyPointsCollected += eneType->points;
    }
    game.states.gameplay->lastEnemyDefeatedPos = pos;
    game.statistics.enemyDefeats++;
    
    if(game.curAreaData->mission.goal == MISSION_GOAL_BATTLE_ENEMIES) {
        game.states.gameplay->missionRemainingMobIds.erase(id);
    }
    
    //Music.
    if(isBoss) {
        switch(game.states.gameplay->bossMusicState) {
        case BOSS_MUSIC_STATE_PLAYING: {
            bool nearBoss;
            game.states.gameplay->isNearEnemyAndBoss(nullptr, &nearBoss);
            if(!nearBoss) {
                //Only play the victory fanfare if they're not near another one.
                game.audio.setCurrentSong(
                    game.sysContentNames.sngBossVictory, true, false, false
                );
                game.states.gameplay->bossMusicState =
                    BOSS_MUSIC_STATE_VICTORY;
            }
        } default: {
            break;
        }
        }
    }
    
    //Particles.
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parEnemyDefeat, this
        );
    particleGenerators.push_back(pg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Enemy::tickClassSpecifics(float deltaT) {
    reviveTimer.tick(deltaT);
    if(reviveTimer.timeLeft > 0) {
        //Override the health wheel with the revive timer.
        health = maxHealth * (1.0f - reviveTimer.getRatioLeft());
    }
}
