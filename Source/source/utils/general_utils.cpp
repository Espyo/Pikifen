/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General-purpose utilities used throughout the project.
 */

#include "general_utils.h"

#include "math_utils.h"
#include "string_utils.h"



/**
 * @brief Constructs a new keyframe interpolator object.
 *
 * @param initial_value Initial value of the thing being interpolated.
 * This gets used at t = 0.
 */
keyframe_interpolator::keyframe_interpolator(const float initial_value) {
    keyframe_times.push_back(0.0f);
    keyframe_values.push_back(initial_value);
    keyframe_eases.push_back(EASE_METHOD_NONE);
}


/**
 * @brief Adds a new keyframe.
 *
 * @param t Time in which this keyframe takes place. Ranges from 0 to 1.
 * @param value Value of the thing to interpolate at the keyframe's moment.
 * @param ease Easing method, if any.
 */
void keyframe_interpolator::add(
    const float t, const float value, const EASING_METHOD ease
) {
    keyframe_times.push_back(t);
    keyframe_values.push_back(value);
    keyframe_eases.push_back(ease);
}


/**
 * @brief Returns the value at a given point in time.
 *
 * @param t Time.
 * @return The value.
 */
float keyframe_interpolator::get(const float t) {
    if(t < 0.0f) return keyframe_values[0];
    
    for(size_t k = 1; k < keyframe_times.size(); ++k) {
        if(t <= keyframe_times[k]) {
            float delta_t = keyframe_times[k] - keyframe_times[k - 1];
            float relative_t = t - keyframe_times[k - 1];
            float ratio = relative_t / delta_t;
            ratio = ease(keyframe_eases[k], ratio);
            float delta_v = keyframe_values[k] - keyframe_values[k - 1];
            return delta_v * ratio + keyframe_values[k - 1];
        }
    }
    
    return keyframe_values.back();
}


/**
 * @brief Returns the values of the coordinates, magnitude, and angle,
 * but "cleaned" up.
 * All parameters are mandatory.
 *
 * @param coords Final X and Y coordinates.
 * @param angle Angle compared to the center.
 * @param magnitude Magnitude from the center.
 */
void movement_t::get_info(
    point* coords, float* angle, float* magnitude
) const {
    *coords = point(right - left, down - up);
    coordinates_to_angle(*coords, angle, magnitude);
    
    //While analog sticks are already correctly clamped between 0 and 1 for
    //magnitude, via the controls manager, digital inputs aren't, e.g. pressing
    //W and D on the keyboard.
    *magnitude = std::max(*magnitude, 0.0f);
    *magnitude = std::min(*magnitude, 1.0f);
}


/**
 * @brief Resets the information.
 */
void movement_t::reset() {
    right = 0.0f;
    up = 0.0f;
    left = 0.0f;
    down = 0.0f;
}



/**
 * @brief Returns a string representing the current date and time.
 *
 * @param filename_friendly If true, slashes become dashes,
 * and semicolons become dots.
 * @return The string.
 */
string get_current_time(const bool filename_friendly) {
    time_t tt;
    time(&tt);
    struct tm t = *localtime(&tt);
    return
        i2s(t.tm_year + 1900) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mon + 1) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mday) +
        (filename_friendly ? "_" : " ") +
        leading_zero(t.tm_hour) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_min) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_sec);
}


/**
 * @brief Sanitizes a file name (or part of it), such that it doesn't use any
 * weird characters.
 * Do not use on paths, since colons, slashes, and backslashes will be replaced!
 *
 * @param s File name to sanitize.
 * @return The sanitized file name.
 */
string sanitize_file_name(const string &s) {
    string ret;
    ret.reserve(s.size());
    for(size_t c = 0; c < s.size(); ++c) {
        if(
            (s[c] >= 'A' && s[c] <= 'Z') ||
            (s[c] >= 'a' && s[c] <= 'z') ||
            (s[c] >= '0' && s[c] <= '9') ||
            s[c] == '-' ||
            s[c] == ' '
        ) {
            ret.push_back(s[c]);
        } else {
            ret.push_back('_');
        }
    }
    return ret;
}


/**
 * @brief Standardizes a path, making it use forward slashes instead of
 * backslashes, and removing excess slashes at the end.
 *
 * @param path Path to standardize.
 * @return The standardized path.
 */
string standardize_path(const string &path) {
    string res = replace_all(path, "\\", "/");
    if(res.back() == '/') res.pop_back();
    return res;
}


#if defined(_WIN32)


/**
 * @brief An implementation of strsignal from POSIX.
 *
 * @param signum Signal number.
 * @return The string.
 */
string strsignal(const int signum) {
    switch(signum) {
    case SIGINT: {
        return "SIGINT";
    } case SIGILL: {
        return "SIGILL";
    } case SIGFPE: {
        return "SIGFPE";
    } case SIGSEGV: {
        return "SIGSEGV";
    } case SIGTERM: {
        return "SIGTERM";
    } case SIGBREAK: {
        return "SIGBREAK";
    } case SIGABRT: {
        return "SIGABRT";
    } case SIGABRT_COMPAT: {
        return "SIGABRT_COMPAT";
    } default: {
        return "Unknown";
    }
    }
}


#endif //if defined(_WIN32)


/**
 * @brief Returns a string that's a join of the strings in the specified vector,
 * but only past a certain position. The strings are joined with a space
 * character.
 *
 * @param v The vector of strings.
 * @param pos Use the string at this position and onward.
 * @return The joined string.
 */
string vector_tail_to_string(const vector<string> &v, const size_t pos) {
    string result = v[pos];
    for(size_t p = pos + 1; p < v.size(); ++p) {
        result += " " + v[p];
    }
    return result;
}


