/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the maker tool structures and functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "../../content/mob/mob.hpp"
#include "../../lib/data_file/data_file.hpp"
#include "../controls_mediator.hpp"


using std::vector;


#pragma region Constants


struct MakerTools;


//List of maker tools.
enum MAKER_TOOL_TYPE {

    //None.
    MAKER_TOOL_TYPE_NONE,
    
    //Clear the terminal's output.
    MAKER_TOOL_TYPE_CLEAR,
    
    //Control camera.
    MAKER_TOOL_TYPE_CONTROL_CAM,
    
    //Create an area maker reminder beneath the cursor.
    MAKER_TOOL_TYPE_CREATE_REMINDER,
    
    //Delete mob.
    MAKER_TOOL_TYPE_DELETE_MOB,
    
    //Fill inventory.
    MAKER_TOOL_TYPE_FILL_INVENTORY,
    
    //Frame advance.
    MAKER_TOOL_TYPE_FRAME_ADVANCE,
    
    //Help.
    MAKER_TOOL_TYPE_HELP,
    
    //Hide HUD.
    MAKER_TOOL_TYPE_HIDE_HUD,
    
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_TYPE_HURT_MOB,
    
    //Get info on the current area.
    MAKER_TOOL_TYPE_INSPECT_AREA,
    
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_TYPE_INSPECT_MOB,
    
    //Create and save an image of the whole area.
    MAKER_TOOL_TYPE_SAVE_AREA_IMAGE,
    
    //Set auto-start data.
    MAKER_TOOL_TYPE_SET_AUTO_START,
    
    //Set song position near loop.
    MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP,
    
    //Set gameplay speed.
    MAKER_TOOL_TYPE_SET_SPEED,
    
    //Show collision box.
    MAKER_TOOL_TYPE_SHOW_COLLISION,
    
    //Show geometry info underneath the mouse cursor.
    MAKER_TOOL_TYPE_SHOW_GEOMETRY_INFO,
    
    //Show hitboxes.
    MAKER_TOOL_TYPE_SHOW_HITBOXES,
    
    //Show path info.
    MAKER_TOOL_TYPE_SHOW_PATH_INFO,
    
    //Show reaches.
    MAKER_TOOL_TYPE_SHOW_REACHES,
    
    //Spawn a new mob underneath the mouse cursor.
    MAKER_TOOL_TYPE_SPAWN_MOB,
    
    //Spawn a new Pikmin underneath the mouse cursor.
    MAKER_TOOL_TYPE_SPAWN_PIKMIN,
    
    //Teleport to mouse cursor.
    MAKER_TOOL_TYPE_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
    
};


//Contexts in which a maker tool's command can run.
enum MAKER_TOOL_CONTEXT {

    //Out of gameplay.
    MAKER_TOOL_CONTEXT_OUT_OF_GAMEPLAY,
    
    //Mid gameplay.
    MAKER_TOOL_CONTEXT_GAMEPLAY,
    
};


//Possible actions to take when activating the control cam maker tool command.
enum MAKER_TOOL_CONTROL_CAM_ACTION {

    //Toggle camera control mode.
    MAKER_TOOL_CONTROL_CAM_ACTION_MODE,
    
    //Toggle freezing the camera's position.
    MAKER_TOOL_CONTROL_CAM_ACTION_FREEZE,
    
};


//Control camera maker tool command action enum naming (internal names).
buildEnumNames(
    makerToolControlCamActionINames, MAKER_TOOL_CONTROL_CAM_ACTION
)({
    { MAKER_TOOL_CONTROL_CAM_ACTION_MODE, "mode" },
    { MAKER_TOOL_CONTROL_CAM_ACTION_FREEZE, "freeze" },
});


//Possible actions to take when activating the frame advance maker tool command.
enum MAKER_TOOL_FRAME_ADVANCE_ACTION {

    //Pause gameplay or advance one frame.
    MAKER_TOOL_FRAME_ADVANCE_ACTION_ADVANCE,
    
    //Resume normal gameplay.
    MAKER_TOOL_FRAME_ADVANCE_ACTION_RESUME,
    
};


