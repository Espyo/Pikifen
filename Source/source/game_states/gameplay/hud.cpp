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
#include "../../utils/string_utils.h"
#include "gameplay.h"


namespace HUD {
//The Sun Meter's sun spins these many radians per second.
const float SUN_METER_SUN_SPIN_SPEED = 0.5f;
}

//Path to the GUI information file.
const string hud_struct::HUD_FILE_NAME =
    GUI_FOLDER_PATH + "/Gameplay.txt";
//How long the leader swap juice animation lasts for.
const float hud_struct::LEADER_SWAP_JUICE_DURATION = 0.7f;
//How long the standby swap juice animation lasts for.
const float hud_struct::STANDBY_SWAP_JUICE_DURATION = 0.5f;


/* ----------------------------------------------------------------------------
 * Creates a new HUD structure instance.
 */
hud_struct::hud_struct() :
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
    standby_icon_mgr(&gui),
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
    
    data_node hud_file_node(HUD_FILE_NAME);
    
    gui.register_coords("time",                  50, 10, 70, 10);
    gui.register_coords("day_bubble",             0,  0,  0,  0);
    gui.register_coords("day_number",             0,  0,  0,  0);
    gui.register_coords("leader_1_icon",          7, 90,  8, 10);
    gui.register_coords("leader_2_icon",          6, 80,  5,  9);
    gui.register_coords("leader_3_icon",          6, 72,  5,  9);
    gui.register_coords("leader_1_health",       16, 90,  8, 10);
    gui.register_coords("leader_2_health",       12, 80,  5,  9);
    gui.register_coords("leader_3_health",       12, 72,  5,  9);
    gui.register_coords("leader_next_button",     4, 83,  3,  3);
    gui.register_coords("standby_icon",          50, 91,  8, 10);
    gui.register_coords("standby_amount",        50, 96, 15,  8);
    gui.register_coords("standby_bubble",         0,  0,  0,  0);
    gui.register_coords("standby_maturity_icon", 54, 88,  4,  8);
    gui.register_coords("standby_next_icon",     58, 93,  6,  8);
    gui.register_coords("standby_next_button",   60, 96,  3,  3);
    gui.register_coords("standby_prev_icon",     42, 93,  6,  8);
    gui.register_coords("standby_prev_button",   40, 96,  3,  3);
    gui.register_coords("group_amount",          73, 91, 15, 14);
    gui.register_coords("group_bubble",          73, 91, 15, 14);
    gui.register_coords("field_amount",          91, 91, 15, 14);
    gui.register_coords("field_bubble",          91, 91, 15, 14);
    gui.register_coords("total_amount",           0,  0,  0,  0);
    gui.register_coords("total_bubble",           0,  0,  0,  0);
    gui.register_coords("counters_x",             0,  0,  0,  0);
    gui.register_coords("counters_slash_1",      82, 91,  4,  8);
    gui.register_coords("counters_slash_2",       0,  0,  0,  0);
    gui.register_coords("counters_slash_3",       0,  0,  0,  0);
    gui.register_coords("spray_1_icon",           6, 36,  4,  7);
    gui.register_coords("spray_1_amount",        13, 37, 10,  5);
    gui.register_coords("spray_1_button",         4, 39,  3,  3);
    gui.register_coords("spray_2_icon",           6, 52,  4,  7);
    gui.register_coords("spray_2_amount",        13, 53, 10,  5);
    gui.register_coords("spray_2_button",         4, 55,  3,  3);
    gui.register_coords("spray_prev_icon",        6, 52,  3,  5);
    gui.register_coords("spray_prev_button",      6, 47,  4,  4);
    gui.register_coords("spray_next_icon",       13, 52,  3,  5);
    gui.register_coords("spray_next_button",     13, 47,  4,  4);
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
            game.states.gameplay->hud->leader_icon_mgr.get_drawing_info(
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
                final_center, final_size
            );
            draw_bitmap_in_box(
                bmp_bubble,
                final_center, final_size
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
            game.states.gameplay->hud->leader_health_mgr.get_drawing_info(
                l, &health, &final_center, &final_size
            );
            
            if(health.ratio <= 0.0f) return;
            
            if(health.caution_timer > 0.0f) {
                float caution_ring_scale =
                    interpolate_number(
                        health.caution_timer,
                        0.0f, leader::HEALTH_CAUTION_RING_DURATION,
                        1.0f, 2.0f
                    );
                unsigned char caution_ring_alpha =
                    health.caution_timer <
                    leader::HEALTH_CAUTION_RING_DURATION / 2.0f ?
                    interpolate_number(
                        health.caution_timer,
                        0.0f, leader::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        0.0f, 192
                    ) :
                    interpolate_number(
                        health.caution_timer,
                        leader::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        leader::HEALTH_CAUTION_RING_DURATION,
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
                final_size
            );
        };
        gui.add_item(leader_health, "leader_" + i2s(l + 1) + "_health");
        leader_health_mgr.register_bubble(l, leader_health);
        
    }
    
    
    //Next leader button.
    gui_item* leader_next_button = new gui_item();
    leader_next_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        if(game.states.gameplay->available_leaders.size() == 1) return;
        control_info* c = find_control(BUTTON_NEXT_LEADER);
        if(!c) return;
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(leader_next_button, "leader_next_button");
    
    
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
            game.config.day_minutes_per_irl_sec * delta_t;
        float post_tick_day_minutes =
            game.states.gameplay->day_minutes;
        float checkpoints[3] = {0.25f, 0.50f, 0.75f};
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
        draw_bitmap_in_box(bmp_day_bubble, center, size);
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
        game.states.gameplay->hud->draw_standby_icon(STANDBY_TYPE_CURRENT);
    };
    gui.add_item(standby_icon, "standby_icon");
    standby_icon_mgr.register_bubble(STANDBY_TYPE_CURRENT, standby_icon);
    
    
    //Next standby subgroup icon.
    gui_item* standby_next_icon = new gui_item();
    standby_next_icon->on_draw =
    [this] (const point & center, const point & size) {
        game.states.gameplay->hud->draw_standby_icon(STANDBY_TYPE_NEXT);
    };
    gui.add_item(standby_next_icon, "standby_next_icon");
    standby_icon_mgr.register_bubble(STANDBY_TYPE_NEXT, standby_next_icon);
    
    
    //Next standby subgroup button.
    gui_item* standby_next_button = new gui_item();
    standby_next_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        subgroup_type* next_type;
        game.states.gameplay->cur_leader_ptr->group->get_next_standby_type(
            false, &next_type
        );
        if(
            next_type ==
            game.states.gameplay->cur_leader_ptr->group->cur_standby_type
        ) {
            return;
        }
        control_info* c = find_control(BUTTON_NEXT_TYPE);
        if(!c) return;
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(standby_next_button, "standby_next_button");
    
    
    //Previous standby subgroup icon.
    gui_item* standby_prev_icon = new gui_item();
    standby_prev_icon->on_draw =
    [this] (const point & center, const point & size) {
        game.states.gameplay->hud->draw_standby_icon(STANDBY_TYPE_PREVIOUS);
    };
    gui.add_item(standby_prev_icon, "standby_prev_icon");
    standby_icon_mgr.register_bubble(STANDBY_TYPE_PREVIOUS, standby_prev_icon);
    
    
    //Previous standby subgroup button.
    gui_item* standby_prev_button = new gui_item();
    standby_prev_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        subgroup_type* prev_type;
        game.states.gameplay->cur_leader_ptr->group->get_next_standby_type(
            true, &prev_type
        );
        subgroup_type* next_type;
        game.states.gameplay->cur_leader_ptr->group->get_next_standby_type(
            false, &next_type
        );
        if(
            prev_type ==
            game.states.gameplay->cur_leader_ptr->group->cur_standby_type ||
            prev_type == next_type
        ) {
            return;
        }
        control_info* c = find_control(BUTTON_PREV_TYPE);
        if(!c) return;
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(standby_prev_button, "standby_prev_button");
    
    
    //Standby group member maturity.
    gui_item* standby_maturity_icon = new gui_item();
    standby_maturity_icon->on_draw =
    [this] (const point & center, const point & size) {
        //Standby group member preparations.
        ALLEGRO_BITMAP* standby_mat_bmp = NULL;
        leader* l_ptr = game.states.gameplay->cur_leader_ptr;
        if(
            l_ptr &&
            l_ptr->group->cur_standby_type &&
            game.states.gameplay->closest_group_member[STANDBY_TYPE_CURRENT]
        ) {
            SUBGROUP_TYPE_CATEGORIES c =
                l_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                pikmin* p_ptr =
                    dynamic_cast<pikmin*>(
                        game.states.gameplay->closest_group_member[STANDBY_TYPE_CURRENT]
                    );
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
    gui.add_item(standby_maturity_icon, "standby_maturity_icon");
    
    
    //Standby subgroup member amount bubble.
    gui_item* standby_bubble = new gui_item();
    standby_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap(
            bmp_counter_bubble_standby,
            center,
            size
        );
    };
    gui.add_item(standby_bubble, "standby_bubble");
    
    
    //Standby subgroup member amount.
    standby_amount = new gui_item();
    standby_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_standby_pikmin = 0;
        leader* l_ptr = game.states.gameplay->cur_leader_ptr;
        if(l_ptr->group->cur_standby_type) {
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
            game.fonts.counter, COLOR_WHITE,
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
        size_t n_group_pikmin =
            game.states.gameplay->cur_leader_ptr->group->members.size();
        for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
            //If this leader is following the current one,
            //then they're not a Pikmin.
            //Subtract them from the group count total.
            if(
                game.states.gameplay->mobs.leaders[l]->following_group ==
                game.states.gameplay->cur_leader_ptr
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
        size_t n_total_pikmin = game.states.gameplay->mobs.pikmin_list.size();
        for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); ++o) {
            onion* o_ptr = game.states.gameplay->mobs.onions[o];
            for(
                size_t t = 0;
                t < o_ptr->oni_type->nest->pik_types.size();
                ++t
            ) {
                for(size_t m = 0; m < N_MATURITIES; ++m) {
                    n_total_pikmin += o_ptr->nest->pikmin_inside[t][m];
                }
            }
        }
        
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
            game.fonts.counter, COLOR_WHITE,
            center, ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, "x"
        );
    };
    gui.add_item(counters_x, "counters_x");
    
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; ++s) {
        gui_item* counter_slash = new gui_item();
        counter_slash->on_draw =
        [this] (const point & center, const point & size) {
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
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        } else if(game.spray_types.size() > 0) {
            top_spray_idx = game.states.gameplay->selected_spray;
        }
        if(top_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[top_spray_idx].bmp_spray, center, size
        );
    };
    gui.add_item(spray_1_icon, "spray_1_icon");
    
    
    //Spray 1 amount.
    spray_1_amount = new gui_item();
    spray_1_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t top_spray_idx = INVALID;
        if(game.spray_types.size() <= 2) {
            top_spray_idx = 0;
        } else if(game.spray_types.size() > 0) {
            top_spray_idx = game.states.gameplay->selected_spray;
        }
        if(top_spray_idx == INVALID) return;
        
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x - size.x / 2.0, center.y),
            point(1.0f, 1.0f) + spray_1_amount->get_juice_value(),
            ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER, size, true,
            "x" +
            i2s(game.states.gameplay->spray_stats[top_spray_idx].nr_sprays)
        );
    };
    gui.add_item(spray_1_amount, "spray_1_amount");
    
    
    //Spray 1 button.
    gui_item* spray_1_button = new gui_item();
    spray_1_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        control_info* c = NULL;
        if(game.spray_types.size() <= 2) {
            c = find_control(BUTTON_USE_SPRAY_1);
        } else if(game.spray_types.size() > 0) {
            c = find_control(BUTTON_USE_SPRAY);
        }
        if(!c) return;
        
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(spray_1_button, "spray_1_button");
    
    
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
    gui.add_item(spray_2_icon, "spray_2_icon");
    
    
    //Spray 2 amount.
    spray_2_amount = new gui_item();
    spray_2_amount->on_draw =
    [this] (const point & center, const point & size) {
        size_t bottom_spray_idx = INVALID;
        if(game.spray_types.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x - size.x / 2.0, center.y),
            point(1.0f, 1.0f) + spray_2_amount->get_juice_value(),
            ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER, size, true,
            "x" +
            i2s(game.states.gameplay->spray_stats[bottom_spray_idx].nr_sprays)
        );
    };
    gui.add_item(spray_2_amount, "spray_2_amount");
    
    
    //Spray 2 button.
    gui_item* spray_2_button = new gui_item();
    spray_2_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        control_info* c = NULL;
        if(game.spray_types.size() == 2) {
            c = find_control(BUTTON_USE_SPRAY_2);
        }
        if(!c) return;
        
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(spray_2_button, "spray_2_button");
    
    
    //Previous spray icon.
    gui_item* prev_spray_icon = new gui_item();
    prev_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t prev_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            prev_spray_idx =
                sum_and_wrap(
                    game.states.gameplay->selected_spray,
                    -1,
                    game.spray_types.size()
                );
        }
        if(prev_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[prev_spray_idx].bmp_spray,
            center, size
        );
    };
    gui.add_item(prev_spray_icon, "spray_prev_icon");
    
    
    //Previous spray button.
    gui_item* prev_spray_button = new gui_item();
    prev_spray_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        control_info* c = NULL;
        if(game.spray_types.size() >= 3) {
            c = find_control(BUTTON_PREV_SPRAY);
        }
        if(!c) return;
        
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(prev_spray_button, "spray_prev_button");
    
    
    //Next spray icon.
    gui_item* next_spray_icon = new gui_item();
    next_spray_icon->on_draw =
    [this] (const point & center, const point & size) {
        size_t next_spray_idx = INVALID;
        if(game.spray_types.size() >= 3) {
            next_spray_idx =
                sum_and_wrap(
                    game.states.gameplay->selected_spray,
                    1,
                    game.spray_types.size()
                );
        }
        if(next_spray_idx == INVALID) return;
        
        draw_bitmap_in_box(
            game.spray_types[next_spray_idx].bmp_spray,
            center, size
        );
    };
    gui.add_item(next_spray_icon, "spray_next_icon");
    
    //Next spray button.
    gui_item* next_spray_button = new gui_item();
    next_spray_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        control_info* c = NULL;
        if(game.spray_types.size() >= 3) {
            c = find_control(BUTTON_NEXT_SPRAY);
        }
        if(!c) return;
        
        draw_control_icon(game.fonts.slim, c, true, center, size);
    };
    gui.add_item(next_spray_button, "spray_next_button");
    
    
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
    leader_icon_mgr.transition_duration = LEADER_SWAP_JUICE_DURATION;
    leader_health_mgr.move_method = HUD_BUBBLE_MOVE_METHOD_CIRCLE;
    leader_health_mgr.transition_duration = LEADER_SWAP_JUICE_DURATION;
    standby_icon_mgr.transition_duration = STANDBY_SWAP_JUICE_DURATION;
    
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
 * Code to draw a standby icon with.
 * which:
 *   Which standby icon to draw -- the previous type's, the current type's,
 *   or the next type's.
 */
