/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Custom mob category class.
 */

#include <algorithm>

#include "custom_category.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a category for the custom mob types.
 */
custom_category::custom_category() :
    mob_category(
        MOB_CATEGORY_CUSTOM, "Custom", "Custom",
        CUSTOM_MOB_FOLDER_PATH, al_map_rgb(224, 128, 224)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all custom types by name.
 */
void custom_category::get_type_names(vector<string> &list) {
    for(auto t = custom_mob_types.begin(); t != custom_mob_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a custom type given its name, or NULL on error.
 */
mob_type* custom_category::get_type(const string &name) {
    auto it = custom_mob_types.find(name);
    if(it == custom_mob_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty custom type.
 */
mob_type* custom_category::create_type() {
    return new mob_type(MOB_CATEGORY_CUSTOM);
}


/* ----------------------------------------------------------------------------
 * Registers a created custom type.
 */
void custom_category::register_type(mob_type* type) {
    custom_mob_types[type->name] = type;
}


/* ----------------------------------------------------------------------------
 * Creates a custom mob and adds it to the list of custom mobs.
 */
mob* custom_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    mob* m = new mob(pos, type, angle);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a custom mob from the list of custom mobs.
 */
void custom_category::erase_mob(mob* m) { }


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of custom mob.
 */
void custom_category::clear_types() {
    for(auto t = custom_mob_types.begin(); t != custom_mob_types.end(); ++t) {
        //TODO warning: deleting object of polymorphic class type 'mob_type'
        //which has non-virtual destructor might cause undefined behaviour
        //[-Wdelete-non-virtual-dtor]
        delete t->second;
    }
    custom_mob_types.clear();
}


custom_category::~custom_category() { }
