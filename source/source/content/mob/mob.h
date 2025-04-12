/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob class and mob-related functions.
 */

#pragma once

#include <float.h>
#include <map>
#include <vector>

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../game_state/gameplay/in_world_hud.h"
#include "../../util/general_utils.h"
#include "../animation/animation.h"
#include "../area/sector.h"
#include "../other/mob_script_action.h"
#include "../other/mob_script.h"
#include "../other/particle.h"
#include "../other/status.h"
#include "mob_utils.h"


using std::map;
using std::size_t;
using std::string;
using std::vector;


class MobType;
class PikminType;

extern size_t next_mob_id;

namespace MOB {
extern const float CARRIED_MOB_ACCELERATION;
extern const float CARRY_STUCK_CIRCLING_RADIUS;
extern const float CARRY_STUCK_SPEED_MULTIPLIER;
extern const float CARRY_SWAY_TIME_MULT;
extern const float CARRY_SWAY_X_TRANSLATION_AMOUNT;
extern const float CARRY_SWAY_Y_TRANSLATION_AMOUNT;
extern const float CARRY_SWAY_ROTATION_AMOUNT;
extern const float DELIVERY_SUCK_SHAKING_TIME_MULT;
extern const float DELIVERY_SUCK_SHAKING_MULT;
extern const float DELIVERY_SUCK_TIME;
extern const float DELIVERY_TOSS_MULT;
extern const float DELIVERY_TOSS_TIME;
extern const float DELIVERY_TOSS_WINDUP_MULT;
extern const float DELIVERY_TOSS_X_OFFSET;
extern const float DAMAGE_SQUASH_DURATION;
extern const float DAMAGE_SQUASH_AMOUNT;
extern const float FREE_MOVE_THRESHOLD;
extern const float GRAVITY_ADDER;
extern const float GROUP_SHUFFLE_DIST;
extern const float GROUP_SPOT_INTERVAL;
extern const float GROUP_SPOT_MAX_DEVIATION;
extern const float HEIGHT_EFFECT_FACTOR;
extern const float KNOCKBACK_H_POWER;
extern const float KNOCKBACK_V_POWER;
extern const float MOB_SPEED_ANIM_MAX_MULT;
extern const float MOB_SPEED_ANIM_MIN_MULT;
extern const float OPPONENT_HIT_REGISTER_TIMEOUT;
extern const float PIKMIN_NEST_CALL_INTERVAL;
extern const float PUSH_EXTRA_AMOUNT;
extern const float PUSH_SOFTLY_AMOUNT;
extern const float PUSH_THROTTLE_FACTOR;
extern const float PUSH_THROTTLE_TIMEOUT;
extern const float SHADOW_STRETCH_MULT;
extern const float SHADOW_Y_MULT;
extern const float SMACK_PARTICLE_DUR;
extern const float SWARM_MARGIN;
extern const float SWARM_VERTICAL_SCALE;
extern const float STATUS_SHAKING_TIME_MULT;
};



/**
 * @brief A mob, short for "mobile object" or "map object",
 * or whatever tickles your fancy, is any instance of
 * an object in the game world. It can move, follow a point,
 * has health, and can be a variety of different sub-types,
 * like leader, Pikmin, enemy, Onion, etc.
 */
class Mob {

public:

    //--- Members ---
    
    //-Basic information-
    
    //What type of (generic) mob it is. (e.g. Olimar, Red Bulborb, etc.)
    MobType* type = nullptr;
    
    //Schedule this mob to be deleted from memory at the end of the frame.
    bool toDelete = false;
    
    //-Position-
    
    //Coordinates.
    Point pos;
    
    //Z coordinate. This is height; the higher the value, the higher in the sky.
    float z = 0.0f;
    
    //Current facing angle. 0 = right, PI / 2 = up, etc.
    float angle = 0.0f;
    
    //The highest ground below the entire mob.
    Sector* groundSector = nullptr;
    
    //Sector that the mob's center is on.
    Sector* centerSector = nullptr;
    
    //Mob this mob is standing on top of, if any.
    Mob* standingOnMob = nullptr;
    
    //-Basic movement-
    
    //X/Y speed at which external movement is applied (i.e. not walking).
    Point speed;
    
    //Same as speed, but for the Z coordinate.
    float speedZ = 0.0f;
    
    //Due to framerate imperfections, thrown Pikmin/leaders can reach higher
    //than intended. z_cap forces a cap. FLT_MAX = no cap.
    float zCap = FLT_MAX;
    
    //Multiply the mob's gravity by this.
    float gravityMult = 1.0f;
    
    //How much it's being pushed by another mob.
    float pushAmount = 0.0f;
    
    //Angle that another mob is pushing it to.
    float pushAngle = 0.0f;
    
    //How much the mob moved this frame, if it's walkable.
    Point walkableMoved;
    
