/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter finite state machine logic.
 */

#include "converter_fsm.h"

#include "../functions.h"
#include "../game.h"
#include "../mobs/converter.h"
#include "../utils/string_utils.h"


/**
 * @brief Creates the finite state machine for the converter's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void converter_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", CONVERTER_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::become_idle);
        }
        efc.new_event(MOB_EV_THROWN_PIKMIN_LANDED); {
            efc.run(converter_fsm::handle_pikmin);
        }
        efc.new_event(MOB_EV_TOUCHED_OBJECT); {
            efc.run(converter_fsm::handle_object_touch);
        }
    }
    
    efc.new_state("bumped", CONVERTER_STATE_BUMPED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::bumped);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finish_being_bumped);
            efc.change_state("closing");
        }
    }
    
    efc.new_state("closing", CONVERTER_STATE_CLOSING); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::open_or_spit);
        }
    }
    
    efc.new_state("spitting", CONVERTER_STATE_SPITTING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::spew);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::open_or_die);
        }
    }
    
    efc.new_state("opening", CONVERTER_STATE_OPENING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::open);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    efc.new_state("dying", CONVERTER_STATE_DYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(converter_fsm::start_dying);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(converter_fsm::finish_dying);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_CONVERTER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_CONVERTER_STATES) + " in enum."
    );
}


/**
 * @brief Enters the idle state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::become_idle(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_IDLING, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        ),
        START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_IDLING;
    con_ptr->type_change_timer.start();
}


/**
 * @brief Does a little bumpy animation after a leader touches it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::bumped(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_BUMPED, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        )
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_BUMPED;
    con_ptr->type_change_timer.stop();
    con_ptr->auto_conversion_timer.stop();
}


/**
 * @brief Makes the converter close after it gets bumped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::finish_being_bumped(mob* m, void* info1, void* info2) {
    ((converter*) m)->close();
}


/**
 * @brief Makes the converter vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::finish_dying(mob* m, void* info1, void* info2) {
    m->to_delete = true;
}


/**
 * @brief Handles an object bumping against it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::handle_object_touch(mob* m, void* info1, void* info2) {
    mob* bumper = (mob*) info1;
    if(bumper->type->category->id == MOB_CATEGORY_LEADERS) {
        m->fsm.set_state(CONVERTER_STATE_BUMPED);
    }
}


/**
 * @brief Code to handle a Pikmin having been thrown inside.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::handle_pikmin(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    pikmin* pik_ptr = (pikmin*) info1;
    
    if(con_ptr->amount_in_buffer == con_ptr->con_type->buffer_size) {
        //A Pikmin tried to sneak in in the middle of a conversion! Denied.
        return;
    }
    
    con_ptr->amount_in_buffer++;
    if(
        con_ptr->con_type->same_type_counts_for_output ||
        pik_ptr->pik_type != con_ptr->current_type
    ) {
        con_ptr->input_pikmin_left--;
    }
    con_ptr->type_change_timer.stop();
    con_ptr->auto_conversion_timer.start();
    
    pik_ptr->to_delete = true;
    
    if(
        con_ptr->input_pikmin_left == 0 ||
        con_ptr->amount_in_buffer == con_ptr->con_type->buffer_size
    ) {
        con_ptr->close();
    }
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height + 1.0,
        24, 1.5, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = game.sys_assets.bmp_smoke;
    p.velocity.x = 70;
    particle_generator pg(0, p, 15);
    pg.emission.number_deviation = 5;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.speed_deviation.x = 10;
    pg.duration_deviation = 0.5;
    pg.emit(game.states.gameplay->particles);
}


/**
 * @brief Makes the converter open up.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::open(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_OPENING, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        )
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_OPENING;
}


/**
 * @brief Changes to the opening state or the dying state, depending
 * on whether it can still output Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::open_or_die(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    if(con_ptr->input_pikmin_left == 0) {
        con_ptr->fsm.set_state(CONVERTER_STATE_DYING);
    } else {
        con_ptr->fsm.set_state(CONVERTER_STATE_OPENING);
    }
}


/**
 * @brief Changes to the opening state or the spitting state, depending
 * on whether it has Pikmin in the buffer or not.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::open_or_spit(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    if(con_ptr->amount_in_buffer == 0) {
        con_ptr->fsm.set_state(CONVERTER_STATE_OPENING);
    } else {
        con_ptr->fsm.set_state(CONVERTER_STATE_SPITTING);
    }
}


/**
 * @brief Spews out the converted seeds.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::spew(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_SPITTING, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        )
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_SPITTING;
    con_ptr->spew();
}


/**
 * @brief Makes the converter start dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::start_dying(mob* m, void* info1, void* info2) {
    converter* con_ptr = (converter*) m;
    
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_DYING, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        )
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_DYING;
}
