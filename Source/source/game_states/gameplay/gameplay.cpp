/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gameplay state class and
 * gameplay state-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../load.h"
#include "../../misc_structs.h"
#include "../../mobs/pile.h"
#include "../../utils/data_file.h"
#include "../../utils/string_utils.h"


//How long the HUD moves for when the area is entered.
const float gameplay_state::AREA_INTRO_HUD_MOVE_TIME = 3.0f;
//How long it takes for the area name to fade away, in-game.
const float gameplay_state::AREA_TITLE_FADE_DURATION = 3.0f;
//Every X seconds, the cursor's position is saved, to create the trail effect.
const float gameplay_state::CURSOR_SAVE_INTERVAL = 0.03f;
//Path to the GUI information file.
const string gameplay_state::HUD_FILE_NAME = GUI_FOLDER_PATH + "/Gameplay.txt";
//The Onion menu can only show, at most, these many Pikmin types per page.
const size_t gameplay_state::ONION_MENU_TYPES_PER_PAGE = 5;
//Swarming arrows move these many units per second.
const float gameplay_state::SWARM_ARROW_SPEED = 400.0f;
//Seconds that need to pass before another swarm arrow appears.
const float gameplay_state::SWARM_ARROWS_INTERVAL = 0.1f;


/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay_state::gameplay_state() :
    game_state(),
    after_hours(false),
    area_time_passed(0.0f),
    area_title_fade_timer(AREA_TITLE_FADE_DURATION),
    closest_group_member(nullptr),
    closest_group_member_distant(false),
    cur_leader_nr(0),
    cur_leader_ptr(nullptr),
    day_minutes(0.0f),
    leader_cursor_sector(nullptr),
    msg_box(nullptr),
    particles(0),
    precipitation(0),
    swarm_angle(0),
    swarm_magnitude(0.0f),
    throw_dest_mob(nullptr),
    throw_dest_sector(nullptr),
    went_to_results(false),
    bmp_bubble(nullptr),
    bmp_counter_bubble_group(nullptr),
    bmp_counter_bubble_field(nullptr),
    bmp_counter_bubble_standby(nullptr),
    bmp_counter_bubble_total(nullptr),
    bmp_day_bubble(nullptr),
    bmp_distant_pikmin_marker(nullptr),
    bmp_fog(nullptr),
    bmp_hard_bubble(nullptr),
    bmp_message_box(nullptr),
    bmp_no_pikmin_bubble(nullptr),
    bmp_sun(nullptr),
    cancel_control_id(INVALID),
    close_to_interactable_to_use(nullptr),
    close_to_nest_to_open(nullptr),
    close_to_pikmin_to_pluck(nullptr),
    close_to_ship_to_heal(nullptr),
    cursor_height_diff_light(0.0f),
    cursor_save_timer(CURSOR_SAVE_INTERVAL),
    day(1),
    is_input_allowed(false),
    lightmap_bmp(nullptr),
    main_control_id(INVALID),
    onion_menu(nullptr),
    pause_menu(nullptr),
    paused(false),
    ready_for_input(false),
    selected_spray(0),
    swarm_next_arrow_timer(SWARM_ARROWS_INTERVAL),
    swarm_cursor(false) {
    
    swarm_next_arrow_timer.on_end = [this] () {
        swarm_next_arrow_timer.start();
        swarm_arrows.push_back(0);
    };
    swarm_next_arrow_timer.start();
}


/* ----------------------------------------------------------------------------
 * Draw the gameplay.
 */
void gameplay_state::do_drawing() {
    do_game_drawing();
    
    if(game.perf_mon) {
        game.perf_mon->leave_state();
    }
}


/* ----------------------------------------------------------------------------
 * Tick the gameplay logic by one frame.
 */
void gameplay_state::do_logic() {
    if(game.perf_mon) {
        if(is_input_allowed) {
            //The first frame will have its speed all broken,
            //because of the long loading time that came before it.
            game.perf_mon->set_paused(false);
            game.perf_mon->enter_state(PERF_MON_STATE_FRAME);
        } else {
            game.perf_mon->set_paused(true);
        }
    }
    
    if(game.maker_tools.change_speed) {
        game.delta_t *= game.maker_tools.change_speed_mult;
    }
    
    if(!paused) {
        do_gameplay_logic();
    }
    do_menu_logic();
    do_aesthetic_logic();
}


/* ----------------------------------------------------------------------------
 * Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void gameplay_state::enter() {
    al_hide_mouse_cursor(game.display);
    update_transformations();
    
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    game.mouse_cursor_s.x = al_get_mouse_state_axis(&mouse_state, 0);
    game.mouse_cursor_s.y = al_get_mouse_state_axis(&mouse_state, 1);
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
    leader_cursor_w = game.mouse_cursor_w;
    leader_cursor_s = game.mouse_cursor_s;
    
    hud.start_animation(GUI_MANAGER_ANIM_OUT_TO_IN, AREA_INTRO_HUD_MOVE_TIME);
    if(went_to_results) {
        game.fade_mgr.start_fade(true, nullptr);
        if(pause_menu) {
            pause_menu->to_delete = true;
        }
    }
    
    ready_for_input = false;
}


const int FOG_BITMAP_SIZE = 128;

/* ----------------------------------------------------------------------------
 * Generates the bitmap that'll draw the fog fade effect.
 * near_radius:
 *   Until this radius, the fog is not present.
 * far_radius:
 *   From this radius on, the fog is fully dense.
 */
