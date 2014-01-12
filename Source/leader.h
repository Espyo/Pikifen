#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

class pikmin;

#include "const.h"
#include "leader_type.h"
#include "mob.h"
#include "misc_structs.h"
#include "party_follower.h"
#include "pikmin.h"
#include "sector.h"

using namespace std;

class leader : public mob {
public:
    leader_type* lea_type;
    
    unsigned int health;
    unsigned int max_health;
    mob* holding_pikmin;
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    bool auto_pluck_mode;
    pikmin* auto_pluck_pikmin;
    
    leader(float x, float y, sector* sec, leader_type* type);
};

#endif //ifndef LEADER_INCLUDED