/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-game HUD classes and functions.
 */

#include <algorithm>

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"
#include "gameplay.h"


namespace HUD {
//Dampen the mission goal indicator's movement by this much.
const float GOAL_INDICATOR_SMOOTHNESS_MULT = 5.5f;
//Path to the GUI information file.
const string GUI_FILE_NAME = GUI_FOLDER_PATH + "/Gameplay.txt";
//How long the leader swap juice animation lasts for.
const float LEADER_SWAP_JUICE_DURATION = 0.7f;
//Standard mission score medal icon scale.
const float MEDAL_ICON_SCALE = 1.5f;
//Multiply time by this much to get the right scale animation amount.
const float MEDAL_ICON_SCALE_MULT = 0.3f;
//Multiply time by this much to get the right scale animation speed.
const float MEDAL_ICON_SCALE_TIME_MULT = 4.0f;
//Dampen the mission score indicator's movement by this much.
const float SCORE_INDICATOR_SMOOTHNESS_MULT = 5.5f;
//How many points to show before and after the mission score ruler flapper.
const int SCORE_RULER_RANGE = 125;
//How long the spray swap juice animation lasts for.
const float SPRAY_SWAP_JUICE_DURATION = 0.7f;
//How long the standby swap juice animation lasts for.
const float STANDBY_SWAP_JUICE_DURATION = 0.5f;
//The Sun Meter's sun spins these many radians per second.
const float SUN_METER_SUN_SPIN_SPEED = 0.5f;
//Speed at which previously-unnecessary items fade in, in alpha per second.
const float UNNECESSARY_ITEMS_FADE_IN_SPEED = 2.5f;
//Delay before unnecessary items start fading out.
const float UNNECESSARY_ITEMS_FADE_OUT_DELAY = 2.5f;
//Speed at which unnecessary items fade out, in alpha per second.
const float UNNECESSARY_ITEMS_FADE_OUT_SPEED = 0.5f;
}


/* ----------------------------------------------------------------------------
 * Creates a new HUD structure instance.
 */
