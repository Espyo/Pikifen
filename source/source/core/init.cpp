/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program initializer and uninitializer functions.
 */

#include <algorithm>
#include <csignal>

#include <allegro5/allegro.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include "init.hpp"

#include "../content/mob_category/bouncer_category.hpp"
#include "../content/mob_category/bridge_category.hpp"
#include "../content/mob_category/converter_category.hpp"
#include "../content/mob_category/custom_category.hpp"
#include "../content/mob_category/decoration_category.hpp"
#include "../content/mob_category/drop_category.hpp"
#include "../content/mob_category/enemy_category.hpp"
#include "../content/mob_category/group_task_category.hpp"
#include "../content/mob_category/interactable_category.hpp"
#include "../content/mob_category/leader_category.hpp"
#include "../content/mob_category/onion_category.hpp"
#include "../content/mob_category/pellet_category.hpp"
#include "../content/mob_category/pikmin_category.hpp"
#include "../content/mob_category/pile_category.hpp"
#include "../content/mob_category/resource_category.hpp"
#include "../content/mob_category/scale_category.hpp"
#include "../content/mob_category/ship_category.hpp"
#include "../content/mob_category/tool_category.hpp"
#include "../content/mob_category/track_category.hpp"
#include "../content/mob_category/treasure_category.hpp"
#include "../content/script/script.hpp"
#include "../content/script/script_utils.hpp"
#include "../game_state/game_state.hpp"
#include "../game_state/gameplay/gameplay.hpp"
#include "../lib/imgui/imgui_impl_allegro5.h"
#include "../util/allegro_utils.hpp"
#include "../util/general_utils.hpp"
#include "../util/imgui_utils.hpp"
#include "../util/string_utils.hpp"
#include "controls_mediator.hpp"
#include "game.hpp"
#include "misc_functions.hpp"


#pragma region Destruction


/**
 * @brief Destroys Allegro and modules.
 */
void destroyAllegro() {
    al_uninstall_joystick();
    al_uninstall_audio();
    al_uninstall_keyboard();
    al_uninstall_mouse();
    al_uninstall_system();
}


/**
 * @brief Destroys Allegro's event-related things.
 *
 * @param mainTimer The main game timer.
 * @param eventQueue Queue of Allegro events.
 */
void destroyEventThings(
    ALLEGRO_TIMER*& mainTimer, ALLEGRO_EVENT_QUEUE*& eventQueue
) {
    al_destroy_event_queue(eventQueue);
    al_destroy_timer(mainTimer);
    al_destroy_display(game.display);
}


/**
 * @brief Destroys miscellaneous things.
 */
void destroyMisc() {
    al_destroy_bitmap(game.bmpError);
    game.audio.destroy();
    
    forIdx(v, game.missionMetricTypes) {
        delete game.missionMetricTypes[v];
    }
    game.missionMetricTypes.clear();
    
    forIdx(e, game.missionEndCondTypes) {
        delete game.missionEndCondTypes[e];
    }
    game.missionEndCondTypes.clear();
}


/**
 * @brief Destroys registered mob categories.
 */
void destroyMobCategories() {
    game.mobCategories.clear();
}


#pragma endregion
#pragma region Initialization


/**
 * @brief Initializes Allegro and its modules.
 */
void initAllegro() {
    if(!al_init()) {
        reportFatalError("Could not initialize Allegro!");
    }
    if(!al_install_mouse()) {
        reportFatalError("Could not install the Allegro mouse module!");
    }
    if(!al_install_keyboard()) {
        reportFatalError("Could not install the Allegro keyboard module!");
    }
    if(!al_install_audio()) {
        reportFatalError("Could not install the Allegro audio module!");
    }
    if(!al_init_image_addon()) {
        reportFatalError("Could not initialize the Allegro image addon!");
    }
    if(!al_init_primitives_addon()) {
        reportFatalError(
            "Could not initialize the Allegro primitives addon!"
        );
    }
    if(!al_init_acodec_addon()) {
        reportFatalError(
            "Could not initialize the Allegro audio codec addon!"
        );
    }
    if(!al_init_font_addon()) {
        reportFatalError(
            "Could not initialize the Allegro font addon!"
        );
    }
    if(!al_init_ttf_addon()) {
        reportFatalError(
            "Could not initialize the Allegro TTF font addon!"
        );
    }
    if(!al_install_joystick()) {
        reportFatalError(
            "Could not initialize Allegro joystick support!"
        );
    }
}


/**
 * @brief Initializes things related to the controls.
 */
void initControls() {
    //Register the existing actions.
    //They must be registered in the same order as the action types enum.
    
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_NONE,
        PLAYER_ACTION_CAT_NONE,
        "---", "", "", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    
    //Main.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Move right",
        "Move the leader right.",
        "move_right", "k_4", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Move down",
        "Move the leader down.",
        "move_down", "k_19", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Move left",
        "Move the leader left.",
        "move_left", "k_1", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Move up",
        "Move the leader up.",
        "move_up", "k_23", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor right",
        "Move the leader's cursor right.",
        "cursor_right", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor down",
        "Move the leader's cursor down.",
        "cursor_down", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor left",
        "Move the leader's cursor left.",
        "cursor_left", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor up",
        "Move the leader's cursor up.",
        "cursor_up", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_THROW,
        PLAYER_ACTION_CAT_MAIN,
        "Throw",
        "Throw a Pikmin at the leader's cursor.",
        "throw", "mb_1", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.2f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_WHISTLE,
        PLAYER_ACTION_CAT_MAIN,
        "Whistle",
        "Whistle around the leader's cursor.",
        "whistle", "mb_2", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_NEXT_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Next Pikmin",
        "Change to the next Pikmin type in the group.",
        "next_type", "mwd", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_PREV_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. Pikmin",
        "Change to the previous Pikmin type in the group.",
        "prev_type", "mwu", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_NEXT_LEADER,
        PLAYER_ACTION_CAT_MAIN,
        "Next leader",
        "Change to the next leader.",
        "next_leader", "k_215", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_GROUP_CURSOR,
        PLAYER_ACTION_CAT_MAIN,
        "Swarm to cursor",
        "Swarm all Pikmin towards the leader's cursor.",
        "swarm_cursor", "k_75", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_DISMISS,
        PLAYER_ACTION_CAT_MAIN,
        "Dismiss",
        "Dismiss all Pikmin.",
        "dismiss", "k_217", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_INVENTORY,
        PLAYER_ACTION_CAT_MAIN,
        "Open inventory",
        "Open the item and special move inventory.",
        "inventory", "k_17", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_PAUSE,
        PLAYER_ACTION_CAT_MAIN,
        "Pause",
        "Pause the game.",
        "pause", "k_59", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, true
    );
    
    //Menus.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu right",
        "Navigate right in a menu.",
        "menu_right", "k_83", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Menu down",
        "Navigate down in a menu.",
        "menu_down", "k_85", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu left",
        "Navigate left in a menu.",
        "menu_left", "k_82", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Menu up",
        "Navigate up in a menu.",
        "menu_up", "k_84", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_OK,
        PLAYER_ACTION_CAT_MENUS,
        "Menu OK",
        "Confirm the selected item in a menu.",
        "menu_ok", "k_67", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.4f, false, true
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan right",
        "Pan the radar to the right.",
        "menu_radar_right", "k_4", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan down",
        "Pan the radar downward.",
        "menu_radar_down", "k_19", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan left",
        "Pan the radar to the left.",
        "menu_radar_left", "k_1", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan up",
        "Pan the radar upward.",
        "menu_radar_up", "k_23", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom in",
        "Zoom the radar in.",
        "menu_radar_zoom_in", "k_18", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom out",
        "Zoom the radar out.",
        "menu_radar_zoom_out", "k_6", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_ONION_CHANGE_10,
        PLAYER_ACTION_CAT_MENUS,
        "Onion menu change 10",
        "Toggle the \"change 10\" mode in the Onion menu.",
        "menu_onion_change_10", "k_215", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_ONION_SELECT_ALL,
        PLAYER_ACTION_CAT_MENUS,
        "Onion menu select all",
        "Toggle the \"select all\" mode in the Onion menu.",
        "menu_onion_select_all", "k_217", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    
    //Advanced.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_GROUP_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm right",
        "Swarm all Pikmin right.",
        "swarm_right", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_GROUP_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm down",
        "Swarm all Pikmin down.",
        "swarm_down", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_GROUP_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm left",
        "Swarm all Pikmin left.",
        "swarm_left", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_GROUP_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm up",
        "Swarm all Pikmin up.",
        "swarm_up", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_PREV_LEADER,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. leader",
        "Change to the previous leader.",
        "prev_leader", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_CAT_ADVANCED,
        "Change zoom",
        "Change the current zoom level.",
        "change_zoom", "k_3", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom in",
        "Change to a closer zoom level.",
        "zoom_in", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_ZOOM_OUT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom out",
        "Change to a farther zoom level.",
        "zoom_out", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_NEXT_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Next maturity",
        "Change to a Pikmin of the next maturity.",
        "next_maturity", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_PREV_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. maturity",
        "Change to a Pikmin of the previous maturity.",
        "prev_maturity", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut A",
        "Use the inventory item set to shortcut A.",
        "inventory_shortcut_a", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut B",
        "Use the inventory item set to shortcut B.",
        "inventory_shortcut_b", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut C",
        "Use the inventory item set to shortcut C.",
        "inventory_shortcut_c", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_D,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut D",
        "Use the inventory item set to shortcut D.",
        "inventory_shortcut_d", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_CUSTOM_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom A",
        "Custom action A, if the current leader supports it.",
        "custom_a", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_CUSTOM_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom B",
        "Custom action B, if the current leader supports it.",
        "custom_b", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_CUSTOM_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom C",
        "Custom action C, if the current leader supports it.",
        "custom_c", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_RADAR,
        PLAYER_ACTION_CAT_ADVANCED,
        "Radar",
        "Open or close the radar.",
        "radar", "k_64", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_BACK,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - back",
        "Go back or cancel in a menu.",
        "menu_back", "k_59", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.4f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - left page",
        "Go to the page to the left in a menu.",
        "menu_page_left", "k_17", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - right page",
        "Go to the page to the right in a menu.",
        "menu_page_right", "k_5", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    
    //General maker tool things.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_CONSOLE,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Console",
        "Open or close the maker toolkit's console.",
        "mt_console", "k_70", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_AUTO_START,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Auto-start",
        "Make the game auto-start on the current state (and content).",
        "mt_auto_start", "k_56", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Set song pos near loop",
        "Set the current song's position to be near the loop point.",
        "mt_set_song_pos_near_loop", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_MOD_1,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 1",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_1", "k_216", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_MOD_2,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 2",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_2", "k_218", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    
    //Gameplay maker tools.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Area image",
        "Save an image of the current area.",
        "mt_area_image", "k_36", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_AREA_INSPECTOR,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Area inspector",
        "Toggle info about the current area.",
        "mt_area_inspector", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Change speed",
        "Change the gameplay speed.",
        "mt_change_speed", "k_28", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_DELETE_MOB,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Delete mob",
        "Delete the inspected mob.",
        "mt_delete_mob", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_FILL_INVENTORY,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Fill inventory",
        "Set the amount of all sprays to 99. "
        "Use \"modifier 1\" to set to 0.",
        "mt_fill_inventory", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_FRAME_ADVANCE,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Frame advance",
        "Pause gameplay and advance by one frame each time. "
        "Use \"modifier 1\" to unpause.",
        "mt_frame_advance", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_FREE_CAM,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Free camera",
        "Allows control of the camera. "
        "Use \"modifier 1\" to fix the camera and keep playing.",
        "mt_free_cam", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Geometry info",
        "Toggle info about the geometry under the mouse cursor.",
        "mt_geometry_info", "k_33", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_HIDE_HUD,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Hide HUD",
        "Toggle the visibility of the HUD.",
        "mt_hud", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_HURT_MOB,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Hurt mob",
        "Hurt the mob under the mouse cursor.",
        "mt_hurt_mob", "k_30", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_MOB_INSPECTOR,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Mob inspector",
        "Toggle info about the mob under the mouse cursor.",
        "mt_mob_inspector", "k_32", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_NEW_PIKMIN,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "New Pikmin",
        "Create a new Pikmin under the mouse cursor.",
        "mt_new_pikmin", "k_31", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_NEW_REMINDER,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "New reminder",
        "Create a new area maker reminder under the mouse cursor.",
        "mt_new_reminder", "k_35", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_PATH_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Path info",
        "Toggle info about paths the inspected mob is taking.",
        "mt_path_info", "k_34", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_COLLISION,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show collision",
        "Toggle drawing each mob's collision.",
        "mt_show_collision", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show hitboxes",
        "Toggle drawing each mob's hitboxes.",
        "mt_show_hitboxes", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_REACHES,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show reaches",
        "Toggle drawing the inspected mob's reaches.",
        "mt_show_reaches", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_TELEPORT,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Teleport",
        "Teleport the leader to the mouse cursor.",
        "mt_teleport", "k_29", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    
    //System.
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_SYSTEM_INFO,
        PLAYER_ACTION_CAT_SYSTEM,
        "System info",
        "Toggle showing system and performance information.",
        "system_info", "k_47", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_SCREENSHOT,
        PLAYER_ACTION_CAT_SYSTEM,
        "Take a screenshot",
        "Take a screenshot and save it in the user data folder.",
        "screenshot", "k_58", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    
    
    //Populate the control binds with some default control binds for player 1.
    //If the options are loaded successfully, these binds are overwritten.
    const vector<PlayerActionType>& actionTypes =
        game.controls.getAllActionTypes();
    forIdx(a, actionTypes) {
        const string& def = actionTypes[a].defaultBindStr;
        if(def.empty()) continue;
        
        Inpution::Bind bind;
        bind.actionTypeId = actionTypes[a].id;
        bind.playerNr = 0;
        bind.inputSource = game.controls.strToInputSource(def);
        game.controls.binds().push_back(bind);
    }
}


