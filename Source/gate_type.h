/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate type class and gate type-related functions.
 */

#ifndef GATE_TYPE_INCLUDED
#define GATE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "data_file.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of gate. Brown, white, blue, whatever.
 */
class gate_type : public mob_type {
private:
    void init_script();
    
public:
    gate_type();
    void load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions);
};

enum GATE_ANIMATIONS {
    GATE_ANIM_IDLE,
    GATE_ANIM_NOTHING,
};

#endif //ifndef GATE_TYPE_INCLUDED