ALLEGRO_BITMAP* gameplay_state::generate_fog_bitmap(
    const float near_radius, const float far_radius
) {
    if(far_radius == 0) return NULL;
    
    ALLEGRO_BITMAP* bmp = al_create_bitmap(FOG_BITMAP_SIZE, FOG_BITMAP_SIZE);
    
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY
        );
    unsigned char* row = (unsigned char*) region->data;
    
    //We need to draw a radial gradient to represent the fog.
    //Between the center and the "near" radius, the opacity is 0%.
    //From there to the edge, the opacity fades to 100%.
    //Because the every quadrant of the image is the same, just mirrored,
    //we only need to process the pixels on the top-left quadrant and then
    //apply them to the respective pixels on the other quadrants as well.
    
    //How close to the edge is the current pixel? 0 = center, 1 = edge.
    float cur_ratio = 0;
    //Alpha to use for this pixel.
    unsigned char cur_a = 0;
    //This is where the "near" section of the fog is.
    float near_ratio = near_radius / far_radius;
    //Memory location of the opposite row's pixels.
    unsigned char* opposite_row;
    
#define fill_pixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = cur_a; \
    
    for(int y = 0; y < ceil(FOG_BITMAP_SIZE / 2.0); ++y) {
        for(int x = 0; x < ceil(FOG_BITMAP_SIZE / 2.0); ++x) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            cur_ratio =
                dist(
                    point(x, y),
                    point(FOG_BITMAP_SIZE / 2.0, FOG_BITMAP_SIZE / 2.0)
                ).to_float() / (FOG_BITMAP_SIZE / 2.0);
            cur_ratio = std::min(cur_ratio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            cur_ratio =
                interpolate_number(cur_ratio, near_ratio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            cur_ratio = clamp(cur_ratio, 0.0f, 1.0f);
            cur_a = 255 * cur_ratio;
            
            opposite_row = row + region->pitch * (FOG_BITMAP_SIZE - y - y - 1);
            fill_pixel(x, row);
            fill_pixel(FOG_BITMAP_SIZE - x - 1, row);
            fill_pixel(x, opposite_row);
            fill_pixel(FOG_BITMAP_SIZE - x - 1, opposite_row);
        }
        row += region->pitch;
    }
    
#undef fill_pixel
    
    al_unlock_bitmap(bmp);
    bmp = recreate_bitmap(bmp); //Refresh mipmaps.
    return bmp;
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string gameplay_state::get_name() const {
    return "gameplay";
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void gameplay_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    //Handle the Onion menu first so events don't bleed from gameplay to it.
    if(onion_menu) {
        onion_menu->gui.handle_event(ev);
    } else if(pause_menu) {
        pause_menu->gui.handle_event(ev);
    }
    
    //Check if there are system key presses.
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        process_system_key_press(ev.keyboard.keycode);
    }
    
    if (ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
        leader_movement.reset(); //TODO replace with a better solution.
    }
    
    //Decode any inputs that result in gameplay actions.
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_button(actions[a].button, actions[a].pos, actions[a].player);
    }
    
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        if(
            ev.type == ALLEGRO_EVENT_MOUSE_AXES &&
            game.options.mouse_moves_cursor[p]
        ) {
            game.mouse_cursor_s.x = ev.mouse.x;
            game.mouse_cursor_s.y = ev.mouse.y;
            game.mouse_cursor_w = game.mouse_cursor_s;
            
            al_transform_coordinates(
                &game.screen_to_world_transform,
                &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
            );
        }
    }
    
    //Finally, let the HUD handle events.
    hud.handle_event(ev);
    
}


/* ----------------------------------------------------------------------------
 * Initializes the HUD.
 */
