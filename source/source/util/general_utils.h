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
#include <unordered_set>
#include <vector>

#include "../lib/data_file/data_file.h"
#include "allegro_utils.h"
#include "geometry_utils.h"
#include "math_utils.h"
#include "string_utils.h"

using std::string;
using std::vector;
using std::unordered_set;


//Bitmask with 8 bits.
typedef uint8_t bitmask_8_t;

//Bitmask with 16 bits.
typedef uint16_t bitmask_16_t;

//Bitmask with 32 bits.
typedef uint32_t bitmask_32_t;


//Turns a bit in a bitmask off.
#define disableFlag(flags, flag) (flags &= ~flag)

//Turns a bit in a bitmask on.
#define enableFlag(flags, flag) (flags |= flag)

//Returns the bitmask corresponding to a certain index. Useful for flags.
#define getIdxBitmask(i) (1 << i)

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define getPrevInVector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define getNextInVector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Returns whether a bit is on or not in a bitmask.
#define hasFlag(flags, flag) (((flags) & (flag)) > 0)


//Cross-platform way of representing an invalid index.
constexpr size_t INVALID = UINT32_MAX;

//Cross-platform way of representing a float value of "invalid" or similar.
constexpr float LARGE_FLOAT = 999999.0f;


template<typename t>
t fromString(const string &s);
template<>
float fromString<float>(const string &s);
template<>
ALLEGRO_COLOR fromString<ALLEGRO_COLOR>(const string &s);
template<>
Point fromString<Point>(const string &s);


/**
 * @brief Customizes how an auto-repeater's triggers change in frequency
 * over time.
 */
struct AutoRepeaterSettings {

    //--- Members ---
    
    //Interval between triggers, at the slowest speed.
    float slowest_interval = 0.3f;
    
    //Interval between triggers, at the fastest speed.
    float fastest_interval = 0.05f;
    
    //How long it takes for the trigger intervals to reach the fastest speed.
    float ramp_time = 0.9f;
    
};


/**
 * @brief Allows something to be automatically repeated over time, with logic
 * to repeat more quickly the longer it's active for.
 */
struct AutoRepeater {

    //--- Members ---
    
    //How long this auto-repeater has been active for. LARGE_FLOAT if inactive.
    float time = LARGE_FLOAT;
    
    //When the next trigger will happen. LARGE_FLOAT if inactive.
    float next_trigger = LARGE_FLOAT;
    
    //Settings to use.
    AutoRepeaterSettings* settings = nullptr;
    
    
    //--- Function declarations ---
    
    AutoRepeater(AutoRepeaterSettings* settings);
    void start();
    void stop();
    size_t tick(float delta_t);
    
};


/**
 * @brief Just a list of different elements in an enum and what their names are.
 */
struct EnumNameDatabase {

    public:
    
    //--- Function declarations ---
    
    void registerItem(size_t enum_idx, const string &name);
    size_t getIdx(const string &name) const;
    string getName(size_t idx) const;
    size_t getNrOfItems() const;
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
template<typename InterT>
struct KeyframeInterpolator {

    public:
    
    //--- Function definitions ---
    
