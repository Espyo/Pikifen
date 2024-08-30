/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include <algorithm>
#include <unordered_set>

#include "enemy.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../mob_types/mob_type.h"
#include "../utils/general_utils.h"
#include "../utils/math_utils.h"
#include "../utils/string_utils.h"


namespace ENEMY {

//Maximum diameter an enemy's spirit can be.
const float SPIRIT_MAX_SIZE = 128;

//Minimum diameter an enemy's spirit can be.
const float SPIRIT_MIN_SIZE = 16;

//Normally, the spirit's diameter is the enemy's. Multiply the spirit by this.
const float SPIRIT_SIZE_MULT = 0.7;

}


/**
 * @brief Constructs a new enemy object.
 *
 * @param pos Starting coordinates.
 * @param type Enemy type this mob belongs to.
 * @param angle Starting angle.
 */
enemy::enemy(const point &pos, enemy_type* type, const float angle) :
    mob(pos, type, angle),
    ene_type(type) {
    
}


/**
 * @brief Returns whether or not an enemy can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive the status.
 */
bool enemy::can_receive_status(status_type* s) const {
    return has_flag(s->affects, STATUS_AFFECTS_FLAG_ENEMIES);
}


/**
 * @brief Draws an enemy.
 */
void enemy::draw_mob() {
    sprite* cur_s_ptr;
    sprite* next_s_ptr;
    float interpolation_factor;
    get_sprite_data(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    bitmap_effect_t eff;
    get_sprite_bitmap_effects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_DAMAGE |
        SPRITE_BMP_EFFECT_CARRY
    );
    draw_bitmap_with_effects(cur_s_ptr->bitmap, eff);
    draw_status_effect_bmp(this, eff);
}


/**
 * @brief Logic specific to enemies for when they finish dying.
 */
void enemy::finish_dying_class_specifics() {
    if(ene_type->drops_corpse) {
        become_carriable(CARRY_DESTINATION_ONION);
        fsm.set_state(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
    }
    particle par(
        PARTICLE_TYPE_ENEMY_SPIRIT, pos, LARGE_FLOAT,
        clamp(
            radius * 2 * ENEMY::SPIRIT_SIZE_MULT,
            ENEMY::SPIRIT_MIN_SIZE, ENEMY::SPIRIT_MAX_SIZE
        ),
        2, PARTICLE_PRIORITY_MEDIUM
    );
    par.bitmap = game.sys_assets.bmp_enemy_spirit;
    par.speed.x = 0;
    par.speed.y = -50;
    par.friction = 0.5;
    par.acceleration = point(0,0);
    par.color.set_keyframe_value(0, al_map_rgb(255, 192, 255));
    par.color.add(1, al_map_rgba(255, 192, 255, 0));
    game.states.gameplay->particles.add(par);
}


/**
 * @brief Sets up stuff for the beginning of the enemy's death process.
 */
void enemy::start_dying_class_specifics() {
    game.states.gameplay->enemy_deaths++;
    game.states.gameplay->enemy_points_collected += ene_type->points;
    game.states.gameplay->last_enemy_killed_pos = pos;
    game.statistics.enemy_deaths++;
    
    if(game.cur_area_data.mission.goal == MISSION_GOAL_BATTLE_ENEMIES) {
        game.states.gameplay->mission_remaining_mob_ids.erase(id);
    }
}