void gameplay_state::init_hud() {
    data_node hud_file_node(HUD_FILE_NAME);
    
    hud.register_coords("time",                  40, 10, 70, 10);
    hud.register_coords("day_bubble",            88, 18, 15, 25);
    hud.register_coords("day_number",            88, 20, 10, 10);
    hud.register_coords("leader_1_icon",          7, 90,  8, 10);
    hud.register_coords("leader_2_icon",          6, 80,  5,  9);
    hud.register_coords("leader_3_icon",          6, 72,  5,  9);
    hud.register_coords("leader_1_health",       16, 90,  8, 10);
    hud.register_coords("leader_2_health",       12, 80,  5,  9);
    hud.register_coords("leader_3_health",       12, 72,  5,  9);
    hud.register_coords("pikmin_standby_icon",   30, 91,  8, 10);
    hud.register_coords("pikmin_standby_m_icon", 35, 88,  4,  8);
    hud.register_coords("pikmin_standby_x",      38, 91,  7,  8);
    hud.register_coords("pikmin_standby_nr",     50, 91, 15, 10);
    hud.register_coords("pikmin_group_nr",       73, 91, 15, 14);
    hud.register_coords("pikmin_field_nr",       91, 91, 15, 14);
    hud.register_coords("pikmin_total_nr",        0,  0,  0,  0);
    hud.register_coords("pikmin_slash_1",        82, 91,  4,  8);
    hud.register_coords("pikmin_slash_2",         0,  0,  0,  0);
    hud.register_coords("pikmin_slash_3",         0,  0,  0,  0);
    hud.register_coords("spray_1_icon",           6, 36,  4,  7);
    hud.register_coords("spray_1_amount",        13, 37, 10,  5);
    hud.register_coords("spray_1_button",        10, 42, 10,  5);
    hud.register_coords("spray_2_icon",           6, 52,  4,  7);
    hud.register_coords("spray_2_amount",        13, 53, 10,  5);
    hud.register_coords("spray_2_button",        10, 47, 10,  5);
    hud.register_coords("spray_prev_icon",        6, 52,  3,  5);
    hud.register_coords("spray_prev_button",      6, 47,  4,  4);
    hud.register_coords("spray_next_icon",       13, 52,  3,  5);
    hud.register_coords("spray_next_button",     13, 47,  4,  4);
    hud.read_coords(hud_file_node.get_child_by_name("positions"));
    
    //Leader health and icons.
    for(size_t l = 0; l < 3; ++l) {
        //Icon.
        gui_item* leader_icon = new gui_item();
        leader_icon->on_draw =
        [this, l] (const point & center, const point & size) {
            if(l >= mobs.leaders.size()) return;
            size_t l_nr =
                (size_t) sum_and_wrap(cur_leader_nr, l, mobs.leaders.size());
                
            al_draw_filled_circle(
                center.x, center.y,
                std::min(size.x, size.y) / 2.0f,
                change_alpha(mobs.leaders[l_nr]->type->main_color, 128)
            );
            draw_bitmap_in_box(
                mobs.leaders[l_nr]->lea_type->bmp_icon,
                center, size
            );
            draw_bitmap_in_box(bmp_bubble, center, size);
        };
        hud.add_item(leader_icon, "leader_" + i2s(l + 1) + "_icon");
        
        //Health wheel.
        gui_item* leader_health = new gui_item();
        leader_health->on_draw =
        [this, l] (const point & center, const point & size) {
            if(l >= mobs.leaders.size()) return;
            size_t l_nr =
                (size_t) sum_and_wrap(cur_leader_nr, l, mobs.leaders.size());
                
            draw_health(
                center,
                mobs.leaders[l_nr]->health_wheel_smoothed_ratio, 1.0f,
                std::min(size.x, size.y) * 0.47f,
                true
            );
            draw_bitmap_in_box(bmp_hard_bubble, center, size);
        };
        hud.add_item(leader_health, "leader_" + i2s(l + 1) + "_health");
    }
    
    //Sun Meter.
    gui_item* sun_meter = new gui_item();
    sun_meter->on_draw =
    [this] (const point & center, const point & size) {
        unsigned char n_hours =
            (game.config.day_minutes_end -
             game.config.day_minutes_start) / 60.0f;
        float day_length =
            game.config.day_minutes_end - game.config.day_minutes_start;
        float day_passed_ratio =
            (float) (day_minutes - game.config.day_minutes_start) /
            (float) (day_length);
        float sun_radius = size.y / 2.0;
        float first_dot_x = (center.x - size.x / 2.0) + sun_radius;
        float last_dot_x = (center.x + size.x / 2.0) - sun_radius;
        float dots_y = center.y;
        //Width, from the center of the first dot to the center of the last.
        float dots_span = last_dot_x - first_dot_x;
        float dot_interval = dots_span / (float) n_hours;
        float sun_meter_sun_angle = area_time_passed * SUN_METER_SUN_SPIN_SPEED;
        
        //Larger bubbles at the start, middle and end of the meter.
        al_hold_bitmap_drawing(true);
        draw_bitmap(
            bmp_hard_bubble, point(first_dot_x + dots_span * 0.0, dots_y),
            point(sun_radius * 0.9, sun_radius * 0.9)
        );
        draw_bitmap(
            bmp_hard_bubble, point(first_dot_x + dots_span * 0.5, dots_y),
            point(sun_radius * 0.9, sun_radius * 0.9)
        );
        draw_bitmap(
            bmp_hard_bubble, point(first_dot_x + dots_span * 1.0, dots_y),
            point(sun_radius * 0.9, sun_radius * 0.9)
        );
        
        for(unsigned char h = 0; h < n_hours + 1; ++h) {
            draw_bitmap(
                bmp_hard_bubble,
                point(first_dot_x + h * dot_interval, dots_y),
                point(sun_radius * 0.6, sun_radius * 0.6)
            );
        }
        al_hold_bitmap_drawing(false);
        
        //Static sun.
        draw_bitmap(
            bmp_sun,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            point(sun_radius * 1.5, sun_radius * 1.5)
        );
        //Spinning sun.
        draw_bitmap(
            bmp_sun,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            point(sun_radius * 1.5, sun_radius * 1.5),
            sun_meter_sun_angle
        );
        //Bubble in front the sun.
        draw_bitmap(
            bmp_hard_bubble,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            point(sun_radius * 1.5, sun_radius * 1.5),
            0, al_map_rgb(255, 192, 128)
        );
    };
    hud.add_item(sun_meter, "time");
    
    //Day number bubble.
    gui_item* day_nr_bubble = new gui_item();
    day_nr_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap_in_box(bmp_day_bubble, center, size);
    };
    hud.add_item(day_nr_bubble, "day_bubble");
    
    //Day number text.
    gui_item* day_nr_text = new gui_item();
    day_nr_text->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            center, ALLEGRO_ALIGN_CENTER, 1,
            size, i2s(day)
        );
    };
    hud.add_item(day_nr_text, "day_number");
    
    //Standby group member icon.
    gui_item* standby_icon = new gui_item();
    standby_icon->on_draw =
    [this] (const point & center, const point & size) {
        //Standby group member preparations.
        ALLEGRO_BITMAP* standby_bmp = NULL;
        if(
            cur_leader_ptr && closest_group_member &&
            cur_leader_ptr->group->cur_standby_type
        ) {
            SUBGROUP_TYPE_CATEGORIES c =
                cur_leader_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_LEADER: {
                leader* l_ptr = dynamic_cast<leader*>(closest_group_member);
                standby_bmp = l_ptr->lea_type->bmp_icon;
                break;
                
            } default: {
                standby_bmp =
                    cur_leader_ptr->group->cur_standby_type->get_icon();
                break;
                
            }
            }
        }
        if(!standby_bmp) standby_bmp = bmp_no_pikmin_bubble;
        
        draw_bitmap_in_box(standby_bmp, center, size * 0.8);
        if(closest_group_member_distant) {
            draw_bitmap_in_box(
                bmp_distant_pikmin_marker, center, size * 0.8
            );
        }
        draw_bitmap_in_box(bmp_bubble, center, size);
    };
    hud.add_item(standby_icon, "pikmin_standby_icon");
    
    //Standby group member maturity.
    gui_item* standby_maturity = new gui_item();
    standby_maturity->on_draw =
    [this] (const point & center, const point & size) {
        //Standby group member preparations.
        ALLEGRO_BITMAP* standby_mat_bmp = NULL;
        if(
            cur_leader_ptr && closest_group_member &&
            cur_leader_ptr->group->cur_standby_type
        ) {
            SUBGROUP_TYPE_CATEGORIES c =
                cur_leader_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                pikmin* p_ptr = dynamic_cast<pikmin*>(closest_group_member);
                standby_mat_bmp =
                    p_ptr->pik_type->bmp_maturity_icon[p_ptr->maturity];
                break;
                
            } default: {
                break;
                
            }
            }
        }
        
        if(standby_mat_bmp) {
            draw_bitmap_in_box(standby_mat_bmp, center, size * 0.8);
            draw_bitmap_in_box(bmp_bubble, center, size);
        }
    };
    hud.add_item(standby_maturity, "pikmin_standby_m_icon");
    
    //Pikmin count "x".
    gui_item* pikmin_count_x = new gui_item();
    pikmin_count_x->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            center, ALLEGRO_ALIGN_CENTER, 1, size, "x"
        );
    };
    hud.add_item(pikmin_count_x, "pikmin_standby_x");
    
    //Standby group member count.
    gui_item* standby_count = new gui_item();
    standby_count->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_standby_pikmin = 0;
        if(cur_leader_ptr->group->cur_standby_type) {
            for(
                size_t m = 0; m < cur_leader_ptr->group->members.size();
                ++m
            ) {
                mob* m_ptr = cur_leader_ptr->group->members[m];
                if(
                    m_ptr->subgroup_type_ptr ==
                    cur_leader_ptr->group->cur_standby_type
                ) {
                    n_standby_pikmin++;
                }
            }
        }
        
        draw_bitmap(bmp_counter_bubble_standby, center, size);
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x + size.x * 0.4, center.y),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, i2s(n_standby_pikmin)
        );
    };
    hud.add_item(standby_count, "pikmin_standby_nr");
    
    //Group Pikmin count.
    gui_item* group_count = new gui_item();
    group_count->on_draw =
    [this] (const point & center, const point & size) {
        size_t pikmin_in_group = cur_leader_ptr->group->members.size();
        for(size_t l = 0; l < mobs.leaders.size(); ++l) {
            //If this leader is following the current one,
            //then they're not a Pikmin.
            //Subtract them from the group count total.
            if(mobs.leaders[l]->following_group == cur_leader_ptr) {
                pikmin_in_group--;
            }
        }
        
        draw_bitmap(bmp_counter_bubble_group, center, size);
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x + size.x * 0.4, center.y),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, i2s(pikmin_in_group)
        );
    };
    hud.add_item(group_count, "pikmin_group_nr");
    
    //Field Pikmin count.
    gui_item* field_count = new gui_item();
    field_count->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap(bmp_counter_bubble_field, center, size);
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x + size.x * 0.4, center.y),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, i2s(mobs.pikmin_list.size())
        );
    };
    hud.add_item(field_count, "pikmin_field_nr");
    
    //Total Pikmin count.
    gui_item* total_count = new gui_item();
    total_count->on_draw =
    [this] (const point & center, const point & size) {
        size_t total_pikmin = mobs.pikmin_list.size();
        for(size_t o = 0; o < mobs.onions.size(); ++o) {
            onion* o_ptr = mobs.onions[o];
            for(
                size_t t = 0;
                t < o_ptr->oni_type->nest->pik_types.size();
                ++t
            ) {
                for(size_t m = 0; m < N_MATURITIES; ++m) {
                    total_pikmin += mobs.onions[o]->nest->pikmin_inside[t][m];
                }
            }
        }
        
        draw_bitmap(bmp_counter_bubble_total, center, size);
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x + size.x * 0.4, center.y),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, i2s(total_pikmin)
        );
    };
    hud.add_item(total_count, "pikmin_total_nr");
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; ++s) {
        gui_item* counter_slash = new gui_item();
        counter_slash->on_draw =
        [this] (const point & center, const point & size) {
            draw_compressed_text(
                game.fonts.counter, al_map_rgb(255, 255, 255),
                center, ALLEGRO_ALIGN_CENTER, 1, size, "/"
            );
        };
        hud.add_item(counter_slash, "pikmin_slash_" + i2s(s + 1));
    }
    
    //Spray 1 icon.
    gui_item* spray_1_icon = new gui_item();
    spray_1_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        } else if(game.spray_types.size() > 0) {
            top_spray_idx = selected_spray;
        }
        if(top_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[top_spray_idx].bmp_spray, center, size
        );
    };
    hud.add_item(spray_1_icon, "spray_1_icon");
    
    //Spray 1 amount.
    gui_item* spray_1_amount = new gui_item();
    spray_1_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        } else if(game.spray_types.size() > 0) {
            top_spray_idx = selected_spray;
        }
        if(top_spray_idx == INVALID) return;
        
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x - size.x / 2.0, center.y),
            ALLEGRO_ALIGN_LEFT, 1, size,
            "x" + i2s(spray_stats[top_spray_idx].nr_sprays)
        );
    };
    hud.add_item(spray_1_amount, "spray_1_amount");
    
    //Spray 1 button.
    gui_item* spray_1_button = new gui_item();
    spray_1_button->on_draw =
    [this] (const point & center, const point & size) {
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        } else if(game.spray_types.size() > 0) {
            top_spray_idx = selected_spray;
        }
        if(top_spray_idx == INVALID) return;
        
        for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
            if(
                (
                    game.options.controls[0][c].action ==
                    BUTTON_USE_SPRAY_1 &&
                    game.spray_types.size() <= 2
                ) || (
                    game.options.controls[0][c].action ==
                    BUTTON_USE_SPRAY &&
                    game.spray_types.size() >= 3
                )
            ) {
                draw_control(
                    game.fonts.standard,
                    game.options.controls[0][c], center, size
                );
                break;
            }
        }
    };
    hud.add_item(spray_1_button, "spray_1_button");
    
    //Spray 2 icon.
    gui_item* spray_2_icon = new gui_item();
    spray_2_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[bottom_spray_idx].bmp_spray, center, size
        );
    };
    hud.add_item(spray_2_icon, "spray_2_icon");
    
    //Spray 2 amount.
    gui_item* spray_2_amount = new gui_item();
    spray_2_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        draw_compressed_text(
            game.fonts.counter, al_map_rgb(255, 255, 255),
            point(center.x - size.x / 2.0, center.y),
            ALLEGRO_ALIGN_LEFT, 1, size,
            "x" + i2s(spray_stats[bottom_spray_idx].nr_sprays)
        );
    };
    hud.add_item(spray_2_amount, "spray_2_amount");
    
    //Spray 2 button.
    gui_item* spray_2_button = new gui_item();
    spray_2_button->on_draw =
    [this] (const point & center, const point & size) {
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
            if(
                game.options.controls[0][c].action ==
                BUTTON_USE_SPRAY_2
            ) {
                draw_control(
                    game.fonts.standard,
                    game.options.controls[0][c], center, size
                );
                break;
            }
        }
    };
    hud.add_item(spray_2_button, "spray_2_button");
    
    //Previous spray icon.
    gui_item* prev_spray_icon = new gui_item();
    prev_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t prev_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            prev_spray_idx =
                sum_and_wrap(selected_spray, -1, game.spray_types.size());
        }
        if(prev_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[prev_spray_idx].bmp_spray,
            center, size
        );
    };
    hud.add_item(prev_spray_icon, "spray_prev_icon");
    
    //Previous spray button.
    gui_item* prev_spray_button = new gui_item();
    prev_spray_button->on_draw =
    [this] (const point & center, const point & size) {
        size_t prev_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            prev_spray_idx =
                sum_and_wrap(selected_spray, -1, game.spray_types.size());
        }
        if(prev_spray_idx == INVALID) return;
        
        for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
            if(
                game.options.controls[0][c].action ==
                BUTTON_PREV_SPRAY
            ) {
                draw_control(
                    game.fonts.standard,
                    game.options.controls[0][c], center, size
                );
                break;
            }
        }
    };
    hud.add_item(prev_spray_button, "spray_prev_button");
    
    //Next spray icon.
    gui_item* next_spray_icon = new gui_item();
    next_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t next_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            next_spray_idx =
                sum_and_wrap(selected_spray, 1, game.spray_types.size());
        }
        if(next_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[next_spray_idx].bmp_spray,
            center, size
        );
    };
    hud.add_item(next_spray_icon, "spray_next_icon");
    
    //Next spray button.
    gui_item* next_spray_button = new gui_item();
    next_spray_button->on_draw =
    [this] (const point & center, const point & size) {
        size_t next_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            next_spray_idx =
                sum_and_wrap(selected_spray, 1, game.spray_types.size());
        }
        if(next_spray_idx == INVALID) return;
        
        for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
            if(
                game.options.controls[0][c].action ==
                BUTTON_NEXT_SPRAY
            ) {
                draw_control(
                    game.fonts.standard,
                    game.options.controls[0][c], center, size
                );
                break;
            }
        }
    };
    hud.add_item(next_spray_button, "spray_next_button");
    
    //Now, load the file settings.
    data_node* bitmaps_node = hud_file_node.get_child_by_name("files");
    
