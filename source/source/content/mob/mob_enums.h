/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for all enums related to mobs.
 */

#pragma once

//Area editor mob property types.
enum AEMP_TYPE {

    //Any user text.
    AEMP_TYPE_TEXT,
    
    //Integer number.
    AEMP_TYPE_INT,
    
    //Float number.
    AEMP_TYPE_FLOAT,
    
    //Boolean.
    AEMP_TYPE_BOOL,
    
    //One of a list of strings.
    AEMP_TYPE_LIST,
    
    //One of a list of numbers, though each has a name.
    AEMP_TYPE_NR_LIST,
    
};


//Carrying destinations.
enum CARRY_DESTINATION {

    //To a ship.
    CARRY_DESTINATION_SHIP,
    
    //To an Onion.
    CARRY_DESTINATION_ONION,

    //To the ship if there is no onion.
    CARRY_DESTINATION_SHIP_NO_ONION,
    
    //To the linked mob.
    CARRY_DESTINATION_LINKED_MOB,
    
    //To one of the linked mobs that match the decided Pikmin type.
    CARRY_DESTINATION_LINKED_MOB_MATCHING_TYPE,
    
};


//States a carrying spot can be in.
enum CARRY_SPOT_STATE {

    //Free.
    CARRY_SPOT_STATE_FREE,
    
    //Reserved; a Pikmin is coming to take this spot.
    CARRY_SPOT_STATE_RESERVED,
    
    //Taken.
    CARRY_SPOT_STATE_USED,
    
};


//Flags that control how a mob will chase.
enum CHASE_FLAG {

    //The mob instantly teleports to the final destination.
    CHASE_FLAG_TELEPORT = 1 << 0,
    
    //When teleporting, do not consider the chase finished.
    CHASE_FLAG_TELEPORTS_CONSTANTLY = 1 << 1,
    
    //The mob can move in any angle instead of just where it's facing.
    CHASE_FLAG_ANY_ANGLE = 1 << 2,
    
};


//States for chasing.
enum CHASE_STATE {

    //No chasing in progress.
    CHASE_STATE_STOPPED,
    
    //Currently chasing.
    CHASE_STATE_CHASING,
    
    //Reached the destination and no longer chasing.
    CHASE_STATE_FINISHED,
    
};


//Animations to play when an item is delivered.
enum DELIVERY_ANIM {

    //Sucked up.
    DELIVERY_ANIM_SUCK,
    
    //Tossed into place.
    DELIVERY_ANIM_TOSS,
    
};


//Extra states for enemies to apply to their FSM.
enum ENEMY_EXTRA_STATE {

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
enum H_MOVE_RESULT {

    //Move happened successfully.
    H_MOVE_RESULT_OK,
    
    //The mob teleported.
    H_MOVE_RESULT_TELEPORTED,
    
    //Move failed.
    H_MOVE_RESULT_FAIL,
    
};


//How to process the rotation of mobs held by other mobs.
enum HOLD_ROTATION_METHOD {

    //Never rotate the held mob.
    HOLD_ROTATION_METHOD_NEVER,
    
    //The held mob should always face the holding mob.
    HOLD_ROTATION_METHOD_FACE_HOLDER,
    
    //The held mob should face the same angle the holding mob faces.
    HOLD_ROTATION_METHOD_COPY_HOLDER,
    
};


//Flags that control how a mob works while inactive.
enum INACTIVE_LOGIC_FLAG {

    //The mob still performs timer and script ticks when inactive.
    INACTIVE_LOGIC_FLAG_TICKS = 1 << 0,
    
    //The mob still performs mob interactions when inactive.
    INACTIVE_LOGIC_FLAG_INTERACTIONS = 1 << 1,
    
};


//Ways to draw limbs between mobs.
enum LIMB_DRAW_METHOD {

    //Draw the limb below the child mob and the parent mob.
    LIMB_DRAW_METHOD_BELOW_BOTH,
    
    //Draw the limb directly below the child mob.
    LIMB_DRAW_METHOD_BELOW_CHILD,
    
    //Draw the limb directly below the parent mob.
    LIMB_DRAW_METHOD_BELOW_PARENT,
    
    //Draw the limb directly above the parent mob.
    LIMB_DRAW_METHOD_ABOVE_PARENT,
    
    //Draw the limb directly above the child mob.
    LIMB_DRAW_METHOD_ABOVE_CHILD,
    