hud_struct::hud_struct(const size_t &m_player_id) :
    bmp_bubble(nullptr),
    bmp_counter_bubble_group(nullptr),
    bmp_counter_bubble_field(nullptr),
    bmp_counter_bubble_standby(nullptr),
    bmp_counter_bubble_total(nullptr),
    bmp_day_bubble(nullptr),
    bmp_distant_pikmin_marker(nullptr),
    bmp_hard_bubble(nullptr),
    bmp_no_pikmin_bubble(nullptr),
    bmp_sun(nullptr),
    leader_icon_mgr(&gui),
    leader_health_mgr(&gui),
    player_id(m_player_id),
    standby_icon_mgr(&gui),
    spray_icon_mgr(&gui),
    standby_items_opacity(0.0f),
    standby_items_fade_timer(0.0f),
    spray_items_opacity(0.0f),
    spray_items_fade_timer(0.0f),
    prev_standby_type(nullptr),
    prev_maturity_icon(nullptr),
    spray_1_amount(nullptr),
    spray_2_amount(nullptr),
    standby_count_nr(0),
    standby_amount(nullptr),
    group_count_nr(0),
    group_amount(nullptr),
    field_count_nr(0),
    field_amount(nullptr),
    total_count_nr(0),
    total_amount(nullptr) {
    
    data_node hud_file_node(HUD::GUI_FILE_NAME);
    
    gui.register_coords("time",                          0,    0,  0,  0);
    gui.register_coords("day_bubble",                    0,    0,  0,  0);
    gui.register_coords("day_number",                    0,    0,  0,  0);
    gui.register_coords("leader_1_icon",                 7,   90,  8, 10);
    gui.register_coords("leader_2_icon",                 6,   80,  5,  9);
    gui.register_coords("leader_3_icon",                 6,   72,  5,  9);
    gui.register_coords("leader_1_health",              16,   90,  8, 10);
    gui.register_coords("leader_2_health",              12,   80,  5,  9);
    gui.register_coords("leader_3_health",              12,   72,  5,  9);
    gui.register_coords("leader_next_input",            4,   83,  3,  3);
    gui.register_coords("standby_icon",                 50,   91,  8, 10);
    gui.register_coords("standby_amount",               50,   96, 15,  8);
    gui.register_coords("standby_bubble",                0,    0,  0,  0);
    gui.register_coords("standby_maturity_icon",        54,   88,  4,  8);
    gui.register_coords("standby_next_icon",            58,   93,  6,  8);
    gui.register_coords("standby_next_input",           60,   96,  3,  3);
    gui.register_coords("standby_prev_icon",            42,   93,  6,  8);
    gui.register_coords("standby_prev_input",           40,   96,  3,  3);
    gui.register_coords("group_amount",                 73,   91, 15, 14);
    gui.register_coords("group_bubble",                 73,   91, 15, 14);
    gui.register_coords("field_amount",                 91,   91, 15, 14);
    gui.register_coords("field_bubble",                 91,   91, 15, 14);
    gui.register_coords("total_amount",                  0,    0,  0,  0);
    gui.register_coords("total_bubble",                  0,    0,  0,  0);
    gui.register_coords("counters_x",                    0,    0,  0,  0);
    gui.register_coords("counters_slash_1",             82,   91,  4,  8);
    gui.register_coords("counters_slash_2",              0,    0,  0,  0);
    gui.register_coords("counters_slash_3",              0,    0,  0,  0);
    gui.register_coords("spray_1_icon",                  6,   36,  4,  7);
    gui.register_coords("spray_1_amount",               13,   37, 10,  5);
    gui.register_coords("spray_1_input",                 4,   39,  3,  3);
    gui.register_coords("spray_2_icon",                  6,   52,  4,  7);
    gui.register_coords("spray_2_amount",               13,   53, 10,  5);
    gui.register_coords("spray_2_input",                 4,   55,  3,  3);
    gui.register_coords("spray_prev_icon",               6,   48,  3,  5);
    gui.register_coords("spray_prev_input",              4,   51,  4,  4);
    gui.register_coords("spray_next_icon",              12,   48,  3,  5);
    gui.register_coords("spray_next_input",             14,   51,  4,  4);
    gui.register_coords("mission_goal_bubble",          18,    8, 32, 12);
    gui.register_coords("mission_goal_cur_label",      9.5, 11.5, 13,  3);
    gui.register_coords("mission_goal_cur",            9.5,  6.5, 13,  7);
    gui.register_coords("mission_goal_req_label",     26.5, 11.5, 13,  3);
    gui.register_coords("mission_goal_req",           26.5,  6.5, 13,  7);
    gui.register_coords("mission_goal_slash",           18,  6.5,  4,  7);
    gui.register_coords("mission_goal_name",            18,    8, 30, 10);
    gui.register_coords("mission_score_bubble",         18,   20, 32, 10);
    gui.register_coords("mission_score_score_label",   7.5, 21.5,  9,  5);
    gui.register_coords("mission_score_points",         18, 21.5, 10,  5);
    gui.register_coords("mission_score_points_label", 28.5, 21.5,  9,  5);
    gui.register_coords("mission_score_ruler",          18,   17, 30,  2);
    gui.register_coords("mission_fail_1_bubble",        82,    8, 32, 12);
    gui.register_coords("mission_fail_1_cur_label",   73.5, 11.5, 13,  3);
    gui.register_coords("mission_fail_1_cur",         73.5,  6.5, 13,  7);
    gui.register_coords("mission_fail_1_req_label",   90.5, 11.5, 13,  3);
    gui.register_coords("mission_fail_1_req",         90.5,  6.5, 13,  7);
    gui.register_coords("mission_fail_1_slash",         82,  6.5,  4,  7);
    gui.register_coords("mission_fail_1_name",          82,    8, 30, 10);
    gui.register_coords("mission_fail_2_bubble",        82,   20, 32, 10);
    gui.register_coords("mission_fail_2_cur_label",   73.5, 22.5, 13,  3);
    gui.register_coords("mission_fail_2_cur",         73.5, 18.5, 13,  5);
    gui.register_coords("mission_fail_2_req_label",   90.5, 22.5, 13,  3);
    gui.register_coords("mission_fail_2_req",         90.5, 18.5, 13,  5);
    gui.register_coords("mission_fail_2_slash",         82, 18.5,  4,  5);
    gui.register_coords("mission_fail_2_name",          82,   20, 30,  8);
    gui.read_coords(hud_file_node.get_child_by_name("positions"));
    
    //Leader health and icons.
    for(size_t l = 0; l < 3; ++l) {
    
        //Icon.
        gui_item* leader_icon = new gui_item();
        leader_icon->on_draw =
        [this, l] (const point & center, const point & size) {
            leader_icon_bubble icon;
            point final_center;
            point final_size;
            this->leader_icon_mgr.get_drawing_info(
                l, &icon, &final_center, &final_size
            );
            
            if(!icon.bmp) return;
            
            al_draw_filled_circle(
                final_center.x, final_center.y,
                std::min(final_size.x, final_size.y) / 2.0f,
                change_alpha(
                    icon.color,
                    128
                )
            );
            draw_bitmap_in_box(
                icon.bmp,
                final_center, final_size, true
            );
            draw_bitmap_in_box(
                bmp_bubble,
                final_center, final_size, true
            );
        };
        gui.add_item(leader_icon, "leader_" + i2s(l + 1) + "_icon");
        leader_icon_mgr.register_bubble(l, leader_icon);
        
        
        //Health wheel.
        gui_item* leader_health = new gui_item();
        leader_health->on_draw =
        [this, l] (const point & center, const point & size) {
            leader_health_bubble health;
            point final_center;
            point final_size;
            this->leader_health_mgr.get_drawing_info(
                l, &health, &final_center, &final_size
            );
            
            if(health.ratio <= 0.0f) return;
            
            if(health.caution_timer > 0.0f) {
                float caution_ring_scale =
                    interpolate_number(
                        health.caution_timer,
                        0.0f, LEADER::HEALTH_CAUTION_RING_DURATION,
                        1.0f, 2.0f
                    );
                unsigned char caution_ring_alpha =
                    health.caution_timer <
                    LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f ?
                    interpolate_number(
                        health.caution_timer,
                        0.0f, LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        0.0f, 192
                    ) :
                    interpolate_number(
                        health.caution_timer,
                        LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        LEADER::HEALTH_CAUTION_RING_DURATION,
                        192, 0
                    );
                float caution_ring_size =
                    std::min(final_size.x, final_size.y) * caution_ring_scale;
                    
                draw_bitmap(
                    game.sys_assets.bmp_bright_ring,
                    final_center,
                    point(caution_ring_size, caution_ring_size),
                    0.0f,
                    al_map_rgba(255, 0, 0, caution_ring_alpha)
                );
            }
            
            draw_health(
                final_center,
                health.ratio,
                1.0f,
                std::min(final_size.x, final_size.y) * 0.47f,
                true
            );
            draw_bitmap_in_box(
                bmp_hard_bubble,
                final_center,
                final_size,
                true
            );
        };
        gui.add_item(leader_health, "leader_" + i2s(l + 1) + "_health");
        leader_health_mgr.register_bubble(l, leader_health);
        
    }
    
    
    //Next leader input.
    gui_item* leader_next_input = new gui_item();
    leader_next_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].available_leaders.size() < 2) return;
        player_input i =
            game.controls.find_bind(PLAYER_ACTION_NEXT_LEADER).input;
        if(i.type == INPUT_TYPE_NONE) return;
        draw_player_input_icon(game.fonts.slim, i, true, center, size);
    };
    gui.add_item(leader_next_input, "leader_next_input");
    
    
    //Sun Meter.
    gui_item* sun_meter = new gui_item();
    sun_meter->on_draw =
    [this, sun_meter] (const point & center, const point & size) {
        unsigned char n_hours =
            (
                game.config.day_minutes_end -
                game.config.day_minutes_start
            ) / 60.0f;
        float day_length =
            game.config.day_minutes_end - game.config.day_minutes_start;
        float day_passed_ratio =
            (
                game.states.gameplay->day_minutes -
                game.config.day_minutes_start
            ) /
            (float) (day_length);
        float sun_radius = size.y / 2.0;
        float first_dot_x = (center.x - size.x / 2.0) + sun_radius;
        float last_dot_x = (center.x + size.x / 2.0) - sun_radius;
        float dots_y = center.y;
        //Width, from the center of the first dot to the center of the last.
        float dots_span = last_dot_x - first_dot_x;
        float dot_interval = dots_span / (float) n_hours;
        float sun_meter_sun_angle =
            game.states.gameplay->area_time_passed *
            HUD::SUN_METER_SUN_SPIN_SPEED;
            
        //Larger bubbles at the start, middle and end of the meter.
        al_hold_bitmap_drawing(true);
        draw_bitmap(
            bmp_hard_bubble,
            point(first_dot_x + dots_span * 0.0, dots_y),
            point(sun_radius * 0.9, sun_radius * 0.9)
        );
        draw_bitmap(
            bmp_hard_bubble,
            point(first_dot_x + dots_span * 0.5, dots_y),
            point(sun_radius * 0.9, sun_radius * 0.9)
        );
        draw_bitmap(
            bmp_hard_bubble,
            point(first_dot_x + dots_span * 1.0, dots_y),
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
        
        point sun_size =
            point(sun_radius * 1.5, sun_radius * 1.5) +
            sun_meter->get_juice_value();
        //Static sun.
        draw_bitmap(
            bmp_sun,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size
        );
        //Spinning sun.
        draw_bitmap(
            bmp_sun,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size,
            sun_meter_sun_angle
        );
        //Bubble in front the sun.
        draw_bitmap(
            bmp_hard_bubble,
            point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size,
            0.0f,
            al_map_rgb(255, 192, 128)
        );
    };
    sun_meter->on_tick =
    [this, sun_meter] (const float delta_t) {
        float day_length =
            game.config.day_minutes_end - game.config.day_minutes_start;
        float pre_tick_day_minutes =
            game.states.gameplay->day_minutes -
            game.cur_area_data.day_time_speed * delta_t / 60.0f;
        float post_tick_day_minutes =
            game.states.gameplay->day_minutes;
        const float checkpoints[3] = {0.25f, 0.50f, 0.75f};
        for(unsigned char c = 0; c < 3; ++c) {
            float checkpoint =
                game.config.day_minutes_start + day_length * checkpoints[c];
            if(
                pre_tick_day_minutes < checkpoint &&
                post_tick_day_minutes >= checkpoint
            ) {
                sun_meter->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_ICON
                );
                break;
            }
        }
    };
    gui.add_item(sun_meter, "time");
    
    
    //Day number bubble.
    gui_item* day_bubble = new gui_item();
    day_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap_in_box(bmp_day_bubble, center, size, true);
    };
    gui.add_item(day_bubble, "day_bubble");
    
    
    //Day number text.
    gui_item* day_nr = new gui_item();
    day_nr->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter, COLOR_WHITE,
            center, ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
            size, i2s(game.states.gameplay->day)
        );
    };
    gui.add_item(day_nr, "day_number");
    
    
    //Standby group member icon.
    gui_item* standby_icon = new gui_item();
    standby_icon->on_draw =
    [this] (const point & center, const point & size) {
        this->draw_standby_icon(BUBBLE_CURRENT);
    };
    gui.add_item(standby_icon, "standby_icon");
    standby_icon_mgr.register_bubble(BUBBLE_CURRENT, standby_icon);
    
    
    //Next standby subgroup icon.
    gui_item* standby_next_icon = new gui_item();
    standby_next_icon->on_draw =
    [this] (const point & center, const point & size) {
        this->draw_standby_icon(BUBBLE_NEXT);
    };
    gui.add_item(standby_next_icon, "standby_next_icon");
    standby_icon_mgr.register_bubble(BUBBLE_NEXT, standby_next_icon);
    
    
    //Next standby subgroup input.
    gui_item* standby_next_input = new gui_item();
    standby_next_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        subgroup_type* next_type;
        game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->get_next_standby_type(
            false, &next_type
        );
        if(
            next_type ==
            game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->cur_standby_type
        ) {
            return;
        }
        player_input i =
            game.controls.find_bind(PLAYER_ACTION_NEXT_TYPE).input;
        if(i.type == INPUT_TYPE_NONE) return;
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->standby_items_opacity * 255
        );
    };
    gui.add_item(standby_next_input, "standby_next_input");
    
    
    //Previous standby subgroup icon.
    gui_item* standby_prev_icon = new gui_item();
    standby_prev_icon->on_draw =
    [this] (const point & center, const point & size) {
        this->draw_standby_icon(BUBBLE_PREVIOUS);
    };
    gui.add_item(standby_prev_icon, "standby_prev_icon");
    standby_icon_mgr.register_bubble(BUBBLE_PREVIOUS, standby_prev_icon);
    
    
    //Previous standby subgroup input.
    gui_item* standby_prev_input = new gui_item();
    standby_prev_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        subgroup_type* prev_type;
        game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->get_next_standby_type(
            true, &prev_type
        );
        subgroup_type* next_type;
        game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->get_next_standby_type(
            false, &next_type
        );
        if(
            prev_type ==
            game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->cur_standby_type ||
            prev_type == next_type
        ) {
            return;
        }
        player_input i =
            game.controls.find_bind(PLAYER_ACTION_PREV_TYPE).input;
        if(i.type == INPUT_TYPE_NONE) return;
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->standby_items_opacity * 255
        );
    };
    gui.add_item(standby_prev_input, "standby_prev_input");
    
    
    //Standby group member maturity.
    gui_item* standby_maturity_icon = new gui_item();
    standby_maturity_icon->on_draw =
    [this, standby_maturity_icon] (const point & center, const point & size) {
        //Standby group member preparations.
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        ALLEGRO_BITMAP* standby_mat_bmp = NULL;
        leader* l_ptr = game.states.gameplay->player_info[this->player_id].cur_leader_ptr;
        if(!l_ptr || !l_ptr->group) return;
        
        mob* closest =
            game.states.gameplay->player_info[this->player_id].closest_group_member[BUBBLE_CURRENT];
            
        if(l_ptr->group->cur_standby_type && closest) {
            SUBGROUP_TYPE_CATEGORIES c =
                l_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                pikmin* p_ptr =
                    dynamic_cast<pikmin*>(closest);
                standby_mat_bmp =
                    p_ptr->pik_type->bmp_maturity_icon[p_ptr->maturity];
                break;
                
            } default: {
                break;
                
            }
            }
        }
        
        ALLEGRO_COLOR color =
            map_alpha(this->standby_items_opacity * 255);
            
        if(standby_mat_bmp) {
            draw_bitmap_in_box(
                standby_mat_bmp, center,
                (size * 0.8) + standby_maturity_icon->get_juice_value(),
                true,
                0.0f, color
            );
            draw_bitmap_in_box(
                bmp_bubble, center,
                size + standby_maturity_icon->get_juice_value(),
                true, 0.0f, color
            );
        }
        
        if(
            l_ptr->group->cur_standby_type != prev_standby_type ||
            standby_mat_bmp != prev_maturity_icon
        ) {
            standby_maturity_icon->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_ICON
            );
            prev_standby_type = l_ptr->group->cur_standby_type;
            prev_maturity_icon = standby_mat_bmp;
        }
    };
    gui.add_item(standby_maturity_icon, "standby_maturity_icon");
    
    
    //Standby subgroup member amount bubble.
    gui_item* standby_bubble = new gui_item();
    standby_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap(
            bmp_counter_bubble_standby,
            center,
            size,
            0.0f,
            map_alpha(this->standby_items_opacity * 255)
        );
    };
    gui.add_item(standby_bubble, "standby_bubble");
    
    
    //Standby subgroup member amount.
    standby_amount = new gui_item();
    standby_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_standby_pikmin = 0;
        leader* l_ptr = game.states.gameplay->player_info[this->player_id].cur_leader_ptr;
        
        if(l_ptr && l_ptr->group->cur_standby_type) {
            for(size_t m = 0; m < l_ptr->group->members.size(); ++m) {
                mob* m_ptr = l_ptr->group->members[m];
                if(m_ptr->subgroup_type_ptr == l_ptr->group->cur_standby_type) {
                    n_standby_pikmin++;
                }
            }
        }
        
        if(n_standby_pikmin != standby_count_nr) {
            standby_amount->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            standby_count_nr = n_standby_pikmin;
        }
        
        draw_compressed_scaled_text(
            game.fonts.counter,
            map_alpha(this->standby_items_opacity * 255),
            center,
            point(1.0f, 1.0f) + standby_amount->get_juice_value(),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
            size * 0.7, true, i2s(n_standby_pikmin)
        );
    };
    gui.add_item(standby_amount, "standby_amount");
    
    
    //Group Pikmin amount bubble.
    gui_item* group_bubble = new gui_item();
    group_bubble->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        draw_bitmap(
            bmp_counter_bubble_group,
            center,
            size
        );
    };
    gui.add_item(group_bubble, "group_bubble");
    
    
    //Group Pikmin amount.
    group_amount = new gui_item();
    group_amount->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        size_t n_group_pikmin =
            game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->members.size();
        for(
            size_t l = 0;
            l < game.states.gameplay->mobs.leaders.size();
            ++l
        ) {
            //If this leader is following the current one,
            //then they're not a Pikmin.
            //Subtract them from the group count total.
            if(
                game.states.gameplay->mobs.leaders[l]->following_group ==
                game.states.gameplay->player_info[this->player_id].cur_leader_ptr
            ) {
                n_group_pikmin--;
            }
        }
        
        if(n_group_pikmin != group_count_nr) {
            group_amount->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            group_count_nr = n_group_pikmin;
        }
        
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            center,
            point(1.0f, 1.0f) + group_amount->get_juice_value(),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
            size * 0.7, true,
            i2s(n_group_pikmin)
        );
    };
    gui.add_item(group_amount, "group_amount");
    
    
    //Field Pikmin amount bubble.
    gui_item* field_bubble = new gui_item();
    field_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap(
            bmp_counter_bubble_field,
            center,
            size
        );
    };
    gui.add_item(field_bubble, "field_bubble");
    
    
    //Field Pikmin amount.
    field_amount = new gui_item();
    field_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_field_pikmin = game.states.gameplay->mobs.pikmin_list.size();
        if(n_field_pikmin != field_count_nr) {
            field_amount->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            field_count_nr = n_field_pikmin;
        }
        
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            center,
            point(1.0f, 1.0f) + field_amount->get_juice_value(),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
            size * 0.7, true,
            i2s(n_field_pikmin)
        );
    };
    gui.add_item(field_amount, "field_amount");
    
    
    //Total Pikmin amount bubble.
    gui_item* total_bubble = new gui_item();
    total_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap(
            bmp_counter_bubble_total,
            center,
            size
        );
    };
    gui.add_item(total_bubble, "total_bubble");
    
    
    //Total Pikmin amount.
    total_amount = new gui_item();
    total_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_total_pikmin = game.states.gameplay->get_total_pikmin_amount();
        
        if(n_total_pikmin != total_count_nr) {
            total_amount->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            total_count_nr = n_total_pikmin;
        }
        
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            center,
            point(1.0f, 1.0f) + total_amount->get_juice_value(),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
            size * 0.7, true,
            i2s(n_total_pikmin)
        );
    };
    gui.add_item(total_amount, "total_amount");
    
    
    //Pikmin counter "x".
    gui_item* counters_x = new gui_item();
    counters_x->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter,
            map_alpha(this->standby_items_opacity * 255),
            center, ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, "x"
        );
    };
    gui.add_item(counters_x, "counters_x");
    
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; ++s) {
        gui_item* counter_slash = new gui_item();
        counter_slash->on_draw =
        [this] (const point & center, const point & size) {
            if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
            draw_compressed_text(
                game.fonts.counter, COLOR_WHITE,
                center, ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, "/"
            );
        };
        gui.add_item(counter_slash, "counters_slash_" + i2s(s + 1));
    }
    
    
    //Spray 1 icon.
    gui_item* spray_1_icon = new gui_item();
    spray_1_icon->on_draw =
    [this] (const point & center, const point & size) {
        draw_spray_icon(BUBBLE_CURRENT);
    };
    gui.add_item(spray_1_icon, "spray_1_icon");
    spray_icon_mgr.register_bubble(BUBBLE_CURRENT, spray_1_icon);
    
    
    //Spray 1 amount.
    spray_1_amount = new gui_item();
    spray_1_amount->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() > 2) {
            top_spray_idx = game.states.gameplay->player_info[this->player_id].selected_spray;
        } else if(!game.spray_types.empty() && game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        }
        if(top_spray_idx == INVALID) return;
        
        draw_compressed_scaled_text(
            game.fonts.counter,
            map_alpha(this->spray_items_opacity * 255),
            point(center.x - size.x / 2.0, center.y),
            point(1.0f, 1.0f) + spray_1_amount->get_juice_value(),
            ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER, size, true,
            "x" +
            i2s(game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[top_spray_idx].nr_sprays)
        );
    };
    gui.add_item(spray_1_amount, "spray_1_amount");
    
    
    //Spray 1 input.
    gui_item* spray_1_input = new gui_item();
    spray_1_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() > 2) {
            top_spray_idx = game.states.gameplay->player_info[this->player_id].selected_spray;
        } else if(!game.spray_types.empty() && game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        }
        if(top_spray_idx == INVALID) return;
        if(game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[top_spray_idx].nr_sprays == 0) {
            return;
        }
        
        player_input i;
        if(game.spray_types.size() > 2) {
            i = game.controls.find_bind(PLAYER_ACTION_USE_SPRAY).input;
        } else if(!game.spray_types.empty() && game.spray_types.size() <= 2) {
            i = game.controls.find_bind(PLAYER_ACTION_USE_SPRAY_1).input;
        }
        if(i.type == INPUT_TYPE_NONE) return;
        
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->spray_items_opacity * 255
        );
    };
    gui.add_item(spray_1_input, "spray_1_input");
    
    
    //Spray 2 icon.
    gui_item* spray_2_icon = new gui_item();
    spray_2_icon->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[bottom_spray_idx].bmp_spray, center, size, true,
            0.0f,
            map_alpha(this->spray_items_opacity * 255)
        );
    };
    gui.add_item(spray_2_icon, "spray_2_icon");
    
    
    //Spray 2 amount.
    spray_2_amount = new gui_item();
    spray_2_amount->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        draw_compressed_scaled_text(
            game.fonts.counter,
            map_alpha(this->spray_items_opacity * 255),
            point(center.x - size.x / 2.0, center.y),
            point(1.0f, 1.0f) + spray_2_amount->get_juice_value(),
            ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER, size, true,
            "x" +
            i2s(game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[bottom_spray_idx].nr_sprays)
        );
    };
    gui.add_item(spray_2_amount, "spray_2_amount");
    
    
    //Spray 2 input.
    gui_item* spray_2_input = new gui_item();
    spray_2_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        if(game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[bottom_spray_idx].nr_sprays == 0) {
            return;
        }
        
        player_input i;
        if(game.spray_types.size() == 2) {
            i = game.controls.find_bind(PLAYER_ACTION_USE_SPRAY_2).input;
        }
        if(i.type == INPUT_TYPE_NONE) return;
        
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->spray_items_opacity * 255
        );
    };
    gui.add_item(spray_2_input, "spray_2_input");
    
    
    //Previous spray icon.
    gui_item* prev_spray_icon = new gui_item();
    prev_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        draw_spray_icon(BUBBLE_PREVIOUS);
    };
    gui.add_item(prev_spray_icon, "spray_prev_icon");
    spray_icon_mgr.register_bubble(BUBBLE_PREVIOUS, prev_spray_icon);
    
    
    //Previous spray input.
    gui_item* prev_spray_input = new gui_item();
    prev_spray_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t prev_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            prev_spray_idx =
                sum_and_wrap(
                    (int) game.states.gameplay->player_info[this->player_id].selected_spray,
                    -1,
                    (int) game.spray_types.size()
                );
        }
        if(prev_spray_idx == INVALID) return;
        
        player_input i;
        if(game.spray_types.size() >= 3) {
            i = game.controls.find_bind(PLAYER_ACTION_PREV_SPRAY).input;
        }
        if(i.type == INPUT_TYPE_NONE) return;
        
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->spray_items_opacity * 255
        );
    };
    gui.add_item(prev_spray_input, "spray_prev_input");
    
    
    //Next spray icon.
    gui_item* next_spray_icon = new gui_item();
    next_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        draw_spray_icon(BUBBLE_NEXT);
    };
    gui.add_item(next_spray_icon, "spray_next_icon");
    spray_icon_mgr.register_bubble(BUBBLE_NEXT, next_spray_icon);
    
    
    //Next spray input.
    gui_item* next_spray_input = new gui_item();
    next_spray_input->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_input_icons) return;
        if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
        
        size_t next_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            next_spray_idx =
                sum_and_wrap(
                    (int) game.states.gameplay->player_info[this->player_id].selected_spray,
                    1,
                    (int) game.spray_types.size()
                );
        }
        if(next_spray_idx == INVALID) return;
        
        player_input i;
        if(game.spray_types.size() >= 3) {
            i = game.controls.find_bind(PLAYER_ACTION_NEXT_SPRAY).input;
        }
        if(i.type == INPUT_TYPE_NONE) return;
        
        draw_player_input_icon(
            game.fonts.slim, i, true, center, size,
            this->spray_items_opacity * 255
        );
    };
    gui.add_item(next_spray_input, "spray_next_input");
    
    
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
    
        //Mission goal bubble.
        gui_item* mission_goal_bubble = new gui_item();
        mission_goal_bubble->on_draw =
        [this] (const point & center, const point & size) {
            int cx = 0;
            int cy = 0;
            int cw = 0;
            int ch = 0;
            al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
            set_combined_clipping_rectangles(
                cx, cy, cw, ch,
                center.x - size.x / 2.0f,
                center.y - size.y / 2.0f,
                size.x * game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].goal_indicator_ratio + 1,
                size.y
            );
            draw_filled_rounded_rectangle(
                center, size, 20.0f, al_map_rgba(86, 149, 50, 160)
            );
            set_combined_clipping_rectangles(
                cx, cy, cw, ch,
                center.x - size.x / 2.0f +
                size.x * game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].goal_indicator_ratio,
                center.y - size.y / 2.0f,
                size.x * (1 - game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].goal_indicator_ratio),
                size.y
            );
            draw_filled_rounded_rectangle(
                center, size, 20.0f, al_map_rgba(34, 102, 102, 80)
            );
            al_set_clipping_rectangle(cx, cy, cw, ch);
            draw_textured_box(
                center, size, game.sys_assets.bmp_bubble_box,
                al_map_rgba(255, 255, 255, 200)
            );
        };
        gui.add_item(mission_goal_bubble, "mission_goal_bubble");
        
        
        string goal_cur_label_text =
            game.mission_goals[game.cur_area_data.mission.team_data[0].goal]->
            get_hud_label();
            
        if(!goal_cur_label_text.empty()) {
            //Mission goal current label.
            gui_item* mission_goal_cur_label = new gui_item();
            mission_goal_cur_label->on_draw =
                [this, goal_cur_label_text]
            (const point & center, const point & size) {
                draw_compressed_scaled_text(
                    game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                    center, point(1.0f, 1.0f),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    goal_cur_label_text
                );
            };
            gui.add_item(mission_goal_cur_label, "mission_goal_cur_label");
            
            
            //Mission goal current.
            gui_item* mission_goal_cur = new gui_item();
            mission_goal_cur->on_draw =
                [this, mission_goal_cur]
            (const point & center, const point & size) {
                int value =
                    game.mission_goals[game.cur_area_data.mission.team_data[0].goal]->
                    get_cur_amount(game.states.gameplay);
                string text;
                if(
                    game.cur_area_data.mission.team_data[0].goal ==
                    MISSION_GOAL_TIMED_SURVIVAL
                ) {
                    text = time_to_str2(value, ":", "");
                } else {
                    text = i2s(value);
                }
                float juicy_grow_amount =
                    mission_goal_cur->get_juice_value();
                draw_compressed_scaled_text(
                    game.fonts.counter, COLOR_WHITE,
                    center,
                    point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    text
                );
            };
            gui.add_item(mission_goal_cur, "mission_goal_cur");
            game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].mission_goal_cur_text = mission_goal_cur;
            
            
            //Mission goal requirement label.
            gui_item* mission_goal_req_label = new gui_item();
            mission_goal_req_label->on_draw =
            [this] (const point & center, const point & size) {
                draw_compressed_scaled_text(
                    game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                    center, point(1.0f, 1.0f),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    "Goal"
                );
            };
            gui.add_item(mission_goal_req_label, "mission_goal_req_label");
            
            
            //Mission goal requirement.
            gui_item* mission_goal_req = new gui_item();
            mission_goal_req->on_draw =
            [this] (const point & center, const point & size) {
                int value =
                    game.mission_goals[game.cur_area_data.mission.team_data[0].goal]->
                    get_req_amount(game.states.gameplay);
                string text;
                if(
                    game.cur_area_data.mission.team_data[0].goal ==
                    MISSION_GOAL_TIMED_SURVIVAL
                ) {
                    text = time_to_str2(value, ":", "");
                } else {
                    text = i2s(value);
                }
                draw_compressed_scaled_text(
                    game.fonts.counter, COLOR_WHITE,
                    center, point(1.0f, 1.0f),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    text
                );
            };
            gui.add_item(mission_goal_req, "mission_goal_req");
            
            
            //Mission goal slash.
            gui_item* mission_goal_slash = new gui_item();
            mission_goal_slash->on_draw =
            [this] (const point & center, const point & size) {
                draw_compressed_scaled_text(
                    game.fonts.counter, COLOR_WHITE,
                    center, point(1.0f, 1.0f),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    "/"
                );
            };
            gui.add_item(mission_goal_slash, "mission_goal_slash");
            
        } else {
        
            //Mission goal name text.
            gui_item* mission_goal_name = new gui_item();
            mission_goal_name->on_draw =
            [this] (const point & center, const point & size) {
                draw_compressed_scaled_text(
                    game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                    center, point(1.0f, 1.0f),
                    ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                    size, true,
                    game.mission_goals[game.cur_area_data.mission.team_data[0].goal]->
                    get_name()
                );
            };
            gui.add_item(mission_goal_name, "mission_goal_name");
            
        }
        
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.grading_mode == MISSION_GRADING_POINTS &&
        game.cur_area_data.mission.point_hud_data != 0
    ) {
    
        //Mission score bubble.
        gui_item* mission_score_bubble = new gui_item();
        mission_score_bubble->on_draw =
        [this] (const point & center, const point & size) {
            draw_filled_rounded_rectangle(
                center, size, 20.0f, al_map_rgba(86, 149, 50, 160)
            );
            draw_textured_box(
                center, size, game.sys_assets.bmp_bubble_box,
                al_map_rgba(255, 255, 255, 200)
            );
        };
        gui.add_item(mission_score_bubble, "mission_score_bubble");
        
        
        //Mission score "score" label.
        gui_item* mission_score_score_label = new gui_item();
        mission_score_score_label->on_draw =
        [this] (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                point(center.x + size.x / 2.0f, center.y), point(1.0f, 1.0f),
                ALLEGRO_ALIGN_RIGHT, TEXT_VALIGN_CENTER,
                size, true,
                "Score:"
            );
        };
        gui.add_item(mission_score_score_label, "mission_score_score_label");
        
        
        //Mission score points.
        gui_item* mission_score_points = new gui_item();
        mission_score_points->on_draw =
            [this, mission_score_points]
        (const point & center, const point & size) {
            float juicy_grow_amount = mission_score_points->get_juice_value();
            draw_compressed_scaled_text(
                game.fonts.counter, COLOR_WHITE,
                center,
                point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                i2s(game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].mission_score)
            );
        };
        gui.add_item(mission_score_points, "mission_score_points");
        game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].mission_score_cur_text = mission_score_points;
        
        
        //Mission score "points" label.
        gui_item* mission_score_points_label = new gui_item();
        mission_score_points_label->on_draw =
        [this] (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                point(center.x - size.x / 2.0f, center.y), point(1.0f, 1.0f),
                ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER,
                size, true,
                "pts"
            );
        };
        gui.add_item(mission_score_points_label, "mission_score_points_label");
        
        
        //Mission score ruler.
        gui_item* mission_score_ruler = new gui_item();
        mission_score_ruler->on_draw =
        [this] (const point & center, const point & size) {
            //Setup.
            const float ruler_start_value =
                game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator -
                HUD::SCORE_RULER_RANGE / 2.0f;
            const float ruler_end_value =
                game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator +
                HUD::SCORE_RULER_RANGE / 2.0f;
            const float ruler_scale =
                size.x / (float) HUD::SCORE_RULER_RANGE;
            const float ruler_start_x = center.x - size.x / 2.0f;
            const float ruler_end_x = center.x + size.x / 2.0f;
            
            const float seg_limits[] = {
                std::min(ruler_start_value, 0.0f),
                0,
                (float) game.cur_area_data.mission.bronze_req,
                (float) game.cur_area_data.mission.silver_req,
                (float) game.cur_area_data.mission.gold_req,
                (float) game.cur_area_data.mission.platinum_req,
                std::max(
                    ruler_end_value,
                    (float) game.cur_area_data.mission.platinum_req
                )
            };
            const ALLEGRO_COLOR seg_colors_top[] = {
                al_map_rgba(152, 160, 152, 128), //Negatives.
                al_map_rgb(158, 166, 158),       //No medal.
                al_map_rgb(203, 117, 37),        //Bronze.
                al_map_rgb(223, 227, 209),       //Silver.
                al_map_rgb(235, 209, 59),        //Gold.
                al_map_rgb(158, 222, 211)        //Platinum.
            };
            const ALLEGRO_COLOR seg_colors_bottom[] = {
                al_map_rgba(152, 160, 152, 128), //Negatives.
                al_map_rgb(119, 128, 118),       //No medal.
                al_map_rgb(131, 52, 18),         //Bronze.
                al_map_rgb(160, 154, 127),       //Silver.
                al_map_rgb(173, 127, 24),        //Gold.
                al_map_rgb(79, 172, 153)         //Platinum.
            };
            ALLEGRO_BITMAP* seg_icons[] = {
                NULL,
                NULL,
                game.sys_assets.bmp_medal_bronze,
                game.sys_assets.bmp_medal_silver,
                game.sys_assets.bmp_medal_gold,
                game.sys_assets.bmp_medal_platinum
            };
            
            //Draw each segment (no medal, bronze, etc.).
            for(int s = 0; s < 6; ++s) {
                float seg_start_value = seg_limits[s];
                float seg_end_value = seg_limits[s + 1];
                if(seg_end_value < ruler_start_value) continue;
                if(seg_start_value > ruler_end_value) continue;
                float seg_start_x =
                    center.x -
                    (game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator - seg_start_value) *
                    ruler_scale;
                float seg_end_x =
                    center.x +
                    (seg_end_value - game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator) *
                    ruler_scale;
                seg_start_x = std::max(seg_start_x, ruler_start_x);
                seg_end_x = std::min(ruler_end_x, seg_end_x);
                
                ALLEGRO_VERTEX vertexes[4];
                for(unsigned char v = 0; v < 4; ++v) {
                    vertexes[v].z = 0.0f;
                }
                vertexes[0].x = seg_start_x;
                vertexes[0].y = center.y - size.y / 2.0f;
                vertexes[0].color = seg_colors_top[s];
                vertexes[1].x = seg_start_x;
                vertexes[1].y = center.y + size.y / 2.0f;
                vertexes[1].color = seg_colors_bottom[s];
                vertexes[2].x = seg_end_x;
                vertexes[2].y = center.y + size.y / 2.0f;
                vertexes[2].color = seg_colors_bottom[s];
                vertexes[3].x = seg_end_x;
                vertexes[3].y = center.y - size.y / 2.0f;
                vertexes[3].color = seg_colors_top[s];
                al_draw_prim(
                    vertexes, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            //Draw the markings.
            for(
                float m = floor(ruler_start_value / 25.0f) * 25.0f;
                m <= ruler_end_value;
                m += 25.0f
            ) {
                if(m < 0.0f || m < ruler_start_value) continue;
                float marking_x =
                    center.x -
                    (game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator - m) *
                    ruler_scale;
                float marking_length =
                    fmod(m, 100) == 0 ?
                    size.y * 0.7f :
                    fmod(m, 50) == 0 ?
                    size.y * 0.4f :
                    size.y * 0.1f;
                al_draw_filled_triangle(
                    marking_x,
                    center.y - size.y / 2.0f + marking_length,
                    marking_x + 2.0f,
                    center.y - size.y / 2.0f,
                    marking_x - 2.0f,
                    center.y - size.y / 2.0f,
                    al_map_rgb(100, 110, 180)
                );
            }
            
            //Draw the medal icons.
            int cur_seg = 0;
            int last_passed_seg = 0;
            float cur_medal_scale =
                HUD::MEDAL_ICON_SCALE +
                sin(
                    game.states.gameplay->area_time_passed *
                    HUD::MEDAL_ICON_SCALE_TIME_MULT
                ) *
                HUD::MEDAL_ICON_SCALE_MULT;
            for(int s = 0; s < 6; ++s) {
                float seg_start_value = seg_limits[s];
                if(seg_start_value <= game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator) {
                    cur_seg = s;
                }
                if(seg_start_value <= ruler_start_value) {
                    last_passed_seg = s;
                }
            }
            for(int s = 0; s < 6; ++s) {
                if(!seg_icons[s]) continue;
                float seg_start_value = seg_limits[s];
                if(seg_start_value < ruler_start_value) continue;
                float seg_start_x =
                    center.x -
                    (game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].score_indicator - seg_start_value) *
                    ruler_scale;
                float icon_x = seg_start_x;
                unsigned char icon_alpha = 255;
                float icon_scale = HUD::MEDAL_ICON_SCALE;
                if(cur_seg == s) {
                    icon_scale = cur_medal_scale;
                }
                if(seg_start_value > ruler_end_value) {
                    icon_x = ruler_end_x;
                    icon_alpha = 128;
                }
                draw_bitmap(
                    seg_icons[s],
                    point(icon_x, center.y),
                    point(-1, size.y * icon_scale),
                    0,
                    al_map_rgba(255, 255, 255, icon_alpha)
                );
                if(seg_start_value > ruler_end_value) {
                    //If we found the first icon that goes past the ruler's end,
                    //then we shouldn't draw the other ones that come after.
                    break;
                }
            }
            if(seg_icons[last_passed_seg] && last_passed_seg == cur_seg) {
                draw_bitmap(
                    seg_icons[last_passed_seg],
                    point(ruler_start_x, center.y),
                    point(-1, size.y * cur_medal_scale)
                );
            }
            
            //Draw the flapper.
            al_draw_filled_triangle(
                center.x, center.y + size.y / 2.0f,
                center.x, center.y,
                center.x + (size.y * 0.4), center.y + size.y / 2.0f,
                al_map_rgb(105, 161, 105)
            );
            al_draw_filled_triangle(
                center.x, center.y + size.y / 2.0f,
                center.x, center.y,
                center.x - (size.y * 0.4), center.y + size.y / 2.0f,
                al_map_rgb(124, 191, 124)
            );
        };
        gui.add_item(mission_score_ruler, "mission_score_ruler");
        
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.team_data[0].fail_hud_primary_cond != INVALID
    ) {
        create_mission_fail_cond_items(true);
    }
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.team_data[0].fail_hud_secondary_cond != INVALID
    ) {
        create_mission_fail_cond_items(false);
    }
    
    
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
    loader(bmp_no_pikmin_bubble,       "no_pikmin_bubble");
    loader(bmp_sun,                    "sun");
    
