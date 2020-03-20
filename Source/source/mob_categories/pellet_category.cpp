/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet mob category class.
 */

#include <algorithm>

#include "pellet_category.h"

#include "../mobs/pellet.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the pellet category.
 */
pellet_category::pellet_category() :
    mob_category(
        MOB_CATEGORY_PELLETS, "Pellet", "Pellets",
        "Pellets", al_map_rgb(73, 204, 126)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of pellet by name.
 */
void pellet_category::get_type_names(vector<string> &list) {
    for(auto &t : pellet_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of pellet given its name, or NULL on error.
 */
mob_type* pellet_category::get_type(const string &name) {
    auto it = pellet_types.find(name);
    if(it == pellet_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of pellet.
 */
mob_type* pellet_category::create_type() {
    return new pellet_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of pellet.
 */
void pellet_category::register_type(mob_type* type) {
    pellet_types[type->name] = (pellet_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a pellet and adds it to the list of pellets.
 */
mob* pellet_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pellet* m = new pellet(pos, (pellet_type*) type, angle);
    pellets.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a pellet from the list of pellets.
 */
void pellet_category::erase_mob(mob* m) {
    pellets.erase(
        find(pellets.begin(), pellets.end(), (pellet*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of pellet.
 */
void pellet_category::clear_types() {
    for(auto &t : pellet_types) {
        delete t.second;
    }
    pellet_types.clear();
}


pellet_category::~pellet_category() { }
