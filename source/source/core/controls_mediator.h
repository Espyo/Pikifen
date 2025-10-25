/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the control-related classes and functions.
 * This is the mediator between Allegro inputs, Pikifen player actions,
 * and the controls manager.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_set>

#include <allegro5/allegro.h>

#include "../lib/data_file/data_file.h"
#include "../lib/inpution/inpution.h"


using std::size_t;
using std::string;
using std::unordered_set;
using std::vector;


//List of player action types.
enum PLAYER_ACTION_TYPE {

    //None.
    PLAYER_ACTION_TYPE_NONE,
    
    //Main.
    
    //Move right.
    PLAYER_ACTION_TYPE_RIGHT,
    
    //Move down.
    PLAYER_ACTION_TYPE_DOWN,
    
    //Move left.
    PLAYER_ACTION_TYPE_LEFT,
    
    //Move up.
    PLAYER_ACTION_TYPE_UP,
    
    //Throw.
    PLAYER_ACTION_TYPE_THROW,
    
    //Whistle.
    PLAYER_ACTION_TYPE_WHISTLE,
    
    //Swap to next standby type.
    PLAYER_ACTION_TYPE_NEXT_TYPE,
    
    //Swap to previous standby type.
    PLAYER_ACTION_TYPE_PREV_TYPE,
    
    //Swap to next leader.
    PLAYER_ACTION_TYPE_NEXT_LEADER,
    
    //Swarm group towards leader cursor.
    PLAYER_ACTION_TYPE_GROUP_CURSOR,
    
    //Dismiss.
    PLAYER_ACTION_TYPE_DISMISS,
    
    //Inventory.
    PLAYER_ACTION_TYPE_INVENTORY,
    
    //Pause.
    PLAYER_ACTION_TYPE_PAUSE,
    
    //Menus.
    
    //Menu navigation right.
    PLAYER_ACTION_TYPE_MENU_RIGHT,
    
    //Menu navigation up.
    PLAYER_ACTION_TYPE_MENU_UP,
    
    //Menu navigation left.
    PLAYER_ACTION_TYPE_MENU_LEFT,
    
    //Menu navigation down.
    PLAYER_ACTION_TYPE_MENU_DOWN,
    
    //Menu navigation OK.
    PLAYER_ACTION_TYPE_MENU_OK,
    
    //Radar pan right.
    PLAYER_ACTION_TYPE_RADAR_RIGHT,
    
    //Radar pan down.
    PLAYER_ACTION_TYPE_RADAR_DOWN,
    
    //Radar pan left.
    PLAYER_ACTION_TYPE_RADAR_LEFT,
    
    //Radar pan up.
    PLAYER_ACTION_TYPE_RADAR_UP,
    
    //Radar zoom in.
    PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
    
    //Radar zoom out.
    PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
    
    //Onion menu change 10 toggle.
    PLAYER_ACTION_TYPE_ONION_CHANGE_10,
    
    //Onion menu select all toggle.
    PLAYER_ACTION_TYPE_ONION_SELECT_ALL,
    
    //Advanced.
    
    //Move leader cursor right.
    PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT,
    
    //Move leader cursor down.
    PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN,
    
    //Move leader cursor left.
    PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT,
    
    //Move leader cursor up.
    PLAYER_ACTION_TYPE_LEADER_CURSOR_UP,
    
    //Swarm group right.
    PLAYER_ACTION_TYPE_GROUP_RIGHT,
    
    //Swarm group down.
    PLAYER_ACTION_TYPE_GROUP_DOWN,
    
    //Swarm group left.
    PLAYER_ACTION_TYPE_GROUP_LEFT,
    
    //Swarm group up.
    PLAYER_ACTION_TYPE_GROUP_UP,
    
    //Swap to previous leader.
    PLAYER_ACTION_TYPE_PREV_LEADER,
    
    //Change zoom level.
    PLAYER_ACTION_TYPE_CHANGE_ZOOM,
    
    //Zoom in.
    PLAYER_ACTION_TYPE_ZOOM_IN,
    
    //Zoom out.
    PLAYER_ACTION_TYPE_ZOOM_OUT,
    
    //Swap to next standby type maturity.
    PLAYER_ACTION_TYPE_NEXT_MATURITY,
    
    //Swap to previous standby type maturity.
    PLAYER_ACTION_TYPE_PREV_MATURITY,
    
    //Inventory shortcut A.
    PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_A,
    
    //Inventory shortcut B.
    PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_B,
    
    //Inventory shortcut C.
    PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_C,
    
    //Inventory shortcut D.
    PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_D,
    
    //Custom A.
    PLAYER_ACTION_TYPE_CUSTOM_A,
    
    //Custom B.
    PLAYER_ACTION_TYPE_CUSTOM_B,
    
    //Custom C.
    PLAYER_ACTION_TYPE_CUSTOM_C,
    
    //Toggle the radar.
    PLAYER_ACTION_TYPE_RADAR,
    
    //Menu navigation back.
    PLAYER_ACTION_TYPE_MENU_BACK,
    
    //Menu navigation page to the left.
    PLAYER_ACTION_TYPE_MENU_PAGE_LEFT,
    
    //Menu navigation page to the right.
    PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT,
    
    //General maker tool things.
    
    //Auto-start.
    PLAYER_ACTION_TYPE_MT_AUTO_START,
    
    //Set song position near loop.
    PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
    
