/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the the player class and related functions.
 */

#pragma once

#include <string>
#include <vector>

#include "../content/mob/leader.h"
#include "../game_state/gameplay/hud.h"
#include "../game_state/gameplay/inventory.h"
#include "../game_state/gameplay/pause_menu.h"
#include "misc_structs.h"

using std::string;
using std::vector;


/**
 * @brief Info about the current amount of sprays and ingredients
 * for the available spray types.
 */
struct SprayStats {

    //--- Members ---
    
    //Number of sprays of this type owned.
    size_t nrSprays = 0;
    
    //Number of concoction ingredients owned.
    size_t nrIngredients = 0;
    
};


struct Player;

/**
 * @brief One of the player teams for when players are playing cooperatively.
 */
struct PlayerTeam {

    public:
    
    //--- Members ---
    
    //How many of each spray/ingredients the player has.
    vector<SprayStats> sprayStats;
    
    //List of players in this team. Cache for convenience.
    vector<Player*> players;
    
};


/**
 * @brief Represents one of the players playing the game.
 */
struct Player {

    public:
    
    //--- Members ---
    
    //Player number.
    unsigned char playerNr = 0;
    
    //Viewport during gameplay.
    Viewport view;
    
    //The HUD.
    Hud* hud = nullptr;
    
    //Inventory.
    Inventory* inventory = nullptr;
    
    //Player team.
    PlayerTeam* team;
    
    //Closest to the leader, for the previous, current, next type.
    Mob* closestGroupMember[3] = { nullptr, nullptr, nullptr };
    
    //Index of the current leader, in the array of available leaders.
    size_t leaderIdx = 0;
    
    //Pointer to the leader, if any. Cache for convenience.
    Leader* leaderPtr = nullptr;
    
    //Is the group member closest to the leader distant?
    bool closestGroupMemberDistant = false;
    
    //Leader cursor's current position, in window coordinates.
    Point leaderCursorWin;
    
    //Leader cursor's current position, in world coordinates.
    Point leaderCursorWorld;
    
    //Sector that the leader's cursor is on, if any.
    Sector* leaderCursorSector = nullptr;
    
    //Amount of enemy or treasure points to show next to the leader cursor.
    int leaderCursorMobPoints = 0;
    
    //Alpha of the enemy or treasure points to show next to the leader cursor.
    float leaderCursorMobPointsAlpha = 0.0f;

    //Multiply the leader's walking speed by this.
    float leaderSpeedMult = 1.0f;
    
    //Index of the shortcut when showing a shortcut's usage on-screen.
    //INVALID for none.
    size_t inventoryShortcutDisplayIdx = INVALID;
    
    //Animation timer when showing a shortcut's usage on-screen.
    float inventoryShortcutDisplayTimer = 0.0f;
    
    //Current leader prompt, if any.
    LeaderPrompt leaderPrompt;
    
    //Index of the selected spray.
    size_t selectedSpray = 0;
    
    //Angle of swarming.
    float swarmAngle = 0.0f;
    
    //General intensity of swarming in the specified angle.
    float swarmMagnitude = 0.0f;
    
    //Destination of the throw.
    Point throwDest;
    
    //Mob that the throw will land on, if any.
    Mob* throwDestMob = nullptr;
    
    //Sector that the throw will land on, if any.
    Sector* throwDestSector = nullptr;
    
    //Movement to control the leader with.
    MovementInfo leaderMovement;
    
    //The leader's whistle.
    Whistle whistle;
    
    //Zoom level to use on the radar.
    float radarZoom = 1.0f;
    
    //Points to an interactable close enough for the player to use, if any.
    Interactable* closeToInteractableToUse = nullptr;
    
    //Points to a nest-like object close enough for the player to open, if any.
    PikminNest* closeToNestToOpen = nullptr;
    
    //Points to a Pikmin close enough for the player to pluck, if any.
    Pikmin* closeToPikminToPluck = nullptr;
    
    //Points to a ship close enough for the player to heal in, if any.
    Ship* closeToShipToHeal = nullptr;
    
    //Ligthten the leader cursor by this due to leader/cursor height difference.
    float leaderCursorHeightDiffLight = 0.0f;
    
    //Movement of the leader cursor via non-mouse means.
    MovementInfo leaderCursorMov;
    
    //Is the player holding the "swarm to leader cursor" button?
    bool swarmToLeaderCursor = false;
    
    //Reach of swarming.
    MovementInfo swarmMovement;
    
};
