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

#include "init.h"

#include "controls.h"
#include "functions.h"
#include "game.h"
#include "game_states/game_state.h"
#include "game_states/gameplay/gameplay.h"
#include "game_states/menus.h"
#include "imgui/imgui_impl_allegro5.h"
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
    al_destroy_font(game.fonts.standard);
    al_destroy_font(game.fonts.value);
    
    al_detach_voice(game.voice);
    al_destroy_mixer(game.mixer);
    al_destroy_voice(game.voice);
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
    al_install_joystick();
}


/* ----------------------------------------------------------------------------
 * Initializes the default controls.
 */
void init_controls() {
    //Declare the existing buttons.
    game.buttons.add(
        BUTTON_NONE, "---", "", ""
    );
    game.buttons.add(
        BUTTON_THROW, "Throw", "throw", "mb_1"
    );
    game.buttons.add(
        BUTTON_WHISTLE, "Whistle", "whistle", "mb_2"
    );
    game.buttons.add(
        BUTTON_RIGHT, "Right", "move_right", "k_4"
    );
    game.buttons.add(
        BUTTON_UP, "Up", "move_up", "k_23"
    );
    game.buttons.add(
        BUTTON_LEFT, "Left", "move_left", "k_1"
    );
    game.buttons.add(
        BUTTON_DOWN, "Down", "move_down", "k_19"
    );
    game.buttons.add(
        BUTTON_CURSOR_RIGHT, "Cursor right", "cursor_right", ""
    );
    game.buttons.add(
        BUTTON_CURSOR_UP, "Cursor up", "cursor_up", ""
    );
    game.buttons.add(
        BUTTON_CURSOR_LEFT, "Cursor left", "cursor_left", ""
    );
    game.buttons.add(
        BUTTON_CURSOR_DOWN, "Cursor down", "cursor_down", ""
    );
    game.buttons.add(
        BUTTON_GROUP_RIGHT, "Swarm right", "swarm_right", ""
    );
    game.buttons.add(
        BUTTON_GROUP_UP, "Swarm up", "swarm_up", ""
    );
    game.buttons.add(
        BUTTON_GROUP_LEFT, "Swarm left", "swarm_left", ""
    );
    game.buttons.add(
        BUTTON_GROUP_DOWN, "Swarm down", "swarm_down", ""
    );
    game.buttons.add(
        BUTTON_GROUP_CURSOR, "Swarm to cursor", "swarm_cursor", "k_75"
    );
    game.buttons.add(
        BUTTON_NEXT_LEADER, "Next leader", "next_leader", "k_64"
    );
    game.buttons.add(
        BUTTON_PREV_LEADER, "Prev. leader", "prev_leader", ""
    );
    game.buttons.add(
        BUTTON_DISMISS, "Dismiss", "dismiss", "k_217"
    );
    game.buttons.add(
        BUTTON_USE_SPRAY_1, "Use spray 1", "use_spray_1", "k_18"
    );
    game.buttons.add(
        BUTTON_USE_SPRAY_2, "Use spray 2", "use_spray_2", "k_6"
    );
    game.buttons.add(
        BUTTON_USE_SPRAY, "Use spray", "use_spray", "k_18"
    );
    game.buttons.add(
        BUTTON_NEXT_SPRAY, "Next spray", "next_spray", "k_5"
    );
    game.buttons.add(
        BUTTON_PREV_SPRAY, "Prev. spray", "prev_spray", "k_17"
    );
    game.buttons.add(
        BUTTON_CHANGE_ZOOM, "Change zoom", "change_zoom", "k_3"
    );
    game.buttons.add(
        BUTTON_ZOOM_IN, "Zoom in", "zoom_in", ""
    );
    game.buttons.add(
        BUTTON_ZOOM_OUT, "Zoom out", "zoom_out", ""
    );
    game.buttons.add(
        BUTTON_NEXT_TYPE, "Next Pikmin", "next_type", "mwd"
    );
    game.buttons.add(
        BUTTON_PREV_TYPE, "Prev. Pikmin", "prev_type", "mwu"
    );
    game.buttons.add(
        BUTTON_NEXT_MATURITY, "Next maturity", "next_maturity", ""
    );
    game.buttons.add(
        BUTTON_PREV_MATURITY, "Prev. maturity", "prev_maturity", ""
    );
    game.buttons.add(
        BUTTON_LIE_DOWN, "Lie down", "lie_down", "k_26"
    );
    game.buttons.add(
        BUTTON_PAUSE, "Pause", "pause", "k_59"
    );
    game.buttons.add(
        BUTTON_MENU_RIGHT, "Menu right", "menu_right", "k_83"
    );
    game.buttons.add(
        BUTTON_MENU_UP, "Menu up", "menu_up", "k_84"
    );
    game.buttons.add(
        BUTTON_MENU_LEFT, "Menu left", "menu_left", "k_82"
    );
    game.buttons.add(
        BUTTON_MENU_DOWN, "Menu down", "menu_down", "k_85"
    );
    game.buttons.add(
        BUTTON_MENU_OK, "Menu OK", "menu_ok", "k_67"
    );
    game.buttons.add(
        BUTTON_MENU_BACK, "Menu back", "menu_back", "k_59"
    );
    
    game.options.controls.assign(MAX_PLAYERS, vector<control_info>());
    
    //Populate the controls information with some default controls for player 1.
    //If the options are loaded successfully, these controls are overwritten.
    for(size_t b = 0; b < N_BUTTONS; ++b) {
        string dc = game.buttons.list[b].default_control_str;
        if(dc.empty()) continue;
        
        game.options.controls[0].push_back(
            control_info(game.buttons.list[b].id, dc)
        );
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
    memcpy(
        game.imgui_default_style,
        &(ImGui::GetStyle().Colors),
        sizeof(ImVec4) * ImGuiCol_COUNT
    );
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
        log_error(
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
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(game.display, "Pikifen");
    int new_bitmap_flags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(game.options.smooth_scaling) {
        new_bitmap_flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
    }
    if(game.options.mipmaps_enabled) {
        new_bitmap_flags |= ALLEGRO_MIPMAP;
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
    
    game.liquid_limit_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
    game.wall_offset_effect_buffer = al_create_bitmap(game.win_w, game.win_h);
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
    reg_param("x", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_param("y", MOB_ACTION_PARAM_FLOAT, false, false);
    reg_action(
        MOB_ACTION_GET_FLOOR_Z,
        "get_floor_z",
        mob_action_runners::get_floor_z,
        nullptr
    );
    
    reg_param("destination var name", MOB_ACTION_PARAM_STRING, true, false);
    reg_param("info", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_FOCUS_INFO,
        "get_focus_info",
        mob_action_runners::get_focus_info,
        mob_action_loaders::get_info
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
    reg_param("info", MOB_ACTION_PARAM_STRING, true, false);
    reg_action(
        MOB_ACTION_GET_INFO,
        "get_info",
        mob_action_runners::get_info,
        mob_action_loaders::get_info
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
    
    reg_action(
        MOB_ACTION_PLAY_SOUND,
        "play_sound",
        mob_action_runners::play_sound,
        nullptr
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
    
    reg_param("angle", MOB_ACTION_PARAM_FLOAT, false, false);
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
 * Initializes the list of sector types.
 */
void init_sector_types() {
    game.sector_types.register_type(SECTOR_TYPE_NORMAL, "Normal");
    game.sector_types.register_type(SECTOR_TYPE_BLOCKING, "Blocking");
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