void hud_struct::draw_standby_icon(STANDBY_TYPE_RELATIONS which) {
    point final_center;
    point final_size;
    ALLEGRO_BITMAP* icon;
    game.states.gameplay->hud->standby_icon_mgr.get_drawing_info(
        which, &icon, &final_center, &final_size
    );
    
    if(!icon) return;
    
    draw_bitmap_in_box(icon, final_center, final_size * 0.8);
    
    if(
        game.states.gameplay->closest_group_member_distant &&
        which == STANDBY_TYPE_CURRENT
    ) {
        draw_bitmap_in_box(
            bmp_distant_pikmin_marker,
            final_center,
            final_size * 0.8
        );
    }
    
    draw_bitmap_in_box(bmp_bubble, final_center, final_size);
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void hud_struct::tick(const float delta_t) {
    //Update bubble managers.
    leader_icon_mgr.tick(delta_t);
    leader_health_mgr.tick(delta_t);
    standby_icon_mgr.tick(delta_t);
    
    //Update leader bubbles.
    for(size_t l = 0; l < 3; ++l) {
        leader* l_ptr = NULL;
        if(l < game.states.gameplay->available_leaders.size()) {
            size_t l_idx =
                (size_t) sum_and_wrap(
                    game.states.gameplay->cur_leader_nr,
                    l,
                    game.states.gameplay->available_leaders.size()
                );
            l_ptr = game.states.gameplay->available_leaders[l_idx];
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
    
    //Update standby bubbles.
    for(unsigned char s = 0; s < 3; ++s) {
    
        ALLEGRO_BITMAP* icon = NULL;
        leader* cur_leader_ptr = game.states.gameplay->cur_leader_ptr;
        mob* member = game.states.gameplay->closest_group_member[s];
        subgroup_type* type = NULL;
        
        switch(s) {
        case STANDBY_TYPE_PREVIOUS: {
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
        case STANDBY_TYPE_CURRENT: {
            type = cur_leader_ptr->group->cur_standby_type;
            break;
        }
        case STANDBY_TYPE_NEXT: {
            subgroup_type* next_type;
            cur_leader_ptr->group->get_next_standby_type(false, &next_type);
            if(next_type != cur_leader_ptr->group->cur_standby_type) {
                type = next_type;
            }
            break;
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
        
        if(!icon && s == STANDBY_TYPE_CURRENT) {
            icon = bmp_no_pikmin_bubble;
        }
        
        standby_icon_mgr.update(s, type, icon);
    }
    
    gui.tick(game.delta_t);
}
