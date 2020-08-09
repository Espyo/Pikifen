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

#include "../game.h"
#include "../mobs/pile.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the pile category.
 */
pile_category::pile_category() :
    mob_category(
        MOB_CATEGORY_PILES, "Pile", "Piles",
        "Piles", al_map_rgb(139, 204, 165)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of pile.
 */
void pile_category::clear_types() {
    for(auto &t : game.mob_types.pile) {
        delete t.second;
    }
    game.mob_types.pile.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a pile and adds it to the list of piles.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* pile_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pile* m = new pile(pos, (pile_type*) type, angle);
    game.states.gameplay_st->mobs.piles.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of pile.
 */
mob_type* pile_category::create_type() {
    return new pile_type();
}


/* ----------------------------------------------------------------------------
 * Clears a pile from the list of piles.
 * m:
 *   The mob to erase.
 */
void pile_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.piles.erase(
        find(
        game.states.gameplay_st->mobs.piles.begin(), 
        game.states.gameplay_st->mobs.piles.end(), 
        (pile*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of pile given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* pile_category::get_type(const string &name) const {
    auto it = game.mob_types.pile.find(name);
    if(it == game.mob_types.pile.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of pile by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void pile_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.pile) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of pile.
 * type:
 *   Mob type to register.
 */
void pile_category::register_type(mob_type* type) {
    game.mob_types.pile[type->name] = (pile_type*) type;
}
