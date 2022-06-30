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


//Carrying destinations.
enum CARRY_DESTINATIONS {
    //To a ship.
    CARRY_DESTINATION_SHIP,
    //To an Onion.
    CARRY_DESTINATION_ONION,
    //To the linked mob.
    CARRY_DESTINATION_LINKED_MOB,
};


//States a carrying spot can be in.
enum CARRY_SPOT_STATES {
    //Free.
    CARRY_SPOT_FREE,
    //Reserved; a Pikmin is coming to take this spot.
    CARRY_SPOT_RESERVED,
    //Taken.
    CARRY_SPOT_USED,
};


//Flags that control how a mob will chase.
enum CHASE_FLAGS {
    //The mob instantly teleports to the final destination.
    CHASE_FLAG_TELEPORT = 1,
    //When teleporting, do not consider the chase finished.
    CHASE_FLAG_TELEPORTS_CONSTANTLY = 2,
    //The mob can move in any angle instead of just where it's facing.
    CHASE_FLAG_ANY_ANGLE = 4,
};


//States for chasing.
enum CHASE_STATES {
    //No chasing in progress.
    CHASE_STATE_STOPPED,
    //Currently chasing.
    CHASE_STATE_CHASING,
    //Reached the destination and no longer chasing.
    CHASE_STATE_FINISHED,
};


//Animations to play when an item is delivered.
enum DELIVERY_ANIMATIONS {
    //Sucked up.
    DELIVERY_ANIM_SUCK,
    //Tossed into place.
    DELIVERY_ANIM_TOSS,
};


//Extra states for enemies to apply to their FSM.
enum ENEMY_EXTRA_STATES {
    //Carriable, waiting.
    ENEMY_EXTRA_STATE_CARRIABLE_WAITING,
    //Carriable, moving.
    ENEMY_EXTRA_STATE_CARRIABLE_MOVING,
    //Carriable, stuck.
    ENEMY_EXTRA_STATE_CARRIABLE_STUCK,
    //Carriable, thrown.
    ENEMY_EXTRA_STATE_CARRIABLE_THROWN,
    //Carriable, being delivered.
    ENEMY_EXTRA_STATE_BEING_DELIVERED,
};


//Possible results for a horizontal movement operation.
enum H_MOVE_RESULTS {
    //Move happened successfully.
    H_MOVE_OK,
    //The mob teleported.
    H_MOVE_TELEPORTED,
    //Move failed.
    H_MOVE_FAIL,
};


//How to process the rotation of mobs held by other mobs.
enum HOLD_ROTATION_METHODS {
    //Never rotate the held mob.
    HOLD_ROTATION_METHOD_NEVER,
    //The held mob should always face the holding mob.
    HOLD_ROTATION_METHOD_FACE_HOLDER,
    //The held mob should face the same angle the holding mob faces.
    HOLD_ROTATION_METHOD_COPY_HOLDER,
};


//Ways to draw limbs between mobs.
enum LIMB_DRAW_METHODS {
    //Draw the limb below the child mob and the parent mob.
    LIMB_DRAW_BELOW_BOTH,
    //Draw the limb directly below the child mob.
    LIMB_DRAW_BELOW_CHILD,
    //Draw the limb directly below the parent mob.
    LIMB_DRAW_BELOW_PARENT,
    //Draw the limb directly above the parent mob.
    LIMB_DRAW_ABOVE_PARENT,
    //Draw the limb directly above the child mob.
    LIMB_DRAW_ABOVE_CHILD,
    //Draw the limb above the child mob and the parent mob.
    LIMB_DRAW_ABOVE_BOTH,
};


//Flags that affect something about a mob.
enum MOB_FLAGS {
    //Can it currently move vertically on its own?
    MOB_FLAG_CAN_MOVE_MIDAIR = 0x01,
    //Is the mob airborne because it was thrown?
    MOB_FLAG_WAS_THROWN = 0x02,
    //Can it not be pushed?
    MOB_FLAG_UNPUSHABLE = 0x04,
    //Can it not be touched by other mobs?
    MOB_FLAG_INTANGIBLE = 0x08,
    //If it should be hidden (not drawn, no shadow, no health).
    MOB_FLAG_HIDDEN = 0x10,
    //If its shadow should be invisible.
    MOB_FLAG_SHADOW_INVISIBLE = 0x20,
    //Can this mob not be hunted down right now?
    MOB_FLAG_NON_HUNTABLE = 0x40,
    //Can this mob not be hurt right now?
    MOB_FLAG_NON_HURTABLE = 0x80,
};


//Types of target a mob can be.
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


//Mob teams.
enum MOB_TEAMS {
    //Has no friends!
    MOB_TEAM_NONE,
    //Player 1. Usually leaders and Pikmin.
    MOB_TEAM_PLAYER_1,
    //Player 2. Usually leaders and Pikmin.
    MOB_TEAM_PLAYER_2,
    //Player 3. Usually leaders and Pikmin.
    MOB_TEAM_PLAYER_3,
    //Player 4. Usually leaders and Pikmin.
    MOB_TEAM_PLAYER_4,
    //Enemies team 1. Useful if you want enemy in-fighting.
    MOB_TEAM_ENEMY_1,
    //Enemies team 2. Useful if you want enemy in-fighting.
    MOB_TEAM_ENEMY_2,
    //Enemies team 3. Useful if you want enemy in-fighting.
    MOB_TEAM_ENEMY_3,
    //Enemies team 4. Useful if you want enemy in-fighting.
    MOB_TEAM_ENEMY_4,
    //Miscellaneous obstacles.
    MOB_TEAM_OBSTACLE,
    //Whatever else.
    MOB_TEAM_OTHER,
    
    //Total amount of mob teams.
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


//Categories of mobs in sub-group types.
enum SUBGROUP_TYPE_CATEGORIES {
    //Pikmin.
    SUBGROUP_TYPE_CATEGORY_PIKMIN,
    //Leaders.
    SUBGROUP_TYPE_CATEGORY_LEADER,
    //Tools, like bomb rocks.
    SUBGROUP_TYPE_CATEGORY_TOOL,
};


#endif //ifndef MOB_ENUMS_INCLUDED
