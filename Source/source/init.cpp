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

#include "area_editor.h"
#include "animation_editor.h"
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
        BUTTON_ZOOM_IN, "Zoom in", "zoom_in", "mwu"
    );
    buttons.add(
        BUTTON_ZOOM_OUT, "Zoom out", "zoom_out", "mwd"
    );
    buttons.add(
        BUTTON_NEXT_TYPE, "Next Pikmin", "next_type", "mb_2"
    );
    buttons.add(
        BUTTON_PREV_TYPE, "Prev. Pikmin", "prev_type", ""
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
    
    controls.assign(4, vector<control_info>());
    
    //Populate the controls information with some default controls for player 1.
    //If the options are loaded successfully, these controls are overwritten.
    for(size_t b = 0; b < N_BUTTONS; ++b) {
        string dc = buttons.list[b].default_control_str;
        if(dc.empty()) continue;
        
        controls[0].push_back(control_info(buttons.list[b].id, dc));
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the development tools from the Tools.txt file.
 */
void init_dev_tools() {
    data_node file(MISC_FOLDER + "/Tools.txt");
    
    if(!s2b(file.get_child_by_name("enabled")->value)) return;
    
    for(unsigned char k = 0; k < 10; k++) {
        string tool_name = file.get_child_by_name("f" + i2s(k + 2))->value;
        if(tool_name == "area_image") {
            dev_tool_keys[k] = DEV_TOOL_AREA_IMAGE;
        } else if(tool_name == "coordinates") {
            dev_tool_keys[k] = DEV_TOOL_COORDINATES;
        } else if(tool_name == "hurt_mob") {
            dev_tool_keys[k] = DEV_TOOL_HURT_MOB;
        } else if(tool_name == "mob_info") {
            dev_tool_keys[k] = DEV_TOOL_MOB_INFO;
        } else if(tool_name == "new_pikmin") {
            dev_tool_keys[k] = DEV_TOOL_NEW_PIKMIN;
        } else if(tool_name == "change_speed") {
            dev_tool_keys[k] = DEV_TOOL_CHANGE_SPEED;
        } else if(tool_name == "teleport") {
            dev_tool_keys[k] = DEV_TOOL_TELEPORT;
        } else {
            dev_tool_keys[k] = DEV_TOOL_NONE;
        }
    }
    
    dev_tool_area_image_size =
        s2i(file.get_child_by_name("area_image_size")->value);
    dev_tool_area_image_name =
        file.get_child_by_name("area_image_file_name")->value;
    dev_tool_area_image_shadows =
        s2b(file.get_child_by_name("area_image_shadows")->value);
    dev_tool_change_speed_mult =
        s2f(file.get_child_by_name("change_speed_multiplier")->value);
        
    dev_tool_auto_start_option =
        file.get_child_by_name("auto_start_option")->value;
    dev_tool_auto_start_mode =
        file.get_child_by_name("auto_start_mode")->value;
        
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
 * Initializes and loads the game's fonts.
 */
void init_fonts() {
    int font_ranges[] = {
        0x0020, 0x007E, //ASCII
        /*0x00A0, 0x00A1, //Non-breaking space and inverted !
        0x00BF, 0x00FF, //Inverted ? and European vowels and such*/
    };
    int counter_font_ranges[] = {
        0x002D, 0x002D, //Dash
        0x002F, 0x0039, //Slash and numbers
        0x0078, 0x0078, //x
    };
    int value_font_ranges[] = {
        0x0024, 0x0024, //Dollar sign
        0x002D, 0x002D, //Dash
        0x0030, 0x0039, //Numbers
    };
    
    //We can't load the font directly because we want to set the ranges.
    //So we load into a bitmap first.
    ALLEGRO_BITMAP* temp_font_bitmap = load_bmp("Font.png");
    if(temp_font_bitmap) {
        font_main = al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
    }
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Area_name_font.png");
    if(temp_font_bitmap) {
        font_area_name =
            al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
    }
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Counter_font.png");
    if(temp_font_bitmap) {
        font_counter =
            al_grab_font_from_bitmap(temp_font_bitmap, 3, counter_font_ranges);
    }
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Value_font.png");
    if(temp_font_bitmap) {
        font_value =
            al_grab_font_from_bitmap(temp_font_bitmap, 3, value_font_ranges);
    }
    al_destroy_bitmap(temp_font_bitmap);
    
    if(font_main) font_main_h = al_get_font_line_height(font_main);
    if(font_counter) font_counter_h = al_get_font_line_height(font_counter);
    
    allegro_font = al_create_builtin_font();
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
    hud_coords[n][0] = x; hud_coords[n][1] = y;
    hud_coords[n][2] = w; hud_coords[n][3] = h;
}


/* ----------------------------------------------------------------------------
 * Initializes the default HUD coordinates.
 */
void init_hud_coordinates() {
    init_hud_coordinate(HUD_ITEM_TIME,                  0.40, 0.10, 0.70, 0.10);
    init_hud_coordinate(HUD_ITEM_DAY_BUBBLE,            0.88, 0.18, 0.15, 0.00);
    init_hud_coordinate(HUD_ITEM_DAY_NUMBER,            0.88, 0.20, 0.10, 0.10);
    init_hud_coordinate(HUD_ITEM_LEADER_1_ICON,         0.07, 0.90, 0.08, 0.00);
    init_hud_coordinate(HUD_ITEM_LEADER_2_ICON,         0.06, 0.80, 0.05, 0.00);
    init_hud_coordinate(HUD_ITEM_LEADER_3_ICON,         0.06, 0.72, 0.05, 0.00);
    init_hud_coordinate(HUD_ITEM_LEADER_1_HEALTH,       0.16, 0.90, 0.08, 0.00);
    init_hud_coordinate(HUD_ITEM_LEADER_2_HEALTH,       0.12, 0.80, 0.05, 0.00);
    init_hud_coordinate(HUD_ITEM_LEADER_3_HEALTH,       0.12, 0.72, 0.05, 0.00);
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_ICON,   0.30, 0.89, 0.08, 0.00);
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_M_ICON, 0.35, 0.86, 0.04, 0.00);
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_NR,     0.38, 0.91, 0.07, 0.08);
    init_hud_coordinate(HUD_ITEM_PIKMIN_STANDBY_X,      2.00, 0.00, 0.01, 0.01);
    init_hud_coordinate(HUD_ITEM_PIKMIN_GROUP_NR,       0.50, 0.90, 0.16, 0.10);
    init_hud_coordinate(HUD_ITEM_PIKMIN_FIELD_NR,       0.68, 0.91, 0.14, 0.08);
    init_hud_coordinate(HUD_ITEM_PIKMIN_TOTAL_NR,       0.87, 0.91, 0.19, 0.08);
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_1,        0.59, 0.92, 0.04, 0.08);
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_2,        0.76, 0.92, 0.04, 0.08);
    init_hud_coordinate(HUD_ITEM_PIKMIN_SLASH_3,        2.00, 0.00, 0.01, 0.01);
    init_hud_coordinate(HUD_ITEM_SPRAY_1_ICON,          0.06, 0.36, 0.04, 0.07);
    init_hud_coordinate(HUD_ITEM_SPRAY_1_AMOUNT,        0.10, 0.37, 0.09, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_1_KEY,           0.10, 0.42, 0.10, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_2_ICON,          0.06, 0.52, 0.04, 0.07);
    init_hud_coordinate(HUD_ITEM_SPRAY_2_AMOUNT,        0.10, 0.53, 0.09, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_2_KEY,           0.10, 0.47, 0.10, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_PREV_ICON,       0.06, 0.52, 0.03, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_PREV_KEY,        0.06, 0.47, 0.04, 0.04);
    init_hud_coordinate(HUD_ITEM_SPRAY_NEXT_ICON,       0.13, 0.52, 0.03, 0.05);
    init_hud_coordinate(HUD_ITEM_SPRAY_NEXT_KEY,        0.13, 0.47, 0.04, 0.04);
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
    
    srand(time(NULL));
    
    //TODO the function is always returning 0.
    // = al_get_new_display_option(ALLEGRO_MAX_BITMAP_SIZE, NULL);
    area_image_size = 800;
    
    cursor_save_timer.on_end = [] () {
        cursor_save_timer.start();
        cursor_spots.push_back(point(mouse_cursor_x, mouse_cursor_y));
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
            (whistle_ring_prev_color + 1) % N_WHISTLE_RING_COLORS;
    };
    whistle_next_ring_timer.start();
    
    particles = particle_manager(max_particles);
    
    zoom_mid_level = max(zoom_min_level, zoom_mid_level);
    zoom_mid_level = min(zoom_mid_level, zoom_max_level);
}


