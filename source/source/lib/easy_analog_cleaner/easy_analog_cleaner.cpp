/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Easy Analog Cleaner class and related functions.
 *
 * Please read the header file for more information.
 */

#undef _CMATH_

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <iostream>

#include "easy_analog_cleaner.h"


//#define EASY_ANALOG_CLEANER


const EasyAnalogCleaner::Settings EasyAnalogCleaner::SETTINGS_NO_CHANGES = {
    .deadzones = {
        .radial = {
            .inner = 0.0f,
            .outer = 1.0f,
            .interpolate = false
        },
        .angular = {
            .horizontal = 0.0f,
            .vertical = 0.0f,
            .diagonal = 0.0f,
            .interpolate = false
        },
        .button = {
            .unpressed = 0.0f,
            .pressed = 1.0f,
            .interpolate = false
        }
    },
    .lowPassFilter = {
        .factor = 0.0f,
        .factorButton = 0.0f,
    },
    .misc = {
        .unitCircleClamp = false
    }
};


/**
 * @brief Cleans an analog stick's input using the default settings.
 *
 * @param coords Pointer to an array of size 2 with the X and Y coordinate
 * respectively, each [-1 - 1]. When the cleaning process ends, this array
 * will contain the cleaned up coordinates.
 */
void EasyAnalogCleaner::clean(float coords[2]) {
    clean(coords, Settings());
}


/**
 * @brief Cleans an analog stick's input according to the settings.
 *
 * @param coords Pointer to an array of size 2 with the X and Y coordinate
 * respectively, each [-1 - 1]. When the cleaning process ends, this array
 * will contain the cleaned up coordinates.
 * @param settings Settings to use.
 * @param previousFrameCoords Pointer to an array of the coordinates given by
 * the cleaner in the previous frame. This is only necessary if
 * low-pass filtering is enabled in the settings.
 */
void EasyAnalogCleaner::clean(
    float coords[2], const Settings& settings, float previousFrameCoords[2]
) {
    //Sanitize the function arguments.
    if(!coords) return;
    coords[0] = std::clamp(coords[0], -1.0f, 1.0f);
    coords[1] = std::clamp(coords[1], -1.0f, 1.0f);
    
    writeDebugStickValues(coords, true);
    
    //Step 1: Clamp to a unit circle.
    processUnitCircle(coords, settings);
    
    //Step 2: Process radial deadzones.
    processRadialDeadzones(coords, settings);
    
    //Step 3: Process angular deadzones.
    processAngularDeadzones(coords, settings);
    
    //Step 4: Low-pass filter.
    processLowPassFilter(coords, previousFrameCoords, settings);
    
    writeDebugStickValues(coords, false);
}


/**
 * @brief Cleans an analog button's input using the default settings.
 *
 * @param pressure Analog button pressure amount [0 - 1].
 */
void EasyAnalogCleaner::cleanButton(float* pressure) {
    cleanButton(pressure, Settings());
}


/**
 * @brief Cleans an analog button's input according to the settings.
 *
 * @param pressure Analog button pressure amount [0 - 1].
 * @param settings Settings to use.
 * @param previousFramePressure Pressure value given by the cleaner in the
 * previous frame. This is only necessary if low-pass filtering is enabled
 * in the settings.
 */