    //Highest value of the Z coordinate since the last time it was grounded.
    //FLT_MAX = not midair.
    float highestMidairZ = 0.0f;
    
    //-Complex states-
    
    //Information about what it is chasing after.
    ChaseInfo chaseInfo;
    
    //Information about the path it is following, if any.
    Path* pathInfo = nullptr;
    
    //Information about the mob/point it's circling, if any.
    CirclingInfo* circlingInfo = nullptr;
    
    //Riding a track. If nullptr, the mob is not riding on any track.
    TrackRideInfo* trackInfo = nullptr;
    
    //Info on how this mob should be carried. Uncarriable if nullptr.
    CarryInfo* carryInfo = nullptr;
    
    //Onion delivery info. If nullptr, the mob is not being delivered.
    DeliveryInfo* deliveryInfo = nullptr;
    
    //-Physical space-
    
    //Current radius.
    float radius = 0.0f;
    
    //Current height.
    float height = 0.0f;
    
    //Current rectangular dimensions.
    Point rectangularDim;
    
    //-Scripting-
    
    //Finite-state machine.
    MobFsm fsm;
    
    //The script-controlled timer.
    Timer scriptTimer;
    
    //Variables.
    map<string, string> vars;
    
    //-Brain and behavior-
    
    //The mob it has focus on.
    Mob* focusedMob = nullptr;
    
    //Further memory of focused mobs.
    map<size_t, Mob*> focusedMobMemory;
    
    //Angle the mob wants to be facing.
    float intendedTurnAngle;
    
    //Variable that holds the position the mob wants to be facing.
    Point* intendedTurnPos = nullptr;
    
    //Starting coordinates; what the mob calls "home".
    Point home;
    
    //Index of the reach to use for "X in reach" events.
    size_t farReach = INVALID;
    
    //Index or the reach to use for "focused mob out of reach" events.
    size_t nearReach = INVALID;
    
    //How long it's been alive for.
    float timeAlive = 0.0f;
    
    //Incremental ID. Used for misc. things.
    size_t id = 0;
    
    //-General state-
    
    //Current health.
    float health = 0.0f;
    
    //Maximum health.
    float maxHealth = 0.0f;
    
    //During this period, the mob cannot be attacked.
    Timer invulnPeriod;
    
    //Mobs that it just hit. Used to stop hitboxes from hitting every frame.
    vector<std::pair<float, Mob*> > hitOpponents;
    
    //How much damage did it take since the last time the itch event triggered?
    float itchDamage = 0.0f;
    
    //How much time has passed the last time the itch event triggered?
    float itchTime = 0.0f;
    
    //Status effects currently inflicted on the mob.
    vector<Status> statuses;
    
    //Hazard of the sector the mob is currently on.
    Hazard* onHazard = nullptr;
    
    //If this mob is a sub-mob, this points to the parent mob.
    Parent* parent = nullptr;
    
    //Miscellanous flags. Use MOB_FLAG_*.
    bitmask_16_t flags = 0;
    
    //-Interactions with other mobs-
    
    //Mobs it is linked to.
    vector<Mob*> links;
    
    //If it's being held by another mob, the information is kept here.
    HoldInfo holder;
    
    //List of mobs it is holding.
    vector<Mob*> holding;
    
    //If it's stored inside another mob, this indicates which mob it is.
    Mob* storedInsideAnother = nullptr;
    
    //List of body parts that will chomp Pikmin.
    vector<int> chompBodyParts;
    
    //List of mobs currently in its mouth, i.e., chomped.
    vector<Mob*> chompingMobs;
    
    //Max number of mobs it can chomp in the current attack.
    size_t chompMax = 0;
    
    //Mob's team (who it can damage).
    MOB_TEAM team = MOB_TEAM_NONE;
    
    //-Group-
    
    //The current mob is following this mob's group.
    Mob* followingGroup = nullptr;
    
    //Index of this mob's spot in the leader's group spots.
    size_t groupSpotIdx = INVALID;
    
    //The current subgroup type.
    SubgroupType* subgroupTypePtr = nullptr;
    
    //Info on the group this mob is a leader of, if any.
    Group* group = nullptr;
    
    //-Animation-
    
    //Current animation instance.
    AnimationInstance anim;
    
    //Force the usage of this specific sprite.
    Sprite* forcedSprite = nullptr;
    
    //If not 0, speed up or slow down the current animation based on the
    //mob's speed, using this value as a baseline (1.0x speed).
    float mobSpeedAnimBaseline = 0.0f;
    
    //-Aesthetic-
    
    //If not LARGE_FLOAT, compare the Z with this to shrink/grow the sprite.
    float heightEffectPivot = LARGE_FLOAT;
    
    //Time left in the current damage squash and stretch animation.
    float damageSquashTime = 0.0f;
    