/**
 * @brief Initializes Dear ImGui.
 */
void initDearImGui() {
    //Misc. setup.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplAllegro5_Init(game.display);
    ImGui::GetIO().IniFilename = "";
    ImGui::GetIO().ConfigDragClickToInputText = true;
    
    //Fonts.
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig editorFontCfg;
    editorFontCfg.OversampleH = editorFontCfg.OversampleV = 1;
    editorFontCfg.PixelSnapH = true;
    
    const auto addFont =
    [ = ] (ImFont** targetVar, const string& assetInternalName, int height) {
        const string& path =
            game.content.bitmaps.manifests
            [assetInternalName].path;
            
        if(!strEndsWith(strToLower(path), ".ttf")) {
            game.errors.report(
                "Could not load the editor font \"" + path + "\"! Only "
                "TTF font files are allowed."
            );
            return;
        }
        
        *targetVar =
            io.Fonts->AddFontFromFileTTF(
                path.c_str(), height, &editorFontCfg
            );
    };
    
    addFont(
        &game.sysContent.fntDearImGuiHeader,
        game.sysContentNames.fntEditorHeader, 22
    );
    addFont(
        &game.sysContent.fntDearImGuiMonospace,
        game.sysContentNames.fntEditorMonospace, 18
    );
    addFont(
        &game.sysContent.fntDearImGuiStandard,
        game.sysContentNames.fntEditorStandard, 18
    );
    
    io.FontDefault = game.sysContent.fntDearImGuiStandard;
    
    //Other stuff.
    initDearImGuiColors();
}


/**
 * @brief Initializes the Dear ImGui color style.
 */
void initDearImGuiColors() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    //Since the default Dear ImGui style is based around blue,
    //we can shift the hue left to get an equivalent green.
    //These color indexes are the ones that really have any blue hue,
    //so only mess with these.
    vector<ImGuiCol_> colorsToChange {
        ImGuiCol_Border,
        ImGuiCol_BorderShadow,
        ImGuiCol_FrameBg,
        ImGuiCol_FrameBgHovered,
        ImGuiCol_FrameBgActive,
        ImGuiCol_TitleBgActive,
        ImGuiCol_CheckMark,
        ImGuiCol_SliderGrab,
        ImGuiCol_SliderGrabActive,
        ImGuiCol_Button,
        ImGuiCol_ButtonHovered,
        ImGuiCol_ButtonActive,
        ImGuiCol_Header,
        ImGuiCol_HeaderHovered,
        ImGuiCol_HeaderActive,
        ImGuiCol_Separator,
        ImGuiCol_SeparatorHovered,
        ImGuiCol_SeparatorActive,
        ImGuiCol_ResizeGrip,
        ImGuiCol_ResizeGripHovered,
        ImGuiCol_ResizeGripActive,
        ImGuiCol_TabHovered,
        ImGuiCol_Tab,
        ImGuiCol_TabSelected,
        ImGuiCol_TabSelectedOverline,
        ImGuiCol_TabDimmed,
        ImGuiCol_TabDimmedSelected,
        ImGuiCol_TabDimmedSelectedOverline,
        ImGuiCol_TextLink,
        ImGuiCol_TextSelectedBg,
        ImGuiCol_NavCursor,
    };
    
    forIdx(c, colorsToChange) {
        ImGui::AdjustColorHSV(
            style.Colors[colorsToChange[c]], -0.25f, 0.0f, 0.0f
        );
    }
    
    //Manually adjust some of them.
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_ButtonHovered], 0.0f, 0.0f, -0.30f
    );
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_ButtonActive], 0.0f, 0.0f, -0.24f
    );
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_SliderGrab], 0.0f, 0.0f, -0.30f
    );
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_SliderGrabActive], 0.0f, 0.0f, -0.24f
    );
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_HeaderHovered], 0.0f, 0.0f, -0.30f
    );
    ImGui::AdjustColorHSV(
        style.Colors[ImGuiCol_HeaderActive], 0.0f, 0.0f, -0.24f
    );
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.10f, 0.10f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.10f, 0.10f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.10f, 0.10f, 1.0f);
    
    //Finally, save the default style colors.
    for(size_t c = 0; c < ImGuiCol_COUNT; c++) {
        game.dearImGuiDefaultStyle[c] = style.Colors[c];
    }
}


/**
 * @brief Initializes the developer debug configuration.
 */
void initDebugConfig() {
    if(!game.options.advanced.engineDev) return;
    if(!al_filename_exists(FILE_PATHS_FROM_ROOT::DEBUG_CONFIG.c_str())) return;
    
    DataNode debugConfigFile(FILE_PATHS_FROM_ROOT::DEBUG_CONFIG);
    
    const auto getProp = [&debugConfigFile] (const string& name) {
        return
            s2b(
                debugConfigFile.getChildByName(
                    name
                )->getValueOrDefault("false")
            );
    };
    
    game.debug.showAreaActiveCells = getProp("show_area_active_cells");
    game.debug.showControllerEvents = getProp("show_controller_events");
    game.debug.showDearImGuiDemo = getProp("show_dear_imgui_demo");
    game.debug.showGroupInfo = getProp("show_group_info");
    game.debug.showPlayerActions = getProp("show_player_actions");
}


