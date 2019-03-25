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

#include "editors/area_editor_imgui/editor.h"
#include "editors/animation_editor/editor.h"
#include "controls.h"
#include "functions.h"
#include "gameplay.h"
#include "game_state.h"
#include "menus.h"
#include "mob_categories/bridge_category.h"
#include "mob_categories/converter_category.h"
#include "mob_categories/custom_category.h"
#include "mob_categories/decoration_category.h"
#include "mob_categories/drop_category.h"
#include "mob_categories/enemy_category.h"
#include "mob_categories/gate_category.h"
#include "mob_categories/interactable_category.h"
#include "mob_categories/leader_category.h"
#include "mob_categories/onion_category.h"
#include "mob_categories/pellet_category.h"
#include "mob_categories/pikmin_category.h"
#include "mob_categories/pile_category.h"
#include "mob_categories/resource_category.h"
#include "mob_categories/ship_category.h"
#include "mob_categories/tool_category.h"
#include "mob_categories/treasure_category.h"
#include "mob_script.h"
#include "utils/string_utils.h"
#include "vars.h"


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
 * Initializes the default asset file names.
 */
void init_asset_file_names() {
    asset_file_names.area_name_font =    "Area_name_font.png";
    asset_file_names.checkbox_check =    "Checkbox_check.png";
    asset_file_names.cursor =            "Cursor.png";
    asset_file_names.cursor_invalid =    "Cursor_invalid.png";
    asset_file_names.counter_font =      "Counter_font.png";
    asset_file_names.editor_icons =      "Editor_icons.png";
    asset_file_names.enemy_spirit =      "Enemy_spirit.png";
    asset_file_names.group_move_arrow =  "Group_move_arrow.png";
    asset_file_names.icon =              "Icon.png";
    asset_file_names.idle_glow =         "Idle_glow.png";
    asset_file_names.main_font =         "Font.png";
    asset_file_names.main_menu =         "Main_menu.jpg";
    asset_file_names.mouse_cursor =      "Mouse_cursor.png";
    asset_file_names.mouse_wd_icon =     "Mouse_wheel_down_icon.png";
    asset_file_names.mouse_wu_icon =     "Mouse_wheel_up_icon.png";
    asset_file_names.notification =      "Notification.png";
    asset_file_names.pikmin_silhouette = "Pikmin_silhouette.png";
    asset_file_names.pikmin_spirit =     "Pikmin_spirit.png";
    asset_file_names.rock =              "Rock.png";
    asset_file_names.shadow =            "Shadow.png";
    asset_file_names.smack =             "Smack.png";
    asset_file_names.smoke =             "Smoke.png";
    asset_file_names.sparkle =           "Sparkle.png";
    asset_file_names.spotlight =         "Spotlight.png";
    asset_file_names.value_font =        "Value_font.png";
    asset_file_names.wave_ring =         "Wave_ring.png";
    
    for(unsigned char i = 0; i < 3; ++i) {
        asset_file_names.mouse_button_icon[i] =
            "Mouse_button_" + i2s(i + 1) + "_icon.png";
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the default controls.
 */
void init_controls() {
    //Declare the existing buttons.
    buttons.add(
        BUTTON_NONE, "---", "", ""
    );
    buttons.add(
        BUTTON_THROW, "Throw", "throw", "mb_1"
    );
    buttons.add(
        BUTTON_WHISTLE, "Whistle", "whistle", "mb_2"
    );
    buttons.add(
        BUTTON_RIGHT, "Right", "move_right", "k_4"
    );
    buttons.add(
        BUTTON_UP, "Up", "move_up", "k_23"
    );
    buttons.add(
        BUTTON_LEFT, "Left", "move_left", "k_1"
    );
    buttons.add(
        BUTTON_DOWN, "Down", "move_down", "k_19"
    );
    buttons.add(
        BUTTON_CURSOR_RIGHT, "Cursor right", "cursor_right", ""
    );
    buttons.add(
        BUTTON_CURSOR_UP, "Cursor up", "cursor_up", ""
    );
    buttons.add(
        BUTTON_CURSOR_LEFT, "Cursor left", "cursor_left", ""
    );
    buttons.add(
        BUTTON_CURSOR_DOWN, "Cursor down", "cursor_down", ""
    );
    buttons.add(
        BUTTON_GROUP_RIGHT, "Group right", "group_right", ""
    );
    buttons.add(
        BUTTON_GROUP_UP, "Group up", "group_up", ""
    );
    buttons.add(
        BUTTON_GROUP_LEFT, "Group left", "group_left", ""
    );
    buttons.add(
        BUTTON_GROUP_DOWN, "Group down", "group_down", ""
    );
    buttons.add(
        BUTTON_GROUP_CURSOR, "Group to cursor", "group_cursor", "k_75"
    );
    buttons.add(
        BUTTON_NEXT_LEADER, "Next leader", "next_leader", "k_64"
    );
    buttons.add(
        BUTTON_PREV_LEADER, "Prev. leader", "prev_leader", ""
    );
    buttons.add(
        BUTTON_DISMISS, "Dismiss", "dismiss", "k_217"
    );
    buttons.add(
        BUTTON_USE_SPRAY_1, "Use spray 1", "use_spray_1", "k_18"
    );
    buttons.add(
        BUTTON_USE_SPRAY_2, "Use spray 2", "use_spray_2", "k_6"
    );
    buttons.add(
        BUTTON_USE_SPRAY, "Use spray", "use_spray", "k_18"
    );
    buttons.add(
        BUTTON_NEXT_SPRAY, "Next spray", "next_spray", "k_5"
    );
    buttons.add(
        BUTTON_PREV_SPRAY, "Prev. spray", "prev_spray", "k_17"
    );
    buttons.add(
        BUTTON_CHANGE_ZOOM, "Change zoom", "change_zoom", "k_3"
    );
    buttons.add(
        BUTTON_ZOOM_IN, "Zoom in", "zoom_in", ""
    );
    buttons.add(
        BUTTON_ZOOM_OUT, "Zoom out", "zoom_out", ""
    );
    buttons.add(
        BUTTON_NEXT_TYPE, "Next Pikmin", "next_type", "mwd"
    );
    buttons.add(
        BUTTON_PREV_TYPE, "Prev. Pikmin", "prev_type", "mwu"
    );
    buttons.add(
        BUTTON_NEXT_MATURITY, "Next maturity", "next_maturity", ""
    );
    buttons.add(
        BUTTON_PREV_MATURITY, "Prev. maturity", "prev_maturity", ""
    );
    buttons.add(
        BUTTON_LIE_DOWN, "Lie down", "lie_down", "k_26"
    );
    buttons.add(
        BUTTON_PAUSE, "Pause", "pause", "k_59"
    );
    buttons.add(
        BUTTON_MENU_RIGHT, "Menu right", "menu_right", "k_83"
    );
    buttons.add(
        BUTTON_MENU_UP, "Menu up", "menu_up", "k_84"
    );
    buttons.add(
        BUTTON_MENU_LEFT, "Menu left", "menu_left", "k_82"
    );
    buttons.add(
        BUTTON_MENU_DOWN, "Menu down", "menu_down", "k_85"
    );
    buttons.add(
        BUTTON_MENU_OK, "Menu OK", "menu_ok", "k_67"
    );
    buttons.add(
        BUTTON_MENU_BACK, "Menu back", "menu_back", "k_59"
    );
    
    controls.assign(MAX_PLAYERS, vector<control_info>());
    
    //Populate the controls information with some default controls for player 1.
    //If the options are loaded successfully, these controls are overwritten.
    for(size_t b = 0; b < N_BUTTONS; ++b) {
        string dc = buttons.list[b].default_control_str;
        if(dc.empty()) continue;
        
        controls[0].push_back(control_info(buttons.list[b].id, dc));
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the error bitmap.
 */
void init_error_bitmap() {
    //Error bitmap.
    bmp_error = al_create_bitmap(32, 32);
    al_set_target_bitmap(bmp_error); {
        al_clear_to_color(al_map_rgba(0, 0, 0, 192));
        al_draw_filled_rectangle(
            0.0, 0.0, 16.0, 16.0,
            al_map_rgba(255, 0, 255, 192)
        );
        al_draw_filled_rectangle(
            16.0, 16.0, 32.0, 32.0,
            al_map_rgba(255, 0, 255, 192)
        );
    } al_set_target_backbuffer(display);
    bmp_error = recreate_bitmap(bmp_error);
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
 */
void init_event_things(
    ALLEGRO_TIMER* &logic_timer, ALLEGRO_EVENT_QUEUE* &logic_queue
) {
    if(window_position_hack) al_set_new_window_position(64, 64);
    if(scr_fullscreen) {
        al_set_new_display_flags(
            al_get_new_display_flags() | ALLEGRO_FULLSCREEN
        );
    }
    display = al_create_display(scr_w, scr_h);
    
    //It's possible that this resolution is not valid for fullscreen.
    //Detect this and try again in windowed.
    if(!display && scr_fullscreen) {
        log_error(
            "Could not create a fullscreen window with the resolution " +
            i2s(scr_w) + "x" + i2s(scr_h) + ". Setting the fullscreen "
            "option back to false. You can try a different resolution, "
            "preferably one from the options menu."
        );
        scr_fullscreen = false;
        intended_scr_fullscreen = false;
        save_options();
        al_set_new_display_flags(
            al_get_new_display_flags() & ~ALLEGRO_FULLSCREEN
        );
        display = al_create_display(scr_w, scr_h);
    }
    
    if(!display) {
        report_fatal_error("Could not create a display!");
    }
    
    logic_timer = al_create_timer(1.0 / game_fps);
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
    al_register_event_source(logic_queue, al_get_display_event_source(display));
    al_register_event_source(
        logic_queue, al_get_timer_event_source(logic_timer)
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the game states.
 */
void init_game_states() {
    game_states[GAME_STATE_MAIN_MENU] = new main_menu();
    game_states[GAME_STATE_AREA_MENU] = new area_menu();
    game_states[GAME_STATE_GAME] = new gameplay();
    game_states[GAME_STATE_OPTIONS_MENU] = new options_menu();
    game_states[GAME_STATE_CONTROLS_MENU] = new controls_menu();
    game_states[GAME_STATE_AREA_EDITOR] = new area_editor_imgui();
    game_states[GAME_STATE_ANIMATION_EDITOR] = new animation_editor();
}


/* ----------------------------------------------------------------------------
 * Initializes the HUD items, along with their default coordinates.
 */
void init_hud_items() {
    hud_items.set_item(HUD_ITEM_TIME,                  40, 10, 70, 10);
    hud_items.set_item(HUD_ITEM_DAY_BUBBLE,            88, 18, 15, 25);
    hud_items.set_item(HUD_ITEM_DAY_NUMBER,            88, 20, 10, 10);
    hud_items.set_item(HUD_ITEM_LEADER_1_ICON,         7,  90, 8,  10);
    hud_items.set_item(HUD_ITEM_LEADER_2_ICON,         6,  80, 5,  9 );
    hud_items.set_item(HUD_ITEM_LEADER_3_ICON,         6,  72, 5,  9 );
    hud_items.set_item(HUD_ITEM_LEADER_1_HEALTH,       16, 90, 8,  10);
    hud_items.set_item(HUD_ITEM_LEADER_2_HEALTH,       12, 80, 5,  9 );
    hud_items.set_item(HUD_ITEM_LEADER_3_HEALTH,       12, 72, 5,  9 );
    hud_items.set_item(HUD_ITEM_PIKMIN_STANDBY_ICON,   30, 91, 8,  10);
    hud_items.set_item(HUD_ITEM_PIKMIN_STANDBY_M_ICON, 35, 88, 4,  8 );
    hud_items.set_item(HUD_ITEM_PIKMIN_STANDBY_NR,     38, 91, 7,  8 );
    hud_items.set_item(HUD_ITEM_PIKMIN_STANDBY_X,      50, 91, 15, 10);
    hud_items.set_item(HUD_ITEM_PIKMIN_GROUP_NR,       73, 91, 15, 14);
    hud_items.set_item(HUD_ITEM_PIKMIN_FIELD_NR,       91, 91, 15, 14);
    hud_items.set_item(HUD_ITEM_PIKMIN_TOTAL_NR,       0,  0,  0,  0 );
    hud_items.set_item(HUD_ITEM_PIKMIN_SLASH_1,        82, 91, 4,  8 );
    hud_items.set_item(HUD_ITEM_PIKMIN_SLASH_2,        0,  0,  0,  0 );
    hud_items.set_item(HUD_ITEM_PIKMIN_SLASH_3,        0,  0,  0,  0 );
    hud_items.set_item(HUD_ITEM_SPRAY_1_ICON,          6,  36, 4,  7 );
    hud_items.set_item(HUD_ITEM_SPRAY_1_AMOUNT,        13, 37, 10, 5 );
    hud_items.set_item(HUD_ITEM_SPRAY_1_BUTTON,        10, 42, 10, 5 );
    hud_items.set_item(HUD_ITEM_SPRAY_2_ICON,          6,  52, 4,  7 );
    hud_items.set_item(HUD_ITEM_SPRAY_2_AMOUNT,        13, 53, 10, 5 );
    hud_items.set_item(HUD_ITEM_SPRAY_2_BUTTON,        10, 47, 10, 5 );
    hud_items.set_item(HUD_ITEM_SPRAY_PREV_ICON,       6,  52, 3,  5 );
    hud_items.set_item(HUD_ITEM_SPRAY_PREV_BUTTON,     6,  47, 4,  4 );
    hud_items.set_item(HUD_ITEM_SPRAY_NEXT_ICON,       13, 52, 3,  5 );
    hud_items.set_item(HUD_ITEM_SPRAY_NEXT_BUTTON,     13, 47, 4,  4 );
}


/* ----------------------------------------------------------------------------
 * Initializes miscellaneous things and settings.
 */
void init_misc() {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(display, "Pikifen");
    int new_bitmap_flags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(smooth_scaling) {
        new_bitmap_flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
    }
    if(mipmaps_enabled) {
        new_bitmap_flags |= ALLEGRO_MIPMAP;
    }
    al_set_new_bitmap_flags(new_bitmap_flags);
    al_reserve_samples(16);
    
    al_identity_transform(&identity_transform);
    
    srand(time(NULL));
    
    cursor_save_timer.on_end = [] () {
        cursor_save_timer.start();
        cursor_spots.push_back(mouse_cursor_s);
        if(cursor_spots.size() > CURSOR_SAVE_N_SPOTS) {
            cursor_spots.erase(cursor_spots.begin());
        }
    };
    cursor_save_timer.start();
    
    framerate_update_timer.on_end = [] () {
        framerate_update_timer.start();
        framerate_counter = round(1.0 / delta_t);
    };
    framerate_update_timer.start();
    
    group_move_next_arrow_timer.on_end = [] () {
        group_move_next_arrow_timer.start();
        group_move_arrows.push_back(0);
    };
    group_move_next_arrow_timer.start();
    
    whistle_next_dot_timer.on_end = [] () {
        whistle_next_dot_timer.start();
        unsigned char dot = 255;
        for(unsigned char d = 0; d < 6; ++d) { //Find WHAT dot to add.
            if(whistle_dot_radius[d] == -1) { dot = d; break;}
        }
        
        if(dot != 255) whistle_dot_radius[dot] = 0;
    };
    whistle_next_dot_timer.start();
    
    whistle_next_ring_timer.on_end = [] () {
        whistle_next_ring_timer.start();
        whistle_rings.push_back(0);
        whistle_ring_colors.push_back(whistle_ring_prev_color);
        whistle_ring_prev_color =
            sum_and_wrap(whistle_ring_prev_color, 1, N_WHISTLE_RING_COLORS);
    };
    whistle_next_ring_timer.start();
    
    particles = particle_manager(max_particles);
    
    zoom_mid_level = clamp(zoom_mid_level, zoom_min_level, zoom_max_level);
    
    //Some creator tool defaults that are convenient to have on.
    creator_tool_keys[10] = CREATOR_TOOL_AREA_IMAGE;
    creator_tool_keys[11] = CREATOR_TOOL_CHANGE_SPEED;
    creator_tool_keys[12] = CREATOR_TOOL_TELEPORT;
    creator_tool_keys[13] = CREATOR_TOOL_HURT_MOB;
    creator_tool_keys[14] = CREATOR_TOOL_NEW_PIKMIN;
    creator_tool_keys[15] = CREATOR_TOOL_MOB_INFO;
    creator_tool_keys[16] = CREATOR_TOOL_GEOMETRY_INFO;
    creator_tool_keys[17] = CREATOR_TOOL_HITBOXES;
}


/* ----------------------------------------------------------------------------
 * Initializes the list of mob categories.
 */
void init_mob_categories() {

    mob_categories.register_category(
        MOB_CATEGORY_NONE, new none_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_PIKMIN, new pikmin_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_ONIONS, new onion_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_LEADERS, new leader_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_ENEMIES, new enemy_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_TREASURES, new treasure_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_PELLETS, new pellet_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_CONVERTERS, new converter_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_DROPS, new drop_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_RESOURCES, new resource_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_PILES, new pile_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_TOOLS, new tool_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_SHIPS, new ship_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_GATES, new gate_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_BRIDGES, new bridge_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_DECORATIONS, new decoration_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_INTERACTABLES, new interactable_category()
    );
    mob_categories.register_category(
        MOB_CATEGORY_CUSTOM, new custom_category()
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the list of sector types.
 */
void init_sector_types() {
    sector_types.register_type(SECTOR_TYPE_NORMAL, "Normal");
    sector_types.register_type(SECTOR_TYPE_BLOCKING, "Blocking");
    sector_types.register_type(SECTOR_TYPE_BRIDGE, "Bridge");
    sector_types.register_type(SECTOR_TYPE_BRIDGE_RAIL, "Bridge rail");
}


/* ----------------------------------------------------------------------------
 * Loads a single animation from the system animations definition file.
 * anim_def_file: The animation definition file.
 * name:          Name of the animation on this file.
 * anim:          The single animation suite structure to fill.
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
 */
void destroy_event_things(
    ALLEGRO_TIMER* &logic_timer, ALLEGRO_EVENT_QUEUE* &logic_queue
) {
    al_destroy_event_queue(logic_queue);
    al_destroy_timer(logic_timer);
    al_destroy_display(display);
}


/* ----------------------------------------------------------------------------
 * Destroys the list of game states.
 */
void destroy_game_states() {
    for(size_t s = 0; s < N_GAME_STATES; s++) {
        delete game_states[s];
    }
}


/* ----------------------------------------------------------------------------
 * Destroys miscellaneous things.
 */
void destroy_misc() {
    al_destroy_bitmap(bmp_error);
    al_destroy_font(font_area_name);
    al_destroy_font(font_counter);
    al_destroy_font(font_main);
    al_destroy_font(font_value);
    
    al_detach_voice(voice);
    al_destroy_mixer(mixer);
    al_destroy_voice(voice);
}


/* ----------------------------------------------------------------------------
 * Destroys registered mob categories.
 */
void destroy_mob_categories() {
    mob_categories.clear();
}
