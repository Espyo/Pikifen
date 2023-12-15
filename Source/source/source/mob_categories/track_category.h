/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track mob category class.
 */

#ifndef TRACK_CATEGORY_INCLUDED
#define TRACK_CATEGORY_INCLUDED

#include "../const.h"
#include "../mob_categories/mob_category.h"


using std::string;
using std::vector;


/* ----------------------------------------------------------------------------
 * Mob category for anything that transports a mob from point A to point B
 * in a more-or-less linear fashion.
 */
class track_category : public mob_category {
public:
    void get_type_names(vector<string> &list) const override;
    mob_type* get_type(const string &name) const override;
    mob_type* create_type() override;
    void register_type(mob_type* type) override;
    mob* create_mob(
        const point &pos, mob_type* type, const float angle
    ) override;
    void erase_mob(mob* m) override;
    void clear_types() override;
    
    track_category();
};


#endif //ifndef TRACK_CATEGORY_INCLUDED
