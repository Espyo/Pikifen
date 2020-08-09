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

#include "../functions.h"
#include "../mob_fsms/bouncer_fsm.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of bouncer mob.
 */
bouncer_type::bouncer_type() :
    mob_type(MOB_CATEGORY_BOUNCERS),
    riders(BOUNCER_RIDER_PIKMIN),
    riding_pose(BOUNCER_RIDING_POSE_STOPPED) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    walkable = true;
    
    area_editor_tips =
        "Link this object to another object, so that\n"
        "bounced Pikmin land in that location.\n"
        "A \"Dummy\" object works perfectly for this.";
        
    bouncer_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector bouncer_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(BOUNCER_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(BOUNCER_ANIM_BOUNCING, "bouncing"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void bouncer_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string riders_str;
    string riding_pose_str;
    data_node* riders_node = NULL;
    data_node* riding_pose_node = NULL;
    
    rs.set("riders", riders_str, &riders_node);
    rs.set("riding_pose", riding_pose_str, &riding_pose_node);
    
    if(riders_node) {
        riders = 0;
        vector<string> riders_str_words = split(riders_str);
        for(size_t r = 0; r < riders_str_words.size(); ++r) {
            if(riders_str_words[r] == "pikmin") {
                riders |= BOUNCER_RIDER_PIKMIN;
            } else if(riders_str_words[r] == "leaders") {
                riders |= BOUNCER_RIDER_LEADERS;
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
            riding_pose = BOUNCER_RIDING_POSE_STOPPED;
        } else if(riding_pose_str == "somersault") {
            riding_pose = BOUNCER_RIDING_POSE_SOMERSAULT;
        } else {
            log_error(
                "Unknown type of riding pose \"" + riding_pose_str + "\"!",
                riding_pose_node
            );
        }
    }
    
}
