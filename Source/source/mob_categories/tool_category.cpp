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


/* ----------------------------------------------------------------------------
 * Creates an instance of the tool category.
 */
tool_category::tool_category() :
    mob_category(
        MOB_CATEGORY_TOOLS, "Tool", "Tools",
        "Tools", al_map_rgb(204, 178, 139)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of tools.
 */
void tool_category::clear_types() {
    for(auto &t : game.mob_types.tool) {
        delete t.second;
    }
    game.mob_types.tool.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a tool and adds it to the list of tools.
 */
mob* tool_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    tool* m = new tool(pos, (tool_type*) type, angle);
    game.gameplay_state->mobs.tool.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of tool.
 */
mob_type* tool_category::create_type() {
    return new tool_type();
}


/* ----------------------------------------------------------------------------
 * Clears a tool from the list of tools.
 */
void tool_category::erase_mob(mob* m) {
    game.gameplay_state->mobs.tool.erase(
        find(game.gameplay_state->mobs.tool.begin(), game.gameplay_state->mobs.tool.end(), (tool*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of tool given its name, or NULL on error.
 */
mob_type* tool_category::get_type(const string &name) {
    auto it = game.mob_types.tool.find(name);
    if(it == game.mob_types.tool.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of tool by name.
 */
void tool_category::get_type_names(vector<string> &list) {
    for(auto &t : game.mob_types.tool) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of tool.
 */
void tool_category::register_type(mob_type* type) {
    game.mob_types.tool[type->name] = (tool_type*) type;
}
