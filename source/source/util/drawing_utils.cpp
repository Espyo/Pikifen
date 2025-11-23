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
void drawBitmap(
    ALLEGRO_BITMAP* bmp, const Point& center,
    const Point& size, float angle, const ALLEGRO_COLOR& tint
) {

    if(size.x == 0 && size.y == 0) return;
    
    Point bmpSize = getBitmapDimensions(bmp);
    Point scale = size / bmpSize;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        tint,
        bmpSize.x / 2.0, bmpSize.y / 2.0,
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
 * @param boxSize Width and height of the box.
 * @param scaleUp If true, the bitmap is scaled up to fit the box.
 * If false, it stays at its original size (unless it needs to be scaled down).
 * @param angle Angle to rotate the bitmap by.
 * The box does not take angling into account.
 * @param tint Tint the bitmap with this color.
 */
void drawBitmapInBox(
    ALLEGRO_BITMAP* bmp, const Point& center, const Point& boxSize,
    bool scaleUp, float angle, const ALLEGRO_COLOR& tint
) {
    if(boxSize.x == 0 || boxSize.y == 0) return;
    int bmpW = al_get_bitmap_width(bmp);
    int bmpH = al_get_bitmap_height(bmp);
    float wDiff = bmpW / boxSize.x;
    float hDiff = bmpH / boxSize.y;
    float maxW = scaleUp ? boxSize.x : std::min((int) boxSize.x, bmpW);
    float maxH = scaleUp ? boxSize.y : std::min((int) boxSize.y, bmpH);
    
    if(wDiff > hDiff) {
        drawBitmap(bmp, center, Point(maxW, -1), angle, tint);
    } else {
        drawBitmap(bmp, center, Point(-1, maxH), angle, tint);
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
void drawEquilateralTriangle(
    const Point& center, float radius, float angle,
    const ALLEGRO_COLOR& color, float thickness
) {
    Point v1 = center + rotatePoint(Point(radius, 0.0f), angle);
    Point v2 = center + rotatePoint(Point(radius, 0.0f), angle + TAU / 3.0f);
    Point v3 = center + rotatePoint(Point(radius, 0.0f), angle - TAU / 3.0f);
    al_draw_triangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, color, thickness);
}


/**
 * @brief Draws a filled diamond shape.
 *
 * @param center Center.
 * @param radius How far each point of the diamond reaches from the center.
 * @param color Color the diamond with this color.
 */
void drawFilledDiamond(
    const Point& center, float radius, const ALLEGRO_COLOR& color
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
void drawFilledEquilateralTriangle(
    const Point& center, float radius, float angle,
    const ALLEGRO_COLOR& color
) {
    Point v1 = center + rotatePoint(Point(radius, 0.0f), angle);
    Point v2 = center + rotatePoint(Point(radius, 0.0f), angle + TAU / 3.0f);
    Point v3 = center + rotatePoint(Point(radius, 0.0f), angle - TAU / 3.0f);
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
void drawFilledRoundedRectangle(
    const Point& center, const Point& size, float radii,
    const ALLEGRO_COLOR& color
) {
    float finalRadii = std::min(radii, size.x / 2.0f);
    finalRadii = std::min(finalRadii, size.y / 2.0f);
    finalRadii = std::max(0.0f, finalRadii);
    al_draw_filled_rounded_rectangle(
        center.x - size.x / 2.0f, center.y - size.y / 2.0f,
        center.x + size.x / 2.0f, center.y + size.y / 2.0f,
        finalRadii, finalRadii,
        color
    );
}


/**
 * @brief Draws a simple rectangle, but using "al_draw_prim" instead of
 * "al_draw_filled_rectangle". This is useful, for instance, to bypass
 * Allegro's limitations on shaders in its simple drawing routines.
 *
 * @param tl Top-left corner coordinates.
 * @param size Width and height.
 * @param color Color of the rectangle.
 * @param texture Texture, if any.
 */
void drawPrimRect(
    const Point& tl, const Point& size, const ALLEGRO_COLOR color,
    ALLEGRO_BITMAP* texture
) {
    ALLEGRO_VERTEX vertexes[4];
    for(unsigned char v = 0; v < 4; v++) {
        vertexes[v].z = 0.0f;
        vertexes[v].color = color;
    }
    
    vertexes[0].x = tl.x;
    vertexes[0].y = tl.y;
    vertexes[0].u = 0.0f;
    vertexes[0].v = 0.0f;
    vertexes[1].x = tl.x + size.x;
    vertexes[1].y = tl.y;
    vertexes[1].u = 1.0f;
    vertexes[1].v = 0.0f;
    vertexes[2].x = tl.x;
    vertexes[2].y = tl.y + size.y;
    vertexes[2].u = 0.0f;
    vertexes[2].v = 1.0f;
    vertexes[3].x = tl.x + size.x;
    vertexes[3].y = tl.y + size.y;
    vertexes[3].u = 1.0f;
    vertexes[3].v = 1.0f;
    
    al_draw_prim(
        vertexes, nullptr, texture, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
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
void drawRotatedRectangle(
    const Point& center, const Point& dimensions,
    float angle, const ALLEGRO_COLOR& color, float thickness
) {
    ALLEGRO_TRANSFORM rotTransform, oldTransform;
    al_copy_transform(&oldTransform, al_get_current_transform());
    al_identity_transform(&rotTransform);
    al_rotate_transform(&rotTransform, angle);
    al_translate_transform(&rotTransform, center.x, center.y);
    al_compose_transform(&rotTransform, &oldTransform);
    
    al_use_transform(&rotTransform); {
        al_draw_rectangle(
            -dimensions.x / 2.0, -dimensions.y / 2.0,
            dimensions.x / 2.0, dimensions.y / 2.0,
            color, thickness
        );
    }; al_use_transform(&oldTransform);
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
void drawRoundedRectangle(
    const Point& center, const Point& size, float radii,
    const ALLEGRO_COLOR& color, float thickness
) {
    float finalRadii = std::min(radii, size.x / 2.0f);
    finalRadii = std::min(finalRadii, size.y / 2.0f);
    finalRadii = std::max(0.0f, finalRadii);
    al_draw_rounded_rectangle(
        center.x - size.x / 2.0f, center.y - size.y / 2.0f,
        center.x + size.x / 2.0f, center.y + size.y / 2.0f,
        finalRadii, finalRadii,
        color, thickness
    );
}


/**
 * @brief Draws plain text, scaled as necessary.
 *
 * @param text Text to draw.
 * @param font Font to use.
 * @param where Coordinates to draw it at.
 * @param boxSize Size of the box it must be scaled to.
 * @param color Tint the text with this color.
 * @param textFlags Allegro text drawing function flags.
 * @param vAlign Vertical alignment.
 * @param settings Settings to control how the text can be scaled.
 * Use TEXT_SETTING_FLAG.
 * @param furtherScale After calculating everything, further scale the
 * text by this much before drawing.
 */
void drawText(
    const string& text, const ALLEGRO_FONT* const font,
    const Point& where, const Point& boxSize, const ALLEGRO_COLOR& color,
    int textFlags, V_ALIGN_MODE vAlign, Bitmask8 settings,
    const Point& furtherScale
) {
    //Initial checks.
    if(text.empty()) return;
    if(boxSize.x == 0 || boxSize.y == 0) return;
    
    //Get the raw text information.
    int textOrigOx;
    int textOrigOy;
    int textOrigW;
    int textOrigH;
    al_get_text_dimensions(
        font, text.c_str(),
        &textOrigOx, &textOrigOy, &textOrigW, &textOrigH
    );
    
    //Figure out the scales.
    Point textOrigSize(textOrigW, textOrigH);
    Point textFinalScale =
        scaleRectangleToBox(
            textOrigSize,
            boxSize,
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_GROW_X),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_GROW_Y),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_X),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_Y),
            hasFlag(settings, TEXT_SETTING_FLAG_CAN_CHANGE_RATIO)
        );
    Point textFinalSize = textOrigSize * textFinalScale;
    
    //Figure out offsets.
    float vAlignOffset =
        getVerticalAlignOffset(vAlign, textFinalSize.y);
        
    //Create the transformation.
    ALLEGRO_TRANSFORM textTransform, oldTransform;
    getTextDrawingTransforms(
        where,
        textFinalScale * furtherScale,
        hasFlag(settings, TEXT_SETTING_FLAG_COMPENSATE_YO) ?
        textOrigOy :
        0.0f,
        vAlignOffset * furtherScale.y,
        &textTransform, &oldTransform
    );
    
    //Draw!
    al_use_transform(&textTransform); {
        al_draw_text(font, color, 0, 0, textFlags, text.c_str());
    }; al_use_transform(&oldTransform);
}


/**
 * @brief Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 *
 * @param text Text to draw.
 * @param font Font to use.
 * @param where Coordinates to draw it at.
 * @param boxSize Size of the box it must be scaled to.
 * @param color Tint the text with this color.
 * @param textFlags Allegro text drawing function flags.
 * @param vAlign Vertical alignment.
 * @param settings Settings to control how the text can be scaled.
 * Use TEXT_SETTING_FLAG.
 * @param furtherScale After calculating everything, further scale the
 * text by this much before drawing.
 */
void drawTextLines(
    const string& text, const ALLEGRO_FONT* const font,
    const Point& where, const Point& boxSize, const ALLEGRO_COLOR& color,
    int textFlags, V_ALIGN_MODE vAlign, Bitmask8 settings,
    const Point& furtherScale
) {
    //Initial checks.
    if(text.empty()) return;
    if(boxSize.x == 0 || boxSize.y == 0) return;
    
    vector<string> lines = split(text, "\n", true);
    
    //Get the basic text information.
    int totalOrigWidth = 0;
    int totalOrigHeight = 0;
    int lineOrigHeight = 0;
    getMultilineTextDimensions(
        lines, font, &totalOrigWidth, &totalOrigHeight, &lineOrigHeight
    );
    Point totalOrigSize(totalOrigWidth, totalOrigHeight);
    
    //Figure out the scales.
    Point totalFinalScale =
        scaleRectangleToBox(
            totalOrigSize, boxSize,
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_GROW_X),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_GROW_Y),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_X),
            !hasFlag(settings, TEXT_SETTING_FLAG_CANT_SHRINK_Y),
            hasFlag(settings, TEXT_SETTING_FLAG_CAN_CHANGE_RATIO)
        );
    Point totalFinalSize = totalOrigSize * totalFinalScale;
    
    //Figure out offsets.
    float vAlignOffset =
        getVerticalAlignOffset(vAlign, totalFinalSize.y);
        
    //Create the transformation.
    ALLEGRO_TRANSFORM textTransform, oldTransform;
    getTextDrawingTransforms(
        where,
        totalFinalScale * furtherScale,
        0.0f, vAlignOffset * furtherScale.y,
        &textTransform, &oldTransform
    );
    
    //Draw!
    al_use_transform(&textTransform); {
        for(size_t l = 0; l < lines.size(); l++) {
            float lineY = (lineOrigHeight + 1) * l;
            al_draw_text(font, color, 0, lineY, textFlags, lines[l].c_str());
        }
    }; al_use_transform(&oldTransform);
}


