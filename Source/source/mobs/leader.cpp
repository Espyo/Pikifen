/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include <algorithm>

#include "leader.h"

#include "../const.h"
#include "../drawing.h"
#include "../functions.h"
#include "../game.h"

namespace LEADER {

//Auto-throw starts at this cooldown.
const float AUTO_THROW_COOLDOWN_MAX_DURATION = 0.7f;

//Auto-throw ends at this cooldown.
const float AUTO_THROW_COOLDOWN_MIN_DURATION = THROW_COOLDOWN_DURATION * 1.2f;

//Auto-throw cooldown lowers at this speed.
const float AUTO_THROW_COOLDOWN_SPEED = 0.3f;

//Members cannot go past this range from the angle of dismissal.
const float DISMISS_ANGLE_RANGE = TAU / 2;

//Multiply the space members take up by this. Lower = more compact subgroups.
const float DISMISS_MEMBER_SIZE_MULTIPLIER = 0.75f;

//Opacity of the dismiss particles.
const float DISMISS_PARTICLE_ALPHA = 1.0f;

//Amount of dismiss particles to spawn.
const size_t DISMISS_PARTICLE_AMOUNT = WHISTLE::N_DOT_COLORS * 3;

//Dismiss particle friction.
const float DISMISS_PARTICLE_FRICTION = 3.2f;

//Dismiss particle maximum duration.
const float DISMISS_PARTICLE_MAX_DURATION = 1.4f;

//Dismiss particle maximum speed.
const float DISMISS_PARTICLE_MAX_SPEED = 210.0f;

//Dismiss particle minimum duration.
const float DISMISS_PARTICLE_MIN_DURATION = 1.0f;

//Dismiss particle minimum speed.
const float DISMISS_PARTICLE_MIN_SPEED = 170.0f;

//Dismiss particle size.
const float DISMISS_PARTICLE_SIZE = 8.0f;

//Dismissed groups must have this much distance between them/the leader.
const float DISMISS_SUBGROUP_DISTANCE = 48.0f;

//Ratio of health at which a leader's health wheel starts giving a warning.
const float HEALTH_CAUTION_RATIO = 0.3f;

//How long the low health caution ring lasts for.
const float HEALTH_CAUTION_RING_DURATION = 2.5f;

//Angle at which leaders hold their group members.
const float HELD_GROUP_MEMBER_ANGLE = TAU / 2;

//How far away from the leader is a held group member placed, horizontally.
const float HELD_GROUP_MEMBER_H_DIST = 1.2f;

//How far away from the leader is a held group member placed, vertically.
const float HELD_GROUP_MEMBER_V_DIST = 0.5f;

//Invulnerability period after getting hit.
const float INVULN_PERIOD = 1.5f;

//Seconds that need to pass before another swarm arrow appears.
const float SWARM_ARROW_INTERVAL = 0.1f;

//Swarm particle opacity.
const float SWARM_PARTICLE_ALPHA = 0.8f;

//Swarm particle random angle deviation.
const float SWARM_PARTICLE_ANGLE_DEVIATION = TAU * 0.04f;

//Swarm particle friction.
const float SWARM_PARTICLE_FRICTION = 2.0f;

//Swarm particle maximum duration.
const float SWARM_PARTICLE_MAX_DURATION = 1.5f;

//Swarm particle minimum duration.
const float SWARM_PARTICLE_MIN_DURATION = 1.0f;

//Swarm particle size.
const float SWARM_PARTICLE_SIZE = 6.0f;

//Swarm particle random speed deviation.
const float SWARM_PARTICLE_SPEED_DEVIATION = 10.0f;

//Swarm particle speed multiplier.
const float SWARM_PARTICLE_SPEED_MULT = 500.0f;

//Throws cannot happen any faster than this interval.
const float THROW_COOLDOWN_DURATION = 0.15f;

//Throw preview maximum thickness.
const float THROW_PREVIEW_DEF_MAX_THICKNESS = 8.0f;

//The throw preview starts fading in at this ratio.
const float THROW_PREVIEW_FADE_IN_RATIO = 0.30f;

//The throw preview starts fading out at this ratio.
const float THROW_PREVIEW_FADE_OUT_RATIO = 1.0f - THROW_PREVIEW_FADE_IN_RATIO;

//Throw preview minimum thickness.
const float THROW_PREVIEW_MIN_THICKNESS = 2.0f;

}


/**
 * @brief Constructs a new leader object.
 *
 * @param pos Starting coordinates.
 * @param type Leader type this mob belongs to.
 * @param angle Starting angle.
 */
