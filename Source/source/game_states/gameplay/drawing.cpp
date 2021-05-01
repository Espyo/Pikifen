/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main gameplay drawing functions.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../mobs/group_task.h"
#include "../../mobs/pile.h"
#include "../../mobs/scale.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Does the drawing for the main game loop.
 * bmp_output:
 *   If not NULL, draw the area onto this.
 * bmp_transform:
 *   Transformation to use when drawing to a bitmap.
 */
void gameplay_state::do_game_drawing(
    ALLEGRO_BITMAP* bmp_output, ALLEGRO_TRANSFORM* bmp_transform
) {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    ALLEGRO_TRANSFORM old_world_to_screen_transform;
    int blend_old_op, blend_old_src, blend_old_dst,
        blend_old_aop, blend_old_asrc, blend_old_adst;
        
    if(bmp_output) {
        old_world_to_screen_transform = game.world_to_screen_transform;
        game.world_to_screen_transform = *bmp_transform;
        al_set_target_bitmap(bmp_output);
        al_get_separate_blender(
            &blend_old_op, &blend_old_src, &blend_old_dst,
            &blend_old_aop, &blend_old_asrc, &blend_old_adst
        );
        al_set_separate_blender(
            ALLEGRO_ADD, ALLEGRO_ALPHA,
            ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
            ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
        );
    }
    
    al_clear_to_color(game.cur_area_data.bg_color);
    
    //Layer 1 -- Background.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- Background");
    }
    draw_background(bmp_output);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 2 -- World components.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- World");
    }
    al_use_transform(&game.world_to_screen_transform);
    draw_world_components(bmp_output);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 3 -- In-game text.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- In-game text");
    }
    if(!bmp_output) {
        draw_ingame_text();
    }
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 4 -- Precipitation.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- precipitation");
    }
    if(!bmp_output) {
        draw_precipitation();
    }
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 5 -- Tree shadows.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- Tree shadows");
    }
    if(!(bmp_output && !game.maker_tools.area_image_shadows)) {
        draw_tree_shadows();
    }
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Finish dumping to a bitmap image here.
    if(bmp_output) {
        al_set_separate_blender(
            blend_old_op, blend_old_src, blend_old_dst,
            blend_old_aop, blend_old_asrc, blend_old_adst
        );
        game.world_to_screen_transform = old_world_to_screen_transform;
        al_set_target_backbuffer(game.display);
        return;
    }
    
    //Layer 6 -- Lighting filter.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- Lighting");
    }
    draw_lighting_filter();
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 7 -- Leader cursor.
    al_use_transform(&game.world_to_screen_transform);
    ALLEGRO_COLOR cursor_color;
    if(closest_group_member) {
        cursor_color = closest_group_member->type->main_color;
    } else {
        cursor_color = game.config.no_pikmin_color;
    }
    cursor_color =
        change_color_lighting(cursor_color, cursor_height_diff_light);
    draw_leader_cursor(cursor_color);
    
    //Layer 8 -- HUD.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Drawing -- HUD");
    }
    al_use_transform(&game.identity_transform);
    if(msg_box) {
        draw_message_box();
    } else if(onion_menu) {
        draw_onion_menu();
    } else if(pause_menu) {
        draw_pause_menu();
    } else {
        draw_mouse_cursor(cursor_color);
        hud.draw();
    }
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Layer 9 -- System stuff.
    draw_system_stuff();
    
    if(area_title_fade_timer.time_left > 0) {
        draw_loading_screen(
            game.cur_area_data.name,
            game.cur_area_data.subtitle,
            area_title_fade_timer.get_ratio_left()
        );
    }
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draws the area background.
 * bmp_output:
 *   If not NULL, draw the background onto this.
 */
