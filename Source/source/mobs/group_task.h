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


/* ----------------------------------------------------------------------------
 * A mob that requires multiple Pikmin to work together in order to clear.
 */
class group_task : public mob {
public:
    //What type of group task it is.
    group_task_type* tas_type;
    
    //Struct with info about a spot.
    struct group_task_spot {
        //Is position relative to the mob's position and angle.
        point relative_pos;
        //Its absolute coordinates.
        point absolute_pos;
        //Current state. 0 = free. 1 = reserved. 2 = occupied.
        unsigned char state;
        //What Pikmin is reserving/occupying. NULL if free.
        pikmin* pikmin_here;
        //Constructor.
        explicit group_task_spot(const point &pos);
    };
    
    //List of spots for Pikmin to use.
    vector<group_task_spot> spots;
    
    //Add a Pikmin as an actual worker.
    void add_worker(pikmin* who);
    //Consider the task finished.
    void finish_task();
    //Free up a spot taken by a Pikmin.
    void free_up_spot(pikmin* whose);
    //Returns information on how to show the fraction.
    bool get_fraction_numbers_info(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    //Returns a free spot, if any.
    group_task_spot* get_free_spot();
    //Returns the current working power.
    float get_power() const;
    //Returns the absolute coordinates of a spot taken by a Pikmin.
    point get_spot_pos(pikmin* whose) const;
    //Reserves a spot for a Pikmin.
    void reserve_spot(group_task_spot* spot, pikmin* who);
    
    //Constructor.
    group_task(const point &pos, group_task_type* type, const float angle);
    
protected:
    //Tick class-specific logic.
    void tick_class_specifics(const float delta_t) override;
    
private:
    //Combined Pikmin power put into the task right now. Cache for performance.
    float power;
    //Has it already run the "task finished" code?
    bool ran_task_finished_code;
    
    //Update the absolute coordinates of each spot.
    void update_spot_absolute_positions();
};


#endif //ifndef GROUP_TASK_INCLUDED
