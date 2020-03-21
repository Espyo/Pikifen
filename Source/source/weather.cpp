/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Weather class and weather-related functions.
 */

#include "weather.h"

weather::weather() {}

/* ----------------------------------------------------------------------------
 * Creates a weather type.
 */
weather::weather(
    const string &n, const vector<pair<size_t, ALLEGRO_COLOR> > &dl,
    const vector<pair<size_t, unsigned char> > &ss,
    const vector<pair<size_t, unsigned char> > &bs,
    const unsigned char pt
) :
    name(n),
    daylight(dl),
    sun_strength(ss),
    blackout_strength(bs),
    fog_near(0),
    fog_far(0),
    precipitation_type(pt) {
    
}
