/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor drawing functions.
 */

#include <algorithm>

#include "editor.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


const float ANIMATION_EDITOR_GRID_INTERVAL = 16.0f;

/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {
    //Render what is needed for the GUI.
    //This will also render the canvas in due time.
    ImGui::Render();
    
    //Actually draw the GUI + canvas on-screen.
    al_clear_to_color(al_map_rgb(0, 0, 0));
    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    
    draw_unsaved_changes_warning();
    
    //And the fade manager atop it all.
    game.fade_mgr.draw();
    
    //Finally, swap buffers.
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draw the canvas. This is called as a callback inside the
 * ImGui rendering process.
 */
void animation_editor::draw_canvas() {
    al_use_transform(&game.world_to_screen_transform);
    
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    al_clear_to_color(al_map_rgb(128, 144, 128));
    
    sprite* s = NULL;
    if(state == EDITOR_STATE_ANIMATION) {
        if(cur_frame_nr != INVALID) {
            string name =
                cur_anim->frames[cur_frame_nr].sprite_name;
            size_t s_pos = anims.find_sprite(name);
            if(s_pos != INVALID) s = anims.sprites[s_pos];
        }
        
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
    bool draw_pikmin_silhouette = pikmin_silhouette_visible;
    float grid_opacity = grid_visible ? 0.33f : 0.0f;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_TOP) {
        draw_hitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        grid_opacity = 0.0f;
        draw_hitboxes = false;
        draw_mob_radius = false;
        draw_pikmin_silhouette = false;
        
        if(s && s->parent_bmp) {
            grid_opacity = 0.0f;
            draw_hitboxes = false;
            draw_mob_radius = false;
            draw_pikmin_silhouette = false;
            
            int bmp_w = al_get_bitmap_width(s->parent_bmp);
            int bmp_h = al_get_bitmap_height(s->parent_bmp);
            int bmp_x = -bmp_w / 2.0;
            int bmp_y = -bmp_h / 2.0;
            al_draw_bitmap(s->parent_bmp, bmp_x, bmp_y, 0);
            
            point scene_tl = point(-1, -1);
            point scene_br = point(canvas_br.x + 1, canvas_br.y + 1);
            al_transform_coordinates(
                &game.screen_to_world_transform, &scene_tl.x, &scene_tl.y
            );
            al_transform_coordinates(
                &game.screen_to_world_transform, &scene_br.x, &scene_br.y
            );
            
            for(unsigned char x = 0; x < 3; ++x) {
                point rec_tl, rec_br;
                switch(x) {
                case 0: {
                    rec_tl.x = scene_tl.x;
                    rec_br.x = bmp_x + s->file_pos.x;
                    break;
                } case 1: {
                    rec_tl.x = bmp_x + s->file_pos.x;
                    rec_br.x = bmp_x + s->file_pos.x + s->file_size.x;
                    break;
                } default: {
                    rec_tl.x = bmp_x + s->file_pos.x + s->file_size.x;
                    rec_br.x = scene_br.x;
                    break;
                }
                }
                
                for(unsigned char y = 0; y < 3; ++y) {
                    if(x == 1 && y == 1) continue;
                    
                    switch(y) {
                    case 0: {
                        rec_tl.y = scene_tl.y;
                        rec_br.y = bmp_y + s->file_pos.y;
                        break;
                    } case 1: {
                        rec_tl.y = bmp_y + s->file_pos.y;
                        rec_br.y = bmp_y + s->file_pos.y + s->file_size.y;
                        break;
                    } default: {
                        rec_tl.y = bmp_y + s->file_pos.y + s->file_size.y;
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
            
            if(s->file_size.x > 0 && s->file_size.y > 0) {
            
                unsigned char outline_alpha =
                    255 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
                al_draw_rectangle(
                    bmp_x + s->file_pos.x + 0.5,
                    bmp_y + s->file_pos.y + 0.5,
                    bmp_x + s->file_pos.x + s->file_size.x - 0.5,
                    bmp_y + s->file_pos.y + s->file_size.y - 0.5,
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
            
            for(int h = n_hitboxes - 1; h >= 0; --h) {
                //Iterate the hitboxes in reverse order, since this is
                //the order of priority the engine has when checking for
                //collisions. Making higher priority hitboxes appear above
                //lower ones makes it all more intuitive and cohesive.
                hitbox* h_ptr = &s->hitboxes[h];
                ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                float hitbox_outline_thickness =
                    cur_hitbox_nr == (size_t) h ?
                    3.0f / game.cam.zoom :
                    2 / game.cam.zoom;
                    
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
                
                if(cur_hitbox_nr == (size_t) h) {
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
            point cur_sprite_size = cur_sprite->scale * cur_sprite->file_size;
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
                    NULL,
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
                    NULL,
                    1.0f / game.cam.zoom
                );
            }
            
        }
    }
    
    //Grid.
    if(grid_opacity != 0.0f) {
    
        draw_grid(
            ANIMATION_EDITOR_GRID_INTERVAL,
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
    
    if(draw_pikmin_silhouette) {
        float x_offset = 32;
        if(loaded_mob_type) {
            x_offset += loaded_mob_type->radius;
        }
        
        if(side_view && state == EDITOR_STATE_HITBOXES) {
            draw_side_view_pikmin_silhouette(x_offset);
        } else {
            draw_top_down_view_pikmin_silhouette(x_offset);
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


/* ----------------------------------------------------------------------------
 * Draws the comparison sprite on the canvas, all tinted and everything.
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
            tint = al_map_rgb(255, 255, 255);
        }
        draw_bitmap(
            comparison_sprite->bitmap,
            comparison_sprite->offset,
            point(
                comparison_sprite->file_size.x * comparison_sprite->scale.x,
                comparison_sprite->file_size.y * comparison_sprite->scale.y
            ),
            comparison_sprite->angle, tint
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws a hitbox on the canvas in the sideways view.
 * h_ptr:
 *   Hitbox to draw.
 * color:
 *   Color to use for the hitbox's main shape.
 * outline_color:
 *   Color to use for the hitbox's outline.
 * outline_thickness:
 *   Thickness of the hitbox's outline.
 */
void animation_editor::draw_side_view_hitbox(
    hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, const float outline_thickness
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


/* ----------------------------------------------------------------------------
 * Draws a Pikmin silhouette on the canvas in the sideways view.
 * x_offset:
 *   From the center, offset the silhouette this much to the right.
 */
void animation_editor::draw_side_view_pikmin_silhouette(const float x_offset) {
    draw_bitmap(
        game.sys_assets.bmp_pikmin_silhouette,
        point(x_offset, -game.config.standard_pikmin_height / 2.0),
        point(-1, game.config.standard_pikmin_height),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/* ----------------------------------------------------------------------------
 * Draws a sprite on the canvas in the sideways view.
 * s:
 *   Sprite to draw.
 */
void animation_editor::draw_side_view_sprite(sprite* s) {
    point min, max;
    ALLEGRO_COLOR color = al_map_rgba(0, 0, 0, 0);
    
    get_transformed_rectangle_bounding_box(
        s->offset, s->file_size * s->scale, s->angle,
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


/* ----------------------------------------------------------------------------
 * Draws a timeline for the current animation.
 */
void animation_editor::draw_timeline() {
    if(!cur_anim || cur_anim->frames.empty()) return;
    
    //Some initial calculations.
    float anim_total_duration = 0;
    float anim_cur_time = 0;
    float anim_loop_time = 0;
    for(size_t f = 0; f < cur_anim->frames.size(); ++f) {
        float f_dur = cur_anim->frames[f].duration;
        
        if(f < cur_frame_nr) {
            anim_cur_time += f_dur;
        } else if(f == cur_frame_nr) {
            anim_cur_time += cur_frame_time;
        }
        
        if(f < cur_anim->loop_frame) {
            anim_loop_time += f_dur;
        }
        
        anim_total_duration += f_dur;
    }
    float scale =
        (canvas_br.x - canvas_tl.x - TIMELINE_PADDING * 2.0f) /
        anim_total_duration;
    float milestone_interval = 32.0f / scale;
    milestone_interval = floor(milestone_interval * 100.0f) / 100.0f;
    milestone_interval = std::max(milestone_interval, 0.01f);
    
    //Draw the entire timeline's rectangle.
    al_draw_filled_rectangle(
        canvas_tl.x, canvas_br.y - TIMELINE_HEIGHT,
        canvas_br.x, canvas_br.y,
        al_map_rgb(160, 180, 160)
    );
    
    //Draw every frame as a rectangle.
    float frame_rectangles_cur_x = canvas_tl.x + TIMELINE_PADDING;
    float frame_rectangle_top =
        canvas_br.y - TIMELINE_HEIGHT + TIMELINE_HEADER_HEIGHT;
    float frame_rectangle_bottom =
        canvas_br.y - TIMELINE_PADDING;
    for(size_t f = 0; f < cur_anim->frames.size(); ++f) {
        float end_x =
            frame_rectangles_cur_x +
            cur_anim->frames[f].duration * scale;
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
            canvas_tl.x + TIMELINE_PADDING +
            anim_loop_time * scale;
        al_draw_filled_triangle(
            loop_x, frame_rectangle_bottom,
            loop_x, frame_rectangle_bottom - TIMELINE_LOOP_TRI_SIZE,
            loop_x + TIMELINE_LOOP_TRI_SIZE, frame_rectangle_bottom,
            al_map_rgb(64, 64, 96)
        );
    }
    
    //Draw a line indicating where we are in the animation.
    float cur_time_line_x =
        canvas_tl.x + TIMELINE_PADDING + anim_cur_time * scale;
    al_draw_line(
        cur_time_line_x, canvas_br.y - TIMELINE_HEIGHT,
        cur_time_line_x, canvas_br.y,
        al_map_rgb(128, 48, 48), 2.0f
    );
    
    //Draw the milestone markers.
    float next_marker_x = 0.0f;
    unsigned char next_marker_type = 0;
    
    while(next_marker_x < canvas_br.x - canvas_tl.x - TIMELINE_PADDING * 2) {
        float x_to_use = next_marker_x + canvas_tl.x + TIMELINE_PADDING;
        switch(next_marker_type) {
        case 0: {
            string text = f2s(next_marker_x / scale);
            if(text.size() >= 4) {
                text = text.substr(1, 3);
            }
            al_draw_text(
                game.fonts.builtin, al_map_rgb(32, 32, 32),
                floor(x_to_use) + 2,
                canvas_br.y - TIMELINE_HEIGHT + 2,
                ALLEGRO_ALIGN_LEFT,
                text.c_str()
            );
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - TIMELINE_HEIGHT,
                x_to_use + 0.5, canvas_br.y - TIMELINE_HEIGHT +
                TIMELINE_HEADER_HEIGHT,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 1:
        case 3: {
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvas_br.y - TIMELINE_HEIGHT + TIMELINE_HEADER_HEIGHT * 0.66f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 2: {
            al_draw_line(
                x_to_use + 0.5, canvas_br.y - TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvas_br.y - TIMELINE_HEIGHT + TIMELINE_HEADER_HEIGHT * 0.33f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        }
        }
        
        next_marker_x += scale * milestone_interval;
        next_marker_type = sum_and_wrap(next_marker_type, 1, 4);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a hitbox on the canvas in the standard top-down view.
 * h_ptr:
 *   Hitbox to draw.
 * color:
 *   Color of the hitbox's main shape.
 * outline_color:
 *   Color of the hitbox's outline.
 * outline_thickness:
 *   Thickness of the hitbox's outline.
 */
void animation_editor::draw_top_down_view_hitbox(
    hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, const float outline_thickness
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


/* ----------------------------------------------------------------------------
 * Draws the mob radius on the canvas in the standard top-down view.
 * mt:
 *   Type of the mob to draw.
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


/* ----------------------------------------------------------------------------
 * Draws a Pikmin silhouette on the canvas in the standard top-down view.
 * x_offset:
 *   From the center, offset the Pikmin silhouette this much to the right.
 */
void animation_editor::draw_top_down_view_pikmin_silhouette(
    const float x_offset
) {
    draw_bitmap(
        game.sys_assets.bmp_pikmin_silhouette, point(x_offset, 0),
        point(-1, game.config.standard_pikmin_height),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/* ----------------------------------------------------------------------------
 * Draws a sprite on the canvas in the standard top-down view.
 * s:
 *   Sprite to draw.
 */
void animation_editor::draw_top_down_view_sprite(sprite* s) {
    if(!comparison_above) {
        draw_comparison();
    }
    
    if(s->bitmap) {
        ALLEGRO_COLOR tint;
        if(
            state == EDITOR_STATE_SPRITE_TRANSFORM &&
            comparison && comparison_tint &&
            comparison_sprite && comparison_sprite->bitmap
        ) {
            tint = al_map_rgb(0, 128, 255);
        } else {
            tint = al_map_rgb(255, 255, 255);
        }
        draw_bitmap(
            s->bitmap, s->offset,
            point(
                s->file_size.x * s->scale.x,
                s->file_size.y * s->scale.y
            ),
            s->angle, tint
        );
    }
    
    if(
        s->top_visible && loaded_mob_type
        && loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
        draw_bitmap(
            top_bmp[cur_maturity],
            s->top_pos, s->top_size,
            s->top_angle
        );
    }
    
    if(comparison_above) {
        draw_comparison();
    }
}
