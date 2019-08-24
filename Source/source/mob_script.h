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


#ifndef MOB_EVENT_INCLUDED
#define MOB_EVENT_INCLUDED

#include <vector>

#include "data_file.h"

using namespace std;

class mob;
class mob_type;
class mob_state;
class hitbox;

const unsigned char STATE_HISTORY_SIZE = 3;

typedef void (*custom_action_code)(mob* m, void* info1, void* info2);


//Types of script action.
enum MOB_ACTION_TYPES {
    MOB_ACTION_UNKNOWN,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC,
    MOB_ACTION_DELETE,
    MOB_ACTION_ELSE,
    MOB_ACTION_END_IF,
    MOB_ACTION_GET_CHOMPED,
    MOB_ACTION_FOCUS,
    MOB_ACTION_FINISH_DYING,
    MOB_ACTION_IF,
    MOB_ACTION_MOVE,
    MOB_ACTION_ORDER_RELEASE,
    MOB_ACTION_PLAY_SOUND,
    MOB_ACTION_RANDOMIZE_VAR,
    MOB_ACTION_RECEIVE_STATUS,
    MOB_ACTION_RELEASE,
    MOB_ACTION_REMOVE_STATUS,
    MOB_ACTION_SEND_MESSAGE,
    MOB_ACTION_SET_ANIMATION,
    MOB_ACTION_SET_FAR_REACH,
    MOB_ACTION_SET_GRAVITY,
    MOB_ACTION_SET_HEALTH,
    MOB_ACTION_SET_HIDING,
    MOB_ACTION_SET_HOLDABLE,
    MOB_ACTION_SET_LIMB_ANIMATION,
    MOB_ACTION_SET_NEAR_REACH,
    MOB_ACTION_SET_STATE,
    MOB_ACTION_SET_TANGIBLE,
    MOB_ACTION_SET_TEAM,
    MOB_ACTION_SET_TIMER,
    MOB_ACTION_SET_VAR,
    MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
    MOB_ACTION_SPAWN,
    MOB_ACTION_STABILIZE_Z,
    MOB_ACTION_START_CHOMPING,
    MOB_ACTION_START_DYING,
    MOB_ACTION_START_HEIGHT_EFFECT,
    MOB_ACTION_START_PARTICLES,
    MOB_ACTION_STOP,
    MOB_ACTION_STOP_CHOMPING,
    MOB_ACTION_STOP_HEIGHT_EFFECT,
    MOB_ACTION_STOP_PARTICLES,
    MOB_ACTION_SWALLOW,
    MOB_ACTION_TELEPORT,
    MOB_ACTION_TURN,
};

//Arachnorb plan logic action sub-types.
enum MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPES {
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN,
};

//Swallowing action sub-types.
enum MOB_ACTION_SWALLOW_TYPES {
    MOB_ACTION_SWALLOW_ALL,
    MOB_ACTION_SWALLOW_NUMBER,
};

//Face action sub-types.
enum MOB_ACTION_TURN_TYPES {
    MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC,
    MOB_ACTION_TURN_FOCUSED_MOB,
    MOB_ACTION_TURN_HOME,
    MOB_ACTION_TURN_RANDOMLY,
    MOB_ACTION_TURN_ABSOLUTE,
    MOB_ACTION_TURN_RELATIVE,
};

//Focus action sub-types.
enum MOB_ACTION_FOCUS_TYPES {
    MOB_ACTION_FOCUS_TRIGGER,
    MOB_ACTION_FOCUS_PARENT,
};

//If action operator types.
enum MOB_ACTION_IF_OPERATOR_TYPES {
    MOB_ACTION_IF_OP_EQUAL,
    MOB_ACTION_IF_OP_NOT,
    MOB_ACTION_IF_OP_LESS,
    MOB_ACTION_IF_OP_MORE,
    MOB_ACTION_IF_OP_LESS_E,
    MOB_ACTION_IF_OP_MORE_E,
};