    //Draw the limb above the child mob and the parent mob.
    LIMB_DRAW_METHOD_ABOVE_BOTH,
    
};


//Flags that affect something about a mob.
enum MOB_FLAG {

    //Can it currently move vertically on its own?
    MOB_FLAG_CAN_MOVE_MIDAIR = 1 << 0,
    
    //Is the mob airborne because it was thrown?
    MOB_FLAG_WAS_THROWN = 1 << 1,
    
    //Can it not be pushed?
    MOB_FLAG_UNPUSHABLE = 1 << 2,
    
    //Can it not be touched by other mobs?
    MOB_FLAG_INTANGIBLE = 1 << 3,
    
    //If it should be hidden (not drawn, no shadow, no health).
    MOB_FLAG_HIDDEN = 1 << 4,
    
    //If its shadow should be invisible.
    MOB_FLAG_SHADOW_INVISIBLE = 1 << 5,
    
    //Can this mob not be hunted down right now?
    MOB_FLAG_NON_HUNTABLE = 1 << 6,
    
    //Can this mob not be hurt right now?
    MOB_FLAG_NON_HURTABLE = 1 << 7,
    
};


//Flags for the types of target a mob can be.
enum MOB_TARGET_FLAG {

    //Cannot be damaged or hunted down.
    MOB_TARGET_FLAG_NONE = 1 << 0,
    
    //Leaders and Pikmin. Can be damaged by enemies, mostly.
    MOB_TARGET_FLAG_PLAYER = 1 << 1,
    
    //Enemies. Can be damaged by Pikmin and leaders, mostly.
    MOB_TARGET_FLAG_ENEMY = 1 << 2,
    
    //Weaker objects that can be damaged by many things.
    MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE = 1 << 3,
    
    //Stronger objects that can be damaged by less-than-many things.
    MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE = 1 << 4,
    
    //Objects that only Pikmin can damage.
    MOB_TARGET_FLAG_PIKMIN_OBSTACLE = 1 << 5,
    
    //Objects that can only be taken down with explosive force.
    MOB_TARGET_FLAG_EXPLODABLE = 1 << 6,
    
    //Objects that Pikmin and explosives can damage.
    MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE = 1 << 7,
    
    //Objects that can get hurt by pretty much everything.
    MOB_TARGET_FLAG_FRAGILE = 1 << 8,
    
};


//Flags for how a loaded script event's actions should work.
enum EVENT_LOAD_FLAG {
    //Run scripted actions after hardcoded actions.
    EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER = 1 << 0,
    
    //Run global actions after state specific actions.
    EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER = 1 << 1,
};


//Mob teams.
enum MOB_TEAM {

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


//Flags for what sprite bitmap effects to use.
enum SPRITE_BMP_EFFECT_FLAG {

    //Animation, position, angle, etc.
    SPRITE_BMP_EFFECT_FLAG_STANDARD = 1 << 0,
    
    //Effects from status effects.
    SPRITE_BMP_EFFECT_FLAG_STATUS = 1 << 1,
    
    //Darkening from the sector brightness.
    SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS = 1 << 2,
    
    //Height effect, both from being in the air and from falling into a pit.
    SPRITE_BMP_EFFECT_FLAG_HEIGHT = 1 << 3,
    
    //Onion delivery shrinking, and other types of delivery too.
    SPRITE_BMP_EFFECT_DELIVERY = 1 << 4,
    
    //Squash and stretch from being damaged.
    SPRITE_BMP_EFFECT_DAMAGE = 1 << 5,
    
    //Sway from being carried.
    SPRITE_BMP_EFFECT_CARRY = 1 << 6,
    
};


//Options for how to start a new animation.
enum START_ANIM_OPTION {

    //Start the new animation like normal.
    START_ANIM_OPTION_NORMAL,
    
    //Start from whatever frame index the previous animation was at.
    START_ANIM_OPTION_NO_RESTART,
    
    //Start on a random time.
    START_ANIM_OPTION_RANDOM_TIME,
    
    //Start on a random time, but only if the mob just spawned.
    START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN,
    
};


//Categories of mobs in sub-group types.
enum SUBGROUP_TYPE_CATEGORY {

    //Pikmin.
    SUBGROUP_TYPE_CATEGORY_PIKMIN,
    
    //Leaders.
    SUBGROUP_TYPE_CATEGORY_LEADER,
    
    //Tools, like bomb rocks.
    SUBGROUP_TYPE_CATEGORY_TOOL,
    
};
