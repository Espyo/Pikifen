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

#include <allegro5/allegro.h>

#include "../lib/controls_manager/controls_manager.h"


using std::size_t;
using std::string;
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
    
};


/**
 * @brief Data about a type of action that can be performed in the game.
 */
struct player_action_type {

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
struct controls_mediator {

    public:
    
    //--- Function declarations ---
    
    void add_player_action_type(
        const PLAYER_ACTION_TYPE id,
        const PLAYER_ACTION_CAT category,
        const string &name,
        const string &description,
        const string &internal_name,
        const string &default_bind_str
    );
    const vector<player_action_type> &get_all_player_action_types() const;
    vector<control_bind> &binds();
    string input_to_str(const player_input &b) const;
    control_bind find_bind(
        const PLAYER_ACTION_TYPE action_type_id
    ) const;
    control_bind find_bind(
        const string &action_type_name
    ) const;
    player_action_type get_player_action_type(int action_id) const;
    string get_player_action_type_internal_name(int action_id);
    float get_player_action_type_value(
        PLAYER_ACTION_TYPE player_action_type_id
    );
    player_input str_to_input(const string &s) const;
    player_input allegro_event_to_input(const ALLEGRO_EVENT &ev) const;
    bool handle_allegro_event(const ALLEGRO_EVENT &ev);
    vector<player_action> new_frame();
    void release_all();
    void set_options(const controls_manager_options &options);
    
    private:
    
    //--- Members ---
    
    //List of known player action types.
    vector<player_action_type> player_action_types;
    
    //Controls manager.
    controls_manager mgr;
    
};
