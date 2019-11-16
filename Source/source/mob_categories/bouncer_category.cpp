/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bouncer mob category class.
 */

#include <algorithm>

#include "bouncer_category.h"

#include "../mobs/bouncer.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the bouncer category.
 */
bouncer_category::bouncer_category() :
    mob_category(
        MOB_CATEGORY_BOUNCERS, "Bouncer", "Bouncers",
        BOUNCERS_FOLDER_PATH, al_map_rgb(112, 112, 80)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of bouncer by name.
 */
void bouncer_category::get_type_names(vector<string> &list) {
    for(auto t = bouncer_types.begin(); t != bouncer_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of bouncer given its name, or NULL on error.
 */
mob_type* bouncer_category::get_type(const string &name) {
    auto it = bouncer_types.find(name);
    if(it == bouncer_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of bouncer.
 */
mob_type* bouncer_category::create_type() {
    return new bouncer_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of bouncer.
 */
void bouncer_category::register_type(mob_type* type) {
    bouncer_types[type->name] = (bouncer_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a bouncer and adds it to the list of bouncers.
 */
mob* bouncer_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    bouncer* m = new bouncer(pos, (bouncer_type*) type, angle);
    bouncers.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a bouncer from the list of bouncers.
 */
void bouncer_category::erase_mob(mob* m) {
    bouncers.erase(
        find(bouncers.begin(), bouncers.end(), (bouncer*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of bouncers.
 */
void bouncer_category::clear_types() {
    for(auto t = bouncer_types.begin(); t != bouncer_types.end(); ++t) {
        delete t->second;
    }
    bouncer_types.clear();
}


bouncer_category::~bouncer_category() { }
