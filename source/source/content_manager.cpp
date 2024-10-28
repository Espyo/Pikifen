/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Content manager class and related functions.
 */

#include "content_manager.h"

#include <algorithm>

#include "game.h"
#include "functions.h"
#include "init.h"
#include "load.h"
#include "mob_types/mob_type.h"
#include "utils/allegro_utils.h"
#include "utils/string_utils.h"


/**
 * @brief Constructs a new content manager object.
 */
content_manager::content_manager() {
    for(size_t c = 0; c < N_CONTENT_TYPES; c++) {
        load_levels[c] = CONTENT_LOAD_LEVEL_UNLOADED;
    }
    areas.insert(areas.begin(), N_AREA_TYPES, vector<area_data*>());
}


/**
 * @brief Fills in a given manifest map.
 *
 * @param manifest Manifest map to fill.
 * @param content_rel_path Path to the content, relative to the start
 * of the package.
 * @param folders True if the content is folders, false if it's files.
 */
void content_manager::fill_manifest(
    map<string, content_manifest> &manifest, const string &content_rel_path, bool folders
) {
    fill_manifest_from_package(manifest, FOLDER_NAMES::BASE_PKG, content_rel_path, folders);
}


/**
 * @brief Fills in a given manifest map from within a package folder.
 *
 * @param manifest Manifest map to fill.
 * @param package_name Name of the package folder.
 * @param content_rel_path Path to the content, relative to the start
 * of the package.
 * @param folders True if the content is folders, false if it's files.
 */
void content_manager::fill_manifest_from_package(
    map<string, content_manifest> &manifest, const string &package_name,
    const string &content_rel_path, bool folders
) {

    const string folder_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + package_name +
        "/" + content_rel_path;
        
    vector<string> items =
        folder_to_vector_recursively(folder_path, folders);
        
    for(size_t i = 0; i < items.size(); i++) {
        manifest[items[i]] = content_manifest(folder_path + "/" + items[i], package_name);
    }
}


/**
 * @brief Loads some game content.
 *
 * @param type Type of game content to load.
 * @param level Level to load at.
 */
void content_manager::load_all(CONTENT_TYPE type, CONTENT_LOAD_LEVEL level) {
    engine_assert(
        load_levels[type] == CONTENT_LOAD_LEVEL_UNLOADED,
        "Tried to load all content of type " + i2s(type) + " even though it's "
        "already loaded!"
    );
    
    switch(type) {
    case CONTENT_TYPE_GLOBAL_ANIMATION: {
        load_global_animations(level);
        break;
    } case CONTENT_TYPE_AREA: {
        load_areas();
        break;
    } case CONTENT_TYPE_SONG: {
        load_songs(level);
        break;
    } case CONTENT_TYPE_SONG_TRACK: {
        load_song_tracks(level);
        break;
    } case CONTENT_TYPE_SOUND: {
        load_sounds(level);
        break;
    } case CONTENT_TYPE_BITMAP: {
        load_bitmaps(level);
        break;
    } case CONTENT_TYPE_CUSTOM_PARTICLE_GEN: {
        load_custom_particle_generators(level);
        break;
    } case CONTENT_TYPE_GUI: {
        load_gui_definitions(level);
        break;
    } case CONTENT_TYPE_HAZARD: {
        load_hazards(level);
        break;
    } case CONTENT_TYPE_LIQUID: {
        load_liquids(level);
        break;
    } case CONTENT_TYPE_MISC: {
        load_misc(level);
        break;
    } case CONTENT_TYPE_MOB_TYPE: {
        load_mob_types(level);
        break;
    } case CONTENT_TYPE_SPIKE_DAMAGE_TYPE: {
        load_spike_damage_types(level);
        break;
    } case CONTENT_TYPE_SPRAY_TYPE: {
        load_spray_types(level);
        break;
    } case CONTENT_TYPE_STATUS_TYPE: {
        load_status_types(level);
        break;
    } case CONTENT_TYPE_WEATHER_CONDITION: {
        load_weather_conditions(level);
        break;
    } case N_CONTENT_TYPES: {
        break;
    }
    }
    
    load_levels[type] = level;
}


