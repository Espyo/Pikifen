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
weather::weather(const string &n, const map<unsigned, ALLEGRO_COLOR> &l, const map<unsigned, unsigned char> &ss, const unsigned char pt, const interval &pf, const interval &ps, const interval &pa) :
    name(n),
    lighting(l),
    sun_strength(ss),
    percipitation_type(pt),
    percipitation_frequency(pf),
    percipitation_speed(ps),
    percipitation_angle(pa) {
    
}