//Frame advance maker tool command action enum naming (internal names).
buildEnumNames(
    makerToolFrameAdvanceActionINames, MAKER_TOOL_FRAME_ADVANCE_ACTION
)({
    { MAKER_TOOL_FRAME_ADVANCE_ACTION_ADVANCE, "advance" },
    { MAKER_TOOL_FRAME_ADVANCE_ACTION_RESUME, "resume" },
});


//Possible actions to take when activating the inspect maker tool command.
enum MAKER_TOOL_INSPECT_MOB_ACTION {

    //Detect the closest to the mouse cursor.
    MAKER_TOOL_INSPECT_MOB_ACTION_GET_CLOSEST,
    
    //Detect all around the mouse cursor and iterate between that list.
    MAKER_TOOL_INSPECT_MOB_ACTION_ITERATE,
    
    //Act as if no mob was detected.
    MAKER_TOOL_INSPECT_MOB_ACTION_STOP,
    
};


//Inspect mob maker tool command action enum naming (internal names).
buildEnumNames(
    makerToolInspectMobActionINames, MAKER_TOOL_INSPECT_MOB_ACTION
)({
    { MAKER_TOOL_INSPECT_MOB_ACTION_GET_CLOSEST, "get_closest" },
    { MAKER_TOOL_INSPECT_MOB_ACTION_ITERATE, "iterate" },
    { MAKER_TOOL_INSPECT_MOB_ACTION_STOP, "stop" },
});


namespace MAKER_TOOLS {
extern const float PLAY_CONFIRMATION_TIMER;
}


#pragma endregion
#pragma region Classes


/**
 * @brief Function that runs a maker tool command's logic.
 *
 * The first parameter is the arguments passed to it.
 * The return value is whether it succeeded.
 */
typedef bool (MakerToolTypeCode)(MakerTools& mgr, const vector<string>& args);


/**
 * @brief Information about a type of maker tool.
 */
struct MakerToolType {

    //--- Public members ---
    
    //Type of maker tool.
    MAKER_TOOL_TYPE type = MAKER_TOOL_TYPE_NONE;
    
    //Name. Also used for its command.
    string name;
    
    //Code to run when its command is executed.
    MakerToolTypeCode* code = nullptr;
    
    //Parameters that it can take.
    vector<CommandParam> parameters;
    
    //Flags for the contexts in which it can be run. Use MAKER_TOOL_CONTEXT.
    Bitmask8 contexts = 0;
    
    //Whether using this tool helps the player in any way.
    bool helpful = false;
    
    //Description.
    string description;
    
};


/**
 * @brief Manager of all of the maker tools.
 */
struct MakerTools {

    //--- Public misc. definitions ---
    
    struct AreaImageSettings {
    
        //Padding around the actual area, in area pixels. Each side
        //of the area receives these many pixels divided by two.
        float padding = 32.0f;
        
        //Show tree shadows in the area image tool?
        bool shadows = true;
        
        //Maximum width or height of the area image.
        int size = 2048;
        
    };
    
    
    //--- Public members ---
    
    //List of all types of maker tool.
    vector<MakerToolType> types;
    
    //Are the tools enabled?
    bool enabled = true;
    
    //If maker tools are limited in play mode, are they allowed now?
    bool allowedInPlayNow = false;
    
    //How many times the player has pressed a maker tool button to confirm,
    //when tools are limited in play mode.
    unsigned char playConfirmationPresses = 0;
    
    //Time left to confirm, when tools are limited in play mode.
    float playConfirmationTimer = 0.0f;
    
    //Whether the first modifier input is held down.
    bool mod1 = false;
    
    //Whether the second modifier input is held down.
    bool mod2 = false;
    
    //Mouse cursor world coordinates when the latest maker tool was started.
    Point toolStartCursor;
    
    //Has the player made use of any tools that could help them play?
    bool usedHelpingTools = false;
    
    //Settings for the hurt mob tool. When used, dock this much of its max HP.
    float hurtMobSettings[3] = { 0.75f, 1.0f, -1.0f };
    
    //Settings for the save area image tool.
    AreaImageSettings saveAreaImageSettings[3];
    
    //Settings for the set speed tool. These are multipliers to change by.
    float setSpeedSettings[3] = { 2.0f, 0.5f, 1.0f };
    