/* ----------------------------------------------------------------------------
 * Initializes miscellaneous fixed graphics.
 */
void init_misc_graphics() {
    //Icon.
    bmp_icon = load_bmp("Icon.png");
    al_set_display_icon(display, bmp_icon);
    
    //Graphics.
    bmp_bubble = load_bmp(          "Bubble.png");
    bmp_checkbox_check = load_bmp(  "Checkbox_check.png");
    bmp_cursor = load_bmp(          "Cursor.png");
    bmp_day_bubble = load_bmp(      "Day_bubble.png");
    bmp_enemy_spirit = load_bmp(    "Enemy_spirit.png");
    bmp_hard_bubble = load_bmp(     "Hard_bubble.png");
    bmp_idle_glow = load_bmp(       "Idle_glow.png");
    bmp_info_spot = load_bmp(       "Info_spot.png");
    bmp_message_box = load_bmp(     "Message_box.png");
    bmp_mouse_cursor = load_bmp(    "Mouse_cursor.png");
    bmp_mouse_wd_icon = load_bmp(   "Mouse_wheel_down_icon.png");
    bmp_mouse_wu_icon = load_bmp(   "Mouse_wheel_up_icon.png");
    bmp_notification = load_bmp(    "Notification.png");
    bmp_group_move_arrow = load_bmp("Group_move_arrow.png");
    bmp_nectar = load_bmp(          "Nectar.png");
    bmp_no_pikmin = load_bmp(       "No_Pikmin.png");
    bmp_number_bubble = load_bmp(   "Number_bubble.png");
    bmp_pikmin_spirit = load_bmp(   "Pikmin_spirit.png");
    bmp_shadow = load_bmp(          "Shadow.png");
    bmp_smack = load_bmp(           "Smack.png");
    bmp_smoke = load_bmp(           "Smoke.png");
    bmp_sparkle = load_bmp(         "Sparkle.png");
    bmp_sun = load_bmp(             "Sun.png");
    bmp_ub_spray = load_bmp(        "Ultra-bitter_spray.png");
    bmp_us_spray = load_bmp(        "Ultra-spicy_spray.png");
    bmp_wave_ring = load_bmp(       "Wave_ring.png");
    
    for(unsigned char i = 0; i < 3; ++i) {
        bmp_mouse_button_icon[i] =
            load_bmp(
                "Mouse_button_" + i2s(i + 1) + "_icon.png"
            );
    }
    
    data_node system_animations_file(SYSTEM_ANIMATIONS_FILE);
    
    init_single_animation(
        &system_animations_file, "leader_damage_sparks", spark_animation
    );
}


