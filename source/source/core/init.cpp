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
void destroy_allegro() {
    al_uninstall_joystick();
    al_uninstall_audio();
    al_uninstall_keyboard();
    al_uninstall_mouse();
    al_uninstall_system();
}


/**
 * @brief Destroys Allegro's event-related things.
 *
 * @param main_timer The main game timer.
 * @param event_queue Queue of Allegro events.
 */
void destroy_event_things(
    ALLEGRO_TIMER* &main_timer, ALLEGRO_EVENT_QUEUE* &event_queue
) {
    al_destroy_event_queue(event_queue);
    al_destroy_timer(main_timer);
    al_destroy_display(game.display);
}


/**
 * @brief Destroys miscellaneous things.
 */
void destroy_misc() {
    al_destroy_bitmap(game.bmp_error);
    game.audio.destroy();
    
    game.sector_types.clear();
    for(size_t g = 0; g < game.mission_goals.size(); g++) {
        delete game.mission_goals[g];
    }
    game.mission_goals.clear();
    for(size_t c = 0; c < game.mission_fail_conds.size(); c++) {
        delete game.mission_fail_conds[c];
    }
    game.mission_fail_conds.clear();
    for(size_t c = 0; c < game.mission_score_criteria.size(); c++) {
        delete game.mission_score_criteria[c];
    }
    game.mission_score_criteria.clear();
}


/**
 * @brief Destroys registered mob categories.
 */
void destroy_mob_categories() {
    game.mob_categories.clear();
}


/**
 * @brief Initializes Allegro and its modules.
 */
void init_allegro() {
    if(!al_init()) {
        report_fatal_error("Could not initialize Allegro!");
    }
    if(!al_install_mouse()) {
        report_fatal_error("Could not install the Allegro mouse module!");
    }
    if(!al_install_keyboard()) {
        report_fatal_error("Could not install the Allegro keyboard module!");
    }
    if(!al_install_audio()) {
        report_fatal_error("Could not install the Allegro audio module!");
    }
    if(!al_init_image_addon()) {
        report_fatal_error("Could not initialize the Allegro image addon!");
    }
    if(!al_init_primitives_addon()) {
        report_fatal_error(
            "Could not initialize the Allegro primitives addon!"
        );
    }
    if(!al_init_acodec_addon()) {
        report_fatal_error(
            "Could not initialize the Allegro audio codec addon!"
        );
    }
    if(!al_init_font_addon()) {
        report_fatal_error(
            "Could not initialize the Allegro font addon!"
        );
    }
    if(!al_init_ttf_addon()) {
        report_fatal_error(
            "Could not initialize the Allegro TTF font addon!"
        );
    }
    if(!al_install_joystick()) {
        report_fatal_error(
            "Could not initialize Allegro joystick support!"
        );
    }
}


/**
 * @brief Initializes things related to the controls.
 */
