/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General-purpose utilities used throughout the project.
 */

#include <algorithm>
#include <csignal>

#include "general_utils.h"

#include "math_utils.h"
#include "string_utils.h"


/**
 * @brief Constructs a new auto-repeater object.
 *
 * @param settings Settings to use.
 */
AutoRepeater::AutoRepeater(AutoRepeaterSettings* settings) :
    settings(settings) {
}


/**
 * @brief Signals the system to start auto-repeating.
 */
void AutoRepeater::start() {
    if(!settings) {
        stop();
        return;
    }
    time = 0.0f;
    next_trigger = settings->slowest_interval;
}


/**
 * @brief Signals the system to stop auto-repeating.
 */
void AutoRepeater::stop() {
    time = LARGE_FLOAT;
    next_trigger = LARGE_FLOAT;
}


/**
 * @brief Ticks one frame of gameplay, and returns how many times auto-repeats
 * got triggered this frame.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @return How many triggers happened.
 */
size_t AutoRepeater::tick(float delta_t) {
    if(!settings) {
        stop();
        return 0;
    }
    
    if(time == LARGE_FLOAT) return 0;
    if(time > next_trigger) return 0;
    
    time += delta_t;
    size_t triggers = 0;
    while(time >= next_trigger) {
        triggers++;
        float cur_interval =
            settings->slowest_interval +
            (time / settings->ramp_time) *
            (settings->fastest_interval - settings->slowest_interval);
        cur_interval =
            std::clamp(
                cur_interval,
                settings->fastest_interval, settings->slowest_interval
            );
        next_trigger += cur_interval;
    }
    
    return triggers;
}


/**
 * @brief Clears all items.
 */
void EnumNameDatabase::clear() {
    names.clear();
}


/**
 * @brief Returns the index number of an item, given its name.
 *
 * @param name Name of the item.
 * @return The index, or INVALID on error.
 */
size_t EnumNameDatabase::get_idx(const string &name) const {
    for(size_t n = 0; n < names.size(); n++) {
        if(names[n] == name) return n;
    }
    return INVALID;
}


/**
 * @brief Returns the name of an item, given its index number.
 *
 * @param idx Index number of the item.
 * @return The name, or an empty string on error.
 */
string EnumNameDatabase::get_name(size_t idx) const {
    if(idx < names.size()) return names[idx];
    return "";
}


/**
 * @brief Returns the number of items registered.
 *
 * @return The amount.
 */
size_t EnumNameDatabase::get_nr_of_items() const {
    return names.size();
}


/**
 * @brief Registers a new item.
 *
 * @param idx Its index number.
 * @param name Its name.
 */
void EnumNameDatabase::register_item(
    size_t idx, const string &name
) {
    if(idx >= names.size()) {
        names.insert(names.end(), (idx + 1) - names.size(), "");
    }
    names[idx] = name;
}


/**
 * @brief Reads a generic value from a string.
 *
 * @tparam The generic type.
 * @param s The string.
 * @return The value.
 */
template<typename t>
t from_string(const string &s) {
    return t{};
}


/**
 * @brief Reads a float value from a string.
 *
 * @param s The string.
 * @return The value.
 */
template<>
float from_string<float>(const string &s) {
    return s2f(s);
}


/**
 * @brief Reads a color value from a string.
 *
 * @param s The string.
 * @return The value.
 */
template<>
ALLEGRO_COLOR from_string<ALLEGRO_COLOR>(const string &s) {
    return s2c(s);
}


/**
 * @brief Reads a point value from a string.
 *
 * @param s The string.
 * @return The value.
 */
template<>
Point from_string<Point>(const string &s) {
    return s2p(s);
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
void MovementInfo::get_info(
    Point* coords, float* angle, float* magnitude
) const {
    *coords = Point(right - left, down - up);
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
void MovementInfo::reset() {
    right = 0.0f;
    up = 0.0f;
    left = 0.0f;
    down = 0.0f;
}



/**
 * @brief Constructs a new timer object.
 *
 * @param duration How long before it reaches the end, in seconds.
 * @param on_end Code to run when time ends.
 */
Timer::Timer(float duration, const std::function<void()> &on_end) :
    time_left(0),
    duration(duration),
    on_end(on_end) {
    
    
}


/**
 * @brief Destroys the timer object.
 */
Timer::~Timer() {
    //TODO Valgrind detects a leak with on_end...
}


/**
 * @brief Returns the ratio of time left
 * (i.e. 0 if done, 1 if all time is left).
 *
 * @return The ratio left.
 */
float Timer::get_ratio_left() const {
    return time_left / duration;
}



/**
 * @brief Starts a timer.
 *
 * @param can_restart If false, calling this while the timer is still
 * ticking down will not do anything.
 */
void Timer::start(bool can_restart) {
    if(!can_restart && time_left > 0) return;
    time_left = duration;
}


/**
 * @brief Starts a timer, but sets a new duration.
 *
 * @param new_duration Its new duration.
 */
void Timer::start(float new_duration) {
    duration = new_duration;
    start();
}


/**
 * @brief Stops a timer, without executing the on_end callback.
 */
void Timer::stop() {
    time_left = 0.0f;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Timer::tick(float delta_t) {
    if(time_left == 0.0f) return;
    time_left = std::max(time_left - delta_t, 0.0f);
    if(time_left == 0.0f && on_end) {
        on_end();
    }
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
    for(size_t c = 0; c < s.size(); c++) {
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


/**
 * @brief Returns a string that's a join of the strings in the specified vector,
 * but only past a certain position. The strings are joined with a space
 * character.
 *
 * @param v The vector of strings.
 * @param pos Use the string at this position and onward.
 * @return The joined string.
 */
string vector_tail_to_string(const vector<string> &v, size_t pos) {
    string result = v[pos];
    for(size_t p = pos + 1; p < v.size(); p++) {
        result += " " + v[p];
    }
    return result;
}
