/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile mob category class.
 */

#include <algorithm>

#include "pile_category.h"

#include "../mobs/pile.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the pile category.
 */
pile_category::pile_category() :
    mob_category(
        MOB_CATEGORY_PILES, "Pile", "Piles",
        PILES_FOLDER_PATH, al_map_rgb(179, 64, 32)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of pile by name.
 */
void pile_category::get_type_names(vector<string> &list) {
    for(auto t = pile_types.begin(); t != pile_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of pile given its name, or NULL on error.
 */
mob_type* pile_category::get_type(const string &name) {
    auto it = pile_types.find(name);
    if(it == pile_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of pile.
 */
mob_type* pile_category::create_type() {
    return new pile_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of pile.
 */
void pile_category::register_type(mob_type* type) {
    pile_types[type->name] = (pile_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a pile and adds it to the list of piles.
 */
mob* pile_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pile* m = new pile(pos, (pile_type*) type, angle);
    piles.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a pile from the list of piles.
 */
void pile_category::erase_mob(mob* m) {
    piles.erase(
        find(piles.begin(), piles.end(), (pile*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of pile.
 */
void pile_category::clear_types() {
    for(auto t = pile_types.begin(); t != pile_types.end(); ++t) {
        delete t->second;
    }
    pile_types.clear();
}


pile_category::~pile_category() { }
