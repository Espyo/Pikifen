/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Group task mob category class.
 */

#include <algorithm>

#include "group_task_category.h"

#include "../game.h"
#include "../mobs/group_task.h"


/**
 * @brief Constructs a new group task category object.
 */
group_task_category::group_task_category() :
    mob_category(
        MOB_CATEGORY_GROUP_TASKS, "Group task", "Group tasks",
        "Group_tasks", al_map_rgb(152, 204, 139)
    ) {
    
}


/**
 * @brief Clears the list of registered types of group tasks.
 */
void group_task_category::clear_types() {
    for(auto &t : game.content.mob_types.group_task) {
        delete t.second;
    }
    game.content.mob_types.group_task.clear();
}


/**
 * @brief Creates a group task and adds it to the list of group tasks.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* group_task_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    group_task* m = new group_task(pos, (group_task_type*) type, angle);
    game.states.gameplay->mobs.group_tasks.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of group task.
 *
 * @return The type.
 */
mob_type* group_task_category::create_type() {
    return new group_task_type();
}


/**
 * @brief Clears a group task from the list of group task.
 *
 * @param m The mob to erase.
 */
void group_task_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.group_tasks.erase(
        find(
            game.states.gameplay->mobs.group_tasks.begin(),
            game.states.gameplay->mobs.group_tasks.end(),
            (group_task*) m
        )
    );
}


/**
 * @brief Returns a type of group task given its name, or nullptr on error.
 *
 * @param name Name of the mob type to get.
 * @return The type.
 */
mob_type* group_task_category::get_type(const string &name) const {
    auto it = game.content.mob_types.group_task.find(name);
    if(it == game.content.mob_types.group_task.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of group tasks by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void group_task_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.group_task) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of group task.
 *
 * @param type The mob type to register.
 */
void group_task_category::register_type(mob_type* type) {
    game.content.mob_types.group_task[type->name] = (group_task_type*) type;
}
