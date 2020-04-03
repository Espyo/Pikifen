/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale mob category class.
 */

#include <algorithm>

#include "scale_category.h"

#include "../mobs/scale.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the scale category.
 */
scale_category::scale_category() :
    mob_category(
        MOB_CATEGORY_SCALES, "Scale", "Scales",
        "Scales", al_map_rgb(139, 165, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of scale.
 */
void scale_category::clear_types() {
    for(auto &t : scale_types) {
        delete t.second;
    }
    scale_types.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a scale and adds it to the list of scales.
 */
mob* scale_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    scale* m = new scale(pos, (scale_type*) type, angle);
    scales.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of scale.
 */
mob_type* scale_category::create_type() {
    return new scale_type();
}


/* ----------------------------------------------------------------------------
 * Clears a scale from the list of scales.
 */
void scale_category::erase_mob(mob* m) {
    scales.erase(
        find(scales.begin(), scales.end(), (scale*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of scale given its name, or NULL on error.
 */
mob_type* scale_category::get_type(const string &name) {
    auto it = scale_types.find(name);
    if(it == scale_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of scale by name.
 */
void scale_category::get_type_names(vector<string> &list) {
    for(auto &t : scale_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of scale.
 */
void scale_category::register_type(mob_type* type) {
    scale_types[type->name] = (scale_type*) type;
}
