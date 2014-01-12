#include "const.h"
#include "leader.h"
#include "vars.h"

leader::leader(float x, float y, sector* sec, leader_type* type)
    : mob(x, y, sec->floors[0].z, type, sec) { //ToDo Pikmin size.
    
    holding_pikmin = NULL;
    health = 10; //ToDo
    max_health = 10;
    auto_pluck_mode = false;
    auto_pluck_pikmin = NULL;
    
    party_spot_info* ps = new party_spot_info(max_pikmin_in_field, 8);
    party = new party_info(ps, x, y);
}