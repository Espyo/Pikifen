/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Container structure utilities used throughout the project.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <algorithm>
#include <vector>

using std::vector;


/**
 * @brief Shorthand for figuring out if a given item is in a container.
 *
 * @tparam ContainerT Type of container.
 * @tparam ContentT Type of contents of the container.
 * @param cont The container.
 * @param item Item to check.
 * @return Whether it contains the item.
 */
template<typename ContainerT, typename ContentT>
bool isInContainer(const ContainerT& cont, const ContentT& item) {
    return std::find(cont.begin(), cont.end(), item) != cont.end();
}


/**
 * @brief Removes elements from a vector if they show up in the ban list.
 *
 * @tparam ContentT Type of contents of the vector and ban list.
 * @param v Vector to filter.
 * @param banList List of items that must be banned.
 * @return The filtered vector.
 */
template<typename ContentT>
vector<ContentT> filterVectorWithBanList(
    const vector<ContentT>& v, const vector<ContentT>& banList
) {
    vector<ContentT> result = v;
    for(size_t i = 0; i < result.size();) {
        if(isInContainer(banList, result[i])) {
            result.erase(result.begin() + i);
        } else {
            i++;
        }
    }
    return result;
}


/**
 * @brief Returns the cyclically next element in a vector, given the
 * current element. If the element is not in the vector, it returns the
 * first element.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector.
 * @param e The current element.
 * @return The next element, or a default-constructed element
 * if the vector is empty.
 */
template<typename ContentT>
ContentT getNextInVector(const vector<ContentT>& v, const ContentT& e) {
    if(v.empty()) return ContentT();
    auto it = std::find(v.begin(), v.end(), e);
    size_t idx =
        it != v.end() ?
        std::distance(v.begin(), it) :
        v.size() - 1;
    return getNextInVectorByIdx(v, idx);
}


/**
 * @brief Returns the cyclically next element in a vector, given the
 * current element's index.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector.
 * @param idx The current element's index.
 * @return The next element, or a default-constructed element
 * if the vector is empty.
 */
template<typename ContentT>
ContentT getNextInVectorByIdx(const vector<ContentT>& v, size_t idx) {
    if(v.empty()) return ContentT();
    return v[idx == v.size() - 1 ? 0 : idx + 1];
}


/**
 * @brief Returns the cyclically previous element in a vector, given the
 * current element. If the element is not in the vector, it returns the
 * last element.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector.
 * @param e The current element.
 * @return The previous element, or a default-constructed element
 * if the vector is empty.
 */
template<typename ContentT>
ContentT getPrevInVector(const vector<ContentT>& v, const ContentT& e) {
    if(v.empty()) return ContentT();
    auto it = std::find(v.begin(), v.end(), e);
    size_t idx =
        it != v.end() ?
        std::distance(v.begin(), it) :
        0;
    return getPrevInVectorByIdx(v, idx);
}


/**
 * @brief Returns the cyclically previous element in a vector, given the
 * current element's index.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector.
 * @param idx The current element's index.
 * @return The previous element, or a default-constructed element
 * if the vector is empty.
 */
template<typename ContentT>
ContentT getPrevInVectorByIdx(const vector<ContentT>& v, size_t idx) {
    if(v.empty()) return ContentT();
    return v[idx == 0 ? v.size() - 1 : idx - 1];
}


/**
 * @brief Shorthand for figuring out if a given key is in a map.
 *
 * @tparam MapT Type of map.
 * @tparam KeyT Type of keys of the map.
 * @param m The map.
 * @param key Key to check.
 * @return Whether it contains the item.
 */
template<typename MapT, typename KeyT>
bool isInMap(const MapT& cont, const KeyT& key) {
    return cont.find(key) != cont.end();
}


/**
 * @brief Returns whether one container is a permutation of another. In other
 * words, if both containers contain the exact same things, even if they
 * are in a different order.
 *
 * @tparam Container1T Type of the first container.
 * @tparam Container2T Type of the second container.
 * @param cont1 First container.
 * @param cont2 Second container.
 * @return Whether cont2 is a permutation of cont1.
 */
template<class Container1T, class Container2T>
bool isPermutation(const Container1T& cont1, const Container2T& cont2) {
    if(cont1.size() != cont2.size()) return false;
    return std::is_permutation(cont1.begin(), cont1.end(), cont2.begin());
}


/**
 * @brief Deterministically randomly shuffles the contents of a vector.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v The vector to shuffle.
 * @param pickRandomFloats Vector of previously-determined random floats to
 * calculate the picks with [0, 1]. This vector must be of the same size
 * as the input vector.
 * @return The shuffled vector.
 */
template<typename ContentT>
vector<ContentT> shuffleVector(
    const vector<ContentT>& v, const vector<float>& pickRandomFloats
) {
    vector<ContentT> result;
    vector<ContentT> itemsAvailable = v;
    for(size_t i = 0; i < v.size(); i++) {
        size_t pick = pickRandomFloats[i] * (itemsAvailable.size());
        //Add a safeguard for if the float is exactly 1.0.
        pick = std::min(pick, itemsAvailable.size() - 1);
        result.push_back(itemsAvailable[pick]);
        itemsAvailable.erase(itemsAvailable.begin() + pick);
    }
    return result;
}


/**
 * @brief Sorts a vector, using the preference list to figure out which
 * elements go before which. Elements not in the preference list will go
 * to the end, in the same order as they are presented in the original
 * vector.
 *
 * @tparam ContentT Type of contents of the vector.
 * @param v Vector to sort.
 * @param preferenceList Preference list.
 * @param equal If not nullptr, use this function to compare whether
 * an item of t1 matches an item of t2.
 * @param less If not nullptr, use this function to sort missing items with.
 * @param unknowns If not nullptr, unknown preferences (i.e. items in
 * the preference list but not inside the vector) will be added here.
 * @return The sorted vector.
 */
template<typename ContentT>
vector<ContentT> sortVectorWithPreferenceList(
    const vector<ContentT>& v, const vector<ContentT>& preferenceList,
    vector<ContentT>* unknowns = nullptr
) {
    vector<ContentT> result;
    vector<ContentT> missingItems;
    result.reserve(v.size());
    
    //Sort the existing items.
    for(size_t p = 0; p < preferenceList.size(); p++) {
        bool foundInVector = false;
        for(auto& i : v) {
            if(i == preferenceList[p]) {
                foundInVector = true;
                result.push_back(i);
                break;
            }
        }
        if(!foundInVector && unknowns) {
            unknowns->push_back(preferenceList[p]);
        }
    }
    
    //Find the missing items.
    for(auto& i : v) {
        bool foundInPreferences = false;
        for(auto& p : preferenceList) {
            if(i == p) {
                foundInPreferences = true;
                break;
            }
        }
        if(!foundInPreferences) {
            //Missing from the list? Add it to the "missing" pile.
            missingItems.push_back(i);
        }
    }
    
    //Sort and place the missing items.
    if(!missingItems.empty()) {
        std::sort(
            missingItems.begin(),
            missingItems.end()
        );
        result.insert(
            result.end(),
            missingItems.begin(),
            missingItems.end()
        );
    }
    
    return result;
}
