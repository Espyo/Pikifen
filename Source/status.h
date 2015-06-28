/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the status effect class and status effect-related functions.
 */

#ifndef STATUS_INCLUDED
#define STATUS_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

/* ----------------------------------------------------------------------------
 * A status effect. Any mob under the influence
 * of a status effect will suffer or benefit
 * from changes in some of its values.
 * Some effects could increase the speed,
 * others could decrease attack power.
 * Others can even slowly kill the mob
 * unless they're cleared out, like
 * Pikmin on fire or drowning.
 */
class status {
public:
    float speed_multiplier;
    float attack_multiplier;
    float defense_multiplier;
    bool freezes_everything;
    ALLEGRO_COLOR color;
    unsigned char affects;      //What kind of mobs it affects.
    
    status(const float speed_multiplier, const float attack_multiplier, const float defense_multiplier, const bool freezes_everything, const ALLEGRO_COLOR &color, const unsigned char affects);
};



enum {
    STATUS_AFFECTS_PIKMIN = 1,
    STATUS_AFFECTS_ENEMIES = 2,
    STATUS_AFFECTS_LEADERS = 4,
    STATUS_AFFECTS_ENEMY_PIKMIN = 8,
    STATUS_AFFECTS_HAZARDS = 16,
};

#endif //ifndef STATUS_INCLUDED
