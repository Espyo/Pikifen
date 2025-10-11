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

#include "../core/game.h"
#include "../core/init.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
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
 * @param areaName Name of the area.
 * @param pack Pack it belongs to.
 * @param type Area type.
 * @return The manifest, or nullptr.
 */
ContentManifest* AreaContentManager::findManifest(
    const string& areaName, const string& pack, AREA_TYPE type
) {
    for(auto& m : manifests[type]) {
        if(m.first == areaName && m.second.pack == pack) {
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
        for(auto& a : manifests[t]) {
            loadAreaIntoVector(&a.second, (AREA_TYPE) t, false);
        }
    }
}


/**
 * @brief Loads an area.
 *
 * @param areaPtr Object to load into.
 * @param requestedAreaPath Path to the area's folder.
 * @param manifPtr Set the manifest pointer to this. If nullptr, it'll be
 * set from the list of manifests.
 * @param level Level to load at.
 * @param fromBackup If true, load from a backup, if any.
 * @return Whether it succeeded.
 */
bool AreaContentManager::loadArea(
    Area* areaPtr, const string& requestedAreaPath,
    ContentManifest* manifPtr, CONTENT_LOAD_LEVEL level, bool fromBackup
) {
    //Setup.
    ContentManifest tempManif;
    AREA_TYPE requestedAreaType;
    pathToManifest(requestedAreaPath, &tempManif, &requestedAreaType);
    string userDataPath =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        tempManif.pack + "/" +
        (
            requestedAreaType == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        ) + "/" +
        tempManif.internalName;
    string baseFolderPath = fromBackup ? userDataPath : tempManif.path;
    
    string dataFilePath = baseFolderPath + "/" + FILE_NAMES::AREA_MAIN_DATA;
    DataNode dataFile = loadDataFile(dataFilePath);
    if(!dataFile.fileWasOpened) return false;
    
    string geometryFilePath = baseFolderPath + "/" + FILE_NAMES::AREA_GEOMETRY;
    DataNode geometryFile = loadDataFile(geometryFilePath);
    if(!geometryFile.fileWasOpened) return false;
    
    areaPtr->type = requestedAreaType;
    areaPtr->userDataPath = userDataPath;
    
    if(manifPtr) {
        areaPtr->manifest = manifPtr;
    } else {
        areaPtr->manifest =
            findManifest(
                tempManif.internalName, tempManif.pack, requestedAreaType
            );
    }
    
    //Main data.
    if(game.perfMon) game.perfMon->startMeasurement("Area -- Data");
    areaPtr->loadMainDataFromDataNode(&dataFile, level);
    areaPtr->loadMissionDataFromDataNode(&dataFile);
    if(game.perfMon) game.perfMon->finishMeasurement();
    
    //Loading screen.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.loadingTextBmp) al_destroy_bitmap(game.loadingTextBmp);
        if(game.loadingSubtextBmp) al_destroy_bitmap(game.loadingSubtextBmp);
        game.loadingTextBmp = nullptr;
        game.loadingSubtextBmp = nullptr;
        drawLoadingScreen(
            areaPtr->name,
            getSubtitleOrMissionGoal(
                areaPtr->subtitle,
                areaPtr->type,
                areaPtr->mission.goal
            ),
            areaPtr->maker,
            1.0f
        );
        al_flip_display();
    }
    
    //Thumbnail image.
    string thumbnailPath = baseFolderPath + "/" + FILE_NAMES::AREA_THUMBNAIL;
    areaPtr->loadThumbnail(thumbnailPath);
    
    //Geometry.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        areaPtr->loadGeometryFromDataNode(&geometryFile, level);
    }
    
    return true;
}


/**
 * @brief Loads an area into the vector of areas. This does not load it as the
 * "current" area.
 *
 * @param manifest Manifest of the area.
 * @param type Type of area this is.
 * @param fromBackup If true, load from a backup, if any.
 */
