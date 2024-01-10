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

#include "controls.h"
#include "functions.h"
#include "game.h"
#include "game_states/game_state.h"
#include "game_states/gameplay/gameplay.h"
#include "game_states/menus.h"
#include "libs/imgui/imgui_impl_allegro5.h"
#include "mob_categories/bouncer_category.h"
#include "mob_categories/bridge_category.h"
#include "mob_categories/converter_category.h"
#include "mob_categories/custom_category.h"
#include "mob_categories/decoration_category.h"
#include "mob_categories/drop_category.h"
#include "mob_categories/enemy_category.h"
#include "mob_categories/group_task_category.h"
#include "mob_categories/interactable_category.h"
#include "mob_categories/leader_category.h"
#include "mob_categories/onion_category.h"
#include "mob_categories/pellet_category.h"
#include "mob_categories/pikmin_category.h"
#include "mob_categories/pile_category.h"
#include "mob_categories/resource_category.h"
#include "mob_categories/scale_category.h"
#include "mob_categories/ship_category.h"
#include "mob_categories/tool_category.h"
#include "mob_categories/track_category.h"
#include "mob_categories/treasure_category.h"
#include "mob_script.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Destroys Allegro and modules.
 */
void destroy_allegro() {
    al_uninstall_joystick();
    al_uninstall_audio();
    al_uninstall_keyboard();
    al_uninstall_mouse();
    al_uninstall_system();
}


/* ----------------------------------------------------------------------------
 * Destroys Allegro's event-related things.
 * logic_timer:
 *   The game logic timer.
 * logic_queue:
 *   Queue of game logic events.
 */
void destroy_event_things(
    ALLEGRO_TIMER* &logic_timer, ALLEGRO_EVENT_QUEUE* &logic_queue
) {
    al_destroy_event_queue(logic_queue);
    al_destroy_timer(logic_timer);
    al_destroy_display(game.display);
}


/* ----------------------------------------------------------------------------
 * Destroys miscellaneous things.
 */
void destroy_misc() {
    al_destroy_bitmap(game.bmp_error);
    al_destroy_font(game.fonts.area_name);
    al_destroy_font(game.fonts.counter);
    al_destroy_font(game.fonts.cursor_counter);
    al_destroy_font(game.fonts.slim);
    al_destroy_font(game.fonts.standard);
    al_destroy_font(game.fonts.value);
    
    game.audio.destroy();
}


/* ----------------------------------------------------------------------------
 * Destroys registered mob categories.
 */
void destroy_mob_categories() {
    game.mob_categories.clear();
}


/* ----------------------------------------------------------------------------
 * Initializes Allegro and its modules.
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


/* ----------------------------------------------------------------------------
 * Initializes things related to the controls.
 */