void EasyAnalogCleaner::cleanButton(
    float* pressure, const Settings& settings, float previousFramePressure
) {
    //Sanitize the function arguments.
    *pressure = std::clamp(*pressure, 0.0f, 1.0f);
    
    //Step 1: Process deadzones.
    processButtonDeadzones(pressure, settings);
    
    //Step 2: Low-pass filter.
    processLowPassFilterButton(pressure, previousFramePressure, settings);
    
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
float EasyAnalogCleaner::getSnapDirDeadzone(
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
 * an interval. Input values can go outside the input range, which results
 * in the output going outside the output range.
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
float EasyAnalogCleaner::interpolate(
    float input, float inputStart, float inputEnd,
    float outputStart, float outputEnd
) {
    float inputDiff = std::max(0.001f, inputEnd - inputStart);
    float result =
        outputStart +
        ((input - inputStart) / inputDiff) *
        (outputEnd - outputStart);
    return result;
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
float EasyAnalogCleaner::interpolateAndClamp(
    float input, float inputStart, float inputEnd,
    float outputStart, float outputEnd
) {
    float result =
        interpolate(input, inputStart, inputEnd, outputStart, outputEnd);
    result = std::clamp(result, outputStart, outputEnd);
    return result;
}


/**
 * @brief Process angular deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processAngularDeadzones(
    float coords[2], const Settings& settings
) {
    //Check if we even have anything to do.
    if(
        settings.deadzones.angular.horizontal == 0.0f &&
        settings.deadzones.angular.vertical == 0.0f &&
        settings.deadzones.angular.diagonal == 0.0f &&
        !settings.deadzones.angular.interpolate
    ) {
        return;
    }
    
    //Sanitize the settings.
    Settings sanitizedSettings = settings;
    sanitizedSettings.deadzones.angular.horizontal =
        std::clamp(sanitizedSettings.deadzones.angular.horizontal, 0.0f, 1.0f);
    sanitizedSettings.deadzones.angular.vertical =
        std::clamp(sanitizedSettings.deadzones.angular.vertical, 0.0f, 1.0f);
    sanitizedSettings.deadzones.angular.diagonal =
        std::clamp(sanitizedSettings.deadzones.angular.diagonal, 0.0f, 1.0f);
        
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
        getSnapDirDeadzone(prevSnapDirIdx, sanitizedSettings);
    float nextSnapDirDeadzone =
        getSnapDirDeadzone(nextSnapDirIdx, sanitizedSettings);
        
    //Do the clean up.
    const float inputSpaceStart =
        prevSnapDirAngle + prevSnapDirDeadzone / 2.0f;
    const float inputSpaceEnd =
        nextSnapDirAngle - nextSnapDirDeadzone / 2.0f;
    const float outputSpaceStart =
        prevSnapDirAngle;
    const float outputSpaceEnd =
        nextSnapDirAngle;
        
    if(sanitizedSettings.deadzones.angular.interpolate) {
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
 * @brief Process analog button deadzone cleaning logic.
 *
 * @param pressure Pressure value to clean.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processButtonDeadzones(
    float* pressure, const Settings& settings
) {
    //Check if we even have anything to do.
    if(
        settings.deadzones.button.unpressed == 0.0f &&
        settings.deadzones.button.pressed == 1.0f &&
        !settings.deadzones.button.interpolate
    ) {
        return;
    }
    
    //Sanitize the settings.
    Settings sanitizedSettings = settings;
    sanitizedSettings.deadzones.button.unpressed =
        std::clamp(sanitizedSettings.deadzones.button.unpressed, 0.0f, 1.0f);
    sanitizedSettings.deadzones.button.pressed =
        std::clamp(sanitizedSettings.deadzones.button.pressed, 0.0f, 1.0f);
        
    //Do the clean up.
    const float inputSpaceStart =
        sanitizedSettings.deadzones.button.unpressed;
    const float inputSpaceEnd =
        sanitizedSettings.deadzones.button.pressed;
    const float outputSpaceStart =
        0.0f;
    const float outputSpaceEnd =
        1.0f;
        
    if(sanitizedSettings.deadzones.button.interpolate) {
        //Interpolate.
        *pressure =
            interpolateAndClamp(
                *pressure,
                inputSpaceStart, inputSpaceEnd,
                outputSpaceStart, outputSpaceEnd
            );
            
    } else {
        //Hard cut-off.
        if(*pressure < inputSpaceStart) {
            *pressure = outputSpaceStart;
        }
        if(*pressure > inputSpaceEnd) {
            *pressure = outputSpaceEnd;
        }
        
    }
}


/**
 * @brief Process low-pass filtering cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param previousFrameCoords Pointer to an array of the coordinates given by
 * the cleaner in the previous frame.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processLowPassFilter(
    float coords[2], float previousFrameCoords[2], const Settings& settings
) {
    //Sanitize the settings.
    Settings sanitizedSettings = settings;
    sanitizedSettings.lowPassFilter.factor =
        std::clamp(sanitizedSettings.lowPassFilter.factor, 0.0f, 1.0f);
        
    //Check if we even have anything to do.
    if(settings.lowPassFilter.factor == 0.0f) return;
    if(!previousFrameCoords) return;
    
    //Filter.
    float finalCoords[2];
    finalCoords[0] =
        (
            coords[0] *
            sanitizedSettings.lowPassFilter.factor
        ) +
        (
            previousFrameCoords[0] *
            (1.0f - sanitizedSettings.lowPassFilter.factor)
        );
    finalCoords[1] =
        (
            coords[1] *
            sanitizedSettings.lowPassFilter.factor
        ) +
        (
            previousFrameCoords[1] *
            (1.0f - sanitizedSettings.lowPassFilter.factor)
        );
        
    coords[0] = finalCoords[0];
    coords[1] = finalCoords[1];
}


/**
 * @brief Process low-pass filtering cleaning logic for an analog button.
 *
 * @param pressure Pressure value to clean.
 * @param previousFramePressure Pressure given by the cleaner in
 * the previous frame.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processLowPassFilterButton(
    float* pressure, float previousFramePressure, const Settings& settings
) {
    //Sanitize the settings.
    Settings sanitizedSettings = settings;
    sanitizedSettings.lowPassFilter.factorButton =
        std::clamp(sanitizedSettings.lowPassFilter.factorButton, 0.0f, 1.0f);
        
    //Check if we even have anything to do.
    if(settings.lowPassFilter.factorButton == 0.0f) return;
    
    //Filter.
    *pressure =
        (
            *pressure *
            sanitizedSettings.lowPassFilter.factorButton
        ) +
        (
            previousFramePressure *
            (1.0f - sanitizedSettings.lowPassFilter.factorButton)
        );
}


/**
 * @brief Process radial deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processRadialDeadzones(
    float coords[2], const Settings& settings
) {
    //Check if we even have anything to do.
    if(
        settings.deadzones.radial.inner == 0.0f &&
        settings.deadzones.radial.outer == 1.0f &&
        !settings.deadzones.radial.interpolate
    ) {
        return;
    }
    
    //Sanitize the settings.
    Settings sanitizedSettings = settings;
    sanitizedSettings.deadzones.radial.inner =
        std::clamp(sanitizedSettings.deadzones.radial.inner, 0.0f, 1.0f);
    sanitizedSettings.deadzones.radial.outer =
        std::clamp(sanitizedSettings.deadzones.radial.outer, 0.0f, 1.0f);
        
    //Get the basics.
    float radius, angle;
    toPolar(coords, angle, radius);
    
    //Do the clean up.
    const float inputSpaceStart =
        sanitizedSettings.deadzones.radial.inner;
    const float inputSpaceEnd =
        sanitizedSettings.deadzones.radial.outer;
    const float outputSpaceStart =
        0.0f;
    const float outputSpaceEnd =
        1.0f;
        
    if(sanitizedSettings.deadzones.radial.interpolate) {
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
 * @brief Process unit circle cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void EasyAnalogCleaner::processUnitCircle(
    float coords[2], const Settings& settings
) {
    //Check if we even have anything to do.
    if(
        !settings.misc.unitCircleClamp
    ) {
        return;
    }
    
    //Do the cleanup.
    float radius, angle;
    toPolar(coords, angle, radius);
    radius = std::clamp(radius, 0.0f, 1.0f);
    toCartesian(coords, angle, radius);
}


/**
 * @brief Converts polar coordinates to Cartesian.
 *
 * @param coords Cartesian coordinates to save to.
 * @param angle Angle to use.
 * @param radius Radius to use.
 */
void EasyAnalogCleaner::toCartesian(
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
void EasyAnalogCleaner::toPolar(
    float coords[2], float& angle, float& radius
) {
    angle = (float) atan2(coords[1], coords[0]);
    radius = (float) sqrt(coords[0] * coords[0] + coords[1] * coords[1]);
}


/**
 * @brief Writes information about some coordinates to stdout.
 *
 * @param coords Coordinates to write about.
 * @param input Whether the values are the inputs or the outputs.
 */
void EasyAnalogCleaner::writeDebugStickValues(float coords[2], bool input) {
#ifdef EASY_ANALOG_CLEANER
    float angle, radius;
    toPolar(coords, angle, radius);
    if(input) std::cout << "--- Easy Analog Cleaner cleanup ---" << std::endl;
    std::cout << (input ? "Input" : "Output") << " coordinates:" << std::endl;
    std::cout << "  X, Y:      " << coords[0] << ", " << coords[1] << std::endl;
    std::cout << "  Angle rad: " << angle << std::endl;
    std::cout << "  Angle deg: " << angle * 180 / M_PI << std::endl;
    std::cout << "  Radius:    " << radius << std::endl;
    if(!input) std::cout << std::endl;
#endif
}
