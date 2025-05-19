/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program initializer and deinitializer functions.
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
    ALLEGRO_TIMER* &mainTimer, ALLEGRO_EVENT_QUEUE* &eventQueue
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
    
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_NONE,
        PLAYER_ACTION_CAT_NONE,
        "---", "", "", ""
    );
    
    //Main.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Move right",
        "Move the leader right.",
        "move_right", "k_4"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Move up",
        "Move the leader up.",
        "move_up", "k_23"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Move left",
        "Move the leader left.",
        "move_left", "k_1"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Move down",
        "Move the leader down.",
        "move_down", "k_19"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_THROW,
        PLAYER_ACTION_CAT_MAIN,
        "Throw",
        "Throw a Pikmin.",
        "throw", "mb_1"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_WHISTLE,
        PLAYER_ACTION_CAT_MAIN,
        "Whistle",
        "Whistle around the cursor.",
        "whistle", "mb_2"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_NEXT_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Next Pikmin",
        "Change to the next Pikmin type in the group.",
        "next_type", "mwd"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_PREV_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. Pikmin",
        "Change to the previous Pikmin type in the group.",
        "prev_type", "mwu"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_NEXT_LEADER,
        PLAYER_ACTION_CAT_MAIN,
        "Next leader",
        "Change to the next leader.",
        "next_leader", "k_215"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_GROUP_CURSOR,
        PLAYER_ACTION_CAT_MAIN,
        "Swarm to cursor",
        "Swarm all Pikmin towards the cursor.",
        "swarm_cursor", "k_75"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_DISMISS,
        PLAYER_ACTION_CAT_MAIN,
        "Dismiss",
        "Dismiss all Pikmin.",
        "dismiss", "k_217"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_USE_SPRAY_1,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 1",
        "Use the spray in slot 1.",
        "use_spray_1", "k_18"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_USE_SPRAY_2,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 2",
        "Use the spray in slot 2.",
        "use_spray_2", "k_6"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_USE_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray",
        "Use the currently selected spray.",
        "use_spray", "k_18"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_NEXT_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Next spray",
        "Change to the next spray.",
        "next_spray", "k_5"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_PREV_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. spray",
        "Change to the previous spray.",
        "prev_spray", "k_17"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_PAUSE,
        PLAYER_ACTION_CAT_MAIN,
        "Pause",
        "Pause the game.",
        "pause", "k_59"
    );
    
    //Menus.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu right",
        "Navigate right in a menu.",
        "menu_right", "k_83", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Menu up",
        "Navigate up in a menu.",
        "menu_up", "k_84", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu left",
        "Navigate left in a menu.",
        "menu_left", "k_82", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Menu down",
        "Navigate down in a menu.",
        "menu_down", "k_85", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_OK,
        PLAYER_ACTION_CAT_MENUS,
        "Menu OK",
        "Confirm the selected item in a menu.",
        "menu_ok", "k_67", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan right",
        "Pan the radar to the right.",
        "menu_radar_right", "k_4"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan up",
        "Pan the radar upward.",
        "menu_radar_up", "k_23"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan left",
        "Pan the radar to the left.",
        "menu_radar_left", "k_1"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan down",
        "Pan the radar downward.",
        "menu_radar_down", "k_19"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom in",
        "Zoom the radar in.",
        "menu_radar_zoom_in", "k_18"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom out",
        "Zoom the radar out.",
        "menu_radar_zoom_out", "k_6"
    );
    
    //Advanced.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CURSOR_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor right",
        "Move the cursor right. Useful if it's not mouse-controlled.",
        "cursor_right", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CURSOR_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor up",
        "Move the cursor up. Useful if it's not mouse-controlled.",
        "cursor_up", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CURSOR_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor left",
        "Move the cursor left. Useful if it's not mouse-controlled.",
        "cursor_left", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CURSOR_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor down",
        "Move the cursor down. Useful if it's not mouse-controlled.",
        "cursor_down", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_GROUP_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm right",
        "Swarm all Pikmin right.",
        "swarm_right", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_GROUP_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm up",
        "Swarm all Pikmin up.",
        "swarm_up", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_GROUP_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm left",
        "Swarm all Pikmin left.",
        "swarm_left", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_GROUP_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm down",
        "Swarm all Pikmin down.",
        "swarm_down", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_PREV_LEADER,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. leader",
        "Change to the previous leader.",
        "prev_leader", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_CAT_ADVANCED,
        "Change zoom",
        "Change the current zoom level.",
        "change_zoom", "k_3"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom in",
        "Change to a closer zoom level.",
        "zoom_in", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_ZOOM_OUT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom out",
        "Change to a farther zoom level.",
        "zoom_out", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_NEXT_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Next maturity",
        "Change to a Pikmin of the next maturity.",
        "next_maturity", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_PREV_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. maturity",
        "Change to a Pikmin of the previous maturity.",
        "prev_maturity", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_LIE_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Lie down",
        "Lie down so Pikmin can carry you.",
        "lie_down", "k_26"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CUSTOM_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom A",
        "Custom action A, if the current leader supports it.",
        "custom_a", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CUSTOM_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom B",
        "Custom action B, if the current leader supports it.",
        "custom_b", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_CUSTOM_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom C",
        "Custom action C, if the current leader supports it.",
        "custom_c", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_RADAR,
        PLAYER_ACTION_CAT_ADVANCED,
        "Radar",
        "Open or close the radar.",
        "radar", "k_64"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_BACK,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - back",
        "Go back or cancel in a menu.",
        "menu_back", "k_59", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - left page",
        "Go to the page to the left in a menu.",
        "menu_page_left", "k_17", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - right page",
        "Go to the page to the right in a menu.",
        "menu_page_right", "k_5", 0.5f
    );
    
    //General maker tool things.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_AUTO_START,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Auto-start",
        "Make the game auto-start on the current state (and content).",
        "mt_auto_start", "k_56"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Set song pos near loop",
        "Set the current song's position to be near the loop point.",
        "mt_set_song_pos_near_loop", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_MOD_1,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 1",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_1", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_MOD_2,
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
        "Modifier 2",
        "Holding this input modifies the behavior of some tools.",
        "mt_mod_2", ""
    );
    
    //Gameplay maker tools.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Area image",
        "Save an image of the current area.",
        "mt_area_image", "k_36"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Change speed",
        "Change the gameplay speed.",
        "mt_change_speed", "k_28"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Geometry info",
        "Toggle info about the geometry under the cursor.",
        "mt_geometry_info", "k_33"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_HUD,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "HUD",
        "Toggle the HUD.",
        "mt_hud", "k_35"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_HURT_MOB,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Hurt mob",
        "Hurt the mob under the cursor.",
        "mt_hurt_mob", "k_30", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_MOB_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Mob info",
        "Toggle info about the mob under the cursor.",
        "mt_mob_info", "k_32", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_NEW_PIKMIN,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "New Pikmin",
        "Create a new Pikmin under the cursor.",
        "mt_new_pikmin", "k_31", 0.5f
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_PATH_INFO,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Path info",
        "Toggle info about paths the info'd mob is taking.",
        "mt_path_info", "k_34"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_COLLISION,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show collision",
        "Toggle drawing each mob's collision.",
        "mt_show_collision", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Show hitboxes",
        "Toggle drawing each mob's hitboxes.",
        "mt_show_hitboxes", ""
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_MT_TELEPORT,
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
        "Teleport",
        "Teleport the leader to the cursor.",
        "mt_teleport", "k_29", 0.5f
    );
    
    //System.
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_SYSTEM_INFO,
        PLAYER_ACTION_CAT_SYSTEM,
        "System info",
        "Toggle showing system and performance information.",
        "system_info", "k_47"
    );
    game.controls.addPlayerActionType(
        PLAYER_ACTION_TYPE_SCREENSHOT,
        PLAYER_ACTION_CAT_SYSTEM,
        "Take a screenshot",
        "Take a screenshot and save it in the user data folder.",
        "screenshot", "k_58"
    );
    
    
    //Populate the control binds with some default control binds for player 1.
    //If the options are loaded successfully, these binds are overwritten.
    const vector<PfePlayerActionType> &actionTypes =
        game.controls.getAllPlayerActionTypes();
    for(size_t a = 0; a < actionTypes.size(); a++) {
        const string &def = actionTypes[a].defaultBindStr;
        if(def.empty()) continue;
        
        ControlBind bind;
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
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig editorFontCfg;
    editorFontCfg.OversampleH = editorFontCfg.OversampleV = 1;
    editorFontCfg.PixelSnapH = true;
    
    const auto addFont =
    [ = ] (ImFont** targetVar, const string &assetInternalName, int height) {
        const string &path =
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
    ImGuiStyle &style = ImGui::GetStyle();
    
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
        ImGui::AdjustColorHSV(style.Colors[colorsToChange[c]], -0.25f, 0.0f, 0.0f);
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
    ALLEGRO_TIMER* &mainTimer, ALLEGRO_EVENT_QUEUE* &eventQueue
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
    
    game.rng.init();
    
    game.states.gameplay->players.push_back(Player());
    
    game.states.gameplay->particles =
        ParticleManager(game.options.advanced.maxParticles);
        
    game.options.advanced.zoomMediumReach =
        std::clamp(
            game.options.advanced.zoomMediumReach,
            game.config.rules.zoomClosestReach,
            game.config.rules.zoomFarthestReach
        );
        
    game.liquidLimitEffectBuffer = al_create_bitmap(game.winW, game.winH);
    game.wallOffsetEffectBuffer = al_create_bitmap(game.winW, game.winH);
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
    
    //Mission goals.
    //Order matters, and should match MISSION_GOAL.
    game.missionGoals.push_back(
        new MissionGoalEndManually()
    );
    game.missionGoals.push_back(
        new MissionGoalCollectTreasures()
    );
    game.missionGoals.push_back(
        new MissionGoalBattleEnemies()
    );
    game.missionGoals.push_back(
        new MissionGoalTimedSurvival()
    );
    game.missionGoals.push_back(
        new MissionGoalGetToExit()
    );
    game.missionGoals.push_back(
        new MissionGoalGrowPikmin()
    );
    
    //Mission fail conditions.
    //Order matters, and should match MISSION_FAIL_COND.
    game.missionFailConds.push_back(
        new MissionFailTimeLimit()
    );
    game.missionFailConds.push_back(
        new MissionFailTooFewPikmin()
    );
    game.missionFailConds.push_back(
        new MissionFailTooManyPikmin()
    );
    game.missionFailConds.push_back(
        new MissionFailLosePikmin()
    );
    game.missionFailConds.push_back(
        new MissionFailTakeDamage()
    );
    game.missionFailConds.push_back(
        new MissionFailLoseLeaders()
    );
    game.missionFailConds.push_back(
        new MissionFailDefeatEnemies()
    );
    game.missionFailConds.push_back(
        new MissionFailPauseMenu()
    );
    
    //Mission score criteria.
    //Order matters, and should match MISSION_SCORE_CRITERIA.
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionPikminBorn()
    );
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionPikminDeath()
    );
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionSecLeft()
    );
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionSecPassed()
    );
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionTreasurePoints()
    );
    game.missionScoreCriteria.push_back(
        new MissionScoreCriterionEnemyPoints()
    );
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
    
    regAction(
        MOB_ACTION_UNKNOWN,
        "unknown",
        nullptr,
        nullptr
    );
    
    regParam("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_ADD_HEALTH,
        "add_health",
        MobActionRunners::addHealth,
        nullptr
    );
    
    regParam("goal", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_ARACHNORB_PLAN_LOGIC,
        "arachnorb_plan_logic",
        MobActionRunners::arachnorbPlanLogic,
        MobActionLoaders::arachnorbPlanLogic
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("operand", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("operation", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("operand", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_CALCULATE,
        "calculate",
        MobActionRunners::calculate,
        MobActionLoaders::calculate
    );
    
    regAction(
        MOB_ACTION_DELETE,
        "delete",
        MobActionRunners::deleteFunction,
        nullptr
    );
    
    regAction(
        MOB_ACTION_DRAIN_LIQUID,
        "drain_liquid",
        MobActionRunners::drainLiquid,
        nullptr
    );
    
    regAction(
        MOB_ACTION_ELSE,
        "else",
        nullptr,
        nullptr
    );
    
    regAction(
        MOB_ACTION_END_IF,
        "end_if",
        nullptr,
        nullptr
    );
    
    regAction(
        MOB_ACTION_FINISH_DYING,
        "finish_dying",
        MobActionRunners::finishDying,
        nullptr
    );
    
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_FOCUS,
        "focus",
        MobActionRunners::focus,
        MobActionLoaders::focus
    );
    
    regParam("label", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_FOLLOW_PATH_RANDOMLY,
        "follow_path_randomly",
        MobActionRunners::followPathRandomly,
        nullptr
    );
    
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("label", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
        "follow_path_to_absolute",
        MobActionRunners::followPathToAbsolute,
        nullptr
    );
    
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
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_AREA_INFO,
        "get_area_info",
        MobActionRunners::getAreaInfo,
        MobActionLoaders::getAreaInfo
    );
    
    regAction(
        MOB_ACTION_GET_CHOMPED,
        "get_chomped",
        MobActionRunners::getChomped,
        nullptr
    );
    
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
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_EVENT_INFO,
        "get_event_info",
        MobActionRunners::getEventInfo,
        MobActionLoaders::getEventInfo
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        MobActionRunners::getFloorZ,
        nullptr
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("focused mob's var name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_FOCUS_VAR,
        "get_focus_var",
        MobActionRunners::getFocusVar,
        nullptr
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("target", MOB_ACTION_PARAM_STRING, true, false);
    regParam("info", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GET_MOB_INFO,
        "get_mob_info",
        MobActionRunners::getMobInfo,
        MobActionLoaders::getMobInfo
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("minimum value", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("maximum value", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_GET_RANDOM_FLOAT,
        "get_random_float",
        MobActionRunners::getRandomFloat,
        nullptr
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("minimum value", MOB_ACTION_PARAM_INT, false, false);
    regParam("maximum value", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_GET_RANDOM_INT,
        "get_random_int",
        MobActionRunners::getRandomInt,
        nullptr
    );
    
    regParam("label name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_GOTO,
        "goto",
        nullptr,
        nullptr
    );
    
    regParam("body part name", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("hold above", MOB_ACTION_PARAM_BOOL, false, true);
    regAction(
        MOB_ACTION_HOLD_FOCUS,
        "hold_focused_mob",
        MobActionRunners::holdFocus,
        MobActionLoaders::holdFocus
    );
    
    regParam("comparand", MOB_ACTION_PARAM_STRING, false, false);
    regParam("operation", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("value", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_IF,
        "if",
        MobActionRunners::ifFunction,
        MobActionLoaders::ifFunction
    );
    
    regParam("label name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_LABEL,
        "label",
        nullptr,
        nullptr
    );
    
    regParam("identifier", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_LINK_WITH_FOCUS,
        "link_with_focused_mob",
        MobActionRunners::linkWithFocus,
        nullptr
    );
    
    regParam("slot", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_LOAD_FOCUS_MEMORY,
        "load_focused_mob_memory",
        MobActionRunners::loadFocusMemory,
        nullptr
    );
    
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_MOVE_TO_ABSOLUTE,
        "move_to_absolute",
        MobActionRunners::moveToAbsolute,
        nullptr
    );
    
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_MOVE_TO_RELATIVE,
        "move_to_relative",
        MobActionRunners::moveToRelative,
        nullptr
    );
    
    regParam("target", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_MOVE_TO_TARGET,
        "move_to_target",
        MobActionRunners::moveToTarget,
        MobActionLoaders::moveToTarget
    );
    
    regAction(
        MOB_ACTION_ORDER_RELEASE,
        "order_release",
        MobActionRunners::orderRelease,
        nullptr
    );
    
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
    
    regParam("text", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_PRINT,
        "print",
        MobActionRunners::print,
        nullptr
    );
    
    regParam("status name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_RECEIVE_STATUS,
        "receive_status",
        MobActionRunners::receiveStatus,
        MobActionLoaders::receiveStatus
    );
    
    regAction(
        MOB_ACTION_RELEASE,
        "release",
        MobActionRunners::release,
        nullptr
    );
    
    regAction(
        MOB_ACTION_RELEASE_STORED_MOBS,
        "release_stored_mobs",
        MobActionRunners::releaseStoredMobs,
        nullptr
    );
    
    regParam("status name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_REMOVE_STATUS,
        "remove_status",
        MobActionRunners::removeStatus,
        MobActionLoaders::removeStatus
    );
    
    regParam("slot", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_SAVE_FOCUS_MEMORY,
        "save_focused_mob_memory",
        MobActionRunners::saveFocusMemory,
        nullptr
    );
    
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
        "send_message_to_focus",
        MobActionRunners::sendMessageToFocus,
        nullptr
    );
    
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regParam("identifier", MOB_ACTION_PARAM_STRING, false, true);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_LINKS,
        "send_message_to_links",
        MobActionRunners::sendMessageToLinks,
        nullptr
    );
    
    regParam("distance", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("message", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
        "send_message_to_nearby",
        MobActionRunners::sendMessageToNearby,
        nullptr
    );
    
    regParam("animation name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("options", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_SET_ANIMATION,
        "set_animation",
        MobActionRunners::setAnimation,
        MobActionLoaders::setAnimation
    );
    
    regParam("blocks", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_CAN_BLOCK_PATHS,
        "set_can_block_paths",
        MobActionRunners::setCanBlockPaths,
        nullptr
    );
    
    regParam("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_FAR_REACH,
        "set_far_reach",
        MobActionRunners::setFarReach,
        MobActionLoaders::setFarReach
    );
    
    regParam("flying", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_FLYING,
        "set_flying",
        MobActionRunners::setFlying,
        nullptr
    );
    
    regParam("multiplier", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_GRAVITY,
        "set_gravity",
        MobActionRunners::setGravity,
        nullptr
    );
    
    regParam("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_HEALTH,
        "set_health",
        MobActionRunners::setHealth,
        nullptr
    );
    
    regParam("height", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_HEIGHT,
        "set_height",
        MobActionRunners::setHeight,
        nullptr
    );
    
    regParam("hiding", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_HIDING,
        "set_hiding",
        MobActionRunners::setHiding,
        nullptr
    );
    
    regParam("huntable", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_HUNTABLE,
        "set_huntable",
        MobActionRunners::setHuntable,
        nullptr
    );
    
    regParam("options", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_SET_HOLDABLE,
        "set_holdable",
        MobActionRunners::setHoldable,
        MobActionLoaders::setHoldable
    );
    
    regParam("animation name", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SET_LIMB_ANIMATION,
        "set_limb_animation",
        MobActionRunners::setLimbAnimation,
        nullptr
    );
    
    regParam("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_NEAR_REACH,
        "set_near_reach",
        MobActionRunners::setNearReach,
        MobActionLoaders::setNearReach
    );
    
    regParam("radius", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_RADIUS,
        "set_radius",
        MobActionRunners::setRadius,
        nullptr
    );
    
    regParam("x speed", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y speed", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_SECTOR_SCROLL,
        "set_sector_scroll",
        MobActionRunners::setSectorScroll,
        nullptr
    );
    
    regParam("visible", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_SHADOW_VISIBILITY,
        "set_shadow_visibility",
        MobActionRunners::setShadowVisibility,
        nullptr
    );
    
    regParam("state name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_SET_STATE,
        "set_state",
        MobActionRunners::setState,
        nullptr
    );
    
    regParam("tangible", MOB_ACTION_PARAM_BOOL, false, false);
    regAction(
        MOB_ACTION_SET_TANGIBLE,
        "set_tangible",
        MobActionRunners::setTangible,
        nullptr
    );
    
    regParam("team name", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SET_TEAM,
        "set_team",
        MobActionRunners::setTeam,
        MobActionLoaders::setTeam
    );
    
    regParam("time", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_SET_TIMER,
        "set_timer",
        MobActionRunners::setTimer,
        nullptr
    );
    
    regParam("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    regParam("value", MOB_ACTION_PARAM_STRING, false, false);
    regAction(
        MOB_ACTION_SET_VAR,
        "set_var",
        MobActionRunners::setVar,
        nullptr
    );
    
    regParam("var name", MOB_ACTION_PARAM_STRING, true, false);
    regAction(
        MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
        "show_message_from_var",
        MobActionRunners::showMessageFromVar,
        nullptr
    );
    
    regParam("spawn data", MOB_ACTION_PARAM_ENUM, true, false);
    regAction(
        MOB_ACTION_SPAWN,
        "spawn",
        MobActionRunners::spawn,
        MobActionLoaders::spawn
    );
    
    regParam("reference", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("offset", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_STABILIZE_Z,
        "stabilize_z",
        MobActionRunners::stabilizeZ,
        MobActionLoaders::stabilizeZ
    );
    
    regParam("victim max", MOB_ACTION_PARAM_INT, false, false);
    regParam("body part", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("more body parts", MOB_ACTION_PARAM_ENUM, true, true);
    regAction(
        MOB_ACTION_START_CHOMPING,
        "start_chomping",
        MobActionRunners::startChomping,
        MobActionLoaders::startChomping
    );
    
    regAction(
        MOB_ACTION_START_DYING,
        "start_dying",
        MobActionRunners::startDying,
        nullptr
    );
    
    regAction(
        MOB_ACTION_START_HEIGHT_EFFECT,
        "start_height_effect",
        MobActionRunners::startHeightEffect,
        nullptr
    );
    
    regParam("generator name", MOB_ACTION_PARAM_ENUM, true, false);
    regParam("offset coordinates", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_START_PARTICLES,
        "start_particles",
        MobActionRunners::startParticles,
        MobActionLoaders::startParticles
    );
    
    regAction(
        MOB_ACTION_STOP,
        "stop",
        MobActionRunners::stop,
        nullptr
    );
    
    regAction(
        MOB_ACTION_STOP_CHOMPING,
        "stop_chomping",
        MobActionRunners::stopChomping,
        nullptr
    );
    
    regAction(
        MOB_ACTION_STOP_HEIGHT_EFFECT,
        "stop_height_effect",
        MobActionRunners::stopHeightEffect,
        nullptr
    );
    
    regAction(
        MOB_ACTION_STOP_PARTICLES,
        "stop_particles",
        MobActionRunners::stopParticles,
        nullptr
    );
    
    regParam("sound ID", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_STOP_SOUND,
        "stop_sound",
        MobActionRunners::stopSound,
        nullptr
    );
    
    regAction(
        MOB_ACTION_STOP_VERTICALLY,
        "stop_vertically",
        MobActionRunners::stopVertically,
        nullptr
    );
    
    regAction(
        MOB_ACTION_STORE_FOCUS_INSIDE,
        "store_focus_inside",
        MobActionRunners::storeFocusInside,
        nullptr
    );
    
    regParam("amount", MOB_ACTION_PARAM_INT, false, false);
    regAction(
        MOB_ACTION_SWALLOW,
        "swallow",
        MobActionRunners::swallow,
        nullptr
    );
    
    regAction(
        MOB_ACTION_SWALLOW_ALL,
        "swallow_all",
        MobActionRunners::swallowAll,
        nullptr
    );
    
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_TELEPORT_TO_ABSOLUTE,
        "teleport_to_absolute",
        MobActionRunners::teleportToAbsolute,
        nullptr
    );
    
    regParam("x", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("z", MOB_ACTION_PARAM_FLOAT, false, false);
    regAction(
        MOB_ACTION_TELEPORT_TO_RELATIVE,
        "teleport_to_relative",
        MobActionRunners::teleportToRelative,
        nullptr
    );
    
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
    
    regParam("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_TURN_TO_ABSOLUTE,
        "turn_to_absolute",
        MobActionRunners::turnToAbsolute,
        nullptr
    );
    
    regParam("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    regParam("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    regAction(
        MOB_ACTION_TURN_TO_RELATIVE,
        "turn_to_relative",
        MobActionRunners::turnToRelative,
        nullptr
    );
    
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
