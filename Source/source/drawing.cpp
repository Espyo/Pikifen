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

#include "game_states/gameplay/gameplay.h"
#include "animation.h"
#include "const.h"
#include "functions.h"
#include "game.h"
#include "mobs/group_task.h"
#include "mobs/pile.h"
#include "mobs/scale.h"
#include "utils/geometry_utils.h"
#include "utils/string_utils.h"


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
 * angle:
 *   Angle to rotate the bitmap by.
 *   The box does not take angling into account.
 * tint:
 *   Tint the bitmap with this color.
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
        ALLEGRO_ALIGN_CENTER, 1, size,
        text
    );
    
    ALLEGRO_COLOR box_tint =
        selected ? al_map_rgb(87, 200, 208) : map_gray(255);
        
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
 *   Vertical align: 0 = top, 1 = middle, 2 = bottom.
 * max_size:
 *   The maximum width and height. Use <= 0 to have no limit.
 * text:
 *   Text to draw.
 */
void draw_compressed_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const unsigned char valign,
    const point &max_size, const string &text
) {

    if(max_size.x == 0 && max_size.y == 0) return;
    
    int x1, x2, y1, y2;
    al_get_text_dimensions(font, text.c_str(), &x1, &y1, &x2, &y2);
    
    int text_width = (x2 - x1) * scale.x;
    int text_height = (y2 - y1) * scale.y;
    point final_scale = scale;
    float final_text_height = text_height;
    
    if(text_width > max_size.x && max_size.x > 0) {
        final_scale.x = max_size.x / text_width;
    }
    if(text_height > max_size.y && max_size.y > 0) {
        final_scale.y = max_size.y / text_height;
        final_text_height = max_size.y;
    }
    
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, final_scale.x, final_scale.y);
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
 *   Vertical align: 0 = top, 1 = middle, 2 = bottom.
 * max_size:
 *   The maximum width and height. Use <= 0 to have no limit.
 * text:
 *   Text to draw.
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
 * Draws a key or button on the screen.
 * font:
 *   Font to use for the name.
 * c:
 *   Info on the control.
 * where:
 *   Center of the place to draw at.
 * max_size:
 *   Max width or height. Used to compress it if needed. 0 = unlimited.
 */
