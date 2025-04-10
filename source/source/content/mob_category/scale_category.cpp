/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale mob category class.
 */

#include <algorithm>

#include "scale_category.h"

#include "../../core/game.h"
#include "../mob/scale.h"


/**
 * @brief Constructs a new scale category object.
 */
ScaleCategory::ScaleCategory() :
    MobCategory(
        MOB_CATEGORY_SCALES, "scale",
        "Scale", "Scales",
        "scales", al_map_rgb(139, 165, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of scale.
 */
void ScaleCategory::clearTypes() {
    for(auto &t : game.content.mob_types.list.scale) {
        delete t.second;
    }
    game.content.mob_types.list.scale.clear();
}


/**
 * @brief Creates a scale and adds it to the list of scales.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* ScaleCategory::createMob(
    const Point &pos, MobType* type, float angle
) {
    Scale* m = new Scale(pos, (ScaleType*) type, angle);
    game.states.gameplay->mobs.scales.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of scale.
 *
 * @return The type.
 */
MobType* ScaleCategory::createType() {
    return new ScaleType();
}


/**
 * @brief Clears a scale from the list of scales.
 *
 * @param m The mob to erase.
 */
void ScaleCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.scales.erase(
        find(
            game.states.gameplay->mobs.scales.begin(),
            game.states.gameplay->mobs.scales.end(),
            (Scale*) m
        )
    );
}


/**
 * @brief Returns a type of scale given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* ScaleCategory::getType(const string &internal_name) const {
    auto it = game.content.mob_types.list.scale.find(internal_name);
    if(it == game.content.mob_types.list.scale.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of scale by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void ScaleCategory::getTypeNames(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.scale) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of scale.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void ScaleCategory::registerType(const string &internal_name, MobType* type) {
    game.content.mob_types.list.scale[internal_name] = (ScaleType*) type;
}
