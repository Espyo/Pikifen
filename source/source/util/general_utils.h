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

//Returns whether a bit is on or not in a bitmask.
#define hasFlag(flags, flag) (((flags) & (flag)) > 0)


//Cross-platform way of representing an invalid index.
constexpr size_t INVALID = UINT32_MAX;

//Cross-platform way of representing a float value of "invalid" or similar.
constexpr float LARGE_FLOAT = 999999.0f;


template<typename t>
t fromString(const string& s);
template<>
float fromString<float>(const string& s);
template<>
ALLEGRO_COLOR fromString<ALLEGRO_COLOR>(const string& s);
template<>
Point fromString<Point>(const string& s);


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
    
    void registerItem(size_t enum_idx, const string& name);
    size_t getIdx(const string& name) const;
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
    
    explicit KeyframeInterpolator(const InterT& initialValue = InterT()) {
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
        float t, const InterT& value,
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
        float t, const InterT& value,
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
    void setKeyframeValue(size_t idx, const InterT& value) {
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
     * @brief Interpolates between two chars, and returns the value at the
     * specified time.
     *
     * @param v1 First value.
     * @param v2 Second value.
     * @param time Time (0 to 1).
     * @return The value.
     */
    unsigned char interpolate(unsigned char v1, unsigned char v2, float time) {
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
        const ALLEGRO_COLOR& c1, const ALLEGRO_COLOR& c2, float time
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
        const Point& p1, const Point& p2, float time
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
 * @brief Represents something shaking, like the camera during an earthquake.
 *
 * Special thanks to https://www.youtube.com/watch?v=tu-Qe66AvtY
 */
struct Shaker {

    //--- Members ---
    
    //Trauma amount to decrease per second.
    float decreaseAmount = 1.0f;
    
    //Scale time by this much, for the offset calculations.
    float timeScale = 30.0f;
    
    //Random noise function seed to use.
    unsigned int seed = 0;
    
    
    //--- Function declarations ---
    Shaker(std::function<float(float, float)> getRandomFloat);
    void shake(float strength);
    void getOffsets(
        float* xOffset = nullptr, float* yOffset = nullptr,
        float* angleOffset = nullptr
    ) const;
    void tick(float deltaT);
    
    
    private:
    
    //--- Members ---
    
    //Current trauma amount (raw shake factor).
    float trauma = 0.0f;
    
    //Time spent so far.
    float time = 0.0f;
    
    //Callback for when a random noise function value is needed.
    //The first parameter is the seed number, the second is the time spent.
    std::function<float(float, float)> getRandomFloat;
    
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
        const std::function<void()>& onEnd = nullptr
    );
    ~Timer();
    void start(bool canRestart = true);
    void start(float newDuration);
    void stop();
    void tick(float deltaT);
    float getRatioLeft() const;
    
};


string getCurrentTime(bool fileNameFriendly);
string sanitizeFileName(const string& s);
string standardizePath(const string& path);
string vectorTailToString(const vector<string>& v, size_t pos);