void init_controls() {
    //Register the existing actions.
    //They must be registered in the same order as the action types enum.
    
    game.controls.add_player_action_type(
        PLAYER_ACTION_NONE,
        PLAYER_ACTION_CAT_NONE,
        "---", "", "", ""
    );
    
    //MAIN.
    game.controls.add_player_action_type(
        PLAYER_ACTION_RIGHT,
        PLAYER_ACTION_CAT_MAIN,
        "Right",
        "Move right.",
        "move_right", "k_4"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_UP,
        PLAYER_ACTION_CAT_MAIN,
        "Up",
        "Move up.",
        "move_up", "k_23"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_LEFT,
        PLAYER_ACTION_CAT_MAIN,
        "Left",
        "Move left.",
        "move_left", "k_1"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_DOWN,
        PLAYER_ACTION_CAT_MAIN,
        "Down",
        "Move down.",
        "move_down", "k_19"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_THROW,
        PLAYER_ACTION_CAT_MAIN,
        "Throw",
        "Throw a Pikmin.",
        "throw", "mb_1"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_WHISTLE,
        PLAYER_ACTION_CAT_MAIN,
        "Whistle",
        "Whistle around the cursor.",
        "whistle", "mb_2"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_NEXT_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Next Pikmin",
        "Change to the next Pikmin type in the group.",
        "next_type", "mwd"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_PREV_TYPE,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. Pikmin",
        "Change to the previous Pikmin type in the group.",
        "prev_type", "mwu"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_NEXT_LEADER,
        PLAYER_ACTION_CAT_MAIN,
        "Next leader",
        "Change to the next leader.",
        "next_leader", "k_64"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_GROUP_CURSOR,
        PLAYER_ACTION_CAT_MAIN,
        "Swarm to cursor",
        "Swarm all Pikmin towards the cursor.",
        "swarm_cursor", "k_75"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_DISMISS,
        PLAYER_ACTION_CAT_MAIN,
        "Dismiss",
        "Dismiss all Pikmin.",
        "dismiss", "k_217"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_USE_SPRAY_1,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 1",
        "Use the spray in slot 1.",
        "use_spray_1", "k_18"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_USE_SPRAY_2,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray 2",
        "Use the spray in slot 2.",
        "use_spray_2", "k_6"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_USE_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Use spray",
        "Use the currently selected spray.",
        "use_spray", "k_18"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_NEXT_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Next spray",
        "Change to the next spray.",
        "next_spray", "k_5"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_PREV_SPRAY,
        PLAYER_ACTION_CAT_MAIN,
        "Prev. spray",
        "Change to the previous spray.",
        "prev_spray", "k_17"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_PAUSE,
        PLAYER_ACTION_CAT_MAIN,
        "Pause",
        "Pause the game.",
        "pause", "k_59"
    );
    
    //Menus.
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_RIGHT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu right",
        "Navigate right in a menu.",
        "menu_right", "k_83"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_UP,
        PLAYER_ACTION_CAT_MENUS,
        "Menu up",
        "Navigate up in a menu.",
        "menu_up", "k_84"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_LEFT,
        PLAYER_ACTION_CAT_MENUS,
        "Menu left",
        "Navigate left in a menu.",
        "menu_left", "k_82"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_DOWN,
        PLAYER_ACTION_CAT_MENUS,
        "Menu down",
        "Navigate down in a menu.",
        "menu_down", "k_85"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_OK,
        PLAYER_ACTION_CAT_MENUS,
        "Menu OK",
        "Confirm the selected item in a menu.",
        "menu_ok", "k_67"
    );
    
    //Advanced.
    game.controls.add_player_action_type(
        PLAYER_ACTION_CURSOR_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor right",
        "Move the cursor right. Useful if it's not mouse-controlled.",
        "cursor_right", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CURSOR_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor up",
        "Move the cursor up. Useful if it's not mouse-controlled.",
        "cursor_up", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CURSOR_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor left",
        "Move the cursor left. Useful if it's not mouse-controlled.",
        "cursor_left", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CURSOR_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Cursor down",
        "Move the cursor down. Useful if it's not mouse-controlled.",
        "cursor_down", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_GROUP_RIGHT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm right",
        "Swarm all Pikmin right.",
        "swarm_right", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_GROUP_UP,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm up",
        "Swarm all Pikmin up.",
        "swarm_up", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_GROUP_LEFT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm left",
        "Swarm all Pikmin left.",
        "swarm_left", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_GROUP_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Swarm down",
        "Swarm all Pikmin down.",
        "swarm_down", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_PREV_LEADER,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. leader",
        "Change to the previous leader.",
        "prev_leader", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CHANGE_ZOOM,
        PLAYER_ACTION_CAT_ADVANCED,
        "Change zoom",
        "Change the current zoom level.",
        "change_zoom", "k_3"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_ZOOM_IN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom in",
        "Change to a closer zoom level.",
        "zoom_in", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_ZOOM_OUT,
        PLAYER_ACTION_CAT_ADVANCED,
        "Zoom out",
        "Change to a farther zoom level.",
        "zoom_out", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_NEXT_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Next maturity",
        "Change to a Pikmin of the next maturity.",
        "next_maturity", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_PREV_MATURITY,
        PLAYER_ACTION_CAT_ADVANCED,
        "Prev. maturity",
        "Change to a Pikmin of the previous maturity.",
        "prev_maturity", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_LIE_DOWN,
        PLAYER_ACTION_CAT_ADVANCED,
        "Lie down",
        "Lie down so Pikmin can carry you.",
        "lie_down", "k_26"
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CUSTOM_A,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom A",
        "Custom action A, if the current leader supports it.",
        "custom_a", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CUSTOM_B,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom B",
        "Custom action B, if the current leader supports it.",
        "custom_b", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_CUSTOM_C,
        PLAYER_ACTION_CAT_ADVANCED,
        "Custom C",
        "Custom action C, if the current leader supports it.",
        "custom_c", ""
    );
    game.controls.add_player_action_type(
        PLAYER_ACTION_MENU_BACK,
        PLAYER_ACTION_CAT_ADVANCED,
        "Menu shortcut - back",
        "Go back or cancel in a menu.",
        "menu_back", "k_59"
    );
    
    
    //Populate the control binds with some default control binds for player 1.
    //If the options are loaded successfully, these binds are overwritten.
    const vector<player_action_type> &action_types =
        game.controls.get_all_player_action_types();
    for(size_t a = 0; a < N_PLAYER_ACTIONS; ++a) {
        string def = action_types[a].default_bind_str;
        if(def.empty()) continue;
        
        control_bind bind;
        bind.action_type_id = action_types[a].id;
        bind.player_nr = 0;
        bind.input = game.controls.str_to_input(def);
        game.controls.binds().push_back(bind);
    }
}


