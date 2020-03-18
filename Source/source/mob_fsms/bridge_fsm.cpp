/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge finite state machine logic.
 */

#include "bridge_fsm.h"

#include "../functions.h"
#include "../mobs/bridge.h"
#include "../utils/string_utils.h"
#include "../vars.h"
#include "gen_mob_fsm.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the bridge's logic.
 */
void bridge_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", BRIDGE_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(bridge_fsm::set_anim);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(gen_mob_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(bridge_fsm::open);
            efc.change_state("destroyed");
        }
    }
    efc.new_state("destroyed", BRIDGE_STATE_DESTROYED); {
    
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_BRIDGE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BRIDGE_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * Opens up the bridge. Updates all relevant sectors,
 * does the particle explosion, etc.
 */
void bridge_fsm::open(mob* m, void* info1, void* info2) {
    bridge* b_ptr = (bridge*) m;
    b_ptr->set_animation(BRIDGE_ANIM_DESTROYED);
    b_ptr->start_dying();
    b_ptr->finish_dying();
    b_ptr->tangible = false;
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height + 1,
        80, 2.75, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = bmp_smoke;
    p.color = al_map_rgb(238, 204, 170);
    particle_generator pg(0, p, 11);
    pg.number_deviation = 1;
    pg.size_deviation = 16;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.total_speed = 75;
    pg.total_speed_deviation = 15;
    pg.duration_deviation = 0.25;
    pg.emit(particles);
    
    for(size_t s = 0; s < b_ptr->secs.size(); s++) {
        sector* s_ptr = b_ptr->secs[s];
        
        if(!s_ptr->tag.empty()) {
            s_ptr->z = s2f(s_ptr->tag);
        }
        
        s_ptr->is_bottomless_pit = false;
        s_ptr->hazards.clear();
        
        s_ptr->texture_info.bitmap =
            (s_ptr->type == SECTOR_TYPE_BRIDGE) ?
            b_ptr->bri_type->bmp_main_texture :
            b_ptr->bri_type->bmp_rail_texture;
        s_ptr->texture_info.file_name =
            (s_ptr->type == SECTOR_TYPE_BRIDGE) ?
            b_ptr->bri_type->main_texture_file_name :
            b_ptr->bri_type->rail_texture_file_name;
        s_ptr->texture_info.rot = m->angle;
        s_ptr->texture_info.scale = point(1.0, 1.0);
        s_ptr->texture_info.tint = al_map_rgb(255, 255, 255);
        
        cur_area_data.generate_edges_blockmap(s_ptr->edges);
        
    }
}


/* ----------------------------------------------------------------------------
 * Sets the standard "idling" animation.
 */
void bridge_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(BRIDGE_ANIM_IDLING);
}