void init_controls() {
    //Register the existing actions.
    //They must be registered in the same order as the action types enum.
    
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_NONE,
        PLAYER_ACTION_CAT_NONE,
        "---", "", "", ""
    );
    
    //Main.
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Move right",
        "Move the leader right.",
        "move_right", "k_4"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Move up",
        "Move the leader up.",
        "move_up", "k_23"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Move left",
        "Move the leader left.",
        "move_left", "k_1"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Move down",
        "Move the leader down.",
        "move_down", "k_19"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_THROW,
        PLAYER_ACTION_CAT_MAIN,
        "Throw",
        "Throw a Pikmin.",
        "throw", "mb_1"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_WHISTLE,
        PLAYER_ACTION_CAT_MAIN,
        "Whistle",
        "Whistle around the cursor.",
        "whistle", "mb_2"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_NEXT_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Next Pikmin",
        "Change to the next Pikmin type in the group.",
        "next_type", "mwd"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_PREV_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. Pikmin",
        "Change to the previous Pikmin type in the group.",
        "prev_type", "mwu"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_NEXT_LEADER,
        PLAYER_ACTION_CAT_MAIN,
        "Next leader",
        "Change to the next leader.",
        "next_leader", "k_215"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_GROUP_CURSOR,
        PLAYER_ACTION_CAT_MAIN,
        "Swarm to cursor",
        "Swarm all Pikmin towards the cursor.",
        "swarm_cursor", "k_75"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_DISMISS,
        PLAYER_ACTION_CAT_MAIN,
        "Dismiss",
        "Dismiss all Pikmin.",
        "dismiss", "k_217"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_USE_SPRAY_1,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 1",
        "Use the spray in slot 1.",
        "use_spray_1", "k_18"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_USE_SPRAY_2,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 2",
        "Use the spray in slot 2.",
        "use_spray_2", "k_6"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_USE_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray",
        "Use the currently selected spray.",
        "use_spray", "k_18"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_NEXT_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Next spray",
        "Change to the next spray.",
        "next_spray", "k_5"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_PREV_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. spray",
        "Change to the previous spray.",
        "prev_spray", "k_17"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_PAUSE,
        PLAYER_ACTION_CAT_MAIN,
        "Pause",
        "Pause the game.",
        "pause", "k_59"
    );
    
    //Menus.
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu right",
        "Navigate right in a menu.",
        "menu_right", "k_83"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Menu up",
        "Navigate up in a menu.",
        "menu_up", "k_84"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu left",
        "Navigate left in a menu.",
        "menu_left", "k_82"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Menu down",
        "Navigate down in a menu.",
        "menu_down", "k_85"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_OK,
        PLAYER_ACTION_CAT_MENUS,
        "Menu OK",
        "Confirm the selected item in a menu.",
        "menu_ok", "k_67"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan right",
        "Pans the radar to the right.",
        "menu_radar_right", "k_4"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan up",
        "Pans the radar upward.",
        "menu_radar_up", "k_23"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan left",
        "Pans the radar to the left.",
        "menu_radar_left", "k_1"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar pan down",
        "Pans the radar downward.",
        "menu_radar_down", "k_19"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_IN,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom in",
        "Zooms the radar in.",
        "menu_radar_zoom_in", "k_18"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT,
        PLAYER_ACTION_CAT_MENUS,
        "Radar zoom out",
        "Zooms the radar out.",
        "menu_radar_zoom_out", "k_6"
    );
    
    //Advanced.
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CURSOR_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor right",
        "Move the cursor right. Useful if it's not mouse-controlled.",
        "cursor_right", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CURSOR_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor up",
        "Move the cursor up. Useful if it's not mouse-controlled.",
        "cursor_up", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CURSOR_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor left",
        "Move the cursor left. Useful if it's not mouse-controlled.",
        "cursor_left", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CURSOR_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor down",
        "Move the cursor down. Useful if it's not mouse-controlled.",
        "cursor_down", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_GROUP_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm right",
        "Swarm all Pikmin right.",
        "swarm_right", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_GROUP_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm up",
        "Swarm all Pikmin up.",
        "swarm_up", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_GROUP_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm left",
        "Swarm all Pikmin left.",
        "swarm_left", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_GROUP_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm down",
        "Swarm all Pikmin down.",
        "swarm_down", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_PREV_LEADER,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. leader",
        "Change to the previous leader.",
        "prev_leader", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_CAT_ADVANCED,
        "Change zoom",
        "Change the current zoom level.",
        "change_zoom", "k_3"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom in",
        "Change to a closer zoom level.",
        "zoom_in", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_ZOOM_OUT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom out",
        "Change to a farther zoom level.",
        "zoom_out", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_NEXT_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Next maturity",
        "Change to a Pikmin of the next maturity.",
        "next_maturity", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_PREV_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. maturity",
        "Change to a Pikmin of the previous maturity.",
        "prev_maturity", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_LIE_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Lie down",
        "Lie down so Pikmin can carry you.",
        "lie_down", "k_26"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CUSTOM_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom A",
        "Custom action A, if the current leader supports it.",
        "custom_a", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CUSTOM_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom B",
        "Custom action B, if the current leader supports it.",
        "custom_b", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_CUSTOM_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom C",
        "Custom action C, if the current leader supports it.",
        "custom_c", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_RADAR,
        PLAYER_ACTION_CAT_ADVANCED,
        "Radar",
        "Open or close the radar.",
        "radar", "k_64"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_BACK,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - back",
        "Go back or cancel in a menu.",
        "menu_back", "k_59"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_PAGE_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - left page",
        "Go to the page to the left in a menu.",
        "menu_page_left", "k_17"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - right page",
        "Go to the page to the right in a menu.",
        "menu_page_right", "k_5"
    );
    
    //Maker tools.
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_AREA_IMAGE,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Area image",
        "Save an image of the current area.",
        "mt_area_image", "k_27"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_CHANGE_SPEED,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Change speed",
        "Changes the gameplay speed.",
        "mt_change_speed", "k_28"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Geometry info",
        "Toggles info about the geometry under the cursor.",
        "mt_geometry_info", "k_33"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_HUD,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "HUD",
        "Toggles the HUD.",
        "mt_hud", "k_36"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_HURT_MOB,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Hurt mob",
        "Hurts the mob under the cursor.",
        "mt_hurt_mob", "k_30"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_MOB_INFO,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Mob info",
        "Toggles info about the mob under the cursor.",
        "mt_mob_info", "k_32"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_NEW_PIKMIN,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "New Pikmin",
        "Creates a new Pikmin under the cursor.",
        "mt_new_pikmin", "k_31"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_PATH_INFO,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Path info",
        "Toggles info about paths the info'd mob is taking.",
        "mt_path_info", "k_34"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Set song pos near loop",
        "Sets the current song's position to be near the loop point.",
        "mt_set_song_pos_near_loop", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_SHOW_COLLISION,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Show collision",
        "Toggles drawing each mob's collision.",
        "mt_show_collision", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Show hitboxes",
        "Toggles drawing each mob's hitboxes.",
        "mt_show_hitboxes", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_TYPE_MT_TELEPORT,
        PLAYER_ACTION_CAT_MAKER_TOOLS,
        "Teleport",
        "Teleports the leader to the cursor.",
        "mt_teleport", "k_29"
    );
    
    
    //Populate the control binds with some default control binds for player 1.
    //If the options are loaded successfully, these binds are overwritten.
    const vector<PlayerActionType> &action_types =
        game.controls.get_all_player_action_types();
    for(size_t a = 0; a < action_types.size(); a++) {
        const string &def = action_types[a].default_bind_str;
        if(def.empty()) continue;
        
        ControlBind bind;
        bind.actionTypeId = action_types[a].id;
        bind.playerNr = 0;
        bind.inputSource = game.controls.str_to_input_source(def);
        game.controls.binds().push_back(bind);
    }
}


