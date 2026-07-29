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

#include "../content/mob/mob.hpp"
#include "../lib/data_file/data_file.hpp"
#include "controls_mediator.hpp"


using std::vector;


#pragma region Constants


struct MakerTools;


//List of maker tools.
enum MAKER_TOOL_TYPE {

    //None.
    MAKER_TOOL_TYPE_NONE,
    
    //Create an image of the whole area.
    MAKER_TOOL_TYPE_AREA_IMAGE,
    
    //Get info on the current area.
    MAKER_TOOL_TYPE_AREA_INSPECTOR,
    
    //Change gameplay speed.
    MAKER_TOOL_TYPE_CHANGE_SPEED,
    
    //Delete mob.
    MAKER_TOOL_TYPE_DELETE_MOB,
    
    //Fill inventory.
    MAKER_TOOL_TYPE_FILL_INVENTORY,
    
    //Frame advance.
    MAKER_TOOL_TYPE_FRAME_ADVANCE,
    
    //Free camera.
    MAKER_TOOL_TYPE_FREE_CAM,
    
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_TYPE_GEOMETRY_INFO,
    
    //Toggle HUD visibility.
    MAKER_TOOL_TYPE_HIDE_HUD,
    
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_TYPE_HURT_MOB,
    
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_TYPE_MOB_INSPECTOR,
    
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_TYPE_NEW_PIKMIN,
    
    //Create an area maker reminder beneath the cursor.
    MAKER_TOOL_TYPE_NEW_REMINDER,
    
    //Show path info.
    MAKER_TOOL_TYPE_PATH_INFO,
    
    //Set auto-start data.
    MAKER_TOOL_TYPE_SET_AUTO_START,
    
    //Set song position near loop.
    MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP,
    
    //Show collision box.
    MAKER_TOOL_TYPE_SHOW_COLLISION,
    
    //Show hitboxes.
    MAKER_TOOL_TYPE_SHOW_HITBOXES,
    
    //Show reaches.
    MAKER_TOOL_TYPE_SHOW_REACHES,
    
    //Teleport to mouse cursor.
    MAKER_TOOL_TYPE_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
    
};


//Contexts in which a maker tool's command can run.
enum MAKER_TOOL_CONTEXT {

    //Anywhere.
    MAKER_TOOL_CONTEXT_ANYWHERE,
    
    //Mid gameplay.
    MAKER_TOOL_CONTEXT_GAMEPLAY,
    
};


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


//Possible actions to take when activating the free cam maker tool command.
enum MAKER_TOOL_FREE_CAM_ACTION {

    //Toggle free cam mode.
    MAKER_TOOL_FREE_CAM_ACTION_MODE,
    
    //Toggle freezing the camera's position.
    MAKER_TOOL_FREE_CAM_ACTION_FREEZE,
    
};


//Frame advance maker tool command action enum naming (internal names).
buildEnumNames(
    makerToolFreeCamActionINames, MAKER_TOOL_FREE_CAM_ACTION
)({
    { MAKER_TOOL_FREE_CAM_ACTION_MODE, "mode" },
    { MAKER_TOOL_FREE_CAM_ACTION_FREEZE, "freeze" },
});


//Possible actions to take when activating the mob inspector maker tool command.
enum MAKER_TOOL_MOB_INSPECTOR_ACTION {

    //Detect the closest to the mouse cursor.
    MAKER_TOOL_MOB_INSPECTOR_ACTION_GET_CLOSEST,
    
    //Detect all around the mouse cursor and iterate between that list.
    MAKER_TOOL_MOB_INSPECTOR_ACTION_ITERATE,
    
    //Act as if no mob was detected.
    MAKER_TOOL_MOB_INSPECTOR_ACTION_STOP,
    
};


