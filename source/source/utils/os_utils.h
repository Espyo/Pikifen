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


bool open_file_explorer(const std::string& path);

#if defined(_WIN32)
string strsignal(int signum);
#endif //#if defined(_WIN32)
