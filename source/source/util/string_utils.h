/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for string-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>


using std::size_t;
using std::string;
using std::vector;


//Flags for the time-to-string functions. This is a bitmask.
enum TIME_TO_STR_FLAG {

    //If true, leading zeros will not appear.
    TIME_TO_STR_FLAG_NO_LEADING_ZEROS = 1 << 0,
    
    //If true, leading portions to the left that are just zeros will not appear.
    TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS = 1 << 1,
    
};


//Converts an integer (or long) to a string.
#define i2s(n) std::to_string((long long) (n))

string amount_str(
    int amount, const string &singular_text,
    const string &plural_text = ""
);
string box_string(
    const string &s, size_t size, const string &finisher = ""
);
string b2s(bool b);
void duplicate_string(const string &orig_str, string &new_str);
string f2s(float f);
string get_matching_string_starts(const string &s1, const string &s2);
string get_path_last_component(const string& s);
bool is_number(const string &s);
string pad_string(const string &s, size_t size, char padding);
string remove_extension(const string &s);
string replace_all(
    string s, const string &search, const string &replacement
);
bool s2b(const string &s);
double s2f(const string &s);
int s2i(const string &s);
vector<string> semicolon_list_to_vector(
    const string &s, const string &sep = ";"
);
vector<string> split(
    string text, const string &del = " ", bool inc_empty = false,
    bool inc_del = false
);
bool str_ends_with(const string &s, const string &end);
bool str_peek(const string &s, size_t where, const string &match);
bool str_starts_with(const string &s, const string &start);
string str_to_lower(string s);
string str_to_sentence(string s);
string str_to_title(string s);
string str_to_upper(string s);
string time_to_str2(
    size_t units,
    const string &suffix1, const string &suffix2,
    uint8_t flags = 0
);
string time_to_str3(
    size_t units,
    const string &suffix1, const string &suffix2, const string &suffix3,
    uint8_t flags = 0
);
string trim_spaces(const string &s, bool left_only = false);
string trim_with_ellipsis(const string &s, size_t size);
string word_wrap(const string &s, size_t n_chars_per_line);
