/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin type class and Pikmin type-related functions.
 */

#ifndef PIKMIN_TYPE_INCLUDED
#define PIKMIN_TYPE_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../hazard.h"
#include "mob_type.h"

using namespace std;

class leader;

/* ----------------------------------------------------------------------------
 * Pikmin types, almost the basic meat of the fangames.
 * The canon ones (at the time of writing this) are
 * Red, Yellow, Blue, White, Purple, Bulbmin, Winged, and Rock,
 * but with the engine, loads of fan-made ones can be made.
 */
class pikmin_type : public mob_type {
public:
    vector<hazard*> resistances;
    float attack_power;
    float carry_strength;
    float carry_speed;
    float throw_strength_mult;
    float max_throw_height;
    bool has_onion;
    bool can_dig;
    bool can_fly;
    bool can_swim;
    bool can_latch;
    bool can_carry_bomb_rocks;
    float sprout_evolution_time[N_MATURITIES];
    //Top (leaf/bud/flower) bitmap for each maturity.
    ALLEGRO_BITMAP* bmp_top[N_MATURITIES];
    //Standby icon.
    ALLEGRO_BITMAP* bmp_icon;
    //Standby maturity icons.
    ALLEGRO_BITMAP* bmp_maturity_icon[N_MATURITIES];
    
    pikmin_type();
    void load_parameters(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions();
    void unload_resources();
    
};

#endif //ifndef PIKMIN_TYPE_INCLUDED