void draw_control(
    const ALLEGRO_FONT* const font, const control_info &c,
    const point &where, const point &max_size
) {

    if(c.type == CONTROL_TYPE_MOUSE_BUTTON) {
        //If it's a mouse click, just draw the icon and be done with it.
        if(c.button >= 1 && c.button <= 3) {
        
            draw_bitmap_in_box(
                game.sys_assets.bmp_mouse_button_icon[c.button - 1],
                where, max_size
            );
            return;
            
        }
    }
    
    if(
        c.type == CONTROL_TYPE_MOUSE_WHEEL_UP ||
        c.type == CONTROL_TYPE_MOUSE_WHEEL_DOWN
    ) {
        //Likewise, if it's a mouse wheel move, just draw the icon and leave.
        ALLEGRO_BITMAP* b = game.sys_assets.bmp_mouse_wu_icon;
        if(c.type == CONTROL_TYPE_MOUSE_WHEEL_DOWN) {
            b = game.sys_assets.bmp_mouse_wd_icon;
        }
        
        draw_bitmap_in_box(b, where, max_size);
        return;
    }
    
    string name;
    switch(c.type) {
    case CONTROL_TYPE_KEYBOARD_KEY: {
        name = str_to_upper(al_keycode_to_name(c.button));
        break;
    } case CONTROL_TYPE_JOYSTICK_AXIS_NEG:
    case CONTROL_TYPE_JOYSTICK_AXIS_POS: {
        name = "AXIS " + i2s(c.stick) + " " + i2s(c.axis);
        name += c.type == CONTROL_TYPE_JOYSTICK_AXIS_NEG ? "-" : "+";
        break;
    } case CONTROL_TYPE_JOYSTICK_BUTTON: {
        name = i2s(c.button + 1);
        break;
    } case CONTROL_TYPE_MOUSE_BUTTON: {
        name = "M" + i2s(c.button);
        break;
    } case CONTROL_TYPE_MOUSE_WHEEL_LEFT: {
        name = "MWL";
        break;
    } case CONTROL_TYPE_MOUSE_WHEEL_RIGHT: {
        name = "MWR";
        break;
    }
    }
    
    int x1, y1, x2, y2;
    al_get_text_dimensions(font, name.c_str(), &x1, &y1, &x2, &y2);
    float total_width =
        std::min(
            (float) (x2 - x1 + 4),
            (max_size.x == 0 ? FLT_MAX : max_size.x)
        );
    float total_height =
        std::min(
            (float) (y2 - y1 + 4),
            (max_size.y == 0 ? FLT_MAX : max_size.y)
        );
    total_width = std::max(total_width, total_height);
    
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
            std::min(16.0, total_width * 0.5),
            std::min(16.0, total_height * 0.5),
            map_alpha(192)
        );
        al_draw_rounded_rectangle(
            where.x - total_width * 0.5, where.y - total_height * 0.5,
            where.x + total_width * 0.5, where.y + total_height * 0.5,
            std::min(16.0, total_width * 0.5),
            std::min(16.0, total_height * 0.5),
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
 * Draws a strength/weight fraction, in the style of Pikmin 2.
 * The strength is above the weight.
 * center:
 *   Center of the text.
 * current:
 *   Current strength.
 * needed:
 *   Needed strength to lift the object (weight).
 * color:
 *   Color of the fraction's text.
 */
void draw_fraction(
    const point &center, const size_t current,
    const size_t needed, const ALLEGRO_COLOR &color
) {
    float first_y =
        center.y - (al_get_font_line_height(game.fonts.standard) * 3) / 2;
    float font_h = al_get_font_line_height(game.fonts.value);
    
    draw_scaled_text(
        game.fonts.value, color, point(center.x, first_y),
        point(
            (current >= needed ? 1.2 : 1.0),
            (current >= needed ? 1.2 : 1.0)
        ),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(current).c_str())
    );
    
    al_draw_text(
        game.fonts.value, color, center.x, first_y + font_h * 0.75,
        ALLEGRO_ALIGN_CENTER, "-"
    );
    
    draw_scaled_text(
        game.fonts.value, color, point(center.x, first_y + font_h * 1.5),
        point(
            (needed > current ? 1.2 : 1.0),
            (needed > current ? 1.2 : 1.0)
        ),
        ALLEGRO_ALIGN_CENTER, 0, (i2s(needed).c_str())
    );
}


