/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Custom mob category class.
 */

#include <algorithm>

#include "custom_category.h"

#include "../../core/game.h"


/**
 * @brief Constructs a new custom category object.
 */
CustomCategory::CustomCategory() :
    MobCategory(
        MOB_CATEGORY_CUSTOM, "custom",
        "Custom", "Custom",
        "custom", al_map_rgb(178, 73, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of custom mob.
 */
void CustomCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.custom) {
        delete t.second;
    }
    game.content.mobTypes.list.custom.clear();
}


/**
 * @brief Creates a custom mob and adds it to the list of custom mobs.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* CustomCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Mob* m = new Mob(pos, type, angle);
    return m;
}


/**
 * @brief Creates a new, empty custom type.
 *
 * @return The type.
 */
MobType* CustomCategory::createType() {
    return new MobType(MOB_CATEGORY_CUSTOM);
}


/**
 * @brief Clears a custom mob from the list of custom mobs.
 *
 * @param m The mob to erase.
 */
void CustomCategory::eraseMob(Mob* m) { }


/**
 * @brief Returns a custom type given its internal name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type.
 */
MobType* CustomCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.custom.find(internalName);
    if(it == game.content.mobTypes.list.custom.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all custom types by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void CustomCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.custom) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created custom type.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void CustomCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.custom[internalName] = type;
}