//If action LHS comparand type.
enum MOB_ACTION_IF_LHS_TYPES {
    MOB_ACTION_IF_LHS_VAR,
    MOB_ACTION_IF_LHS_BODY_PART,
    MOB_ACTION_IF_LHS_CHOMPED_PIKMIN,
    MOB_ACTION_IF_LHS_DAY_MINUTES,
    MOB_ACTION_IF_LHS_FIELD_PIKMIN,
    MOB_ACTION_IF_LHS_FRAME_SIGNAL,
    MOB_ACTION_IF_LHS_HEALTH,
    MOB_ACTION_IF_LHS_LATCHED_PIKMIN,
    MOB_ACTION_IF_LHS_LATCHED_PIKMIN_WEIGHT,
    MOB_ACTION_IF_LHS_MESSAGE,
    MOB_ACTION_IF_LHS_MESSAGE_SENDER,
    MOB_ACTION_IF_LHS_MOB_CATEGORY,
    MOB_ACTION_IF_LHS_MOB_TYPE,
    MOB_ACTION_IF_LHS_OTHER_BODY_PART,
};

//If action RHS comparand types.
enum MOB_ACTION_IF_RHS_TYPES {
    MOB_ACTION_IF_RHS_VAR,
    MOB_ACTION_IF_RHS_CONST,
};

//Moving action sub-types.
enum MOB_ACTION_MOVE_TYPES {
    MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB,
    MOB_ACTION_MOVE_FOCUSED_MOB,
    MOB_ACTION_MOVE_FOCUSED_MOB_POS,
    MOB_ACTION_MOVE_HOME,
    MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC,
    MOB_ACTION_MOVE_LINKED_MOB_AVERAGE,
    MOB_ACTION_MOVE_COORDS,
    MOB_ACTION_MOVE_REL_COORDS,
    MOB_ACTION_MOVE_RANDOMLY,
};

//Timer action sub-types.
enum MOB_ACTION_SET_TIMER_TYPES {
    MOB_ACTION_SET_TIMER_NUMBER,
    MOB_ACTION_SET_TIMER_RANDOM,
};

//Set var action sub-types.
enum MOB_ACTION_SET_VAR_TYPES {
    MOB_ACTION_SET_VAR_VALUE,
    MOB_ACTION_SET_VAR_SUM,
    MOB_ACTION_SET_VAR_SUBTRACT,
    MOB_ACTION_SET_VAR_MULTIPLY,
    MOB_ACTION_SET_VAR_DIVIDE,
    MOB_ACTION_SET_VAR_MODULO,
};

//Stabilize Z action sub-types.
enum MOB_ACTION_STABILIZE_Z_TYPES {
    MOB_ACTION_STABILIZE_Z_HIGHEST,
    MOB_ACTION_STABILIZE_Z_LOWEST,
};

//Numerical action sub-types.
enum MOB_ACTION_NUMERICAL_TYPES {
    MOB_ACTION_NUMERICAL_ABSOLUTE,
    MOB_ACTION_NUMERICAL_RELATIVE,
};

//Types of script events.
enum MOB_EVENT_TYPES {
    //"Special" events.
    
    MOB_EVENT_UNKNOWN,
    //When the state is entered.
    MOB_EVENT_ON_ENTER,
    //When the state is left.
    MOB_EVENT_ON_LEAVE,
    //When the game ticks a frame.
    MOB_EVENT_ON_TICK,
    
    //Script file stuff.
    
