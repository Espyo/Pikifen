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

#ifndef STRING_UTILS_INCLUDED
#define STRING_UTILS_INCLUDED

#include <string>
#include <vector>

using namespace std;

//Converts an integer (or long) to a string.
#define i2s(n) to_string((long long) (n))

string box_string(
    const string &s, const size_t size, const string &finisher = ""
);
string b2s(const bool b);
string f2s(const float f);
bool is_number(const string &s);
string replace_all(string s, string search, string replacement);
bool s2b(const string &s);
double s2f(const string &s);
int s2i(const string &s);
vector<string> split(
    string text, const string &del = " ", const bool inc_empty = false,
    const bool inc_del = false
);
string str_to_lower(string s);
string str_to_upper(string s);
string trim_spaces(const string &s, const bool left_only = false);

#endif //ifndef STRING_UTILS_INCLUDED