    //Automatically pick this from the list of the selected auto-entry mode.
    string autoStartOption;
    
    //Automatically enter this game mode when the game boots.
    string autoStartState;
    
    //Whether control camera mode is active.
    bool inControlCam = false;
    
    //Whether the control camera is frozen (player controls leader)
    //or mobile (player controls camera).
    bool controlCamFrozen = false;
    
    //Are we currently paused for frame advance?
    bool inFrameAdvanceMode = false;
    
    //Do we have to advance one game frame on the next processing frame?
    bool mustAdvanceOneFrame = false;
    
    //Are we inspecting the current area?
    bool inspectingArea = false;
    
    //Mob currently being inspected. nullptr if none.
    Mob* inspectedMob = nullptr;
    
    //Are we currently changing the game speed?
    bool changingSpeed = false;
    
    //Which game speed change multiplier to use.
    float changedSpeedMultiplier = 2.0f;
    
    //Are collision boxes visible in-game?
    bool showingCollision = false;
    
    //Is the geometry information tool enabled?
    bool showingGeometryInfo = false;
    
    //Are hitboxes visible in-game?
    bool showingHitboxes = false;
    
    //Is the HUD visible?
    bool showingHud = true;
    
    //Show path info of the currently inspected mob?
    bool showingPathInfo = false;
    
    //Show the reaches of the currently inspected mob?
    bool showingReaches = false;
    
    //When we last spawned a Pikmin, what was its type?
    PikminType* lastPikminType = nullptr;
    
    //Use the performance monitor?
    bool usePerfMon = false;
    
    
    //--- Public function declarations ---
    
    bool checkMakerToolsAllowed(float inputValue);
    bool handleGameplayPlayerAction(const Inpution::Action& action);
    bool handleGeneralPlayerAction(const Inpution::Action& action);
    void loadFromDataNode(DataNode* node);
    void resetForGameplay();
    bool runTool(const string& typeName, const vector<string>& args);
    bool runTool(MAKER_TOOL_TYPE type, const vector<string>& args);
    void saveToDataNode(DataNode* node);
    void tick(float deltaT);
    unsigned char getMakerToolSettingIdx() const;
    
    
private:

    //--- Private function declarations ---
    
    bool checkContextFlags(Bitmask8 flags) const;
    MAKER_TOOL_CONTEXT getContext() const;
    string getContextName() const;
    
};


#pragma endregion
#pragma region Tool runner functions


namespace MakerToolRunners {
bool clear(MakerTools& mgr, const vector<string>& args);
bool controlCam(MakerTools& mgr, const vector<string>& args);
bool createReminder(MakerTools& mgr, const vector<string>& args);
bool deleteMob(MakerTools& mgr, const vector<string>& args);
bool fillInventory(MakerTools& mgr, const vector<string>& args);
bool frameAdvance(MakerTools& mgr, const vector<string>& args);
bool help(MakerTools& mgr, const vector<string>& args);
bool hideHud(MakerTools& mgr, const vector<string>& args);
bool hurtMob(MakerTools& mgr, const vector<string>& args);
bool inspectArea(MakerTools& mgr, const vector<string>& args);
bool inspectMob(MakerTools& mgr, const vector<string>& args);
bool saveAreaImage(MakerTools& mgr, const vector<string>& args);
bool setAutoStart(MakerTools& mgr, const vector<string>& args);
bool setSongPosNearLoop(MakerTools& mgr, const vector<string>& args);
bool setSpeed(MakerTools& mgr, const vector<string>& args);
bool showCollision(MakerTools& mgr, const vector<string>& args);
bool showGeometryInfo(MakerTools& mgr, const vector<string>& args);
bool showHitboxes(MakerTools& mgr, const vector<string>& args);
bool showPathInfo(MakerTools& mgr, const vector<string>& args);
bool showReaches(MakerTools& mgr, const vector<string>& args);
bool spawnMob(MakerTools& mgr, const vector<string>& args);
bool spawnPikmin(MakerTools& mgr, const vector<string>& args);
bool teleport(MakerTools& mgr, const vector<string>& args);
}


#pragma endregion
