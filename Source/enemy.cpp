#include "enemy.h"

enemy::enemy(float x, float y, sector* s, enemy_type* type)
    : mob(x, y, s->floors[0].z, type, s) {
    
    ene_type = type;
    team = MOB_TEAM_ENEMIES; //ToDo removeish.
}