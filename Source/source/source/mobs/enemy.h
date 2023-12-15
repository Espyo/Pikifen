/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy class and enemy-related functions.
 */

#ifndef ENEMY_INCLUDED
#define ENEMY_INCLUDED

#include "../mob_types/enemy_type.h"
#include "mob.h"


namespace ENEMY {
extern const float SPIRIT_SIZE_MULT;
extern const float SPIRIT_MAX_SIZE;
extern const float SPIRIT_MIN_SIZE;
}


/* ----------------------------------------------------------------------------
 * I don't need to explain what an enemy is.
 */
class enemy : public mob {
public:
    //What type of enemy it is.
    enemy_type* ene_type;
    
    //Constructor.
    enemy(const point &pos, enemy_type* type, const float angle);
    
    //Can the mob currently receive the specified status effect?
    bool can_receive_status(status_type* s) const override;
    //Mob drawing routine.
    void draw_mob() override;
    //Death finish class-specific logic.
    void finish_dying_class_specifics() override;
    //Death start class-specific logic.
    void start_dying_class_specifics() override;
};


#endif //ifndef ENEMY_INCLUDED
