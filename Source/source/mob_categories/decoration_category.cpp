/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration mob category class.
 */

#include <algorithm>

#include "decoration_category.h"

#include "../game.h"
#include "../mobs/decoration.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the decoration category.
 */
decoration_category::decoration_category() :
    mob_category(
        MOB_CATEGORY_DECORATIONS, "Decoration", "Decorations",
        "Decorations", al_map_rgb(191, 204, 139)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of decorations.
 */
void decoration_category::clear_types() {
    for(auto &t : game.mob_types.decoration) {
        delete t.second;
    }
    game.mob_types.decoration.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a decoration and adds it to the list of decorations.
 */
mob* decoration_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    decoration* m = new decoration(pos, (decoration_type*) type, angle);
    game.states.gameplay_st->mobs.decoration.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of decoration.
 */
mob_type* decoration_category::create_type() {
    return new decoration_type();
}


/* ----------------------------------------------------------------------------
 * Clears a decoration from the list of decorations.
 */
void decoration_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.decoration.erase(
        find(game.states.gameplay_st->mobs.decoration.begin(), game.states.gameplay_st->mobs.decoration.end(), (decoration*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of decoration given its name, or NULL on error.
 */
mob_type* decoration_category::get_type(const string &name) const {
    auto it = game.mob_types.decoration.find(name);
    if(it == game.mob_types.decoration.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of decoration by name.
 */
void decoration_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.decoration) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of decoration.
 */
void decoration_category::register_type(mob_type* type) {
    game.mob_types.decoration[type->name] = (decoration_type*) type;
}
