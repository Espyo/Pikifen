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
    
    target_type = MOB_TARGET_FLAG_NONE;
    walkable = true;
    
    area_editor_tips =
        "Link this object to another object, so that "
        "bounced Pikmin land in that location. "
        "A \"Dummy\" object works perfectly for this.";
        
    bouncer_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
anim_conversion_vector BouncerType::getAnimConversions() const {
    anim_conversion_vector v;
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
    ReaderSetter rs(file);
    
    string riders_str;
    string riding_pose_str;
    DataNode* riders_node = nullptr;
    DataNode* riding_pose_node = nullptr;
    
    rs.set("riders", riders_str, &riders_node);
    rs.set("riding_pose", riding_pose_str, &riding_pose_node);
    
    if(riders_node) {
        riders = 0;
        vector<string> riders_str_words = split(riders_str);
        for(size_t r = 0; r < riders_str_words.size(); r++) {
            if(riders_str_words[r] == "pikmin") {
                enableFlag(riders, BOUNCER_RIDER_FLAG_PIKMIN);
            } else if(riders_str_words[r] == "leaders") {
                enableFlag(riders, BOUNCER_RIDER_FLAG_LEADERS);
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
            riding_pose = BOUNCER_RIDING_POSE_STOPPED;
        } else if(riding_pose_str == "somersault") {
            riding_pose = BOUNCER_RIDING_POSE_SOMERSAULT;
        } else {
            game.errors.report(
                "Unknown type of riding pose \"" + riding_pose_str + "\"!",
                riding_pose_node
            );
        }
    }
    
}
