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
#include "../functions.h"
#include "pile.h"
#include "../vars.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a pile.
 */
pile::pile(const point &pos, pile_type* type, const float angle) :
    mob(pos, type, angle),
    pil_type(type),
    amount(type->max_amount) {
    
    team = MOB_TEAM_OBSTACLE;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void pile::read_script_vars(const string &vars) {
    mob::read_script_vars(vars);
    amount = s2i(get_var_value(vars, "amount", i2s(pil_type->max_amount)));
    amount = min(amount, pil_type->max_amount);
    
    health = pil_type->health_per_resource * amount;
}
