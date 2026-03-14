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

#include "init.h"

#include "../content/mob_category/bouncer_category.h"
#include "../content/mob_category/bridge_category.h"
#include "../content/mob_category/converter_category.h"
#include "../content/mob_category/custom_category.h"
#include "../content/mob_category/decoration_category.h"
#include "../content/mob_category/drop_category.h"
#include "../content/mob_category/enemy_category.h"
#include "../content/mob_category/group_task_category.h"
#include "../content/mob_category/interactable_category.h"
#include "../content/mob_category/leader_category.h"
#include "../content/mob_category/onion_category.h"
#include "../content/mob_category/pellet_category.h"
#include "../content/mob_category/pikmin_category.h"
#include "../content/mob_category/pile_category.h"
#include "../content/mob_category/resource_category.h"
#include "../content/mob_category/scale_category.h"
#include "../content/mob_category/ship_category.h"
#include "../content/mob_category/tool_category.h"
#include "../content/mob_category/track_category.h"
#include "../content/mob_category/treasure_category.h"
#include "../content/script/script.h"
#include "../game_state/game_state.h"
#include "../game_state/gameplay/gameplay.h"
#include "../lib/imgui/imgui_impl_allegro5.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/imgui_utils.h"
#include "../util/string_utils.h"
#include "controls_mediator.h"
#include "game.h"
#include "misc_functions.h"


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
    
    for(size_t e = 0; e < game.missionEndCondTypes.size(); e++) {
        delete game.missionEndCondTypes[e];
    }
    game.missionEndCondTypes.clear();
    for(size_t c = 0; c < game.missionScoreCriterionTypes.size(); c++) {
        delete game.missionScoreCriterionTypes[c];
    }
    game.missionScoreCriterionTypes.clear();
    for(size_t g = 0; g < game.missionGoals.size(); g++) {
        delete game.missionGoals[g];
    }
    game.missionGoals.clear();
    for(size_t c = 0; c < game.missionFailConds.size(); c++) {
        delete game.missionFailConds[c];
    }
    game.missionFailConds.clear();
    for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
        delete game.missionScoreCriteria[c];
    }
    game.missionScoreCriteria.clear();
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
        PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Change speed",
        "Change the gameplay speed.",
        "mt_change_speed", "k_28", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
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
        PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Geometry info",
        "Toggle info about the geometry under the mouse cursor.",
        "mt_geometry_info", "k_33", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addNewActionType(
        PLAYER_ACTION_TYPE_MT_HUD,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "HUD",
        "Toggle the HUD.",
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
        PLAYER_ACTION_TYPE_MT_MOB_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Mob info",
        "Toggle info about the mob under the mouse cursor.",
        "mt_mob_info", "k_32", Inpution::ACTION_VALUE_TYPE_DIGITAL,
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
        "Toggle info about paths the info'd mob is taking.",
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
        "Toggle drawing the info'd mob's reaches.",
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
    for(size_t a = 0; a < actionTypes.size(); a++) {
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
    
    for(size_t c = 0; c < colorsToChange.size(); c++) {
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
    //Error bitmap.
    game.bmpError = al_create_bitmap(32, 32);
    al_set_target_bitmap(game.bmpError); {
        al_clear_to_color(al_map_rgba(0, 0, 0, 192));
        al_draw_filled_rectangle(
            0.0, 0.0, 16.0, 16.0,
            al_map_rgba(255, 0, 255, 192)
        );
        al_draw_filled_rectangle(
            16.0, 16.0, 32.0, 32.0,
            al_map_rgba(255, 0, 255, 192)
        );
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
    game.states.gameplay->players[0].playerNr = 0;
    
    game.states.gameplay->particles =
        ParticleManager(game.options.advanced.maxParticles);
        
    game.liquidLimitEffectBuffer = al_create_bitmap(game.winW, game.winH);
    game.wallOffsetEffectBuffer = al_create_bitmap(game.winW, game.winH);
    
    game.editorsView.boxMargin = GAMEPLAY::CAMERA_BOX_MARGIN;
    
    game.controls.setGameState(CONTROLS_GAME_STATE_MENUS);
}


/**
 * @brief Initializes the list of sector types, mission goals, etc.
 */
void initMiscDatabases() {
    //Mission end conditions.
    //Order matters, and should match MISSION_END_COND.
    game.missionEndCondTypes = {
        new MissionEndCondTypePauseMenu(),
        new MissionEndCondTypeMobGroup(),
        new MissionEndCondTypeTimeLimit(),
        new MissionEndCondTypeLeadersInRegion(),
        new MissionEndCondTypePikminOrMore(),
        new MissionEndCondTypePikminOrFewer(),
        new MissionEndCondTypeLosePikmin(),
        new MissionEndCondTypeLoseLeaders(),
        new MissionEndCondTypeTakeDamage(),
    };
    
    //Mission score criteria.
    //Order matters, and should match MISSION_SCORE_CRITERION.
    game.missionScoreCriterionTypes = {
        new MissionScoreCriterionTypeMobGroup(),
        new MissionScoreCriterionTypePikmin(),
        new MissionScoreCriterionTypePikminBorn(),
        new MissionScoreCriterionTypePikminDeaths(),
        new MissionScoreCriterionTypeSecLeft(),
        new MissionScoreCriterionTypeSecPassed(),
        new MissionScoreCriterionTypeCollectionPts(),
        new MissionScoreCriterionTypeDefeatPts(),
    };
    
    //Mission goals.
    //Order matters, and should match MISSION_GOAL.
    game.missionGoals = {
        new MissionGoalEndManually(),
        new MissionGoalCollectTreasures(),
        new MissionGoalBattleEnemies(),
        new MissionGoalTimedSurvival(),
        new MissionGoalGetToExit(),
        new MissionGoalGrowPikmin(),
    };
    
    //Mission fail conditions.
    //Order matters, and should match MISSION_FAIL_COND.
    game.missionFailConds = {
        new MissionFailTimeLimit(),
        new MissionFailTooFewPikmin(),
        new MissionFailTooManyPikmin(),
        new MissionFailLosePikmin(),
        new MissionFailTakeDamage(),
        new MissionFailLoseLeaders(),
        new MissionFailDefeatEnemies(),
        new MissionFailPauseMenu(),
    };
    
    //Mission score criteria.
    //Order matters, and should match MISSION_SCORE_CRITERIA.
    game.missionScoreCriteria = {
        new MissionScoreCriterionPikminBorn(),
        new MissionScoreCriterionPikminDeath(),
        new MissionScoreCriterionSecLeft(),
        new MissionScoreCriterionSecPassed(),
        new MissionScoreCriterionTreasurePoints(),
        new MissionScoreCriterionEnemyPoints(),
    };
}


/**
 * @brief Initializes the list of script action types.
 */
void initScriptActionTypes() {

#define queueParam(pName, pType, constant, extras) \
    params.push_back(ScriptActionTypeParam(pName, pType, constant, extras));
#define commitAction(aType, aName, runCode, loadCode) \
    a = &(game.scriptActionTypes[aType]); \
    a->type = aType; \
    a->name = aName; \
    a->code = runCode; \
    a->extraLoadLogic = loadCode; \
    a->parameters = params; \
    params.clear();


    game.scriptActionTypes.assign(N_SCRIPT_ACTIONS, ScriptActionType());
    vector<ScriptActionTypeParam> params;
    ScriptActionType* a;
    
    //Unknown.
    commitAction(
        SCRIPT_ACTION_UNKNOWN,
        "unknown",
        nullptr,
        nullptr
    );
    
    //Absolute number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_ABSOLUTE_NUMBER,
        "absolute_number",
        ScriptActionRunners::absoluteNumber,
        nullptr
    );
    
    //Add health.
    queueParam("amount", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_ADD_HEALTH,
        "add_health",
        ScriptActionRunners::addHealth,
        nullptr
    );
    
    //Arachnorb plan logic.
    queueParam("goal", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_ARACHNORB_PLAN_LOGIC,
        "arachnorb_plan_logic",
        ScriptActionRunners::arachnorbPlanLogic,
        ScriptActionLoaders::arachnorbPlanLogic
    );
    
    //Calculate.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("left operand", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("operation", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("right operand", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_CALCULATE,
        "calculate",
        ScriptActionRunners::calculate,
        ScriptActionLoaders::calculate
    );
    
    //Ceil number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_CEIL_NUMBER,
        "ceil_number",
        ScriptActionRunners::ceilNumber,
        nullptr
    );
    
    //Delete.
    commitAction(
        MOB_ACTION_DELETE,
        "delete",
        ScriptActionRunners::deleteFunction,
        nullptr
    );
    
    //Drain liquid.
    commitAction(
        MOB_ACTION_DRAIN_LIQUID,
        "drain_liquid",
        ScriptActionRunners::drainLiquid,
        nullptr
    );
    
    //Ease number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("method", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_EASE_NUMBER,
        "ease_number",
        ScriptActionRunners::easeNumber,
        ScriptActionLoaders::easeNumber
    );
    
    //Else.
    commitAction(
        MOB_ACTION_ELSE,
        "else",
        nullptr,
        nullptr
    );
    
    //Else if.
    queueParam("comparand", SCRIPT_ACTION_PARAM_STRING, false, false);
    queueParam("operation", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("value", SCRIPT_ACTION_PARAM_STRING, false, true);
    commitAction(
        MOB_ACTION_ELSE_IF,
        "else_if",
        ScriptActionRunners::ifFunction,
        ScriptActionLoaders::ifFunction
    );
    
    //End if.
    commitAction(
        MOB_ACTION_END_IF,
        "end_if",
        nullptr,
        nullptr
    );
    
    //Finish dying.
    commitAction(
        MOB_ACTION_FINISH_DYING,
        "finish_dying",
        ScriptActionRunners::finishDying,
        nullptr
    );
    
    //Floor number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_FLOOR_NUMBER,
        "floor_number",
        ScriptActionRunners::floorNumber,
        nullptr
    );
    
    //Focus.
    queueParam("target", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_FOCUS,
        "focus",
        ScriptActionRunners::focus,
        ScriptActionLoaders::focus
    );
    
    //Follow mob as a leader.
    queueParam("target", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("silent", SCRIPT_ACTION_PARAM_BOOL, false, true);
    commitAction(
        MOB_ACTION_FOLLOW_MOB_AS_LEADER,
        "follow_mob_as_leader",
        ScriptActionRunners::followMobAsLeader,
        ScriptActionLoaders::followMobAsLeader
    );
    
    //Follow path randomly.
    queueParam("label", SCRIPT_ACTION_PARAM_STRING, false, true);
    commitAction(
        MOB_ACTION_FOLLOW_PATH_RANDOMLY,
        "follow_path_randomly",
        ScriptActionRunners::followPathRandomly,
        nullptr
    );
    
    //Follow path to absolute.
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("label", SCRIPT_ACTION_PARAM_STRING, false, true);
    commitAction(
        MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
        "follow_path_to_absolute",
        ScriptActionRunners::followPathToAbsolute,
        nullptr
    );
    
    //Get angle.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("center x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("center y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("target x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("target y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_ANGLE,
        "get_angle",
        ScriptActionRunners::getAngle,
        nullptr
    );
    
    //Get angle clockwise difference.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("angle 1", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("angle 2", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_ANGLE_CW_DIFF,
        "get_angle_clockwise_difference",
        ScriptActionRunners::getAngleCwDiff,
        nullptr
    );
    
    //Get angle smallest difference.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("angle 1", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("angle 2", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_ANGLE_SMALLEST_DIFF,
        "get_angle_smallest_difference",
        ScriptActionRunners::getAngleSmallestDiff,
        nullptr
    );
    
    //Get area info.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("info", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_GET_AREA_INFO,
        "get_area_info",
        ScriptActionRunners::getAreaInfo,
        ScriptActionLoaders::getAreaInfo
    );
    
    //Get chomped.
    commitAction(
        MOB_ACTION_GET_CHOMPED,
        "get_chomped",
        ScriptActionRunners::getChomped,
        nullptr
    );
    
    //Get coordinates from angle.
    queueParam("x destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("y destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("angle", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("distance", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_COORDINATES_FROM_ANGLE,
        "get_coordinates_from_angle",
        ScriptActionRunners::getCoordinatesFromAngle,
        nullptr
    );
    
    //Get distance.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("center x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("center y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("target x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("target y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_DISTANCE,
        "get_distance",
        ScriptActionRunners::getDistance,
        nullptr
    );
    
    //Get event info.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("info", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_GET_EVENT_INFO,
        "get_event_info",
        ScriptActionRunners::getEventInfo,
        ScriptActionLoaders::getEventInfo
    );
    
    //Get floor Z.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        ScriptActionRunners::getFloorZ,
        nullptr
    );
    
    //Get focus var.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("focused mob's var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_GET_FOCUS_VAR,
        "get_focus_var",
        ScriptActionRunners::getFocusVar,
        nullptr
    );
    
    //Get mob info.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("target", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("info", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_GET_MOB_INFO,
        "get_mob_info",
        ScriptActionRunners::getMobInfo,
        ScriptActionLoaders::getMobInfo
    );
    
    //Get random float.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("minimum value", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("maximum value", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_GET_RANDOM_FLOAT,
        "get_random_float",
        ScriptActionRunners::getRandomFloat,
        nullptr
    );
    
    //Get random int.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("minimum value", SCRIPT_ACTION_PARAM_INT, false, false);
    queueParam("maximum value", SCRIPT_ACTION_PARAM_INT, false, false);
    commitAction(
        MOB_ACTION_GET_RANDOM_INT,
        "get_random_int",
        ScriptActionRunners::getRandomInt,
        nullptr
    );
    
    //Goto.
    queueParam("label name", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_GOTO,
        "goto",
        nullptr,
        nullptr
    );
    
    //Hold focused mob.
    queueParam("body part name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("hold above", SCRIPT_ACTION_PARAM_BOOL, false, true);
    commitAction(
        MOB_ACTION_HOLD_FOCUS,
        "hold_focused_mob",
        ScriptActionRunners::holdFocus,
        ScriptActionLoaders::holdFocus
    );
    
    //If.
    queueParam("comparand", SCRIPT_ACTION_PARAM_STRING, false, false);
    queueParam("operation", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("value", SCRIPT_ACTION_PARAM_STRING, false, true);
    commitAction(
        MOB_ACTION_IF,
        "if",
        ScriptActionRunners::ifFunction,
        ScriptActionLoaders::ifFunction
    );
    
    //Interpolate number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("input number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("input start", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("input end", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("output start", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("output end", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_INTERPOLATE_NUMBER,
        "interpolate_number",
        ScriptActionRunners::interpolateNumber,
        nullptr
    );
    
    //Label.
    queueParam("label name", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_LABEL,
        "label",
        nullptr,
        nullptr
    );
    
    //Link with focused mob.
    commitAction(
        MOB_ACTION_LINK_WITH_FOCUS,
        "link_with_focused_mob",
        ScriptActionRunners::linkWithFocus,
        nullptr
    );
    
    //Load focused mob memory.
    queueParam("slot", SCRIPT_ACTION_PARAM_INT, false, false);
    commitAction(
        MOB_ACTION_LOAD_FOCUS_MEMORY,
        "load_focused_mob_memory",
        ScriptActionRunners::loadFocusMemory,
        nullptr
    );
    
    //Move to absolute.
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("z", SCRIPT_ACTION_PARAM_FLOAT, false, true);
    commitAction(
        MOB_ACTION_MOVE_TO_ABSOLUTE,
        "move_to_absolute",
        ScriptActionRunners::moveToAbsolute,
        nullptr
    );
    
    //Move to relative.
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("z", SCRIPT_ACTION_PARAM_FLOAT, false, true);
    commitAction(
        MOB_ACTION_MOVE_TO_RELATIVE,
        "move_to_relative",
        ScriptActionRunners::moveToRelative,
        nullptr
    );
    
    //Move to target.
    queueParam("target", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_MOVE_TO_TARGET,
        "move_to_target",
        ScriptActionRunners::moveToTarget,
        ScriptActionLoaders::moveToTarget
    );
    
    //Order release.
    commitAction(
        MOB_ACTION_ORDER_RELEASE,
        "order_release",
        ScriptActionRunners::orderRelease,
        nullptr
    );
    
    //Play sound.
    queueParam("sound data", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam(
        "sound ID destination var name", SCRIPT_ACTION_PARAM_STRING, true, true
    );
    commitAction(
        MOB_ACTION_PLAY_SOUND,
        "play_sound",
        ScriptActionRunners::playSound,
        ScriptActionLoaders::playSound
    );
    
    //Print.
    queueParam("text", SCRIPT_ACTION_PARAM_STRING, false, true);
    commitAction(
        MOB_ACTION_PRINT,
        "print",
        ScriptActionRunners::print,
        nullptr
    );
    
    //Receive status.
    queueParam("status name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_RECEIVE_STATUS,
        "receive_status",
        ScriptActionRunners::receiveStatus,
        ScriptActionLoaders::receiveStatus
    );
    
    //Release.
    commitAction(
        MOB_ACTION_RELEASE,
        "release",
        ScriptActionRunners::release,
        nullptr
    );
    
    //Release stored mobs.
    commitAction(
        MOB_ACTION_RELEASE_STORED_MOBS,
        "release_stored_mobs",
        ScriptActionRunners::releaseStoredMobs,
        nullptr
    );
    
    //Remove status.
    queueParam("status name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_REMOVE_STATUS,
        "remove_status",
        ScriptActionRunners::removeStatus,
        ScriptActionLoaders::removeStatus
    );
    
    //Round number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_ROUND_NUMBER,
        "round_number",
        ScriptActionRunners::roundNumber,
        nullptr
    );
    
    //Save focused mob memory.
    queueParam("slot", SCRIPT_ACTION_PARAM_INT, false, false);
    commitAction(
        MOB_ACTION_SAVE_FOCUS_MEMORY,
        "save_focused_mob_memory",
        ScriptActionRunners::saveFocusMemory,
        nullptr
    );
    
    //Send message to focus.
    queueParam("message", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
        "send_message_to_focus",
        ScriptActionRunners::sendMessageToFocus,
        nullptr
    );
    
    //Send message to links.
    queueParam("message", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SEND_MESSAGE_TO_LINKS,
        "send_message_to_links",
        ScriptActionRunners::sendMessageToLinks,
        nullptr
    );
    
    //Send message to nearby.
    queueParam("distance", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("message", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
        "send_message_to_nearby",
        ScriptActionRunners::sendMessageToNearby,
        nullptr
    );
    
    //Set animation.
    queueParam("animation name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("options", SCRIPT_ACTION_PARAM_ENUM, true, true);
    commitAction(
        MOB_ACTION_SET_ANIMATION,
        "set_animation",
        ScriptActionRunners::setAnimation,
        ScriptActionLoaders::setAnimation
    );
    
    //Set can block paths.
    queueParam("blocks", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_CAN_BLOCK_PATHS,
        "set_can_block_paths",
        ScriptActionRunners::setCanBlockPaths,
        nullptr
    );
    
    //Set far reach.
    queueParam("reach name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_SET_FAR_REACH,
        "set_far_reach",
        ScriptActionRunners::setFarReach,
        ScriptActionLoaders::setFarReach
    );
    
    //Set flying.
    queueParam("flying", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_FLYING,
        "set_flying",
        ScriptActionRunners::setFlying,
        nullptr
    );
    
    //Set focus var.
    queueParam(
        "focused mob's destination var name",
        SCRIPT_ACTION_PARAM_STRING, false, false
    );
    queueParam("value", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SET_FOCUS_VAR,
        "set_focus_var",
        ScriptActionRunners::setFocusVar,
        nullptr
    );
    
    //Set gravity.
    queueParam("multiplier", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_GRAVITY,
        "set_gravity",
        ScriptActionRunners::setGravity,
        nullptr
    );
    
    //Set health.
    queueParam("amount", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_HEALTH,
        "set_health",
        ScriptActionRunners::setHealth,
        nullptr
    );
    
    //Set height.
    queueParam("height", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_HEIGHT,
        "set_height",
        ScriptActionRunners::setHeight,
        nullptr
    );
    
    //Set hiding.
    queueParam("hiding", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_HIDING,
        "set_hiding",
        ScriptActionRunners::setHiding,
        nullptr
    );
    
    //Set huntable.
    queueParam("huntable", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_HUNTABLE,
        "set_huntable",
        ScriptActionRunners::setHuntable,
        nullptr
    );
    
    //Set holdable.
    queueParam("options", SCRIPT_ACTION_PARAM_ENUM, true, true);
    commitAction(
        MOB_ACTION_SET_HOLDABLE,
        "set_holdable",
        ScriptActionRunners::setHoldable,
        ScriptActionLoaders::setHoldable
    );
    
    //Set limb animation.
    queueParam("animation name", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SET_LIMB_ANIMATION,
        "set_limb_animation",
        ScriptActionRunners::setLimbAnimation,
        nullptr
    );
    
    //Set near reach.
    queueParam("reach name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_SET_NEAR_REACH,
        "set_near_reach",
        ScriptActionRunners::setNearReach,
        ScriptActionLoaders::setNearReach
    );
    
    //Set radius.
    queueParam("radius", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_RADIUS,
        "set_radius",
        ScriptActionRunners::setRadius,
        nullptr
    );
    
    //Set sector scroll.
    queueParam("x speed", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y speed", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_SECTOR_SCROLL,
        "set_sector_scroll",
        ScriptActionRunners::setSectorScroll,
        nullptr
    );
    
    //Set shadow visibility.
    queueParam("visible", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_SHADOW_VISIBILITY,
        "set_shadow_visibility",
        ScriptActionRunners::setShadowVisibility,
        nullptr
    );
    
    //Set state.
    queueParam("state name", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_SET_STATE,
        "set_state",
        ScriptActionRunners::setState,
        nullptr
    );
    
    //Set tangible.
    queueParam("tangible", SCRIPT_ACTION_PARAM_BOOL, false, false);
    commitAction(
        MOB_ACTION_SET_TANGIBLE,
        "set_tangible",
        ScriptActionRunners::setTangible,
        nullptr
    );
    
    //Set team.
    queueParam("team name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_SET_TEAM,
        "set_team",
        ScriptActionRunners::setTeam,
        ScriptActionLoaders::setTeam
    );
    
    //Set timer.
    queueParam("time", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SET_TIMER,
        "set_timer",
        ScriptActionRunners::setTimer,
        nullptr
    );
    
    //Set var.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("value", SCRIPT_ACTION_PARAM_STRING, false, false);
    commitAction(
        MOB_ACTION_SET_VAR,
        "set_var",
        ScriptActionRunners::setVar,
        nullptr
    );
    
    //Shake camera.
    queueParam("amount", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SHAKE_CAMERA,
        "shake_camera",
        ScriptActionRunners::shakeCamera,
        nullptr
    );
    
    //Show message from var.
    queueParam("var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    commitAction(
        MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
        "show_message_from_var",
        ScriptActionRunners::showMessageFromVar,
        nullptr
    );
    
    //Square root number.
    queueParam("destination var name", SCRIPT_ACTION_PARAM_STRING, true, false);
    queueParam("number", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_SQUARE_ROOT_NUMBER,
        "square_root_number",
        ScriptActionRunners::squareRootNumber,
        nullptr
    );
    
    //Spawn.
    queueParam("spawn data", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_SPAWN,
        "spawn",
        ScriptActionRunners::spawn,
        ScriptActionLoaders::spawn
    );
    
    //Stabilize Z.
    queueParam("reference", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("offset", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_STABILIZE_Z,
        "stabilize_z",
        ScriptActionRunners::stabilizeZ,
        ScriptActionLoaders::stabilizeZ
    );
    
    //Start chomping.
    queueParam("victim max", SCRIPT_ACTION_PARAM_INT, false, false);
    queueParam("body part", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("more body parts", SCRIPT_ACTION_PARAM_ENUM, true, true);
    commitAction(
        MOB_ACTION_START_CHOMPING,
        "start_chomping",
        ScriptActionRunners::startChomping,
        ScriptActionLoaders::startChomping
    );
    
    //Start dying.
    commitAction(
        MOB_ACTION_START_DYING,
        "start_dying",
        ScriptActionRunners::startDying,
        nullptr
    );
    
    //Start height effect.
    commitAction(
        MOB_ACTION_START_HEIGHT_EFFECT,
        "start_height_effect",
        ScriptActionRunners::startHeightEffect,
        nullptr
    );
    
    //Start particles.
    queueParam("generator name", SCRIPT_ACTION_PARAM_ENUM, true, false);
    queueParam("offset coordinates", SCRIPT_ACTION_PARAM_FLOAT, false, true);
    commitAction(
        MOB_ACTION_START_PARTICLES,
        "start_particles",
        ScriptActionRunners::startParticles,
        ScriptActionLoaders::startParticles
    );
    
    //Stop.
    commitAction(
        MOB_ACTION_STOP,
        "stop",
        ScriptActionRunners::stop,
        nullptr
    );
    
    //Stop chomping.
    commitAction(
        MOB_ACTION_STOP_CHOMPING,
        "stop_chomping",
        ScriptActionRunners::stopChomping,
        nullptr
    );
    
    //Stop height effect.
    commitAction(
        MOB_ACTION_STOP_HEIGHT_EFFECT,
        "stop_height_effect",
        ScriptActionRunners::stopHeightEffect,
        nullptr
    );
    
    //Stop particles.
    commitAction(
        MOB_ACTION_STOP_PARTICLES,
        "stop_particles",
        ScriptActionRunners::stopParticles,
        nullptr
    );
    
    //Stop sound.
    queueParam("sound ID", SCRIPT_ACTION_PARAM_INT, false, false);
    commitAction(
        MOB_ACTION_STOP_SOUND,
        "stop_sound",
        ScriptActionRunners::stopSound,
        nullptr
    );
    
    //Stop vertically.
    commitAction(
        MOB_ACTION_STOP_VERTICALLY,
        "stop_vertically",
        ScriptActionRunners::stopVertically,
        nullptr
    );
    
    //Store focus inside.
    commitAction(
        MOB_ACTION_STORE_FOCUS_INSIDE,
        "store_focus_inside",
        ScriptActionRunners::storeFocusInside,
        nullptr
    );
    
    //Swallow.
    queueParam("amount", SCRIPT_ACTION_PARAM_INT, false, false);
    commitAction(
        MOB_ACTION_SWALLOW,
        "swallow",
        ScriptActionRunners::swallow,
        nullptr
    );
    
    //Swallow all.
    commitAction(
        MOB_ACTION_SWALLOW_ALL,
        "swallow_all",
        ScriptActionRunners::swallowAll,
        nullptr
    );
    
    //Teleport to absolute.
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("z", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_TELEPORT_TO_ABSOLUTE,
        "teleport_to_absolute",
        ScriptActionRunners::teleportToAbsolute,
        nullptr
    );
    
    //Teleport to relative.
    queueParam("x", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("z", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_TELEPORT_TO_RELATIVE,
        "teleport_to_relative",
        ScriptActionRunners::teleportToRelative,
        nullptr
    );
    
    //Throw focused mob.
    queueParam("x coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("z coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("max height", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    commitAction(
        MOB_ACTION_THROW_FOCUS,
        "throw_focused_mob",
        ScriptActionRunners::throwFocus,
        nullptr
    );
    
    //Turn to absolute.
    queueParam("angle or x coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, true);
    commitAction(
        MOB_ACTION_TURN_TO_ABSOLUTE,
        "turn_to_absolute",
        ScriptActionRunners::turnToAbsolute,
        nullptr
    );
    
    //Turn to relative.
    queueParam("angle or x coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, false);
    queueParam("y coordinate", SCRIPT_ACTION_PARAM_FLOAT, false, true);
    commitAction(
        MOB_ACTION_TURN_TO_RELATIVE,
        "turn_to_relative",
        ScriptActionRunners::turnToRelative,
        nullptr
    );
    
    //Turn to target.
    queueParam("target", SCRIPT_ACTION_PARAM_ENUM, true, false);
    commitAction(
        MOB_ACTION_TURN_TO_TARGET,
        "turn_to_target",
        ScriptActionRunners::turnToTarget,
        ScriptActionLoaders::turnToTarget
    );
    
    
#undef queueParam
#undef commitAction
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


#pragma endregion
