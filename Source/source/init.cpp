/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program initializer and deinitializer functions.
 */

#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "init.h"

#include "editors/area_editor.h"
#include "editors/animation_editor.h"
#include "controls.h"
#include "data_file.h"
#include "functions.h"
#include "game_state.h"
#include "menus.h"
#include "mob_script.h"
#include "vars.h"


/* ----------------------------------------------------------------------------
 * Initializes Allegro and its modules.
 */
void init_allegro() {
    al_init();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
    al_install_joystick();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_acodec_addon();
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
        BUTTON_RIGHT, "Right", "right", "k_4"
    );
    buttons.add(
        BUTTON_UP, "Up", "up", "k_23"
    );
    buttons.add(
        BUTTON_LEFT, "Left", "left", "k_1"
    );
    buttons.add(
        BUTTON_DOWN, "Down", "down", "k_19"
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
        BUTTON_GROUP_CURSOR, "Group cursor", "group_cursor", "k_75"
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
    int bmp_error_w, bmp_error_h;
    al_get_text_dimensions(
        al_create_builtin_font(), "ERROR", NULL, NULL,
        &bmp_error_w, &bmp_error_h
    );
    bmp_error = al_create_bitmap(bmp_error_w, bmp_error_h);
    al_set_target_bitmap(bmp_error); {
        al_clear_to_color(al_map_rgba(64, 0, 0, 128));
        al_draw_text(
            al_create_builtin_font(), al_map_rgb(255, 0, 0), 0, 0, 0, "ERROR"
        );
    } al_set_target_backbuffer(display);
    bmp_error = recreate_bitmap(bmp_error);
}


/* ----------------------------------------------------------------------------
 * Initializes things regarding Allegro events, like the queue, timer, etc.
 */
void init_event_things(
    ALLEGRO_TIMER* &logic_timer, ALLEGRO_EVENT_QUEUE* &logic_queue
) {
    if(window_position_hack) al_set_new_window_position(64, 64);
    display = al_create_display(scr_w, scr_h);
    logic_timer = al_create_timer(1.0 / game_fps);
    
    logic_queue = al_create_event_queue();
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
    game_states[GAME_STATE_AREA_EDITOR] = new area_editor();
    game_states[GAME_STATE_ANIMATION_EDITOR] = new animation_editor();
}


/* ----------------------------------------------------------------------------
 * Helper function to initialize a HUD coordinate.
 */
void init_hud_coordinate(
    const int n, const float x, const float y, const float w, const float h
) {
    hud_coords[n][0] = x / 100; hud_coords[n][1] = y / 100;
    hud_coords[n][2] = w / 100; hud_coords[n][3] = h / 100;
}


/* ----------------------------------------------------------------------------
 * Initializes the default HUD coordinates.
 */
void init_hud_coordinates() {
    init_hud_coordinate(HUD_ITEM_TIME,                  40, 10, 70, 10);
    init_hud_coordinate(HUD_ITEM_DAY_BUBBLE,            88, 18, 15, 0 );
    init_hud_coordinate(HUD_ITEM_DAY_NUMBER,            88, 20, 10, 10);
    init_hud_coordinate(HUD_ITEM_LEADER_1_ICON,         7,  90, 8,  0 );
    init_hud_coordinate(HUD_ITEM_LEADER_2_ICON,         6,  80, 5,  0 );
    init_hud_coordinate(HUD_ITEM_LEADER_3_ICON,         6,  72, 5,  0 );
    init_hud_coordinate(HUD_ITEM_LEADER_1_HEALTH,       16, 90, 8,  0 );
    init_hud_coordinate(HUD_ITEM_LEADER_2_HEALTH,       12, 80, 5,  0 );
    init_hud_coordinate(HUD_ITEM_LEADER_3_HEALTH,       12, 72, 5,  0 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_ICON,   30, 89, 8,  0 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_M_ICON, 35, 86, 4,  0 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_NR,     38, 91, 7,  8 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_X,      50, 91, 15, 10);
    init_hud_coordinate(HUD_ITEM_PIKMIN_GROUP_NR,       73, 91, 15, 14);
    init_hud_coordinate(HUD_ITEM_PIKMIN_FIELD_NR,       91, 91, 15, 14);
    init_hud_coordinate(HUD_ITEM_PIKMIN_TOTAL_NR,       0,  0,  0,  0 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_1,        82, 91, 4,  8 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_2,        0,  0,  0,  0 );
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_3,        0,  0,  0,  0 );
    init_hud_coordinate(HUD_ITEM_SPRAY_1_ICON,          6,  36, 4,  7 );
    init_hud_coordinate(HUD_ITEM_SPRAY_1_AMOUNT,        10, 37, 9,  5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_1_KEY,           10, 42, 10, 5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_2_ICON,          6,  52, 4,  7 );
    init_hud_coordinate(HUD_ITEM_SPRAY_2_AMOUNT,        10, 53, 9,  5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_2_KEY,           10, 47, 10, 5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_PREV_ICON,       6,  52, 3,  5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_PREV_KEY,        6,  47, 4,  4 );
    init_hud_coordinate(HUD_ITEM_SPRAY_NEXT_ICON,       13, 52, 3,  5 );
    init_hud_coordinate(HUD_ITEM_SPRAY_NEXT_KEY,        13, 47, 4,  4 );
}


/* ----------------------------------------------------------------------------
 * Initializes miscellaneous things and settings.
 */
void init_misc() {
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(display, "Pikmin fangame engine");
    int new_bitmap_flags = ALLEGRO_NO_PREMULTIPLIED_ALPHA;
    if(smooth_scaling) {
        new_bitmap_flags |=
            ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR | ALLEGRO_MIPMAP;
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
    
    zoom_mid_level = max(zoom_min_level, zoom_mid_level);
    zoom_mid_level = min(zoom_mid_level, zoom_max_level);
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
        MOB_CATEGORY_SPECIAL, new special_category()
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
        MOB_CATEGORY_CUSTOM, new custom_category()
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the list of sector types.
 */
void init_sector_types() {
    sector_types.register_type(SECTOR_TYPE_NORMAL, "Normal");
    sector_types.register_type(SECTOR_TYPE_BOTTOMLESS_PIT, "Bottomless pit");
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
 * Initializes the special mob types.
 */
void init_special_mob_types() {
    mob_category* cat = mob_categories.get(MOB_CATEGORY_SPECIAL);
    
    //Info spot.
    mob_type* info_spot_mt = new mob_type(MOB_CATEGORY_SPECIAL);
    info_spot_mt->name = "Info spot";
    info_spot_mt->radius = 16;
    info_spot_mt->create_mob =
    [] (const point pos, const float angle, const string & vars) -> mob* {
        info_spot* m = new info_spot(pos, angle, vars);
        info_spots.push_back(m);
        return m;
    };
    cat->register_type(info_spot_mt);
    
    //Nectar.
    mob_type* nectar_mt = new mob_type(MOB_CATEGORY_SPECIAL);
    nectar_mt->name = "Nectar";
    nectar_mt->always_active = true;
    nectar_mt->radius = 8;
    nectar_mt->create_mob =
    [] (const point pos, const float angle, const string & vars) -> mob* {
        nectar* m = new nectar(pos, vars);
        nectars.push_back(m);
        return m;
    };
    cat->register_type(nectar_mt);
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
        //TODO create the missing destructors for each state type.
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
 * Destroys the list of mob types.
 */
void destroy_special_mob_types() {
    for(auto t = spec_mob_types.begin(); t != spec_mob_types.end(); ++t) {
        delete t->second;
    }
    
    spec_mob_types.clear();
}
