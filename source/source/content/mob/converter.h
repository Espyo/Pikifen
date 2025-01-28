/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter class and converter-related functions.
 */

#pragma once

#include "../mob_type/converter_type.h"
#include "mob.h"

namespace CONVERTER {
extern const float NEW_SEED_Z_OFFSET;
extern const float SPEW_ANGLE_SHIFT;
extern const float SPEW_H_SPEED;
extern const float SPEW_H_SPEED_DEVIATION;
extern const float SPEW_V_SPEED;
}


/**
 * @brief A converter mob.
 *
 * This is like the Candypop Buds in the canon games, in the sense that it
 * converts a thrown Pikmin from one type into a Pikmin from a different type.
 */
class converter : public mob, public mob_with_anim_groups {

public:

    //--- Members ---
    
    //What type of converter it is.
    converter_type* con_type = nullptr;
    
    //Amount of Pikmin currently inside the converter, in its "buffer".
    size_t amount_in_buffer = 0;
    
    //How many Pikmin are left until the converter dies.
    size_t input_pikmin_left = 0;
    
    //Type of Pikmin it will convert to right now.
    pikmin_type* current_type = nullptr;
    
    //If it cycles between types, this is the index of the current type.
    size_t current_type_idx = 0;
    
    //Time left before it cycles to the next type.
    timer type_change_timer;
    
    //Time left before it auto-closes and converts the Pikmin in the buffer.
    timer auto_conversion_timer;
    
    //Angle it will spit the next seed towards.
    float next_spew_angle = 0.0f;
    
    
    //--- Function declarations ---
    
    converter(const point &pos, converter_type* con_type, float angle);
    void change_type();
    void close();
    void spew();
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(float delta_t) override;
    
};
