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

#include <functional>
#include <string>
#include <vector>

#include "geometry_utils.h"
#include "math_utils.h"

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
 */
struct keyframe_interpolator {

    public:
    
    //--- Function declarations ---
    
    float get(float t);
    void add(
        float t, float value, EASING_METHOD ease = EASE_METHOD_NONE
    );
    explicit keyframe_interpolator(float initial_value);
    
    private:
    
    //--- Members ---
    
    //Keyframe times.
    vector<float> keyframe_times;
    
    //Keyframe values.
    vector<float> keyframe_values;
    
    //Keyframe easing methods.
    vector<EASING_METHOD> keyframe_eases;
    
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
string strsignal(int signum);
#endif //#if defined(_WIN32)
