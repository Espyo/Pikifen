/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Analog stick cleaner class and related functions.
 */

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
void analog_stick_cleaner::clean(float coords[2], const settings_t &settings) {
    //First, sanitize the function arguments.
    coords[0] = clamp(coords[0], -1.0f, 1.0f);
    coords[1] = clamp(coords[1], -1.0f, 1.0f);
    
    //Step 1: Process radial deadzones.
    process_radial_deadzones(coords, settings);
    
    //Step 2: Process angular deadzones.
    process_angular_deadzones(coords, settings);
}


/**
 * @brief Limits the given number to the given range, inclusive.
 *
 * @param number Number to clamp.
 * @param lower_limit Minimum value it can have, inclusive.
 * @param higher_limit Maximum value it can have, inclusive.
 * @return The clamped number.
 */
float analog_stick_cleaner::clamp(
    float value, float lower_limit, float higher_limit
) {
    value = std::max(value, lower_limit);
    value = std::min(value, higher_limit);
    return value;
}


/**
 * @brief Returns the deadzone size in the settings for the specified
 * snap direction. 0 is right, 1 is diagonal down-right, etc.
 * Due to the way this is used in the cleaning process, it also supports
 * values above 7.
 *
 * @param snap_dir_idx Index of the direction.
 * @param settings Settings to use.
 * @return The deadzone size.
 */
float analog_stick_cleaner::get_snap_dir_deadzone(
    int snap_dir_idx, const settings_t &settings
) {
    switch(snap_dir_idx % 8) {
    case 0:
    case 4: {
        return settings.angular_horizontal_deadzone;
    }
    case 2:
    case 6: {
        return settings.angular_vertical_deadzone;
    }
    default: {
        return settings.angular_diagonal_deadzone;
    }
    }
}


/**
 * @brief Returns the interpolation between two numbers, given a number in
 * an interval. Then, it clamps it to that interval.
 *
 * @param input The input number.
 * @param input_start Start of the interval the input number falls on,
 * inclusive. The closer to input_start, the closer the output is
 * to output_start.
 * @param input_end End of the interval the number falls on, inclusive.
 * @param output_start Number on the starting tip of the interpolation.
 * @param output_end Number on the ending tip of the interpolation.
 * @return The interpolated number.
 */
float analog_stick_cleaner::interpolate_and_clamp(
    float input, float input_start, float input_end,
    float output_start, float output_end
) {
    float input_diff = std::max(0.001f, input_end - input_start);
    float result =
        output_start +
        ((input - input_start) / input_diff) *
        (output_end - output_start);
    result = clamp(result, output_start, output_end);
    return result;
}


/**
 * @brief Process angular deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void analog_stick_cleaner::process_angular_deadzones(
    float coords[2], const settings_t &settings
) {
    //Get the basics.
    float radius, angle;
    to_polar(coords, angle, radius);
    angle = std::fmod(angle + M_PI * 2, M_PI * 2);
    
    //Start by finding the previous snap direction (i.e. the closest one
    //counter-clockwise), and the next snap direction (i.e. closest clockwise).
    int prev_snap_dir_idx =
        (int) (std::floor(angle / M_PI_4) + 8) % 8;
    int next_snap_dir_idx =
        prev_snap_dir_idx + 1;
    float prev_snap_dir_angle =
        M_PI_4 * prev_snap_dir_idx;
    float next_snap_dir_angle =
        M_PI_4 * next_snap_dir_idx;
    float prev_snap_dir_deadzone =
        get_snap_dir_deadzone(prev_snap_dir_idx, settings);
    float next_snap_dir_deadzone =
        get_snap_dir_deadzone(next_snap_dir_idx, settings);
        
    //Do the clean up.
    const float input_space_start =
        prev_snap_dir_angle + prev_snap_dir_deadzone / 2.0f;
    const float input_space_end =
        next_snap_dir_angle - next_snap_dir_deadzone / 2.0f;
    const float output_space_start =
        prev_snap_dir_angle;
    const float output_space_end =
        next_snap_dir_angle;
        
    if(settings.angular_deadzones_interpolate) {
        //Interpolate.
        angle =
            interpolate_and_clamp(
                angle,
                input_space_start, input_space_end,
                output_space_start, output_space_end
            );
            
    } else {
        //Hard cut-off.
        if(angle < input_space_start) {
            angle = output_space_start;
        }
        if(angle > input_space_end) {
            angle = output_space_end;
        }
    }
    
    //Finally, save the clean input.
    to_cartesian(coords, angle, radius);
}


/**
 * @brief Process radial deadzone cleaning logic.
 *
 * @param coords Coordinates to clean.
 * @param settings Settings to use.
 */
void analog_stick_cleaner::process_radial_deadzones(
    float coords[2], const settings_t &settings
) {
    //Get the basics.
    float radius, angle;
    to_polar(coords, angle, radius);
    
    //Do the clean up.
    const float input_space_start =
        settings.radial_inner_deadzone;
    const float input_space_end =
        settings.radial_outer_deadzone;
    const float output_space_start =
        0.0f;
    const float output_space_end =
        1.0f;
        
    if(settings.radial_deadzones_interpolate) {
        //Interpolate.
        radius =
            interpolate_and_clamp(
                radius,
                input_space_start, input_space_end,
                output_space_start, output_space_end
            );
            
    } else {
        //Hard cut-off.
        if(radius < input_space_start) {
            radius = output_space_start;
        }
        if(radius > input_space_end) {
            radius = output_space_end;
        }
        
    }
    
    //Finally, save the clean input.
    to_cartesian(coords, angle, radius);
}


/**
 * @brief Converts polar coordinates to Cartesian.
 *
 * @param coords Cartesian coordinates to save to.
 * @param angle Angle to use.
 * @param radius Radius to use.
 */
void analog_stick_cleaner::to_cartesian(
    float coords[2], float angle, float radius
) {
    coords[0] = (float) std::cos(angle) * radius;
    coords[1] = (float) std::sin(angle) * radius;
}


/**
 * @brief Converts Cartesian coordinates to polar.
 *
 * @param coords Cartesian coordinates to use.
 * @param angle Angle to save to.
 * @param radius Radius to save to.
 */
void analog_stick_cleaner::to_polar(
    float coords[2], float &angle, float &radius
) {
    angle = std::atan2(coords[1], coords[0]);
    radius = std::sqrt(coords[0] * coords[0] + coords[1] * coords[1]);
}
