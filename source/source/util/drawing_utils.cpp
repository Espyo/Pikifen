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
    ALLEGRO_BITMAP* bmp, const Point &center,
    const Point &size, float angle, const ALLEGRO_COLOR &tint
) {

    if(size.x == 0 && size.y == 0) return;
    
    Point bmp_size = get_bitmap_dimensions(bmp);
    Point scale = size / bmp_size;
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
    ALLEGRO_BITMAP* bmp, const Point &center, const Point &box_size,
    bool scale_up, float angle, const ALLEGRO_COLOR &tint
) {
    if(box_size.x == 0 || box_size.y == 0) return;
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    float w_diff = bmp_w / box_size.x;
    float h_diff = bmp_h / box_size.y;
    float max_w = scale_up ? box_size.x : std::min((int) box_size.x, bmp_w);
    float max_h = scale_up ? box_size.y : std::min((int) box_size.y, bmp_h);
    
    if(w_diff > h_diff) {
        draw_bitmap(bmp, center, Point(max_w, -1), angle, tint);
    } else {
        draw_bitmap(bmp, center, Point(-1, max_h), angle, tint);
    }
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
    const Point &center, float radius, float angle,
    const ALLEGRO_COLOR &color, float thickness
) {
    Point v1 = center + rotate_point(Point(radius, 0.0f), angle);
    Point v2 = center + rotate_point(Point(radius, 0.0f), angle + TAU / 3.0f);
    Point v3 = center + rotate_point(Point(radius, 0.0f), angle - TAU / 3.0f);
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
    const Point &center, float radius, const ALLEGRO_COLOR &color
) {
    ALLEGRO_VERTEX vert[4];
    for(unsigned char v = 0; v < 4; v++) {
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
    const Point &center, float radius, float angle,
    const ALLEGRO_COLOR &color
) {
    Point v1 = center + rotate_point(Point(radius, 0.0f), angle);
    Point v2 = center + rotate_point(Point(radius, 0.0f), angle + TAU / 3.0f);
    Point v3 = center + rotate_point(Point(radius, 0.0f), angle - TAU / 3.0f);
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
    const Point &center, const Point &size, float radii,
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
    const Point &center, const Point &dimensions,
    float angle, const ALLEGRO_COLOR &color, float thickness
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
    const Point &center, const Point &size, float radii,
    const ALLEGRO_COLOR &color, float thickness
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
 * @brief Draws plain text, scaled as necessary.
 *
 * @param text Text to draw.
 * @param font Font to use.
 * @param where Coordinates to draw it at.
 * @param box_size Size of the box it must be scaled to.
 * @param color Tint the text with this color.
 * @param text_flags Allegro text drawing function flags.
 * @param v_align Vertical alignment.
 * @param settings Settings to control how the text can be scaled.
 * Use TEXT_SETTING_FLAG.
 * @param further_scale After calculating everything, further scale the
 * text by this much before drawing.
 */
void draw_text(
    const string &text, const ALLEGRO_FONT* const font,
    const Point &where, const Point &box_size, const ALLEGRO_COLOR &color,
    int text_flags, V_ALIGN_MODE v_align, bitmask_8_t settings,
    const Point &further_scale
) {
    //Initial checks.
    if(text.empty()) return;
    if(box_size.x == 0 || box_size.y == 0) return;
    
    //Get the raw text information.
    int text_orig_ox;
    int text_orig_oy;
    int text_orig_w;
    int text_orig_h;
    al_get_text_dimensions(
        font, text.c_str(),
        &text_orig_ox, &text_orig_oy, &text_orig_w, &text_orig_h
    );
    
    //Figure out the scales.
    Point text_orig_size(text_orig_w, text_orig_h);
    Point text_final_scale =
        scale_rectangle_to_box(
            text_orig_size,
            box_size,
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_GROW_X),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_GROW_Y),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_X),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_Y),
            has_flag(settings, TEXT_SETTING_FLAG_CAN_CHANGE_RATIO)
        );
    Point text_final_size = text_orig_size * text_final_scale;
    
    //Figure out offsets.
    float v_align_offset =
        get_vertical_align_offset(v_align, text_final_size.y);
        
    //Create the transformation.
    ALLEGRO_TRANSFORM text_transform, old_transform;
    get_text_drawing_transforms(
        where,
        text_final_scale * further_scale,
        has_flag(settings, TEXT_SETTING_COMPENSATE_Y_OFFSET) ?
        text_orig_oy :
        0.0f,
        v_align_offset * further_scale.y,
        &text_transform, &old_transform
    );
    
    //Draw!
    al_use_transform(&text_transform); {
        al_draw_text(font, color, 0, 0, text_flags, text.c_str());
    }; al_use_transform(&old_transform);
}


/**
 * @brief Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 *
 * @param text Text to draw.
 * @param font Font to use.
 * @param where Coordinates to draw it at.
 * @param box_size Size of the box it must be scaled to.
 * @param color Tint the text with this color.
 * @param text_flags Allegro text drawing function flags.
 * @param v_align Vertical alignment.
 * @param settings Settings to control how the text can be scaled.
 * Use TEXT_SETTING_FLAG.
 * @param further_scale After calculating everything, further scale the
 * text by this much before drawing.
 */