    //Particle generators attached to it.
    vector<ParticleGenerator> particleGenerators;
    
    //Data about its on-screen health wheel, if any.
    InWorldHealthWheel* healthWheel = nullptr;
    
    //Data about its on-screen fraction numbers, if any.
    InWorldFraction* fraction = nullptr;
    
    //-Caches-
    
    //Cached value of the angle's cosine.
    float angleCos = 0.0f;
    
    //Cached value of the angle's sine.
    float angleSin = 0.0f;
    
    //How far its radius or hitboxes reach from the center.
    //Cache for performance.
    float physicalSpan = 0.0f;
    
    //How far it can interact with another mob, from the center.
    //This includes the physical span and the span of the reaches.
    //Cache for performance.
    float interactionSpan;
    
    //It's invisible due to a status effect. Cache for performance.
    bool hasInvisibilityStatus = false;
    
    //Whether it's active this frame. Cache for performance.
    bool isActive = false;
    
    
    //--- Function declarations ---
    
    Mob(const Point &pos, MobType* type, float angle);
    virtual ~Mob();
    
    void tick(float delta_t);
    void drawLimb();
    virtual void drawMob();
    
    void setAnimation(
        size_t idx,
        const START_ANIM_OPTION options = START_ANIM_OPTION_NORMAL,
        bool pre_named = true,
        float mob_speed_baseline = 0.0f
    );
    void setAnimation(
        const string &name,
        const START_ANIM_OPTION options = START_ANIM_OPTION_NORMAL,
        float mob_speed_baseline = 0.0f
    );
    void setHealth(bool add, bool ratio, float amount);
    void setTimer(float time);
    void setVar(const string &name, const string &value);
    void setRadius(float radius);
    void setRectangularDim(const Point &rectangular_dim);
    void setCanBlockPaths(bool blocks);
    
    void becomeCarriable(const CARRY_DESTINATION destination);
    void becomeUncarriable();
    
    void applyAttackDamage(
        Mob* attacker, Hitbox* attack_h, Hitbox* victim_h, float damage
    );
    void addToGroup(Mob* new_member);
    void applyKnockback(float knockback, float knockback_angle);
    bool calculateCarryingDestination(
        Mob* added, Mob* removed,
        PikminType** target_type, Mob** target_mob, Point* target_point
    ) const;
    Onion* calculateCarryingOnion(
        Mob* added, Mob* removed, PikminType** target_type
    ) const;
    Ship* calculateCarryingShip() const;
    bool calculateDamage(
        Mob* victim, Hitbox* attack_h, const Hitbox* victim_h, float* damage
    ) const;
    void calculateKnockback(
        const Mob* victim, const Hitbox* attack_h,
        Hitbox* victim_h, float* knockback, float* angle
    ) const;
    void causeSpikeDamage(Mob* victim, bool is_ingestion);
    void chomp(Mob* m, const Hitbox* hitbox_info);
    void getSpriteData(
        Sprite** out_cur_sprite_ptr, Sprite** out_next_sprite_ptr,
        float* out_interpolation_factor
    ) const;
    void getHitboxHoldPoint(
        const Mob* mob_to_hold, const Hitbox* h_ptr,
        float* offset_dist, float* offset_angle, float* vertical_dist
    ) const;
    size_t getLatchedPikminAmount() const;
    float getLatchedPikminWeight() const;
    void doAttackEffects(
        const Mob* attacker, const Hitbox* attack_h, const Hitbox* victim_h,
        float damage, float knockback
    );
    bool isStoredInsideMob() const;
    bool isOffCamera() const;
    bool isPointOn(const Point &p) const;
    void focusOnMob(Mob* m);
    void unfocusFromMob();
    void leaveGroup();
    void hold(
        Mob* m, size_t hitbox_idx,
        float offset_dist, float offset_angle,
        float vertical_dist,
        bool force_above_holder, const HOLD_ROTATION_METHOD rotation_method
    );
    void release(Mob* m);
    bool canHurt(Mob* m) const;
    bool canHunt(Mob* m) const;
    MobType::Vulnerability getHazardVulnerability(
        Hazard* h_ptr
    ) const;
    bool isResistantToHazards(const vector<Hazard*> &hazards) const;
    size_t playSound(size_t sound_data_idx);
    void swallowChompedPikmin(size_t amount);
    void swallowChompedPikmin(Mob* m_ptr);
    float getDrawingHeight() const;
    void startHeightEffect();
    void stopHeightEffect();
    void storeMobInside(Mob* m);
    void releaseChompedPikmin();
    void releaseStoredMobs();
    void sendScriptMessage(Mob* receiver, string &msg) const;
    Mob* spawn(const MobType::SpawnInfo* info, MobType* type_ptr = nullptr);
    void startDying();
    void finishDying();
    void respawn();
    Distance getDistanceBetween(
        const Mob* m2_ptr, const Distance* regular_distance_cache = nullptr
    ) const;
    Hitbox* getHitbox(size_t idx) const;
    Hitbox* getClosestHitbox(
        const Point &p, size_t h_type = INVALID, Distance* d = nullptr
    ) const;
    bool hasClearLine(const Mob* target_mob) const;
    
