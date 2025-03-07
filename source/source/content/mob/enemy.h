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
extern const float SPIRIT_SIZE_MULT;
extern const float SPIRIT_MAX_SIZE;
extern const float SPIRIT_MIN_SIZE;
}


/**
 * @brief I don't need to explain what an enemy is.
 */
class Enemy : public Mob {

public:

    //--- Members ---
    
    //What type of enemy it is.
    EnemyType* ene_type = nullptr;
    
    
    //--- Function declarations ---
    
    Enemy(const Point &pos, EnemyType* type, float angle);
    bool can_receive_status(StatusType* s) const override;
    void draw_mob() override;
    void finish_dying_class_specifics() override;
    void start_dying_class_specifics() override;
    
};
