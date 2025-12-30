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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
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
const float HEALTH_CAUTION_RING_DURATION = 1.5f;

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

//Duration of the tidy single dismiss mode.
const float TIDY_SINGLE_DISMISS_DURATION = 3.0f;

}


/**
 * @brief Constructs a new leader object.
 *
 * @param pos Starting coordinates.
 * @param type Leader type this mob belongs to.
 * @param angle Starting angle.
 */
Leader::Leader(const Point& pos, LeaderType* type, float angle) :
    Mob(pos, type, angle),
    leaType(type),
    autoThrowRepeater(&game.autoThrowSettings),
    healthWheelShaker([] (float s, float t) { return simpleNoise(s, t); }) {
    
    team = MOB_TEAM_PLAYER_1;
    invulnPeriod = Timer(LEADER::INVULN_PERIOD_NORMAL);
    
    subgroupTypePtr =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
        
    swarmNextArrowTimer.onEnd = [this] () {
        if(!player) return;
        swarmNextArrowTimer.start();
        swarmArrows.push_back(0);
        
        Particle p;
        unsigned char colorIdx = game.rng.i(0, WHISTLE::N_DOT_COLORS);
        p.bitmap = game.sysContent.bmpBrightCircle;
        ALLEGRO_COLOR c =
            al_map_rgba(
                WHISTLE::DOT_COLORS[colorIdx][0],
                WHISTLE::DOT_COLORS[colorIdx][1],
                WHISTLE::DOT_COLORS[colorIdx][2],
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
        float pSpeed =
            player->swarmMagnitude *
            LEADER::SWARM_PARTICLE_SPEED_MULT +
            game.rng.f(
                -LEADER::SWARM_PARTICLE_SPEED_DEVIATION,
                LEADER::SWARM_PARTICLE_SPEED_DEVIATION
            );
        float pAngle =
            player->swarmAngle +
            game.rng.f(
                -LEADER::SWARM_PARTICLE_ANGLE_DEVIATION,
                LEADER::SWARM_PARTICLE_ANGLE_DEVIATION
            );
        p.linearSpeed =
            KeyframeInterpolator<Point>(
                rotatePoint(Point(pSpeed, 0.0f), pAngle)
            );
        p.time = p.duration;
        p.z = this->z + this->height / 2.0f;
        game.states.gameplay->particles.add(p);
    };
    swarmNextArrowTimer.start();
    
    ParticleGenerator antennaPG =
        standardParticleGenSetup(
            leaType->lightParticleGenIName, nullptr
        );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        antennaPG.baseParticle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR newColor = c;
        newColor.r *= leaType->lightParticleTint.r;
        newColor.g *= leaType->lightParticleTint.g;
        newColor.b *= leaType->lightParticleTint.b;
        newColor.a *= leaType->lightParticleTint.a;
        return newColor;
    }
    );
    antennaPG.id = MOB_PARTICLE_GENERATOR_ID_ANTENNA;
    particleGenerators.push_back(antennaPG);
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
        groundSector->hazard->blocksPaths
    ) {
        MobType::Vulnerability vuln =
            m->getHazardVulnerability(groundSector->hazard);
        if(vuln.effectMult != 0.0f) {
            return false;
        }
        if(vuln.effectMult == 0.0f && vuln.invulnBlockedBySectors) {
            return false;
        }
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
    if(
        z - m->z > GEOMETRY::STEP_HEIGHT &&
        centerSector->z == m->centerSector->z &&
        standingOnMob == m->standingOnMob
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
 * @brief Makes a leader (try to) dismiss their group.
 * The group is then organized in subgroups, by type,
 * and is dismissed close to the leader.
 *
 * @param subtle If true, no sounds or particles will happen.
 */
void Leader::dismiss(bool subtle) {
    setAnimation(LEADER_ANIM_DISMISSING);
    if(!subtle) {
        dismissDetails();
    }
    if(!group->members.empty()) {
        dismissLogic();
    }
}


/**
 * @brief Runs the aesthetic and secondary details about dismissing.
 */
void Leader::dismissDetails() {
    //Sound.
    MobType::Sound* sound =
        &type->sounds[leaType->soundDataIdxs[LEADER_SOUND_DISMISSING]];
    SoundSourceConfig soundConfig = sound->config;
    soundConfig.speed =
        group->members.empty() ?
        0.9f :
        tidySingleDismissTime > 0.0f ?
        1.03f :
        1.0f;
    game.audio.createMobSoundSource(sound->sample, this, false, soundConfig);
    
    //Particles.
    unsigned char particleAlpha =
        LEADER::DISMISS_PARTICLE_ALPHA * 255 *
        (group->members.empty() ? 0.75f : 1.0f);
    for(size_t p = 0; p < LEADER::DISMISS_PARTICLE_AMOUNT; p++) {
        Particle par;
        const unsigned char* colorIdx =
            WHISTLE::DOT_COLORS[p % WHISTLE::N_DOT_COLORS];
        ALLEGRO_COLOR c =
            al_map_rgba(colorIdx[0], colorIdx[1], colorIdx[2], particleAlpha);
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
        par.size.setKeyframeValue(
            0,
            LEADER::DISMISS_PARTICLE_SIZE *
            (group->members.empty() ? 0.75f : 1.0f)
        );
        float parSpeed =
            game.rng.f(
                LEADER::DISMISS_PARTICLE_MIN_SPEED,
                LEADER::DISMISS_PARTICLE_MAX_SPEED
            );
        if(group->members.empty()) parSpeed *= 0.75f;
        float parAngle = TAU / LEADER::DISMISS_PARTICLE_AMOUNT * p;
        par.linearSpeed =
            KeyframeInterpolator<Point>(
                rotatePoint(Point(parSpeed, 0.0f), parAngle)
            );
        par.time = par.duration;
        par.z = z + height / 2.0f;
        game.states.gameplay->particles.add(par);
    }
}


/**
 * @brief Runs the logic to actually separate, position, and disband
 * the group for a dismiss action.
 */
void Leader::dismissLogic() {
    if(!player) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit for each subgroup, depending on a few factors.
    float baseAngle;
    
    //First, calculate what direction the group should be dismissed to.
    if(player->swarmMagnitude > 0) {
        //If the leader's swarming,
        //they should be dismissed in that direction.
        baseAngle = player->swarmAngle;
    } else {
        //Leftmost member coordinate, rightmost, etc.
        Point minCoords, maxCoords;
        
        for(size_t m = 0; m < group->members.size(); m++) {
            Mob* memberPtr = group->members[m];
            
            if(memberPtr->pos.x < minCoords.x || m == 0)
                minCoords.x = memberPtr->pos.x;
            if(memberPtr->pos.x > maxCoords.x || m == 0)
                maxCoords.x = memberPtr->pos.x;
            if(memberPtr->pos.y < minCoords.y || m == 0)
                minCoords.y = memberPtr->pos.y;
            if(memberPtr->pos.y > maxCoords.y || m == 0)
                maxCoords.y = memberPtr->pos.y;
        }
        
        Point groupCenter(
            (minCoords.x + maxCoords.x) / 2,
            (minCoords.y + maxCoords.y) / 2
        );
        baseAngle = getAngle(pos, groupCenter);
    }
    
    /**
     * @brief Info about a group subgroup when being dismissed.
     */
    struct DismissSubgroup {
    
        //--- Members ---
        
        //Subgroup type.
        SubgroupType* type = nullptr;
        
        //Radius of the group.
        float radius = 0.0f;
        
        //Group members of this subgroup type.
        vector<Mob*> members;
        
        //Center point of the subgroup, relative to the leader.
        Point center;
        
    };
    vector<DismissSubgroup> subgroupsInfo;
    
    //Go through all subgroups and populate the vector of data.
    SubgroupType* firstType =
        game.states.gameplay->subgroupTypes.getFirstType();
    SubgroupType* curType = firstType;
    
    do {
    
        if(
            curType !=
            game.states.gameplay->subgroupTypes.getType(
                SUBGROUP_TYPE_CATEGORY_LEADER
            )
        ) {
        
            bool subgroupExists = false;
            
            for(size_t m = 0; m < group->members.size(); m++) {
                Mob* mPtr = group->members[m];
                if(mPtr->subgroupTypePtr != curType) continue;
                
                if(!subgroupExists) {
                    subgroupsInfo.push_back(DismissSubgroup());
                    subgroupsInfo.back().type = curType;
                    subgroupExists = true;
                }
                
                subgroupsInfo.back().members.push_back(mPtr);
            }
            
        }
        
        curType =
            game.states.gameplay->subgroupTypes.getNextType(curType);
            
    } while(curType != firstType);
    
    bool keepCurType =
        !game.options.misc.dismissAll &&
        subgroupsInfo.size() > 1;
        
    //Let's move the current standby type to the first element.
    //This way, when dismissing all Pikmin while keeping the standby type,
    //there will be a gap where that group would go, and dismissing again
    //will place those Pikmin in that missing group's place.
    auto firstSubgroup =
        std::find_if(
            subgroupsInfo.begin(), subgroupsInfo.end(),
    [this] (const auto & s) -> bool {
        return s.type == this->group->curStandbyType;
    }
        );
    if(firstSubgroup != subgroupsInfo.end()) {
        std::rotate(subgroupsInfo.begin(), firstSubgroup, firstSubgroup + 1);
    }
    
    //Let's figure out each subgroup's size.
    //Subgroups will be made by placing the members in
    //rows of circles surrounding a central point.
    //The first row is just one spot.
    //The second row is 6 spots around that one.
    //The third is 12 spots around those 6.
    //And so on. Each row fits an additional 6.
    for(size_t s = 0; s < subgroupsInfo.size(); s++) {
        size_t nRows = getDismissRows(subgroupsInfo[s].members.size());
        
        //Since each row loops all around,
        //it appears to the left and right of the center.
        //So count each one twice. Except for the central one.
        subgroupsInfo[s].radius =
            game.config.pikmin.standardRadius +
            game.config.pikmin.standardRadius * 2 *
            LEADER::DISMISS_MEMBER_SIZE_MULTIPLIER * (nRows - 1);
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
        float distBetweenCenter;
        
        //How thick this row is.
        float thickness;
        
        //How much is taken up by Pikmin and padding.
        float angleOccupation;
        
        
        //--- Function definitions ---
        
        Row() {
            distBetweenCenter = 0;
            thickness = 0;
            angleOccupation = 0;
        }
        
    };
    
    bool done = false;
    vector<Row> rows;
    Row curRow;
    curRow.distBetweenCenter = LEADER::DISMISS_SUBGROUP_DISTANCE;
    size_t curSubgroupIdx = 0;
    
    while(!done && !subgroupsInfo.empty()) {
        float newThickness =
            std::max(
                curRow.thickness, subgroupsInfo[curSubgroupIdx].radius * 2
            );
            
        float newAngleOccupation = 0;
        for(size_t s = 0; s < curRow.subgroups.size(); s++) {
            newAngleOccupation +=
                linearDistToAngular(
                    subgroupsInfo[curRow.subgroups[s]].radius * 2.0,
                    curRow.distBetweenCenter +
                    curRow.thickness / 2.0f
                );
            if(s < curRow.subgroups.size() - 1) {
                newAngleOccupation +=
                    linearDistToAngular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        curRow.distBetweenCenter +
                        curRow.thickness / 2.0f
                    );
            }
        }
        if(!curRow.subgroups.empty()) {
            newAngleOccupation +=
                linearDistToAngular(
                    LEADER::DISMISS_SUBGROUP_DISTANCE,
                    curRow.distBetweenCenter +
                    newThickness / 2.0f
                );
        }
        newAngleOccupation +=
            linearDistToAngular(
                subgroupsInfo[curSubgroupIdx].radius * 2.0,
                curRow.distBetweenCenter +
                newThickness / 2.0f
            );
            
        //Will this group fit?
        if(newAngleOccupation <= LEADER::DISMISS_ANGLE_RANGE) {
            //This subgroup still fits. Next!
            curRow.thickness = newThickness;
            curRow.angleOccupation = newAngleOccupation;
            
            curRow.subgroups.push_back(curSubgroupIdx);
            curSubgroupIdx++;
        }
        
        if(
            newAngleOccupation > LEADER::DISMISS_ANGLE_RANGE ||
            curSubgroupIdx == subgroupsInfo.size()
        ) {
            //This subgroup doesn't fit. It'll have to be put in the next row.
            //Or this is the last subgroup, and the row needs to be committed.
            
            rows.push_back(curRow);
            curRow.distBetweenCenter +=
                curRow.thickness + LEADER::DISMISS_SUBGROUP_DISTANCE;
            curRow.subgroups.clear();
            curRow.thickness = 0;
            curRow.angleOccupation = 0;
        }
        
        if(curSubgroupIdx == subgroupsInfo.size()) done = true;
    }
    
    //Now that we know which subgroups go into which row,
    //simply decide the positioning.
    for(size_t r = 0; r < rows.size(); r++) {
        float startAngle = -(rows[r].angleOccupation / 2.0f);
        float curAngle = startAngle;
        
        for(size_t s = 0; s < rows[r].subgroups.size(); s++) {
            size_t sIdx = rows[r].subgroups[s];
            float subgroupAngle = curAngle;
            
            curAngle +=
                linearDistToAngular(
                    subgroupsInfo[sIdx].radius * 2.0,
                    rows[r].distBetweenCenter + rows[r].thickness / 2.0
                );
            if(s < rows[r].subgroups.size() - 1) {
                curAngle +=
                    linearDistToAngular(
                        LEADER::DISMISS_SUBGROUP_DISTANCE,
                        rows[r].distBetweenCenter + rows[r].thickness / 2.0
                    );
            }
            
            //Center the subgroup's angle.
            subgroupAngle +=
                linearDistToAngular(
                    subgroupsInfo[sIdx].radius,
                    rows[r].distBetweenCenter + rows[r].thickness / 2.0
                );
                
            subgroupsInfo[sIdx].center =
                angleToCoordinates(
                    baseAngle + subgroupAngle,
                    rows[r].distBetweenCenter + rows[r].thickness / 2.0f
                );
                
        }
    }
    
    //Now, dismiss!
    if(tidySingleDismissTime > 0.0f && subgroupsInfo.size() == 1) {
        //We recently dismissed all other subgroups except this one.
        //Let's dismiss this single one towards where it would go if it got
        //dismissed alongside. That way all the Pikmin are organized tidily.
        specificDismiss(
            subgroupsInfo[0].members,
            tidySingleDismissRelCenter, tidySingleDismissLeaderPos
        );
        tidySingleDismissTime = 0.0f;
        
    } else {
        //Let's dismiss normally, possibly keeping the current standby type.
        for(size_t s = 0; s < subgroupsInfo.size(); s++) {
            if(
                subgroupsInfo[s].type == group->curStandbyType &&
                keepCurType
            ) {
                tidySingleDismissRelCenter = subgroupsInfo[s].center;
                tidySingleDismissLeaderPos = pos;
                tidySingleDismissTime = LEADER::TIDY_SINGLE_DISMISS_DURATION;
                continue;
            }
            
            specificDismiss(
                subgroupsInfo[s].members,
                subgroupsInfo[s].center, pos
            );
        }
        
    }
    
    //Dismiss leaders now.
    for(size_t m = 0; m < group->members.size(); m++) {
        if(group->members[m]->type->category->id == MOB_CATEGORY_LEADERS) {
            group->members[m]->fsm.runEvent(MOB_EV_DISMISSED, nullptr);
            group->members[m]->leaveGroup();
            m--;
        }
    }
}


/**
 * @brief Draw a leader mob.
 */
void Leader::drawMob() {
    Sprite* curSPtr;
    Sprite* nextSPtr;
    float interpolationFactor;
    getSpriteData(&curSPtr, &nextSPtr, &interpolationFactor);
    if(!curSPtr) return;
    
    //The leader themself.
    BitmapEffect mobEff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &mobEff,
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY |
        (type->useDamageSquashAndStretch ? SPRITE_BMP_EFFECT_DAMAGE : 0)
    );
    BitmapEffect leaSpriteEff = mobEff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &leaSpriteEff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        (type->useDamageSquashAndStretch ? SPRITE_BMP_EFFECT_DAMAGE : 0)
    );
    
    drawBitmapWithEffects(curSPtr->bitmap, leaSpriteEff);
    
    //Light.
    ParticleGenerator* antennaPG = nullptr;
    for(size_t p = 0; p < particleGenerators.size(); p++) {
        if(
            particleGenerators[p].id ==
            MOB_PARTICLE_GENERATOR_ID_ANTENNA
        ) {
            antennaPG = &particleGenerators[p];
            break;
        }
    }
    antennaPG->canEmit = false;
    
    if(curSPtr->topVisible && leaType->bmpLight) {
        Point lightCoords;
        float lightAngle;
        Point lightSize;
        BitmapEffect lightEff = mobEff;
        getSpriteBasicTopEffects(
            curSPtr, nextSPtr, interpolationFactor,
            &lightCoords, &lightAngle, &lightSize
        );
        //To get the height effect to work, we'll need to scale the translation
        //too, otherwise the light will detach from the leader visually as
        //the leader falls into a pit. The "right" scale is a bit of a guess
        //at this point in the code, but honestly, either X scale or Y scale
        //will work. In the off-chance they are different, using an average
        //will be more than enough.
        float avgScale =
            (lightEff.tf.scale.x + lightEff.tf.scale.y) / 2.0f;
        Point topBmpSize = getBitmapDimensions(leaType->bmpLight);
        lightEff.tf.trans +=
            pos + rotatePoint(lightCoords, angle) * avgScale;
        lightEff.tf.scale *= lightSize / topBmpSize;
        lightEff.tf.rot += angle + lightAngle;
        lightEff.tintColor = leaType->lightBmpTint;
        
        drawBitmapWithEffects(leaType->bmpLight, lightEff);
        
        //This is the best place to position the light particles, so do that.
        antennaPG->baseParticle.pos = lightEff.tf.trans;
        antennaPG->baseParticle.bmpAngle = lightEff.tf.rot;
        antennaPG->baseParticle.z = z + height + 1.0f;
        adjustKeyframeInterpolatorValues<float>(
            antennaPG->baseParticle.size,
        [ = ] (float s) {
            return std::max(lightSize.x, lightSize.y);
        }
        );
        antennaPG->canEmit = true;
    }
    
    //Invulnerability sparks.
    if(invulnPeriod.timeLeft > 0.0f) {
        Sprite* sparkS;
        game.sysContent.anmSparks.getSpriteData(
            &sparkS, nullptr, nullptr
        );
        
        if(sparkS && sparkS->bitmap) {
            BitmapEffect sparkEff = leaSpriteEff;
            Point size = getBitmapDimensions(curSPtr->bitmap) * mobEff.tf.scale;
            Point sparkSize = getBitmapDimensions(sparkS->bitmap);
            sparkEff.tf.scale = size / sparkSize;
            drawBitmapWithEffects(sparkS->bitmap, sparkEff);
        }
    }
    
    drawStatusEffectBmp(this, mobEff);
}


