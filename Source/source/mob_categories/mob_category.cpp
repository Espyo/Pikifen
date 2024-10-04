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

#include "../functions.h"
#include "../game.h"
#include "../mob_types/mob_type.h"
#include "../mobs/mob.h"


using std::size_t;
using std::string;


/**
 * @brief Constructs a new mob category object.
 *
 * @param id This category's ID.
 * @param name Standard category name, in singular.
 * @param plural_name Standard category name, in plural.
 * @param folder_name Name of the folder where the mob types for this
 * category are.
 * @param editor_color In the area editor, objects of this category get
 * this color.
 */
mob_category::mob_category(
    const MOB_CATEGORY id, const string &name, const string &plural_name,
    const string &folder_name, const ALLEGRO_COLOR editor_color
) :
    name(name),
    id(id),
    plural_name(plural_name),
    folder_path(MOB_TYPES_FOLDER_PATH + "/" + folder_name),
    editor_color(editor_color) {
    
}


/**
 * @brief Clears the list of registered categories, freeing memory.
 */
void mob_category_manager::clear() {
    for(size_t c = 0; c < categories.size(); ++c) {
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
mob_type* mob_category_manager::find_mob_type(const string &name) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        mob_type* t = categories[n]->get_type(name);
        if(t) return t;
    }
    return nullptr;
}


/**
 * @brief Finds a mob type given its folder's name in the game data folder.
 *
 * @param cat Category the mob is in.
 * @param name Name of the folder.
 * @return The type.
 */
mob_type* mob_category_manager::find_mob_type_from_folder_name(
    const mob_category* cat, const string &name
) const {
    vector<string> types;
    cat->get_type_names(types);
    for(size_t t = 0 ; t < types.size(); ++t) {
        mob_type* mt = cat->get_type(types[t]);
        if(mt->folder_name == name) return mt;
    }
    return nullptr;
}


/**
 * @brief Returns a category given its ID.
 *
 * @param id ID of the category.
 * @return The category, or nullptr on error.
 */
mob_category* mob_category_manager::get(const MOB_CATEGORY id) const {
    if(id >= categories.size()) return nullptr;
    return categories[id];
}


/**
 * @brief Returns a category given its folder name.
 *
 * @param name Name of the folder.
 * @return The category, or nullptr on error.
 */
mob_category* mob_category_manager::get_from_folder_name(
    const string &name
) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->folder_path == name) return categories[n];
    }
    game.errors.report(
        "Mob category with the folder name \"" + name + "\" not found!"
    );
    return nullptr;
}


/**
 * @brief Returns a category given its name.
 *
 * @param name Name of the category.
 * @return The category, or nullptr on error.
 */
mob_category* mob_category_manager::get_from_name(const string &name) const {
    for(size_t n = 0; n < categories.size(); ++n) {
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
mob_category* mob_category_manager::get_from_pname(const string &pname) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->plural_name == pname) return categories[n];
    }
    return nullptr;
}


/**
 * @brief Registers a new mob category.
 *
 * @param nr ID of the category.
 * @param category Pointer to its data.
 */
void mob_category_manager::register_category(
    MOB_CATEGORY nr,
    mob_category* category
) {
    if(nr >= categories.size()) {
        categories.insert(
            categories.end(),
            (nr + 1) - categories.size(),
            nullptr
        );
    }
    categories[nr] = category;
}


/**
 * @brief Constructs a new none category object.
 *
 */
none_category::none_category() :
    mob_category(
        MOB_CATEGORY_NONE, "None", "None",
        "", al_map_rgb(255, 0, 0)
    ) {
    
}


/**
 * @brief Clears the list of registered types.
 */
void none_category::clear_types() { }


/**
 * @brief Creates a mob.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* none_category::create_mob(
    const point &pos, mob_type* type, float angle
) { return nullptr; }


/**
 * @brief Creates a new, empty type of mob.
 *
 * @return The type.
 */
mob_type* none_category::create_type() { return nullptr; }


/**
 * @brief Clears a mob from the list.
 *
 * @param m The mob to erase.
 */
void none_category::erase_mob(mob* m) { }


/**
 * @brief Returns a type of mob given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* none_category::get_type(const string &name) const { return nullptr; }


/**
 * @brief Returns all types of leader by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void none_category::get_type_names(vector<string> &list) const { }


/**
 * @brief Registers a created type of leader.
 *
 * @param type Mob type to register.
 */
void none_category::register_type(mob_type* type) { }
