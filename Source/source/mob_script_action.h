/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob script action classes and
 * related functions.
 */


#ifndef MOB_SCRIPT_ACTION_INCLUDED
#define MOB_SCRIPT_ACTION_INCLUDED

#include "mob_script.h"


//Types of script action.
enum MOB_ACTION_TYPES {
    MOB_ACTION_UNKNOWN,
    MOB_ACTION_ADD_HEALTH,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC,
    MOB_ACTION_CALCULATE,
    MOB_ACTION_DELETE,
    MOB_ACTION_DRAIN_LIQUID,
    MOB_ACTION_ELSE,
    MOB_ACTION_END_IF,
    MOB_ACTION_FINISH_DYING,
    MOB_ACTION_FOCUS,
    MOB_ACTION_GET_CHOMPED,
    MOB_ACTION_GET_INFO,
    MOB_ACTION_GET_FOCUS_VAR,
    MOB_ACTION_GET_RANDOM_DECIMAL,
    MOB_ACTION_GET_RANDOM_INT,
    MOB_ACTION_GOTO,
    MOB_ACTION_IF,
    MOB_ACTION_LABEL,
    MOB_ACTION_MOVE_TO_ABSOLUTE,
    MOB_ACTION_MOVE_TO_RELATIVE,
    MOB_ACTION_MOVE_TO_TARGET,
    MOB_ACTION_ORDER_RELEASE,
    MOB_ACTION_PLAY_SOUND,
    MOB_ACTION_PRINT,
    MOB_ACTION_RECEIVE_STATUS,
    MOB_ACTION_RELEASE,
    MOB_ACTION_REMOVE_STATUS,
    MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
    MOB_ACTION_SEND_MESSAGE_TO_LINKS,
    MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
    MOB_ACTION_SET_ANIMATION,
    MOB_ACTION_SET_FAR_REACH,
    MOB_ACTION_SET_GRAVITY,
    MOB_ACTION_SET_HEALTH,
    MOB_ACTION_SET_HEIGHT,
    MOB_ACTION_SET_HIDING,
    MOB_ACTION_SET_HOLDABLE,
    MOB_ACTION_SET_HUNTABLE,
    MOB_ACTION_SET_LIMB_ANIMATION,
    MOB_ACTION_SET_NEAR_REACH,
    MOB_ACTION_SET_SECTOR_SCROLL,
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
    MOB_ACTION_STOP_VERTICALLY,
    MOB_ACTION_SWALLOW,
    MOB_ACTION_SWALLOW_ALL,
    MOB_ACTION_TELEPORT_TO_ABSOLUTE,
    MOB_ACTION_TELEPORT_TO_RELATIVE,
    MOB_ACTION_TURN_TO_ABSOLUTE,
    MOB_ACTION_TURN_TO_RELATIVE,
    MOB_ACTION_TURN_TO_TARGET,
    
    N_MOB_ACTIONS
};


//Arachnorb plan logic action sub-types.
enum MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPES {
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN,
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN,
};


//Face action sub-types.
enum MOB_ACTION_TURN_TYPES {
    MOB_ACTION_TURN_ARACHNORB_HEAD_LOGIC,
    MOB_ACTION_TURN_FOCUSED_MOB,
    MOB_ACTION_TURN_HOME,
};


