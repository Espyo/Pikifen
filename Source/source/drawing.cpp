/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Global drawing functions.
 */

#include <algorithm>
#include <typeinfo>

#include "drawing.h"

#include "animation.h"
#include "const.h"
#include "functions.h"
#include "game.h"
#include "game_states/gameplay/gameplay.h"
#include "mobs/group_task.h"
#include "mobs/pile.h"
#include "mobs/scale.h"
#include "utils/geometry_utils.h"
#include "utils/string_utils.h"


namespace CONTROL_BIND_ICON {
//Base rectangle outline color.
const ALLEGRO_COLOR BASE_OUTLINE_COLOR = {0.10f, 0.10f, 0.10f, 1.0f};
//Base rectangle body color.
const ALLEGRO_COLOR BASE_RECT_COLOR = {0.45f, 0.45f, 0.45f, 1.0f};
//Base text color.
const ALLEGRO_COLOR BASE_TEXT_COLOR = {0.95f, 0.95f, 0.95f, 1.0f};
//Rectangle outline thickness.
const float OUTLINE_THICKNESS = 2.0f;
//Padding between text and rectangle limit.
const float PADDING = 4.0f;
}


namespace DRAWING {
//Default healt wheel radius.
const float DEF_HEALTH_WHEEL_RADIUS = 20;
//Liquid surfaces wobble by offsetting X by this much, at most.
const float LIQUID_WOBBLE_DELTA_X = 3.0f;
//Liquid surfaces wobble using this time scale.
const float LIQUID_WOBBLE_TIME_SCALE = 2.0f;
//Loading screen subtitle text padding.
const int LOADING_SCREEN_PADDING = 64;
//Loading screen subtitle text scale.
const float LOADING_SCREEN_SUBTITLE_SCALE = 0.6f;
//Notification opacity.
const unsigned char NOTIFICATION_ALPHA = 160;
//Size of a control bind icon in a notification.
const float NOTIFICATION_CONTROL_SIZE = 24.0f;
//Padding between a notification's text and its limit.
const float NOTIFICATION_PADDING = 8.0f;
}


/* ----------------------------------------------------------------------------
 * Draws a series of logos, to serve as a background.
 * They move along individually, and wrap around when they reach a screen edge.
 * time_spent:
 *   How much time has passed.
 * rows:
 *   Rows of logos to draw.
 * cols:
 *   Columns of logos to draw.
 * logo_size:
 *   Width and height of the logos.
 * tint:
 *   Tint the logos with this color.
 * speed:
 *   Horizontal and vertical movement speed of each logo.
 * rotation_speed:
 *   Rotation speed of each logo.
 */
void draw_background_logos(
    const float time_spent, const size_t rows, const size_t cols,
    const point &logo_size, const ALLEGRO_COLOR &tint,
    const point &speed, const float rotation_speed
) {
    al_hold_bitmap_drawing(true);
    
    float spacing_x = (game.win_w + logo_size.x) / cols;
    float spacing_y = (game.win_h + logo_size.y) / rows;
    
    for(size_t c = 0; c < cols; ++c) {
        for(size_t r = 0; r < rows; ++r) {
            float x = (c * spacing_x) + time_spent * speed.x;
            if(r % 2 == 0) {
                x += spacing_x / 2.0f;
            }
            x =
                wrap_float(
                    x,
                    0 - logo_size.x * 0.5f,
                    game.win_w + logo_size.x * 0.5f
                );
            float y =
                wrap_float(
                    (r * spacing_y) + time_spent * speed.y,
                    0 - logo_size.y * 0.5f,
                    game.win_h + logo_size.y * 0.5f
                );
            draw_bitmap(
                game.sys_assets.bmp_icon,
                point(x, y),
                point(logo_size.x, logo_size.y),
                time_spent * rotation_speed,
                tint
            );
        }
    }
    
    al_hold_bitmap_drawing(false);
}


/* ----------------------------------------------------------------------------
 * Draws a bitmap.
 * bmp:
 *   The bitmap.
 * center:
 *   Center coordinates.
 * size:
 *   Final width and height.
 *   Make this -1 on one of them to keep the aspect ratio from the other.
 * angle:
 *   Angle to rotate the bitmap by.
 * tint:
 *   Tint the bitmap with this color.
 */
