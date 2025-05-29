/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob type class and mob type-related functions.
 */

#pragma once

#include <functional>
#include <map>

#include <allegro5/allegro.h>

#include "../../core/audio.h"
#include "../../core/const.h"
#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/general_utils.h"
#include "../animation/animation.h"
#include "../content.h"
#include "../mob_category/mob_category.h"
#include "../mob/mob_enums.h"
#include "../other/mob_script.h"
#include "../other/spike_damage.h"
#include "../other/status.h"


using std::size_t;
using std::string;
using std::vector;


typedef vector<std::pair<size_t, string> > AnimConversionVector;

namespace MOB_TYPE {
extern const size_t ANIM_IDLING;
extern const float DEF_ACCELERATION;
extern const float DEF_ROTATION_SPEED;
}


/**
 * @brief A mob type.
 *
 * There are specific types, like Pikmin,
 * leader, etc., but these are used
 * to create more generic mob types,
 * like some teleporter pad, or a door.
 */
class MobType : public Content {

public:

    //--- Misc. declarations ---
    
    /**
     * @brief Info about a mob's reach.
     */
    struct Reach {
    
        //--- Members ---
        
        //Name of this reach.
        string name;
        
        //Radius of possibility 1.
        float radius1 = -1.0f;
        
        //Angle of possibility 1.
        float angle1 = -1.0f;
        
        //Radius of possibility 2.
        float radius2 = -1.0f;
        
        //Angle of possibility 2.
        float angle2 = -1.0f;
        
    };
    
    /**
     * @brief Info about how a mob spawns another.
     */
    struct SpawnInfo {
    
        //--- Members ---
        
        //Name of this spawn information block.
        string name;
        
        //Name of the mob type to spawn.
        string mobTypeName;
        
        //Spawn in coordinates relative to the spawner?
        bool relative = true;
        
        //Coordenates to spawn on.
        Point coordsXY;
        
        //Z coordinate to spawn on.
        float coordsZ = 0.0f;
        
        //Angle of the spawned object. Could be relative or absolute.
        float angle = 0.0f;
        
        //Script vars to give the spawned object.
        string vars;
        
        //Should the spawner link to the spawned?
        bool linkObjectToSpawn = false;
        
        //Should the spawned link to the spawner?
        bool linkSpawnToObject = false;
        
        //Momentum to apply in a random direction upon spawn, if any.
        float momentum = 0.0f;
        
    };
    
    /**
     * @brief Info about how a mob can be a child of another.
     */
    struct Child {
    
        //--- Members ---
        
        //Name of this child information block.
        string name;
        
        //Name of the spawn information block to use.
        string spawnName;
        
        //Does the parent mob hold the child mob?
        bool parentHolds = false;
        
        //If the parent holds, this is the name of the body part that holds.
        string holdBodyPart;
        
        //If the parent holds, this is how far from the body part center.
        float holdOffsetDist = 0.0f;
        
        //If the parent holds, this is how far from the body part Z.
        float holdOffsetVertDist = 0.0f;
        
        //If the parent holds, this is in what direction from the body part.
        float holdOffsetAngle = 0.0f;
        
        //Method by which the parent should hold the child.
        HOLD_ROTATION_METHOD holdRotationMethod = HOLD_ROTATION_METHOD_NEVER;
        
        //Should the child handle damage?
        bool handleDamage = false;
        
        //Should the child relay damage to the parent?
        bool relayDamage = false;
        
        //Should the child handle script events?
        bool handleEvents = false;
        
        //Should the child relay script events to the parent?
        bool relayEvents = false;
        
        //Should the child handle status effects?
        bool handleStatuses = false;
        
        //Should the child relay status effects to the parent?
        bool relayStatuses = false;
        
        //Name of the limb animation between parent and child.
        string limbAnimName;
        
        //Thickness of the limb.
        float limbThickness = 32.0f;
        
        //Body part of the parent to link the limb to.
        string limbParentBodyPart;
        
        //Offset from the parent body part to link the limb at.
        float limbParentOffset = 0.0f;
        
        //Body part of the child to link the limb to.
        string limbChildBodyPart;
        
        //Offset from the child body part to link the limb at.
        float limbChildOffset = 0.0f;
        
        //Method by which the limb should be drawn.
        LIMB_DRAW_METHOD limbDrawMethod = LIMB_DRAW_METHOD_ABOVE_BOTH;
        
    };
    
    /**
     * @brief Info on a widget to present in the area editor,
     * to better help users set the properties of a mob instance.
     */
    struct AreaEditorProp {
    
