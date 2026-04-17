/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the FSM event type classes and related functions.
 */


#pragma once


#include "../../util/general_utils.h"


//Types of FSM event.
enum FSM_EV {

    //"Special" events.
    
    //Unknown.
    FSM_EV_UNKNOWN,
    
    //When the state is entered.
    FSM_EV_ON_ENTER,
    
    //When the state is left.
    FSM_EV_ON_LEAVE,
    
    //When the game ticks a frame.
    FSM_EV_ON_TICK,
    
    //For mobs, triggered when the mob has been created, and has links and
    //such set up and ready.
    FSM_EV_ON_READY,
    
    //Script file stuff.
    
    //When the player's active leader is not the mob's current leader.
    FSM_EV_ACTIVE_LEADER_CHANGED,
    
    //When the current animation ends.
    FSM_EV_ANIMATION_END,
    
    //When it lands on a bottomless pit.
    FSM_EV_BOTTOMLESS_PIT,
    
    //When it is damaged.
    FSM_EV_DAMAGE,
    
    //When the mob is far away from its home.
    FSM_EV_FAR_FROM_HOME,
    
    //When the mob it was focused on died.
    FSM_EV_FOCUS_DIED,
    
    //When the mob it was focused on went past the "far" reach.
    FSM_EV_FOCUS_OFF_REACH,
    
    //When a frame of animation sends a signal.
    FSM_EV_FRAME_SIGNAL,
    
    //When it just got held by another mob.
    FSM_EV_HELD,
    
    //When one of its normal hitboxes touches another mob's eating hitbox.
    FSM_EV_HITBOX_TOUCH_EAT,
    
    //When the player performs an input.
    FSM_EV_INPUT_RECEIVED,
    
    //When it has been damaged enough to want to shake.
    FSM_EV_ITCH,
    
    //When it leaves a hazard in a sector.
    FSM_EV_LEFT_HAZARD,
    
    //When an object is within the "near" reach.
    FSM_EV_OBJECT_IN_REACH,
    
    //When an opponent is within the "near" reach.
    FSM_EV_OPPONENT_IN_REACH,
    
    //When a Pikmin lands on it.
    FSM_EV_THROWN_PIKMIN_LANDED,
    
    //When it reaches its destination.
    FSM_EV_REACHED_DESTINATION,
    
    //When it receives a message from another mob.
    FSM_EV_RECEIVE_MESSAGE,
    
    //When it is safely released from the leader's/enemy's grasp.
    FSM_EV_RELEASED,
    
    //When a mob has landed on top of it. Only if this mob's walkable.
    FSM_EV_RIDER_ADDED,
    
    //When a mob that was on top of it has left. Only if this mob's walkable.
    FSM_EV_RIDER_REMOVED,
    
    //When it is swallowed by an enemy.
    FSM_EV_SWALLOWED,
    
    //When it gets touched by a leader.
    FSM_EV_TOUCHED_ACTIVE_LEADER,
    
    //When it touches a hazard (sector or hitbox).
    FSM_EV_TOUCHED_HAZARD,
    
    //When it touches a sprayed spray.
    FSM_EV_TOUCHED_SPRAY,
    
    //When it gets touched by an object.
    FSM_EV_TOUCHED_OBJECT,
    
    //When it gets touched by an opponent.
    FSM_EV_TOUCHED_OPPONENT,
    
    //When it touches a wall.
    FSM_EV_TOUCHED_WALL,
    
    //When its timer ticks.
    FSM_EV_TIMER,
    
    //When weight has been added on top of it. Only if this mob's walkable.
    FSM_EV_WEIGHT_ADDED,
    
    //When weight was removed from on top of it. Only if this mob's walkable.
    FSM_EV_WEIGHT_REMOVED,
    
    //More internal script stuff.
    
    //When it is plucked off the ground (Pikmin only).
    FSM_EV_PLUCKED,
    
    //When it is grabbed by a friend.
    FSM_EV_GRABBED_BY_FRIEND,
    