    //When the current animation ends.
    MOB_EVENT_ANIMATION_END,
    //When it lands on a bottomless pit.
    MOB_EVENT_BOTTOMLESS_PIT,
    //When it is damaged.
    MOB_EVENT_DAMAGE,
    //When it dies.
    MOB_EVENT_DEATH,
    //When the mob is far away from its home.
    MOB_EVENT_FAR_FROM_HOME,
    //When the mob it was focused on died.
    MOB_EVENT_FOCUS_DIED,
    //When the mob it was focused on went past the "far" reach.
    MOB_EVENT_FOCUS_OFF_REACH,
    //When a frame of animation sends a signal.
    MOB_EVENT_FRAME_SIGNAL,
    //When it just got held by another mob.
    MOB_EVENT_HELD,
    //When one of its normal hitboxes touches another mob's eating hitbox.
    MOB_EVENT_HITBOX_TOUCH_EAT,
    //When it has been damaged enough to want to shake.
    MOB_EVENT_ITCH,
    //When it leaves a hazard in a sector.
    MOB_EVENT_LEFT_HAZARD,
    //When an object is within the "near" reach.
    MOB_EVENT_OBJECT_IN_REACH,
    //When an opponent is within the "near" reach.
    MOB_EVENT_OPPONENT_IN_REACH,
    //When a Pikmin lands on it.
    MOB_EVENT_PIKMIN_LANDED,
    //When it reaches its destination.
    MOB_EVENT_REACHED_DESTINATION,
    //When it receives a message from another mob.
    MOB_EVENT_RECEIVE_MESSAGE,
    //When it is safely released from the leader's/enemy's grasp.
    MOB_EVENT_RELEASED,
    //When it gets touched by a leader.
    MOB_EVENT_TOUCHED_ACTIVE_LEADER,
    //When it touches a hazard (sector or hitbox).
    MOB_EVENT_TOUCHED_HAZARD,
    //When it touches a sprayed spray.
    MOB_EVENT_TOUCHED_SPRAY,
    //When it gets touched by an object.
    MOB_EVENT_TOUCHED_OBJECT,
    //When it gets touched by an opponent.
    MOB_EVENT_TOUCHED_OPPONENT,
    //When its timer ticks.
    MOB_EVENT_TIMER,
    //When it touches a wall.
    MOB_EVENT_TOUCHED_WALL,
    
    //More internal script stuff.
    
    //When it is plucked off the ground (Pikmin only).
    MOB_EVENT_PLUCKED,
    //When it is grabbed by a friend.
    MOB_EVENT_GRABBED_BY_FRIEND,
    //When it is dismissed by its leader.
    MOB_EVENT_DISMISSED,
    //When it is thrown.
    MOB_EVENT_THROWN,
    //When it lands on the ground.
    MOB_EVENT_LANDED,
    //When it is ordered to release whatever it is holding.
    MOB_EVENT_RELEASE_ORDER,
    //When it is near a task (Pikmin only).
    MOB_EVENT_NEAR_TASK,
    //When it is whistled by a leader.
    MOB_EVENT_WHISTLED,
    //When its spot on the group is now near, and the mob is in the group.
    MOB_EVENT_SPOT_IS_NEAR,
    //When its spot on the group is now far, and the mob is in the group.
    MOB_EVENT_SPOT_IS_FAR,
    //When the group the mob is on started to move (C stick).
    MOB_EVENT_GROUP_MOVE_STARTED,
    //When the group the mob is on stopped moving (C stick).
    MOB_EVENT_GROUP_MOVE_ENDED,
    //When the object the Pikmin was carrying gets delivered.
    MOB_EVENT_FINISHED_CARRYING,
    //When the Pikmin is near an object that can be carried.
    MOB_EVENT_NEAR_CARRIABLE_OBJECT,
    //When the Pikmin is near a tool object.
    MOB_EVENT_NEAR_TOOL,
    //When it has reached its carrying spot on the carriable object.
    //TODO replace with MOB_EVENT_REACHED_DESTINATION?
    MOB_EVENT_REACHED_CARRIABLE_OBJECT,
    //When one of its attack hitboxes touches another mob's normal hitbox.
    MOB_EVENT_HITBOX_TOUCH_N,
    //When one of its normal hitboxes touches another mob's attack hitbox.
    MOB_EVENT_HITBOX_TOUCH_N_A,
    //When a Pikmin was added to the list of Pikmin carrying this mob.
    MOB_EVENT_CARRIER_ADDED,
    //When a Pikmin was removed from the list of Pikmin carrying this mob.
    MOB_EVENT_CARRIER_REMOVED,
    //When the mob needs to begin moving, as it's being carried.
    MOB_EVENT_CARRY_BEGIN_MOVE,
    //When the mob needs to stop moving, as it's no longer being carried.
    MOB_EVENT_CARRY_STOP_MOVE,
    //When the mob being carried becomes stuck.
    MOB_EVENT_CARRY_STUCK,
    //When the mob was sucessfully delivered
    //to its destination after being carried.
    MOB_EVENT_CARRY_DELIVERED,
    //When the focused mob stops being carriable.
    MOB_EVENT_FOCUSED_MOB_UNCARRIABLE,
    //When the mob receives an object that was carried to it.
    MOB_EVENT_RECEIVE_DELIVERY,
    //When the mob touches a drop that it can consume.
    MOB_EVENT_TOUCHED_DROP,
    
