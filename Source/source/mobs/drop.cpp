/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop class and drop related functions.
 */

#include "drop.h"

/* ----------------------------------------------------------------------------
 * Creates a drop mob.
 */
drop::drop(const point &pos, drop_type* dro_type, const float angle) :
    mob(pos, dro_type, angle),
    dro_type(dro_type),
    doses_left(dro_type->total_doses) {
    
    
}
