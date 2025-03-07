/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Content type manager classes and related functions.
 */

#include <algorithm>

#include "content_type_manager.h"

#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/init.h"
#include "../core/load.h"
#include "../util/allegro_utils.h"
#include "../util/string_utils.h"


/**
 * @brief Clears the manifests.
 */
void AreaContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void AreaContentManager::fill_manifests() {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        manifests.push_back(map<string, ContentManifest>());
        fill_manifests_map(
            manifests[t],
            t == AREA_TYPE_SIMPLE ?
            FOLDER_PATHS_FROM_PACK::SIMPLE_AREAS :
            FOLDER_PATHS_FROM_PACK::MISSION_AREAS,
            true
        );
    }
}


/**
 * @brief Returns the manifest matching the specified area, or nullptr if none
 * was found.
 *
 * @param area_name Name of the area.
 * @param pack Pack it belongs to.
 * @param type Area type.
 * @return The manifest, or nullptr.
 */
ContentManifest* AreaContentManager::find_manifest(
    const string &area_name, const string &pack, AREA_TYPE type
) {
    for(auto &m : manifests[type]) {
        if(m.first == area_name && m.second.pack == pack) {
            return &m.second;
        }
    }
    return nullptr;
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string AreaContentManager::get_name() const {
    return "area";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string AreaContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void AreaContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        list.push_back(vector<Area*>());
        for(auto &a : manifests[t]) {
            load_area_into_vector(&a.second, (AREA_TYPE) t, false);
        }
    }
}


/**
 * @brief Loads an area.
 *
 * @param area_ptr Object to load into.
 * @param requested_area_path Path to the area's folder.
 * @param manif_ptr Set the manifest pointer to this. If nullptr, it'll be
 * set from the list of manifests.
 * @param level Level to load at.
 * @param from_backup If true, load from a backup, if any.
 * @return Whether it succeeded.
 */
bool AreaContentManager::load_area(
    Area* area_ptr, const string &requested_area_path,
    ContentManifest* manif_ptr, CONTENT_LOAD_LEVEL level, bool from_backup
) {
    //Setup.
    ContentManifest temp_manif;
    AREA_TYPE requested_area_type;
    path_to_manifest(requested_area_path, &temp_manif, &requested_area_type);
    string user_data_path =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        temp_manif.pack + "/" +
        (
            requested_area_type == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        ) + "/" +
        temp_manif.internal_name;
    string base_folder_path = from_backup ? user_data_path : temp_manif.path;
    
    string data_file_path = base_folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
    DataNode data_file = load_data_file(data_file_path);
    if(!data_file.fileWasOpened) return false;
    
    string geometry_file_path = base_folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
    DataNode geometry_file = load_data_file(geometry_file_path);
    if(!geometry_file.fileWasOpened) return false;
    
    area_ptr->type = requested_area_type;
    area_ptr->user_data_path = user_data_path;
    
    if(manif_ptr) {
        area_ptr->manifest = manif_ptr;
    } else {
        area_ptr->manifest =
            find_manifest(
                temp_manif.internal_name, temp_manif.pack, requested_area_type
            );
    }
    
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
    string thumbnail_path = base_folder_path + "/" + FILE_NAMES::AREA_THUMBNAIL;
    area_ptr->load_thumbnail(thumbnail_path);
    
    //Geometry.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        area_ptr->load_geometry_from_data_node(&geometry_file, level);
    }
    
    return true;
}


/**
 * @brief Loads an area into the vector of areas. This does not load it as the
 * "current" area.
 *
 * @param manifest Manifest of the area.
 * @param type Type of area this is.
 * @param from_backup If true, load from a backup, if any.
 */
void AreaContentManager::load_area_into_vector(
    ContentManifest* manifest, AREA_TYPE type,
    bool from_backup
) {
    Area* new_area = new Area();
    list[type].push_back(new_area);
    load_area(
        new_area, manifest->path, manifest,
        CONTENT_LOAD_LEVEL_BASIC, from_backup
    );
}


/**
 * @brief Returns the path to an area given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the area.
 * @param type Type of area.
 * @return The path.
 */
string AreaContentManager::manifest_to_path(
    const ContentManifest &manifest, AREA_TYPE type
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        (
            type == AREA_TYPE_SIMPLE ?
            FOLDER_PATHS_FROM_PACK::SIMPLE_AREAS :
            FOLDER_PATHS_FROM_PACK::MISSION_AREAS
        ) + "/" +
        manifest.internal_name;
}


