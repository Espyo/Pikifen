/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter class and converter-related functions.
 */

#ifndef CONVERTER_INCLUDED
#define CONVERTER_INCLUDED

#include "../mob_types/converter_type.h"
#include "mob.h"


enum CONVERTER_STATES {
    CONVERTER_STATE_IDLING,
    CONVERTER_STATE_BUMPING,
    CONVERTER_STATE_CLOSING,
    CONVERTER_STATE_SPITTING,
    CONVERTER_STATE_OPENING,
    CONVERTER_STATE_DYING,
    
    N_CONVERTER_STATES,
};


/* ----------------------------------------------------------------------------
 * A converter mob. This is like the Candypop Buds in the canon games, in the
 * sense that it converts a thrown Pikmin from one type into a Pikmin
 * from a different type.
 */
class converter : public mob, public mob_with_anim_groups {
public:
    converter_type* con_type;
    
    size_t amount_in_buffer;
    size_t input_pikmin_left;
    pikmin_type* current_type;
    size_t current_type_nr;
    timer type_change_timer;
    timer auto_conversion_timer;
    float next_spew_angle;
    
    void change_type();
    void close();
    void spew();
    
    converter(const point &pos, converter_type* con_type, const float angle);
    virtual void tick_class_specifics();
    
};

#endif //ifndef CONVERTER_INCLUDED