        //--- Members ---
        
        //Name of the widget.
        string name;
        
        //Variable it sets.
        string var;
        
        //What type of content this var has.
        AEMP_TYPE type = AEMP_TYPE_TEXT;
        
        //Default value.
        string defValue;
        
        //Minimum value.
        float minValue = -LARGE_FLOAT;
        
        //Maximum value.
        float maxValue = LARGE_FLOAT;
        
        //If it's a list, this list the values.
        vector<string> valueList;
        
        //Tooltip to show on the widget, if any.
        string tooltip;
        
    };
    
    /**
     * @brief Info on how vulnerable the object is to a certain source.
     */
    struct Vulnerability {
    
        //--- Members ---
        
        //Multiply the effects (damage taken, speed reduction, etc.) by this.
        float effectMult = 1.0f;
        
        //When affected by the source, receive this status effect.
        StatusType* statusToApply = nullptr;
        
        //If "statusToApply" overrides any status effect that'd be received.
        bool statusOverrides = true;
        
    };
    
    /**
     * @brief Info on a sound effect this mob can emit.
     */
    struct Sound {
    
        //--- Members ---
        
        //Its name.
        string name;
        
        //The loaded sample.
        ALLEGRO_SAMPLE* sample = nullptr;
        
        //Type of sound.
        SOUND_TYPE type = SOUND_TYPE_GAMEPLAY_POS;
        
        //Configuration.
        SoundSourceConfig config;
        
    };
    
    
    //--- Members ---
    
    //- Basic information -
    
    //Mob category.
    MobCategory* category = nullptr;
    
    //Custom category name. Used in editors.
    string customCategoryName;
    
    //- Visuals -
    
    //Database with all its animation data.
    AnimationDatabase* animDb = nullptr;
    
    //A color that represents this mob.
    ALLEGRO_COLOR mainColor = al_map_rgb(128, 128, 128);
    
    //Show its health?
    bool showHealth = true;
    
    //Does it cast a shadow?
    bool castsShadow = true;
    
    //How much light does it cast in a blackout? <0 to use the mob's radius.
    float blackoutRadius = -1.0f;
    
    //List of sounds it can play.
    vector<Sound> sounds;
    
    //- Movement -
    
    //Moves these many units per second.
    float moveSpeed = 0.0f;
    
    //Acceleration. This is in units per second per second.
    float acceleration = MOB_TYPE::DEF_ACCELERATION;
    
    //Rotates these many radians per second.
    float rotationSpeed = MOB_TYPE::DEF_ROTATION_SPEED;
    
    //True if it can move in any direction, as opposed to just forward.
    bool canFreeMove = false;
    
    //- Physical space -
    
    //Radius of the space it occupies. Can be overridden on a per-mob basis.
    float radius = 0.0f;
    
    //Height. Can be overridden on a per-mob basis.
    float height = 0.0f;
    
    //Rectangular dimensions, if it's meant to use them instead of a radius.
    Point rectangularDim;
    
    //Pikmin strength needed to carry it.
    float weight = 0.0f;
    
    //How many Pikmin can carry it, at most.
    size_t maxCarriers = 0;
    
    //Pushes other mobs (only those that can be pushed).
    bool pushes = false;
    
    //Can be pushed by other mobs.
    bool pushable = false;
    
    //If true, the push is soft and allows squeezing through with persistance.
    bool pushesSoftly = false;
    
    //If true, the push is via hitbox, as opposed to the mob's radius.
    bool pushesWithHitboxes = false;
    
    //Radius for terrain collision. Negative = use regular radius property.
    float terrainRadius = - 1.0f;
    
    //Can you walk on top of this mob?
    bool walkable = false;
    
    //Can this mob walk on top of other mobs?
    bool canWalkOnOthers = false;
    
    //If true, carrier Pikmin will be considered blocked if it's in the way.
    bool canBlockPaths = false;
    
    //Override the carrying spots with these coordinates, if not-empty.
    vector<Point> customCarrySpots;
    
    //- General behavior -
    
    //Maximum health. Can be overridden on a per-mob basis.
    float maxHealth = 100.0f;
    
    //Regenerates these many health points per second.
    float healthRegen = 0.0f;
    
    //How far its territory reaches from the home point.
    float territoryRadius = 0.0f;
    
    //Information on all of its "reaches".
    vector<Reach> reaches;
    
    //After it takes this much damage, it sends an "itch" event to the FSM.
    float itchDamage = 0.0f;
    
