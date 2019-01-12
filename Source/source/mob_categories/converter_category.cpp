/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter mob category class.
 */

#include <algorithm>

#include "converter_category.h"

#include "../mobs/converter.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the converter category.
 */
converter_category::converter_category() :
    mob_category(
        MOB_CATEGORY_CONVERTERS, "Converter", "Converters",
        CONVERTERS_FOLDER_PATH, al_map_rgb(240, 80, 200)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of converter by name.
 */
void converter_category::get_type_names(vector<string> &list) {
    for(auto t = converter_types.begin(); t != converter_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of converter given its name, or NULL on error.
 */
mob_type* converter_category::get_type(const string &name) {
    auto it = converter_types.find(name);
    if(it == converter_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of converter.
 */
mob_type* converter_category::create_type() {
    return new converter_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of converter.
 */
void converter_category::register_type(mob_type* type) {
    converter_types[type->name] = (converter_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a converter and adds it to the list of converters.
 */
mob* converter_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    converter* m = new converter(pos, (converter_type*) type, angle);
    converters.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a converter from the list of converters.
 */
void converter_category::erase_mob(mob* m) {
    converters.erase(
        find(converters.begin(), converters.end(), (converter*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of converters.
 */
void converter_category::clear_types() {
    for(auto t = converter_types.begin(); t != converter_types.end(); ++t) {
        delete t->second;
    }
    converter_types.clear();
}


converter_category::~converter_category() { }