/**
 * @brief Returns how many Pikmin are in the group.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t Leader::getAmountOfGroupPikmin(const PikminType* filter) {
    size_t total = 0;
    
    for(size_t m = 0; m < group->members.size(); m++) {
        Mob* mPtr = group->members[m];
        if(mPtr->type->category->id != MOB_CATEGORY_PIKMIN) continue;
        if(filter && mPtr->type != filter) continue;
        total++;
    }
    
    return total;
}


/**
 * @brief Returns how many rows will be needed to fit all of the members.
 * Used to calculate how subgroup members will be placed when dismissing.
 *
 * @param nMembers Total number of group members to dismiss.
 * @return The amount of rows.
 */
size_t Leader::getDismissRows(size_t nMembers) const {
    size_t membersThatFit = 1;
    size_t rowsNeeded = 1;
    while(membersThatFit < nMembers) {
        rowsNeeded++;
        membersThatFit += 6 * (rowsNeeded - 1);
    }
    return rowsNeeded;
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param outSpot The final coordinates are returned here.
 * @param outDist The final distance to those coordinates is returned here.
 */
void Leader::getGroupSpotInfo(
    Point* outSpot, float* outDist
) const {
    outSpot->x = 0.0f;
    outSpot->y = 0.0f;
    *outDist = 0.0f;
    
    if(!followingGroup || !followingGroup->group) {
        return;
    }
    
    Group* leaderGroupPtr = followingGroup->group;
    
    float distance =
        followingGroup->radius +
        radius + game.config.pikmin.standardRadius;
        
    for(size_t me = 0; me < leaderGroupPtr->members.size(); me++) {
        Mob* memberPtr = leaderGroupPtr->members[me];
        if(memberPtr == this) {
            break;
        } else if(memberPtr->subgroupTypePtr == subgroupTypePtr) {
            //If this member is also a leader,
            //then that means the current leader should stick behind.
            distance +=
                memberPtr->radius * 2 + MOB::GROUP_SPOT_INTERVAL;
        }
    }
    
    *outSpot = followingGroup->pos;
    *outDist = distance;
}


/**
 * @brief Orders Pikmin from the group to leave the group, and head for the
 * specified nest, with the goal of being stored inside.
 * This function prioritizes less matured Pikmin, and ones closest to the nest.
 *
 * @param type Type of Pikmin to order.
 * @param nPtr Nest to enter.
 * @param amount Amount of Pikmin of the given type to order.
 * @return Whether the specified number of Pikmin were successfully ordered.
 * Returns false if there were not enough Pikmin of that type in the group
 * to fulfill the order entirely.
 */
bool Leader::orderPikminToOnion(
    const PikminType* type, PikminNest* nPtr, size_t amount
) {
    //Find Pikmin of that type.
    vector<std::pair<Distance, Pikmin*>> candidates;
    size_t amountOrdered = 0;
    
    for(size_t m = 0; m < group->members.size(); m++) {
        Mob* mobPtr = group->members[m];
        if(
            mobPtr->type->category->id != MOB_CATEGORY_PIKMIN ||
            mobPtr->type != type
        ) {
            continue;
        }
        
        candidates.push_back(
            std::make_pair(
                Distance(mobPtr->pos, nPtr->mPtr->pos),
                (Pikmin*) mobPtr
            )
        );
    }
    
    //Sort them by maturity first, distance second.
    std::sort(
        candidates.begin(),
        candidates.end(),
        [] (
            const std::pair<Distance, Pikmin*>& p1,
            const std::pair<Distance, Pikmin*>& p2
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
    
        Pikmin* pikPtr = candidates[p].second;
        MobEvent* ev = pikPtr->fsm.getEvent(MOB_EV_GO_TO_ONION);
        if(!ev) continue;
        
        ev->run(pikPtr, (void*) nPtr);
        
        amountOrdered++;
        if(amountOrdered == amount) {
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
 * @brief Dismisses some group members in a specific way.
 *
 * @param members Members to dismiss.
 * @param relCenter Center coordinates of where they will be dismissed to,
 * relative to the leader's position.
 * @param leaderPos Position of the leader to use.
 */
void Leader::specificDismiss(
    const vector<Mob*>& members,
    const Point& relCenter, const Point& leaderPos
) {
    size_t curRowidx = 0;
    size_t curRowSpotIdx = 0;
    size_t curRowSpots = 1;
    
    for(size_t m = 0; m < members.size(); m++) {
        Point destination;
        
        if(curRowidx == 0) {
            //The first Pikmin always goes to the dead center.
            destination = relCenter;
        } else {
            float memberAngle = ((float) curRowSpotIdx / curRowSpots) * TAU;
            destination =
                relCenter +
                angleToCoordinates(
                    memberAngle,
                    curRowidx * game.config.pikmin.standardRadius * 2 *
                    LEADER::DISMISS_MEMBER_SIZE_MULTIPLIER
                );
        }
        
        //Prepare the next row.
        curRowSpotIdx++;
        if(curRowSpotIdx == curRowSpots) {
            curRowidx++;
            curRowSpotIdx = 0;
            if(curRowidx == 1) {
                curRowSpots = 6;
            } else {
                curRowSpots += 6;
            }
        }
        
        //Fudge the location a bit so it looks more natural.
        destination += Point(game.rng.f(-5.0, 5.0), game.rng.f(-5.0, 5.0));
        destination += leaderPos;
        
        //Remove it from the group and order it to go to that spot.
        members[m]->leaveGroup();
        members[m]->fsm.runEvent(
            MOB_EV_DISMISSED, (void*) &destination
        );
        
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
        ALLEGRO_COLOR newColor = c;
        newColor.r *= type->mainColor.r;
        newColor.g *= type->mainColor.g;
        newColor.b *= type->mainColor.b;
        newColor.a *= type->mainColor.a;
        return newColor;
    }
    );
    pg.id = MOB_PARTICLE_GENERATOR_ID_THROW;
    particleGenerators.push_back(pg);
}


/**
 * @brief Makes the leader start whistling.
 */
void Leader::startWhistling() {
    if(!player) return;
    
    player->whistle.startWhistling();
    
    size_t whistlingSoundIdx =
        leaType->soundDataIdxs[LEADER_SOUND_WHISTLING];
    if(whistlingSoundIdx != INVALID) {
        MobType::Sound* whistlingSound =
            &type->sounds[whistlingSoundIdx];
        whistleSoundSourceId =
            game.audio.createPosSoundSource(
                whistlingSound->sample,
                player->leaderCursorWorld, false,
                whistlingSound->config
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
    if(!player) return;
    if(!player->whistle.whistling) return;
    player->whistle.stopWhistling();
    game.audio.destroySoundSource(whistleSoundSourceId);
    whistleSoundSourceId = 0;
}


/**
 * @brief Swaps out the currently held Pikmin for a different one.
 *
 * @param newPik The new Pikmin to hold.
 */
void Leader::swapHeldPikmin(Mob* newPik) {
    if(holding.empty()) return;
    
    Mob* oldPik = holding[0];
    
    MobEvent* oldPikEv = oldPik->fsm.getEvent(MOB_EV_RELEASED);
    MobEvent* newPikEv = newPik->fsm.getEvent(MOB_EV_GRABBED_BY_FRIEND);
    
    group->sort(newPik->subgroupTypePtr);
    
    if(!oldPikEv || !newPikEv) return;
    
    release(holding[0]);
    
    newPikEv->run(newPik);
    hold(
        newPik, INVALID,
        LEADER::HELD_GROUP_MEMBER_H_DIST, LEADER::HELD_GROUP_MEMBER_ANGLE,
        LEADER::HELD_GROUP_MEMBER_V_DIST,
        false, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Leader::tickClassSpecifics(float deltaT) {
    //Throw-related things.
    if(throwCooldown > 0.0f) {
        throwCooldown -= deltaT;
    }
    
    size_t nAutoThrows = autoThrowRepeater.tick(deltaT);
    if(nAutoThrows > 0) {
        bool grabbed = grabClosestGroupMember(player);
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
    
    //Others.
    if(player && player->whistle.whistling) {
        game.audio.setSoundSourcePos(
            whistleSoundSourceId, player->leaderCursorWorld
        );
    }
    
    if(tidySingleDismissTime > 0.0f) {
        tidySingleDismissTime -= deltaT;
    }
    
    healthWheelShaker.tick(deltaT);
    
    //Health wheel logic.
    healthWheelVisibleRatio +=
        ((health / maxHealth) - healthWheelVisibleRatio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * deltaT);
        
    if(
        health < maxHealth * LEADER::HEALTH_CAUTION_RATIO ||
        healthWheelCautionTimer > 0.0f
    ) {
        healthWheelCautionTimer += deltaT;
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
    if(!player) return;
    
    if(!holding.empty()) {
        throwee = holding[0];
    } else {
        throwee = player->closestGroupMember[BUBBLE_RELATION_CURRENT];
    }
    
    if(!throwee) {
        return;
    }
    
    float targetZ;
    if(player->throwDestMob) {
        targetZ = player->throwDestMob->z + player->throwDestMob->height;
    } else if(player->throwDestSector) {
        targetZ = player->throwDestSector->z;
    } else {
        targetZ = z;
    }
    
    float maxHeight;
    switch (throwee->type->category->id) {
    case MOB_CATEGORY_PIKMIN: {
        maxHeight = ((Pikmin*) throwee)->pikType->maxThrowHeight;
        break;
    } case MOB_CATEGORY_LEADERS: {
        maxHeight = ((Leader*) throwee)->leaType->maxThrowHeight;
        break;
    } default: {
        maxHeight = std::max(128.0f, (targetZ - z) * 1.2f);
        break;
    }
    }
    
    //Due to floating point inaccuracies, it's hard for mobs to actually
    //reach the intended value. Let's bump it up just a smidge.
    maxHeight += 0.5f;
    
    if(maxHeight >= (targetZ - z)) {
        //Can reach.
        throweeCanReach = true;
    } else {
        //Can't reach! Just do a convincing throw that is sure to fail.
        //Limiting the "target" Z makes it so the horizontal velocity isn't
        //so wild.
        targetZ = z + maxHeight * 0.75;
        throweeCanReach = false;
    }
    
    throweeMaxZ = z + maxHeight;
    
    calculateThrow(
        pos,
        z,
        player->throwDest,
        targetZ,
        maxHeight,
        MOB::GRAVITY_ADDER,
        &throweeSpeed,
        &throweeSpeedZ,
        &throweeAngle
    );
}


/**
 * @brief Switch active leader.
 *
 * @param player The player responsible.
 * @param forward If true, switch to the next one. If false, to the previous.
 * @param forceSuccess If true, switch to this leader even if they can't
 * currently handle the leader switch script event.
 * @param keepIdx If true, swap to a leader that has the same index in the
 * list of available leaders as the current one does.
 * Usually this is used because the current leader is no longer available.
 */
void changeToNextLeader(
    Player* player, bool forward, bool forceSuccess, bool keepIdx
) {
    if(game.states.gameplay->availableLeaders.empty()) {
        //There are no leaders remaining. Set the current leader to none.
        if(player->leaderPtr) player->leaderPtr->player = nullptr;
        player->leaderIdx = INVALID;
        player->leaderPtr = nullptr;
        game.states.gameplay->updateClosestGroupMembers(player);
        return;
    }
    
    if(
        game.states.gameplay->availableLeaders.size() == 1 &&
        player->leaderPtr && !keepIdx
    ) {
        return;
    }
    
    if(
        (
            player->leaderPtr &&
            !player->leaderPtr->fsm.getEvent(LEADER_EV_INACTIVATED)
        ) &&
        !forceSuccess
    ) {
        //This leader isn't ready to be switched out of. Forget it.
        return;
    }
    
    //We'll send the switch event to the next leader on the list.
    //If they accept, they run a function to change leaders.
    //If not, we try the next leader.
    //If we return to the current leader without anything being
    //changed, then stop trying; no leader can be switched to.
    
    int newLeaderIdx = (int) player->leaderIdx;
    if(keepIdx) {
        forward ? newLeaderIdx-- : newLeaderIdx++;
    }
    Leader* newLeaderPtr = nullptr;
    bool searching = true;
    Leader* originalLeaderPtr = player->leaderPtr;
    bool cantFindNewLeader = false;
    bool success = false;
    
    while(searching) {
        newLeaderIdx =
            sumAndWrap(
                newLeaderIdx,
                (forward ? 1 : -1),
                (int) game.states.gameplay->availableLeaders.size()
            );
        newLeaderPtr = game.states.gameplay->availableLeaders[newLeaderIdx];
        
        if(newLeaderPtr == originalLeaderPtr) {
            //Back to the original; stop trying.
            cantFindNewLeader = true;
            searching = false;
        }
        
        newLeaderPtr->fsm.runEvent(LEADER_EV_ACTIVATED, (void*) player);
        
        //If after we called the event, the leader is the same,
        //then that means the leader can't be switched to.
        //Try a new one.
        if(player->leaderPtr != originalLeaderPtr) {
            searching = false;
            success = true;
        }
    }
    
    if(cantFindNewLeader && forceSuccess) {
        //Ok, we need to force a leader to accept the focus. Let's do so.
        player->leaderIdx =
            sumAndWrap(
                newLeaderIdx,
                (forward ? 1 : -1),
                (int) game.states.gameplay->availableLeaders.size()
            );
        player->leaderPtr =
            game.states.gameplay->availableLeaders[player->leaderIdx];
            
        player->leaderPtr->fsm.setState(LEADER_STATE_ACTIVE);
        success = true;
    }
    
    if(success) {
        game.states.gameplay->updateClosestGroupMembers(player);
        player->leaderPtr->swarmArrows.clear();
        if(originalLeaderPtr) originalLeaderPtr->player = nullptr;
    }
}


/**
 * @brief Makes the current leader grab the closest group member of the
 * standby type.
 *
 * @param player The player responsible.
 * @return Whether it succeeded.
 */
bool grabClosestGroupMember(Player* player) {
    if(!player->leaderPtr) return false;
    
    //Check if there is even a closest group member.
    if(!player->closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        return false;
    }
    
    //Check if the leader can grab, and the group member can be grabbed.
    MobEvent* grabbedEv =
        player->closestGroupMember[BUBBLE_RELATION_CURRENT]->fsm.getEvent(
            MOB_EV_GRABBED_BY_FRIEND
        );
    MobEvent* grabberEv =
        player->leaderPtr->fsm.getEvent(LEADER_EV_HOLDING);
    if(!grabberEv || !grabbedEv) {
        return false;
    }
    
    //Check if there's anything in the way.
    if(
        !player->leaderPtr->hasClearLine(
            player->closestGroupMember[BUBBLE_RELATION_CURRENT]
        )
    ) {
        return false;
    }
    
    //Run the grabbing logic then.
    grabberEv->run(
        player->leaderPtr,
        (void*) player->closestGroupMember[BUBBLE_RELATION_CURRENT]
    );
    grabbedEv->run(
        player->closestGroupMember[BUBBLE_RELATION_CURRENT],
        (void*) player->leaderPtr
    );
    
    return true;
}
