/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Weather class and weather-related functions.
 */

#include <algorithm>

#include "weather.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/math_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new weather object.
 */
Weather::Weather() {
}


/**
 * @brief Constructs a new weather object.
 *
 * @param n Its name.
 * @param dl Daylight information table.
 * @param ss Sun strength information table.
 * @param bs Blackout strength information table.
 * @param pt Precipitation type.
 */
Weather::Weather(
    const string& n, const vector<std::pair<int, ALLEGRO_COLOR> >& dl,
    const vector<std::pair<int, unsigned char> >& ss,
    const vector<std::pair<int, unsigned char> >& bs,
    const PRECIPITATION_TYPE pt
) :
    daylight(dl),
    sunStrength(ss),
    blackoutStrength(bs),
    precipitationType(pt) {
    
    name = n;
}


/**
 * @brief Returns the blackout effect's strength for the current time.
 *
 * @return The blackout strength.
 */
unsigned char Weather::getBlackoutStrength() {
    float ratio;
    unsigned char strength1;
    unsigned char strength2;
    bool success =
        getTableValues(
            blackoutStrength, game.states.gameplay->dayMinutes,
            &ratio, &strength1, &strength2
        );
        
    if(success) {
        return interpolateNumber(ratio, 0.0f, 1.0f, strength1, strength2);
    } else {
        return 0;
    }
}


/**
 * @brief Returns the daylight color for the current time.
 *
 * @return The daylight color.
 */
ALLEGRO_COLOR Weather::getDaylightColor() {
    float ratio;
    ALLEGRO_COLOR color1;
    ALLEGRO_COLOR color2;
    bool success =
        getTableValues(
            daylight, game.states.gameplay->dayMinutes,
            &ratio, &color1, &color2
        );
        
    if(success) {
        return interpolateColor(ratio, 0.0f, 1.0f, color1, color2);
    } else {
        return COLOR_EMPTY_WHITE;
    }
}


/**
 * @brief Returns the fog color for the current time.
 *
 * @return The fog color.
 */
ALLEGRO_COLOR Weather::getFogColor() {
    float ratio;
    ALLEGRO_COLOR color1;
    ALLEGRO_COLOR color2;
    bool success =
        getTableValues(
            fogColor, game.states.gameplay->dayMinutes,
            &ratio, &color1, &color2
        );
        
    if(success) {
        return interpolateColor(ratio, 0.0f, 1.0f, color1, color2);
    } else {
        return COLOR_EMPTY_WHITE;
    }
}


/**
 * @brief Returns the sun strength for the current time [0 - 1].
 *
 * @return The sun strength.
 */
float Weather::getSunStrength() {
    float ratio;
    unsigned char strength1;
    unsigned char strength2;
    bool success =
        getTableValues(
            sunStrength, game.states.gameplay->dayMinutes,
            &ratio, &strength1, &strength2
        );
        
    if(success) {
        return
            interpolateNumber(ratio, 0.0f, 1.0f, strength1, strength2) /
            255.0f;
    } else {
        return 1.0f;
    }
}


/**
 * @brief Loads weather data from a data node.
 *
 * @param node Data node to load from.
 */
void Weather::loadFromDataNode(DataNode* node) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter wRS(node);
    
    wRS.set("fog_near", fogNear);
    wRS.set("fog_far", fogFar);
    
    fogNear = std::max(fogNear, 0.0f);
    fogFar = std::max(fogFar, fogNear);
    
    //Lighting.
    vector<std::pair<int, string> > lightingTable =
        getWeatherTable(node->getChildByName("lighting"));
        
    for(size_t p = 0; p < lightingTable.size(); p++) {
        daylight.push_back(
            std::make_pair(
                lightingTable[p].first,
                s2c(lightingTable[p].second)
            )
        );
    }
    
    //Sun's strength.
    vector<std::pair<int, string> > sunStrengthTable =
        getWeatherTable(node->getChildByName("sun_strength"));
        
    for(size_t p = 0; p < sunStrengthTable.size(); p++) {
        sunStrength.push_back(
            std::make_pair(
                sunStrengthTable[p].first,
                s2i(sunStrengthTable[p].second)
            )
        );
    }
    
    //Blackout effect's strength.
    vector<std::pair<int, string> > blackoutStrengthTable =
        getWeatherTable(
            node->getChildByName("blackout_strength")
        );
        
    for(size_t p = 0; p < blackoutStrengthTable.size(); p++) {
        blackoutStrength.push_back(
            std::make_pair(
                blackoutStrengthTable[p].first,
                s2i(blackoutStrengthTable[p].second)
            )
        );
    }
    
    //Fog.
    vector<std::pair<int, string> > fogColorTable =
        getWeatherTable(
            node->getChildByName("fog_color")
        );
    for(size_t p = 0; p < fogColorTable.size(); p++) {
        fogColor.push_back(
            std::make_pair(
                fogColorTable[p].first,
                s2c(fogColorTable[p].second)
            )
        );
    }
}