/**
 * @brief Loads an area.
 *
 * @param area_ptr Object to load into.
 * @param folder_name Name of the area's folder.
 * @param package_name Name of the package it is in.
 * @param type Type of area this is.
 * @param level Level to load at.
 * @param from_backup If true, load from a backup, if any.
 */
void content_manager::load_area(
    area_data* area_ptr,
    const string &folder_name, const string &package_name, AREA_TYPE type,
    CONTENT_LOAD_LEVEL level, bool from_backup
) {
    //Setup.
    string data_file_path;
    string geometry_file_path;
    string folder_path;
    
    if(from_backup) {
        folder_path =
            get_base_area_folder_path(type, false, package_name) +
            "/" + folder_name;
        data_file_path = folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA_BACKUP;
        geometry_file_path = folder_path + "/" + FILE_NAMES::AREA_GEOMETRY_BACKUP;
    } else {
        folder_path =
            get_base_area_folder_path(type, true, package_name) +
            "/" + folder_name;
        data_file_path = folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
        geometry_file_path = folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
    }
    
    data_node data_file = load_data_file(data_file_path);
    if(!data_file.file_was_opened) return;
    data_node geometry_file = load_data_file(geometry_file_path);
    if(!geometry_file.file_was_opened) return;
    
    area_ptr->internal_name = folder_name;
    area_ptr->path = folder_path;
    area_ptr->type = type;
    
    //Main data.
    if(game.perf_mon) game.perf_mon->start_measurement("Area -- Data");
    area_ptr->load_main_data_from_data_node(&data_file, level);
    area_ptr->load_mission_data_from_data_node(&data_file);
    if(game.perf_mon) game.perf_mon->finish_measurement();
    
    //Loading screen.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.loading_text_bmp) al_destroy_bitmap(game.loading_text_bmp);
        if(game.loading_subtext_bmp) al_destroy_bitmap(game.loading_subtext_bmp);
        game.loading_text_bmp = nullptr;
        game.loading_subtext_bmp = nullptr;
        draw_loading_screen(
            area_ptr->name,
            get_subtitle_or_mission_goal(
                area_ptr->subtitle,
                area_ptr->type,
                area_ptr->mission.goal
            ),
            1.0f
        );
        al_flip_display();
    }
    
    //Thumbnail image.
    string thumbnail_path =
        folder_path +
        (from_backup ? "/thumbnail_backup.png" : "/thumbnail.png");
    area_ptr->load_thumbnail(thumbnail_path);
    
    //Geometry.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.perf_mon) game.perf_mon->start_measurement("Area -- Geometry");
        area_ptr->load_geometry_from_data_node(&geometry_file, level);
        if(game.perf_mon) game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads an area as the "current area". This does not load it into
 * the vector of areas.
 *
 * @param folder_name Name of the area's folder.
 * @param package_name Name of the package it is in.
 * @param type Type of area this is.
 * @param level Level to load at.
 * @param from_backup If true, load from a backup, if any.
 */
void content_manager::load_area_as_current(
    const string &folder_name, const string &package_name, AREA_TYPE type,
    CONTENT_LOAD_LEVEL level, bool from_backup
) {
    engine_assert(
        game.cur_area_data == nullptr,
        "Tried to load area \"" + folder_name + "\" (package \"" +
        package_name + "\") as the current one even though there is "
        "already a loaded current area, \"" + game.cur_area_data->path + "\"!"
    );
    
    game.cur_area_data = new area_data();
    load_area(
        game.cur_area_data, folder_name, package_name, type, level, from_backup
    );
}


/**
 * @brief Loads an area into the vector of areas. This does not load it as the
 * "current" area.
 *
 * @param folder_name Name of the area's folder.
 * @param package_name Name of the package it is in.
 * @param type Type of area this is.
 * @param from_backup If true, load from a backup, if any.
 */
void content_manager::load_area_into_vector(
    const string &folder_name, const string &package_name, AREA_TYPE type,
    bool from_backup
) {
    area_data* new_area = new area_data();
    areas[type].push_back(new_area);
    load_area(
        new_area, folder_name, package_name, type,
        CONTENT_LOAD_LEVEL_BASIC, from_backup
    );
}


/**
 * @brief Loads areas into the areas vector. This does not load any area as the
 * "current" area.
 */
