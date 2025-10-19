/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Class representing a piece of game content.
 */

#include "content.h"

#include "../core/misc_structs.h"
#include "../util/string_utils.h"


/**
 * @brief Loads content metadata from a data node.
 *
 * @param node Data node to load from.
 */
void Content::loadMetadataFromDataNode(DataNode* node) {
    if(manifest) name = manifest->internalName;
    
    ReaderSetter mRS(node);
    
    mRS.set("name", name);
    mRS.set("description", description);
    mRS.set("tags", tags);
    mRS.set("maker", maker);
    mRS.set("version", version);
    mRS.set("engine_version", engineVersion);
    mRS.set("maker_notes", makerNotes);
    mRS.set("notes", notes);
}


/**
 * @brief Resets the metadata.
 */
void Content::resetMetadata() {
    name.clear();
    description.clear();
    tags.clear();
    maker.clear();
    version.clear();
    engineVersion.clear();
    makerNotes.clear();
    notes.clear();
}


/**
 * @brief Saves content metadata to a data node.
 *
 * @param node Data node to save to.
 */
void Content::saveMetadataToDataNode(DataNode* node) const {
    const auto doProp =
    [node] (const string& name, const string& value) {
        DataNode* propNode = nullptr;
        if(node->getNrOfChildrenByName(name) > 0) {
            propNode = node->getChildByName(name);
        }
        if(propNode) {
            if(value.empty()) {
                node->remove(propNode);
            } else {
                propNode->value = value;
            }
        } else {
            if(!value.empty()) {
                node->addNew(name, value);
            }
        }
    };
    
    doProp("name", name);
    doProp("description", description);
    doProp("tags", tags);
    doProp("maker", maker);
    doProp("version", version);
    doProp("engine_version", engineVersion);
    doProp("maker_notes", makerNotes);
    doProp("notes", notes);
}


/**
 * @brief Constructs a new content manifest object.
 */
ContentManifest::ContentManifest() {}


/**
 * @brief Constructs a new content manifest object.
 *
 * @param name Internal name. Basically file name sans extension or folder name.
 * @param path Path to the content, relative to the packs folder.
 * @param pack Pack it belongs to.
 */
ContentManifest::ContentManifest(
    const string& name, const string& path, const string& pack
) :
    internalName(name),
    path(path),
    pack(pack) {
    
}


/**
 * @brief Clears all the information in a manifest.
 */
void ContentManifest::clear() {
    internalName.clear();
    path.clear();
    pack.clear();
}


/**
 * @brief Fills in the information using the provided path. It'll all be empty
 * if the path is not valid.
 *
 * @param path Path to fill from.
 */
void ContentManifest::fillFromPath(const string& path) {
    clear();
    
    vector<string> parts = split(path, "/");
    size_t gameDataIdx = string::npos;
    for(size_t p = 0; p < parts.size(); p++) {
        if(parts[p] == FOLDER_NAMES::GAME_DATA) {
            gameDataIdx = p;
            break;
        }
    }
    if(gameDataIdx == string::npos) return;
    if((int) gameDataIdx >= (int) parts.size() - 2) return;
    
    this->path = path;
    this->pack = parts[gameDataIdx + 1];
    this->internalName = removeExtension(parts.back());
}
