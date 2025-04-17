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

#include "../../core/const.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"

namespace LEADER {

//Auto-throw ends at this interval.
const float AUTO_THROW_FASTEST_INTERVAL = THROW_COOLDOWN_DURATION * 1.2f;

//Auto-throw takes this long to go from slow to fast.
const float AUTO_THROW_RAMP_TIME = 1.0f;

//Auto-throw starts at this interval.
const float AUTO_THROW_SLOWEST_INTERVAL = 0.5f;

//Maximum amount of time for the random boredom animation delay.
const float BORED_ANIM_MAX_DELAY = 5.0f;

//Minimum amount of time for hte random boredom animation delay.
const float BORED_ANIM_MIN_DELAY = 1.0f;

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

//Invulnerability period after getting knocked back.
const float INVULN_PERIOD_KB = 2.5f;

//Invulnerability period after getting hit.
const float INVULN_PERIOD_NORMAL = 1.5f;

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
Leader::Leader(const Point &pos, LeaderType* type, float angle) :
    Mob(pos, type, angle),
    leaType(type),
    autoThrowRepeater(&game.autoThrowSettings) {
    
    team = MOB_TEAM_PLAYER_1;
    invulnPeriod = Timer(LEADER::INVULN_PERIOD_NORMAL);
    
    subgroupTypePtr =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
        
    swarmNextArrowTimer.onEnd = [this] () {
        swarmNextArrowTimer.start();
        swarmArrows.push_back(0);
        
        Particle p;
        unsigned char color_idx = game.rng.i(0, WHISTLE::N_DOT_COLORS);
        p.bitmap = game.sysContent.bmpBrightCircle;
        ALLEGRO_COLOR c =
            al_map_rgba(
                WHISTLE::DOT_COLORS[color_idx][0],
                WHISTLE::DOT_COLORS[color_idx][1],
                WHISTLE::DOT_COLORS[color_idx][2],
                LEADER::SWARM_PARTICLE_ALPHA * 255
            );
        p.color = KeyframeInterpolator<ALLEGRO_COLOR>(c);
        p.color.add(1, changeAlpha(c, 0));
        p.duration =
            game.rng.f(
                LEADER::SWARM_PARTICLE_MIN_DURATION,
                LEADER::SWARM_PARTICLE_MAX_DURATION
            );
        p.friction = LEADER::SWARM_PARTICLE_FRICTION;
        p.pos = this->pos;
        p.pos.x += game.rng.f(-this->radius * 0.5f, this->radius * 0.5f);
        p.pos.y += game.rng.f(-this->radius * 0.5f, this->radius * 0.5f);
        p.priority = PARTICLE_PRIORITY_MEDIUM;
        p.size.setKeyframeValue(0, LEADER::SWARM_PARTICLE_SIZE);
        float p_speed =
            game.states.gameplay->swarmMagnitude *
            LEADER::SWARM_PARTICLE_SPEED_MULT +
            game.rng.f(
                -LEADER::SWARM_PARTICLE_SPEED_DEVIATION,
                LEADER::SWARM_PARTICLE_SPEED_DEVIATION
            );
        float p_angle =
            game.states.gameplay->swarmAngle +
            game.rng.f(
                -LEADER::SWARM_PARTICLE_ANGLE_DEVIATION,
                LEADER::SWARM_PARTICLE_ANGLE_DEVIATION
            );
        p.linearSpeed = KeyframeInterpolator<Point>(rotatePoint(Point(p_speed, 0.0f), p_angle));
        p.time = p.duration;
        p.z = this->z + this->height / 2.0f;
        game.states.gameplay->particles.add(p);
    };
    swarmNextArrowTimer.start();
}


/**
 * @brief Returns whether or not a leader can grab a given group member.
 *
 * @param m Group member to check.
 * @return Whether it can grab.
 */
bool Leader::canGrabGroupMember(Mob* m) const {
    //Check if the leader is on a hazard that the member can't go to.
    if(
        groundSector &&
        groundSector->hazard &&
        !standingOnMob &&
        groundSector->hazard->blocksPaths &&
        m->getHazardVulnerability(groundSector->hazard).effectMult != 0.0f
    ) {
        return false;
    }
    
    //Check if the mob is within range.
    if(
        Distance(m->pos, pos) >
        game.config.leaders.groupMemberGrabRange
    ) {
        return false;
    }
    
    //Check if there's anything in the way.
    if(!hasClearLine(m)) {
        return false;
    }
    
    //Check if the mob isn't too far under the leader
    //when on the same height sector.
    if(z - m->z > GEOMETRY::STEP_HEIGHT &&
       centerSector->z == m->centerSector->z
       && standingOnMob == m->standingOnMob
      ) {
        return false;
    }
    
    //All good!
    return true;
}


/**
 * @brief Returns whether or not a leader can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive the status.
 */
bool Leader::canReceiveStatus(StatusType* s) const {
    return hasFlag(s->affects, STATUS_AFFECTS_FLAG_LEADERS);
}


/**
 * @brief Returns whether or not a leader can throw.
 *
 * @return Whether it can throw.
 */
bool Leader::checkThrowOk() const {
    if(holding.empty()) {
        return false;
    }
    
    MobEvent* ev = fsm.getEvent(LEADER_EV_THROW);
    
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
void Leader::dismiss() {
    size_t n_group_members = group->members.size();
    if(n_group_members == 0) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit for each subgroup, depending on a few factors.
    float base_angle;
    
    //First, calculate what direction the group should be dismissed to.
    if(game.states.gameplay->swarmMagnitude > 0) {
        //If the leader's swarming,
        //they should be dismissed in that direction.
        base_angle = game.states.gameplay->swarmAngle;
    } else {
        //Leftmost member coordinate, rightmost, etc.
        Point min_coords, max_coords;
        
        for(size_t m = 0; m < n_group_members; m++) {
            Mob* member_ptr = group->members[m];
            
            if(member_ptr->pos.x < min_coords.x || m == 0)
                min_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.x > max_coords.x || m == 0)
                max_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.y < min_coords.y || m == 0)
                min_coords.y = member_ptr->pos.y;
            if(member_ptr->pos.y > max_coords.y || m == 0)
                max_coords.y = member_ptr->pos.y;
        }
        
        Point group_center(
            (min_coords.x + max_coords.x) / 2,
            (min_coords.y + max_coords.y) / 2
        );
        base_angle = getAngle(pos, group_center);
    }
    