    //When it is dismissed by its leader.
    FSM_EV_DISMISSED,
    
    //When it is thrown.
    FSM_EV_THROWN,
    
    //When it lands on the ground.
    FSM_EV_LANDED,
    
    //When it is ordered to release whatever it is holding.
    FSM_EV_RELEASE_ORDER,
    
    //When it is whistled by a leader.
    FSM_EV_WHISTLED,
    
    //When its spot on the group is now far, and the mob is in the group.
    FSM_EV_SPOT_IS_FAR,
    
    //When the group the mob is on started swarming.
    FSM_EV_SWARM_STARTED,
    
    //When the group the mob is on stopped swarming.
    FSM_EV_SWARM_ENDED,
    
    //When the mob is ordered to go to an Onion to be stored inside.
    FSM_EV_GO_TO_ONION,
    
    //When the Pikmin successfully finish their current task, like carrying.
    FSM_EV_FINISHED_TASK,
    
    //When the Pikmin is near an object that can be carried.
    FSM_EV_NEAR_CARRIABLE_OBJECT,
    
    //When the Pikmin is near a tool object.
    FSM_EV_NEAR_TOOL,
    
    //When the Pikmin is near a group task.
    FSM_EV_NEAR_GROUP_TASK,
    
    //When one of its attack hitboxes touches another mob's normal hitbox.
    FSM_EV_HITBOX_TOUCH_A_N,
    
    //When one of its normal hitboxes touches another mob's attack hitbox.
    FSM_EV_HITBOX_TOUCH_N_A,
    
    //When one of its normal hitboxes touches another mob's normal hitbox.
    FSM_EV_HITBOX_TOUCH_N_N,
    
    //When a Pikmin is confirmed to have to take damage from an attack.
    FSM_EV_PIKMIN_DAMAGE_CONFIRMED,
    
    //When a Pikmin was added to the list of Pikmin carrying this mob.
    FSM_EV_CARRIER_ADDED,
    
    //When a Pikmin was removed from the list of Pikmin carrying this mob.
    FSM_EV_CARRIER_REMOVED,
    
    //When the mob needs to begin moving, as it's being carried.
    FSM_EV_CARRY_BEGIN_MOVE,
    
    //When the mob needs to stop moving, as it's no longer being carried.
    FSM_EV_CARRY_STOP_MOVE,
    
    //When the mob was successfully delivered
    //to its destination after being carried.
    FSM_EV_CARRY_DELIVERED,
    
    //When the mob following a path encounters an obstacle.
    FSM_EV_PATH_BLOCKED,
    
    //When the paths in the area changed, and the mob may have a new way to go.
    FSM_EV_PATHS_CHANGED,
    
    //When the focused mob stops being able to be focused.
    FSM_EV_FOCUSED_MOB_UNAVAILABLE,
    
    //When the mob starts to receive an object that was carried to it.
    FSM_EV_STARTED_RECEIVING_DELIVERY,
    
    //When the mob finishes receiving an object that was carried to it.
    FSM_EV_FINISHED_RECEIVING_DELIVERY,
    
    //When the mob touches a drop that it can consume.
    FSM_EV_TOUCHED_DROP,
    
    //When the mob touches a track object.
    FSM_EV_TOUCHED_TRACK,
    
    //When the mob touches a bouncer object.
    FSM_EV_TOUCHED_BOUNCER,
    
    //When it has zero health.
    FSM_EV_ZERO_HEALTH,
    
    //Events that only leaders can really handle.
    
    //When the leader becomes the one controlled by the player.
    LEADER_EV_ACTIVATED,
    
    //When the leader stops being the one controlled by the player.
    LEADER_EV_INACTIVATED,
    
    //When the leader begins moving.
    LEADER_EV_MOVE_START,
    
    //When the leader stops moving.
    LEADER_EV_MOVE_END,
    
    //When the leader is holding a Pikmin in their hand.
    LEADER_EV_HOLDING,
    
    //When the leader throws the Pikmin in their hand.
    LEADER_EV_THROW,
    