#undef loader
    
    leader_icon_mgr.move_method = HUD_BUBBLE_MOVE_METHOD_CIRCLE;
    leader_icon_mgr.transition_duration = HUD::LEADER_SWAP_JUICE_DURATION;
    leader_health_mgr.move_method = HUD_BUBBLE_MOVE_METHOD_CIRCLE;
    leader_health_mgr.transition_duration = HUD::LEADER_SWAP_JUICE_DURATION;
    standby_icon_mgr.move_method = HUD_BUBBLE_MOVE_METHOD_STRAIGHT;
    standby_icon_mgr.transition_duration = HUD::STANDBY_SWAP_JUICE_DURATION;
    spray_icon_mgr.move_method = HUD_BUBBLE_MOVE_METHOD_STRAIGHT;
    spray_icon_mgr.transition_duration = HUD::SPRAY_SWAP_JUICE_DURATION;
    
}


/* ----------------------------------------------------------------------------
 * Destructor for the HUD struct.
 */
hud_struct::~hud_struct() {
    game.bitmaps.detach(bmp_bubble);
    game.bitmaps.detach(bmp_counter_bubble_field);
    game.bitmaps.detach(bmp_counter_bubble_group);
    game.bitmaps.detach(bmp_counter_bubble_standby);
    game.bitmaps.detach(bmp_counter_bubble_total);
    game.bitmaps.detach(bmp_day_bubble);
    game.bitmaps.detach(bmp_distant_pikmin_marker);
    game.bitmaps.detach(bmp_hard_bubble);
    game.bitmaps.detach(bmp_no_pikmin_bubble);
    game.bitmaps.detach(bmp_sun);
    
}