leader::leader(const point &pos, leader_type* type, const float angle) :
    mob(pos, type, angle),
    lea_type(type) {
    
    team = MOB_TEAM_PLAYER_1;
    invuln_period = timer(LEADER::INVULN_PERIOD);
    
    subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
        
    swarm_next_arrow_timer.on_end = [this] () {
        swarm_next_arrow_timer.start();
        swarm_arrows.push_back(0);
        
        particle p;
        unsigned char color_idx = randomi(0, WHISTLE::N_DOT_COLORS);
        p.bitmap = game.sys_assets.bmp_bright_circle;
        p.color.r = WHISTLE::DOT_COLORS[color_idx][0] / 255.0f;
        p.color.g = WHISTLE::DOT_COLORS[color_idx][1] / 255.0f;
        p.color.b = WHISTLE::DOT_COLORS[color_idx][2] / 255.0f;
        p.color.a = LEADER::SWARM_PARTICLE_ALPHA;
        p.duration =
            randomf(
                LEADER::SWARM_PARTICLE_MIN_DURATION,
                LEADER::SWARM_PARTICLE_MAX_DURATION
            );
        p.friction = LEADER::SWARM_PARTICLE_FRICTION;
        p.pos = this->pos;
        p.pos.x += randomf(-this->radius * 0.5f, this->radius * 0.5f);
        p.pos.y += randomf(-this->radius * 0.5f, this->radius * 0.5f);
        p.priority = PARTICLE_PRIORITY_MEDIUM;
        p.size = LEADER::SWARM_PARTICLE_SIZE;
        float p_speed =
            game.states.gameplay->swarm_magnitude *
            LEADER::SWARM_PARTICLE_SPEED_MULT +
            randomf(
                -LEADER::SWARM_PARTICLE_SPEED_DEVIATION,
                LEADER::SWARM_PARTICLE_SPEED_DEVIATION
            );
        float p_angle =
            game.states.gameplay->swarm_angle +
            randomf(
                -LEADER::SWARM_PARTICLE_ANGLE_DEVIATION,
                LEADER::SWARM_PARTICLE_ANGLE_DEVIATION
            );
        p.speed = rotate_point(point(p_speed, 0.0f), p_angle);
        p.time = p.duration;
        p.type = PARTICLE_TYPE_BITMAP;
        p.z = this->z + this->height / 2.0f;
        game.states.gameplay->particles.add(p);
    };
    swarm_next_arrow_timer.start();
}


/**
 * @brief Returns whether or not a leader can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive the status.
 */
bool leader::can_receive_status(status_type* s) const {
    return has_flag(s->affects, STATUS_AFFECTS_LEADERS);
}


/**
 * @brief Returns whether or not a leader can throw.
 *
 * @return Whether it can throw.
 */
bool leader::check_throw_ok() const {
    if(holding.empty()) {
        return false;
    }
    
    mob_event* ev = fsm.get_event(LEADER_EV_THROW);
    
    if(!ev) {
        return false;
    }
    
    return true;
}