    /**
     * @brief Info about a group subgroup when being dismissed.
     */
    struct DismissSubgroup {
    
        //--- Members ---
        
        //Radius of the group.
        float radius;
        
        //Group members of this subgroup type.
        vector<Mob*> members;
        
        //Center point of the subgroup.
        Point center;
        
    };
    vector<DismissSubgroup> subgroups_info;
    
    //Go through all subgroups and populate the vector of data.
    SubgroupType* first_type =
        game.states.gameplay->subgroupTypes.getFirstType();
    SubgroupType* cur_type = first_type;
    
    do {
    
        if(
            cur_type !=
            game.states.gameplay->subgroupTypes.getType(
                SUBGROUP_TYPE_CATEGORY_LEADER
            )
        ) {
        
            bool subgroup_exists = false;
            
            for(size_t m = 0; m < n_group_members; m++) {
                Mob* m_ptr = group->members[m];
                if(m_ptr->subgroupTypePtr != cur_type) continue;
                
                if(!subgroup_exists) {
                    subgroups_info.push_back(DismissSubgroup());
                    subgroup_exists = true;
                }
                
                subgroups_info.back().members.push_back(m_ptr);
            }
            
        }
        
        cur_type =
            game.states.gameplay->subgroupTypes.getNextType(cur_type);
            
    } while(cur_type != first_type);
    
