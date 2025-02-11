/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General-purpose utilities used throughout the project.
 */

#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "../lib/data_file/data_file.h"
#include "allegro_utils.h"
#include "geometry_utils.h"
#include "math_utils.h"
#include "string_utils.h"

using std::string;
using std::vector;


typedef uint8_t bitmask_8_t;
typedef uint16_t bitmask_16_t;
typedef uint32_t bitmask_32_t;


//Turns a bit in a bitmask off.
#define disable_flag(flags, flag) (flags &= ~flag)

//Turns a bit in a bitmask on.
#define enable_flag(flags, flag) (flags |= flag)

//Returns the bitmask corresponding to a certain index. Useful for flags.
#define get_idx_bitmask(i) (1 << i)

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Returns whether a bit is on or not in a bitmask.
#define has_flag(flags, flag) (((flags) & (flag)) > 0)


//Cross-platform way of representing an invalid index.
constexpr size_t INVALID = UINT32_MAX;

//Cross-platform way of representing a float value of "invalid" or similar.
constexpr float LARGE_FLOAT = 999999.0f;


template<typename t>
t from_string(const string &s);
template<>
float from_string<float>(const string &s);
template<>
ALLEGRO_COLOR from_string<ALLEGRO_COLOR>(const string &s);
template<>
point from_string<point>(const string &s);



/**
 * @brief Just a list of different elements in an enum and what their names are.
 */
struct enum_name_database {

    public:
    
    //--- Function declarations ---
    
    void register_item(size_t enum_idx, const string &name);
    size_t get_idx(const string &name) const;
    string get_name(size_t idx) const;
    size_t get_nr_of_items() const;
    void clear();
    
    private:
    
    //--- Members ---
    
    //Known items.
    vector<string> names;
    
};


/**
 * @brief A struct that makes it simpler to obtain data
 * for a given simple keyframe animation based on interpolation.
 * Keyframe times go from 0 (beginning) to 1 (end).
 */
template<typename inter_t>
struct keyframe_interpolator {

    public:
    
    //--- Function definitions ---
    
