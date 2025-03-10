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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/converter.h"


/**
 * @brief Creates the finite state machine for the converter's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void converter_fsm::create_fsm(MobType* typ) {
    EasyFsmCreator efc;
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
void converter_fsm::become_idle(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
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
void converter_fsm::bumped(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
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
void converter_fsm::finish_being_bumped(Mob* m, void* info1, void* info2) {
    ((Converter*) m)->close();
}


/**
 * @brief Makes the converter vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::finish_dying(Mob* m, void* info1, void* info2) {
    m->to_delete = true;
}


/**
 * @brief Handles an object bumping against it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::handle_object_touch(Mob* m, void* info1, void* info2) {
    Mob* bumper = (Mob*) info1;
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
void converter_fsm::handle_pikmin(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    Pikmin* pik_ptr = (Pikmin*) info1;
    
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
    
    ParticleGenerator pg =
        standard_particle_gen_setup(
            game.sys_content_names.part_converter_insertion, m
        );
    m->particle_generators.push_back(pg);
}


/**
 * @brief Makes the converter open up.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void converter_fsm::open(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
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
void converter_fsm::open_or_die(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
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
void converter_fsm::open_or_spit(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
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
void converter_fsm::spew(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
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
void converter_fsm::start_dying(Mob* m, void* info1, void* info2) {
    Converter* con_ptr = (Converter*) m;
    
    con_ptr->set_animation(
        con_ptr->get_animation_idx_from_base_and_group(
            CONVERTER_ANIM_DYING, N_CONVERTER_ANIMS, con_ptr->current_type_idx
        )
    );
    con_ptr->cur_base_anim_idx = CONVERTER_ANIM_DYING;
}