    //Events that only leaders can really handle.
    
    //When the leader becomes the one controlled by the player.
    LEADER_EVENT_FOCUSED,
    //When the leader stops being the one controlled by the player.
    LEADER_EVENT_UNFOCUSED,
    //When the leader begins moving.
    LEADER_EVENT_MOVE_START,
    //When the leader stops moving.
    LEADER_EVENT_MOVE_END,
    //When the leader is holding a Pikmin on their hand.
    LEADER_EVENT_HOLDING,
    //When the leader throws the Pikmin on their hand.
    LEADER_EVENT_THROW,
    //When the leader begins whistling.
    LEADER_EVENT_START_WHISTLE,
    //When the leader stops whistling.
    LEADER_EVENT_STOP_WHISTLE,
    //When the leader throws a punch.
    LEADER_EVENT_PUNCH,
    //When the leader dismisses their group.
    LEADER_EVENT_DISMISS,
    //When the leader uses a spray.
    LEADER_EVENT_SPRAY,
    //When the leader lies down.
    LEADER_EVENT_LIE_DOWN,
    //When the leader has to go towards the Pikmin it intends to pluck.
    LEADER_EVENT_GO_PLUCK,
    //When the leader has to go help pluck Pikmin, as an inactive leader.
    LEADER_EVENT_INACTIVE_SEARCH_SEED,
    //When the leader reaches the seed they're meant to pluck.
    LEADER_EVENT_REACHED_SEED,
    //When the leader's pluck is canceled.
    LEADER_EVENT_CANCEL,
    
    N_MOB_EVENTS,
};

class mob_action {
public:
    unsigned char type;
    custom_action_code code;
    bool valid;
    vector<int> vi;
    vector<float> vf;
    vector<string> vs;
    
    bool run(
        mob* m, void* custom_data_1, void* custom_data_2,
        const size_t parent_event
    );
    mob_action(data_node* dn, vector<mob_state*>* states, mob_type* mt);
    mob_action(unsigned char type);
    mob_action(custom_action_code code);
};


class mob_event {
public:
    unsigned char type;
    vector<mob_action*> actions;
    
    void run(mob* m, void* custom_data_1 = NULL, void* custom_data_2 = NULL);
    mob_event(data_node* d, const vector<mob_action*> &a);
    mob_event(
        const unsigned char t,
        const vector<mob_action*> &a = vector<mob_action*>()
    );
};


class mob_state {
public:
    string name;
    size_t id;
    mob_event* events[N_MOB_EVENTS];
    mob_event* get_event(const size_t type);
    
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
    
    mob_event* get_event(const size_t type);
    void run_event(
        const size_t type,
        void* custom_data_1 = NULL, void* custom_data_2 = NULL
    );
    void set_state(
        const size_t new_state, void* info1 = NULL, void* info2 = NULL
    );
    mob_fsm(mob* m = NULL);
};


/* ----------------------------------------------------------------------------
 * The easy fsm creator makes it easy to create mob FSMs in C++ code.
 * For mobs created by the game creator, the state machine is simpler,
 * and written in plain text using a text file. But for the engine and
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
private:
    vector<mob_state*> states;
    mob_state* cur_state;
    mob_event* cur_event;
    void commit_state();
    void commit_event();
    
public:
    void new_state(const string &name, const size_t id);
    void new_event(const unsigned char type);
    void change_state(const string &new_state);
    void run(custom_action_code code);
    vector<mob_state*> finish();
    easy_fsm_creator();
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

size_t fix_states(vector<mob_state*> &states, const string &starting_state);
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action*>* actions
);
void load_script(mob_type* mt, data_node* node, vector<mob_state*>* states);
void unload_script(mob_type* mt);
bool assert_if_actions(const vector<mob_action*> &actions, data_node* dn);


#endif //ifndef MOB_EVENT_INCLUDED
