/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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
class hitbox_instance;

typedef void (*custom_action_code)(mob* m, void* info1, void* info2);


//Types of script action.
enum MOB_ACTION_TYPES {
    MOB_ACTION_UNKNOWN,
    MOB_ACTION_CHOMP_HITBOXES,
    MOB_ACTION_EAT,
    MOB_ACTION_FOCUS,
    MOB_ACTION_IF,
    MOB_ACTION_MOVE,
    MOB_ACTION_SET_SPEED,
    MOB_ACTION_SET_GRAVITY,
    MOB_ACTION_PLAY_SOUND,
    MOB_ACTION_SET_VAR,
    MOB_ACTION_SET_ANIMATION,
    MOB_ACTION_SPECIAL_FUNCTION,
    MOB_ACTION_SPAWN_PROJECTILE,
    MOB_ACTION_SPAWN_PARTICLE,
    MOB_ACTION_SET_TIMER,
    MOB_ACTION_SET_HEALTH,
    MOB_ACTION_SET_STATE,
    MOB_ACTION_TURN,
    MOB_ACTION_WAIT,
};

//Types of script events.
enum MOB_EVENT_TYPES {
    //"Special" events.
    MOB_EVENT_UNKNOWN,
    MOB_EVENT_ON_ENTER, //When the state is entered.
    MOB_EVENT_ON_LEAVE, //When the state is left.
    MOB_EVENT_ON_TICK,  //When the game ticks a frame.
    
    //Script file stuff.
    MOB_EVENT_ANIMATION_END,       //When the current animation ends.
    MOB_EVENT_BIG_DAMAGE,          //When it reaches a certain health %.
    MOB_EVENT_DAMAGE,              //When it is damaged.
    MOB_EVENT_DEATH,               //When it dies.
    MOB_EVENT_ENTERED_HAZARD,      //When it enters a hazard sector.
    MOB_EVENT_FAR_FROM_HOME,       //When the mob is far away from its home.
    MOB_EVENT_FACING_OPPONENT,     //When it faces and is near its opponent.
    MOB_EVENT_FACING_OBJECT,       //When it faces and is near its target object.
    MOB_EVENT_FOCUSED_MOB_DIED,    //When the mob it was focused on died.
    MOB_EVENT_LEFT_HAZARD,         //When it leaves a hazard sector.
    MOB_EVENT_LOST_FOCUSED_MOB,    //When it can no longer see the focused mob.
    MOB_EVENT_MOUTH_OCCUPIED,      //When its mouth has a Pikmin in it.
    MOB_EVENT_MOUTH_EMPTY,         //When its mouth is empty.
    MOB_EVENT_NEAR_OBJECT,         //When it gets near its target object.
    MOB_EVENT_NEAR_OPPONENT,       //When it gets near its opponent.
    MOB_EVENT_PIKMIN_LANDED,       //When a Pikmin lands on it.
    MOB_EVENT_PIKMIN_LATCHED,      //When a Pikmin latches on to it.
    MOB_EVENT_PIKMIN_TOUCHED,      //When a Pikmin touches it.
    MOB_EVENT_REACHED_DESTINATION, //When it reaches its destination.
    MOB_EVENT_REVIVED,             //When it revives from being dead.
    MOB_EVENT_SEEN_OBJECT,         //When it sees an object.
    MOB_EVENT_SEEN_OPPONENT,       //When it sees an opponent.
    MOB_EVENT_TOUCHED_LEADER,      //When it gets touched by a leader.
    MOB_EVENT_TOUCHED_OBJECT,      //When it gets touched by an object.
    MOB_EVENT_TOUCHED_OPPONENT,    //When it gets touched by an opponent.
    MOB_EVENT_TIMER,               //When its timer ticks.
    MOB_EVENT_TOUCHED_WALL,        //When it touches a wall.
    
    //More internal script stuff.
    MOB_EVENT_PLUCKED,                  //When it is plucked off the ground (Pikmin only).
    MOB_EVENT_GRABBED_BY_FRIEND,        //When it is grabbed by a friend.
    MOB_EVENT_DISMISSED,                //When it is dismissed by its leader.
    MOB_EVENT_THROWN,                   //When it is thrown.
    MOB_EVENT_RELEASED,                 //When it is released from the leader's/enemy's grasp (e.g. swap for another Pikmin while holding)
    MOB_EVENT_LANDED,                   //When it lands on the ground.
    MOB_EVENT_NEAR_TASK,                //When it is near a task (Pikmin only).
    MOB_EVENT_WHISTLED,                 //When it is whistled by a leader.
    MOB_EVENT_SPOT_IS_NEAR,             //When its spot on the group is now near, and the mob is in the group.
    MOB_EVENT_SPOT_IS_FAR,              //When its spot on the group is now far, and the mob is in the group.
    MOB_EVENT_GROUP_MOVE_STARTED,       //When the group the mob is on started to move (C stick).
    MOB_EVENT_GROUP_MOVE_ENDED,         //When the group the mob is on stopped moving (C stick).
    MOB_EVENT_FINISHED_CARRYING,        //When the object it was carrying gets delivered.
    MOB_EVENT_NEAR_CARRIABLE_OBJECT,    //When it is near an object that can be carried.
    MOB_EVENT_HITBOX_TOUCH_A_N,         //When one of its attack hitboxes touches another mob's normal hitbox.
    MOB_EVENT_HITBOX_TOUCH_N_A,         //When one of its normal hitboxes touches another mob's attack hitbox.
    MOB_EVENT_HITBOX_TOUCH_EAT,         //When one of its normal hitboxes touches another mob's eating hitbox.
    MOB_EVENT_REACHED_CARRIABLE_OBJECT, //When it has reached its carrying spot on the carriable object.
    MOB_EVENT_FOCUSED_MOB_UNCARRIABLE,  //When the focused mob became uncarriable.
    MOB_EVENT_EATEN,                    //When the Pikmin is eaten from being grabbed by an enemy's jaws.
    