void draw_text_lines(
    const string &text, const ALLEGRO_FONT* const font,
    const Point &where, const Point &box_size, const ALLEGRO_COLOR &color,
    int text_flags, V_ALIGN_MODE v_align, bitmask_8_t settings,
    const Point &further_scale
) {
    //Initial checks.
    if(text.empty()) return;
    if(box_size.x == 0 || box_size.y == 0) return;
    
    vector<string> lines = split(text, "\n", true);
    
    //Get the basic text information.
    int total_orig_width = 0;
    int total_orig_height = 0;
    int line_orig_height = 0;
    get_multiline_text_dimensions(
        lines, font, &total_orig_width, &total_orig_height, &line_orig_height
    );
    Point total_orig_size(total_orig_width, total_orig_height);
    
    //Figure out the scales.
    Point total_final_scale =
        scale_rectangle_to_box(
            total_orig_size, box_size,
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_GROW_X),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_GROW_Y),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_X),
            !has_flag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_Y),
            has_flag(settings, TEXT_SETTING_FLAG_CAN_CHANGE_RATIO)
        );
    Point total_final_size = total_orig_size * total_final_scale;
    
    //Figure out offsets.
    float v_align_offset =
        get_vertical_align_offset(v_align, total_final_size.y);
        
    //Create the transformation.
    ALLEGRO_TRANSFORM text_transform, old_transform;
    get_text_drawing_transforms(
        where,
        total_final_scale * further_scale,
        0.0f, v_align_offset * further_scale.y,
        &text_transform, &old_transform
    );
    
    //Draw!
    al_use_transform(&text_transform); {
        for(size_t l = 0; l < lines.size(); l++) {
            float line_y = (line_orig_height + 1) * l;
            al_draw_text(font, color, 0, line_y, text_flags, lines[l].c_str());
        }
    }; al_use_transform(&old_transform);
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
    const Point &center, const Point &size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR &tint
) {
    //While using al_hold_bitmap_drawing is an optimization, we can't use it
    //since it stops using the transformation, meaning any textured boxes
    //meant to be drawn with transformations wouldn't.
    
    //Some caches.
    //Vertex total. 9 sections * 2 tris * 3 vertexes.
    constexpr size_t total_vertexes = 9 * 2 * 3;
    //Top-left coordinates.
    const Point tl = center - size / 2.0f;
    //Bitmap size.
    const int bmp_w = al_get_bitmap_width(texture);
    const int bmp_h = al_get_bitmap_height(texture);
    //Minimum size at which the corner graphics are drawn in full.
    //Workaround: For some reason there's a seam visible when the edges are
    //around < 6 pixels wide. I can't figure out why. So I'm bumping
    //this threshold to be 8 pixels longer than normal.
    const Point corner_treshold(
        std::max(8.0f, size.x / 2.0f - 8),
        std::max(8.0f, size.y / 2.0f - 8)
    );
    //Corner size.
    Point corner_size(bmp_w / 3.0f, bmp_h / 3.0f);
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
    for(unsigned char v = 0; v < total_vertexes; v++) {
        vert[v].color = tint;
        vert[v].z = 0;
    }
    
    size_t v = 0;
    for(size_t r = 0; r < 3; r++) {
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
        
        for(size_t c = 0; c < 3; c++) {
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


/**
 * @brief Returns the width and height of a block of multi-line text.
 *
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap
 * that would hold a drawing by draw_text_lines().
 *
 * @param lines The text lines.
 * @param font The text's font.
 * @param out_width If not nullptr, the width is returned here.
 * @param out_height If not nullptr, the height is returned here.
 * @param out_line_height If not nullptr, the line height is returned here.
 */
void get_multiline_text_dimensions(
    const vector<string> &lines, const ALLEGRO_FONT* const font,
    int* out_width, int* out_height, int* out_line_height
) {
    int lh = al_get_font_line_height(font);
    
    if(out_height) {
        *out_height = std::max(0, (int) ((lh + 1) * lines.size()) - 1);
    }
    
    if(out_width) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); l++) {
            largest_w =
                std::max(
                    largest_w, al_get_text_width(font, lines[l].c_str())
                );
        }
        
        *out_width = largest_w;
    }
    
    if(out_line_height) *out_line_height = lh;
}


/**
 * @brief Returns the Allegro transform to use to draw text in the
 * specified way.
 *
 * @param where Coordinates to draw the text at.
 * @param scale Text scale.
 * @param text_orig_oy The text's original Y offset,
 * from al_get_text_dimensions.
 * @param v_align_offset Vertical alignment offset.
 * @param out_text_transform The text transform is returned here.
 * @param out_old_transform The old (current) transform is returned here.
 */
void get_text_drawing_transforms(
    const Point &where, const Point &scale,
    float text_orig_oy, float v_align_offset,
    ALLEGRO_TRANSFORM* out_text_transform, ALLEGRO_TRANSFORM* out_old_transform
) {
    al_copy_transform(out_old_transform, al_get_current_transform());
    al_identity_transform(out_text_transform);
    al_scale_transform(out_text_transform, scale.x, scale.y);
    al_translate_transform(
        out_text_transform,
        where.x,
        where.y - v_align_offset - text_orig_oy
    );
    al_compose_transform(out_text_transform, out_old_transform);
}
