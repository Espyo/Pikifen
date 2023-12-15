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


/* ----------------------------------------------------------------------------
 * Creates an enemy mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Enemy type this mob belongs to.
 * angle:
 *   Starting angle.
 */
enemy::enemy(const point &pos, enemy_type* type, const float angle) :
    mob(pos, type, angle),
    ene_type(type) {
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an enemy can receive a given status effect.
 * s:
 *   Status type to check.
 */
bool enemy::can_receive_status(status_type* s) const {
    return has_flag(s->affects, STATUS_AFFECTS_ENEMIES);
}


/* ----------------------------------------------------------------------------
 * Draws an enemy.
 */
void enemy::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(
        s_ptr, &eff,
        SPRITE_BITMAP_EFFECT_STANDARD |
        SPRITE_BITMAP_EFFECT_STATUS |
        SPRITE_BITMAP_EFFECT_SECTOR_BRIGHTNESS |
        SPRITE_BITMAP_EFFECT_HEIGHT |
        SPRITE_BITMAP_EFFECT_DELIVERY |
        SPRITE_BITMAP_EFFECT_DAMAGE |
        SPRITE_BITMAP_EFFECT_CARRY
    );
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
    draw_status_effect_bmp(this, eff);
}


/* ----------------------------------------------------------------------------
 * Logic specific to enemies for when they finish dying.
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
    par.gravity = 0;
    par.color = al_map_rgb(255, 192, 255);
    game.states.gameplay->particles.add(par);
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the beginning of the enemy's death process.
 */
void enemy::start_dying_class_specifics() {
    game.states.gameplay->mission_info[0].enemy_deaths++;
    game.states.gameplay->mission_info[0].enemy_points_collected += ene_type->points;
    game.states.gameplay->mission_info[0].last_enemy_killed_pos = pos;
    game.statistics.enemy_deaths++;
    
    if(game.cur_area_data.mission.goal == MISSION_GOAL_BATTLE_ENEMIES) {
        game.states.gameplay->mission_info[0].mission_remaining_mob_ids.erase(id);
    }
}