    //Events that only leaders can really handle.
    LEADER_EVENT_FOCUSED,        //When the leader becomes the one controlled by the player.
    LEADER_EVENT_UNFOCUSED,      //When the leader stops being the one controlled by the player.
    LEADER_EVENT_MOVE_START,     //When the leader begins moving.
    LEADER_EVENT_MOVE_END,       //When the leader stops moving.
    LEADER_EVENT_HOLDING,        //When the leader is holding a Pikmin on their hand.
    LEADER_EVENT_THROW,          //When the leader throws the Pikmin on their hand.
    LEADER_EVENT_RELEASE,        //When the leader releases the Pikmin on their hand.
    LEADER_EVENT_START_WHISTLE,  //When the leader begins whistling.
    LEADER_EVENT_STOP_WHISTLE,   //When the leader stops whistling.
    LEADER_EVENT_DISMISS,        //When the leader dismisses their squad.
    LEADER_EVENT_SPRAY,          //When the leader uses a spray.
    LEADER_EVENT_LIE_DOWN,       //When the leader lies down.
    LEADER_EVENT_GO_PLUCK,       //When the leader has to go towards the Pikmin it intends to pluck.
    LEADER_EVENT_INACTIVE_SEARCH_SEED, //When the leader has to go help pluck Pikmin, as an inactive leader.
    LEADER_EVENT_REACHED_SEED,   //When the leader reaches the seed they're meant to pluck.
    LEADER_EVENT_CANCEL,   //When the leader's pluck is canceled.
    
    N_MOB_EVENTS,
};

//Eating action sub-types.
enum MOB_ACTION_EAT_TYPES {
    MOB_ACTION_EAT_ALL,
    MOB_ACTION_EAT_NUMBER,
};

//Moving action sub-types.
enum MOB_ACTION_MOVE_TYPES {
    MOB_ACTION_MOVE_FOCUSED_MOB,
    MOB_ACTION_MOVE_HOME,
    MOB_ACTION_MOVE_STOP,
    MOB_ACTION_MOVE_COORDS,
    MOB_ACTION_MOVE_REL_COORDS,
};

//Set health action sub-types.
enum MOB_ACTION_SET_HEALTH_TYPES {
    MOB_ACTION_SET_HEALTH_ABSOLUTE,
    MOB_ACTION_SET_HEALTH_RELATIVE,
};

//Special function action sub-types.
enum MOB_ACTION_SPECIAL_FUNCTION_TYPES {
    MOB_ACTION_SPECIAL_FUNCTION_DIE_START,
    MOB_ACTION_SPECIAL_FUNCTION_DIE_END,
    MOB_ACTION_SPECIAL_FUNCTION_LOOP,
};

//Waiting action sub-types.
enum MOB_ACTION_WAIT_TYPES {
    MOB_ACTION_WAIT_ANIMATION,
    MOB_ACTION_WAIT_TIME,
};


class mob_action {
public:
    MOB_ACTION_TYPES type;
    unsigned char sub_type;
    custom_action_code code;
    bool valid;
    vector<int> vi;
    vector<float> vf;
    vector<string> vs;
    
    void run(mob* m, size_t* action_nr, void* custom_data_1, void* custom_data_2);
    mob_action(data_node* dn, vector<mob_state*>* states, mob_type* mt);
    mob_action(MOB_ACTION_TYPES type, unsigned char sub_type = 0);
    mob_action(custom_action_code code);
};


class mob_event {
public:
    MOB_EVENT_TYPES type;
    vector<mob_action*> actions;
    
    void run(mob* m, void* custom_data_1 = NULL, void* custom_data_2 = NULL);
    mob_event(data_node* d, vector<mob_action*> a);
    mob_event(const MOB_EVENT_TYPES t, vector<mob_action*> a = vector<mob_action*>());
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
    vector<size_t> pre_named_conversions; //Conversion between pre-named states and in-file states.
    
    mob_event* get_event(const size_t type);
    void run_event(const size_t type, void* custom_data_1 = NULL, void* custom_data_2 = NULL);
    void set_state(const size_t new_state, void* info1 = NULL, void* info2 = NULL);
    mob_fsm(mob* m = NULL);
};


/* ----------------------------------------------------------------------------
 * The easy fsm creator makes it easy to create mob FSMs in C++ code.
 * For mobs created by the game-dev, the state machine is simpler,
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
    void new_event(const MOB_EVENT_TYPES type);
    void change_state(const string &new_state);
    void run_function(custom_action_code code);
    vector<mob_state*> finish();
    easy_fsm_creator();
};

struct hitbox_touch_info {
    mob* mob2; //Mob that touched our mob.
    hitbox_instance* hi1; //Hitbox of our mob that got touched.
    hitbox_instance* hi2; //Hitbox of the other mob.
    hitbox_touch_info(mob* mob2 = NULL, hitbox_instance* hi1 = NULL, hitbox_instance* hi2 = NULL);
};

vector<mob_state*> load_script(mob_type* mt, data_node* node);

size_t fix_states(vector<mob_state*> &states, const string &starting_state);

#endif //ifndef MOB_EVENT_INCLUDED
