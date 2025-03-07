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

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"


using std::size_t;
using std::string;


namespace ONION {

//How quickly an Onion fades to and from see-through, in values per second.
const float FADE_SPEED = 255.0f;

//Delay before the Onion starts the generation process.
const float GENERATION_DELAY = 2.0f;

//An Onion-spat seed starts with this Z offset from the Onion.
const float NEW_SEED_Z_OFFSET = 320.0f;

//Interval between each individual Pikmin generation.
const float NEXT_GENERATION_INTERVAL = 0.10f;

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
Onion::Onion(const Point &pos, OnionType* type, float angle) :
    Mob(pos, type, angle),
    oni_type(type) {
    
    nest = new PikminNest(this, oni_type->nest);
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravity_mult = 0.0f;
    z++;
    
    generation_delay_timer.on_end =
    [this] () { start_generating(); };
    next_generation_timer.on_end =
    [this] () {
        for(size_t t = 0; t < oni_type->nest->pik_types.size(); t++) {
            if(generation_queue[t] > 0) {
                next_generation_timer.start();
                generate();
                return;
            }
        }
        stop_generating();
    };
    
    for(size_t t = 0; t < oni_type->nest->pik_types.size(); t++) {
        generation_queue.push_back(0);
    }
}


/**
 * @brief Destroys the Onion object.
 */
Onion::~Onion() {
    delete nest;
}


/**
 * @brief Draws an Onion.
 */
void Onion::draw_mob() {
    Sprite* cur_s_ptr;
    Sprite* next_s_ptr;
    float interpolation_factor;
    get_sprite_data(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    BitmapEffect eff;
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
void Onion::read_script_vars(const ScriptVarReader &svr) {
    Mob::read_script_vars(svr);
    
    nest->read_script_vars(svr);
}


/**
 * @brief Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void Onion::generate() {
    for(size_t t = 0; t < generation_queue.size(); t++) {
        if(generation_queue[t] == 0) continue;
        
        generation_queue[t]--;
        
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
            
            ParticleGenerator pg =
                standard_particle_gen_setup(
                    game.sys_content_names.part_onion_gen_inside, this
                );
            pg.base_particle.priority = PARTICLE_PRIORITY_LOW;
            particle_generators.push_back(pg);
            
            return;
        }
        
        float horizontal_strength =
            ONION::SPEW_H_SPEED +
            game.rng.f(
                -ONION::SPEW_H_SPEED_DEVIATION,
                ONION::SPEW_H_SPEED_DEVIATION
            );
        spew_pikmin_seed(
            pos, z + ONION::NEW_SEED_Z_OFFSET, oni_type->nest->pik_types[t],
            next_spew_angle, horizontal_strength, ONION::SPEW_V_SPEED
        );
        
        next_spew_angle += ONION::SPEW_ANGLE_SHIFT;
        next_spew_angle = normalize_angle(next_spew_angle);
        
        play_sound(oni_type->sound_pop_idx);
        
        return;
    }
}


/**
 * @brief Starts generating Pikmin.
 */
void Onion::start_generating() {
    generation_delay_timer.stop();
    next_generation_timer.start();
    string msg = "started_generation";
    send_message(this, msg);
}


/**
 * @brief Stops generating Pikmin.
 */
void Onion::stop_generating() {
    generation_delay_timer.stop();
    next_generation_timer.stop();
    string msg = "stopped_generation";
    send_message(this, msg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Onion::tick_class_specifics(float delta_t) {
    generation_delay_timer.tick(delta_t);
    next_generation_timer.tick(delta_t);
    
    unsigned char final_alpha = 255;
    
    if(
        game.states.gameplay->cur_leader_ptr &&
        bbox_check(
            game.states.gameplay->cur_leader_ptr->pos, pos,
            game.states.gameplay->cur_leader_ptr->radius +
            radius * 3
        )
    ) {
        final_alpha = ONION::SEETHROUGH_ALPHA;
    }
    
    if(
        game.states.gameplay->cur_leader_ptr &&
        bbox_check(
            game.states.gameplay->leader_cursor_w, pos,
            game.states.gameplay->cur_leader_ptr->radius +
            radius * 3
        )
    ) {
        final_alpha = ONION::SEETHROUGH_ALPHA;
    }
    
    if(seethrough != final_alpha) {
        if(final_alpha < seethrough) {
            seethrough =
                std::max(
                    (double) final_alpha,
                    (double) seethrough - ONION::FADE_SPEED * delta_t
                );
        } else {
            seethrough =
                std::min(
                    (double) final_alpha,
                    (double) seethrough + ONION::FADE_SPEED * delta_t
                );
        }
    }
    
    nest->tick(delta_t);
}
