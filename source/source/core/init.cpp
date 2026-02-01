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
#include "../content/other/mob_script.h"
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
    
    game.sectorTypes.clear();
    game.missionPresetNames.clear();
    game.missionMobChecklistTypeNames.clear();
    game.missionScoreCriterionTypeNames.clear();
    game.missionHudItemIdNames.clear();
    game.missionHudItemContentTypeNames.clear();
    game.missionHudItemAmountTypeNames.clear();
    for(size_t e = 0; e < game.missionEvTypes.size(); e++) {
        delete game.missionEvTypes[e];
    }
    for(size_t a = 0; a < game.missionActionTypes.size(); a++) {
        delete game.missionActionTypes[a];
    }
    game.missionGoals.clear();
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
    
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_NONE,
        PLAYER_ACTION_CAT_NONE,
        "---", "", "", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    
    //Main.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Move right",
        "Move the leader right.",
        "move_right", "k_4", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Move down",
        "Move the leader down.",
        "move_down", "k_19", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Move left",
        "Move the leader left.",
        "move_left", "k_1", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Move up",
        "Move the leader up.",
        "move_up", "k_23", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor right",
        "Move the leader's cursor right.",
        "cursor_right", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor down",
        "Move the leader's cursor down.",
        "cursor_down", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor left",
        "Move the leader's cursor left.",
        "cursor_left", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_LEADER_CURSOR_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Leader cursor up",
        "Move the leader's cursor up.",
        "cursor_up", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_THROW,
        PLAYER_ACTION_CAT_MAIN,
        "Throw",
        "Throw a Pikmin at the leader's cursor.",
        "throw", "mb_1", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.2f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_WHISTLE,
        PLAYER_ACTION_CAT_MAIN,
        "Whistle",
        "Whistle around the leader's cursor.",
        "whistle", "mb_2", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_NEXT_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Next Pikmin",
        "Change to the next Pikmin type in the group.",
        "next_type", "mwd", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_PREV_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. Pikmin",
        "Change to the previous Pikmin type in the group.",
        "prev_type", "mwu", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_NEXT_LEADER,
        PLAYER_ACTION_CAT_MAIN,
        "Next leader",
        "Change to the next leader.",
        "next_leader", "k_215", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_GROUP_CURSOR,
        PLAYER_ACTION_CAT_MAIN,
        "Swarm to cursor",
        "Swarm all Pikmin towards the leader's cursor.",
        "swarm_cursor", "k_75", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_DISMISS,
        PLAYER_ACTION_CAT_MAIN,
        "Dismiss",
        "Dismiss all Pikmin.",
        "dismiss", "k_217", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_INVENTORY,
        PLAYER_ACTION_CAT_MAIN,
        "Open inventory",
        "Open the item and special move inventory.",
        "inventory", "k_17", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_PAUSE,
        PLAYER_ACTION_CAT_MAIN,
        "Pause",
        "Pause the game.",
        "pause", "k_59", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, true
    );
    
    //Menus.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu right",
        "Navigate right in a menu.",
        "menu_right", "k_83", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Menu down",
        "Navigate down in a menu.",
        "menu_down", "k_85", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu left",
        "Navigate left in a menu.",
        "menu_left", "k_82", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Menu up",
        "Navigate up in a menu.",
        "menu_up", "k_84", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_OK,
        PLAYER_ACTION_CAT_MENUS,
        "Menu OK",
        "Confirm the selected item in a menu.",
        "menu_ok", "k_67", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.4f, false, true
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan right",
        "Pan the radar to the right.",
        "menu_radar_right", "k_4", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan down",
        "Pan the radar downward.",
        "menu_radar_down", "k_19", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan left",
        "Pan the radar to the left.",
        "menu_radar_left", "k_1", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan up",
        "Pan the radar upward.",
        "menu_radar_up", "k_23", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom in",
        "Zoom the radar in.",
        "menu_radar_zoom_in", "k_18", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom out",
        "Zoom the radar out.",
        "menu_radar_zoom_out", "k_6", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_ONION_CHANGE_10,
        PLAYER_ACTION_CAT_MENUS,
        "Onion menu change 10",
        "Toggle the \"change 10\" mode in the Onion menu.",
        "menu_onion_change_10", "k_215", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_ONION_SELECT_ALL,
        PLAYER_ACTION_CAT_MENUS,
        "Onion menu select all",
        "Toggle the \"select all\" mode in the Onion menu.",
        "menu_onion_select_all", "k_217", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    
    //Advanced.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_GROUP_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm right",
        "Swarm all Pikmin right.",
        "swarm_right", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_GROUP_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm down",
        "Swarm all Pikmin down.",
        "swarm_down", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_GROUP_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm left",
        "Swarm all Pikmin left.",
        "swarm_left", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_GROUP_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm up",
        "Swarm all Pikmin up.",
        "swarm_up", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_PREV_LEADER,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. leader",
        "Change to the previous leader.",
        "prev_leader", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_CAT_ADVANCED,
        "Change zoom",
        "Change the current zoom level.",
        "change_zoom", "k_3", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom in",
        "Change to a closer zoom level.",
        "zoom_in", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_ZOOM_OUT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom out",
        "Change to a farther zoom level.",
        "zoom_out", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_NEXT_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Next maturity",
        "Change to a Pikmin of the next maturity.",
        "next_maturity", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_PREV_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. maturity",
        "Change to a Pikmin of the previous maturity.",
        "prev_maturity", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut A",
        "Use the inventory item set to shortcut A.",
        "inventory_shortcut_a", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut B",
        "Use the inventory item set to shortcut B.",
        "inventory_shortcut_b", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut C",
        "Use the inventory item set to shortcut C.",
        "inventory_shortcut_c", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_INVENTORY_SHORTCUT_D,
        PLAYER_ACTION_CAT_ADVANCED,
        "Inventory shortcut D",
        "Use the inventory item set to shortcut D.",
        "inventory_shortcut_d", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_CUSTOM_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom A",
        "Custom action A, if the current leader supports it.",
        "custom_a", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_CUSTOM_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom B",
        "Custom action B, if the current leader supports it.",
        "custom_b", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_CUSTOM_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom C",
        "Custom action C, if the current leader supports it.",
        "custom_c", "", Inpution::ACTION_VALUE_TYPE_ANALOG,
        0.0f, 0.0f, true, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_RADAR,
        PLAYER_ACTION_CAT_ADVANCED,
        "Radar",
        "Open or close the radar.",
        "radar", "k_64", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_BACK,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - back",
        "Go back or cancel in a menu.",
        "menu_back", "k_59", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.4f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - left page",
        "Go to the page to the left in a menu.",
        "menu_page_left", "k_17", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - right page",
        "Go to the page to the right in a menu.",
        "menu_page_right", "k_5", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    
    //General maker tool things.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_AUTO_START,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Auto-start",
        "Make the game auto-start on the current state (and content).",
        "mt_auto_start", "k_56", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Set song pos near loop",
        "Set the current song's position to be near the loop point.",
        "mt_set_song_pos_near_loop", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_MOD_1,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 1",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_1", "k_216", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_MOD_2,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 2",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_2", "k_218", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    
    //Gameplay maker tools.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Area image",
        "Save an image of the current area.",
        "mt_area_image", "k_36", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Change speed",
        "Change the gameplay speed.",
        "mt_change_speed", "k_28", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_FRAME_ADVANCE,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Frame advance",
        "Pause gameplay and advance by one frame each time. "
        "Use \"modifier 1\" to unpause.",
        "mt_frame_advance", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Geometry info",
        "Toggle info about the geometry under the mouse cursor.",
        "mt_geometry_info", "k_33", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_HUD,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "HUD",
        "Toggle the HUD.",
        "mt_hud", "k_35", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_HURT_MOB,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Hurt mob",
        "Hurt the mob under the mouse cursor.",
        "mt_hurt_mob", "k_30", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_MOB_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Mob info",
        "Toggle info about the mob under the mouse cursor.",
        "mt_mob_info", "k_32", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_NEW_PIKMIN,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "New Pikmin",
        "Create a new Pikmin under the mouse cursor.",
        "mt_new_pikmin", "k_31", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_PATH_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Path info",
        "Toggle info about paths the info'd mob is taking.",
        "mt_path_info", "k_34", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_COLLISION,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show collision",
        "Toggle drawing each mob's collision.",
        "mt_show_collision", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show hitboxes",
        "Toggle drawing each mob's hitboxes.",
        "mt_show_hitboxes", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_REACHES,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show reaches",
        "Toggle drawing the info'd mob's reaches.",
        "mt_show_reaches", "", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_MT_TELEPORT,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Teleport",
        "Teleport the leader to the mouse cursor.",
        "mt_teleport", "k_29", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.5f, 0.0f, false, false
    );
    
    //System.
    game.controls.addActionType(
        PLAYER_ACTION_TYPE_SYSTEM_INFO,
        PLAYER_ACTION_CAT_SYSTEM,
        "System info",
        "Toggle showing system and performance information.",
        "system_info", "k_47", Inpution::ACTION_VALUE_TYPE_DIGITAL,
        0.0f, 0.0f, false, false
    );
    game.controls.addActionType(
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
    
    game.debug.showPlayerActions = getProp("show_player_actions");
    game.debug.showAreaActiveCells = getProp("show_area_active_cells");
    game.debug.showControllerEvents = getProp("show_controller_events");
    game.debug.showDearImGuiDemo = getProp("show_dear_imgui_demo");
    game.debug.showGroupInfo = getProp("show_group_info");
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
    //Sector types.
    game.sectorTypes.registerItem(
        SECTOR_TYPE_NORMAL, "normal"
    );
    game.sectorTypes.registerItem(
        SECTOR_TYPE_BLOCKING, "blocking"
    );
    
    //Mission preset names.
    game.missionPresetNames.registerItem(
        MISSION_PRESET_GROW_PIKMIN, "Grow Pikmin"
    );
    game.missionPresetNames.registerItem(
        MISSION_PRESET_COLLECT_TREASURE, "Collect Treasure"
    );
    game.missionPresetNames.registerItem(
        MISSION_PRESET_BATTLE_ENEMIES, "Battle Enemies"
    );
    game.missionPresetNames.registerItem(
        MISSION_PRESET_DEFEAT_BOSSES, "Defeat Bosses"
    );
    game.missionPresetNames.registerItem(
        MISSION_PRESET_COLLECT_EVERYTHING, "Collect Everything"
    );
    game.missionPresetNames.registerItem(
        MISSION_PRESET_CUSTOM, "Custom"
    );
    
    //Mission mob checklist type names.
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_CUSTOM, "Custom"
    );
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_TREASURES, "Treasures"
    );
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_ENEMIES, "Enemies"
    );
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_TREASURES_ENEMIES, "Treasures and enemies"
    );
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_LEADERS, "Leaders"
    );
    game.missionMobChecklistTypeNames.registerItem(
        MISSION_MOB_CHECKLIST_PIKMIN, "Pikmin"
    );
    
    //Mission score criterion type names.
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_MOB_CHECKLIST, "Mob checklist item"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_PIKMIN, "Total Pikmin"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_PIKMIN_BORN, "Pikmin born"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_PIKMIN_DEATHS, "Pikmin deaths"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_SEC_LEFT, "Seconds left"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_SEC_PASSED, "Seconds passed"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_COLLECTION_PTS, "Mob collection points"
    );
    game.missionScoreCriterionTypeNames.registerItem(
        MISSION_SCORE_CRITERION_DEFEAT_PTS, "Enemy defeat points"
    );
    
    //Mission HUD item ID names.
    game.missionHudItemIdNames.registerItem(
        MISSION_HUD_ITEM_ID_GOAL_MAIN, "Goal, main"
    );
    game.missionHudItemIdNames.registerItem(
        MISSION_HUD_ITEM_ID_GOAL_SEC, "Goal, secondary"
    );
    game.missionHudItemIdNames.registerItem(
        MISSION_HUD_ITEM_ID_FAIL_MAIN, "Failure, main"
    );
    game.missionHudItemIdNames.registerItem(
        MISSION_HUD_ITEM_ID_FAIL_SEC, "Failure, secondary"
    );
    
    //Mission HUD item content type names.
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_TEXT, "Custom text"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_CLOCK, "Clock"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_SCORE, "Score"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_CUR_TOT, "Current amount / total"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_REM_TOT, "Remaining amount / total"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_CUR_AMT, "Current amount"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_REM_AMT, "Remaining amount"
    );
    game.missionHudItemContentTypeNames.registerItem(
        MISSION_HUD_ITEM_CONTENT_TOT_AMT, "Total amount"
    );
    
    //Mission HUD item amount type names.
    game.missionHudItemAmountTypeNames.registerItem(
        MISSION_HUD_ITEM_AMT_MOB_CHECKLIST, "Mob checklist"
    );
    game.missionHudItemAmountTypeNames.registerItem(
        MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION, "Leaders in region"
    );
    game.missionHudItemAmountTypeNames.registerItem(
        MISSION_HUD_ITEM_AMT_PIKMIN, "Pikmin count"
    );
    game.missionHudItemAmountTypeNames.registerItem(
        MISSION_HUD_ITEM_AMT_LEADERS, "Leader count"
    );
    game.missionHudItemAmountTypeNames.registerItem(
        MISSION_HUD_ITEM_AMT_PIKMIN_DEATHS, "Pikmin deaths"
    );
    
    //Mission events.
    //Order matters, and should match MISSION_EV.
    game.missionEvTypes = {
        new MissionEvTypePauseEnd(),
        new MissionEvTypeMobChecklist(),
        new MissionEvTypeTimeLimit(),
        new MissionEvTypeLeadersInRegion(),
        new MissionEvTypePikminOrMore(),
        new MissionEvTypePikminOrFewer(),
        new MissionEvTypeLosePikmin(),
        new MissionEvTypeLoseLeaders(),
        new MissionEvTypeTakeDamage(),
    };
    
    //Mission actions.
    //Order matters, and should match MISSION_ACTION.
    game.missionActionTypes = {
        new MissionActionTypeEndClear(),
        new MissionActionTypeEndFail(),
        new MissionActionTypeScriptMessage(),
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
 * @brief Initializes the list of mob actions.
 */
void initMobActions() {

#define regParam(pName, pType, constant, extras) \
    params.push_back(MobActionParam(pName, pType, constant, extras));
#define regAction(aType, aName, runCode, loadCode) \
    a = &(game.mobActions[aType]); \
    a->type = aType; \
    a->name = aName; \
    a->code = runCode; \
    a->extraLoadLogic = loadCode; \
    a->parameters = params; \
    params.clear();


    game.mobActions.assign(N_MOB_ACTIONS, MobAction());
    vector<MobActionParam> params;
    MobAction* a;
    
    //Unknown.
    regAction(
        MOB_ACTION_UNKNOWN,
        "unknown",
        nullptr,
        nullptr
    );
    
    //Absolute number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_ABSOLUTE_NUMBER,
        "absolute_number",
        MobActionRunners::absoluteNumber,
        nullptr
    );
    
    //Add health.
    regParam("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_ADD_HEALTH,
        "add_health",
        MobActionRunners::addHealth,
        nullptr
    );
    
    //Arachnorb plan logic.
    regParam("goal", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_ARACHNORB_PLAN_LOGIC,
        "arachnorb_plan_logic",
        MobActionRunners::arachnorbPlanLogic,
        MobActionLoaders::arachnorbPlanLogic
    );
    
    //Calculate.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("left operand", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("operation", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("right operand", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_CALCULATE,
        "calculate",
        MobActionRunners::calculate,
        MobActionLoaders::calculate
    );
    
    //Ceil number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_CEIL_NUMBER,
        "ceil_number",
        MobActionRunners::ceilNumber,
        nullptr
    );
    
    //Delete.
    regAction(
        MOB_ACTION_DELETE,
        "delete",
        MobActionRunners::deleteFunction,
        nullptr
    );
    
    //Drain liquid.
    regAction(
        MOB_ACTION_DRAIN_LIQUID,
        "drain_liquid",
        MobActionRunners::drainLiquid,
        nullptr
    );
    
    //Ease number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("method", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_EASE_NUMBER,
        "ease_number",
        MobActionRunners::easeNumber,
        MobActionLoaders::easeNumber
    );
    
    //Else.
    regAction(
        MOB_ACTION_ELSE,
        "else",
        nullptr,
        nullptr
    );
    
    //Else if.
    regParam("comparand", MOB_ACTION_PARAM_STRING, false, false);
    regParam("operation", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("value", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_ELSE_IF,
        "else_if",
        MobActionRunners::ifFunction,
        MobActionLoaders::ifFunction
    );
    
    //End if.
    regAction(
        MOB_ACTION_END_IF,
        "end_if",
        nullptr,
        nullptr
    );
    
    //Finish dying.
    regAction(
        MOB_ACTION_FINISH_DYING,
        "finish_dying",
        MobActionRunners::finishDying,
        nullptr
    );
    
    //Floor number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_FLOOR_NUMBER,
        "floor_number",
        MobActionRunners::floorNumber,
        nullptr
    );
    
    //Focus.
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_FOCUS,
        "focus",
        MobActionRunners::focus,
        MobActionLoaders::focus
    );
    
    //Follow mob as a leader.
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("silent", MOB_ACTION_PARAM_BOOL, false, true);
    regAction(
        MOB_ACTION_FOLLOW_MOB_AS_LEADER,
        "follow_mob_as_leader",
        MobActionRunners::followMobAsLeader,
        MobActionLoaders::followMobAsLeader
    );
    
    //Follow path randomly.
    regParam("label", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_FOLLOW_PATH_RANDOMLY,
        "follow_path_randomly",
        MobActionRunners::followPathRandomly,
        nullptr
    );
    
    //Follow path to absolute.
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("label", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
        "follow_path_to_absolute",
        MobActionRunners::followPathToAbsolute,
        nullptr
    );
    
    //Get angle.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("center x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("center y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("target x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("target y", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_ANGLE,
        "get_angle",
        MobActionRunners::getAngle,
        nullptr
    );
    
    //Get angle clockwise difference.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("angle 1", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("angle 2", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_ANGLE_CW_DIFF,
        "get_angle_clockwise_difference",
        MobActionRunners::getAngleCwDiff,
        nullptr
    );
    
    //Get angle smallest difference.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("angle 1", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("angle 2", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_ANGLE_SMALLEST_DIFF,
        "get_angle_smallest_difference",
        MobActionRunners::getAngleSmallestDiff,
        nullptr
    );
    
    //Get area info.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_AREA_INFO,
        "get_area_info",
        MobActionRunners::getAreaInfo,
        MobActionLoaders::getAreaInfo
    );
    
    //Get chomped.
    regAction(
        MOB_ACTION_GET_CHOMPED,
        "get_chomped",
        MobActionRunners::getChomped,
        nullptr
    );
    
    //Get coordinates from angle.
    regParam("x destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("y destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("angle", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("distance", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_COORDINATES_FROM_ANGLE,
        "get_coordinates_from_angle",
        MobActionRunners::getCoordinatesFromAngle,
        nullptr
    );
    
    //Get distance.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("center x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("center y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("target x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("target y", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_DISTANCE,
        "get_distance",
        MobActionRunners::getDistance,
        nullptr
    );
    
    //Get event info.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_EVENT_INFO,
        "get_event_info",
        MobActionRunners::getEventInfo,
        MobActionLoaders::getEventInfo
    );
    
    //Get floor Z.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        MobActionRunners::getFloorZ,
        nullptr
    );
    
    //Get focus var.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("focused mob's var name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_FOCUS_VAR,
        "get_focus_var",
        MobActionRunners::getFocusVar,
        nullptr
    );
    
    //Get mob info.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("target", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_MOB_INFO,
        "get_mob_info",
        MobActionRunners::getMobInfo,
        MobActionLoaders::getMobInfo
    );
    
    //Get random float.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("minimum value", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("maximum value", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_RANDOM_FLOAT,
        "get_random_float",
        MobActionRunners::getRandomFloat,
        nullptr
    );
    
    //Get random int.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("minimum value", MOB_ACTION_PARAM_INT, false, false);
    regParam("maximum value", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_GET_RANDOM_INT,
        "get_random_int",
        MobActionRunners::getRandomInt,
        nullptr
    );
    
    //Goto.
    regParam("label name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GOTO,
        "goto",
        nullptr,
        nullptr
    );
    
    //Hold focused mob.
    regParam("body part name", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("hold above", MOB_ACTION_PARAM_BOOL, false, true);
    regAction(
        MOB_ACTION_HOLD_FOCUS,
        "hold_focused_mob",
        MobActionRunners::holdFocus,
        MobActionLoaders::holdFocus
    );
    
    //If.
    regParam("comparand", MOB_ACTION_PARAM_STRING, false, false);
    regParam("operation", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("value", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_IF,
        "if",
        MobActionRunners::ifFunction,
        MobActionLoaders::ifFunction
    );
    
    //Interpolate number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("input number", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("input start", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("input end", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("output start", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("output end", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_INTERPOLATE_NUMBER,
        "interpolate_number",
        MobActionRunners::interpolateNumber,
        nullptr
    );
    
    //Label.
    regParam("label name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_LABEL,
        "label",
        nullptr,
        nullptr
    );
    
    //Link with focused mob.
    regAction(
        MOB_ACTION_LINK_WITH_FOCUS,
        "link_with_focused_mob",
        MobActionRunners::linkWithFocus,
        nullptr
    );
    
    //Load focused mob memory.
    regParam("slot", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_LOAD_FOCUS_MEMORY,
        "load_focused_mob_memory",
        MobActionRunners::loadFocusMemory,
        nullptr
    );
    
    //Move to absolute.
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_MOVE_TO_ABSOLUTE,
        "move_to_absolute",
        MobActionRunners::moveToAbsolute,
        nullptr
    );
    
    //Move to relative.
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_MOVE_TO_RELATIVE,
        "move_to_relative",
        MobActionRunners::moveToRelative,
        nullptr
    );
    
    //Move to target.
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_MOVE_TO_TARGET,
        "move_to_target",
        MobActionRunners::moveToTarget,
        MobActionLoaders::moveToTarget
    );
    
    //Order release.
    regAction(
        MOB_ACTION_ORDER_RELEASE,
        "order_release",
        MobActionRunners::orderRelease,
        nullptr
    );
    
    //Play sound.
    regParam("sound data", MOB_ACTION_PARAM_ENUM, true, false);
    regParam(
        "sound ID destination var name", MOB_ACTION_PARAM_STRING, true, true
    );
    regAction(
        MOB_ACTION_PLAY_SOUND,
        "play_sound",
        MobActionRunners::playSound,
        MobActionLoaders::playSound
    );
    
    //Print.
    regParam("text", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_PRINT,
        "print",
        MobActionRunners::print,
        nullptr
    );
    
    //Receive status.
    regParam("status name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_RECEIVE_STATUS,
        "receive_status",
        MobActionRunners::receiveStatus,
        MobActionLoaders::receiveStatus
    );
    
    //Release.
    regAction(
        MOB_ACTION_RELEASE,
        "release",
        MobActionRunners::release,
        nullptr
    );
    
    //Release stored mobs.
    regAction(
        MOB_ACTION_RELEASE_STORED_MOBS,
        "release_stored_mobs",
        MobActionRunners::releaseStoredMobs,
        nullptr
    );
    
    //Remove status.
    regParam("status name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_REMOVE_STATUS,
        "remove_status",
        MobActionRunners::removeStatus,
        MobActionLoaders::removeStatus
    );
    
    //Round number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_ROUND_NUMBER,
        "round_number",
        MobActionRunners::roundNumber,
        nullptr
    );
    
    //Save focused mob memory.
    regParam("slot", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_SAVE_FOCUS_MEMORY,
        "save_focused_mob_memory",
        MobActionRunners::saveFocusMemory,
        nullptr
    );
    
    //Send message to focus.
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
        "send_message_to_focus",
        MobActionRunners::sendMessageToFocus,
        nullptr
    );
    
    //Send message to links.
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_LINKS,
        "send_message_to_links",
        MobActionRunners::sendMessageToLinks,
        nullptr
    );
    
    //Send message to nearby.
    regParam("distance", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
        "send_message_to_nearby",
        MobActionRunners::sendMessageToNearby,
        nullptr
    );
    
    //Set animation.
    regParam("animation name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("options", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_SET_ANIMATION,
        "set_animation",
        MobActionRunners::setAnimation,
        MobActionLoaders::setAnimation
    );
    
    //Set can block paths.
    regParam("blocks", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_CAN_BLOCK_PATHS,
        "set_can_block_paths",
        MobActionRunners::setCanBlockPaths,
        nullptr
    );
    
    //Set far reach.
    regParam("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_FAR_REACH,
        "set_far_reach",
        MobActionRunners::setFarReach,
        MobActionLoaders::setFarReach
    );
    
    //Set flying.
    regParam("flying", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_FLYING,
        "set_flying",
        MobActionRunners::setFlying,
        nullptr
    );
    
    //Set focus var.
    regParam(
        "focused mob's destination var name",
        MOB_ACTION_PARAM_STRING, false, false
    );
    regParam("value", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SET_FOCUS_VAR,
        "set_focus_var",
        MobActionRunners::setFocusVar,
        nullptr
    );
    
    //Set gravity.
    regParam("multiplier", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_GRAVITY,
        "set_gravity",
        MobActionRunners::setGravity,
        nullptr
    );
    
    //Set health.
    regParam("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_HEALTH,
        "set_health",
        MobActionRunners::setHealth,
        nullptr
    );
    
    //Set height.
    regParam("height", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_HEIGHT,
        "set_height",
        MobActionRunners::setHeight,
        nullptr
    );
    
    //Set hiding.
    regParam("hiding", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_HIDING,
        "set_hiding",
        MobActionRunners::setHiding,
        nullptr
    );
    
    //Set huntable.
    regParam("huntable", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_HUNTABLE,
        "set_huntable",
        MobActionRunners::setHuntable,
        nullptr
    );
    
    //Set holdable.
    regParam("options", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_SET_HOLDABLE,
        "set_holdable",
        MobActionRunners::setHoldable,
        MobActionLoaders::setHoldable
    );
    
    //Set limb animation.
    regParam("animation name", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SET_LIMB_ANIMATION,
        "set_limb_animation",
        MobActionRunners::setLimbAnimation,
        nullptr
    );
    
    //Set near reach.
    regParam("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_NEAR_REACH,
        "set_near_reach",
        MobActionRunners::setNearReach,
        MobActionLoaders::setNearReach
    );
    
    //Set radius.
    regParam("radius", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_RADIUS,
        "set_radius",
        MobActionRunners::setRadius,
        nullptr
    );
    
    //Set sector scroll.
    regParam("x speed", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y speed", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_SECTOR_SCROLL,
        "set_sector_scroll",
        MobActionRunners::setSectorScroll,
        nullptr
    );
    
    //Set shadow visibility.
    regParam("visible", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_SHADOW_VISIBILITY,
        "set_shadow_visibility",
        MobActionRunners::setShadowVisibility,
        nullptr
    );
    
    //Set state.
    regParam("state name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_SET_STATE,
        "set_state",
        MobActionRunners::setState,
        nullptr
    );
    
    //Set tangible.
    regParam("tangible", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_TANGIBLE,
        "set_tangible",
        MobActionRunners::setTangible,
        nullptr
    );
    
    //Set team.
    regParam("team name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_TEAM,
        "set_team",
        MobActionRunners::setTeam,
        MobActionLoaders::setTeam
    );
    
    //Set timer.
    regParam("time", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_TIMER,
        "set_timer",
        MobActionRunners::setTimer,
        nullptr
    );
    
    //Set var.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("value", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SET_VAR,
        "set_var",
        MobActionRunners::setVar,
        nullptr
    );
    
    //Shake camera.
    regParam("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SHAKE_CAMERA,
        "shake_camera",
        MobActionRunners::shakeCamera,
        nullptr
    );
    
    //Show message from var.
    regParam("var name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
        "show_message_from_var",
        MobActionRunners::showMessageFromVar,
        nullptr
    );
    
    //Square root number.
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("number", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SQUARE_ROOT_NUMBER,
        "square_root_number",
        MobActionRunners::squareRootNumber,
        nullptr
    );
    
    //Spawn.
    regParam("spawn data", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SPAWN,
        "spawn",
        MobActionRunners::spawn,
        MobActionLoaders::spawn
    );
    
    //Stabilize Z.
    regParam("reference", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("offset", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_STABILIZE_Z,
        "stabilize_z",
        MobActionRunners::stabilizeZ,
        MobActionLoaders::stabilizeZ
    );
    
    //Start chomping.
    regParam("victim max", MOB_ACTION_PARAM_INT, false, false);
    regParam("body part", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("more body parts", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_START_CHOMPING,
        "start_chomping",
        MobActionRunners::startChomping,
        MobActionLoaders::startChomping
    );
    
    //Start dying.
    regAction(
        MOB_ACTION_START_DYING,
        "start_dying",
        MobActionRunners::startDying,
        nullptr
    );
    
    //Start height effect.
    regAction(
        MOB_ACTION_START_HEIGHT_EFFECT,
        "start_height_effect",
        MobActionRunners::startHeightEffect,
        nullptr
    );
    
    //Start particles.
    regParam("generator name", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("offset coordinates", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_START_PARTICLES,
        "start_particles",
        MobActionRunners::startParticles,
        MobActionLoaders::startParticles
    );
    
    //Stop.
    regAction(
        MOB_ACTION_STOP,
        "stop",
        MobActionRunners::stop,
        nullptr
    );
    
    //Stop chomping.
    regAction(
        MOB_ACTION_STOP_CHOMPING,
        "stop_chomping",
        MobActionRunners::stopChomping,
        nullptr
    );
    
    //Stop height effect.
    regAction(
        MOB_ACTION_STOP_HEIGHT_EFFECT,
        "stop_height_effect",
        MobActionRunners::stopHeightEffect,
        nullptr
    );
    
    //Stop particles.
    regAction(
        MOB_ACTION_STOP_PARTICLES,
        "stop_particles",
        MobActionRunners::stopParticles,
        nullptr
    );
    
    //Stop sound.
    regParam("sound ID", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_STOP_SOUND,
        "stop_sound",
        MobActionRunners::stopSound,
        nullptr
    );
    
    //Stop vertically.
    regAction(
        MOB_ACTION_STOP_VERTICALLY,
        "stop_vertically",
        MobActionRunners::stopVertically,
        nullptr
    );
    
    //Store focus inside.
    regAction(
        MOB_ACTION_STORE_FOCUS_INSIDE,
        "store_focus_inside",
        MobActionRunners::storeFocusInside,
        nullptr
    );
    
    //Swallow.
    regParam("amount", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_SWALLOW,
        "swallow",
        MobActionRunners::swallow,
        nullptr
    );
    
    //Swallow all.
    regAction(
        MOB_ACTION_SWALLOW_ALL,
        "swallow_all",
        MobActionRunners::swallowAll,
        nullptr
    );
    
    //Teleport to absolute.
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_TELEPORT_TO_ABSOLUTE,
        "teleport_to_absolute",
        MobActionRunners::teleportToAbsolute,
        nullptr
    );
    
    //Teleport to relative.
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_TELEPORT_TO_RELATIVE,
        "teleport_to_relative",
        MobActionRunners::teleportToRelative,
        nullptr
    );
    
    //Throw focused mob.
    regParam("x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("max height", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_THROW_FOCUS,
        "throw_focused_mob",
        MobActionRunners::throwFocus,
        nullptr
    );
    
    //Turn to absolute.
    regParam("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_TURN_TO_ABSOLUTE,
        "turn_to_absolute",
        MobActionRunners::turnToAbsolute,
        nullptr
    );
    
    //Turn to relative.
    regParam("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_TURN_TO_RELATIVE,
        "turn_to_relative",
        MobActionRunners::turnToRelative,
        nullptr
    );
    
    //Turn to target.
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_TURN_TO_TARGET,
        "turn_to_target",
        MobActionRunners::turnToTarget,
        MobActionLoaders::turnToTarget
    );
    
    
#undef regParam
#undef regAction
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
