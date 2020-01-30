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

#include "../mobs/tool.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the tool category.
 */
tool_category::tool_category() :
    mob_category(
        MOB_CATEGORY_TOOLS, "Tool", "Tools",
        TOOLS_FOLDER_PATH, al_map_rgb(204, 178, 139)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of tool by name.
 */
void tool_category::get_type_names(vector<string> &list) {
    for(auto t = tool_types.begin(); t != tool_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of tool given its name, or NULL on error.
 */
mob_type* tool_category::get_type(const string &name) {
    auto it = tool_types.find(name);
    if(it == tool_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of tool.
 */
mob_type* tool_category::create_type() {
    return new tool_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of tool.
 */
void tool_category::register_type(mob_type* type) {
    tool_types[type->name] = (tool_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a tool and adds it to the list of tools.
 */
mob* tool_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    tool* m = new tool(pos, (tool_type*) type, angle);
    tools.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a tool from the list of tools.
 */
void tool_category::erase_mob(mob* m) {
    tools.erase(
        find(tools.begin(), tools.end(), (tool*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of tools.
 */
void tool_category::clear_types() {
    for(auto t = tool_types.begin(); t != tool_types.end(); ++t) {
        delete t->second;
    }
    tool_types.clear();
}


tool_category::~tool_category() { }
