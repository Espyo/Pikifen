/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Operative system utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <cstdlib>
#include <signal.h>
#include <time.h>

#include "os_utils.h"

#include "string_utils.h"


/**
 * @brief Returns a string representing the current date and time,
 * in ISO 8601 format (YYYY/MM/DD)).
 *
 * @param fileNameFriendly If true, slashes become dashes,
 * and semicolons become dots.
 * @return The string.
 */
string getCurrentTime(bool fileNameFriendly) {
    time_t tt;
    time(&tt);
    struct tm t;
    
#ifdef _WIN32
    localtime_s(&t, &tt);
#else
    localtime_r(&tt, &t);
#endif
    
    return
        i2s(t.tm_year + 1900) +
        (fileNameFriendly ? "-" : "/") +
        leadingZero(t.tm_mon + 1) +
        (fileNameFriendly ? "-" : "/") +
        leadingZero(t.tm_mday) +
        (fileNameFriendly ? "_" : " ") +
        leadingZero(t.tm_hour) +
        (fileNameFriendly ? "." : ":") +
        leadingZero(t.tm_min) +
        (fileNameFriendly ? "." : ":") +
        leadingZero(t.tm_sec);
}


/**
 * @brief Opens the operative system's file explorer on the specified folder.
 *
 * @param path Path of the folder to open.
 * @return Whether it succeeded.
 */
bool openFileExplorer(const string& path) {
#ifdef _WIN32
    string command = "explorer \"" + path + "\"";
#elif __APPLE__
    string command = "open \"" + path + "\"";
#elif __linux__
    string command = "xdg-open \"" + path + "\"";
#else
    return false;
#endif
    return std::system(command.c_str()) == 0;
}


/**
 * @brief Opens the operative system's web browser on the specified URL.
 *
 * @param url URL to open.
 * @return Whether it succeeded.
 */
bool openWebBrowser(const string& url) {
#ifdef _WIN32
    string command = "start \"" + url + "\"";
#elif __APPLE__
    string command = "open \"" + url + "\"";
#elif __linux__
    string command = "xdg-open \"" + url + "\"";
#else
    return false;
#endif
    return std::system(command.c_str()) == 0;
}


#if defined(_WIN32)
/**
 * @brief An implementation of strsignal from POSIX.
 *
 * @param signum Signal number.
 * @return The string.
 */
string strsignal(int signum) {
    switch(signum) {
    case SIGINT: {
        return "SIGINT";
    } case SIGILL: {
        return "SIGILL";
    } case SIGFPE: {
        return "SIGFPE";
    } case SIGSEGV: {
        return "SIGSEGV";
    } case SIGTERM: {
        return "SIGTERM";
    } case SIGBREAK: {
        return "SIGBREAK";
    } case SIGABRT: {
        return "SIGABRT";
    } case SIGABRT_COMPAT: {
        return "SIGABRT_COMPAT";
    } default: {
        return "Unknown";
    }
    }
}
#endif //if defined(_WIN32)
