/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter class and converter related functions.
 */

#include "converter.h"

#include "../functions.h"
#include "../game.h"
#include "../misc_structs.h"

/* ----------------------------------------------------------------------------
 * Creates a converter mob.
 */
converter::converter(
    const point &pos, converter_type* con_type, const float angle
) :
    mob(pos, con_type, angle),
    con_type(con_type),
    amount_in_buffer(0),
    input_pikmin_left(con_type->total_input_pikmin),
    current_type(con_type->available_pikmin_types[0]),
    current_type_nr(0),
    type_change_timer(con_type->type_change_interval),
    auto_conversion_timer(con_type->auto_conversion_timeout),
    next_spew_angle(0) {
    
    type_change_timer.on_end =
    [this] () { this->change_type(); };
    
    auto_conversion_timer.on_end =
    [this] () { this->close(); };
}


/* ----------------------------------------------------------------------------
 * Changes to the next type in the list, if applicable.
 */
void converter::change_type() {
    current_type_nr =
        sum_and_wrap(
            current_type_nr, 1,
            con_type->available_pikmin_types.size()
        );
    current_type = con_type->available_pikmin_types[current_type_nr];
    
    set_animation(
        get_animation_nr_from_base_and_group(
            cur_base_anim_nr, N_CONVERTER_ANIMS, current_type_nr
        ),
        true, false
    );
    
    type_change_timer.start();
}


/* ----------------------------------------------------------------------------
 * Closes up and gets ready for a conversion.
 */
void converter::close() {
    fsm.set_state(CONVERTER_STATE_CLOSING);
    set_animation(
        get_animation_nr_from_base_and_group(
            CONVERTER_ANIM_CLOSING, N_CONVERTER_ANIMS, current_type_nr
        )
    );
    cur_base_anim_nr = CONVERTER_ANIM_CLOSING;
    auto_conversion_timer.stop();
}


//A converter-spat seed starts with this Z offset from the converter.
const float CONVERTER_NEW_SEED_Z_OFFSET = 32.0f;
//After spitting a seed, the next seed's angle shifts by this much.
const float CONVERTER_SPEW_ANGLE_SHIFT = TAU * 0.12345;
//A converter-spat seed is this quick, horizontally.
const float CONVERTER_SPEW_H_SPEED = 90.0f;
//Deviate the seed's horizontal speed by this much, more or less.
const float CONVERTER_SPEW_H_SPEED_DEVIATION = 10.0f;
//A converter-spat seed is this quick, vertically.
const float CONVERTER_SPEW_V_SPEED = 1200.0f;

/* ----------------------------------------------------------------------------
 * Spews out the converted seeds.
 */
void converter::spew() {
    size_t total_to_spit = amount_in_buffer * con_type->pikmin_per_conversion;
    
    for(size_t s = 0; s < total_to_spit; ++s) {
        if(game.states.gameplay_st->mobs.pikmin_list.size() == game.config.max_pikmin_in_field) break;
        
        float horizontal_strength =
            CONVERTER_SPEW_H_SPEED +
            randomf(
                -CONVERTER_SPEW_H_SPEED_DEVIATION,
                CONVERTER_SPEW_H_SPEED_DEVIATION
            );
        spew_pikmin_seed(
            pos, z + CONVERTER_NEW_SEED_Z_OFFSET, current_type,
            next_spew_angle, horizontal_strength, CONVERTER_SPEW_V_SPEED
        );
        
        next_spew_angle += CONVERTER_SPEW_ANGLE_SHIFT;
        next_spew_angle = normalize_angle(next_spew_angle);
    }
    
    amount_in_buffer = 0;
    
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to converters.
 */
void converter::tick_class_specifics(const float delta_t) {
    type_change_timer.tick(delta_t);
    auto_conversion_timer.tick(delta_t);
}