    explicit keyframe_interpolator(const inter_t &initial_value = inter_t()) {
        keyframe_times.push_back(0.0f);
        keyframe_values.push_back(initial_value);
        keyframe_eases.push_back(EASE_METHOD_NONE);
    };
    
    
    /**
     * @brief Returns the interpolated value at a given time.
     *
     * @param t The time.
     * @return The value.
     */
    inter_t get(float t) {
        if(t < 0.0f) return keyframe_values[0];
        
        if(t < keyframe_times[0]) {
            return keyframe_values[0];
        }
        
        for(size_t k = 1; k < keyframe_times.size(); ++k) {
            if(t <= keyframe_times[k]) {
                float delta_t =
                    std::max(keyframe_times[k] - keyframe_times[k - 1], 0.01f);
                float relative_t =
                    t - keyframe_times[k - 1];
                float ratio =
                    relative_t / delta_t;
                ratio = ease(keyframe_eases[k], ratio);
                return
                    interpolate(
                        keyframe_values[k - 1], keyframe_values[k], ratio
                    );
            }
        }
        
        return keyframe_values.back();
    }
    
    
    /**
     * @brief Adds a keyframe.
     *
     * @param t Time (0 to 1).
     * @param value Value at that point.
     * @param ease Easing method between it and the previous keyframe.
     * @param out_idx If not nullptr, the index of the newly added keyframe
     * is returned here.
     */
    void add(
        float t, const inter_t &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* out_idx = nullptr
    ) {
        size_t new_idx = get_insertion_idx(t);
        
        if(out_idx) *out_idx = new_idx;
        
        keyframe_times.insert(keyframe_times.begin() + new_idx, t);
        keyframe_values.insert(keyframe_values.begin() + new_idx, value);
        keyframe_eases.insert(keyframe_eases.begin() + new_idx, ease);
    }
    
    
    /**
     * @brief Adds a keyframe, or sets the data of the keyframe at the
     * specified time.
     *
     * @param t Time (0 to 1).
     * @param value Value at that point.
     * @param ease Easing method between it and the previous keyframe.
     * @param out_idx If not nullptr, the index of the newly added keyframe
     * is returned here.
     */
    void add_or_set(
        float t, const inter_t &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* out_idx = nullptr
    ) {
        for(size_t k = 0; k < keyframe_times.size(); ++k) {
            if(keyframe_times[k] == t) {
                if(out_idx) *out_idx = k;
                set_keyframe_value(k, value);
                return;
            }
        }
        
        add(t, value, ease, out_idx);
    }
    
    
    /**
     * @brief Removes a keyframe.
     *
     * @param idx Its index.
     */
    void remove(size_t idx) {
        keyframe_times.erase(keyframe_times.begin() + idx);
        keyframe_values.erase(keyframe_values.begin() + idx);
        keyframe_eases.erase(keyframe_eases.begin() + idx);
    }
    
    
    /**
     * @brief Returns how many keyframes there are.
     *
     * @return The total.
     */
    size_t get_keyframe_count() {
        return keyframe_times.size();
    }
    
    
    /**
     * @brief Gets data about the keyframe at the specified index.
     *
     * @param idx The keyframe's index.
     * @return A pair with the keyframe's time and value.
     */
    std::pair<float, inter_t> get_keyframe(size_t idx) {
        return std::make_pair(keyframe_times[idx], keyframe_values[idx]);
    }
    
    
    /**
     * @brief Sets the value of the keyframe at the specified index.
     *
     * @param idx They keyframe's index.
     * @param value The new value.
     */
    void set_keyframe_value(size_t idx, const inter_t &value) {
        keyframe_values[idx] = value;
    }
    
    
    /**
     * @brief Sets the time of the keyframe at the specified index.
     *
     * @param idx The keyframe's index.
     * @param time The new time.
     * @param out_new_idx If not nullptr, the new index of the keyframe is
     * returned here.
     */
    void set_keyframe_time(
        size_t idx, float time, size_t* out_new_idx = nullptr
    ) {
        size_t cur_idx = idx;
        
        while(
            cur_idx > 0 &&
            time < keyframe_times[cur_idx - 1]
        ) {
            std::swap(keyframe_times[cur_idx], keyframe_times[cur_idx - 1]);
            std::swap(keyframe_values[cur_idx], keyframe_values[cur_idx - 1]);
            std::swap(keyframe_eases[cur_idx], keyframe_eases[cur_idx - 1]);
            cur_idx--;
        }
        while(
            cur_idx < (get_keyframe_count() - 1) &&
            time > keyframe_times[cur_idx + 1]
        ) {
            std::swap(keyframe_times[cur_idx], keyframe_times[cur_idx + 1]);
            std::swap(keyframe_values[cur_idx], keyframe_values[cur_idx + 1]);
            std::swap(keyframe_eases[cur_idx], keyframe_eases[cur_idx + 1]);
            cur_idx++;
        }
        
        if(out_new_idx) *out_new_idx = cur_idx;
        
        keyframe_times[cur_idx] = time;
    }
    
    
    /**
     * @brief Loads interpolator data from a data node.
     *
     * @param node The data node to load from.
     */
    void load_from_data_node(data_node* node) {
        if(node->get_nr_of_children() == 0) {
            //There are no values to load, let's not even try.
            return;
        }
        
        keyframe_times.clear();
        keyframe_values.clear();
        keyframe_eases.clear();
        
        for(size_t c = 0; c < node->get_nr_of_children(); c++) {
            data_node* c_node = node->get_child(c);
            inter_t value = from_string<inter_t>(c_node->value);
            add(s2f(c_node->name), value, EASE_METHOD_NONE);
        }
    }
    
    
private:

    //--- Members ---
    
    //Keyframe times.
    vector<float> keyframe_times;
    
    //Keyframe values.
    vector<inter_t> keyframe_values;
    
    //Keyframe easing methods.
    vector<EASING_METHOD> keyframe_eases;
    
    
    //--- Function definitions ---
    
    /**
     * @brief Returns the index at which a keyframe would be inserted to,
     * given the specified time.
     *
     * @param t The time.
     * @return The index.
     */
    size_t get_insertion_idx(float t) {
        size_t idx = 0;
        for(; idx < keyframe_times.size(); idx++) {
            if(keyframe_times[idx] >= t) break;
        }
        return idx;
    }
    
    
    /**
     * @brief Interpolates between two floats, and returns the value at the
     * specified time.
     *
     * @param v1 First value.
     * @param v2 Second value.
     * @param time Time (0 to 1).
     * @return The value.
     */
    float interpolate(float v1, float v2, float time) {
        return v1 + (v2 - v1) * time;
    }
    
    
    /**
     * @brief Interpolates between two colors, and returns the color at the
     * specified time.
     *
     * @param c1 First color.
     * @param c2 Second color.
     * @param time Time (0 to 1).
     * @return The color.
     */
    ALLEGRO_COLOR interpolate(
        const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2, float time
    ) {
        return interpolate_color(time, 0.0f, 1.0f, c1, c2);
    }
    
    
    /**
     * @brief Interpolates between two points, and returns the point at the
     * specified time.
     *
     * @param c1 First point.
     * @param c2 Second point.
     * @param time Time (0 to 1).
     * @return The point.
     */
    point interpolate(
        const point &p1, const point &p2, float time
    ) {
        return interpolate_point(time, 0.0f, 1.0f, p1, p2);
    }
    
};