void gameplay_state::draw_background(ALLEGRO_BITMAP* bmp_output) {
    if(!game.cur_area_data.bg_bmp) return;
    
    ALLEGRO_VERTEX bg_v[4];
    for(unsigned char v = 0; v < 4; ++v) {
        bg_v[v].color = map_gray(255);
        bg_v[v].z = 0;
    }
    
    //Not gonna lie, this uses some fancy-shmancy numbers.
    //I mostly got here via trial and error.
    //I apologize if you're trying to understand what it means.
    int bmp_w = bmp_output ? al_get_bitmap_width(bmp_output) : game.win_w;
    int bmp_h = bmp_output ? al_get_bitmap_height(bmp_output) : game.win_h;
    float zoom_to_use = bmp_output ? 0.5 : game.cam.zoom;
    point final_zoom(
        bmp_w * 0.5 * game.cur_area_data.bg_dist / zoom_to_use,
        bmp_h * 0.5 * game.cur_area_data.bg_dist / zoom_to_use
    );
    
    bg_v[0].x =
        0;
    bg_v[0].y =
        0;
    bg_v[0].u =
        (game.cam.pos.x - final_zoom.x) / game.cur_area_data.bg_bmp_zoom;
    bg_v[0].v =
        (game.cam.pos.y - final_zoom.y) / game.cur_area_data.bg_bmp_zoom;
    bg_v[1].x =
        bmp_w;
    bg_v[1].y =
        0;
    bg_v[1].u =
        (game.cam.pos.x + final_zoom.x) / game.cur_area_data.bg_bmp_zoom;
    bg_v[1].v =
        (game.cam.pos.y - final_zoom.y) / game.cur_area_data.bg_bmp_zoom;
    bg_v[2].x =
        bmp_w;
    bg_v[2].y =
        bmp_h;
    bg_v[2].u =
        (game.cam.pos.x + final_zoom.x) / game.cur_area_data.bg_bmp_zoom;
    bg_v[2].v =
        (game.cam.pos.y + final_zoom.y) / game.cur_area_data.bg_bmp_zoom;
    bg_v[3].x =
        0;
    bg_v[3].y =
        bmp_h;
    bg_v[3].u =
        (game.cam.pos.x - final_zoom.x) / game.cur_area_data.bg_bmp_zoom;
    bg_v[3].v =
        (game.cam.pos.y + final_zoom.y) / game.cur_area_data.bg_bmp_zoom;
        
    al_draw_prim(
        bg_v, NULL, game.cur_area_data.bg_bmp,
        0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
    );
}


/* ----------------------------------------------------------------------------
 * Draws the in-game text.
 */
