/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob category classes and mob category-related functions.
 */

#include <algorithm>

#include "mob_category.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../mob_type/mob_type.h"
#include "../mob/mob.h"


using std::size_t;
using std::string;


/**
 * @brief Clears the list of registered categories, freeing memory.
 */
void CategoryManager::clear() {
    for(size_t c = 0; c < categories.size(); c++) {
        delete categories[c];
    }
    categories.clear();
}


/**
 * @brief Finds a mob type given its name.
 * This finds the first occurence, in case multiple categories have a mob
 * type of that name.
 *
 * @param name Name of the mob type.
 * @return The type, or nullptr on error.
 */
MobType* CategoryManager::findMobType(const string& name) const {
    for(size_t n = 0; n < categories.size(); n++) {
        MobType* t = categories[n]->getType(name);
        if(t) return t;
    }
    return nullptr;
}


/**
 * @brief Returns a category given its ID.
 *
 * @param id ID of the category.
 * @return The category, or nullptr on error.
 */
MobCategory* CategoryManager::get(const MOB_CATEGORY id) const {
    if(id >= categories.size()) return nullptr;
    return categories[id];
}


/**
 * @brief Returns a category given its folder name.
 *
 * @param name Name of the folder.
 * @return The category, or nullptr on error.
 */
MobCategory* CategoryManager::getFromFolderName(
    const string& name
) const {
    for(size_t n = 0; n < categories.size(); n++) {
        if(categories[n]->folderName == name) return categories[n];
    }
    game.errors.report(
        "Mob category with the folder name \"" + name + "\" not found!"
    );
    return nullptr;
}


/**
 * @brief Returns a category given its internal name.
 *
 * @param internalName Internal name of the category.
 * @return The category, or nullptr on error.
 */
MobCategory* CategoryManager::getFromInternalName(
    const string& internalName
) const {
    for(size_t n = 0; n < categories.size(); n++) {
        if(categories[n]->internalName == internalName) return categories[n];
    }
    return nullptr;
}


/**
 * @brief Returns a category given its name.
 *
 * @param name Name of the category.
 * @return The category, or nullptr on error.
 */
MobCategory* CategoryManager::getFromName(const string& name) const {
    for(size_t n = 0; n < categories.size(); n++) {
        if(categories[n]->name == name) return categories[n];
    }
    return nullptr;
}


/**
 * @brief Returns a category given its plural name.
 *
 * @param pname Plural name of the category.
 * @return The category, or nullptr on error.
 */
MobCategory* CategoryManager::getFromPName(const string& pname) const {
    for(size_t n = 0; n < categories.size(); n++) {
        if(categories[n]->pluralName == pname) return categories[n];
    }
    return nullptr;
}


/**
 * @brief Registers a new mob category.
 *
 * @param id ID of the category.
 * @param category Pointer to its data.
 */
void CategoryManager::registerCategory(
    MOB_CATEGORY id, MobCategory* category
) {
    if(id >= categories.size()) {
        categories.insert(
            categories.end(),
            (id + 1) - categories.size(),
            nullptr
        );
    }
    categories[id] = category;
}


/**
 * @brief Constructs a new mob category object.
 *
 * @param id This category's ID.
 * @param internalName Internal name.
 * @param name Standard category name, in singular.
 * @param pluralName Standard category name, in plural.
 * @param folderName Name of the folder where the mob types for this
 * category are.
 * @param editorColor In the area editor, objects of this category get
 * this color.
 */
MobCategory::MobCategory(
    const MOB_CATEGORY id, const string& internalName,
    const string& name, const string& pluralName,
    const string& folderName, const ALLEGRO_COLOR editorColor
) :
    internalName(internalName),
    name(name),
    id(id),
    pluralName(pluralName),
    folderName(folderName),
    editorColor(editorColor) {
    
}


/**
 * @brief Constructs a new none category object.
 *
 */
NoneCategory::NoneCategory() :
    MobCategory(
        MOB_CATEGORY_NONE, "none", "None", "None",
        "", al_map_rgb(255, 0, 0)
    ) {
    
}


/**
 * @brief Clears the list of registered types.
 */
void NoneCategory::clearTypes() { }


/**
 * @brief Creates a mob.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* NoneCategory::createMob(
    const Point& pos, MobType* type, float angle
) { return nullptr; }


/**
 * @brief Creates a new, empty type of mob.
 *
 * @return The type.
 */
MobType* NoneCategory::createType() { return nullptr; }


/**
 * @brief Clears a mob from the list.
 *
 * @param m The mob to erase.
 */
void NoneCategory::eraseMob(Mob* m) { }


/**
 * @brief Returns a type of mob given its internal name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* NoneCategory::getType(const string& internalName) const {
    return nullptr;
}


/**
 * @brief Returns all types of leader by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void NoneCategory::getTypeNames(vector<string>& list) const { }


/**
 * @brief Registers a created type of leader.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void NoneCategory::registerType(const string& internalName, MobType* type) { }
