/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile class and pile-related functions.
 */

#include "../drawing.h"
#include "pile.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a pile.
 */
pile::pile(const point &pos, pile_type* type, const float angle) :
    mob(pos, type, angle),
    pil_type(type) {
    
}
