/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the all script action classes and related functions.
 */


#pragma once

#include "../../util/general_utils.h"


#pragma region Constants


class Mob;
class MobType;
class ScriptActionDef;
class ScriptVM;


//Types of script action.
enum SCRIPT_ACTION {

    //Unknown.
    SCRIPT_ACTION_UNKNOWN,
    
    //Get the absolute value of a float number.
    MOB_ACTION_ABSOLUTE_NUMBER,
    
    //Add health.
    MOB_ACTION_ADD_HEALTH,
    
    //Plan something. Used for arachnorbs.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC,
    
    //Perform a numeric calculation.
    MOB_ACTION_CALCULATE,
    
    //Ceil a float number.
    MOB_ACTION_CEIL_NUMBER,
    
    //Delete mob.
    MOB_ACTION_DELETE,
    
    //Drain liquid.
    MOB_ACTION_DRAIN_LIQUID,
    
    //Ease a float number.
    MOB_ACTION_EASE_NUMBER,
    
    //Conditional "else" marker.
    MOB_ACTION_ELSE,
    
    //Conditional "else if" marker.
    MOB_ACTION_ELSE_IF,
    
    //Conditional "end if" marker.
    MOB_ACTION_END_IF,
    
    //Finish the death procedure.
    MOB_ACTION_FINISH_DYING,
    
    //Floor a float number.
    MOB_ACTION_FLOOR_NUMBER,
    
    //Focus on another mob.
    MOB_ACTION_FOCUS,
    
    //Follow a mob as its leader.
    MOB_ACTION_FOLLOW_MOB_AS_LEADER,
    
    //Follow a path randomly.
    MOB_ACTION_FOLLOW_PATH_RANDOMLY,
    
    //Follow a path towards an absolute position.
    MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
    
    //Get angle between two sets of coordinates.
    MOB_ACTION_GET_ANGLE,
    
    //Get the clockwise difference between two angles.
    MOB_ACTION_GET_ANGLE_CW_DIFF,
    
    //Get the smallest difference between two angles.
    MOB_ACTION_GET_ANGLE_SMALLEST_DIFF,
    
    //Get information about the area.
    MOB_ACTION_GET_AREA_INFO,
    
    //Get chomped by another mob.
    MOB_ACTION_GET_CHOMPED,
    
    //Get coordinates from a given angle.
    MOB_ACTION_GET_COORDINATES_FROM_ANGLE,
    
    //Get distance between two sets of coordinates.
    MOB_ACTION_GET_DISTANCE,
    
    //Get information about an event.
    MOB_ACTION_GET_EVENT_INFO,
    
    //Get the Z of the floor at a set of coordinates.
    MOB_ACTION_GET_FLOOR_Z,
    
    //Get a script variable's value from the focused mob.
    MOB_ACTION_GET_FOCUS_VAR,
    
    //Get information about a mob.
    MOB_ACTION_GET_MOB_INFO,
    
    //Get a random float number.
    MOB_ACTION_GET_RANDOM_FLOAT,
    
    //Get a random integer number.
    MOB_ACTION_GET_RANDOM_INT,
    
    //Go to a different part of the script.
    MOB_ACTION_GOTO,
    
    //Hold focused mob.
    MOB_ACTION_HOLD_FOCUS,
    
    //Conditional "if" condition.
    MOB_ACTION_IF,
    
    //Interpolate a float number.
    MOB_ACTION_INTERPOLATE_NUMBER,
    
    //Label for use with "goto".
    MOB_ACTION_LABEL,
    
    //Create a link with the focused mob.
    MOB_ACTION_LINK_WITH_FOCUS,
    
    //Load focused mob from focused mobs memory.
    MOB_ACTION_LOAD_FOCUS_MEMORY,
    
    //Move to absolute coordinates.
    MOB_ACTION_MOVE_TO_ABSOLUTE,
    
    //Move to relative coordinates.
    MOB_ACTION_MOVE_TO_RELATIVE,
    
    //Move to a target.
    MOB_ACTION_MOVE_TO_TARGET,
    
    //Order to be released.
    MOB_ACTION_ORDER_RELEASE,
    
    //Play a sound.
    MOB_ACTION_PLAY_SOUND,
    
    //Print some content debug text.
    MOB_ACTION_PRINT,
    
