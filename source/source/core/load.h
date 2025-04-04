/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for data loading and unloading functions.
 */

#pragma once

#include <string>

#include <allegro5/allegro.h>

#include "../content/area/area.h"
#include "../game_state/editor.h"
#include "../lib/data_file/data_file.h"
#include "misc_structs.h"


using std::string;


void load_area_mission_record(
    DataNode* file, Area* area_ptr, MissionRecord &record
);
ALLEGRO_AUDIO_STREAM* load_audio_stream(
    const string &file_name, DataNode* node = nullptr,
    bool report_errors = true
);
ALLEGRO_BITMAP* load_bmp(
    const string &path, DataNode* node = nullptr,
    bool report_error = true, bool error_bmp_on_error = true,
    bool error_bmp_on_empty = true
);
DataNode load_data_file(const string &file_path);
void load_maker_tools();
ALLEGRO_FONT* load_font(
    const string &file_name, int n, const int ranges[], int size
);
void load_fonts();
void load_misc_graphics();
void load_misc_sounds();
void load_options();
ALLEGRO_SAMPLE* load_sample(
    const string &file_name, DataNode* node = nullptr,
    bool report_errors = true
);
void load_statistics();

void unload_misc_resources();
