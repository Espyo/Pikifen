/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob script classes and
 * related functions.
 */


#pragma once

#include <vector>

#include "../../core/const.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/general_utils.h"


using std::size_t;
using std::string;
using std::vector;


class Mob;
struct MobActionCall;
class MobType;
class MobState;
class Hitbox;

/**
 * @brief Function to run custom mob actions with.
 *
 * The first parameter is the mob running the action.
 * The second parameter depends on the function.
 * The third parameter depends on the function.
 */
typedef void (*custom_action_code_t)(Mob* m, void* info1, void* info2);

const unsigned char STATE_HISTORY_SIZE = 3;

//Types of script events.
enum MOB_EV {

    //"Special" events.
    
    //Unknown.
    MOB_EV_UNKNOWN,
    
    //When the state is entered.
    MOB_EV_ON_ENTER,
    
    //When the state is left.
    MOB_EV_ON_LEAVE,
    
    //When the game ticks a frame.
    MOB_EV_ON_TICK,
    
    //When the mob has been created, and has links and such set up and ready.
    MOB_EV_ON_READY,
    
    //Script file stuff.
    
    //When the current animation ends.
    MOB_EV_ANIMATION_END,
    
    //When it lands on a bottomless pit.
    MOB_EV_BOTTOMLESS_PIT,
    
    //When it is damaged.
    MOB_EV_DAMAGE,
    
    //When the mob is far away from its home.
    MOB_EV_FAR_FROM_HOME,
    
    //When the mob it was focused on died.
    MOB_EV_FOCUS_DIED,
    
    //When the mob it was focused on went past the "far" reach.
    MOB_EV_FOCUS_OFF_REACH,
    
    //When a frame of animation sends a signal.
    MOB_EV_FRAME_SIGNAL,
    
    //When it just got held by another mob.
    MOB_EV_HELD,
    
    //When one of its normal hitboxes touches another mob's eating hitbox.
    MOB_EV_HITBOX_TOUCH_EAT,
    
    //When the player performs an input.
    MOB_EV_INPUT_RECEIVED,
    
    //When it has been damaged enough to want to shake.
    MOB_EV_ITCH,
    
    //When it leaves a hazard in a sector.
    MOB_EV_LEFT_HAZARD,
    
    //When an object is within the "near" reach.
    MOB_EV_OBJECT_IN_REACH,
    
    //When an opponent is within the "near" reach.
    MOB_EV_OPPONENT_IN_REACH,
    
    //When a Pikmin lands on it.
    MOB_EV_THROWN_PIKMIN_LANDED,
    
    //When it reaches its destination.
    MOB_EV_REACHED_DESTINATION,
    
    //When it receives a message from another mob.
    MOB_EV_RECEIVE_MESSAGE,
    
    //When it is safely released from the leader's/enemy's grasp.
    MOB_EV_RELEASED,
    
    //When a mob has landed on top of it. Only if this mob's walkable.
    MOB_EV_RIDER_ADDED,
    
    //When a mob that was on top of it has left. Only if this mob's walkable.
    MOB_EV_RIDER_REMOVED,
    
    //When it is swallowed by an enemy.
    MOB_EV_SWALLOWED,
    
    //When it gets touched by a leader.
    MOB_EV_TOUCHED_ACTIVE_LEADER,
    
    //When it touches a hazard (sector or hitbox).
    MOB_EV_TOUCHED_HAZARD,
    
    //When it touches a sprayed spray.
    MOB_EV_TOUCHED_SPRAY,
    
    //When it gets touched by an object.
    MOB_EV_TOUCHED_OBJECT,
    
    //When it gets touched by an opponent.
    MOB_EV_TOUCHED_OPPONENT,
    
    //When it touches a wall.
    MOB_EV_TOUCHED_WALL,
    
    //When its timer ticks.
    MOB_EV_TIMER,
    
    //When weight has been added on top of it. Only if this mob's walkable.
    MOB_EV_WEIGHT_ADDED,
    
    //When weight was removed from on top of it. Only if this mob's walkable.
    MOB_EV_WEIGHT_REMOVED,
    
    //More internal script stuff.
    
    //When it is plucked off the ground (Pikmin only).
    MOB_EV_PLUCKED,
    
    //When it is grabbed by a friend.
    MOB_EV_GRABBED_BY_FRIEND,
    
