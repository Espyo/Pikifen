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

#include "os_utils.h"


/**
 * @brief Opens the operative system's file explorer on the specified folder.
 *
 * @param path Path of the folder to open.
 * @return Whether it succeeded.
 */
bool open_file_explorer(const string &path) {
#ifdef _WIN32
    string command = "explorer " + path;
#elif __APPLE__
    string command = "open " + path;
#elif __linux__
    string command = "xdg-open " + path;
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
bool open_web_browser(const string &url) {
#ifdef _WIN32
    string command = "start " + url;
#elif __APPLE__
    string command = "open " + url;
#elif __linux__
    string command = "xdg-open " + url;
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
