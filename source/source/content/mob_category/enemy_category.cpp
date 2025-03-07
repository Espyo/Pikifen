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

#include "../../core/game.h"
#include "../mob/enemy.h"


/**
 * @brief Constructs a new enemy category object.
 */
EnemyCategory::EnemyCategory() :
    MobCategory(
        MOB_CATEGORY_ENEMIES, "enemy",
        "Enemy", "Enemies",
        "enemies", al_map_rgb(204, 71, 71)
    ) {
    
}


/**
 * @brief Clears the list of registered types of enemy.
 */
void EnemyCategory::clear_types() {
    for(auto &t : game.content.mob_types.list.enemy) {
        delete t.second;
    }
    game.content.mob_types.list.enemy.clear();
}


/**
 * @brief Creates an enemy and adds it to the list of enemies.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* EnemyCategory::create_mob(
    const Point &pos, MobType* type, float angle
) {
    Enemy* m = new Enemy(pos, (EnemyType*) type, angle);
    game.states.gameplay->mobs.enemies.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of enemy.
 *
 * @return The type.
 */
MobType* EnemyCategory::create_type() {
    return new EnemyType();
}


/**
 * @brief Clears an enemy from the list of enemies.
 *
 * @param m The mob to erase.
 */
void EnemyCategory::erase_mob(Mob* m) {
    game.states.gameplay->mobs.enemies.erase(
        find(
            game.states.gameplay->mobs.enemies.begin(),
            game.states.gameplay->mobs.enemies.end(),
            (Enemy*) m
        )
    );
}


/**
 * @brief Returns a type of enemy given its internal name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* EnemyCategory::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.enemy.find(internal_name);
    if(it == game.content.mob_types.list.enemy.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of enemy by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void EnemyCategory::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.enemy) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of enemy.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void EnemyCategory::register_type(const string &internal_name, MobType* type) {
    game.content.mob_types.list.enemy[internal_name] = (EnemyType*) type;
}
