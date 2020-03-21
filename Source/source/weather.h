/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the weather class and weather-related functions.
 */

#ifndef WEATHER_INCLUDED
#define WEATHER_INCLUDED

#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

using namespace std;


/* ----------------------------------------------------------------------------
 * Weather information.
 * Daylight is mixed in with the weather, as
 * different weather conditions imply different
 * lighting throughout the day (on a sunny day,
 * everything is bright all the way through,
 * but on a cloudy day, everything is darker
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
    //Fog -- distance at which everything is still fully visible.
    float fog_near;
    //Fog -- distance at which everything is 100% foggy.
    float fog_far;
    //Fog -- color and density at 100% fogginess. Values throughout the day.
    vector<pair<size_t, ALLEGRO_COLOR> > fog_color;
    unsigned char precipitation_type;
    
    weather();
    weather(
        const string &n, const vector<pair<size_t, ALLEGRO_COLOR> > &dl,
        const vector<pair<size_t, unsigned char> > &ss,
        const vector<pair<size_t, unsigned char> > &bs,
        const unsigned char pt
    );
};



enum PRECIPITATION_TYPES {
    PRECIPITATION_TYPE_NONE,
    PRECIPITATION_TYPE_RAIN,
    PRECIPITATION_TYPE_WIND,
};

#endif //ifndef WEATHER_INCLUDED