    //When it is dismissed by its leader.
    MOB_EV_DISMISSED,
    
    //When it is thrown.
    MOB_EV_THROWN,
    
    //When it lands on the ground.
    MOB_EV_LANDED,
    
    //When it is ordered to release whatever it is holding.
    MOB_EV_RELEASE_ORDER,
    
    //When it is whistled by a leader.
    MOB_EV_WHISTLED,
    
    //When its spot on the group is now far, and the mob is in the group.
    MOB_EV_SPOT_IS_FAR,
    
    //When the group the mob is on started swarming.
    MOB_EV_SWARM_STARTED,
    
    //When the group the mob is on stopped swarming.
    MOB_EV_SWARM_ENDED,
    
    //When the mob is ordered to go to an Onion to be stored inside.
    MOB_EV_GO_TO_ONION,
    
    //When the object the Pikmin was carrying gets delivered.
    MOB_EV_FINISHED_CARRYING,
    
    //When the Pikmin is near an object that can be carried.
    MOB_EV_NEAR_CARRIABLE_OBJECT,
    
    //When the Pikmin is near a tool object.
    MOB_EV_NEAR_TOOL,
    
    //When the Pikmin is near a group task.
    MOB_EV_NEAR_GROUP_TASK,
    
    //When one of its attack hitboxes touches another mob's normal hitbox.
    MOB_EV_HITBOX_TOUCH_A_N,
    
    //When one of its normal hitboxes touches another mob's attack hitbox.
    MOB_EV_HITBOX_TOUCH_N_A,
    
    //When one of its normal hitboxes touches another mob's normal hitbox.
    MOB_EV_HITBOX_TOUCH_N_N,
    
    //When a Pikmin is confirmed to have to take damage from an attack.
    MOB_EV_PIKMIN_DAMAGE_CONFIRMED,
    
    //When a Pikmin was added to the list of Pikmin carrying this mob.
    MOB_EV_CARRIER_ADDED,
    
    //When a Pikmin was removed from the list of Pikmin carrying this mob.
    MOB_EV_CARRIER_REMOVED,
    
    //When the mob needs to begin moving, as it's being carried.
    MOB_EV_CARRY_BEGIN_MOVE,
    
    //When the mob needs to stop moving, as it's no longer being carried.
    MOB_EV_CARRY_STOP_MOVE,
    
    //When the mob was sucessfully delivered
    //to its destination after being carried.
    MOB_EV_CARRY_DELIVERED,
    
    //When the mob following a path encounters an obstacle.
    MOB_EV_PATH_BLOCKED,
    
    //When the paths in the area changed, and the mob may have a new way to go.
    MOB_EV_PATHS_CHANGED,
    
    //When the focused mob stops being able to be focused.
    MOB_EV_FOCUSED_MOB_UNAVAILABLE,
    
    //When the mob starts to receive an object that was carried to it.
    MOB_EV_STARTED_RECEIVING_DELIVERY,
    
    //When the mob finishes receiving an object that was carried to it.
    MOB_EV_FINISHED_RECEIVING_DELIVERY,
    
    //When the mob touches a drop that it can consume.
    MOB_EV_TOUCHED_DROP,
    
    //When the mob touches a track object.
    MOB_EV_TOUCHED_TRACK,
    
    //When the mob touches a bouncer object.
    MOB_EV_TOUCHED_BOUNCER,
    
    //When it has zero health.
    MOB_EV_ZERO_HEALTH,
    
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
    
    //When the leader lies down.
    LEADER_EV_LIE_DOWN,
    
    //When the leader has to go towards the Pikmin it intends to pluck.
    LEADER_EV_GO_PLUCK,
    
    //When the leader has to go help pluck Pikmin, as an inactive leader.
    LEADER_EV_MUST_SEARCH_SEED,
    
    //When the leader has to follow a path via Go Here.
    LEADER_EV_GO_HERE,
    
    //When the leader's pluck is canceled.
    LEADER_EV_CANCEL,
    
    
    //Total.
    
    //Total amount of mob event types.
    N_MOB_EVENTS,
    
};


/**
 * @brief Actions to run on an event, inside a FSM.
 */
class MobEvent {

public:

    //--- Members ---
    
    //Type of event.
    MOB_EV type = MOB_EV_UNKNOWN;
    
    //Actions to run.
    vector<MobActionCall*> actions;
    
    
    //--- Function declarations ---
    
