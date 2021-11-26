/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#ifndef BRIDGE_INCLUDED
#define BRIDGE_INCLUDED

#include "../mob_types/bridge_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * A bridge mob. Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and a series of components.
 * Each component is a mob that other mobs can walk on top of, serving
 * either as the floor of the bridge, or one of the rails.
 * Every time the bridge expands, it is considered that a new chunk has
 * been added, which may either generate new components, or stretch the
 * existing ones.
 */
class bridge : public mob {
private:
    //How many chunks are needed to fully build this bridge.
    size_t total_chunks_needed;
    //Total length that the bridge should have.
    float total_length;
    //Total vertical offset over the bridge.
    float delta_z;
    //Starting position of the bridge.
    point start_pos;
    //Starting vertical position of the bridge.
    float start_z;
    //How many chunks have successfully been created so far.
    size_t chunks;
    //Z offset of the previous chunk. Cache for convenience.
    float prev_chunk_z_offset;
    //Components of the previous chunk. Cache for convenience.
    vector<mob*> prev_chunk_components;
    //How many times did we combine chunks? Cache for convenience.
    size_t prev_chunk_combo;
    
public:
    //What type of bridge it is.
    bridge_type* bri_type;
    
    //Sectors it will affect when it opens.
    vector<sector*> secs;
    
    //Constructor.
    bridge(const point &pos, bridge_type* bri_type, const float angle);
    
    //Draws a bridge component.
    static void draw_component(mob* m);
    //Checks if any chunks need to be created, and creates them if needed.
    bool check_health();
    //Reads the provided script variables, if any, and does stuff with them.
    void read_script_vars(const script_var_reader &svr);
    //Sets up the bridge using its linked mob.
    void setup();
};


#endif //ifndef BRIDGE_INCLUDED