    //Receive a status effect.
    MOB_ACTION_RECEIVE_STATUS,
    
    //Release held mob.
    MOB_ACTION_RELEASE,
    
    //Release mobs that are stored inside.
    MOB_ACTION_RELEASE_STORED_MOBS,
    
    //Remove a status effect.
    MOB_ACTION_REMOVE_STATUS,
    
    //Round a float number.
    MOB_ACTION_ROUND_NUMBER,
    
    //Save focused mob into focused mobs memory.
    MOB_ACTION_SAVE_FOCUS_MEMORY,
    
    //Send a message to the focused mob.
    MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
    
    //Send a message to all linked mobs.
    MOB_ACTION_SEND_MESSAGE_TO_LINKS,
    
    //Send a message to nearby mobs.
    MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
    
    //Set animation.
    MOB_ACTION_SET_ANIMATION,
    
    //Set whether it can block paths.
    MOB_ACTION_SET_CAN_BLOCK_PATHS,
    
    //Set its far reach.
    MOB_ACTION_SET_FAR_REACH,
    
    //Set whether it is flying.
    MOB_ACTION_SET_FLYING,
    
    //Set a script variable's value of the focused mob.
    MOB_ACTION_SET_FOCUS_VAR,
    
    //Set its gravity.
    MOB_ACTION_SET_GRAVITY,
    
    //Set its health.
    MOB_ACTION_SET_HEALTH,
    
    //Set its height.
    MOB_ACTION_SET_HEIGHT,
    
    //Set whether it is hiding.
    MOB_ACTION_SET_HIDING,
    
    //Set whether it is holdable.
    MOB_ACTION_SET_HOLDABLE,
    
    //Set whether it is huntable.
    MOB_ACTION_SET_HUNTABLE,
    
    //Set limb animation.
    MOB_ACTION_SET_LIMB_ANIMATION,
    
    //Set its near reach.
    MOB_ACTION_SET_NEAR_REACH,
    
    //Set its radius.
    MOB_ACTION_SET_RADIUS,
    
    //Set scrolling of its sector.
    MOB_ACTION_SET_SECTOR_SCROLL,
    
    //Set whether its shadow is visible.
    MOB_ACTION_SET_SHADOW_VISIBILITY,
    
    //Set state.
    MOB_ACTION_SET_STATE,
    
    //Set whether it is tangible.
    MOB_ACTION_SET_TANGIBLE,
    
    //Set team.
    MOB_ACTION_SET_TEAM,
    
    //Set a timer.
    MOB_ACTION_SET_TIMER,
    
    //Set a script variable.
    MOB_ACTION_SET_VAR,
    
    //Shake the camera.
    MOB_ACTION_SHAKE_CAMERA,
    
    //Show a message that is inside a script variable.
    MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
    
    //Spawn something.
    MOB_ACTION_SPAWN,
    
    //Square root a number.
    MOB_ACTION_SQUARE_ROOT_NUMBER,
    
    //Stabilize its Z coordinate.
    MOB_ACTION_STABILIZE_Z,
    
    //Start chomping.
    MOB_ACTION_START_CHOMPING,
    
    //Start the death procedure.
    MOB_ACTION_START_DYING,
    
    //Start the height effect.
    MOB_ACTION_START_HEIGHT_EFFECT,
    
    //Start some particle generator.
    MOB_ACTION_START_PARTICLES,
    
    //Stop moving.
    MOB_ACTION_STOP,
    
    //Stop chomping.
    MOB_ACTION_STOP_CHOMPING,
    
    //Stop the height effect.
    MOB_ACTION_STOP_HEIGHT_EFFECT,
    
    //Stop some particle generator.
    MOB_ACTION_STOP_PARTICLES,
    
    //Stop a playing sound.
    MOB_ACTION_STOP_SOUND,
    
    //Stop vertical movement.
    MOB_ACTION_STOP_VERTICALLY,
    
    //Store the focused mob inside.
    MOB_ACTION_STORE_FOCUS_INSIDE,
    
    //Swallow some chomped Pikmin.
    MOB_ACTION_SWALLOW,
    
    //Swallow all chomped Pikmin.
    MOB_ACTION_SWALLOW_ALL,
    
    //Teleport to absolute coordinates.
    MOB_ACTION_TELEPORT_TO_ABSOLUTE,
    