/**
 * @brief Makes a leader dismiss their group.
 * The group is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void leader::dismiss() {
    size_t n_group_members = group->members.size();
    if(n_group_members == 0) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit for each subgroup, depending on a few factors.
    float base_angle;
    
    //First, calculate what direction the group should be dismissed to.
    if(game.states.gameplay->swarm_magnitude > 0) {
        //If the leader's swarming,
        //they should be dismissed in that direction.
        base_angle = game.states.gameplay->swarm_angle;
    } else {
        //Leftmost member coordinate, rightmost, etc.
        point min_coords, max_coords;
        
        for(size_t m = 0; m < n_group_members; ++m) {
            mob* member_ptr = group->members[m];
            
            if(member_ptr->pos.x < min_coords.x || m == 0)
                min_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.x > max_coords.x || m == 0)
                max_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.y < min_coords.y || m == 0)
                min_coords.y = member_ptr->pos.y;
            if(member_ptr->pos.y > max_coords.y || m == 0)
                max_coords.y = member_ptr->pos.y;
        }
        
        point group_center(
            (min_coords.x + max_coords.x) / 2,
            (min_coords.y + max_coords.y) / 2
        );
        base_angle = get_angle(pos, group_center);
    }
    
    /**
     * @brief Info about a group subgroup when being dismissed.
     */
    struct subgroup_dismiss_info {
    
        //--- Members ---
        
        //Radius of the group.
        float radius;
        
        //Group members of this subgroup type.
        vector<mob*> members;
        
        //Center point of the subgroup.
        point center;
        
    };
    vector<subgroup_dismiss_info> subgroups_info;
    
    //Go through all subgroups and populate the vector of data.
    subgroup_type* first_type =
        game.states.gameplay->subgroup_types.get_first_type();
    subgroup_type* cur_type = first_type;
    
    do {
    
        if(
            cur_type !=
            game.states.gameplay->subgroup_types.get_type(
                SUBGROUP_TYPE_CATEGORY_LEADER
            )
        ) {
        
            bool subgroup_exists = false;
            
            for(size_t m = 0; m < n_group_members; ++m) {
                mob* m_ptr = group->members[m];
                if(m_ptr->subgroup_type_ptr != cur_type) continue;
                
                if(!subgroup_exists) {
                    subgroups_info.push_back(subgroup_dismiss_info());
                    subgroup_exists = true;
                }
                
                subgroups_info.back().members.push_back(m_ptr);
            }
            
        }
        
        cur_type =
            game.states.gameplay->subgroup_types.get_next_type(cur_type);
            
    } while(cur_type != first_type);
    
    //Let's figure out each subgroup's size.
    //Subgroups will be made by placing the members in
    //rows of circles surrounding a central point.
    //The first row is just one spot.
    //The second row is 6 spots around that one.
    //The third is 12 spots around those 6.
    //And so on. Each row fits an additional 6.
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        size_t n_rows = get_dismiss_rows(subgroups_info[s].members.size());
        
        //Since each row loops all around,
        //it appears to the left and right of the center.
        //So count each one twice. Except for the central one.
        subgroups_info[s].radius =
            game.config.standard_pikmin_radius +
            game.config.standard_pikmin_radius * 2 *
            LEADER::DISMISS_MEMBER_SIZE_MULTIPLIER * (n_rows - 1);
    }
    
    /**
     * @brief We'll need to place the subgroups inside arched rows.
     * Like stripes on a rainbow.
     * For each row, we must fit as many Pikmin subgroups as possible.
     * Each row can have a different thickness,
     * based on the size of the subgroups within.
     * Starts off on the row closest to the leader.
     * We place the first subgroup, then some padding, then the next group,
     * etc. For every subgroup we place, we must update the thickness.
     */
    struct row_info {
    
        //--- Members ---
        
        //Index of subgroups in this row.
        vector<size_t> subgroups;
        
        //Angular distance spread out from the row center.
        float dist_between_center;
        
        //How thick this row is.
        float thickness;
        
        //How much is taken up by Pikmin and padding.
        float angle_occupation;
        
        
        //--- Function definitions ---
        
        row_info() {
            dist_between_center = 0;
            thickness = 0;
            angle_occupation = 0;
        }
        
    };
    
    bool done = false;
    vector<row_info> rows;
    row_info cur_row;
    cur_row.dist_between_center = LEADER::DISMISS_SUBGROUP_DISTANCE;
    size_t cur_row_nr = 0;
    size_t cur_subgroup_nr = 0;
    
    while(!done && !subgroups_info.empty()) {
        float new_thickness =
            std::max(
                cur_row.thickness, subgroups_info[cur_subgroup_nr].radius * 2
            );
            
        float new_angle_occupation = 0;
        for(size_t s = 0; s < cur_row.subgroups.size(); ++s) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    subgroups_info[cur_row.subgroups[s]].radius * 2.0,
                    cur_row.dist_between_center +
                    cur_row.thickness / 2.0f
                );
            if(s < cur_row.subgroups.size() - 1) {
                new_angle_occupation +=
                    linear_dist_to_angular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        cur_row.dist_between_center +
                        cur_row.thickness / 2.0f
                    );
            }
        }
        if(!cur_row.subgroups.empty()) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    LEADER::DISMISS_SUBGROUP_DISTANCE,
                    cur_row.dist_between_center +
                    new_thickness / 2.0f
                );
        }
        new_angle_occupation +=
            linear_dist_to_angular(
                subgroups_info[cur_subgroup_nr].radius * 2.0,
                cur_row.dist_between_center +
                new_thickness / 2.0f
            );
            
        //Will this group fit?
        if(new_angle_occupation <= LEADER::DISMISS_ANGLE_RANGE) {
            //This subgroup still fits. Next!
            cur_row.thickness = new_thickness;
            cur_row.angle_occupation = new_angle_occupation;
            
            cur_row.subgroups.push_back(cur_subgroup_nr);
            cur_subgroup_nr++;
        }
        
        if(
            new_angle_occupation > LEADER::DISMISS_ANGLE_RANGE ||
            cur_subgroup_nr == subgroups_info.size()
        ) {
            //This subgroup doesn't fit. It'll have to be put in the next row.
            //Or this is the last subgroup, and the row needs to be committed.
            
            rows.push_back(cur_row);
            cur_row_nr++;
            cur_row.dist_between_center +=
                cur_row.thickness + LEADER::DISMISS_SUBGROUP_DISTANCE;
            cur_row.subgroups.clear();
            cur_row.thickness = 0;
            cur_row.angle_occupation = 0;
        }
        
        if(cur_subgroup_nr == subgroups_info.size()) done = true;
    }
    
    //Now that we know which subgroups go into which row,
    //simply decide the positioning.
    for(size_t r = 0; r < rows.size(); ++r) {
        float start_angle = -(rows[r].angle_occupation / 2.0f);
        float cur_angle = start_angle;
        
        for(size_t s = 0; s < rows[r].subgroups.size(); ++s) {
            size_t s_nr = rows[r].subgroups[s];
            float subgroup_angle = cur_angle;
            
            cur_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius * 2.0,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
            if(s < rows[r].subgroups.size() - 1) {
                cur_angle +=
                    linear_dist_to_angular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        rows[r].dist_between_center + rows[r].thickness / 2.0
                    );
            }
            
            //Center the subgroup's angle.
            subgroup_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
                
            subgroups_info[s_nr].center =
                angle_to_coordinates(
                    base_angle + subgroup_angle,
                    rows[r].dist_between_center + rows[r].thickness / 2.0f
                );
                
        }
    }
    
    //Now, dismiss!
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        cur_row_nr = 0;
        size_t cur_row_spot_nr = 0;
        size_t cur_row_spots = 1;
        
        for(size_t m = 0; m < subgroups_info[s].members.size(); ++m) {
        
            point destination;
            
            if(cur_row_nr == 0) {
                destination = subgroups_info[s].center;
            } else {
                float member_angle =
                    ((float) cur_row_spot_nr / cur_row_spots) * TAU;
                destination =
                    subgroups_info[s].center +
                    angle_to_coordinates(
                        member_angle,
                        cur_row_nr * game.config.standard_pikmin_radius * 2 *
                        LEADER::DISMISS_MEMBER_SIZE_MULTIPLIER
                    );
            }
            
            destination +=
                point(
                    randomf(-5.0, 5.0),
                    randomf(-5.0, 5.0)
                );
                
            cur_row_spot_nr++;
            if(cur_row_spot_nr == cur_row_spots) {
                cur_row_nr++;
                cur_row_spot_nr = 0;
                if(cur_row_nr == 1) {
                    cur_row_spots = 6;
                } else {
                    cur_row_spots += 6;
                }
            }
            
            destination += this->pos;
            
            subgroups_info[s].members[m]->leave_group();
            subgroups_info[s].members[m]->fsm.run_event(
                MOB_EV_DISMISSED, (void*) &destination
            );
            
        }
    }
    
    //Dismiss leaders now.
    while(!group->members.empty()) {
        group->members[0]->fsm.run_event(MOB_EV_DISMISSED, NULL);
        group->members[0]->leave_group();
    }
    
    //Final things.
    play_sound(lea_type->sfx_data_idxs[LEADER_SOUND_DISMISSING]);
    for(size_t p = 0; p < LEADER::DISMISS_PARTICLE_AMOUNT; ++p) {
        particle par;
        const unsigned char* color_idx =
            WHISTLE::DOT_COLORS[p % WHISTLE::N_DOT_COLORS];
        par.color.r = color_idx[0] / 255.0f;
        par.color.g = color_idx[1] / 255.0f;
        par.color.b = color_idx[2] / 255.0f;
        par.color.a = LEADER::DISMISS_PARTICLE_ALPHA;
        par.bitmap = game.sys_assets.bmp_bright_circle;
        par.duration =
            randomf(
                LEADER::DISMISS_PARTICLE_MIN_DURATION,
                LEADER::DISMISS_PARTICLE_MAX_DURATION
            );
        par.friction = LEADER::DISMISS_PARTICLE_FRICTION;
        par.pos = pos;
        par.priority = PARTICLE_PRIORITY_MEDIUM;
        par.size = LEADER::DISMISS_PARTICLE_SIZE;
        float par_speed =
            randomf(
                LEADER::DISMISS_PARTICLE_MIN_SPEED,
                LEADER::DISMISS_PARTICLE_MAX_SPEED
            );
        float par_angle = TAU / LEADER::DISMISS_PARTICLE_AMOUNT * p;
        par.speed = rotate_point(point(par_speed, 0.0f), par_angle);
        par.time = par.duration;
        par.type = PARTICLE_TYPE_BITMAP;
        par.z = z + height / 2.0f;
        game.states.gameplay->particles.add(par);
    }
    set_animation(LEADER_ANIM_DISMISSING);
}


