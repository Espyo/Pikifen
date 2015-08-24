/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
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

/* ----------------------------------------------------------------------------
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
    
    bool is_in_walking_anim;
    
    leader(const float x, const float y, leader_type* type, const float angle, const string &vars);
    
    static void whistle(mob* m, void* info1, void* info2);
    static void stop_whistle(mob* m, void* info1, void* info2);
    static void join_group(mob* m, void* info1, void* info2);
    static void focus(mob* m, void* info1, void* info2);
    static void unfocus(mob* m, void* info1, void* info2);
    static void enter_idle(mob* m, void* info1, void* info2);
    static void enter_active(mob* m, void* info1, void* info2);
    static void move(mob* m, void* info1, void* info2);
    static void stop(mob* m, void* info1, void* info2);
    static void set_walk_anim(mob* m, void* info1, void* info2);
    static void set_stop_anim(mob* m, void* info1, void* info2);
    static void grab_mob(mob* m, void* info1, void* info2);
    static void do_throw(mob* m, void* info1, void* info2);
    static void release(mob* m, void* info1, void* info2);
    static void dismiss(mob* m, void* info1, void* info2);
    static void spray(mob* m, void* info1, void* info2);
    static void lose_health(mob* m, void* info1, void* info2);
    static void inactive_lose_health(mob* m, void* info1, void* info2);
    static void die(mob* m, void* info1, void* info2);
    static void inactive_die(mob* m, void* info1, void* info2);
    static void suffer_pain(mob* m, void* info1, void* info2);
    static void get_knocked_back(mob* m, void* info1, void* info2);
    static void fall_asleep(mob* m, void* info1, void* info2);
    static void start_waking_up(mob* m, void* info1, void* info2);
    static void chase_leader(mob* m, void* info1, void* info2);
    static void stop_in_group(mob* m, void* info1, void* info2);
    static void be_dismissed(mob* m, void* info1, void* info2);
    static void go_pluck(mob* m, void* info1, void* info2);
    static void start_pluck(mob* m, void* info1, void* info2);
    static void stop_pluck(mob* m, void* info1, void* info2);
    static void search_seed(mob* m, void* info1, void* info2);
    static void inactive_search_seed(mob* m, void* info1, void* info2);
    
};



void dismiss();
float get_leader_to_group_center_dist(mob* l);



enum LEADER_STATES {
    LEADER_STATE_IDLE,
    LEADER_STATE_ACTIVE,
    LEADER_STATE_WHISTLING,
    LEADER_STATE_HOLDING,
    LEADER_STATE_DISMISSING,
    LEADER_STATE_SPRAYING,
    LEADER_STATE_PAIN,
    LEADER_STATE_INACTIVE_PAIN,
    LEADER_STATE_KNOCKED_BACK,
    LEADER_STATE_INACTIVE_KNOCKED_BACK,
    LEADER_STATE_DYING,
    LEADER_STATE_INACTIVE_DYING,
    LEADER_STATE_IN_GROUP_CHASING,
    LEADER_STATE_IN_GROUP_STOPPED,
    LEADER_STATE_GOING_TO_PLUCK,
    LEADER_STATE_PLUCKING,
    LEADER_STATE_INACTIVE_GOING_TO_PLUCK,
    LEADER_STATE_INACTIVE_PLUCKING,
    LEADER_STATE_SLEEPING,
    LEADER_STATE_INACTIVE_SLEEPING,
    LEADER_STATE_WAKING_UP, //Time during which the leader is getting up.
    LEADER_STATE_INACTIVE_WAKING_UP, //Time during which the leader is getting up.
    
};

enum LEADER_ANIMATIONS {
    LEADER_ANIM_IDLE,
    LEADER_ANIM_WALK,
    LEADER_ANIM_PLUCK,
    LEADER_ANIM_GET_UP,
    LEADER_ANIM_DISMISS,
    LEADER_ANIM_THROW,
    LEADER_ANIM_WHISTLING,
    LEADER_ANIM_LIE,
    LEADER_ANIM_PAIN,
    LEADER_ANIM_KNOCKED_DOWN,
    LEADER_ANIM_SPRAYING,
};

const float LEADER_INVULN_PERIOD = 1.5f;
const float LEADER_ZAP_ANIM_PARTS = 20;

#endif //ifndef LEADER_INCLUDED
