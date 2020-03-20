/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader mob category class.
 */

#include <algorithm>

#include "leader_category.h"

#include "../mobs/leader.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the leader category.
 */
leader_category::leader_category() :
    mob_category(
        MOB_CATEGORY_LEADERS, "Leader", "Leaders",
        "Leaders", al_map_rgb(73, 204, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of leader by name.
 */
void leader_category::get_type_names(vector<string> &list) {
    for(auto &t : leader_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of leader given its name, or NULL on error.
 */
mob_type* leader_category::get_type(const string &name) {
    auto it = leader_types.find(name);
    if(it == leader_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of leader.
 */
mob_type* leader_category::create_type() {
    return new leader_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of leader.
 */
void leader_category::register_type(mob_type* type) {
    leader_types[type->name] = (leader_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a leader and adds it to the list of leaders.
 */
mob* leader_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    leader* m = new leader(pos, (leader_type*) type, angle);
    leaders.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a leader from the list of leaders.
 */
void leader_category::erase_mob(mob* m) {
    leaders.erase(
        find(leaders.begin(), leaders.end(), (leader*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of leader.
 */
void leader_category::clear_types() {
    for(auto &t : leader_types) {
        delete t.second;
    }
    leader_types.clear();
}


leader_category::~leader_category() { }
