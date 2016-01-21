/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interval class and interval-related functions.
 */

#include "functions.h"
#include "interval.h"

subinterval::subinterval(float l, float u, const float d) :
    divisor(d) {
    
    if(l > u) swap(l, u);
    lower = l;
    upper = u;
}


interval::interval(const string &s) {
    vector<string> subinterval_strs = split(s, ";");
    size_t n_subintervals = subinterval_strs.size();
    
    for(size_t si = 0; si < n_subintervals; ++si) {
        float lower = FLT_MIN;
        float upper = FLT_MAX;
        float divisor = 0;
        
        if(!subinterval_strs[si].empty()) {
            vector<string> divisor_parts = split(subinterval_strs[si], "every", false, true);
            if(divisor_parts.size() >= 2) {
                divisor = s2f(divisor_parts.back());
            }
            
            if(divisor_parts[0] != "every") {
                vector<string> range_parts = split(divisor_parts[0], "to", false, true);
                if(range_parts.size() == 1 && range_parts[0] != "to") { //No "to".
                    lower = upper = s2f(range_parts[0]);
                } else {
                    if(range_parts.size() >= 2) {
                        if(trim_spaces(range_parts[0]) != "any") {
                            lower = s2f(range_parts[0]);
                        }
                    }
                    if(range_parts.size() >= 3) {
                        if(trim_spaces(range_parts[2]) != "any") {
                            upper = s2f(range_parts[2]);
                        }
                    }
                }
            }
        }
        
        subintervals.push_back(subinterval(lower, upper, divisor));
    }
}


float interval::get_random_number() {
    size_t n_subintervals = subintervals.size();
    if(n_subintervals == 0) return 0;
    return subintervals[0].lower; //TODO
}


bool interval::is_number_in_interval(const float n) {
    size_t n_subintervals = subintervals.size();
    if(n_subintervals == 0) return false;
    
    for(size_t s = 0; s < n_subintervals; ++s) {
        subinterval* s_ptr = &subintervals[s];
        if(n >= s_ptr->lower && n <= s_ptr->upper) {
        
            if(s_ptr->divisor == 0) return true;
            
            float modulus_begin = ((s_ptr->lower == FLT_MIN) ? 0 : s_ptr->lower);
            if(fmod(n - modulus_begin, s_ptr->divisor) == 0) {
                return true;
            }
        }
    }
    
    return false;
}
