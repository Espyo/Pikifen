#include "enemy.h"

enemy::enemy(float x, float y, sector* s, enemy_type* type)
    : mob(x, y, 0, type, s) {
    
    ene_type = type;
}