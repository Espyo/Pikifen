/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter class and converter-related functions.
 */

#pragma once

#include "../mob_type/converter_type.h"
#include "mob.h"

namespace CONVERTER {
extern const float NEW_SEED_Z_OFFSET;
extern const float SPIT_ANGLE_SHIFT;
extern const float SPIT_H_SPEED;
extern const float SPIT_H_SPEED_DEVIATION;
extern const float SPIT_V_SPEED;
}


/**
 * @brief A converter mob.
 *
 * This is like the Candypop Buds in the canon games, in the sense that it
 * converts a thrown Pikmin from one type into a Pikmin from a different type.
 */
class Converter : public Mob, public MobWithAnimGroups {

public:

    //--- Public members ---
    
    //What type of converter it is.
    ConverterType* conType = nullptr;
    
    //Amount of Pikmin currently inside the converter, in its "buffer".
    size_t amountInBuffer = 0;
    
    //How many Pikmin are left until the converter dies.
    size_t inputPikminLeft = 0;
    
    //Type of Pikmin it will convert to right now.
    PikminType* currentType = nullptr;
    
    //If it cycles between types, this is the index of the current type.
    size_t currentTypeIdx = 0;
    
    //Time left before it cycles to the next type.
    Timer typeChangeTimer;
    
    //Time left before it auto-closes and converts the Pikmin in the buffer.
    Timer autoConversionTimer;
    
    //Number of seeds it has spit so far.
    unsigned int nSpits = 0;
    
    
    //--- Public function declarations ---
    
    Converter(const Point& pos, ConverterType* type, float angle);
    void changeType();
    void close();
    void spit();
    
protected:

    //--- Protected function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
};
