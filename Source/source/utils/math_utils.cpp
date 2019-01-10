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

using namespace std;

/* ----------------------------------------------------------------------------
 * Limits the given number to the given range, inclusive.
 */
float clamp(const float number, const float minimum, const float maximum) {
    return min(maximum, max(minimum, number));
}


/* ----------------------------------------------------------------------------
 * Returns a random number, between 0 and 1, but it's deterministic
 * if you use the same seed. i.e., if you feed it X, it
 * will always return Y. Because of its simplicity and predictability,
 * it should only be used for tiny details with unimportant randomness.
 * seed: The seed number.
 */
float deterministic_random(const unsigned int seed) {
    //This was built pretty much ad-hoc.
    return
        (
            ((seed * 1234567890L + (seed << 4)) % (seed ^ 981524)) % 65536
        ) / 65535.0f;
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
float randomf(float min, float max) {
    if(min == max) return min;
    if(min > max) swap(min, max);
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}


/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int min, int max) {
    if(min == max) return min;
    if(min > max) swap(min, max);
    return ((rand()) % (max - min + 1)) + min;
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
