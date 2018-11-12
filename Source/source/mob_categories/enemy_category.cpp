/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy mob category class.
 */

#include <algorithm>

#include "enemy_category.h"
#include "../mobs/enemy.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an enemy category.
 */
enemy_category::enemy_category() :
    mob_category(
        MOB_CATEGORY_ENEMIES, "Enemy", "Enemies",
        ENEMIES_FOLDER_PATH, al_map_rgb(224, 96, 128)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of enemy by name.
 */
void enemy_category::get_type_names(vector<string> &list) {
    for(auto t = enemy_types.begin(); t != enemy_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of enemy given its name, or NULL on error.
 */
mob_type* enemy_category::get_type(const string &name) {
    auto it = enemy_types.find(name);
    if(it == enemy_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of enemy.
 */
mob_type* enemy_category::create_type() {
    return new enemy_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of enemy.
 */
void enemy_category::register_type(mob_type* type) {
    enemy_types[type->name] = (enemy_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates an enemy and adds it to the list of enemies.
 */
mob* enemy_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    enemy* m = new enemy(pos, (enemy_type*) type, angle);
    enemies.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears an enemy from the list of enemies.
 */
void enemy_category::erase_mob(mob* m) {
    enemies.erase(
        find(enemies.begin(), enemies.end(), (enemy*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of enemy.
 */
void enemy_category::clear_types() {
    for(auto t = enemy_types.begin(); t != enemy_types.end(); ++t) {
        //TODO warning: deleting object of polymorphic class type 'enemy_type'
        //which has non-virtual destructor might cause undefined behaviour
        //[-Wdelete-non-virtual-dtor]
        delete t->second;
    }
    enemy_types.clear();
}


enemy_category::~enemy_category() { }
