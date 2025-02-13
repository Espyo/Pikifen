/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor drawing logic.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"



/**
 * @brief Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    draw_op_error_cursor();
}


/**
 * @brief Draw the canvas.
 *
 * This is called as a callback inside the Dear ImGui rendering process.
 */
void animation_editor::draw_canvas() {
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    if(use_bg && bg) {
        point texture_tl = canvas_tl;
        point texture_br = canvas_br;
        al_transform_coordinates(
            &game.screen_to_world_transform, &texture_tl.x, &texture_tl.y
        );
        al_transform_coordinates(
            &game.screen_to_world_transform, &texture_br.x, &texture_br.y
        );
        ALLEGRO_VERTEX bg_vertexes[4];
        for(size_t v = 0; v < 4; v++) {
            bg_vertexes[v].z = 0;
            bg_vertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bg_vertexes[0].x = canvas_tl.x;
        bg_vertexes[0].y = canvas_tl.y;
        bg_vertexes[0].u = texture_tl.x;
        bg_vertexes[0].v = texture_tl.y;
        //Top-right vertex.
        bg_vertexes[1].x = canvas_br.x;
        bg_vertexes[1].y = canvas_tl.y;
        bg_vertexes[1].u = texture_br.x;
        bg_vertexes[1].v = texture_tl.y;
        //Bottom-right vertex.
        bg_vertexes[2].x = canvas_br.x;
        bg_vertexes[2].y = canvas_br.y;
        bg_vertexes[2].u = texture_br.x;
        bg_vertexes[2].v = texture_br.y;
        //Bottom-left vertex.
        bg_vertexes[3].x = canvas_tl.x;
        bg_vertexes[3].y = canvas_br.y;
        bg_vertexes[3].u = texture_tl.x;
        bg_vertexes[3].v = texture_br.y;
        
        al_draw_prim(
            bg_vertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.world_to_screen_transform);
    
    sprite* s = nullptr;
    
    if(state == EDITOR_STATE_ANIMATION && cur_anim_i.valid_frame()) {
        s = cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx].sprite_ptr;
        
    } else if(
        state == EDITOR_STATE_SPRITE || state == EDITOR_STATE_TOP ||
        state == EDITOR_STATE_HITBOXES ||
        state == EDITOR_STATE_SPRITE_BITMAP ||
        state == EDITOR_STATE_SPRITE_TRANSFORM
    ) {
        s = cur_sprite;
        
    }
    
    bool draw_hitboxes = hitboxes_visible;
    bool draw_mob_radius = mob_radius_visible;
    bool draw_leader_silhouette = leader_silhouette_visible;
    float grid_opacity = grid_visible ? 0.33f : 0.0f;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_TOP) {
        draw_hitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        grid_opacity = 0.0f;
        draw_mob_radius = false;
        draw_leader_silhouette = false;
        
        if(s && s->parent_bmp) {
            int bmp_w = al_get_bitmap_width(s->parent_bmp);
            int bmp_h = al_get_bitmap_height(s->parent_bmp);
            int bmp_x = -bmp_w / 2.0;
            int bmp_y = -bmp_h / 2.0;
            al_draw_bitmap(s->parent_bmp, bmp_x, bmp_y, 0);
            
            point scene_tl = point(-1.0f);
            point scene_br = point(canvas_br.x + 1, canvas_br.y + 1);
            al_transform_coordinates(
                &game.screen_to_world_transform, &scene_tl.x, &scene_tl.y
            );
            al_transform_coordinates(
                &game.screen_to_world_transform, &scene_br.x, &scene_br.y
            );
            
            for(unsigned char x = 0; x < 3; x++) {
                point rec_tl, rec_br;
                switch(x) {
                case 0: {
                    rec_tl.x = scene_tl.x;
                    rec_br.x = bmp_x + s->bmp_pos.x;
                    break;
                } case 1: {
                    rec_tl.x = bmp_x + s->bmp_pos.x;
                    rec_br.x = bmp_x + s->bmp_pos.x + s->bmp_size.x;
                    break;
                } default: {
                    rec_tl.x = bmp_x + s->bmp_pos.x + s->bmp_size.x;
                    rec_br.x = scene_br.x;
                    break;
                }
                }
                
                for(unsigned char y = 0; y < 3; y++) {
                    if(x == 1 && y == 1) continue;
                    
                    switch(y) {
                    case 0: {
                        rec_tl.y = scene_tl.y;
                        rec_br.y = bmp_y + s->bmp_pos.y;
                        break;
                    } case 1: {
                        rec_tl.y = bmp_y + s->bmp_pos.y;
                        rec_br.y = bmp_y + s->bmp_pos.y + s->bmp_size.y;
                        break;
                    } default: {
                        rec_tl.y = bmp_y + s->bmp_pos.y + s->bmp_size.y;
                        rec_br.y = scene_br.y;
                        break;
                    }
                    }
                    
                    al_draw_filled_rectangle(
                        rec_tl.x, rec_tl.y,
                        rec_br.x, rec_br.y,
                        al_map_rgba(0, 0, 0, 128)
                    );
                }
            }
            
            if(s->bmp_size.x > 0 && s->bmp_size.y > 0) {
            
                unsigned char outline_alpha =
                    255 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
                al_draw_rectangle(
                    bmp_x + s->bmp_pos.x + 0.5,
                    bmp_y + s->bmp_pos.y + 0.5,
                    bmp_x + s->bmp_pos.x + s->bmp_size.x - 0.5,
                    bmp_y + s->bmp_pos.y + s->bmp_size.y - 0.5,
                    al_map_rgba(224, 192, 0, outline_alpha), 1.0
                );
            }
        }
        
    } else if(s) {
    
        if(side_view && state == EDITOR_STATE_HITBOXES) {
            draw_side_view_sprite(s);
        } else {
            draw_top_down_view_sprite(s);
        }
        
        if(draw_hitboxes) {
            unsigned char hitbox_outline_alpha =
                63 + 192 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
            size_t n_hitboxes = s->hitboxes.size();
            
            for(int h = (int) n_hitboxes - 1; h >= 0; --h) {
                //Iterate the hitboxes in reverse order, since this is
                //the order of priority the engine has when checking for
                //collisions. Making higher priority hitboxes appear above
                //lower ones makes it all more intuitive and cohesive.
                hitbox* h_ptr = &s->hitboxes[h];
                ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                float hitbox_outline_thickness = 2.0f / game.cam.zoom;
                
                switch(h_ptr->type) {
                case HITBOX_TYPE_NORMAL: {
                    hitbox_color = al_map_rgba(0, 128, 0, 128);
                    hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                    break;
                } case HITBOX_TYPE_ATTACK: {
                    hitbox_color = al_map_rgba(128, 0, 0, 128);
                    hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                    break;
                } case HITBOX_TYPE_DISABLED: {
                    hitbox_color = al_map_rgba(128, 128, 0, 128);
                    hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
                    break;
                }
                }
                
                if(
                    cur_hitbox_idx == (size_t) h &&
                    state == EDITOR_STATE_HITBOXES
                ) {
                    hitbox_outline_thickness =
                        3.0f / game.cam.zoom;
                    hitbox_outline_color =
                        change_alpha(hitbox_color, hitbox_outline_alpha);
                }
                
                if(side_view && state == EDITOR_STATE_HITBOXES) {
                    draw_side_view_hitbox(
                        h_ptr, hitbox_color,
                        hitbox_outline_color, hitbox_outline_thickness
                    );
                } else {
                    draw_top_down_view_hitbox(
                        h_ptr, hitbox_color,
                        hitbox_outline_color, hitbox_outline_thickness
                    );
                }
            }
        }
        
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            point cur_sprite_size = cur_sprite->scale * cur_sprite->bmp_size;
            cur_transformation_widget.draw(
                &cur_sprite->offset,
                &cur_sprite_size,
                &cur_sprite->angle,
                1.0f / game.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_TOP && s->top_visible) {
            cur_transformation_widget.draw(
                &s->top_pos,
                &s->top_size,
                &s->top_angle,
                1.0f / game.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_HITBOXES && cur_hitbox) {
            if(!side_view) {
                point hitbox_size(
                    cur_hitbox->radius * 2.0f, cur_hitbox->radius * 2.0f
                );
                cur_transformation_widget.draw(
                    &cur_hitbox->pos,
                    &hitbox_size,
                    nullptr,
                    1.0f / game.cam.zoom
                );
            } else if(cur_hitbox->height != 0.0f) {
                point hitbox_center(
                    cur_hitbox->pos.x,
                    (-(cur_hitbox->height / 2.0f)) - cur_hitbox->z
                );
                point hitbox_size(
                    cur_hitbox->radius * 2.0f, cur_hitbox->height
                );
                cur_transformation_widget.draw(
                    &hitbox_center,
                    &hitbox_size,
                    nullptr,
                    1.0f / game.cam.zoom
                );
            }
            
        }
    }
    
    //Grid.
    if(grid_opacity != 0.0f) {
    
        draw_grid(
            ANIM_EDITOR::GRID_INTERVAL,
            al_map_rgba(64, 64, 64, grid_opacity * 255),
            al_map_rgba(48, 48, 48, grid_opacity * 255)
        );
        
        point cam_top_left_corner(0, 0);
        point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &cam_top_left_corner.x, &cam_top_left_corner.y
        );
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
        );
        
        al_draw_line(
            0, cam_top_left_corner.y, 0, cam_bottom_right_corner.y,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
        al_draw_line(
            cam_top_left_corner.x, 0, cam_bottom_right_corner.x, 0,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
    }
    
    if(draw_mob_radius && loaded_mob_type) {
        if(side_view && state == EDITOR_STATE_HITBOXES) {
            //The radius isn't meant to be shown in side view.
        } else {
            draw_top_down_view_mob_radius(loaded_mob_type);
        }
    }
    
    if(draw_leader_silhouette) {
        float x_offset = 32;
        if(loaded_mob_type) {
            x_offset += loaded_mob_type->radius;
        }
        
        if(side_view && state == EDITOR_STATE_HITBOXES) {
            draw_side_view_leader_silhouette(x_offset);
        } else {
            draw_top_down_view_leader_silhouette(x_offset);
        }
    }
    
    if(state == EDITOR_STATE_ANIMATION) {
        al_use_transform(&game.identity_transform);
        draw_timeline();
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
}


/**
 * @brief Draws the comparison sprite on the canvas, all tinted and everything.
 */
void animation_editor::draw_comparison() {
    if(
        comparison && comparison_blink_show &&
        comparison_sprite && comparison_sprite->bitmap
    ) {
        ALLEGRO_COLOR tint;
        if(comparison_tint) {
            tint = al_map_rgb(255, 128, 0);
        } else {
            tint = comparison_sprite->tint;
        }
        draw_bitmap(
            comparison_sprite->bitmap,
            comparison_sprite->offset,
            point(
                comparison_sprite->bmp_size.x * comparison_sprite->scale.x,
                comparison_sprite->bmp_size.y * comparison_sprite->scale.y
            ),
            comparison_sprite->angle, tint
        );
    }
}


/**
 * @brief Draws a hitbox on the canvas in the sideways view.
 *
 * @param h_ptr Hitbox to draw.
 * @param color Color to use for the hitbox's main shape.
 * @param outline_color Color to use for the hitbox's outline.
 * @param outline_thickness Thickness of the hitbox's outline.
 */
void animation_editor::draw_side_view_hitbox(
    hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, float outline_thickness
) {
    float dummy = 0;
    float z_to_use = h_ptr->z;
    float h_to_use = h_ptr->height;
    
    if(h_ptr->height == 0) {
        //Set the coordinates to the screen top and screen bottom. Add some
        //padding just to make sure.
        z_to_use = game.win_h + 1;
        h_to_use = 0 - 1;
        al_transform_coordinates(
            &game.screen_to_world_transform, &dummy, &z_to_use
        );
        al_transform_coordinates(
            &game.screen_to_world_transform, &dummy, &h_to_use
        );
        //The height is the height from the top of the screen to the bottom.
        h_to_use = z_to_use - h_to_use;
        //Z needs to be flipped.
        z_to_use = -z_to_use;
    }
    
    al_draw_filled_rectangle(
        h_ptr->pos.x - h_ptr->radius,
        -z_to_use,
        h_ptr->pos.x + h_ptr->radius,
        -z_to_use - h_to_use,
        color
    );
    
    al_draw_rectangle(
        h_ptr->pos.x - h_ptr->radius,
        -z_to_use,
        h_ptr->pos.x + h_ptr->radius,
        -z_to_use - h_to_use,
        outline_color, outline_thickness
    );
}


/**
 * @brief Draws a leader's silhouette on the canvas in the sideways view.
 *
 * @param x_offset From the center, offset the silhouette this much
 * to the right.
 */
void animation_editor::draw_side_view_leader_silhouette(float x_offset) {
    draw_bitmap(
        game.sys_assets.bmp_leader_silhouette_side,
        point(x_offset, -game.config.standard_leader_height / 2.0),
        point(-1, game.config.standard_leader_height),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/**
 * @brief Draws a sprite on the canvas in the sideways view.
 *
 * @param s Sprite to draw.
 */
void animation_editor::draw_side_view_sprite(const sprite* s) {
    point min, max;
    ALLEGRO_COLOR color = COLOR_EMPTY;
    
    get_transformed_rectangle_bounding_box(
        s->offset, s->bmp_size * s->scale, s->angle,
        &min, &max
    );
    max.y = 0; //Bottom aligns with the floor.
    
    if(loaded_mob_type) {
        color = loaded_mob_type->main_color;
        min.y = loaded_mob_type->height;
    } else {
        min.y = max.x - min.x;
    }
    if(color.a == 0) {
        color = al_map_rgb(128, 32, 128);
    }
    min.y = -min.y; //Up is negative Y.
    al_draw_filled_rectangle(min.x, min.y, max.x, max.y, color);
}


/**
 * @brief Draws a timeline for the current animation.
 */
void animation_editor::draw_timeline() {
    if(!cur_anim_i.valid_frame()) return;
    
    //Some initial calculations.
    float anim_total_duration = 0;
    float anim_cur_time = 0;
    float anim_loop_time = 0;
    for(size_t f = 0; f < cur_anim_i.cur_anim->frames.size(); f++) {
        float f_dur = cur_anim_i.cur_anim->frames[f].duration;
        
        if(f < cur_anim_i.cur_frame_idx) {
            anim_cur_time += f_dur;
        } else if(f == cur_anim_i.cur_frame_idx) {
            anim_cur_time += cur_anim_i.cur_frame_time;
        }
        
        if(f < cur_anim_i.cur_anim->loop_frame) {
            anim_loop_time += f_dur;
        }
        
        anim_total_duration += f_dur;
    }
    if(anim_total_duration == 0.0f) return;
    
    float scale =
        (canvas_br.x - canvas_tl.x - ANIM_EDITOR::TIMELINE_PADDING * 2.0f) /
        anim_total_duration;
    float milestone_interval = 32.0f / scale;
    milestone_interval = floor(milestone_interval * 100.0f) / 100.0f;
    milestone_interval = std::max(milestone_interval, 0.01f);
    
    //Draw the entire timeline's rectangle.
    al_draw_filled_rectangle(
        canvas_tl.x, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        canvas_br.x, canvas_br.y,
        al_map_rgb(160, 180, 160)
    );
    
    //Draw every frame as a rectangle.
    float frame_rectangles_cur_x = canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING;
    float frame_rectangle_top =
        canvas_br.y -
        ANIM_EDITOR::TIMELINE_HEIGHT + ANIM_EDITOR::TIMELINE_HEADER_HEIGHT;
    float frame_rectangle_bottom =
        canvas_br.y - ANIM_EDITOR::TIMELINE_PADDING;
    for(size_t f = 0; f < cur_anim_i.cur_anim->frames.size(); f++) {
        float end_x =
            frame_rectangles_cur_x +
            cur_anim_i.cur_anim->frames[f].duration * scale;
        ALLEGRO_COLOR color =
            f % 2 == 0 ?
            al_map_rgb(128, 132, 128) :
            al_map_rgb(148, 152, 148);
            
        al_draw_filled_rectangle(
            frame_rectangles_cur_x, frame_rectangle_top,
            end_x, frame_rectangle_bottom,
            color
        );
        frame_rectangles_cur_x = end_x;
    }
    
    //Draw a triangle for the start of the loop frame.
    if(anim_total_duration) {
        float loop_x =
            canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING +
            anim_loop_time * scale;
        al_draw_filled_triangle(
            loop_x,
            frame_rectangle_bottom,
            loop_x,
            frame_rectangle_bottom - ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            loop_x + ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            frame_rectangle_bottom,
            al_map_rgb(64, 64, 96)
        );
    }
    
    //Draw a line indicating where we are in the animation.
    float cur_time_line_x =
        canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING + anim_cur_time * scale;
    al_draw_line(
        cur_time_line_x, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        cur_time_line_x, canvas_br.y,
        al_map_rgb(128, 48, 48), 2.0f
    );
    
    //Draw the milestone markers.
    float next_marker_x = 0.0f;
    unsigned char next_marker_type = 0;
    
    while(
        next_marker_x <
        canvas_br.x - canvas_tl.x - ANIM_EDITOR::TIMELINE_PADDING * 2
    ) {
        float x_to_use =
            next_marker_x + canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING;
        switch(next_marker_type) {
        case 0: {
            string text = f2s(next_marker_x / scale);
            if(text.size() >= 4) {
                text = text.substr(1, 3);
            }
            draw_text(
                text, game.sys_assets.fnt_builtin,
                point(
                    floor(x_to_use) + 2,
                    canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT + 2
                ),
                point(LARGE_FLOAT, 8.0f), al_map_rgb(32, 32, 32),
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
            );
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 1:
        case 3: {
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.66f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 2: {
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvas_br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.33f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        }
        }
        
        next_marker_x += scale * milestone_interval;
        next_marker_type = sum_and_wrap(next_marker_type, 1, 4);
    }
}


/**
 * @brief Draws a hitbox on the canvas in the standard top-down view.
 *
 * @param h_ptr Hitbox to draw.
 * @param color Color of the hitbox's main shape.
 * @param outline_color Color of the hitbox's outline.
 * @param outline_thickness Thickness of the hitbox's outline.
 */
void animation_editor::draw_top_down_view_hitbox(
    hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, float outline_thickness
) {
    if(h_ptr->radius <= 0) return;
    
    al_draw_filled_circle(
        h_ptr->pos.x, h_ptr->pos.y, h_ptr->radius, color
    );
    
    al_draw_circle(
        h_ptr->pos.x, h_ptr->pos.y,
        h_ptr->radius, outline_color, outline_thickness
    );
}


/**
 * @brief Draws a leader silhouette on the canvas in the standard top-down view.
 *
 * @param x_offset From the center, offset the leader silhouette this much
 * to the right.
 */
void animation_editor::draw_top_down_view_leader_silhouette(
    float x_offset
) {
    draw_bitmap(
        game.sys_assets.bmp_leader_silhouette_top, point(x_offset, 0),
        point(-1, game.config.standard_leader_radius * 2.0f),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/**
 * @brief Draws the mob radius on the canvas in the standard top-down view.
 *
 * @param mt Type of the mob to draw.
 */
void animation_editor::draw_top_down_view_mob_radius(mob_type* mt) {
    al_draw_circle(
        0, 0, mt->radius,
        al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
    );
    if(mt->rectangular_dim.x != 0) {
        al_draw_rectangle(
            -mt->rectangular_dim.x / 2.0, -mt->rectangular_dim.y / 2.0,
            mt->rectangular_dim.x / 2.0, mt->rectangular_dim.y / 2.0,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
    }
}


/**
 * @brief Draws a sprite on the canvas in the standard top-down view.
 *
 * @param s Sprite to draw.
 */
void animation_editor::draw_top_down_view_sprite(sprite* s) {
    if(!comparison_above) {
        draw_comparison();
    }
    
    sprite* next_s = nullptr;
    float interpolation_factor = 0.0f;
    if(state == EDITOR_STATE_ANIMATION && cur_anim_i.valid_frame()) {
        cur_anim_i.get_sprite_data(
            nullptr, &next_s, &interpolation_factor
        );
    }
    
    if(s->bitmap) {
        point coords;
        float angle;
        point scale;
        ALLEGRO_COLOR tint;
        
        get_sprite_basic_effects(
            point(), 0, LARGE_FLOAT, LARGE_FLOAT,
            s, next_s, interpolation_factor,
            &coords, &angle, &scale, &tint
        );
        
        if(
            state == EDITOR_STATE_SPRITE_TRANSFORM &&
            comparison && comparison_tint &&
            comparison_sprite && comparison_sprite->bitmap
        ) {
            tint = al_map_rgb(0, 128, 255);
        }
        draw_bitmap(
            s->bitmap, coords,
            point(
                s->bmp_size.x * scale.x,
                s->bmp_size.y * scale.y
            ),
            angle, tint
        );
    }
    
    if(
        s->top_visible && loaded_mob_type
        && loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
        point coords;
        float angle;
        point size;
        get_sprite_basic_top_effects(
            s, next_s, interpolation_factor,
            &coords, &angle, &size
        );
        draw_bitmap(top_bmp[cur_maturity], coords, size, angle);
    }
    
    if(comparison_above) {
        draw_comparison();
    }
}