/**
 * @brief Draw a leader mob.
 */
void leader::draw_mob() {
    mob::draw_mob();
    
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
        SPRITE_BITMAP_EFFECT_CARRY
    );
    
    if(invuln_period.time_left > 0.0f) {
        sprite* spark_s =
            game.sys_assets.spark_animation.instance.get_cur_sprite();
            
        if(spark_s && spark_s->bitmap) {
            bitmap_effect_info spark_eff = eff;
            point size(
                al_get_bitmap_width(s_ptr->bitmap) * eff.scale.x,
                al_get_bitmap_height(s_ptr->bitmap) * eff.scale.y
            );
            spark_eff.scale.x = size.x / al_get_bitmap_width(spark_s->bitmap);
            spark_eff.scale.y = size.y / al_get_bitmap_height(spark_s->bitmap);
            draw_bitmap_with_effects(spark_s->bitmap, spark_eff);
        }
    }
    
    draw_status_effect_bmp(this, eff);
}


/**
 * @brief Returns how many rows will be needed to fit all of the members.
 * Used to calculate how subgroup members will be placed when dismissing.
 *
 * @param n_members Total number of group members to dismiss.
 * @return The amount of rows.
 */
size_t leader::get_dismiss_rows(const size_t n_members) const {
    size_t members_that_fit = 1;
    size_t rows_needed = 1;
    while(members_that_fit < n_members) {
        rows_needed++;
        members_that_fit += 6 * (rows_needed - 1);
    }
    return rows_needed;
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param final_spot The final coordinates are returned here.
 * @param final_dist The final distance to those coordinates is returned here.
 */
void leader::get_group_spot_info(
    point* final_spot, float* final_dist
) const {
    final_spot->x = 0.0f;
    final_spot->y = 0.0f;
    *final_dist = 0.0f;
    
    if(!following_group || !following_group->group) {
        return;
    }
    
    group_info_struct* leader_group_ptr = following_group->group;
    
    float distance =
        following_group->radius +
        radius + game.config.standard_pikmin_radius;
        
    for(size_t me = 0; me < leader_group_ptr->members.size(); ++me) {
        mob* member_ptr = leader_group_ptr->members[me];
        if(member_ptr == this) {
            break;
        } else if(member_ptr->subgroup_type_ptr == subgroup_type_ptr) {
            //If this member is also a leader,
            //then that means the current leader should stick behind.
            distance +=
                member_ptr->radius * 2 + MOB::GROUP_SPOT_INTERVAL;
        }
    }
    
    *final_spot = following_group->pos;
    *final_dist = distance;
}


/**
 * @brief Orders Pikmin from the group to leave the group, and head for the
 * specified nest, with the goal of being stored inside.
 * This function prioritizes less matured Pikmin, and ones closest to the nest.
 *
 * @param type Type of Pikmin to order.
 * @param n_ptr Nest to enter.
 * @param amount Amount of Pikmin of the given type to order.
 * @return Whether the specified number of Pikmin were successfully ordered.
 * Returns false if there were not enough Pikmin of that type in the group
 * to fulfill the order entirely.
 */
bool leader::order_pikmin_to_onion(
    const pikmin_type* type, pikmin_nest_struct* n_ptr, const size_t amount
) {
    //Find Pikmin of that type.
    vector<std::pair<dist, pikmin*>> candidates;
    size_t amount_ordered = 0;
    
    for(size_t m = 0; m < group->members.size(); ++m) {
        mob* mob_ptr = group->members[m];
        if(
            mob_ptr->type->category->id != MOB_CATEGORY_PIKMIN ||
            mob_ptr->type != type
        ) {
            continue;
        }
        
        candidates.push_back(
            std::make_pair(
                dist(mob_ptr->pos, n_ptr->m_ptr->pos),
                (pikmin*) mob_ptr
            )
        );
    }
    
    //Sort them by maturity first, distance second.
    std::sort(
        candidates.begin(),
        candidates.end(),
        [] (
            const std::pair<dist, pikmin*> &p1,
            const std::pair<dist, pikmin*> &p2
    ) -> bool {
        if(p1.second->maturity != p2.second->maturity) {
            return p1.second->maturity < p2.second->maturity;
        } else {
            return p1.first < p2.first;
        }
    }
    );
    
    //Order Pikmin, in order.
    for(size_t p = 0; p < candidates.size(); ++p) {
    
        pikmin* pik_ptr = candidates[p].second;
        mob_event* ev = pik_ptr->fsm.get_event(MOB_EV_GO_TO_ONION);
        if(!ev) continue;
        
        ev->run(pik_ptr, (void*) n_ptr);
        
        amount_ordered++;
        if(amount_ordered == amount) {
            return true;
        }
    }
    
    //If it got here, that means we couldn't order enough Pikmin to fulfill
    //the requested amount.
    return false;
}


/**
 * @brief Queues up a throw. This will cause the throw to go through whenever
 * the throw cooldown ends.
 */
void leader::queue_throw() {
    if(!check_throw_ok()) {
        return;
    }
    
    throw_queued = true;
}


/**
 * @brief Signals the group members that the swarm mode stopped.
 */
void leader::signal_swarm_end() const {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EV_SWARM_ENDED);
    }
}


