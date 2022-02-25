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


#ifndef MOB_EV_INCLUDED
#define MOB_EV_INCLUDED

#include <vector>

#include "utils/data_file.h"


using std::size_t;
using std::string;
using std::vector;


class mob;
struct mob_action_call;
class mob_type;
class mob_state;
class hitbox;

typedef void (*custom_action_code)(mob* m, void* info1, void* info2);

const unsigned char STATE_HISTORY_SIZE = 3;

//Types of script events.
enum MOB_EV_TYPES {
    //"Special" events.
    
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
    //When it dies.
    MOB_EV_DEATH,
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
    //When the leader's pluck is canceled.
    LEADER_EV_CANCEL,
    
    N_MOB_EVENTS,
};


class mob_event {
public:
    MOB_EV_TYPES type;
    vector<mob_action_call*> actions;
    
    void run(mob* m, void* custom_data_1 = NULL, void* custom_data_2 = NULL);
    mob_event(data_node* node, const vector<mob_action_call*> &actions);
    mob_event(
        const MOB_EV_TYPES t,
        const vector<mob_action_call*> &a = vector<mob_action_call*>()
    );
};


class mob_state {
public:
    string name;
    size_t id;
    mob_event* events[N_MOB_EVENTS];
    mob_event* get_event(const MOB_EV_TYPES type) const;
    
    mob_state(const string &name);
    mob_state(const string &name, mob_event* evs[N_MOB_EVENTS]);
    mob_state(const string &name, const size_t id);
};


class mob_fsm {
public:
    mob* m;
    mob_state* cur_state;
    //Conversion between pre-named states and in-file states.
    vector<size_t> pre_named_conversions;
    //Knowing the previous states' names helps with debugging.
    string prev_state_names[STATE_HISTORY_SIZE];
    //If this is INVALID, use the mob type's first state nr. Else, use this.
    size_t first_state_override;
    
    mob_event* get_event(const MOB_EV_TYPES type) const;
    size_t get_state_nr(const string &name) const;
    void run_event(
        const MOB_EV_TYPES type,
        void* custom_data_1 = NULL, void* custom_data_2 = NULL
    );
    bool set_state(
        const size_t new_state, void* info1 = NULL, void* info2 = NULL
    );
    mob_fsm(mob* m = NULL);
};


/* ----------------------------------------------------------------------------
 * The easy fsm creator makes it easy to create mob FSMs in C++ code.
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
class easy_fsm_creator {
public:
    void new_state(const string &name, const size_t id);
    void new_event(const MOB_EV_TYPES type);
    void change_state(const string &new_state);
    void run(custom_action_code code);
    vector<mob_state*> finish();
    easy_fsm_creator();
    
private:
    vector<mob_state*> states;
    mob_state* cur_state;
    mob_event* cur_event;
    void commit_state();
    void commit_event();
    
};


struct hitbox_interaction {
    mob* mob2;  //Mob that touched our mob.
    hitbox* h1; //Hitbox of our mob that got touched.
    hitbox* h2; //Hitbox of the other mob.
    hitbox_interaction(
        mob* mob2 = NULL,
        hitbox* h1 = NULL, hitbox* h2 = NULL
    );
};


size_t fix_states(
    vector<mob_state*> &states, const string &starting_state, mob_type* mt
);
void load_script(mob_type* mt, data_node* node, vector<mob_state*>* states);
void unload_script(mob_type* mt);


#endif //ifndef MOB_EV_INCLUDED