//Focus action sub-types.
enum MOB_ACTION_FOCUS_TYPES {
    MOB_ACTION_FOCUS_LINK,
    MOB_ACTION_FOCUS_PARENT,
    MOB_ACTION_FOCUS_TRIGGER,
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


//Get info action info types.
enum MOB_ACTION_GET_INFO_TYPES {
    MOB_ACTION_GET_INFO_BODY_PART,
    MOB_ACTION_GET_INFO_CHOMPED_PIKMIN,
    MOB_ACTION_GET_INFO_DAY_MINUTES,
    MOB_ACTION_GET_INFO_FIELD_PIKMIN,
    MOB_ACTION_GET_INFO_FRAME_SIGNAL,
    MOB_ACTION_GET_INFO_HEALTH,
    MOB_ACTION_GET_INFO_LATCHED_PIKMIN,
    MOB_ACTION_GET_INFO_LATCHED_PIKMIN_WEIGHT,
    MOB_ACTION_GET_INFO_MESSAGE,
    MOB_ACTION_GET_INFO_MESSAGE_SENDER,
    MOB_ACTION_GET_INFO_MOB_CATEGORY,
    MOB_ACTION_GET_INFO_MOB_TYPE,
    MOB_ACTION_GET_INFO_OTHER_BODY_PART,
    MOB_ACTION_GET_INFO_WEIGHT,
};


//Moving action sub-types.
enum MOB_ACTION_MOVE_TYPES {
    MOB_ACTION_MOVE_AWAY_FROM_FOCUSED_MOB,
    MOB_ACTION_MOVE_FOCUSED_MOB,
    MOB_ACTION_MOVE_FOCUSED_MOB_POS,
    MOB_ACTION_MOVE_HOME,
    MOB_ACTION_MOVE_ARACHNORB_FOOT_LOGIC,
    MOB_ACTION_MOVE_LINKED_MOB_AVERAGE,
};


enum MOB_ACTION_SET_ANIMATION_OPTIONS {
    MOB_ACTION_SET_ANIMATION_NO_RESTART,
};


//Set var action sub-types.
enum MOB_ACTION_SET_VAR_TYPES {
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


//Types of variables that a parameter can use.
enum MOB_ACTION_PARAM_TYPE {
    //Signed integer.
    MOB_ACTION_PARAM_INT,
    //Float.
    MOB_ACTION_PARAM_FLOAT,
    //Boolean.
    MOB_ACTION_PARAM_BOOL,
    //STL string.
    MOB_ACTION_PARAM_STRING,
    //STL string that gets turned into an int.
    MOB_ACTION_PARAM_ENUM,
};


struct mob_action_param {
    //Name of the parameter.
    string name;
    //Type of variable it's meant to hold.
    MOB_ACTION_PARAM_TYPE type;
    //If true, it must be a constant value. Else, it can also be a var.
    bool force_const;
    //If true, this is an array of them (minimum amount 0).
    bool is_extras;
    
    mob_action_param(
        const string &name,
        const MOB_ACTION_PARAM_TYPE type,
        const bool force_const,
        const bool is_extras
    );
};


struct mob_action_run_data {
    mob* m;
    mob_action_call* call;
    vector<string> args;
    void* custom_data_1;
    void* custom_data_2;
    bool return_value;
    mob_action_run_data(mob* m, mob_action_call* call);
};


typedef void (mob_action_code)(mob_action_run_data &data);
typedef bool (mob_action_load_code)(mob_action_call &call);


struct mob_action {
    unsigned char type;
    string name;
    mob_action_code* code;
    mob_action_load_code* extra_load_logic;
    vector<mob_action_param> parameters;
    
    mob_action();
};


struct mob_action_call {
    mob_action* action;
    custom_action_code code;
    
    vector<string> args;
    vector<bool> arg_is_var;
    
    string custom_error;
    MOB_EV_TYPES parent_event;
    mob_type* mt;
    
    bool load_from_data_node(data_node* dn, mob_type* mt);
    bool run(mob* m, void* custom_data_1, void* custom_data_2);
    