/**
 * @brief Signals the group members that the swarm mode started.
 */
void leader::signal_swarm_start() const {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EV_SWARM_STARTED);
    }
}


/**
 * @brief Starts the auto-throw mode.
 */
void leader::start_auto_throwing() {
    auto_throwing = true;
    auto_throw_cooldown = 0.0f;
    auto_throw_cooldown_duration = LEADER::AUTO_THROW_COOLDOWN_MAX_DURATION;
}


/**
 * @brief Starts the particle generator that leaves a trail behind a
 * thrown Pikmin.
 */
void leader::start_throw_trail() {
    particle throw_p(
        PARTICLE_TYPE_CIRCLE, pos, z,
        radius, 0.6, PARTICLE_PRIORITY_LOW
    );
    throw_p.size_grow_speed = -5;
    throw_p.color = change_alpha(type->main_color, 128);
    particle_generator pg(MOB::THROW_PARTICLE_INTERVAL, throw_p, 1);
    pg.follow_mob = this;
    pg.id = MOB_PARTICLE_GENERATOR_THROW;
    particle_generators.push_back(pg);
}


/**
 * @brief Makes the leader start whistling.
 */
void leader::start_whistling() {
    game.states.gameplay->whistle.start_whistling();
    
    size_t whistling_sfx_idx =
        lea_type->sfx_data_idxs[LEADER_SOUND_WHISTLING];
    if(whistling_sfx_idx != INVALID) {
        mob_type::sfx_struct* whistling_sfx =
            &type->sounds[whistling_sfx_idx];
        whistle_sfx_source_id =
            game.audio.create_world_pos_sfx_source(
                whistling_sfx->sample,
                game.states.gameplay->leader_cursor_w,
                whistling_sfx->config
            );
    }
    set_animation(LEADER_ANIM_WHISTLING);
    script_timer.start(2.5f);
    game.statistics.whistle_uses++;
}