/**
 * @brief Initializes Dear ImGui.
 */
void init_dear_imgui() {
    //Misc. setup.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplAllegro5_Init(game.display);
    ImGui::GetIO().IniFilename = "";
    ImGui::GetIO().ConfigDragClickToInputText = true;
    
    //Fonts.
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig editor_font_cfg;
    editor_font_cfg.OversampleH = editor_font_cfg.OversampleV = 1;
    editor_font_cfg.PixelSnapH = true;
    
    const auto add_font =
    [ = ] (ImFont** target_var, const string &asset_internal_name, int height) {
        const string &path =
            game.content.bitmaps.manifests
            [asset_internal_name].path;
            
        if(!str_ends_with(str_to_lower(path), ".ttf")) {
            game.errors.report(
                "Could not load the editor font \"" + path + "\"! Only "
                "TTF font files are allowed."
            );
            return;
        }
        
        *target_var =
            io.Fonts->AddFontFromFileTTF(
                path.c_str(), height, &editor_font_cfg
            );
    };
    
    add_font(
        &game.sys_content.fnt_imgui_header,
        game.sys_content_names.fnt_editor_header, 22
    );
    add_font(
        &game.sys_content.fnt_imgui_monospace,
        game.sys_content_names.fnt_editor_monospace, 18
    );
    add_font(
        &game.sys_content.fnt_imgui_standard,
        game.sys_content_names.fnt_editor_standard, 18
    );
    
    io.FontDefault = game.sys_content.fnt_imgui_standard;
    
    //Other stuff.
    init_dear_imgui_colors();
}


/**
 * @brief Initializes the Dear ImGui color style.
 */
