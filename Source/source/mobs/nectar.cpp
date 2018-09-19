/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Nectar class and nectar-related functions.
 */

#include "../const.h"
#include "../drawing.h"
#include "../functions.h"
#include "nectar.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a nectar mob.
 */
nectar::nectar(const point &pos, const string &vars, mob* parent) :
    mob(pos, spec_mob_types["Nectar"], 0, vars, parent),
    amount_left(nectar_amount) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the nectar mob.
 */
void nectar::draw_mob(bitmap_effect_manager* effect_manager) {
    float radius =
        type->radius * (amount_left + nectar_amount) / (nectar_amount * 2) * 2;
        
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    draw_bitmap_with_effects(
        bmp_nectar,
        pos, point(radius * 2, radius * 2),
        0, &effects
    );
}