/**
 * @brief Returns the manifest of an area given its path.
 *
 * @param path Path to the area.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the area type is returned here.
 */
void AreaContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest, AREA_TYPE* out_type
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_type) {
        if(path.find("/" + FOLDER_NAMES::MISSION_AREAS + "/") != string::npos) {
            *out_type = AREA_TYPE_MISSION;
        } else {
            *out_type = AREA_TYPE_SIMPLE;
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void AreaContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < list.size(); t++) {
        for(size_t a = 0; a < list[t].size(); a++) {
            delete list[t][a];
        }
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void BitmapContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void BitmapContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::GRAPHICS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string BitmapContentManager::get_name() const {
    return "bitmap";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string BitmapContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void BitmapContentManager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a bitmap given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the bitmap.
 * @param extension Extension of the bitmap file, dot included.
 * @return The path.
 */
string BitmapContentManager::manifest_to_path(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GRAPHICS + "/" +
        manifest.internal_name + extension;
}


/**
 * @brief Returns the manifest of a bitmap given its path.
 *
 * @param path Path to the bitmap.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void BitmapContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_extension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *out_extension = "";
        } else {
            *out_extension = path.substr(i);
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void BitmapContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Fills in a given manifests map.
 *
 * @param manifests Manifests map to fill.
 * @param content_rel_path Path to the content, relative to the start
 * of the pack.
 * @param folders True if the content is folders, false if it's files.
 */
void ContentTypeManager::fill_manifests_map(
    map<string, ContentManifest> &manifests, const string &content_rel_path, bool folders
) {
    for(const auto &p : game.content.packs.manifests_with_base) {
        fill_manifests_map_from_pack(manifests, p, content_rel_path, folders);
    }
}


/**
 * @brief Fills in a given manifests map from within a pack folder.
 *
 * @param manifests Manifests map to fill.
 * @param pack_name Name of the pack folder.
 * @param content_rel_path Path to the content, relative to the start
 * of the pack.
 * @param folders True if the content is folders, false if it's files.
 */
void ContentTypeManager::fill_manifests_map_from_pack(
    map<string, ContentManifest> &manifests, const string &pack_name,
    const string &content_rel_path, bool folders
) {
    const string folder_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + pack_name +
        "/" + content_rel_path;
        
    vector<string> items =
        folder_to_vector_recursively(folder_path, folders);
        
    for(size_t i = 0; i < items.size(); i++) {
        string internal_name = remove_extension(items[i]);
        manifests[internal_name] = ContentManifest(internal_name, folder_path + "/" + items[i], pack_name);
    }
}


/**
 * @brief Clears the manifests.
 */
void GlobalAnimContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void GlobalAnimContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::GLOBAL_ANIMATIONS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string GlobalAnimContentManager::get_name() const {
    return "global animation database";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string GlobalAnimContentManager::get_perf_mon_measurement_name() const {
    return "Global animation databases";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void GlobalAnimContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &a : manifests) {
        load_animation_db(&a.second, level);
    }
}


/**
 * @brief Loads a global animation database.
 *
 * @param manifest Manifest of the animation database.
 * @param level Level to load at.
 */
void GlobalAnimContentManager::load_animation_db(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file(manifest->path);
    AnimationDatabase db;
    db.manifest = manifest;
    db.load_from_data_node(&file);
    list[manifest->internal_name] = db;
}


/**
 * @brief Returns the path to a global animation database given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the animation database.
 * @return The path.
 */
string GlobalAnimContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GLOBAL_ANIMATIONS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a global animation database given its path.
 *
 * @param path Path to the animation database.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void GlobalAnimContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void GlobalAnimContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto &a : list) {
        a.second.destroy();
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void GuiContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void GuiContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::GUI, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string GuiContentManager::get_name() const {
    return "GUI definition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string GuiContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void GuiContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(const auto &g : manifests) {
        list[g.first] = load_data_file(g.second.path);
    }
}


/**
 * @brief Returns the path to a GUI definition given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the definition.
 * @return The path.
 */
string GuiContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GUI + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a GUI definition given its path.
 *
 * @param path Path to the definition.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void GuiContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void GuiContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void HazardContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void HazardContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::HAZARDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string HazardContentManager::get_name() const {
    return "hazard";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string HazardContentManager::get_perf_mon_measurement_name() const {
    return "Hazards";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void HazardContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &h : manifests) {
        load_hazard(&h.second, level);
    }
}


/**
 * @brief Loads a hazard.
 *
 * @param manifest Manifest of the hazard.
 * @param level Level to load at.
 */
void HazardContentManager::load_hazard(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    Hazard new_h;
    new_h.manifest = manifest;
    new_h.load_from_data_node(&file);
    list[manifest->internal_name] = new_h;
}


/**
 * @brief Returns the path to a hazard given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the hazard.
 * @return The path.
 */
string HazardContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::HAZARDS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a hazard given its path.
 *
 * @param path Path to the hazard.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void HazardContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void HazardContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void LiquidContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void LiquidContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::LIQUIDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string LiquidContentManager::get_name() const {
    return "liquid";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string LiquidContentManager::get_perf_mon_measurement_name() const {
    return "Liquids";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void LiquidContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &l : manifests) {
        load_liquid(&l.second, level);
    }
}


/**
 * @brief Loads a liquid.
 *
 * @param manifest Manifest of the liquid.
 * @param level Level to load at.
 */
void LiquidContentManager::load_liquid(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    Liquid* new_l = new Liquid();
    new_l->manifest = manifest;
    new_l->load_from_data_node(&file, level);
    list[manifest->internal_name] = new_l;
}


/**
 * @brief Returns the path to a liquid given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the liquid.
 * @return The path.
 */
string LiquidContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::LIQUIDS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a liquid given its path.
 *
 * @param path Path to the liquid.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void LiquidContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void LiquidContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(const auto &l : list) {
        delete l.second;
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void MiscConfigContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MiscConfigContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::MISC, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string MiscConfigContentManager::get_name() const {
    return "misc. config";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MiscConfigContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MiscConfigContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    //Game config.
    string config_file_internal_name =
        remove_extension(FILE_NAMES::GAME_CONFIG);
    DataNode game_config_file =
        load_data_file(manifests[config_file_internal_name].path);
    game.config.load(&game_config_file);
    
    al_set_window_title(
        game.display,
        game.config.name.empty() ? "Pikifen" : game.config.name.c_str()
    );
    
    //System content names.
    string scn_file_internal_name =
        remove_extension(FILE_NAMES::SYSTEM_CONTENT_NAMES);
    DataNode scn_file =
        load_data_file(manifests[scn_file_internal_name].path);
    game.sys_content_names.load(&scn_file);
}


/**
 * @brief Returns the path to a misc. config given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the config.
 * @return The path.
 */
string MiscConfigContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::MISC + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a misc. config given its path.
 *
 * @param path Path to the config.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void MiscConfigContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void MiscConfigContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void MobAnimContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MobAnimContentManager::fill_manifests() {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.push_back(map<string, ContentManifest>());
        if(c == MOB_CATEGORY_NONE) continue;
        MobCategory* category = game.mob_categories.get((MOB_CATEGORY) c);
        if(category->folder_name.empty()) return;
        
        for(const auto &p : game.content.packs.manifests_with_base) {
            fill_cat_manifests_from_pack(category, p);
        }
    }
}


/**
 * @brief Fills in the manifests from a specific pack.
 */
void MobAnimContentManager::fill_cat_manifests_from_pack(
    MobCategory* category, const string &pack_name
) {
    const string category_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        pack_name + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category->folder_name;
    vector<string> type_folders = folder_to_vector_recursively(category_path, true);
    for(size_t f = 0; f < type_folders.size(); f++) {
        string internal_name = type_folders[f];
        manifests[category->id][internal_name] =
            ContentManifest(
                internal_name,
                category_path + "/" +
                internal_name + "/" +
                FILE_NAMES::MOB_TYPE_ANIMATION,
                FOLDER_NAMES::BASE_PACK
            );
    }
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string MobAnimContentManager::get_name() const {
    return "mob animation database";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MobAnimContentManager::get_perf_mon_measurement_name() const {
    return "Object animation databases";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MobAnimContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        list.push_back(map<string, AnimationDatabase>());
        for(auto &a : manifests[c]) {
            load_animation_db(&a.second, level, (MOB_CATEGORY) c);
        }
    }
}


/**
 * @brief Loads a mob animation database.
 *
 * @param manifest Manifest of the animation database.
 * @param level Level to load at.
 * @param category_id Mob category ID.
 */
void MobAnimContentManager::load_animation_db(ContentManifest* manifest, CONTENT_LOAD_LEVEL level, MOB_CATEGORY category_id) {
    DataNode file(manifest->path);
    AnimationDatabase db;
    db.manifest = manifest;
    db.load_from_data_node(&file);
    list[category_id][manifest->internal_name] = db;
}


/**
 * @brief Returns the path to a mob animation database a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the animation database.
 * @param category Mob category folder name.
 * @param type Mob type folder name.
 * @return The path.
 */
string MobAnimContentManager::manifest_to_path(
    const ContentManifest &manifest, const string &category,
    const string &type
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category + "/" +
        type + "/" +
        FILE_NAMES::MOB_TYPE_ANIMATION;
}


/**
 * @brief Returns the manifest of a mob animation database given its path.
 *
 * @param path Path to the animation database.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_category If not nullptr, the mob category folder name
 * is returned here.
 * @param out_type If not nullptr, the mob type folder name
 * is returned here.
 */
void MobAnimContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest,
    string* out_category, string* out_type
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_category || out_type) {
        vector<string> parts = split(path, "/");
        if(out_category) {
            *out_category = "";
            if(parts.size() >= 3) {
                *out_category = parts[parts.size() - 3];
            }
        }
        if(out_type) {
            *out_type = "";
            if(parts.size() >= 2) {
                *out_type = parts[parts.size() - 2];
            }
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void MobAnimContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < list.size(); t++) {
        for(auto &a : list[t]) {
            a.second.destroy();
        }
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void MobTypeContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MobTypeContentManager::fill_manifests() {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.push_back(map<string, ContentManifest>());
        if(c == MOB_CATEGORY_NONE) continue;
        fill_manifests_map(
            manifests[c],
            FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
            game.mob_categories.get((MOB_CATEGORY) c)->folder_name,
            true
        );
    }
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string MobTypeContentManager::get_name() const {
    return "mob type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MobTypeContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MobTypeContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) {
            continue;
        }
        
        MobCategory* category = game.mob_categories.get((MOB_CATEGORY) c);
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
    for(auto &p : list.pikmin) {
        if(
            std::find(
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
        if(list.pikmin.find(s) != list.pikmin.end()) {
            game.config.pikmin_order.push_back(list.pikmin[s]);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    vector<string> missing_leader_order_types;
    for(auto &l : list.leader) {
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
        if(list.leader.find(s) != list.leader.end()) {
            game.config.leader_order.push_back(list.leader[s]);
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
void MobTypeContentManager::load_mob_types_of_category(MobCategory* category, CONTENT_LOAD_LEVEL level) {
    if(category->folder_name.empty()) return;
    
    map<string, ContentManifest> &man = manifests[category->id];
    for(auto &t : man) {
        DataNode file(t.second.path + "/data.txt");
        if(!file.fileWasOpened) continue;
        
        MobType* mt;
        mt = category->create_type();
        mt->manifest = &t.second;
        mt->load_from_data_node(&file, level, t.second.path);
        category->register_type(t.first, mt);
        
    }
}


/**
 * @brief Returns the path to a mob type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the mob type.
 * @param category Mob category folder name.
 * @return The path.
 */
string MobTypeContentManager::manifest_to_path(
    const ContentManifest &manifest, const string &category
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category + "/" +
        manifest.internal_name;
}


/**
 * @brief Returns the manifest of a mob type given its path.
 *
 * @param path Path to the mob type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_category If not nullptr, the mob category folder name
 * is returned here.
 */
void MobTypeContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest, string* out_category
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_category) {
        vector<string> parts = split(path, "/");
        *out_category = "";
        if(parts.size() >= 2) {
            *out_category = parts[parts.size() - 2];
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void MobTypeContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    game.config.leader_order.clear();
    game.config.pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        MobCategory* category = game.mob_categories.get((MOB_CATEGORY) c);
        unload_mob_types_of_category(category, level);
    }
}


/**
 * @brief Unloads a type of mob.
 *
 * @param mt Mob type to unload.
 * @param level Should match the level at which the content got loaded.
 */
void MobTypeContentManager::unload_mob_type(MobType* mt, CONTENT_LOAD_LEVEL level) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* s_ptr = mt->sounds[s].sample;
        if(!s) continue;
        game.content.sounds.list.free(s_ptr);
    }
    unload_script(mt);
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        mt->unload_resources();
    }
}


/**
 * @brief Unloads all loaded types of mob from a category.
 *
 * @param category Pointer to the mob category.
 * @param level Should match the level at which the content got loaded.
 */
void MobTypeContentManager::unload_mob_types_of_category(MobCategory* category, CONTENT_LOAD_LEVEL level) {

    vector<string> type_names;
    category->get_type_names(type_names);
    
    for(size_t t = 0; t < type_names.size(); t++) {
        MobType* mt = category->get_type(type_names[t]);
        unload_mob_type(mt, level);
    }
    
    category->clear_types();
}


/**
 * @brief Clears the manifests.
 */
void ParticleGenContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void ParticleGenContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::PARTICLE_GENERATORS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string ParticleGenContentManager::get_name() const {
    return "particle generator";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string ParticleGenContentManager::get_perf_mon_measurement_name() const {
    return "Particle generators";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void ParticleGenContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &g : manifests) {
        load_generator(&g.second, level);
    }
}


/**
 * @brief Loads a user-made particle generator.
 *
 * @param manifest Manifest of the particle generator.
 * @param level Level to load at.
 */
void ParticleGenContentManager::load_generator(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    ParticleGenerator new_pg;
    new_pg.manifest = manifest;
    new_pg.load_from_data_node(&file, level);
    list[manifest->internal_name] = new_pg;
}


/**
 * @brief Returns the path to a particle generator given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the generator.
 * @return The path.
 */
string ParticleGenContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::PARTICLE_GENERATORS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a particle generator given its path.
 *
 * @param path Path to the generator.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void ParticleGenContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void ParticleGenContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto g = list.begin(); g != list.end(); ++g) {
        game.content.bitmaps.list.free(g->second.base_particle.bitmap);
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SoundContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SoundContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::SOUNDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SoundContentManager::get_name() const {
    return "audio sample";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SoundContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SoundContentManager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a sample given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the sample.
 * @param extension Extension of the sample file, dot included.
 * @return The path.
 */
string SoundContentManager::manifest_to_path(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SOUNDS + "/" +
        manifest.internal_name + extension;
}


/**
 * @brief Returns the manifest of a sample given its path.
 *
 * @param path Path to the sample.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void SoundContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_extension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *out_extension = "";
        } else {
            *out_extension = path.substr(i);
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SoundContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void SongContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SongContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::SONGS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SongContentManager::get_name() const {
    return "song";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SongContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SongContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_song(&s.second, level);
    }
}


/**
 * @brief Loads a song.
 *
 * @param manifest Manifest of the song.
 * @param level Level to load at.
 */
void SongContentManager::load_song(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    Song new_song;
    new_song.manifest = manifest;
    new_song.load_from_data_node(&file);
    list[manifest->internal_name] = new_song;
}


/**
 * @brief Returns the path to a song given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the song.
 * @return The path.
 */
string SongContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SONGS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a song given its path.
 *
 * @param path Path to the song.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SongContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SongContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : list) {
        s.second.unload();
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SongTrackContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SongTrackContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::SONG_TRACKS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SongTrackContentManager::get_name() const {
    return "song track";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SongTrackContentManager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SongTrackContentManager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a song track given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the song track.
 * @param extension Extension of the song track file, dot included.
 * @return The path.
 */
string SongTrackContentManager::manifest_to_path(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SONG_TRACKS + "/" +
        manifest.internal_name + extension;
}


/**
 * @brief Returns the manifest of a song track given its path.
 *
 * @param path Path to the song track.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void SongTrackContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
    
    if(out_extension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *out_extension = "";
        } else {
            *out_extension = path.substr(i);
        }
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SongTrackContentManager::unload_all(CONTENT_LOAD_LEVEL level) {

}


/**
 * @brief Clears the manifests.
 */
void SpikeDamageTypeContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SpikeDamageTypeContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::SPIKE_DAMAGES_TYPES, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SpikeDamageTypeContentManager::get_name() const {
    return "spike damage type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SpikeDamageTypeContentManager::get_perf_mon_measurement_name() const {
    return "Spike damage types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SpikeDamageTypeContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_spike_damage_type(&s.second, level);
    }
}


/**
 * @brief Loads a spike damage type.
 *
 * @param manifest Manifest of the spike damage type.
 * @param level Level to load at.
 */
void SpikeDamageTypeContentManager::load_spike_damage_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    SpikeDamageType new_t;
    new_t.manifest = manifest;
    new_t.load_from_data_node(&file);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Returns the path to a spike damage type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spike damage type.
 * @return The path.
 */
string SpikeDamageTypeContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SPIKE_DAMAGES_TYPES + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a spike damage type given its path.
 *
 * @param path Path to the spike damage type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SpikeDamageTypeContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SpikeDamageTypeContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SprayTypeContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SprayTypeContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::SPRAYS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SprayTypeContentManager::get_name() const {
    return "spray type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SprayTypeContentManager::get_perf_mon_measurement_name() const {
    return "Spray types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SprayTypeContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_spray_type(&s.second, level);
    }
    
    //Spray type order.
    vector<string> missing_spray_order_types;
    for(auto &s : list) {
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
        if(list.find(s) != list.end()) {
            game.config.spray_order.push_back(&list[s]);
        } else {
            game.errors.report(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the config file!"
            );
        }
    }
}


/**
 * @brief Loads a spray type.
 *
 * @param manifest Manifest of the spray type.
 * @param level Level to load at.
 */
void SprayTypeContentManager::load_spray_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    SprayType new_t;
    new_t.manifest = manifest;
    new_t.load_from_data_node(&file, level);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Returns the path to a spray type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spray type.
 * @return The path.
 */
string SprayTypeContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SPRAYS + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a spray type given its path.
 *
 * @param path Path to the spray type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SprayTypeContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SprayTypeContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(const auto &s : list) {
        game.content.bitmaps.list.free(s.second.bmp_spray);
    }
    game.config.spray_order.clear();
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void StatusTypeContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void StatusTypeContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::STATUSES, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string StatusTypeContentManager::get_name() const {
    return "status type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string StatusTypeContentManager::get_perf_mon_measurement_name() const {
    return "Status types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void StatusTypeContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    vector<StatusType*> types_with_replacements;
    vector<string> types_with_replacements_names;
    
    for(auto &s : manifests) {
        load_status_type(&s.second, level);
    }
    
    for(auto &s : list) {
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
        for(auto &s2 : list) {
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
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the status type.
 * @param level Level to load at.
 */
void StatusTypeContentManager::load_status_type(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    StatusType* new_t = new StatusType();
    new_t->manifest = manifest;
    new_t->load_from_data_node(&file, level);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Returns the path to a status type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the status type.
 * @return The path.
 */
string StatusTypeContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::STATUSES + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a status type given its path.
 *
 * @param path Path to the status type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void StatusTypeContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void StatusTypeContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto s : list) {
        delete s.second;
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void WeatherConditionContentManager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void WeatherConditionContentManager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PACK::WEATHER, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string WeatherConditionContentManager::get_name() const {
    return "weather condition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string WeatherConditionContentManager::get_perf_mon_measurement_name() const {
    return "Weather conditions";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void WeatherConditionContentManager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &w : manifests) {
        load_weather_condition(&w.second, level);
    }
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the weather condition.
 * @param level Level to load at.
 */
void WeatherConditionContentManager::load_weather_condition(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = load_data_file(manifest->path);
    if(!file.fileWasOpened) return;
    
    Weather new_w;
    new_w.manifest = manifest;
    new_w.load_from_data_node(&file);
    list[manifest->internal_name] = new_w;
}


/**
 * @brief Returns the path to a weather condition given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the weather condition.
 * @return The path.
 */
string WeatherConditionContentManager::manifest_to_path(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::WEATHER + "/" +
        manifest.internal_name + ".txt";
}


/**
 * @brief Returns the manifest of a weather condition given its path.
 *
 * @param path Path to the weather condition.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void WeatherConditionContentManager::path_to_manifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fill_from_path(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void WeatherConditionContentManager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}
