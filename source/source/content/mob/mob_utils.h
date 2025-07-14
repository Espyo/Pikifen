/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob utility classes and functions.
 */

#pragma once

#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../core/pathing.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../animation/animation.h"
#include "../area/sector.h"
#include "../mob_type/bouncer_type.h"
#include "../mob_type/bridge_type.h"
#include "../mob_type/converter_type.h"
#include "../mob_type/decoration_type.h"
#include "../mob_type/drop_type.h"
#include "../mob_type/enemy_type.h"
#include "../mob_type/group_task_type.h"
#include "../mob_type/interactable_type.h"
#include "../mob_type/leader_type.h"
#include "../mob_type/pellet_type.h"
#include "../mob_type/pikmin_type.h"
#include "../mob_type/pile_type.h"
#include "../mob_type/resource_type.h"
#include "../mob_type/scale_type.h"
#include "../mob_type/tool_type.h"
#include "../mob_type/track_type.h"
#include "../mob_type/treasure_type.h"
#include "mob_enums.h"


using std::size_t;
using std::vector;


class Mob;
struct MobGen;

/**
 * @brief Info on a carrying spot around a mob's perimeter.
 */
struct CarrierSpot {

    //--- Members ---
    
    //State.
    CARRY_SPOT_STATE state = CARRY_SPOT_STATE_FREE;
    
    //Relative coordinates of each spot. Cache for performance.
    Point pos;
    
    //Pikmin that is in this spot.
    Mob* pikPtr = nullptr;
    
    
    //--- Function declarations ---
    
    explicit CarrierSpot(const Point& pos);
    
};


/**
 * @brief Info on how the mob should be carried.
 */
struct CarryInfo {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Generic type of delivery destination.
    CARRY_DESTINATION destination = CARRY_DESTINATION_SHIP;
    
    //Information about each carrier spot.
    vector<CarrierSpot> spotInfo;
    
    //Current carrying strength. Cache for performance.
    float curCarryingStrength = 0.0f;
    
    //Number of carriers, including reserves. Cache for performance.
    size_t curNCarriers = 0;
    
    //Is the object moving at the moment?
    bool isMoving = false;
    
    //When the object begins moving, the idea is to carry it to this mob.
    Mob* intendedMob = nullptr;
    
    //When the object begins moving, the idea is to carry it to this point.
    Point intendedPoint;
    
    //When delivering to an Onion, this is the Pikmin type that will benefit.
    PikminType* intendedPikType = nullptr;
    
    //True if a destination does exist, false otherwise.
    bool destinationExists = false;
    
    //Is the Pikmin meant to return somewhere after carrying?
    bool mustReturn = false;
    
    //Location to return to once they finish carrying.
    Point returnPoint;
    
    //Distance from the return point to stop at.
    float returnDist = 0.0f;
    
    
    //--- Function declarations ---
    
    CarryInfo(Mob* m, const CARRY_DESTINATION destination);
    bool isEmpty() const;
    bool isFull() const;
    vector<Hazard*> getCarrierInvulnerabilities() const;
    bool canFly() const;
    size_t getPlayerTeamIdx() const;
    float getSpeed() const;
    void rotatePoints(float angle);
    
};


/**
 * @brief Info on what point the mob is chasing after.
 */
struct ChaseInfo {

    //--- Members ---
    
    //Current chasing state.
    CHASE_STATE state = CHASE_STATE_STOPPED;
    
    //Flags that control how to chase. Use CHASE_FLAG.
    Bitmask8 flags = 0;
    
    //Chase after these coordinates, relative to the "origin" coordinates.
    Point offset;
    
    //Same as above, but for the Z coordinate.
    float offsetZ = 0.0f;
    
    //Pointer to the origin of the coordinates, or nullptr for the world origin.
    Point* origCoords = nullptr;
    
    //Same as above, but for the Z coordinate.
    float* origZ = nullptr;
    
    //Distance from the target in which the mob is considered as being there.
    float targetDist = 0.0f;
    
    //Acceleration to apply, in units per second per second.
    float acceleration = 0.0f;
    
    //Current speed to move towards the target at.
    float curSpeed = 0.0f;
    
    //Maximum speed.
    float maxSpeed = -1.0f;
    
};


/**
 * @brief Info about what mob or point that this mob is circling around,
 * if any.
 */
struct CirclingInfo {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Mob that it is circling.
    Mob* circlingMob = nullptr;
    
    //Point that it is circling, if it's not circling a mob.
    Point circlingPoint;
    
    //Radius at which to circle around.
    float radius = 0.0f;
    
