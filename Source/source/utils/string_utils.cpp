/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * String-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <algorithm>
#include <assert.h>
#include <iomanip>
#include <sstream>

#include "string_utils.h"


/* ----------------------------------------------------------------------------
 * Returns a string representing an amount, and the unit, though the unit
 * is in either plural form or singular form, depending on the amount.
 * amount:
 *   Amount to compare against.
 * singular_text:
 *   Text to write if the amount is singular.
 * plural_text:
 *   Text to write if the amount is plural. If empty, it'll use the singular
 *   text plus an 's'.
 */
string amount_str(
    const int amount, const string &singular_text, const string &plural_text
) {
    string result = i2s(amount) + " ";
    if(amount == 1) {
        result += singular_text;
    } else if(plural_text.empty()) {
        result += singular_text + "s";
    } else {
        result += plural_text;
    }
    return result;
}


/* ----------------------------------------------------------------------------
 * Converts a boolean to a string, returning either "true" or "false".
 * b:
 *   Boolean to convert.
 */
string b2s(const bool b) {
    return b ? "true" : "false";
}


/* ----------------------------------------------------------------------------
 * Boxes a string so that it becomes a specific size.
 * Truncates if it's too big, pads with spaces if it's too small.
 * s:
 *   String to box.
 * size:
 *   Maximum size of the return string.
 * finisher:
 *   This comes after s and before the padding (if any). This must
 *   always be present, even if that means that s needs to get truncated.
 */
string box_string(const string &s, const size_t size, const string &finisher) {
    assert(size > finisher.size());
    size_t core_size = std::min(s.size(), size - finisher.size());
    return
        s.substr(0, core_size) +
        finisher +
        string(size - core_size - finisher.size(), ' ');
}


/* ----------------------------------------------------------------------------
 * Duplicates a string.
 * This is necessary because under C++11, with _GLIBCXX_USE_CXX11_ABI=0,
 * assigning a string to another string (e.g. "str_a = str_b") will cause it
 * to use the same C-string pointer. This could be undesirable in some cases.
 * This function creates a copy of a string while ensuring the underlying
 * C-string pointer is different.
 * orig_str:
 *   Original string.
 * new_str:
 *   Reference to the new string.
 */
void duplicate_string(const string &orig_str, string &new_str) {
    new_str = string(orig_str.c_str());
}


/* ----------------------------------------------------------------------------
 * Converts a float to a string, with 4 decimal places.
 * f:
 *   Float to convert.
 */
string f2s(const float f) {
    std::stringstream s;
    s << std::fixed << std::setprecision(4) << f;
    return s.str();
}


/* ----------------------------------------------------------------------------
 * Returns a substring representing the start of one string, up until it
 * no longer matches with the other string.
 * This check is case-sensitive.
 * Returns an empty string if there's no match.
 * s1:
 *   First string.
 * s2:
 *   Second string.
 */
string get_matching_string_starts(const string &s1, const string &s2) {
    size_t chars_to_check = std::min(s1.size(), s2.size());
    size_t nr_matching_chars = 0;
    
    for(size_t c = 0; c < chars_to_check; ++c) {
        if(s1[c] == s2[c]) {
            nr_matching_chars++;
        } else {
            break;
        }
    }
    
    if(nr_matching_chars == 0) return string();
    
    return s1.substr(0, nr_matching_chars);
}


/* ----------------------------------------------------------------------------
 * Checks if the contents of a string are a number or not.
 * s:
 *   String to check.
 */