/**
 * @brief Stops the auto-throw mode.
 */
void leader::stop_auto_throwing() {
    auto_throwing = false;
}


/**
 * @brief Makes the leader stop whistling.
 */
void leader::stop_whistling() {
    if(!game.states.gameplay->whistle.whistling) return;
    game.states.gameplay->whistle.stop_whistling();
    game.audio.destroy_sfx_source(whistle_sfx_source_id);
    whistle_sfx_source_id = 0;
}


/**
 * @brief Swaps out the currently held Pikmin for a different one.
 *
 * @param new_pik The new Pikmin to hold.
 */
void leader::swap_held_pikmin(mob* new_pik) {
    if(holding.empty()) return;
    
    mob* old_pik = holding[0];
    
    mob_event* old_pik_ev = old_pik->fsm.get_event(MOB_EV_RELEASED);
    mob_event* new_pik_ev = new_pik->fsm.get_event(MOB_EV_GRABBED_BY_FRIEND);
    
    group->sort(new_pik->subgroup_type_ptr);
    
    if(!old_pik_ev || !new_pik_ev) return;
    
    release(holding[0]);
    
    new_pik_ev->run(new_pik);
    hold(
        new_pik, INVALID,
        LEADER::HELD_GROUP_MEMBER_H_DIST, LEADER::HELD_GROUP_MEMBER_ANGLE,
        LEADER::HELD_GROUP_MEMBER_V_DIST,
        false, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void leader::tick_class_specifics(const float delta_t) {
    //Throw-related things.
    if(auto_throw_cooldown > 0.0f) {
        auto_throw_cooldown -= delta_t;
    }
    if(throw_cooldown > 0.0f) {
        throw_cooldown -= delta_t;
    }
    
    if(auto_throwing && auto_throw_cooldown <= 0.0f) {
        bool grabbed = grab_closest_group_member();
        if(grabbed) {
            queue_throw();
        }
        auto_throw_cooldown = auto_throw_cooldown_duration;
    }
    
    if(
        throw_queued &&
        throw_cooldown <= 0.0f &&
        check_throw_ok()
    ) {
        fsm.run_event(LEADER_EV_THROW);
        update_throw_variables();
        throw_cooldown = LEADER::THROW_COOLDOWN_DURATION;
        throw_queued = false;
    }
    
    if(throw_cooldown <= 0.0f) {
        throw_queued = false;
    }
    
    auto_throw_cooldown_duration =
        std::max(
            auto_throw_cooldown_duration -
            LEADER::AUTO_THROW_COOLDOWN_SPEED * delta_t,
            LEADER::AUTO_THROW_COOLDOWN_MIN_DURATION
        );
        
    if(group && group->members.empty()) {
        stop_auto_throwing();
    }
    
    if(game.states.gameplay->whistle.whistling) {
        game.audio.set_sfx_source_pos(
            whistle_sfx_source_id,
            game.states.gameplay->leader_cursor_w
        );
    }
    
    //Health wheel logic.
    health_wheel_visible_ratio +=
        ((health / max_health) - health_wheel_visible_ratio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * delta_t);
        
    if(
        health < max_health * LEADER::HEALTH_CAUTION_RATIO ||
        health_wheel_caution_timer > 0.0f
    ) {
        health_wheel_caution_timer += delta_t;
        if(health_wheel_caution_timer >= LEADER::HEALTH_CAUTION_RING_DURATION) {
            health_wheel_caution_timer = 0.0f;
        }
    }
}


/**
 * @brief Updates variables related to how the leader's throw would go.
 */
void leader::update_throw_variables() {
    throwee = NULL;
    if(!holding.empty()) {
        throwee = holding[0];
    } else if(game.states.gameplay->cur_leader_ptr == this) {
        throwee = game.states.gameplay->closest_group_member[BUBBLE_CURRENT];
    }
    
    if(!throwee) {
        return;
    }
    
    float target_z;
    if(game.states.gameplay->throw_dest_mob) {
        target_z =
            game.states.gameplay->throw_dest_mob->z +
            game.states.gameplay->throw_dest_mob->height;
    } else if(game.states.gameplay->throw_dest_sector) {
        target_z = game.states.gameplay->throw_dest_sector->z;
    } else {
        target_z = z;
    }
    
    float max_height;
    switch (throwee->type->category->id) {
    case MOB_CATEGORY_PIKMIN: {
        max_height = ((pikmin*) throwee)->pik_type->max_throw_height;
        break;
    } case MOB_CATEGORY_LEADERS: {
        max_height = ((leader*) throwee)->lea_type->max_throw_height;
        break;
    } default: {
        max_height = std::max(128.0f, (target_z - z) * 1.2f);
        break;
    }
    }
    
    //Due to floating point inaccuracies, it's hard for mobs to actually
    //reach the intended value. Let's bump it up just a smidge.
    max_height += 0.5f;
    
    if(max_height >= (target_z - z)) {
        //Can reach.
        throwee_can_reach = true;
    } else {
        //Can't reach! Just do a convincing throw that is sure to fail.
        //Limiting the "target" Z makes it so the horizontal velocity isn't
        //so wild.
        target_z = z + max_height * 0.75;
        throwee_can_reach = false;
    }
    
    throwee_max_z = z + max_height;
    
    calculate_throw(
        pos,
        z,
        game.states.gameplay->throw_dest,
        target_z,
        max_height,
        MOB::GRAVITY_ADDER,
        &throwee_speed,
        &throwee_speed_z,
        &throwee_angle
    );
}


/**
 * @brief Switch active leader.
 *
 * @param forward If true, switch to the next one. If false, to the previous.
 * @param force_success If true, switch to this leader even if they can't
 * currently handle the leader switch script event.
 * @param keep_idx If true, swap to a leader that has the same index in the
 * list of available leaders as the current one does.
 * Usually this is used because the current leader is no longer available.
 */
void change_to_next_leader(
    const bool forward, const bool force_success, const bool keep_idx
) {
    if(game.states.gameplay->available_leaders.empty()) {
        //There are no leaders remaining. Set the current leader to none.
        game.states.gameplay->cur_leader_nr = INVALID;
        game.states.gameplay->cur_leader_ptr = NULL;
        game.states.gameplay->update_closest_group_members();
        return;
    }
    
    if(
        game.states.gameplay->available_leaders.size() == 1 &&
        game.states.gameplay->cur_leader_ptr &&
        !keep_idx
    ) {
        return;
    }
    
    if(
        (
            game.states.gameplay->cur_leader_ptr &&
            !game.states.gameplay->cur_leader_ptr->fsm.get_event(
                LEADER_EV_INACTIVATED
            )
        ) &&
        !force_success
    ) {
        //This leader isn't ready to be switched out of. Forget it.
        return;
    }
    
    //We'll send the switch event to the next leader on the list.
    //If they accept, they run a function to change leaders.
    //If not, we try the next leader.
    //If we return to the current leader without anything being
    //changed, then stop trying; no leader can be switched to.
    
    int new_leader_nr = (int) game.states.gameplay->cur_leader_nr;
    if(keep_idx) {
        forward ? new_leader_nr-- : new_leader_nr++;
    }
    leader* new_leader_ptr = NULL;
    bool searching = true;
    leader* original_leader_ptr = game.states.gameplay->cur_leader_ptr;
    bool cant_find_new_leader = false;
    bool success = false;
    
    while(searching) {
        new_leader_nr =
            sum_and_wrap(
                new_leader_nr,
                (forward ? 1 : -1),
                (int) game.states.gameplay->available_leaders.size()
            );
        new_leader_ptr = game.states.gameplay->available_leaders[new_leader_nr];
        
        if(new_leader_ptr == original_leader_ptr) {
            //Back to the original; stop trying.
            cant_find_new_leader = true;
            searching = false;
        }
        
        new_leader_ptr->fsm.run_event(LEADER_EV_ACTIVATED);
        
        //If after we called the event, the leader is the same,
        //then that means the leader can't be switched to.
        //Try a new one.
        if(game.states.gameplay->cur_leader_ptr != original_leader_ptr) {
            searching = false;
            success = true;
        }
    }
    
    if(cant_find_new_leader && force_success) {
        //Ok, we need to force a leader to accept the focus. Let's do so.
        game.states.gameplay->cur_leader_nr =
            sum_and_wrap(
                new_leader_nr,
                (forward ? 1 : -1),
                (int) game.states.gameplay->available_leaders.size()
            );
        game.states.gameplay->cur_leader_ptr =
            game.states.gameplay->
            available_leaders[game.states.gameplay->cur_leader_nr];
            
        game.states.gameplay->cur_leader_ptr->fsm.set_state(
            LEADER_STATE_ACTIVE
        );
        success = true;
    }
    
    if(success) {
        game.states.gameplay->update_closest_group_members();
        game.states.gameplay->cur_leader_ptr->swarm_arrows.clear();
    }
}


/**
 * @brief Makes the current leader grab the closest group member of the
 * standby type.
 *
 * @return Whether it succeeded.
 */
bool grab_closest_group_member() {
    if(!game.states.gameplay->cur_leader_ptr) return false;
    
    //Check if there is even a closest group member.
    if(!game.states.gameplay->closest_group_member[BUBBLE_CURRENT]) {
        return false;
    }
    
    //Check if the leader can grab, and the group member can be grabbed.
    mob_event* grabbed_ev =
        game.states.gameplay->
        closest_group_member[BUBBLE_CURRENT]->fsm.get_event(
            MOB_EV_GRABBED_BY_FRIEND
        );
    mob_event* grabber_ev =
        game.states.gameplay->cur_leader_ptr->fsm.get_event(
            LEADER_EV_HOLDING
        );
    if(!grabber_ev || !grabbed_ev) {
        return false;
    }
    
    //Check if there's anything in the way.
    if(
        !game.states.gameplay->cur_leader_ptr->has_clear_line(
            game.states.gameplay->closest_group_member[BUBBLE_CURRENT]
        )
    ) {
        return false;
    }
    
    //Run the grabbing logic then.
    grabber_ev->run(
        game.states.gameplay->cur_leader_ptr,
        (void*) game.states.gameplay->closest_group_member[BUBBLE_CURRENT]
    );
    grabbed_ev->run(
        game.states.gameplay->closest_group_member[BUBBLE_CURRENT],
        (void*) game.states.gameplay->cur_leader_ptr
    );
    
    return true;
}
