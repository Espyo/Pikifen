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

#include "../game.h"
#include "../mob_fsms/leader_fsm.h"
#include "../mobs/leader.h"


/**
 * @brief Constructs a new leader category object.
 */
leader_category::leader_category() :
    mob_category(
        MOB_CATEGORY_LEADERS, "leader",
        "Leader", "Leaders",
        "leaders", al_map_rgb(73, 204, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of leader.
 */
void leader_category::clear_types() {
    for(auto &t : game.content.mob_types.list.leader) {
        delete t.second;
    }
    game.content.mob_types.list.leader.clear();
}


/**
 * @brief Creates a leader and adds it to the list of leaders.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* leader_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    leader* m = new leader(pos, (leader_type*) type, angle);
    game.states.gameplay->mobs.leaders.push_back(m);
    game.states.gameplay->update_available_leaders();
    return m;
}


/**
 * @brief Creates a new, empty type of leader.
 *
 * @return The type.
 */
mob_type* leader_category::create_type() {
    return new leader_type();
}


/**
 * @brief Clears a leader from the list of leaders.
 *
 * @param m The mob to erase.
 */
void leader_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.leaders.erase(
        find(
            game.states.gameplay->mobs.leaders.begin(),
            game.states.gameplay->mobs.leaders.end(),
            (leader*) m
        )
    );
    leader_fsm::die((leader*) m, nullptr, nullptr);
}


/**
 * @brief Returns a type of leader given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* leader_category::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.leader.find(internal_name);
    if(it == game.content.mob_types.list.leader.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of leader by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void leader_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.leader) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of leader.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void leader_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.leader[internal_name] = (leader_type*) type;
}