    mob_action_call(MOB_ACTION_TYPES type = MOB_ACTION_UNKNOWN);
    mob_action_call(custom_action_code code);
};


namespace mob_action_runners {
void add_health(mob_action_run_data &data);
void add_health(mob_action_run_data &data);
void arachnorb_plan_logic(mob_action_run_data &data);
void calculate(mob_action_run_data &data);
void delete_function(mob_action_run_data &data);
void drain_liquid(mob_action_run_data &data);
void finish_dying(mob_action_run_data &data);
void focus(mob_action_run_data &data);
void get_chomped(mob_action_run_data &data);
void get_info(mob_action_run_data &data);
void get_focus_var(mob_action_run_data &data);
void get_random_decimal(mob_action_run_data &data);
void get_random_int(mob_action_run_data &data);
void goto_function(mob_action_run_data &data);
void if_function(mob_action_run_data &data);
void move_to_absolute(mob_action_run_data &data);
void move_to_relative(mob_action_run_data &data);
void move_to_target(mob_action_run_data &data);
void order_release(mob_action_run_data &data);
void play_sound(mob_action_run_data &data);
void print(mob_action_run_data &data);
void receive_status(mob_action_run_data &data);
void release(mob_action_run_data &data);
void remove_status(mob_action_run_data &data);
void send_message_to_focus(mob_action_run_data &data);
void send_message_to_links(mob_action_run_data &data);
void send_message_to_nearby(mob_action_run_data &data);
void set_animation(mob_action_run_data &data);
void set_far_reach(mob_action_run_data &data);
void set_gravity(mob_action_run_data &data);
void set_health(mob_action_run_data &data);
void set_height(mob_action_run_data &data);
void set_hiding(mob_action_run_data &data);
void set_holdable(mob_action_run_data &data);
void set_huntable(mob_action_run_data &data);
void set_limb_animation(mob_action_run_data &data);
void set_near_reach(mob_action_run_data &data);
void set_state(mob_action_run_data &data);
void set_sector_scroll(mob_action_run_data &data);
void set_tangible(mob_action_run_data &data);
void set_team(mob_action_run_data &data);
void set_timer(mob_action_run_data &data);
void set_var(mob_action_run_data &data);
void show_message_from_var(mob_action_run_data &data);
void spawn(mob_action_run_data &data);
void stabilize_z(mob_action_run_data &data);
void start_chomping(mob_action_run_data &data);
void start_dying(mob_action_run_data &data);
void start_height_effect(mob_action_run_data &data);
void start_particles(mob_action_run_data &data);
void stop(mob_action_run_data &data);
void stop_chomping(mob_action_run_data &data);
void stop_height_effect(mob_action_run_data &data);
void stop_particles(mob_action_run_data &data);
void stop_vertically(mob_action_run_data &data);
void swallow(mob_action_run_data &data);
void swallow_all(mob_action_run_data &data);
void teleport_to_absolute(mob_action_run_data &data);
void teleport_to_relative(mob_action_run_data &data);
void turn_to_absolute(mob_action_run_data &data);
void turn_to_relative(mob_action_run_data &data);
void turn_to_target(mob_action_run_data &data);
};


namespace mob_action_loaders {
bool arachnorb_plan_logic(mob_action_call &call);
bool calculate(mob_action_call &call);
bool focus(mob_action_call &call);
bool get_info(mob_action_call &call);
bool if_function(mob_action_call &call);
bool move_to_target(mob_action_call &call);
bool receive_status(mob_action_call &call);
bool remove_status(mob_action_call &call);
bool set_animation(mob_action_call &call);
bool set_far_reach(mob_action_call &call);
bool set_holdable(mob_action_call &call);
bool set_near_reach(mob_action_call &call);
bool set_team(mob_action_call &call);
bool spawn(mob_action_call &call);
bool stabilize_z(mob_action_call &call);
bool start_chomping(mob_action_call &call);
bool start_particles(mob_action_call &call);
bool turn_to_target(mob_action_call &call);

void report_enum_error(mob_action_call &call, const size_t arg_nr);
};


bool assert_actions(
    const vector<mob_action_call*> &actions, data_node* dn
);
void load_init_actions(
    mob_type* mt, data_node* node, vector<mob_action_call*>* actions
);


#endif //ifndef MOB_SCRIPT_ACTION_INCLUDED
