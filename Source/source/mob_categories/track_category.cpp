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

#include "../mobs/track.h"
#include "../vars.h"


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
 * Returns all types of track by name.
 */
void track_category::get_type_names(vector<string> &list) {
    for(auto &t : track_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of track given its name, or NULL on error.
 */
mob_type* track_category::get_type(const string &name) {
    auto it = track_types.find(name);
    if(it == track_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of track.
 */
mob_type* track_category::create_type() {
    return new track_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of track.
 */
void track_category::register_type(mob_type* type) {
    track_types[type->name] = (track_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a track and adds it to the list of tracks.
 */
mob* track_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    track* m = new track(pos, (track_type*) type, angle);
    tracks.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a track from the list of tracks.
 */
void track_category::erase_mob(mob* m) {
    tracks.erase(
        find(tracks.begin(), tracks.end(), (track*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of tracks.
 */
void track_category::clear_types() {
    for(auto &t : track_types) {
        delete t.second;
    }
    track_types.clear();
}


track_category::~track_category() { }