    //Let's figure out each subgroup's size.
    //Subgroups will be made by placing the members in
    //rows of circles surrounding a central point.
    //The first row is just one spot.
    //The second row is 6 spots around that one.
    //The third is 12 spots around those 6.
    //And so on. Each row fits an additional 6.
    for(size_t s = 0; s < subgroups_info.size(); s++) {
        size_t n_rows = getDismissRows(subgroups_info[s].members.size());
        
        //Since each row loops all around,
        //it appears to the left and right of the center.
        //So count each one twice. Except for the central one.
        subgroups_info[s].radius =
            game.config.pikmin.standardRadius +
            game.config.pikmin.standardRadius * 2 *
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
    struct Row {
    
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
        
        Row() {
            dist_between_center = 0;
            thickness = 0;
            angle_occupation = 0;
        }
        
    };
    
    bool done = false;
    vector<Row> rows;
    Row cur_row;
    cur_row.dist_between_center = LEADER::DISMISS_SUBGROUP_DISTANCE;
    size_t cur_row_idx = 0;
    size_t cur_subgroup_idx = 0;
    
    while(!done && !subgroups_info.empty()) {
        float new_thickness =
            std::max(
                cur_row.thickness, subgroups_info[cur_subgroup_idx].radius * 2
            );
            
        float new_angle_occupation = 0;
        for(size_t s = 0; s < cur_row.subgroups.size(); s++) {
            new_angle_occupation +=
                linearDistToAngular(
                    subgroups_info[cur_row.subgroups[s]].radius * 2.0,
                    cur_row.dist_between_center +
                    cur_row.thickness / 2.0f
                );
            if(s < cur_row.subgroups.size() - 1) {
                new_angle_occupation +=
                    linearDistToAngular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        cur_row.dist_between_center +
                        cur_row.thickness / 2.0f
                    );
            }
        }
        if(!cur_row.subgroups.empty()) {
            new_angle_occupation +=
                linearDistToAngular(
                    LEADER::DISMISS_SUBGROUP_DISTANCE,
                    cur_row.dist_between_center +
                    new_thickness / 2.0f
                );
        }
        new_angle_occupation +=
            linearDistToAngular(
                subgroups_info[cur_subgroup_idx].radius * 2.0,
                cur_row.dist_between_center +
                new_thickness / 2.0f
            );
            
        //Will this group fit?
        if(new_angle_occupation <= LEADER::DISMISS_ANGLE_RANGE) {
            //This subgroup still fits. Next!
            cur_row.thickness = new_thickness;
            cur_row.angle_occupation = new_angle_occupation;
            
            cur_row.subgroups.push_back(cur_subgroup_idx);
            cur_subgroup_idx++;
        }
        
        if(
            new_angle_occupation > LEADER::DISMISS_ANGLE_RANGE ||
            cur_subgroup_idx == subgroups_info.size()
        ) {
            //This subgroup doesn't fit. It'll have to be put in the next row.
            //Or this is the last subgroup, and the row needs to be committed.
            
            rows.push_back(cur_row);
            cur_row_idx++;
            cur_row.dist_between_center +=
                cur_row.thickness + LEADER::DISMISS_SUBGROUP_DISTANCE;
            cur_row.subgroups.clear();
            cur_row.thickness = 0;
            cur_row.angle_occupation = 0;
        }
        
        if(cur_subgroup_idx == subgroups_info.size()) done = true;
    }
    
