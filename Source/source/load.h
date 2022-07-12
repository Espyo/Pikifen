/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for data loading and unloading functions.
 */

#ifndef LOAD_INCLUDED
#define LOAD_INCLUDED

#include <string>

#include <allegro5/allegro.h>

#include "area.h"
#include "misc_structs.h"
#include "utils/data_file.h"


using std::string;


void load_area(
    const AREA_TYPES type, const string &name,
    const bool load_for_editor, const bool from_backup
);
void load_asset_file_names();
ALLEGRO_BITMAP* load_bmp(
    const string &file_name, data_node* node = NULL,
    const bool report_error = true, const bool error_bmp_on_error = true,
    const bool error_bmp_on_empty = true, const bool path_from_root = false
);
void load_custom_particle_generators(const bool load_resources);
data_node load_data_file(const string &file_name);
void load_maker_tools();
void load_fonts();
void load_game_config();
void load_hazards();
void load_liquids(const bool load_resources);
void load_misc_graphics();
void load_misc_sounds();
void load_options();
sample_struct load_sample(const string &file_name);
void load_spike_damage_types();
void load_spray_types(const bool load_resources);
void load_status_types(const bool load_resources);
void load_system_animations();
void load_weather();

void unload_area();
void unload_custom_particle_generators();
void unload_hazards();
void unload_liquids();
void unload_misc_resources();
void unload_spike_damage_types();
void unload_spray_types();
void unload_status_types(const bool unload_resources);
void unload_weather();


#endif //ifndef LOAD_INCLUDED
