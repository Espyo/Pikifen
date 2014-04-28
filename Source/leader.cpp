#include "const.h"
#include "leader.h"
#include "vars.h"

leader::leader(float x, float y, sector* sec, leader_type* type)
    : mob(x, y, sec->floors[0].z, type, sec) {
    
    lea_type = type;
    
    holding_pikmin = NULL;
    auto_pluck_mode = false;
    auto_pluck_pikmin = NULL;
    pluck_time = -1;
    
    party_spot_info* ps = new party_spot_info(max_pikmin_in_field, 12);
    party = new party_info(ps, x, y);
}