/* ----------------------------------------------------------------------------
 * Initializes miscellaneous fixed sound effects.
 */
void init_misc_sounds() {
    //Sound effects.
    voice =
        al_create_voice(
            44100, ALLEGRO_AUDIO_DEPTH_INT16,   ALLEGRO_CHANNEL_CONF_2
        );
    mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_voice(mixer, voice);
    sfx_attack = load_sample(              "Attack.ogg",               mixer);
    sfx_pikmin_attack = load_sample(       "Pikmin_attack.ogg",        mixer);
    sfx_pikmin_carrying = load_sample(     "Pikmin_carrying.ogg",      mixer);
    sfx_pikmin_carrying_grab = load_sample("Pikmin_carrying_grab.ogg", mixer);
    sfx_pikmin_caught = load_sample(       "Pikmin_caught.ogg",        mixer);
    sfx_pikmin_dying = load_sample(        "Pikmin_dying.ogg",         mixer);
    sfx_pikmin_held = load_sample(         "Pikmin_held.ogg",          mixer);
    sfx_pikmin_idle = load_sample(         "Pikmin_idle.ogg",          mixer);
    sfx_pikmin_thrown = load_sample(       "Pikmin_thrown.ogg",        mixer);
    sfx_pikmin_plucked = load_sample(      "Pikmin_plucked.ogg",       mixer);
    sfx_pikmin_called = load_sample(       "Pikmin_called.ogg",        mixer);
    sfx_olimar_whistle = load_sample(      "Olimar_whistle.ogg",       mixer);
    sfx_louie_whistle = load_sample(       "Louie_whistle.ogg",        mixer);
    sfx_president_whistle = load_sample(   "President_whistle.ogg",    mixer);
    sfx_olimar_name_call = load_sample(    "Olimar_name_call.ogg",     mixer);
    sfx_louie_name_call = load_sample(     "Louie_name_call.ogg",      mixer);
    sfx_president_name_call = load_sample( "President_name_call.ogg",  mixer);
    sfx_pluck = load_sample(               "Pluck.ogg",                mixer);
    sfx_throw = load_sample(               "Throw.ogg",                mixer);
    sfx_switch_pikmin = load_sample(       "Switch_Pikmin.ogg",        mixer);
    sfx_camera = load_sample(              "Camera.ogg",               mixer);
}