/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * center:
 *   Center of the wheel.
 * ratio:
 *   Ratio of health that is filled. 0 is empty, 1 is full.
 * alpha:
 *   Total opacity of the healt wheel.
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
 */
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, const float scale
) {

    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        av[v].z = 0;
    }
    
    float liquid_opacity_mult = 1.0f;
    if(s_ptr->draining_liquid) {
        liquid_opacity_mult = s_ptr->liquid_drain_left / LIQUID_DRAIN_DURATION;
    }
    
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
                game.states.gameplay->area_time_passed *
                LIQUID_WOBBLE_TIME_SCALE
            ) * LIQUID_WOBBLE_DELTA_X;
        float ground_texture_dy =
            al_get_bitmap_height(s_ptr->texture_info.bitmap) * 0.5;
            
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
                    128 * liquid_opacity_mult
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
    ALLEGRO_COLOR tint_color = l_ptr->main_color;
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
        av[v].color.a *= liquid_opacity_mult;
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
        
        for(size_t v = 0; v < n_vertexes; ++v) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            av[v].u =
                vx +
                (game.states.gameplay->area_time_passed * layer_speed[l]);
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

    const float LOADING_SCREEN_SUBTITLE_SCALE = 0.6f;
    const int LOADING_SCREEN_PADDING = 64;
    
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
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                draw_text_lines(
                    game.fonts.area_name, al_map_rgb(255, 215, 0),
                    point(), ALLEGRO_ALIGN_LEFT, 0,
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
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                draw_text_lines(
                    game.fonts.area_name, al_map_rgb(224, 224, 224),
                    point(),
                    ALLEGRO_ALIGN_LEFT, 0,
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
            (game.win_h * 0.5 - LOADING_SCREEN_PADDING * 0.5 - text_h);
        al_draw_tinted_bitmap(
            game.loading_text_bmp, al_map_rgba(255, 255, 255, 255.0 * opacity),
            game.win_w * 0.5 - text_w * 0.5, text_y, 0
        );
        
    }
    
    //Draw the subtext bitmap in its place.
    float subtext_y = game.win_h * 0.5 + LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_tinted_scaled_bitmap(
            game.loading_subtext_bmp,
            al_map_rgba(255, 255, 255, 255.0 * opacity),
            0, 0, subtext_w, subtext_h,
            game.win_w * 0.5 -
            (subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5),
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
            std::min((int) (LOADING_SCREEN_PADDING * 0.5), text_h);
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
                (int) (LOADING_SCREEN_PADDING * 0.5),
                (int) (text_h * LOADING_SCREEN_SUBTITLE_SCALE)
            );
        //Top-left vertex.
        subtext_vertexes[0].x =
            game.win_w * 0.5 - subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[0].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[0].z = 0;
        subtext_vertexes[0].u = 0;
        subtext_vertexes[0].v = subtext_h;
        subtext_vertexes[0].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        subtext_vertexes[1].x =
            game.win_w * 0.5 + subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[1].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE;
        subtext_vertexes[1].z = 0;
        subtext_vertexes[1].u = subtext_w;
        subtext_vertexes[1].v = subtext_h;
        subtext_vertexes[1].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        subtext_vertexes[2].x =
            game.win_w * 0.5 + subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[2].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE +
            subtext_reflection_h;
        subtext_vertexes[2].z = 0;
        subtext_vertexes[2].u = subtext_w;
        subtext_vertexes[2].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        subtext_vertexes[3].x =
            game.win_w * 0.5 - subtext_w * LOADING_SCREEN_SUBTITLE_SCALE * 0.5;
        subtext_vertexes[3].y =
            subtext_y + subtext_h * LOADING_SCREEN_SUBTITLE_SCALE +
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
            game.win_w - 8 - al_get_text_width(game.fonts.standard, "Loading...") -
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
        diameter + (diameter * shadow_stretch * MOB_SHADOW_STRETCH_MULT);
        
    if(game.states.gameplay->day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + diameter * 0.5;
        shadow_x -= shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadow_x = -(diameter * 0.5);
        shadow_x += shadow_stretch * delta_z * MOB_SHADOW_Y_MULT;
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
 * Draws a notification, like a note saying that the player can press
 * a certain button to pluck.
 * target:
 *   Spot that the notification is pointing at.
 * text:
 *   Text to say.
 * control:
 *   If not NULL, draw the control's button/key/etc. before the text.
 */
void draw_notification(
    const point &target, const string &text, control_info* control
) {

    ALLEGRO_TRANSFORM tra, old;
    al_identity_transform(&tra);
    al_translate_transform(
        &tra, target.x * game.cam.zoom, target.y * game.cam.zoom
    );
    al_scale_transform(&tra, 1.0f / game.cam.zoom, 1.0f / game.cam.zoom);
    al_copy_transform(&old, al_get_current_transform());
    al_compose_transform(&tra, &old);
    al_use_transform(&tra);
    
    int bmp_w = al_get_bitmap_width(game.sys_assets.bmp_notification);
    int bmp_h = al_get_bitmap_height(game.sys_assets.bmp_notification);
    
    float text_box_x1 = -bmp_w * 0.5 + NOTIFICATION_PADDING;
    float text_box_x2 = bmp_w * 0.5 - NOTIFICATION_PADDING;
    float text_box_y1 = -bmp_h - NOTIFICATION_PADDING;
    float text_box_y2 = NOTIFICATION_PADDING;
    
    draw_bitmap(
        game.sys_assets.bmp_notification,
        point(0, -bmp_h * 0.5),
        point(bmp_w, bmp_h),
        0,
        map_alpha(NOTIFICATION_ALPHA)
    );
    
    if(control) {
        text_box_x1 += NOTIFICATION_CONTROL_SIZE + NOTIFICATION_PADDING;
        draw_control(
            game.fonts.standard, *control,
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
        game.fonts.standard, map_alpha(NOTIFICATION_ALPHA),
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
 *   Vertical align. 0: top, 1: center, 2: bottom.
 * text:
 *   Text to draw.
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
 * s_ptr:
 *   The sector to draw.
 * where:
 *   Top-left coordinates.
 * scale:
 *   Drawing scale.
 */
void draw_sector_shadows(sector* s_ptr, const point &where, const float scale) {
    if(s_ptr->is_bottomless_pit) return;
    
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edge* e_ptr = s_ptr->edges[e];
        ALLEGRO_VERTEX av[4];
        
        sector* other_sector = e_ptr->get_other_sector(s_ptr);
        
        if(!casts_shadow(other_sector, s_ptr)) continue;
        
        float shadow_length =
            get_wall_shadow_length(other_sector->z - s_ptr->z);
            
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
        float e_cos_front = cos(e_angle - TAU / 4);
        float e_sin_front = sin(e_angle - TAU / 4);
        
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
        float neighbor_angles[2] = {TAU / 4, TAU / 4};
        //Difference between angle of current edge and neighbors.
        float neighbor_angle_difs[2] = {0, 0};
        //Midway angle.
        float mid_angles[2] = {TAU / 4, TAU / 4};
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
                else d = get_angle_cw_dif(e_angle + TAU / 2, ve_angle);
                
                if(
                    d < neighbor_angle_difs[v] ||
                    !got_first[v]
                ) {
                    //Save this as the next edge.
                    neighbor_angles[v] = ve_angle;
                    neighbor_angle_difs[v] = d;
                    got_first[v] = true;
                    
                    sector* edge_other_sec = ve_ptr->get_other_sector(s_ptr);
                    neighbor_shadow[v] =
                        casts_shadow(edge_other_sec, s_ptr);
                        
                    //Get the shadow length.
                    //Defaulting to the current sector's length
                    //makes it easier to calculate things later on.
                    neighbor_shadow_length[v] =
                        neighbor_shadow[v] ?
                        get_wall_shadow_length(edge_other_sec->z - s_ptr->z) :
                        shadow_length;
                }
            }
        }
        
        e_angle = normalize_angle(e_angle);
        for(unsigned char n = 0; n < 2; ++n) {
            neighbor_angles[n] = normalize_angle(neighbor_angles[n]);
            mid_angles[n] =
                (n == 0 ? neighbor_angles[n] : e_angle + TAU / 2) +
                neighbor_angle_difs[n] / 2;
        }
        
        point shadow_point[2];
        ALLEGRO_VERTEX extra_av[8];
        for(unsigned char eav = 0; eav < 8; ++eav) {
            extra_av[eav].z = 0;
        }
        //How many vertexes of the extra polygon to draw.
        unsigned char draw_extra[2] = {0, 0};
        
        for(unsigned char v = 0; v < 2; ++v) {
        
            if(neighbor_angle_difs[v] < TAU / 2 && neighbor_shadow[v]) {
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
                
                if(neighbor_angle_difs[v] > TAU / 4) {
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
                    
                    if(neighbor_angle_difs[v] > TAU / 2) {
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
                    texture_sector[t]->texture_info.tint.r * brightness_mult,
                    texture_sector[t]->texture_info.tint.g * brightness_mult,
                    texture_sector[t]->texture_info.tint.b * brightness_mult,
                    texture_sector[t]->texture_info.tint.a * alpha_mult *
                    opacity
                );
        }
        
        al_hold_bitmap_drawing(false);
        
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
            0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
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
        point(m->type->radius * 2 * status_bmp_scale, -1)
    );
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
 *   Vertical align: 0 for top, 1 for center, 2 for bottom.
 * text:
 *   Text to write, line breaks included ('\n').
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
    al_hold_bitmap_drawing(true);
    
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
        float y1;
        float y2;
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
            float x1;
            float x2;
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
    
    al_hold_bitmap_drawing(false);
}


/* ----------------------------------------------------------------------------
 * Eases a number [0, 1] in accordance to a non-linear interpolation
 * method. Normally used for camera movement and such.
 * method:
 *   The method to use. Use EASE_*.
 * n:
 *   The number to ease, in the range [0, 1].
 */
float ease(const unsigned char method, const float n) {
    switch(method) {
    case EASE_IN:
        return pow(n, 3);
    case EASE_OUT:
        return 1 - (pow((1 - n), 3));
    case EASE_UP_AND_DOWN:
        return sin(n * TAU / 2);
    }
    
    return n;
}
