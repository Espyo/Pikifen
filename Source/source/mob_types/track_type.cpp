/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Track type class and track type-related functions.
 */

#include "track_type.h"

#include "../functions.h"
#include "../mob_fsms/track_fsm.h"
#include "../utils/string_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of track mob.
 */
track_type::track_type() :
    mob_type(MOB_CATEGORY_TRACKS),
    riders(TRACK_RIDER_PIKMIN),
    riding_pose(TRACK_RIDING_POSE_STOPPED),
    ride_speed(0.5f),
    cancellable_with_whistle(false) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    track_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector track_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(DROP_ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void track_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string riders_str;
    string riding_pose_str;
    data_node* riders_node;
    data_node* riding_pose_node;
    
    rs.set("cancellable_with_whistle", cancellable_with_whistle);
    rs.set("ride_speed", ride_speed);
    rs.set("riders", riders_str, &riders_node);
    rs.set("riding_pose", riding_pose_str, &riding_pose_node);
    
    if(riders_node) {
        riders = 0;
        vector<string> riders_str_words = split(riders_str);
        for(size_t r = 0; r < riders_str_words.size(); ++r) {
            if(riders_str_words[r] == "pikmin") {
                riders |= TRACK_RIDER_PIKMIN;
            } else if(riders_str_words[r] == "leaders") {
                riders |= TRACK_RIDER_LEADERS;
            } else {
                log_error(
                    "Unknown type of rider \"" + riders_str_words[r] + "\"!",
                    riders_node
                );
            }
        }
    }
    
    if(riding_pose_node) {
        if(riding_pose_str == "stopped") {
            riding_pose = TRACK_RIDING_POSE_STOPPED;
        } else if(riding_pose_str == "sliding") {
            riding_pose = TRACK_RIDING_POSE_SLIDING;
        } else if(riding_pose_str == "climbing") {
            riding_pose = TRACK_RIDING_POSE_CLIMBING;
        } else {
            log_error(
                "Unknown type of riding pose \"" + riding_pose_str + "\"!",
                riding_pose_node
            );
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void track_type::load_resources(data_node* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check if the track has 2+ checkpoints.
    if(anims.body_parts.size() < 2) {
        log_error(
            "The track type \"" + name + "\" needs to have at least 2 "
            "checkpoints (body parts), but it only has " +
            i2s(anims.body_parts.size()) + "!"
        );
    }
}
