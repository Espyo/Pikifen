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


/* ----------------------------------------------------------------------------
 * Creates an instance of the group task category.
 */
group_task_category::group_task_category() :
    mob_category(
        MOB_CATEGORY_GROUP_TASKS, "Group task", "Group tasks",
        "Group_tasks", al_map_rgb(152, 204, 139)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of group tasks.
 */
void group_task_category::clear_types() {
    for(auto &t : game.mob_types.group_task) {
        delete t.second;
    }
    game.mob_types.group_task.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a group task and adds it to the list of group tasks.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* group_task_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    group_task* m = new group_task(pos, (group_task_type*) type, angle);
    game.states.gameplay->mobs.group_tasks.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of group task.
 */
mob_type* group_task_category::create_type() {
    return new group_task_type();
}


/* ----------------------------------------------------------------------------
 * Clears a group task from the list of group task.
 * m:
 *   The mob to erase.
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


/* ----------------------------------------------------------------------------
 * Returns a type of group task given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* group_task_category::get_type(const string &name) const {
    auto it = game.mob_types.group_task.find(name);
    if(it == game.mob_types.group_task.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of group tasks by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void group_task_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.group_task) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of group task.
 * type:
 *   Mob type to register.
 */
void group_task_category::register_type(mob_type* type) {
    game.mob_types.group_task[type->name] = (group_task_type*) type;
}
