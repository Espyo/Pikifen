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

enum CONVERTER_ANIMATIONS {
    /* Because converters can have multiple animations that do the same thing,
     * due to the different types, there are actually
     * N_CONVERTER_ANIMS * <number of types> in total.
     * The first bunch of animations are for the first type, the second bunch
     * are for the second type, etc.
     */
    CONVERTER_ANIM_IDLING,
    CONVERTER_ANIM_BUMPING,
    CONVERTER_ANIM_CLOSING,
    CONVERTER_ANIM_SPITTING,
    CONVERTER_ANIM_OPENING,
    CONVERTER_ANIM_DYING,
    
    N_CONVERTER_ANIMS,
};

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
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef CONVERTER_TYPE_INCLUDED