void init_dear_imgui_colors() {
    ImGuiStyle &style = ImGui::GetStyle();
    
    //Since the default Dear ImGui style is based around blue,
    //we can shift the hue left to get an equivalent green.
    //These color indexes are the ones that really have any blue hue,
    //so only mess with these.
    vector<ImGuiCol_> colors_to_change {
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
    
    for(size_t c = 0; c < colors_to_change.size(); c++) {
        ImGui::AdjustColorHSV(style.Colors[colors_to_change[c]], -0.25f, 0.0f, 0.0f);
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
        game.imgui_default_style[c] = style.Colors[c];
    }
}


/**
 * @brief Initializes the error bitmap.
 */
void init_error_bitmap() {
    //Error bitmap.
    game.bmp_error = al_create_bitmap(32, 32);
    al_set_target_bitmap(game.bmp_error); {
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
    game.bmp_error = recreate_bitmap(game.bmp_error);
}


/**
 * @brief Initializes some essential things.
 */
void init_essentials() {
    //Signal handlers.
    signal(SIGFPE,  signal_handler);
    signal(SIGILL,  signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
}


/**
 * @brief Initializes things regarding Allegro events, like the queue,
 * timer, etc.
 *
 * @param main_timer The main game timer.
 * @param event_queue Queue of Allegro events.
 */
void init_event_things(
    ALLEGRO_TIMER* &main_timer, ALLEGRO_EVENT_QUEUE* &event_queue
) {
    al_set_new_display_flags(
        al_get_new_display_flags() |
        ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE
    );
    if(game.options.advanced.window_pos_hack) al_set_new_window_position(64, 64);
    if(game.win_fullscreen) {
        al_set_new_display_flags(
            al_get_new_display_flags() |
            (
                game.options.graphics.true_fullscreen ?
                ALLEGRO_FULLSCREEN :
                ALLEGRO_FULLSCREEN_WINDOW
            )
        );
    }
    game.display = al_create_display(game.win_w, game.win_h);
    
    //It's possible that this resolution is not valid for fullscreen.
    //Detect this and try again in windowed.
    if(!game.display && game.win_fullscreen) {
        game.errors.report(
            "Could not create a fullscreen window with the resolution " +
            i2s(game.win_w) + "x" + i2s(game.win_h) + ". "
            "Setting the fullscreen option back to false. "
            "You can try a different resolution, "
            "preferably one from the options menu."
        );
        game.win_fullscreen = false;
        game.options.graphics.intended_win_fullscreen = false;
        save_options();
        al_set_new_display_flags(
            al_get_new_display_flags() & ~ALLEGRO_FULLSCREEN
        );
        game.display = al_create_display(game.win_w, game.win_h);
    }
    
    if(!game.display) {
        report_fatal_error("Could not create a display!");
    }
    
    //For some reason some resolutions aren't properly created under Windows.
    //This hack fixes it.
    al_resize_display(game.display, game.win_w, game.win_h);
    
    main_timer = al_create_timer(1.0f / game.options.advanced.target_fps);
    if(!main_timer) {
        report_fatal_error("Could not create the main game timer!");
    }
    
    event_queue = al_create_event_queue();
    if(!event_queue) {
        report_fatal_error("Could not create the main event queue!");
    }
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_joystick_event_source());
    al_register_event_source(
        event_queue, al_get_display_event_source(game.display)
    );
    al_register_event_source(
        event_queue, al_get_timer_event_source(main_timer)
    );
}


/**
 * @brief Initializes miscellaneous things and settings.
 */
void init_misc() {
    game.mouse_cursor.init();
    game.shaders.compile_shaders();
    
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(game.display, "Pikifen");
    int new_bitmap_flags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(game.options.advanced.smooth_scaling) {
        enable_flag(new_bitmap_flags, ALLEGRO_MAG_LINEAR);
        enable_flag(new_bitmap_flags, ALLEGRO_MIN_LINEAR);
    }
    if(game.options.advanced.mipmaps_enabled) {
        enable_flag(new_bitmap_flags, ALLEGRO_MIPMAP);
    }
    al_set_new_bitmap_flags(new_bitmap_flags);
    al_reserve_samples(16);
    
    al_identity_transform(&game.identity_transform);
    
    game.rng.init();
    
    game.states.gameplay->whistle.next_dot_timer.start();
    game.states.gameplay->whistle.next_ring_timer.start();
    
    game.states.gameplay->particles =
        ParticleManager(game.options.advanced.max_particles);
        
    game.options.advanced.zoom_mid_level =
        clamp(
            game.options.advanced.zoom_mid_level,
            game.config.zoom_min_level,
            game.config.zoom_max_level
        );
        
    game.liquid_limit_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
    game.wall_offset_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
}


/**
 * @brief Initializes the list of sector types, mission goals, etc.
 */
void init_misc_databases() {
    //Sector types.
    game.sector_types.register_item(
        SECTOR_TYPE_NORMAL, "normal"
    );
    game.sector_types.register_item(
        SECTOR_TYPE_BLOCKING, "blocking"
    );
    
    //Mission goals.
    //Order matters, and should match MISSION_GOAL.
    game.mission_goals.push_back(
        new MissionGoalEndManually()
    );
    game.mission_goals.push_back(
        new MissionGoalCollectTreasures()
    );
    game.mission_goals.push_back(
        new MissionGoalBattleEnemies()
    );
    game.mission_goals.push_back(
        new MissionGoalTimedSurvival()
    );
    game.mission_goals.push_back(
        new MissionGoalGetToExit()
    );
    game.mission_goals.push_back(
        new MissionGoalGrowPikmin()
    );
    
    //Mission fail conditions.
    //Order matters, and should match MISSION_FAIL_COND.
    game.mission_fail_conds.push_back(
        new MissionFailTimeLimit()
    );
    game.mission_fail_conds.push_back(
        new MissionFailTooFewPikmin()
    );
    game.mission_fail_conds.push_back(
        new MissionFailTooManyPikmin()
    );
    game.mission_fail_conds.push_back(
        new MissionFailLosePikmin()
    );
    game.mission_fail_conds.push_back(
        new MissionFailTakeDamage()
    );
    game.mission_fail_conds.push_back(
        new MissionFailLoseLeaders()
    );
    game.mission_fail_conds.push_back(
        new MissionFailKillEnemies()
    );
    game.mission_fail_conds.push_back(
        new MissionFailPauseMenu()
    );
    
    //Mission score criteria.
    //Order matters, and should match MISSION_SCORE_CRITERIA.
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionPikminBorn()
    );
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionPikminDeath()
    );
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionSecLeft()
    );
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionSecPassed()
    );
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionTreasurePoints()
    );
    game.mission_score_criteria.push_back(
        new MissionScoreCriterionEnemyPoints()
    );
}


