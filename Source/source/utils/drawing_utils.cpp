/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drawing-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <allegro5/allegro_primitives.h>

#include "drawing_utils.h"

#include "math_utils.h"
#include "string_utils.h"


/**
 * @brief Draws a bitmap.
 *
 * @param bmp The bitmap.
 * @param center Center coordinates.
 * @param size Final width and height.
 * Make this -1 on one of them to keep the aspect ratio from the other.
 * @param angle Angle to rotate the bitmap by.
 * @param tint Tint the bitmap with this color.
 */
void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle, const ALLEGRO_COLOR &tint
) {

    if(size.x == 0 && size.y == 0) return;
    
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


/**
 * @brief Draws a bitmap, but keeps its aspect ratio,
 * and scales it to fit in an imaginary box.
 *
 * @param bmp The bitmap.
 * @param center Center coordinates.
 * @param box_size Width and height of the box.
 * @param scale_up If true, the bitmap is scaled up to fit the box.
 * If false, it stays at its original size (unless it needs to be scaled down).
 * @param angle Angle to rotate the bitmap by.
 * The box does not take angling into account.
 * @param tint Tint the bitmap with this color.
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


/**
 * @brief Draws text, scaled, but also compresses (scales) it
 * to fit within the specified range.
 *
 * @param font Font to use.
 * @param color Tint the text by this color.
 * @param where Coordinates to draw it at.
 * @param scale Scale to use.
 * @param flags Allegro text render function flags.
 * @param valign Vertical alignment.
 * @param max_size The maximum width and height. Use <= 0 to have no limit.
 * @param scale_past_max If true, the max size will only be taken into
 * account when the scale is 1. If it is any bigger, it will overflow
 * past the max size.
 * @param text Text to draw.
 */
void draw_compressed_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODE valign,
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
        valign == TEXT_VALIGN_MODE_CENTER ?
        final_text_height / 2.0f :
        valign == TEXT_VALIGN_MODE_BOTTOM ?
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


/**
 * @brief Draws text on the screen, but compresses (scales) it
 * to fit within the specified range.
 *
 * @param font Font to use.
 * @param color Tint the text by this color.
 * @param where Coordinates to draw it at.
 * @param flags Allegro text render function flags.
 * @param valign Vertical alignment.
 * @param max_size The maximum width and height. Use <= 0 to have no limit.
 * @param text Text to draw.
 */
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const TEXT_VALIGN_MODE valign,
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
        valign == TEXT_VALIGN_MODE_CENTER ?
        final_text_height / 2.0f :
        valign == TEXT_VALIGN_MODE_BOTTOM ?
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


/**
 * @brief Draws an equilateral triangle made of three lines.
 *
 * @param center Center point of the triangle.
 * @param radius Radius between the center and each vertex.
 * @param angle Angle at which its first vertex points.
 * @param color Its color.
 * @param thickness Thickness of the lines.
 */
void draw_equilateral_triangle(
    const point &center, float radius, float angle,
    const ALLEGRO_COLOR &color, float thickness
) {
    point v1 = center + rotate_point(point(radius, 0.0f), angle);
    point v2 = center + rotate_point(point(radius, 0.0f), angle + TAU / 3.0f);
    point v3 = center + rotate_point(point(radius, 0.0f), angle - TAU / 3.0f);
    al_draw_triangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, color, thickness);
}


/**
 * @brief Draws a filled diamond shape.
 *
 * @param center Center.
 * @param radius How far each point of the diamond reaches from the center.
 * @param color Color the diamond with this color.
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
    
    al_draw_prim(vert, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
}


/**
 * @brief Draws a filled equilateral triangle made.
 *
 * @param center Center point of the triangle.
 * @param radius Radius between the center and each vertex.
 * @param angle Angle at which its first vertex points.
 * @param color Its color.
 */
void draw_filled_equilateral_triangle(
    const point &center, float radius, float angle,
    const ALLEGRO_COLOR &color
) {
    point v1 = center + rotate_point(point(radius, 0.0f), angle);
    point v2 = center + rotate_point(point(radius, 0.0f), angle + TAU / 3.0f);
    point v3 = center + rotate_point(point(radius, 0.0f), angle - TAU / 3.0f);
    al_draw_filled_triangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, color);
}


/**
 * @brief Draws a filled rounded rectangle.
 * This is basically Allegro's function, but safer and simpler.
 *
 * @param center Center.
 * @param size Width and height.
 * @param radii Radii of the corners. Will be smaller if the rectangle is
 * too small.
 * @param color Color the rectangle with this color.
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


/**
 * @brief Draws a rotated rectangle.
 *
 * @param center Center of the rectangle.
 * @param dimensions Width and height of the rectangle.
 * @param angle Angle the rectangle is facing.
 * @param color Color to use.
 * @param thickness Thickness to use.
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


/**
 * @brief Draws a rounded rectangle.
 * This is basically Allegro's function, but safer and simpler.
 *
 * @param center Center.
 * @param size Width and height.
 * @param radii Radii of the corners. Will be smaller if the rectangle is
 * too small.
 * @param color Color the diamond with this color.
 * @param thickness Line thickness.
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


/**
 * @brief Draws text, scaled.
 *
 * @param font Font to use.
 * @param color Tint the text with this color.
 * @param where Coordinates to draw in.
 * @param scale Horizontal or vertical scale.
 * @param flags Same flags you'd use for al_draw_text.
 * @param valign Vertical alignment.
 * @param text Text to draw.
 */
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODE valign, const string &text
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


/**
 * @brief Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 *
 * @param font Font to use.
 * @param color Color.
 * @param where Coordinates of the text.
 * @param flags Flags, just like the ones you'd pass to al_draw_text.
 * @param valign Vertical alignment.
 * @param text Text to write, line breaks included ('\n').
 */
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags,
    const TEXT_VALIGN_MODE valign, const string &text
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    float top;
    
    if(valign == TEXT_VALIGN_MODE_TOP) {
        top = where.y;
    } else {
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = (int) n_lines * fh + (int) (n_lines - 1);
        if(valign == TEXT_VALIGN_MODE_CENTER) {
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


/**
 * @brief Draws a box, using a texture.
 *
 * The texture is split into three-by-three.
 * The corners of the box will use the corners of the texture as they are.
 * The remaining sections of the texture will be stretched to fill the
 * box's center and sides.
 * If the box's width or height is smaller than the two relevant corners
 * combined, then the corner graphics will be shrunk down, though.
 *
 * @param center Center of the box.
 * @param size Width and height of the box.
 * @param texture Texture to use.
 * @param tint Tint the texture with this color.
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
        vert, nullptr, texture, 0, total_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
}
