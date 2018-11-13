/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interectable mob category class.
 */

#include <algorithm>

#include "interactable_category.h"
#include "../mob_types/interactable_type.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an interactable category.
 */
interactable_category::interactable_category() :
    mob_category(
        MOB_CATEGORY_INTERACTABLES, "interactable", "interactables",
        INTERACTABLES_FOLDER_PATH, al_map_rgb(115, 230, 194)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of interactable by name.
 */
void interactable_category::get_type_names(vector<string> &list) {
    for(
        auto t = interactable_types.begin();
        t != interactable_types.end(); ++t
    ) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of interactable given its name, or NULL on error.
 */
mob_type* interactable_category::get_type(const string &name) {
    auto it = interactable_types.find(name);
    if(it == interactable_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of interactable.
 */
mob_type* interactable_category::create_type() {
    return new interactable_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of interactable.
 */
void interactable_category::register_type(mob_type* type) {
    interactable_types[type->name] = (interactable_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates an interactable and adds it to the list of interactables.
 */
mob* interactable_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    interactable* m = new interactable(pos, (interactable_type*) type, angle);
    interactables.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears an interactable from the list of interactables.
 */
void interactable_category::erase_mob(mob* m) {
    interactables.erase(
        find(interactables.begin(), interactables.end(), (interactable*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of interactables.
 */
void interactable_category::clear_types() {
    for(
        auto t = interactable_types.begin();
        t != interactable_types.end(); ++t
    ) {
        delete t->second;
    }
    interactable_types.clear();
}


interactable_category::~interactable_category() { }
