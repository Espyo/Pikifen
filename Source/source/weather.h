/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
private:
    vector<pair<size_t, string> > get_table();
public:
    string name;
    //Vector with the lighting colors for specific times of day, in minutes.
    vector<pair<size_t, ALLEGRO_COLOR> > daylight;
    //Vector with the sun strength for specific times of day, in minutes.
    vector<pair<size_t, unsigned char> > sun_strength;
    //Vector with the blackout effect's strength
    //for specific times of day, in minutes.
    vector<pair<size_t, unsigned char> > blackout_strength;
    unsigned char precipitation_type;
    interval precipitation_frequency;
    interval precipitation_speed;
    interval precipitation_angle;
    
    weather();
    weather(
        const string &n, const vector<pair<size_t, ALLEGRO_COLOR> > &dl,
        const vector<pair<size_t, unsigned char> > &ss,
        const vector<pair<size_t, unsigned char> > &bs,
        const unsigned char pt, const interval &pf,
        const interval &ps, const interval &pa
    );
};



enum PRECIPITATION_TYPES {
    PRECIPITATION_TYPE_NONE,
    PRECIPITATION_TYPE_RAIN,
    PRECIPITATION_TYPE_WIND,
};

#endif //ifndef WEATHER_INCLUDED
