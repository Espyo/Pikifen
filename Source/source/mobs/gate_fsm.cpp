/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate finite state machine logic.
 */

#include "../functions.h"
#include "gate.h"
#include "gate_fsm.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the gate's logic.
 */
void gate_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", GATE_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gate_fsm::set_anim);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(gate_fsm::take_damage);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run_function(gate_fsm::open);
            efc.change_state("destroyed");
        }
    }
    
    efc.new_state("destroyed", GATE_STATE_DESTROYED); {
    
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    if(typ->states.size() != N_GATE_STATES) {
        log_error(
            "ENGINE WARNING: Number of gate states on the FSM (" +
            i2s(typ->states.size()) +
            ") and the enum (" + i2s(N_GATE_STATES) + ") do not match."
        );
    }
}


/* ----------------------------------------------------------------------------
 * When a gate is opened. This creates the particle explosion,
 * changes the associated sector, etc.
 */
void gate_fsm::open(mob* m, void* info1, void* info2) {
    gate* g_ptr = (gate*) m;
    g_ptr->sec->type = SECTOR_TYPE_NORMAL;
    m->set_animation(GATE_ANIM_DESTROYED);
    m->start_dying();
    m->finish_dying();
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->x, m->y,
        80, 2.75, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = bmp_smoke;
    p.color = al_map_rgb(238, 204, 170);
    particle_generator pg(0, p, 11);
    pg.number_deviation = 1;
    pg.size_deviation = 16;
    pg.angle = 0;
    pg.angle_deviation = M_PI;
    pg.speed = 75;
    pg.speed_deviation = 15;
    pg.duration_deviation = 0.25;
    pg.emit(particles);
}


/* ----------------------------------------------------------------------------
 * When a gate takes damage, based on the Pikmin, hitboxes, etc.
 */
void gate_fsm::take_damage(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = calculate_damage(info->mob2, m, info->h2, info->h1);
    m->health -= damage;
}


/* ----------------------------------------------------------------------------
 * When a gate needs to enter its default "idling" animation.
 */
void gate_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(GATE_ANIM_IDLING);
}
