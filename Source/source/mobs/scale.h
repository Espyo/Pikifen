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


/**
 * @brief A scale is something that measures the weight being applied on
 * top of it, and does something depending on the value.
 */
class scale : public mob {

public:
    
    //--- Members ---

    //What type of scale it is.
    scale_type* sca_type;
    
    //Weight number that must be met to reach a goal. 0 for none. Type override.
    size_t goal_number;
    
    
    //--- Function declarations ---
    
    scale(const point &pos, scale_type* type, float angle);
    float calculate_cur_weight() const;
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    void read_script_vars(const script_var_reader &svr) override;
    
};


#endif //ifndef SCALE_INCLUDED