void AreaContentManager::loadAreaIntoVector(
    ContentManifest* manifest, AREA_TYPE type, bool fromBackup
) {
    Area* newArea = new Area();
    if (
        loadArea(
            newArea, manifest->path, manifest,
            CONTENT_LOAD_LEVEL_BASIC, fromBackup
        )
    ) {
        list[type].push_back(newArea);
    } else {
        delete newArea;
    }
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
    const ContentManifest& manifest, AREA_TYPE type
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outType If not nullptr, the area type is returned here.
 */
void AreaContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest, AREA_TYPE* outType
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outType) {
        if(path.find("/" + FOLDER_NAMES::MISSION_AREAS + "/") != string::npos) {
            *outType = AREA_TYPE_MISSION;
        } else {
            *outType = AREA_TYPE_SIMPLE;
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
    const ContentManifest& manifest,
    const string& extension
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outExtension If not nullptr, the file extension is returned here.
 */
void BitmapContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest, string* outExtension
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outExtension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *outExtension = "";
        } else {
            *outExtension = path.substr(i);
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
 * @param contentRelPath Path to the content, relative to the start
 * of the pack.
 * @param folders True if the content is folders, false if it's files.
 */
void ContentTypeManager::fillManifestsMap(
    map<string, ContentManifest>& manifests,
    const string& contentRelPath, bool folders
) {
    for(const auto& p : game.content.packs.manifestsWithBase) {
        fillManifestsMapFromPack(manifests, p, contentRelPath, folders);
    }
}


/**
 * @brief Fills in a given manifests map from within a pack folder.
 *
 * @param manifests Manifests map to fill.
 * @param packName Name of the pack folder.
 * @param contentRelPath Path to the content, relative to the start
 * of the pack.
 * @param folders True if the content is folders, false if it's files.
 */
void ContentTypeManager::fillManifestsMapFromPack(
    map<string, ContentManifest>& manifests, const string& packName,
    const string& contentRelPath, bool folders
) {
    const string folderPath =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + packName +
        "/" + contentRelPath;
        
    vector<string> items =
        folderToVectorRecursively(folderPath, folders);
        
    for(size_t i = 0; i < items.size(); i++) {
        string internalName = removeExtension(items[i]);
        manifests[internalName] =
            ContentManifest(
                internalName, folderPath + "/" + items[i], packName
            );
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
    fillManifestsMap(
        manifests, FOLDER_PATHS_FROM_PACK::GLOBAL_ANIMATIONS, false
    );
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
    for(auto& a : manifests) {
        loadAnimationDb(&a.second, level);
    }
}


/**
 * @brief Loads a global animation database.
 *
 * @param manifest Manifest of the animation database.
 * @param level Level to load at.
 */
void GlobalAnimContentManager::loadAnimationDb(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
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
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void GlobalAnimContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void GlobalAnimContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto& a : list) {
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
    for(const auto& g : manifests) {
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
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void GuiContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
    for(auto& h : manifests) {
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
    
    Hazard newH;
    newH.manifest = manifest;
    newH.loadFromDataNode(&file);
    list[manifest->internalName] = newH;
}


/**
 * @brief Returns the path to a hazard given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the hazard.
 * @return The path.
 */
string HazardContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void HazardContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
    for(auto& l : manifests) {
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
    
    Liquid* newL = new Liquid();
    newL->manifest = manifest;
    newL->loadFromDataNode(&file, level);
    list[manifest->internalName] = newL;
}


/**
 * @brief Returns the path to a liquid given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the liquid.
 * @return The path.
 */
string LiquidContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void LiquidContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void LiquidContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(const auto& l : list) {
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
    string configFileInternalName =
        removeExtension(FILE_NAMES::GAME_CONFIG);
    DataNode gameConfigFile =
        loadDataFile(manifests[configFileInternalName].path);
    game.config.load(&gameConfigFile);
    
    al_set_window_title(
        game.display,
        game.config.general.name.empty() ?
        "Pikifen" :
        game.config.general.name.c_str()
    );
    
    //System content names.
    string scnFileInternalName =
        removeExtension(FILE_NAMES::SYSTEM_CONTENT_NAMES);
    DataNode scnFile =
        loadDataFile(manifests[scnFileInternalName].path);
    game.sysContentNames.load(&scnFile);
}


/**
 * @brief Returns the path to a misc. config given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the config.
 * @return The path.
 */
string MiscConfigContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void MiscConfigContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
 * @brief Fills in the manifests from a specific pack.
 *
 * @param category Mob category this applies to.
 * @param packName Internal name of the pack.
 */
void MobAnimContentManager::fillCatManifestsFromPack(
    MobCategory* category, const string& packName
) {
    const string categoryPath =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        packName + "/" +
        FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/" +
        category->folderName;
    vector<string> typeFolders = folderToVectorRecursively(categoryPath, true);
    for(size_t f = 0; f < typeFolders.size(); f++) {
        string internalName = typeFolders[f];
        manifests[category->id][internalName] =
            ContentManifest(
                internalName,
                categoryPath + "/" +
                internalName + "/" +
                FILE_NAMES::MOB_TYPE_ANIMATION,
                packName
            );
    }
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
        
        for(const auto& p : game.content.packs.manifestsWithBase) {
            fillCatManifestsFromPack(category, p);
        }
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
        for(auto& a : manifests[c]) {
            loadAnimationDb(&a.second, level, (MOB_CATEGORY) c);
        }
    }
}


/**
 * @brief Loads a mob animation database.
 *
 * @param manifest Manifest of the animation database.
 * @param level Level to load at.
 * @param categoryId Mob category ID.
 */
void MobAnimContentManager::loadAnimationDb(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level, MOB_CATEGORY categoryId
) {
    DataNode file(manifest->path);
    AnimationDatabase db;
    db.manifest = manifest;
    db.loadFromDataNode(&file);
    list[categoryId][manifest->internalName] = db;
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
    const ContentManifest& manifest, const string& category,
    const string& type
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outCategory If not nullptr, the mob category folder name
 * is returned here.
 * @param outType If not nullptr, the mob type folder name
 * is returned here.
 */
void MobAnimContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest,
    string* outCategory, string* outType
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outCategory || outType) {
        vector<string> parts = split(path, "/");
        if(outCategory) {
            *outCategory = "";
            if(parts.size() >= 3) {
                *outCategory = parts[parts.size() - 3];
            }
        }
        if(outType) {
            *outType = "";
            if(parts.size() >= 2) {
                *outType = parts[parts.size() - 2];
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
        for(auto& a : list[t]) {
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
    vector<string> missingPikminOrderTypes;
    for(auto& p : list.pikmin) {
        if(!isInContainer(game.config.pikmin.orderStrings, p.first)) {
            //Missing from the list? Add it to the "missing" pile.
            missingPikminOrderTypes.push_back(p.first);
        }
    }
    if(!missingPikminOrderTypes.empty()) {
        std::sort(
            missingPikminOrderTypes.begin(),
            missingPikminOrderTypes.end()
        );
        game.config.pikmin.orderStrings.insert(
            game.config.pikmin.orderStrings.end(),
            missingPikminOrderTypes.begin(),
            missingPikminOrderTypes.end()
        );
    }
    for(size_t o = 0; o < game.config.pikmin.orderStrings.size(); o++) {
        string s = game.config.pikmin.orderStrings[o];
        if(isInMap(list.pikmin, s)) {
            game.config.pikmin.order.push_back(list.pikmin[s]);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the game configuration!"
            );
        }
    }
    
    //Leader type order.
    vector<string> missingLeaderOrderTypes;
    for(auto& l : list.leader) {
        if(!isInContainer(game.config.leaders.orderStrings, l.first)) {
            //Missing from the list? Add it to the "missing" pile.
            missingLeaderOrderTypes.push_back(l.first);
        }
    }
    if(!missingLeaderOrderTypes.empty()) {
        std::sort(
            missingLeaderOrderTypes.begin(),
            missingLeaderOrderTypes.end()
        );
        game.config.leaders.orderStrings.insert(
            game.config.leaders.orderStrings.end(),
            missingLeaderOrderTypes.begin(),
            missingLeaderOrderTypes.end()
        );
    }
    for(size_t o = 0; o < game.config.leaders.orderStrings.size(); o++) {
        string s = game.config.leaders.orderStrings[o];
        if(isInMap(list.leader, s)) {
            game.config.leaders.order.push_back(list.leader[s]);
        } else {
            game.errors.report(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the game configuration!"
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
void MobTypeContentManager::loadMobTypesOfCategory(
    MobCategory* category, CONTENT_LOAD_LEVEL level
) {
    if(category->folderName.empty()) return;
    
    map<string, ContentManifest>& man = manifests[category->id];
    for(auto& t : man) {
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
    const ContentManifest& manifest, const string& category
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outCategory If not nullptr, the mob category folder name
 * is returned here.
 */
void MobTypeContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest, string* outCategory
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outCategory) {
        vector<string> parts = split(path, "/");
        *outCategory = "";
        if(parts.size() >= 2) {
            *outCategory = parts[parts.size() - 2];
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
void MobTypeContentManager::unloadMobType(
    MobType* mt, CONTENT_LOAD_LEVEL level
) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* sPtr = mt->sounds[s].sample;
        if(!s) continue;
        game.content.sounds.list.free(sPtr);
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
void MobTypeContentManager::unloadMobTypesOfCategory(
    MobCategory* category, CONTENT_LOAD_LEVEL level
) {

    vector<string> typeNames;
    category->getTypeNames(typeNames);
    
    for(size_t t = 0; t < typeNames.size(); t++) {
        MobType* mt = category->getType(typeNames[t]);
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
    fillManifestsMap(
        manifests, FOLDER_PATHS_FROM_PACK::PARTICLE_GENERATORS, false
    );
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
    for(auto& g : manifests) {
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
    
    ParticleGenerator newPg;
    newPg.manifest = manifest;
    newPg.loadFromDataNode(&file, level);
    list[manifest->internalName] = newPg;
}


/**
 * @brief Returns the path to a particle generator given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the generator.
 * @return The path.
 */
string ParticleGenContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void ParticleGenContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
    for(auto& s : manifests) {
        loadSong(&s.second, level);
    }
}


/**
 * @brief Loads a song.
 *
 * @param manifest Manifest of the song.
 * @param level Level to load at.
 */
void SongContentManager::loadSong(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Song newSong;
    newSong.manifest = manifest;
    newSong.loadFromDataNode(&file);
    list[manifest->internalName] = newSong;
}


/**
 * @brief Returns the path to a song given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the song.
 * @return The path.
 */
string SongContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void SongContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SongContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(auto& s : list) {
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
    const ContentManifest& manifest,
    const string& extension
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outExtension If not nullptr, the file extension is returned here.
 */
void SongTrackContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest, string* outExtension
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outExtension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *outExtension = "";
        } else {
            *outExtension = path.substr(i);
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
    const ContentManifest& manifest,
    const string& extension
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
 * @param outManifest If not nullptr, the manifest is returned here.
 * @param outExtension If not nullptr, the file extension is returned here.
 */
void SoundContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest, string* outExtension
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
    
    if(outExtension) {
        size_t i = path.find_last_of(".");
        if(i == string::npos) {
            *outExtension = "";
        } else {
            *outExtension = path.substr(i);
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
void SpikeDamageTypeContentManager::clearManifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void SpikeDamageTypeContentManager::fillManifests() {
    fillManifestsMap(
        manifests, FOLDER_PATHS_FROM_PACK::SPIKE_DAMAGES_TYPES, false
    );
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
    for(auto& s : manifests) {
        loadSpikeDamageType(&s.second, level);
    }
}


/**
 * @brief Loads a spike damage type.
 *
 * @param manifest Manifest of the spike damage type.
 * @param level Level to load at.
 */
void SpikeDamageTypeContentManager::loadSpikeDamageType(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    SpikeDamageType newT;
    newT.manifest = manifest;
    newT.loadFromDataNode(&file);
    list[manifest->internalName] = newT;
}


/**
 * @brief Returns the path to a spike damage type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spike damage type.
 * @return The path.
 */
string SpikeDamageTypeContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void SpikeDamageTypeContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
    for(auto& s : manifests) {
        loadSprayType(&s.second, level);
    }
    
    //Spray type order.
    vector<string> missingSprayOrderTypes;
    for(auto& s : list) {
        if(!isInContainer(game.config.misc.sprayOrderStrings, s.first)) {
            //Missing from the list? Add it to the "missing" pile.
            missingSprayOrderTypes.push_back(s.first);
        }
    }
    if(!missingSprayOrderTypes.empty()) {
        std::sort(
            missingSprayOrderTypes.begin(),
            missingSprayOrderTypes.end()
        );
        game.config.misc.sprayOrderStrings.insert(
            game.config.misc.sprayOrderStrings.end(),
            missingSprayOrderTypes.begin(),
            missingSprayOrderTypes.end()
        );
    }
    for(size_t o = 0; o < game.config.misc.sprayOrderStrings.size(); o++) {
        string s = game.config.misc.sprayOrderStrings[o];
        if(isInMap(list, s)) {
            game.config.misc.sprayOrder.push_back(&list[s]);
        } else {
            game.errors.report(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the game configuration!"
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
void SprayTypeContentManager::loadSprayType(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    SprayType newT;
    newT.manifest = manifest;
    newT.loadFromDataNode(&file, level);
    list[manifest->internalName] = newT;
}


/**
 * @brief Returns the path to a spray type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the spray type.
 * @return The path.
 */
string SprayTypeContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void SprayTypeContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
    }
}


/**
 * @brief Unloads all loaded content.
 *
 * @param level Load level. Should match the level used to load the content.
 */
void SprayTypeContentManager::unloadAll(CONTENT_LOAD_LEVEL level) {
    for(const auto& s : list) {
        game.content.bitmaps.list.free(s.second.bmpIcon);
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
    vector<StatusType*> typesWithReplacements;
    vector<string> typesWithReplacementsNames;
    
    for(auto& s : manifests) {
        loadStatusType(&s.second, level);
    }
    
    for(auto& s : list) {
        if(!s.second->replacementOnTimeoutStr.empty()) {
            typesWithReplacements.push_back(s.second);
            typesWithReplacementsNames.push_back(
                s.second->replacementOnTimeoutStr
            );
        }
    }
    
    for(size_t s = 0; s < typesWithReplacements.size(); s++) {
        string rn = typesWithReplacementsNames[s];
        bool found = false;
        for(auto& s2 : list) {
            if(s2.first == rn) {
                typesWithReplacements[s]->replacementOnTimeout =
                    s2.second;
                found = true;
                break;
            }
        }
        if(found) continue;
        
        game.errors.report(
            "The status effect type \"" +
            typesWithReplacements[s]->name +
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
void StatusTypeContentManager::loadStatusType(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    StatusType* newT = new StatusType();
    newT->manifest = manifest;
    newT->loadFromDataNode(&file, level);
    list[manifest->internalName] = newT;
}


/**
 * @brief Returns the path to a status type given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the status type.
 * @return The path.
 */
string StatusTypeContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void StatusTypeContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
    for(auto& w : manifests) {
        loadWeatherCondition(&w.second, level);
    }
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the weather condition.
 * @param level Level to load at.
 */
void WeatherConditionContentManager::loadWeatherCondition(
    ContentManifest* manifest, CONTENT_LOAD_LEVEL level
) {
    DataNode file = loadDataFile(manifest->path);
    if(!file.fileWasOpened) return;
    
    Weather newW;
    newW.manifest = manifest;
    newW.loadFromDataNode(&file);
    list[manifest->internalName] = newW;
}


/**
 * @brief Returns the path to a weather condition given a manifest
 * (that's missing the path).
 *
 * @param manifest Manifest of the weather condition.
 * @return The path.
 */
string WeatherConditionContentManager::manifestToPath(
    const ContentManifest& manifest
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
 * @param outManifest If not nullptr, the manifest is returned here.
 */
void WeatherConditionContentManager::pathToManifest(
    const string& path, ContentManifest* outManifest
) const {
    if(outManifest) {
        outManifest->fillFromPath(path);
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
