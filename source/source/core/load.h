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
    DataNode* file, Area* areaPtr, MissionRecord& record
);
ALLEGRO_AUDIO_STREAM* loadAudioStream(
    const string& fileName, DataNode* node = nullptr,
    bool reportErrors = true
);
ALLEGRO_BITMAP* loadBmp(
    const string& path, DataNode* node = nullptr,
    bool reportError = true, bool errorBmpOnError = true,
    bool errorBmpOnEmpty = true
);
DataNode loadDataFile(const string& filePath);
void loadMakerTools();
ALLEGRO_FONT* loadFont(
    const string& fileName, int n, const int ranges[], int size
);
void loadFonts();
void loadMiscGraphics();
void loadMiscSounds();
void loadOptions();
ALLEGRO_SAMPLE* loadSample(
    const string& fileName, DataNode* node = nullptr,
    bool reportErrors = true
);
void loadStatistics();

void unloadMiscResources();
