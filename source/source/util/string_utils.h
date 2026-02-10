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


//Returns a string with a number, adding a leading zero if it's less than 10.
#define leadingZero(n) (((n) < 10 ? "0" : (string) "") + i2s((n)))

//Converts an integer (or long) to a string.
#define i2s(n) std::to_string((long long) (n))

string amountStr(
    int amount, const string& singularText,
    const string& pluralText = "", bool justWord = false
);
string b2s(bool b);
void duplicateString(const string& origStr, string& newStr);
string f2s(float f);
string getMatchingStringStarts(const string& s1, const string& s2);
string getPathLastComponent(const string& s);
bool isNumber(const string& s);
string padString(const string& s, size_t size, char padding);
string removeExtension(const string& s);
string replaceAll(
    string s, const string& search, const string& replacement
);
string resizeString(
    const string& s, size_t intendedSize,
    bool canShrink = true, bool canGrow = true, bool changeLeft = false,
    unsigned char filler = ' ', const string& between = ""
);
bool s2b(const string& s);
double s2f(const string& s);
int s2i(const string& s);
vector<string> semicolonListToVector(
    const string& s, const string& sep = ";"
);
vector<string> split(
    string text, const string& del = " ", bool incEmpty = false,
    bool incDel = false
);
bool strEndsWith(const string& s, const string& end);
bool strPeek(const string& s, size_t where, const string& match);
bool strStartsWith(const string& s, const string& start);
string strToLower(string s);
string strToSentence(string s);
string strToTitle(string s);
string strToUpper(string s);
string timeToStr2(
    size_t units,
    const string& suffix1 = ":", const string& suffix2 = "",
    uint8_t flags = 0
);
string timeToStr3(
    size_t units, const string& suffix1 = "",
    const string& suffix2 = "", const string& suffix3 = "",
    uint8_t flags = 0
);
string trimSpaces(const string& s, bool leftOnly = false);
string trimWithEllipsis(const string& s, size_t size);
string wordWrap(const string& s, size_t nCharsPerLine);



/**
 * @brief Joins a list of strings together into one final string,
 * using a delimiter between them.
 *
 * @tparam ContainerT Type of the container of parts.
 * @param parts Parts to join.
 * @param delimiter The delimiter to place between each part.
 * @return The joined string.
 */
template<typename ContainerT>
string join(const ContainerT& parts, const string& delimiter = " ") {
    string result;
    for(const auto& p : parts) {
        if(!result.empty()) result += delimiter;
        result += p;
    }
    return result;
}
