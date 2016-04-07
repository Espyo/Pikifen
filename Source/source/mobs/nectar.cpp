/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
 * Creates a nectar.
 */
nectar::nectar(float x, float y, const string &vars) :
    mob(x, y, spec_mob_types["Nectar"], 0, vars),
    amount_left(nectar_amount) {
    
}

void nectar::draw() {
    float radius = type->radius * (amount_left + nectar_amount) / (nectar_amount * 2) * 2;
    draw_sprite(
        bmp_nectar,
        x, y,
        radius * 2, radius * 2, 0,
        map_gray(get_sprite_brightness(this))
    );
}
