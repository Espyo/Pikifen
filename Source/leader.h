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

class leader : public mob {
public:
    leader_type* lea_type;
    
    mob* holding_pikmin;
    
    bool auto_pluck_mode;
    pikmin* auto_pluck_pikmin; //-1 = not plucking.
    float pluck_time; //Time left until the Pikmin pops out.
    
    leader(const float x, const float y, sector* sec, leader_type* type);
};

#endif //ifndef LEADER_INCLUDED