/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the group task class and group task-related functions.
 */

#pragma once

#include <vector>

#include "../mob_type/group_task_type.h"
#include "mob.h"
#include "pikmin.h"


/**
 * @brief A mob that requires multiple Pikmin to work together
 * in order to clear.
 */
class GroupTask : public Mob {

public:

    //--- Misc. declarations ---
    
    /**
     * @brief Info about a spot.
     */
    struct GroupTaskSpot {
    
        //--- Members ---
        
        //Is position relative to the mob's position and angle.
        Point relativePos;
        
        //Its absolute coordinates.
        Point absolutePos;
        
        //Current state. 0 = free. 1 = reserved. 2 = occupied.
        unsigned char state = 0;
        
        //What Pikmin is reserving/occupying. nullptr if free.
        Pikmin* pikminHere = nullptr;
        
        
        //--- Function declarations ---
        
        explicit GroupTaskSpot(const Point& pos);
        
    };
    
    
    //--- Members ---
    
    //What type of group task it is.
    GroupTaskType* tasType = nullptr;
    
    //Power requirement in order to reach the goal. Group task type override.
    size_t powerGoal = 0;
    
    //List of spots for Pikmin to use.
    vector<GroupTaskSpot> spots;
    
    
    //--- Function declarations ---
    
    GroupTask(const Point& pos, GroupTaskType* type, float angle);
    void addWorker(Pikmin* who);
    void finishTask();
    void freeUpSpot(Pikmin* whose);
    FRACTION_NR_VISIBILITY getFractionNumbersInfo(
        float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
    ) const override;
    GroupTaskSpot* getFreeSpot();
    float getPower() const;
    Point getSpotPos(const Pikmin* whose) const;
    void reserveSpot(GroupTaskSpot* spot, Pikmin* who);
    void readScriptVars(const ScriptVarReader& svr) override;
    
protected:

    //--- Function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
private:

    //--- Members ---
    
    //Combined Pikmin power put into the task right now. Cache for performance.
    float power = 0.0f;
    
    //Has it already run the "task finished" code?
    bool ranTaskFinishedCode = false;
    
    
    //--- Function declarations ---
    
    void updateSpotAbsolutePositions();
    
};
