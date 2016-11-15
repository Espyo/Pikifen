/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
    const vector<pair<size_t, unsigned char> > &ss, const unsigned char pt,
    const interval &pf, const interval &ps, const interval &pa
) :
    name(n),
    daylight(dl),
    sun_strength(ss),
    precipitation_type(pt),
    precipitation_frequency(pf),
    precipitation_speed(ps),
    precipitation_angle(pa) {
    
}