    //When the leader begins whistling.
    LEADER_EV_START_WHISTLE,
    
    //When the leader stops whistling.
    LEADER_EV_STOP_WHISTLE,
    
    //When the leader throws a punch.
    LEADER_EV_PUNCH,
    
    //When the leader dismisses their group.
    LEADER_EV_DISMISS,
    
    //When the leader uses a spray.
    LEADER_EV_SPRAY,
    
    //When the leader opens the inventory.
    LEADER_EV_INVENTORY,
    
    //When the leader falls asleep.
    LEADER_EV_FALL_ASLEEP,
    
    //When the leader has to go towards the Pikmin it intends to pluck.
    LEADER_EV_GO_PLUCK,
    
    //When the leader has to go help pluck Pikmin, as an inactive leader.
    LEADER_EV_MUST_SEARCH_SEED,
    
    //When the leader has to follow a path via Go Here.
    LEADER_EV_GO_HERE,
    
    //When the leader's current "thing" is canceled.
    LEADER_EV_CANCEL,
    
    
    //Total.
    
    //Total amount of FSM event types.
    N_FSM_EVENTS,
    
};


//FSM event enum naming (internal names for script files only).
buildEnumNames(scriptEvScriptFileINames, FSM_EV)({
    { FSM_EV_ON_ENTER, "on_enter" },
    { FSM_EV_ON_LEAVE, "on_leave" },
    { FSM_EV_ON_TICK, "on_tick" },
    { FSM_EV_ON_READY, "on_ready" },
    { FSM_EV_ACTIVE_LEADER_CHANGED, "on_active_leader_changed" },
    { FSM_EV_ANIMATION_END, "on_animation_end" },
    { FSM_EV_DAMAGE, "on_damage" },
    { FSM_EV_FAR_FROM_HOME, "on_far_from_home" },
    { FSM_EV_FINISHED_RECEIVING_DELIVERY, "on_finish_receiving_delivery" },
    { FSM_EV_FOCUS_OFF_REACH, "on_focus_off_reach" },
    { FSM_EV_FRAME_SIGNAL, "on_frame_signal" },
    { FSM_EV_HELD, "on_held" },
    { FSM_EV_HITBOX_TOUCH_EAT, "on_hitbox_touch_eat" },
    { FSM_EV_HITBOX_TOUCH_A_N, "on_hitbox_touch_a_n" },
    { FSM_EV_HITBOX_TOUCH_N_N, "on_hitbox_touch_n_n" },
    { FSM_EV_INPUT_RECEIVED, "on_input_received" },
    { FSM_EV_ITCH, "on_itch" },
    { FSM_EV_LANDED, "on_land" },
    { FSM_EV_LEFT_HAZARD, "on_leave_hazard" },
    { FSM_EV_OBJECT_IN_REACH, "on_object_in_reach" },
    { FSM_EV_OPPONENT_IN_REACH, "on_opponent_in_reach" },
    { FSM_EV_THROWN_PIKMIN_LANDED, "on_pikmin_land" },
    { FSM_EV_RECEIVE_MESSAGE, "on_receive_message" },
    { FSM_EV_RELEASED, "on_released" },
    { FSM_EV_REACHED_DESTINATION, "on_reach_destination" },
    { FSM_EV_STARTED_RECEIVING_DELIVERY, "on_start_receiving_delivery" },
    { FSM_EV_SWALLOWED, "on_swallowed" },
    { FSM_EV_TIMER, "on_timer" },
    { FSM_EV_TOUCHED_HAZARD, "on_touch_hazard" },
    { FSM_EV_TOUCHED_OBJECT, "on_touch_object" },
    { FSM_EV_TOUCHED_OPPONENT, "on_touch_opponent" },
    { FSM_EV_TOUCHED_WALL, "on_touch_wall" },
    { FSM_EV_WEIGHT_ADDED, "on_weight_added" },
    { FSM_EV_WEIGHT_REMOVED, "on_weight_removed" },
});
