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
#include "../game.h"
#include "../mob_fsms/track_fsm.h"
#include "../utils/string_utils.h"


/**
 * @brief Constructs a new track type object.
 */
track_type::track_type() :
    mob_type(MOB_CATEGORY_TRACKS) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    track_fsm::create_fsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector track_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(DROP_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void track_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string riders_str;
    string riding_pose_str;
    data_node* riders_node = nullptr;
    data_node* riding_pose_node = nullptr;
    
    rs.set("cancellable_with_whistle", cancellable_with_whistle);
    rs.set("ride_speed", ride_speed);
    rs.set("riders", riders_str, &riders_node);
    rs.set("riding_pose", riding_pose_str, &riding_pose_node);
    
    if(riders_node) {
        riders = 0;
        vector<string> riders_str_words = split(riders_str);
        for(size_t r = 0; r < riders_str_words.size(); ++r) {
            if(riders_str_words[r] == "pikmin") {
                enable_flag(riders, TRACK_RIDER_PIKMIN);
            } else if(riders_str_words[r] == "leaders") {
                enable_flag(riders, TRACK_RIDER_LEADERS);
            } else {
                game.errors.report(
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
            game.errors.report(
                "Unknown type of riding pose \"" + riding_pose_str + "\"!",
                riding_pose_node
            );
        }
    }
    
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void track_type::load_resources(data_node* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check if the track has 2+ checkpoints.
    if(anims.body_parts.size() < 2) {
        game.errors.report(
            "The track type \"" + name + "\" needs to have at least 2 "
            "checkpoints (body parts), but it only has " +
            i2s(anims.body_parts.size()) + "!"
        );
    }
}
