/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drawing-related functions.
 */

#include <algorithm>
#include <typeinfo>

#include "animation.h"
#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "gameplay.h"
#include "geometry_utils.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Does the drawing for the main game loop.
 * bmp_output: if not NULL, draw the area onto this.
 */
void gameplay::do_game_drawing(
    ALLEGRO_BITMAP* bmp_output, ALLEGRO_TRANSFORM* bmp_transform
) {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    if(!paused) {
    
        cur_sun_strength = get_sun_strength();
        
        ALLEGRO_TRANSFORM world_to_screen_drawing_transform;
        
        if(bmp_output) {
            world_to_screen_drawing_transform = *bmp_transform;
            al_set_target_bitmap(bmp_output);
            al_set_separate_blender(
                ALLEGRO_ADD, ALLEGRO_ALPHA,
                ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
                ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
            );
        } else {
            world_to_screen_drawing_transform = world_to_screen_transform;
        }
        
        al_clear_to_color(cur_area_data.bg_color);
        
        //Layer 1 -- Background.
        draw_background(bmp_output);
        
        //Layer 2 -- Area layout.
        draw_layout(bmp_output, world_to_screen_drawing_transform);
        
        //Layer 3 -- Particles before mobs.
        if(!(bmp_output && !creator_tool_area_image_mobs)) {
            if(!bmp_output) {
                particles.draw_all(true, cam_box[0], cam_box[1]);
            } else {
                particles.draw_all(true);
            }
        }
        
        //Layer 4 -- Mobs.
        if(!(bmp_output && !creator_tool_area_image_mobs)) {
            draw_mobs(bmp_output);
        }
        
        //Layer 5 -- Particles after mobs.
        if(!(bmp_output && !creator_tool_area_image_mobs)) {
            if(!bmp_output) {
                particles.draw_all(false, cam_box[0], cam_box[1]);
            } else {
                particles.draw_all(false);
            }
        }
        
        //Layer 6 -- In-game text.
        if(!bmp_output) {
            draw_ingame_text();
        }
        
        //Layer 7 -- Precipitation.
        if(!bmp_output) {
            draw_precipitation();
        }
        
        //Layer 8 -- Tree shadows.
        if(!(bmp_output && !creator_tool_area_image_shadows)) {
            draw_tree_shadows();
        }
        
        //Finish dumping to a bitmap image here.
        if(bmp_output) {
            al_set_target_backbuffer(display);
            return;
        }
        
        //Layer 9 -- Lighting filter.
        draw_lighting_filter();
        
        //Layer 10 -- Cursor.
        draw_cursor(world_to_screen_drawing_transform);
        
        //Layer 11 -- HUD
        if(cur_message.empty()) {
            draw_hud();
        } else {
            draw_message_box();
        }
        
        //Layer 12 -- System stuff.
        draw_system_stuff();
        
    } else { //Paused.
    
    }
    
    if(area_title_fade_timer.time_left > 0) {
        draw_loading_screen(
            cur_area_data.name,
            cur_area_data.subtitle,
            area_title_fade_timer.get_ratio_left()
        );
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draws the area background.
 */
void gameplay::draw_background(ALLEGRO_BITMAP* bmp_output) {
    if(!cur_area_data.bg_bmp) return;
    
    ALLEGRO_VERTEX bg_v[4];
    for(unsigned char v = 0; v < 4; ++v) {
        bg_v[v].color = map_gray(255);
        bg_v[v].z = 0;
    }
    
    //Not gonna lie, this uses some fancy-shmancy numbers.
    //I mostly got here via trial and error.
    //I apologize if you're trying to understand what it means.
    int bmp_w = bmp_output ? al_get_bitmap_width(bmp_output) : scr_w;
    int bmp_h = bmp_output ? al_get_bitmap_height(bmp_output) : scr_h;
    float zoom_to_use = bmp_output ? 0.5 : cam_zoom;
    point final_zoom(
        bmp_w * 0.5 * cur_area_data.bg_dist / zoom_to_use,
        bmp_h * 0.5 * cur_area_data.bg_dist / zoom_to_use
    );
    
    bg_v[0].x = 0;
    bg_v[0].y = 0;
    bg_v[0].u = (cam_pos.x - final_zoom.x) / cur_area_data.bg_bmp_zoom;
    bg_v[0].v = (cam_pos.y - final_zoom.y) / cur_area_data.bg_bmp_zoom;
    bg_v[1].x = bmp_w;
    bg_v[1].y = 0;
    bg_v[1].u = (cam_pos.x + final_zoom.x) / cur_area_data.bg_bmp_zoom;
    bg_v[1].v = (cam_pos.y - final_zoom.y) / cur_area_data.bg_bmp_zoom;
    bg_v[2].x = bmp_w;
    bg_v[2].y = bmp_h;
    bg_v[2].u = (cam_pos.x + final_zoom.x) / cur_area_data.bg_bmp_zoom;
    bg_v[2].v = (cam_pos.y + final_zoom.y) / cur_area_data.bg_bmp_zoom;
    bg_v[3].x = 0;
    bg_v[3].y = bmp_h;
    bg_v[3].u = (cam_pos.x - final_zoom.x) / cur_area_data.bg_bmp_zoom;
    bg_v[3].v = (cam_pos.y + final_zoom.y) / cur_area_data.bg_bmp_zoom;
    
    al_draw_prim(
        bg_v, NULL, cur_area_data.bg_bmp,
        0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
    );
}


/* ----------------------------------------------------------------------------
 * Draws the cursor.
 */
void gameplay::draw_cursor(
    ALLEGRO_TRANSFORM &world_to_screen_drawing_transform
) {

    al_use_transform(&world_to_screen_drawing_transform);
    
    size_t n_arrows = group_move_arrows.size();
    for(size_t a = 0; a < n_arrows; ++a) {
        point pos(
            cos(group_move_angle) * group_move_arrows[a],
            sin(group_move_angle) * group_move_arrows[a]
        );
        float alpha =
            64 + min(
                191,
                (int) (
                    191 * (group_move_arrows[a] / (cursor_max_dist * 0.4))
                )
            );
        draw_bitmap(
            bmp_group_move_arrow,
            cur_leader_ptr->pos + pos,
            point(16 * (1 + group_move_arrows[a] / cursor_max_dist), -1),
            group_move_angle,
            map_alpha(alpha)
        );
    }
    
    size_t n_rings = whistle_rings.size();
    for(size_t r = 0; r < n_rings; ++r) {
        point pos(
            cur_leader_ptr->pos.x + cos(cursor_angle) * whistle_rings[r],
            cur_leader_ptr->pos.y + sin(cursor_angle) * whistle_rings[r]
        );
        unsigned char n = whistle_ring_colors[r];
        al_draw_circle(
            pos.x, pos.y, 8,
            al_map_rgba(
                WHISTLE_RING_COLORS[n][0],
                WHISTLE_RING_COLORS[n][1],
                WHISTLE_RING_COLORS[n][2],
                192
            ), 3
        );
    }
    
    if(whistle_radius > 0 || whistle_fade_timer.time_left > 0.0f) {
        if(pretty_whistle) {
            unsigned char n_dots = 16 * 6;
            for(unsigned char d = 0; d < 6; ++d) {
                for(unsigned char d2 = 0; d2 < 16; ++d2) {
                    unsigned char current_dot = d2 * 6 + d;
                    float angle =
                        M_PI * 2 / n_dots *
                        current_dot -
                        WHISTLE_DOT_SPIN_SPEED * area_time_passed;
                        
                    point dot_pos(
                        leader_cursor_w.x +
                        cos(angle) * whistle_dot_radius[d],
                        leader_cursor_w.y +
                        sin(angle) * whistle_dot_radius[d]
                    );
                    
                    ALLEGRO_COLOR c;
                    float alpha_mult;
                    if(whistle_fade_timer.time_left > 0.0f)
                        alpha_mult = whistle_fade_timer.get_ratio_left();
                    else
                        alpha_mult = 1;
                        
                    if(d == 0) {
                        //Red.
                        c = al_map_rgba(255, 0,   0,   255 * alpha_mult);
                    } else if(d == 1) {
                        //Orange.
                        c = al_map_rgba(255, 128, 0,   210 * alpha_mult);
                    } else if(d == 2) {
                        //Lime.
                        c = al_map_rgba(128, 255, 0,   165 * alpha_mult);
                    } else if(d == 3) {
                        //Cyan.
                        c = al_map_rgba(0,   255, 255, 120 * alpha_mult);
                    } else if(d == 4) {
                        //Blue.
                        c = al_map_rgba(0,   0,   255, 75  * alpha_mult);
                    } else {
                        //Purple.
                        c = al_map_rgba(128, 0,   255, 30  * alpha_mult);
                    }
                    
                    al_draw_filled_circle(dot_pos.x, dot_pos.y, 2, c);
                }
            }
        } else {
            unsigned char alpha = whistle_fade_timer.get_ratio_left() * 255;
            float radius = whistle_fade_radius;
            if(whistle_radius > 0) {
                alpha = 255;
                radius = whistle_radius;
            }
            al_draw_circle(
                leader_cursor_w.x, leader_cursor_w.y, radius,
                al_map_rgba(192, 192, 0, alpha), 2
            );
        }
    }
    
    //Cursor trail
    al_use_transform(&identity_transform);
    if(draw_cursor_trail) {
        for(size_t p = 1; p < cursor_spots.size(); ++p) {
            point* p_ptr = &cursor_spots[p];
            point* pp_ptr = &cursor_spots[p - 1]; //Previous point.
            if(
                (*p_ptr) != (*pp_ptr) &&
                dist(*p_ptr, *pp_ptr) > 4
            ) {
                al_draw_line(
                    p_ptr->x, p_ptr->y,
                    pp_ptr->x, pp_ptr->y,
                    change_alpha(
                        cur_leader_ptr->lea_type->main_color,
                        (p / (float) cursor_spots.size()) * 64
                    ),
                    p * 3
                );
            }
        }
    }
    
    //Mouse cursor.
    draw_bitmap(
        bmp_mouse_cursor,
        mouse_cursor_s,
        point(
            cam_zoom * al_get_bitmap_width(bmp_mouse_cursor) * 0.5,
            cam_zoom * al_get_bitmap_height(bmp_mouse_cursor) * 0.5
        ),
        -(area_time_passed * cursor_spin_speed),
        change_color_lighting(
            cur_leader_ptr->lea_type->main_color,
            cursor_height_diff_light
        )
    );
    
    //Leader cursor.
    al_use_transform(&world_to_screen_drawing_transform);
    draw_bitmap(
        bmp_cursor,
        leader_cursor_w,
        point(
            al_get_bitmap_width(bmp_cursor) * 0.5,
            al_get_bitmap_height(bmp_cursor) * 0.5
        ),
        cursor_angle,
        change_color_lighting(
            cur_leader_ptr->lea_type->main_color,
            cursor_height_diff_light
        )
    );
    
    if(!throw_can_reach_cursor) {
        unsigned char alpha =
            0 + (sin(cursor_invalid_effect) + 1) * 127.0;
            
        draw_bitmap(
            bmp_cursor_invalid,
            leader_cursor_w,
            point(
                al_get_bitmap_width(bmp_cursor) * 0.5,
                al_get_bitmap_height(bmp_cursor) * 0.5
            ),
            0,
            change_alpha(cur_leader_ptr->lea_type->main_color, alpha)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws the HUD.
 */
void gameplay::draw_hud() {
    al_use_transform(&identity_transform);
    point i_center, i_size;
    
    //Leader health.
    for(size_t l = 0; l < 3; ++l) {
        if(leaders.size() < l + 1) continue;
        
        size_t l_nr = sum_and_wrap(cur_leader_nr, l, leaders.size());
        size_t icon_id = HUD_ITEM_LEADER_1_ICON + l;
        size_t health_id = HUD_ITEM_LEADER_1_HEALTH + l;
        
        //Leader's icon.
        if(hud_items.get_draw_data(icon_id, &i_center, &i_size)) {
            al_draw_filled_circle(
                i_center.x, i_center.y,
                min(i_size.x, i_size.y) / 2.0f,
                change_alpha(leaders[l_nr]->type->main_color, 128)
            );
            draw_bitmap_in_box(
                leaders[l_nr]->lea_type->bmp_icon,
                i_center, i_size
            );
            draw_bitmap_in_box(bmp_bubble, i_center, i_size);
            
        }
        
        //Health wheel.
        if(hud_items.get_draw_data(health_id, &i_center, &i_size)) {
            draw_health(
                i_center,
                leaders[l_nr]->health, leaders[l_nr]->type->max_health,
                min(i_size.x, i_size.y) * 0.4f,
                true
            );
            draw_bitmap_in_box(bmp_hard_bubble, i_center, i_size);
        }
        
    }
    
    //Sun Meter.
    if(hud_items.get_draw_data(HUD_ITEM_TIME, &i_center, &i_size)) {
        unsigned char n_hours =
            (day_minutes_end - day_minutes_start) / 60.0f;
        float day_passed_ratio =
            (float) (day_minutes - day_minutes_start) /
            (float) (day_minutes_end - day_minutes_start);
        float sun_radius = i_size.y / 2.0;
        float first_dot_x = (i_center.x - i_size.x / 2.0) + sun_radius;
        float last_dot_x = (i_center.x + i_size.x / 2.0) - sun_radius;
        float dots_y = i_center.y;
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
    }
    
    //Day number bubble.
    if(hud_items.get_draw_data(HUD_ITEM_DAY_BUBBLE, &i_center, &i_size)) {
        draw_bitmap_in_box(bmp_day_bubble, i_center, i_size);
    }
    
    //Day number text.
    if(hud_items.get_draw_data(HUD_ITEM_DAY_NUMBER, &i_center, &i_size)) {
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            i_center, ALLEGRO_ALIGN_CENTER, 1,
            i_size, i2s(day)
        );
    }
    
    //Standby group member.
    ALLEGRO_BITMAP* standby_bmp = NULL;
    ALLEGRO_BITMAP* standby_mat_bmp = NULL;
    if(closest_group_member) {
        if(
            closest_group_member->type->category->id ==
            MOB_CATEGORY_PIKMIN
        ) {
            pikmin* p_ptr = dynamic_cast<pikmin*>(closest_group_member);
            standby_bmp = p_ptr->pik_type->bmp_icon;
            standby_mat_bmp =
                p_ptr->pik_type->bmp_maturity_icon[p_ptr->maturity];
                
        } else if(
            closest_group_member->type->category->id ==
            MOB_CATEGORY_LEADERS
        ) {
            leader* l_ptr = dynamic_cast<leader*>(closest_group_member);
            standby_bmp = l_ptr->lea_type->bmp_icon;
        }
        
    }
    if(!standby_bmp) standby_bmp = bmp_no_pikmin_bubble;
    
    //Standby group member icon.
    if(
        hud_items.get_draw_data(
            HUD_ITEM_PIKMIN_STANDBY_ICON, &i_center, &i_size
        )
    ) {
        draw_bitmap_in_box(standby_bmp, i_center, i_size * 0.8);
        if(closest_group_member_distant) {
            draw_bitmap_in_box(
                bmp_distant_pikmin_marker, i_center, i_size * 0.8
            );
        }
        draw_bitmap_in_box(bmp_bubble, i_center, i_size);
    }
    
    //Standby group member maturity.
    if(
        hud_items.get_draw_data(
            HUD_ITEM_PIKMIN_STANDBY_M_ICON, &i_center, &i_size
        )
    ) {
        if(standby_mat_bmp) {
            draw_bitmap_in_box(standby_mat_bmp, i_center, i_size * 0.8);
            draw_bitmap_in_box(bmp_bubble, i_center, i_size);
        }
    }
    
    //Pikmin count "x".
    if(hud_items.get_draw_data(HUD_ITEM_PIKMIN_STANDBY_X, &i_center, &i_size)) {
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            i_center, ALLEGRO_ALIGN_CENTER, 1, i_size, "x"
        );
    }
    
    //Standby group member count.
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_STANDBY_NR, &i_center, &i_size)
    ) {
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
        
        draw_bitmap(bmp_counter_bubble_standby, i_center, i_size);
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            point(i_center.x + i_size.x * 0.4, i_center.y),
            ALLEGRO_ALIGN_RIGHT, 1, i_size * 0.7, i2s(n_standby_pikmin)
        );
    }
    
    //Group Pikmin count.
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_GROUP_NR, &i_center, &i_size)
    ) {
        size_t pikmin_in_group = cur_leader_ptr->group->members.size();
        for(size_t l = 0; l < leaders.size(); ++l) {
            //If this leader is following the current one,
            //then they're not a Pikmin.
            //Subtract them from the group count total.
            if(leaders[l]->following_group == cur_leader_ptr) {
                pikmin_in_group--;
            }
        }
        
        draw_bitmap(bmp_counter_bubble_group, i_center, i_size);
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            point(i_center.x + i_size.x * 0.4, i_center.y),
            ALLEGRO_ALIGN_RIGHT, 1, i_size * 0.7, i2s(pikmin_in_group)
        );
    }
    
    //Field Pikmin count.
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_FIELD_NR, &i_center, &i_size)
    ) {
        draw_bitmap(bmp_counter_bubble_field, i_center, i_size);
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            point(i_center.x + i_size.x * 0.4, i_center.y),
            ALLEGRO_ALIGN_RIGHT, 1, i_size * 0.7, i2s(pikmin_list.size())
        );
    }
    
    //Total Pikmin count.
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_TOTAL_NR, &i_center, &i_size)
    ) {
        unsigned long total_pikmin = pikmin_list.size();
        for(
            auto o = pikmin_in_onions.begin();
            o != pikmin_in_onions.end(); ++o
        ) {
            total_pikmin += o->second;
        }
        
        draw_bitmap(bmp_counter_bubble_total, i_center, i_size);
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            point(i_center.x + i_size.x * 0.4, i_center.y),
            ALLEGRO_ALIGN_RIGHT, 1, i_size * 0.7, i2s(total_pikmin)
        );
    }
    
    //Pikmin counter slashes.
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_SLASH_1, &i_center, &i_size)
    ) {
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            i_center, ALLEGRO_ALIGN_CENTER, 1, i_size, "/"
        );
    }
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_SLASH_2, &i_center, &i_size)
    ) {
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            i_center, ALLEGRO_ALIGN_CENTER, 1, i_size, "/"
        );
    }
    if(
        hud_items.get_draw_data(HUD_ITEM_PIKMIN_SLASH_3, &i_center, &i_size)
    ) {
        draw_compressed_text(
            font_counter, al_map_rgb(255, 255, 255),
            i_center, ALLEGRO_ALIGN_CENTER, 1, i_size, "/"
        );
    }
    
    
    //Sprays.
    if(spray_types.size() > 0) {
        size_t top_spray_nr;
        if(spray_types.size() <= 2) top_spray_nr = 0;
        else top_spray_nr = selected_spray;
        
        //Top or current spray.
        if(
            hud_items.get_draw_data(HUD_ITEM_SPRAY_1_ICON, &i_center, &i_size)
        ) {
            draw_bitmap_in_box(
                spray_types[top_spray_nr].bmp_spray, i_center, i_size
            );
        }
        
        if(
            hud_items.get_draw_data(HUD_ITEM_SPRAY_1_AMOUNT, &i_center, &i_size)
        ) {
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                point(i_center.x - i_size.x / 2.0, i_center.y),
                ALLEGRO_ALIGN_LEFT, 1, i_size,
                "x" + i2s(spray_amounts[top_spray_nr])
            );
        }
        
        if(
            hud_items.get_draw_data(HUD_ITEM_SPRAY_1_BUTTON, &i_center, &i_size)
        ) {
            for(size_t c = 0; c < controls[0].size(); ++c) {
                if(
                    (
                        controls[0][c].action == BUTTON_USE_SPRAY_1 &&
                        spray_types.size() <= 2
                    ) || (
                        controls[0][c].action == BUTTON_USE_SPRAY &&
                        spray_types.size() >= 3
                    )
                ) {
                    draw_control(font_main, controls[0][c], i_center, i_size);
                    break;
                }
            }
        }
        
        if(spray_types.size() == 2) {
        
            //Secondary spray, when there're only two types.
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_2_ICON, &i_center, &i_size
                )
            ) {
                draw_bitmap_in_box(spray_types[1].bmp_spray, i_center, i_size);
            }
            
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_2_AMOUNT, &i_center, &i_size
                )
            ) {
            
                draw_compressed_text(
                    font_counter, al_map_rgb(255, 255, 255),
                    point(i_center.x - i_size.x / 2.0, i_center.y),
                    ALLEGRO_ALIGN_LEFT, 1, i_size,
                    "x" + i2s(spray_amounts[1])
                );
            }
            
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_2_BUTTON, &i_center, &i_size
                )
            ) {
                for(size_t c = 0; c < controls[0].size(); ++c) {
                    if(controls[0][c].action == BUTTON_USE_SPRAY_2) {
                        draw_control(
                            font_main, controls[0][c], i_center, i_size
                        );
                        break;
                    }
                }
            }
            
        } else if(spray_types.size() >= 3) {
        
            //Previous spray info.
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_PREV_ICON, &i_center, &i_size
                )
            ) {
                draw_bitmap_in_box(
                    spray_types[
                        sum_and_wrap(selected_spray, -1, spray_types.size())
                    ].bmp_spray,
                    i_center, i_size
                );
            }
            
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_PREV_BUTTON, &i_center, &i_size
                )
            ) {
                for(size_t c = 0; c < controls[0].size(); ++c) {
                    if(controls[0][c].action == BUTTON_PREV_SPRAY) {
                        draw_control(
                            font_main, controls[0][c], i_center, i_size
                        );
                        break;
                    }
                }
            }
            
            //Next spray info.
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_NEXT_ICON, &i_center, &i_size
                )
            ) {
                draw_bitmap_in_box(
                    spray_types[
                        sum_and_wrap(selected_spray, 1, spray_types.size())
                    ].bmp_spray,
                    i_center, i_size
                );
            }
            
            if(
                hud_items.get_draw_data(
                    HUD_ITEM_SPRAY_NEXT_BUTTON, &i_center, &i_size
                )
            ) {
                for(size_t c = 0; c < controls[0].size(); ++c) {
                    if(controls[0][c].action == BUTTON_NEXT_SPRAY) {
                        draw_control(
                            font_main, controls[0][c], i_center, i_size
                        );
                        break;
                    }
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws the in-game text.
 */
void gameplay::draw_ingame_text() {
    //Fractions and health.
    size_t n_mobs = mobs.size();
    for(size_t m = 0; m < n_mobs; ++m) {
        mob* mob_ptr = mobs[m];
        
        if(mob_ptr->carry_info) {
            if(mob_ptr->carry_info->cur_carrying_strength > 0) {
                ALLEGRO_COLOR color;
                bool valid = false;
                if(mob_ptr->carry_info->is_moving) {
                    if(mob_ptr->carry_info->carry_to_ship) {
                        color = carrying_color_move;
                        valid = true;
                    } else if(mob_ptr->carrying_target) {
                        color =
                            (
                                (onion*) (mob_ptr->carrying_target)
                            )->oni_type->pik_type->main_color;
                        valid = true;
                    }
                }
                if(!valid) {
                    color = carrying_color_stop;
                }
                draw_fraction(
                    point(
                        mob_ptr->pos.x,
                        mob_ptr->pos.y - mob_ptr->type->radius -
                        font_main_h * 1.25
                    ),
                    mob_ptr->carry_info->cur_carrying_strength,
                    mob_ptr->type->weight,
                    color
                );
            }
        }
        
        if(
            mob_ptr->type->show_health &&
            !mob_ptr->hide &&
            mob_ptr->health < mob_ptr->type->max_health &&
            mob_ptr->health > 0
        ) {
            draw_health(
                point(
                    mob_ptr->pos.x,
                    mob_ptr->pos.y - mob_ptr->type->radius -
                    DEF_HEALTH_WHEEL_RADIUS - 4
                ),
                mob_ptr->health, mob_ptr->type->max_health
            );
        }
    }
    
    bool done = false;
    
    //Lying down stop notification.
    if(
        cur_leader_ptr->carry_info &&
        whistle_control_id != INVALID
    ) {
        draw_notification(
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y -
                cur_leader_ptr->type->radius
            ),
            "Get up", &controls[0][whistle_control_id]
        );
        done = true;
    }
    
    //Pluck stop notification.
    if(
        !done &&
        cur_leader_ptr->auto_plucking &&
        whistle_control_id != INVALID
    ) {
        draw_notification(
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y -
                cur_leader_ptr->type->radius
            ),
            "Stop plucking", &controls[0][whistle_control_id]
        );
        done = true;
    }
    
    //Ship healing notification.
    if(
        !done &&
        close_to_ship_to_heal &&
        click_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_ship_to_heal->beam_final_pos.x,
                close_to_ship_to_heal->beam_final_pos.y -
                close_to_ship_to_heal->shi_type->beam_radius
            ),
            "Repair suit", &controls[0][click_control_id]
        );
        done = true;
    }
    
    //Info spot notification.
    if(
        !done &&
        close_to_spot_to_read &&
        click_control_id != INVALID
    ) {
        float pivot_y =
            close_to_spot_to_read->pos.y - close_to_spot_to_read->type->radius;
        if(!close_to_spot_to_read->opens_box) {
            draw_notification(
                point(close_to_spot_to_read->pos.x, pivot_y),
                close_to_spot_to_read->text, NULL
            );
            done = true;
            
        } else if(click_control_id != INVALID) {
            draw_notification(
                point(close_to_spot_to_read->pos.x, pivot_y),
                "Read", &controls[0][click_control_id]
            );
            done = true;
            
        }
    }
    
    //Pikmin pluck notification.
    if(
        !done &&
        close_to_pikmin_to_pluck &&
        click_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_pikmin_to_pluck->pos.x,
                close_to_pikmin_to_pluck->pos.y -
                close_to_pikmin_to_pluck->type->radius
            ),
            "Pluck", &controls[0][click_control_id]
        );
        done = true;
    }
    
    //Onion open notification.
    if(
        !done &&
        close_to_onion_to_open &&
        click_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_onion_to_open->pos.x,
                close_to_onion_to_open->pos.y -
                close_to_onion_to_open->type->radius
            ),
            "Call a Pikmin", &controls[0][click_control_id]
        );
        done = true;
    }
}


