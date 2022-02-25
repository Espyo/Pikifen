/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter type class and converter type-related functions.
 */

#ifndef CONVERTER_TYPE_INCLUDED
#define CONVERTER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "mob_type.h"
#include "pikmin_type.h"


/* Converter object animations.
 * Because converters can have multiple animations that do the same thing,
 * due to the different types, there are actually
 * N_CONVERTER_ANIMS * <number of types> in total.
 * The first bunch of animations are for the first type, the second bunch
 * are for the second type, etc.
 */
enum CONVERTER_ANIMATIONS {
    //Idling.
    CONVERTER_ANIM_IDLING,
    //Bumped against.
    CONVERTER_ANIM_BUMPED,
    //Closing.
    CONVERTER_ANIM_CLOSING,
    //Spitting.
    CONVERTER_ANIM_SPITTING,
    //Opening.
    CONVERTER_ANIM_OPENING,
    //Dying.
    CONVERTER_ANIM_DYING,
    
    //Total amount of converter object animations.
    N_CONVERTER_ANIMS,
};


//Converter object states.
enum CONVERTER_STATES {
    //Idling.
    CONVERTER_STATE_IDLING,
    //Bumped against.
    CONVERTER_STATE_BUMPED,
    //Closing.
    CONVERTER_STATE_CLOSING,
    //Spitting.
    CONVERTER_STATE_SPITTING,
    //Opening.
    CONVERTER_STATE_OPENING,
    //Dying.
    CONVERTER_STATE_DYING,
    
    //Total amount of converter object states.
    N_CONVERTER_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of converter, which is a mob that can convert Pikmin from one type
 * to another.
 */
class converter_type : public mob_type, public mob_type_with_anim_groups {
public:
    vector<pikmin_type*> available_pikmin_types;
    float type_change_interval;
    size_t total_input_pikmin;
    size_t pikmin_per_conversion;
    size_t buffer_size;
    bool same_type_counts_for_output;
    float auto_conversion_timeout;
    size_t max_pikmin_spawn_requirement;
    
    converter_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef CONVERTER_TYPE_INCLUDED
