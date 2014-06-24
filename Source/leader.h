/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

class pikmin;

#include "const.h"
#include "leader_type.h"
#include "mob.h"
#include "misc_structs.h"
#include "pikmin.h"
#include "sector.h"

using namespace std;

/*
 * A leader controls Pikmin, and
 * is controlled by the player.
 */

class leader : public mob {
public:
    leader_type* lea_type;
    
    mob* holding_pikmin;
    
    bool auto_pluck_mode;
    pikmin* auto_pluck_pikmin; //-1 = not plucking.
    float pluck_time; //Time left until the Pikmin pops out.
    
    leader(const float x, const float y, leader_type* type, const float angle, const string &vars);
};

void dismiss();
float get_leader_to_group_center_dist(mob* l);
void go_pluck(leader* l, pikmin* p);
void pluck_pikmin(leader* new_leader, pikmin* p, leader* leader_who_plucked);
void stop_auto_pluck(leader* l);
void stop_whistling();

#endif //ifndef LEADER_INCLUDED