/* ----------------------------------------------------------------------------
 * Initializes Dear ImGui.
 */
void init_dear_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplAllegro5_Init(game.display);
    ImGui::GetIO().IniFilename = "";
    ImGui::GetIO().ConfigDragClickToInputText = true;
    for(size_t c = 0; c < ImGuiCol_COUNT; ++c) {
        game.imgui_default_style[c] = ImGui::GetStyle().Colors[c];
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the error bitmap.
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


/* ----------------------------------------------------------------------------
 * Initializes some essential things.
 */
void init_essentials() {
    //Signal handlers.
    signal(SIGFPE,  signal_handler);
    signal(SIGILL,  signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
}


/* ----------------------------------------------------------------------------
 * Initializes things regarding Allegro events, like the queue, timer, etc.
 * logic_timer:
 *   The game logic timer.
 * logic_queue:
 *   Queue of game logic events.
 */
void init_event_things(
    ALLEGRO_TIMER* &logic_timer, ALLEGRO_EVENT_QUEUE* &logic_queue
) {
    al_set_new_display_flags(
        al_get_new_display_flags() |
        ALLEGRO_OPENGL
    );
    if(game.options.window_position_hack) al_set_new_window_position(64, 64);
    if(game.win_fullscreen) {
        al_set_new_display_flags(
            al_get_new_display_flags() |
            (
                game.options.true_fullscreen ?
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
        game.options.intended_win_fullscreen = false;
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
    
    logic_timer = al_create_timer(1.0f / game.options.target_fps);
    if(!logic_timer) {
        report_fatal_error("Could not create the main logic timer!");
    }
    
    logic_queue = al_create_event_queue();
    if(!logic_queue) {
        report_fatal_error("Could not create the main logic event queue!");
    }
    al_register_event_source(logic_queue, al_get_mouse_event_source());
    al_register_event_source(logic_queue, al_get_keyboard_event_source());
    al_register_event_source(logic_queue, al_get_joystick_event_source());
    al_register_event_source(
        logic_queue, al_get_display_event_source(game.display)
    );
    al_register_event_source(
        logic_queue, al_get_timer_event_source(logic_timer)
    );
}


/* ----------------------------------------------------------------------------
 * Initializes miscellaneous things and settings.
 */
void init_misc() {
    game.mouse_cursor.init();
    
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(game.display, "Pikifen");
    int new_bitmap_flags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(game.options.smooth_scaling) {
        enable_flag(new_bitmap_flags, ALLEGRO_MAG_LINEAR);
        enable_flag(new_bitmap_flags, ALLEGRO_MIN_LINEAR);
    }
    if(game.options.mipmaps_enabled) {
        enable_flag(new_bitmap_flags, ALLEGRO_MIPMAP);
    }
    al_set_new_bitmap_flags(new_bitmap_flags);
    al_reserve_samples(16);
    
    al_identity_transform(&game.identity_transform);
    
    srand(time(NULL));
    
    game.states.gameplay->whistle.next_dot_timer.start();
    game.states.gameplay->whistle.next_ring_timer.start();
    
    game.states.gameplay->particles =
        particle_manager(game.options.max_particles);
        
    game.options.zoom_mid_level =
        clamp(
            game.options.zoom_mid_level,
            game.config.zoom_min_level,
            game.config.zoom_max_level
        );
        
    //Some maker tool defaults that are convenient to have on.
    game.maker_tools.keys[10] = MAKER_TOOL_AREA_IMAGE;
    game.maker_tools.keys[11] = MAKER_TOOL_CHANGE_SPEED;
    game.maker_tools.keys[12] = MAKER_TOOL_TELEPORT;
    game.maker_tools.keys[13] = MAKER_TOOL_HURT_MOB;
    game.maker_tools.keys[14] = MAKER_TOOL_NEW_PIKMIN;
    game.maker_tools.keys[15] = MAKER_TOOL_MOB_INFO;
    game.maker_tools.keys[16] = MAKER_TOOL_GEOMETRY_INFO;
    game.maker_tools.keys[17] = MAKER_TOOL_HITBOXES;
    game.maker_tools.keys[18] = MAKER_TOOL_COLLISION;
    game.maker_tools.keys[19] = MAKER_TOOL_HUD;
    
    game.liquid_limit_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
    game.wall_offset_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
}


/* ----------------------------------------------------------------------------
 * Initializes the list of sector types, mission goals, etc.
 */
void init_misc_databases() {
    //Sector types.
    game.sector_types.register_item(
        SECTOR_TYPE_NORMAL, "Normal"
    );
    game.sector_types.register_item(
        SECTOR_TYPE_BLOCKING, "Blocking"
    );
    
    //Mission goals.
    //Order matters, and should match MISSION_GOALS.
    game.mission_goals.push_back(
        new mission_goal_end_manually()
    );
    game.mission_goals.push_back(
        new mission_goal_collect_treasures()
    );
    game.mission_goals.push_back(
        new mission_goal_battle_enemies()
    );
    game.mission_goals.push_back(
        new mission_goal_timed_survival()
    );
    game.mission_goals.push_back(
        new mission_goal_get_to_exit()
    );
    game.mission_goals.push_back(
        new mission_goal_grow_pikmin()
    );
    
    //Mission fail conditions.
    //Order matters, and should match MISSION_FAIL_CONDITIONS.
    game.mission_fail_conds.push_back(
        new mission_fail_time_limit()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_too_few_pikmin()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_too_many_pikmin()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_lose_pikmin()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_take_damage()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_lose_leaders()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_kill_enemies()
    );
    game.mission_fail_conds.push_back(
        new mission_fail_pause_menu()
    );
    
    //Mission score criteria.
    //Order matters, and should match MISSION_SCORE_CRITERIA.
    game.mission_score_criteria.push_back(
        new mission_score_criterion_pikmin_born()
    );
    game.mission_score_criteria.push_back(
        new mission_score_criterion_pikmin_death()
    );
    game.mission_score_criteria.push_back(
        new mission_score_criterion_sec_left()
    );
    game.mission_score_criteria.push_back(
        new mission_score_criterion_sec_passed()
    );
    game.mission_score_criteria.push_back(
        new mission_score_criterion_treasure_points()
    );
    game.mission_score_criteria.push_back(
        new mission_score_criterion_enemy_points()
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the list of mob actions.
 */
void init_mob_actions() {

#define reg_param(p_name, p_type, constant, extras) \
    params.push_back(mob_action_param(p_name, p_type, constant, extras));
#define reg_action(a_type, a_name, run_code, load_code) \
    a = &(game.mob_actions[a_type]); \
    a->type = a_type; \
    a->name = a_name; \
    a->code = run_code; \
    a->extra_load_logic = load_code; \
    a->parameters = params; \
    params.clear();


    game.mob_actions.assign(N_MOB_ACTIONS, mob_action());
    vector<mob_action_param> params;
    mob_action* a;
    
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
        MOB_ACTION_GET_RANDOM_DECIMAL,
        "get_random_decimal",
        mob_action_runners::get_random_decimal,
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


/* ----------------------------------------------------------------------------
 * Initializes the list of mob categories.
 */
void init_mob_categories() {

    game.mob_categories.register_category(
        MOB_CATEGORY_NONE, new none_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PIKMIN, new pikmin_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_ONIONS, new onion_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_LEADERS, new leader_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_ENEMIES, new enemy_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TREASURES, new treasure_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PELLETS, new pellet_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_CONVERTERS, new converter_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_DROPS, new drop_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_RESOURCES, new resource_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_PILES, new pile_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TOOLS, new tool_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_SHIPS, new ship_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_BRIDGES, new bridge_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_GROUP_TASKS, new group_task_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_SCALES, new scale_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_TRACKS, new track_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_BOUNCERS, new bouncer_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_DECORATIONS, new decoration_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_INTERACTABLES, new interactable_category()
    );
    game.mob_categories.register_category(
        MOB_CATEGORY_CUSTOM, new custom_category()
    );
}


/* ----------------------------------------------------------------------------
 * Loads a single animation from the system animations definition file.
 * anim_def_file:
 *   The animation definition file.
 * name:
 *   Name of the animation on this file.
 * anim:
 *   The single animation suite structure to fill.
 */
void init_single_animation(
    data_node* anim_def_file, const string &name,
    single_animation_suite &anim
) {
    data_node file(
        ANIMATIONS_FOLDER_PATH + "/" +
        anim_def_file->get_child_by_name(name)->value
    );
    anim.database = load_animation_database_from_file(&file);
    anim.instance.cur_anim = anim.database.animations[0];
    anim.instance.start();
}
