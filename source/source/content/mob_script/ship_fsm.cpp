/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship finite state machine logic.
 */

#include "ship_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/resource.h"
#include "../mob/ship.h"
#include "../other/particle.h"


/**
 * @brief Creates the finite state machine for the ship's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void ship_fsm::create_fsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.new_state("idling", SHIP_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(ship_fsm::set_anim);
        }
        efc.new_event(MOB_EV_STARTED_RECEIVING_DELIVERY); {
            efc.run(ship_fsm::start_delivery);
        }
        efc.new_event(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(ship_fsm::receive_mob);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_SHIP_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_SHIP_STATES) + " in enum."
    );
}


/**
 * @brief When a ship finishes receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob.
 * @param info2 Unused.
 */
void ship_fsm::receive_mob(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Mob* delivery = (Mob*) info1;
    Ship* shi_ptr = (Ship*) m;
    
    switch(delivery->type->category->id) {
    case MOB_CATEGORY_TREASURES: {
        Treasure* tre_ptr = (Treasure*) delivery;
        game.states.gameplay->treasures_collected++;
        game.states.gameplay->treasure_points_collected +=
            tre_ptr->tre_type->points;
        game.states.gameplay->last_ship_that_got_treasure_pos = m->pos;
        
        if(game.cur_area_data->mission.goal == MISSION_GOAL_COLLECT_TREASURE) {
            auto it =
                game.states.gameplay->mission_remaining_mob_ids.find(
                    delivery->id
                );
            if(it != game.states.gameplay->mission_remaining_mob_ids.end()) {
                game.states.gameplay->mission_remaining_mob_ids.erase(it);
                game.states.gameplay->goal_treasures_collected++;
            }
        }
        break;
        
    } case MOB_CATEGORY_RESOURCES: {
        Resource* res_ptr = (Resource*) delivery;
        switch(res_ptr->res_type->delivery_result) {
        case RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS: {
            game.states.gameplay->treasures_collected++;
            game.states.gameplay->treasure_points_collected +=
                res_ptr->res_type->point_amount;
            game.states.gameplay->last_ship_that_got_treasure_pos = m->pos;
            if(
                game.cur_area_data->mission.goal ==
                MISSION_GOAL_COLLECT_TREASURE
            ) {
                unordered_set<size_t> &goal_mobs =
                    game.states.gameplay->mission_remaining_mob_ids;
                auto it = goal_mobs.find(delivery->id);
                if(it != goal_mobs.end()) {
                    goal_mobs.erase(it);
                    game.states.gameplay->goal_treasures_collected++;
                } else if(res_ptr->origin_pile) {
                    it = goal_mobs.find(res_ptr->origin_pile->id);
                    if(it != goal_mobs.end()) {
                        game.states.gameplay->goal_treasures_collected++;
                    }
                }
            }
            break;
        } case RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS: {
            size_t type_idx = res_ptr->res_type->spray_to_concoct;
            game.states.gameplay->spray_stats[type_idx].nr_ingredients++;
            if(
                game.states.gameplay->spray_stats[type_idx].nr_ingredients >=
                game.config.spray_order[type_idx]->ingredients_needed
            ) {
                game.states.gameplay->spray_stats[type_idx].nr_ingredients -=
                    game.config.spray_order[type_idx]->ingredients_needed;
                game.states.gameplay->change_spray_count(type_idx, 1);
            }
            break;
        } case RESOURCE_DELIVERY_RESULT_DAMAGE_MOB:
        case RESOURCE_DELIVERY_RESULT_STAY: {
            break;
        }
        }
        break;
        
    } default: {
        break;
    }
    }
    
    shi_ptr->mobs_being_beamed--;
    ParticleGenerator pg =
        standard_particle_gen_setup(
            game.sys_content_names.part_onion_insertion, shi_ptr
        );
    pg.follow_pos_offset = shi_ptr->shi_type->receptacle_offset;
    pg.follow_z_offset -= 2.0f; //Must appear below the ship's receptacle.
    shi_ptr->particle_generators.push_back(pg);
    
}


/**
 * @brief When a ship needs to enter its default "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ship_fsm::set_anim(Mob* m, void* info1, void* info2) {
    m->set_animation(
        SHIP_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When a ship starts receiving a mob carried by Pikmin.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ship_fsm::start_delivery(Mob* m, void* info1, void* info2) {
    Ship* shi_ptr = (Ship*) m;
    shi_ptr->mobs_being_beamed++;
}
