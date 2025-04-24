/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bouncer type class and bouncer type-related functions.
 */

#include "bouncer_type.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob_script/bouncer_fsm.h"


/**
 * @brief Constructs a new bouncer type object.
 */
BouncerType::BouncerType() :
    MobType(MOB_CATEGORY_BOUNCERS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    walkable = true;
    
    areaEditorTips =
        "Link this object to another object, so that "
        "bounced Pikmin land in that location. "
        "A \"Dummy\" object works perfectly for this.";
        
    BouncerFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
AnimConversionVector BouncerType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(BOUNCER_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(BOUNCER_ANIM_BOUNCING, "bouncing"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void BouncerType::loadCatProperties(DataNode* file) {
    ReaderSetter bRS(file);
    
    string ridersStr;
    string ridingPoseStr;
    DataNode* ridersNode = nullptr;
    DataNode* ridingPoseNode = nullptr;
    
    bRS.set("riders", ridersStr, &ridersNode);
    bRS.set("riding_pose", ridingPoseStr, &ridingPoseNode);
    
    if(ridersNode) {
        riders = 0;
        vector<string> ridersStrWords = split(ridersStr);
        for(size_t r = 0; r < ridersStrWords.size(); r++) {
            if(ridersStrWords[r] == "pikmin") {
                enableFlag(riders, BOUNCER_RIDER_FLAG_PIKMIN);
            } else if(ridersStrWords[r] == "leaders") {
                enableFlag(riders, BOUNCER_RIDER_FLAG_LEADERS);
            } else {
                game.errors.report(
                    "Unknown type of rider \"" + ridersStrWords[r] + "\"!",
                    ridersNode
                );
            }
        }
    }
    
    if(ridingPoseNode) {
        if(ridingPoseStr == "stopped") {
            ridingPose = BOUNCER_RIDING_POSE_STOPPED;
        } else if(ridingPoseStr == "somersault") {
            ridingPose = BOUNCER_RIDING_POSE_SOMERSAULT;
        } else {
            game.errors.report(
                "Unknown type of riding pose \"" + ridingPoseStr + "\"!",
                ridingPoseNode
            );
        }
    }
    
}
