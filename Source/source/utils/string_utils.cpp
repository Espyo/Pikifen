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

#undef _CMATH_

#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "string_utils.h"


/**
 * @brief Returns a string representing an amount, and the unit, though the unit
 * is in either plural form or singular form, depending on the amount.
 *
 * @param amount Amount to compare against.
 * @param singular_text Text to write if the amount is singular.
 * @param plural_text Text to write if the amount is plural.
 * If empty, it'll use the singular text plus an 's'.
 * @return The string.
 */
string amount_str(
    int amount, const string &singular_text, const string &plural_text
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


/**
 * @brief Converts a boolean to a string, returning either "true" or "false".
 *
 * @param b Boolean to convert.
 * @return The string.
 */
string b2s(bool b) {
    return b ? "true" : "false";
}


/**
 * @brief Boxes a string so that it becomes a specific size.
 * Truncates if it's too big, pads with spaces if it's too small.
 *
 * @param s String to box.
 * @param size Maximum size of the return string.
 * @param finisher This comes after s and before the padding (if any).
 * This must always be present, even if that means that s needs to
 * get truncated.
 * @return The boxed string.
 */
string box_string(const string &s, size_t size, const string &finisher) {
    assert(size > finisher.size());
    size_t core_size = std::min(s.size(), size - finisher.size());
    return
        s.substr(0, core_size) +
        finisher +
        string(size - core_size - finisher.size(), ' ');
}


/**
 * @brief Duplicates a string.
 *
 * This is necessary because under C++11, with _GLIBCXX_USE_CXX11_ABI=0,
 * assigning a string to another string (e.g. "str_a = str_b") will cause it
 * to use the same C-string pointer. This could be undesirable in some cases.
 * This function creates a copy of a string while ensuring the underlying
 * C-string pointer is different.
 *
 * @param orig_str Original string.
 * @param new_str Reference to the new string.
 */
void duplicate_string(const string &orig_str, string &new_str) {
    new_str = string(orig_str.c_str());
}


/**
 * @brief Converts a float to a string, with a precision of 4 significant
 * figures.
 *
 * @param f Float to convert.
 * @return The string.
 */
string f2s(float f) {
    std::stringstream s;
    s << std::fixed << std::setprecision(4) << f;
    return s.str();
}


/**
 * @brief Returns a substring representing the start of one string, up until it
 * no longer matches with the other string.
 * This check is case-sensitive.
 *
 * @param s1 First string.
 * @param s2 Second string.
 * @return The match, or an empty string if there's no match.
 */
string get_matching_string_starts(const string &s1, const string &s2) {
    size_t chars_to_check = std::min(s1.size(), s2.size());
    size_t nr_matching_chars = 0;
    
    for(size_t c = 0; c < chars_to_check; c++) {
        if(s1[c] == s2[c]) {
            nr_matching_chars++;
        } else {
            break;
        }
    }
    
    if(nr_matching_chars == 0) return string();
    
    return s1.substr(0, nr_matching_chars);
}


/**
 * @brief Checks if the contents of a string are a number or not.
 *
 * @param s String to check.
 * @return Whether it is a number.
 */
bool is_number(const string &s) {
    for(size_t c = 0; c < s.size(); c++) {
        unsigned char ch = s[c];
        if((ch < '0' || ch > '9') && ch != '-' && ch != ',' && ch != '.') {
            return false;
        }
    }
    return true;
}


/**
 * @brief Pads a given string such that it is at least the given size.
 * It uses the provided character to pad out the remaining space.
 * This only pads the left side of the string.
 *
 * @param s String to pad.
 * @param size Final minimum string size.
 * @param padding What character to pad with.
 * @return The padded string.
 */
string pad_string(const string &s, size_t size, char padding) {
    string result = s;
    if(size > s.size()) {
        result.insert(0, size - s.size(), padding);
    }
    return result;
}


/**
 * @brief Given a file name as a string, it removes the extension.
 *
 * @param s String to remove the extension from.
 * @return The file name without an extension, or the original string if there
 * was no extension.
 */
string remove_extension(const string &s) {
    size_t pos = s.find_last_of('.');
    if(pos == string::npos) {
        return s;
    }
    return s.substr(0, pos);
}


/**
 * @brief Replaces all instances of x with y.
 *
 * @param s String with the text to change.
 * @param search Search term that will be replaced.
 * @param replacement What to replace found search terms with.
 * @return The string with the instances replaced.
 */
string replace_all(string s, const string &search, const string &replacement) {
    size_t pos = s.find(search);
    while(pos != string::npos) {
        s.replace(pos, search.size(), replacement);
        pos = s.find(search, pos + replacement.size());
    };
    
    return s;
}


/**
 * @brief Converts a string to a boolean, judging by
 * the English language words that represent true and false.
 *
 * @param s String to convert.
 * @return The boolean.
 */
bool s2b(const string &s) {
    string s2 = s;
    s2 = str_to_lower(s2);
    s2 = trim_spaces(s2);
    if(s2 == "yes" || s2 == "true" || s2 == "y" || s2 == "t") return true;
    else return (s2i(s2) != 0);
}


/**
 * @brief Converts a string to a float,
 * trimming the spaces and accepting commas or points.
 *
 * @param s String to convert.
 * @return The float.
 */
double s2f(const string &s) {
    string s2 = trim_spaces(s);
    replace(s2.begin(), s2.end(), ',', '.');
    return atof(s2.c_str());
}


/**
 * @brief Converts a string to an integer.
 *
 * @param s String to convert.
 * @return The integer.
 */
int s2i(const string &s) {
    return (int) s2f(s);
}


/**
 * @brief Returns a vector with all items inside a semicolon-separated list.
 *
 * @param s The string containing the list.
 * @param sep Separator to use, in case you need something else.
 * Default is semicolon.
 * @return The vector.
 */
vector<string> semicolon_list_to_vector(const string &s, const string &sep) {
    vector<string> parts = split(s, sep);
    for(size_t p = 0; p < parts.size(); p++) {
        parts[p] = trim_spaces(parts[p]);
    }
    return parts;
}


/**
 * @brief Splits a string into several substrings, by the specified delimiter.
 *
 * @param text The string to split.
 * @param del The delimiter. Default is space.
 * @param inc_empty If true, include empty substrings on the vector.
 * i.e. if two delimiters come together in a row,
 * keep an empty substring between.
 * @param inc_del If true, include the delimiters on the vector as a substring.
 * @return The substrings.
 */
vector<string> split(
    string text, const string &del, bool inc_empty, bool inc_del
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


/**
 * @brief Peeks the next characters in a string, and returns whether they match
 * the specified filter.
 *
 * @param s String to parse.
 * @param where What character index to start peeking at.
 * @param match What string to match with.
 * @return Whether it matches.
 */
bool str_peek(const string &s, size_t where, const string &match) {
    if(where + match.size() > s.size()) return false;
    return s.substr(where, match.size()) == match;
}


/**
 * @brief Converts an entire string into lowercase.
 *
 * @param s String to convert.
 * @return The string in lowercase.
 */
string str_to_lower(string s) {
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; c++) {
        s[c] = (char) tolower(s[c]);
    }
    return s;
}


/**
 * @brief Converts an entire string into title case.
 *
 * @param s String to convert.
 * @return The string in title case.
 */
string str_to_title(string s) {
    size_t letter_streak = 0;
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; c++) {
        if(isalpha(s[c])) {
            if(letter_streak == 0) {
                s[c] = (char) toupper(s[c]);
            } else {
                s[c] = (char) tolower(s[c]);
            }
            letter_streak++;
        } else {
            letter_streak = 0;
        }
    }
    return s;
}


