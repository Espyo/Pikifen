/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Analog stick cleaner class and related functions.
 *
 * Please read the header file for more information.
 */

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>

#include "analog_stick_cleaner.h"


/**
 * @brief Cleans an analog stick's input according to the settings.
 *
 * @param coords Pointer to an array of size 2 with the X and Y coordinate
 * respectively. When the cleaning process ends, this array will contain
 * the cleaned up coordinates.
 * @param settings Settings to use.
 */
void AnalogStickCleaner::clean(float coords[2], const Settings& settings) {
    //First, sanitize the function arguments.
    coords[0] = std::clamp(coords[0], -1.0f, 1.0f);
    coords[1] = std::clamp(coords[1], -1.0f, 1.0f);
    
    //Step 1: Process radial deadzones.
    processRadialDeadzones(coords, settings);
    
    //Step 2: Process angular deadzones.
    processAngularDeadzones(coords, settings);
}


/**
 * @brief Returns the deadzone size in the settings for the specified
 * snap direction. 0 is right, 1 is diagonal down-right, etc.
 * Due to the way this is used in the cleaning process, it also supports
 * values above 7.
 *
 * @param snapDirIdx Index of the direction.
 * @param settings Settings to use.
 * @return The deadzone size.
 */
float AnalogStickCleaner::getSnapDirDeadzone(
    int snapDirIdx, const Settings& settings
) {
    switch(snapDirIdx % 8) {
    case 0:
    case 4: {
        return settings.deadzones.angular.horizontal;
    }
    case 2:
    case 6: {
        return settings.deadzones.angular.vertical;
    }
    default: {
        return settings.deadzones.angular.diagonal;
    }
    }
}


/**
 * @brief Returns the interpolation between two numbers, given a number in
 * an interval. Then, it clamps it to that interval.
 *
 * @param input The input number.
 * @param inputStart Start of the interval the input number falls on,
 * inclusive. The closer to inputStart, the closer the output is
 * to outputStart.
 * @param inputEnd End of the interval the number falls on, inclusive.
 * @param outputStart Number on the starting tip of the interpolation.
 * @param outputEnd Number on the ending tip of the interpolation.
 * @return The interpolated number.
 */
float AnalogStickCleaner::interpolateAndClamp(
    float input, float inputStart, float inputEnd,
    float outputStart, float outputEnd
) {
    float inputDiff = std::max(0.001f, inputEnd - inputStart);
    float result =
        outputStart +
        ((input - inputStart) / inputDiff) *
        (outputEnd - outputStart);
    result = std::clamp(result, outputStart, outputEnd);
    return result;
}


/**
 * @brief Process angular deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void AnalogStickCleaner::processAngularDeadzones(
    float coords[2], const Settings& settings
) {
    //Get the basics.
    float radius, angle;
    toPolar(coords, angle, radius);
    angle = (float) fmod(angle + M_PI * 2, M_PI * 2);
    
    //Start by finding the previous snap direction (i.e. the closest one
    //counterclockwise), and the next snap direction (i.e. closest clockwise).
    int prevSnapDirIdx =
        (int) (floor(angle / M_PI_4) + 8) % 8;
    int nextSnapDirIdx =
        prevSnapDirIdx + 1;
    float prevSnapDirAngle =
        (float) (M_PI_4 * prevSnapDirIdx);
    float nextSnapDirAngle =
        (float) (M_PI_4 * nextSnapDirIdx);
    float prevSnapDirDeadzone =
        getSnapDirDeadzone(prevSnapDirIdx, settings);
    float nextSnapDirDeadzone =
        getSnapDirDeadzone(nextSnapDirIdx, settings);
        
    //Do the clean up.
    const float inputSpaceStart =
        prevSnapDirAngle + prevSnapDirDeadzone / 2.0f;
    const float inputSpaceEnd =
        nextSnapDirAngle - nextSnapDirDeadzone / 2.0f;
    const float outputSpaceStart =
        prevSnapDirAngle;
    const float outputSpaceEnd =
        nextSnapDirAngle;
        
    if(settings.deadzones.angular.interpolate) {
        //Interpolate.
        angle =
            interpolateAndClamp(
                angle,
                inputSpaceStart, inputSpaceEnd,
                outputSpaceStart, outputSpaceEnd
            );
            
    } else {
        //Hard cut-off.
        if(angle < inputSpaceStart) {
            angle = outputSpaceStart;
        }
        if(angle > inputSpaceEnd) {
            angle = outputSpaceEnd;
        }
    }
    
    //Finally, save the clean input.
    toCartesian(coords, angle, radius);
}


/**
 * @brief Process radial deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void AnalogStickCleaner::processRadialDeadzones(
    float coords[2], const Settings& settings
) {
    //Get the basics.
    float radius, angle;
    toPolar(coords, angle, radius);
    
    //Do the clean up.
    const float inputSpaceStart =
        settings.deadzones.radial.inner;
    const float inputSpaceEnd =
        settings.deadzones.radial.outer;
    const float outputSpaceStart =
        0.0f;
    const float outputSpaceEnd =
        1.0f;
        
    if(settings.deadzones.radial.interpolate) {
        //Interpolate.
        radius =
            interpolateAndClamp(
                radius,
                inputSpaceStart, inputSpaceEnd,
                outputSpaceStart, outputSpaceEnd
            );
            
    } else {
        //Hard cut-off.
        if(radius < inputSpaceStart) {
            radius = outputSpaceStart;
        }
        if(radius > inputSpaceEnd) {
            radius = outputSpaceEnd;
        }
        
    }
    
    //Finally, save the clean input.
    toCartesian(coords, angle, radius);
}


/**
 * @brief Converts polar coordinates to Cartesian.
 *
 * @param coords Cartesian coordinates to save to.
 * @param angle Angle to use.
 * @param radius Radius to use.
 */
void AnalogStickCleaner::toCartesian(
    float coords[2], float angle, float radius
) {
    coords[0] = (float) cos(angle) * radius;
    coords[1] = (float) sin(angle) * radius;
}


/**
 * @brief Converts Cartesian coordinates to polar.
 *
 * @param coords Cartesian coordinates to use.
 * @param angle Angle to save to.
 * @param radius Radius to save to.
 */
void AnalogStickCleaner::toPolar(
    float coords[2], float& angle, float& radius
) {
    angle = (float) atan2(coords[1], coords[0]);
    radius = (float) sqrt(coords[0] * coords[0] + coords[1] * coords[1]);
}
