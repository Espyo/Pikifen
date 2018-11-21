/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
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
    
    N_CONVERTER_STATES,
};


/* ----------------------------------------------------------------------------
 * A converter mob. This is like the Candypop Buds in the canon games, in the
 * sense that it converts a thrown Pikmin from one type into a Pikmin
 * from a different type.
 */
class converter : public mob {
public:
    converter_type* con_type;
    
    converter(const point &pos, converter_type* con_type, const float angle);
    
};

#endif //ifndef CONVERTER_INCLUDED
