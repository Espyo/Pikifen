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


/* ----------------------------------------------------------------------------
 * Creates a weather type.
 */
weather::weather() :
    fog_near(0.0f),
    fog_far(0.0f),
    precipitation_type(PRECIPITATION_TYPE_NONE) {
}


/* ----------------------------------------------------------------------------
 * Creates a weather type.
 * n:
 *   Its name.
 * dl:
 *   Daylight information table.
 * ss:
 *   Sun strength information table.
 * bs:
 *   Blackout strength information table.
 * pt:
 *   Precipitation type.
 */
weather::weather(
    const string &n, const vector<std::pair<size_t, ALLEGRO_COLOR> > &dl,
    const vector<std::pair<size_t, unsigned char> > &ss,
    const vector<std::pair<size_t, unsigned char> > &bs,
    const PRECIPITATION_TYPES pt
) :
    name(n),
    daylight(dl),
    sun_strength(ss),
    blackout_strength(bs),
    fog_near(0),
    fog_far(0),
    precipitation_type(pt) {
    
}