/**
 * @brief Converts an entire string into uppercase.
 *
 * @param s String to convert.
 * @return The string in uppercase.
 */
string str_to_upper(string s) {
    size_t n_characters = s.size();
    for(size_t c = 0; c < n_characters; c++) {
        s[c] = (char) toupper(s[c]);
    }
    return s;
}


/**
 * @brief Represents units of time in a more human-readable format,
 * by dividing the units by 60 so that you end up with two portions.
 *
 * @param units How many units of time in total.
 * @param suffix1 Suffix for the first portion. Can be empty.
 * @param suffix2 Suffix for the second portion. Can be empty.
 * @param flags Flags to change behavior with. Use TIME_TO_STR_FLAG.
 * @return The time string.
 */
string time_to_str2(
    size_t units,
    const string &suffix1, const string &suffix2,
    uint8_t flags
) {
    size_t units1 = units / 60;
    size_t units2 = units % 60;
    bool has_leading_01 =
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZEROS) == 0) && (units1 < 10);
    bool has_leading_02 =
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZEROS) == 0) && (units2 < 10);
    string portion1;
    if(
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS) == 0) ||
        units1 != 0
    ) {
        portion1 =
            (has_leading_01 ? "0" : "") +
            i2s(units1) +
            suffix1;
    }
    string portion2 =
        (has_leading_02 ? "0" : "") +
        i2s(units2) +
        suffix2;
        
    return portion1 + portion2;
    
}