    //Now that we know which subgroups go into which row,
    //simply decide the positioning.
    for(size_t r = 0; r < rows.size(); r++) {
        float start_angle = -(rows[r].angle_occupation / 2.0f);
        float cur_angle = start_angle;
        
        for(size_t s = 0; s < rows[r].subgroups.size(); s++) {
            size_t s_idx = rows[r].subgroups[s];
            float subgroup_angle = cur_angle;
            
            cur_angle +=
                linearDistToAngular(
                    subgroups_info[s_idx].radius * 2.0,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
            if(s < rows[r].subgroups.size() - 1) {
                cur_angle +=
                    linearDistToAngular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        rows[r].dist_between_center + rows[r].thickness / 2.0
                    );
            }
            
            //Center the subgroup's angle.
            subgroup_angle +=
                linearDistToAngular(
                    subgroups_info[s_idx].radius,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
                
            subgroups_info[s_idx].center =
                angleToCoordinates(
                    base_angle + subgroup_angle,
                    rows[r].dist_between_center + rows[r].thickness / 2.0f
                );
                
        }
    }
    
    //Now, dismiss!
    for(size_t s = 0; s < subgroups_info.size(); s++) {
        cur_row_idx = 0;
        size_t cur_row_spot_idx = 0;
        size_t cur_row_spots = 1;
        
        for(size_t m = 0; m < subgroups_info[s].members.size(); m++) {
        
            Point destination;
            
            if(cur_row_idx == 0) {
                destination = subgroups_info[s].center;
            } else {
                float member_angle =
                    ((float) cur_row_spot_idx / cur_row_spots) * TAU;
                destination =
                    subgroups_info[s].center +
                    angleToCoordinates(
                        member_angle,
                        cur_row_idx * game.config.pikmin.standardRadius * 2 *
                        LEADER::DISMISS_MEMBER_SIZE_MULTIPLIER
                    );
            }
            
            destination +=
                Point(
                    game.rng.f(-5.0, 5.0),
                    game.rng.f(-5.0, 5.0)
                );
                
            cur_row_spot_idx++;
            if(cur_row_spot_idx == cur_row_spots) {
                cur_row_idx++;
                cur_row_spot_idx = 0;
                if(cur_row_idx == 1) {
                    cur_row_spots = 6;
                } else {
                    cur_row_spots += 6;
                }
            }
            
            destination += this->pos;
            
            subgroups_info[s].members[m]->leaveGroup();
            subgroups_info[s].members[m]->fsm.runEvent(
                MOB_EV_DISMISSED, (void*) &destination
            );
            
        }
    }
    
    //Dismiss leaders now.
    while(!group->members.empty()) {
        group->members[0]->fsm.runEvent(MOB_EV_DISMISSED, nullptr);
        group->members[0]->leaveGroup();
    }
    
    //Final things.
    playSound(leaType->soundDataIdxs[LEADER_SOUND_DISMISSING]);
    for(size_t p = 0; p < LEADER::DISMISS_PARTICLE_AMOUNT; p++) {
        Particle par;
        const unsigned char* color_idx =
            WHISTLE::DOT_COLORS[p % WHISTLE::N_DOT_COLORS];
        ALLEGRO_COLOR c =
            al_map_rgba(
                color_idx[0],
                color_idx[1],
                color_idx[2],
                LEADER::DISMISS_PARTICLE_ALPHA * 255
            );
            
        par.color.setKeyframeValue(0, c);
        par.color.add(1, changeAlpha(c, 0));
        par.bitmap = game.sysContent.bmpBrightCircle;
        par.duration =
            game.rng.f(
                LEADER::DISMISS_PARTICLE_MIN_DURATION,
                LEADER::DISMISS_PARTICLE_MAX_DURATION
            );
        par.friction = LEADER::DISMISS_PARTICLE_FRICTION;
        par.pos = pos;
        par.priority = PARTICLE_PRIORITY_MEDIUM;
        par.size.setKeyframeValue(0, LEADER::DISMISS_PARTICLE_SIZE);
        float par_speed =
            game.rng.f(
                LEADER::DISMISS_PARTICLE_MIN_SPEED,
                LEADER::DISMISS_PARTICLE_MAX_SPEED
            );
        float par_angle = TAU / LEADER::DISMISS_PARTICLE_AMOUNT * p;
        par.linearSpeed = KeyframeInterpolator<Point>(rotatePoint(Point(par_speed, 0.0f), par_angle));
        par.time = par.duration;
        par.z = z + height / 2.0f;
        game.states.gameplay->particles.add(par);
    }
    setAnimation(LEADER_ANIM_DISMISSING);
}


/**
 * @brief Draw a leader mob.
 */
void Leader::drawMob() {
    Mob::drawMob();
    
    Sprite* cur_s_ptr;
    Sprite* next_s_ptr;
    float interpolation_factor;
    getSpriteData(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY
    );
    
    if(invulnPeriod.timeLeft > 0.0f) {
        Sprite* spark_s;
        game.sysContent.anmSparks.getSpriteData(
            &spark_s, nullptr, nullptr
        );
        
        if(spark_s && spark_s->bitmap) {
            BitmapEffect spark_eff = eff;
            Point size = getBitmapDimensions(cur_s_ptr->bitmap) * eff.scale;
            Point spark_size = getBitmapDimensions(spark_s->bitmap);
            spark_eff.scale = size / spark_size;
            drawBitmapWithEffects(spark_s->bitmap, spark_eff);
        }
    }
    
    drawStatusEffectBmp(this, eff);
}


/**
 * @brief Returns how many rows will be needed to fit all of the members.
 * Used to calculate how subgroup members will be placed when dismissing.
 *
 * @param n_members Total number of group members to dismiss.
 * @return The amount of rows.
 */
size_t Leader::getDismissRows(size_t n_members) const {
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
 * @param out_spot The final coordinates are returned here.
 * @param out_dist The final distance to those coordinates is returned here.
 */
void Leader::getGroupSpotInfo(
    Point* out_spot, float* out_dist
) const {
    out_spot->x = 0.0f;
    out_spot->y = 0.0f;
    *out_dist = 0.0f;
    
    if(!followingGroup || !followingGroup->group) {
        return;
    }
    
    Group* leader_group_ptr = followingGroup->group;
    
    float distance =
        followingGroup->radius +
        radius + game.config.pikmin.standardRadius;
        
    for(size_t me = 0; me < leader_group_ptr->members.size(); me++) {
        Mob* member_ptr = leader_group_ptr->members[me];
        if(member_ptr == this) {
            break;
        } else if(member_ptr->subgroupTypePtr == subgroupTypePtr) {
            //If this member is also a leader,
            //then that means the current leader should stick behind.
            distance +=
                member_ptr->radius * 2 + MOB::GROUP_SPOT_INTERVAL;
        }
    }
    
    *out_spot = followingGroup->pos;
    *out_dist = distance;
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
bool Leader::orderPikminToOnion(
    const PikminType* type, PikminNest* n_ptr, size_t amount
) {
    //Find Pikmin of that type.
    vector<std::pair<Distance, Pikmin*>> candidates;
    size_t amount_ordered = 0;
    
    for(size_t m = 0; m < group->members.size(); m++) {
        Mob* mob_ptr = group->members[m];
        if(
            mob_ptr->type->category->id != MOB_CATEGORY_PIKMIN ||
            mob_ptr->type != type
        ) {
            continue;
        }
        
        candidates.push_back(
            std::make_pair(
                Distance(mob_ptr->pos, n_ptr->m_ptr->pos),
                (Pikmin*) mob_ptr
            )
        );
    }
    
    //Sort them by maturity first, distance second.
    std::sort(
        candidates.begin(),
        candidates.end(),
        [] (
            const std::pair<Distance, Pikmin*> &p1,
            const std::pair<Distance, Pikmin*> &p2
    ) -> bool {
        if(p1.second->maturity != p2.second->maturity) {
            return p1.second->maturity < p2.second->maturity;
        } else {
            return p1.first < p2.first;
        }
    }
    );
    
    //Order Pikmin, in order.
    for(size_t p = 0; p < candidates.size(); p++) {
    
        Pikmin* pik_ptr = candidates[p].second;
        MobEvent* ev = pik_ptr->fsm.getEvent(MOB_EV_GO_TO_ONION);
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
void Leader::queueThrow() {
    if(!checkThrowOk()) {
        return;
    }
    
    throwQueued = true;
}


/**
 * @brief Signals the group members that the swarm mode stopped.
 */
void Leader::signalSwarmEnd() const {
    for(size_t m = 0; m < group->members.size(); m++) {
        group->members[m]->fsm.runEvent(MOB_EV_SWARM_ENDED);
    }
}


/**
 * @brief Signals the group members that the swarm mode started.
 */
void Leader::signalSwarmStart() const {
    for(size_t m = 0; m < group->members.size(); m++) {
        group->members[m]->fsm.runEvent(MOB_EV_SWARM_STARTED);
    }
}


/**
 * @brief Starts the auto-throw mode.
 */
void Leader::startAutoThrowing() {
    autoThrowRepeater.start();
    //Already do the first throw, but two frames from now. This is because
    //manual press players can only throw as quickly as two frames.
    autoThrowRepeater.nextTrigger = game.deltaT * 2.0f;
}


/**
 * @brief Starts the particle generator that leaves a trail behind a
 * thrown leader.
 */
void Leader::startThrowTrail() {
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parThrowTrail, this
        );
    pg.followZOffset = 0.0f;
    adjustKeyframeInterpolatorValues<float>(
        pg.baseParticle.size,
    [ = ] (const float & f) { return f * radius; }
    );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        pg.baseParticle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR new_c = c;
        new_c.r *= type->mainColor.r;
        new_c.g *= type->mainColor.g;
        new_c.b *= type->mainColor.b;
        new_c.a *= type->mainColor.a;
        return new_c;
    }
    );
    pg.id = MOB_PARTICLE_GENERATOR_ID_THROW;
    particleGenerators.push_back(pg);
}


/**
 * @brief Makes the leader start whistling.
 */
void Leader::startWhistling() {
    game.states.gameplay->whistle.startWhistling();
    
    size_t whistling_sound_idx =
        leaType->soundDataIdxs[LEADER_SOUND_WHISTLING];
    if(whistling_sound_idx != INVALID) {
        MobType::Sound* whistling_sound =
            &type->sounds[whistling_sound_idx];
        whistleSoundSourceId =
            game.audio.createPosSoundSource(
                whistling_sound->sample,
                game.states.gameplay->leaderCursorW, false,
                whistling_sound->config
            );
    }
    setAnimation(LEADER_ANIM_WHISTLING);
    scriptTimer.start(2.5f);
    game.statistics.whistleUses++;
}


/**
 * @brief Stops the auto-throw mode.
 */
void Leader::stopAutoThrowing() {
    autoThrowRepeater.stop();
}


/**
 * @brief Makes the leader stop whistling.
 */
void Leader::stopWhistling() {
    if(!game.states.gameplay->whistle.whistling) return;
    game.states.gameplay->whistle.stopWhistling();
    game.audio.destroySoundSource(whistleSoundSourceId);
    whistleSoundSourceId = 0;
}


/**
 * @brief Swaps out the currently held Pikmin for a different one.
 *
 * @param new_pik The new Pikmin to hold.
 */
void Leader::swapHeldPikmin(Mob* new_pik) {
    if(holding.empty()) return;
    
    Mob* old_pik = holding[0];
    
    MobEvent* old_pik_ev = old_pik->fsm.getEvent(MOB_EV_RELEASED);
    MobEvent* new_pik_ev = new_pik->fsm.getEvent(MOB_EV_GRABBED_BY_FRIEND);
    
    group->sort(new_pik->subgroupTypePtr);
    
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
void Leader::tickClassSpecifics(float delta_t) {
    //Throw-related things.
    if(throwCooldown > 0.0f) {
        throwCooldown -= delta_t;
    }
    
    size_t n_auto_throws = autoThrowRepeater.tick(delta_t);
    if(n_auto_throws > 0) {
        bool grabbed = grabClosestGroupMember();
        if(grabbed) {
            queueThrow();
        }
    }
    
    if(
        throwQueued &&
        throwCooldown <= 0.0f &&
        checkThrowOk()
    ) {
        fsm.runEvent(LEADER_EV_THROW);
        updateThrowVariables();
        throwCooldown = LEADER::THROW_COOLDOWN_DURATION;
        throwQueued = false;
    }
    
    if(throwCooldown <= 0.0f) {
        throwQueued = false;
    }
    
    if(group && group->members.empty()) {
        stopAutoThrowing();
    }
    
    if(game.states.gameplay->whistle.whistling) {
        game.audio.setSoundSourcePos(
            whistleSoundSourceId,
            game.states.gameplay->leaderCursorW
        );
    }
    
    //Health wheel logic.
    healthWheelVisibleRatio +=
        ((health / maxHealth) - healthWheelVisibleRatio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * delta_t);
        
    if(
        health < maxHealth * LEADER::HEALTH_CAUTION_RATIO ||
        healthWheelCautionTimer > 0.0f
    ) {
        healthWheelCautionTimer += delta_t;
        if(healthWheelCautionTimer >= LEADER::HEALTH_CAUTION_RING_DURATION) {
            healthWheelCautionTimer = 0.0f;
        }
    }
}


/**
 * @brief Updates variables related to how the leader's throw would go.
 */
void Leader::updateThrowVariables() {
    throwee = nullptr;
    if(!holding.empty()) {
        throwee = holding[0];
    } else if(game.states.gameplay->curLeaderPtr == this) {
        throwee = game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT];
    }
    
    if(!throwee) {
        return;
    }
    
    float target_z;
    if(game.states.gameplay->throwDestMob) {
        target_z =
            game.states.gameplay->throwDestMob->z +
            game.states.gameplay->throwDestMob->height;
    } else if(game.states.gameplay->throwDestSector) {
        target_z = game.states.gameplay->throwDestSector->z;
    } else {
        target_z = z;
    }
    
    float max_height;
    switch (throwee->type->category->id) {
    case MOB_CATEGORY_PIKMIN: {
        max_height = ((Pikmin*) throwee)->pikType->maxThrowHeight;
        break;
    } case MOB_CATEGORY_LEADERS: {
        max_height = ((Leader*) throwee)->leaType->maxThrowHeight;
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
        throweeCanReach = true;
    } else {
        //Can't reach! Just do a convincing throw that is sure to fail.
        //Limiting the "target" Z makes it so the horizontal velocity isn't
        //so wild.
        target_z = z + max_height * 0.75;
        throweeCanReach = false;
    }
    
    throweeMaxZ = z + max_height;
    
    calculateThrow(
        pos,
        z,
        game.states.gameplay->throwDest,
        target_z,
        max_height,
        MOB::GRAVITY_ADDER,
        &throweeSpeed,
        &throweeSpeedZ,
        &throweeAngle
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
void changeToNextLeader(
    bool forward, bool force_success, bool keep_idx
) {
    if(game.states.gameplay->availableLeaders.empty()) {
        //There are no leaders remaining. Set the current leader to none.
        game.states.gameplay->curLeaderIdx = INVALID;
        game.states.gameplay->curLeaderPtr = nullptr;
        game.states.gameplay->updateClosestGroupMembers();
        return;
    }
    
    if(
        game.states.gameplay->availableLeaders.size() == 1 &&
        game.states.gameplay->curLeaderPtr &&
        !keep_idx
    ) {
        return;
    }
    
    if(
        (
            game.states.gameplay->curLeaderPtr &&
            !game.states.gameplay->curLeaderPtr->fsm.getEvent(
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
    
    int new_leader_idx = (int) game.states.gameplay->curLeaderIdx;
    if(keep_idx) {
        forward ? new_leader_idx-- : new_leader_idx++;
    }
    Leader* new_leader_ptr = nullptr;
    bool searching = true;
    Leader* original_leader_ptr = game.states.gameplay->curLeaderPtr;
    bool cant_find_new_leader = false;
    bool success = false;
    
    while(searching) {
        new_leader_idx =
            sumAndWrap(
                new_leader_idx,
                (forward ? 1 : -1),
                (int) game.states.gameplay->availableLeaders.size()
            );
        new_leader_ptr = game.states.gameplay->availableLeaders[new_leader_idx];
        
        if(new_leader_ptr == original_leader_ptr) {
            //Back to the original; stop trying.
            cant_find_new_leader = true;
            searching = false;
        }
        
        new_leader_ptr->fsm.runEvent(LEADER_EV_ACTIVATED);
        
        //If after we called the event, the leader is the same,
        //then that means the leader can't be switched to.
        //Try a new one.
        if(game.states.gameplay->curLeaderPtr != original_leader_ptr) {
            searching = false;
            success = true;
        }
    }
    
    if(cant_find_new_leader && force_success) {
        //Ok, we need to force a leader to accept the focus. Let's do so.
        game.states.gameplay->curLeaderIdx =
            sumAndWrap(
                new_leader_idx,
                (forward ? 1 : -1),
                (int) game.states.gameplay->availableLeaders.size()
            );
        game.states.gameplay->curLeaderPtr =
            game.states.gameplay->
            availableLeaders[game.states.gameplay->curLeaderIdx];
            
        game.states.gameplay->curLeaderPtr->fsm.setState(
            LEADER_STATE_ACTIVE
        );
        success = true;
    }
    
    if(success) {
        game.states.gameplay->updateClosestGroupMembers();
        game.states.gameplay->curLeaderPtr->swarmArrows.clear();
    }
}


/**
 * @brief Makes the current leader grab the closest group member of the
 * standby type.
 *
 * @return Whether it succeeded.
 */
bool grabClosestGroupMember() {
    if(!game.states.gameplay->curLeaderPtr) return false;
    
    //Check if there is even a closest group member.
    if(!game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        return false;
    }
    
    //Check if the leader can grab, and the group member can be grabbed.
    MobEvent* grabbed_ev =
        game.states.gameplay->
        closestGroupMember[BUBBLE_RELATION_CURRENT]->fsm.getEvent(
            MOB_EV_GRABBED_BY_FRIEND
        );
    MobEvent* grabber_ev =
        game.states.gameplay->curLeaderPtr->fsm.getEvent(
            LEADER_EV_HOLDING
        );
    if(!grabber_ev || !grabbed_ev) {
        return false;
    }
    
    //Check if there's anything in the way.
    if(
        !game.states.gameplay->curLeaderPtr->hasClearLine(
            game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT]
        )
    ) {
        return false;
    }
    
    //Run the grabbing logic then.
    grabber_ev->run(
        game.states.gameplay->curLeaderPtr,
        (void*) game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT]
    );
    grabbed_ev->run(
        game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT],
        (void*) game.states.gameplay->curLeaderPtr
    );
    
    return true;
}
