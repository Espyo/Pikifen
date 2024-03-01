/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile class and pile-related functions.
 */

#ifndef PILE_INCLUDED
#define PILE_INCLUDED

#include "../mob_types/pile_type.h"
#include "mob.h"


/**
 * @brief A pile is an object that represents a collection of resource-type mobs.
 * Pikmin attack it in some form, and it ends up yeilding a resource, bit by
 * bit, until it is exhausted.
 */
class pile : public mob, public mob_with_anim_groups {

public:
    
    //--- Members ---

    //What type of pile it is.
    pile_type* pil_type;
    
    //Current amount of resources.
    size_t amount;
    
    //Time left until it recharges.
    timer recharge_timer;
    
    
    //--- Function declarations ---

    pile(const point &pos, pile_type* type, const float angle);
    void change_amount(const int change);
    void recharge();
    void update();
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    void read_script_vars(const script_var_reader &svr) override;
    
protected:

    //--- Function declarations ---
        
    void tick_class_specifics(const float delta_t) override;
    
};


#endif //ifndef PILE_INCLUDED
