/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile class and pile-related functions.
 */

#include "pile.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"
#include "resource.h"

/* ----------------------------------------------------------------------------
 * Creates a pile.
 */
pile::pile(const point &pos, pile_type* type, const float angle) :
    mob(pos, type, angle),
    pil_type(type),
    amount(type->max_amount) {
    
    team = MOB_TEAM_OBSTACLE;
    
    recharge_timer =
    timer(pil_type->recharge_interval, [this] () { this->recharge(); });
}


/* ----------------------------------------------------------------------------
 * Changes the amount in the pile, and updates the appropriate variables.
 */
void pile::change_amount(const int change) {
    if(change < 0 && amount == 0) return;
    if(change > 0 && amount == pil_type->max_amount) return;
    
    amount += change;
    set_health(true, false, change * pil_type->health_per_resource);
    
    update();
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void pile::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    size_t amount_var;
    
    if(svr.get("amount", amount_var)) {
        amount = amount_var;
        amount = clamp(amount, 0, pil_type->max_amount);
    }
    
    health = pil_type->health_per_resource * amount;
    update();
}


/* ----------------------------------------------------------------------------
 * Adds some more to the pile from a periodic recharge.
 */
void pile::recharge() {
    recharge_timer.start();
    change_amount(pil_type->recharge_amount);
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to piles.
 */
void pile::tick_class_specifics(const float delta_t) {
    recharge_timer.tick(delta_t);
    
    if(amount == 0 && pil_type->delete_when_finished) {
        //Ready to delete. Unless it's being used, that is.
        
        for(size_t r = 0; r < game.states.gameplay_st->mobs.resources.size(); ++r) {
            if(game.states.gameplay_st->mobs.resources[r]->origin_pile == this) {
                return;
            }
        }
        
        to_delete = true;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the animation to the right one, the recharge timer, and
 * some other things.
 */
void pile::update() {
    amount = clamp(amount, 0, pil_type->max_amount);
    
    if(amount == pil_type->max_amount) {
        recharge_timer.stop();
    }
    
    size_t anim_amount_nr = 0;
    size_t n_groups = pil_type->animation_group_suffixes.size();
    if(n_groups > 1 && amount > 0) {
        anim_amount_nr =
            ceil(
                (n_groups - 1) *
                ((float) amount / (float) pil_type->max_amount)
            );
        anim_amount_nr = clamp(anim_amount_nr, 0, n_groups - 1);
    }
    set_animation(
        get_animation_nr_from_base_and_group(
            PILE_ANIM_IDLING, N_PILE_ANIMS, anim_amount_nr
        ),
        true, false
    );
    
    if(pil_type->hide_when_empty) {
        if(amount == 0) {
            hide = true;
            tangible = false;
        } else {
            hide = false;
            tangible = true;
        }
    }
}
