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

#include "functions.h"
#include "game.h"
#include "utils/math_utils.h"


/**
 * @brief Constructs a new weather object.
 */
weather::weather() :
    fog_near(0.0f),
    fog_far(0.0f),
    precipitation_type(PRECIPITATION_TYPE_NONE) {
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
weather::weather(
    const string &n, const vector<std::pair<int, ALLEGRO_COLOR> > &dl,
    const vector<std::pair<int, unsigned char> > &ss,
    const vector<std::pair<int, unsigned char> > &bs,
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


/**
 * @brief Returns the blackout effect's strength for the current time.
 * 
 * @return The blackout strength.
 */
unsigned char weather::get_blackout_strength() {
    float ratio;
    unsigned char strength1;
    unsigned char strength2;
    bool success =
        get_table_values(
            blackout_strength, game.states.gameplay->day_minutes,
            &ratio, &strength1, &strength2
        );
        
    if(success) {
        return interpolate_number(ratio, 0.0f, 1.0f, strength1, strength2);
    } else {
        return 0;
    }
}


/**
 * @brief Returns the daylight color for the current time.
 * 
 * @return The daylight color.
 */
ALLEGRO_COLOR weather::get_daylight_color() {
    float ratio;
    ALLEGRO_COLOR color1;
    ALLEGRO_COLOR color2;
    bool success =
        get_table_values(
            daylight, game.states.gameplay->day_minutes,
            &ratio, &color1, &color2
        );
        
    if(success) {
        return interpolate_color(ratio, 0.0f, 1.0f, color1, color2);
    } else {
        return al_map_rgba(255, 255, 255, 0);
    }
}


/**
 * @brief Returns the fog color for the current time.
 * 
 * @return The fog color.
 */
ALLEGRO_COLOR weather::get_fog_color() {
    float ratio;
    ALLEGRO_COLOR color1;
    ALLEGRO_COLOR color2;
    bool success =
        get_table_values(
            fog_color, game.states.gameplay->day_minutes,
            &ratio, &color1, &color2
        );
        
    if(success) {
        return interpolate_color(ratio, 0.0f, 1.0f, color1, color2);
    } else {
        return al_map_rgba(255, 255, 255, 0);
    }
}


/**
 * @brief Returns the sun strength for the current time, in the range 0 - 1.
 * 
 * @return The sun strength.
 */
float weather::get_sun_strength() {
    float ratio;
    unsigned char strength1;
    unsigned char strength2;
    bool success =
        get_table_values(
            sun_strength, game.states.gameplay->day_minutes,
            &ratio, &strength1, &strength2
        );
        
    if(success) {
        return
            interpolate_number(ratio, 0.0f, 1.0f, strength1, strength2) /
            255.0f;
    } else {
        return 1.0f;
    }
}
