/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Math-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <algorithm>
#include <cmath>

#include "math_utils.h"


/**
 * @brief Eases a number [0, 1] in accordance to a non-linear
 * interpolation method.
 *
 * @param method The method to use.
 * @param n The number to ease, in the range [0, 1].
 * @return The eased number.
 */
float ease(const EASING_METHOD method, float n) {
    switch(method) {
    case EASE_METHOD_NONE: {
        return n;
    } case EASE_METHOD_IN: {
        return (float) pow(n, 3);
    }
    case EASE_METHOD_OUT: {
        return (float) (1 - (pow((1 - n), 3)));
    }
    case EASE_METHOD_IN_BACK: {
        const float mag1 = 1.70158f;
        const float mag2 = mag1 + 1.0f;
        return (float) (mag2 * n * n * n - mag1 * n * n);
    }
    case EASE_METHOD_OUT_BACK: {
        const float mag1 = 1.70158f;
        const float mag2 = mag1 + 1.0f;
        return
            (float)
            (1.0f + mag2 * pow(n - 1.0f, 3) + mag1 * pow(n - 1.0f, 2));
    }
    case EASE_METHOD_IN_OUT_BACK: {
        const float mag1 = 1.70158f;
        const float mag2 = mag1 * 1.525f;
        return
            n < 0.5 ?
            (float)
            (pow(2 * n, 2) * ((mag2 + 1.0f) * 2 * n - mag2)) / 2 :
            (float)
            (pow(2 * n - 2, 2) * ((mag2 + 1.0f) * (n * 2 - 2) + mag2) + 2) / 2;
    } case EASE_METHOD_IN_ELASTIC: {
        const float mag = TAU / 3;
        return
            n == 0.0f ?
            0.0f :
            n == 1.0f ?
            1.0f :
            (float) - pow(2.0f, 10.0f * n - 10.0f) *
            (float) sin((n * 10.0f - 10.75f) * mag);
    }
    case EASE_METHOD_OUT_ELASTIC: {
        const float mag = TAU / 3;
        return
            n == 0.0f ?
            0.0f :
            n == 1.0f ?
            1.0f :
            (float) pow(2.0f, -10.0f * n) *
            (float) sin((n * 10.0f - 0.75f) * mag) + 1.0f;
    }
    case EASE_METHOD_UP_AND_DOWN: {
        return (float) sin(n * TAU / 2);
    }
    case EASE_METHOD_UP_AND_DOWN_ELASTIC: {
        const float cp1 = 0.50f;
        const float cp2 = 0.80f;
        const float mag1 = -0.4f;
        const float mag2 = 0.15f;
        float aux;
        if(n < cp1) {
            aux = n * 1.0f / cp1;
            return (float) sin(aux * TAU / 2);
        } else if(n < cp2) {
            aux = n - cp1;
            aux *= 1.0f / (cp2 - cp1);
            return (float) sin(aux * TAU / 2) * mag1;
        } else {
            aux = n - cp2;
            aux *= 1.0f / (1.0f - cp2);
            return (float) sin(aux * TAU / 2) * mag2;
        }
    }
    
    }
    
    return n;
}


/**
 * @brief Performs a deterministic weighted random pick,
 * and returns the index of the chosen item.
 *
 * @param weights A vector with the weight of each item.
 * @param pointRandomFloat A previously-determined random float to
 * calculate the weight sum point with [0, 1].
 * @return Index of the chosen item, or 0 on error.
 */
size_t getRandomIdxWithWeights(
    const vector<float>& weights, float pointRandomFloat
) {
    float weightSum = 0.0f;
    for(size_t i = 0; i < weights.size(); i++) {
        weightSum += weights[i];
    }
    float r = pointRandomFloat * weightSum;
    for(size_t i = 0; i < weights.size(); i++) {
        if(r < weights[i]) return i;
        r -= weights[i];
    }
    return 0;
}


/**
 * @brief Given an input, it returns a 32-bit unsigned integer hash of
 * that input.
 *
 * @param input The input number.
 * @return The hash.
 */
uint32_t hashNr(unsigned int input) {
    //Robert Jenkins' 32 bit integer hash function.
    //From https://gist.github.com/badboy/6267743
    //This algorithm is the simplest, lightest, fairest one I could find.
    uint32_t n = (input + 0x7ED55D16) + (input << 12);
    n = (n ^ 0xC761C23C) ^ (n >> 19);
    n = (n + 0x165667B1) + (n << 5);
    n = (n + 0xD3A2646C) ^ (n << 9);
    n = (n + 0xFD7046C5) + (n << 3);
    n = (n ^ 0xB55A4F09) ^ (n >> 16);
    return n;
}


/**
 * @brief Given two inputs, it returns a 32-bit unsigned integer hash of
 * those inputs.
 *
 * @param input1 First input number.
 * @param input2 Second input number.
 * @return The hash.
 */
uint32_t hashNr2(unsigned int input1, unsigned int input2) {
    uint32_t n1 = hashNr(input1);
    
    //Same algorithm as in hashNr() with one argument,
    //but I changed the magic numbers to other random stuff.
    uint32_t n2 = (input2 + 0x5D795E0E) + (input2 << 12);
    n2 = (n2 ^ 0xC07C34BD) ^ (n2 >> 19);
    n2 = (n2 + 0x4969B10A) + (n2 << 5);
    n2 = (n2 + 0x583EB559) ^ (n2 << 9);
    n2 = (n2 + 0x72F56900) + (n2 << 3);
    n2 = (n2 ^ 0x8B121972) ^ (n2 >> 16);
    
    return n1 * n2;
}


/**
 * @brief Given a starting number, it increases or decreases it towards the
 * target value, but the change will not be higher than the max step.
 *
 * @param start Starting value.
 * @param target Target value.
 * @param maxStep Maximum change in value allowed.
 * @return The inched number.
 */
float inchTowards(float start, float target, float maxStep) {
    if(fabs(target - start) <= maxStep) return target;
    if(start < target) return start + maxStep;
    return start - maxStep;
}


/**
 * @brief Returns the interpolation between two numbers, given a number in
 * an interval.
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
float interpolateNumber(
    float input, float inputStart, float inputEnd,
    float outputStart, float outputEnd
) {
    return
        outputStart +
        ((input - inputStart) / (float) (inputEnd - inputStart)) *
        (outputEnd - outputStart);
}


/**
 * @brief Generates a random number using a linear congruential generator
 * and advances that random number generator's state.
 *
 * @param state Pointer to the state to calculate from and advance.
 * @return The generated number.
 */
int32_t linearCongruentialGenerator(int32_t* state) {
    int32_t result = ((*state * 1103515245U) + 12345U) & 0x7fffffff;
    *state = result;
    return result;
}


/**
 * @brief Sums a number to another (even if negative), and then
 * wraps that number across a limit, applying a modulus operation.
 *
 * @param nr Base number.
 * @param sum Number to add (or subtract).
 * @param wrapLimit Wrap between [0 - wrapLimit[.
 * @return The wrapped number.
 */
int sumAndWrap(int nr, int sum, int wrapLimit) {
    int finalNr = nr + sum;
    while(finalNr < 0) {
        finalNr += wrapLimit;
    }
    return finalNr % wrapLimit;
}


/**
 * @brief Wraps a floating point number between the specified interval.
 *
 * @param nr Base number.
 * @param minimum Minimum of the interval.
 * @param maximum Maximum of the interval.
 * @return The wrapped number.
 */
float wrapFloat(float nr, float minimum, float maximum) {
    const float diff = maximum - minimum;
    return
        minimum + (float) fmod(diff + (float) fmod(nr - minimum, diff), diff);
}
