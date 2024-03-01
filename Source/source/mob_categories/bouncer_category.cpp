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

#include "../game.h"
#include "../mobs/bouncer.h"


/**
 * @brief Constructs a new bouncer category object.
 */
bouncer_category::bouncer_category() :
    mob_category(
        MOB_CATEGORY_BOUNCERS, "Bouncer", "Bouncers",
        "Bouncers", al_map_rgb(192, 139, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of bouncers.
 */
void bouncer_category::clear_types() {
    for(auto &t : game.mob_types.bouncer) {
        delete t.second;
    }
    game.mob_types.bouncer.clear();
}


/**
 * @brief Creates a bouncer and adds it to the list of bouncers.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The created mob.
 */
mob* bouncer_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    bouncer* m = new bouncer(pos, (bouncer_type*) type, angle);
    game.states.gameplay->mobs.bouncers.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of bouncer.
 *
 * @return The type.
 */
mob_type* bouncer_category::create_type() {
    return new bouncer_type();
}


/**
 * @brief Clears a bouncer from the list of bouncers.
 *
 * @param m The mob to erase.
 */
void bouncer_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.bouncers.erase(
        find(
            game.states.gameplay->mobs.bouncers.begin(),
            game.states.gameplay->mobs.bouncers.end(),
            (bouncer*) m
        )
    );
}


/**
 * @brief Returns a type of bouncer given its name
 *
 * @param name Name of the mob type to get.
 * @return The type, or NULL on error.
 */
mob_type* bouncer_category::get_type(const string &name) const {
    auto it = game.mob_types.bouncer.find(name);
    if(it == game.mob_types.bouncer.end()) return NULL;
    return it->second;
}


/**
 * @brief Returns all types of bouncer by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void bouncer_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.bouncer) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of bouncer.
 *
 * @param type Mob type to register.
 */
void bouncer_category::register_type(mob_type* type) {
    game.mob_types.bouncer[type->name] = (bouncer_type*) type;
}
