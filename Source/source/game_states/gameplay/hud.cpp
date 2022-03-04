/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-game HUD classes and functions.
 */

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"
#include "gameplay.h"


//Path to the GUI information file.
const string hud_struct::HUD_FILE_NAME = GUI_FOLDER_PATH + "/Gameplay.txt";
//How long the leader swap juice animation lasts for.
const float hud_struct::LEADER_SWAP_JUICE_DURATION = 0.7f;


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
    leader_swap_juice_timer(0.0f),
    spray_1_amount(nullptr),
    spray_2_amount(nullptr),
    standby_count_nr(0),
    standby_count(nullptr),
    group_count_nr(0),
    group_count(nullptr),
    field_count_nr(0),
    field_count(nullptr),
    total_count_nr(0),
    total_count(nullptr) {
    
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
    gui.register_coords("next_leader_button",     4, 83,  3,  3);
    gui.register_coords("pikmin_standby_icon",   30, 91,  8, 10);
    gui.register_coords("pikmin_standby_m_icon", 35, 88,  4,  8);
    gui.register_coords("prev_pikmin_button",    27, 95,  3,  3);
    gui.register_coords("next_pikmin_button",    33, 95,  3,  3);
    gui.register_coords("pikmin_standby_x",      38, 91,  7,  8);
    gui.register_coords("pikmin_standby_nr",     50, 91, 15, 10);
    gui.register_coords("pikmin_group_nr",       73, 91, 15, 14);
    gui.register_coords("pikmin_field_nr",       91, 91, 15, 14);
    gui.register_coords("pikmin_total_nr",        0,  0,  0,  0);
    gui.register_coords("pikmin_slash_1",        82, 91,  4,  8);
    gui.register_coords("pikmin_slash_2",         0,  0,  0,  0);
    gui.register_coords("pikmin_slash_3",         0,  0,  0,  0);
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
            size_t l_idx;
            point final_center;
            point final_size;
            game.states.gameplay->hud->leader_icon_mgr.get_drawing_info(
                l,
                game.states.gameplay->hud->leader_swap_juice_timer /
                LEADER_SWAP_JUICE_DURATION,
                &l_idx, &final_center, &final_size
            );
            
            if(
                l_idx == INVALID ||
                l_idx >= game.states.gameplay->mobs.leaders.size()
            ) {
                return;
            }
            leader* l_ptr = game.states.gameplay->mobs.leaders[l_idx];
            
            al_draw_filled_circle(
                final_center.x, final_center.y,
                std::min(final_size.x, final_size.y) / 2.0f,
                change_alpha(
                    l_ptr->type->main_color,
                    128
                )
            );
            draw_bitmap_in_box(
                l_ptr->lea_type->bmp_icon,
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
            size_t l_idx;
            point final_center;
            point final_size;
            game.states.gameplay->hud->leader_health_mgr.get_drawing_info(
                l,
                game.states.gameplay->hud->leader_swap_juice_timer /
                LEADER_SWAP_JUICE_DURATION,
                &l_idx, &final_center, &final_size
            );
            
            if(
                l_idx == INVALID ||
                l_idx >= game.states.gameplay->mobs.leaders.size()
            ) {
                return;
            }
            leader* l_ptr = game.states.gameplay->mobs.leaders[l_idx];
            
            draw_health(
                final_center,
                l_ptr->health_wheel_visible_ratio, 1.0f,
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
    gui_item* next_leader_button = new gui_item();
    next_leader_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        if(game.states.gameplay->mobs.leaders.size() == 1) return;
        control_info* c = find_control(BUTTON_NEXT_LEADER);
        if(!c) return;
        draw_control_icon(game.fonts.standard, c, center, size);
    };
    gui.add_item(next_leader_button, "next_leader_button");
    
    
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
            SUN_METER_SUN_SPIN_SPEED;
            
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
    gui_item* day_nr_bubble = new gui_item();
    day_nr_bubble->on_draw =
    [this] (const point & center, const point & size) {
        draw_bitmap_in_box(bmp_day_bubble, center, size);
    };
    gui.add_item(day_nr_bubble, "day_bubble");
    
    
    //Day number text.
    gui_item* day_nr_text = new gui_item();
    day_nr_text->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter, COLOR_WHITE,
            center, ALLEGRO_ALIGN_CENTER, 1,
            size, i2s(game.states.gameplay->day)
        );
    };
    gui.add_item(day_nr_text, "day_number");
    
    
    //Standby group member icon.
    gui_item* standby_icon = new gui_item();
    standby_icon->on_draw =
    [this] (const point & center, const point & size) {
        //Standby group member preparations.
        ALLEGRO_BITMAP* standby_bmp = NULL;
        leader* l_ptr = game.states.gameplay->cur_leader_ptr;
        if(
            l_ptr &&
            l_ptr->group->cur_standby_type &&
            game.states.gameplay->closest_group_member
        ) {
            SUBGROUP_TYPE_CATEGORIES c =
                l_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_LEADER: {
                leader* l_ptr =
                    dynamic_cast<leader*>(
                        game.states.gameplay->closest_group_member
                    );
                standby_bmp = l_ptr->lea_type->bmp_icon;
                break;
                
            } default: {
                standby_bmp =
                    l_ptr->group->cur_standby_type->get_icon();
                break;
                
            }
            }
        }
        if(!standby_bmp) {
            standby_bmp = bmp_no_pikmin_bubble;
        }
        
        draw_bitmap_in_box(standby_bmp, center, size * 0.8);
        if(game.states.gameplay->closest_group_member_distant) {
            draw_bitmap_in_box(
                bmp_distant_pikmin_marker,
                center,
                size * 0.8
            );
        }
        draw_bitmap_in_box(bmp_bubble, center, size);
        
    };
    gui.add_item(standby_icon, "pikmin_standby_icon");
    
    
    //Standby group member maturity.
    gui_item* standby_maturity = new gui_item();
    standby_maturity->on_draw =
    [this] (const point & center, const point & size) {
        //Standby group member preparations.
        ALLEGRO_BITMAP* standby_mat_bmp = NULL;
        leader* l_ptr = game.states.gameplay->cur_leader_ptr;
        if(
            l_ptr &&
            l_ptr->group->cur_standby_type &&
            game.states.gameplay->closest_group_member
        ) {
            SUBGROUP_TYPE_CATEGORIES c =
                l_ptr->group->cur_standby_type->get_category();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                pikmin* p_ptr =
                    dynamic_cast<pikmin*>(
                        game.states.gameplay->closest_group_member
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
    gui.add_item(standby_maturity, "pikmin_standby_m_icon");
    
    
    //Previous Pikmin button.
    gui_item* prev_pikmin_button = new gui_item();
    prev_pikmin_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        if(game.states.gameplay->cur_leader_ptr->group->members.empty()) return;
        control_info* c = find_control(BUTTON_PREV_TYPE);
        if(!c) return;
        draw_control_icon(game.fonts.standard, c, center, size);
    };
    gui.add_item(prev_pikmin_button, "prev_pikmin_button");
    
    
    //Next Pikmin button.
    gui_item* next_pikmin_button = new gui_item();
    next_pikmin_button->on_draw =
    [this] (const point & center, const point & size) {
        if(!game.options.show_hud_controls) return;
        if(game.states.gameplay->cur_leader_ptr->group->members.empty()) return;
        control_info* c = find_control(BUTTON_NEXT_TYPE);
        if(!c) return;
        draw_control_icon(game.fonts.standard, c, center, size);
    };
    gui.add_item(next_pikmin_button, "next_pikmin_button");
    
    
    //Pikmin count "x".
    gui_item* pikmin_count_x = new gui_item();
    pikmin_count_x->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.counter, COLOR_WHITE,
            center, ALLEGRO_ALIGN_CENTER, 1, size, "x"
        );
    };
    gui.add_item(pikmin_count_x, "pikmin_standby_x");
    
    
    //Standby group member count.
    standby_count = new gui_item();
    standby_count->on_draw =
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
            standby_count->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            standby_count_nr = n_standby_pikmin;
        }
        
        draw_bitmap(
            bmp_counter_bubble_standby,
            center,
            size
        );
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x + size.x * 0.4, center.y),
            point(1.0f, 1.0f) + standby_count->get_juice_value(),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, true, i2s(n_standby_pikmin)
        );
    };
    gui.add_item(standby_count, "pikmin_standby_nr");
    
    
    //Group Pikmin count.
    group_count = new gui_item();
    group_count->on_draw =
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
            group_count->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            group_count_nr = n_group_pikmin;
        }
        
        draw_bitmap(
            bmp_counter_bubble_group,
            center,
            size
        );
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x + size.x * 0.4, center.y),
            point(1.0f, 1.0f) + group_count->get_juice_value(),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, true,
            i2s(n_group_pikmin)
        );
    };
    gui.add_item(group_count, "pikmin_group_nr");
    
    
    //Field Pikmin count.
    field_count = new gui_item();
    field_count->on_draw =
    [this] (const point & center, const point & size) {
        size_t n_field_pikmin = game.states.gameplay->mobs.pikmin_list.size();
        if(n_field_pikmin != field_count_nr) {
            field_count->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            field_count_nr = n_field_pikmin;
        }
        
        draw_bitmap(
            bmp_counter_bubble_field,
            center,
            size
        );
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x + size.x * 0.4, center.y),
            point(1.0f, 1.0f) + field_count->get_juice_value(),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, true,
            i2s(n_field_pikmin)
        );
    };
    gui.add_item(field_count, "pikmin_field_nr");
    
    
    //Total Pikmin count.
    total_count = new gui_item();
    total_count->on_draw =
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
            total_count->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            total_count_nr = n_total_pikmin;
        }
        
        draw_bitmap(
            bmp_counter_bubble_total,
            center,
            size
        );
        draw_compressed_scaled_text(
            game.fonts.counter, COLOR_WHITE,
            point(center.x + size.x * 0.4, center.y),
            point(1.0f, 1.0f) + total_count->get_juice_value(),
            ALLEGRO_ALIGN_RIGHT, 1, size * 0.7, true,
            i2s(n_total_pikmin)
        );
    };
    gui.add_item(total_count, "pikmin_total_nr");
    
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; ++s) {
        gui_item* counter_slash = new gui_item();
        counter_slash->on_draw =
        [this] (const point & center, const point & size) {
            draw_compressed_text(
                game.fonts.counter, COLOR_WHITE,
                center, ALLEGRO_ALIGN_CENTER, 1, size, "/"
            );
        };
        gui.add_item(counter_slash, "pikmin_slash_" + i2s(s + 1));
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
            ALLEGRO_ALIGN_LEFT, 1, size, true,
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
        
        draw_control_icon(game.fonts.standard, c, center, size);
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
            ALLEGRO_ALIGN_LEFT, 1, size, true,
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
        
        draw_control_icon(game.fonts.standard, c, center, size);
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
        
        draw_control_icon(game.fonts.standard, c, center, size);
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
        
        draw_control_icon(game.fonts.standard, c, center, size);
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
 * Starts animating the juice effect for leader icons being swapped.
 * old_leader_nr:
 *   Player's leader number before the transition.
 */
void hud_struct::start_leader_swap_juice(
    const size_t old_leader_nr
) {
    leader_swap_juice_timer = LEADER_SWAP_JUICE_DURATION;
    leader_icon_mgr.setup_transition();
    leader_health_mgr.setup_transition();
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void hud_struct::tick(const float delta_t) {
    if(leader_swap_juice_timer > 0.0f) {
        leader_swap_juice_timer -= delta_t;
        leader_swap_juice_timer = std::max(0.0f, leader_swap_juice_timer);
    }
    
    for(size_t l = 0; l < 3; ++l) {
        size_t l_idx = INVALID;
        if(l < game.states.gameplay->mobs.leaders.size()) {
            l_idx =
                (size_t) sum_and_wrap(
                    game.states.gameplay->cur_leader_nr,
                    l,
                    game.states.gameplay->mobs.leaders.size()
                );
        }
        leader_icon_mgr.update_content_index(l, l_idx);
        leader_health_mgr.update_content_index(l, l_idx);
    }
    
    gui.tick(game.delta_t);
}



/* ----------------------------------------------------------------------------
 * Creates a HUD bubble manager instance.
 */
hud_bubble_manager::hud_bubble_manager() {
}


/* ----------------------------------------------------------------------------
 * Returns the content index associated with a bubble.
 * Returns INVALID if anything goes wrong.
 * number:
 *   Number of the registered bubble.
 */
size_t hud_bubble_manager::get_content_index(const size_t number) {
    auto it = bubbles.find(number);
    if(it == bubbles.end()) return INVALID;
    return it->second.content_index;
}


/* ----------------------------------------------------------------------------
 * Returns the necessary information for the bubble to know how to draw itself.
 * number:
 *   Number of the registered bubble.
 * transition_anim_ratio:
 *   Ratio of time left in the current transition animation, or 0 for none.
 * content_idx:
 *   The index of the content the bubble should use is returned here.
 *   INVALID is returned on error.
 * pos:
 *   The final position it should use is returned here.
 * size:
 *   The final size it should use is returned here.
 */
void hud_bubble_manager::get_drawing_info(
    const size_t number, const float transition_anim_ratio,
    size_t* content_idx, point* pos, point* size
) {
    auto it = bubbles.find(number);
    if(it == bubbles.end()) {
        *content_idx = INVALID;
        return;
    }
    
    bool visible =
        game.states.gameplay->hud->gui.get_item_draw_info(
            it->second.bubble, pos, size
        );
        
    if(!visible) {
        *content_idx = INVALID;
        return;
    }
    
    map<size_t, bubble_info>::iterator match_it;
    gui_item* match_ptr = NULL;
    point match_pos;
    point match_size;
    
    //First, check if there's any matching bubble that we can move to/from.
    for(match_it = bubbles.begin(); match_it != bubbles.end(); ++match_it) {
        if(
            transition_anim_ratio > 0.5f &&
            match_it->second.content_index ==
            it->second.pre_transition_content_index
        ) {
            //In the first half of the animation, we want to search for a
            //bubble that has the content that our bubble had pre-transition.
            break;
        }
        if(
            transition_anim_ratio <= 0.5f &&
            match_it->second.pre_transition_content_index ==
            it->second.content_index
        ) {
            //In the second half, the match had the content our bubble has.
            break;
        }
    }
    if(match_it != bubbles.end()) {
        match_ptr = match_it->second.bubble;
        if(
            !game.states.gameplay->hud->gui.get_item_draw_info(
                match_ptr, &match_pos, &match_size
            )
        ) {
            match_ptr = NULL;
        }
    }
    
    //Figure out how to animate it, if we even should animate it.
    if(match_ptr) {
        //This bubble is heading to a new spot.
        
        point match_pivot(
            (pos->x + match_pos.x) / 2.0f,
            (pos->y + match_pos.y) / 2.0f
        );
        float mov_ratio = ease(EASE_IN_ELASTIC, 1.0f - transition_anim_ratio);
        float pivot_dist = dist(*pos, match_pivot).to_float();
        
        if(transition_anim_ratio > 0.5f) {
            //First half of the animation. Move to the first half.
            float match_start_angle = get_angle(match_pivot, *pos);
            *pos =
                match_pivot +
                rotate_point(
                    point(pivot_dist, 0.0f),
                    match_start_angle + mov_ratio * TAU / 2.0f
                );
            *size =
                interpolate_point(
                    ease(EASE_OUT, 1.0f - transition_anim_ratio),
                    0.0f, 1.0f,
                    *size, match_size
                );
                
        } else {
            //Second half of the animation. Move from the first half.
            float match_start_angle = get_angle(match_pivot, match_pos);
            *pos =
                match_pivot +
                rotate_point(
                    point(pivot_dist, 0.0f),
                    match_start_angle + mov_ratio * TAU / 2.0f
                );
            *size =
                interpolate_point(
                    ease(EASE_OUT, 1.0f - transition_anim_ratio),
                    0.0f, 1.0f,
                    match_size, *size
                );
                
        }
        
    } else {
        //This bubble has no equivalent to go to.
        
        if(transition_anim_ratio > 0.5f) {
            //First half of the animation. Fade out.
            *size *= ease(EASE_OUT, (transition_anim_ratio - 0.5f) * 2.0f);
            
        } else {
            //Second half of the animation. Fade in.
            *size *= ease(EASE_OUT, 1 - transition_anim_ratio * 2.0f);
            
        }
    }
    
    //Set the content index.
    if(transition_anim_ratio > 0.5f) {
        *content_idx = it->second.pre_transition_content_index;
    } else {
        *content_idx = it->second.content_index;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the content index associated with a bubble, before a transition
 * was started.
 * Returns INVALID if anything goes wrong.
 * number:
 *   Number of the registered bubble.
 */
size_t hud_bubble_manager::get_pre_transition_content_index(const size_t number) {
    auto it = bubbles.find(number);
    if(it == bubbles.end()) return INVALID;
    return it->second.pre_transition_content_index;
}


/* ----------------------------------------------------------------------------
 * Registers a bubble.
 * bubble:
 *   GUI item that represents this bubble.
 * number:
 *   Number of this item in its "family". For instance, if this is the icon
 *   for the second leader, this value is 2.
 */
void hud_bubble_manager::register_bubble(
    const size_t number, gui_item* bubble
) {
    bubbles[number] = bubble_info(bubble);
}


/* ----------------------------------------------------------------------------
 * Sets up the start of an animated transition.
 */
void hud_bubble_manager::setup_transition() {
    for(auto &b : bubbles) {
        b.second.pre_transition_content_index = b.second.content_index;
    }
}



/* ----------------------------------------------------------------------------
 * Updates the content index of a given bubble.
 * number:
 *   Number of the registered bubble.
 * new_index:
 *   New content index.
 */
void hud_bubble_manager::update_content_index(
    const size_t number, const size_t new_index
) {
    auto it = bubbles.find(number);
    if(it == bubbles.end()) return;
    it->second.content_index = new_index;
}



/* ----------------------------------------------------------------------------
 * Creates a HUD bubble manager bubble info instance.
 */
hud_bubble_manager::bubble_info::bubble_info(gui_item* bubble) :
    bubble(bubble),
    content_index(INVALID),
    pre_transition_content_index(INVALID) {
}
