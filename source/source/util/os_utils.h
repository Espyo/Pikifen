/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for operative system utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <string>

using std::string;


string get_current_time(bool file_name_friendly);
bool open_file_explorer(const std::string &path);
bool open_web_browser(const std::string &url);

#if defined(_WIN32)
string strsignal(int signum);
#endif //#if defined(_WIN32)
