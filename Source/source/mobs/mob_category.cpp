/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob category classes and mob category-related functions.
 */

#include <algorithm>

#include "mob.h"
#include "mob_category.h"
#include "mob_type.h"
#include "../functions.h"
#include "../vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Initializes a mob category.
 */
mob_category::mob_category(
    const size_t id, const string &name, const string &plural_name,
    const string &folder, const ALLEGRO_COLOR editor_color
) :
    id(id),
    name(name),
    plural_name(plural_name),
    folder(folder),
    editor_color(editor_color) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a "none" category.
 */
none_category::none_category() :
    mob_category(
        MOB_CATEGORY_NONE, "None", "None",
        "", al_map_rgb(255, 0, 0)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Dummies.
 */
void none_category::get_type_names(vector<string> &list) { }
mob_type* none_category::get_type(const string &name) { return NULL; }
mob_type* none_category::create_type() { return NULL; }
void none_category::register_type(mob_type* type) { }
mob* none_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) { return NULL; }
void none_category::erase_mob(mob* m) { }



/* ----------------------------------------------------------------------------
 * Registers a new mob category.
 */
void mob_category_manager::register_category(
    size_t nr,
    mob_category* category
) {
    if(nr >= categories.size()) {
        categories.insert(
            categories.end(),
            (nr + 1) - categories.size(),
            NULL
        );
    }
    categories[nr] = category;
}


/* ----------------------------------------------------------------------------
 * Returns a category given its ID.
 * Returns NULL on error.
 */
mob_category* mob_category_manager::get(const size_t id) {
    if(id >= categories.size()) return NULL;
    return categories[id];
}


/* ----------------------------------------------------------------------------
 * Returns a category given its name.
 * Returns NULL on error.
 */
mob_category* mob_category_manager::get_from_name(const string &name) {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->name == name) return categories[n];
    }
    log_error("Mob category \"" + name + "\" not found!");
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a category given its plural name.
 * Returns NULL on error.
 */
