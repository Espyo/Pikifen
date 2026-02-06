/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for enum-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 * 
 * Based on code by https://github.com/andrewstephens75/EnumMapping
 */

#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>


using std::string;
using std::vector;


//Macro for constructing the mapping object.
#define buildEnumNames(mapName, enumType) \
    constexpr auto mapName = \
    std::to_array<const EnumNamePair<enumType> >


/**
 * @brief Holds the pairing between enum value and name.
 * 
 * @tparam T Enum type.
 */
template<class EnumT>
struct EnumNamePair {
    using SavedEnumT = EnumT;
    const EnumT value;
    const char* const name;
};


/**
 * @brief Returns the amount of values an enum has,
 * if it had been previously mapped.
 * 
 * @tparam MapT Type of the object containing the enum mappings.
 * @tparam EnumT Enum type.
 * @param mapObj Object containing the enum mappings.
 * @return The value count.
 */
template<class MapT>
size_t enumGetCount(const MapT& mapObj) {
    return std::distance(std::begin(mapObj), std::end(mapObj));
}


/**
 * @brief Returns the name of an enum value, if it had been previously mapped.
 * 
 * @tparam MapT Type of the object containing the enum mappings.
 * @tparam EnumT Enum type.
 * @param mapObj Object containing the enum mappings.
 * @param value Value to check for.
 * @param found If not nullptr, whether the value was found or not
 * is returned here.
 * @return The name, or an empty string if not found.
 */
template<class MapT, class EnumT>
string enumGetName(
    const MapT& mapObj, EnumT value, bool* found = nullptr
) {
    auto it =
        std::find_if(
            std::begin(mapObj), std::end(mapObj),
            [&value] (const typename MapT::value_type& p) {
                return (p.value == value);
            }
        );

    if(it != std::end(mapObj)) {
        if(found) *found = true;
        return it->name;
    }

    if(found) *found = false;
    return "";
}


/**
 * @brief Returns a vector with all the names of an enum,
 * if it had been previously mapped.
 * 
 * @tparam MapT Type of the object containing the enum mappings.
 * @tparam EnumT Enum type.
 * @param mapObj Object containing the enum mappings.
 * @return The name list.
 */
template<class MapT>
vector<string> enumGetNames(const MapT& mapObj) {
    vector<string> result;

    std::for_each(
        std::begin(mapObj), std::end(mapObj),
        [&result] (const typename MapT::value_type& p) {
            result.push_back(p.name);
        }
    );

    return result;
}


/**
 * @brief Returns the value of an enum name, if it had been previously mapped.
 * 
 * @tparam MapT Type of the object containing the enum mappings.
 * @param mapObj Object containing the enum mappings.
 * @param name Name to check for.
 * @param found If not nullptr, whether the name was found or not
 * is returned here.
 * @return The value, or the equivalent of 0 if not found.
 */
template<class MapT>
typename MapT::value_type::SavedEnumT enumGetValue(
    const MapT& mapObj, const std::string& name, bool* found = nullptr
) {
    auto it = std::find_if(
        std::begin(mapObj), std::end(mapObj),
        [&name] (const typename MapT::value_type& p) {
            return (p.name == name);
        }
    );

    if(it != std::end(mapObj)) {
        if(found) *found = true;
        return it->value;
    }
    
    if(found) *found = false;
    return {};
}