/**
 * @brief Draws a box, using a 9-slice texture.
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
void drawTexturedBox(
    const Point& center, const Point& size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR& tint
) {
    //While using al_hold_bitmap_drawing is an optimization, we can't use it
    //since it stops using the transformation, meaning any textured boxes
    //meant to be drawn with transformations wouldn't.
    
    //Some caches.
    //Vertex total. 9 sections * 2 tris * 3 vertexes.
    constexpr size_t totalVertexes = 9 * 2 * 3;
    //Top-left coordinates.
    const Point tl = center - size / 2.0f;
    //Bitmap size.
    const int bmpW = al_get_bitmap_width(texture);
    const int bmpH = al_get_bitmap_height(texture);
    //Minimum size at which the corner graphics are drawn in full.
    //Workaround: For some reason there's a seam visible when the edges are
    //around < 6 pixels wide. I can't figure out why. So I'm bumping
    //this threshold to be 8 pixels longer than normal.
    const Point cornerThreshold(
        std::max(8.0f, size.x / 2.0f - 8),
        std::max(8.0f, size.y / 2.0f - 8)
    );
    //Corner size.
    Point cornerSize(bmpW / 3.0f, bmpH / 3.0f);
    if(cornerThreshold.x < cornerSize.x) {
        cornerSize.x = cornerThreshold.x;
        cornerSize.y = cornerSize.x * (bmpW / bmpH);
    }
    if(cornerThreshold.y < cornerSize.y) {
        cornerSize.y = cornerThreshold.y;
        cornerSize.x = cornerSize.y * (bmpH / bmpW);
    }
    
    //Initialize the vertexes.
    ALLEGRO_VERTEX vert[totalVertexes];
    for(unsigned char v = 0; v < totalVertexes; v++) {
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
            y2 = tl.y + cornerSize.y;
            break;
        } case 1: {
            y1 = tl.y + cornerSize.y;
            y2 = tl.y + size.y - cornerSize.y;
            break;
        } case 2: {
            y1 = tl.y + size.y - cornerSize.y;
            y2 = tl.y + size.y;
            break;
        }
        }
        
        //And the start and end Y texture coordinates.
        float v1 = (bmpH / 3.0f) * r;
        float v2 = (bmpH / 3.0f) * (r + 1);
        
        for(size_t c = 0; c < 3; c++) {
            //For every column.
            
            //Figure out the start and end X drawing coordinates.
            float x1 = 0.0f;
            float x2 = 0.0f;
            switch(c) {
            case 0: {
                x1 = tl.x;
                x2 = tl.x + cornerSize.x;
                break;
            } case 1: {
                x1 = tl.x + cornerSize.x;
                x2 = tl.x + size.x - cornerSize.x;
                break;
            } case 2: {
                x1 = tl.x + size.x - cornerSize.x;
                x2 = tl.x + size.x;
                break;
            }
            }
            
            //And the start and end X texture coordinates.
            float u1 = (bmpW / 3.0f) * c;
            float u2 = (bmpW / 3.0f) * (c + 1);
            
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
        vert, nullptr, texture, 0, totalVertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
}


/**
 * @brief Returns the width and height of a block of multi-line text.
 *
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap
 * that would hold a drawing by drawTextLines().
 *
 * @param lines The text lines.
 * @param font The text's font.
 * @param outWidth If not nullptr, the width is returned here.
 * @param outHeight If not nullptr, the height is returned here.
 * @param outLineHeight If not nullptr, the line height is returned here.
 */