    explicit KeyframeInterpolator(const InterT &initial_value = InterT()) {
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
    InterT get(float t) {
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
        float t, const InterT &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* out_idx = nullptr
    ) {
        size_t new_idx = getInsertionIdx(t);
        
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
    void addOrSet(
        float t, const InterT &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* out_idx = nullptr
    ) {
        for(size_t k = 0; k < keyframe_times.size(); ++k) {
            if(keyframe_times[k] == t) {
                if(out_idx) *out_idx = k;
                setKeyframeValue(k, value);
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
    size_t getKeyframeCount() {
        return keyframe_times.size();
    }
    
    
    /**
     * @brief Gets data about the keyframe at the specified index.
     *
     * @param idx The keyframe's index.
     * @return A pair with the keyframe's time and value.
     */
    std::pair<float, InterT> getKeyframe(size_t idx) {
        return std::make_pair(keyframe_times[idx], keyframe_values[idx]);
    }
    
    
    /**
     * @brief Sets the value of the keyframe at the specified index.
     *
     * @param idx They keyframe's index.
     * @param value The new value.
     */
    void setKeyframeValue(size_t idx, const InterT &value) {
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
    void setKeyframeTime(
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
            cur_idx < (getKeyframeCount() - 1) &&
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
    void loadFromDataNode(DataNode* node) {
        if(node->getNrOfChildren() == 0) {
            //There are no values to load, let's not even try.
            return;
        }
        
        keyframe_times.clear();
        keyframe_values.clear();
        keyframe_eases.clear();
        
        for(size_t c = 0; c < node->getNrOfChildren(); c++) {
            DataNode* c_node = node->getChild(c);
            InterT value = fromString<InterT>(c_node->value);
            add(s2f(c_node->name), value, EASE_METHOD_NONE);
        }
    }
    
    
private:

    //--- Members ---
    
    //Keyframe times.
    vector<float> keyframe_times;
    
    //Keyframe values.
    vector<InterT> keyframe_values;
    
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
    size_t getInsertionIdx(float t) {
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
        return interpolateColor(time, 0.0f, 1.0f, c1, c2);
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
    Point interpolate(
        const Point &p1, const Point &p2, float time
    ) {
        return interpolatePoint(time, 0.0f, 1.0f, p1, p2);
    }
    
};



/**
 * @brief Info about where the player wants a leader
 * (or something else) to go, based on the player's inputs.
 */
struct MovementInfo {

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
    
    void getInfo(Point* coords, float* angle, float* magnitude) const;
    void reset();
    
};


/**
 * @brief A timer. You can set it to start at a pre-determined time,
 * to tick, etc.
 */
struct Timer {

    //--- Members ---
    
    //How much time is left until 0.
    float time_left = 0.0f;
    
    //When the timer starts, its time is set to this.
    float duration = 0.0f;
    
    //Code to run when the timer ends, if any.
    std::function<void()> on_end = nullptr;
    
    
    //--- Function declarations ---
    
    explicit Timer(
        float duration = 0,
        const std::function<void()> &on_end = nullptr
    );
    ~Timer();
    void start(bool can_restart = true);
    void start(float new_duration);
    void stop();
    void tick(float delta_t);
    float getRatioLeft() const;
    
};


string getCurrentTime(bool file_name_friendly);
string sanitizeFileName(const string &s);


/**
 * @brief Deterministically randomly shuffles the contents of a vector.
 *
 * @tparam t Type of contents of the vector.
 * @param v The vector to shuffle.
 * @param pick_random_floats Vector of previously-determined random floats to
 * calculate the picks with [0, 1]. This vector must be of the same size
 * as the input vector.
 * @return The shuffled vector.
 */
template<typename t>
vector<t> shuffleVector(
    const vector<t> &v, const vector<float> pick_random_floats
) {
    vector<t> result;
    vector<t> items_available = v;
    for(size_t i = 0; i < v.size(); i++) {
        size_t pick = pick_random_floats[i] * (items_available.size());
        //Add a safeguard for if the float is exactly 1.0.
        pick = std::min(pick, items_available.size() - 1);
        result.push_back(items_available[pick]);
        items_available.erase(items_available.begin() + pick);
    }
    return result;
}


string standardizePath(const string &path);
string vectorTailToString(const vector<string> &v, size_t pos);


/**
 * @brief Shorthand for figuring out if a given item is in a container.
 *
 * @tparam t Type of container.
 * @tparam i Type of contents of the container.
 * @param cont The container.
 * @param item Item to check.
 * @return Whether it contains the item.
 */
template<typename t, typename i>
bool isInContainer(const t &cont, const i &item) {
    return std::find(cont.begin(), cont.end(), item) != cont.end();
}


/**
 * @brief Removes elements from a vector if they show up in the ban list.
 *
 * @tparam t Type of contents of the vector and ban list.
 * @param v Vector to filter.
 * @param ban_list List of items that must be banned.
 * @return The filtered vector.
 */
template<typename t>
vector<t> filterVectorWithBanList(
    const vector<t> &v, const vector<t> &ban_list
) {
    vector<t> result = v;
    for(size_t i = 0; i < result.size();) {
        if(isInContainer(ban_list, result[i])) {
            result.erase(result.begin() + i);
        } else {
            i++;
        }
    }
    return result;
}


/**
 * @brief Removes all instances of a given item inside of a vector.
 *
 * @tparam t Type of the vector's contents.
 * @param item Item to compare with.
 * @param vec Vector to check.
 * @return The finished vector.
 */
template<typename t>
vector<t> removeAllInVector(
    const t &item, const vector<t> &vec
) {
    vector<t> result = vec;
    for(size_t i = 0; i < result.size();) {
        if(result[i] == item) {
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
vector<t> sortVectorWithPreferenceList(
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
bool vectorsContainSame(const vector<t> &v1, const vector<t> &v2) {
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
