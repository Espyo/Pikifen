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

/* ----------------------------------------------------------------------------
 * Limits the given number to the given range, inclusive.
 */
float clamp(const float number, const float minimum, const float maximum) {
    return std::min(maximum, std::max(minimum, number));
}


/* ----------------------------------------------------------------------------
 * Given an input, it returns a 32-bit unsigned integer hash of that input.
 * input: The input number.
 */
uint32_t hash_nr(const unsigned int input) {
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


/* ----------------------------------------------------------------------------
 * Given two inputs, it returns a 32-bit unsigned integer hash of those inputs.
 * input1: First input number.
 * input2: Second input number.
 */
uint32_t hash_nr2(const unsigned int input1, const unsigned int input2) {
    uint32_t n1 = hash_nr(input1);
    
    //Same algorithm has in hash_nr() with one argument,
    //but I changed the magic numbers to other random stuff.
    uint32_t n2 = (input2 + 0x5D795E0E) + (input2 << 12);
    n2 = (n2 ^ 0xC07C34BD) ^ (n2 >> 19);
    n2 = (n2 + 0x4969B10A) + (n2 << 5);
    n2 = (n2 + 0x583EB559) ^ (n2 << 9);
    n2 = (n2 + 0x72F56900) + (n2 << 3);
    n2 = (n2 ^ 0x8B121972) ^ (n2 >> 16);
    
    return n1 * n2;
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation of the value between two positions.
 */
float interpolate_number(
    const float p, const float p1, const float p2,
    const float v1, const float v2
) {
    return v1 + ((p - p1) / (float) (p2 - p1)) * (v2 - v1);
}


/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 */
float randomf(float minimum, float maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    return (float) rand() / ((float) RAND_MAX / (maximum - minimum)) + minimum;
}


/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int minimum, int maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    return ((rand()) % (maximum - minimum + 1)) + minimum;
}


/* ----------------------------------------------------------------------------
 * Sums a number to another (even if negative), and then
 * wraps that number across a limit, applying a modulus operation.
 * nr:         Base number.
 * sum:        Number to add (or subtract).
 * wrap_limit: Wrap between [0 - wrap_limit[.
 */
int sum_and_wrap(const int nr, const int sum, const int wrap_limit) {
    int final_nr = nr + sum;
    while(final_nr < 0) {
        final_nr += wrap_limit;
    }
    return final_nr % wrap_limit;
}
