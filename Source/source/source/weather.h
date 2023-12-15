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
    //Daylight color table for specific times of day, in minutes.
    vector<std::pair<int, ALLEGRO_COLOR> > daylight;
    //Sun strength table for specific times of day, in minutes.
    vector<std::pair<int, unsigned char> > sun_strength;
    //Blackout effect's strength table for specific times of day, in minutes.
    vector<std::pair<int, unsigned char> > blackout_strength;
    //Fog -- distance at which everything is still fully visible.
    float fog_near;
    //Fog -- distance at which everything is 100% foggy.
    float fog_far;
    //Fog -- color and density at 100% fogginess. Values throughout the day.
    vector<std::pair<int, ALLEGRO_COLOR> > fog_color;
    //Precipitation type, if any.
    PRECIPITATION_TYPES precipitation_type;
    
    weather();
    weather(
        const string &n, const vector<std::pair<int, ALLEGRO_COLOR> > &dl,
        const vector<std::pair<int, unsigned char> > &ss,
        const vector<std::pair<int, unsigned char> > &bs,
        const PRECIPITATION_TYPES pt
    );
    unsigned char get_blackout_strength();
    ALLEGRO_COLOR get_daylight_color();
    ALLEGRO_COLOR get_fog_color();
    float get_sun_strength();
    
private:
    /* ------------------------------------------------------------------------
     * Returns how to obtain a value from a table of values, based on the
     * current time.
     * Returns true on success, false on failure.
     * table:
     *   Table to read from.
     * cur_time:
     *   What time it is, in minutes.
     * ratio:
     *   The ratio between value1 and value2 that results in the final
     *   value is returned here.
     * value1:
     *   First value to interpolate with.
     * value2:
     *   Second value to interpolate with.
     */
    template<typename T>
    bool get_table_values(
        const vector<std::pair<int, T> > &table, const int cur_time,
        float* ratio, T* value1, T* value2
    ) {
        if(table.empty()) {
            return false;
        }
        if(table.size() == 1) {
            *value1 = table[0].second;
            *value2 = table[0].second;
            *ratio = 0.0f;
            return true;
        }
        
        for(size_t t = 0; t < table.size() - 1; ++t) {
            auto prev_ptr = &table[t];
            auto next_ptr = &table[t + 1];
            
            int prev_time = prev_ptr->first;
            int next_time = next_ptr->first;
            
            if(cur_time >= prev_time && cur_time < next_time) {
                *ratio =
                    (cur_time - prev_time) /
                    (float) (next_time - prev_time);
                *value1 = prev_ptr->second;
                *value2 = next_ptr->second;
                return true;
            }
        }
        
        return false;
    }
};



#endif //ifndef WEATHER_INCLUDED
