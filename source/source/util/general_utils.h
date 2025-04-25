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
typedef uint8_t Bitmask8;

//Bitmask with 16 bits.
typedef uint16_t Bitmask16;

//Bitmask with 32 bits.
typedef uint32_t Bitmask32;


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
    float slowestInterval = 0.3f;
    
    //Interval between triggers, at the fastest speed.
    float fastestInterval = 0.05f;
    
    //How long it takes for the trigger intervals to reach the fastest speed.
    float rampTime = 0.9f;
    
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
    float nextTrigger = LARGE_FLOAT;
    
    //Settings to use.
    AutoRepeaterSettings* settings = nullptr;
    
    
    //--- Function declarations ---
    
    AutoRepeater(AutoRepeaterSettings* settings);
    void start();
    void stop();
    size_t tick(float deltaT);
    
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
    
    explicit KeyframeInterpolator(const InterT &initialValue = InterT()) {
        keyframeTimes.push_back(0.0f);
        keyframeValues.push_back(initialValue);
        keyframeEases.push_back(EASE_METHOD_NONE);
    };
    
    
    /**
     * @brief Returns the interpolated value at a given time.
     *
     * @param t The time.
     * @return The value.
     */
    InterT get(float t) {
        if(t < 0.0f) return keyframeValues[0];
        
        if(t < keyframeTimes[0]) {
            return keyframeValues[0];
        }
        
        for(size_t k = 1; k < keyframeTimes.size(); ++k) {
            if(t <= keyframeTimes[k]) {
                float deltaT =
                    std::max(keyframeTimes[k] - keyframeTimes[k - 1], 0.01f);
                float relativeT =
                    t - keyframeTimes[k - 1];
                float ratio =
                    relativeT / deltaT;
                ratio = ease(keyframeEases[k], ratio);
                return
                    interpolate(
                        keyframeValues[k - 1], keyframeValues[k], ratio
                    );
            }
        }
        
        return keyframeValues.back();
    }
    
    
    /**
     * @brief Adds a keyframe.
     *
     * @param t Time (0 to 1).
     * @param value Value at that point.
     * @param ease Easing method between it and the previous keyframe.
     * @param outIdx If not nullptr, the index of the newly added keyframe
     * is returned here.
     */
    void add(
        float t, const InterT &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* outIdx = nullptr
    ) {
        size_t newIdx = getInsertionIdx(t);
        
        if(outIdx) *outIdx = newIdx;
        
        keyframeTimes.insert(keyframeTimes.begin() + newIdx, t);
        keyframeValues.insert(keyframeValues.begin() + newIdx, value);
        keyframeEases.insert(keyframeEases.begin() + newIdx, ease);
    }
    
    
    /**
     * @brief Adds a keyframe, or sets the data of the keyframe at the
     * specified time.
     *
     * @param t Time (0 to 1).
     * @param value Value at that point.
     * @param ease Easing method between it and the previous keyframe.
     * @param outIdx If not nullptr, the index of the newly added keyframe
     * is returned here.
     */
    void addOrSet(
        float t, const InterT &value,
        EASING_METHOD ease = EASE_METHOD_NONE, size_t* outIdx = nullptr
    ) {
        for(size_t k = 0; k < keyframeTimes.size(); ++k) {
            if(keyframeTimes[k] == t) {
                if(outIdx) *outIdx = k;
                setKeyframeValue(k, value);
                return;
            }
        }
        
        add(t, value, ease, outIdx);
    }
    
    
    /**
     * @brief Removes a keyframe.
     *
     * @param idx Its index.
     */
    void remove(size_t idx) {
        keyframeTimes.erase(keyframeTimes.begin() + idx);
        keyframeValues.erase(keyframeValues.begin() + idx);
        keyframeEases.erase(keyframeEases.begin() + idx);
    }
    
    
    /**
     * @brief Returns how many keyframes there are.
     *
     * @return The total.
     */
    size_t getKeyframeCount() {
        return keyframeTimes.size();
    }
    
    
    /**
     * @brief Gets data about the keyframe at the specified index.
     *
     * @param idx The keyframe's index.
     * @return A pair with the keyframe's time and value.
     */
    std::pair<float, InterT> getKeyframe(size_t idx) {
        return std::make_pair(keyframeTimes[idx], keyframeValues[idx]);
    }
    
    
    /**
     * @brief Sets the value of the keyframe at the specified index.
     *
     * @param idx They keyframe's index.
     * @param value The new value.
     */
    void setKeyframeValue(size_t idx, const InterT &value) {
        keyframeValues[idx] = value;
    }
    
    
    /**
     * @brief Sets the time of the keyframe at the specified index.
     *
     * @param idx The keyframe's index.
     * @param time The new time.
     * @param outNewIdx If not nullptr, the new index of the keyframe is
     * returned here.
     */
    void setKeyframeTime(
        size_t idx, float time, size_t* outNewIdx = nullptr
    ) {
        size_t curIdx = idx;
        
        while(
            curIdx > 0 &&
            time < keyframeTimes[curIdx - 1]
        ) {
            std::swap(keyframeTimes[curIdx], keyframeTimes[curIdx - 1]);
            std::swap(keyframeValues[curIdx], keyframeValues[curIdx - 1]);
            std::swap(keyframeEases[curIdx], keyframeEases[curIdx - 1]);
            curIdx--;
        }
        while(
            curIdx < (getKeyframeCount() - 1) &&
            time > keyframeTimes[curIdx + 1]
        ) {
            std::swap(keyframeTimes[curIdx], keyframeTimes[curIdx + 1]);
            std::swap(keyframeValues[curIdx], keyframeValues[curIdx + 1]);
            std::swap(keyframeEases[curIdx], keyframeEases[curIdx + 1]);
            curIdx++;
        }
        
        if(outNewIdx) *outNewIdx = curIdx;
        
        keyframeTimes[curIdx] = time;
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
        
        keyframeTimes.clear();
        keyframeValues.clear();
        keyframeEases.clear();
        
        for(size_t c = 0; c < node->getNrOfChildren(); c++) {
            DataNode* cNode = node->getChild(c);
            InterT value = fromString<InterT>(cNode->value);
            add(s2f(cNode->name), value, EASE_METHOD_NONE);
        }
    }
    
    
