/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include <algorithm>

#include "onion.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"


using std::size_t;
using std::string;


namespace ONION {

//How quickly an Onion fades to and from see-through, in values per second.
const float FADE_SPEED = 255.0f;

//Delay before the Onion starts the seed spewing process.
const float FULL_SPEW_DELAY = 2.0f;

//An Onion-spat seed starts with this Z offset from the Onion.
const float NEW_SEED_Z_OFFSET = 320.0f;

//Delay between each individual seed being spit.
const float NEXT_SPEW_DELAY = 0.10f;

//Onion opacity when it goes see-through.
const unsigned char SEETHROUGH_ALPHA = 128;

//After spitting a seed, the next seed's angle shifts by this much.
const float SPEW_ANGLE_SHIFT = TAU * 0.12345;

//An Onion-spat seed is this quick, horizontally.
const float SPEW_H_SPEED = 80.0f;

//Deviate the seed's horizontal speed by this much, more or less.
const float SPEW_H_SPEED_DEVIATION = 10.0f;

//An Onion-spat seed is this quick, vertically.
const float SPEW_V_SPEED = 600.0f;
}




/**
 * @brief Constructs a new Onion object.
 *
 * @param pos Starting coordinates.
 * @param type Onion type this mob belongs to.
 * @param angle Starting angle.
 */
onion::onion(const point &pos, onion_type* type, const float angle) :
    mob(pos, type, angle),
    oni_type(type) {
    
    nest = new pikmin_nest_t(this, oni_type->nest);
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravity_mult = 0.0f;
    z++;
    
    full_spew_timer.on_end =
    [this] () { next_spew_timer.start(); };
    next_spew_timer.on_end =
    [this] () {
        for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
            if(spew_queue[t] > 0) {
                next_spew_timer.start();
                spew();
                return;
            }
        }
    };
    
    for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
        spew_queue.push_back(0);
    }
    
    set_animation(
        MOB_TYPE::ANIM_IDLING, true,
        START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN
    );
}


/**
 * @brief Destroys the Onion object.
 */
onion::~onion() {
    delete nest;
}


/**
 * @brief Draws an Onion.
 */
void onion::draw_mob() {
    sprite* cur_s_ptr;
    sprite* next_s_ptr;
    float interpolation_factor;
    get_sprite_data(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    bitmap_effect_t eff;
    get_sprite_bitmap_effects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    eff.tint_color.a *= (seethrough / 255.0f);
    
    draw_bitmap_with_effects(cur_s_ptr->bitmap, eff);
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void onion::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    nest->read_script_vars(svr);
}


/**
 * @brief Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void onion::spew() {
    for(size_t t = 0; t < spew_queue.size(); ++t) {
        if(spew_queue[t] == 0) continue;
        
        spew_queue[t]--;
        
        game.statistics.pikmin_births++;
        game.states.gameplay->pikmin_born++;
        game.states.gameplay->pikmin_born_per_type[
            oni_type->nest->pik_types[t]
        ]++;
        game.states.gameplay->last_pikmin_born_pos = pos;
        
        size_t total_after =
            game.states.gameplay->mobs.pikmin_list.size() + 1;
            
        if(total_after > game.config.max_pikmin_in_field) {
            nest->pikmin_inside[t][0]++;
            return;
        }
        
        float horizontal_strength =
            ONION::SPEW_H_SPEED +
            randomf(
                -ONION::SPEW_H_SPEED_DEVIATION,
                ONION::SPEW_H_SPEED_DEVIATION
            );
        spew_pikmin_seed(
            pos, z + ONION::NEW_SEED_Z_OFFSET, oni_type->nest->pik_types[t],
            next_spew_angle, horizontal_strength, ONION::SPEW_V_SPEED
        );
        
        next_spew_angle += ONION::SPEW_ANGLE_SHIFT;
        next_spew_angle = normalize_angle(next_spew_angle);
        
        play_sound(oni_type->sfx_pop_idx);
        
        return;
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void onion::tick_class_specifics(const float delta_t) {
    bool needs_to_spew = false;
    for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
        if(spew_queue[t] > 0) {
            needs_to_spew = true;
            break;
        }
    }
    
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); ++o) {
        onion* o_ptr = game.states.gameplay->mobs.onions[o];
        
        if(needs_to_spew) {
        
            o_ptr->full_spew_timer.tick(delta_t);
            o_ptr->next_spew_timer.tick(delta_t);
            
        }
        
        unsigned char final_alpha = 255;
        
        if(
            game.states.gameplay->cur_leader_ptr &&
            bbox_check(
                game.states.gameplay->cur_leader_ptr->pos, o_ptr->pos,
                game.states.gameplay->cur_leader_ptr->radius +
                o_ptr->radius * 3
            )
        ) {
            final_alpha = ONION::SEETHROUGH_ALPHA;
        }
        
        if(
            game.states.gameplay->cur_leader_ptr &&
            bbox_check(
                game.states.gameplay->leader_cursor_w, o_ptr->pos,
                game.states.gameplay->cur_leader_ptr->radius +
                o_ptr->radius * 3
            )
        ) {
            final_alpha = ONION::SEETHROUGH_ALPHA;
        }
        
        if(o_ptr->seethrough != final_alpha) {
            if(final_alpha < o_ptr->seethrough) {
                o_ptr->seethrough =
                    std::max(
                        (double) final_alpha,
                        (double) o_ptr->seethrough - ONION::FADE_SPEED * delta_t
                    );
            } else {
                o_ptr->seethrough =
                    std::min(
                        (double) final_alpha,
                        (double) o_ptr->seethrough + ONION::FADE_SPEED * delta_t
                    );
            }
        }
    }
    
    nest->tick(delta_t);
}
