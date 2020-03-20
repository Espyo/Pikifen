/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource mob category class.
 */

#include <algorithm>

#include "resource_category.h"

#include "../mobs/resource.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the resource category.
 */
resource_category::resource_category() :
    mob_category(
        MOB_CATEGORY_RESOURCES, "Resource", "Resources",
        "Resources", al_map_rgb(139, 204, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of resource by name.
 */
void resource_category::get_type_names(vector<string> &list) {
    for(auto &t : resource_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of resource given its name, or NULL on error.
 */
mob_type* resource_category::get_type(const string &name) {
    auto it = resource_types.find(name);
    if(it == resource_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of resource.
 */
mob_type* resource_category::create_type() {
    return new resource_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of resource.
 */
void resource_category::register_type(mob_type* type) {
    resource_types[type->name] = (resource_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a resource and adds it to the list of resources.
 */
mob* resource_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    resource* m = new resource(pos, (resource_type*) type, angle);
    resources.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a resource from the list of resources.
 */
void resource_category::erase_mob(mob* m) {
    resources.erase(
        find(resources.begin(), resources.end(), (resource*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of resource.
 */
void resource_category::clear_types() {
    for(auto &t : resource_types) {
        delete t.second;
    }
    resource_types.clear();
}


resource_category::~resource_category() { }