private:

    //--- Members ---
    
    //Keyframe times.
    vector<float> keyframeTimes;
    
    //Keyframe values.
    vector<InterT> keyframeValues;
    
    //Keyframe easing methods.
    vector<EASING_METHOD> keyframeEases;
    
    
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
        for(; idx < keyframeTimes.size(); idx++) {
            if(keyframeTimes[idx] >= t) break;
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
    float timeLeft = 0.0f;
    
    //When the timer starts, its time is set to this.
    float duration = 0.0f;
    
    //Code to run when the timer ends, if any.
    std::function<void()> onEnd = nullptr;
    
    
    //--- Function declarations ---
    
    explicit Timer(
        float duration = 0,
        const std::function<void()> &onEnd = nullptr
    );
    ~Timer();
    void start(bool canRestart = true);
    void start(float newDuration);
    void stop();
    void tick(float deltaT);
    float getRatioLeft() const;
    
};


string getCurrentTime(bool fileNameFriendly);
string sanitizeFileName(const string &s);


/**
 * @brief Deterministically randomly shuffles the contents of a vector.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector to shuffle.
 * @param pickRandomFloats Vector of previously-determined random floats to
 * calculate the picks with [0, 1]. This vector must be of the same size
 * as the input vector.
 * @return The shuffled vector.
 */
