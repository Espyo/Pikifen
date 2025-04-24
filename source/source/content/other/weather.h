/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the weather class and weather-related functions.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../content.h"


using std::size_t;
using std::string;
using std::vector;


//Types of precipitation.
enum PRECIPITATION_TYPE {

    //None.
    PRECIPITATION_TYPE_NONE,
    
    //Rain.
    PRECIPITATION_TYPE_RAIN,
    
    //Wind.
    PRECIPITATION_TYPE_WIND,
    
};



/**
 * @brief Weather information.
 *
 * Daylight is mixed in with the weather, as
 * different weather conditions imply different
 * lighting throughout the day (on a sunny day,
 * everything is bright all the way through,
 * but on a cloudy day, everything is darker
 * and grayer).
 */
class Weather : public Content {

public:

    //--- Members ---
    
    //Daylight color table for specific times of day, in minutes.
    vector<std::pair<int, ALLEGRO_COLOR> > daylight;
    
    //Sun strength table for specific times of day, in minutes.
    vector<std::pair<int, unsigned char> > sunStrength;
    
    //Blackout effect's strength table for specific times of day, in minutes.
    vector<std::pair<int, unsigned char> > blackoutStrength;
    
    //Fog -- distance at which everything is still fully visible.
    float fogNear = 0.0f;
    
    //Fog -- distance at which everything is 100% foggy.
    float fogFar = 0.0f;
    
    //Fog -- color and density at 100% fogginess. Values throughout the day.
    vector<std::pair<int, ALLEGRO_COLOR> > fogColor;
    
    //Precipitation type, if any.
    PRECIPITATION_TYPE precipitationType = PRECIPITATION_TYPE_NONE;
    
    
    //--- Function declarations ---
    
    Weather();
    Weather(
        const string &n, const vector<std::pair<int, ALLEGRO_COLOR> > &dl,
        const vector<std::pair<int, unsigned char> > &ss,
        const vector<std::pair<int, unsigned char> > &bs,
        const PRECIPITATION_TYPE pt
    );
    unsigned char getBlackoutStrength();
    ALLEGRO_COLOR getDaylightColor();
    ALLEGRO_COLOR getFogColor();
    float getSunStrength();
    void loadFromDataNode(DataNode* node);
    
private:

    //--- Function definitions ---
    
    /**
     * @brief Returns how to obtain a value from a table of values, based on the
     * current time.
     *
     * @param table Table to read from.
     * @param curTime What time it is, in minutes.
     * @param ratio The ratio between value1 and value2 that results in
     * the final value is returned here.
     * @param value1 First value to interpolate with.
     * @param value2 Second value to interpolate with.
     * @return Whether it succeeded.
     */
    template<typename T>
    bool getTableValues(
        const vector<std::pair<int, T> > &table, int curTime,
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
        
        for(size_t t = 0; t < table.size() - 1; t++) {
            auto prevPtr = &table[t];
            auto nextPtr = &table[t + 1];
            
            int prevTime = prevPtr->first;
            int nextTime = nextPtr->first;
            
            if(curTime >= prevTime && curTime < nextTime) {
                *ratio =
                    (curTime - prevTime) /
                    (float) (nextTime - prevTime);
                *value1 = prevPtr->second;
                *value2 = nextPtr->second;
                return true;
            }
        }
        
        return false;
    }
    
};
