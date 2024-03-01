/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the group task class and group task-related functions.
 */

#ifndef GROUP_TASK_INCLUDED
#define GROUP_TASK_INCLUDED

#include <vector>

#include "../mob_types/group_task_type.h"
#include "mob.h"
#include "pikmin.h"


/**
 * @brief A mob that requires multiple Pikmin to work together
 * in order to clear.
 */
class group_task : public mob {

public:
    
    //--- Misc. declarations ---
    
    /**
     * @brief Info about a spot.
     */
    struct group_task_spot {

        //--- Members ---

        //Is position relative to the mob's position and angle.
        point relative_pos;

        //Its absolute coordinates.
        point absolute_pos;

        //Current state. 0 = free. 1 = reserved. 2 = occupied.
        unsigned char state;

        //What Pikmin is reserving/occupying. NULL if free.
        pikmin* pikmin_here;

        
        //--- Function declarations ---

        explicit group_task_spot(const point &pos);

    };
    

    //--- Members ---

    //What type of group task it is.
    group_task_type* tas_type;

    //Power requirement in order to reach the goal. Group task type override.
    size_t power_goal;
    
    //List of spots for Pikmin to use.
    vector<group_task_spot> spots;
    

    //--- Function declarations ---

    group_task(const point &pos, group_task_type* type, const float angle);
    void add_worker(pikmin* who);
    void finish_task();
    void free_up_spot(pikmin* whose);
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    group_task_spot* get_free_spot();
    float get_power() const;
    point get_spot_pos(const pikmin* whose) const;
    void reserve_spot(group_task_spot* spot, pikmin* who);
    void read_script_vars(const script_var_reader &svr) override;
    
protected:

    //--- Function declarations ---

    void tick_class_specifics(const float delta_t) override;
    
private:
    
    //--- Members ---
    
    //Combined Pikmin power put into the task right now. Cache for performance.
    float power;
    
    //Has it already run the "task finished" code?
    bool ran_task_finished_code;
    

    //--- Function declarations ---

    void update_spot_absolute_positions();
    
};


#endif //ifndef GROUP_TASK_INCLUDED
