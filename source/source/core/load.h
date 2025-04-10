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


void loadAreaMissionRecord(
    DataNode* file, Area* area_ptr, MissionRecord &record
);
ALLEGRO_AUDIO_STREAM* loadAudioStream(
    const string &file_name, DataNode* node = nullptr,
    bool report_errors = true
);
ALLEGRO_BITMAP* loadBmp(
    const string &path, DataNode* node = nullptr,
    bool report_error = true, bool error_bmp_on_error = true,
    bool error_bmp_on_empty = true
);
DataNode loadDataFile(const string &file_path);
void loadMakerTools();
ALLEGRO_FONT* loadFont(
    const string &file_name, int n, const int ranges[], int size
);
void loadFonts();
void loadMiscGraphics();
void loadMiscSounds();
void loadOptions();
ALLEGRO_SAMPLE* loadSample(
    const string &file_name, DataNode* node = nullptr,
    bool report_errors = true
);
void loadStatistics();

void unloadMiscResources();