    //Is it circling clockwise?
    bool clockwise = true;
    
    //Speed at which to move.
    float speed = 0.0f;
    
    //Can the mob move freely, or only forward?
    bool canFreeMove = false;
    
    //Angle of the circle to go to.
    float curAngle = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit CirclingInfo(Mob* m);
    
};


/**
 * @brief Info on a mob that's being delivered to an Onion, ship, etc.
 */
struct DeliveryInfo {

    //--- Members ---
    
    //Animation type.
    DELIVERY_ANIM animType = DELIVERY_ANIM_SUCK;
    
    //Ratio of time left in the animation.
    float animTimeRatioLeft = 1.0f;
    
    //Color to make the mob glow with.
    ALLEGRO_COLOR color;
    
    //Intended delivery Pikmin type, in the case of Onions.
    PikminType* intendedPikType = nullptr;
    
    //Index of the player team in charge, or INVALID if none.
    size_t playerTeamIdx = INVALID;
    
    //--- Function declarations ---
    
    DeliveryInfo();
    
};


/**
 * @brief Info on a mob's group.
 *
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct Group {

    //--- Misc. declarations ---
    
    //Ways for Pikmin to follow the leader.
    enum MODE {
    
        //Follow the leader's back.
        MODE_FOLLOW_BACK,
        
        //Casually shuffle with the leader, if needed.
        MODE_SHUFFLE,
        
        //Swarming.
        MODE_SWARM,
        
    };
    
    /**
     * @brief A spot in the group.
     */
    struct GroupSpot {
    
        //--- Members ---
        
        //Position relative to the anchor.
        Point pos;
        
        //Mob in this spot.
        Mob* mobPtr = nullptr;
        
        
        //--- Function declarations ---
        
        explicit GroupSpot(const Point& p = Point(), Mob* m = nullptr) :
            pos(p), mobPtr(m) {}
            
    };
    
    
    //--- Members ---
    
    //All group members.
    vector<Mob*> members;
    
    //Information about each spot.
    vector<GroupSpot> spots;
    
    //Radius of the group.
    float radius = 0.0f;
    
    //Absolute position of element 0 of the group (frontmost member).
    Point anchor;
    
    //Angle from the leader to the anchor.
    float anchorAngle = TAU / 2.0f;
    
    //Transformation to apply to the group, like from swarming.
    ALLEGRO_TRANSFORM transform;
    
    //Currently selected standby type.
    SubgroupType* curStandbyType = nullptr;
    
    //Mode of operation.
    MODE mode = MODE_SHUFFLE;
    
    
    //--- Function declarations ---
    
    explicit Group(Mob* leaderptr);
    void initSpots(Mob* affectedMobPtr = nullptr);
    void sort(SubgroupType* leadingType);
    void changeStandbyTypeIfNeeded();
    size_t getAmountByType(const MobType* type) const;
    Point getAverageMemberPos() const;
    vector<Hazard*> getGroupInvulnerabilities(
        Mob* includeLeader = nullptr
    ) const;
    bool getNextStandbyType(
        bool moveBackwards, SubgroupType** newType
    );
    Point getSpotOffset(size_t spotIdx) const;
    void reassignSpots();
    bool changeStandbyType(bool moveBackwards);
};


/**
 * @brief Info about how this mob is currently being held by
 * another, if it is.
 */
struct HoldInfo {

    //--- Members ---
    
    //Points to the mob holding the current one, if any.
    Mob* m = nullptr;
    
    //Index of the hitbox the mob is attached to.
    //If INVALID, it's attached to the mob center.
    size_t hitboxIdx = INVALID;
    
    //Ratio of distance from the hitbox/body center. 1 is the full radius.
    float offsetDist = 0.0f;
    
    //Angle the mob makes with the center of the hitbox/body.
    float offsetAngle = 0.0f;
    
    //Ratio of distance from the hitbox/body's bottom. 1 is the very top.
    float verticalDist = 0.0f;
    
    //If true, force the mob to be drawn above the holder?
    bool forceAboveHolder = false;
    
    //How should the held object rotate?
    HOLD_ROTATION_METHOD rotationMethod = HOLD_ROTATION_METHOD_NEVER;
    
    
    //--- Function declarations ---
    
    void clear();
    Point getFinalPos(float* outz) const;
    
};


class Bouncer;
class Bridge;
class Converter;
class Decoration;
class Drop;
class Enemy;
class GroupTask;
class Interactable;
class Leader;
class Onion;
class Pellet;
class Pikmin;
class Pile;
class Resource;
class Scale;
class Ship;
class Tool;
class Track;
class Treasure;