#define loader(var, name) \
    var = \
          game.bitmaps.get( \
                            bitmaps_node->get_child_by_name(name)->value, \
                            bitmaps_node->get_child_by_name(name) \
                          );
    
    loader(bmp_bubble,                 "bubble");
    loader(bmp_counter_bubble_field,   "counter_bubble_field");
    loader(bmp_counter_bubble_group,   "counter_bubble_group");
    loader(bmp_counter_bubble_standby, "counter_bubble_standby");
    loader(bmp_counter_bubble_total,   "counter_bubble_total");
    loader(bmp_day_bubble,             "day_bubble");
    loader(bmp_distant_pikmin_marker,  "distant_pikmin_marker");
    loader(bmp_hard_bubble,            "hard_bubble");
    loader(bmp_message_box,            "message_box");
    loader(bmp_no_pikmin_bubble,       "no_pikmin_bubble");
    loader(bmp_sun,                    "sun");
    
#undef loader
    
}


/* ----------------------------------------------------------------------------
 * Leaves the gameplay state and enters the main menu,
 * or area selection, or etc.
 * target:
 *   Where to leave to.
 */
void gameplay_state::leave(const LEAVE_TARGET target) {
    if(game.perf_mon) {
        //Don't register the final frame, since it won't draw anything.
        game.perf_mon->set_paused(true);
    }
    
    al_show_mouse_cursor(game.display);
    
    switch(target) {
    case LEAVE_TO_RETRY: {
        game.change_state(game.states.gameplay);
        break;
    } case LEAVE_TO_FINISH: {
        game.states.results->time_taken = area_time_passed;
        went_to_results = true;
        //Change state, but don't unload this one, since the player
        //may pick the "keep playing" option in the results screen.
        game.change_state(game.states.results, false);
        break;
    } case LEAVE_TO_AREA_SELECT: {
        if(game.states.area_ed->quick_play_area.empty()) {
            game.change_state(game.states.area_menu);
        } else {
            game.change_state(game.states.area_ed);
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the "gameplay" state into memory.
 */
void gameplay_state::load() {
    if(game.perf_mon) {
        game.perf_mon->reset();
        game.perf_mon->enter_state(PERF_MON_STATE_LOADING);
        game.perf_mon->set_paused(false);
    }
    
    size_t errors_reported_at_start = game.errors_reported_so_far;
    went_to_results = false;
    
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    //Game content.
    load_game_content();
    
    //Initializing game things.
    size_t n_spray_types = game.spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) {
        spray_stats.push_back(spray_stats_struct());
    }
    
    load_area(area_to_load, false, false);
    
    if(!game.cur_area_data.weather_condition.blackout_strength.empty()) {
        lightmap_bmp = al_create_bitmap(game.win_w, game.win_h);
    }
    if(!game.cur_area_data.weather_condition.fog_color.empty()) {
        bmp_fog =
            generate_fog_bitmap(
                game.cur_area_data.weather_condition.fog_near,
                game.cur_area_data.weather_condition.fog_far
            );
    }
    
    //Generate mobs.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object generation");
    }
    
    vector<mob*> mobs_per_gen;
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        mobs_per_gen.push_back(
            create_mob(
                m_ptr->category, m_ptr->pos, m_ptr->type,
                m_ptr->angle, m_ptr->vars
            )
        );
    }
    
    //Panic check -- If there are no leaders, abort.
    if(mobs.leaders.empty()) {
        show_message_box(
            game.display, "No leaders!", "No leaders!",
            "This area has no leaders! You need at least one "
            "in order to play.",
            NULL, ALLEGRO_MESSAGEBOX_WARN
        );
        
        leave(LEAVE_TO_AREA_SELECT);
        return;
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen number X
    //does not necessarily correspond to mob number X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) number.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        for(size_t l = 0; l < m_ptr->link_nrs.size(); ++l) {
            mobs_per_gen[m]->links.push_back(mobs_per_gen[m_ptr->link_nrs[l]]);
        }
    }
    
    //Save each path stop's sector.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.path_stops[s]->sector_ptr =
            get_sector(game.cur_area_data.path_stops[s]->pos, NULL, true);
    }
    
    //Sort leaders.
    sort(
        mobs.leaders.begin(), mobs.leaders.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t priority_l1 =
        find(
            game.config.leader_order.begin(),
            game.config.leader_order.end(), l1->lea_type
        ) -
        game.config.leader_order.begin();
        size_t priority_l2 =
        find(
            game.config.leader_order.begin(),
            game.config.leader_order.end(), l2->lea_type
        ) -
        game.config.leader_order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    path_mgr.handle_area_load();
    
    init_hud();
    
    cur_leader_nr = 0;
    cur_leader_ptr = mobs.leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    cur_leader_ptr->active = true;
    
    game.cam.set_pos(cur_leader_ptr->pos);
    game.cam.set_zoom(game.options.zoom_mid_level);
    
    cursor_save_timer.on_end = [this] () {
        cursor_save_timer.start();
        cursor_spots.push_back(game.mouse_cursor_s);
        if(cursor_spots.size() > CURSOR_SAVE_N_SPOTS) {
            cursor_spots.erase(cursor_spots.begin());
        }
    };
    cursor_save_timer.start();
    
    cur_leader_ptr->stop_whistling();
    update_closest_group_member();
    
    day_minutes = game.config.day_minutes_start;
    area_time_passed = 0.0f;
    after_hours = false;
    paused = false;
    game.maker_tools.reset_for_gameplay();
    
    map<string, string> spray_strs =
        get_var_map(game.cur_area_data.spray_amounts);
        
    for(auto &s : spray_strs) {
        size_t spray_id = 0;
        for(; spray_id < game.spray_types.size(); ++spray_id) {
            if(game.spray_types[spray_id].name == s.first) {
                break;
            }
        }
        if(spray_id == game.spray_types.size()) {
            log_error(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.cur_area_data.name + "\"!", NULL
            );
            continue;
        }
        
        spray_stats[spray_id].nr_sprays = s2i(s.second);
    }
    
    for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
        if(game.options.controls[0][c].action == BUTTON_THROW) {
            main_control_id = c;
            break;
        }
    }
    for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
        if(game.options.controls[0][c].action == BUTTON_WHISTLE) {
            cancel_control_id = c;
            break;
        }
    }
    
    game.states.results->reset();
    game.states.results->area_name = game.cur_area_data.name;
    game.states.results->enemies_total = mobs.enemies.size();
    for(size_t t = 0; t < mobs.treasures.size(); ++t) {
        game.states.results->points_total +=
            mobs.treasures[t]->tre_type->points;
    }
    for(size_t e = 0; e < mobs.enemies.size(); ++e) {
        for(size_t s = 0; s < mobs.enemies[e]->specific_spoils.size(); ++s) {
            mob_type* s_type = mobs.enemies[e]->specific_spoils[s];
            if(s_type->category->id == MOB_CATEGORY_TREASURES) {
                game.states.results->points_total +=
                    ((treasure_type*) s_type)->points;
            }
        }
    }
    for(size_t p = 0; p < mobs.piles.size(); ++p) {
        pile* p_ptr = mobs.piles[p];
        resource_type* res_type = p_ptr->pil_type->contents;
        if(res_type->delivery_result != RESOURCE_DELIVERY_RESULT_ADD_POINTS) {
            continue;
        }
        game.states.results->points_total +=
            p_ptr->amount * res_type->point_amount;
    }
    
    //TODO Uncomment this when replays are implemented.
    /*
    replay_timer = timer(
        REPLAY_SAVE_FREQUENCY,
    [this] () {
        this->replay_timer.start();
        vector<mob*> obstacles; //TODO
        session_replay.add_state(
            leaders, pikmin_list, enemies, treasures, onions, obstacles,
            cur_leader_nr
        );
    }
    );
    replay_timer.start();
    session_replay.clear();*/
    
    area_title_fade_timer.start();
    
    if(game.errors_reported_so_far > errors_reported_at_start) {
        print_info(
            "\n\n\nERRORS FOUND!\n"
            "See \"" + ERROR_LOG_FILE_PATH + "\".\n\n\n",
            20.0f, 3.0f
        );
    }
    
    game.framerate_last_avg_point = 0;
    game.framerate_history.clear();
    
    if(game.perf_mon) {
        game.perf_mon->set_area_name(game.cur_area_data.name);
        game.perf_mon->leave_state();
    }
    
    enter();
}


