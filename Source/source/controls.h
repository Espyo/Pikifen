/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the control-related classes and functions.
 * This refers to both hardware input receiving
 * and the corresponding in-game actions.
 */

#ifndef CONTROLS_INCLUDED
#define CONTROLS_INCLUDED

#include <functional>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "controls_manager.h"


using std::size_t;
using std::string;
using std::vector;


//List of player action types.
enum PLAYER_ACTION_TYPES {
    //None.
    PLAYER_ACTION_NONE,
    //Throw.
    PLAYER_ACTION_THROW,
    //Whistle.
    PLAYER_ACTION_WHISTLE,
    //Move right.
    PLAYER_ACTION_RIGHT,
    //Move up.
    PLAYER_ACTION_UP,
    //Move left.
    PLAYER_ACTION_LEFT,
    //Move down.
    PLAYER_ACTION_DOWN,
    //Move cursor right.
    PLAYER_ACTION_CURSOR_RIGHT,
    //Move cursor up.
    PLAYER_ACTION_CURSOR_UP,
    //Move cursor left.
    PLAYER_ACTION_CURSOR_LEFT,
    //Move cursor down.
    PLAYER_ACTION_CURSOR_DOWN,
    //Swarm group right.
    PLAYER_ACTION_GROUP_RIGHT,
    //Swarm group up.
    PLAYER_ACTION_GROUP_UP,
    //Swarm group left.
    PLAYER_ACTION_GROUP_LEFT,
    //Swarm group down.
    PLAYER_ACTION_GROUP_DOWN,
    //Swarm group towards cursor.
    PLAYER_ACTION_GROUP_CURSOR,
    //Swap to next leader.
    PLAYER_ACTION_NEXT_LEADER,
    //Swap to previous leader.
    PLAYER_ACTION_PREV_LEADER,
    //Dismiss.
    PLAYER_ACTION_DISMISS,
    //Use spray #1.
    PLAYER_ACTION_USE_SPRAY_1,
    //Use spray #2.
    PLAYER_ACTION_USE_SPRAY_2,
    //Use currently selected spray.
    PLAYER_ACTION_USE_SPRAY,
    //Swap to next spray.
    PLAYER_ACTION_NEXT_SPRAY,
    //Swap to previous spray.
    PLAYER_ACTION_PREV_SPRAY,
    //Change zoom level.
    PLAYER_ACTION_CHANGE_ZOOM,
    //Zoom in.
    PLAYER_ACTION_ZOOM_IN,
    //Zoom out.
    PLAYER_ACTION_ZOOM_OUT,
    //Swap to next standby type.
    PLAYER_ACTION_NEXT_TYPE,
    //Swap to previous standby type.
    PLAYER_ACTION_PREV_TYPE,
    //Swap to next standby type maturity.
    PLAYER_ACTION_NEXT_MATURITY,
    //Swap to previous standby type maturity.
    PLAYER_ACTION_PREV_MATURITY,
    //Lie down.
    PLAYER_ACTION_LIE_DOWN,
    //Pause.
    PLAYER_ACTION_PAUSE,
    //Menu navigation right.
    PLAYER_ACTION_MENU_RIGHT,
    //Menu navigation up.
    PLAYER_ACTION_MENU_UP,
    //Menu navigation left.
    PLAYER_ACTION_MENU_LEFT,
    //Menu navigation down.
    PLAYER_ACTION_MENU_DOWN,
    //Menu navigation OK.
    PLAYER_ACTION_MENU_OK,
    //Menu navigation back.
    PLAYER_ACTION_MENU_BACK,
    
    //Total amount of player action types.
    N_PLAYER_ACTIONS,
};


/* ----------------------------------------------------------------------------
 * Data about a type of action that can be performed in the game.
 */
struct player_action_type {
    //ID of the action type.
    PLAYER_ACTION_TYPES id;
    //Name, for use in stuff like the options menu.
    string name;
    //Its name in the options file.
    string internal_name;
    //String representing of this action type's default control binding.
    string default_binding_str;
    player_action_type();
};


/* ----------------------------------------------------------------------------
 * Manager for the types of player actions in the game.
 */
struct player_action_type_manager {
    //List of known action types.
    vector<player_action_type> list;
    
    void add(
        const PLAYER_ACTION_TYPES id,
        const string &name, const string &internal_name,
        const string &default_binding_str
    );
    string binding_to_str(const control_binding &b) const;
    void feed_event_to_controls_manager(const ALLEGRO_EVENT &ev);
    control_binding find_binding(
        const PLAYER_ACTION_TYPES action_type_id
    ) const;
    control_binding find_binding(
        const string &action_type_name
    ) const;
    control_binding str_to_binding(const string &s) const;
};

#endif //ifndef CONTROLS_INCLUDED
