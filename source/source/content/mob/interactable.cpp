/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interactable class and interactable-related functions.
 */

#include "interactable.h"


/**
 * @brief Constructs a new interactable object.
 *
 * @param pos Starting coordinates.
 * @param type Interactable type this mob belongs to.
 * @param angle Starting angle.
 */
Interactable::Interactable(
    const Point& pos, InteractableType* type, float angle
):
    Mob(pos, type, angle),
    intType(type) {
    
}
