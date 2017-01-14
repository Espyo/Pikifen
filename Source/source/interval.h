/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the interval struct and interval-related functions.
 */

#ifndef INTERVAL_INCLUDED
#define INTERVAL_INCLUDED

#include <cfloat>
#include <vector>

using namespace std;

/*
 * An interval specifies a range from which
 * random numbers may be picked.
 */

struct subinterval {
    //Lower boundary of the interval, inclusive. FLT_MIN for none.
    float lower;
    //Upper boundary of the interval, inclusive. FLT_MAX for none.
    float upper;
    //Every X numbers, starting on the lower boundary
    //(or on 0 if there is no lower boundary). 0 for no divisor.
    float divisor;
    subinterval(float l = FLT_MIN, float u = FLT_MAX, const float d = 0);
};



struct interval {
private:
    vector<subinterval> subintervals;
    
public:
    interval(const string &s = "");
    float get_random_number();
    bool is_number_in_interval(const float n);
};

#endif //ifndef INTERVAL_INCLUDED