/**
 * @brief Initializes the list of mob actions.
 */
void init_mob_actions() {

#define reg_param(p_name, p_type, constant, extras) \
    params.push_back(MobActionParam(p_name, p_type, constant, extras));
#define reg_action(a_type, a_name, run_code, load_code) \
    a = &(game.mob_actions[a_type]); \
    a->type = a_type; \
    a->name = a_name; \
    a->code = run_code; \
    a->extra_load_logic = load_code; \
    a->parameters = params; \
    params.clear();


    game.mob_actions.assign(N_MOB_ACTIONS, MobAction());
    vector<MobActionParam> params;
    MobAction* a;
    
    reg_action(
        MOB_ACTION_UNKNOWN,
        "unknown",
        nullptr,
        nullptr
    );
    
    reg_param("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_ADD_HEALTH,
        "add_health",
        mob_action_runners::add_health,
        nullptr
    );
    
    reg_param("goal", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_ARACHNORB_PLAN_LOGIC,
        "arachnorb_plan_logic",
        mob_action_runners::arachnorb_plan_logic,
        mob_action_loaders::arachnorb_plan_logic
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("operand", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("operation", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("operand", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_CALCULATE,
        "calculate",
        mob_action_runners::calculate,
        mob_action_loaders::calculate
    );
    
    reg_action(
        MOB_ACTION_DELETE,
        "delete",
        mob_action_runners::delete_function,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_DRAIN_LIQUID,
        "drain_liquid",
        mob_action_runners::drain_liquid,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_ELSE,
        "else",
        nullptr,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_END_IF,
        "end_if",
        nullptr,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_FINISH_DYING,
        "finish_dying",
        mob_action_runners::finish_dying,
        nullptr
    );
    
    reg_param("target", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_FOCUS,
        "focus",
        mob_action_runners::focus,
        mob_action_loaders::focus
    );
    
    reg_param("label", MOB_ACTION_PARAM_STRING, false, true);
    reg_action(
        MOB_ACTION_FOLLOW_PATH_RANDOMLY,
        "follow_path_randomly",
        mob_action_runners::follow_path_randomly,
        nullptr
    );
    
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("label", MOB_ACTION_PARAM_STRING, false, true);
    reg_action(
        MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
        "follow_path_to_absolute",
        mob_action_runners::follow_path_to_absolute,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("center x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("center y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("target x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("target y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_ANGLE,
        "get_angle",
        mob_action_runners::get_angle,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("info", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_AREA_INFO,
        "get_area_info",
        mob_action_runners::get_area_info,
        mob_action_loaders::get_area_info
    );
    
    reg_action(
        MOB_ACTION_GET_CHOMPED,
        "get_chomped",
        mob_action_runners::get_chomped,
        nullptr
    );
    
    reg_param("x destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("y destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("angle", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("distance", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_COORDINATES_FROM_ANGLE,
        "get_coordinates_from_angle",
        mob_action_runners::get_coordinates_from_angle,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("center x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("center y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("target x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("target y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_DISTANCE,
        "get_distance",
        mob_action_runners::get_distance,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("info", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_EVENT_INFO,
        "get_event_info",
        mob_action_runners::get_event_info,
        mob_action_loaders::get_event_info
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        mob_action_runners::get_floor_z,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("focused mob's var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_FOCUS_VAR,
        "get_focus_var",
        mob_action_runners::get_focus_var,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("target", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("info", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_MOB_INFO,
        "get_mob_info",
        mob_action_runners::get_mob_info,
        mob_action_loaders::get_mob_info
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("minimum value", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("maximum value", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_RANDOM_FLOAT,
        "get_random_float",
        mob_action_runners::get_random_float,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("minimum value", MOB_ACTION_PARAM_INT, false, false);
    reg_param("maximum value", MOB_ACTION_PARAM_INT, false, false);
    reg_action(
        MOB_ACTION_GET_RANDOM_INT,
        "get_random_int",
        mob_action_runners::get_random_int,
        nullptr
    );
    
    reg_param("label name", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GOTO,
        "goto",
        nullptr,
        nullptr
    );
    
    reg_param("body part name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("hold above", MOB_ACTION_PARAM_BOOL, false, true);
    reg_action(
        MOB_ACTION_HOLD_FOCUS,
        "hold_focused_mob",
        mob_action_runners::hold_focus,
        mob_action_loaders::hold_focus
    );
    
    reg_param("comparand", MOB_ACTION_PARAM_STRING, false, false);
    reg_param("operation", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("value", MOB_ACTION_PARAM_STRING, false, true);
    reg_action(
        MOB_ACTION_IF,
        "if",
        mob_action_runners::if_function,
        mob_action_loaders::if_function
    );
    
    reg_param("label name", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_LABEL,
        "label",
        nullptr,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_LINK_WITH_FOCUS,
        "link_with_focused_mob",
        mob_action_runners::link_with_focus,
        nullptr
    );
    
    reg_param("slot", MOB_ACTION_PARAM_INT, false, false);
    reg_action(
        MOB_ACTION_LOAD_FOCUS_MEMORY,
        "load_focused_mob_memory",
        mob_action_runners::load_focus_memory,
        nullptr
    );
    
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("z", MOB_ACTION_PARAM_FLOAT, false, true);
    reg_action(
        MOB_ACTION_MOVE_TO_ABSOLUTE,
        "move_to_absolute",
        mob_action_runners::move_to_absolute,
        nullptr
    );
    
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("z", MOB_ACTION_PARAM_FLOAT, false, true);
    reg_action(
        MOB_ACTION_MOVE_TO_RELATIVE,
        "move_to_relative",
        mob_action_runners::move_to_relative,
        nullptr
    );
    
    reg_param("target", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_MOVE_TO_TARGET,
        "move_to_target",
        mob_action_runners::move_to_target,
        mob_action_loaders::move_to_target
    );
    
    reg_action(
        MOB_ACTION_ORDER_RELEASE,
        "order_release",
        mob_action_runners::order_release,
        nullptr
    );
    
    reg_param("sound data", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param(
        "sound ID destination var name", MOB_ACTION_PARAM_STRING, true, true
    );
    reg_action(
        MOB_ACTION_PLAY_SOUND,
        "play_sound",
        mob_action_runners::play_sound,
        mob_action_loaders::play_sound
    );
    
    reg_param("text", MOB_ACTION_PARAM_STRING, false, true);
    reg_action(
        MOB_ACTION_PRINT,
        "print",
        mob_action_runners::print,
        nullptr
    );
    
    reg_param("status name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_RECEIVE_STATUS,
        "receive_status",
        mob_action_runners::receive_status,
        mob_action_loaders::receive_status
    );
    
    reg_action(
        MOB_ACTION_RELEASE,
        "release",
        mob_action_runners::release,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_RELEASE_STORED_MOBS,
        "release_stored_mobs",
        mob_action_runners::release_stored_mobs,
        nullptr
    );
    
    reg_param("status name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_REMOVE_STATUS,
        "remove_status",
        mob_action_runners::remove_status,
        mob_action_loaders::remove_status
    );
    
    reg_param("slot", MOB_ACTION_PARAM_INT, false, false);
    reg_action(
        MOB_ACTION_SAVE_FOCUS_MEMORY,
        "save_focused_mob_memory",
        mob_action_runners::save_focus_memory,
        nullptr
    );
    
    reg_param("message", MOB_ACTION_PARAM_STRING, false, false);
    reg_action(
        MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
        "send_message_to_focus",
        mob_action_runners::send_message_to_focus,
        nullptr
    );
    
    reg_param("message", MOB_ACTION_PARAM_STRING, false, false);
    reg_action(
        MOB_ACTION_SEND_MESSAGE_TO_LINKS,
        "send_message_to_links",
        mob_action_runners::send_message_to_links,
        nullptr
    );
    
    reg_param("distance", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("message", MOB_ACTION_PARAM_STRING, false, false);
    reg_action(
        MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
        "send_message_to_nearby",
        mob_action_runners::send_message_to_nearby,
        nullptr
    );
    
    reg_param("animation name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("options", MOB_ACTION_PARAM_ENUM, true, true);
    reg_action(
        MOB_ACTION_SET_ANIMATION,
        "set_animation",
        mob_action_runners::set_animation,
        mob_action_loaders::set_animation
    );
    
    reg_param("blocks", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_CAN_BLOCK_PATHS,
        "set_can_block_paths",
        mob_action_runners::set_can_block_paths,
        nullptr
    );
    
    reg_param("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_SET_FAR_REACH,
        "set_far_reach",
        mob_action_runners::set_far_reach,
        mob_action_loaders::set_far_reach
    );
    
    reg_param("flying", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_FLYING,
        "set_flying",
        mob_action_runners::set_flying,
        nullptr
    );
    
    reg_param("multiplier", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_GRAVITY,
        "set_gravity",
        mob_action_runners::set_gravity,
        nullptr
    );
    
    reg_param("amount", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_HEALTH,
        "set_health",
        mob_action_runners::set_health,
        nullptr
    );
    
    reg_param("height", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_HEIGHT,
        "set_height",
        mob_action_runners::set_height,
        nullptr
    );
    
    reg_param("hiding", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_HIDING,
        "set_hiding",
        mob_action_runners::set_hiding,
        nullptr
    );
    
    reg_param("huntable", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_HUNTABLE,
        "set_huntable",
        mob_action_runners::set_huntable,
        nullptr
    );
    
    reg_param("options", MOB_ACTION_PARAM_ENUM, true, true);
    reg_action(
        MOB_ACTION_SET_HOLDABLE,
        "set_holdable",
        mob_action_runners::set_holdable,
        mob_action_loaders::set_holdable
    );
    
    reg_param("animation name", MOB_ACTION_PARAM_STRING, false, false);
    reg_action(
        MOB_ACTION_SET_LIMB_ANIMATION,
        "set_limb_animation",
        mob_action_runners::set_limb_animation,
        nullptr
    );
    
    reg_param("reach name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_SET_NEAR_REACH,
        "set_near_reach",
        mob_action_runners::set_near_reach,
        mob_action_loaders::set_near_reach
    );
    
    reg_param("radius", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_RADIUS,
        "set_radius",
        mob_action_runners::set_radius,
        nullptr
    );
    
    reg_param("x speed", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y speed", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_SECTOR_SCROLL,
        "set_sector_scroll",
        mob_action_runners::set_sector_scroll,
        nullptr
    );
    
    reg_param("visible", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_SHADOW_VISIBILITY,
        "set_shadow_visibility",
        mob_action_runners::set_shadow_visibility,
        nullptr
    );
    
    reg_param("state name", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_SET_STATE,
        "set_state",
        mob_action_runners::set_state,
        nullptr
    );
    
    reg_param("tangible", MOB_ACTION_PARAM_BOOL, false, false);
    reg_action(
        MOB_ACTION_SET_TANGIBLE,
        "set_tangible",
        mob_action_runners::set_tangible,
        nullptr
    );
    
    reg_param("team name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_SET_TEAM,
        "set_team",
        mob_action_runners::set_team,
        mob_action_loaders::set_team
    );
    
    reg_param("time", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_SET_TIMER,
        "set_timer",
        mob_action_runners::set_timer,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("value", MOB_ACTION_PARAM_STRING, false, false);
    reg_action(
        MOB_ACTION_SET_VAR,
        "set_var",
        mob_action_runners::set_var,
        nullptr
    );
    
    reg_param("var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
        "show_message_from_var",
        mob_action_runners::show_message_from_var,
        nullptr
    );
    
    reg_param("spawn data", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_SPAWN,
        "spawn",
        mob_action_runners::spawn,
        mob_action_loaders::spawn
    );
    
    reg_param("reference", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("offset", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_STABILIZE_Z,
        "stabilize_z",
        mob_action_runners::stabilize_z,
        mob_action_loaders::stabilize_z
    );
    
    reg_param("victim max", MOB_ACTION_PARAM_INT, false, false);
    reg_param("body part", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("more body parts", MOB_ACTION_PARAM_ENUM, true, true);
    reg_action(
        MOB_ACTION_START_CHOMPING,
        "start_chomping",
        mob_action_runners::start_chomping,
        mob_action_loaders::start_chomping
    );
    
    reg_action(
        MOB_ACTION_START_DYING,
        "start_dying",
        mob_action_runners::start_dying,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_START_HEIGHT_EFFECT,
        "start_height_effect",
        mob_action_runners::start_height_effect,
        nullptr
    );
    
    reg_param("generator name", MOB_ACTION_PARAM_ENUM, true, false);
    reg_param("offset coordinates", MOB_ACTION_PARAM_FLOAT, false, true);
    reg_action(
        MOB_ACTION_START_PARTICLES,
        "start_particles",
        mob_action_runners::start_particles,
        mob_action_loaders::start_particles
    );
    
    reg_action(
        MOB_ACTION_STOP,
        "stop",
        mob_action_runners::stop,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_STOP_CHOMPING,
        "stop_chomping",
        mob_action_runners::stop_chomping,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_STOP_HEIGHT_EFFECT,
        "stop_height_effect",
        mob_action_runners::stop_height_effect,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_STOP_PARTICLES,
        "stop_particles",
        mob_action_runners::stop_particles,
        nullptr
    );
    
    reg_param("sound ID", MOB_ACTION_PARAM_INT, false, false);
    reg_action(
        MOB_ACTION_STOP_SOUND,
        "stop_sound",
        mob_action_runners::stop_sound,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_STOP_VERTICALLY,
        "stop_vertically",
        mob_action_runners::stop_vertically,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_STORE_FOCUS_INSIDE,
        "store_focus_inside",
        mob_action_runners::store_focus_inside,
        nullptr
    );
    
    reg_param("amount", MOB_ACTION_PARAM_INT, false, false);
    reg_action(
        MOB_ACTION_SWALLOW,
        "swallow",
        mob_action_runners::swallow,
        nullptr
    );
    
    reg_action(
        MOB_ACTION_SWALLOW_ALL,
        "swallow_all",
        mob_action_runners::swallow_all,
        nullptr
    );
    
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("z", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_TELEPORT_TO_ABSOLUTE,
        "teleport_to_absolute",
        mob_action_runners::teleport_to_absolute,
        nullptr
    );
    
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("z", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_TELEPORT_TO_RELATIVE,
        "teleport_to_relative",
        mob_action_runners::teleport_to_relative,
        nullptr
    );
    
    reg_param("x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("z coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("max height", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_THROW_FOCUS,
        "throw_focused_mob",
        mob_action_runners::throw_focus,
        nullptr
    );
    
    reg_param("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    reg_action(
        MOB_ACTION_TURN_TO_ABSOLUTE,
        "turn_to_absolute",
        mob_action_runners::turn_to_absolute,
        nullptr
    );
    
    reg_param("angle or x coordinate", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y coordinate", MOB_ACTION_PARAM_FLOAT, false, true);
    reg_action(
        MOB_ACTION_TURN_TO_RELATIVE,
        "turn_to_relative",
        mob_action_runners::turn_to_relative,
        nullptr
    );
    
    reg_param("target", MOB_ACTION_PARAM_ENUM, true, false);
    reg_action(
        MOB_ACTION_TURN_TO_TARGET,
        "turn_to_target",
        mob_action_runners::turn_to_target,
        mob_action_loaders::turn_to_target
    );
    
    
#undef param
#undef reg_action
}


/**
 * @brief Initializes the list of mob categories.
 */
void init_mob_categories() {

    game.mob_categories.register_category(
        MOB_CATEGORY_NONE, new NoneCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PIKMIN, new PikminCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_ONIONS, new OnionCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_LEADERS, new LeaderCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_ENEMIES, new EnemyCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TREASURES, new TreasureCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PELLETS, new PelletCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_CONVERTERS, new ConverterCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_DROPS, new DropCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_RESOURCES, new ResourceCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PILES, new PileCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TOOLS, new ToolCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_SHIPS, new ShipCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_BRIDGES, new BridgeCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_GROUP_TASKS, new GroupTaskCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_SCALES, new ScaleCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TRACKS, new TrackCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_BOUNCERS, new BouncerCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_DECORATIONS, new DecorationCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_INTERACTABLES, new InteractableCategory()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_CUSTOM, new CustomCategory()
    );
}