    //Only send an "itch" event after these many seconds have passed.
    float itchTime = 0.0f;
    
    //Other mobs decide if they can/want to hurt it by this target type.
    MOB_TARGET_FLAG targetType = MOB_TARGET_FLAG_NONE;
    
    //What types of targets this mob can hunt down.
    Bitmask16 huntableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY;
        
    //What types of targets this mob can hurt.
    Bitmask16 hurtableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_FRAGILE;
        
    //Its initial team.
    MOB_TEAM startingTeam = MOB_TEAM_NONE;
    
    //Logic for when it's inactive. Use INACTIVE_LOGIC_FLAG.
    Bitmask8 inactiveLogic = 0;
    
    //Custom behavior callbacks.
    void(*drawMobCallback)(Mob* m) = nullptr;
    
    //- Script -
    
    //Actions to run on spawn.
    vector<MobActionCall*> initActions;
    
    //The states, events and actions. Basically, the FSM.
    vector<MobState*> states;
    
    //Index of the state a mob starts at.
    size_t firstStateIdx = INVALID;
    
    //Name of the state to go to when it's dying.
    string dyingStateName;
    
    //Index of the state to go to when it's dying.
    size_t dyingStateIdx = INVALID;

    //Index of the state to go to when it's dying.
    size_t reviveStateIdx = INVALID;
    
    //States that ignore the death event.
    vector<string> statesIgnoringDeath;
    
    //States that ignore the spray event.
    vector<string> statesIgnoringSpray;
    
    //States that ignore the hazard events.
    vector<string> statesIgnoringHazard;
    
    //Interactions with other objects
    
    //Information on everything it can spawn.
    vector<SpawnInfo> spawns;
    
    //Information on its children mobs.
    vector<Child> children;
    
    //Does this mob have a group of other mobs following it (e.g. leader)?
    bool hasGroup = false;
    
    //- Vulnerabilities -
    
    //All damage received is multiplied by this much.
    float defaultVulnerability = 1.0f;
    
    //For every hazard, multiply its effects by this much.
    map<Hazard*, Vulnerability> hazardVulnerabilities;
    
    //What sort of spike damage it causes, if any.
    SpikeDamageType* spikeDamage = nullptr;
    
    //For every type of spike damage, multiply its effects by this much.
    map<SpikeDamageType*, Vulnerability> spikeDamageVulnerabilities;
    
    //For every type of status, multiply its effects by this much.
    map<StatusType*, Vulnerability> statusVulnerabilities;
    
    //- Editor info -
    
    //Tips to show in the area editor about this mob type, if any.
    string areaEditorTips;
    
    //Widgets to show on the area editor, to help parametrize each mob.
    vector<AreaEditorProp> areaEditorProps;
    
    //Can the player choose to place one of these in the area editor?
    bool appearsInAreaEditor = true;
    
    //Should it have links going out of it?
    bool areaEditorRecommendLinksFrom = false;
    
    //Should it have links going into it?
    bool areaEditorRecommendLinksTo = false;
    
    //- Caches -
    
    //How far its radius or hitboxes reach from the center.
    //Cache for performance.
    float physicalSpan = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit MobType(MOB_CATEGORY category_id);
    virtual ~MobType();
    void loadFromDataNode(
        DataNode* node, CONTENT_LOAD_LEVEL level, const string& folder
    );
    virtual void loadCatProperties(DataNode* file);
    virtual void loadCatResources(DataNode* file);
    virtual AnimConversionVector getAnimConversions() const;
    virtual void unloadResources();
    void addCarryingStates();
    
};


/**
 * @brief A mob type that has animation groups.
 *
 * These have a series of "base" animations, like idling, dying, etc.,
 * but can also have several looks for these same base animations.
 * So in practice, it can have an idling blue animation, idling yellow,
 * dying red, etc. Because this would otherwise be a nightmare to organize,
 * this base class comes with some helpful functions and members.
 * A "group" is the "look" mentioned before, so "red", "yellow", "blue", etc.
 * The mob type should load a property somewhere that lists what suffixes to
 * use for each group when loading animation names from the animation database.
 */
class MobTypeWithAnimGroups {

public:

    //--- Members ---
    
    //Suffixes used for each animation group.
    vector<string> animationGroupSuffixes;
    
    
    //--- Function declarations ---
    
    virtual ~MobTypeWithAnimGroups() = default;
    AnimConversionVector getAnimConversionsWithGroups(
        const AnimConversionVector& v, size_t baseAnimTotal
    ) const;
    
};


void createSpecialMobTypes();
