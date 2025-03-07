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
        Point relative_pos;
        
        //Its absolute coordinates.
        Point absolute_pos;
        
        //Current state. 0 = free. 1 = reserved. 2 = occupied.
        unsigned char state = 0;
        
        //What Pikmin is reserving/occupying. nullptr if free.
        Pikmin* pikmin_here = nullptr;
        
        
        //--- Function declarations ---
        
        explicit GroupTaskSpot(const Point &pos);
        
    };
    
    
    //--- Members ---
    
    //What type of group task it is.
    GroupTaskType* tas_type = nullptr;
    
    //Power requirement in order to reach the goal. Group task type override.
    size_t power_goal = 0;
    
    //List of spots for Pikmin to use.
    vector<GroupTaskSpot> spots;
    
    
    //--- Function declarations ---
    
    GroupTask(const Point &pos, GroupTaskType* type, float angle);
    void add_worker(Pikmin* who);
    void finish_task();
    void free_up_spot(Pikmin* whose);
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    GroupTaskSpot* get_free_spot();
    float get_power() const;
    Point get_spot_pos(const Pikmin* whose) const;
    void reserve_spot(GroupTaskSpot* spot, Pikmin* who);
    void read_script_vars(const ScriptVarReader &svr) override;
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(float delta_t) override;
    
private:

    //--- Members ---
    
    //Combined Pikmin power put into the task right now. Cache for performance.
    float power = 0.0f;
    
    //Has it already run the "task finished" code?
    bool ran_task_finished_code = false;
    
    
    //--- Function declarations ---
    
    void update_spot_absolute_positions();
    
};