void content_manager::load_areas() {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        manifests.areas.push_back(map<string, content_manifest>());
        fill_manifest(
            manifests.areas[t],
            t == AREA_TYPE_SIMPLE ?
            FOLDER_PATHS_FROM_PKG::SIMPLE_AREAS :
            FOLDER_PATHS_FROM_PKG::MISSION_AREAS,
            true
        );
    }
    
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        for(const auto &a : manifests.areas[t]) {
            load_area_into_vector(
                remove_extension(a.first), a.second.package,
                (AREA_TYPE) t, false
            );
        }
    }
}


/**
 * @brief Loads a global animation.
 *
 * @param path Path to the animation.
 * @param level Level to load at.
 */
void content_manager::load_global_animation(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file(path);
    single_animation_suite anim;
    anim.database.internal_name = internal_name;
    anim.database.path = path;
    anim.database.load_from_data_node(&file);
    anim.instance.cur_anim = anim.database.animations[0];
    anim.instance.to_start();
    global_animations[internal_name] = anim;
}


/**
 * @brief Loads global animations.
 *
 * @param level Level to load at.
 */
void content_manager::load_global_animations(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.global_animations, FOLDER_PATHS_FROM_PKG::GLOBAL_ANIMATIONS, false);
    
    for(const auto &a : manifests.global_animations) {
        load_global_animation(remove_extension(a.first), a.second.path, level);
    }
}


/**
 * @brief Loads a manifest of the bitmaps.
 *
 * @param level Level to load at.
 */
void content_manager::load_bitmaps(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.bitmaps, FOLDER_PATHS_FROM_PKG::GRAPHICS, false);
}


/**
 * @brief Loads a user-made particle generator.
 *
 * @param path Path to the particle generator.
 * @param level Level to load at.
 */
void content_manager::load_custom_particle_generator(
    const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    particle_generator new_pg;
    new_pg.internal_name = internal_name;
    new_pg.path = path;
    new_pg.load_from_data_node(&file, level);
    custom_particle_generators[internal_name] = new_pg;
}


/**
 * @brief Loads user-made particle generators.
 *
 * @param level Level to load at.
 */
void content_manager::load_custom_particle_generators(
    CONTENT_LOAD_LEVEL level
) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Custom particle generators");
    }
    
    fill_manifest(manifests.custom_particle_generators, FOLDER_PATHS_FROM_PKG::PARTICLE_GENERATORS, false);
    
    for(const auto &g : manifests.custom_particle_generators) {
        load_custom_particle_generator(remove_extension(g.first), g.second.path, level);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads GUI definitions.
 *
 * @param level Level to load at.
 */
void content_manager::load_gui_definitions(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.gui, FOLDER_PATHS_FROM_PKG::GUI, false);
    
    for(const auto &g : manifests.gui) {
        gui[g.first] = load_data_file(g.second.path);
    }
}


/**
 * @brief Loads a hazard.
 *
 * @param path Path to the hazard.
 * @param level Level to load at.
 */
void content_manager::load_hazard(
    const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    hazard new_h;
    new_h.internal_name = internal_name;
    new_h.path = path;
    new_h.load_from_data_node(&file);
    hazards[internal_name] = new_h;
}


/**
 * @brief Loads hazards.
 *
 * @param level Level to load at.
 */