    //Maker tool modifier 1.
    PLAYER_ACTION_TYPE_MT_MOD_1,
    
    //Maker tool modifier 2.
    PLAYER_ACTION_TYPE_MT_MOD_2,
    
    //Gameplay maker tools.
    
    //Area image.
    PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
    
    //Change speed.
    PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
    
    //Frame advance.
    PLAYER_ACTION_TYPE_MT_FRAME_ADVANCE,
    
    //Geometry info.
    PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
    
    //HUD.
    PLAYER_ACTION_TYPE_MT_HUD,
    
    //Hurt mob.
    PLAYER_ACTION_TYPE_MT_HURT_MOB,
    
    //Mob info.
    PLAYER_ACTION_TYPE_MT_MOB_INFO,
    
    //New Pikmin.
    PLAYER_ACTION_TYPE_MT_NEW_PIKMIN,
    
    //Path info.
    PLAYER_ACTION_TYPE_MT_PATH_INFO,
    
    //Show collision.
    PLAYER_ACTION_TYPE_MT_SHOW_COLLISION,
    
    //Show hitboxes.
    PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES,
    
    //Show reaches.
    PLAYER_ACTION_TYPE_MT_SHOW_REACHES,
    
    //Teleport.
    PLAYER_ACTION_TYPE_MT_TELEPORT,
    
    //System.
    
    //System info.
    PLAYER_ACTION_TYPE_SYSTEM_INFO,
    
    //Screenshot.
    PLAYER_ACTION_TYPE_SCREENSHOT,
    
};


//Categories of player action types.
enum PLAYER_ACTION_CAT {

    //None.
    PLAYER_ACTION_CAT_NONE,
    
    //Main.
    PLAYER_ACTION_CAT_MAIN,
    
    //Menus.
    PLAYER_ACTION_CAT_MENUS,
    
    //Advanced.
    PLAYER_ACTION_CAT_ADVANCED,
    
    //General maker tool things.
    PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
    
    //Gameplay maker tools.
    PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
    
    //System.
    PLAYER_ACTION_CAT_SYSTEM,
    
};


//Game states, as far as the controls are concerned.
enum CONTROLS_GAME_STATE {

    //Menus outside the gameplay state.
    CONTROLS_GAME_STATE_MENUS,
    
    //Interlude in the gameplay state.
    CONTROLS_GAME_STATE_INTERLUDE,
    
    //Normal gameplay.
    CONTROLS_GAME_STATE_GAMEPLAY,
    
};


/**
 * @brief Data about a type of action that can be performed in the game.
 * This data is pertinent only to Pikifen, not the library.
 */
struct PlayerActionType : public Inpution::ActionType {

    //--- Members ---
    
    //ID of the action type.
    PLAYER_ACTION_TYPE id = PLAYER_ACTION_TYPE_NONE;
    
    //Category, for use in stuff like the options menu.
    PLAYER_ACTION_CAT category = PLAYER_ACTION_CAT_NONE;
    
    //Name, for use in the options menu.
    string name;
    
    //Description, for use in the options menu.
    string description;
    
    //Its name in the options file.
    string internalName;
    
    //String representing of this action type's default control bind.
    string defaultBindStr;
    
};


/**
 * @brief Mediates everything control-related in Pikifen.
 */
struct ControlsMediator {

    public:
    
    //--- Function declarations ---
    
    bool actionTypesShareInputSource(
        const vector<PLAYER_ACTION_TYPE> actionTypes
    );
    void addPlayerActionType(
        PLAYER_ACTION_TYPE id,
        PLAYER_ACTION_CAT category,
        const string& name,
        const string& description,
        const string& internalName,
        const string& defaultBindStr,
        Inpution::ACTION_VALUE_TYPE valueType,
        float autoRepeat = 0.0f,
        float reinsertionTTL = 0.0f
    );
    const vector<PlayerActionType>& getAllPlayerActionTypes() const;
    vector<Inpution::Bind>& binds();
    string inputSourceToStr(const Inpution::InputSource& s) const;
    Inpution::Bind findBind(
        const PLAYER_ACTION_TYPE actionTypeId
    ) const;
    Inpution::Bind findBind(
        const string& actionTypeName
    ) const;
    float getInputSourceValue(const Inpution::InputSource& source) const;
    PlayerActionType getPlayerActionType(int actionId) const;
    string getPlayerActionTypeInternalName(int actionId);
    float getPlayerActionTypeValue(
        PLAYER_ACTION_TYPE playerActionTypeId
    );
    void loadBindsFromDataNode(DataNode* node, unsigned char playerNr);
    Inpution::InputSource strToInputSource(const string& s) const;
    Inpution::Input allegroEventToInput(const ALLEGRO_EVENT& ev) const;
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    vector<Inpution::Action> newFrame(float deltaT);
    void reinsertAction(const Inpution::Action& action);
    void releaseAll();
    void saveBindsToDataNode(DataNode* node, unsigned char playerNr);
    void setGameState(CONTROLS_GAME_STATE state);
    void setOptions(const Inpution::ManagerOptions& options);
    void startIgnoringActions();
    void startIgnoringInputSource(
        const Inpution::InputSource& inputSource, bool nowOnly
    );
    void stopIgnoringActions();
    
    private:
    
    //--- Members ---
    
    //List of registered player action types.
    vector<PlayerActionType> playerActionTypes;
    
    //Inpution manager.
    Inpution::Manager mgr;
    
};
