/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile class and pile-related functions.
 */

#ifndef PILE_INCLUDED
#define PILE_INCLUDED

#include "mob.h"
#include "../mob_types/pile_type.h"

enum PILE_STATES {
    PILE_STATE_IDLING,
    
    N_PILE_STATES,
};


/* ----------------------------------------------------------------------------
 * A pile is an object that represents a collection of resource-type mobs.
 * Pikmin attack it in some form, and it ends up yeilding a resource, bit by
 * bit, until it is exhausted.
 */
class pile : public mob, public mob_with_anim_groups {
public:

    pile_type* pil_type;
    size_t amount;
    timer recharge_timer;
    
    void change_amount(const int change);
    void recharge();
    void update();
    
    pile(const point &pos, pile_type* type, const float angle);
    virtual void read_script_vars(const string &vars);
    virtual void tick_class_specifics();
};

#endif //ifndef PILE_INCLUDED
