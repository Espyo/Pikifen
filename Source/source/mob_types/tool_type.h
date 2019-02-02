/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the tool type class and tool type-related functions.
 */

#ifndef TOOL_TYPE_INCLUDED
#define TOOL_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of tool. A type of hand-held explosive, for instance.
 */
class tool_type : public mob_type {
public:

    bool can_be_hotswapped;
    bool dropped_when_pikmin_lands;
    bool dropped_when_pikmin_lands_on_mob;
    
    tool_type();
    ~tool_type();
    void load_parameters(data_node* file);
};

#endif //ifndef TOOL_TYPE_INCLUDED