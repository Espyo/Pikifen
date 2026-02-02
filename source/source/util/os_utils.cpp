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

#undef _CMATH_

#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <time.h>

#ifdef _WIN32
#include <filesystem>
#include <windows.h>
#include <shellapi.h> //For ShellExecuteA().
#endif

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
    string absPath = std::filesystem::absolute(path).string();
    INT_PTR result =
        (INT_PTR) ShellExecuteA(
            NULL, "open", absPath.c_str(), NULL, NULL, SW_SHOWDEFAULT
        );
    if(result <= 32) {
        std::cout << "Failed to open file explorer with path \"";
        std::cout << absPath << "\"! Error: " << ((int) result) << std::endl;
        return false;
    }
    return true;
    
#elif __APPLE__
    string command = "open \"" + path + "\"";
    return std::system(command.c_str()) == 0;
    
#elif __linux__
    string command = "xdg-open \"" + path + "\"";
    return std::system(command.c_str()) == 0;
    
#else
    return false;
    
#endif
}


/**
 * @brief Opens the operative system's web browser on the specified URL.
 *
 * @param url URL to open.
 * @return Whether it succeeded.
 */
bool openWebBrowser(const string& url) {
#ifdef _WIN32
    INT_PTR result =
        (INT_PTR) ShellExecuteA(
            NULL, "open", url.c_str(), NULL, NULL, SW_SHOWDEFAULT
        );
    if(result <= 32) {
        std::cout << "Failed to open web browser with URL \"";
        std::cout << url << "\"! Error: " << ((int) result) << std::endl;
        return false;
    }
    return true;
    
#elif __APPLE__
    return openFileExplorer(url);
    
#elif __linux__
    return openFileExplorer(url);
    
#else
    return false;
    
#endif
}


#ifdef _WIN32
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