class OnionType;
class ShipType;

/**
 * @brief Lists of all mobs in the area.
 */
struct MobLists {

    //--- Members ---
    
    //All mobs in the area.
    vector<Mob*> all;
    
    //Bouncers.
    vector<Bouncer*> bouncers;
    
    //Bridges.
    vector<Bridge*> bridges;
    
    //Converters.
    vector<Converter*> converters;
    
    //Decorations.
    vector<Decoration*> decorations;
    
    //Drops.
    vector<Drop*> drops;
    
    //Enemies.
    vector<Enemy*> enemies;
    
    //Group tasks.
    vector<GroupTask*> groupTasks;
    
    //Interactables.
    vector<Interactable*> interactables;
    
    //Leaders.
    vector<Leader*> leaders;
    
    //Onions.
    vector<Onion*> onions;
    
    //Pellets.
    vector<Pellet*> pellets;
    
    //Pikmin.
    vector<Pikmin*> pikmin;
    
    //Piles.
    vector<Pile*> piles;
    
    //Resources.
    vector<Resource*> resources;
    
    //Mobs that can be walked on top of. Cache for performance.
    vector<Mob*> walkables;
    
    //Scales.
    vector<Scale*> scales;
    
    //Ships.
    vector<Ship*> ships;
    
    //Tools.
    vector<Tool*> tools;
    
    //Tracks.
    vector<Track*> tracks;
    
    //Treasures.
    vector<Treasure*> treasures;
    
};


/**
 * @brief Lists of all mob types.
 */
struct MobTypeLists {

    //--- Members ---
    
    //Bouncer types.
    map<string, BouncerType*> bouncer;
    
    //Bridge types.
    map<string, BridgeType*> bridge;
    
    //Converter types.
    map<string, ConverterType*> converter;
    
    //Custom mob types.
    map<string, MobType*> custom;
    
    //Decoration types.
    map<string, DecorationType*> decoration;
    
    //Drop types.
    map<string, DropType*> drop;
    
    //Enemy types.
    map<string, EnemyType*> enemy;
    
    //Group task types.
    map<string, GroupTaskType*> groupTask;
    
    //Interactable types.
    map<string, InteractableType*> interactable;
    
    //Leader types.
    map<string, LeaderType*> leader;
    
    //Onion types.
    map<string, OnionType*> onion;
    
    //Pellet types.
    map<string, PelletType*> pellet;
    
    //Pikmin types.
    map<string, PikminType*> pikmin;
    
    //Pile types.
    map<string, PileType*> pile;
    
    //Resource types.
    map<string, ResourceType*> resource;
    
    //Scale types.
    map<string, ScaleType*> scale;
    
    //Ship types.
    map<string, ShipType*> ship;
    
    //Tool types.
    map<string, ToolType*> tool;
    
    //Track types.
    map<string, TrackType*> track;
    
    //Treasure types.
    map<string, TreasureType*> treasure;
    
};


/**
 * @brief Info about this mob's parent, if any.
 */
struct Parent {

    //--- Members ---
    
    //Mob serving as the parent.
    Mob* m = nullptr;
    
    //Should the child handle damage?
    bool handleDamage = false;
    
    //Should the child relay damage to the parent?
    bool relayDamage = false;
    
    //Should the child handle status effects?
    bool handleStatuses = false;
    
    //Should the child relay status effects to the parent?
    bool relayStatuses = false;
    
    //Should the child handle script events?
    bool handleEvents = false;
    
    //Should the child relay script events to the parent?
    bool relayEvents = false;
    
    //Animation used for the limb connecting child and parent.
    AnimationInstance limbAnim;
    
    //Thickness of the limb.
    float limbThickness = 32.0f;
    
    //Body part of the parent to link the limb to.
    size_t limbParentBodyPart = INVALID;
    
    //Offset from the parent body part to link the limb at.
    float limbParentOffset = 0.0f;
    
    //Body part of the child to link the limb to.
    size_t limbChildBodyPart = INVALID;
    
    //Offset from the child body part to link the limb at.
    float limbChildOffset = 0.0f;
    
    //Method by which the limb should be drawn.
    LIMB_DRAW_METHOD limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_CHILD;
    
    
    //--- Function declarations ---
    
    explicit Parent(Mob* m);
    
};


/**
 * @brief Info on how to travel through the path graph that
 * the mob currently intends to travel.
 */
struct Path {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Path to take the mob to while being carried.
    vector<PathStop*> path;
    
