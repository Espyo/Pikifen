/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include "bridge.h"
#include "../functions.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a bridge.
 */
bridge::bridge(const float x, const float y, const float angle, const string &vars) :
    mob(x, y, spec_mob_types["Bridge"], angle, vars) {
    
    //Search neighboring sectors.
    get_neighbor_bridge_sectors(get_sector(x, y, NULL, true));
    team = MOB_TEAM_OBSTACLE;
    
}

void bridge::get_neighbor_bridge_sectors(sector* s_ptr) {

    if(!s_ptr) return;
    
    //If this sector is not a bridge sector, skip it.
    if(
        s_ptr->type != SECTOR_TYPE_BRIDGE &&
        s_ptr->type != SECTOR_TYPE_BRIDGE_RAIL
    ) return;
    
    //If this sector is already on the list, skip.
    for(size_t s = 0; s < secs.size(); ++s) {
        if(secs[s] == s_ptr) return;
    }
    
    secs.push_back(s_ptr);
    
    linedef* l_ptr = NULL;
    for(size_t l = 0; l < s_ptr->linedefs.size(); ++l) {
        l_ptr = s_ptr->linedefs[l];
        get_neighbor_bridge_sectors(l_ptr->sectors[(l_ptr->sectors[0] == s_ptr ? 1 : 0)]);
    }
}


void bridge::open(mob* m, void* info1, void* info2) {
    bridge* b_ptr = (bridge*) m;
    b_ptr->set_animation(BRIDGE_ANIM_NOTHING);
    b_ptr->start_dying();
    b_ptr->finish_dying();
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke, b_ptr->x, b_ptr->y,
        60, 90, 10, 12, 2.5, 3, 64, 96, al_map_rgb(238, 204, 170)
    );
    
    for(size_t s = 0; s < b_ptr->secs.size(); s++) {
        sector* s_ptr = b_ptr->secs[s];
        
        s_ptr->z = s2f(s_ptr->tag);
        
        sector_correction sc(s_ptr);
        //TODO the file name is so static... plus, the railing should have its own texture.
        sc.new_texture.bitmap = bitmaps.get("Textures/Bridge.png", NULL);
        sc.new_texture.rot = m->angle;
        
        cur_area_map.sector_corrections.push_back(sc);
        cur_area_map.generate_linedefs_blockmap(s_ptr->linedefs);
        
    }
}

void bridge::take_damage(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    m->health -= damage;
}

void bridge::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(BRIDGE_ANIM_IDLE);
}

void init_bridge_mob_type(mob_type* mt) {
    mt->always_active = true;
    mt->radius = 32;
    mt->max_health = 2000;
    mt->casts_shadow = false;
    mt->create_mob = [] (float x, float y, float angle, const string & vars) {
        create_mob(new bridge(x, y, angle, vars));
    };
    mt->load_from_file_func = [] (data_node * file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
        if(load_resources) {
            anim_conversions->push_back(make_pair(0, "idle"));
            anim_conversions->push_back(make_pair(1, "nothing"));
        }
    };
    
    easy_fsm_creator efc;
    efc.new_state("idle", 0); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(bridge::set_anim);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(bridge::take_damage);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run_function(bridge::open);
            efc.change_state("dead");
        }
    }
    efc.new_state("dead", 1); {
    
    }
    
    
    mt->states = efc.finish();
    mt->first_state_nr = fix_states(mt->states, "idle");
}