/**
 * @brief Initializes the error bitmap.
 */
void initErrorBitmap() {
    const ALLEGRO_COLOR ERROR_BLACK = al_map_rgba(0, 0, 0, 192);
    const ALLEGRO_COLOR ERROR_PINK = al_map_rgba(255, 0, 255, 192);
    //Error bitmap.
    game.bmpError = al_create_bitmap(32, 32);
    al_set_target_bitmap(game.bmpError); {
        al_clear_to_color(ERROR_BLACK);
        al_draw_filled_rectangle(0.0, 0.0, 16.0, 16.0, ERROR_PINK);
        al_draw_filled_rectangle(16.0, 16.0, 32.0, 32.0, ERROR_PINK);
    } al_set_target_backbuffer(game.display);
    game.bmpError = recreateBitmap(game.bmpError);
}


/**
 * @brief Initializes some essential things.
 */
void initEssentials() {
    //Signal handlers.
    signal(SIGFPE,  signalHandler);
    signal(SIGILL,  signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
}


/**
 * @brief Initializes things regarding Allegro events, like the queue,
 * timer, etc.
 *
 * @param mainTimer The main game timer.
 * @param eventQueue Queue of Allegro events.
 */
void initEventThings(
    ALLEGRO_TIMER*& mainTimer, ALLEGRO_EVENT_QUEUE*& eventQueue
) {
    al_set_new_display_flags(
        al_get_new_display_flags() |
        ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE
    );
    if(game.options.advanced.windowPosHack) al_set_new_window_position(64, 64);
    if(game.winFullscreen) {
        al_set_new_display_flags(
            al_get_new_display_flags() |
            (
                game.options.graphics.trueFullscreen ?
                ALLEGRO_FULLSCREEN :
                ALLEGRO_FULLSCREEN_WINDOW
            )
        );
    }
    game.display = al_create_display(game.winW, game.winH);
    
    //It's possible that this resolution is not valid for fullscreen.
    //Detect this and try again in windowed.
    if(!game.display && game.winFullscreen) {
        game.errors.report(
            "Could not create a fullscreen window with the resolution " +
            i2s(game.winW) + "x" + i2s(game.winH) + ". "
            "Setting the fullscreen option back to false. "
            "You can try a different resolution, "
            "preferably one from the options menu."
        );
        game.winFullscreen = false;
        game.options.graphics.intendedWinFullscreen = false;
        saveOptions();
        al_set_new_display_flags(
            al_get_new_display_flags() & ~ALLEGRO_FULLSCREEN
        );
        game.display = al_create_display(game.winW, game.winH);
    }
    
    if(!game.display) {
        reportFatalError("Could not create a display!");
    }
    
    //For some reason some resolutions aren't properly created under Windows.
    //This hack fixes it.
    al_resize_display(game.display, game.winW, game.winH);
    
    mainTimer = al_create_timer(1.0f / game.options.advanced.targetFps);
    if(!mainTimer) {
        reportFatalError("Could not create the main game timer!");
    }
    
    eventQueue = al_create_event_queue();
    if(!eventQueue) {
        reportFatalError("Could not create the main event queue!");
    }
    al_register_event_source(eventQueue, al_get_mouse_event_source());
    al_register_event_source(eventQueue, al_get_keyboard_event_source());
    al_register_event_source(eventQueue, al_get_joystick_event_source());
    al_register_event_source(
        eventQueue, al_get_display_event_source(game.display)
    );
    al_register_event_source(
        eventQueue, al_get_timer_event_source(mainTimer)
    );
}


/**
 * @brief Initializes the list of FSM event types. This only initializes the
 * external, script file events. Internal ones are not accounted for.
 */
void initFsmEventTypes() {
    game.fsmEventTypes.assign(N_FSM_EVENTS, FsmEventType());
    Bitmask8 contexts = 0;
    
    auto commitEvent =
    [&contexts] (FSM_EV eventType, const string eventName) {
        FsmEventType* eventTypePtr;
        eventTypePtr = &(game.fsmEventTypes[eventType]);
        eventTypePtr->type = eventType;
        eventTypePtr->name = eventName;
        eventTypePtr->contexts = contexts;
    };
    
    
    //-Common events-
    contexts =
        getIdxBitmask(SCRIPT_CONTEXT_MOB) |
        getIdxBitmask(SCRIPT_CONTEXT_AREA);
        
        
    //Unknown.
    commitEvent(
        FSM_EV_UNKNOWN,
        "unknown"
    );
    
    //On enter.
    commitEvent(
        FSM_EV_ON_ENTER,
        "on_enter"
    );
    
    //On leave.
    commitEvent(
        FSM_EV_ON_LEAVE,
        "on_leave"
    );
    
    //Focus off reach.
    commitEvent(
        FSM_EV_FOCUS_OFF_REACH,
        "on_focus_off_reach"
    );
    
    //Input received.
    commitEvent(
        FSM_EV_INPUT_RECEIVED,
        "on_input_received"
    );
    
    //Receive script message.
    commitEvent(
        FSM_EV_RECEIVE_SCRIPT_MESSAGE,
        "on_receive_message"
    );
    
    //On tick.
    commitEvent(
        FSM_EV_ON_TICK,
        "on_tick"
    );
    
    //Timer.
    commitEvent(
        FSM_EV_TIMER,
        "on_timer"
    );
    
    
    //-Mob events-
    contexts = getIdxBitmask(SCRIPT_CONTEXT_MOB);
    
    
    //DEPRECATED in 1.2.0 by the "ready" block.
    //On ready.
    commitEvent(
        FSM_EV_ON_READY,
        "on_ready"
    );
    
    //Active leader changed.
    commitEvent(
        FSM_EV_ACTIVE_LEADER_CHANGED,
        "on_active_leader_changed"
    );
    
    //Animation end.
    commitEvent(
        FSM_EV_ANIMATION_END,
        "on_animation_end"
    );
    
    //Damage.
    commitEvent(
        FSM_EV_DAMAGE,
        "on_damage"
    );
    
    //Far from home.
    commitEvent(
        FSM_EV_FAR_FROM_HOME,
        "on_far_from_home"
    );
    
    //Finished receiving delivery.
    commitEvent(
        FSM_EV_FINISHED_RECEIVING_DELIVERY,
        "on_finish_receiving_delivery"
    );
    
    //Frame signal.
    commitEvent(
        FSM_EV_FRAME_SIGNAL,
        "on_frame_signal"
    );
    
    //Held.
    commitEvent(
        FSM_EV_HELD,
        "on_held"
    );
    
    //Hitbox touch eat.
    commitEvent(
        FSM_EV_HITBOX_TOUCH_EAT,
        "on_hitbox_touch_eat"
    );
    
    //Hitbox touch attack-normal.
    commitEvent(
        FSM_EV_HITBOX_TOUCH_A_N,
        "on_hitbox_touch_a_n"
    );
    
    //Hitbox touch normal-normal.
    commitEvent(
        FSM_EV_HITBOX_TOUCH_N_N,
        "on_hitbox_touch_n_n"
    );
    
    //Itch.
    commitEvent(
        FSM_EV_ITCH,
        "on_itch"
    );
    
    //Landed.
    commitEvent(
        FSM_EV_LANDED,
        "on_land"
    );
    
    //Left hazard.
    commitEvent(
        FSM_EV_LEFT_HAZARD,
        "on_leave_hazard"
    );
    
    //Object in reach.
    commitEvent(
        FSM_EV_OBJECT_IN_REACH,
        "on_object_in_reach"
    );
    
    //Opponent in reach.
    commitEvent(
        FSM_EV_OPPONENT_IN_REACH,
        "on_opponent_in_reach"
    );
    
    //Thrown Pikmin landed.
    commitEvent(
        FSM_EV_THROWN_PIKMIN_LANDED,
        "on_pikmin_land"
    );
    
    //Released.
    commitEvent(
        FSM_EV_RELEASED,
        "on_released"
    );
    
    //Reached destination.
    commitEvent(
        FSM_EV_REACHED_DESTINATION,
        "on_reach_destination"
    );
    
    //Receiving delivery.
    commitEvent(
        FSM_EV_STARTED_RECEIVING_DELIVERY,
        "on_start_receiving_delivery"
    );
    
    //Swallowed.
    commitEvent(
        FSM_EV_SWALLOWED,
        "on_swallowed"
    );
    
    //Touched hazard.
    commitEvent(
        FSM_EV_TOUCHED_HAZARD,
        "on_touch_hazard"
    );
    
    //Touched object.
    commitEvent(
        FSM_EV_TOUCHED_OBJECT,
        "on_touch_object"
    );
    
    //Touched opponent.
    commitEvent(
        FSM_EV_TOUCHED_OPPONENT,
        "on_touch_opponent"
    );
    
    //Touched wall.
    commitEvent(
        FSM_EV_TOUCHED_WALL,
        "on_touch_wall"
    );
    
    //Weight added.
    commitEvent(
        FSM_EV_WEIGHT_ADDED,
        "on_weight_added"
    );
    
    //Weight removed.
    commitEvent(
        FSM_EV_WEIGHT_REMOVED,
        "on_weight_removed"
    );
}


/**
 * @brief Initializes the list of inventory items.
 */
void initInventoryItems() {
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPRAY_TYPE,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    game.inventoryItems.init();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}


/**
 * @brief Initializes the list of maker tool types.
 */
void initMakerToolTypes() {
    game.makerTools.types.assign(N_MAKER_TOOLS, MakerToolType());
    vector<CommandParam> params;
    Bitmask8 contexts = 0;
    
    //Convenient lambdas and aliases.
    auto queueParam =
        [&params] (
            const string& paramName, COMMAND_PARAM_TYPE paramType,
            Bitmask8 paramFlags = 0, const string& paramDefValue = ""
    ) {
        params.push_back(
            CommandParam(
                paramName, paramType, paramFlags, paramDefValue
            )
        );
    };
    
    auto commitTool =
        [&params, &contexts] (
            MAKER_TOOL_TYPE toolType, const string& toolName,
            bool helpful, MakerToolTypeCode * toolRunCode,
            const string& description
    ) {
        validateCommandParams(
            params, "Maker tool type \"" + toolName + "\""
        );
        
        MakerToolType* toolTypePtr;
        toolTypePtr = &(game.makerTools.types[toolType]);
        toolTypePtr->type = toolType;
        toolTypePtr->name = toolName;
        toolTypePtr->code = toolRunCode;
        toolTypePtr->parameters = params;
        toolTypePtr->contexts = contexts;
        toolTypePtr->helpful = helpful;
        toolTypePtr->description = description;
        params.clear();
    };
    
    const COMMAND_PARAM_TYPE ptInt = COMMAND_PARAM_TYPE_INT;
    const COMMAND_PARAM_TYPE ptFloat = COMMAND_PARAM_TYPE_FLOAT;
    const COMMAND_PARAM_TYPE ptBool = COMMAND_PARAM_TYPE_BOOL;
    const COMMAND_PARAM_TYPE ptString = COMMAND_PARAM_TYPE_STRING;
    const COMMAND_PARAM_TYPE ptEnum = COMMAND_PARAM_TYPE_ENUM;
    const COMMAND_PARAM_FLAG pfOpt = COMMAND_PARAM_FLAG_OPTIONAL;
    
    
    //-Anywhere tools-
    contexts =
        getIdxBitmask(MAKER_TOOL_CONTEXT_ANYWHERE) |
        getIdxBitmask(MAKER_TOOL_CONTEXT_GAMEPLAY);
        
    //Unknown.
    commitTool(
        MAKER_TOOL_TYPE_NONE,
        "none", false,
        nullptr, ""
    );
    
    //Set auto-start data.
    commitTool(
        MAKER_TOOL_TYPE_SET_AUTO_START,
        "set_auto_start", false,
        MakerToolRunners::setAutoStart,
        "Sets Pikifen to auto-start on the state and content you're "
        "currently on."
    );
    
    
    //-Gameplay tools-
    contexts =
        getIdxBitmask(MAKER_TOOL_CONTEXT_GAMEPLAY);
        
    //Area image.
    queueParam("size", ptInt, pfOpt, "2048");
    params.back().minValue = 1.0f;
    params.back().description =
        "Maximum image width or height, in pixels. "
        "2048 is a good size for medium or large areas, "
        "otherwise 1024 works fine.";
    queueParam("padding", ptInt, pfOpt, "32");
    params.back().minValue = 0.0f;
    params.back().description =
        "Padding around the actual area. "
        "This is measured in area pixels, and is the total for "
        "either dimension. e.g. 32 means 16 area pixels above "
        "the area and 16 area pixels below the area will be "
        "included; same for 16 pixels to the left and 16 to the right.";
    queueParam("shadows", ptBool, pfOpt, "true");
    params.back().description =
        "Whether you want to see tree shadows.";
    commitTool(
        MAKER_TOOL_TYPE_AREA_IMAGE,
        "area_image", true,
        MakerToolRunners::areaImage,
        "Creates an image of the entire area."
    );
    
    //Area inspector.
    commitTool(
        MAKER_TOOL_TYPE_AREA_INSPECTOR,
        "area_inspector", true,
        MakerToolRunners::areaInspector,
        "Toggles showing information about the current area and its script."
    );
    
    //Change speed.
    queueParam("multiplier", ptFloat, pfOpt, "2");
    params.back().minValue = 0.001f;
    params.back().description =
        "Game speed multiplier. "
        "1 is normal speed, 0.5 is half speed, 2.0 is double speed, etc.";
    commitTool(
        MAKER_TOOL_TYPE_CHANGE_SPEED,
        "change_speed", true,
        MakerToolRunners::changeSpeed,
        "Changes the gameplay speed."
    );
    
    //Delete mob.
    commitTool(
        MAKER_TOOL_TYPE_DELETE_MOB,
        "delete_mob", true,
        MakerToolRunners::deleteMob,
        "Deletes the inspected mob."
    );
    
    //Fill inventory.
    queueParam("amount", ptInt, pfOpt, "99");
    params.back().minValue = 0.0f;
    params.back().description =
        "Amount to set all inventory items to.";
    commitTool(
        MAKER_TOOL_TYPE_FILL_INVENTORY,
        "fill_inventory", true,
        MakerToolRunners::fillInventory,
        "Changes the amount of all inventory items to a given amount."
    );
    
    //Frame advance.
    queueParam("action", ptEnum, pfOpt, "advance");
    params.back().enumValues = enumGetNames(makerToolFrameAdvanceActionINames);
    params.back().description =
        "Action to perform. \"advance\" pauses gameplay if unpaused, otherwise "
        "advances one frame. \"resume\" resumes normal gameplay.";
    commitTool(
        MAKER_TOOL_TYPE_FRAME_ADVANCE,
        "frame_advance", true,
        MakerToolRunners::frameAdvance,
        "Pauses gameplay and advances only one frame at a time."
    );
    
    //Free cam.
    queueParam("action", ptEnum, pfOpt, "control");
    params.back().enumValues = enumGetNames(makerToolFreeCamActionINames);
    params.back().description =
        "Action to perform. \"mode\" toggles free cam mode. "
        "\"freeze\" toggles whether the free camera is frozen "
        "(player controls the leader) or mobile "
        "(player controls the camera).";
    commitTool(
        MAKER_TOOL_TYPE_FREE_CAM,
        "free_cam", true,
        MakerToolRunners::freeCam,
        "Allows control of the camera. Includes freezing it in place as "
        "the player controls the leader."
    );
    
    //Geometry info.
    commitTool(
        MAKER_TOOL_TYPE_GEOMETRY_INFO,
        "geometry_info", true,
        MakerToolRunners::geometryInfo,
        "Toggles showing information about the area layout data under "
        "the mouse cursor, as well as information about the cursor proper."
    );
    
    //Hide HUD.
    commitTool(
        MAKER_TOOL_TYPE_HIDE_HUD,
        "hide_hud", false,
        MakerToolRunners::hideHud,
        "Toggles visibility of the HUD."
    );
    
    //Hurt mob.
    queueParam("percentage", ptFloat, pfOpt, "0.5");
    params.back().description =
        "Ratio of damage. 0.5 means it loses 50% of its max health. "
        "1.0 means it loses all its health, -0.2 means it regains 20% of its "
        "max health, etc.";
    commitTool(
        MAKER_TOOL_TYPE_HURT_MOB,
        "hurt_mob", true,
        MakerToolRunners::hurtMob,
        "Hurts or heals the mob closest to the mouse cursor."
    );
    
    //Mob inspector.
    queueParam("action", ptEnum, pfOpt, "closest");
    params.back().enumValues = enumGetNames(makerToolMobInspectorActionINames);
    params.back().description =
        "Action to perform. \"closest\" makes it detect the mob closest "
        "to the mouse cursor. \"iterate\" makes it detect mobs near the "
        "mouse cursor or swap to the next one in that list. \"stop\" makes "
        "it stop inspecting.";
    commitTool(
        MAKER_TOOL_TYPE_MOB_INSPECTOR,
        "mob_inspector", true,
        MakerToolRunners::mobInspector,
        "Inspects a mob, showing information about itself and its script."
    );
    
    //New Pikmin.
    queueParam("type", ptString, pfOpt, "next");
    params.back().description =
        "Type of Pikmin to spawn. This can be a proper type's internal name, "
        "or \"next\" to spawn one of the next type from the last spawned one, "
        "or \"same\" to spawn one of the same type as the last spawned one.";
    queueParam("maturity", ptEnum, pfOpt, "flower");
    params.back().enumValues = enumGetNames(maturityINames);
    commitTool(
        MAKER_TOOL_TYPE_NEW_PIKMIN,
        "new_pikmin", true,
        MakerToolRunners::newPikmin,
        "Spawns a new idle Pikmin under the mouse cursor."
    );
    
    //New reminder.
    commitTool(
        MAKER_TOOL_TYPE_NEW_REMINDER,
        "new_reminder", false,
        MakerToolRunners::newReminder,
        "Creates a new reminder for the area maker, using the mouse "
        "cursor's position as its position. You are then asked to "
        "fill in the reminder's data."
    );
    
    //Path info.
    commitTool(
        MAKER_TOOL_TYPE_PATH_INFO,
        "path_info", true,
        MakerToolRunners::pathInfo,
        "Toggles showing information about the path that the inspected mob "
        "is taking, if any."
    );
    
    //Show collision.
    commitTool(
        MAKER_TOOL_TYPE_SHOW_COLLISION,
        "show_collision", true,
        MakerToolRunners::showCollision,
        "Toggles visibility of mob collision boxes and bubbles."
    );
    
    //Show hitboxes.
    commitTool(
        MAKER_TOOL_TYPE_SHOW_HITBOXES,
        "show_hitboxes", true,
        MakerToolRunners::showHitboxes,
        "Toggles visibility of mob hitboxes."
    );
    
    //Show reaches.
    commitTool(
        MAKER_TOOL_TYPE_SHOW_REACHES,
        "show_reaches", true,
        MakerToolRunners::showReaches,
        "Toggles visibility of mob reach circles and pie-slices for the "
        "inspected mob."
    );
    
    //Teleport.
    queueParam("use inspected mob", ptBool, pfOpt, "false");
    params.back().description =
        "Whether to teleport the current leader, or to instead teleport "
        "the inspected mob.";
    queueParam("leave group members behind", ptBool, pfOpt, "false");
    params.back().description =
        "Whether to teleport the target mob and its group, or to "
        "instead leave the group members behind.";
    commitTool(
        MAKER_TOOL_TYPE_TELEPORT,
        "teleport", true,
        MakerToolRunners::teleport,
        "Teleports the current leader or inspected mob to where the mouse "
        "cursor is."
    );
}


/**
 * @brief Initializes miscellaneous things and settings.
 */
void initMisc() {
    game.mouseCursor.init();
    game.shaders.compileShaders();
    
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(game.display, "Pikifen");
    int newBitmapFlags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(game.options.advanced.smoothScaling) {
        enableFlag(newBitmapFlags, ALLEGRO_MAG_LINEAR);
        enableFlag(newBitmapFlags, ALLEGRO_MIN_LINEAR);
    }
    if(game.options.advanced.mipmapsEnabled) {
        enableFlag(newBitmapFlags, ALLEGRO_MIPMAP);
    }
    al_set_new_bitmap_flags(newBitmapFlags);
    al_reserve_samples(16);
    
    al_identity_transform(&game.identityTransform);
    
    //Inhibit the screensaver for controller-only players.
    al_inhibit_screensaver(true);
    
    game.rng.init();
    
    game.states.gameplay->players.push_back(Player());
    game.states.gameplay->players[0].playerNr = 1;
    
    game.states.gameplay->particles =
        ParticleManager(game.options.advanced.maxParticles);
        
    game.liquidLimitEffectBuffer = al_create_bitmap(game.winW, game.winH);
    game.wallOffsetEffectBuffer = al_create_bitmap(game.winW, game.winH);
    
    game.editorsView.boxMargin = GAMEPLAY::CAMERA_BOX_MARGIN;
    
    game.controls.setGameState(CONTROLS_GAME_STATE_MENUS);
}


/**
 * @brief Initializes the list of sector types, mission end conditions, etc.
 */
void initMiscDatabases() {
    //Mission metric types.
    //Order matters, and should match MISSION_METRIC.
    game.missionMetricTypes = {
        new MissionMetricTypeMobGroup(),
        new MissionMetricTypeMobGroupHealth(),
        new MissionMetricTypeSecsLeft(),
        new MissionMetricTypeSecsPassed(),
        new MissionMetricTypeLeadersInRegion(),
        new MissionMetricTypeLivingPikmin(),
        new MissionMetricTypePikminBorn(),
        new MissionMetricTypePikminDeaths(),
        new MissionMetricTypeLeadersLost(),
        new MissionMetricTypeObjectCollectionPts(),
        new MissionMetricTypeTreasureCollectionPts(),
        new MissionMetricTypeEnemyCollectionPts(),
        new MissionMetricTypeEnemyDefeatPts(),
        new MissionMetricTypeScriptSlot(),
    };
    
    //Mission end condition types.
    //Order matters, and should match MISSION_END_COND.
    game.missionEndCondTypes = {
        new MissionEndCondTypePauseMenu(),
        new MissionEndCondTypeMetricOrMore(),
        new MissionEndCondTypeMetricOrLess(),
        new MissionEndCondTypeTakeDamage(),
    };
}


/**
 * @brief Initializes the list of mob categories.
 */
void initMobCategories() {

    game.mobCategories.registerCategory(
        MOB_CATEGORY_NONE, new NoneCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_PIKMIN, new PikminCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_ONIONS, new OnionCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_LEADERS, new LeaderCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_ENEMIES, new EnemyCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_TREASURES, new TreasureCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_PELLETS, new PelletCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_CONVERTERS, new ConverterCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_DROPS, new DropCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_RESOURCES, new ResourceCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_PILES, new PileCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_TOOLS, new ToolCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_SHIPS, new ShipCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_BRIDGES, new BridgeCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_GROUP_TASKS, new GroupTaskCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_SCALES, new ScaleCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_TRACKS, new TrackCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_BOUNCERS, new BouncerCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_DECORATIONS, new DecorationCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_INTERACTABLES, new InteractableCategory()
    );
    game.mobCategories.registerCategory(
        MOB_CATEGORY_CUSTOM, new CustomCategory()
    );
}


/**
 * @brief Initializes the list of script action types.
 */
void initScriptActionTypes() {
    game.scriptActionTypes.assign(N_SCRIPT_ACTIONS, ScriptActionType());
    vector<CommandParam> params;
    Bitmask8 contexts = 0;
    
    //Convenient lambdas and aliases.
    auto queueParam =
        [&params] (
            const string& paramName, COMMAND_PARAM_TYPE paramType,
            Bitmask8 paramFlags = 0, const string& paramDefValue = ""
    ) {
        params.push_back(
            CommandParam(
                paramName, paramType, paramFlags, paramDefValue
            )
        );
    };
    
    auto commitAction =
        [&params, &contexts] (
            SCRIPT_ACTION actionType, const string& actionName,
            ScriptActionTypeCode * actionRunCode
    ) {
        validateCommandParams(
            params, "Script action type \"" + actionName + "\""
        );
        
        ScriptActionType* actionTypePtr;
        actionTypePtr = &(game.scriptActionTypes[actionType]);
        actionTypePtr->type = actionType;
        actionTypePtr->name = actionName;
        actionTypePtr->code = actionRunCode;
        actionTypePtr->parameters = params;
        actionTypePtr->contexts = contexts;
        params.clear();
    };
    
    const COMMAND_PARAM_TYPE ptInt = COMMAND_PARAM_TYPE_INT;
    const COMMAND_PARAM_TYPE ptFloat = COMMAND_PARAM_TYPE_FLOAT;
    const COMMAND_PARAM_TYPE ptBool = COMMAND_PARAM_TYPE_BOOL;
    const COMMAND_PARAM_TYPE ptString = COMMAND_PARAM_TYPE_STRING;
    const COMMAND_PARAM_TYPE ptEnum = COMMAND_PARAM_TYPE_ENUM;
    const COMMAND_PARAM_FLAG pfConst = COMMAND_PARAM_FLAG_CONST;
    const COMMAND_PARAM_FLAG pfOpt = COMMAND_PARAM_FLAG_OPTIONAL;
    const COMMAND_PARAM_FLAG pfVector = COMMAND_PARAM_FLAG_VECTOR;
    
    
    //-Common actions-
    contexts =
        getIdxBitmask(SCRIPT_CONTEXT_MOB) |
        getIdxBitmask(SCRIPT_CONTEXT_AREA);
        
    //Unknown.
    commitAction(
        SCRIPT_ACTION_UNKNOWN,
        "unknown",
        nullptr
    );
    
    //Absolute number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_ABSOLUTE_NUMBER,
        "absolute_number",
        ScriptActionRunners::absoluteNumber
    );
    
    //Add list item.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("new item", ptString);
    queueParam("item number", ptInt, pfOpt, "0");
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_ADD_LIST_ITEM,
        "add_list_item",
        ScriptActionRunners::addListItem
    );
    
    //Add to string.
    queueParam("destination var name", ptString, pfConst);
    queueParam("base string", ptString);
    queueParam("content to add", ptString);
    queueParam("add a space between", ptBool, pfOpt, "false");
    commitAction(
        SCRIPT_ACTION_ADD_TO_STRING,
        "add_to_string",
        ScriptActionRunners::addToString
    );
    
    //Calculate.
    queueParam("destination var name", ptString, pfConst);
    queueParam("left operand", ptFloat);
    queueParam("operation", ptEnum, pfConst);
    queueParam("right operand", ptFloat);
    commitAction(
        SCRIPT_ACTION_CALCULATE,
        "calculate",
        ScriptActionRunners::calculate
    );
    
    //Clear var.
    queueParam("var name", ptString, pfConst);
    commitAction(
        SCRIPT_ACTION_CLEAR_VAR,
        "clear_var",
        ScriptActionRunners::clearVar
    );
    
    //Ceil number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_CEIL_NUMBER,
        "ceil_number",
        ScriptActionRunners::ceilNumber
    );
    
    //Clamp number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    queueParam("lower bound", ptFloat);
    queueParam("upper bound", ptFloat);
    commitAction(
        SCRIPT_ACTION_CLAMP_NUMBER,
        "clamp_number",
        ScriptActionRunners::clampNumber
    );
    
    //Start do-while loop.
    commitAction(
        SCRIPT_ACTION_DO_WHILE,
        "do_while",
        nullptr
    );
    
    //Ease number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    queueParam("method", ptEnum);
    commitAction(
        SCRIPT_ACTION_EASE_NUMBER,
        "ease_number",
        ScriptActionRunners::easeNumber
    );
    
    //Else.
    commitAction(
        SCRIPT_ACTION_ELSE,
        "else",
        nullptr
    );
    
    //Else if.
    queueParam("comparand", ptString);
    queueParam("operation", ptEnum, pfConst);
    queueParam("value", ptString, pfVector);
    commitAction(
        SCRIPT_ACTION_ELSE_IF,
        "else_if",
        ScriptActionRunners::ifAction
    );
    
    //End do-while loop.
    queueParam("comparand", ptString);
    queueParam("operation", ptEnum, pfConst);
    queueParam("value", ptString);
    commitAction(
        SCRIPT_ACTION_END_DO_WHILE,
        "end_do_while",
        ScriptActionRunners::endDoWhile
    );
    
    //End for loop.
    commitAction(
        SCRIPT_ACTION_END_FOR,
        "end_for",
        nullptr
    );
    
    //End for-each loop.
    commitAction(
        SCRIPT_ACTION_END_FOR_EACH,
        "end_for_each",
        nullptr
    );
    
    //End if.
    commitAction(
        SCRIPT_ACTION_END_IF,
        "end_if",
        nullptr
    );
    
    //End while-do loop.
    commitAction(
        SCRIPT_ACTION_END_WHILE_DO,
        "end_while_do",
        nullptr
    );
    
    //Focus.
    queueParam("target", ptEnum);
    commitAction(
        SCRIPT_ACTION_FOCUS,
        "focus",
        ScriptActionRunners::focus
    );
    
    //Focus on ID.
    queueParam("mob ID", ptInt);
    commitAction(
        SCRIPT_ACTION_FOCUS_ON_ID,
        "focus_on_id",
        ScriptActionRunners::focusOnId
    );
    
    //Start for loop.
    queueParam("iterator var name", ptString, pfConst);
    queueParam("target value", ptInt);
    queueParam("starting value", ptInt, pfOpt, "1");
    queueParam("jump", ptInt, pfOpt, "1");
    commitAction(
        SCRIPT_ACTION_FOR,
        "for",
        ScriptActionRunners::forAction
    );
    
    //Start for each loop.
    queueParam("iterator var name", ptString, pfConst);
    queueParam("item var name", ptInt);
    queueParam("list or string", ptString);
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_FOR_EACH,
        "for_each",
        ScriptActionRunners::forEach
    );
    
    //Get angle.
    queueParam("destination var name", ptString, pfConst);
    queueParam("center x", ptFloat);
    queueParam("center y", ptFloat);
    queueParam("target x", ptFloat);
    queueParam("target y", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_ANGLE,
        "get_angle",
        ScriptActionRunners::getAngle
    );
    
    //Get angle clockwise difference.
    queueParam("destination var name", ptString, pfConst);
    queueParam("angle 1", ptFloat);
    queueParam("angle 2", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_ANGLE_CW_DIFF,
        "get_angle_clockwise_difference",
        ScriptActionRunners::getAngleCwDiff
    );
    
    //Get angle smallest difference.
    queueParam("destination var name", ptString, pfConst);
    queueParam("angle 1", ptFloat);
    queueParam("angle 2", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_ANGLE_SMALLEST_DIFF,
        "get_angle_smallest_difference",
        ScriptActionRunners::getAngleSmallestDiff
    );
    
    //DEPRECATED in 1.2.0 by "get_misc_info".
    //Get area info.
    queueParam("destination var name", ptString, pfConst);
    queueParam("info", ptString);
    commitAction(
        SCRIPT_ACTION_GET_AREA_INFO,
        "get_area_info",
        ScriptActionRunners::getAreaInfo
    );
    
    //Get coordinates from angle.
    queueParam("x destination var name", ptString, pfConst);
    queueParam("y destination var name", ptString, pfConst);
    queueParam("angle", ptFloat);
    queueParam("distance", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_COORDINATES_FROM_ANGLE,
        "get_coordinates_from_angle",
        ScriptActionRunners::getCoordinatesFromAngle
    );
    
    //Get distance.
    queueParam("destination var name", ptString, pfConst);
    queueParam("center x", ptFloat);
    queueParam("center y", ptFloat);
    queueParam("target x", ptFloat);
    queueParam("target y", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_DISTANCE,
        "get_distance",
        ScriptActionRunners::getDistance
    );
    
    //Get event info.
    queueParam("destination var name", ptString, pfConst);
    queueParam("info", ptString);
    commitAction(
        SCRIPT_ACTION_GET_EVENT_INFO,
        "get_event_info",
        ScriptActionRunners::getEventInfo
    );
    
    //Get focus var.
    queueParam("destination var name", ptString, pfConst);
    queueParam("focused mob's var name", ptString, pfConst);
    commitAction(
        SCRIPT_ACTION_GET_FOCUS_VAR,
        "get_focus_var",
        ScriptActionRunners::getFocusVar
    );
    
    //Get leader Pikmin count var.
    queueParam("destination var name", ptString, pfConst);
    queueParam("target", ptString);
    queueParam("Pikmin type", ptString, pfOpt);
    commitAction(
        SCRIPT_ACTION_GET_LEADER_PIKMIN_COUNT,
        "get_leader_pikmin_count",
        ScriptActionRunners::getLeaderPikminCount
    );
    
    //Get list item.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("item number", ptInt, pfOpt, "0");
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_GET_LIST_ITEM,
        "get_list_item",
        ScriptActionRunners::getListItem
    );
    
    //Get list item number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("item to search", ptInt);
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_GET_LIST_ITEM_NUMBER,
        "get_list_item_number",
        ScriptActionRunners::getListItemNumber
    );
    
    //Get list size.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_GET_LIST_SIZE,
        "get_list_size",
        ScriptActionRunners::getListSize
    );
    
    //Get misc. info.
    queueParam("destination var name", ptString, pfConst);
    queueParam("info", ptString);
    commitAction(
        SCRIPT_ACTION_GET_MISC_INFO,
        "get_misc_info",
        ScriptActionRunners::getMiscInfo
    );
    
    //Get mission metric.
    queueParam("destination var name", ptString, pfConst);
    queueParam("metric", ptEnum);
    queueParam("number data", ptInt, pfOpt, "1");
    queueParam("get auto target instead", ptBool, pfOpt, "false");
    commitAction(
        SCRIPT_ACTION_GET_MISSION_METRIC,
        "get_mission_metric",
        ScriptActionRunners::getMissionMetric
    );
    
    //Get mob IDs in region.
    queueParam("destination var name", ptString, pfConst);
    queueParam("region number", ptInt);
    commitAction(
        SCRIPT_ACTION_GET_MOB_IDS_IN_REGION,
        "get_mob_ids_in_region",
        ScriptActionRunners::getMobIdsInRegion
    );
    
    //Get mob IDs with var.
    queueParam("destination var name", ptString, pfConst);
    queueParam("var", ptString);
    queueParam("value", ptString, pfOpt, "");
    commitAction(
        SCRIPT_ACTION_GET_MOB_IDS_WITH_VAR,
        "get_mob_ids_with_var",
        ScriptActionRunners::getMobIdsWithVar
    );
    
    //Get mob info.
    queueParam("destination var name", ptString, pfConst);
    queueParam("target", ptString);
    queueParam("info", ptString);
    commitAction(
        SCRIPT_ACTION_GET_MOB_INFO,
        "get_mob_info",
        ScriptActionRunners::getMobInfo
    );
    
    //Get random float.
    queueParam("destination var name", ptString, pfConst);
    queueParam("minimum value", ptFloat);
    queueParam("maximum value", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_RANDOM_FLOAT,
        "get_random_float",
        ScriptActionRunners::getRandomFloat
    );
    
    //Get random int.
    queueParam("destination var name", ptString, pfConst);
    queueParam("minimum value", ptInt);
    queueParam("maximum value", ptInt);
    commitAction(
        SCRIPT_ACTION_GET_RANDOM_INT,
        "get_random_int",
        ScriptActionRunners::getRandomInt
    );
    
    //Get var presence.
    queueParam("destination var name", ptString, pfConst);
    queueParam("var name", ptString, pfConst);
    commitAction(
        SCRIPT_ACTION_GET_VAR_PRESENCE,
        "get_var_presence",
        ScriptActionRunners::getVarPresence
    );
    
    //Goto.
    queueParam("label name", ptString);
    commitAction(
        SCRIPT_ACTION_GOTO,
        "goto",
        nullptr
    );
    
    //If.
    queueParam("comparand", ptString);
    queueParam("operation", ptEnum, pfConst);
    queueParam("value", ptString);
    commitAction(
        SCRIPT_ACTION_IF,
        "if",
        ScriptActionRunners::ifAction
    );
    
    //Interpolate number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("input number", ptFloat);
    queueParam("input start", ptFloat);
    queueParam("input end", ptFloat);
    queueParam("output start", ptFloat);
    queueParam("output end", ptFloat);
    commitAction(
        SCRIPT_ACTION_INTERPOLATE_NUMBER,
        "interpolate_number",
        ScriptActionRunners::interpolateNumber
    );
    
    //Label.
    queueParam("label name", ptString, pfConst);
    commitAction(
        SCRIPT_ACTION_LABEL,
        "label",
        nullptr
    );
    
    //Max number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("first number", ptFloat);
    queueParam("second number", ptFloat);
    commitAction(
        SCRIPT_ACTION_MAX_NUMBER,
        "max_number",
        ScriptActionRunners::maxNumber
    );
    
    //Min number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("first number", ptFloat);
    queueParam("second number", ptFloat);
    commitAction(
        SCRIPT_ACTION_MIN_NUMBER,
        "min_number",
        ScriptActionRunners::minNumber
    );
    
    //Print.
    queueParam("text", ptString, pfVector);
    commitAction(
        SCRIPT_ACTION_PRINT,
        "print",
        ScriptActionRunners::print
    );
    
    //Remove list item.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("item number", ptInt, pfOpt, "0");
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_REMOVE_LIST_ITEM,
        "remove_list_item",
        ScriptActionRunners::removeListItem
    );
    
    //Round number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_ROUND_NUMBER,
        "round_number",
        ScriptActionRunners::roundNumber
    );
    
    //Run next action as.
    queueParam("target", ptEnum);
    commitAction(
        SCRIPT_ACTION_RUN_NEXT_ACTION_AS,
        "run_next_action_as",
        ScriptActionRunners::runNextActionAs
    );
    
    //Send script message to all mobs.
    queueParam("message", ptString);
    commitAction(
        SCRIPT_ACTION_SEND_MESSAGE_TO_ALL_MOBS,
        "send_message_to_all_mobs",
        ScriptActionRunners::sendMessageToAllMobs
    );
    
    //Send script message to area.
    queueParam("message", ptString);
    commitAction(
        SCRIPT_ACTION_SEND_MESSAGE_TO_AREA,
        "send_message_to_area",
        ScriptActionRunners::sendMessageToArea
    );
    
    //Send script message to focus.
    queueParam("message", ptString);
    commitAction(
        SCRIPT_ACTION_SEND_MESSAGE_TO_FOCUS,
        "send_message_to_focus",
        ScriptActionRunners::sendMessageToFocus
    );
    
    //Set focus var.
    queueParam("focused mob's destination var name", ptString, pfConst);
    queueParam("value", ptString);
    commitAction(
        SCRIPT_ACTION_SET_FOCUS_VAR,
        "set_focus_var",
        ScriptActionRunners::setFocusVar
    );
    
    //Set list item.
    queueParam("destination var name", ptString, pfConst);
    queueParam("list or string", ptString);
    queueParam("new item", ptString);
    queueParam("item number", ptInt, pfOpt, "0");
    queueParam("list delimiter", ptEnum, pfOpt, "colon");
    commitAction(
        SCRIPT_ACTION_SET_LIST_ITEM,
        "set_list_item",
        ScriptActionRunners::setListItem
    );
    
    //Set mission metric script slot.
    queueParam("slot number", ptInt);
    queueParam("value", ptInt);
    commitAction(
        SCRIPT_ACTION_SET_MISSION_METRIC_SCRIPT_SLOT,
        "set_mission_metric_script_slot",
        ScriptActionRunners::setMissionMetricScriptSlot
    );
    
    //Set state.
    queueParam("state name", ptString);
    commitAction(
        SCRIPT_ACTION_SET_STATE,
        "set_state",
        ScriptActionRunners::setState
    );
    
    //Set timer.
    queueParam("time", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_TIMER,
        "set_timer",
        ScriptActionRunners::setTimer
    );
    
    //Set var.
    queueParam("destination var name", ptString, pfConst);
    queueParam("value", ptString);
    commitAction(
        SCRIPT_ACTION_SET_VAR,
        "set_var",
        ScriptActionRunners::setVar
    );
    
    //Show cutscene message.
    queueParam("text", ptString, pfVector);
    commitAction(
        SCRIPT_ACTION_SHOW_CUTSCENE_MESSAGE,
        "show_cutscene_message",
        ScriptActionRunners::showCutsceneMessage
    );
    
    //DEPRECATED in 1.2.0 by "show_cutscene_message".
    //Show cutscene message from var.
    queueParam("var name", ptString, pfConst);
    commitAction(
        SCRIPT_ACTION_SHOW_MESSAGE_FROM_VAR,
        "show_message_from_var",
        ScriptActionRunners::showMessageFromVar
    );
    
    //Sign number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_SIGN_NUMBER,
        "sign_number",
        ScriptActionRunners::signNumber
    );
    
    //Square root number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_SQUARE_ROOT_NUMBER,
        "square_root_number",
        ScriptActionRunners::squareRootNumber
    );
    
    //Truncate number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_TRUNCATE_NUMBER,
        "truncate_number",
        ScriptActionRunners::truncateNumber
    );
    
    //Unfocus.
    commitAction(
        SCRIPT_ACTION_UNFOCUS,
        "unfocus",
        ScriptActionRunners::unfocus
    );
    
    
    //-Mob script actions-
    contexts = getIdxBitmask(SCRIPT_CONTEXT_MOB);
    
    //Add health.
    queueParam("amount", ptFloat);
    commitAction(
        SCRIPT_ACTION_ADD_HEALTH,
        "add_health",
        ScriptActionRunners::addHealth
    );
    
    //Arachnorb plan logic.
    queueParam("goal", ptEnum);
    commitAction(
        SCRIPT_ACTION_ARACHNORB_PLAN_LOGIC,
        "arachnorb_plan_logic",
        ScriptActionRunners::arachnorbPlanLogic
    );
    
    //Be chomped.
    commitAction(
        SCRIPT_ACTION_BE_CHOMPED,
        "be_chomped",
        ScriptActionRunners::beChomped
    );
    
    //Delete.
    commitAction(
        SCRIPT_ACTION_DELETE,
        "delete",
        ScriptActionRunners::deleteAction
    );
    
    //Drain liquid.
    commitAction(
        SCRIPT_ACTION_DRAIN_LIQUID,
        "drain_liquid",
        ScriptActionRunners::drainLiquid
    );
    
    //Finish dying.
    commitAction(
        SCRIPT_ACTION_FINISH_DYING,
        "finish_dying",
        ScriptActionRunners::finishDying
    );
    
    //Floor number.
    queueParam("destination var name", ptString, pfConst);
    queueParam("number", ptFloat);
    commitAction(
        SCRIPT_ACTION_FLOOR_NUMBER,
        "floor_number",
        ScriptActionRunners::floorNumber
    );
    
    //Follow mob as a leader.
    queueParam("target", ptEnum);
    queueParam("silent", ptBool, pfOpt, "false");
    commitAction(
        SCRIPT_ACTION_FOLLOW_MOB_AS_LEADER,
        "follow_mob_as_leader",
        ScriptActionRunners::followMobAsLeader
    );
    
    //Follow path randomly.
    queueParam("label", ptString, pfOpt);
    commitAction(
        SCRIPT_ACTION_FOLLOW_PATH_RANDOMLY,
        "follow_path_randomly",
        ScriptActionRunners::followPathRandomly
    );
    
    //Follow path to absolute.
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    queueParam("label", ptString, pfOpt);
    commitAction(
        SCRIPT_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
        "follow_path_to_absolute",
        ScriptActionRunners::followPathToAbsolute
    );
    
    //DEPRECATED in 1.2.0 by "be_chomped".
    //Get chomped.
    commitAction(
        SCRIPT_ACTION_GET_CHOMPED,
        "get_chomped",
        ScriptActionRunners::beChomped
    );
    
    //Get floor Z.
    queueParam("destination var name", ptString, pfConst);
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    commitAction(
        SCRIPT_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        ScriptActionRunners::getFloorZ
    );
    
    //Hold focused mob.
    queueParam("body part name", ptEnum);
    queueParam("hold above", ptBool, pfOpt, "false");
    queueParam("rotation method", ptEnum, pfOpt, "copy_holder");
    commitAction(
        SCRIPT_ACTION_HOLD_FOCUS,
        "hold_focused_mob",
        ScriptActionRunners::holdFocus
    );
    
    //Link with focused mob.
    commitAction(
        SCRIPT_ACTION_LINK_WITH_FOCUS,
        "link_with_focused_mob",
        ScriptActionRunners::linkWithFocus
    );
    
    //DEPRECATED in 1.2.0 by "get_mob_info" and "focus_on_id".
    //Load focused mob memory.
    queueParam("slot", ptInt);
    commitAction(
        SCRIPT_ACTION_LOAD_FOCUS_MEMORY,
        "load_focused_mob_memory",
        ScriptActionRunners::loadFocusMemory
    );
    
    //Move to absolute.
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    queueParam("z", ptFloat, pfOpt);
    commitAction(
        SCRIPT_ACTION_MOVE_TO_ABSOLUTE,
        "move_to_absolute",
        ScriptActionRunners::moveToAbsolute
    );
    
    //Move to relative.
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    queueParam("z", ptFloat, pfOpt);
    commitAction(
        SCRIPT_ACTION_MOVE_TO_RELATIVE,
        "move_to_relative",
        ScriptActionRunners::moveToRelative
    );
    
    //Move to target.
    queueParam("target", ptEnum);
    commitAction(
        SCRIPT_ACTION_MOVE_TO_TARGET,
        "move_to_target",
        ScriptActionRunners::moveToTarget
    );
    
    //Order release.
    commitAction(
        SCRIPT_ACTION_ORDER_RELEASE,
        "order_release",
        ScriptActionRunners::orderRelease
    );
    
    //Play sound.
    queueParam("sound data", ptEnum);
    queueParam("sound ID destination var name", ptString, pfOpt);
    commitAction(
        SCRIPT_ACTION_PLAY_SOUND,
        "play_sound",
        ScriptActionRunners::playSound
    );
    
    //Receive status.
    queueParam("status name", ptEnum);
    commitAction(
        SCRIPT_ACTION_RECEIVE_STATUS,
        "receive_status",
        ScriptActionRunners::receiveStatus
    );
    
    //Release.
    commitAction(
        SCRIPT_ACTION_RELEASE,
        "release",
        ScriptActionRunners::release
    );
    
    //Release stored mobs.
    commitAction(
        SCRIPT_ACTION_RELEASE_STORED_MOBS,
        "release_stored_mobs",
        ScriptActionRunners::releaseStoredMobs
    );
    
    //Remove status.
    queueParam("status name", ptEnum);
    commitAction(
        SCRIPT_ACTION_REMOVE_STATUS,
        "remove_status",
        ScriptActionRunners::removeStatus
    );
    
    //DEPRECATED in 1.2.0 by "get_mob_info" and "focus_on_id".
    //Save focused mob memory.
    queueParam("slot", ptInt);
    commitAction(
        SCRIPT_ACTION_SAVE_FOCUS_MEMORY,
        "save_focused_mob_memory",
        ScriptActionRunners::saveFocusMemory
    );
    
    //Send script message to links.
    queueParam("message", ptString);
    commitAction(
        SCRIPT_ACTION_SEND_MESSAGE_TO_LINKS,
        "send_message_to_links",
        ScriptActionRunners::sendMessageToLinks
    );
    
    //Send script message to nearby.
    queueParam("distance", ptFloat);
    queueParam("message", ptString);
    commitAction(
        SCRIPT_ACTION_SEND_MESSAGE_TO_NEARBY,
        "send_message_to_nearby",
        ScriptActionRunners::sendMessageToNearby
    );
    
    //Set animation.
    queueParam("animation name", ptString);
    queueParam("options", ptEnum, pfOpt, "normal");
    queueParam("depend on mob speed", ptBool, pfOpt, "false");
    commitAction(
        SCRIPT_ACTION_SET_ANIMATION,
        "set_animation",
        ScriptActionRunners::setAnimation
    );
    
    //Set can block paths.
    queueParam("blocks", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_CAN_BLOCK_PATHS,
        "set_can_block_paths",
        ScriptActionRunners::setCanBlockPaths
    );
    
    //Set far reach.
    queueParam("reach name", ptEnum);
    commitAction(
        SCRIPT_ACTION_SET_FAR_REACH,
        "set_far_reach",
        ScriptActionRunners::setFarReach
    );
    
    //Set flying.
    queueParam("flying", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_FLYING,
        "set_flying",
        ScriptActionRunners::setFlying
    );
    
    //Set gravity.
    queueParam("multiplier", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_GRAVITY,
        "set_gravity",
        ScriptActionRunners::setGravity
    );
    
    //Set health.
    queueParam("amount", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_HEALTH,
        "set_health",
        ScriptActionRunners::setHealth
    );
    
    //Set height.
    queueParam("height", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_HEIGHT,
        "set_height",
        ScriptActionRunners::setHeight
    );
    
    //Set hiding.
    queueParam("hiding", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_HIDING,
        "set_hiding",
        ScriptActionRunners::setHiding
    );
    
    //Set huntable.
    queueParam("huntable", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_HUNTABLE,
        "set_huntable",
        ScriptActionRunners::setHuntable
    );
    
    //Set holdable.
    queueParam("options", ptEnum, pfOpt | pfVector);
    commitAction(
        SCRIPT_ACTION_SET_HOLDABLE,
        "set_holdable",
        ScriptActionRunners::setHoldable
    );
    
    //Set limb animation.
    queueParam("animation name", ptString);
    commitAction(
        SCRIPT_ACTION_SET_LIMB_ANIMATION,
        "set_limb_animation",
        ScriptActionRunners::setLimbAnimation
    );
    
    //Set near reach.
    queueParam("reach name", ptEnum);
    commitAction(
        SCRIPT_ACTION_SET_NEAR_REACH,
        "set_near_reach",
        ScriptActionRunners::setNearReach
    );
    
    //Set radius.
    queueParam("radius", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_RADIUS,
        "set_radius",
        ScriptActionRunners::setRadius
    );
    
    //Set sector scroll.
    queueParam("x speed", ptFloat);
    queueParam("y speed", ptFloat);
    commitAction(
        SCRIPT_ACTION_SET_SECTOR_SCROLL,
        "set_sector_scroll",
        ScriptActionRunners::setSectorScroll
    );
    
    //Set shadow visibility.
    queueParam("visible", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_SHADOW_VISIBILITY,
        "set_shadow_visibility",
        ScriptActionRunners::setShadowVisibility
    );
    
    //Set tangible.
    queueParam("tangible", ptBool);
    commitAction(
        SCRIPT_ACTION_SET_TANGIBLE,
        "set_tangible",
        ScriptActionRunners::setTangible
    );
    
    //Set team.
    queueParam("team name", ptEnum);
    commitAction(
        SCRIPT_ACTION_SET_TEAM,
        "set_team",
        ScriptActionRunners::setTeam
    );
    
    //Shake camera.
    queueParam("amount", ptFloat);
    commitAction(
        SCRIPT_ACTION_SHAKE_CAMERA,
        "shake_camera",
        ScriptActionRunners::shakeCamera
    );
    
    //Spawn.
    queueParam("spawn data", ptEnum);
    commitAction(
        SCRIPT_ACTION_SPAWN,
        "spawn",
        ScriptActionRunners::spawn
    );
    
    //Stabilize Z.
    queueParam("reference", ptEnum);
    queueParam("offset", ptFloat);
    commitAction(
        SCRIPT_ACTION_STABILIZE_Z,
        "stabilize_z",
        ScriptActionRunners::stabilizeZ
    );
    
    //Start chomping.
    queueParam("victim max", ptInt);
    queueParam("body part", ptEnum);
    queueParam("more body parts", ptEnum, pfOpt | pfVector);
    commitAction(
        SCRIPT_ACTION_START_CHOMPING,
        "start_chomping",
        ScriptActionRunners::startChomping
    );
    
    //Start dying.
    commitAction(
        SCRIPT_ACTION_START_DYING,
        "start_dying",
        ScriptActionRunners::startDying
    );
    
    //Start height effect.
    commitAction(
        SCRIPT_ACTION_START_HEIGHT_EFFECT,
        "start_height_effect",
        ScriptActionRunners::startHeightEffect
    );
    
    //Start particles.
    queueParam("generator name", ptEnum);
    queueParam("offset X", ptFloat, pfOpt, "0");
    queueParam("offset Y", ptFloat, pfOpt, "0");
    queueParam("offset Z", ptFloat, pfOpt, "0");
    commitAction(
        SCRIPT_ACTION_START_PARTICLES,
        "start_particles",
        ScriptActionRunners::startParticles
    );
    
    //Stop.
    commitAction(
        SCRIPT_ACTION_STOP,
        "stop",
        ScriptActionRunners::stop
    );
    
    //Stop chomping.
    commitAction(
        SCRIPT_ACTION_STOP_CHOMPING,
        "stop_chomping",
        ScriptActionRunners::stopChomping
    );
    
    //Stop height effect.
    commitAction(
        SCRIPT_ACTION_STOP_HEIGHT_EFFECT,
        "stop_height_effect",
        ScriptActionRunners::stopHeightEffect
    );
    
    //Stop particles.
    commitAction(
        SCRIPT_ACTION_STOP_PARTICLES,
        "stop_particles",
        ScriptActionRunners::stopParticles
    );
    
    //Stop sound.
    queueParam("sound ID", ptInt);
    commitAction(
        SCRIPT_ACTION_STOP_SOUND,
        "stop_sound",
        ScriptActionRunners::stopSound
    );
    
    //Stop vertically.
    commitAction(
        SCRIPT_ACTION_STOP_VERTICALLY,
        "stop_vertically",
        ScriptActionRunners::stopVertically
    );
    
    //Store focus inside.
    commitAction(
        SCRIPT_ACTION_STORE_FOCUS_INSIDE,
        "store_focus_inside",
        ScriptActionRunners::storeFocusInside
    );
    
    //Swallow.
    queueParam("amount", ptInt);
    commitAction(
        SCRIPT_ACTION_SWALLOW,
        "swallow",
        ScriptActionRunners::swallow
    );
    
    //Swallow all.
    commitAction(
        SCRIPT_ACTION_SWALLOW_ALL,
        "swallow_all",
        ScriptActionRunners::swallowAll
    );
    
    //Teleport to absolute.
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    queueParam("z", ptFloat);
    commitAction(
        SCRIPT_ACTION_TELEPORT_TO_ABSOLUTE,
        "teleport_to_absolute",
        ScriptActionRunners::teleportToAbsolute
    );
    
    //Teleport to relative.
    queueParam("x", ptFloat);
    queueParam("y", ptFloat);
    queueParam("z", ptFloat);
    commitAction(
        SCRIPT_ACTION_TELEPORT_TO_RELATIVE,
        "teleport_to_relative",
        ScriptActionRunners::teleportToRelative
    );
    
    //Throw focused mob.
    queueParam("x coordinate", ptFloat);
    queueParam("y coordinate", ptFloat);
    queueParam("z coordinate", ptFloat);
    queueParam("max height", ptFloat);
    commitAction(
        SCRIPT_ACTION_THROW_FOCUS,
        "throw_focused_mob",
        ScriptActionRunners::throwFocus
    );
    
    //Turn to absolute.
    queueParam("angle or x coordinate", ptFloat);
    queueParam("y coordinate", ptFloat, pfOpt);
    commitAction(
        SCRIPT_ACTION_TURN_TO_ABSOLUTE,
        "turn_to_absolute",
        ScriptActionRunners::turnToAbsolute
    );
    
    //Turn to relative.
    queueParam("angle or x coordinate", ptFloat);
    queueParam("y coordinate", ptFloat, pfOpt);
    commitAction(
        SCRIPT_ACTION_TURN_TO_RELATIVE,
        "turn_to_relative",
        ScriptActionRunners::turnToRelative
    );
    
    //Turn to target.
    queueParam("target", ptEnum);
    commitAction(
        SCRIPT_ACTION_TURN_TO_TARGET,
        "turn_to_target",
        ScriptActionRunners::turnToTarget
    );
    
    //Start while-do loop.
    queueParam("comparand", ptString);
    queueParam("operation", ptEnum, pfConst);
    queueParam("value", ptString);
    commitAction(
        SCRIPT_ACTION_WHILE_DO,
        "while_do",
        ScriptActionRunners::whileDo
    );
}


#pragma endregion
