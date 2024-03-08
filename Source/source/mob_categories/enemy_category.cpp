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


/**
 * @brief Constructs a new enemy category object.
 */
enemy_category::enemy_category() :
    mob_category(
        MOB_CATEGORY_ENEMIES, "Enemy", "Enemies",
        "Enemies", al_map_rgb(204, 71, 71)
    ) {
    
}


/**
 * @brief Clears the list of registered types of enemy.
 */
void enemy_category::clear_types() {
    for(auto &t : game.mob_types.enemy) {
        delete t.second;
    }
    game.mob_types.enemy.clear();
}


/**
 * @brief Creates an enemy and adds it to the list of enemies.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* enemy_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    enemy* m = new enemy(pos, (enemy_type*) type, angle);
    game.states.gameplay->mobs.enemies.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of enemy.
 *
 * @return The type.
 */
mob_type* enemy_category::create_type() {
    return new enemy_type();
}


/**
 * @brief Clears an enemy from the list of enemies.
 *
 * @param m The mob to erase.
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


/**
 * @brief Returns a type of enemy given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* enemy_category::get_type(const string &name) const {
    auto it = game.mob_types.enemy.find(name);
    if(it == game.mob_types.enemy.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of enemy by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void enemy_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.enemy) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of enemy.
 *
 * @param type Mob type to register.
 */
void enemy_category::register_type(mob_type* type) {
    game.mob_types.enemy[type->name] = (enemy_type*) type;
}
