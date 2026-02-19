/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Track mob category class.
 */

#include <algorithm>

#include "track_category.h"

#include "../../core/game.h"
#include "../mob/track.h"


/**
 * @brief Constructs a new track category object.
 */
TrackCategory::TrackCategory() :
    MobCategory(
        MOB_CATEGORY_TRACKS, "track",
        "Track", "Tracks",
        "tracks", al_map_rgb(152, 139, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of tracks.
 */
void TrackCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.track) {
        delete t.second;
    }
    game.content.mobTypes.list.track.clear();
}


/**
 * @brief Creates a track and adds it to the list of tracks.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* TrackCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Track* m = new Track(pos, (TrackType*) type, angle);
    game.states.gameplay->mobs.tracks.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of track.
 *
 * @return The type.
 */
MobType* TrackCategory::createType() {
    return new TrackType();
}


/**
 * @brief Removes and deletes a track from the list of tracks.
 *
 * @param m The mob to delete.
 */
void TrackCategory::deleteMob(Mob* m) {
    game.states.gameplay->mobs.tracks.erase(
        find(
            game.states.gameplay->mobs.tracks.begin(),
            game.states.gameplay->mobs.tracks.end(),
            (Track*) m
        )
    );
}


/**
 * @brief Returns a type of track given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* TrackCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.track.find(internalName);
    if(it == game.content.mobTypes.list.track.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of track by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void TrackCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.track) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of track.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void TrackCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.track[internalName] = (TrackType*) type;
}
