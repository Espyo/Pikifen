/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for math-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#define _USE_MATH_DEFINES

#include <cstddef>
#include <math.h>
#include <string>
#include <stdint.h>
#include <vector>


using std::vector;


constexpr float TAU = (float)M_PI * 2.0f;


//Methods for easing numbers.
enum EASING_METHOD {

    //No easing. AKA linear interpolation.
    EASE_METHOD_NONE,
    
    //Eased as it goes in, then gradually goes out normally.
    EASE_METHOD_IN,
    
    //Gradually goes in normally, then eased as it goes out.
    EASE_METHOD_OUT,
    
    //Eased as it goes in and also as it goes out.
    EASE_METHOD_IN_OUT,
    
    //Goes backwards before going in.
    EASE_METHOD_IN_BACK,
    
    //Overshoots at the end then finishes.
    EASE_METHOD_OUT_BACK,
    
    //Combines in back with out back.
    EASE_METHOD_IN_OUT_BACK,
    
    //Springs backwards before going in.
    EASE_METHOD_IN_ELASTIC,
    
    //Near the end, it overshoots and then springs to normal.
    EASE_METHOD_OUT_ELASTIC,
    
    //Goes up to 1, then back down to 0, in a sine-wave.
    EASE_METHOD_UP_AND_DOWN,
    
    //Goes up to 1, then down to 0, and wobbles around 0 for a bit.
    EASE_METHOD_UP_AND_DOWN_ELASTIC,
    
};


//Rounds a number. Ugh, why do I even have to create this.
#define round(n) (((n) > 0) ? floor((n) + 0.5) : ceil((n) - 0.5))

//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)

float ease(const EASING_METHOD method, float n);
uint32_t hashNr(uint32_t input);
uint32_t hashNr2(uint32_t input1, uint32_t input2);
float inchTowards(float start, float target, float maxStep);
float interpolateNumber(
    float input, float inputStart, float inputEnd,
    float outputStart, float outputEnd
);
int32_t linearCongruentialGenerator(int32_t* state);
size_t getRandomIdxWithWeights(
    const vector<float>& weights, float pointRandomFloat
);
float simpleNoise(uint32_t seed, float x);
int sumAndWrap(int nr, int sum, int wrapLimit);
float wrapFloat(float nr, float minimum, float maximum);