void gameplay_state::draw_ingame_text() {
    //Fractions and health.
    size_t n_mobs = mobs.all.size();
    for(size_t m = 0; m < n_mobs; ++m) {
        mob* mob_ptr = mobs.all[m];
        
        if(
            mob_ptr->carry_info &&
            mob_ptr->carry_info->cur_carrying_strength > 0
        ) {
            bool destination_is_onion =
                mob_ptr->carry_info->intended_mob &&
                mob_ptr->carry_info->intended_mob->type->category->id ==
                MOB_CATEGORY_ONIONS;
                
            if(mob_ptr->type->weight > 1 || destination_is_onion) {
                ALLEGRO_COLOR color;
                if(mob_ptr->carry_info->is_moving) {
                    if(
                        mob_ptr->carry_info->destination ==
                        CARRY_DESTINATION_SHIP
                    ) {
                        color = game.config.carrying_color_move;
                        
                    } else if(destination_is_onion) {
                        color =
                            mob_ptr->carry_info->intended_pik_type->main_color;
                    } else {
                        color = game.config.carrying_color_move;
                    }
                    
                } else {
                    color = game.config.carrying_color_stop;
                }
                
                draw_fraction(
                    point(
                        mob_ptr->pos.x,
                        mob_ptr->pos.y - mob_ptr->type->radius -
                        al_get_font_line_height(game.fonts.standard) * 1.25
                    ),
                    mob_ptr->carry_info->cur_carrying_strength,
                    mob_ptr->type->weight,
                    color
                );
            }
        }
        
        for(size_t p = 0; p < mobs.piles.size(); ++p) {
            pile* p_ptr = mobs.piles[p];
            if(p_ptr->amount > 0 && p_ptr->pil_type->show_amount) {
                draw_text_lines(
                    game.fonts.standard,
                    game.config.carrying_color_stop,
                    point(
                        p_ptr->pos.x,
                        p_ptr->pos.y - p_ptr->type->radius -
                        al_get_font_line_height(game.fonts.standard) * 1.25
                    ),
                    ALLEGRO_ALIGN_CENTER,
                    1,
                    i2s(p_ptr->amount)
                );
            }
        }
        
        for(size_t t = 0; t < mobs.group_tasks.size(); ++t) {
            group_task* t_ptr = mobs.group_tasks[t];
            if(t_ptr->get_power() > 0) {
                draw_fraction(
                    point(
                        t_ptr->pos.x,
                        t_ptr->pos.y - t_ptr->type->radius -
                        al_get_font_line_height(game.fonts.standard) * 1.25
                    ),
                    t_ptr->get_power(),
                    t_ptr->tas_type->power_goal,
                    game.config.carrying_color_stop
                );
            }
        }
        
        for(size_t s = 0; s < mobs.scales.size(); ++s) {
            scale* s_ptr = mobs.scales[s];
            if(s_ptr->health <= 0) continue;
            float w = s_ptr->calculate_cur_weight();
            if(w > 0) {
                if(s_ptr->sca_type->goal_number > 0) {
                    draw_fraction(
                        point(
                            s_ptr->pos.x,
                            s_ptr->pos.y - s_ptr->type->radius -
                            al_get_font_line_height(game.fonts.standard) * 1.25
                        ),
                        w,
                        s_ptr->sca_type->goal_number,
                        game.config.carrying_color_stop
                    );
                } else {
                    draw_text_lines(
                        game.fonts.standard,
                        game.config.carrying_color_stop,
                        point(
                            s_ptr->pos.x,
                            s_ptr->pos.y - s_ptr->type->radius -
                            al_get_font_line_height(game.fonts.standard) * 1.25
                        ),
                        ALLEGRO_ALIGN_CENTER,
                        1,
                        i2s(w)
                    );
                }
            }
        }
        
        if(
            mob_ptr->type->show_health &&
            !mob_ptr->hide &&
            mob_ptr->health < mob_ptr->type->max_health &&
            mob_ptr->health_wheel_alpha > 0.0f
        ) {
            draw_health(
                point(
                    mob_ptr->pos.x,
                    mob_ptr->pos.y - mob_ptr->type->radius -
                    DEF_HEALTH_WHEEL_RADIUS - 4
                ),
                mob_ptr->health_wheel_smoothed_ratio,
                mob_ptr->health_wheel_alpha
            );
        }
        
        //Maker tool -- draw hitboxes.
        if(game.maker_tools.hitboxes) {
            sprite* s = mob_ptr->anim.get_cur_sprite();
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); ++h) {
                    hitbox* h_ptr = &s->hitboxes[h];
                    ALLEGRO_COLOR hc;
                    switch(h_ptr->type) {
                    case HITBOX_TYPE_NORMAL: {
                        hc = al_map_rgba(0, 128, 0, 192); //Green.
                        break;
                    } case HITBOX_TYPE_ATTACK: {
                        hc = al_map_rgba(128, 0, 0, 192); //Red.
                        break;
                    } case HITBOX_TYPE_DISABLED: {
                        hc = al_map_rgba(128, 128, 0, 192); //Yellow.
                        break;
                    }
                    }
                    point p =
                        mob_ptr->pos + rotate_point(h_ptr->pos, mob_ptr->angle);
                    al_draw_filled_circle(p.x, p.y, h_ptr->radius, hc);
                }
            }
        }
    }
    
    bool done = false;
    
    //Lying down stop notification.
    if(
        cur_leader_ptr->carry_info &&
        cancel_control_id != INVALID
    ) {
        draw_notification(
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y -
                cur_leader_ptr->type->radius
            ),
            "Get up", &game.options.controls[0][cancel_control_id]
        );
        done = true;
    }
    
    //Pluck stop notification.
    if(
        !done &&
        cur_leader_ptr->auto_plucking &&
        cancel_control_id != INVALID
    ) {
        draw_notification(
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y -
                cur_leader_ptr->type->radius
            ),
            "Stop", &game.options.controls[0][cancel_control_id]
        );
        done = true;
    }
    
    //Ship healing notification.
    if(
        !done &&
        close_to_ship_to_heal &&
        main_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_ship_to_heal->beam_final_pos.x,
                close_to_ship_to_heal->beam_final_pos.y -
                close_to_ship_to_heal->shi_type->beam_radius
            ),
            "Repair suit", &game.options.controls[0][main_control_id]
        );
        done = true;
    }
    
    //Interactable mob notification.
    if(
        !done &&
        close_to_interactable_to_use &&
        main_control_id != INVALID
    ) {
        float pivot_y =
            close_to_interactable_to_use->pos.y -
            close_to_interactable_to_use->type->radius;
        draw_notification(
            point(close_to_interactable_to_use->pos.x, pivot_y),
            close_to_interactable_to_use->int_type->prompt_text,
            &game.options.controls[0][main_control_id]
        );
        done = true;
    }
    
    //Pikmin pluck notification.
    if(
        !done &&
        close_to_pikmin_to_pluck &&
        main_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_pikmin_to_pluck->pos.x,
                close_to_pikmin_to_pluck->pos.y -
                close_to_pikmin_to_pluck->type->radius
            ),
            "Pluck", &game.options.controls[0][main_control_id]
        );
        done = true;
    }
    
    //Nest open notification.
    if(
        !done &&
        close_to_nest_to_open &&
        main_control_id != INVALID
    ) {
        draw_notification(
            point(
                close_to_nest_to_open->m_ptr->pos.x,
                close_to_nest_to_open->m_ptr->pos.y -
                close_to_nest_to_open->m_ptr->type->radius
            ),
            "Check", &game.options.controls[0][main_control_id]
        );
        done = true;
    }
}


/* ----------------------------------------------------------------------------
 * Draws the leader's cursor and associated effects.
 * color:
 *   Color to tint it by.
 */