    //Teleport to relative coordinates.
    MOB_ACTION_TELEPORT_TO_RELATIVE,
    
    //Throw focused mob.
    MOB_ACTION_THROW_FOCUS,
    
    //Turn towards an absolute angle.
    MOB_ACTION_TURN_TO_ABSOLUTE,
    
    //Turn towards a relative angle.
    MOB_ACTION_TURN_TO_RELATIVE,
    
    //Turn towards a target.
    MOB_ACTION_TURN_TO_TARGET,
    
    //Total amount of script actions.
    N_SCRIPT_ACTIONS,
    
};


//Arachnorb plan logic action sub-types.
enum MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE {

    //Plan to go home.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME,
    
    //Plan to move forward.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD,
    
    //Plan a clockwise turn.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN,
    
    //Plan a counterclockwise turn.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN,
    
};


//Mob action arachnorb plan logic type enum naming (internal names).
buildEnumNames(
    mobActionArachnorbPlanLogicTypeINames, MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE
)({
    { MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME, "home" },
    { MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD, "forward" },
    { MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN, "cw_turn" },
    { MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN, "ccw_turn" },
});


//Turn action sub-types.
enum MOB_ACTION_TURN_TYPE {

    //Logic for an arachnorb's head to turn.
    MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC,
    
    //Turn towards the focused mob.
    MOB_ACTION_TURN_TYPE_FOCUSED_MOB,
    
    //Turn towards home.
    MOB_ACTION_TURN_TYPE_HOME,
    
};


//Mob action turn type enum naming (internal names).
buildEnumNames(
    mobActionTurnTypeINames, MOB_ACTION_TURN_TYPE
)({
    { MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC, "arachnorb_head_logic" },
    { MOB_ACTION_TURN_TYPE_FOCUSED_MOB, "focused_mob" },
    { MOB_ACTION_TURN_TYPE_HOME, "home" },
});


//If action operator types.
enum MOB_ACTION_IF_OP {

    //Check if two values are equal.
    MOB_ACTION_IF_OP_EQUAL,
    
    //Check if two values are different.
    MOB_ACTION_IF_OP_NOT,
    
    //Check if a value is less than another value.
    MOB_ACTION_IF_OP_LESS,
    
    //Check if a value is more than another value.
    MOB_ACTION_IF_OP_MORE,
    
    //Check if a value is less than another value, or equal.
    MOB_ACTION_IF_OP_LESS_E,
    
    //Check if a value is more than another value, or equal.
    MOB_ACTION_IF_OP_MORE_E,
    
};


//Mob action if operator enum naming (internal names).
buildEnumNames(mobActionIfOpINames, MOB_ACTION_IF_OP)({
    { MOB_ACTION_IF_OP_EQUAL, "=" },
    { MOB_ACTION_IF_OP_NOT, "!=" },
    { MOB_ACTION_IF_OP_LESS, "<" },
    { MOB_ACTION_IF_OP_MORE, ">" },
    { MOB_ACTION_IF_OP_LESS_E, "<=" },
    { MOB_ACTION_IF_OP_MORE_E, ">=" },
});


//Target types for actions that target mobs.
enum MOB_ACTION_MOB_TARGET_TYPE {

    //Targets the mob the script belongs to.
    MOB_ACTION_MOB_TARGET_TYPE_SELF,
    
    //Targets the currently focused mob, if any.
    MOB_ACTION_MOB_TARGET_TYPE_FOCUS,
    
    //Targets the mob that triggered the event, if any.
    MOB_ACTION_MOB_TARGET_TYPE_TRIGGER,
    
    //Targets the first linked object, if any.
    MOB_ACTION_MOB_TARGET_TYPE_LINK,
    
    //Targets the parent mob, if any.
    MOB_ACTION_MOB_TARGET_TYPE_PARENT,
    
};


//Mob action mob target type enum naming (internal names).
buildEnumNames(mobActionMobTargetTypeINames, MOB_ACTION_MOB_TARGET_TYPE)({
    { MOB_ACTION_MOB_TARGET_TYPE_SELF, "self" },
    { MOB_ACTION_MOB_TARGET_TYPE_FOCUS, "focus" },
    { MOB_ACTION_MOB_TARGET_TYPE_TRIGGER, "trigger" },
    { MOB_ACTION_MOB_TARGET_TYPE_LINK, "link" },
    { MOB_ACTION_MOB_TARGET_TYPE_PARENT, "parent" },
});