/* ----------------------------------------------------------------------------
 * Creates either the primary or the secondary mission fail condition HUD items.
 * primary:
 *   True if it's the primary HUD item, false if it's the secondary.
 */
void hud_struct::create_mission_fail_cond_items(const bool primary) {
    MISSION_FAIL_CONDITIONS cond =
        primary ?
        (MISSION_FAIL_CONDITIONS)
        game.cur_area_data.mission.team_data[0].fail_hud_primary_cond :
        (MISSION_FAIL_CONDITIONS)
        game.cur_area_data.mission.team_data[0].fail_hud_secondary_cond;
        
    //Mission fail condition bubble.
    gui_item* mission_fail_bubble = new gui_item();
    mission_fail_bubble->on_draw =
    [this, primary] (const point & center, const point & size) {
        int cx = 0;
        int cy = 0;
        int cw = 0;
        int ch = 0;
        float ratio =
            primary ?
            game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].fail_1_indicator_ratio :
            game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].fail_2_indicator_ratio;
        al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
        set_combined_clipping_rectangles(
            cx, cy, cw, ch,
            center.x - size.x / 2.0f,
            center.y - size.y / 2.0f,
            size.x * ratio + 1,
            size.y
        );
        draw_filled_rounded_rectangle(
            center, size, 20.0f, al_map_rgba(149, 80, 50, 160)
        );
        set_combined_clipping_rectangles(
            cx, cy, cw, ch,
            center.x - size.x / 2.0f +
            size.x * ratio,
            center.y - size.y / 2.0f,
            size.x * (1 - ratio),
            size.y
        );
        draw_filled_rounded_rectangle(
            center, size, 20.0f, al_map_rgba(149, 130, 50, 160)
        );
        al_set_clipping_rectangle(cx, cy, cw, ch);
        draw_textured_box(
            center, size, game.sys_assets.bmp_bubble_box,
            al_map_rgba(255, 255, 255, 200)
        );
    };
    gui.add_item(
        mission_fail_bubble,
        primary ?
        "mission_fail_1_bubble" :
        "mission_fail_2_bubble"
    );
    
    
    if(game.mission_fail_conds[cond]->has_hud_content()) {
    
        //Mission fail condition current label.
        gui_item* mission_fail_cur_label = new gui_item();
        mission_fail_cur_label->on_draw =
        [this, cond] (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                center, point(1.0f, 1.0f),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                game.mission_fail_conds[cond]->
                get_hud_label(game.states.gameplay)
            );
        };
        gui.add_item(
            mission_fail_cur_label,
            primary ?
            "mission_fail_1_cur_label" :
            "mission_fail_2_cur_label"
        );
        
        
        //Mission fail condition current.
        gui_item* mission_fail_cur = new gui_item();
        mission_fail_cur->on_draw =
            [this, cond, mission_fail_cur]
        (const point & center, const point & size) {
            int value =
                game.mission_fail_conds[cond]->
                get_cur_amount(game.states.gameplay);
            string text;
            if(cond == MISSION_FAIL_COND_TIME_LIMIT) {
                text = time_to_str2(value, ":", "");
            } else {
                text = i2s(value);
            }
            float juicy_grow_amount = mission_fail_cur->get_juice_value();
            draw_compressed_scaled_text(
                game.fonts.counter, COLOR_WHITE,
                center,
                point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                text
            );
        };
        gui.add_item(
            mission_fail_cur,
            primary ?
            "mission_fail_1_cur" :
            "mission_fail_2_cur"
        );
        if(primary) {
            game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].mission_fail_1_cur_text = mission_fail_cur;
        } else {
            game.states.gameplay->mission_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].mission_fail_2_cur_text = mission_fail_cur;
        }
        
        
        //Mission fail condition requirement label.
        gui_item* mission_fail_req_label = new gui_item();
        mission_fail_req_label->on_draw =
            [this]
        (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                center, point(1.0f, 1.0f),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                "Fail"
            );
        };
        gui.add_item(
            mission_fail_req_label,
            primary ?
            "mission_fail_1_req_label" :
            "mission_fail_2_req_label"
        );
        
        
        //Mission fail condition requirement.
        gui_item* mission_fail_req = new gui_item();
        mission_fail_req->on_draw =
        [this, cond] (const point & center, const point & size) {
            int value =
                game.mission_fail_conds[cond]->
                get_req_amount(game.states.gameplay);
            string text;
            if(cond == MISSION_FAIL_COND_TIME_LIMIT) {
                text = time_to_str2(value, ":", "");
            } else {
                text = i2s(value);
            }
            draw_compressed_scaled_text(
                game.fonts.counter, COLOR_WHITE,
                center, point(1.0f, 1.0f),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                text
            );
        };
        gui.add_item(
            mission_fail_req,
            primary ?
            "mission_fail_1_req" :
            "mission_fail_2_req"
        );
        
        
        //Mission primary fail condition slash.
        gui_item* mission_fail_slash = new gui_item();
        mission_fail_slash->on_draw =
        [this] (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.counter, COLOR_WHITE,
                center, point(1.0f, 1.0f),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                "/"
            );
        };
        gui.add_item(
            mission_fail_slash,
            primary ?
            "mission_fail_1_slash" :
            "mission_fail_2_slash"
        );
        
    } else {
    
        //Mission fail condition name text.
        gui_item* mission_fail_name = new gui_item();
        mission_fail_name->on_draw =
        [this, cond] (const point & center, const point & size) {
            draw_compressed_scaled_text(
                game.fonts.standard, al_map_rgba(255, 255, 255, 128),
                center, point(1.0f, 1.0f),
                ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER,
                size, true,
                "Fail: " +
                game.mission_fail_conds[cond]->get_name()
            );
        };
        gui.add_item(
            mission_fail_name,
            primary ?
            "mission_fail_1_name" :
            "mission_fail_2_name"
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Code to draw a spray icon with. This does not apply to the second spray.
 * which:
 *   Which spray icon to draw -- the previous type's, the current type's,
 *   or the next type's.
 */
void hud_struct::draw_spray_icon(BUBBLE_RELATIONS which) {
    if(!game.states.gameplay->player_info[this->player_id].cur_leader_ptr) return;
    
    point final_center;
    point final_size;
    ALLEGRO_BITMAP* icon;
    this->spray_icon_mgr.get_drawing_info(
        which, &icon, &final_center, &final_size
    );
    
    if(!icon) return;
    draw_bitmap_in_box(
        icon, final_center, final_size, true, 0.0f,
        map_alpha(this->spray_items_opacity * 255)
    );
}


/* ----------------------------------------------------------------------------
 * Code to draw a standby icon with.
 * which:
 *   Which standby icon to draw -- the previous type's, the current type's,
 *   or the next type's.
 */
void hud_struct::draw_standby_icon(BUBBLE_RELATIONS which) {
    point final_center;
    point final_size;
    ALLEGRO_BITMAP* icon;
    this->standby_icon_mgr.get_drawing_info(
        which, &icon, &final_center, &final_size
    );
    
    if(!icon) return;
    
    ALLEGRO_COLOR color =
        map_alpha(this->standby_items_opacity * 255);
        
    draw_bitmap_in_box(icon, final_center, final_size * 0.8, true, 0.0f, color);
    
    if(
        game.states.gameplay->player_info[this->player_id].closest_group_member_distant &&
        which == BUBBLE_CURRENT
    ) {
        draw_bitmap_in_box(
            bmp_distant_pikmin_marker,
            final_center,
            final_size * 0.8,
            true,
            0.0f, color
        );
    }
    
    draw_bitmap_in_box(bmp_bubble, final_center, final_size, true, 0.0f, color);
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void hud_struct::tick(const float delta_t) {
    //Update leader bubbles.
    for(size_t l = 0; l < 3; ++l) {
        leader* l_ptr = NULL;
        if(l < game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].available_leaders.size()) {
            size_t l_idx =
                (size_t) sum_and_wrap(
                    (int) game.states.gameplay->player_info[this->player_id].cur_leader_nr,
                    (int) l,
                    (int) game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].available_leaders.size()
                );
            l_ptr = game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].available_leaders[l_idx];
        }
        
        leader_icon_bubble icon;
        icon.bmp = NULL;
        icon.color = COLOR_EMPTY;
        if(l_ptr) {
            icon.bmp = l_ptr->lea_type->bmp_icon;
            icon.color = l_ptr->lea_type->main_color;
        }
        
        leader_icon_mgr.update(l, l_ptr, icon);
        
        leader_health_bubble health;
        health.ratio = 0.0f;
        health.caution_timer = 0.0f;
        if(l_ptr) {
            health.ratio = l_ptr->health_wheel_visible_ratio;
            health.caution_timer = l_ptr->health_wheel_caution_timer;
        }
        leader_health_mgr.update(l, l_ptr, health);
    }
    leader_icon_mgr.tick(delta_t);
    leader_health_mgr.tick(delta_t);
    
    //Update standby bubbles.
    for(unsigned char s = 0; s < 3; ++s) {
    
        ALLEGRO_BITMAP* icon = NULL;
        leader* cur_leader_ptr = game.states.gameplay->player_info[this->player_id].cur_leader_ptr;
        mob* member = game.states.gameplay->player_info[this->player_id].closest_group_member[s];
        subgroup_type* type = NULL;
        
        if(cur_leader_ptr) {
            switch(s) {
            case BUBBLE_PREVIOUS: {
                subgroup_type* prev_type;
                cur_leader_ptr->group->get_next_standby_type(true, &prev_type);
                subgroup_type* next_type;
                cur_leader_ptr->group->get_next_standby_type(false, &next_type);
                if(
                    prev_type != cur_leader_ptr->group->cur_standby_type &&
                    prev_type != next_type
                ) {
                    type = prev_type;
                }
                break;
            }
            case BUBBLE_CURRENT: {
                type = cur_leader_ptr->group->cur_standby_type;
                break;
            }
            case BUBBLE_NEXT: {
                subgroup_type* next_type;
                cur_leader_ptr->group->get_next_standby_type(false, &next_type);
                if(next_type != cur_leader_ptr->group->cur_standby_type) {
                    type = next_type;
                }
                break;
            }
            }
        }
        
        if(cur_leader_ptr && type && member) {
            SUBGROUP_TYPE_CATEGORIES cat = type->get_category();
            
            switch(cat) {
            case SUBGROUP_TYPE_CATEGORY_LEADER: {
                leader* l_ptr = dynamic_cast<leader*>(member);
                icon = l_ptr->lea_type->bmp_icon;
                break;
            } default: {
                icon = type->get_icon();
                break;
            }
            }
        }
        
        if(!icon && s == BUBBLE_CURRENT) {
            icon = bmp_no_pikmin_bubble;
        }
        
        standby_icon_mgr.update(s, type, icon);
    }
    standby_icon_mgr.tick(delta_t);
    
    //Update spray bubbles.
    size_t top_spray_idx = INVALID;
    if(game.spray_types.size() > 2) {
        top_spray_idx = game.states.gameplay->player_info[this->player_id].selected_spray;
    } else if(!game.spray_types.empty() && game.spray_types.size() <= 2) {
        top_spray_idx = 0;
    }
    spray_icon_mgr.update(
        BUBBLE_CURRENT,
        top_spray_idx == INVALID ? NULL :
        &game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[top_spray_idx],
        top_spray_idx == INVALID ? NULL :
        game.spray_types[top_spray_idx].bmp_spray
    );
    
    size_t prev_spray_idx = INVALID;
    if(game.spray_types.size() >= 3) {
        prev_spray_idx =
            sum_and_wrap(
                (int) game.states.gameplay->player_info[this->player_id].selected_spray,
                -1,
                (int) game.spray_types.size()
            );
    }
    spray_icon_mgr.update(
        BUBBLE_PREVIOUS,
        prev_spray_idx == INVALID ? NULL :
        &game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[prev_spray_idx],
        prev_spray_idx == INVALID ? NULL :
        game.spray_types[prev_spray_idx].bmp_spray
    );
    
    size_t next_spray_idx = INVALID;
    if(game.spray_types.size() >= 3) {
        next_spray_idx =
            sum_and_wrap(
                (int) game.states.gameplay->player_info[this->player_id].selected_spray,
                1,
                (int) game.spray_types.size()
            );
    }
    spray_icon_mgr.update(
        BUBBLE_NEXT,
        next_spray_idx == INVALID ? NULL :
        &game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[next_spray_idx],
        next_spray_idx == INVALID ? NULL :
        game.spray_types[next_spray_idx].bmp_spray
    );
    
    spray_icon_mgr.tick(delta_t);
    
    //Update the standby items opacity.
    if(
        !game.states.gameplay->player_info[this->player_id].cur_leader_ptr ||
        game.states.gameplay->player_info[this->player_id].cur_leader_ptr->group->members.empty()
    ) {
        if(standby_items_fade_timer > 0.0f) {
            standby_items_fade_timer -= delta_t;
        } else {
            standby_items_opacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * delta_t;
        }
    } else {
        standby_items_fade_timer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        standby_items_opacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * delta_t;
    }
    standby_items_opacity = clamp(0.0f, standby_items_opacity, 1.0f);
    
    //Update the spray items opacity.
    size_t total_sprays = 0;
    for(size_t s = 0; s < game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats.size(); ++s) {
        total_sprays +=
            game.states.gameplay->team_info[game.states.gameplay->player_info[this->player_id].team-MOB_TEAM_PLAYER_1].spray_stats[s].nr_sprays;
    }
    if(total_sprays == 0) {
        if(spray_items_fade_timer > 0.0f) {
            spray_items_fade_timer -= delta_t;
        } else {
            spray_items_opacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * delta_t;
        }
    } else {
        spray_items_fade_timer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        spray_items_opacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * delta_t;
    }
    spray_items_opacity = clamp(0.0f, spray_items_opacity, 1.0f);
    
    //Tick the GUI items proper.
    gui.tick(game.delta_t);
}