    void chase(
        Point* orig_coords, float* orig_z,
        const Point &offset = Point(), float offset_z = 0.0f,
        unsigned char flags = 0,
        float target_distance = PATHS::DEF_CHASE_TARGET_DISTANCE,
        float speed = LARGE_FLOAT, float acceleration = LARGE_FLOAT
    );
    void chase(
        const Point &coords, float coords_z,
        bitmask_8_t flags = 0,
        float target_distance = PATHS::DEF_CHASE_TARGET_DISTANCE,
        float speed = LARGE_FLOAT, float acceleration = LARGE_FLOAT
    );
    void stopChasing();
    void stopTurning();
    bool followPath(
        const PathFollowSettings &settings,
        float speed, float acceleration
    );
    void stopFollowingPath();
    void circleAround(
        Mob* m, const Point &p, float radius, bool clockwise,
        float speed, bool can_free_move
    );
    void stopCircling();
    void face(
        float new_angle, Point* new_pos, bool instantly = false
    );
    Point getChaseTarget(float* out_z = nullptr) const;
    virtual float getBaseSpeed() const;
    float getSpeedMultiplier() const;
    
    void arachnorbHeadTurnLogic();
    void arachnorbPlanLogic(MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE goal);
    void arachnorbFootMoveLogic();
    
    void applyStatusEffect(
        StatusType* s, bool given_by_parent, bool from_hazard
    );
    void deleteOldStatusEffects();
    void removeParticleGenerator(const MOB_PARTICLE_GENERATOR_ID id);
    ALLEGRO_BITMAP* getStatusBitmap(float* bmp_scale) const;
    virtual bool canReceiveStatus(StatusType* s) const;
    virtual void getGroupSpotInfo(
        Point* out_spot, float* out_dist
    ) const;
    virtual bool getFractionNumbersInfo(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const;
    virtual void handleStatusEffectGain(StatusType* sta_type);
    virtual void handleStatusEffectLoss(StatusType* sta_type);
    virtual void readScriptVars(const ScriptVarReader &svr);
    virtual void startDyingClassSpecifics();
    virtual void finishDyingClassSpecifics();
    bool tickTrackRide();
    void stopTrackRide();
    void updateInteractionSpan();
    
    //Drawing tools.
    void getSpriteBitmapEffects(
        Sprite* s_ptr, Sprite* next_s_ptr, float interpolation_factor,
        BitmapEffect* info, bitmask_16_t effects
    ) const;
    
    string printStateHistory() const;
    
protected:

    //--- Members ---
    
    //Is it currently capable of blocking paths?
    bool canBlockPaths = false;
    
    
    //--- Function declarations ---
    
    PikminType* decideCarryPikminType(
        const unordered_set<PikminType*> &available_types,
        Mob* added, Mob* removed
    ) const;
    Mob* getMobToWalkOn() const;
    H_MOVE_RESULT getMovementEdgeIntersections(
        const Point &new_pos, vector<Edge*>* intersecting_edges
    ) const;
    H_MOVE_RESULT getPhysicsHorizontalMovement(
        float delta_t, float move_speed_mult, Point* move_speed
    );
    H_MOVE_RESULT getWallSlideAngle(
        const Edge* e_ptr, unsigned char wall_sector, float move_angle,
        float* slide_angle
    ) const;
    void moveToPathEnd(float speed, float acceleration);
    void tickAnimation(float delta_t);
    void tickBrain(float delta_t);
    void tickHorizontalMovementPhysics(
        float delta_t, const Point &attempted_move_speed,
        bool* touched_wall
    );
    void tickMiscLogic(float delta_t);
    void tickPhysics(float delta_t);
    void tickRotationPhysics(
        float delta_t, float move_speed_mult
    );
    void tickScript(float delta_t);
    void tickVerticalMovementPhysics(
        float delta_t, float pre_move_ground_z,
        bool was_teleport = false
    );
    void tickWalkableRidingPhysics(float delta_t);
    virtual void tickClassSpecifics(float delta_t);
    
};


/**
 * @brief See MobTypeWithAnimGroups.
 */
class MobWithAnimGroups {

public:

    //--- Members ---
    
    //Index of its current base animation.
    size_t curBaseAnimIdx = INVALID;
    
    
    //--- Function declarations ---
    
    size_t getAnimationIdxFromBaseAndGroup(
        size_t base_anim_idx, size_t group_idx,
        size_t base_anim_total
    ) const;
    
};
