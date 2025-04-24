/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#pragma once

#include "../mob_type/bridge_type.h"
#include "mob.h"


namespace BRIDGE {
extern const float FLOOR_WIDTH;
extern const float STEP_HEIGHT;
}


/**
 * @brief A bridge mob.
 *
 * Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and a series of components.
 * Each component is a mob that other mobs can walk on top of, serving
 * either as the floor of the bridge, or one of the rails.
 * Every time the bridge expands, it is considered that a new chunk has
 * been added, which may either generate new components, or stretch the
 * existing ones.
 */
class Bridge : public Mob {

private:

    //--- Members ---
    
    //How many chunks are needed to fully build this bridge.
    size_t totalChunksNeeded = 10;
    
    //Total length that the bridge should have.
    float totalLength = 192.0f;
    
    //Total vertical offset over the bridge.
    float deltaZ = 0.0f;
    
    //Starting position of the bridge.
    Point startPos;
    
    //Starting vertical position of the bridge.
    float startZ = 0.0f;
    
    //How many chunks have successfully been created so far.
    size_t chunks = 0;
    
    //Z offset of the previous chunk. Cache for convenience.
    float prevChunkZOffset = LARGE_FLOAT;
    
    //Components of the previous chunk. Cache for convenience.
    vector<Mob*> prevChunkComponents;
    
    //How many times did we combine chunks? Cache for convenience.
    size_t prevChunkCombo = 0;
    
public:

    //--- Members ---
    
    //What type of bridge it is.
    BridgeType* briType = nullptr;
    
    
    //--- Function declarations ---
    
    Bridge(const Point &pos, BridgeType* Type, float angle);
    static void drawComponent(Mob* m);
    bool checkHealth();
    Point getStartPoint();
    void readScriptVars(const ScriptVarReader &svr) override;
    void setup();
    
};
