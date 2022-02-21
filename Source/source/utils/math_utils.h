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

#ifndef MATH_UTILS_INCLUDED
#define MATH_UTILS_INCLUDED

#define _USE_MATH_DEFINES

#include <cstddef>
#include <math.h>
#include <string>
#include <stdint.h>

constexpr float TAU = (float)M_PI * 2.0f;

//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + i2s((n)))

//Rounds a number. Ugh, why do I even have to create this.
#define round(n) (((n) > 0) ? floor((n) + 0.5) : ceil((n) - 0.5))

//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)

float clamp(const float number, const float minimum, const float maximum);
uint32_t hash_nr(const unsigned int input);
uint32_t hash_nr2(const unsigned int input1, const unsigned int input2);
float interpolate_number(
    const float input, const float input_start, const float input_end,
    const float output_start, const float output_end
);
float randomf(float min, float max);
int randomi(int min, int max);
int sum_and_wrap(const int nr, const int sum, const int wrap_limit);
float wrap_float(const float nr, const float minimum, const float maximum);

#endif //ifndef MATH_UTILS_INCLUDED
