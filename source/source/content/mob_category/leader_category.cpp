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

#include "../../core/game.h"
#include "../mob/leader.h"
#include "../mob_script/leader_fsm.h"


/**
 * @brief Constructs a new leader category object.
 */
LeaderCategory::LeaderCategory() :
    MobCategory(
        MOB_CATEGORY_LEADERS, "leader",
        "Leader", "Leaders",
        "leaders", al_map_rgb(73, 204, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of leader.
 */
void LeaderCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.leader) {
        delete t.second;
    }
    game.content.mobTypes.list.leader.clear();
}


/**
 * @brief Creates a leader and adds it to the list of leaders.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* LeaderCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Leader* m = new Leader(pos, (LeaderType*) type, angle);
    game.states.gameplay->mobs.leaders.push_back(m);
    game.states.gameplay->updateAvailableLeaders();
    return m;
}


/**
 * @brief Creates a new, empty type of leader.
 *
 * @return The type.
 */
MobType* LeaderCategory::createType() {
    return new LeaderType();
}


/**
 * @brief Clears a leader from the list of leaders.
 *
 * @param m The mob to erase.
 */
void LeaderCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.leaders.erase(
        find(
            game.states.gameplay->mobs.leaders.begin(),
            game.states.gameplay->mobs.leaders.end(),
            (Leader*) m
        )
    );
    LeaderFsm::die((Leader*) m, nullptr, nullptr);
}


/**
 * @brief Returns a type of leader given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* LeaderCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.leader.find(internalName);
    if(it == game.content.mobTypes.list.leader.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of leader by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void LeaderCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.leader) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of leader.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void LeaderCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.leader[internalName] = (LeaderType*) type;
}