//Get area info action info types.
enum MOB_ACTION_GET_AREA_INFO_TYPE {

    //Get time of day, in minutes.
    MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES,
    
    //Get number of Pikmin on the field.
    MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN,
    
};


//Mob action get area info type enum naming (internal names).
buildEnumNames(mobActionGetAreaInfoTypeINames, MOB_ACTION_GET_AREA_INFO_TYPE)({
    { MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES, "day_minutes" },
    { MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN, "field_pikmin" },
});


//Get event info action info types.
enum MOB_ACTION_GET_EV_INFO_TYPE {

    //Get body part that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART,
    
    //Get frame signal that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL,
    
    //Get name of hazard that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_HAZARD,
    
    //Get the name of the input that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME,
    
    //Get the value of the input that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE,
    
    //Get message that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE,
    
    //Get the other body part that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART,
    
};


//Mob action get event info type enum naming (internal names).
buildEnumNames(mobActionGetEvInfoTypeINames, MOB_ACTION_GET_EV_INFO_TYPE)({
    { MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART, "body_part" },
    { MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL, "frame_signal" },
    { MOB_ACTION_GET_EV_INFO_TYPE_HAZARD, "hazard" },
    { MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME, "input_name" },
    { MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE, "input_value" },
    { MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE, "message" },
    { MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART, "other_body_part" },
});


//Get mob info action info types.
enum MOB_ACTION_GET_MOB_INFO_TYPE {

    //Get angle.
    MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE,
    
    //Get amount of chomped Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN,
    
    //Get distance towards focused mob.
    MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE,
    
    //Get total power on the group task.
    MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER,
    
    //Get health.
    MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH,
    
    //Get health ratio.
    MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO,
    
    //Get the numerical ID.
    MOB_ACTION_GET_MOB_INFO_TYPE_ID,
    
    //Get amount of latched Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN,
    
    //Get total weight of latched Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT,
    
    //Get category of mob that triggered the event.
    MOB_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY,
    
    //Get type of mob that triggered the event.
    MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE,
    
    //Get name of current state.
    MOB_ACTION_GET_MOB_INFO_TYPE_STATE,
    
    //Get current weight on top of it.
    MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT,
    
    //Get X.
    MOB_ACTION_GET_MOB_INFO_TYPE_X,
    
    //Get Y.
    MOB_ACTION_GET_MOB_INFO_TYPE_Y,
    
    //Get Z.
    MOB_ACTION_GET_MOB_INFO_TYPE_Z,
    
};


//Mob action get mob info type enum naming (internal names).
buildEnumNames(mobActionGetMobInfoTypeINames, MOB_ACTION_GET_MOB_INFO_TYPE)({
    { MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE, "angle" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN, "chomped_pikmin" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE, "focus_distance" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER, "group_task_power" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH, "health" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO, "health_ratio" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_ID, "id" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN, "latched_pikmin" },
    {
        MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT,
        "latched_pikmin_weight"
    },
    { MOB_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY, "mob_category" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE, "mob_type" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_STATE, "state" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT, "weight" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_X, "x" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_Y, "y" },
    { MOB_ACTION_GET_MOB_INFO_TYPE_Z, "z" },
});


//Moving action sub-types.
enum MOB_ACTION_MOVE_TYPE {

    //Move away from focused mob.
    MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS,
    
    //Move towards focused mob.
    MOB_ACTION_MOVE_TYPE_FOCUS,
    
    //Move towards the position the focus mob is on right now.
    MOB_ACTION_MOVE_TYPE_FOCUS_POS,
    
    //Move towards home.
    MOB_ACTION_MOVE_TYPE_HOME,
    
    //Follow arachnorb foot movement logic.
    MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC,
    
    //Move towards the average spot of the linked mobs.
    MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE,
    
};


//Mob action move type enum naming (internal names).
buildEnumNames(mobActionMoveTypeINames, MOB_ACTION_MOVE_TYPE)({
    { MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS, "away_from_focused_mob" },
    { MOB_ACTION_MOVE_TYPE_FOCUS, "focused_mob" },
    { MOB_ACTION_MOVE_TYPE_FOCUS_POS, "focus_mob_position" },
    { MOB_ACTION_MOVE_TYPE_HOME, "home" },
    { MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC, "arachnorb_foot_logic" },
    { MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE, "linked_mob_average" },
});


