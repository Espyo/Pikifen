/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for Allegro-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#ifndef ALLEGRO_UTILS_INCLUDED
#define ALLEGRO_UTILS_INCLUDED

#include <allegro5/allegro.h>


bool operator==(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);
bool operator!=(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);

#endif //ifndef ALLEGRO_UTILS_INCLUDED