//Frame advance maker tool command action enum naming (internal names).
buildEnumNames(
    makerToolMobInspectorActionINames, MAKER_TOOL_MOB_INSPECTOR_ACTION
)({
    { MAKER_TOOL_MOB_INSPECTOR_ACTION_GET_CLOSEST, "get_closest" },
    { MAKER_TOOL_MOB_INSPECTOR_ACTION_ITERATE, "iterate" },
    { MAKER_TOOL_MOB_INSPECTOR_ACTION_STOP, "stop" },
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
    
    //Different area image settings.
    AreaImageSettings areaImageSettings[3];
    
    //Automatically pick this from the list of the selected auto-entry mode.
    string autoStartOption;
    
    //Automatically enter this game mode when the game boots.
    string autoStartState;
    
    //Are we currently changing the game speed?
    bool changeSpeed = false;
    
    //Which game speed change multiplier to use.
    float changeSpeedMultiplier = 2.0f;
    
    //Are we currently paused for frame advance?
    bool frameAdvanceMode = false;
    
    //Whether the free camera is frozen (player controls leader)
    //or mobile (player controls camera).
    bool freeCamFrozen = false;
    
    //Whether free camera mode is active.
    bool freeCamMode = false;
    
    //Do we have to advance one game frame on the next processing frame?
    bool mustAdvanceOneFrame = false;
    
    //Different game speed change settings. These are multipliers to change by.
    float changeSpeedSettings[3] = { 2.0f, 0.5f, 1.0f };
    
    //Are collision boxes visible in-game?
    bool collision = false;
    
    //Is the geometry information tool enabled?
    bool geometryInfo = false;
    
    //Are hitboxes visible in-game?
    bool hitboxes = false;
    
    //Is the HUD visible?
    bool hud = true;
    
    //Are we inspecting the current area?
    bool inspectingArea = false;
    
    //Mob currently being inspected. nullptr if none.
    Mob* inspectedMob = nullptr;
    
    //When we last spawned a Pikmin, what was its type?
    PikminType* lastPikminType = nullptr;
    
    //Different mob hurting settings. When used, dock this much of its max HP.
    float mobHurtingSettings[3] = { 0.75f, 1.0f, -1.0f };
    
    //Whether the first modifier input is held down.
    bool mod1 = false;
    
    //Whether the second modifier input is held down.
    bool mod2 = false;
    
    //Show path info of the currently inspected mob?
    bool pathInfo = false;
    
    //Show the reaches of the currently inspected mob?
    bool reaches = false;
    
    //Mouse cursor world coordinates when the latest maker tool was started.
    Point toolStartCursor;
    
    //How many times the player has pressed a maker tool button to confirm,
    //when tools are limited in play mode.
    unsigned char playConfirmationPresses = 0;
    
    //Time left to confirm, when tools are limited in play mode.
    float playConfirmationTimer = 0.0f;
    
    //Use the performance monitor?
    bool usePerfMon = false;
    
    //Has the player made use of any tools that could help them play?
    bool usedHelpingTools = false;
    
    
    //--- Public function declarations ---
    
    bool checkMakerToolsAllowed(float inputValue);
    bool handleGameplayPlayerAction(const Inpution::Action& action);
    bool handleGeneralPlayerAction(const Inpution::Action& action);
    void loadFromDataNode(DataNode* node);
    void resetForGameplay();
    void saveToDataNode(DataNode* node);
    void tick(float deltaT);
    unsigned char getMakerToolSettingIdx() const;
    
};


#pragma endregion
#pragma region Tool runner functions


namespace MakerToolRunners {
bool areaImage(MakerTools& mgr, const vector<string>& args);
bool areaInspector(MakerTools& mgr, const vector<string>& args);
bool changeSpeed(MakerTools& mgr, const vector<string>& args);
bool deleteMob(MakerTools& mgr, const vector<string>& args);
bool fillInventory(MakerTools& mgr, const vector<string>& args);
bool frameAdvance(MakerTools& mgr, const vector<string>& args);
bool freeCam(MakerTools& mgr, const vector<string>& args);
bool geometryInfo(MakerTools& mgr, const vector<string>& args);
bool hideHud(MakerTools& mgr, const vector<string>& args);
bool hurtMob(MakerTools& mgr, const vector<string>& args);
bool mobInspector(MakerTools& mgr, const vector<string>& args);
bool newPikmin(MakerTools& mgr, const vector<string>& args);
bool newReminder(MakerTools& mgr, const vector<string>& args);
bool pathInfo(MakerTools& mgr, const vector<string>& args);
bool setAutoStart(MakerTools& mgr, const vector<string>& args);
bool setSongPosNearLoop(MakerTools& mgr, const vector<string>& args);
bool showCollision(MakerTools& mgr, const vector<string>& args);
bool showHitboxes(MakerTools& mgr, const vector<string>& args);
bool showReaches(MakerTools& mgr, const vector<string>& args);
bool teleport(MakerTools& mgr, const vector<string>& args);
}


#pragma endregion