void gameplay_state::draw_leader_cursor(const ALLEGRO_COLOR &color) {

    size_t n_arrows = swarm_arrows.size();
    for(size_t a = 0; a < n_arrows; ++a) {
        point pos(
            cos(swarm_angle) * swarm_arrows[a],
            sin(swarm_angle) * swarm_arrows[a]
        );
        float alpha =
            64 + std::min(
                191,
                (int) (
                    191 *
                    (swarm_arrows[a] / (game.config.cursor_max_dist * 0.4))
                )
            );
        draw_bitmap(
            game.sys_assets.bmp_swarm_arrow,
            cur_leader_ptr->pos + pos,
            point(16 * (1 + swarm_arrows[a] / game.config.cursor_max_dist), -1),
            swarm_angle,
            map_alpha(alpha)
        );
    }
    
    size_t n_rings = whistle.rings.size();
    float cursor_angle = get_angle(cur_leader_ptr->pos, leader_cursor_w);
    for(size_t r = 0; r < n_rings; ++r) {
        point pos(
            cur_leader_ptr->pos.x + cos(cursor_angle) * whistle.rings[r],
            cur_leader_ptr->pos.y + sin(cursor_angle) * whistle.rings[r]
        );
        unsigned char n = whistle.ring_colors[r];
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
    
    if(whistle.radius > 0 || whistle.fade_timer.time_left > 0.0f) {
        if(game.options.pretty_whistle) {
            unsigned char n_dots = 16 * 6;
            for(unsigned char d = 0; d < 6; ++d) {
                for(unsigned char d2 = 0; d2 < 16; ++d2) {
                    unsigned char current_dot = d2 * 6 + d;
                    float angle =
                        TAU / n_dots *
                        current_dot -
                        WHISTLE_DOT_SPIN_SPEED * area_time_passed;
                        
                    point dot_pos(
                        leader_cursor_w.x +
                        cos(angle) * whistle.dot_radius[d],
                        leader_cursor_w.y +
                        sin(angle) * whistle.dot_radius[d]
                    );
                    
                    ALLEGRO_COLOR c;
                    float alpha_mult;
                    if(whistle.fade_timer.time_left > 0.0f)
                        alpha_mult = whistle.fade_timer.get_ratio_left();
                    else
                        alpha_mult = 1;
                        
                    switch(d) {
                    case 0: {
                        //Red.
                        c = al_map_rgba(255, 0,   0,   255 * alpha_mult);
                        break;
                    } case 1: {
                        //Orange.
                        c = al_map_rgba(255, 128, 0,   210 * alpha_mult);
                        break;
                    } case 2: {
                        //Lime.
                        c = al_map_rgba(128, 255, 0,   165 * alpha_mult);
                        break;
                    } case 3: {
                        //Cyan.
                        c = al_map_rgba(0,   255, 255, 120 * alpha_mult);
                        break;
                    } case 4: {
                        //Blue.
                        c = al_map_rgba(0,   0,   255, 75  * alpha_mult);
                        break;
                    } default: {
                        //Purple.
                        c = al_map_rgba(128, 0,   255, 30  * alpha_mult);
                        break;
                    }
                    }
                    
                    al_draw_filled_circle(dot_pos.x, dot_pos.y, 2, c);
                }
            }
        } else {
            unsigned char alpha = whistle.fade_timer.get_ratio_left() * 255;
            float radius = whistle.fade_radius;
            if(whistle.radius > 0) {
                alpha = 255;
                radius = whistle.radius;
            }
            al_draw_circle(
                leader_cursor_w.x, leader_cursor_w.y, radius,
                al_map_rgba(192, 192, 0, alpha), 2
            );
        }
    }
    
    //Leader cursor.
    int bmp_cursor_w = al_get_bitmap_width(game.sys_assets.bmp_cursor);
    int bmp_cursor_h = al_get_bitmap_height(game.sys_assets.bmp_cursor);
    
    draw_bitmap(
        game.sys_assets.bmp_cursor,
        leader_cursor_w,
        point(bmp_cursor_w * 0.5, bmp_cursor_h * 0.5),
        cursor_angle,
        change_color_lighting(
            color,
            cursor_height_diff_light
        )
    );
    
    if(!throw_can_reach_cursor) {
        unsigned char alpha =
            0 +
            (sin(area_time_passed * CURSOR_INVALID_EFFECT_SPEED) + 1) * 127.0;
            
        draw_bitmap(
            game.sys_assets.bmp_cursor_invalid,
            leader_cursor_w,
            point(bmp_cursor_w * 0.5, bmp_cursor_h * 0.5),
            0,
            change_alpha(color, alpha)
        );
    }
    
    //Standby type count.
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
    
    al_use_transform(&game.identity_transform);
    
    float count_offset =
        std::max(bmp_cursor_w, bmp_cursor_h) * 0.18f * game.cam.zoom;
        
    if(n_standby_pikmin > 0) {
        draw_scaled_text(
            game.fonts.cursor_counter,
            color,
            leader_cursor_s +
            point(count_offset, count_offset),
            point(1.0f, 1.0f),
            ALLEGRO_ALIGN_LEFT,
            0,
            i2s(n_standby_pikmin)
        );
    }
    
    al_use_transform(&game.world_to_screen_transform);
}


/* ----------------------------------------------------------------------------
 * Draws the full-screen effects that will represent lighting.
 */
void gameplay_state::draw_lighting_filter() {
    al_use_transform(&game.identity_transform);
    
    //Draw the fog effect.
    ALLEGRO_COLOR fog_c = get_fog_color();
    if(fog_c.a > 0) {
        //Start by drawing the central fog fade out effect.
        
        point fog_top_left =
            game.cam.pos -
            point(
                game.cur_area_data.weather_condition.fog_far,
                game.cur_area_data.weather_condition.fog_far
            );
        point fog_bottom_right =
            game.cam.pos +
            point(
                game.cur_area_data.weather_condition.fog_far,
                game.cur_area_data.weather_condition.fog_far
            );
        al_transform_coordinates(
            &game.world_to_screen_transform,
            &fog_top_left.x, &fog_top_left.y
        );
        al_transform_coordinates(
            &game.world_to_screen_transform,
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
            game.win_w, fog_bottom_right.y,
            fog_c
        );
        //Bottom-right and bottom-center.
        al_draw_filled_rectangle(
            fog_top_left.x, fog_bottom_right.y,
            game.win_w, game.win_h,
            fog_c
        );
        //Bottom-left and center-left.
        al_draw_filled_rectangle(
            0, fog_top_left.y,
            fog_top_left.x, game.win_h,
            fog_c
        );
        
    }
    
    //Draw the daylight.
    ALLEGRO_COLOR daylight_c = get_daylight_color();
    if(daylight_c.a > 0) {
        al_draw_filled_rectangle(0, 0, game.win_w, game.win_h, daylight_c);
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
        for(size_t m = 0; m < mobs.all.size(); ++m) {
            if(mobs.all[m]->hide) continue;
            
            point pos = mobs.all[m]->pos;
            al_transform_coordinates(
                &game.world_to_screen_transform,
                &pos.x, &pos.y
            );
            float radius = mobs.all[m]->type->radius * 4.0 * game.cam.zoom;
            al_draw_scaled_bitmap(
                game.sys_assets.bmp_spotlight,
                0, 0, 64, 64,
                pos.x - radius, pos.y - radius,
                radius * 2.0, radius * 2.0,
                0
            );
        }
        al_hold_bitmap_drawing(false);
        
        //Now, simply darken the screen using the map.
        al_set_target_backbuffer(game.display);
        
        al_draw_bitmap(lightmap_bmp, 0, 0, 0);
        
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Draws a message box.
 */
void gameplay_state::draw_message_box() {
    draw_mouse_cursor(
        change_color_lighting(
            cur_leader_ptr->lea_type->main_color,
            cursor_height_diff_light
        )
    );
    
    al_use_transform(&game.identity_transform);
    
    draw_bitmap(
        bmp_message_box,
        point(
            game.win_w / 2,
            game.win_h - al_get_font_line_height(game.fonts.standard) * 2 - 4
        ),
        point(
            game.win_w - 16,
            al_get_font_line_height(game.fonts.standard) * 4
        )
    );
    
    if(msg_box->speaker_icon) {
        draw_bitmap(
            msg_box->speaker_icon,
            point(
                40,
                game.win_h -
                al_get_font_line_height(game.fonts.standard) * 4 - 16
            ),
            point(48, 48)
        );
        draw_bitmap(
            bmp_bubble,
            point(
                40,
                game.win_h -
                al_get_font_line_height(game.fonts.standard) * 4 - 16
            ),
            point(64, 64)
        );
    }
    
    vector<string> lines = msg_box->get_current_lines();
    
    for(size_t l = 0; l < lines.size(); ++l) {
    
        draw_compressed_text(
            game.fonts.standard, al_map_rgb(255, 255, 255),
            point(
                24,
                game.win_h -
                al_get_font_line_height(game.fonts.standard) * (4 - l) + 8
            ),
            ALLEGRO_ALIGN_LEFT, 0, point(game.win_w - 64, 0),
            lines[l]
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Draws the mouse cursor.
 * color:
 *   Color to tint it with.
 */
void gameplay_state::draw_mouse_cursor(const ALLEGRO_COLOR &color) {
    //Cursor trail.
    al_use_transform(&game.identity_transform);
    if(game.options.draw_cursor_trail) {
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
                        color,
                        (p / (float) cursor_spots.size()) * 64
                    ),
                    p * 3
                );
            }
        }
    }
    
    //Mouse cursor.
    draw_bitmap(
        game.sys_assets.bmp_mouse_cursor,
        game.mouse_cursor_s,
        point(
            al_get_bitmap_width(game.sys_assets.bmp_mouse_cursor),
            al_get_bitmap_height(game.sys_assets.bmp_mouse_cursor)
        ),
        -(game.time_passed * game.config.cursor_spin_speed),
        color
    );
}


/* ----------------------------------------------------------------------------
 * Draws the current Onion menu.
 */
void gameplay_state::draw_onion_menu() {
    al_draw_filled_rectangle(
        0, 0, game.win_w, game.win_h, al_map_rgba(24, 64, 60, 200)
    );
    
    onion_menu->gui.draw();
    
    draw_mouse_cursor(al_map_rgb(188, 230, 230));
}


/* ----------------------------------------------------------------------------
 * Draws the current pause menu.
 */
void gameplay_state::draw_pause_menu() {
    al_draw_filled_rectangle(
        0, 0, game.win_w, game.win_h, al_map_rgba(24, 64, 60, 200)
    );
    
    pause_menu->gui.draw();
    
    draw_mouse_cursor(al_map_rgb(188, 230, 230));
}


/* ----------------------------------------------------------------------------
 * Draws the precipitation.
 */
void gameplay_state::draw_precipitation() {
    if(
        game.cur_area_data.weather_condition.precipitation_type !=
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
void gameplay_state::draw_system_stuff() {
    if(!game.maker_tools.info_print_text.empty()) {
        float alpha_mult = 1;
        if(
            game.maker_tools.info_print_timer.time_left <
            game.maker_tools.info_print_fade_duration
        ) {
            alpha_mult =
                game.maker_tools.info_print_timer.time_left /
                game.maker_tools.info_print_fade_duration;
        }
        
        size_t n_lines =
            split(game.maker_tools.info_print_text, "\n", true).size();
        int fh = al_get_font_line_height(game.fonts.builtin);
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = n_lines * fh + (n_lines - 1);
        
        al_draw_filled_rectangle(
            0, 0, game.win_w, total_height + 16,
            al_map_rgba(0, 0, 0, 96 * alpha_mult)
        );
        draw_text_lines(
            game.fonts.builtin, al_map_rgba(255, 255, 255, 128 * alpha_mult),
            point(8, 8), 0, 0, game.maker_tools.info_print_text
        );
    }
    
    if(game.show_system_info) {
        //Draw the framerate chart.
        al_draw_filled_rectangle(
            game.win_w - FRAMERATE_HISTORY_SIZE, 0,
            game.win_w, 100,
            al_map_rgba(0, 0, 0, 192)
        );
        for(size_t f = 0; f < game.framerate_history.size(); ++f) {
            al_draw_line(
                game.win_w - FRAMERATE_HISTORY_SIZE + f + 0.5, 0,
                game.win_w - FRAMERATE_HISTORY_SIZE + f + 0.5,
                round(game.framerate_history[f]),
                al_map_rgba(24, 96, 192, 192), 1
            );
        }
        al_draw_line(
            game.win_w - FRAMERATE_HISTORY_SIZE, game.options.target_fps,
            game.win_w, game.options.target_fps,
            al_map_rgba(128, 224, 128, 48), 1
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws the current area and mobs to a bitmap and returns it.
 */
ALLEGRO_BITMAP* gameplay_state::draw_to_bitmap() {
    //First, get the full dimensions of the map.
    float min_x = FLT_MAX, min_y = FLT_MAX, max_x = -FLT_MAX, max_y = -FLT_MAX;
    
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); v++) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        min_x = std::min(v_ptr->x, min_x);
        min_y = std::min(v_ptr->y, min_y);
        max_x = std::max(v_ptr->x, max_x);
        max_y = std::max(v_ptr->y, max_y);
    }
    
    //Figure out the scale that will fit on the image.
    float area_w = max_x - min_x;
    float area_h = max_y - min_y;
    float scale = 1.0f;
    float final_bmp_w = game.maker_tools.area_image_size;
    float final_bmp_h = game.maker_tools.area_image_size;
    
    if(area_w > area_h) {
        scale = game.maker_tools.area_image_size / area_w;
        final_bmp_h *= area_h / area_w;
    } else {
        scale = game.maker_tools.area_image_size / area_h;
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
 * Draws tree shadows.
 */
void gameplay_state::draw_tree_shadows() {
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
        
        unsigned char alpha =
            ((s_ptr->alpha / 255.0) * get_sun_strength()) * 255;
            
        draw_bitmap(
            s_ptr->bitmap,
            point(
                s_ptr->center.x + TREE_SHADOW_SWAY_AMOUNT*
                sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                s_ptr->sway.x,
                s_ptr->center.y + TREE_SHADOW_SWAY_AMOUNT*
                sin(TREE_SHADOW_SWAY_SPEED * area_time_passed) *
                s_ptr->sway.y
            ),
            s_ptr->size,
            s_ptr->angle, map_alpha(alpha)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws the components that make up the game world: layout, objects, etc.
 * bmp_output:
 *   If not NULL, draw the area onto this.
 */
void gameplay_state::draw_world_components(ALLEGRO_BITMAP* bmp_output) {
    ALLEGRO_BITMAP* custom_liquid_limit_effect_buffer = NULL;
    ALLEGRO_BITMAP* custom_wall_offset_effect_buffer = NULL;
    if(!bmp_output) {
        update_offset_effect_buffer(
            game.cam.box[0], game.cam.box[1],
            game.liquid_limit_effect_buffer,
            true,
            does_edge_have_liquid_limit,
            get_liquid_limit_length,
            get_liquid_limit_color
        );
        update_offset_effect_buffer(
            game.cam.box[0], game.cam.box[1],
            game.wall_offset_effect_buffer,
            true,
            does_edge_have_ledge_smoothing,
            get_ledge_smoothing_length,
            get_ledge_smoothing_color
        );
        update_offset_effect_buffer(
            game.cam.box[0], game.cam.box[1],
            game.wall_offset_effect_buffer,
            false,
            does_edge_have_wall_shadow,
            get_wall_shadow_length,
            get_wall_shadow_color
        );
        
    } else {
        custom_liquid_limit_effect_buffer =
            al_create_bitmap(
                al_get_bitmap_width(bmp_output),
                al_get_bitmap_height(bmp_output)
            );
        custom_wall_offset_effect_buffer =
            al_create_bitmap(
                al_get_bitmap_width(bmp_output),
                al_get_bitmap_height(bmp_output)
            );
        update_offset_effect_buffer(
            point(-FLT_MAX, -FLT_MAX), point(FLT_MAX, FLT_MAX),
            custom_liquid_limit_effect_buffer,
            true,
            does_edge_have_liquid_limit,
            get_liquid_limit_length,
            get_liquid_limit_color
        );
        update_offset_effect_buffer(
            point(-FLT_MAX, -FLT_MAX), point(FLT_MAX, FLT_MAX),
            custom_wall_offset_effect_buffer,
            true,
            does_edge_have_ledge_smoothing,
            get_ledge_smoothing_length,
            get_ledge_smoothing_color
        );
        update_offset_effect_buffer(
            point(-FLT_MAX, -FLT_MAX), point(FLT_MAX, FLT_MAX),
            custom_wall_offset_effect_buffer,
            false,
            does_edge_have_wall_shadow,
            get_wall_shadow_length,
            get_wall_shadow_color
        );
    }
    
    vector<world_component> components;
    //Let's reserve some space. We might need more or less,
    //but this is a nice estimate.
    components.reserve(
        game.cur_area_data.sectors.size() + //Sectors
        mobs.all.size() + //Mob shadows
        mobs.all.size() + //Mobs
        particles.get_count() //Particles
    );
    
    //Sectors.
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        
        if(
            !bmp_output &&
            !rectangles_intersect(
                s_ptr->bbox[0], s_ptr->bbox[1],
                game.cam.box[0], game.cam.box[1]
            )
        ) {
            //Off-camera.
            continue;
        }
        
        world_component c;
        c.sector_ptr = s_ptr;
        c.z = s_ptr->z;
        components.push_back(c);
    }
    
    //Particles.
    particles.fill_component_list(components, game.cam.box[0], game.cam.box[1]);
    
    //Mobs.
    for(size_t m = 0; m < mobs.all.size(); ++m) {
        mob* mob_ptr = mobs.all[m];
        
        if(!bmp_output && mob_ptr->is_off_camera()) {
            //Off-camera.
            continue;
        }
        
        if(mob_ptr->hide) continue;
        
        //Shadows.
        if(mob_ptr->type->casts_shadow) {
            world_component c;
            c.mob_shadow_ptr = mob_ptr;
            if(mob_ptr->standing_on_mob) {
                c.z =
                    mob_ptr->standing_on_mob->z +
                    mob_ptr->standing_on_mob->height;
            } else {
                c.z = mob_ptr->ground_sector->z;
            }
            components.push_back(c);
        }
        
        //Limbs.
        if(mob_ptr->parent && mob_ptr->parent->limb_anim.anim_db) {
            unsigned char method = mob_ptr->parent->limb_draw_method;
            world_component c;
            c.mob_limb_ptr = mob_ptr;
            
            switch(method) {
            case LIMB_DRAW_BELOW_BOTH: {
                c.z = std::min(mob_ptr->z, mob_ptr->parent->m->z);
                break;
            } case LIMB_DRAW_BELOW_CHILD: {
                c.z = mob_ptr->z;
                break;
            } case LIMB_DRAW_BELOW_PARENT: {
                c.z = mob_ptr->parent->m->z;
                break;
            } case LIMB_DRAW_ABOVE_PARENT: {
                c.z =
                    mob_ptr->parent->m->z +
                    mob_ptr->parent->m->height +
                    0.001;
                break;
            } case LIMB_DRAW_ABOVE_CHILD: {
                c.z = mob_ptr->z + mob_ptr->height + 0.001;
                break;
            } case LIMB_DRAW_ABOVE_BOTH: {
                c.z =
                    std::max(
                        mob_ptr->parent->m->z +
                        mob_ptr->parent->m->height +
                        0.001,
                        mob_ptr->z + mob_ptr->height + 0.001
                    );
                break;
            }
            }
            
            components.push_back(c);
        }
        
        //The mob proper.
        world_component c;
        c.mob_ptr = mob_ptr;
        if(mob_ptr->holder.m && mob_ptr->holder.above_holder) {
            c.z = mob_ptr->holder.m->z + mob_ptr->holder.m->height + 0.01;
        } else {
            c.z = mob_ptr->z + mob_ptr->height;
        }
        components.push_back(c);
        
    }
    
    //Time to draw!
    for(size_t c = 0; c < components.size(); ++c) {
        components[c].nr = c;
    }
    
    sort(
        components.begin(), components.end(),
    [] (world_component c1, world_component c2) -> bool {
        if(c1.z == c2.z) {
            return c1.nr < c2.nr;
        }
        return c1.z < c2.z;
    }
    );
    
    float mob_shadow_stretch = 0;
    
    if(day_minutes < 60 * 5 || day_minutes > 60 * 20) {
        mob_shadow_stretch = 1;
    } else if(day_minutes < 60 * 12) {
        mob_shadow_stretch = 1 - ((day_minutes - 60 * 5) / (60 * 12 - 60 * 5));
    } else {
        mob_shadow_stretch = (day_minutes - 60 * 12) / (60 * 20 - 60 * 12);
    }
    
    for(size_t c = 0; c < components.size(); ++c) {
        world_component* c_ptr = &components[c];
        
        if(c_ptr->sector_ptr) {
        
            draw_sector_texture(c_ptr->sector_ptr, point(), 1.0f, 1.0f);
            
            for(size_t h = 0; h < c_ptr->sector_ptr->hazards.size(); ++h) {
                if(c_ptr->sector_ptr->hazards[h]->associated_liquid) {
                    draw_liquid(
                        c_ptr->sector_ptr,
                        c_ptr->sector_ptr->hazards[h]->associated_liquid,
                        point(),
                        1.0f
                    );
                    break;
                }
            }
            
            draw_sector_edge_offsets(
                c_ptr->sector_ptr,
                bmp_output ?
                custom_liquid_limit_effect_buffer :
                game.liquid_limit_effect_buffer,
                1.0f
            );
            draw_sector_edge_offsets(
                c_ptr->sector_ptr,
                bmp_output ?
                custom_wall_offset_effect_buffer :
                game.wall_offset_effect_buffer,
                1.0f
            );
            
        } else if(c_ptr->mob_shadow_ptr) {
        
            float delta_z = 0;
            if(!c_ptr->mob_shadow_ptr->standing_on_mob) {
                delta_z =
                    c_ptr->mob_shadow_ptr->z -
                    c_ptr->mob_shadow_ptr->ground_sector->z;
            }
            draw_mob_shadow(
                c_ptr->mob_shadow_ptr->pos,
                c_ptr->mob_shadow_ptr->type->radius * 2,
                delta_z,
                mob_shadow_stretch
            );
            
        } else if(c_ptr->mob_limb_ptr) {
        
            if(!c_ptr->mob_limb_ptr->hide) {
                c_ptr->mob_limb_ptr->draw_limb();
            }
            
        } else if(c_ptr->mob_ptr) {
        
            if(!c_ptr->mob_ptr->hide) {
                c_ptr->mob_ptr->draw_mob();
            }
            
        } else if(c_ptr->particle_ptr) {
        
            c_ptr->particle_ptr->draw();
            
        }
    }
    
    if(bmp_output) {
        al_destroy_bitmap(custom_liquid_limit_effect_buffer);
        al_destroy_bitmap(custom_wall_offset_effect_buffer);
    }
}
