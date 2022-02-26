/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile finite state machine logic.
 */

#include <algorithm>

#include "pile_fsm.h"

#include "../const.h"
#include "../functions.h"
#include "../game.h"
#include "../mobs/pile.h"
#include "../mobs/resource.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


using std::size_t;


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the pile's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void pile_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idling", PILE_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pile_fsm::become_idle);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pile_fsm::be_attacked);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_PILE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PILE_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * Handles being attacked, and checks if it must drop another resource or not.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pile_fsm::be_attacked(mob* m, void* info1, void* info2) {
    gen_mob_fsm::be_attacked(m, info1, info2);
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pile* pil_ptr = (pile*) m;
    
    size_t amount_before = pil_ptr->amount;
    int intended_amount =
        ceil(pil_ptr->health / pil_ptr->pil_type->health_per_resource);
    int amount_to_spawn = pil_ptr->amount - intended_amount;
    amount_to_spawn = std::max((int) 0, amount_to_spawn);
    
    if(amount_to_spawn == 0) return;
    
    if(amount_to_spawn > 1 && !pil_ptr->pil_type->can_drop_multiple) {
        //Can't drop multiple? Let's knock that number down.
        amount_to_spawn = 1;
        intended_amount = pil_ptr->amount - 1;
        pil_ptr->health = pil_ptr->pil_type->health_per_resource * intended_amount;
    }
    
    resource* resource_to_pick_up = NULL;
    pikmin* pikmin_to_start_carrying = NULL;
    
    for(size_t r = 0; r < (size_t) amount_to_spawn; ++r) {
        point spawn_pos;
        float spawn_z = 0;
        float spawn_angle = 0;
        float spawn_h_speed = 0;
        float spawn_v_speed = 0;
        
        if(r == 0 && info->mob2->type->category->id == MOB_CATEGORY_PIKMIN) {
            pikmin_to_start_carrying = (pikmin*) (info->mob2);
            //If this was a Pikmin's attack, spawn the first resource nearby
            //so it can pick it up.
            spawn_angle = get_angle(pil_ptr->pos, pikmin_to_start_carrying->pos);
            spawn_pos =
                pikmin_to_start_carrying->pos +
                angle_to_coordinates(
                    spawn_angle, game.config.standard_pikmin_radius * 1.5
                );
        } else {
            spawn_pos = pil_ptr->pos;
            spawn_z = pil_ptr->height + 32.0f;
            spawn_angle = randomf(0, TAU);
            spawn_h_speed = pil_ptr->radius * 3;
            spawn_v_speed = 600.0f;
        }
        
        resource* new_resource =
            (
                (resource*)
                create_mob(
                    game.mob_categories.get(MOB_CATEGORY_RESOURCES),
                    spawn_pos, pil_ptr->pil_type->contents,
                    spawn_angle, "",
        [pil_ptr] (mob * m) { ((resource*) m)->origin_pile = pil_ptr; }
                )
            );
            
        new_resource->z = spawn_z;
        new_resource->speed.x = cos(spawn_angle) * spawn_h_speed;
        new_resource->speed.y = sin(spawn_angle) * spawn_h_speed;
        new_resource->speed_z = spawn_v_speed;
        new_resource->links = pil_ptr->links;
        
        if(r == 0) {
            resource_to_pick_up = new_resource;
        }
    }
    
    if(pikmin_to_start_carrying) {
        pikmin_to_start_carrying->force_carry(resource_to_pick_up);
    }
    
    pil_ptr->amount = intended_amount;
    
    if(amount_before == pil_ptr->pil_type->max_amount) {
        pil_ptr->recharge_timer.start();
    }
    pil_ptr->update();
}


/* ----------------------------------------------------------------------------
 * When a pile starts idling.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pile_fsm::become_idle(mob* m, void* info1, void* info2) {
    pile* pil_ptr = (pile*) m;
    pil_ptr->update();
}
