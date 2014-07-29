/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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
weather::weather(const string &n, const map<unsigned, ALLEGRO_COLOR> &l, const map<unsigned, unsigned char> &ss, const unsigned char pt, const interval pf, const interval ps, const interval pa) {
    this->name = n;
    this->lighting = l;
    this->sun_strength = ss;
    this->percipitation_type = pt;
    this->percipitation_frequency = pf;
    this->percipitation_speed = ps;
    this->percipitation_angle = pa;
}