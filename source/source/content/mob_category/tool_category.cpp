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

#include "../../core/game.h"
#include "../mob/tool.h"


/**
 * @brief Constructs a new tool category object.
 */
ToolCategory::ToolCategory() :
    MobCategory(
        MOB_CATEGORY_TOOLS, "tool",
        "Tool", "Tools",
        "tools", al_map_rgb(204, 178, 139)
    ) {
    
}


/**
 * @brief Clears the list of registered types of tools.
 */
void ToolCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.tool) {
        delete t.second;
    }
    game.content.mobTypes.list.tool.clear();
}


/**
 * @brief Creates a tool and adds it to the list of tools.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* ToolCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Tool* m = new Tool(pos, (ToolType*) type, angle);
    game.states.gameplay->mobs.tools.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of tool.
 *
 * @return The type.
 */
MobType* ToolCategory::createType() {
    return new ToolType();
}


/**
 * @brief Clears a tool from the list of tools.
 *
 * @param m The mob to erase.
 */
void ToolCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.tools.erase(
        find(
            game.states.gameplay->mobs.tools.begin(),
            game.states.gameplay->mobs.tools.end(),
            (Tool*) m
        )
    );
}


/**
 * @brief Returns a type of tool given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* ToolCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.tool.find(internalName);
    if(it == game.content.mobTypes.list.tool.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of tool by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void ToolCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.tool) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of tool.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void ToolCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.tool[internalName] = (ToolType*) type;
}
