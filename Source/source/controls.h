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

#ifndef CONTROLS_INCLUDED
#define CONTROLS_INCLUDED

#include <functional>
#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "libs/controls_manager.h"


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
    //String representing of this action type's default control bind.
    string default_bind_str;
    player_action_type();
};


/* ----------------------------------------------------------------------------
 * Mediates everything control-related in Pikifen.
 */
struct controls_mediator {
public:
    //Player action type functions.
    void add_player_action_type(
        const PLAYER_ACTION_TYPES id,
        const string &name,
        const string &internal_name,
        const string &default_bind_str
    );
    void clear_player_action_types();
    const vector<player_action_type> &get_all_player_action_types() const;
    
    //Control bind functions.
    void add_bind(const control_bind &bind);
    string bind_to_str(const control_bind &b) const;
    void clear_binds();
    control_bind find_bind(
        const PLAYER_ACTION_TYPES action_type_id
    ) const;
    control_bind find_bind(
        const string &action_type_name
    ) const;
    const vector<control_bind> &get_all_binds() const;
    control_bind str_to_bind(const string &s) const;
    
    //Event loop functions.
    bool handle_allegro_event(const ALLEGRO_EVENT &ev);
    void new_frame();
    bool read_action(player_action &action);
    
private:
    //List of known player action types.
    vector<player_action_type> player_action_types;
    //Controls manager.
    controls_manager mgr;
    
};


#endif //ifndef CONTROLS_INCLUDED
