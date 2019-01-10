/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate mob category class.
 */

#include <algorithm>

#include "gate_category.h"
#include "../mobs/gate.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the gate category.
 */
gate_category::gate_category() :
    mob_category(
        MOB_CATEGORY_GATES, "Gate", "Gates",
        GATES_FOLDER_PATH, al_map_rgb(224, 192, 192)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of gate by name.
 */
void gate_category::get_type_names(vector<string> &list) {
    for(auto t = gate_types.begin(); t != gate_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of gate given its name, or NULL on error.
 */
mob_type* gate_category::get_type(const string &name) {
    auto it = gate_types.find(name);
    if(it == gate_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of gate.
 */
mob_type* gate_category::create_type() {
    return new gate_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of gate.
 */
void gate_category::register_type(mob_type* type) {
    gate_types[type->name] = (gate_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a gate and adds it to the list of gates.
 */
mob* gate_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    gate* m = new gate(pos, (gate_type*) type, angle);
    gates.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a gate from the list of gates.
 */
void gate_category::erase_mob(mob* m) {
    gates.erase(
        find(gates.begin(), gates.end(), (gate*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of gates.
 */
void gate_category::clear_types() {
    for(auto t = gate_types.begin(); t != gate_types.end(); ++t) {
        delete t->second;
    }
    gate_types.clear();
}


gate_category::~gate_category() { }