/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void gameplay_state::load_game_content() {
    load_custom_particle_generators(true);
    load_liquids(true);
    load_status_types(true);
    load_spray_types(true);
    load_hazards();
    load_weather();
    load_spike_damage_types();
    
    //Mob types.
    load_mob_types(true);
    
    //Register leader sub-group types.
    for(size_t p = 0; p < game.config.pikmin_order.size(); ++p) {
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, game.config.pikmin_order[p],
            game.config.pikmin_order[p]->bmp_icon
        );
    }
    
    vector<string> tool_types_vector;
    for(auto &t : game.mob_types.tool) {
        tool_types_vector.push_back(t.first);
    }
    sort(tool_types_vector.begin(), tool_types_vector.end());
    for(size_t t = 0; t < tool_types_vector.size(); ++t) {
        tool_type* tt_ptr = game.mob_types.tool[tool_types_vector[t]];
        subgroup_types.register_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, tt_ptr, tt_ptr->bmp_icon
        );
    }
    
    subgroup_types.register_type(SUBGROUP_TYPE_CATEGORY_LEADER);
}


/* ----------------------------------------------------------------------------
 * Starts the fade out to leave the gameplay state.
 * target:
 *   Where to leave to.
 */
void gameplay_state::start_leaving(const LEAVE_TARGET target) {
    game.fade_mgr.start_fade( false, [this, target] () { leave(target); });
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay_state::unload() {
    al_show_mouse_cursor(game.display);
    
    hud.destroy();
    path_mgr.clear();
    
    cur_leader_ptr = NULL;
    
    close_to_interactable_to_use = NULL;
    close_to_nest_to_open = NULL;
    close_to_pikmin_to_pluck = NULL;
    close_to_ship_to_heal = NULL;
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    while(!mobs.all.empty()) {
        delete_mob(*mobs.all.begin(), true);
    }
    
    if(lightmap_bmp) {
        al_destroy_bitmap(lightmap_bmp);
        lightmap_bmp = NULL;
    }
    
    unload_area();
    
    spray_stats.clear();
    particles.clear();
    
    unload_game_content();
    
    game.bitmaps.detach(bmp_bubble);
    game.bitmaps.detach(bmp_counter_bubble_field);
    game.bitmaps.detach(bmp_counter_bubble_group);
    game.bitmaps.detach(bmp_counter_bubble_standby);
    game.bitmaps.detach(bmp_counter_bubble_total);
    game.bitmaps.detach(bmp_day_bubble);
    game.bitmaps.detach(bmp_distant_pikmin_marker);
    game.bitmaps.detach(bmp_hard_bubble);
    game.bitmaps.detach(bmp_message_box);
    game.bitmaps.detach(bmp_no_pikmin_bubble);
    game.bitmaps.detach(bmp_sun);
    if(bmp_fog) {
        al_destroy_bitmap(bmp_fog);
        bmp_fog = NULL;
    }
    
    if(msg_box) {
        delete msg_box;
        msg_box = NULL;
    }
    if(onion_menu) {
        delete onion_menu;
        onion_menu = NULL;
    }
    if(pause_menu) {
        delete pause_menu;
        pause_menu = NULL;
    }
    game.maker_tools.info_print_text.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded game content.
 */
void gameplay_state::unload_game_content() {
    unload_weather();
    
    subgroup_types.clear();
    
    unload_mob_types(true);
    
    unload_spike_damage_types();
    unload_hazards();
    unload_spray_types();
    unload_status_types(true);
    unload_liquids();
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Updates the variable that indicates what the closest
 * group member of the standby subgroup is.
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * NULL if there is no member of that subgroup available.
 */
void gameplay_state::update_closest_group_member() {
    //Closest members so far for each maturity.
    dist closest_dists[N_MATURITIES];
    mob* closest_ptrs[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        closest_ptrs[m] = NULL;
    }
    
    game.states.gameplay->closest_group_member = NULL;
    
    //Fetch the closest, for each maturity.
    size_t n_members = cur_leader_ptr->group->members.size();
    for(size_t m = 0; m < n_members; ++m) {
    
        mob* member_ptr = cur_leader_ptr->group->members[m];
        if(
            member_ptr->subgroup_type_ptr !=
            cur_leader_ptr->group->cur_standby_type
        ) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(member_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((pikmin*) member_ptr)->maturity;
        }
        
        dist d(cur_leader_ptr->pos, member_ptr->pos);
        
        if(!closest_ptrs[maturity] || d < closest_dists[maturity]) {
            closest_dists[maturity] = d;
            closest_ptrs[maturity] = member_ptr;
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    dist closest_dist;
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        if(!closest_ptrs[2 - m]) continue;
        if(closest_dists[2 - m] > game.config.group_member_grab_range) continue;
        game.states.gameplay->closest_group_member = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!game.states.gameplay->closest_group_member) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; ++m) {
            if(!closest_ptrs[m]) continue;
            
            if(
                !game.states.gameplay->closest_group_member ||
                closest_dists[m] < closest_dist
            ) {
                game.states.gameplay->closest_group_member = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
        
    }
    
    game.states.gameplay->closest_group_member_distant = false;
    
    if(!game.states.gameplay->closest_group_member) {
        return;
    }
    
    //Figure out if it can be reached, or if it's too distant.
    
    if(
        cur_leader_ptr->ground_sector &&
        !cur_leader_ptr->ground_sector->hazards.empty()
    ) {
        if(
            !game.states.gameplay->closest_group_member->
            is_resistant_to_hazards(
                cur_leader_ptr->ground_sector->hazards
            )
        ) {
            //The leader is on a hazard that the member isn't resistent to.
            //Don't let the leader grab it.
            closest_group_member_distant = true;
        }
    }
    
    if(closest_dist > game.config.group_member_grab_range) {
        //The group member is physically too far away.
        game.states.gameplay->closest_group_member_distant = true;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void gameplay_state::update_transformations() {
    //World coordinates to screen coordinates.
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + game.win_w / 2.0 / game.cam.zoom,
        -game.cam.pos.y + game.win_h / 2.0 / game.cam.zoom
    );
    al_scale_transform(
        &game.world_to_screen_transform, game.cam.zoom, game.cam.zoom
    );
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}
