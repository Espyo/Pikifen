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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../core/misc_structs.h"


namespace CONVERTER {

//A converter-spat seed starts with this Z offset from the converter.
const float NEW_SEED_Z_OFFSET = 32.0f;

//After spitting a seed, the next seed's angle shifts by this much.
const float SPEW_ANGLE_SHIFT = TAU * 0.12345;

//A converter-spat seed is this quick, horizontally.
const float SPEW_H_SPEED = 90.0f;

//Deviate the seed's horizontal speed by this much, more or less.
const float SPEW_H_SPEED_DEVIATION = 10.0f;

//A converter-spat seed is this quick, vertically.
const float SPEW_V_SPEED = 1200.0f;

}


/**
 * @brief Constructs a new converter object.
 *
 * @param pos Starting coordinates.
 * @param type Convert type this mob belongs to.
 * @param angle Starting angle.
 */
Converter::Converter(
    const Point& pos, ConverterType* type, float angle
) :
    Mob(pos, type, angle),
    conType(type),
    inputPikminLeft(conType->totalInputPikmin),
    currentType(conType->availablePikminTypes[0]),
    typeChangeTimer(conType->typeChangeInterval),
    autoConversionTimer(conType->autoConversionTimeout) {
    
    typeChangeTimer.onEnd =
    [this] () { this->changeType(); };
    
    autoConversionTimer.onEnd =
    [this] () { this->close(); };
}


/**
 * @brief Changes to the next type in the list, if applicable.
 */
void Converter::changeType() {
    currentTypeIdx =
        sumAndWrap(
            (int) currentTypeIdx, 1,
            (int) conType->availablePikminTypes.size()
        );
    currentType = conType->availablePikminTypes[currentTypeIdx];
    
    setAnimation(
        getAnimationIdxFromBaseAndGroup(
            curBaseAnimIdx, N_CONVERTER_ANIMS, currentTypeIdx
        ),
        START_ANIM_OPTION_NO_RESTART, true
    );
    
    typeChangeTimer.start();
}


/**
 * @brief Closes up and gets ready for a conversion.
 */
void Converter::close() {
    fsm.setState(CONVERTER_STATE_CLOSING);
    setAnimation(
        getAnimationIdxFromBaseAndGroup(
            CONVERTER_ANIM_CLOSING, N_CONVERTER_ANIMS, currentTypeIdx
        )
    );
    curBaseAnimIdx = CONVERTER_ANIM_CLOSING;
    autoConversionTimer.stop();
}


/**
 * @brief Spews out the converted seeds.
 */
void Converter::spew() {
    size_t totalToSpit = amountInBuffer * conType->pikminPerConversion;
    
    for(size_t s = 0; s < totalToSpit; s++) {
        if(
            game.states.gameplay->mobs.pikmin.size() ==
            game.config.rules.maxPikminInField
        ) {
            break;
        }
        
        float horizontalStrength =
            CONVERTER::SPEW_H_SPEED +
            game.rng.f(
                -CONVERTER::SPEW_H_SPEED_DEVIATION,
                CONVERTER::SPEW_H_SPEED_DEVIATION
            );
        spewPikminSeed(
            pos, z + CONVERTER::NEW_SEED_Z_OFFSET, currentType,
            nextSpewAngle, horizontalStrength, CONVERTER::SPEW_V_SPEED
        );
        
        nextSpewAngle += CONVERTER::SPEW_ANGLE_SHIFT;
        nextSpewAngle = normalizeAngle(nextSpewAngle);
    }
    
    amountInBuffer = 0;
    
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Converter::tickClassSpecifics(float deltaT) {
    typeChangeTimer.tick(deltaT);
    autoConversionTimer.tick(deltaT);
}
