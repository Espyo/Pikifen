/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for all enums related to mobs.
 */

#ifndef MOB_ENUMS_INCLUDED
#define MOB_ENUMS_INCLUDED


//Area editor mob property types.
enum AEMP_TYPES {
    //Any user text.
    AEMP_TEXT,
    //Integer number.
    AEMP_INT,
    //Decimal number.
    AEMP_DECIMAL,
    //Boolean.
    AEMP_BOOL,
    //One of a list of strings.
    AEMP_LIST,
    //One of a list of numbers, though each has a name.
    AEMP_NUMBER_LIST,
};


enum CARRY_DESTINATIONS {
    CARRY_DESTINATION_SHIP,
    CARRY_DESTINATION_ONION,
    CARRY_DESTINATION_LINKED_MOB,
};


enum CARRY_SPOT_STATES {
    CARRY_SPOT_FREE,
    CARRY_SPOT_RESERVED,
    CARRY_SPOT_USED,
};


enum CHASE_FLAGS {
    //The mob instantly teleports to the final destination.
    CHASE_FLAG_TELEPORT = 1,
    //When teleporting, do not consider the chase finished.
    CHASE_FLAG_TELEPORTS_CONSTANTLY = 2,
    //The mob can move in any angle instead of just where it's facing.
    CHASE_FLAG_ANY_ANGLE = 4,
};


enum CHASE_STATES {
    //No chasing in progress.
    CHASE_STATE_STOPPED,
    //Currently chasing.
    CHASE_STATE_CHASING,
    //Reached the destination and no longer chasing.
    CHASE_STATE_FINISHED,
};


enum DELIVERY_ANIMATIONS {
    DELIVERY_ANIM_SUCK,
    DELIVERY_ANIM_TOSS,
};


enum ENEMY_EXTRA_STATES {
    ENEMY_EXTRA_STATE_CARRIABLE_WAITING,
    ENEMY_EXTRA_STATE_CARRIABLE_MOVING,
    ENEMY_EXTRA_STATE_CARRIABLE_STUCK,
    ENEMY_EXTRA_STATE_CARRIABLE_THROWN,
    ENEMY_EXTRA_STATE_BEING_DELIVERED,
};


enum H_MOVE_RESULTS {
    H_MOVE_OK,
    H_MOVE_TELEPORTED,
    H_MOVE_FAIL,
};


enum MOB_TARGET_TYPES {
    //Cannot be damaged or hunted down.
    MOB_TARGET_TYPE_NONE = 0x00,
    //Leaders and Pikmin. Can be damaged by enemies, mostly.
    MOB_TARGET_TYPE_PLAYER = 0x01,
    //Enemies. Can be damaged by Pikmin and leaders, mostly.
    MOB_TARGET_TYPE_ENEMY = 0x02,
    //Weaker objects that can be damaged by many things.
    MOB_TARGET_TYPE_WEAK_PLAIN_OBSTACLE = 0x04,
    //Stronger objects that can be damaged by less-than-many things.
    MOB_TARGET_TYPE_STRONG_PLAIN_OBSTACLE = 0x08,
    //Objects that only Pikmin can damage.
    MOB_TARGET_TYPE_PIKMIN_OBSTACLE = 0x10,
    //Objects that can only be taken down with explosive force.
    MOB_TARGET_TYPE_EXPLODABLE = 0x20,
    //Objects that Pikmin and explosives can damage.
    MOB_TARGET_TYPE_EXPLODABLE_PIKMIN_OBSTACLE = 0x40,
    //Objects that can get hurt by pretty much everything.
    MOB_TARGET_TYPE_FRAGILE = 0x80,
};


enum MOB_TEAMS {
    //Has no friends!
    MOB_TEAM_NONE,
    //Players -- usually leaders and Pikmin.
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    //Enemies -- useful if you want enemy in-fighting.
    MOB_TEAM_ENEMY_1,
    MOB_TEAM_ENEMY_2,
    MOB_TEAM_ENEMY_3,
    MOB_TEAM_ENEMY_4,
    //Miscellaneous obstacles.
    MOB_TEAM_OBSTACLE,
    //Whatever else.
    MOB_TEAM_OTHER,
    //Number of teams.
    N_MOB_TEAMS,
};


//Options for how to start a new animation.
enum START_ANIMATION_OPTIONS {
    //Start the new animation like normal.
    START_ANIMATION_NORMAL,
    //Start from whatever frame number the previous animation was at.
    START_ANIMATION_NO_RESTART,
    //Start on a random time.
    START_ANIMATION_RANDOM_TIME,
    //Start on a random time, but only if the mob just spawned.
    START_ANIMATION_RANDOM_TIME_ON_SPAWN,
};


#endif //ifndef MOB_ENUMS_INCLUDED
