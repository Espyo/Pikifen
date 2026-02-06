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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob_script/track_fsm.h"


/**
 * @brief Constructs a new track type object.
 */
TrackType::TrackType() :
    MobType(MOB_CATEGORY_TRACKS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    TrackFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector TrackType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(DROP_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void TrackType::loadCatProperties(DataNode* file) {
    ReaderSetter tRS(file);
    
    string ridersStr;
    string ridingPoseStr;
    DataNode* ridersNode = nullptr;
    DataNode* ridingPoseNode = nullptr;
    
    tRS.set("cancellable_with_whistle", cancellableWithWhistle);
    tRS.set("ride_speed", rideSpeed);
    tRS.set("riders", ridersStr, &ridersNode);
    tRS.set("riding_pose", ridingPoseStr, &ridingPoseNode);
    
    if(ridersNode) {
        riders = 0;
        vector<string> ridersStrWords = split(ridersStr);
        for(size_t r = 0; r < ridersStrWords.size(); r++) {
            TRACK_RIDER_FLAG rf;
            if(
                readEnumProp(
                    trackRiderFlagINames, ridersStrWords[r], &rf,
                    "rider type", ridersNode
                )
            ) {
                riders |= (Bitmask16) rf;
            }
        }
    }
    
    if(ridingPoseNode) {
        readEnumProp(
            trackRidingPoseINames, ridingPoseStr, &ridingPose,
            "type of riding pose", ridingPoseNode
        );
    }
    
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void TrackType::loadCatResources(DataNode* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check if the track has 2+ checkpoints.
    if(animDb->bodyParts.size() < 2) {
        game.errors.report(
            "The track type \"" + name + "\" needs to have at least 2 "
            "checkpoints (body parts), but it only has " +
            i2s(animDb->bodyParts.size()) + "!"
        );
    }
}