template<typename ContentT>
vector<ContentT> shuffleVector(
    const vector<ContentT> &v, const vector<float> pickRandomFloats
) {
    vector<ContentT> result;
    vector<ContentT> itemsAvailable = v;
    for(size_t i = 0; i < v.size(); i++) {
        size_t pick = pickRandomFloats[i] * (itemsAvailable.size());
        //Add a safeguard for if the float is exactly 1.0.
        pick = std::min(pick, itemsAvailable.size() - 1);
        result.push_back(itemsAvailable[pick]);
        itemsAvailable.erase(itemsAvailable.begin() + pick);
    }
    return result;
}


string standardizePath(const string &path);
string vectorTailToString(const vector<string> &v, size_t pos);


/**
 * @brief Shorthand for figuring out if a given item is in a container.
 *
 * @tparam ContainerT Type of container.
 * @tparam ContentT Type of contents of the container.
 * @param cont The container.
 * @param item Item to check.
 * @return Whether it contains the item.
 */
template<typename ContainerT, typename ContentT>
bool isInContainer(const ContainerT &cont, const ContentT &item) {
    return std::find(cont.begin(), cont.end(), item) != cont.end();
}


/**
 * @brief Removes elements from a vector if they show up in the ban list.
 *
 * @tparam ContentT Type of contents of the vector and ban list.
 * @param v Vector to filter.
 * @param banList List of items that must be banned.
 * @return The filtered vector.
 */
template<typename ContentT>
vector<ContentT> filterVectorWithBanList(
    const vector<ContentT> &v, const vector<ContentT> &banList
) {
    vector<ContentT> result = v;
    for(size_t i = 0; i < result.size();) {
        if(isInContainer(banList, result[i])) {
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
 * @tparam ContentT Type of the vector's contents.
 * @param item Item to compare with.
 * @param vec Vector to check.
 * @return The finished vector.
 */
template<typename ContentT>
vector<ContentT> removeAllInVector(
    const ContentT &item, const vector<ContentT> &vec
) {
    vector<ContentT> result = vec;
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
 * @tparam ContentT Type of contents of the vector.
 * @param v Vector to sort.
 * @param preferenceList Preference list.
 * @param equal If not nullptr, use this function to compare whether
 * an item of t1 matches an item of t2.
 * @param less If not nullptr, use this function to sort missing items with.
 * @param unknowns If not nullptr, unknown preferences (i.e. items in
 * the preference list but not inside the vector) will be added here.
 * @return The sorted vector.
 */
template<typename ContentT>
vector<ContentT> sortVectorWithPreferenceList(
    const vector<ContentT> &v, const vector<ContentT> preferenceList,
    vector<ContentT>* unknowns = nullptr
) {
    vector<ContentT> result;
    vector<ContentT> missingItems;
    result.reserve(v.size());
    
    //Sort the existing items.
    for(size_t p = 0; p < preferenceList.size(); p++) {
        bool foundInVector = false;
        for(auto &i : v) {
            if(i == preferenceList[p]) {
                foundInVector = true;
                result.push_back(i);
                break;
            }
        }
        if(!foundInVector && unknowns) {
            unknowns->push_back(preferenceList[p]);
        }
    }
    
    //Find the missing items.
    for(auto &i : v) {
        bool foundInPreferences = false;
        for(auto &p : preferenceList) {
            if(i == p) {
                foundInPreferences = true;
                break;
            }
        }
        if(!foundInPreferences) {
            //Missing from the list? Add it to the "missing" pile.
            missingItems.push_back(i);
        }
    }
    
    //Sort and place the missing items.
    if(!missingItems.empty()) {
        std::sort(
            missingItems.begin(),
            missingItems.end()
        );
        result.insert(
            result.end(),
            missingItems.begin(),
            missingItems.end()
        );
    }
    
    return result;
}


/**
 * @brief Returns whether or not the two vectors contain the same items,
 * regardless of order.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Whether they contain the same items.
 */
template<typename ContentT>
bool vectorsContainSame(const vector<ContentT> &v1, const vector<ContentT> &v2) {
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
