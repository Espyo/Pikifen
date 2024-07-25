/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Tool mob category class.
 */

#include <algorithm>

#include "tool_category.h"

#include "../game.h"
#include "../mobs/tool.h"


/**
 * @brief Constructs a new tool category object.
 */
tool_category::tool_category() :
    mob_category(
        MOB_CATEGORY_TOOLS, "Tool", "Tools",
        "Tools", al_map_rgb(204, 178, 139)
    ) {
    
}


/**
 * @brief Clears the list of registered types of tools.
 */
void tool_category::clear_types() {
    for(auto &t : game.content.mob_types.tool) {
        delete t.second;
    }
    game.content.mob_types.tool.clear();
}


/**
 * @brief Creates a tool and adds it to the list of tools.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* tool_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    tool* m = new tool(pos, (tool_type*) type, angle);
    game.states.gameplay->mobs.tools.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of tool.
 *
 * @return The type.
 */
mob_type* tool_category::create_type() {
    return new tool_type();
}


/**
 * @brief Clears a tool from the list of tools.
 *
 * @param m The mob to erase.
 */
void tool_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.tools.erase(
        find(
            game.states.gameplay->mobs.tools.begin(),
            game.states.gameplay->mobs.tools.end(),
            (tool*) m
        )
    );
}


/**
 * @brief Returns a type of tool given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* tool_category::get_type(const string &name) const {
    auto it = game.content.mob_types.tool.find(name);
    if(it == game.content.mob_types.tool.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of tool by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void tool_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.tool) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of tool.
 *
 * @param type Mob type to register.
 */
void tool_category::register_type(mob_type* type) {
    game.content.mob_types.tool[type->name] = (tool_type*) type;
}
