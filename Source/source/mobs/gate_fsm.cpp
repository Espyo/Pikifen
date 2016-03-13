/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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

void gate_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idle", GATE_STATE_IDLE); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gate_fsm::set_anim);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(gate_fsm::take_damage);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run_function(gate_fsm::open);
            efc.change_state("dead");
        }
    }
    
    efc.new_state("dead", GATE_STATE_DEAD); {
    
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle");
    
    if(typ->states.size() != N_GATE_STATES) {
        error_log(
            "ENGINE WARNING: Number of gate states on the FSM (" + i2s(typ->states.size()) +
            ") and the enum (" + i2s(N_GATE_STATES) + ") do not match."
        );
    }
}

void gate_fsm::open(mob* m, void* info1, void* info2) {
    gate* g_ptr = (gate*) m;
    g_ptr->sec->type = SECTOR_TYPE_NORMAL;
    m->set_animation(GATE_ANIM_NOTHING);
    m->start_dying();
    m->finish_dying();
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke, g_ptr->x, g_ptr->y,
        60, 90, 10, 12, 2.5, 3, 64, 96, al_map_rgb(238, 204, 170)
    );
}

void gate_fsm::take_damage(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    m->health -= damage;
}

void gate_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(GATE_ANIM_IDLE);
}
