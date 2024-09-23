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

#include <string>
#include <vector>

#include "geometry_utils.h"
#include "math_utils.h"
#include "allegro_utils.h"

using std::string;
using std::vector;


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



/**
 * @brief A struct that makes it simpler to obtain data
 * for a given simple keyframe animation based on interpolation.
 */

template<typename inter_t>
struct keyframe_interpolator {

    public:
    
    //--- Function declarations ---
    
    explicit keyframe_interpolator(const inter_t initial_value) {
        keyframe_times.push_back(0.0f);
        keyframe_values.push_back(initial_value);
        keyframe_eases.push_back(EASE_METHOD_NONE);
    };

    inter_t get(const float t) {
        if (t < 0.0f) return keyframe_values[0];

        if (t < keyframe_times[0]) {
            return keyframe_values[0];
        }

        for (size_t k = 1; k < keyframe_times.size(); ++k) {
            if (t <= keyframe_times[k]) {
                float delta_t = std::max(keyframe_times[k] - keyframe_times[k - 1], 0.01f);
                float relative_t = t - keyframe_times[k - 1];
                float ratio = relative_t / delta_t;
                ratio = ease(keyframe_eases[k], ratio);
                return interpolate(keyframe_values[k - 1], keyframe_values[k], ratio);
            }
        }

        return keyframe_values.back();
    }
    void add(
        const float t, const inter_t value, EASING_METHOD ease = EASE_METHOD_NONE,
        int* out_idx = nullptr
    ) {
        int new_idx = get_insertion_index(t);

        if(out_idx)
            *out_idx = new_idx;

        keyframe_times.insert(keyframe_times.begin() + new_idx, t);
        keyframe_values.insert(keyframe_values.begin() + new_idx, value);
        keyframe_eases.insert(keyframe_eases.begin() + new_idx, ease);
    }

    void remove(int idx) {
        keyframe_times.erase(keyframe_times.begin() + idx);
        keyframe_values.erase(keyframe_values.begin() + idx);
        keyframe_eases.erase(keyframe_eases.begin() + idx);
    }

    int keyframe_count() { return keyframe_times.size(); }

    std::pair<float, inter_t> get_keyframe(int idx) {
        return std::make_pair(keyframe_times[idx], keyframe_values[idx]);
    }

    void set_keyframe_value(int idx, inter_t value) {
        keyframe_values[idx] = value;
    }

    void set_keyframe_time(int idx, float time, int* out_new_idx = nullptr) {
        int cur_idx = idx;

        while (cur_idx > 0 && time < keyframe_times[cur_idx - 1]) {
            std::swap(keyframe_times[cur_idx], keyframe_times[cur_idx - 1]);
            std::swap(keyframe_values[cur_idx], keyframe_values[cur_idx - 1]);
            std::swap(keyframe_eases[cur_idx], keyframe_eases[cur_idx - 1]);
            cur_idx--;
        }
        while (cur_idx < (keyframe_count() - 1) && time > keyframe_times[cur_idx + 1]) {
            std::swap(keyframe_times[cur_idx], keyframe_times[cur_idx + 1]);
            std::swap(keyframe_values[cur_idx], keyframe_values[cur_idx + 1]);
            std::swap(keyframe_eases[cur_idx], keyframe_eases[cur_idx + 1]);
            cur_idx++;
        }

        if (out_new_idx)
            *out_new_idx = cur_idx;

        keyframe_times[cur_idx] = time;
    }

    private:
    
    //--- Members ---
    
    //Keyframe times.
    vector<float> keyframe_times;
    
    //Keyframe values.
    vector<inter_t> keyframe_values;
    
    //Keyframe easing methods.
    vector<EASING_METHOD> keyframe_eases;
    
    int get_insertion_index(float t) {
        int idx = 0;

        for (; idx < keyframe_times.size(); idx++) {
            if (keyframe_times[idx] >= t)
                break;
        }
        return idx;
    }


    float interpolate(float v1, float v2, float time) {
        return v1 + (v2 - v1) * time;
    }
    ALLEGRO_COLOR interpolate(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float time) {
        return interpolate_color(time, 0, 1, c1, c2);
    }
    point interpolate(point p1, point p2, float time) {
        return interpolate_point(time, 0, 1, p1, p2);
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



string get_current_time(const bool file_name_friendly);
string sanitize_file_name(const string &s);
string standardize_path(const string &path);
string vector_tail_to_string(const vector<string> &v, const size_t pos);


/**
 * @brief Returns whether or not the two vectors contain the same items,
 * regardless of order.
 *
 * @tparam t The contents of the vector.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Whether they contain the same items.
 */

template<typename t>
bool vectors_contain_same(const vector<t> &v1, const vector<t> &v2) {
    if(v1.size() != v2.size()) {
        return false;
    }
    
    for(size_t i1 = 0; i1 < v1.size(); ++i1) {
        bool found = false;
        for(size_t i2 = 0; i2 < v2.size(); ++i2) {
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


#if defined(_WIN32)
string strsignal(const int signum);
#endif //#if defined(_WIN32)
