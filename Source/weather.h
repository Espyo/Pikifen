/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the weather class and weather-related functions.
 */

#ifndef WEATHER_INCLUDED
#define WEATHER_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>

#include "interval.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Weather information.
 * Daylight is mixed in with the weather, as
 * different weather conditions imply different
 * lighting throughout the day (on a sunny day,
 * everything is bright all the way through,
 * but on a foggy day, everything is darker
 * and grayer).
 */
class weather {
public:
    string name;
    map<unsigned, ALLEGRO_COLOR> lighting; //Map with the lighting color for each specific time of day, in minutes.
    map<unsigned, unsigned char> sun_strength; //Map with the sun's strength for each specific time of day, in minutes.
    unsigned char percipitation_type;
    interval percipitation_frequency;
    interval percipitation_speed;
    interval percipitation_angle;
    
    weather();
    weather(const string &n, const map<unsigned, ALLEGRO_COLOR> &l, const map<unsigned, unsigned char> &ss, const unsigned char pt, const interval &pf, const interval &ps, const interval &pa);
};



enum PERCIPITATION_TYPES {
    PERCIPITATION_TYPE_NONE,
    PERCIPITATION_TYPE_RAIN,
    PERCIPITATION_TYPE_WIND,
};

#endif //ifndef WEATHER_INCLUDED
