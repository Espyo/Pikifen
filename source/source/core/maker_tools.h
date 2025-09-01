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

#include "../content/mob/mob.h"
#include "../lib/data_file/data_file.h"
#include "controls_mediator.h"


using std::vector;


//List of maker tools.
enum MAKER_TOOL_TYPE {

    //None.
    MAKER_TOOL_TYPE_NONE,
    
    //Create an image of the whole area.
    MAKER_TOOL_TYPE_AREA_IMAGE,
    
    //Change gameplay speed.
    MAKER_TOOL_TYPE_CHANGE_SPEED,
    
    //Show collision box.
    MAKER_TOOL_TYPE_COLLISION,
    
    //Frame advance.
    MAKER_TOOL_TYPE_FRAME_ADVANCE,
    
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_TYPE_GEOMETRY_INFO,
    
    //Show hitboxes.
    MAKER_TOOL_TYPE_HITBOXES,
    
    //Toggle HUD visibility.
    MAKER_TOOL_TYPE_HUD,
    
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_TYPE_HURT_MOB,
    
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_TYPE_MOB_INFO,
    
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_TYPE_NEW_PIKMIN,
    
    //Show path info.
    MAKER_TOOL_TYPE_PATH_INFO,
    
    //Set song position near loop.
    MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP,
    
    //Teleport to mouse cursor.
    MAKER_TOOL_TYPE_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
    
};


namespace MAKER_TOOLS {
extern const string NAMES[N_MAKER_TOOLS];
extern const float PLAY_CONFIRMATION_TIMER;
}


/**
 * @brief Info about all of the maker tools.
 */
struct MakerTools {

    //--- Misc. definitions ---
    
    struct AreaImageSettings {
    
        //Padding around the area in the area image tool.
        float padding = 32.0f;
        
        //Show tree shadows in the area image tool?
        bool shadows = true;
        
        //Maximum width or height of the area image.
        int size = 2048;
        
        //Show mobs in the area image?
        bool mobs = true;
        
    };
    
    
    //--- Members ---
    
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
    
    //Are we currently paused for frame advance?
    bool frameAdvanceMode = false;
    
    //Do we have to advance one game frame on the next processing frame?
    bool mustAdvanceOneFrame = false;
    
    //Which game speed change setting to use.
    unsigned char changeSpeedSettingIdx = 0;
    
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
    
    //Mob currently locked-on to for the mob information tool. nullptr if off.
    Mob* infoLock = nullptr;
    
    //When we last spawned a Pikmin, what was its type?
    PikminType* lastPikminType = nullptr;
    
    //Different mob hurting settings. When used, dock this much of its max HP.
    float mobHurtingSettings[3] = { 0.75f, 1.0f, -1.0f };
    
    //Whether the first modifier input is held down.
    bool mod1 = false;
    
    //Whether the second modifier input is held down.
    bool mod2 = false;
    
    //Show path info?
    bool pathInfo = false;
    
    //How many times the player has pressed a maker tool button to confirm,
    //when tools are limited in play mode.
    unsigned char playConfirmationPresses = 0;
    
    //Time left to confirm, when tools are limited in play mode.
    float playConfirmationTimer = 0.0f;
    
    //Use the performance monitor?
    bool usePerfMon = false;
    
    //Has the player made use of any tools that could help them play?
    bool usedHelpingTools = false;
    
    
    //--- Function declarations ---
    
    bool checkMakerToolsAllowed(float inputValue);
    bool handleGameplayPlayerAction(const PlayerAction& action);
    bool handleGeneralPlayerAction(const PlayerAction& action);
    void loadFromDataNode(DataNode* node);
    void resetForGameplay();
    void saveToDataNode(DataNode* node);
    void tick(float deltaT);
    
    
private:

    //--- Function declarations ---
    
    unsigned char getMakerToolSettingIdx() const;
    
};
