/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy mob category class.
 */

#include <algorithm>

#include "enemy_category.h"

#include "../game.h"
#include "../mobs/enemy.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the enemy category.
 */
enemy_category::enemy_category() :
    mob_category(
        MOB_CATEGORY_ENEMIES, "Enemy", "Enemies",
        "Enemies", al_map_rgb(204, 71, 71)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of enemy.
 */
void enemy_category::clear_types() {
    for(auto &t : game.mob_types.enemy) {
        delete t.second;
    }
    game.mob_types.enemy.clear();
}


/* ----------------------------------------------------------------------------
 * Creates an enemy and adds it to the list of enemies.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* enemy_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    enemy* m = new enemy(pos, (enemy_type*) type, angle);
    game.states.gameplay->mobs.enemies.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of enemy.
 */
mob_type* enemy_category::create_type() {
    return new enemy_type();
}


/* ----------------------------------------------------------------------------
 * Clears an enemy from the list of enemies.
 * m:
 *   The mob to erase.
 */
void enemy_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.enemies.erase(
        find(
        game.states.gameplay->mobs.enemies.begin(),
        game.states.gameplay->mobs.enemies.end(),
        (enemy*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of enemy given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* enemy_category::get_type(const string &name) const {
    auto it = game.mob_types.enemy.find(name);
    if(it == game.mob_types.enemy.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of enemy by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void enemy_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.enemy) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of enemy.
 * type:
 *   Mob type to register.
 */
void enemy_category::register_type(mob_type* type) {
    game.mob_types.enemy[type->name] = (enemy_type*) type;
}
