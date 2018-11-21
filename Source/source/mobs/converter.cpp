/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter class and converter related functions.
 */

#include "converter.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a converter mob.
 */
converter::converter(
    const point &pos, converter_type* con_type, const float angle
) :
    mob(pos, con_type, angle),
    con_type(con_type) {
    
    
}