void content_manager::load_hazards(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Hazards");
    }
    
    fill_manifest(manifests.hazards, FOLDER_PATHS_FROM_PKG::HAZARDS, false);
    
    for(const auto &h : manifests.hazards) {
        load_hazard(remove_extension(h.first), h.second.path, level);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a liquid.
 *
 * @param internal_name Internal name of the liquid.
 * @param path Path to the liquid.
 * @param level Level to load at.
 */
void content_manager::load_liquid(
    const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    liquid* new_l = new liquid();
    new_l->internal_name = internal_name;
    new_l->path = path;
    new_l->load_from_data_node(&file, level);
    liquids[internal_name] = new_l;
}


/**
 * @brief Loads liquids.
 *
 * @param level Level to load at.
 */
void content_manager::load_liquids(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Liquid types");
    }
    
    fill_manifest(manifests.liquids, FOLDER_PATHS_FROM_PKG::LIQUIDS, false);
    
    for(const auto &l : manifests.liquids) {
        load_liquid(remove_extension(l.first), l.second.path, level);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads misc. configurations.
 *
 * @param level Level to load at.
 */
void content_manager::load_misc(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.misc_configs, FOLDER_PATHS_FROM_PKG::MISC, false);
    
    //Game config.
    data_node game_config_file =
        load_data_file(manifests.misc_configs[FILE_NAMES::GAME_CONFIG].path);
    game.config.load(&game_config_file);
    al_set_window_title(
        game.display,
        game.config.name.empty() ? "Pikifen" : game.config.name.c_str()
    );
    
    //System animations.
    data_node system_animations_file =
        load_data_file(manifests.misc_configs[FILE_NAMES::SYSTEM_ANIMS].path);
    init_single_animation(
        &system_animations_file, "leader_damage_sparks",
        game.sys_assets.spark_animation
    );
    
    //System asset file names.
    data_node system_asset_fn_file =
        load_data_file(manifests.misc_configs[FILE_NAMES::SYSTEM_ASSET_FILE_NAMES].path);
    game.asset_file_names.load(&system_asset_fn_file);
}


/**
 * @brief Loads mob types.
 *
 * @param level Level to load at.
 */
void content_manager::load_mob_types(CONTENT_LOAD_LEVEL level) {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.mob_types.push_back(map<string, content_manifest>());
        fill_manifest(
            manifests.mob_types[c],
            FOLDER_PATHS_FROM_PKG::MOB_TYPES + "/" +
            game.mob_categories.get((MOB_CATEGORY) c)->folder_name,
            true
        );
    }
    
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) {
            continue;
        }
        
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        if(game.perf_mon) {
            game.perf_mon->start_measurement(
                "Object types -- " + category->name
            );
        }
        
        load_mob_types_of_category(category, level);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
    }
    
    //Pikmin type order.
    vector<string> missing_pikmin_order_types;
    for(auto &p : mob_types.pikmin) {
        if(
            find(
                game.config.pikmin_order_strings.begin(),
                game.config.pikmin_order_strings.end(),
                p.first
            ) == game.config.pikmin_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_pikmin_order_types.push_back(p.first);
        }
    }
    if(!missing_pikmin_order_types.empty()) {
        std::sort(
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
        game.config.pikmin_order_strings.insert(
            game.config.pikmin_order_strings.end(),
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.pikmin_order_strings.size(); o++) {
        string s = game.config.pikmin_order_strings[o];
        if(mob_types.pikmin.find(s) != mob_types.pikmin.end()) {
            game.config.pikmin_order.push_back(mob_types.pikmin[s]);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    vector<string> missing_leader_order_types;
    for(auto &l : mob_types.leader) {
        if(
            find(
                game.config.leader_order_strings.begin(),
                game.config.leader_order_strings.end(),
                l.first
            ) == game.config.leader_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_leader_order_types.push_back(l.first);
        }
    }
    if(!missing_leader_order_types.empty()) {
        std::sort(
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
        game.config.leader_order_strings.insert(
            game.config.leader_order_strings.end(),
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.leader_order_strings.size(); o++) {
        string s = game.config.leader_order_strings[o];
        if(mob_types.leader.find(s) != mob_types.leader.end()) {
            game.config.leader_order.push_back(mob_types.leader[s]);
        } else {
            game.errors.report(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the config file!"
            );
        }
    }
    
    //Create the special mob types.
    create_special_mob_types();
}


/**
 * @brief Loads the mob types from a category's folder.
 *
 * @param category Pointer to the mob category.
 * @param level Level to load at.
 */
void content_manager::load_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level) {
    if(category->folder_name.empty()) return;
    
    map<string, content_manifest> &man = manifests.mob_types[category->id];
    for(const auto &t : man) {
        data_node file(t.second.path + "/data.txt");
        if(!file.file_was_opened) continue;
        
        mob_type* mt;
        mt = category->create_type();
        mt->load_from_data_node(&file, level, t.second.path);
        category->register_type(t.first, mt);
        mt->internal_name = t.first;
        mt->path = t.second.path;
        
    }
    
}


/**
 * @brief Loads a song.
 *
 * @param path Path to the song.
 * @param level Level to load at.
 */
void content_manager::load_song(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    song new_song;
    new_song.internal_name = internal_name;
    new_song.path = path;
    new_song.load_from_data_node(&file);
    songs[internal_name] = new_song;
}


/**
 * @brief Loads songs.
 *
 * @param level Level to load at.
 */
void content_manager::load_songs(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.songs, FOLDER_PATHS_FROM_PKG::SONGS, false);
    
    for(const auto &s : manifests.songs) {
        load_song(remove_extension(s.first), s.second.path, level);
    }
}


/**
 * @brief Loads song tracks.
 *
 * @param level Level to load at.
 */
void content_manager::load_song_tracks(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.song_tracks, FOLDER_PATHS_FROM_PKG::SONG_TRACKS, false);
}


/**
 * @brief Loads sound effects.
 *
 * @param level Level to load at.
 */
void content_manager::load_sounds(CONTENT_LOAD_LEVEL level) {
    fill_manifest(manifests.sounds, FOLDER_PATHS_FROM_PKG::SOUNDS, false);
}


/**
 * @brief Loads a spike damage type.
 *
 * @param path Path to the spike damage type.
 * @param level Level to load at.
 */
void content_manager::load_spike_damage_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    spike_damage_type new_t;
    new_t.internal_name = internal_name;
    new_t.path = path;
    new_t.load_from_data_node(&file);
    spike_damage_types[internal_name] = new_t;
}


/**
 * @brief Loads spike damage types.
 *
 * @param level Level to load at.
 */
void content_manager::load_spike_damage_types(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spike damage types");
    }
    
    fill_manifest(manifests.spike_damage_types, FOLDER_PATHS_FROM_PKG::SPIKE_DAMAGES_TYPES, false);
    
    for(const auto &s : manifests.spike_damage_types) {
        load_spike_damage_type(remove_extension(s.first), s.second.path, level);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a spray type.
 *
 * @param path Path to the spray type.
 * @param level Level to load at.
 */
void content_manager::load_spray_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    spray_type new_t;
    new_t.internal_name = internal_name;
    new_t.path = path;
    new_t.load_from_data_node(&file, level);
    spray_types[internal_name] = new_t;
}


/**
 * @brief Loads spray types.
 *
 * @param level Level to load at.
 */
void content_manager::load_spray_types(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spray types");
    }
    
    fill_manifest(manifests.spray_types, FOLDER_PATHS_FROM_PKG::SPRAYS, false);
    
    for(const auto &s : manifests.spray_types) {
        load_spray_type(remove_extension(s.first), s.second.path, level);
    }
    
    //Spray type order.
    vector<string> missing_spray_order_types;
    for(auto &s : spray_types) {
        if(
            find(
                game.config.spray_order_strings.begin(),
                game.config.spray_order_strings.end(),
                s.first
            ) == game.config.spray_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_spray_order_types.push_back(s.first);
        }
    }
    if(!missing_spray_order_types.empty()) {
        std::sort(
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
        game.config.spray_order_strings.insert(
            game.config.spray_order_strings.end(),
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.spray_order_strings.size(); o++) {
        string s = game.config.spray_order_strings[o];
        if(spray_types.find(s) != spray_types.end()) {
            game.config.spray_order.push_back(&spray_types[s]);
        } else {
            game.errors.report(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the config file!"
            );
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a status type.
 *
 * @param path Path to the status type.
 * @param level Level to load at.
 */
void content_manager::load_status_type(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    status_type* new_t = new status_type();
    new_t->internal_name = internal_name;
    new_t->path = path;
    new_t->load_from_data_node(&file, level);
    status_types[internal_name] = new_t;
}


/**
 * @brief Loads status types.
 *
 * @param level Level to load at.
 */
void content_manager::load_status_types(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Status types");
    }
    
    vector<status_type*> types_with_replacements;
    vector<string> types_with_replacements_names;
    
    fill_manifest(manifests.status_types, FOLDER_PATHS_FROM_PKG::STATUSES, false);
    
    for(const auto &s : manifests.status_types) {
        load_status_type(remove_extension(s.first), s.second.path, level);
    }
    
    for(auto &s : status_types) {
        if(!s.second->replacement_on_timeout_str.empty()) {
            types_with_replacements.push_back(s.second);
            types_with_replacements_names.push_back(
                s.second->replacement_on_timeout_str
            );
        }
    }
    
    for(size_t s = 0; s < types_with_replacements.size(); s++) {
        string rn = types_with_replacements_names[s];
        bool found = false;
        for(auto &s2 : status_types) {
            if(s2.first == rn) {
                types_with_replacements[s]->replacement_on_timeout =
                    s2.second;
                found = true;
                break;
            }
        }
        if(found) continue;
        
        game.errors.report(
            "The status effect type \"" +
            types_with_replacements[s]->name +
            "\" has a replacement effect called \"" + rn + "\", but there is "
            "no status effect with that name!"
        );
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads a weather condition.
 *
 * @param path Path to the weather condition.
 * @param level Level to load at.
 */
void content_manager::load_weather_condition(const string &internal_name, const string &path, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(path);
    if(!file.file_was_opened) return;
    
    weather new_w;
    new_w.internal_name = internal_name;
    new_w.path = path;
    new_w.load_from_data_node(&file);
    weather_conditions[internal_name] = new_w;
}


/**
 * @brief Loads weather conditions.
 *
 * @param level Level to load at.
 */
void content_manager::load_weather_conditions(CONTENT_LOAD_LEVEL level) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Weather");
    }
    
    fill_manifest(manifests.weather_conditions, FOLDER_PATHS_FROM_PKG::WEATHER, false);
    
    for(const auto &w : manifests.weather_conditions) {
        load_weather_condition(remove_extension(w.first), w.second.path, level);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Unloads the "current area".
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_current_area(CONTENT_LOAD_LEVEL level) {
    if(!game.cur_area_data) return;
    game.cur_area_data->clear();
    delete game.cur_area_data;
    game.cur_area_data = nullptr;
}


/**
 * @brief Unloads some loaded content.
 *
 * @param type Type of content to unload.
 */
void content_manager::unload_all(CONTENT_TYPE type) {
    switch(type) {
    case CONTENT_TYPE_GLOBAL_ANIMATION: {
        unload_global_animations(load_levels[type]);
        break;
    } case CONTENT_TYPE_AREA: {
        unload_areas(load_levels[type]);
        break;
    } case CONTENT_TYPE_SONG: {
        unload_songs(load_levels[type]);
        break;
    } case CONTENT_TYPE_SONG_TRACK: {
        unload_song_tracks(load_levels[type]);
        break;
    } case CONTENT_TYPE_SOUND: {
        unload_sounds(load_levels[type]);
        break;
    } case CONTENT_TYPE_BITMAP: {
        unload_bitmaps(load_levels[type]);
        break;
    } case CONTENT_TYPE_CUSTOM_PARTICLE_GEN: {
        unload_custom_particle_generators(load_levels[type]);
        break;
    } case CONTENT_TYPE_GUI: {
        unload_gui(load_levels[type]);
        break;
    } case CONTENT_TYPE_HAZARD: {
        unload_hazards(load_levels[type]);
        break;
    } case CONTENT_TYPE_LIQUID: {
        unload_liquids(load_levels[type]);
        break;
    } case CONTENT_TYPE_MISC: {
        unload_misc(load_levels[type]);
        break;
    } case CONTENT_TYPE_MOB_TYPE: {
        unload_mob_types(load_levels[type]);
        break;
    } case CONTENT_TYPE_SPIKE_DAMAGE_TYPE: {
        unload_spike_damage_types(load_levels[type]);
        break;
    } case CONTENT_TYPE_SPRAY_TYPE: {
        unload_spray_types(load_levels[type]);
        break;
    } case CONTENT_TYPE_STATUS_TYPE: {
        unload_status_types(load_levels[type]);
        break;
    } case CONTENT_TYPE_WEATHER_CONDITION: {
        unload_weather_conditions(load_levels[type]);
        break;
    } case N_CONTENT_TYPES: {
        break;
    }
    }
    
    load_levels[type] = CONTENT_LOAD_LEVEL_UNLOADED;
}


/**
* @brief Unloads loaded areas.

* @param level Should match the level at which the content got loaded.
*/
void content_manager::unload_areas(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < areas.size(); t++) {
        for(size_t a = 0; a < areas[t].size(); a++) {
            delete areas[t][a];
        }
        areas[t].clear();
    }
    manifests.areas.clear();
}


/**
* @brief Unloads the loaded bitmap manifest.

* @param level Should match the level at which the content got loaded.
*/
void content_manager::unload_bitmaps(CONTENT_LOAD_LEVEL level) {
    manifests.bitmaps.clear();
}


/**
 * @brief Unloads loaded user-made particle generators.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_custom_particle_generators(CONTENT_LOAD_LEVEL level) {
    for(
        auto g = custom_particle_generators.begin();
        g != custom_particle_generators.end();
        ++g
    ) {
        game.content.bitmaps.free(g->second.base_particle.bitmap);
    }
    custom_particle_generators.clear();
    manifests.custom_particle_generators.clear();
}


/**
 * @brief Unloads loaded global animations.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_global_animations(CONTENT_LOAD_LEVEL level) {
    manifests.global_animations.clear();
}


/**
 * @brief Unloads loaded GUI definitions.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_gui(CONTENT_LOAD_LEVEL level) {
    gui.clear();
    manifests.gui.clear();
}


/**
 * @brief Unloads loaded hazards.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_hazards(CONTENT_LOAD_LEVEL level) {
    hazards.clear();
    manifests.hazards.clear();
}


/**
 * @brief Unloads loaded liquids.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_liquids(CONTENT_LOAD_LEVEL level) {
    liquids.clear();
    manifests.liquids.clear();
}


/**
 * @brief Unloads misc. configurations.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_misc(CONTENT_LOAD_LEVEL level) {
    manifests.misc_configs.clear();
}


/**
 * @brief Unloads a type of mob.
 *
 * @param mt Mob type to unload.
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* s_ptr = mt->sounds[s].sample;
        if(!s) continue;
        samples.free(s_ptr);
    }
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        mt->anims.destroy();
        unload_script(mt);
        
        mt->unload_resources();
    }
}


/**
 * @brief Unloads loaded mob types.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_mob_types(CONTENT_LOAD_LEVEL level) {
    game.config.leader_order.clear();
    game.config.pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        unload_mob_types_of_category(category, level);
    }
    manifests.mob_types.clear();
}


/**
 * @brief Unloads all loaded types of mob from a category.
 *
 * @param category Pointer to the mob category.
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level) {

    vector<string> type_names;
    category->get_type_names(type_names);
    
    for(size_t t = 0; t < type_names.size(); t++) {
        mob_type* mt = category->get_type(type_names[t]);
        unload_mob_type(mt, level);
    }
    
    category->clear_types();
}


/**
 * @brief Unloads loaded song tracks.
 */
void content_manager::unload_song_tracks(CONTENT_LOAD_LEVEL level) {
    manifests.song_tracks.clear();
}


/**
 * @brief Unloads loaded songs.
 */
void content_manager::unload_songs(CONTENT_LOAD_LEVEL level) {
    for(auto &s : songs) {
        s.second.unload();
    }
    songs.clear();
    manifests.songs.clear();
}


/**
 * @brief Unloads loaded sound effects.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_sounds(CONTENT_LOAD_LEVEL level) {
    manifests.sounds.clear();
}


/**
 * @brief Unloads loaded spike damage types.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_spike_damage_types(CONTENT_LOAD_LEVEL level) {
    spike_damage_types.clear();
    manifests.spike_damage_types.clear();
}


/**
 * @brief Unloaded loaded spray types.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_spray_types(CONTENT_LOAD_LEVEL level) {
    for(const auto &s : spray_types) {
        bitmaps.free(s.second.bmp_spray);
    }
    game.config.spray_order.clear();
    spray_types.clear();
    manifests.spray_types.clear();
}


/**
 * @brief Unloaded loaded status types.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_status_types(CONTENT_LOAD_LEVEL level) {
    status_types.clear();
    manifests.status_types.clear();
}


/**
 * @brief Unloads loaded weather conditions.
 *
 * @param level Should match the level at which the content got loaded.
 */
void content_manager::unload_weather_conditions(CONTENT_LOAD_LEVEL level) {
    weather_conditions.clear();
    manifests.weather_conditions.clear();
}


/**
 * @brief Constructs a new content manifest object.
 */
content_manifest::content_manifest() {}


/**
 * @brief Constructs a new content manifest object.
 *
 * @param path Path to the content, relative to the packages folder.
 * @param package Package it belongs to.
 */
content_manifest::content_manifest(const string &path, const string &package) :
    path(path),
    package(package) {
    
}