//Calculate action sub-types.
enum MOB_ACTION_CALCULATE_TYPE {

    //Sum two numbers.
    MOB_ACTION_CALCULATE_TYPE_SUM,
    
    //Subtract one number from another.
    MOB_ACTION_CALCULATE_TYPE_SUBTRACT,
    
    //Multiply two numbers.
    MOB_ACTION_CALCULATE_TYPE_MULTIPLY,
    
    //Divide one number by another.
    MOB_ACTION_CALCULATE_TYPE_DIVIDE,
    
    //Get the modulo of a number with another.
    MOB_ACTION_CALCULATE_TYPE_MODULO,
    
    //Raise one number to the power of another.
    MOB_ACTION_CALCULATE_TYPE_POWER,
    
};


//Mob action calculate type enum naming (internal names).
buildEnumNames(mobActionCalculateTypeINames, MOB_ACTION_CALCULATE_TYPE)({
    { MOB_ACTION_CALCULATE_TYPE_SUM, "+" },
    { MOB_ACTION_CALCULATE_TYPE_SUBTRACT, "-" },
    { MOB_ACTION_CALCULATE_TYPE_MULTIPLY, "*" },
    { MOB_ACTION_CALCULATE_TYPE_DIVIDE, "/" },
    { MOB_ACTION_CALCULATE_TYPE_MODULO, "%" },
    { MOB_ACTION_CALCULATE_TYPE_POWER, "^" },
});


//Stabilize Z action sub-types.
enum MOB_ACTION_STABILIZE_Z_TYPE {

    //Stabilize towards highest Z.
    MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST,
    
    //Stabilize towards lowest z.
    MOB_ACTION_STABILIZE_Z_TYPE_LOWEST,
    
};


//Mob action stabilize Z type enum naming (internal names).
buildEnumNames(mobActionStabilizeZTypeINames, MOB_ACTION_STABILIZE_Z_TYPE)({
    { MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST, "highest" },
    { MOB_ACTION_STABILIZE_Z_TYPE_LOWEST, "lowest" },
});


//Types of variables that a parameter can use.
enum SCRIPT_ACTION_PARAM {

    //Signed integer.
    SCRIPT_ACTION_PARAM_INT,
    
    //Float.
    SCRIPT_ACTION_PARAM_FLOAT,
    
    //Boolean.
    SCRIPT_ACTION_PARAM_BOOL,
    
    //STL string.
    SCRIPT_ACTION_PARAM_STRING,
    
    //STL string that gets turned into an int.
    SCRIPT_ACTION_PARAM_ENUM,
    
};


#pragma endregion
#pragma region Classes


/**
 * @brief Info about a parameter that a script action type
 * can receive.
 */
struct ScriptActionTypeParam {

    //--- Public members ---
    
    //Name of the parameter.
    string name;
    
    //Type of variable it's meant to hold.
    SCRIPT_ACTION_PARAM type = SCRIPT_ACTION_PARAM_STRING;
    
    //If true, it must be a constant value. Else, it can also be a var.
    bool forceConst = false;
    
    //If true, this is an array of them (minimum amount 0).
    bool isExtras = false;
    
    
    //--- Public function declarations ---
    
    ScriptActionTypeParam(
        const string& name, const SCRIPT_ACTION_PARAM type,
        bool forceConst, bool isExtras
    );
    
};


/**
 * @brief Info about how to run a specific instance of a script action.
 */
struct ScriptActionInstRunData {

    //--- Public members ---
    
    //Script VM under which the action will be run.
    ScriptVM* scriptVM = nullptr;
    
    //Action definition information.
    ScriptActionDef* actionDef = nullptr;
    
    //Arguments used.
    vector<string> args;
    
    //Event custom data 1.
    void* customData1 = nullptr;
    
    //Event custom data 2.
    void* customData2 = nullptr;
    
    //Return value, if applicable.
    bool returnValue = false;
    
    
    //--- Public function declarations ---
    
    ScriptActionInstRunData(ScriptVM* scriptVM, ScriptActionDef* call);
    
};


