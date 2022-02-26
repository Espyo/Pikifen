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


using std::size_t;
using std::string;
using std::vector;


//Types of precipitation.
enum PRECIPITATION_TYPES {
    //None.
    PRECIPITATION_TYPE_NONE,
    //Rain.
    PRECIPITATION_TYPE_RAIN,
    //Wind.
    PRECIPITATION_TYPE_WIND,
};



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
public:
    //Name of this weather type.
    string name;
    //Vector with the lighting colors for specific times of day, in minutes.
    vector<std::pair<size_t, ALLEGRO_COLOR> > daylight;
    //Vector with the sun strength for specific times of day, in minutes.
    vector<std::pair<size_t, unsigned char> > sun_strength;
    //Vector with the blackout effect's strength
    //for specific times of day, in minutes.
    vector<std::pair<size_t, unsigned char> > blackout_strength;
    //Fog -- distance at which everything is still fully visible.
    float fog_near;
    //Fog -- distance at which everything is 100% foggy.
    float fog_far;
    //Fog -- color and density at 100% fogginess. Values throughout the day.
    vector<std::pair<size_t, ALLEGRO_COLOR> > fog_color;
    //Precipitation type, if any.
    PRECIPITATION_TYPES precipitation_type;
    
    weather();
    weather(
        const string &n, const vector<std::pair<size_t, ALLEGRO_COLOR> > &dl,
        const vector<std::pair<size_t, unsigned char> > &ss,
        const vector<std::pair<size_t, unsigned char> > &bs,
        const PRECIPITATION_TYPES pt
    );
};



#endif //ifndef WEATHER_INCLUDED