void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle, const ALLEGRO_COLOR &tint
) {

    if(size.x == 0 && size.y == 0) return;
    
    if(!bmp) {
        bmp = game.bmp_error;
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
 * bmp:
 *   The bitmap.
 * center:
 *   Center coordinates.
 * box_size:
 *   Width and height of the box.
 * scale_up:
 *   If true, the bitmap is scaled up to fit the box. If false, it stays
 *   at its original size (unless it needs to be scaled down).
 * angle:
 *   Angle to rotate the bitmap by.
 *   The box does not take angling into account.
 * tint:
 *   Tint the bitmap with this color.
 */
void draw_bitmap_in_box(
    ALLEGRO_BITMAP* bmp, const point &center, const point &box_size,
    const bool scale_up, const float angle, const ALLEGRO_COLOR &tint
) {
    if(box_size.x == 0 || box_size.y == 0) return;
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    float w_diff = bmp_w / box_size.x;
    float h_diff = bmp_h / box_size.y;
    float max_w = scale_up ? box_size.x : std::min((int) box_size.x, bmp_w);
    float max_h = scale_up ? box_size.y : std::min((int) box_size.y, bmp_h);
    
    if(w_diff > h_diff) {
        draw_bitmap(bmp, center, point(max_w, -1), angle, tint);
    } else {
        draw_bitmap(bmp, center, point(-1, max_h), angle, tint);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a bitmap, applying bitmap effects.
 * bmp:
 *   The bitmap.
 * effects:
 *   Effects to use.
 */
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const bitmap_effect_info &effects
) {

    if(!bmp) {
        bmp = game.bmp_error;
    }
    
    point bmp_size(al_get_bitmap_width(bmp), al_get_bitmap_height(bmp));
    float scale_x =
        (effects.scale.x == LARGE_FLOAT) ? effects.scale.y : effects.scale.x;
    float scale_y =
        (effects.scale.y == LARGE_FLOAT) ? effects.scale.x : effects.scale.y;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        effects.tint_color,
        bmp_size.x / 2, bmp_size.y / 2,
        effects.translation.x, effects.translation.y,
        scale_x, scale_y,
        effects.rotation,
        0
    );
    
    if(effects.glow_color.a > 0) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        al_draw_tinted_scaled_rotated_bitmap(
            bmp,
            effects.glow_color,
            bmp_size.x / 2, bmp_size.y / 2,
            effects.translation.x, effects.translation.y,
            scale_x, scale_y,
            effects.rotation,
            0
        );
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws a button on the screen.
 * center:
 *   Center coordinates.
 * size:
 *   Width and height.
 * text:
 *   Text inside the button.
 * font:
 *   What font to write the text in.
 * color:
 *   Color to draw the text with.
 * selected:
 *   Is the button currently selected?
 * juicy_grow_amount:
 *   If it's in the middle of a juicy grow animation, specify the amount here.
 */
void draw_button(
    const point &center, const point &size, const string &text,
    ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    const bool selected, const float juicy_grow_amount
) {
    draw_compressed_scaled_text(
        font, color,
        center,
        point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
        ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, true,
        text
    );
    
    ALLEGRO_COLOR box_tint =
        selected ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
        
    draw_textured_box(
        center, size, game.sys_assets.bmp_bubble_box, box_tint
    );
    
    if(selected) {
        draw_textured_box(
            center,
            size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
            game.sys_assets.bmp_focus_box
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws text, scaled, but also compresses (scales) it
 * to fit within the specified range.
 * font:
 *   Font to use.
 * color:
 *   Tint the text by this color.
 * where:
 *   Coordinates to draw it at.
 * scale:
 *   Scale to use.
 * flags:
 *   Allegro text render function flags.
 * valign:
 *   Vertical alignment.
 * max_size:
 *   The maximum width and height. Use <= 0 to have no limit.
 * scale_past_max:
 *   If true, the max size will only be taken into account when the scale is 1.
 *   If it is any bigger, it will overflow past the max size.
 * text:
 *   Text to draw.
 */
void draw_compressed_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODES valign,
    const point &max_size, const bool scale_past_max, const string &text
) {

    if(max_size.x == 0 && max_size.y == 0) return;
    
    int text_ox;
    int text_oy;
    int text_w;
    int text_h;
    al_get_text_dimensions(
        font, text.c_str(),
        &text_ox, &text_oy, &text_w, &text_h
    );
    
    point normal_text_size(text_w, text_h);
    point text_size_to_check = normal_text_size;
    point final_scale(1.0f, 1.0f);
    
    if(!scale_past_max) {
        final_scale = scale;
        text_size_to_check = normal_text_size * scale;
    }
    
    if(max_size.x > 0 && text_size_to_check.x > max_size.x) {
        final_scale.x = max_size.x / normal_text_size.x;
    }
    if(max_size.y > 0 && text_size_to_check.y > max_size.y) {
        final_scale.y = max_size.y / normal_text_size.y;
    }
    
    if(scale_past_max) {
        final_scale = final_scale * scale;
    }
    
    float final_text_height = normal_text_size.y * final_scale.y;
    float valign_offset =
        valign == TEXT_VALIGN_CENTER ?
        final_text_height / 2.0f :
        valign == TEXT_VALIGN_BOTTOM ?
        final_text_height :
        0.0f;
        
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, final_scale.x, final_scale.y);
    al_translate_transform(
        &scale_transform,
        where.x,
        where.y - valign_offset
    );
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        al_draw_text(font, color, 0, 0, flags, text.c_str());
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws text on the screen, but compresses (scales) it
 * to fit within the specified range.
 * font:
 *   Font to use.
 * color:
 *   Tint the text by this color.
 * where:
 *   Coordinates to draw it at.
 * flags:
 *   Allegro text render function flags.
 * valign:
 *   Vertical alignment.
 * max_size:
 *   The maximum width and height. Use <= 0 to have no limit.
 * text:
 *   Text to draw.
 */
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const TEXT_VALIGN_MODES valign,
    const point &max_size, const string &text
) {
    if(max_size.x == 0 && max_size.y == 0) return;
    
    int text_ox;
    int text_oy;
    int text_w;
    int text_h;
    al_get_text_dimensions(
        font, text.c_str(),
        &text_ox, &text_oy, &text_w, &text_h
    );
    point scale(1.0, 1.0);
    float final_text_height = text_h;
    
    if(text_w > max_size.x && max_size.x > 0) {
        scale.x = max_size.x / text_w;
    }
    if(text_h > max_size.y && max_size.y > 0) {
        scale.y = max_size.y / text_h;
        final_text_height = max_size.y;
    }
    
    float valign_offset =
        valign == TEXT_VALIGN_CENTER ?
        final_text_height / 2.0f :
        valign == TEXT_VALIGN_BOTTOM ?
        final_text_height :
        0.0f;
        
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale.x, scale.y);
    al_translate_transform(
        &scale_transform,
        where.x,
        where.y - valign_offset - text_oy
    );
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        al_draw_text(font, color, 0, 0, flags, text.c_str());
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws a filled diamond shape.
 * center:
 *   Center.
 * radius:
 *   How far each point of the diamond reaches from the center.
 * color:
 *   Color the diamond with this color.
 */
void draw_filled_diamond(
    const point &center, const float radius, const ALLEGRO_COLOR &color
) {
    ALLEGRO_VERTEX vert[4];
    for(unsigned char v = 0; v < 4; ++v) {
        vert[v].color = color;
        vert[v].z = 0;
    }
    
    vert[0].x = center.x;
    vert[0].y = center.y - radius;
    vert[1].x = center.x + radius;
    vert[1].y = center.y;
    vert[2].x = center.x;
    vert[2].y = center.y + radius;
    vert[3].x = center.x - radius;
    vert[3].y = center.y;
    
    al_draw_prim(vert, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
}


/* ----------------------------------------------------------------------------
 * Draws a filled rounded rectangle.
 * This is basically Allegro's function, but safer and simpler.
 * center:
 *   Center.
 * size:
 *   Width and height.
 * radii:
 *   Radii of the corners. Will be smaller if the rectangle is too small.
 * color:
 *   Color the rectangle with this color.
 */
void draw_filled_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color
) {
    float final_radii = std::min(radii, size.x / 2.0f);
    final_radii = std::min(final_radii, size.y / 2.0f);
    final_radii = std::max(0.0f, final_radii);
    al_draw_filled_rounded_rectangle(
        center.x - size.x / 2.0f, center.y - size.y / 2.0f,
        center.x + size.x / 2.0f, center.y + size.y / 2.0f,
        final_radii, final_radii,
        color
    );
}


/* ----------------------------------------------------------------------------
 * Draws a fraction, so one number above another, divided by a bar.
 * The top number usually represents the current value of some attribute,
 * and the bottom number usually represents the required value for some goal.
 * bottom:
 *   Bottom center point of the text.
 * value_nr:
 *   Number that represents the current value.
 * requirement_nr:
 *   Number that represents the requirement.
 * color:
 *   Color of the fraction's text.
 * scale:
 *   Scale the fraction by this much.
 */
void draw_fraction(
    const point &bottom, const size_t value_nr,
    const size_t requirement_nr, const ALLEGRO_COLOR &color, const float scale
) {
    float font_h = al_get_font_line_height(game.fonts.value) * scale;
    
    float value_nr_y = bottom.y - font_h * 3;
    float value_nr_scale =
        value_nr >= requirement_nr ? scale * 1.2f : scale * 1.0f;
    draw_scaled_text(
        game.fonts.value, color, point(bottom.x, value_nr_y),
        point(value_nr_scale, value_nr_scale),
        ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_TOP, (i2s(value_nr).c_str())
    );
    
    float bar_y = bottom.y - font_h * 2;
    draw_scaled_text(
        game.fonts.value, color, point(bottom.x, bar_y),
        point(scale, scale),
        ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_TOP, "-"
    );
    
    float req_nr_y = bottom.y - font_h;
    float req_nr_scale =
        requirement_nr > value_nr ? scale * 1.2f : scale * 1.0f;
    draw_scaled_text(
        game.fonts.value, color, point(bottom.x, req_nr_y),
        point(req_nr_scale, req_nr_scale),
        ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_TOP, (i2s(requirement_nr).c_str())
    );
}


/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * center:
 *   Center of the wheel.
 * ratio:
 *   Ratio of health that is filled. 0 is empty, 1 is full.
 * alpha:
 *   Total opacity of the health wheel.
 * radius:
 *   Radius of the wheel (the whole wheel, not just the pieslice).
 * just_chart:
 *   If true, only draw the actual pieslice (pie-chart).
 *   Used for leader HP on the HUD.
 */
void draw_health(
    const point &center,
    const float ratio, const float alpha,
    const float radius, const bool just_chart
) {
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgba_f(1 - (ratio - 0.5) * 2, 1, 0, alpha);
    } else {
        c = al_map_rgba_f(1, (ratio * 2), 0, alpha);
    }
    
    if(!just_chart) {
        al_draw_filled_circle(
            center.x, center.y, radius, al_map_rgba(0, 0, 0, 128 * alpha)
        );
    }
    al_draw_filled_pieslice(
        center.x, center.y, radius, -TAU / 4, -ratio * TAU, c
    );
    if(!just_chart) {
        al_draw_circle(
            center.x, center.y, radius + 1, al_map_rgba(0, 0, 0, alpha * 255), 2
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws a liquid sector.
 * s_ptr:
 *   Pointer to the sector.
 * l_ptr:
 *   Pointer to the liquid.
 * where:
 *   X and Y offset.
 * scale:
 *   Scale the sector by this much.
 * time:
 *   How much time has passed. Used to animate.
 */
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, const float scale,
    const float time
) {

    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        av[v].z = 0;
    }
    
    float liquid_opacity_mult = 1.0f;
    if(s_ptr->draining_liquid) {
        liquid_opacity_mult =
            s_ptr->liquid_drain_left / GEOMETRY::LIQUID_DRAIN_DURATION;
    }
    float brightness_mult = s_ptr->brightness / 255.0;
    
    //Layer 1 - Transparent wobbling ground texture.
    if(s_ptr->texture_info.bitmap) {
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -s_ptr->texture_info.translation.x,
            -s_ptr->texture_info.translation.y,
            1.0f / s_ptr->texture_info.scale.x,
            1.0f / s_ptr->texture_info.scale.y,
            -s_ptr->texture_info.rot
        );
        
        float ground_wobble =
            -sin(
                time *
                DRAWING::LIQUID_WOBBLE_TIME_SCALE
            ) * DRAWING::LIQUID_WOBBLE_DELTA_X;
        float ground_texture_dy =
            al_get_bitmap_height(s_ptr->texture_info.bitmap) * 0.8;
            
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
                al_map_rgba_f(
                    s_ptr->texture_info.tint.r * brightness_mult,
                    s_ptr->texture_info.tint.g * brightness_mult,
                    s_ptr->texture_info.tint.b * brightness_mult,
                    liquid_opacity_mult / 2
                );
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        al_draw_prim(
            av, NULL, s_ptr->texture_info.bitmap,
            0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
    }
    
    //Layer 2 - Tint.
    ALLEGRO_COLOR tint_color =
        al_map_rgba_f(
            l_ptr->main_color.r * brightness_mult,
            l_ptr->main_color.g * brightness_mult,
            l_ptr->main_color.b * brightness_mult,
            l_ptr->main_color.a * liquid_opacity_mult
        );
        
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
        0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
    
    //Layers 3 and 4 - Water surface texture.
    for(unsigned char l = 0; l < 2; ++l) {
    
        sprite* anim_sprite = NULL;
        float layer_2_dy = 0;
        float layer_speed[2];
        layer_speed[0] = l_ptr->surface_speed[0];
        layer_speed[1] = l_ptr->surface_speed[1];
        float alpha = l_ptr->surface_alpha * liquid_opacity_mult;
        
        if(l_ptr->anim_instance.get_cur_sprite()) {
            anim_sprite =
                l_ptr->anim_instance.get_cur_sprite();
            if(anim_sprite->bitmap) {
                layer_2_dy =
                    (anim_sprite->file_size.y * 0.5) * anim_sprite->scale.x;
            }
        }
        
        if(!anim_sprite) continue;
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            av[v].u =
                vx +
                (time * layer_speed[l]);
            av[v].v = vy + (layer_2_dy * l);
            av[v].color =
                al_map_rgba(
                    s_ptr->brightness,
                    s_ptr->brightness,
                    s_ptr->brightness,
                    alpha * brightness_mult
                );
            av[v].x *= scale;
            av[v].y *= scale;
            av[v].u /= anim_sprite->scale.x;
            av[v].v /= anim_sprite->scale.x;
        }
        
        al_draw_prim(
            av, NULL, anim_sprite->bitmap,
            0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
    }
    
    delete[] av;
}



/* ----------------------------------------------------------------------------
 * Draws the loading screen for an area (or anything else, really).
 * text:
 *   The main text to show, optional.
 * subtext:
 *   Subtext to show under the main text, optional.
 * opacity:
 *   0 to 1. The background blackness lowers in opacity much faster.
 */
void draw_loading_screen(
    const string &text, const string &subtext, const float opacity
) {
    unsigned char blackness_alpha = 255.0f * std::max(0.0f, opacity * 4 - 3);
    al_draw_filled_rectangle(
        0, 0, game.win_w, game.win_h, al_map_rgba(0, 0, 0, blackness_alpha)
    );
    
    int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
    al_get_separate_blender(
        &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
    );
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
    
    //Set up the bitmap that will hold the text.
    int text_w = 0, text_h = 0;
    if(!text.empty()) {
        if(!game.loading_text_bmp) {
            //No main text buffer? Create it!
            
            get_multiline_text_dimensions(
                game.fonts.area_name, text, &text_w, &text_h
            );
            game.loading_text_bmp =
                al_create_bitmap(text_w, text_h);
                
            //Draw the main text on its bitmap.
            al_set_target_bitmap(game.loading_text_bmp); {
                al_clear_to_color(COLOR_EMPTY);
                draw_text_lines(
                    game.fonts.area_name, COLOR_GOLD,
                    point(), ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_TOP,
                    text
                );
            } al_set_target_backbuffer(game.display);
            
        } else {
            text_w =
                al_get_bitmap_width(game.loading_text_bmp);
            text_h =
                al_get_bitmap_height(game.loading_text_bmp);
        }
        
    }
    
    int subtext_w = 0, subtext_h = 0;
    if(!subtext.empty()) {
    
        if(!game.loading_subtext_bmp) {
            //No subtext buffer? Create it!
            get_multiline_text_dimensions(
                game.fonts.area_name, subtext, &subtext_w, &subtext_h
            );
            game.loading_subtext_bmp =
                al_create_bitmap(subtext_w, subtext_h);
                
            al_set_target_bitmap(game.loading_subtext_bmp); {
                al_clear_to_color(COLOR_EMPTY);
                draw_text_lines(
                    game.fonts.area_name, al_map_rgb(224, 224, 224),
                    point(),
                    ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_TOP,
                    subtext
                );
                
            } al_set_target_backbuffer(game.display);
            
            //We'll be scaling this, so let's update the mipmap.
            game.loading_subtext_bmp =
                recreate_bitmap(game.loading_subtext_bmp);
                
        } else {
            subtext_w = al_get_bitmap_width(game.loading_subtext_bmp);
            subtext_h = al_get_bitmap_height(game.loading_subtext_bmp);
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
            (game.win_h * 0.5 - text_h * 0.5) :
            (game.win_h * 0.5 - DRAWING::LOADING_SCREEN_PADDING * 0.5 - text_h);
        al_draw_tinted_bitmap(
            game.loading_text_bmp, al_map_rgba(255, 255, 255, 255.0 * opacity),
            game.win_w * 0.5 - text_w * 0.5, text_y, 0
        );
        
    }
    
    //Draw the subtext bitmap in its place.
    float subtext_y = game.win_h * 0.5 + DRAWING::LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_tinted_scaled_bitmap(
            game.loading_subtext_bmp,
            al_map_rgba(255, 255, 255, 255.0 * opacity),
            0, 0, subtext_w, subtext_h,
            game.win_w * 0.5 -
            (subtext_w * DRAWING::LOADING_SCREEN_SUBTITLE_SCALE * 0.5),
            subtext_y,
            subtext_w * DRAWING::LOADING_SCREEN_SUBTITLE_SCALE,
            subtext_h * DRAWING::LOADING_SCREEN_SUBTITLE_SCALE,
            0
        );
        
    }
    
    unsigned char reflection_alpha = 128.0 * opacity;
    
    //Now, draw the polygon that will hold the reflection for the text.
    if(!text.empty()) {
    
        ALLEGRO_VERTEX text_vertexes[4];
        float text_reflection_h =
            std::min((int) (DRAWING::LOADING_SCREEN_PADDING * 0.5), text_h);
        //Top-left vertex.
        text_vertexes[0].x = game.win_w * 0.5 - text_w * 0.5;
        text_vertexes[0].y = text_y + text_h;
        text_vertexes[0].z = 0;
        text_vertexes[0].u = 0;
        text_vertexes[0].v = text_h;
        text_vertexes[0].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        text_vertexes[1].x = game.win_w * 0.5 + text_w * 0.5;
        text_vertexes[1].y = text_y + text_h;
        text_vertexes[1].z = 0;
        text_vertexes[1].u = text_w;
        text_vertexes[1].v = text_h;
        text_vertexes[1].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        text_vertexes[2].x = game.win_w * 0.5 + text_w * 0.5;
        text_vertexes[2].y = text_y + text_h + text_reflection_h;
        text_vertexes[2].z = 0;
        text_vertexes[2].u = text_w;
        text_vertexes[2].v = text_h - text_reflection_h;
        text_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        text_vertexes[3].x = game.win_w * 0.5 - text_w * 0.5;
        text_vertexes[3].y = text_y + text_h + text_reflection_h;
        text_vertexes[3].z = 0;
        text_vertexes[3].u = 0;
        text_vertexes[3].v = text_h - text_reflection_h;
        text_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            text_vertexes, NULL, game.loading_text_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //And the polygon for the subtext.
    if(!subtext.empty()) {
    
        ALLEGRO_VERTEX subtext_vertexes[4];
        float subtext_reflection_h =
            std::min(
                (int) (DRAWING::LOADING_SCREEN_PADDING * 0.5),
                (int) (text_h * DRAWING::LOADING_SCREEN_SUBTITLE_SCALE)
            );
        //Top-left vertex.
        subtext_vertexes[0].x =
            game.win_w * 0.5 - subtext_w *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[0].y =
            subtext_y + subtext_h *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[0].z = 0;
        subtext_vertexes[0].u = 0;
        subtext_vertexes[0].v = subtext_h;
        subtext_vertexes[0].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        subtext_vertexes[1].x =
            game.win_w * 0.5 + subtext_w *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[1].y =
            subtext_y + subtext_h *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[1].z = 0;
        subtext_vertexes[1].u = subtext_w;
        subtext_vertexes[1].v = subtext_h;
        subtext_vertexes[1].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        subtext_vertexes[2].x =
            game.win_w * 0.5 + subtext_w *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[2].y =
            subtext_y + subtext_h *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE +
            subtext_reflection_h;
        subtext_vertexes[2].z = 0;
        subtext_vertexes[2].u = subtext_w;
        subtext_vertexes[2].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        subtext_vertexes[3].x =
            game.win_w * 0.5 - subtext_w *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[3].y =
            subtext_y + subtext_h *
            DRAWING::LOADING_SCREEN_SUBTITLE_SCALE +
            subtext_reflection_h;
        subtext_vertexes[3].z = 0;
        subtext_vertexes[3].u = 0;
        subtext_vertexes[3].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            subtext_vertexes, NULL, game.loading_subtext_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //Draw the game's logo to the left of the "Loading..." text.
    if(opacity == 1.0f) {
        point icon_pos(
            game.win_w - 8 -
            al_get_text_width(game.fonts.standard, "Loading...") -
            8 - al_get_font_line_height(game.fonts.standard) * 0.5,
            game.win_h - 8 - al_get_font_line_height(game.fonts.standard) * 0.5
        );
        
        if(
            game.sys_assets.bmp_icon &&
            game.sys_assets.bmp_icon != game.bmp_error
        ) {
            draw_bitmap(
                game.sys_assets.bmp_icon, icon_pos,
                point(-1, al_get_font_line_height(game.fonts.standard)),
                0, al_map_rgba(255, 255, 255, opacity * 255.0)
            );
        }
        
        //Draw the "Loading..." text, if we're not fading.
        al_draw_text(
            game.fonts.standard, al_map_rgb(192, 192, 192),
            game.win_w - 8,
            game.win_h - 8 - al_get_font_line_height(game.fonts.standard),
            ALLEGRO_ALIGN_RIGHT, "Loading..."
        );
    }
    
}


/* ----------------------------------------------------------------------------
 * Draws a mob's shadow.
 * center:
 *   Center of the mob.
 * diameter:
 *   Diameter of the mob.
 * delta_z:
 *   The mob is these many units above the floor directly below it.
 * shadow_stretch:
 *   How much to stretch the shadow by
 *   (used to simulate sun shadow direction casting).
 */
void draw_mob_shadow(
    const point &center, const float diameter,
    const float delta_z, const float shadow_stretch
) {

    if(shadow_stretch <= 0) return;
    
    float shadow_x = 0;
    float shadow_w =
        diameter + (diameter * shadow_stretch * MOB::SHADOW_STRETCH_MULT);
        
    if(game.states.gameplay->day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + diameter * 0.5;
        shadow_x -= shadow_stretch * delta_z * MOB::SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadow_x = -(diameter * 0.5);
        shadow_x += shadow_stretch * delta_z * MOB::SHADOW_Y_MULT;
    }
    
    
    draw_bitmap(
        game.sys_assets.bmp_shadow,
        point(center.x + shadow_x + shadow_w / 2, center.y),
        point(shadow_w, diameter),
        0,
        map_alpha(255 * (1 - shadow_stretch))
    );
}


/* ----------------------------------------------------------------------------
 * Draws the mouse cursor.
 * color:
 *   Color to tint it with.
 */
void draw_mouse_cursor(const ALLEGRO_COLOR &color) {
    al_use_transform(&game.identity_transform);
    
    //Cursor trail.
    if(game.options.draw_cursor_trail) {
        size_t anchor = 0;
        
        for(size_t s = 1; s < game.mouse_cursor.history.size(); ++s) {
            point anchor_diff =
                game.mouse_cursor.history[anchor] -
                game.mouse_cursor.history[s];
            if(
                fabs(anchor_diff.x) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF &&
                fabs(anchor_diff.y) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF
            ) {
                continue;
            }
            
            float start_ratio =
                anchor / (float) game.mouse_cursor.history.size();
            float start_thickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * start_ratio;
            unsigned char start_alpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * start_ratio;
            ALLEGRO_COLOR start_color =
                change_alpha(color, start_alpha);
            point start_p1;
            point start_p2;
            
            float end_ratio =
                s / (float) GAME::CURSOR_TRAIL_SAVE_N_SPOTS;
            float end_thickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * end_ratio;
            unsigned char end_alpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * end_ratio;
            ALLEGRO_COLOR end_color =
                change_alpha(color, end_alpha);
            point end_p1;
            point end_p2;
            
            if(anchor == 0) {
                point cur_to_next =
                    game.mouse_cursor.history[s] -
                    game.mouse_cursor.history[anchor];
                point cur_to_next_normal(-cur_to_next.y, cur_to_next.x);
                cur_to_next_normal = normalize_vector(cur_to_next_normal);
                point spot_offset = cur_to_next_normal * start_thickness / 2.0f;
                start_p1 = game.mouse_cursor.history[anchor] - spot_offset;
                start_p2 = game.mouse_cursor.history[anchor] + spot_offset;
            } else {
                get_miter_points(
                    game.mouse_cursor.history[anchor - 1],
                    game.mouse_cursor.history[anchor],
                    game.mouse_cursor.history[anchor + 1],
                    -start_thickness,
                    &start_p1,
                    &start_p2,
                    30.0f
                );
            }
            
            if(s == game.mouse_cursor.history.size() - 1) {
                point prev_to_cur =
                    game.mouse_cursor.history[s] -
                    game.mouse_cursor.history[anchor];
                point prev_to_cur_normal(-prev_to_cur.y, prev_to_cur.x);
                prev_to_cur_normal = normalize_vector(prev_to_cur_normal);
                point spot_offset = prev_to_cur_normal * start_thickness / 2.0f;
                end_p1 = game.mouse_cursor.history[s] - spot_offset;
                end_p2 = game.mouse_cursor.history[s] + spot_offset;
            } else {
                get_miter_points(
                    game.mouse_cursor.history[s - 1],
                    game.mouse_cursor.history[s],
                    game.mouse_cursor.history[s + 1],
                    -end_thickness,
                    &end_p1,
                    &end_p2,
                    30.0f
                );
            }
            
            ALLEGRO_VERTEX vertexes[4];
            for(unsigned char v = 0; v < 4; ++v) {
                vertexes[v].z = 0.0f;
            }
            
            vertexes[0].x = start_p1.x;
            vertexes[0].y = start_p1.y;
            vertexes[0].color = start_color;
            vertexes[1].x = start_p2.x;
            vertexes[1].y = start_p2.y;
            vertexes[1].color = start_color;
            vertexes[2].x = end_p1.x;
            vertexes[2].y = end_p1.y;
            vertexes[2].color = end_color;
            vertexes[3].x = end_p2.x;
            vertexes[3].y = end_p2.y;
            vertexes[3].color = end_color;
            
            al_draw_prim(
                vertexes, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
            
            anchor = s;
        }
    }
    
    //Mouse cursor graphic.
    draw_bitmap(
        game.sys_assets.bmp_mouse_cursor,
        game.mouse_cursor.s_pos,
        point(
            al_get_bitmap_width(game.sys_assets.bmp_mouse_cursor),
            al_get_bitmap_height(game.sys_assets.bmp_mouse_cursor)
        ),
        -(game.time_passed * game.config.cursor_spin_speed),
        color
    );
}


/* ----------------------------------------------------------------------------
 * Draws an icon representing some control bind.
 * font:
 *   Font to use for the name, if necessary.
 * i:
 *   Info on the player input. If invalid, a "NONE" icon will be used.
 * condensed:
 *   If true, only the icon's fundamental information is presented. If false,
 *   disambiguation information is included too. For instance, keyboard keys
 *   that come in pairs specify whether they are the left or right key,
 *   controller inputs specify what controller number it is, etc.
 * where:
 *   Center of the place to draw at.
 * max_size:
 *   Max width or height. Used to compress it if needed. 0 = unlimited.
 * alpha:
 *   Opacity.
 */
void draw_player_input_icon(
    const ALLEGRO_FONT* const font, const player_input &i,
    const bool condensed, const point &where, const point &max_size,
    const unsigned char alpha
) {
    if(alpha == 0) return;
    
    //Final text color.
    const ALLEGRO_COLOR final_text_color =
        change_alpha(CONTROL_BIND_ICON::BASE_TEXT_COLOR, alpha);
        
    //Start by getting the icon's info for drawing.
    PLAYER_INPUT_ICON_SHAPES shape;
    PLAYER_INPUT_ICON_SPRITES bitmap_sprite;
    string text;
    get_player_input_icon_info(
        i, condensed,
        &shape, &bitmap_sprite, &text
    );
    
    //If it's a bitmap, just draw it and be done with it.
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int icon_size =
            al_get_bitmap_height(game.sys_assets.bmp_player_input_icons);
        ALLEGRO_BITMAP* bmp =
            al_create_sub_bitmap(
                game.sys_assets.bmp_player_input_icons,
                (icon_size + 1) * (int) bitmap_sprite, 0,
                icon_size, icon_size
            );
        draw_bitmap_in_box(bmp, where, max_size, true, 0.0f, map_alpha(alpha));
        al_destroy_bitmap(bmp);
        return;
    }
    
    //The size of the rectangle will depend on the text within.
    int text_ox;
    int text_oy;
    int text_w;
    int text_h;
    al_get_text_dimensions(
        font, text.c_str(),
        &text_ox, &text_oy, &text_w, &text_h
    );
    float total_width =
        std::min(
            (float) (text_w + CONTROL_BIND_ICON::PADDING * 2),
            (max_size.x == 0 ? FLT_MAX : max_size.x)
        );
    float total_height =
        std::min(
            (float) (text_h + CONTROL_BIND_ICON::PADDING * 2),
            (max_size.y == 0 ? FLT_MAX : max_size.y)
        );
    //Force it to always be a square or horizontal rectangle. Never vertical.
    total_width = std::max(total_width, total_height);
    
    //Now, draw the rectangle, either sharp or rounded.
    switch(shape) {
    case PLAYER_INPUT_ICON_SHAPE_RECTANGLE: {
        draw_textured_box(
            where, point(total_width, total_height),
            game.sys_assets.bmp_key_box
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_ROUNDED: {
        draw_textured_box(
            where, point(total_width, total_height),
            game.sys_assets.bmp_button_box
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_BITMAP: {
        break;
    }
    }
    
    //And finally, the text inside.
    draw_compressed_text(
        font,
        final_text_color,
        point(
            where.x,
            where.y
        ),
        ALLEGRO_ALIGN_CENTER,
        TEXT_VALIGN_CENTER,
        point(
            (max_size.x == 0 ? 0 : max_size.x - CONTROL_BIND_ICON::PADDING),
            (max_size.y == 0 ? 0 : max_size.y - CONTROL_BIND_ICON::PADDING)
        ),
        text
    );
}


/* ----------------------------------------------------------------------------
 * Draws a rotated rectangle.
 * center:
 *   Center of the rectangle.
 * dimensions:
 *   Width and height of the rectangle.
 * angle:
 *   Angle the rectangle is facing.
 * color:
 *   Color to use.
 * thickness:
 *   Thickness to use.
 */
void draw_rotated_rectangle(
    const point &center, const point &dimensions,
    const float angle, const ALLEGRO_COLOR &color, const float thickness
) {
    ALLEGRO_TRANSFORM rot_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&rot_transform);
    al_rotate_transform(&rot_transform, angle);
    al_translate_transform(&rot_transform, center.x, center.y);
    al_compose_transform(&rot_transform, &old_transform);
    
    al_use_transform(&rot_transform); {
        al_draw_rectangle(
            -dimensions.x / 2.0, -dimensions.y / 2.0,
            dimensions.x / 2.0, dimensions.y / 2.0,
            color, thickness
        );
    }; al_use_transform(&old_transform);
}


/* ----------------------------------------------------------------------------
 * Draws a rounded rectangle.
 * This is basically Allegro's function, but safer and simpler.
 * center:
 *   Center.
 * size:
 *   Width and height.
 * radii:
 *   Radii of the corners. Will be smaller if the rectangle is too small.
 * color:
 *   Color the diamond with this color.
 * thickness:
 *   Line thickness.
 */
void draw_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color, const float thickness
) {
    float final_radii = std::min(radii, size.x / 2.0f);
    final_radii = std::min(final_radii, size.y / 2.0f);
    final_radii = std::max(0.0f, final_radii);
    al_draw_rounded_rectangle(
        center.x - size.x / 2.0f, center.y - size.y / 2.0f,
        center.x + size.x / 2.0f, center.y + size.y / 2.0f,
        final_radii, final_radii,
        color, thickness
    );
}


/* ----------------------------------------------------------------------------
 * Draws text, scaled.
 * font:
 *   Font to use.
 * color:
 *   Tint the text with this color.
 * where:
 *   Coordinates to draw in.
 * scale:
 *   Horizontal or vertical scale.
 * flags:
 *   Same flags you'd use for al_draw_text.
 * valign:
 *   Vertical alignment.
 * text:
 *   Text to draw.
 */
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODES valign, const string &text
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
 * Draws a sector, but only the texture (no wall shadows).
 * s_ptr:
 *   Pointer to the sector.
 * where:
 *   X and Y offset.
 * scale:
 *   Scale the sector by this much.
 * opacity:
 *   Draw the textures at this opacity, 0 - 1.
 */
void draw_sector_texture(
    sector* s_ptr, const point &where, const float scale, const float opacity
) {
    if(!s_ptr) return;
    if(s_ptr->is_bottomless_pit) return;
    
    unsigned char n_textures = 1;
    sector* texture_sector[2] = {NULL, NULL};
    
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
        
        if(!texture_sector[t] || texture_sector[t]->is_bottomless_pit) {
            continue;
        }
        
        size_t n_vertexes = s_ptr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
        
        sector_texture_info* texture_info_to_use =
            &texture_sector[t]->texture_info;
            
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        if(texture_sector[t]) {
            al_build_transform(
                &tra,
                -texture_info_to_use->translation.x,
                -texture_info_to_use->translation.y,
                1.0f / texture_info_to_use->scale.x,
                1.0f / texture_info_to_use->scale.y,
                -texture_info_to_use->rot
            );
        }
        
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
                    texture_sector[t]->texture_info.tint.r * brightness_mult,
                    texture_sector[t]->texture_info.tint.g * brightness_mult,
                    texture_sector[t]->texture_info.tint.b * brightness_mult,
                    texture_sector[t]->texture_info.tint.a * alpha_mult *
                    opacity
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
            
        al_draw_prim(
            av, NULL, tex,
            0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
}


/* ----------------------------------------------------------------------------
 * Draws a status effect's bitmap.
 * m:
 *   Mob that has this status effect.
 * effects:
 *   List of bitmap effects to use.
 */
void draw_status_effect_bmp(mob* m, bitmap_effect_info &effects) {
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = m->get_status_bitmap(&status_bmp_scale);
    
    if(!status_bmp) return;
    
    draw_bitmap(
        status_bmp,
        m->pos,
        point(m->radius * 2 * status_bmp_scale, -1)
    );
}


/* ----------------------------------------------------------------------------
 * Draws string tokens.
 * tokens:
 *   Vector of tokens to draw.
 * text_font:
 *   Text font.
 * control_font:
 *   Font for control bind icons.
 * controls_condensed:
 *   Whether control binds should be condensed.
 * where:
 *   Top-left coordinates to draw at.
 * flags:
 *   Allegro text flags.
 * max_size:
 *   Maximum width and height of the whole thing.
 * scale:
 *   Scale each token by this amount.
 */
void draw_string_tokens(
    const vector<string_token> &tokens, const ALLEGRO_FONT* const text_font,
    const ALLEGRO_FONT* const control_font, bool controls_condensed,
    const point &where, const int flags, const point &max_size,
    const point &scale
) {
    unsigned int total_width = 0;
    float x_scale = 1.0f;
    for(size_t t = 0; t < tokens.size(); ++t) {
        total_width += tokens[t].width;
    }
    if(total_width > max_size.x) {
        x_scale = max_size.x / total_width;
    }
    float y_scale = 1.0f;
    unsigned int line_height = al_get_font_line_height(text_font);
    if(line_height > max_size.y) {
        y_scale = max_size.y / line_height;
    }
    
    float start_x = where.x;
    if(has_flag(flags, ALLEGRO_ALIGN_CENTER)) {
        start_x -= (total_width * x_scale) / 2.0f;
    } else if(has_flag(flags, ALLEGRO_ALIGN_RIGHT)) {
        start_x -= total_width * x_scale;
    }
    
    float caret = start_x;
    for(size_t t = 0; t < tokens.size(); ++t) {
        float token_final_width = tokens[t].width * x_scale;
        switch(tokens[t].type) {
        case STRING_TOKEN_CHAR: {
            draw_scaled_text(
                text_font, COLOR_WHITE,
                point(caret, where.y),
                point(x_scale * scale.x, y_scale * scale.y),
                ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_TOP,
                tokens[t].content
            );
            break;
        }
        case STRING_TOKEN_CONTROL_BIND: {
            draw_player_input_icon(
                control_font,
                game.controls.find_bind(tokens[t].content).input,
                controls_condensed,
                point(
                    caret + token_final_width / 2.0f,
                    where.y + max_size.y / 2.0f
                ),
                point(token_final_width * scale.x, max_size.y * scale.y)
            );
            break;
        }
        default: {
            break;
        }
        }
        caret += token_final_width;
    }
}


/* ----------------------------------------------------------------------------
 * Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 * font:
 *   Font to use.
 * color:
 *   Color.
 * where:
 *   Coordinates of the text.
 * flags:
 *   Flags, just like the ones you'd pass to al_draw_text.
 * valign:
 *   Vertical alignment.
 * text:
 *   Text to write, line breaks included ('\n').
 */
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags,
    const TEXT_VALIGN_MODES valign, const string &text
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    float top;
    
    if(valign == TEXT_VALIGN_TOP) {
        top = where.y;
    } else {
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = (int) n_lines * fh + (int) (n_lines - 1);
        if(valign == TEXT_VALIGN_CENTER) {
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
 * Draws a box, using a texture. The texture is split into three-by-three.
 * The corners of the box will use the corners of the texture as they are.
 * The remaining sections of the texture will be stretched to fill the
 * box's center and sides.
 * If the box's width or height is smaller than the two relevant corners
 * combined, then the corner graphics will be shrunk down, though.
 * center:
 *   Center of the box.
 * size:
 *   Width and height of the box.
 * texture:
 *   Texture to use.
 * tint:
 *   Tint the texture with this color.
 */
void draw_textured_box(
    const point &center, const point &size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR &tint
) {
    //While using al_hold_bitmap_drawing is an optimization, we can't use it
    //since it stops using the transformation, meaning any textured boxes
    //meant to be drawn with transformations wouldn't.
    
    //Some caches.
    //Vertex total. 9 sections * 2 tris * 3 vertexes.
    constexpr size_t total_vertexes = 9 * 2 * 3;
    //Top-left coordinates.
    const point tl = center - size / 2.0f;
    //Bitmap size.
    const int bmp_w = al_get_bitmap_width(texture);
    const int bmp_h = al_get_bitmap_height(texture);
    //Minimum size at which the corner graphics are drawn in full.
    //Workaround: For some reason there's a seam visible when the edges are
    //around < 6 pixels wide. I can't figure out why. So I'm bumping
    //this threshold to be 8 pixels longer than normal.
    const point corner_treshold(
        std::max(8.0f, size.x / 2.0f - 8),
        std::max(8.0f, size.y / 2.0f - 8)
    );
    //Corner size.
    point corner_size(bmp_w / 3.0f, bmp_h / 3.0f);
    if(corner_treshold.x < corner_size.x) {
        corner_size.x = corner_treshold.x;
        corner_size.y = corner_size.x * (bmp_w / bmp_h);
    }
    if(corner_treshold.y < corner_size.y) {
        corner_size.y = corner_treshold.y;
        corner_size.x = corner_size.y * (bmp_h / bmp_w);
    }
    
    //Initialize the vertexes.
    ALLEGRO_VERTEX vert[total_vertexes];
    for(unsigned char v = 0; v < total_vertexes; ++v) {
        vert[v].color = tint;
        vert[v].z = 0;
    }
    
    size_t v = 0;
    for(size_t r = 0; r < 3; ++r) {
        //For every row.
        
        //Figure out the start and end Y drawing coordinates.
        float y1 = 0.0f;
        float y2 = 0.0f;
        switch(r) {
        case 0: {
            y1 = tl.y;
            y2 = tl.y + corner_size.y;
            break;
        } case 1: {
            y1 = tl.y + corner_size.y;
            y2 = tl.y + size.y - corner_size.y;
            break;
        } case 2: {
            y1 = tl.y + size.y - corner_size.y;
            y2 = tl.y + size.y;
            break;
        }
        }
        
        //And the start and end Y texture coordinates.
        float v1 = (bmp_h / 3.0f) * r;
        float v2 = (bmp_h / 3.0f) * (r + 1);
        
        for(size_t c = 0; c < 3; ++c) {
            //For every column.
            
            //Figure out the start and end X drawing coordinates.
            float x1 = 0.0f;
            float x2 = 0.0f;
            switch(c) {
            case 0: {
                x1 = tl.x;
                x2 = tl.x + corner_size.x;
                break;
            } case 1: {
                x1 = tl.x + corner_size.x;
                x2 = tl.x + size.x - corner_size.x;
                break;
            } case 2: {
                x1 = tl.x + size.x - corner_size.x;
                x2 = tl.x + size.x;
                break;
            }
            }
            
            //And the start and end X texture coordinates.
            float u1 = (bmp_w / 3.0f) * c;
            float u2 = (bmp_w / 3.0f) * (c + 1);
            
            //Finally, fill the vertex info!
            //First triangle (top-left).
            vert[v + 0].x = x1;
            vert[v + 0].u = u1;
            vert[v + 0].y = y1;
            vert[v + 0].v = v1;
            vert[v + 1].x = x2;
            vert[v + 1].u = u2;
            vert[v + 1].y = y1;
            vert[v + 1].v = v1;
            vert[v + 2].x = x1;
            vert[v + 2].u = u1;
            vert[v + 2].y = y2;
            vert[v + 2].v = v2;
            
            //Second triangle (bottom-right).
            vert[v + 3].x = x2;
            vert[v + 3].u = u2;
            vert[v + 3].y = y1;
            vert[v + 3].v = v1;
            vert[v + 4].x = x1;
            vert[v + 4].u = u1;
            vert[v + 4].y = y2;
            vert[v + 4].v = v2;
            vert[v + 5].x = x2;
            vert[v + 5].u = u2;
            vert[v + 5].y = y2;
            vert[v + 5].v = v2;
            
            v += 6;
        }
    }
    
    al_draw_prim(
        vert, NULL, texture, 0, total_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
}


/* ----------------------------------------------------------------------------
 * Returns information about how a control bind icon should be drawn.
 * i:
 *   Info on the player input. If invalid, a "NONE" icon will be used.
 * condensed:
 *   If true, only the icon's fundamental information is presented. If false,
 *   disambiguation information is included too. For instance, keyboard keys
 *   that come in pairs specify whether they are the left or right key,
 *   controller inputs specify what controller number it is, etc.
 * shape:
 *   The shape is returned here.
 * bitmap_sprite:
 *   If it's one of the icons in the control bind icon spritesheet, the index
 *   of the sprite is returned here.
 * text:
 *   The text to be written inside is returned here, or an empty string is
 *   returned if there's nothing to write.
 */
void get_player_input_icon_info(
    const player_input &i, const bool condensed,
    PLAYER_INPUT_ICON_SHAPES* shape,
    PLAYER_INPUT_ICON_SPRITES* bitmap_sprite,
    string* text
) {
    *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
    *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LMB;
    *text = "(NONE)";
    
    if(i.type == INPUT_TYPE_NONE) return;
    
    //Figure out if it's one of those that has a bitmap icon.
    //If so, just return that.
    if(i.type == INPUT_TYPE_MOUSE_BUTTON) {
        if(i.button_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LMB;
            return;
        } else if(i.button_nr == 2) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_RMB;
            return;
        } else if(i.button_nr == 3) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MMB;
            return;
        }
    } else if(i.type == INPUT_TYPE_MOUSE_WHEEL_UP) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MWU;
        return;
    } else if(i.type == INPUT_TYPE_MOUSE_WHEEL_DOWN) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MWD;
        return;
    } else if(i.type == INPUT_TYPE_KEYBOARD_KEY) {
        if(i.button_nr == ALLEGRO_KEY_UP) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_UP;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_LEFT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LEFT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_DOWN) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_DOWN;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_RIGHT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_RIGHT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_BACKSPACE) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_BACKSPACE;
            return;
        } else if(
            condensed &&
            (
                i.button_nr == ALLEGRO_KEY_LSHIFT ||
                i.button_nr == ALLEGRO_KEY_RSHIFT
            )
        ) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_SHIFT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_TAB) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_TAB;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_ENTER) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_ENTER;
            return;
        }
    } else if(i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG && condensed) {
        if(i.axis_nr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_LEFT;
            return;
        } else if(i.axis_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_UP;
            return;
        }
    } else if(i.type == INPUT_TYPE_CONTROLLER_AXIS_POS && condensed) {
        if(i.axis_nr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_RIGHT;
            return;
        } else if(i.axis_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_DOWN;
            return;
        }
    }
    
    //Otherwise, use an actual shape and some text inside.
    switch(i.type) {
    case INPUT_TYPE_KEYBOARD_KEY: {
        *shape = PLAYER_INPUT_ICON_SHAPE_RECTANGLE;
        *text = get_key_name(i.button_nr, condensed);
        break;
        
    } case INPUT_TYPE_CONTROLLER_AXIS_NEG:
    case INPUT_TYPE_CONTROLLER_AXIS_POS: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(i.device_nr + 1) +
                " stick " + i2s(i.stick_nr + 1);
            if(
                i.axis_nr == 0 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " left";
            } else if(
                i.axis_nr == 0 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " right";
            } else if(
                i.axis_nr == 1 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " up";
            } else if(
                i.axis_nr == 1 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " down";
            } else {
                *text +=
                    " axis " + i2s(i.axis_nr) +
                    (
                        i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG ?
                        "-" :
                        "+"
                    );
            }
            
        } else {
            *text = "Stick " + i2s(i.stick_nr);
        }
        break;
        
    } case INPUT_TYPE_CONTROLLER_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(i.device_nr + 1) +
                " button " + i2s(i.button_nr + 1);
        } else {
            *text = i2s(i.button_nr + 1);
        }
        break;
        
    } case INPUT_TYPE_MOUSE_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse button " + i2s(i.button_nr);
        } else {
            *text = "M" + i2s(i.button_nr);
        }
        break;
        
    } case INPUT_TYPE_MOUSE_WHEEL_LEFT: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse wheel left";
        } else {
            *text = "MWL";
        }
        break;
        
    } case INPUT_TYPE_MOUSE_WHEEL_RIGHT: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse wheel right";
        } else {
            *text = "MWR";
        }
        break;
        
    } default: {
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the width of a control bind icon, for drawing purposes.
 * font:
 *   Font to use for the name, if necessary.
 * i:
 *   Info on the player input. If invalid, a "NONE" icon will be used.
 * condensed:
 *   If true, only the icon's fundamental information is presented. If false,
 *   disambiguation information is included too. For instance, keyboard keys
 *   that come in pairs specify whether they are the left or right key,
 *   controller inputs specify what controller number it is, etc.
 * max_bitmap_height:
 *   If bitmap icons need to be condensed vertically to fit a certain space,
 *   then their width will be affected too. Specify the maximum height here.
 *   Use 0 to indicate no maximum height.
 */
float get_player_input_icon_width(
    const ALLEGRO_FONT* font, const player_input &i, const bool condensed,
    const float max_bitmap_height
) {
    PLAYER_INPUT_ICON_SHAPES shape;
    PLAYER_INPUT_ICON_SPRITES bitmap_sprite;
    string text;
    get_player_input_icon_info(
        i, condensed,
        &shape, &bitmap_sprite, &text
    );
    
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int bmp_height =
            al_get_bitmap_height(game.sys_assets.bmp_player_input_icons);
        if(max_bitmap_height == 0.0f || bmp_height < max_bitmap_height) {
            return bmp_height;
        } else {
            return max_bitmap_height;
        }
    } else {
        return
            al_get_text_width(font, text.c_str()) +
            CONTROL_BIND_ICON::PADDING * 2;
    }
}