/**
 * @brief Function that runs a script action instance's logic.
 *
 * The first parameter is the data to run with.
 */
typedef void (ScriptActionTypeCode)(ScriptActionInstRunData& data);

/**
 * @brief Function to run when a script action instance is loaded
 * from a script.
 *
 * The first parameter is the action call data.
 * The second parameter is the mob type it belongs to, if any.
 * Returns whether it loaded successfully.
 */
typedef bool (ScriptActionTypeLoadCode)(
    ScriptActionDef& call, MobType* mt
);


/**
 * @brief Info about an type of a script action.
 */
struct ScriptActionType {

    //--- Public members ---
    
    //Type of script action.
    SCRIPT_ACTION type = SCRIPT_ACTION_UNKNOWN;
    
    //Name.
    string name;
    
    //Code to run.
    ScriptActionTypeCode* code = nullptr;
    
    //Extra logic to run when this action is loaded from a script file.
    ScriptActionTypeLoadCode* extraLoadLogic = nullptr;
    
    //Parameters that it can take.
    vector<ScriptActionTypeParam> parameters;
    
};


#pragma endregion
#pragma region Loaders and runners


namespace ScriptActionRunners {
void absoluteNumber(ScriptActionInstRunData& data);
void addHealth(ScriptActionInstRunData& data);
void arachnorbPlanLogic(ScriptActionInstRunData& data);
void calculate(ScriptActionInstRunData& data);
void ceilNumber(ScriptActionInstRunData& data);
void deleteFunction(ScriptActionInstRunData& data);
void drainLiquid(ScriptActionInstRunData& data);
void easeNumber(ScriptActionInstRunData& data);
void finishDying(ScriptActionInstRunData& data);
void floorNumber(ScriptActionInstRunData& data);
void focus(ScriptActionInstRunData& data);
void followMobAsLeader(ScriptActionInstRunData& data);
void followPathRandomly(ScriptActionInstRunData& data);
void followPathToAbsolute(ScriptActionInstRunData& data);
void getAngle(ScriptActionInstRunData& data);
void getAngleCwDiff(ScriptActionInstRunData& data);
void getAngleSmallestDiff(ScriptActionInstRunData& data);
void getChomped(ScriptActionInstRunData& data);
void getCoordinatesFromAngle(ScriptActionInstRunData& data);
void getDistance(ScriptActionInstRunData& data);
void getEventInfo(ScriptActionInstRunData& data);
void getAreaInfo(ScriptActionInstRunData& data);
void getFloorZ(ScriptActionInstRunData& data);
void getMobInfo(ScriptActionInstRunData& data);
void getFocusVar(ScriptActionInstRunData& data);
void getRandomFloat(ScriptActionInstRunData& data);
void getRandomInt(ScriptActionInstRunData& data);
void holdFocus(ScriptActionInstRunData& data);
void ifFunction(ScriptActionInstRunData& data);
void interpolateNumber(ScriptActionInstRunData& data);
void linkWithFocus(ScriptActionInstRunData& data);
void loadFocusMemory(ScriptActionInstRunData& data);
void moveToAbsolute(ScriptActionInstRunData& data);
void moveToRelative(ScriptActionInstRunData& data);
void moveToTarget(ScriptActionInstRunData& data);
void orderRelease(ScriptActionInstRunData& data);
void playSound(ScriptActionInstRunData& data);
void print(ScriptActionInstRunData& data);
void receiveStatus(ScriptActionInstRunData& data);
void release(ScriptActionInstRunData& data);
void releaseStoredMobs(ScriptActionInstRunData& data);
void removeStatus(ScriptActionInstRunData& data);
void roundNumber(ScriptActionInstRunData& data);
void saveFocusMemory(ScriptActionInstRunData& data);
void sendMessageToFocus(ScriptActionInstRunData& data);
void sendMessageToLinks(ScriptActionInstRunData& data);
void sendMessageToNearby(ScriptActionInstRunData& data);
void setAnimation(ScriptActionInstRunData& data);
void setCanBlockPaths(ScriptActionInstRunData& data);
void setFarReach(ScriptActionInstRunData& data);
void setFlying(ScriptActionInstRunData& data);
void setFocusVar(ScriptActionInstRunData& data);
void setGravity(ScriptActionInstRunData& data);
void setHealth(ScriptActionInstRunData& data);
void setHeight(ScriptActionInstRunData& data);
void setHiding(ScriptActionInstRunData& data);
void setHoldable(ScriptActionInstRunData& data);
void setHuntable(ScriptActionInstRunData& data);
void setLimbAnimation(ScriptActionInstRunData& data);
void setNearReach(ScriptActionInstRunData& data);
void setRadius(ScriptActionInstRunData& data);
void setState(ScriptActionInstRunData& data);
void setSectorScroll(ScriptActionInstRunData& data);
void setShadowVisibility(ScriptActionInstRunData& data);
void setTangible(ScriptActionInstRunData& data);
void setTeam(ScriptActionInstRunData& data);
void setTimer(ScriptActionInstRunData& data);
void setVar(ScriptActionInstRunData& data);
void shakeCamera(ScriptActionInstRunData& data);
void showMessageFromVar(ScriptActionInstRunData& data);
void spawn(ScriptActionInstRunData& data);
void squareRootNumber(ScriptActionInstRunData& data);
void stabilizeZ(ScriptActionInstRunData& data);
void startChomping(ScriptActionInstRunData& data);
void startDying(ScriptActionInstRunData& data);
void startHeightEffect(ScriptActionInstRunData& data);
void startParticles(ScriptActionInstRunData& data);
void stop(ScriptActionInstRunData& data);
void stopChomping(ScriptActionInstRunData& data);
void stopHeightEffect(ScriptActionInstRunData& data);
void stopParticles(ScriptActionInstRunData& data);
void stopSound(ScriptActionInstRunData& data);
void stopVertically(ScriptActionInstRunData& data);
void storeFocusInside(ScriptActionInstRunData& data);
void swallow(ScriptActionInstRunData& data);
void swallowAll(ScriptActionInstRunData& data);
void teleportToAbsolute(ScriptActionInstRunData& data);
void teleportToRelative(ScriptActionInstRunData& data);
void throwFocus(ScriptActionInstRunData& data);
void turnToAbsolute(ScriptActionInstRunData& data);
void turnToRelative(ScriptActionInstRunData& data);
void turnToTarget(ScriptActionInstRunData& data);
};


