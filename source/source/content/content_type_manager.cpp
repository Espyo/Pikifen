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
void AreaContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void AreaContentManager::fillManifests() {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        manifests.push_back(map<string, ContentManifest>());
        fillManifestsMap(
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
ContentManifest* AreaContentManager::findManifest(
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
string AreaContentManager::getName() const {
    return "area";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string AreaContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void AreaContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        list.push_back(vector<Area*>());
        for(auto &a : manifests[t]) {
            loadAreaIntoVector(&a.second, (AREA_TYPE) t, false);
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
bool AreaContentManager::loadArea(
    Area* area_ptr, const string &requested_area_path,
    ContentManifest* manif_ptr, CONTENT_LOAD_LEVEL level, bool from_backup
) {
    //Setup.
    ContentManifest temp_manif;
    AREA_TYPE requested_area_type;
    pathToManifest(requested_area_path, &temp_manif, &requested_area_type);
    string user_data_path =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        temp_manif.pack + "/" +
        (
            requested_area_type == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        ) + "/" +
        temp_manif.internalName;
    string base_folder_path = from_backup ? user_data_path : temp_manif.path;
    
    string data_file_path = base_folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
    DataNode data_file = loadDataFile(data_file_path);
    if(!data_file.fileWasOpened) return false;
    
    string geometry_file_path = base_folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
    DataNode geometry_file = loadDataFile(geometry_file_path);
    if(!geometry_file.fileWasOpened) return false;
    
    area_ptr->type = requested_area_type;
    area_ptr->userDataPath = user_data_path;
    
    if(manif_ptr) {
        area_ptr->manifest = manif_ptr;
    } else {
        area_ptr->manifest =
            findManifest(
                temp_manif.internalName, temp_manif.pack, requested_area_type
            );
    }
    
    //Main data.
    if(game.perfMon) game.perfMon->startMeasurement("Area -- Data");
    area_ptr->loadMainDataFromDataNode(&data_file, level);
    area_ptr->loadMissionDataFromDataNode(&data_file);
    if(game.perfMon) game.perfMon->finishMeasurement();
    
    //Loading screen.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.loadingTextBmp) al_destroy_bitmap(game.loadingTextBmp);
        if(game.loadingSubtextBmp) al_destroy_bitmap(game.loadingSubtextBmp);
        game.loadingTextBmp = nullptr;
        game.loadingSubtextBmp = nullptr;
        drawLoadingScreen(
            area_ptr->name,
            getSubtitleOrMissionGoal(
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
    area_ptr->loadThumbnail(thumbnail_path);
    
    //Geometry.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        area_ptr->loadGeometryFromDataNode(&geometry_file, level);
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
void AreaContentManager::loadAreaIntoVector(
    ContentManifest* manifest, AREA_TYPE type,
    bool from_backup
) {
    Area* new_area = new Area();
    list[type].push_back(new_area);
    loadArea(
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
string AreaContentManager::manifestToPath(
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
        manifest.internalName;
}


/**
 * @brief Returns the manifest of an area given its path.
 *
 * @param path Path to the area.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the area type is returned here.
 */
void AreaContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest, AREA_TYPE* out_type
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void AreaContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
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
void BitmapContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void BitmapContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::GRAPHICS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string BitmapContentManager::getName() const {
    return "bitmap";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string BitmapContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void BitmapContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a bitmap given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the bitmap.
 * @param extension Extension of the bitmap file, dot included.
 * @return The path.
 */
string BitmapContentManager::manifestToPath(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GRAPHICS + "/" +
        manifest.internalName + extension;
}


/**
 * @brief Returns the manifest of a bitmap given its path.
 *
 * @param path Path to the bitmap.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void BitmapContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void BitmapContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Fills in a given manifests map.
 *
 * @param manifests Manifests map to fill.
 * @param content_rel_path Path to the content, relative to the start
 * of the pack.
 * @param folders True if the content is folders, false if it's files.
 */
void ContentTypeManager::fillManifestsMap(
    map<string, ContentManifest> &manifests, const string &content_rel_path, bool folders
) {
    for(const auto &p : game.content.packs.manifestsWithBase) {
        fillManifestsMapFromPack(manifests, p, content_rel_path, folders);
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
void ContentTypeManager::fillManifestsMapFromPack(
    map<string, ContentManifest> &manifests, const string &pack_name,
    const string &content_rel_path, bool folders
) {
    const string folder_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + pack_name +
        "/" + content_rel_path;
        
    vector<string> items =
        folderToVectorRecursively(folder_path, folders);
        
    for(size_t i = 0; i < items.size(); i++) {
        string internal_name = removeExtension(items[i]);
        manifests[internal_name] = ContentManifest(internal_name, folder_path + "/" + items[i], pack_name);
    }
}


/**
 * @brief Clears the manifests.
 */
void GlobalAnimContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void GlobalAnimContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::GLOBAL_ANIMATIONS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string GlobalAnimContentManager::getName() const {
    return "global animation database";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string GlobalAnimContentManager::getPerfMonMeasurementName() const {
    return "Global animation databases";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void GlobalAnimContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &a : manifests) {
        loadAnimationDb(&a.second, level);
    }
}


/**
 * @brief Loads a global animation database.
 *
 * @param manifest Manifest of the animation database.
 * @param level Level to load at.
 */
void GlobalAnimContentManager::loadAnimationDb(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file(manifest->path);
    AnimationDatabase db;
    db.manifest = manifest;
    db.loadFromDataNode(&file);
    list[manifest->internalName] = db;
}


/**
 * @brief Returns the path to a global animation database given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the animation database.
 * @return The path.
 */
string GlobalAnimContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GLOBAL_ANIMATIONS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a global animation database given its path.
 *
 * @param path Path to the animation database.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void GlobalAnimContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void GlobalAnimContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &a : list) {
        a.second.destroy();
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void GuiContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void GuiContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::GUI, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string GuiContentManager::getName() const {
    return "GUI definition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string GuiContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void GuiContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(const auto &g : manifests) {
        list[g.first] = loadDataFile(g.second.path);
    }
}


/**
 * @brief Returns the path to a GUI definition given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the definition.
 * @return The path.
 */
string GuiContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::GUI + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a GUI definition given its path.
 *
 * @param path Path to the definition.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void GuiContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void GuiContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void HazardContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void HazardContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::HAZARDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string HazardContentManager::getName() const {
    return "hazard";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string HazardContentManager::getPerfMonMeasurementName() const {
    return "Hazards";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void HazardContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &h : manifests) {
        loadHazard(&h.second, level);
    }
}


/**
 * @brief Loads a hazard.
 *
 * @param manifest Manifest of the hazard.
 * @param level Level to load at.
 */
void HazardContentManager::loadHazard(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Hazard new_h;
    new_h.manifest = manifest;
    new_h.loadFromDataNode(&file);
    list[manifest->internalName] = new_h;
}


/**
 * @brief Returns the path to a hazard given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the hazard.
 * @return The path.
 */
string HazardContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::HAZARDS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a hazard given its path.
 *
 * @param path Path to the hazard.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void HazardContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void HazardContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void LiquidContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void LiquidContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::LIQUIDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string LiquidContentManager::getName() const {
    return "liquid";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string LiquidContentManager::getPerfMonMeasurementName() const {
    return "Liquids";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void LiquidContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &l : manifests) {
        loadLiquid(&l.second, level);
    }
}


/**
 * @brief Loads a liquid.
 *
 * @param manifest Manifest of the liquid.
 * @param level Level to load at.
 */
void LiquidContentManager::loadLiquid(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Liquid* new_l = new Liquid();
    new_l->manifest = manifest;
    new_l->loadFromDataNode(&file, level);
    list[manifest->internalName] = new_l;
}


/**
 * @brief Returns the path to a liquid given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the liquid.
 * @return The path.
 */
string LiquidContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::LIQUIDS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a liquid given its path.
 *
 * @param path Path to the liquid.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void LiquidContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void LiquidContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(const auto &l : list) {
        delete l.second;
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void MiscConfigContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MiscConfigContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::MISC, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string MiscConfigContentManager::getName() const {
    return "misc. config";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MiscConfigContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MiscConfigContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    //Game config.
    string config_file_internal_name =
        removeExtension(FILE_NAMES::GAME_CONFIG);
    DataNode game_config_file =
        loadDataFile(manifests[config_file_internal_name].path);
    game.config.load(&game_config_file);
    
    al_set_window_title(
        game.display,
        game.config.general.name.empty() ?
        "Pikifen" :
        game.config.general.name.c_str()
    );
    
    //System content names.
    string scn_file_internal_name =
        removeExtension(FILE_NAMES::SYSTEM_CONTENT_NAMES);
    DataNode scn_file =
        loadDataFile(manifests[scn_file_internal_name].path);
    game.sysContentNames.load(&scn_file);
}


/**
 * @brief Returns the path to a misc. config given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the config.
 * @return The path.
 */
string MiscConfigContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::MISC + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a misc. config given its path.
 *
 * @param path Path to the config.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void MiscConfigContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void MiscConfigContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void MobAnimContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MobAnimContentManager::fillManifests() {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.push_back(map<string, ContentManifest>());
        if(c == MOB_CATEGORY_NONE) continue;
        MobCategory* category = game.mobCategories.get((MOB_CATEGORY) c);
        if(category->folderName.empty()) return;
        
        for(const auto &p : game.content.packs.manifestsWithBase) {
            fillCatManifestsFromPack(category, p);
        }
    }
}


/**
 * @brief Fills in the manifests from a specific pack.
 */
void MobAnimContentManager::fillCatManifestsFromPack(
    MobCategory* category, const string &pack_name
) {
    const string category_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        pack_name + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category->folderName;
    vector<string> type_folders = folderToVectorRecursively(category_path, true);
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
string MobAnimContentManager::getName() const {
    return "mob animation database";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MobAnimContentManager::getPerfMonMeasurementName() const {
    return "Object animation databases";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MobAnimContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        list.push_back(map<string, AnimationDatabase>());
        for(auto &a : manifests[c]) {
            loadAnimationDb(&a.second, level, (MOB_CATEGORY) c);
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
void MobAnimContentManager::loadAnimationDb(ContentManifest* manifest, CONTENT_LOAD_LEVEL level, MOB_CATEGORY category_id) {
    DataNode file(manifest->path);
    AnimationDatabase db;
    db.manifest = manifest;
    db.loadFromDataNode(&file);
    list[category_id][manifest->internalName] = db;
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
string MobAnimContentManager::manifestToPath(
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
void MobAnimContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest,
    string* out_category, string* out_type
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void MobAnimContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
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
void MobTypeContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void MobTypeContentManager::fillManifests() {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.push_back(map<string, ContentManifest>());
        if(c == MOB_CATEGORY_NONE) continue;
        fillManifestsMap(
            manifests[c],
            FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
            game.mobCategories.get((MOB_CATEGORY) c)->folderName,
            true
        );
    }
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string MobTypeContentManager::getName() const {
    return "mob type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string MobTypeContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void MobTypeContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) {
            continue;
        }
        
        MobCategory* category = game.mobCategories.get((MOB_CATEGORY) c);
        if(game.perfMon) {
            game.perfMon->startMeasurement(
                "Object types -- " + category->name
            );
        }
        
        loadMobTypesOfCategory(category, level);
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
    }
    
    //Pikmin type order.
    vector<string> missing_pikmin_order_types;
    for(auto &p : list.pikmin) {
        if(!isInContainer(game.config.pikmin.orderStrings, p.first)) {
            //Missing from the list? Add it to the "missing" pile.
            missing_pikmin_order_types.push_back(p.first);
        }
    }
    if(!missing_pikmin_order_types.empty()) {
        std::sort(
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
        game.config.pikmin.orderStrings.insert(
            game.config.pikmin.orderStrings.end(),
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.pikmin.orderStrings.size(); o++) {
        string s = game.config.pikmin.orderStrings[o];
        if(list.pikmin.find(s) != list.pikmin.end()) {
            game.config.pikmin.order.push_back(list.pikmin[s]);
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
                game.config.leaders.orderStrings.begin(),
                game.config.leaders.orderStrings.end(),
                l.first
            ) == game.config.leaders.orderStrings.end()
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
        game.config.leaders.orderStrings.insert(
            game.config.leaders.orderStrings.end(),
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.leaders.orderStrings.size(); o++) {
        string s = game.config.leaders.orderStrings[o];
        if(list.leader.find(s) != list.leader.end()) {
            game.config.leaders.order.push_back(list.leader[s]);
        } else {
            game.errors.report(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the config file!"
            );
        }
    }
    
    //Create the special mob types.
    createSpecialMobTypes();
}


/**
 * @brief Loads the mob types from a category's folder.
 *
 * @param category Pointer to the mob category.
 * @param level Level to load at.
 */
void MobTypeContentManager::loadMobTypesOfCategory(MobCategory* category, CONTENT_LOAD_LEVEL level) {
    if(category->folderName.empty()) return;
    
    map<string, ContentManifest> &man = manifests[category->id];
    for(auto &t : man) {
        DataNode file(t.second.path + "/data.txt");
        if(!file.fileWasOpened) continue;
        
        MobType* mt;
        mt = category->createType();
        mt->manifest = &t.second;
        mt->loadFromDataNode(&file, level, t.second.path);
        category->registerType(t.first, mt);
        
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
string MobTypeContentManager::manifestToPath(
    const ContentManifest &manifest, const string &category
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category + "/" +
        manifest.internalName;
}


/**
 * @brief Returns the manifest of a mob type given its path.
 *
 * @param path Path to the mob type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_category If not nullptr, the mob category folder name
 * is returned here.
 */
void MobTypeContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest, string* out_category
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void MobTypeContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    game.config.leaders.order.clear();
    game.config.pikmin.order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        MobCategory* category = game.mobCategories.get((MOB_CATEGORY) c);
        unloadMobTypesOfCategory(category, level);
    }
}


/**
 * @brief Unloads a type of mob.
 *
 * @param mt Mob type to unload.
 * @param level Should match the level at which the content got loaded.
 */
void MobTypeContentManager::unloadMobType(MobType* mt, CONTENT_LOAD_LEVEL level) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* s_ptr = mt->sounds[s].sample;
        if(!s) continue;
        game.content.sounds.list.free(s_ptr);
    }
    unloadScript(mt);
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        mt->unloadResources();
    }
}


/**
 * @brief Unloads all loaded types of mob from a category.
 *
 * @param category Pointer to the mob category.
 * @param level Should match the level at which the content got loaded.
 */
void MobTypeContentManager::unloadMobTypesOfCategory(MobCategory* category, CONTENT_LOAD_LEVEL level) {

    vector<string> type_names;
    category->getTypeNames(type_names);
    
    for(size_t t = 0; t < type_names.size(); t++) {
        MobType* mt = category->getType(type_names[t]);
        unloadMobType(mt, level);
    }
    
    category->clearTypes();
}


/**
 * @brief Clears the manifests.
 */
void ParticleGenContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void ParticleGenContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::PARTICLE_GENERATORS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string ParticleGenContentManager::getName() const {
    return "particle generator";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string ParticleGenContentManager::getPerfMonMeasurementName() const {
    return "Particle generators";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void ParticleGenContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &g : manifests) {
        loadGenerator(&g.second, level);
    }
}


/**
 * @brief Loads a user-made particle generator.
 *
 * @param manifest Manifest of the particle generator.
 * @param level Level to load at.
 */
void ParticleGenContentManager::loadGenerator(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    ParticleGenerator new_pg;
    new_pg.manifest = manifest;
    new_pg.loadFromDataNode(&file, level);
    list[manifest->internalName] = new_pg;
}


/**
 * @brief Returns the path to a particle generator given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the generator.
 * @return The path.
 */
string ParticleGenContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::PARTICLE_GENERATORS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a particle generator given its path.
 *
 * @param path Path to the generator.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void ParticleGenContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void ParticleGenContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto g = list.begin(); g != list.end(); ++g) {
        game.content.bitmaps.list.free(g->second.baseParticle.bitmap);
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SoundContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SoundContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::SOUNDS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SoundContentManager::getName() const {
    return "audio sample";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SoundContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SoundContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a sample given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the sample.
 * @param extension Extension of the sample file, dot included.
 * @return The path.
 */
string SoundContentManager::manifestToPath(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SOUNDS + "/" +
        manifest.internalName + extension;
}


/**
 * @brief Returns the manifest of a sample given its path.
 *
 * @param path Path to the sample.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void SoundContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void SoundContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void SongContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SongContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::SONGS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SongContentManager::getName() const {
    return "song";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SongContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SongContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        loadSong(&s.second, level);
    }
}


/**
 * @brief Loads a song.
 *
 * @param manifest Manifest of the song.
 * @param level Level to load at.
 */
void SongContentManager::loadSong(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Song new_song;
    new_song.manifest = manifest;
    new_song.loadFromDataNode(&file);
    list[manifest->internalName] = new_song;
}


/**
 * @brief Returns the path to a song given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the song.
 * @return The path.
 */
string SongContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SONGS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a song given its path.
 *
 * @param path Path to the song.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SongContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SongContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &s : list) {
        s.second.unload();
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SongTrackContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SongTrackContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::SONG_TRACKS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SongTrackContentManager::getName() const {
    return "song track";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SongTrackContentManager::getPerfMonMeasurementName() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SongTrackContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Returns the path to a song track given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the song track.
 * @param extension Extension of the song track file, dot included.
 * @return The path.
 */
string SongTrackContentManager::manifestToPath(
    const ContentManifest &manifest,
    const string &extension
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SONG_TRACKS + "/" +
        manifest.internalName + extension;
}


/**
 * @brief Returns the manifest of a song track given its path.
 *
 * @param path Path to the song track.
 * @param out_manifest If not nullptr, the manifest is returned here.
 * @param out_type If not nullptr, the file extension is returned here.
 */
void SongTrackContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest, string* out_extension
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
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
void SongTrackContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {

}


/**
 * @brief Clears the manifests.
 */
void SpikeDamageTypeContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SpikeDamageTypeContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::SPIKE_DAMAGES_TYPES, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SpikeDamageTypeContentManager::getName() const {
    return "spike damage type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SpikeDamageTypeContentManager::getPerfMonMeasurementName() const {
    return "Spike damage types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SpikeDamageTypeContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        loadSpikeDamageType(&s.second, level);
    }
}


/**
 * @brief Loads a spike damage type.
 *
 * @param manifest Manifest of the spike damage type.
 * @param level Level to load at.
 */
void SpikeDamageTypeContentManager::loadSpikeDamageType(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    SpikeDamageType new_t;
    new_t.manifest = manifest;
    new_t.loadFromDataNode(&file);
    list[manifest->internalName] = new_t;
}


/**
 * @brief Returns the path to a spike damage type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spike damage type.
 * @return The path.
 */
string SpikeDamageTypeContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SPIKE_DAMAGES_TYPES + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a spike damage type given its path.
 *
 * @param path Path to the spike damage type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SpikeDamageTypeContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SpikeDamageTypeContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void SprayTypeContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SprayTypeContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::SPRAYS, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string SprayTypeContentManager::getName() const {
    return "spray type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string SprayTypeContentManager::getPerfMonMeasurementName() const {
    return "Spray types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void SprayTypeContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        loadSprayType(&s.second, level);
    }
    
    //Spray type order.
    vector<string> missing_spray_order_types;
    for(auto &s : list) {
        if(
            find(
                game.config.misc.sprayOrderStrings.begin(),
                game.config.misc.sprayOrderStrings.end(),
                s.first
            ) == game.config.misc.sprayOrderStrings.end()
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
        game.config.misc.sprayOrderStrings.insert(
            game.config.misc.sprayOrderStrings.end(),
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.misc.sprayOrderStrings.size(); o++) {
        string s = game.config.misc.sprayOrderStrings[o];
        if(list.find(s) != list.end()) {
            game.config.misc.sprayOrder.push_back(&list[s]);
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
void SprayTypeContentManager::loadSprayType(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    SprayType new_t;
    new_t.manifest = manifest;
    new_t.loadFromDataNode(&file, level);
    list[manifest->internalName] = new_t;
}


/**
 * @brief Returns the path to a spray type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spray type.
 * @return The path.
 */
string SprayTypeContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::SPRAYS + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a spray type given its path.
 *
 * @param path Path to the spray type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void SprayTypeContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SprayTypeContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(const auto &s : list) {
        game.content.bitmaps.list.free(s.second.bmpSpray);
    }
    game.config.misc.sprayOrder.clear();
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void StatusTypeContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void StatusTypeContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::STATUSES, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string StatusTypeContentManager::getName() const {
    return "status type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string StatusTypeContentManager::getPerfMonMeasurementName() const {
    return "Status types";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void StatusTypeContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    vector<StatusType*> types_with_replacements;
    vector<string> types_with_replacements_names;
    
    for(auto &s : manifests) {
        loadStatusType(&s.second, level);
    }
    
    for(auto &s : list) {
        if(!s.second->replacementOnTimeoutStr.empty()) {
            types_with_replacements.push_back(s.second);
            types_with_replacements_names.push_back(
                s.second->replacementOnTimeoutStr
            );
        }
    }
    
    for(size_t s = 0; s < types_with_replacements.size(); s++) {
        string rn = types_with_replacements_names[s];
        bool found = false;
        for(auto &s2 : list) {
            if(s2.first == rn) {
                types_with_replacements[s]->replacementOnTimeout =
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
void StatusTypeContentManager::loadStatusType(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    StatusType* new_t = new StatusType();
    new_t->manifest = manifest;
    new_t->loadFromDataNode(&file, level);
    list[manifest->internalName] = new_t;
}


/**
 * @brief Returns the path to a status type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the status type.
 * @return The path.
 */
string StatusTypeContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::STATUSES + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a status type given its path.
 *
 * @param path Path to the status type.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void StatusTypeContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void StatusTypeContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto s : list) {
        delete s.second;
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void WeatherConditionContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void WeatherConditionContentManager::fillManifests() {
    fillManifestsMap(manifests, FOLDER_PATHS_FROM_PACK::WEATHER, false);
}


/**
 * @brief Returns the content type's name.
 *
 * @return The name.
 */
string WeatherConditionContentManager::getName() const {
    return "weather condition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 *
 * @return The name.
 */
string WeatherConditionContentManager::getPerfMonMeasurementName() const {
    return "Weather conditions";
}


/**
 * @brief Loads all content in the manifests.
 *
 * @param level Level to load at.
 */
void WeatherConditionContentManager::loadAll(CONTENT_LOAD_LEVEL level) {
    for(auto &w : manifests) {
        loadWeatherCondition(&w.second, level);
    }
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the weather condition.
 * @param level Level to load at.
 */
void WeatherConditionContentManager::loadWeatherCondition(ContentManifest* manifest, CONTENT_LOAD_LEVEL level) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Weather new_w;
    new_w.manifest = manifest;
    new_w.loadFromDataNode(&file);
    list[manifest->internalName] = new_w;
}


/**
 * @brief Returns the path to a weather condition given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the weather condition.
 * @return The path.
 */
string WeatherConditionContentManager::manifestToPath(
    const ContentManifest &manifest
) const {
    return
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        manifest.pack + "/" +
        FOLDER_PATHS_FROM_PACK::WEATHER + "/" +
        manifest.internalName + ".txt";
}


/**
 * @brief Returns the manifest of a weather condition given its path.
 *
 * @param path Path to the weather condition.
 * @param out_manifest If not nullptr, the manifest is returned here.
 */
void WeatherConditionContentManager::pathToManifest(
    const string &path, ContentManifest* out_manifest
) const {
    if(out_manifest) {
        out_manifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void WeatherConditionContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    list.clear();
}