bool is_number(const string &s) {
    for(size_t c = 0; c < s.size(); ++c) {
        unsigned char ch = s[c];
        if((ch < '0' || ch > '9') && ch != '-' && ch != ',' && ch != '.') {
            return false;
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Pads a given string such that it is at least the given size.
 * It uses the provided character to pad out the remaining space.
 * This only pads the left side of the string.
 * s:
 *   String to pad.
 * size:
 *   Final minimum string size.
 * padding:
 *   What character to pad with.
 */
string pad_string(const string &s, const size_t size, const char padding) {
    string result = s;
    if(size > s.size()) {
        result.insert(0, size - s.size(), padding);
    }
    return result;
}


/* ----------------------------------------------------------------------------
 * Given a file name as a string, it removes the extension.
 * Returns the string as it is if there is no extension.
 * s:
 *   String to remove the extension from.
 */
string remove_extension(const string &s) {
    size_t pos = s.find_last_of('.');
    if(pos == string::npos) {
        return s;
    }
    return s.substr(0, pos);
}


/* ----------------------------------------------------------------------------
 * Replaces all instances of x with y.
 * s:
 *   String with the text to change.
 * search:
 *   Search term that will be replaced.
 * replacement:
 *   What to replace found search terms with.
 */
string replace_all(string s, const string &search, const string &replacement) {
    size_t pos = s.find(search);
    while(pos != string::npos) {
        s.replace(pos, search.size(), replacement);
        pos = s.find(search, pos + replacement.size());
    };
    
    return s;
}


/* ----------------------------------------------------------------------------
 * Converts a string to a boolean, judging by
 * the English language words that represent true and false.
 * s:
 *   String to convert.
 */
bool s2b(const string &s) {
    string s2 = s;
    s2 = str_to_lower(s2);
    s2 = trim_spaces(s2);
    if(s2 == "yes" || s2 == "true" || s2 == "y" || s2 == "t") return true;
    else return (s2i(s2) != 0);
}


/* ----------------------------------------------------------------------------
 * Converts a string to a float,
 * trimming the spaces and accepting commas or points.
 * s:
 *   String to convert.
 */
double s2f(const string &s) {
    string s2 = trim_spaces(s);
    replace(s2.begin(), s2.end(), ',', '.');
    return atof(s2.c_str());
}


/* ----------------------------------------------------------------------------
 * Converts a string to an integer.
 * s:
 *   String to convert.
 */
int s2i(const string &s) {
    return s2f(s);
}


/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:
 *   The string to split.
 * del:
 *   The delimiter. Default is space.
 * inc_empty:
 *   If true, include empty substrings on the vector.
 *   i.e. if two delimiters come together in a row,
 *   keep an empty substring between.
 * inc_del:
 *   If true, include the delimiters on the vector as a substring.
 */
vector<string> split(
    string text, const string &del, const bool inc_empty, const bool inc_del
) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if(pos != string::npos) {  //If it DID find the delimiter.
            //Get the text between the start and the delimiter.
            string sub = text.substr(0, pos);
            
            //Add the text before the delimiter to the vector.
            if(sub != "" || inc_empty)
                v.push_back(sub);
                
            //Add the delimiter to the vector, but only if requested.
            if(inc_del)
                v.push_back(del);
                
            //Delete everything before the delimiter,
            //including the delimiter itself, and search again.
            text.erase(text.begin(), text.begin() + pos + del_size);
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter.
    //(If there is one. If not, it's just the whole string.)
    
    //If it's a blank string,
    //only add it if we want empty strings.
    if(text != "" || inc_empty) {
        v.push_back(text);
    }
    
    return v;
}


/* ----------------------------------------------------------------------------
 * Peeks the next characters in a string, and returns whether they match
 * the specified filter.
 * s:
 *   String to parse.
 * where:
 *   What character index to start peeking at.
 * match:
 *   What string to match with.
 */
bool str_peek(const string &s, const size_t where, const string &match) {
    if(where + match.size() > s.size()) return false;
    return s.substr(where, match.size()) == match;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into lowercase.
 * s:
 *   String to convert.
 */
string str_to_lower(string s) {
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; ++c) {
        s[c] = tolower(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into titlecase.
 * s:
 *   String to convert.
 */
string str_to_title(string s) {
    size_t letter_streak = 0;
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; ++c) {
        if(isalpha(s[c])) {
            if(letter_streak == 0) {
                s[c] = toupper(s[c]);
            } else {
                s[c] = tolower(s[c]);
            }
            letter_streak++;
        } else {
            letter_streak = 0;
        }
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into uppercase.
 * s:
 *   String to convert.
 */
string str_to_upper(string s) {
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; ++c) {
        s[c] = toupper(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 * s:
 *   The original string.
 * left_only:
 *   If true, only trim the spaces at the left.
 */
string trim_spaces(const string &s, const bool left_only) {
    string orig = s;
    //Spaces before.
    if(orig.size()) {
        while(orig[0] == ' ' || orig[0] == '\t') {
            orig.erase(0, 1);
            if(orig.empty()) break;
        }
    }
    
    if(!left_only) {
        //Spaces after.
        if(orig.size()) {
            while(
                orig[orig.size() - 1] == ' ' ||
                orig[orig.size() - 1] == '\t'
            ) {
                orig.erase(orig.size() - 1, 1);
                if(orig.empty()) break;
            }
        }
    }
    
    return orig;
}
