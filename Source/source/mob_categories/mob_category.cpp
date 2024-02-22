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


/* ----------------------------------------------------------------------------
 * Initializes an instance of a mob category.
 * id:
 *   This category's ID.
 * name:
 *   Standard category name, in singular.
 * plural_name:
 *   Standard category name, in plural.
 * folder:
 *   Name of the folder where the mob types for this category are.
 * editor_color:
 *   In the area editor, objects of this category get this color.
 */
mob_category::mob_category(
    const MOB_CATEGORIES id, const string &name, const string &plural_name,
    const string &folder, const ALLEGRO_COLOR editor_color
) :
    name(name),
    id(id),
    plural_name(plural_name),
    folder(MOB_TYPES_FOLDER_PATH + "/" + folder),
    editor_color(editor_color) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered categories, freeing memory.
 */
void mob_category_manager::clear() {
    for(size_t c = 0; c < categories.size(); ++c) {
        delete categories[c];
    }
    categories.clear();
}


/* ----------------------------------------------------------------------------
 * Finds a mob type given its name. This finds the first occurence, in case
 * multiple categories have a mob type of that name.
 * Returns NULL on error.
 * name:
 *   Name of the mob type.
 */
mob_type* mob_category_manager::find_mob_type(const string &name) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        mob_type* t = categories[n]->get_type(name);
        if(t) return t;
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Finds a mob type given its folder's name in the Game data folder.
 * cat:
 *   Category the mob is in.
 * name:
 *   Name of the folder.
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
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a category given its ID.
 * Returns NULL on error.
 * id:
 *   ID of the category.
 */
mob_category* mob_category_manager::get(const MOB_CATEGORIES id) const {
    if(id >= categories.size()) return NULL;
    return categories[id];
}


/* ----------------------------------------------------------------------------
 * Returns a category given its folder name.
 * Returns NULL on error.
 * name:
 *   Name of the folder.
 */
mob_category* mob_category_manager::get_from_folder_name(
    const string &name
) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->folder == name) return categories[n];
    }
    game.errors.report(
        "Mob category with the folder name \"" + name + "\" not found!"
    );
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a category given its name.
 * Returns NULL on error.
 * name:
 *   Name of the category.
 */
mob_category* mob_category_manager::get_from_name(const string &name) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->name == name) return categories[n];
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a category given its plural name.
 * Returns NULL on error.
 * pname:
 *   Plural name of the category.
 */
mob_category* mob_category_manager::get_from_pname(const string &pname) const {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->plural_name == pname) return categories[n];
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Registers a new mob category.
 * nr:
 *   ID of the category.
 * category:
 *   Pointer to its data.
 */
void mob_category_manager::register_category(
    MOB_CATEGORIES nr,
    mob_category* category
) {
    if(nr >= categories.size()) {
        categories.insert(
            categories.end(),
            (nr + 1) - categories.size(),
            NULL
        );
    }
    categories[nr] = category;
}


/* ----------------------------------------------------------------------------
 * Creates a "none" category.
 */
none_category::none_category() :
    mob_category(
        MOB_CATEGORY_NONE, "None", "None",
        "", al_map_rgb(255, 0, 0)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types.
 */
void none_category::clear_types() { }


/* ----------------------------------------------------------------------------
 * Creates a mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* none_category::create_mob(
    const point &pos, mob_type* type, const float angle
) { return NULL; }


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of mob.
 */
mob_type* none_category::create_type() { return NULL; }


/* ----------------------------------------------------------------------------
 * Clears a mob from the list.
 * m:
 *   The mob to erase.
 */
void none_category::erase_mob(mob* m) { }


/* ----------------------------------------------------------------------------
 * Returns a type of mob given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* none_category::get_type(const string &name) const { return NULL; }


/* ----------------------------------------------------------------------------
 * Returns all types of leader by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void none_category::get_type_names(vector<string> &list) const { }


/* ----------------------------------------------------------------------------
 * Registers a created type of leader.
 * type:
 *   Mob type to register.
 */
void none_category::register_type(mob_type* type) { }