    //Index of the current stop in the projected carrying path.
    size_t curPathStopIdx = 0;
    
    //Result of the path calculation.
    PATH_RESULT result = PATH_RESULT_NOT_CALCULATED;
    
    //Is the way forward currently blocked? If so, why?
    PATH_BLOCK_REASON blockReason = PATH_BLOCK_REASON_NONE;
    
    //Settings about how the path should be followed.
    PathFollowSettings settings;
    
    
    //--- Function declarations ---
    
    Path(
        Mob* m,
        const PathFollowSettings& settings
    );
    bool checkBlockage(PATH_BLOCK_REASON* outReason = nullptr);
    
};


/**
 * @brief Info that a mob type may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct PikminNestType {

    //--- Members ---
    
    //Pikmin types it can manage.
    vector<PikminType*> pikTypes;
    
    //Body parts that represent legs -- pairs of hole + foot.
    vector<string> legBodyParts;
    
    //Speed at which Pikmin enter the nest.
    float pikminEnterSpeed = 0.7f;
    
    //Speed at which Pikmin exit the nest.
    float pikminExitSpeed = 2.0f;
    
    //Sound data index for the Pikmin entry sound. Cache for performance.
    size_t soundPikminEntryIdx = INVALID;
    
    //Sound data index for the Pikmin exit sound. Cache for performance.
    size_t soundPikminExitIdx = INVALID;
    
    
    //--- Function declarations ---
    
    void loadProperties(DataNode* file, MobType* mobType);
    
};


/**
 * @brief Info that a mob may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct PikminNest {

    public:
    
    //--- Members ---
    
    //Pointer to the nest mob responsible.
    Mob* mPtr = nullptr;
    
    //Pointer to the type of nest.
    PikminNestType* nestType = nullptr;
    
    //How many Pikmin are inside, per type, per maturity.
    vector<vector<size_t> > pikminInside;
    
    //How many Pikmin are queued up to be called out, of each type.
    vector<size_t> callQueue;
    
    //Which leader is calling the Pikmin over?
    Leader* callingLeader = nullptr;
    
    //Time left until it can eject the next Pikmin in the call queue.
    float nextCallTime = 0.0f;
    
    
    //--- Function declarations ---
    
    PikminNest(Mob* mPtr, PikminNestType* type);
    bool callPikmin(Mob* mPtr, size_t typeIdx);
    size_t getAmountByType(const PikminType* type);
    void readScriptVars(const ScriptVarReader& svr);
    void requestPikmin(
        size_t typeIdx, size_t amount, Leader* lPtr
    );
    void storePikmin(Pikmin* pPtr);
    void tick(float deltaT);
    
};


/**
 * @brief Info about the track mob that a mob is currently
 * riding. Includes things like current progress.
 */
struct TrackRideInfo {

    //--- Members ---
    
    //Pointer to the track mob.
    Mob* m = nullptr;
    
    //List of checkpoints (body part indexes) to cross.
    vector<size_t> checkpoints;
    
    //Current checkpoint of the track. This is the last checkpoint crossed.
    size_t curCpIdx = 0;
    
    //Progress within the current checkpoint. 0 means at the checkpoint.
    //1 means it's at the next checkpoint.
    float curCpProgress = 0.0f;
    
    //Speed to ride at, in ratio per second.
    float rideSpeed = 0.0f;
    
    
    //--- Function declarations ---
    
    TrackRideInfo(
        Mob* m, const vector<size_t>& checkpoints, float speed
    );
    
};


float calculateMobPhysicalSpan(
    float radius, float animHitboxSpan,
    const Point& rectangularDim
);
Mob* createMob(
    MobCategory* category, const Point& pos, MobType* type,
    float angle, const string& vars,
    std::function<void(Mob*)> codeAfterCreation = nullptr,
    size_t firstStateOverride = INVALID
);
Mob* createMob(
    MobGen* gen
);

void deleteMob(Mob* m, bool completeDestruction = false);
string getErrorMessageMobInfo(Mob* m);
vector<Hazard*> getMobTypeListInvulnerabilities(
    const unordered_set<MobType*>& types
);
MobType::SpawnInfo* getSpawnInfoFromChildInfo(
    MobType* type, const MobType::Child* childInfo
);
bool isMobInReach(
    MobType::Reach* reachTPtr, const Distance& distBetween, float angleDiff
);
MOB_TARGET_FLAG stringToMobTargetType(const string& typeStr);
MOB_TEAM stringToTeamNr(const string& teamStr);