/**
 * @brief Info about where the player wants a leader
 * (or something else) to go, based on the player's inputs.
 */
struct movement_t {

    //--- Members ---
    
    //Amount to the east.
    float right = 0.0f;
    
    //Amount to the north.
    float up = 0.0f;
    
    //Amount to the west.
    float left = 0.0f;
    
    //Amount to the south.
    float down = 0.0f;
    
    
    //--- Function declarations ---
    
    void get_info(point* coords, float* angle, float* magnitude) const;
    void reset();
    
};


/**
 * @brief A timer. You can set it to start at a pre-determined time,
 * to tick, etc.
 */
struct timer {

    //--- Members ---
    
    //How much time is left until 0.
    float time_left = 0.0f;
    
    //When the timer starts, its time is set to this.
    float duration = 0.0f;
    
    //Code to run when the timer ends, if any.
    std::function<void()> on_end = nullptr;
    
    
    //--- Function declarations ---
    
    explicit timer(
        float duration = 0,
        const std::function<void()> &on_end = nullptr
    );
    ~timer();
    void start(bool can_restart = true);
    void start(float new_duration);
    void stop();
    void tick(float delta_t);
    float get_ratio_left() const;
    
};


string get_current_time(bool file_name_friendly);
string sanitize_file_name(const string &s);
string standardize_path(const string &path);
string vector_tail_to_string(const vector<string> &v, size_t pos);



/**
 * @brief Removes elements from a vector if they show up in the ban list.
 *
 * @tparam t Type of contents of the vector and ban list.
 * @param v Vector to filter.
 * @param ban_list List of items that must be banned.
 * @return The filtered vector.
 */
template<typename t>
vector<t> filter_vector_with_ban_list(
    const vector<t> &v, const vector<t> &ban_list
) {
    vector<t> result = v;
    for(size_t i = 0; i < result.size();) {
        if(
            std::find(ban_list.begin(), ban_list.end(), result[i]) !=
            ban_list.end()
        ) {
            result.erase(result.begin() + i);
        } else {
            i++;
        }
    }
    return result;
}


/**
 * @brief Sorts a vector, using the preference list to figure out which
 * elements go before which. Elements not in the preference list will go
 * to the end, in the same order as they are presented in the original
 * vector.
 *
 * @tparam t Type of contents of the vector.
 * @param v Vector to sort.
 * @param preference_list Preference list.
 * @param equal If not nullptr, use this function to compare whether
 * an item of t1 matches an item of t2.
 * @param less If not nullptr, use this function to sort missing items with.
 * @param unknowns If not nullptr, unknown preferences (i.e. items in
 * the preference list but not inside the vector) will be added here.
 * @return The sorted vector.
 */
template<typename t>
vector<t> sort_vector_with_preference_list(
    const vector<t> &v, const vector<t> preference_list,
    vector<t>* unknowns = nullptr
) {
    vector<t> result;
    vector<t> missing_items;
    result.reserve(v.size());
    
    //Sort the existing items.
    for(size_t p = 0; p < preference_list.size(); p++) {
        bool found_in_vector = false;
        for(auto &i : v) {
            if(i == preference_list[p]) {
                found_in_vector = true;
                result.push_back(i);
                break;
            }
        }
        if(!found_in_vector && unknowns) {
            unknowns->push_back(preference_list[p]);
        }
    }
    
    //Find the missing items.
    for(auto &i : v) {
        bool found_in_preferences = false;
        for(auto &p : preference_list) {
            if(i == p) {
                found_in_preferences = true;
                break;
            }
        }
        if(!found_in_preferences) {
            //Missing from the list? Add it to the "missing" pile.
            missing_items.push_back(i);
        }
    }
    
    //Sort and place the missing items.
    if(!missing_items.empty()) {
        std::sort(
            missing_items.begin(),
            missing_items.end()
        );
        result.insert(
            result.end(),
            missing_items.begin(),
            missing_items.end()
        );
    }
    
    return result;
}


/**
 * @brief Returns whether or not the two vectors contain the same items,
 * regardless of order.
 *
 * @tparam t Type of contents of the vector.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Whether they contain the same items.
 */
template<typename t>
bool vectors_contain_same(const vector<t> &v1, const vector<t> &v2) {
    if(v1.size() != v2.size()) {
        return false;
    }
    
    for(size_t i1 = 0; i1 < v1.size(); i1++) {
        bool found = false;
        for(size_t i2 = 0; i2 < v2.size(); i2++) {
            if(v1[i1] == v2[i2]) {
                found = true;
                break;
            }
        }
        if(!found) {
            return false;
        }
    }
    
    return true;
}
