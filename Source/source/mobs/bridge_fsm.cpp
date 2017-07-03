/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge finite state machine logic.
 */

#include "../functions.h"
#include "bridge.h"
#include "bridge_fsm.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the bridge's logic.
 */
void bridge_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", 0); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(bridge_fsm::set_anim);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(bridge_fsm::take_damage);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(bridge_fsm::open);
            efc.change_state("destroyed");
        }
    }
    efc.new_state("destroyed", 1); {
    
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    assert(typ->states.size() == N_BRIDGE_STATES);
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
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos,
        80, 2.75, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = bmp_smoke;
    p.color = al_map_rgb(238, 204, 170);
    particle_generator pg(0, p, 11);
    pg.number_deviation = 1;
    pg.size_deviation = 16;
    pg.angle = 0;
    pg.angle_deviation = M_PI;
    pg.total_speed = 75;
    pg.total_speed_deviation = 15;
    pg.duration_deviation = 0.25;
    pg.emit(particles);
    
    for(size_t s = 0; s < b_ptr->secs.size(); s++) {
        sector* s_ptr = b_ptr->secs[s];
        
        if(!s_ptr->tag.empty()) {
            s_ptr->z = s2f(s_ptr->tag);
        }
        
        s_ptr->hazards.clear();
        s_ptr->associated_liquid = NULL;
        
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
        s_ptr->texture_info.translation = point();
        
        cur_area_data.generate_edges_blockmap(s_ptr->edges);
        
    }
}


/* ----------------------------------------------------------------------------
 * Damage the bridge, depending on the Pikmin, hitbox, etc.
 */
void bridge_fsm::take_damage(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = calculate_damage(info->mob2, m, info->h2, info->h1);
    m->health -= damage;
}


/* ----------------------------------------------------------------------------
 * Sets the standard "idling" animation.
 */
void bridge_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(BRIDGE_ANIM_IDLING);
}