void getMultilineTextDimensions(
    const vector<string>& lines, const ALLEGRO_FONT* const font,
    int* outWidth, int* outHeight, int* outLineHeight
) {
    int lh = al_get_font_line_height(font);
    
    if(outHeight) {
        *outHeight = std::max(0, (int) ((lh + 1) * lines.size()) - 1);
    }
    
    if(outWidth) {
        int largestW = 0;
        for(size_t l = 0; l < lines.size(); l++) {
            largestW =
                std::max(
                    largestW, al_get_text_width(font, lines[l].c_str())
                );
        }
        
        *outWidth = largestW;
    }
    
    if(outLineHeight) *outLineHeight = lh;
}


/**
 * @brief Returns the Allegro transform to use to draw text in the
 * specified way.
 *
 * @param where Coordinates to draw the text at.
 * @param scale Text scale.
 * @param textOrigOY The text's original Y offset,
 * from al_get_text_dimensions.
 * @param vAlignOffset Vertical alignment offset.
 * @param outTextTransform The text transform is returned here.
 * @param outOldTransform The old (current) transform is returned here.
 */
void getTextDrawingTransforms(
    const Point& where, const Point& scale,
    float textOrigOY, float vAlignOffset,
    ALLEGRO_TRANSFORM* outTextTransform, ALLEGRO_TRANSFORM* outOldTransform
) {
    al_copy_transform(outOldTransform, al_get_current_transform());
    al_identity_transform(outTextTransform);
    al_scale_transform(outTextTransform, scale.x, scale.y);
    al_translate_transform(
        outTextTransform,
        where.x,
        where.y - vAlignOffset - textOrigOY
    );
    al_compose_transform(outTextTransform, outOldTransform);
}
