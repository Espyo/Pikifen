/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter mob category class.
 */

#include <algorithm>

#include "converter_category.h"

#include "../../core/game.h"
#include "../mob/converter.h"


/**
 * @brief Constructs a new converter category object.
 *
 */
ConverterCategory::ConverterCategory() :
    MobCategory(
        MOB_CATEGORY_CONVERTERS, "converter",
        "Converter", "Converters",
        "converters", al_map_rgb(73, 126, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of converters.
 */
void ConverterCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.converter) {
        delete t.second;
    }
    game.content.mobTypes.list.converter.clear();
}


/**
 * @brief Creates a converter and adds it to the list of converters.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* ConverterCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Converter* m = new Converter(pos, (ConverterType*) type, angle);
    game.states.gameplay->mobs.converters.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of converter.
 *
 * @return The type.
 */
MobType* ConverterCategory::createType() {
    return new ConverterType();
}


/**
 * @brief Clears a converter from the list of converters.
 *
 * @param m The mob to erase.
 */
void ConverterCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.converters.erase(
        find(
            game.states.gameplay->mobs.converters.begin(),
            game.states.gameplay->mobs.converters.end(),
            (Converter*) m
        )
    );
}


/**
 * @brief Returns a type of converter given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* ConverterCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.converter.find(internalName);
    if(it == game.content.mobTypes.list.converter.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of converter by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void ConverterCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.converter) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of converter.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void ConverterCategory::registerType(
    const string& internalName, MobType* type
) {
    game.content.mobTypes.list.converter[internalName] = (ConverterType*) type;
}
