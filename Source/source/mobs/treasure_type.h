/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the treasure type class and treasure type-related functions.
 */

#ifndef TREASURE_TYPE_INCLUDED
#define TREASURE_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of treasure.
 * Although uncommon, there can be several
 * treasures of the same type at once.
 * Like the "small red marble" treasure
 * type in Pikmin 2; you can see multiple
 * treasures of that type in some
 * Challenge Mode levels.
 */
class treasure_type : public mob_type {
public:

    float value;

    treasure_type();
    void load_from_file(
        data_node* file, const bool load_resources,
        vector<pair<size_t, string> >* anim_conversions
    );
};

#endif //ifndef TREASURE_TYPE_INCLUDED
