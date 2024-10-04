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
#include "../utils/general_utils.h"
#include "../utils/string_utils.h"
#include "resource.h"


/**
 * @brief Constructs a new pile object.
 *
 * @param pos Starting coordinates.
 * @param type Pile type this mob belongs to.
 * @param angle Starting angle.
 */
pile::pile(const point &pos, pile_type* type, float angle) :
    mob(pos, type, angle),
    pil_type(type),
    amount(type->max_amount) {
    
    team = MOB_TEAM_OBSTACLE;
    
    recharge_timer =
    timer(pil_type->recharge_interval, [this] () { this->recharge(); });
}


/**
 * @brief Changes the amount in the pile, and updates the appropriate variables.
 *
 * @param change Amount to increase by.
 */
void pile::change_amount(int change) {
    if(change < 0 && amount == 0) return;
    if(change > 0 && amount == pil_type->max_amount) return;
    
    amount += change;
    set_health(true, false, change * pil_type->health_per_resource);
    
    update();
}


/**
 * @brief Returns information on how to show the fraction numbers.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 *
 * @param fraction_value_nr The fraction's value (upper) number gets set here.
 * @param fraction_req_nr The fraction's required (lower) number gets set here.
 * @param fraction_color The fraction's color gets set here.
 * @return Whether the numbers should be shown.
 */
bool pile::get_fraction_numbers_info(
    float* fraction_value_nr, float* fraction_req_nr,
    ALLEGRO_COLOR* fraction_color
) const {
    if(amount == 0 || !pil_type->show_amount) return false;
    *fraction_value_nr = amount;
    *fraction_req_nr = 0;
    *fraction_color = game.config.carrying_color_stop;
    return true;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void pile::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    size_t amount_var;
    
    if(svr.get("amount", amount_var)) {
        amount = amount_var;
        amount = clamp(amount, 0, pil_type->max_amount);
    }
    
    health = pil_type->health_per_resource * amount;
    max_health = health;
    update();
}


/**
 * @brief Adds some more to the pile from a periodic recharge.
 */
void pile::recharge() {
    recharge_timer.start();
    change_amount(pil_type->recharge_amount);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void pile::tick_class_specifics(float delta_t) {
    recharge_timer.tick(delta_t);
    
    if(amount == 0 && pil_type->delete_when_finished) {
        //Ready to delete. Unless it's being used, that is.
        
        for(
            size_t r = 0;
            r < game.states.gameplay->mobs.resources.size(); ++r
        ) {
            resource* r_ptr = game.states.gameplay->mobs.resources[r];
            if(r_ptr->origin_pile == this) {
                return;
            }
        }
        
        to_delete = true;
    }
}


/**
 * @brief Updates the animation to the right one, the recharge timer, and
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
        get_animation_idx_from_base_and_group(
            PILE_ANIM_IDLING, N_PILE_ANIMS, anim_amount_nr
        ),
        START_ANIM_OPTION_NO_RESTART, true
    );
    
    if(pil_type->auto_shrink_smallest_radius != 0.0f) {
        set_radius(
            interpolate_number(
                amount, 1, pil_type->max_amount,
                pil_type->auto_shrink_smallest_radius, pil_type->radius
            )
        );
    }
    
    if(pil_type->hide_when_empty) {
        if(amount == 0) {
            enable_flag(flags, MOB_FLAG_HIDDEN);
            enable_flag(flags, MOB_FLAG_INTANGIBLE);
        } else {
            disable_flag(flags, MOB_FLAG_HIDDEN);
            disable_flag(flags, MOB_FLAG_INTANGIBLE);
        }
    }
}
