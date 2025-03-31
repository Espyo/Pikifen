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
#include "../lib/controls_manager/controls_manager.h"


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
    
    //Move up.
    PLAYER_ACTION_TYPE_UP,
    
    //Move left.
    PLAYER_ACTION_TYPE_LEFT,
    
    //Move down.
    PLAYER_ACTION_TYPE_DOWN,
    
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
    
    //Swarm group towards cursor.
    PLAYER_ACTION_TYPE_GROUP_CURSOR,
    
    //Dismiss.
    PLAYER_ACTION_TYPE_DISMISS,
    
    //Use spray #1.
    PLAYER_ACTION_TYPE_USE_SPRAY_1,
    
    //Use spray #2.
    PLAYER_ACTION_TYPE_USE_SPRAY_2,
    
    //Use currently selected spray.
    PLAYER_ACTION_TYPE_USE_SPRAY,
    
    //Swap to next spray.
    PLAYER_ACTION_TYPE_NEXT_SPRAY,
    
    //Swap to previous spray.
    PLAYER_ACTION_TYPE_PREV_SPRAY,
    
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
    
    //Radar pan up.
    PLAYER_ACTION_TYPE_RADAR_UP,
    
    //Radar pan left.
    PLAYER_ACTION_TYPE_RADAR_LEFT,
    
    //Radar pan down.
    PLAYER_ACTION_TYPE_RADAR_DOWN,
    
    //Radar zoom in.
    PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
    
    //Radar zoom out.
    PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
    
    //Advanced.
    
    //Move cursor right.
    PLAYER_ACTION_TYPE_CURSOR_RIGHT,
    
    //Move cursor up.
    PLAYER_ACTION_TYPE_CURSOR_UP,
    
    //Move cursor left.
    PLAYER_ACTION_TYPE_CURSOR_LEFT,
    
    //Move cursor down.
    PLAYER_ACTION_TYPE_CURSOR_DOWN,
    
    //Swarm group right.
    PLAYER_ACTION_TYPE_GROUP_RIGHT,
    
    //Swarm group up.
    PLAYER_ACTION_TYPE_GROUP_UP,
    
    //Swarm group left.
    PLAYER_ACTION_TYPE_GROUP_LEFT,
    
    //Swarm group down.
    PLAYER_ACTION_TYPE_GROUP_DOWN,
    
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
    
    //Lie down.
    PLAYER_ACTION_TYPE_LIE_DOWN,
    
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
    
    //Gameplay maker tools.
    
    //Area image.
    PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
    
    //Change speed.
    PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
    
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
    
    //Teleport.
    PLAYER_ACTION_TYPE_MT_TELEPORT,
    
    //Global maker tools.
    
    //Auto-start.
    PLAYER_ACTION_TYPE_MT_AUTO_START,
    
    //Set song position near loop.
    PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
    
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
    
    //Gameplay maker tools.
    PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
    
    //Global maker tools.
    PLAYER_ACTION_CAT_GLOBAL_MAKER_TOOLS,
    
    //System.
    PLAYER_ACTION_CAT_SYSTEM,
    
};


/**
 * @brief Data about a type of action that can be performed in the game.
 * This data is pertinent only to Pikifen, not the library.
 */
struct PfePlayerActionType : public PlayerActionType {

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
    string internal_name;
    
    //String representing of this action type's default control bind.
    string default_bind_str;
    
};


/**
 * @brief Mediates everything control-related in Pikifen.
 */
struct ControlsMediator {

    public:
    
    //--- Function declarations ---
    
    void add_player_action_type(
        PLAYER_ACTION_TYPE id,
        PLAYER_ACTION_CAT category,
        const string &name,
        const string &description,
        const string &internal_name,
        const string &default_bind_str,
        float auto_repeat = 0.0f
    );
    const vector<PfePlayerActionType> &get_all_player_action_types() const;
    vector<ControlBind> &binds();
    string input_source_to_str(const PlayerInputSource &s) const;
    ControlBind find_bind(
        const PLAYER_ACTION_TYPE action_type_id
    ) const;
    ControlBind find_bind(
        const string &action_type_name
    ) const;
    PfePlayerActionType get_player_action_type(int action_id) const;
    string get_player_action_type_internal_name(int action_id);
    float get_player_action_type_value(
        PLAYER_ACTION_TYPE player_action_type_id
    );
    void load_binds_from_data_node(DataNode* node, unsigned char player_nr);
    PlayerInputSource str_to_input_source(const string &s) const;
    PlayerInput allegro_event_to_input(const ALLEGRO_EVENT &ev) const;
    bool handle_allegro_event(const ALLEGRO_EVENT &ev);
    vector<PlayerAction> new_frame(float delta_t);
    void release_all();
    void save_binds_to_data_node(DataNode* node, unsigned char player_nr);
    void set_options(const ControlsManagerOptions &options);
    void start_ignoring_actions();
    void start_ignoring_input_source(const PlayerInputSource &input_source);
    void stop_ignoring_actions();
    
    private:
    
    //--- Members ---
    
    //List of registered player action types.
    vector<PfePlayerActionType> player_action_types;
    
    //Controls manager.
    ControlsManager mgr;
    
};
