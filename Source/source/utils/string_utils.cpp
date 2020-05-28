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


//Converts a boolean to a string, returning either "true" or "false".
string b2s(const bool b) { return b ? "true" : "false"; }


/* ----------------------------------------------------------------------------
 * Boxes a string so that it becomes a specific size.
 * Truncates if it's too big, pads with spaces if it's too small.
 * s:        String to box.
 * size:     Maximum size of the return string.
 * finisher: This comes after s and before the padding (if any). This must
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


//Converts a float to a string, with 4 decimal places.
string f2s(const float f) {
    std::stringstream s;
    s << std::fixed << std::setprecision(4) << f;
    return s.str();
}


/* ----------------------------------------------------------------------------
 * Checks if the contents of a string are a number or not.
 */
bool is_number(const string &s) {
    unsigned char ch;
    for(size_t c = 0; c < s.size(); ++c) {
        ch = s[c];
        if((ch < '0' || ch > '9') && ch != '-' && ch != ',' && ch != '.') {
            return false;
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Given a file name as a string, it removes the extension.
 * Returns the string as it is if there is no extension.
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
 */
string replace_all(string s, const string &search, const string &replacement) {
    size_t pos = s.find(search);
    while(pos != string::npos) {
        s.replace(pos, search.size(), replacement);
        pos = s.find(search, pos + replacement.size());
    };
    
    return s;
}


//Converts a string to a boolean, judging by
//the English language words that represent true and false.
bool s2b(const string &s) {
    string s2 = s;
    s2 = str_to_lower(s2);
    s2 = trim_spaces(s2);
    if(s2 == "yes" || s2 == "true" || s2 == "y" || s2 == "t") return true;
    else return (s2i(s2) != 0);
}


//Converts a string to a float,
//trimming the spaces and accepting commas or points.
double s2f(const string &s) {
    string s2 = trim_spaces(s);
    replace(s2.begin(), s2.end(), ',', '.');
    return atof(s2.c_str());
}


//Converts a string to an integer.
int s2i(const string &s) { return s2f(s); }


/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 *   i.e. if two delimiters come together in a row,
 *   keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(
    string text, const string &del, const bool inc_empty, const bool inc_del
) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if (pos != string::npos) {  //If it DID find the delimiter.
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
    if (text != "" || inc_empty) {
        v.push_back(text);
    }
    
    return v;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into lowercase.
 */
string str_to_lower(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; ++c) {
        s[c] = tolower(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into uppercase.
 */
string str_to_upper(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; ++c) {
        s[c] = toupper(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 * s:         The original string.
 * left_only: If true, only trim the spaces at the left.
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