/* ----------------------------------------------------------------------------
 * Initializes the list of mob categories.
 */
void init_mob_categories() {

    mob_categories.register_category(
        MOB_CATEGORY_NONE, "None", "None", "",
        al_map_rgb(255, 0, 0),
    [] (vector<string> &) { },
    [] (const string &) -> mob_type* { return nullptr; },
    [] () -> mob_type* { return nullptr; },
    [] (mob_type * mt) {}
    );
    
    mob_categories.register_category(
        MOB_CATEGORY_ENEMIES, "Enemies", "Enemy", ENEMIES_FOLDER,
        al_map_rgb(224, 96, 128),
    [] (vector<string> &li) {
        for(auto e = enemy_types.begin(); e != enemy_types.end(); ++e) {
            li.push_back(e->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = enemy_types.find(n);
        if(it == enemy_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new enemy_type();
    }, [] (mob_type * et) {
        enemy_types[et->name] = (enemy_type*) et;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_LEADERS, "Leaders", "Leader", LEADERS_FOLDER,
        al_map_rgb(48, 80, 192),
    [] (vector<string> &li) {
        for(auto l = leader_types.begin(); l != leader_types.end(); ++l) {
            li.push_back(l->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = leader_types.find(n);
        if(it == leader_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new leader_type();
    }, [] (mob_type * lt) {
        leader_types[lt->name] = (leader_type*) lt;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_MISC, "Misc.", "Misc.", MISC_MOB_FOLDER,
        al_map_rgb(224, 128, 224),
    [] (vector<string> &li) {
        for(auto m = misc_mob_types.begin(); m != misc_mob_types.end(); ++m) {
            li.push_back(m->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = misc_mob_types.find(n);
        if(it == misc_mob_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new mob_type();
    }, [] (mob_type * mt) {
        misc_mob_types[mt->name] = mt;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_ONIONS, "Onions", "Onion", ONIONS_FOLDER,
        al_map_rgb(48, 160, 48),
    [] (vector<string> &li) {
        for(auto o = onion_types.begin(); o != onion_types.end(); ++o) {
            li.push_back(o->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = onion_types.find(n);
        if(it == onion_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new onion_type();
    }, [] (mob_type * ot) {
        onion_types[ot->name] = (onion_type*) ot;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_PELLETS, "Pellets", "Pellet", PELLETS_FOLDER,
        al_map_rgb(208, 224, 96),
    [] (vector<string> &li) {
        for(auto p = pellet_types.begin(); p != pellet_types.end(); ++p) {
            li.push_back(p->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = pellet_types.find(n);
        if(it == pellet_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new pellet_type();
    }, [] (mob_type * pt) {
        pellet_types[pt->name] = (pellet_type*) pt;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_PIKMIN, "Pikmin", "Pikmin", PIKMIN_FOLDER,
        al_map_rgb(64, 255, 64),
    [] (vector<string> &li) {
        for(auto p = pikmin_types.begin(); p != pikmin_types.end(); ++p) {
            li.push_back(p->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = pikmin_types.find(n);
        if(it == pikmin_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new pikmin_type();
    }, [] (mob_type * pt) -> void {
        pikmin_types[pt->name] = (pikmin_type*) pt;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_SHIPS, "Ships", "Ship", SHIPS_FOLDER,
        al_map_rgb(128, 128, 192),
    [] (vector<string> &li) {
        for(auto s = ship_types.begin(); s != ship_types.end(); ++s) {
            li.push_back(s->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = ship_types.find(n);
        if(it == ship_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new ship_type();
    }, [] (mob_type * st) {
        ship_types[st->name] = (ship_type*) st;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_SPECIAL, "Special", "Special", "",
        al_map_rgb(32, 160, 160),
    [] (vector<string> &li) {
        for(auto s = spec_mob_types.begin(); s != spec_mob_types.end(); ++s) {
            li.push_back(s->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = spec_mob_types.find(n);
        if(it == spec_mob_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new mob_type();
    }, [] (mob_type * mt) {
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_TREASURES, "Treasures", "Treasure", TREASURES_FOLDER,
        al_map_rgb(255, 240, 64),
    [] (vector<string> &li) {
        for(auto t = treasure_types.begin(); t != treasure_types.end(); ++t) {
            li.push_back(t->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = treasure_types.find(n);
        if(it == treasure_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new treasure_type();
    }, [] (mob_type * tt) {
        treasure_types[tt->name] = (treasure_type*) tt;
    });
    
    mob_categories.register_category(
        MOB_CATEGORY_GATES, "Gates", "Gate", GATES_FOLDER,
        al_map_rgb(224, 192, 192),
    [] (vector<string> &li) {
        for(auto g = gate_types.begin(); g != gate_types.end(); ++g) {
            li.push_back(g->first);
        }
    }, [] (const string & n) -> mob_type* {
        auto it = gate_types.find(n);
        if(it == gate_types.end()) return NULL;
        return it->second;
    }, [] () -> mob_type* {
        return new gate_type();
    }, [] (mob_type * gt) {
        gate_types[gt->name] = (gate_type*) gt;
    });
}


/* ----------------------------------------------------------------------------
 * Initializes the list of sector types.
 */
void init_sector_types() {
    sector_types.register_type(SECTOR_TYPE_NORMAL, "Normal");
    sector_types.register_type(SECTOR_TYPE_BOTTOMLESS_PIT, "Bottomless pit");
    sector_types.register_type(SECTOR_TYPE_BLOCKING, "Blocking");
    sector_types.register_type(SECTOR_TYPE_GATE, "Gate");
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
        ANIMATIONS_FOLDER + "/" + anim_def_file->get_child_by_name(name)->value
    );
    anim.database = load_animation_database_from_file(&file);
    anim.instance.cur_anim = anim.database.animations[0];
    anim.instance.start();
}


/* ----------------------------------------------------------------------------
 * Initializes the special mob types.
 */
void init_special_mob_types() {
    //Info spot.
    mob_type* info_spot_mt = new mob_type();
    info_spot_mt->name = "Info spot";
    info_spot_mt->radius = 16;
    info_spot_mt->create_mob =
    [] (float x, float y, float angle, const string & vars) {
        create_mob(new info_spot(x, y, angle, vars));
    };
    spec_mob_types["Info spot"] = info_spot_mt;
    
    //Nectar.
    mob_type* nectar_mt = new mob_type();
    nectar_mt->name = "Nectar";
    nectar_mt->always_active = true;
    nectar_mt->radius = 8;
    nectar_mt->create_mob =
    [] (float x, float y, float angle, const string & vars) {
        create_mob(new nectar(x, y, vars));
    };
    spec_mob_types["Nectar"] = nectar_mt;
    
    //Bridge.
    mob_type* bridge_mt = new mob_type();
    bridge_mt->name = "Bridge";
    bridge_mt->is_obstacle = true;
    init_bridge_mob_type(bridge_mt);
    spec_mob_types["Bridge"] = bridge_mt;
    
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
 * Destroys the list of mob types.
 */
void destroy_special_mob_types() {
    for(auto t = spec_mob_types.begin(); t != spec_mob_types.end(); ++t) {
        delete t->second;
    }
    
    spec_mob_types.clear();
}


/* ----------------------------------------------------------------------------
 * Destroys miscellaneous graphics, sounds, and other resources.
 */
void destroy_resources() {
    al_destroy_bitmap(bmp_error);
    al_destroy_font(font_area_name);
    al_destroy_font(font_counter);
    al_destroy_font(font_main);
    al_destroy_font(font_value);
    
    al_destroy_bitmap(bmp_bubble);
    al_destroy_bitmap(bmp_checkbox_check);
    al_destroy_bitmap(bmp_cursor);
    al_destroy_bitmap(bmp_day_bubble);
    al_destroy_bitmap(bmp_enemy_spirit);
    al_destroy_bitmap(bmp_hard_bubble);
    al_destroy_bitmap(bmp_icon);
    al_destroy_bitmap(bmp_idle_glow);
    al_destroy_bitmap(bmp_info_spot);
    al_destroy_bitmap(bmp_message_box);
    al_destroy_bitmap(bmp_mouse_cursor);
    al_destroy_bitmap(bmp_mouse_wd_icon);
    al_destroy_bitmap(bmp_mouse_wu_icon);
    al_destroy_bitmap(bmp_notification);
    al_destroy_bitmap(bmp_group_move_arrow);
    al_destroy_bitmap(bmp_nectar);
    al_destroy_bitmap(bmp_no_pikmin);
    al_destroy_bitmap(bmp_number_bubble);
    al_destroy_bitmap(bmp_pikmin_spirit);
    al_destroy_bitmap(bmp_shadow);
    al_destroy_bitmap(bmp_smack);
    al_destroy_bitmap(bmp_smoke);
    al_destroy_bitmap(bmp_sparkle);
    al_destroy_bitmap(bmp_sun);
    al_destroy_bitmap(bmp_ub_spray);
    al_destroy_bitmap(bmp_us_spray);
    
    for(unsigned char i = 0; i < 3; ++i) {
        al_destroy_bitmap(bmp_mouse_button_icon[i]);
    }
    
    al_detach_voice(voice);
    al_destroy_mixer(mixer);
    al_destroy_voice(voice);
    
    sfx_attack.destroy();
    sfx_pikmin_attack.destroy();
    sfx_pikmin_carrying.destroy();
    sfx_pikmin_carrying_grab.destroy();
    sfx_pikmin_caught.destroy();
    sfx_pikmin_dying.destroy();
    sfx_pikmin_held.destroy();
    sfx_pikmin_idle.destroy();
    sfx_pikmin_thrown.destroy();
    sfx_pikmin_plucked.destroy();
    sfx_pikmin_called.destroy();
    sfx_olimar_whistle.destroy();
    sfx_louie_whistle.destroy();
    sfx_president_whistle.destroy();
    sfx_olimar_name_call.destroy();
    sfx_louie_name_call.destroy();
    sfx_president_name_call.destroy();
    sfx_throw.destroy();
    sfx_switch_pikmin.destroy();
    sfx_camera.destroy();
}
