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

#include "../game.h"
#include "../mobs/track.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the track category.
 */
track_category::track_category() :
    mob_category(
        MOB_CATEGORY_TRACKS, "Track", "Tracks",
        "Tracks", al_map_rgb(152, 139, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of tracks.
 */
void track_category::clear_types() {
    for(auto &t : game.mob_types.track) {
        delete t.second;
    }
    game.mob_types.track.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a track and adds it to the list of tracks.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* track_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    track* m = new track(pos, (track_type*) type, angle);
    game.states.gameplay->mobs.tracks.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of track.
 */
mob_type* track_category::create_type() {
    return new track_type();
}


/* ----------------------------------------------------------------------------
 * Clears a track from the list of tracks.
 * m:
 *   The mob to erase.
 */
void track_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.tracks.erase(
        find(
        game.states.gameplay->mobs.tracks.begin(), 
        game.states.gameplay->mobs.tracks.end(), 
        (track*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of track given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* track_category::get_type(const string &name) const {
    auto it = game.mob_types.track.find(name);
    if(it == game.mob_types.track.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of track by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void track_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.track) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of track.
 * type:
 *   Mob type to register.
 */
void track_category::register_type(mob_type* type) {
    game.mob_types.track[type->name] = (track_type*) type;
}