/* ----------------------------------------------------------------------------
 * Draws the area layout: sectors, wall shadows, etc.
 */
void gameplay::draw_layout(
    ALLEGRO_BITMAP* bmp_output,
    ALLEGRO_TRANSFORM &world_to_screen_drawing_transform
) {
    al_use_transform(&world_to_screen_drawing_transform);
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        
        if(
            !bmp_output &&
            !rectangles_intersect(
                s_ptr->bbox[0], s_ptr->bbox[1],
                cam_box[0], cam_box[1]
            )
        ) {
            //Off-camera.
            continue;
        }
        
        draw_sector_texture(s_ptr, point(), 1.0f, 1.0f);
        
        if(s_ptr->associated_liquid) {
            draw_liquid(s_ptr, point(), 1.0f);
        }
        
        draw_sector_shadows(s_ptr, point(), 1.0f);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the full-screen effects that will represent lighting.
 */
void gameplay::draw_lighting_filter() {
    al_use_transform(&identity_transform);
    
    //Draw the fog effect.
    ALLEGRO_COLOR fog_c = get_fog_color();
    if(fog_c.a > 0) {
        //Start by drawing the central fog fade out effect.
        
        point fog_top_left =
            cam_pos -
            point(
                cur_area_data.weather_condition.fog_far,
                cur_area_data.weather_condition.fog_far
            );
        point fog_bottom_right =
            cam_pos +
            point(
                cur_area_data.weather_condition.fog_far,
                cur_area_data.weather_condition.fog_far
            );
        al_transform_coordinates(
            &world_to_screen_transform,
            &fog_top_left.x, &fog_top_left.y
        );
        al_transform_coordinates(
            &world_to_screen_transform,
            &fog_bottom_right.x, &fog_bottom_right.y
        );
        
        if(bmp_fog) {
            draw_bitmap(
                bmp_fog,
                (fog_top_left + fog_bottom_right) / 2,
                (fog_bottom_right - fog_top_left),
                0, fog_c
            );
        }
        
        //Now draw the fully opaque fog around the central fade.
        //Top-left and top-center.
        al_draw_filled_rectangle(
            0, 0,
            fog_bottom_right.x, fog_top_left.y,
            fog_c
        );
        //Top-right and center-right.
        al_draw_filled_rectangle(
            fog_bottom_right.x, 0,
            scr_w, fog_bottom_right.y,
            fog_c
        );
        //Bottom-right and bottom-center.
        al_draw_filled_rectangle(
            fog_top_left.x, fog_bottom_right.y,
            scr_w, scr_h,
            fog_c
        );
        //Bottom-left and center-left.
        al_draw_filled_rectangle(
            0, fog_top_left.y,
            fog_top_left.x, scr_h,
            fog_c
        );
        
    }
    
    //Draw the daylight.
    ALLEGRO_COLOR daylight_c = get_daylight_color();
    if(daylight_c.a > 0) {
        al_draw_filled_rectangle(0, 0, scr_w, scr_h, daylight_c);
    }
    
    //Draw the blackout effect.
    unsigned char blackout_s = get_blackout_strength();
    if(blackout_s > 0) {
        //First, we'll create the lightmap.
        //This is inverted (white = darkness, black = light), because we'll
        //apply it to the screen using a subtraction operation.
        al_set_target_bitmap(lightmap_bmp);
        
        //For starters, the whole screen is dark (white in the map).
        al_clear_to_color(map_gray(blackout_s));
        
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_separate_blender(
            ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE,
            ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE
        );
        
        //Then, find out spotlights, and draw
        //their lights on the map (as black).
        al_hold_bitmap_drawing(true);
        for(size_t m = 0; m < mobs.size(); ++m) {
            if(mobs[m]->hide) continue;
            
            point pos = mobs[m]->pos;
            al_transform_coordinates(
                &world_to_screen_transform,
                &pos.x, &pos.y
            );
            float radius = mobs[m]->type->radius * 4.0 * cam_zoom;
            al_draw_scaled_bitmap(
                bmp_spotlight,
                0, 0, 64, 64,
                pos.x - radius, pos.y - radius,
                radius * 2.0, radius * 2.0,
                0
            );
        }
        al_hold_bitmap_drawing(false);
        
        //Now, simply darken the screen using the map.
        al_set_target_backbuffer(display);
        
        al_draw_bitmap(lightmap_bmp, 0, 0, 0);
        
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Draws a message box.
 */
void gameplay::draw_message_box() {
    al_use_transform(&identity_transform);
    
    draw_bitmap(
        bmp_message_box,
        point(scr_w / 2, scr_h - font_main_h * 2 - 4),
        point(scr_w - 16, font_main_h * 4)
    );
    
    if(cur_message_speaker) {
        draw_bitmap(
            cur_message_speaker,
            point(40, scr_h - font_main_h * 4 - 16),
            point(48, 48)
        );
        draw_bitmap(
            bmp_bubble,
            point(40, scr_h - font_main_h * 4 - 16),
            point(64, 64)
        );
    }
    
    string text =
        cur_message.substr(
            cur_message_stopping_chars[cur_message_section],
            cur_message_char -
            cur_message_stopping_chars[cur_message_section]
        );
    vector<string> lines = split(text, "\n");
    
    for(size_t l = 0; l < lines.size(); ++l) {
    
        draw_compressed_text(
            font_main, al_map_rgb(255, 255, 255),
            point(24, scr_h - font_main_h * (4 - l) + 8),
            ALLEGRO_ALIGN_LEFT, 0, point(scr_w - 64, 0),
            lines[l]
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Draws the mobs.
 */
void gameplay::draw_mobs(ALLEGRO_BITMAP* bmp_output) {
    vector<mob*> sorted_mobs;
    sorted_mobs = mobs;
    sort(
        sorted_mobs.begin(), sorted_mobs.end(),
    [] (mob * m1, mob * m2) -> bool {
        if(m1->z == m2->z) {
            if(m1->type->height == m2->type->height) {
                return m1->id < m2->id;
            }
            return m1->type->height < m2->type->height;
        }
        return m1->z < m2->z;
    }
    );
    
    float shadow_stretch = 0;
    
    if(day_minutes < 60 * 5 || day_minutes > 60 * 20) {
        shadow_stretch = 1;
    } else if(day_minutes < 60 * 12) {
        shadow_stretch = 1 - ((day_minutes - 60 * 5) / (60 * 12 - 60 * 5));
    } else {
        shadow_stretch = (day_minutes - 60 * 12) / (60 * 20 - 60 * 12);
    }
    
    mob* mob_ptr = NULL;
    //Draw the mob shadows.
    al_hold_bitmap_drawing(true);
    for(size_t m = 0; m < sorted_mobs.size(); ++m) {
        mob_ptr = sorted_mobs[m];
        
        if(!mob_ptr->type->casts_shadow || mob_ptr->hide) {
            continue;
        }
        
        if(
            !bmp_output &&
            !bbox_check(
                cam_box[0], cam_box[1],
                mob_ptr->pos, mob_ptr->type->radius
            )
        ) {
            //Off-camera.
            continue;
        }
        
        float floor_z = mob_ptr->ground_floor->z;
        
        draw_mob_shadow(
            mob_ptr->pos,
            mob_ptr->type->radius * 2,
            mob_ptr->z - floor_z,
            shadow_stretch
        );
    }
    al_hold_bitmap_drawing(false);
    
    //And now the mobs themselves.
    for(size_t m = 0; m < sorted_mobs.size(); ++m) {
        mob_ptr = sorted_mobs[m];
        
        if(
            !bmp_output &&
            !bbox_check(
                cam_box[0], cam_box[1],
                mob_ptr->pos, mob_ptr->type->radius
            )
        ) {
            //Off-camera.
            continue;
        }
        
        mob_ptr->draw();
        
        //Creator tool -- draw hitboxes.
        if(creator_tool_hitboxes) {
            sprite* s = mob_ptr->anim.get_cur_sprite();
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); ++h) {
                    hitbox* h_ptr = &s->hitboxes[h];
                    ALLEGRO_COLOR hc;
                    if(h_ptr->type == HITBOX_TYPE_NORMAL) {
                        hc = al_map_rgba(0, 128, 0, 192); //Green.
                    } else if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                        hc = al_map_rgba(128, 0, 0, 192); //Red.
                    } else {
                        hc = al_map_rgba(128, 128, 0, 192); //Yellow.
                    }
                    point p =
                        mob_ptr->pos +
                        rotate_point(h_ptr->pos, mob_ptr->angle);
                    al_draw_filled_circle(p.x, p.y, h_ptr->radius, hc);
                }
            }
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Draws the precipitation.
 */
void gameplay::draw_precipitation() {
    if(
        cur_area_data.weather_condition.precipitation_type !=
        PRECIPITATION_TYPE_NONE
    ) {
        size_t n_precipitation_particles = precipitation.size();
        for(size_t p = 0; p < n_precipitation_particles; ++p) {
            al_draw_filled_circle(
                precipitation[p].x, precipitation[p].y,
                3, al_map_rgb(255, 255, 255)
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws system stuff.
 */
void gameplay::draw_system_stuff() {
    if(!info_print_text.empty()) {
        float alpha_mult = 1;
        if(
            info_print_timer.time_left <
            INFO_PRINT_DURATION - INFO_PRINT_FADE_DELAY
        ) {
            alpha_mult =
                info_print_timer.time_left /
                (INFO_PRINT_DURATION - INFO_PRINT_FADE_DELAY);
        }
        al_draw_filled_rectangle(
            0, 0, scr_w, scr_h * 0.3,
            al_map_rgba(0, 0, 0, 96 * alpha_mult)
        );
        draw_text_lines(
            font_builtin, al_map_rgba(255, 255, 255, 128 * alpha_mult),
            point(8, 8), 0, 0, info_print_text
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws tree shadows.
 */
void gameplay::draw_tree_shadows() {
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
        
        unsigned char alpha =
            ((s_ptr->alpha / 255.0) * cur_sun_strength) * 255;
            
        draw_bitmap(
            s_ptr->bitmap,
            point(
                s_ptr->center.x + TREE_SHADOW_SWAY_AMOUNT *
                sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                s_ptr->sway.x,
                s_ptr->center.y + TREE_SHADOW_SWAY_AMOUNT *
                sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                s_ptr->sway.y
            ),
            s_ptr->size,
            s_ptr->angle, map_alpha(alpha)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws the current area and mobs to a bitmap and returns it.
 */
ALLEGRO_BITMAP* gameplay::draw_to_bitmap() {
    //First, get the full dimensions of the map.
    float min_x = FLT_MAX, min_y = FLT_MAX, max_x = FLT_MIN, max_y = FLT_MIN;
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); v++) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        min_x = min(v_ptr->x, min_x);
        min_y = min(v_ptr->y, min_y);
        max_x = max(v_ptr->x, max_x);
        max_y = max(v_ptr->y, max_y);
    }
    
    //Figure out the scale that will fit on the image.
    float area_w = max_x - min_x;
    float area_h = max_y - min_y;
    float scale = 1.0f;
    float final_bmp_w = creator_tool_area_image_size;
    float final_bmp_h = creator_tool_area_image_size;
    
    if(area_w > area_h) {
        scale = creator_tool_area_image_size / area_w;
        final_bmp_h *= area_h / area_w;
    } else {
        scale = creator_tool_area_image_size / area_h;
        final_bmp_w *= area_w / area_h;
    }
    
    //Create the bitmap.
    ALLEGRO_BITMAP* bmp = al_create_bitmap(final_bmp_w, final_bmp_h);
    
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(&t, -min_x, -min_y);
    al_scale_transform(&t, scale, scale);
    
    //Begin drawing!
    do_game_drawing(bmp, &t);
    
    return bmp;
}


/* ----------------------------------------------------------------------------
 * Draws a key or button on the screen.
 * font:     Font to use for the name.
 * c:        Info on the control.
 * where:    Center of the place to draw at.
 * max_size: Max width or height. Used to compress it if needed.
 *   0 = unlimited.
 */
void draw_control(
    const ALLEGRO_FONT* const font, const control_info &c,
    const point &where, const point &max_size
) {

    if(c.type == CONTROL_TYPE_MOUSE_BUTTON) {
        //If it's a mouse click, just draw the icon and be done with it.
        if(c.button >= 1 && c.button <= 3) {
        
            draw_bitmap_in_box(
                bmp_mouse_button_icon[c.button - 1], where, max_size
            );
            return;
            
        }
    }
    
    if(
        c.type == CONTROL_TYPE_MOUSE_WHEEL_UP ||
        c.type == CONTROL_TYPE_MOUSE_WHEEL_DOWN
    ) {
        //Likewise, if it's a mouse wheel move, just draw the icon and leave.
        ALLEGRO_BITMAP* b = bmp_mouse_wu_icon;
        if(c.type == CONTROL_TYPE_MOUSE_WHEEL_DOWN) {
            b = bmp_mouse_wd_icon;
        }
        
        draw_bitmap_in_box(b, where, max_size);
        return;
    }
    
    string name;
    if(c.type == CONTROL_TYPE_KEYBOARD_KEY) {
        name = str_to_upper(al_keycode_to_name(c.button));
    } else if(
        c.type == CONTROL_TYPE_JOYSTICK_AXIS_NEG ||
        c.type == CONTROL_TYPE_JOYSTICK_AXIS_POS
    ) {
        name = "AXIS " + i2s(c.stick) + " " + i2s(c.axis);
        name += c.type == CONTROL_TYPE_JOYSTICK_AXIS_NEG ? "-" : "+";
    } else if(c.type == CONTROL_TYPE_JOYSTICK_BUTTON) {
        name = i2s(c.button + 1);
    } else if(c.type == CONTROL_TYPE_MOUSE_BUTTON) {
        name = "M" + i2s(c.button);
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_LEFT) {
        name = "MWL";
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT) {
        name = "MWR";
    }
    
    int x1, y1, x2, y2;
    al_get_text_dimensions(font, name.c_str(), &x1, &y1, &x2, &y2);
    float total_width =
        min((float) (x2 - x1 + 4), (max_size.x == 0 ? FLT_MAX : max_size.x));
    float total_height =
        min((float) (y2 - y1 + 4), (max_size.y == 0 ? FLT_MAX : max_size.y));
    total_width = max(total_width, total_height);
    
    if(c.type == CONTROL_TYPE_KEYBOARD_KEY) {
        al_draw_filled_rectangle(
            where.x - total_width * 0.5, where.y - total_height * 0.5,
            where.x + total_width * 0.5, where.y + total_height * 0.5,
            map_alpha(192)
        );
        al_draw_rectangle(
            where.x - total_width * 0.5, where.y - total_height * 0.5,
            where.x + total_width * 0.5, where.y + total_height * 0.5,
            al_map_rgba(160, 160, 160, 192), 2
        );
    } else {
        al_draw_filled_rounded_rectangle(
            where.x - total_width * 0.5, where.y - total_height * 0.5,
            where.x + total_width * 0.5, where.y + total_height * 0.5,
            min(16.0, total_width * 0.5),
            min(16.0, total_height * 0.5),
            map_alpha(192)
        );
        al_draw_rounded_rectangle(
            where.x - total_width * 0.5, where.y - total_height * 0.5,
            where.x + total_width * 0.5, where.y + total_height * 0.5,
            min(16.0, total_width * 0.5),
            min(16.0, total_height * 0.5),
            al_map_rgba(160, 160, 160, 192), 2
        );
    }
    draw_compressed_text(
        font, map_alpha(192), where, ALLEGRO_ALIGN_CENTER, 1,
        point(
            (max_size.x == 0 ? 0 : max_size.x - 2),
            (max_size.y == 0 ? 0 : max_size.y - 2)
        ), name
    );
}


/* ----------------------------------------------------------------------------
 * Draws a bitmap.
 * bmp:    The bitmap.
 * center: Center coordinates.
 * size:   Final width and height.
 *   Make this -1 on one of them to keep the aspect ratio from the other.
 * angle:  Angle to rotate the bitmap by.
 * tint:   Tint the bitmap with this color.
 */
void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle, const ALLEGRO_COLOR &tint
) {

    if(size.x == 0 && size.y == 0) return;
    
    if(!bmp) {
        bmp = bmp_error;
    }
    
    point bmp_size(al_get_bitmap_width(bmp), al_get_bitmap_height(bmp));
    point scale = size / bmp_size;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        tint,
        bmp_size.x / 2.0, bmp_size.y / 2.0,
        center.x, center.y,
        (size.x == -1) ? scale.y : scale.x,
        (size.y == -1) ? scale.x : scale.y,
        angle,
        0
    );
}


/* ----------------------------------------------------------------------------
 * Draws a bitmap, but keeps its aspect ratio,
 * and scales it to fit in an imaginary box.
 * bmp:      The bitmap.
 * center:   Center coordinates.
 * box_size: Width and height of the box.
 * angle:    Angle to rotate the bitmap by.
 *   The box does not take angling into account.
 * tint:     Tint the bitmap with this color.
 */
void draw_bitmap_in_box(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &box_size, const float angle, const ALLEGRO_COLOR &tint
) {
    if(box_size.x == 0 || box_size.y == 0) return;
    float w_diff = al_get_bitmap_width(bmp) / box_size.x;
    float h_diff = al_get_bitmap_height(bmp) / box_size.y;
    
    if(w_diff > h_diff) {
        draw_bitmap(bmp, center, point(box_size.x, -1), angle, tint);
    } else {
        draw_bitmap(bmp, center, point(-1, box_size.y), angle, tint);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a bitmap, applying bitmap effects.
 * bmp:      The bitmap.
 * center:   Center coordinates.
 * size:     Final width and height.
 *   Make this -1 on one of them to keep the aspect ratio from the other.
 * angle:    Angle to rotate the bitmap by.
 * effects:  Bitmap effect manager with the effects.
 */
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle,
    bitmap_effect_manager* effects
) {

    if(!bmp) {
        bmp = bmp_error;
    }
    
    bitmap_effect_props final_props = effects->get_final_properties();
    
    point bmp_size(al_get_bitmap_width(bmp), al_get_bitmap_height(bmp));
    point scale(
        (size.x / bmp_size.x) * final_props.scale.x,
        (size.y / bmp_size.y) * final_props.scale.y
    );
    point final_pos = center + final_props.translation;
    float final_angle = angle + final_props.rotation;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        final_props.tint_color,
        bmp_size.x / 2, bmp_size.y / 2,
        final_pos.x, final_pos.y,
        (size.x == -1) ? scale.y : scale.x,
        (size.y == -1) ? scale.x : scale.y,
        final_angle,
        0
    );
    
    if(final_props.glow_color.a > 0) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        al_draw_tinted_scaled_rotated_bitmap(
            bmp,
            final_props.glow_color,
            bmp_size.x / 2, bmp_size.y / 2,
            final_pos.x, final_pos.y,
            (size.x == -1) ? scale.y : scale.x,
            (size.y == -1) ? scale.x : scale.y,
            final_angle,
            0
        );
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws text on the screen, but compresses (scales) it
 * to fit within the specified range.
 * font - flags: The parameters you'd use for al_draw_text.
 * valign:       Vertical align: 0 = top, 1 = middle, 2 = bottom.
 * max_size:     The maximum width and height. Use <= 0 to have no limit.
 * text:         Text to draw.
 */
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const unsigned char valign,
    const point &max_size, const string &text
) {
    if(max_size.x == 0 && max_size.y == 0) return;
    
    int x1, x2, y1, y2;
    al_get_text_dimensions(font, text.c_str(), &x1, &y1, &x2, &y2);
    int text_width = x2 - x1, text_height = y2 - y1;
    point scale(1.0, 1.0);
    float final_text_height = text_height;
    
    if(text_width > max_size.x && max_size.x > 0) {
        scale.x = max_size.x / text_width;
    }
    if(text_height > max_size.y && max_size.y > 0) {
        scale.y = max_size.y / text_height;
        final_text_height = max_size.y;
    }
    
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale.x, scale.y);
    al_translate_transform(
        &scale_transform, where.x,
        (
            (valign == 1) ?
            where.y - final_text_height * 0.5 :
            ((valign == 2) ? where.y - final_text_height : where.y)
        )
    );
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        al_draw_text(font, color, 0, 0, flags, text.c_str());
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws a strength/weight fraction, in the style of Pikmin 2.
 * The strength is above the weight.
 * center:  Center of the text.
 * current: Current strength.
 * needed:  Needed strength to lift the object (weight).
 * color:   Color of the fraction's text.
 */
void draw_fraction(
    const point &center, const unsigned int current,
    const unsigned int needed, const ALLEGRO_COLOR &color
) {
    float first_y = center.y - (font_main_h * 3) / 2;
    float font_h = al_get_font_line_height(font_value);
    
    draw_scaled_text(
        font_value, color, point(center.x, first_y),
        point(
            (current >= needed ? 1.2 : 1.0),
            (current >= needed ? 1.2 : 1.0)
        ),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(current).c_str())
    );
    
    al_draw_text(
        font_value, color, center.x, first_y + font_h * 0.75,
        ALLEGRO_ALIGN_CENTER, "-"
    );
    
    draw_scaled_text(
        font_value, color, point(center.x, first_y + font_h * 1.5),
        point(
            (needed > current ? 1.2 : 1.0),
            (needed > current ? 1.2 : 1.0)
        ),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(needed).c_str())
    );
}


/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * center:     Center of the wheel.
 * health:     Current amount of health of the mob
 *   whose health we're representing.
 * max_health: Maximum amount of health of the mob;
 *   health for when it's fully healed.
 * radius:     Radius of the wheel (the whole wheel, not just the pieslice).
 * just_chart: If true, only draw the actual pieslice (pie-chart).
 *   Used for leader HP on the HUD.
 */
void draw_health(
    const point &center,
    const float health, const float max_health,
    const float radius, const bool just_chart
) {
    float ratio = health / max_health;
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgb_f(1 - (ratio - 0.5) * 2, 1, 0);
    } else {
        c = al_map_rgb_f(1, (ratio * 2), 0);
    }
    
    if(!just_chart) {
        al_draw_filled_circle(
            center.x, center.y, radius, al_map_rgba(0, 0, 0, 128)
        );
    }
    al_draw_filled_pieslice(
        center.x, center.y, radius, -M_PI_2, -ratio * M_PI * 2, c
    );
    if(!just_chart) {
        al_draw_circle(
            center.x, center.y, radius + 1, al_map_rgb(0, 0, 0), 2
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws a liquid sector.
 * where:   X and Y offset.
 * scale:   Scale the sector by this much.
 */
void draw_liquid(
    sector* s_ptr, const point &where, const float scale
) {

    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        av[v].z = 0;
    }
    
    //Layer 1 - Transparent wobbling ground texture.
    if(s_ptr->floors[0].texture_bitmap) {
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -s_ptr->floors[0].texture_translation.x,
            -s_ptr->floors[0].texture_translation.y,
            1.0 / s_ptr->floors[0].texture_scale.x,
            1.0 / s_ptr->floors[0].texture_scale.y,
            -s_ptr->floors[0].texture_rot
        );
        
        float ground_wobble =
            -sin(area_time_passed * LIQUID_WOBBLE_TIME_SCALE) *
            LIQUID_WOBBLE_DELTA_X;
        float ground_texture_dy =
            al_get_bitmap_height(s_ptr->floors[0].texture_bitmap) * 0.5;
            
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx + ground_wobble;
            av[v].v = vy + ground_texture_dy;
            av[v].color =
                al_map_rgba(
                    s_ptr->brightness, s_ptr->brightness, s_ptr->brightness,
                    128
                );
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        al_draw_prim(
            av, NULL, s_ptr->floors[0].texture_bitmap,
            0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
    }
    
    //Layer 2 - Tint.
    ALLEGRO_COLOR tint_color = s_ptr->associated_liquid->main_color;
    tint_color.r *= s_ptr->brightness / 255.0f;
    tint_color.g *= s_ptr->brightness / 255.0f;
    tint_color.b *= s_ptr->brightness / 255.0f;
    for(size_t v = 0; v < n_vertexes; ++v) {
    
        const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
        vertex* v_ptr = t_ptr->points[v % 3];
        float vx = v_ptr->x;
        float vy = v_ptr->y;
        
        av[v].x = vx - where.x;
        av[v].y = vy - where.y;
        av[v].color = tint_color;
        av[v].x *= scale;
        av[v].y *= scale;
    }
    
    al_draw_prim(
        av, NULL, NULL,
        0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
    
    //Layers 3 and 4 - Water surface texture.
    for(unsigned char l = 0; l < 2; ++l) {
    
        sprite* anim_sprite = NULL;
        float layer_2_dy = 0;
        float layer_speed[2];
        layer_speed[0] = s_ptr->associated_liquid->surface_speed[0];
        layer_speed[1] = s_ptr->associated_liquid->surface_speed[1];
        float alpha = s_ptr->associated_liquid->surface_alpha;
        
        if(s_ptr->associated_liquid->anim_instance.get_cur_sprite()) {
            anim_sprite =
                s_ptr->associated_liquid->anim_instance.get_cur_sprite();
            if(anim_sprite->bitmap) {
                layer_2_dy =
                    (anim_sprite->file_size.y * 0.5) * anim_sprite->scale.x;
            }
        }
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            av[v].u = vx + (area_time_passed * layer_speed[l]);
            av[v].v = vy + (layer_2_dy * l);
            av[v].color =
                al_map_rgba(
                    s_ptr->brightness,
                    s_ptr->brightness,
                    s_ptr->brightness,
                    alpha
                );
            av[v].x *= scale;
            av[v].y *= scale;
            av[v].u /= anim_sprite->scale.x;
            av[v].v /= anim_sprite->scale.x;
        }
        
        al_draw_prim(
            av, NULL, anim_sprite->bitmap,
            0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
    }
    
    delete[] av;
}



/* ----------------------------------------------------------------------------
 * Draws the loading screen for an area (or anything else, really).
 * text:    The main text to show, optional.
 * subtext: Subtext to show under the main text, optional.
 * opacity: 0 to 1. The background blackness lowers in opacity much faster.
 */
void draw_loading_screen(
    const string &text, const string &subtext, const float opacity
) {

    const float LOADING_SCREEN_SUBTITLE_SCALE = 0.6f;
    const int LOADING_SCREEN_PADDING = 64;
    
    unsigned char blackness_alpha = 255.0f * max(0.0f, opacity * 4 - 3);
    al_draw_filled_rectangle(
        0, 0, scr_w, scr_h, al_map_rgba(0, 0, 0, blackness_alpha)
    );
    
    int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
    al_get_separate_blender(
        &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
    );
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
    
    //Set up the bitmap that will hold the text.
    int text_w = 0, text_h = 0;
    if(!text.empty()) {
        if(!loading_text_bmp) {
            //No main text buffer? Create it!
            
            get_multiline_text_dimensions(
                font_area_name, text, &text_w, &text_h
            );
            loading_text_bmp = al_create_bitmap(text_w, text_h);
            
            //Draw the main text on its bitmap.
            al_set_target_bitmap(loading_text_bmp); {
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                draw_text_lines(
                    font_area_name, al_map_rgb(255, 215, 0),
                    point(), ALLEGRO_ALIGN_LEFT, 0,
                    text
                );
            } al_set_target_backbuffer(display);
            
        } else {
            text_w = al_get_bitmap_width(loading_text_bmp);
            text_h = al_get_bitmap_height(loading_text_bmp);
        }
        
    }
    
    int subtext_w = 0, subtext_h = 0;
    if(!subtext.empty()) {
    
        if(!loading_subtext_bmp) {
            //No subtext buffer? Create it!
            get_multiline_text_dimensions(
                font_area_name, subtext, &subtext_w, &subtext_h
            );
            loading_subtext_bmp = al_create_bitmap(subtext_w, subtext_h);
            
            al_set_target_bitmap(loading_subtext_bmp); {
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                draw_text_lines(
                    font_area_name, al_map_rgb(224, 224, 224),
                    point(),
                    ALLEGRO_ALIGN_LEFT, 0,
                    subtext
                );
                
            } al_set_target_backbuffer(display);
            
            //We'll be scaling this, so let's update the mipmap.
            loading_subtext_bmp = recreate_bitmap(loading_subtext_bmp);
            
        } else {
            subtext_w = al_get_bitmap_width(loading_subtext_bmp);
            subtext_h = al_get_bitmap_height(loading_subtext_bmp);
        }
        
    }
    
    al_set_separate_blender(
        old_op, old_src, old_dst, old_aop, old_asrc, old_adst
    );
    
    //Draw the text bitmap in its place.
    float text_y = 0;
    if(!text.empty()) {
    
        text_y =
            subtext.empty() ?
            (scr_h * 0.5 - text_h * 0.5) :
            (scr_h * 0.5 - LOADING_SCREEN_PADDING * 0.5 - text_h);
        al_draw_tinted_bitmap(
            loading_text_bmp, al_map_rgba(255, 255, 255, 255.0 * opacity),
            scr_w * 0.5 - text_w * 0.5, text_y, 0
        );
        
    }
    
    //Draw the subtext bitmap in its place.
    float subtext_y = scr_h * 0.5 + LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_tinted_scaled_bitmap(
            loading_subtext_bmp, al_map_rgba(255, 255, 255, 255.0 * opacity),
            0, 0, subtext_w, subtext_h,
            scr_w * 0.5 - (subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5),
            subtext_y,
            subtext_w * LOADING_SCREEN_SUBTITLE_SCALE,
            subtext_h * LOADING_SCREEN_SUBTITLE_SCALE,
            0
        );
        
    }
    
    unsigned char reflection_alpha = 128.0 * opacity;
    
    //Now, draw the polygon that will hold the reflection for the text.
    if(!text.empty()) {
    
        ALLEGRO_VERTEX text_vertexes[4];
        float text_reflection_h =
            min((int) (LOADING_SCREEN_PADDING * 0.5), text_h);
        //Top-left vertex.
        text_vertexes[0].x = scr_w * 0.5 - text_w * 0.5;
        text_vertexes[0].y = text_y + text_h;
        text_vertexes[0].z = 0;
        text_vertexes[0].u = 0;
        text_vertexes[0].v = text_h;
        text_vertexes[0].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        text_vertexes[1].x = scr_w * 0.5 + text_w * 0.5;
        text_vertexes[1].y = text_y + text_h;
        text_vertexes[1].z = 0;
        text_vertexes[1].u = text_w;
        text_vertexes[1].v = text_h;
        text_vertexes[1].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        text_vertexes[2].x = scr_w * 0.5 + text_w * 0.5;
        text_vertexes[2].y = text_y + text_h + text_reflection_h;
        text_vertexes[2].z = 0;
        text_vertexes[2].u = text_w;
        text_vertexes[2].v = text_h - text_reflection_h;
        text_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        text_vertexes[3].x = scr_w * 0.5 - text_w * 0.5;
        text_vertexes[3].y = text_y + text_h + text_reflection_h;
        text_vertexes[3].z = 0;
        text_vertexes[3].u = 0;
        text_vertexes[3].v = text_h - text_reflection_h;
        text_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            text_vertexes, NULL, loading_text_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //And the polygon for the subtext.
    if(!subtext.empty()) {
    
        ALLEGRO_VERTEX subtext_vertexes[4];
        float subtext_reflection_h =
            min(
                (int) (LOADING_SCREEN_PADDING * 0.5),
                (int) (text_h * LOADING_SCREEN_SUBTITLE_SCALE)
            );
        //Top-left vertex.
        subtext_vertexes[0].x =
            scr_w * 0.5 - subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[0].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[0].z = 0;
        subtext_vertexes[0].u = 0;
        subtext_vertexes[0].v = subtext_h;
        subtext_vertexes[0].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        subtext_vertexes[1].x =
            scr_w * 0.5 + subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[1].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[1].z = 0;
        subtext_vertexes[1].u = subtext_w;
        subtext_vertexes[1].v = subtext_h;
        subtext_vertexes[1].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        subtext_vertexes[2].x =
            scr_w * 0.5 + subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[2].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE +
            subtext_reflection_h;
        subtext_vertexes[2].z = 0;
        subtext_vertexes[2].u = subtext_w;
        subtext_vertexes[2].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        subtext_vertexes[3].x =
            scr_w * 0.5 - subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[3].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE +
            subtext_reflection_h;
        subtext_vertexes[3].z = 0;
        subtext_vertexes[3].u = 0;
        subtext_vertexes[3].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            subtext_vertexes, NULL, loading_subtext_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //Draw the game's logo to the left of the "Loading..." text.
    if(opacity == 1.0f) {
        point icon_pos(
            scr_w - 8 - al_get_text_width(font_main, "Loading...") -
            8 - font_main_h * 0.5,
            scr_h - 8 - font_main_h * 0.5
        );
        
        if(bmp_icon && bmp_icon != bmp_error) {
            draw_bitmap(
                bmp_icon, icon_pos, point(-1, font_main_h),
                0, al_map_rgba(255, 255, 255, opacity * 255.0)
            );
        }
        
        //Draw the "Loading..." text, if we're not fading.
        al_draw_text(
            font_main, al_map_rgb(192, 192, 192),
            scr_w - 8,
            scr_h - 8 - font_main_h,
            ALLEGRO_ALIGN_RIGHT, "Loading..."
        );
    }
    
}


/* ----------------------------------------------------------------------------
 * Draws a notification, like a note saying that the player can press
 * a certain button to pluck.
 * target:  Spot that the notification is pointing at.
 * text:    Text to say.
 * control: If not NULL, draw the control's button/key/etc. before the text.
 */
void draw_notification(
    const point &target, const string &text, control_info* control
) {

    ALLEGRO_TRANSFORM tra, old;
    al_identity_transform(&tra);
    al_translate_transform(&tra, target.x * cam_zoom, target.y * cam_zoom);
    al_scale_transform(&tra, 1.0 / cam_zoom, 1.0 / cam_zoom);
    al_copy_transform(&old, al_get_current_transform());
    al_compose_transform(&tra, &old);
    al_use_transform(&tra);
    
    int bmp_w = al_get_bitmap_width(bmp_notification);
    int bmp_h = al_get_bitmap_height(bmp_notification);
    
    float text_box_x1 = -bmp_w * 0.5 + NOTIFICATION_PADDING;
    float text_box_x2 = bmp_w * 0.5 - NOTIFICATION_PADDING;
    float text_box_y1 = -bmp_h - NOTIFICATION_PADDING;
    float text_box_y2 = NOTIFICATION_PADDING;
    
    draw_bitmap(
        bmp_notification,
        point(0, -bmp_h * 0.5),
        point(bmp_w, bmp_h),
        0,
        map_alpha(NOTIFICATION_ALPHA)
    );
    
    if(control) {
        text_box_x1 += NOTIFICATION_CONTROL_SIZE + NOTIFICATION_PADDING;
        draw_control(
            font_main, *control,
            point(
                -bmp_w * 0.5 + NOTIFICATION_PADDING +
                NOTIFICATION_CONTROL_SIZE * 0.5,
                -bmp_h * 0.5
            ),
            point(
                NOTIFICATION_CONTROL_SIZE,
                NOTIFICATION_CONTROL_SIZE
            )
        );
    }
    
    draw_compressed_text(
        font_main, map_alpha(NOTIFICATION_ALPHA),
        point(
            (text_box_x1 + text_box_x2) * 0.5,
            (text_box_y1 + text_box_y2) * 0.5
        ),
        ALLEGRO_ALIGN_CENTER,
        1,
        point(
            text_box_x2 - text_box_x1,
            text_box_y2 - text_box_y1
        ),
        text
    );
    
    al_use_transform(&old);
}


/* ----------------------------------------------------------------------------
 * Draws a mob's shadow.
 * center:         Center of the mob.
 * diameter:       Diameter of the mob.
 * delta_z:        The mob is these many units above the floor
 *   directly below it.
 * shadow_stretch: How much to stretch the shadow by
 *   (used to simulate sun shadow direction casting).
 */
void draw_mob_shadow(
    const point &center, const float diameter,
    const float delta_z, const float shadow_stretch
) {

    if(shadow_stretch <= 0) return;
    
    float shadow_x = 0;
    float shadow_w =
        diameter + (diameter * shadow_stretch * MOB_SHADOW_STRETCH_MULT);
        
    if(day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + diameter * 0.5;
        shadow_x -= shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadow_x = -(diameter * 0.5);
        shadow_x += shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
    }
    
    
    draw_bitmap(
        bmp_shadow,
        point(center.x + shadow_x + shadow_w / 2, center.y),
        point(shadow_w, diameter),
        0,
        map_alpha(255 * (1 - shadow_stretch))
    );
}


/* ----------------------------------------------------------------------------
 * Draws text, scaled
 * font - color: The parameters you'd use for al_draw_text.
 * where:        Coordinates to draw in.
 * scale:        Horizontal or vertical scale.
 * flags:        Same flags you'd use for al_draw_text.
 * valign:       Vertical align. 0: top, 1: center, 2: bottom.
 * text:         Text to draw.
 */
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const unsigned char valign, const string &text
) {

    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale.x, scale.y);
    al_translate_transform(&scale_transform, where.x, where.y);
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        draw_text_lines(font, color, point(), flags, valign, text);
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws the wall shadows that are being cast on top of this sector.
 * s:        The sector to draw.
 * where:    Top-left coordinates.
 * scale:    Drawing scale.
 */
void draw_sector_shadows(sector* s_ptr, const point &where, const float scale) {
    if(s_ptr->is_bottomless_pit) return;
    
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edge* e_ptr = s_ptr->edges[e];
        ALLEGRO_VERTEX av[4];
        
        sector* other_sector =
            e_ptr->sectors[(e_ptr->sectors[0] == s_ptr ? 1 : 0)];
            
        if(!casts_shadow(other_sector, s_ptr)) continue;
        
        float shadow_length =
            get_wall_shadow_length(
                other_sector->floors[0].z -
                s_ptr->floors[0].z
            );
            
        /*
         * We need to record the two vertexes of the edge as
         * the two starting points of the procedure.
         * Starting from vertex 0, if our sector is to the "left"
         * then vertex 0 of the shadow is vertex of the edge.
         * Otherwise, swap it around.
         */
        vertex* ev[2];
        
        if(e_ptr->sectors[0] == s_ptr) {
            ev[0] = e_ptr->vertexes[0];
            ev[1] = e_ptr->vertexes[1];
        } else {
            ev[0] = e_ptr->vertexes[1];
            ev[1] = e_ptr->vertexes[0];
        }
        
        float e_angle =
            get_angle(point(ev[0]->x, ev[0]->y), point(ev[1]->x, ev[1]->y));
        float e_dist =
            dist(
                point(ev[0]->x, ev[0]->y),
                point(ev[1]->x, ev[1]->y)
            ).to_float();
        float e_cos_front = cos(e_angle - M_PI_2);
        float e_sin_front = sin(e_angle - M_PI_2);
        
        //Record the first two vertexes of the shadow.
        for(size_t v = 0; v < 2; ++v) {
            av[v].x = ev[v]->x;
            av[v].y = ev[v]->y;
            av[v].color = al_map_rgba(0, 0, 0, WALL_SHADOW_OPACITY);
            av[v].z = 0;
            
        }
        
        
        /*
         * Now, check the neighbor edges.
         * Record which angle this edge makes against
         * them. The shadow of the current edge
         * spreads outward from the edge, but the edges must
         * be tilted so that the shadow from this edge
         * meets up with the shadow from the next, on a middle
         * angle. For 90 degrees, at least. Less or more degrees
         * requires specific treatment.
         */
        
        //Angle of the neighbors, from the common vertex to the other.
        float neighbor_angles[2] = {M_PI_2, M_PI_2};
        //Difference between angle of current edge and neighbors.
        float neighbor_angle_difs[2] = {0, 0};
        //Midway angle.
        float mid_angles[2] = {M_PI_2, M_PI_2};
        //Is this neighbor casting a shadow to the same sector?
        bool neighbor_shadow[2] = {false, false};
        //Length of the neighbor's shadow.
        float neighbor_shadow_length[2] = {0.0f, 0.0f};
        //Do we have an edge for this vertex?
        bool got_first[2] = {false, false};
        
        //For both neighbors.
        for(unsigned char v = 0; v < 2; ++v) {
        
            vertex* cur_vertex = ev[v];
            for(size_t ve = 0; ve < cur_vertex->edges.size(); ++ve) {
            
                edge* ve_ptr = cur_vertex->edges[ve];
                
                if(ve_ptr == e_ptr) continue;
                
                vertex* other_vertex =
                    ve_ptr->vertexes[
                        (ve_ptr->vertexes[0] == cur_vertex ? 1 : 0)
                ];
                float ve_angle =
                    get_angle(
                        point(cur_vertex->x, cur_vertex->y),
                        point(other_vertex->x, other_vertex->y)
                    );
                    
                float d;
                if(v == 0) d = get_angle_cw_dif(ve_angle, e_angle);
                else d = get_angle_cw_dif(e_angle + M_PI, ve_angle);
                
                if(
                    d < neighbor_angle_difs[v] ||
                    !got_first[v]
                ) {
                    //Save this as the next edge.
                    neighbor_angles[v] = ve_angle;
                    neighbor_angle_difs[v] = d;
                    got_first[v] = true;
                    
                    sector* other_sector =
                        ve_ptr->sectors[(ve_ptr->sectors[0] == s_ptr ? 1 : 0)];
                    neighbor_shadow[v] =
                        casts_shadow(other_sector, s_ptr);
                        
                    //Get the shadow length.
                    //Defaulting to the current sector's length
                    //makes it easier to calculate things later on.
                    neighbor_shadow_length[v] =
                        neighbor_shadow[v] ?
                        get_wall_shadow_length(
                            other_sector->floors[0].z -
                            s_ptr->floors[0].z
                        ) :
                        shadow_length;
                }
            }
        }
        
        e_angle = normalize_angle(e_angle);
        for(unsigned char n = 0; n < 2; ++n) {
            neighbor_angles[n] = normalize_angle(neighbor_angles[n]);
            mid_angles[n] =
                (n == 0 ? neighbor_angles[n] : e_angle + M_PI) +
                neighbor_angle_difs[n] / 2;
        }
        
        point shadow_point[2];
        ALLEGRO_VERTEX extra_av[8];
        for(unsigned char e = 0; e < 8; ++e) { extra_av[e].z = 0;}
        //How many vertexes of the extra polygon to draw.
        unsigned char draw_extra[2] = {0, 0};
        
        for(unsigned char v = 0; v < 2; ++v) {
        
            if(neighbor_angle_difs[v] < M_PI && neighbor_shadow[v]) {
                //If the shadow of the current and neighbor edges
                //meet at less than 180 degrees, and the neighbor casts
                //a shadow, then both this shadow and the neighbor's
                //must blend in with one another. This shadow's final
                //point should be where they both intersect.
                //The neighbor's shadow will do the same when we get to it.
                
                float ul;
                float shadow_length_mid =
                    (shadow_length + neighbor_shadow_length[v]) / 2.0f;
                lines_intersect(
                    point(
                        av[0].x + e_cos_front * shadow_length_mid,
                        av[0].y + e_sin_front * shadow_length_mid
                    ),
                    point(
                        av[1].x + e_cos_front * shadow_length_mid,
                        av[1].y + e_sin_front * shadow_length_mid
                    ),
                    point(
                        av[v].x,
                        av[v].y
                    ),
                    point(
                        av[v].x + cos(
                            neighbor_shadow[v] ?
                            mid_angles[v] :
                            neighbor_angles[v]
                        ) * e_dist,
                        av[v].y + sin(
                            neighbor_shadow[v] ?
                            mid_angles[v] :
                            neighbor_angles[v]
                        ) * e_dist
                    ),
                    NULL, &ul
                );
                
                //Clamp ul to prevent long, close walls from
                //creating jagged shadows outside the wall.
                ul = clamp(ul, 0.0f, 1.0f);
                
                shadow_point[v].x =
                    av[0].x + e_cos_front * shadow_length_mid +
                    cos(e_angle) * e_dist * ul;
                shadow_point[v].y =
                    av[0].y + e_sin_front * shadow_length_mid +
                    sin(e_angle) * e_dist * ul;
                    
            } else {
                //Otherwise, just draw the
                //shadows as a rectangle, away
                //from the edge. Then, if the angle is greater
                //than 180, draw a "join" between both
                //edge's shadows. Like a kneecap.
                
                if(neighbor_angle_difs[v] > M_PI_2) {
                    shadow_point[v].x =
                        av[v].x + e_cos_front * shadow_length;
                    shadow_point[v].y =
                        av[v].y + e_sin_front * shadow_length;
                        
                    extra_av[v * 4 + 0].x = av[v].x;
                    extra_av[v * 4 + 0].y = av[v].y;
                    extra_av[v * 4 + 0].color =
                        al_map_rgba(0, 0, 0, WALL_SHADOW_OPACITY);
                    extra_av[v * 4 + 1].x = shadow_point[v].x;
                    extra_av[v * 4 + 1].y = shadow_point[v].y;
                    extra_av[v * 4 + 1].color = al_map_rgba(0, 0, 0, 0);
                    
                    if(neighbor_angle_difs[v] > M_PI) {
                        float shadow_length_mid =
                            (shadow_length + neighbor_shadow_length[v]) / 2.0f;
                            
                        //Draw the "kneecap".
                        extra_av[v * 4 + 2].x =
                            av[v].x + cos(mid_angles[v]) * shadow_length_mid;
                        extra_av[v * 4 + 2].y =
                            av[v].y + sin(mid_angles[v]) * shadow_length_mid;
                        extra_av[v * 4 + 2].color = al_map_rgba(0, 0, 0, 0);
                        
                        draw_extra[v] = 3;
                    }
                    
                    if(!neighbor_shadow[v]) {
                        //If the neighbor casts no shadow,
                        //add an extra polygon vertex;
                        //this glues the current edge's shadow to the neighbor.
                        
                        unsigned char index =
                            (draw_extra[v] == 3) ? (v * 4 + 3) : (v * 4 + 2);
                            
                        extra_av[index].x =
                            ev[v]->x + cos(neighbor_angles[v]) *
                            shadow_length;
                        extra_av[index].y =
                            ev[v]->y + sin(neighbor_angles[v]) *
                            shadow_length;
                        extra_av[index].color = al_map_rgba(0, 0, 0, 0);
                        
                        draw_extra[v] = (draw_extra[v] == 3) ? 4 : 3;
                    }
                    
                } else {
                
                    shadow_point[v].x =
                        ev[v]->x + cos(neighbor_angles[v]) * shadow_length;
                    shadow_point[v].y =
                        ev[v]->y + sin(neighbor_angles[v]) * shadow_length;
                        
                }
                
            }
            
        }
        
        av[2].x = shadow_point[1].x;
        av[2].y = shadow_point[1].y;
        av[2].color = al_map_rgba(0, 0, 0, 0);
        av[2].z = 0;
        av[3].x = shadow_point[0].x;
        av[3].y = shadow_point[0].y;
        av[3].color = al_map_rgba(0, 0, 0, 0);
        av[3].z = 0;
        
        //Before drawing, let's offset according to the area image.
        for(unsigned char a = 0; a < 4; ++a) {
            av[a].x -= where.x;
            av[a].y -= where.y;
        }
        for(unsigned char a = 0; a < 8; ++a) {
            extra_av[a].x -= where.x;
            extra_av[a].y -= where.y;
        }
        
        //Do the scaling.
        for(size_t v = 0; v < 4; ++v) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        for(size_t v = 0; v < 8; ++v) {
            extra_av[v].x *= scale;
            extra_av[v].y *= scale;
        }
        
        //Draw!
        al_draw_prim(av, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
        
        for(size_t v = 0; v < 2; ++v) {
            if(draw_extra[v] > 0) {
                al_draw_prim(
                    extra_av, NULL, NULL, v * 4,
                    v * 4 + draw_extra[v],
                    ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Draws a sector, but only the texture (no wall shadows).
 * s_ptr:   Pointer to the sector.
 * where:   X and Y offset.
 * scale:   Scale the sector by this much.
 * opacity: Draw the textures at this opacity, 0 - 1.
 */
void draw_sector_texture(
    sector* s_ptr, const point &where, const float scale, const float opacity
) {
    if(s_ptr->is_bottomless_pit) return;
    
    unsigned char n_textures = 1;
    sector* texture_sector[2] = {NULL, NULL};
    sector_floor* texture_floors[2] = {NULL, NULL};
    
    if(s_ptr->fade) {
        s_ptr->get_texture_merge_sectors(
            &texture_sector[0], &texture_sector[1]
        );
        if(!texture_sector[0] && !texture_sector[1]) {
            //Can't draw this sector.
            return;
        }
        n_textures = 2;
        
    } else {
        texture_sector[0] = s_ptr;
        if(!texture_sector[0]) {
            //Can't draw this sector.
            return;
        }
        
    }
    
    for(unsigned char t = 0; t < n_textures; ++t) {
    
        bool draw_sector_0 = true;
        if(!texture_sector[0]) draw_sector_0 = false;
        else if(texture_sector[0]->is_bottomless_pit) {
            draw_sector_0 = false;
        }
        
        if(n_textures == 2 && !draw_sector_0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        size_t n_vertexes = s_ptr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
        
        texture_floors[t] =
            &texture_sector[t]->floors[texture_sector[t]->n_floors - 1];
            
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        if(texture_sector[t]) {
            al_build_transform(
                &tra,
                -texture_floors[t]->texture_translation.x,
                -texture_floors[t]->texture_translation.y,
                1.0 / texture_floors[t]->texture_scale.x,
                1.0 / texture_floors[t]->texture_scale.y,
                -texture_floors[t]->texture_rot
            );
        }
        
        al_hold_bitmap_drawing(true);
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            float alpha_mult = 1;
            float brightness_mult = texture_sector[t]->brightness / 255.0;
            
            if(t == 1) {
                if(!draw_sector_0) {
                    alpha_mult = 0;
                    for(
                        size_t e = 0; e < texture_sector[1]->edges.size();
                        ++e
                    ) {
                        if(
                            texture_sector[1]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[1]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 1;
                        }
                    }
                } else {
                    for(
                        size_t e = 0; e < texture_sector[0]->edges.size();
                        ++e
                    ) {
                        if(
                            texture_sector[0]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[0]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            if(texture_sector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba_f(
                    texture_floors[t]->texture_tint.r * brightness_mult,
                    texture_floors[t]->texture_tint.g * brightness_mult,
                    texture_floors[t]->texture_tint.b * brightness_mult,
                    texture_floors[t]->texture_tint.a * alpha_mult *
                    opacity
                );
        }
        
        al_hold_bitmap_drawing(false);
        
        for(size_t v = 0; v < n_vertexes; ++v) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            texture_floors[t] ?
            texture_floors[t]->texture_bitmap :
            texture_floors[t == 0 ? 1 : 0]->texture_bitmap;
            
        al_draw_prim(
            av, NULL, tex,
            0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
}


/* ----------------------------------------------------------------------------
 * Draws a status effect's bitmap.
 */
void draw_status_effect_bmp(mob* m, bitmap_effect_manager* effects) {
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = m->get_status_bitmap(&status_bmp_scale);
    if(status_bmp) {
        draw_bitmap_with_effects(
            status_bmp,
            m->pos,
            point(m->type->radius * 2 * status_bmp_scale, -1),
            0, effects
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 * font:   Font to use.
 * color:  Color.
 * where:  Coordinates of the text.
 * flags:  Flags, just like the ones you'd pass to al_draw_text.
 * valign: Vertical align: 0 for top, 1 for center, 2 for bottom.
 * text:   Text to write, line breaks included ('\n').
 */
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags,
    const unsigned char valign, const string &text
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    float top;
    
    if(valign == 0) {
        top = where.y;
    } else {
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = n_lines * fh + (n_lines - 1);
        if(valign == 1) {
            top = where.y - total_height / 2;
        } else {
            top = where.y - total_height;
        }
    }
    
    for(size_t l = 0; l < n_lines; ++l) {
        float line_y = (fh + 1) * l + top;
        al_draw_text(font, color, where.x, line_y, flags, lines[l].c_str());
    }
}


/* ----------------------------------------------------------------------------
 * Eases a number [0, 1] in accordance to a non-linear interpolation
 * method. Normally used for camera movement and such.
 * method: the method to use. Use EASE_*.
 * n:      the number to ease, in the range [0, 1].
 */
float ease(const unsigned char method, const float n) {
    switch(method) {
    case EASE_IN:
        return pow(n, 3);
    case EASE_OUT:
        return 1 - (pow((1 - n), 3));
    case EASE_UP_AND_DOWN:
        return sin(n * M_PI);
    }
    
    return n;
}