namespace ScriptActionLoaders {
bool arachnorbPlanLogic(ScriptActionDef& call, MobType* mt);
bool calculate(ScriptActionDef& call, MobType* mt);
bool easeNumber(ScriptActionDef& call, MobType* mt);
bool focus(ScriptActionDef& call, MobType* mt);
bool followMobAsLeader(ScriptActionDef& call, MobType* mt);
bool getAreaInfo(ScriptActionDef& call, MobType* mt);
bool getEventInfo(ScriptActionDef& call, MobType* mt);
bool getMobInfo(ScriptActionDef& call, MobType* mt);
bool holdFocus(ScriptActionDef& call, MobType* mt);
bool ifFunction(ScriptActionDef& call, MobType* mt);
bool moveToTarget(ScriptActionDef& call, MobType* mt);
bool playSound(ScriptActionDef& call, MobType* mt);
bool receiveStatus(ScriptActionDef& call, MobType* mt);
bool removeStatus(ScriptActionDef& call, MobType* mt);
bool setAnimation(ScriptActionDef& call, MobType* mt);
bool setFarReach(ScriptActionDef& call, MobType* mt);
bool setHoldable(ScriptActionDef& call, MobType* mt);
bool setNearReach(ScriptActionDef& call, MobType* mt);
bool setTeam(ScriptActionDef& call, MobType* mt);
bool spawn(ScriptActionDef& call, MobType* mt);
bool stabilizeZ(ScriptActionDef& call, MobType* mt);
bool startChomping(ScriptActionDef& call, MobType* mt);
bool startParticles(ScriptActionDef& call, MobType* mt);
bool turnToTarget(ScriptActionDef& call, MobType* mt);

void reportEnumError(ScriptActionDef& call, size_t argIdx);
bool loadMobTargetType(ScriptActionDef& call, size_t argIdx);
};


#pragma endregion
#pragma region Global functions


Mob* getTriggerMob(ScriptActionInstRunData& data);
Mob* getTargetMob(
    ScriptActionInstRunData& data, MOB_ACTION_MOB_TARGET_TYPE type
);


#pragma endregion