/**
 * @brief Represents units of time in a more human-readable format,
 * by dividing the units by 60 so that you end up with three portions.
 *
 * @param units How many units of time in total.
 * @param suffix1 Suffix for the first portion. Can be empty.
 * @param suffix2 Suffix for the second portion. Can be empty.
 * @param suffix3 Suffix for the third portion. Can be empty.
 * @param flags Flags to change behavior with. Use TIME_TO_STR_FLAG.
 * @return The time string.
 */
string time_to_str3(
    size_t units,
    const string &suffix1, const string &suffix2, const string &suffix3,
    uint8_t flags
) {
    size_t units1 = units / 60 / 60;
    size_t units2 = (units / 60) % 60;
    size_t units3 = units % 60;
    bool has_leading_01 =
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZEROS) == 0) && (units1 < 10);
    bool has_leading_02 =
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZEROS) == 0) && (units2 < 10);
    bool has_leading_03 =
        ((flags & TIME_TO_STR_FLAG_NO_LEADING_ZEROS) == 0) && (units3 < 10);
    string portion1;
    if(
        (flags & TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS) == 0 ||
        units1 != 0
    ) {
        portion1 =
            (has_leading_01 ? "0" : "") +
            i2s(units1) +
            suffix1;
    }
    string portion2;
    if(
        (flags & TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS) == 0 ||
        units1 != 0 ||
        units2 != 0
    ) {
        portion2 =
            (has_leading_02 ? "0" : "") +
            i2s(units2) +
            suffix2;
    }
    string portion3 =
        (has_leading_03 ? "0" : "") +
        i2s(units3) +
        suffix3;
        
    return
        portion1 + portion2 + portion3;
}


/**
 * @brief Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 *
 * @param s The original string.
 * @param left_only If true, only trim the spaces at the left.
 * @return The trimmed string.
 */
string trim_spaces(const string &s, bool left_only) {
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


/**
 * @brief Given a string, representing a long line of text, it automatically
 * adds line breaks along the text in order to break it up into smaller lines,
 * such that no line exceeds the number of characters in nr_chars_per_line
 * (if possible). Lines are only split at space characters.
 * This is a naive approach, in that it doesn't care about font size.
 *
 * @param s Input string.
 * @param nr_chars_per_line Number of characters that a line cannot exceed,
 * unless it's impossible to split.
 * @return The wrapped string.
 */
string word_wrap(const string &s, size_t nr_chars_per_line) {
    string result;
    string word_in_queue;
    size_t cur_line_width = 0;
    for(size_t c = 0; c < s.size() + 1; c++) {
    
        if(c < s.size() && s[c] != ' ' && s[c] != '\n') {
            //Keep building the current word.
            
            word_in_queue.push_back(s[c]);
            
        } else {
            //Finished the current word.
            
            if(word_in_queue.empty()) {
                continue;
            }
            size_t width_after_word = cur_line_width + 1 + word_in_queue.size();
            bool broke_due_to_length = false;
            if(width_after_word > nr_chars_per_line && !result.empty()) {
                //The current word doesn't fit in the current line. Break.
                result.push_back('\n');
                cur_line_width = 0;
                broke_due_to_length = true;
            }
            
            if(cur_line_width > 0) {
                result.push_back(' ');
                cur_line_width++;
            }
            result += word_in_queue;
            cur_line_width += word_in_queue.size();
            word_in_queue.clear();
            
            if(s[c] == '\n' && !broke_due_to_length) {
                //A real line break character. Break.
                result.push_back('\n');
                cur_line_width = 0;
            }
        }
    }
    return result;
}
