/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy class and enemy-related functions.
 */

#pragma once

#include "../mob_type/enemy_type.h"
#include "mob.h"


namespace ENEMY {
extern const float SOUL_SIZE_MULT;
extern const float SOUL_MAX_SIZE;
extern const float SOUL_MIN_SIZE;
}


/**
 * @brief I don't need to explain what an enemy is.
 */
class Enemy : public Mob {

public:

    //--- Members ---
    
    //What type of enemy it is.
    EnemyType* eneType = nullptr;
    
    //Time left until it comes back to life.
    Timer reviveTimer;
    
    //--- Function declarations ---
    
    Enemy(const Point& pos, EnemyType* type, float angle);
    bool canReceiveStatus(StatusType* s) const override;
    void drawMob() override;
    void revive();
    void finishDyingClassSpecifics() override;
    void startDyingClassSpecifics() override;
    void tickClassSpecifics(float deltaT) override;
};