    MobEvent(
        const DataNode* node, const vector<MobActionCall*> &actions
    );
    explicit MobEvent(
        const MOB_EV t,
        const vector<MobActionCall*> &a = vector<MobActionCall*>()
    );
    void run(Mob* m, void* custom_data_1 = nullptr, void* custom_data_2 = nullptr);
    
};


/**
 * @brief A mob's state in its FSM. A mob can only be in one state at any given
 * time. Multiple mobs can share these states.
 */
class MobState {

public:

    //--- Members ---
    
    //Name of the state.
    string name;
    
    //State ID.
    size_t id = INVALID;
    
    //List of events to handle in this state.
    MobEvent* events[N_MOB_EVENTS];
    
    
    //--- Function declarations ---
    
    explicit MobState(const string &name);
    MobState(const string &name, MobEvent* evs[N_MOB_EVENTS]);
    MobState(const string &name, size_t id);
    MobEvent* get_event(const MOB_EV type) const;
    
};


/**
 * @brief A mob's instance of a finite state machine. It contains information
 * about what state it is in, and so on, but does not contain the list
 * of states, events, and actions.
 */
class MobFsm {

public:

    //--- Members ---
    
    //Mob that this FSM belongs to.
    Mob* m = nullptr;
    
    //Current state the mob is in.
    MobState* cur_state = nullptr;
    
    //Conversion between pre-named states and in-file states.
    vector<size_t> pre_named_conversions;
    
    //Knowing the previous states' names helps with debugging.
    string prev_state_names[STATE_HISTORY_SIZE];
    
    //If this is INVALID, use the mob type's first state index. Else, use this.
    size_t first_state_override = INVALID;
    
    
    //--- Function declarations ---
    
    explicit MobFsm(Mob* m = nullptr);
    MobEvent* get_event(const MOB_EV type) const;
    size_t get_state_idx(const string &name) const;
    void run_event(
        const MOB_EV type,
        void* custom_data_1 = nullptr, void* custom_data_2 = nullptr
    );
    bool set_state(
        size_t new_state, void* info1 = nullptr, void* info2 = nullptr
    );
    
};


/**
 * @brief The easy fsm creator makes it easy to create mob FSMs in C++ code.

 * For mobs created by the game creator, the state machine is simpler,
 * and written in plain text using a data file. But for the engine and
 * some preset FSMs, like the Pikmin and leader logic, there's no good way
 * to create a finite state machine with something as simple as plain text
 * AND still give the events custom code to run.
 * The only way is to manually create a vector of states, for every
 * state, manually create the events, and for every event, manually
 * create the actions. Boring and ugly. That's why this class was born.
 * Creating a state, event, or action, are now all a single line, much like
 * they would be in a plain text file!
 */
class EasyFsmCreator {

public:

    //--- Function declarations ---
    
    void new_state(const string &name, size_t id);
    void new_event(const MOB_EV type);
    void change_state(const string &new_state);
    void run(custom_action_code_t code);
    vector<MobState*> finish();
    
private:

    //--- Members ---
    
    //List of registered states.
    vector<MobState*> states;
    
    //State currently being staged.
    MobState* cur_state = nullptr;
    
    //Event currently being staged.
    MobEvent* cur_event = nullptr;
    
    
    //--- Function declarations ---
    
    void commit_state();
    void commit_event();
    
};


/**
 * @brief Info about how two hitboxes interacted.
 */
struct HitboxInteraction {

    //--- Members ---
    
    //Mob that touched our mob.
    Mob* mob2 = nullptr;
    
    //Hitbox of our mob that got touched.
    Hitbox* h1 = nullptr;
    
    //Hitbox of the other mob.
    Hitbox* h2 = nullptr;
    
    
    //--- Function declarations ---
    
    explicit HitboxInteraction(
        Mob* mob2 = nullptr,
        Hitbox* h1 = nullptr, Hitbox* h2 = nullptr
    );
    
};


size_t fix_states(
    vector<MobState*> &states, const string &starting_state, const MobType* mt
);
void load_script(
    MobType* mt, DataNode* script_node, DataNode* global_node,
    vector<MobState*>* out_states
);
void load_state(
    MobType* mt, DataNode* state_node, DataNode* global_node,
    MobState* state_ptr
);
void unload_script(MobType* mt);
