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
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/math_utils.h"
#include "../../util/string_utils.h"
#include "../mob_type/mob_type.h"


namespace ENEMY {

//Maximum diameter an enemy's spirit can be.
const float SPIRIT_MAX_SIZE = 128;

//Minimum diameter an enemy's spirit can be.
const float SPIRIT_MIN_SIZE = 16;

//Normally, the spirit's diameter is the enemy's. Multiply the spirit by this.
const float SPIRIT_SIZE_MULT = 0.7;

}


/**
 * @brief Constructs a new enemy object.
 *
 * @param pos Starting coordinates.
 * @param type Enemy type this mob belongs to.
 * @param angle Starting angle.
 */
Enemy::Enemy(const Point &pos, EnemyType* type, float angle) :
    Mob(pos, type, angle),
    eneType(type) {
    
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
    if(eneType->dropsCorpse) {
        becomeCarriable(CARRY_DESTINATION_SHIP_NO_ONION);
        fsm.setState(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
    }
    
    //Soul.
    Particle par(
        pos, LARGE_FLOAT,
        std::clamp(
            radius * 2 * ENEMY::SPIRIT_SIZE_MULT,
            ENEMY::SPIRIT_MIN_SIZE, ENEMY::SPIRIT_MAX_SIZE
        ),
        2, PARTICLE_PRIORITY_MEDIUM
    );
    par.bitmap = game.sysContent.bmpEnemySpirit;
    par.friction = 0.5f;
    par.linearSpeed = KeyframeInterpolator<Point>(Point(-50, -50));
    par.linearSpeed.add(0.5f, Point(50, -50));
    par.linearSpeed.add(1, Point(-50, -50));
    
    par.color = KeyframeInterpolator<ALLEGRO_COLOR>(al_map_rgba(255, 192, 255, 0));
    par.color.add(0.1f, al_map_rgb(255, 192, 255));
    par.color.add(0.6f, al_map_rgb(255, 192, 255));
    par.color.add(1, al_map_rgba(255, 192, 255, 0));
    game.states.gameplay->particles.add(par);
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
    if(eneType->isBoss) {
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
