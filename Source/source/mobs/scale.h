/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the scale class and scale-related functions.
 */

#ifndef SCALE_INCLUDED
#define SCALE_INCLUDED

#include "mob.h"

#include "../mob_types/scale_type.h"


/* ----------------------------------------------------------------------------
 * A scale is something that measures the weight being applied on top of it,
 * and does something depending on the value.
 */
class scale : public mob {
public:
    //What type of scale it is.
    scale_type* sca_type;
    
    //Return the weight currently on top of it.
    float calculate_cur_weight() const;
    //Returns information on how to show the fraction.
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const;
    
    //Constructor.
    scale(const point &pos, scale_type* type, float angle);
};


#endif //ifndef SCALE_INCLUDED
