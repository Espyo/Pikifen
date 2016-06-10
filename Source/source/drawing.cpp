/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Does the drawing for the main game loop.
 * bmp_output: if not NULL, draw the area onto this.
 */
void do_game_drawing(
    ALLEGRO_BITMAP* bmp_output, ALLEGRO_TRANSFORM* bmp_transform
) {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    if(!paused) {
    
        size_t n_leaders =     leaders.size();
        size_t n_spray_types = spray_types.size();
        
        cur_sun_strength = get_sun_strength();
        
        
        ALLEGRO_TRANSFORM normal_transform;
        al_identity_transform(&normal_transform);
        
        ALLEGRO_TRANSFORM world_to_screen_transform;
        
        if(bmp_output) {
            world_to_screen_transform = *bmp_transform;
            al_set_target_bitmap(bmp_output);
            al_set_separate_blender(
                ALLEGRO_ADD, ALLEGRO_ALPHA,
                ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
                ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
            );
        } else {
            world_to_screen_transform = get_world_to_screen_transform();
        }
        
        
        /* Layer 1
        ************************
        *                +---+ *
        *   Background   |###| *
        *                +---+ *
        ***********************/
        
        al_clear_to_color(cur_area_data.bg_color);
        
        if(cur_area_data.bg_bmp) {
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
            float zoom_x = bmp_w * 0.5 * cur_area_data.bg_dist / zoom_to_use;
            float zoom_y = bmp_h * 0.5 * cur_area_data.bg_dist / zoom_to_use;
            
            bg_v[0].x = 0;
            bg_v[0].y = 0;
            bg_v[0].u = (cam_x - zoom_x) / cur_area_data.bg_bmp_zoom;
            bg_v[0].v = (cam_y - zoom_y) / cur_area_data.bg_bmp_zoom;
            bg_v[1].x = bmp_w;
            bg_v[1].y = 0;
            bg_v[1].u = (cam_x + zoom_x) / cur_area_data.bg_bmp_zoom;
            bg_v[1].v = (cam_y - zoom_y) / cur_area_data.bg_bmp_zoom;
            bg_v[2].x = bmp_w;
            bg_v[2].y = bmp_h;
            bg_v[2].u = (cam_x + zoom_x) / cur_area_data.bg_bmp_zoom;
            bg_v[2].v = (cam_y + zoom_y) / cur_area_data.bg_bmp_zoom;
            bg_v[3].x = 0;
            bg_v[3].y = bmp_h;
            bg_v[3].u = (cam_x - zoom_x) / cur_area_data.bg_bmp_zoom;
            bg_v[3].v = (cam_y + zoom_y) / cur_area_data.bg_bmp_zoom;
            
            al_draw_prim(
                bg_v, NULL, cur_area_data.bg_bmp,
                0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
            );
        }
        
        
        /* Layer 2
        *******************
        *          ^^^^^^ *
        *   Area   ^^^^^^ *
        *          ^^^^^^ *
        ******************/
        
        //Pre-rendered geometry.
        al_use_transform(&world_to_screen_transform);
        size_t area_image_cols = area_images.size();
        for(size_t x = 0; x < area_image_cols; ++x) {
        
            auto x_ptr = &area_images[x];
            size_t area_image_rows = x_ptr->size();
            
            for(size_t y = 0; y < area_image_rows; ++y) {
            
                al_draw_scaled_bitmap(
                    x_ptr->operator[](y),
                    0, 0, area_image_size, area_image_size,
                    (x * area_image_size + area_images_x1) / area_images_scale,
                    (y * area_image_size + area_images_y1) / area_images_scale,
                    area_image_size / area_images_scale,
                    area_image_size / area_images_scale,
                    0
                );
                
            }
            
        }
        
        //Liquids.
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = cur_area_data.sectors[s];
            if(s_ptr->associated_liquid) {
                draw_liquid(s_ptr, 0, 0, 1.0f);
            }
        }
        
        //Sector corrections.
        for(size_t c = 0; c < cur_area_data.sector_corrections.size(); ++c) {
            sector_correction* c_ptr = &cur_area_data.sector_corrections[c];
            if(c_ptr->new_texture.bitmap) {
                draw_sector(c_ptr->sec, 0, 0, 1.0f, &c_ptr->new_texture);
            }
        }
        
        
        /* Layer 3
        *************************************
        *                               *   *
        *   Particles (before mobs)   *   * *
        *                              * *  *
        ************************************/
        
        particles.draw_all(true);
        
        
        /* Layer 4
        ****************
        *          \o/ *
        *   Mobs    |  *
        *          / \ *
        ***************/
        
        vector<mob*> sorted_mobs;
        sorted_mobs = mobs;
        sort(
            sorted_mobs.begin(),
            sorted_mobs.end(),
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
        for(size_t m = 0; m < sorted_mobs.size(); ++m) {
            mob_ptr = sorted_mobs[m];
            if(mob_ptr->type->casts_shadow && !mob_ptr->hide) {
                draw_mob_shadow(
                    mob_ptr->x, mob_ptr->y,
                    mob_ptr->type->radius * 2,
                    mob_ptr->z - mob_ptr->ground_sector->z,
                    shadow_stretch
                );
            }
            mob_ptr->draw();
        }
        
        
        /* Layer 5
        ************************************
        *                              *   *
        *   Particles (after mobs)   *   * *
        *                             * *  *
        ***********************************/
        
        particles.draw_all(false);
        
        
        /* Layer 6
        ***************************
        *                   Help  *
        *   In-game text   --  -- *
        *                    \/   *
        **************************/
        
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
                        mob_ptr->x,
                        mob_ptr->y - mob_ptr->type->radius - font_main_h * 1.25,
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
                    mob_ptr->x,
                    mob_ptr->y - mob_ptr->type->radius -
                    DEF_HEALTH_WHEEL_RADIUS - 4,
                    mob_ptr->health, mob_ptr->type->max_health
                );
            }
        }
        
        //Info spots.
        size_t n_info_spots = info_spots.size();
        for(size_t i = 0; i < n_info_spots; ++i) {
            if(
                dist(
                    cur_leader_ptr->x, cur_leader_ptr->y,
                    info_spots[i]->x, info_spots[i]->y
                ) <= info_spot_trigger_range
            ) {
                float pivot_y = info_spots[i]->y - info_spots[i]->type->radius;
                if(!info_spots[i]->opens_box) {
                    draw_notification(
                        info_spots[i]->x, pivot_y, info_spots[i]->text, NULL
                    );
                    
                } else if(click_control_id != INVALID) {
                    draw_notification(
                        info_spots[i]->x, pivot_y, "Read",
                        &controls[0][click_control_id]
                    );
                    
                }
            }
        }
        
        
        /* Layer 7
        ***************************
        *                    /  / *
        *   Percipitation     / / *
        *                   /  /  *
        **************************/
        
        if(
            cur_area_data.weather_condition.percipitation_type !=
            PERCIPITATION_TYPE_NONE
        ) {
            size_t n_percipitation_particles = percipitation.size();
            for(size_t p = 0; p < n_percipitation_particles; ++p) {
                al_draw_filled_circle(
                    percipitation[p].x, percipitation[p].y,
                    3, al_map_rgb(255, 255, 255)
                );
            }
        }
        
        
        /* Layer 8
        **************************
        *                  *###* *
        *   Tree shadows   #| |# *
        *                   |_|  *
        *************************/
        
        if(!(bmp_output && !dev_tool_area_image_shadows)) {
        
            for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
                tree_shadow* s_ptr = cur_area_data.tree_shadows[s];
                
                unsigned char alpha =
                    ((s_ptr->alpha / 255.0) * cur_sun_strength) * 255;
                    
                draw_sprite(
                    s_ptr->bitmap,
                    s_ptr->x + TREE_SHADOW_SWAY_AMOUNT *
                    sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                    s_ptr->sway_x,
                    s_ptr->y + TREE_SHADOW_SWAY_AMOUNT *
                    sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                    s_ptr->sway_y,
                    s_ptr->w, s_ptr->h,
                    s_ptr->angle, map_alpha(alpha)
                );
            }
            
        }
        if(bmp_output) {
            al_set_target_backbuffer(display);
            return;
        }
        
        
        /* Layer 9
        ***********************
        *              --==## *
        *   Daylight   --==## *
        *              --==## *
        **********************/
        
        al_use_transform(&normal_transform);
        
        al_draw_filled_rectangle(0, 0, scr_w, scr_h, get_daylight_color());
        
        
        /* Layer 10
        *********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             '-'   *
        ********************/
        
        al_use_transform(&world_to_screen_transform);
        
        size_t n_arrows = group_move_arrows.size();
        for(size_t a = 0; a < n_arrows; ++a) {
            float x = cos(group_move_angle) * group_move_arrows[a];
            float y = sin(group_move_angle) * group_move_arrows[a];
            float alpha =
                64 + min(
                    191,
                    (int) (
                        191 * (group_move_arrows[a] / (cursor_max_dist * 0.4))
                    )
                );
            draw_sprite(
                bmp_group_move_arrow,
                cur_leader_ptr->x + x, cur_leader_ptr->y + y,
                16 * (1 + group_move_arrows[a] / cursor_max_dist), -1,
                group_move_angle,
                map_alpha(alpha)
            );
        }
        
        size_t n_rings = whistle_rings.size();
        for(size_t r = 0; r < n_rings; ++r) {
            float x = cur_leader_ptr->x + cos(cursor_angle) * whistle_rings[r];
            float y = cur_leader_ptr->y + sin(cursor_angle) * whistle_rings[r];
            unsigned char n = whistle_ring_colors[r];
            al_draw_circle(
                x, y, 8,
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
                            
                        float x = cursor_x + cos(angle) * whistle_dot_radius[d];
                        float y = cursor_y + sin(angle) * whistle_dot_radius[d];
                        
                        ALLEGRO_COLOR c;
                        float alpha_mult;
                        if(whistle_fade_timer.time_left > 0.0f)
                            alpha_mult = whistle_fade_timer.get_ratio_left();
                        else
                            alpha_mult = 1;
                            
                        if(d == 0) {
                            c = al_map_rgba(255, 0,   0,   255 * alpha_mult);
                        } else if(d == 1) {
                            c = al_map_rgba(255, 128, 0,   210 * alpha_mult);
                        } else if(d == 2) {
                            c = al_map_rgba(128, 255, 0,   165 * alpha_mult);
                        } else if(d == 3) {
                            c = al_map_rgba(0,   255, 255, 120 * alpha_mult);
                        } else if(d == 4) {
                            c = al_map_rgba(0,   0,   255, 75  * alpha_mult);
                        } else {
                            c = al_map_rgba(128, 0,   255, 30  * alpha_mult);
                        }
                        
                        al_draw_filled_circle(x, y, 2, c);
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
                    cursor_x, cursor_y, radius,
                    al_map_rgba(192, 192, 0, alpha), 2
                );
            }
        }
        
        //Cursor trail
        al_use_transform(&normal_transform);
        if(draw_cursor_trail) {
            for(size_t p = 1; p < cursor_spots.size(); ++p) {
                point* p_ptr = &cursor_spots[p];
                point* pp_ptr = &cursor_spots[p - 1]; //Previous point.
                if(
                    (*p_ptr) != (*pp_ptr) &&
                    dist(p_ptr->x, p_ptr->y, pp_ptr->x, pp_ptr->y) > 4
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
        
        //The actual cursor and mouse cursor
        draw_sprite(
            bmp_mouse_cursor,
            mouse_cursor_x, mouse_cursor_y,
            cam_zoom * al_get_bitmap_width(bmp_mouse_cursor) * 0.5,
            cam_zoom * al_get_bitmap_height(bmp_mouse_cursor) * 0.5,
            -(area_time_passed * cursor_spin_speed),
            change_color_lighting(
                cur_leader_ptr->lea_type->main_color,
                cursor_height_diff_light
            )
        );
        al_use_transform(&world_to_screen_transform);
        draw_sprite(
            bmp_cursor,
            cursor_x, cursor_y,
            al_get_bitmap_width(bmp_cursor) * 0.5,
            al_get_bitmap_height(bmp_cursor) * 0.5,
            cursor_angle,
            change_color_lighting(
                cur_leader_ptr->lea_type->main_color,
                cursor_height_diff_light
            )
        );
        
        
        /* Layer 11
        *****************
        *           (1) *
        *   HUD         *
        *         1/2/3 *
        ****************/
        
        al_use_transform(&normal_transform);
        
        if(cur_message.empty()) {
        
            //Leader health.
            for(size_t l = 0; l < 3; ++l) {
                if(n_leaders < l + 1) continue;
                
                size_t l_nr = (cur_leader_nr + l) % n_leaders;
                int icon_id = HUD_ITEM_LEADER_1_ICON + l;
                int health_id = HUD_ITEM_LEADER_1_HEALTH + l;
                
                al_draw_filled_circle(
                    hud_coords[icon_id][0],
                    hud_coords[icon_id][1],
                    max(hud_coords[icon_id][2], hud_coords[icon_id][3]) / 2.0f,
                    change_alpha(leaders[l_nr]->type->main_color, 128)
                );
                draw_sprite(
                    leaders[l_nr]->lea_type->bmp_icon,
                    hud_coords[icon_id][0],
                    hud_coords[icon_id][1],
                    hud_coords[icon_id][2],
                    hud_coords[icon_id][3]
                );
                draw_sprite(
                    bmp_bubble,
                    hud_coords[icon_id][0],
                    hud_coords[icon_id][1],
                    hud_coords[icon_id][2],
                    hud_coords[icon_id][3]
                );
                
                draw_health(
                    hud_coords[health_id][0],
                    hud_coords[health_id][1],
                    leaders[l_nr]->health, leaders[l_nr]->type->max_health,
                    max(
                        hud_coords[health_id][2],
                        hud_coords[health_id][3]
                    ) * 0.4f,
                    true
                );
                draw_sprite(
                    bmp_hard_bubble,
                    hud_coords[health_id][0],
                    hud_coords[health_id][1],
                    hud_coords[health_id][2],
                    hud_coords[health_id][3]
                );
            }
            
            //Sun Meter.
            unsigned char n_hours =
                (day_minutes_end - day_minutes_start) / 60.0f;
            float day_passed_ratio =
                (float) (day_minutes - day_minutes_start) /
                (float) (day_minutes_end - day_minutes_start);
            float sun_radius =
                hud_coords[HUD_ITEM_TIME][3] / 2.0;
            float first_dot_x =
                hud_coords[HUD_ITEM_TIME][0] -
                hud_coords[HUD_ITEM_TIME][2] * 0.5 + sun_radius;
            float last_dot_x =
                hud_coords[HUD_ITEM_TIME][0] +
                hud_coords[HUD_ITEM_TIME][2] * 0.5 - sun_radius;
            float dots_y =
                hud_coords[HUD_ITEM_TIME][1];
            //Width, from the center of the first dot to the center of the last.
            float dots_span =
                last_dot_x - first_dot_x;
            float dot_interval =
                dots_span / (float) n_hours;
            float sun_meter_sun_angle =
                area_time_passed * SUN_METER_SUN_SPIN_SPEED;
                
            //Larger bubbles at the start, middle and end of the meter.
            draw_sprite(
                bmp_hard_bubble, first_dot_x + dots_span * 0.0, dots_y,
                sun_radius * 0.9, sun_radius * 0.9
            );
            draw_sprite(
                bmp_hard_bubble, first_dot_x + dots_span * 0.5, dots_y,
                sun_radius * 0.9, sun_radius * 0.9
            );
            draw_sprite(
                bmp_hard_bubble, first_dot_x + dots_span * 1.0, dots_y,
                sun_radius * 0.9, sun_radius * 0.9
            );
            
            for(unsigned char h = 0; h < n_hours + 1; ++h) {
                draw_sprite(
                    bmp_hard_bubble,
                    first_dot_x + h * dot_interval, dots_y,
                    sun_radius * 0.6, sun_radius * 0.6);
            }
            
            draw_sprite(
                bmp_sun,
                first_dot_x + day_passed_ratio * dots_span, dots_y,
                sun_radius * 1.5, sun_radius * 1.5
            ); //Static sun.
            draw_sprite(
                bmp_sun,
                first_dot_x + day_passed_ratio * dots_span, dots_y,
                sun_radius * 1.5, sun_radius * 1.5,
                sun_meter_sun_angle
            ); //Spinning sun.
            draw_sprite(
                bmp_sun_bubble,
                first_dot_x + day_passed_ratio * dots_span, dots_y,
                sun_radius * 2.0, sun_radius * 2.0
            ); //Bubble in front the sun.
            
            //Day number.
            draw_sprite(
                bmp_day_bubble,
                hud_coords[HUD_ITEM_DAY_BUBBLE][0],
                hud_coords[HUD_ITEM_DAY_BUBBLE][1],
                hud_coords[HUD_ITEM_DAY_BUBBLE][2],
                hud_coords[HUD_ITEM_DAY_BUBBLE][3]
            );
            
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_DAY_NUMBER][0],
                hud_coords[HUD_ITEM_DAY_NUMBER][1],
                ALLEGRO_ALIGN_CENTER, 1,
                hud_coords[HUD_ITEM_DAY_NUMBER][2],
                hud_coords[HUD_ITEM_DAY_NUMBER][3],
                i2s(day)
            );
            
            //Pikmin count.
            //Count how many Pikmin only.
            n_leaders = leaders.size();
            size_t pikmin_in_group = cur_leader_ptr->group->members.size();
            for(size_t l = 0; l < n_leaders; ++l) {
                //If this leader is following the current one,
                //then they're not a Pikmin.
                //Subtract them from the group count total.
                if(leaders[l]->following_group == cur_leader_ptr) {
                    pikmin_in_group--;
                }
            }
            
            //Closest group member.
            ALLEGRO_BITMAP* bm = NULL;
            if(closest_group_member) {
                if(typeid(*closest_group_member) == typeid(pikmin)) {
                    pikmin* p_ptr = dynamic_cast<pikmin*>(closest_group_member);
                    bm = p_ptr->pik_type->bmp_icon[p_ptr->maturity];
                    
                } else if(typeid(*closest_group_member) == typeid(leader)) {
                    leader* l_ptr = dynamic_cast<leader*>(closest_group_member);
                    bm = l_ptr->lea_type->bmp_icon;
                }
                
            }
            
            size_t n_standby_pikmin = 0;
            if(closest_group_member) {
                for(
                    size_t m = 0; m < cur_leader_ptr->group->members.size();
                    ++m
                ) {
                    mob* m_ptr = cur_leader_ptr->group->members[m];
                    if(
                        (
                            typeid(*m_ptr) == typeid(leader) &&
                            typeid(*closest_group_member) == typeid(leader)
                        ) ||
                        (m_ptr->type == closest_group_member->type)
                    ) {
                        n_standby_pikmin++;
                    }
                }
            }
            
            if(!bm) bm = bmp_no_pikmin;
            float sprite_w =
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][2] == -1 ? -1 :
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][2] * 0.8;
            float sprite_h =
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][3] == -1 ? -1 :
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][3] * 0.8;
            draw_sprite(
                bm,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][0],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][1],
                sprite_w, sprite_h
            );
            draw_sprite(
                bmp_bubble,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][0],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][1],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][2],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_ICON][3]
            );
            
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_X][0],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_X][1],
                ALLEGRO_ALIGN_CENTER, 1,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_X][2],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_X][3],
                "x"
            );
            
            //Pikmin count numbers.
            unsigned long total_pikmin = pikmin_list.size();
            for(
                auto o = pikmin_in_onions.begin();
                o != pikmin_in_onions.end(); ++o
            ) {
                total_pikmin += o->second;
            }
            
            draw_sprite(
                bmp_number_bubble,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][0],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][1],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][2],
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][3]
            );
            draw_sprite(
                bmp_number_bubble,
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][0],
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][1],
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][2],
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][3]
            );
            draw_sprite(
                bmp_number_bubble,
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][0],
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][1],
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][2],
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][3]
            );
            draw_sprite(
                bmp_number_bubble,
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][0],
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][1],
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][2],
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][3]
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_SLASH_1][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_1][1],
                ALLEGRO_ALIGN_CENTER, 1,
                hud_coords[HUD_ITEM_PIKMIN_SLASH_1][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_1][0],
                "/"
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_SLASH_2][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_2][1],
                ALLEGRO_ALIGN_CENTER, 1,
                hud_coords[HUD_ITEM_PIKMIN_SLASH_2][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_2][0],
                "/"
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_SLASH_3][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_3][1],
                ALLEGRO_ALIGN_CENTER, 1,
                hud_coords[HUD_ITEM_PIKMIN_SLASH_3][0],
                hud_coords[HUD_ITEM_PIKMIN_SLASH_3][0],
                "/"
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][0] +
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][2] * 0.4,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][1],
                ALLEGRO_ALIGN_RIGHT, 1,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][2] * 0.7,
                hud_coords[HUD_ITEM_PIKMIN_STANDBY_NR][3] * 0.7,
                i2s(n_standby_pikmin)
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][0] +
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][2] * 0.4,
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][1],
                ALLEGRO_ALIGN_RIGHT, 1,
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][2] * 0.7,
                hud_coords[HUD_ITEM_PIKMIN_GROUP_NR][3] * 0.7,
                i2s(pikmin_in_group)
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][0] +
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][2] * 0.4,
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][1],
                ALLEGRO_ALIGN_RIGHT, 1,
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][2] * 0.7,
                hud_coords[HUD_ITEM_PIKMIN_FIELD_NR][3] * 0.7,
                i2s(pikmin_list.size())
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255),
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][0] +
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][2] * 0.4,
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][1],
                ALLEGRO_ALIGN_RIGHT, 1,
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][2] * 0.7,
                hud_coords[HUD_ITEM_PIKMIN_TOTAL_NR][3] * 0.7,
                i2s(total_pikmin)
            );
            
            //Sprays.
            //TODO optimize this by saving the controls somewhere,
            //instead of searching for them every time.
            if(n_spray_types > 0) {
                size_t top_spray_nr;
                if(n_spray_types < 3) top_spray_nr = 0;
                else top_spray_nr = selected_spray;
                
                draw_sprite(
                    spray_types[top_spray_nr].bmp_spray,
                    hud_coords[HUD_ITEM_SPRAY_1_ICON][0],
                    hud_coords[HUD_ITEM_SPRAY_1_ICON][1],
                    hud_coords[HUD_ITEM_SPRAY_1_ICON][2],
                    hud_coords[HUD_ITEM_SPRAY_1_ICON][3]
                );
                draw_compressed_text(
                    font_counter, al_map_rgb(255, 255, 255),
                    hud_coords[HUD_ITEM_SPRAY_1_AMOUNT][0],
                    hud_coords[HUD_ITEM_SPRAY_1_AMOUNT][1],
                    ALLEGRO_ALIGN_LEFT, 1,
                    hud_coords[HUD_ITEM_SPRAY_1_AMOUNT][2],
                    hud_coords[HUD_ITEM_SPRAY_1_AMOUNT][3],
                    "x" + i2s(spray_amounts[top_spray_nr])
                );
                
                for(size_t c = 0; c < controls[0].size(); ++c) {
                    if(
                        controls[0][c].action == BUTTON_USE_SPRAY_1 ||
                        controls[0][c].action == BUTTON_USE_SPRAY
                    ) {
                        draw_control(
                            font_main, controls[0][c],
                            hud_coords[HUD_ITEM_SPRAY_1_KEY][0],
                            hud_coords[HUD_ITEM_SPRAY_1_KEY][1],
                            hud_coords[HUD_ITEM_SPRAY_1_KEY][2],
                            hud_coords[HUD_ITEM_SPRAY_1_KEY][3]
                        );
                        break;
                    }
                }
                
                if(n_spray_types == 2) {
                    draw_sprite(
                        spray_types[1].bmp_spray,
                        hud_coords[HUD_ITEM_SPRAY_2_ICON][0],
                        hud_coords[HUD_ITEM_SPRAY_2_ICON][1],
                        hud_coords[HUD_ITEM_SPRAY_2_ICON][2],
                        hud_coords[HUD_ITEM_SPRAY_2_ICON][3]
                    );
                    draw_compressed_text(
                        font_counter, al_map_rgb(255, 255, 255),
                        hud_coords[HUD_ITEM_SPRAY_2_AMOUNT][0],
                        hud_coords[HUD_ITEM_SPRAY_2_AMOUNT][1],
                        ALLEGRO_ALIGN_LEFT, 1,
                        hud_coords[HUD_ITEM_SPRAY_2_AMOUNT][2],
                        hud_coords[HUD_ITEM_SPRAY_2_AMOUNT][3],
                        "x" + i2s(spray_amounts[1])
                    );
                    for(size_t c = 0; c < controls[0].size(); ++c) {
                        if(controls[0][c].action == BUTTON_USE_SPRAY_2) {
                            draw_control(
                                font_main, controls[0][c],
                                hud_coords[HUD_ITEM_SPRAY_2_KEY][0],
                                hud_coords[HUD_ITEM_SPRAY_2_KEY][1],
                                hud_coords[HUD_ITEM_SPRAY_2_KEY][2],
                                hud_coords[HUD_ITEM_SPRAY_2_KEY][3]
                            );
                            break;
                        }
                    }
                    
                } else if(n_spray_types > 2) {
                    draw_sprite(
                        spray_types[
                            (
                                selected_spray == 0 ?
                                spray_types.size() - 1 :
                                selected_spray - 1
                            )
                        ].bmp_spray,
                        hud_coords[HUD_ITEM_SPRAY_PREV_ICON][0],
                        hud_coords[HUD_ITEM_SPRAY_PREV_ICON][1],
                        hud_coords[HUD_ITEM_SPRAY_PREV_ICON][2],
                        hud_coords[HUD_ITEM_SPRAY_PREV_ICON][3]
                    );
                    draw_sprite(
                        spray_types[
                            (selected_spray + 1) % spray_types.size()
                        ].bmp_spray,
                        hud_coords[HUD_ITEM_SPRAY_NEXT_ICON][0],
                        hud_coords[HUD_ITEM_SPRAY_NEXT_ICON][1],
                        hud_coords[HUD_ITEM_SPRAY_NEXT_ICON][2],
                        hud_coords[HUD_ITEM_SPRAY_NEXT_ICON][3]
                    );
                    for(size_t c = 0; c < controls[0].size(); ++c) {
                        if(controls[0][c].action == BUTTON_SWITCH_SPRAY_LEFT) {
                            draw_control(
                                font_main, controls[0][c],
                                hud_coords[HUD_ITEM_SPRAY_PREV_KEY][0],
                                hud_coords[HUD_ITEM_SPRAY_PREV_KEY][1],
                                hud_coords[HUD_ITEM_SPRAY_PREV_KEY][2],
                                hud_coords[HUD_ITEM_SPRAY_PREV_KEY][3]
                            );
                            break;
                        }
                    }
                    for(size_t c = 0; c < controls[0].size(); ++c) {
                        if(controls[0][c].action == BUTTON_SWITCH_SPRAY_RIGHT) {
                            draw_control(
                                font_main, controls[0][c],
                                hud_coords[HUD_ITEM_SPRAY_NEXT_KEY][0],
                                hud_coords[HUD_ITEM_SPRAY_NEXT_KEY][1],
                                hud_coords[HUD_ITEM_SPRAY_NEXT_KEY][2],
                                hud_coords[HUD_ITEM_SPRAY_NEXT_KEY][3]
                            );
                            break;
                        }
                    }
                }
            }
            
        } else { //Show a message.
        
            draw_sprite(
                bmp_message_box,
                scr_w / 2, scr_h - font_main_h * 2 - 4,
                scr_w - 16, font_main_h * 4
            );
            
            if(cur_message_speaker) {
                draw_sprite(
                    cur_message_speaker,
                    40, scr_h - font_main_h * 4 - 16,
                    48, 48);
                draw_sprite(
                    bmp_bubble,
                    40, scr_h - font_main_h * 4 - 16,
                    64, 64);
            }
            
            string text =
                cur_message.substr(
                    cur_message_stopping_chars[cur_message_section],
                    cur_message_char -
                    cur_message_stopping_chars[cur_message_section]
                );
            vector<string> lines = split(text, "\n");
            
            al_hold_bitmap_drawing(true);
            
            for(size_t l = 0; l < lines.size(); ++l) {
            
                draw_compressed_text(
                    font_main, al_map_rgb(255, 255, 255),
                    24, scr_h - font_main_h * (4 - l) + 8,
                    ALLEGRO_ALIGN_LEFT, 0, scr_w - 64, 0,
                    lines[l]
                );
                
            }
            
            al_hold_bitmap_drawing(false);
            
        }
        
        /* Layer 12
        ***********************
        *                     *
        *   System stuff   >_ *
        *                     *
        ***********************/
        
        if(!info_print_text.empty()) {
            al_draw_filled_rectangle(
                0, 0, scr_w, scr_h * 0.3,
                al_map_rgba(0, 0, 0, 96)
            );
            draw_text_lines(
                allegro_font, al_map_rgba(255, 255, 255, 128),
                8, 8, 0, 0, info_print_text
            );
        }
        
        
        
    } else { //Paused.
    
    }
    
    //Framerate.
    if(show_framerate) {
        framerate_update_timer.tick(delta_t);
        al_draw_text(
            font_main,
            (
                framerate_counter >= (unsigned) (game_fps - 1) ?
                al_map_rgb(64, 128, 64) : al_map_rgb(128, 64, 64)
            ),
            0, 0, 0,
            (i2s(framerate_counter) + "FPS").c_str()
        );
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
 * Draws a key or button on the screen.
 * font:  Font to use for the name.
 * c:     Info on the control.
 * x, y:  Center of the place to draw at.
 * max_*: Max width or height. Used to compress it if needed.
 */
void draw_control(
    const ALLEGRO_FONT* const font, const control_info &c,
    const float x, const float y, const float max_w, const float max_h
) {

    if(c.type == CONTROL_TYPE_MOUSE_BUTTON) {
        //If it's a mouse click, just draw the icon and be done with it.
        if(c.button >= 1 && c.button <= 3) {
        
            int bmp_w =
                al_get_bitmap_width(bmp_mouse_button_icon[c.button - 1]);
            int bmp_h =
                al_get_bitmap_height(bmp_mouse_button_icon[c.button - 1]);
            draw_sprite(
                bmp_mouse_button_icon[c.button - 1],
                x, y,
                min((float) bmp_w, max_w),
                min((float) bmp_h, max_h)
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
        
        int bmp_w = al_get_bitmap_width(b);
        int bmp_h = al_get_bitmap_height(b);
        draw_sprite(
            b, x, y,
            min((float) bmp_w, max_w),
            min((float) bmp_h, max_h)
        );
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
        min((float) (x2 - x1 + 4), (max_w == 0 ? FLT_MAX : max_w));
    float total_height =
        min((float) (y2 - y1 + 4), (max_h == 0 ? FLT_MAX : max_h));
    total_width = max(total_width, total_height);
    
    if(c.type == CONTROL_TYPE_KEYBOARD_KEY) {
        al_draw_filled_rectangle(
            x - total_width * 0.5, y - total_height * 0.5,
            x + total_width * 0.5, y + total_height * 0.5,
            map_alpha(192)
        );
        al_draw_rectangle(
            x - total_width * 0.5, y - total_height * 0.5,
            x + total_width * 0.5, y + total_height * 0.5,
            al_map_rgba(160, 160, 160, 192), 2
        );
    } else {
        al_draw_filled_ellipse(
            x, y, total_width * 0.5, total_height * 0.5, map_alpha(192)
        );
        al_draw_ellipse(
            x, y, total_width * 0.5, total_height * 0.5,
            al_map_rgba(160, 160, 160, 192), 2
        );
    }
    draw_compressed_text(
        font, map_alpha(192), x, y, ALLEGRO_ALIGN_CENTER, 1,
        (max_w == 0 ? 0 : max_w - 2), (max_h == 0 ? 0 : max_h - 2), name
    );
}


/* ----------------------------------------------------------------------------
 * Does sector s1 cast a shadow onto sector s2?
 */
bool casts_shadow(sector* s1, sector* s2) {
    if(!s1 || !s2) return false;
    if(s1->type == SECTOR_TYPE_BOTTOMLESS_PIT) return false;
    if(s2->type == SECTOR_TYPE_BOTTOMLESS_PIT) return false;
    if(s1->z > s2->z && s1->always_cast_shadow) return true;
    if(s1->z <= s2->z + SECTOR_STEP) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Draws text on the screen, but compresses (scales) it
 * to fit within the specified range.
 * font - flags: The parameters you'd use for al_draw_text.
 * valign:       Vertical align: 0 = top, 1 = middle, 2 = bottom.
 * max_w, max_h: The maximum width and height. Use <= 0 to have no limit.
 * text:         Text to draw.
 */
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const float x, const float y, const int flags, const unsigned char valign,
    const float max_w, const float max_h, const string &text
) {
    int x1, x2, y1, y2;
    al_get_text_dimensions(font, text.c_str(), &x1, &y1, &x2, &y2);
    int text_width = x2 - x1, text_height = y2 - y1;
    float scale_x = 1, scale_y = 1;
    float final_text_height = text_height;
    
    if(text_width > max_w && max_w > 0) scale_x = max_w / text_width;
    if(text_height > max_h && max_h > 0) {
        scale_y = max_h / text_height;
        final_text_height = max_h;
    }
    
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale_x, scale_y);
    al_translate_transform(
        &scale_transform, x,
        (
            (valign == 1) ?
            y - final_text_height * 0.5 :
            ((valign == 2) ? y - final_text_height : y)
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
 * c*:      Center of the text.
 * current: Current strength.
 * needed:  Needed strength to lift the object (weight).
 * color:   Color of the fraction's text.
 */
void draw_fraction(
    const float cx, const float cy, const unsigned int current,
    const unsigned int needed, const ALLEGRO_COLOR &color
) {
    float first_y = cy - (font_main_h * 3) / 2;
    float font_h = al_get_font_line_height(font_value);
    
    draw_scaled_text(
        font_value, color, cx, first_y,
        (current >= needed ? 1.2 : 1.0),
        (current >= needed ? 1.2 : 1.0),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(current).c_str())
    );
    
    al_draw_text(
        font_value, color, cx, first_y + font_h * 0.75,
        ALLEGRO_ALIGN_CENTER, "-"
    );
    
    draw_scaled_text(
        font_value, color, cx, first_y + font_h * 1.5,
        (needed > current ? 1.2 : 1.0),
        (needed > current ? 1.2 : 1.0),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(needed).c_str())
    );
}


/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * c*:         Center of the wheel.
 * health:     Current amount of health of the mob
   * whose health we're representing.
 * max_health: Maximum amount of health of the mob;
   * health for when it's fully healed.
 * radius:     Radius of the wheel (the whole wheel, not just the pieslice).
 * just_chart: If true, only draw the actual pieslice (pie-chart).
   * Used for leader HP on the HUD.
 */
void draw_health(
    const float cx, const float cy,
    const unsigned int health, const unsigned int max_health,
    const float radius, const bool just_chart
) {
    float ratio = (float) health / (float) max_health;
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgb_f(1 - (ratio - 0.5) * 2, 1, 0);
    } else {
        c = al_map_rgb_f(1, (ratio * 2), 0);
    }
    
    if(!just_chart) {
        al_draw_filled_circle(cx, cy, radius, al_map_rgba(0, 0, 0, 128));
    }
    al_draw_filled_pieslice(cx, cy, radius, -M_PI_2, -ratio * M_PI * 2, c);
    if(!just_chart) {
        al_draw_circle(cx, cy, radius + 1, al_map_rgb(0, 0, 0), 2);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a notification, like a note saying that the player can press
 * a certain button to pluck.
 * x, y:    Spot that the notification is pointing at.
 * text:    Text to say.
 * control: If not NULL, draw the control's button/key/etc. before the text.
 */
void draw_notification(
    const float x, const float y, const string &text, control_info* control
) {

    ALLEGRO_TRANSFORM tra, old;
    al_identity_transform(&tra);
    al_translate_transform(&tra, x * cam_zoom, y * cam_zoom);
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
    
    draw_sprite(
        bmp_notification,
        0,
        -bmp_h * 0.5,
        bmp_w, bmp_h,
        0,
        map_alpha(NOTIFICATION_ALPHA)
    );
    
    if(control) {
        text_box_x1 += NOTIFICATION_CONTROL_SIZE + NOTIFICATION_PADDING;
        draw_control(
            font_main, *control,
            -bmp_w * 0.5 + NOTIFICATION_PADDING +
            NOTIFICATION_CONTROL_SIZE * 0.5,
            -bmp_h * 0.5,
            NOTIFICATION_CONTROL_SIZE,
            NOTIFICATION_CONTROL_SIZE
        );
    }
    
    draw_compressed_text(
        font_main, map_alpha(NOTIFICATION_ALPHA),
        (text_box_x1 + text_box_x2) * 0.5,
        (text_box_y1 + text_box_y2) * 0.5,
        ALLEGRO_ALIGN_CENTER,
        1,
        text_box_x2 - text_box_x1,
        text_box_y2 - text_box_y1,
        text
    );
    
    al_use_transform(&old);
}


/* ----------------------------------------------------------------------------
 * Draws text, scaled
 * font - y: The parameters you'd use for al_draw_text.
 * scale_*:  Horizontal or vertical scale.
 * flags:    Same flags you'd use for al_draw_text.
 * valign:   Vertical align. 0: top, 1: center, 2: bottom.
 * text:     Text to draw.
 */
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const float x, const float y, const float scale_x, const float scale_y,
    const int flags, const unsigned char valign, const string &text
) {

    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale_x, scale_y);
    al_translate_transform(&scale_transform, x, y);
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        draw_text_lines(font, color, 0, 0, flags, valign, text);
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws a sector on the current bitmap.
 * s:        The sector to draw.
 * x, y:     Top-left coordinates.
 * scale:    Drawing scale.
 * texture:  Custom texture. If NULL, uses the normal one.
 */
void draw_sector(
    sector* s_ptr, const float x, const float y, const float scale,
    sector_texture_info* texture
) {

    if(s_ptr->type == SECTOR_TYPE_BOTTOMLESS_PIT) return;
    
    draw_sector_texture(s_ptr, x, y, scale, texture);
    
    
    //Wall shadows.
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edge* e_ptr = s_ptr->edges[e];
        ALLEGRO_VERTEX av[4];
        
        sector* other_sector =
            e_ptr->sectors[(e_ptr->sectors[0] == s_ptr ? 1 : 0)];
            
        if(!casts_shadow(other_sector, s_ptr)) continue;
        
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
        
        float e_angle = atan2(ev[1]->y - ev[0]->y, ev[1]->x - ev[0]->x);
        float e_dist = dist(ev[0]->x, ev[0]->y, ev[1]->x, ev[1]->y).to_float();
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
        float neighbor_shadow[2] = {false, false};
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
                    atan2(
                        other_vertex->y - cur_vertex->y,
                        other_vertex->x - cur_vertex->x
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
                lines_intersect(
                    av[0].x + e_cos_front * WALL_SHADOW_LENGTH, av[0].y +
                    e_sin_front * WALL_SHADOW_LENGTH,
                    av[1].x + e_cos_front * WALL_SHADOW_LENGTH, av[1].y +
                    e_sin_front * WALL_SHADOW_LENGTH,
                    av[v].x,
                    av[v].y,
                    av[v].x + cos(
                        neighbor_shadow[v] ? mid_angles[v] : neighbor_angles[v]
                    ) * e_dist,
                    av[v].y + sin(
                        neighbor_shadow[v] ? mid_angles[v] : neighbor_angles[v]
                    ) * e_dist,
                    NULL, &ul
                );
                shadow_point[v].x =
                    av[0].x + e_cos_front * WALL_SHADOW_LENGTH +
                    cos(e_angle) * e_dist * ul;
                shadow_point[v].y =
                    av[0].y + e_sin_front * WALL_SHADOW_LENGTH +
                    sin(e_angle) * e_dist * ul;
                    
            } else {
                //Otherwise, just draw the
                //shadows as a rectangle, away
                //from the edge. Then, if the angle is greater
                //than 180, draw a "join" between both
                //edge's shadows. Like a kneecap.
                
                if(neighbor_angle_difs[v] > M_PI_2) {
                    shadow_point[v].x =
                        av[v].x + e_cos_front * WALL_SHADOW_LENGTH;
                    shadow_point[v].y =
                        av[v].y + e_sin_front * WALL_SHADOW_LENGTH;
                        
                    extra_av[v * 4 + 0].x = av[v].x;
                    extra_av[v * 4 + 0].y = av[v].y;
                    extra_av[v * 4 + 0].color =
                        al_map_rgba(0, 0, 0, WALL_SHADOW_OPACITY);
                    extra_av[v * 4 + 1].x = shadow_point[v].x;
                    extra_av[v * 4 + 1].y = shadow_point[v].y;
                    extra_av[v * 4 + 1].color = al_map_rgba(0, 0, 0, 0);
                    
                    if(neighbor_angle_difs[v] > M_PI) {
                        //Draw the "kneecap".
                        extra_av[v * 4 + 2].x =
                            av[v].x + cos(mid_angles[v]) * WALL_SHADOW_LENGTH;
                        extra_av[v * 4 + 2].y =
                            av[v].y + sin(mid_angles[v]) * WALL_SHADOW_LENGTH;
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
                            WALL_SHADOW_LENGTH;
                        extra_av[index].y =
                            ev[v]->y + sin(neighbor_angles[v]) *
                            WALL_SHADOW_LENGTH;
                        extra_av[index].color = al_map_rgba(0, 0, 0, 0);
                        
                        draw_extra[v] = (draw_extra[v] == 3) ? 4 : 3;
                    }
                    
                } else {
                
                    shadow_point[v].x =
                        ev[v]->x + cos(neighbor_angles[v]) * WALL_SHADOW_LENGTH;
                    shadow_point[v].y =
                        ev[v]->y + sin(neighbor_angles[v]) * WALL_SHADOW_LENGTH;
                        
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
            av[a].x -= x;
            av[a].y -= y;
        }
        for(unsigned char a = 0; a < 8; ++a) {
            extra_av[a].x -= x;
            extra_av[a].y -= y;
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
 * x, y:    X and Y offset.
 * scale:   Scale the sector by this much.
 * texture: Custom texture. If NULL, uses the normal one.
 */
void draw_sector_texture(
    sector* s_ptr, const float x, const float y, const float scale,
    sector_texture_info* texture
) {
    unsigned char n_textures = 1;
    sector* texture_sector[2] = {NULL, NULL};
    
    if(s_ptr->fade) {
        //Check all edges to find which two textures need merging.
        edge* e_ptr = NULL;
        sector* neighbor = NULL;
        bool valid = true;
        map<sector*, dist> neighbors;
        
        //The two neighboring sectors with the lenghtiest edges are picked.
        //So save all sector/length pairs.
        //Sectors with different heights from the current one are also saved,
        //but they have lower priority compared to same-heigh sectors.
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            e_ptr = s_ptr->edges[e];
            valid = true;
            
            if(e_ptr->sectors[0] == s_ptr) neighbor = e_ptr->sectors[1];
            else neighbor = e_ptr->sectors[0];
            
            if(neighbor) {
                if(neighbor->fade) valid = false;
            }
            
            if(valid) {
                neighbors[neighbor] +=
                    dist(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    );
            }
        }
        
        //Find the two lengthiest ones.
        vector<pair<dist, sector*> > neighbors_vec;
        for(auto n = neighbors.begin(); n != neighbors.end(); ++n) {
            neighbors_vec.push_back(
                make_pair(
                    //Yes, we do need these casts, for g++.
                    (dist) (n->second), (sector*) (n->first)
                )
            );
        }
        sort(
            neighbors_vec.begin(), neighbors_vec.end(),
        [s_ptr] (pair<dist, sector*> p1, pair<dist, sector*> p2) -> bool {
            return p1.first < p2.first;
        }
        );
        if(neighbors_vec.size() >= 1) {
            texture_sector[0] = neighbors_vec.back().second;
        }
        if(neighbors_vec.size() >= 2) {
            texture_sector[1] = neighbors_vec[neighbors_vec.size() - 2].second;
        }
        
        if(!texture_sector[1] && texture_sector[0]) {
            //0 is always the bottom one. If we're fading into nothingness,
            //we should swap first.
            swap(texture_sector[0], texture_sector[1]);
        } else if(!texture_sector[1]) {
            //Nothing to draw.
            return;
        } else if(texture_sector[1]->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            swap(texture_sector[0], texture_sector[1]);
        }
        
        n_textures = 2;
        
    } else {
        texture_sector[0] = s_ptr;
        
    }
    
    for(unsigned char t = 0; t < n_textures; ++t) {
    
        bool draw_sector_0 = true;
        if(!texture_sector[0]) draw_sector_0 = false;
        else if(texture_sector[0]->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            draw_sector_0 = false;
        }
        
        if(n_textures == 2 && !draw_sector_0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        size_t n_vertexes = s_ptr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
        
        sector_texture_info* texture_info_to_use;
        if(texture) texture_info_to_use = texture;
        else texture_info_to_use = &texture_sector[t]->texture_info;
        
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        if(texture_sector[t]) {
            al_build_transform(
                &tra,
                -texture_info_to_use->trans_x,
                -texture_info_to_use->trans_y,
                1.0 / texture_info_to_use->scale_x,
                1.0 / texture_info_to_use->scale_y,
                -texture_info_to_use->rot
            );
        }
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            unsigned char alpha = 255;
            
            if(t == 1) {
                if(!draw_sector_0) {
                    alpha = 0;
                    for(
                        size_t e = 0; e < texture_sector[1]->edges.size();
                        ++e
                    ) {
                        if(
                            texture_sector[1]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[1]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha = 255;
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
                            alpha = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - x;
            av[v].y = vy - y;
            if(texture_sector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba(
                    s_ptr->brightness, s_ptr->brightness,
                    s_ptr->brightness, alpha
                );
        }
        
        for(size_t v = 0; v < n_vertexes; ++v) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            texture_sector[t] ?
            texture_sector[t]->texture_info.bitmap :
            texture_sector[t == 0 ? 1 : 0]->texture_info.bitmap;
        if(texture) tex = texture->bitmap;
        
        al_draw_prim(
            av, NULL, tex,
            0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
}


/* ----------------------------------------------------------------------------
 * Draws a liquid sector.
 * x, y:    X and Y offset.
 * scale:   Scale the sector by this much.
 */
void draw_liquid(
    sector* s_ptr, const float x, const float y, const float scale
) {

    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        av[v].z = 0;
    }
    
    //Layer 1 - Transparent wobbling ground texture.
    if(s_ptr->texture_info.bitmap) {
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -s_ptr->texture_info.trans_x,
            -s_ptr->texture_info.trans_y,
            1.0 / s_ptr->texture_info.scale_x,
            1.0 / s_ptr->texture_info.scale_y,
            -s_ptr->texture_info.rot
        );
        
        float ground_wobble =
            -sin(area_time_passed * LIQUID_WOBBLE_TIME_SCALE) *
            LIQUID_WOBBLE_DELTA_X;
        float ground_texture_dy =
            al_get_bitmap_height(s_ptr->texture_info.bitmap) * 0.5;
            
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - x;
            av[v].y = vy - y;
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
            av, NULL, s_ptr->texture_info.bitmap,
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
        
        av[v].x = vx - x;
        av[v].y = vy - y;
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
    
        frame* anim_frame = NULL;
        float layer_2_dy = 0;
        float layer_speed[2];
        layer_speed[0] = s_ptr->associated_liquid->surface_speed[0];
        layer_speed[1] = s_ptr->associated_liquid->surface_speed[1];
        float alpha = s_ptr->associated_liquid->surface_alpha;
        
        if(s_ptr->associated_liquid->anim_instance.get_frame()) {
            anim_frame = s_ptr->associated_liquid->anim_instance.get_frame();
            if(anim_frame->bitmap) {
                layer_2_dy =
                    (anim_frame->file_h * 0.5) *
                    (anim_frame->game_w / anim_frame->file_w);
            }
        }
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - x;
            av[v].y = vy - y;
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
            av[v].u *= (anim_frame->file_w / anim_frame->game_w);
            av[v].v *= (anim_frame->file_w / anim_frame->game_w);
        }
        
        al_draw_prim(
            av, NULL, anim_frame->bitmap,
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
    
    ALLEGRO_BITMAP* text_bmp = NULL;
    ALLEGRO_BITMAP* subtext_bmp = NULL;
    
    unsigned char blackness_alpha = 255.0f * max(0.0f, opacity * 4 - 3);
    al_draw_filled_rectangle(
        0, 0, scr_w, scr_h, al_map_rgba(0, 0, 0, blackness_alpha)
    );
    
    //Set up the bitmap that will hold the text.
    int text_w = 0, text_h = 0;
    if(!text.empty()) {
    
        get_multiline_text_dimensions(font_area_name, text, &text_w, &text_h);
        text_bmp = al_create_bitmap(text_w, text_h);
        
        //Draw the main text on its bitmap.
        al_set_target_bitmap(text_bmp); {
            al_clear_to_color(al_map_rgba(0, 0, 0, 0));
            draw_text_lines(
                font_area_name, al_map_rgba(255, 215, 0, opacity * 255.0),
                0, 0, ALLEGRO_ALIGN_LEFT, 0,
                text
            );
        } al_set_target_backbuffer(display);
        
    }
    
    //Now, the bitmap for the subtext.
    int subtext_w = 0, subtext_h = 0;
    if(!subtext.empty()) {
    
        get_multiline_text_dimensions(
            font_area_name, subtext, &subtext_w, &subtext_h
        );
        subtext_bmp = al_create_bitmap(subtext_w, subtext_h);
        
        al_set_target_bitmap(subtext_bmp); {
            al_clear_to_color(al_map_rgba(0, 0, 0, 0));
            draw_text_lines(
                font_area_name, al_map_rgba(224, 224, 224, opacity * 255.0),
                0, 0,
                ALLEGRO_ALIGN_LEFT, 0,
                subtext
            );
            
        } al_set_target_backbuffer(display);
        
        //We'll be scaling this, so let's update the mipmap.
        subtext_bmp = recreate_bitmap(subtext_bmp);
        
    }
    
    //Draw the text bitmap in its place.
    float text_y = 0;
    if(!text.empty()) {
    
        text_y =
            subtext.empty() ?
            (scr_h * 0.5 - text_h * 0.5) :
            (scr_h * 0.5 - LOADING_SCREEN_PADDING * 0.5 - text_h);
        al_draw_bitmap(text_bmp, scr_w * 0.5 - text_w * 0.5, text_y, 0);
        
    }
    
    //Draw the subtext bitmap in its place.
    float subtext_y = scr_h * 0.5 + LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_scaled_bitmap(
            subtext_bmp,
            0, 0, subtext_w, subtext_h,
            scr_w * 0.5 - (subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5),
            subtext_y,
            subtext_w * LOADING_SCREEN_SUBTITLE_SCALE,
            subtext_h * LOADING_SCREEN_SUBTITLE_SCALE,
            0
        );
        
    }
    
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
        text_vertexes[0].color = al_map_rgba(255, 255, 255, 128);
        //Top-right vertex.
        text_vertexes[1].x = scr_w * 0.5 + text_w * 0.5;
        text_vertexes[1].y = text_y + text_h;
        text_vertexes[1].z = 0;
        text_vertexes[1].u = text_w;
        text_vertexes[1].v = text_h;
        text_vertexes[1].color = al_map_rgba(255, 255, 255, 128);
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
            text_vertexes, NULL, text_bmp,
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
        subtext_vertexes[0].color = al_map_rgba(255, 255, 255, 128);
        //Top-right vertex.
        subtext_vertexes[1].x =
            scr_w * 0.5 + subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[1].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[1].z = 0;
        subtext_vertexes[1].u = subtext_w;
        subtext_vertexes[1].v = subtext_h;
        subtext_vertexes[1].color = al_map_rgba(255, 255, 255, 128);
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
            subtext_vertexes, NULL, subtext_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //Draw the game's logo to the left of the "Loading..." text.
    if(opacity == 1.0f) {
        float icon_x =
            scr_w - 8 - al_get_text_width(font_main, "Loading...") -
            8 - font_main_h * 0.5;
        float icon_y = scr_h - 8 - font_main_h * 0.5;
        
        if(bmp_icon && bmp_icon != bmp_error) {
            draw_sprite(
                bmp_icon, icon_x, icon_y,
                -1, font_main_h, 0, al_map_rgba(255, 255, 255, opacity * 255.0)
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
    
    al_destroy_bitmap(text_bmp);
    al_destroy_bitmap(subtext_bmp);
    
}


/* ----------------------------------------------------------------------------
 * Draws a mob's shadow.
 * c*:             Center of the mob.
 * size:           Size of the mob.
 * delta_z:        The mob is these many units above the floor
   * directly below it.
 * shadow_stretch: How much to stretch the shadow by
   * (used to simulate sun shadow direction casting).
 */
void draw_mob_shadow(
    const float cx, const float cy, const float size,
    const float delta_z, const float shadow_stretch
) {

    if(shadow_stretch <= 0) return;
    
    float shadow_x = 0;
    float shadow_w = size + (size * shadow_stretch * MOB_SHADOW_STRETCH_MULT);
    
    if(day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + size * 0.5;
        shadow_x -= shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadow_x = -(size * 0.5);
        shadow_x += shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
    }
    
    
    draw_sprite(
        bmp_shadow,
        cx + shadow_x + shadow_w / 2, cy,
        shadow_w, size,
        0,
        map_alpha(255 * (1 - shadow_stretch))
    );
}


/* ----------------------------------------------------------------------------
 * Draws a sprite.
 * bmp:   The bitmap.
 * c*:    Center coordinates.
 * w/h:   Final width and height.
 ** Make this -1 on one of them to keep the aspect ratio from the other.
 * angle: Angle to rotate the sprite by.
 * tint:  Tint the sprite with this color.
 */
void draw_sprite(
    ALLEGRO_BITMAP* bmp, const float cx, const float cy,
    const float w, const float h, const float angle, const ALLEGRO_COLOR &tint
) {

    if(!bmp) {
        bmp = bmp_error;
    }
    
    float bmp_w = al_get_bitmap_width(bmp);
    float bmp_h = al_get_bitmap_height(bmp);
    float x_scale = (w / bmp_w);
    float y_scale = (h / bmp_h);
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        tint,
        bmp_w / 2, bmp_h / 2,
        cx, cy,
        (w == -1) ? y_scale : x_scale,
        (h == -1) ? x_scale : y_scale,
        angle,
        0);
}


/* ----------------------------------------------------------------------------
 * Draws the current area and mobs to a bitmap and returns it.
 */
ALLEGRO_BITMAP* draw_to_bitmap() {
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
    float final_bmp_w = dev_tool_area_image_size;
    float final_bmp_h = dev_tool_area_image_size;
    
    if(area_w > area_h) {
        scale = dev_tool_area_image_size / area_w;
        final_bmp_h *= area_h / area_w;
    } else {
        scale = dev_tool_area_image_size / area_h;
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
 * Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 * f:    Font to use.
 * c:    Color.
 * x/y:  Coordinates of the text.
 * fl:   Flags, just like the ones you'd pass to al_draw_text.
 * va:   Vertical align: 0 for top, 1 for center, 2 for bottom.
 * text: Text to write, line breaks included ('\n').
 */
void draw_text_lines(
    const ALLEGRO_FONT* const f, const ALLEGRO_COLOR &c,
    const float x, const float y, const int fl,
    const unsigned char va, const string &text
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(f);
    size_t n_lines = lines.size();
    float top;
    
    if(va == 0) {
        top = y;
    } else {
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = n_lines * fh + (n_lines - 1);
        if(va == 1) {
            top = y - total_height / 2;
        } else {
            top = y - total_height;
        }
    }
    
    for(size_t l = 0; l < n_lines; ++l) {
        float line_y = (fh + 1) * l + top;
        al_draw_text(f, c, x, line_y, fl, lines[l].c_str());
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
