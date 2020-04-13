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
#include "../../vars.h"

/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {

    gui->draw();
    
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
    
    bool draw_origin = origin_visible;
    bool draw_hitboxes = hitboxes_visible;
    bool draw_mob_radius = mob_radius_visible;
    bool draw_pikmin_silhouette = pikmin_silhouette_visible;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_TOP) {
        draw_hitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        draw_origin = false;
        draw_hitboxes = false;
        draw_mob_radius = false;
        draw_pikmin_silhouette = false;
        
        if(s && s->parent_bmp) {
            draw_origin = false;
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
                if(x == 0) {
                    rec_tl.x = scene_tl.x;
                    rec_br.x = bmp_x + s->file_pos.x;
                } else if(x == 1) {
                    rec_tl.x = bmp_x + s->file_pos.x;
                    rec_br.x = bmp_x + s->file_pos.x + s->file_size.x;
                } else {
                    rec_tl.x = bmp_x + s->file_pos.x + s->file_size.x;
                    rec_br.x = scene_br.x;
                }
                for(unsigned char y = 0; y < 3; ++y) {
                    if(x == 1 && y == 1) continue;
                    if(y == 0) {
                        rec_tl.y = scene_tl.y;
                        rec_br.y = bmp_y + s->file_pos.y;
                    } else if(y == 1) {
                        rec_tl.y = bmp_y + s->file_pos.y;
                        rec_br.y = bmp_y + s->file_pos.y + s->file_size.y;
                    } else {
                        rec_tl.y = bmp_y + s->file_pos.y + s->file_size.y;
                        rec_br.y = scene_br.y;
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
            
            for(size_t h = 0; h < n_hitboxes; ++h) {
                hitbox* h_ptr = &s->hitboxes[h];
                ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                float hitbox_outline_thickness =
                    (cur_hitbox_nr == h ? 3 / game.cam.zoom : 2 / game.cam.zoom);
                    
                if(h_ptr->type == HITBOX_TYPE_NORMAL) {
                    hitbox_color = al_map_rgba(0, 128, 0, 128);
                    hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                } else if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                    hitbox_color = al_map_rgba(128, 0, 0, 128);
                    hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                } else {
                    hitbox_color = al_map_rgba(128, 128, 0, 128);
                    hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
                }
                
                if(cur_hitbox_nr == h) {
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
            cur_sprite_tc.draw_handles();
        } else if(state == EDITOR_STATE_TOP && s->top_visible) {
            top_tc.draw_handles();
        } else if(state == EDITOR_STATE_HITBOXES && cur_hitbox) {
            cur_hitbox_tc.draw_handles();
        }
    }
    
    if(draw_origin) {
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
            al_map_rgb(240, 240, 240), 1 / game.cam.zoom
        );
        al_draw_line(
            cam_top_left_corner.x, 0, cam_bottom_right_corner.x, 0,
            al_map_rgb(240, 240, 240), 1 / game.cam.zoom
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
    
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
    
    draw_unsaved_changes_warning();
    
    game.fade_mgr.draw();
    
    al_flip_display();
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
 */
void animation_editor::draw_side_view_pikmin_silhouette(const float x_offset) {
    draw_bitmap(
        bmp_pikmin_silhouette,
        point(x_offset, -game.config.standard_pikmin_height / 2.0),
        point(-1, game.config.standard_pikmin_height),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/* ----------------------------------------------------------------------------
 * Draws a sprite on the canvas in the sideways view.
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
 * Draws a hitbox on the canvas in the standard top-down view.
 */
void animation_editor::draw_top_down_view_hitbox(
    hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, const float outline_thickness
) {
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
 */
void animation_editor::draw_top_down_view_mob_radius(mob_type* mt) {
    al_draw_circle(
        0, 0, mt->radius,
        al_map_rgb(240, 240, 240), 1.0 / game.cam.zoom
    );
    if(mt->rectangular_dim.x != 0) {
        al_draw_rectangle(
            -mt->rectangular_dim.x / 2.0, -mt->rectangular_dim.y / 2.0,
            mt->rectangular_dim.x / 2.0, mt->rectangular_dim.y / 2.0,
            al_map_rgb(240, 240, 240), 1.0 / game.cam.zoom
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws a Pikmin silhouette on the canvas in the standard top-down view.
 */
void animation_editor::draw_top_down_view_pikmin_silhouette(
    const float x_offset
) {
    draw_bitmap(
        bmp_pikmin_silhouette, point(x_offset, 0),
        point(-1, game.config.standard_pikmin_height),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/* ----------------------------------------------------------------------------
 * Draws a sprite on the canvas in the standard top-down view.
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