mob_category* mob_category_manager::get_from_pname(const string &pname) {
    for(size_t n = 0; n < categories.size(); ++n) {
        if(categories[n]->plural_name == pname) return categories[n];
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered categories, freeing memory.
 */
void mob_category_manager::clear() {
    for(size_t c = 0; c < categories.size(); ++c) {
        delete categories[c];
    }
    categories.clear();
}



/* ----------------------------------------------------------------------------
 * Creates a Pikmin category.
 */
pikmin_category::pikmin_category() :
    mob_category(
        MOB_CATEGORY_PIKMIN, "Pikmin", "Pikmin",
        PIKMIN_FOLDER_PATH, al_map_rgb(64, 255, 64)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of Pikmin by name.
 */
void pikmin_category::get_type_names(vector<string> &list) {
    for(auto t = pikmin_types.begin(); t != pikmin_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of Pikmin given its name, or NULL on error.
 */
mob_type* pikmin_category::get_type(const string &name) {
    auto it = pikmin_types.find(name);
    if(it == pikmin_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of Pikmin.
 */
mob_type* pikmin_category::create_type() {
    return new pikmin_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of Pikmin.
 */
void pikmin_category::register_type(mob_type* type) {
    pikmin_types[type->name] = (pikmin_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a Pikmin and adds it to the list of Pikmin.
 */
mob* pikmin_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    pikmin* m = new pikmin(pos, (pikmin_type*) type, angle, vars);
    pikmin_list.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a Pikmin from the list of Pikmin.
 */
void pikmin_category::erase_mob(mob* m) {
    pikmin_list.erase(
        find(pikmin_list.begin(), pikmin_list.end(), (pikmin*) m)
    );
}


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
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    enemy* m = new enemy(pos, (enemy_type*) type, angle, vars);
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
 * Creates a leader category.
 */
leader_category::leader_category() :
    mob_category(
        MOB_CATEGORY_LEADERS, "Leader", "Leaders",
        LEADERS_FOLDER_PATH, al_map_rgb(48, 80, 192)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of leader by name.
 */
void leader_category::get_type_names(vector<string> &list) {
    for(auto t = leader_types.begin(); t != leader_types.end(); ++t) {
        list.push_back(t->first);
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
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    leader* m = new leader(pos, (leader_type*) type, angle, vars);
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
 * Creates an Onion category.
 */
onion_category::onion_category() :
    mob_category(
        MOB_CATEGORY_ONIONS, "Onion", "Onions",
        ONIONS_FOLDER_PATH, al_map_rgb(48, 160, 48)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of Onion by name.
 */
void onion_category::get_type_names(vector<string> &list) {
    for(auto t = onion_types.begin(); t != onion_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of Onion given its name, or NULL on error.
 */
mob_type* onion_category::get_type(const string &name) {
    auto it = onion_types.find(name);
    if(it == onion_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of Onion.
 */
mob_type* onion_category::create_type() {
    return new onion_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of Onion.
 */
void onion_category::register_type(mob_type* type) {
    onion_types[type->name] = (onion_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates an Onion and adds it to the list of Onions.
 */
mob* onion_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    onion* m = new onion(pos, (onion_type*) type, angle, vars);
    onions.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears an Onion from the list of Onions.
 */
void onion_category::erase_mob(mob* m) {
    onions.erase(
        find(onions.begin(), onions.end(), (onion*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Creates a pellet category.
 */
pellet_category::pellet_category() :
    mob_category(
        MOB_CATEGORY_PELLETS, "Pellet", "Pellets",
        PELLETS_FOLDER_PATH, al_map_rgb(208, 224, 96)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of pellet by name.
 */
void pellet_category::get_type_names(vector<string> &list) {
    for(auto t = pellet_types.begin(); t != pellet_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of pellet given its name, or NULL on error.
 */
mob_type* pellet_category::get_type(const string &name) {
    auto it = pellet_types.find(name);
    if(it == pellet_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of pellet.
 */
mob_type* pellet_category::create_type() {
    return new pellet_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of pellet.
 */
void pellet_category::register_type(mob_type* type) {
    pellet_types[type->name] = (pellet_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a pellet and adds it to the list of pellets.
 */
mob* pellet_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    pellet* m = new pellet(pos, (pellet_type*) type, angle, vars);
    pellets.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a pellet from the list of pellets.
 */
void pellet_category::erase_mob(mob* m) {
    pellets.erase(
        find(pellets.begin(), pellets.end(), (pellet*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Creates a ship category.
 */
ship_category::ship_category() :
    mob_category(
        MOB_CATEGORY_SHIPS, "Ship", "Ships",
        SHIPS_FOLDER_PATH, al_map_rgb(128, 128, 192)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of ship by name.
 */
void ship_category::get_type_names(vector<string> &list) {
    for(auto t = ship_types.begin(); t != ship_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of ship given its name, or NULL on error.
 */
mob_type* ship_category::get_type(const string &name) {
    auto it = ship_types.find(name);
    if(it == ship_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of ship.
 */
mob_type* ship_category::create_type() {
    return new ship_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of ship.
 */
void ship_category::register_type(mob_type* type) {
    ship_types[type->name] = (ship_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a ship and adds it to the list of ships.
 */
mob* ship_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    ship* m = new ship(pos, (ship_type*) type, angle, vars);
    ships.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a ship from the list of ships.
 */
void ship_category::erase_mob(mob* m) {
    ships.erase(
        find(ships.begin(), ships.end(), (ship*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Creates a treasure category.
 */
treasure_category::treasure_category() :
    mob_category(
        MOB_CATEGORY_TREASURES, "Treasure", "Treasures",
        TREASURES_FOLDER_PATH, al_map_rgb(255, 240, 64)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of treasure by name.
 */
void treasure_category::get_type_names(vector<string> &list) {
    for(auto t = treasure_types.begin(); t != treasure_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of treasure given its name, or NULL on error.
 */
mob_type* treasure_category::get_type(const string &name) {
    auto it = treasure_types.find(name);
    if(it == treasure_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of treasure.
 */
mob_type* treasure_category::create_type() {
    return new treasure_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of treasure.
 */
void treasure_category::register_type(mob_type* type) {
    treasure_types[type->name] = (treasure_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a treasure and adds it to the list of treasures.
 */
mob* treasure_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    treasure* m = new treasure(pos, (treasure_type*) type, angle, vars);
    treasures.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a treasure from the list of treasures.
 */
void treasure_category::erase_mob(mob* m) {
    treasures.erase(
        find(treasures.begin(), treasures.end(), (treasure*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Creates a gate category.
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
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    gate* m = new gate(pos, (gate_type*) type, angle, vars);
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
 * Creates a bridge category.
 */
bridge_category::bridge_category() :
    mob_category(
        MOB_CATEGORY_BRIDGES, "Bridge", "Bridges",
        BRIDGES_FOLDER_PATH, al_map_rgb(224, 200, 180)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of bridge by name.
 */
void bridge_category::get_type_names(vector<string> &list) {
    for(auto t = bridge_types.begin(); t != bridge_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of bridge given its name, or NULL on error.
 */
mob_type* bridge_category::get_type(const string &name) {
    auto it = bridge_types.find(name);
    if(it == bridge_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of bridge.
 */
mob_type* bridge_category::create_type() {
    return new bridge_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of bridge.
 */
void bridge_category::register_type(mob_type* type) {
    bridge_types[type->name] = (bridge_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a bridge and adds it to the list of bridges.
 */
mob* bridge_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    bridge* m = new bridge(pos, (bridge_type*) type, angle, vars);
    bridges.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a bridge from the list of bridges.
 */
void bridge_category::erase_mob(mob* m) {
    bridges.erase(
        find(bridges.begin(), bridges.end(), (bridge*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Creates a category for the special mob types.
 */
special_category::special_category() :
    mob_category(
        MOB_CATEGORY_SPECIAL, "Special", "Special",
        "", al_map_rgb(32, 160, 160)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all special types by name.
 */
void special_category::get_type_names(vector<string> &list) {
    for(auto t = spec_mob_types.begin(); t != spec_mob_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a special type given its name, or NULL on error.
 */
mob_type* special_category::get_type(const string &name) {
    auto it = spec_mob_types.find(name);
    if(it == spec_mob_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty special type.
 */
mob_type* special_category::create_type() {
    return new mob_type(MOB_CATEGORY_SPECIAL);
}


/* ----------------------------------------------------------------------------
 * Registers a created special type.
 */
void special_category::register_type(mob_type* type) {
    spec_mob_types[type->name] = type;
}


/* ----------------------------------------------------------------------------
 * Creates a special mob and adds it to the list of special mobs.
 */
mob* special_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    mob* m = new mob(pos, type, angle, vars);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a special mob from the list of special mobs.
 */
void special_category::erase_mob(mob* m) { }


/* ----------------------------------------------------------------------------
 * Creates a category for the custom mob types.
 */
custom_category::custom_category() :
    mob_category(
        MOB_CATEGORY_CUSTOM, "Custom", "Custom",
        CUSTOM_MOB_FOLDER_PATH, al_map_rgb(224, 128, 224)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all custom types by name.
 */
void custom_category::get_type_names(vector<string> &list) {
    for(auto t = custom_mob_types.begin(); t != custom_mob_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a custom type given its name, or NULL on error.
 */
mob_type* custom_category::get_type(const string &name) {
    auto it = custom_mob_types.find(name);
    if(it == custom_mob_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty custom type.
 */
mob_type* custom_category::create_type() {
    return new mob_type(MOB_CATEGORY_CUSTOM);
}


/* ----------------------------------------------------------------------------
 * Registers a created custom type.
 */
void custom_category::register_type(mob_type* type) {
    custom_mob_types[type->name] = type;
}


/* ----------------------------------------------------------------------------
 * Creates a custom mob and adds it to the list of custom mobs.
 */
mob* custom_category::create_mob(
    const point &pos, mob_type* type,
    const float angle, const string &vars
) {
    mob* m = new mob(pos, type, angle, vars);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a custom mob from the list of custom mobs.
 */
void custom_category::erase_mob(mob* m) { }
