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

#include <string>
#include <vector>

using std::string;
using std::vector;


//Turns a bit in a bitmask off.
#define disable_flag(flags, flag) (flags &= ~flag)

//Turns a bit in a bitmask on.
#define enable_flag(flags, flag) (flags |= flag)

//Returns the bitmask corresponding to a certain index. Useful for flags.
#define get_idx_bitmask(i) (1 << i)

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Returns whether a bit is on or not in a bitmask.
#define has_flag(flags, flag) ((flags & flag) > 0)


string get_current_time(const bool filename_friendly);
string sanitize_file_name(const string &s);
string standardize_path(const string &path);
string vector_tail_to_string(const vector<string> &v, const size_t pos);


/**
 * @brief Returns whether or not the two vectors contain the same items,
 * regardless of order.
 *
 * @tparam t The contents of the vector.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return Whether they contain the same items.
 */

template<typename t>
bool vectors_contain_same(const vector<t> &v1, const vector<t> &v2) {
    if(v1.size() != v2.size()) {
        return false;
    }
    
    for(size_t i1 = 0; i1 < v1.size(); ++i1) {
        bool found = false;
        for(size_t i2 = 0; i2 < v2.size(); ++i2) {
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


#if defined(_WIN32)
string strsignal(const int signum);
#endif //#if defined(_WIN32)
