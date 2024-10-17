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

#include "area/area.h"
#include "misc_structs.h"
#include "libs/data_file.h"


using std::string;


void load_area(
    const string &requested_area_folder_name,
    const AREA_TYPE requested_area_type,
    CONTENT_LOAD_LEVEL level, bool from_backup
);
void load_area_mission_data(data_node* node, mission_data &data);
void load_area_mission_record(
    data_node* file,
    const string &area_name, const string &area_subtitle,
    const string &area_maker, const string &area_version,
    mission_record &record
);
void load_asset_file_names();
ALLEGRO_AUDIO_STREAM* load_audio_stream(
    const string &file_name, data_node* node = nullptr,
    bool report_errors = true
);
ALLEGRO_BITMAP* load_bmp(
    const string &file_name, data_node* node = nullptr,
    bool report_error = true, bool error_bmp_on_error = true,
    bool error_bmp_on_empty = true, bool path_from_root = false
);
data_node load_data_file(const string &file_path);
void load_maker_tools();
ALLEGRO_FONT* load_font(
    const string &file_name, int n, const int ranges[], int size
);
void load_fonts();
void load_game_config();
void load_misc_graphics();
void load_misc_sounds();
void load_options();
ALLEGRO_SAMPLE* load_sample(
    const string &file_name, data_node* node = nullptr,
    bool report_errors = true
);
void load_songs();
void load_statistics();
void load_system_animations();

void unload_area();
void unload_misc_resources();
void unload_songs();
