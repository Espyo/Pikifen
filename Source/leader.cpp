#include "const.h"
#include "leader.h"
#include "vars.h"

leader::leader(float x, float y, sector* sec)
    : mob(x, y, sec->floors[0].z, LEADER_MOVE_SPEED, sec) { //ToDo Pikmin size.
    
    holding_pikmin = NULL;
    health = 10; //ToDo
    max_health = 10;
    size = 32; //ToDo
    weight = 1; //ToDo
    auto_pluck_mode = false;
    auto_pluck_pikmin = NULL;
    
    group_spot_info* gs = new group_spot_info(max_pikmin_in_field, 8);
    party = new party_info(gs);
}