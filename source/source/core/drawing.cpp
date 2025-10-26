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

#include "../content/animation/animation.h"
#include "../content/mob/group_task.h"
#include "../content/mob/pile.h"
#include "../content/mob/scale.h"
#include "../game_state/gameplay/gameplay.h"
#include "../util/allegro_utils.h"
#include "../util/drawing_utils.h"
#include "../util/general_utils.h"
#include "../util/geometry_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "game.h"
#include "misc_functions.h"


namespace BIND_INPUT_ICON {

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

//Distance after which camera shakes from mob actions no longer have
//an effect.
const float CAM_SHAKE_DROPOFF_DIST = 1000.0f;

//Maximum amount in any direction that the camera is allowed to offset
//when shaking.
const float CAM_SHAKE_MAX_OFFSET = 30.0f;

//Default health wheel radius.
const float DEF_HEALTH_WHEEL_RADIUS = 20;

//Alpha change speed for the enemy/treasure point value near the leader cursor,
//in amount per second.
const float LEADER_CURSOR_PTS_ALPHA_SPEED = 3.0f;

//Liquid surfaces wobble by offsetting X by this much, at most.
const float LIQUID_WOBBLE_DELTA_X = 3.0f;

//Liquid surfaces wobble using this time scale.
const float LIQUID_WOBBLE_TIME_SCALE = 2.0f;

//Loading screen subtext padding.
const int LOADING_SCREEN_PADDING = 64;

//Loading screen subtext scale.
const float LOADING_SCREEN_SUBTEXT_SCALE = 0.6f;

//Loading screen text height, in window ratio.
const float LOADING_SCREEN_TEXT_HEIGHT = 0.10f;

//Loading screen text width, in window ratio.
const float LOADING_SCREEN_TEXT_WIDTH = 0.70f;

//Notification opacity.
const unsigned char NOTIFICATION_ALPHA = 160;

//Size of a control bind icon in a notification.
const float NOTIFICATION_INPUT_SIZE = 24.0f;

//Padding between a notification's text and its limit.
const float NOTIFICATION_PADDING = 8.0f;

}


/**
 * @brief Draws a series of logos, to serve as a background.
 * They move along individually, and wrap around when they reach a window edge.
 *
 * @param timeSpent How much time has passed.
 * @param rows Rows of logos to draw.
 * @param cols Columns of logos to draw.
 * @param logoSize Width and height of the logos.
 * @param tint Tint the logos with this color.
 * @param speed Horizontal and vertical movement speed of each logo.
 * @param rotationSpeed Rotation speed of each logo.
 */
void drawBackgroundLogos(
    float timeSpent, size_t rows, size_t cols,
    const Point& logoSize, const ALLEGRO_COLOR& tint,
    const Point& speed, float rotationSpeed
) {
    al_hold_bitmap_drawing(true);
    
    float spacingX = (game.winW + logoSize.x) / cols;
    float spacingY = (game.winH + logoSize.y) / rows;
    
    for(size_t c = 0; c < cols; c++) {
        for(size_t r = 0; r < rows; r++) {
            float x = (c * spacingX) + timeSpent * speed.x;
            if(r % 2 == 0) {
                x += spacingX / 2.0f;
            }
            x =
                wrapFloat(
                    x,
                    0 - logoSize.x * 0.5f,
                    game.winW + logoSize.x * 0.5f
                );
            float y =
                wrapFloat(
                    (r * spacingY) + timeSpent * speed.y,
                    0 - logoSize.y * 0.5f,
                    game.winH + logoSize.y * 0.5f
                );
            drawBitmap(
                game.sysContent.bmpIcon,
                Point(x, y),
                Point(logoSize.x, logoSize.y),
                timeSpent * rotationSpeed,
                tint
            );
        }
    }
    
    al_hold_bitmap_drawing(false);
}


/**
 * @brief Draws a bitmap, applying bitmap effects.
 *
 * @param bmp The bitmap.
 * @param effects Effects to use.
 */
void drawBitmapWithEffects(
    ALLEGRO_BITMAP* bmp, const BitmapEffect& effects
) {

    Point bmpSize = getBitmapDimensions(bmp);
    float scaleX =
        (effects.scale.x == LARGE_FLOAT) ? effects.scale.y : effects.scale.x;
    float scaleY =
        (effects.scale.y == LARGE_FLOAT) ? effects.scale.x : effects.scale.y;
        
    if(effects.colorize.a > 0.0f) {
        al_use_shader(game.shaders.getShader(SHADER_TYPE_COLORIZER));
        al_set_shader_float_vector(
            "colorizer_color", 4, (float*) &effects.colorize, 1
        );
    }
    
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        effects.tintColor,
        bmpSize.x / 2, bmpSize.y / 2,
        effects.translation.x, effects.translation.y,
        scaleX, scaleY,
        effects.rotation,
        0
    );
    
    if(effects.colorize.a > 0.0f) {
        al_use_shader(nullptr);
    }
}


/**
 * @brief Draws a button.
 *
 * @param center Center coordinates.
 * @param size Width and height.
 * @param text Text inside the button.
 * @param font What font to write the text in.
 * @param textColor Color to draw the text with.
 * @param focused Is the button currently focused?
 * @param juicyGrowAmount If it's in the middle of a juicy grow animation,
 * specify the amount here.
 * @param tint General tint color.
 */
void drawButton(
    const Point& center, const Point& size, const string& text,
    const ALLEGRO_FONT* font, const ALLEGRO_COLOR& textColor,
    bool focused, float juicyGrowAmount, const ALLEGRO_COLOR& tint
) {
    drawText(
        text, font, center, size * GUI::STANDARD_CONTENT_SIZE,
        tintColor(textColor, tint),
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        Point(1.0 + juicyGrowAmount)
    );
    
    ALLEGRO_COLOR boxTint =
        focused ?
        tintColor(game.config.guiColors.focusedItem, tint) :
        tint;
        
    drawTexturedBox(
        center, size, game.sysContent.bmpBubbleBox, boxTint
    );
}


/**
 * @brief Draws a fraction, so one number above another, divided by a bar.
 * The top number usually represents the current value of some attribute,
 * and the bottom number usually represents the required value for some goal.
 *
 * @param bottom Bottom center point of the text.
 * @param valueNr Number that represents the current value.
 * @param requirementNr Number that represents the requirement.
 * @param color Color of the fraction's text.
 * @param scale Scale the text by this much.
 */
void drawFraction(
    const Point& bottom, size_t valueNr,
    size_t requirementNr, const ALLEGRO_COLOR& color, float scale
) {
    const float valueNrY = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT * 3;
    const float valueNrScale = valueNr >= requirementNr ? 1.2f : 1.0f;
    drawText(
        i2s(valueNr), game.sysContent.fntValue, Point(bottom.x, valueNrY),
        Point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0,
        Point(valueNrScale)
    );
    
    const float barY = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT * 2;
    drawText(
        "-", game.sysContent.fntValue, Point(bottom.x, barY),
        Point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0
    );
    
    float reqNrY = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT;
    float reqNrScale = requirementNr > valueNr ? 1.2f : 1.0f;
    drawText(
        i2s(requirementNr), game.sysContent.fntValue,
        Point(bottom.x, reqNrY),
        Point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0,
        Point(reqNrScale)
    );
}


/**
 * @brief Draws a health wheel, with a pie-slice that's fuller the more
 * HP is full.
 *
 * @param center Center of the wheel.
 * @param ratio Ratio of health that is filled. 0 is empty, 1 is full.
 * @param alpha Total opacity of the health wheel.
 * @param radius Radius of the wheel (the whole wheel, not just the pie-slice).
 * @param justChart If true, only draw the actual pie-slice (pie-chart).
 * Used for leader HP on the HUD.
 */
void drawHealth(
    const Point& center,
    float ratio, float alpha,
    float radius, bool justChart
) {
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgba_f(1 - (ratio - 0.5) * 2, 1, 0, alpha);
    } else {
        c = al_map_rgba_f(1, (ratio * 2), 0, alpha);
    }
    
    if(!justChart) {
        al_draw_filled_circle(
            center.x, center.y, radius, al_map_rgba(0, 0, 0, 128 * alpha)
        );
    }
    al_draw_filled_pieslice(
        center.x, center.y, radius, -TAU / 4, -ratio * TAU, c
    );
    if(!justChart) {
        al_draw_circle(
            center.x, center.y, radius + 1, al_map_rgba(0, 0, 0, alpha * 255), 2
        );
    }
}


/**
 * @brief Draws a liquid sector.
 *
 * @param sPtr Pointer to the sector.
 * @param lPtr Pointer to the liquid.
 * @param where X and Y offset.
 * @param scale Scale the sector by this much.
 * @param time How much time has passed. Used to animate.
 */
void drawLiquid(
    Sector* sPtr, Liquid* lPtr, const Point& where, float scale,
    float time
) {
    //Setup.
    if(!sPtr) return;
    if(sPtr->isBottomlessPit) return;
    
    float liquidOpacityMult = 1.0f;
    if(sPtr->drainingLiquid) {
        liquidOpacityMult =
            sPtr->liquidDrainLeft / GEOMETRY::LIQUID_DRAIN_DURATION;
    }
    float brightnessMult = sPtr->brightness / 255.0;
    float sectorScroll[2] = {
        sPtr->scroll.x,
        sPtr->scroll.y
    };
    float distortionAmount[2] = {
        lPtr->distortionAmount.x,
        lPtr->distortionAmount.y
    };
    float liquidTint[4] = {
        lPtr->bodyColor.r,
        lPtr->bodyColor.g,
        lPtr->bodyColor.b,
        lPtr->bodyColor.a
    };
    float shineColor[4] = {
        lPtr->shineColor.r,
        lPtr->shineColor.g,
        lPtr->shineColor.b,
        lPtr->shineColor.a
    };
    
    //TODO Uncomment when liquids use foam edges.
    /*
     * We need to get a list of edges that the shader needs to check,
     * this can extend to other sectors whenever a liquid occupies more
     * than one sector, so we need to loop through all of the connected sectors.
     * This could likely be optimized, but this has no noticeable impact
     * on performance.
    
    vector<sector*> checkedS {sPtr};
    vector<edge*> borderEdges;
    
    for(size_t s = 0; s < checkedS.size(); s++) {
        sector* s2Ptr = checkedS[s];
        for(size_t e = 0; e < s2Ptr->edges.size(); e++) {
            edge* ePtr = s2Ptr->edges[e];
            sector* uS = nullptr;
            sector* aS = nullptr;
            if(doesEdgeHaveLiquidLimit(ePtr, &uS, &aS)) {
                borderEdges.push_back(ePtr);
            }
    
            sector* otherPtr = ePtr->getOtherSector(s2Ptr);
            if(otherPtr) {
                for(size_t h = 0; h < otherPtr->hazards.size(); h++) {
                    if(otherPtr->hazards[h]->associatedLiquid) {
                        if(!isInContainer(checkedS, otherPtr)) {
                            checkedS.push_back(otherPtr);
                        }
                    }
                }
            }
        }
    }
    
    uint edgeCount = borderEdges.size();
    
    float bufferEdges[edgeCount * 4];
    
    for(size_t e = 0; e < edgeCount; e++) {
        edge* edge = borderEdges[e];
    
        bufferEdges[4 * e    ] = edge->vertexes[0]->x;
        bufferEdges[4 * e + 1] = edge->vertexes[0]->y;
        bufferEdges[4 * e + 2] = edge->vertexes[1]->x;
        bufferEdges[4 * e + 3] = edge->vertexes[1]->y;
    }
    
    //Put the buffer onto the shader
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(bufferEdges),
        bufferEdges, GL_STREAM_READ
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    */
    
    //Set up the shader.
    ALLEGRO_SHADER* liqShader = game.shaders.getShader(SHADER_TYPE_LIQUID);
    al_use_shader(liqShader);
    al_set_shader_float("area_time", time * lPtr->animSpeed);
    al_set_shader_float("opacity", liquidOpacityMult);
    al_set_shader_float("sector_brightness", brightnessMult);
    al_set_shader_float_vector("sector_scroll", 2, &sectorScroll[0], 1);
    al_set_shader_float("shine_min_threshold", lPtr->shineMinThreshold);
    al_set_shader_float("shine_max_threshold", lPtr->shineMaxThreshold);
    al_set_shader_float_vector("distortion_amount", 2, &distortionAmount[0], 1);
    al_set_shader_float_vector("surface_color", 4, &liquidTint[0], 1);
    al_set_shader_float_vector("shine_color", 4, &shineColor[0], 1);
    
    //Draw the sector liquid now!
    unsigned char nTextures = 1;
    Sector* textureSector[2] = {nullptr, nullptr};
    if(sPtr->fade) {
        sPtr->getTextureMergeSectors(
            &textureSector[0], &textureSector[1]
        );
        if(!textureSector[0] && !textureSector[1]) {
            //Can't draw this sector's liquid.
            nTextures = 0;
        } else {
            nTextures = 2;
        }
        
    } else {
        textureSector[0] = sPtr;
        
    }
    
    for(unsigned char t = 0; t < nTextures; t++) {
        bool drawSector0 = true;
        if(!textureSector[0]) drawSector0 = false;
        else if(textureSector[0]->isBottomlessPit) {
            drawSector0 = false;
        }
        
        if(nTextures == 2 && !drawSector0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        if(!textureSector[t] || textureSector[t]->isBottomlessPit) {
            continue;
        }
        size_t nVertexes = sPtr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[nVertexes];
        
        SectorTexture* textureInfoToUse =
            &textureSector[t]->textureInfo;
            
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -textureInfoToUse->translation.x,
            -textureInfoToUse->translation.y,
            1.0f / textureInfoToUse->scale.x,
            1.0f / textureInfoToUse->scale.y,
            -textureInfoToUse->rot
        );
        
        float textureOffset[2] = {
            textureInfoToUse->translation.x,
            textureInfoToUse->translation.y
        };
        float textureScale[2] = {
            textureInfoToUse->scale.x,
            textureInfoToUse->scale.y
        };
        
        for(size_t v = 0; v < nVertexes; v++) {
        
            const Triangle* tPtr = &sPtr->triangles[floor(v / 3.0)];
            Vertex* vPtr = tPtr->points[v % 3];
            float vx = vPtr->x;
            float vy = vPtr->y;
            
            float alphaMult = 1;
            float tsBrightnessMult = textureSector[t]->brightness / 255.0;
            
            if(t == 1) {
                if(!drawSector0) {
                    alphaMult = 0;
                    for(
                        size_t e = 0; e < textureSector[1]->edges.size(); e++
                    ) {
                        if(
                            textureSector[1]->edges[e]->vertexes[0] == vPtr ||
                            textureSector[1]->edges[e]->vertexes[1] == vPtr
                        ) {
                            alphaMult = 1;
                        }
                    }
                } else {
                    for(
                        size_t e = 0; e < textureSector[0]->edges.size(); e++
                    ) {
                        if(
                            textureSector[0]->edges[e]->vertexes[0] == vPtr ||
                            textureSector[0]->edges[e]->vertexes[1] == vPtr
                        ) {
                            alphaMult = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            if(textureSector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba_f(
                    textureSector[t]->textureInfo.tint.r * tsBrightnessMult,
                    textureSector[t]->textureInfo.tint.g * tsBrightnessMult,
                    textureSector[t]->textureInfo.tint.b * tsBrightnessMult,
                    textureSector[t]->textureInfo.tint.a * alphaMult
                );
        }
        
        for(size_t v = 0; v < nVertexes; v++) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            textureSector[t] ?
            textureSector[t]->textureInfo.bitmap :
            textureSector[t == 0 ? 1 : 0]->textureInfo.bitmap;
            
        int bmp_size[2] = {
            al_get_bitmap_width(tex),
            al_get_bitmap_height(tex)
        };
        al_set_shader_float_vector("tex_translation", 2, textureOffset, 1);
        al_set_shader_float_vector("tex_scale", 2, textureScale, 1);
        al_set_shader_float("tex_rotation", textureInfoToUse->rot);
        al_set_shader_int_vector("bmp_size", 2, &bmp_size[0], 1);
        
        al_draw_prim(
            av, nullptr, tex,
            0, (int) nVertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
    
    //Finish up.
    al_use_shader(nullptr);
    
}


/**
 * @brief Draws the loading screen for an area (or anything else, really).
 *
 * @param text The main text to show, optional.
 * @param subtext Subtext to show under the main text, optional.
 * @param maker Name of the maker, optional.
 * @param opacity [0 - 1]. The background blackness lowers in opacity
 * much faster.
 */
void drawLoadingScreen(
    const string& text, const string& subtext, const string& maker,
    float opacity
) {
    const float textW = game.winW * DRAWING::LOADING_SCREEN_TEXT_WIDTH;
    const float textH = game.winH * DRAWING::LOADING_SCREEN_TEXT_HEIGHT;
    const float subtextW = textW * DRAWING::LOADING_SCREEN_SUBTEXT_SCALE;
    const float subtextH = textH * DRAWING::LOADING_SCREEN_SUBTEXT_SCALE;
    
    unsigned char blacknessAlpha = 255.0f * std::max(0.0f, opacity * 4 - 3);
    al_draw_filled_rectangle(
        0, 0, game.winW, game.winH, al_map_rgba(0, 0, 0, blacknessAlpha)
    );
    
    int oldOp, oldSrc, oldDst, oldAop, oldAsrc, oldAdst;
    al_get_separate_blender(
        &oldOp, &oldSrc, &oldDst, &oldAop, &oldAsrc, &oldAdst
    );
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
    
    //Set up the bitmap that will hold the text if it doesn't exist.
    if(!text.empty() && !game.loadingTextBmp) {
        game.loadingTextBmp = al_create_bitmap(textW, textH);
        
        al_set_target_bitmap(game.loadingTextBmp); {
            al_clear_to_color(COLOR_EMPTY);
            drawText(
                text, game.sysContent.fntAreaName,
                Point(textW * 0.5f, textH * 0.5f),
                Point(textW, textH),
                game.config.guiColors.gold,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0
            );
        } al_set_target_backbuffer(game.display);
        
    }
    
    //Set up the bitmap that will hold the text if it doesn't exist.
    if(!subtext.empty() && !game.loadingSubtextBmp) {
        game.loadingSubtextBmp = al_create_bitmap(subtextW, subtextH);
        
        al_set_target_bitmap(game.loadingSubtextBmp); {
            al_clear_to_color(COLOR_EMPTY);
            drawText(
                subtext, game.sysContent.fntAreaName,
                Point(subtextW * 0.5f, subtextH * 0.5f),
                Point(subtextW, subtextH),
                mapGray(224),
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0
            );
            
        } al_set_target_backbuffer(game.display);
        
    }
    
    al_set_separate_blender(
        oldOp, oldSrc, oldDst, oldAop, oldAsrc, oldAdst
    );
    
    //Draw the text bitmap in its place.
    const float textX = game.winW * 0.5 - textW * 0.5;
    float textY = game.winH * 0.5 - textH * 0.5;
    if(!text.empty()) {
        if(!subtext.empty()) {
            textY -= DRAWING::LOADING_SCREEN_PADDING * 0.5;
        }
        al_draw_tinted_bitmap(
            game.loadingTextBmp,
            mapAlpha(255.0 * opacity),
            game.winW * 0.5 - textW * 0.5, textY, 0
        );
        
    }
    
    //Draw the subtext bitmap in its place.
    const float subtextX = game.winW * 0.5 - subtextW * 0.5;
    float subtextY = game.winH * 0.5 + DRAWING::LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_tinted_bitmap(
            game.loadingSubtextBmp,
            mapAlpha(255.0 * opacity),
            game.winW * 0.5 - subtextW * 0.5, subtextY, 0
        );
        
    }
    
    const unsigned char reflectionAlpha = 128.0 * opacity;
    
    //Now, draw the polygon that will hold the reflection for the text.
    if(!text.empty()) {
    
        ALLEGRO_VERTEX textVertexes[4];
        const float textReflectionH = textH * 0.80f;
        //Top-left vertex.
        textVertexes[0].x = textX;
        textVertexes[0].y = textY + textH;
        textVertexes[0].z = 0;
        textVertexes[0].u = 0;
        textVertexes[0].v = textH;
        textVertexes[0].color = mapAlpha(reflectionAlpha);
        //Top-right vertex.
        textVertexes[1].x = textX + textW;
        textVertexes[1].y = textY + textH;
        textVertexes[1].z = 0;
        textVertexes[1].u = textW;
        textVertexes[1].v = textH;
        textVertexes[1].color = mapAlpha(reflectionAlpha);
        //Bottom-right vertex.
        textVertexes[2].x = textX + textW;
        textVertexes[2].y = textY + textH + textReflectionH;
        textVertexes[2].z = 0;
        textVertexes[2].u = textW;
        textVertexes[2].v = textH - textReflectionH;
        textVertexes[2].color = COLOR_EMPTY_WHITE;
        //Bottom-left vertex.
        textVertexes[3].x = textX;
        textVertexes[3].y = textY + textH + textReflectionH;
        textVertexes[3].z = 0;
        textVertexes[3].u = 0;
        textVertexes[3].v = textH - textReflectionH;
        textVertexes[3].color = COLOR_EMPTY_WHITE;
        
        al_draw_prim(
            textVertexes, nullptr, game.loadingTextBmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //And the polygon for the subtext.
    if(!subtext.empty()) {
    
        ALLEGRO_VERTEX subtextVertexes[4];
        const float subtextReflectionH = subtextH * 0.80f;
        //Top-left vertex.
        subtextVertexes[0].x = subtextX;
        subtextVertexes[0].y = subtextY + subtextH;
        subtextVertexes[0].z = 0;
        subtextVertexes[0].u = 0;
        subtextVertexes[0].v = subtextH;
        subtextVertexes[0].color = mapAlpha(reflectionAlpha);
        //Top-right vertex.
        subtextVertexes[1].x = subtextX + subtextW;
        subtextVertexes[1].y = subtextY + subtextH;
        subtextVertexes[1].z = 0;
        subtextVertexes[1].u = subtextW;
        subtextVertexes[1].v = subtextH;
        subtextVertexes[1].color = mapAlpha(reflectionAlpha);
        //Bottom-right vertex.
        subtextVertexes[2].x = subtextX + subtextW;
        subtextVertexes[2].y = subtextY + subtextH + subtextReflectionH;
        subtextVertexes[2].z = 0;
        subtextVertexes[2].u = subtextW;
        subtextVertexes[2].v = subtextH - subtextReflectionH;
        subtextVertexes[2].color = COLOR_EMPTY_WHITE;
        //Bottom-left vertex.
        subtextVertexes[3].x = subtextX;
        subtextVertexes[3].y = subtextY + subtextH + subtextReflectionH;
        subtextVertexes[3].z = 0;
        subtextVertexes[3].u = 0;
        subtextVertexes[3].v = subtextH - subtextReflectionH;
        subtextVertexes[3].color = COLOR_EMPTY_WHITE;
        
        al_draw_prim(
            subtextVertexes, nullptr, game.loadingSubtextBmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //Draw the area's maker, if applicable.
    if(!maker.empty()) {
        const Point textBox(game.winW * 0.20f, game.winH * 0.03f);
        drawText(
            "Made by: " + maker, game.sysContent.fntStandard,
            Point(8, game.winH - 8), textBox,
            al_map_rgba(192, 192, 192, opacity * 255.0),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_BOTTOM, 0,
            Point(0.8f, 0.8f)
        );
    }
    
    //Draw the game's logo to the left of the "Loading..." text,
    //if we're not fading.
    if(opacity == 1.0f) {
        const Point textBox(game.winW * 0.11f, game.winH * 0.03f);
        
        if(
            game.sysContent.bmpIcon &&
            game.sysContent.bmpIcon != game.bmpError
        ) {
            Point iconPos(
                game.winW - 8 - textBox.x - 8 - textBox.y / 2.0f,
                game.winH - 8 - textBox.y / 2.0f
            );
            drawBitmap(
                game.sysContent.bmpIcon, iconPos,
                Point(-1, textBox.y),
                0, mapAlpha(opacity * 255.0)
            );
        }
        
        drawText(
            "Loading...", game.sysContent.fntStandard,
            Point(game.winW - 8, game.winH - 8), textBox,
            mapGray(192), ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_BOTTOM
        );
    }
    
}


/**
 * @brief Draws the icon for a menu button.
 *
 * @param icon Icon ID.
 * @param buttonCenter Center coordinates of the button.
 * @param buttonSize Dimensions of the button.
 * @param leftSide If true, place the icon to the left side of the button.
 * If false, place it to the right.
 * @param tint Color to tint with.
 */
void drawMenuButtonIcon(
    MENU_ICON icon, const Point& buttonCenter, const Point& buttonSize,
    bool leftSide, const ALLEGRO_COLOR& tint
) {
    //All icons are square, and in a row, so the spritesheet height works.
    int iconSize =
        al_get_bitmap_height(game.sysContent.bmpMenuIcons);
    ALLEGRO_BITMAP* bmp =
        al_create_sub_bitmap(
            game.sysContent.bmpMenuIcons,
            (iconSize + 1) * (int) icon, 0,
            iconSize, iconSize
        );
    Point iconCenter(
        leftSide ?
        buttonCenter.x - buttonSize.x * 0.5 + buttonSize.y * 0.5 :
        buttonCenter.x + buttonSize.x * 0.5 - buttonSize.y * 0.5,
        buttonCenter.y
    );
    drawBitmapInBox(
        bmp, iconCenter,
        Point(buttonSize.y),
        true, 0.0f, tint
    );
    al_destroy_bitmap(bmp);
}


/**
 * @brief Draws a mob's shadow.
 *
 * @param m mob to draw the shadow for.
 * @param deltaZ The mob is these many units above the floor directly below it.
 * @param shadowStretch How much to stretch the shadow by
 * (used to simulate sun shadow direction casting).
 */
void drawMobShadow(const Mob* m, float deltaZ, float shadowStretch) {
    Point shadowSize = Point(m->radius * 2.2f);
    if(m->rectangularDim.x != 0) {
        shadowSize = m->rectangularDim * 1.1f;
    }
    
    if(shadowStretch <= 0) return;
    
    float diameter = shadowSize.x;
    float shadowX = 0;
    float shadowW =
        diameter + (diameter * shadowStretch * MOB::SHADOW_STRETCH_MULT);
        
    if(game.states.gameplay->dayMinutes < 60 * 12) {
        //Shadows point to the West.
        shadowX = -shadowW + diameter * 0.5;
        shadowX -= shadowStretch * deltaZ * MOB::SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadowX = -(diameter * 0.5);
        shadowX += shadowStretch * deltaZ * MOB::SHADOW_Y_MULT;
    }
    
    if(m->rectangularDim.x != 0) {
        drawBitmap(
            game.sysContent.bmpShadowSquare,
            Point(m->pos.x + shadowX + shadowW / 2, m->pos.y),
            shadowSize,
            m->angle,
            mapAlpha(255 * (1 - shadowStretch))
        );
    } else {
        drawBitmap(
            game.sysContent.bmpShadow,
            Point(m->pos.x + shadowX + shadowW / 2, m->pos.y),
            Point(shadowW, diameter),
            0,
            mapAlpha(255 * (1 - shadowStretch))
        );
    }
}


/**
 * @brief Draws the mouse cursor.
 *
 * @param color Color to tint it with.
 */
void drawMouseCursor(const ALLEGRO_COLOR& color) {
    if(game.mouseCursor.alpha == 0.0f) return;
    
    al_use_transform(&game.identityTransform);
    
    //Cursor trail.
    if(game.options.advanced.drawCursorTrail) {
        size_t anchor = 0;
        
        for(size_t s = 1; s < game.mouseCursor.history.size(); s++) {
            Point anchorDiff =
                game.mouseCursor.history[anchor] -
                game.mouseCursor.history[s];
            if(
                fabs(anchorDiff.x) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF &&
                fabs(anchorDiff.y) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF
            ) {
                continue;
            }
            
            float startRatio =
                anchor / (float) game.mouseCursor.history.size();
            float startThickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * startRatio;
            unsigned char startAlpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * startRatio;
            ALLEGRO_COLOR startColor =
                changeAlpha(color, startAlpha * game.mouseCursor.alpha);
            Point startP1;
            Point startP2;
            
            float endRatio =
                s / (float) GAME::CURSOR_TRAIL_SAVE_N_SPOTS;
            float endThickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * endRatio;
            unsigned char endAlpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * endRatio;
            ALLEGRO_COLOR endColor =
                changeAlpha(color, endAlpha * game.mouseCursor.alpha);
            Point endP1;
            Point endP2;
            
            if(anchor == 0) {
                Point curToNext =
                    game.mouseCursor.history[s] -
                    game.mouseCursor.history[anchor];
                Point curToNextNormal(-curToNext.y, curToNext.x);
                curToNextNormal = normalizeVector(curToNextNormal);
                Point spotOffset = curToNextNormal * startThickness / 2.0f;
                startP1 = game.mouseCursor.history[anchor] - spotOffset;
                startP2 = game.mouseCursor.history[anchor] + spotOffset;
            } else {
                getMiterPoints(
                    game.mouseCursor.history[anchor - 1],
                    game.mouseCursor.history[anchor],
                    game.mouseCursor.history[anchor + 1],
                    -startThickness,
                    &startP1,
                    &startP2,
                    30.0f
                );
            }
            
            if(s == game.mouseCursor.history.size() - 1) {
                Point prevToCur =
                    game.mouseCursor.history[s] -
                    game.mouseCursor.history[anchor];
                Point prevToCurNormal(-prevToCur.y, prevToCur.x);
                prevToCurNormal = normalizeVector(prevToCurNormal);
                Point spotOffset = prevToCurNormal * startThickness / 2.0f;
                endP1 = game.mouseCursor.history[s] - spotOffset;
                endP2 = game.mouseCursor.history[s] + spotOffset;
            } else {
                getMiterPoints(
                    game.mouseCursor.history[s - 1],
                    game.mouseCursor.history[s],
                    game.mouseCursor.history[s + 1],
                    -endThickness,
                    &endP1,
                    &endP2,
                    30.0f
                );
            }
            
            ALLEGRO_VERTEX vertexes[4];
            for(unsigned char v = 0; v < 4; v++) {
                vertexes[v].z = 0.0f;
            }
            
            vertexes[0].x = startP1.x;
            vertexes[0].y = startP1.y;
            vertexes[0].color = startColor;
            vertexes[1].x = startP2.x;
            vertexes[1].y = startP2.y;
            vertexes[1].color = startColor;
            vertexes[2].x = endP1.x;
            vertexes[2].y = endP1.y;
            vertexes[2].color = endColor;
            vertexes[3].x = endP2.x;
            vertexes[3].y = endP2.y;
            vertexes[3].color = endColor;
            
            al_draw_prim(
                vertexes, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
            
            anchor = s;
        }
    }
    
    //Mouse cursor graphic.
    if(game.mouseCursor.onWindow) {
        drawBitmap(
            game.sysContent.bmpMouseCursor,
            game.mouseCursor.winPos,
            getBitmapDimensions(game.sysContent.bmpMouseCursor),
            -(game.timePassed * game.config.aestheticGen.mouseCursorSpinSpeed),
            changeAlpha(color, 255 * game.mouseCursor.alpha)
        );
    }
}


/**
 * @brief Draws an icon representing some control bind.
 *
 * @param font Font to use for the name, if necessary.
 * @param s Info on the player input source.
 * If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param where Center of the place to draw at.
 * @param maxSize Max width or height. Used to compress it if needed.
 * 0 = unlimited.
 * @param tint Color to tint the icon with.
 */
void drawPlayerInputSourceIcon(
    const ALLEGRO_FONT* const font, const Inpution::InputSource& s,
    bool condensed, const Point& where, const Point& maxSize,
    const ALLEGRO_COLOR& tint
) {
    if(tint.a == 0) return;
    
    //Final text color.
    const ALLEGRO_COLOR finalTextColor = {
        BIND_INPUT_ICON::BASE_TEXT_COLOR.r * tint.r,
        BIND_INPUT_ICON::BASE_TEXT_COLOR.g * tint.g,
        BIND_INPUT_ICON::BASE_TEXT_COLOR.b * tint.b,
        BIND_INPUT_ICON::BASE_TEXT_COLOR.a * tint.a,
    };
    
    //Start by getting the icon's info for drawing.
    PLAYER_INPUT_ICON_SHAPE shape;
    PLAYER_INPUT_ICON_SPRITE bitmapSprite;
    string text;
    getPlayerInputIconInfo(
        s, condensed,
        &shape, &bitmapSprite, &text
    );
    
    //If it's a bitmap, just draw it and be done with it.
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int icon_size =
            al_get_bitmap_height(game.sysContent.bmpPlayerInputIcons);
        ALLEGRO_BITMAP* bmp =
            al_create_sub_bitmap(
                game.sysContent.bmpPlayerInputIcons,
                (icon_size + 1) * (int) bitmapSprite, 0,
                icon_size, icon_size
            );
        drawBitmapInBox(bmp, where, maxSize, true, 0.0f, tint);
        al_destroy_bitmap(bmp);
        return;
    }
    
    //The size of the rectangle will depend on the text within.
    int textOx;
    int textOy;
    int textW;
    int textH;
    al_get_text_dimensions(
        font, text.c_str(),
        &textOx, &textOy, &textW, &textH
    );
    float totalWidth =
        std::min(
            (float) (textW + BIND_INPUT_ICON::PADDING * 2),
            (maxSize.x == 0 ? FLT_MAX : maxSize.x)
        );
    float totalHeight =
        std::min(
            (float) (textH + BIND_INPUT_ICON::PADDING * 2),
            (maxSize.y == 0 ? FLT_MAX : maxSize.y)
        );
    //Force it to always be a square or horizontal rectangle. Never vertical.
    totalWidth = std::max(totalWidth, totalHeight);
    
    //Now, draw the rectangle, either sharp or rounded.
    switch(shape) {
    case PLAYER_INPUT_ICON_SHAPE_RECTANGLE: {
        drawTexturedBox(
            where, Point(totalWidth, totalHeight),
            game.sysContent.bmpKeyBox, tint
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_ROUNDED: {
        drawTexturedBox(
            where, Point(totalWidth, totalHeight),
            game.sysContent.bmpButtonBox, tint
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_BITMAP: {
        break;
    }
    }
    
    //And finally, the text inside.
    drawText(
        text, font, where,
        Point(
            (maxSize.x == 0 ? 0 : maxSize.x - BIND_INPUT_ICON::PADDING),
            (maxSize.y == 0 ? 0 : maxSize.y - BIND_INPUT_ICON::PADDING)
        ),
        finalTextColor, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_COMPENSATE_Y_OFFSET
    );
}


/**
 * @brief Draws a sector, but only the texture (no wall shadows).
 *
 * @param sPtr Pointer to the sector.
 * @param where X and Y offset.
 * @param scale Scale the sector by this much.
 * @param opacity Draw the textures at this opacity [0 - 1].
 */
void drawSectorTexture(
    Sector* sPtr, const Point& where, float scale, float opacity
) {
    if(!sPtr) return;
    if(sPtr->isBottomlessPit) return;
    
    unsigned char nTextures = 1;
    Sector* textureSector[2] = {nullptr, nullptr};
    
    if(sPtr->fade) {
        sPtr->getTextureMergeSectors(
            &textureSector[0], &textureSector[1]
        );
        if(!textureSector[0] && !textureSector[1]) {
            //Can't draw this sector.
            return;
        }
        nTextures = 2;
        
    } else {
        textureSector[0] = sPtr;
        
    }
    
    for(unsigned char t = 0; t < nTextures; t++) {
    
        bool drawSector0 = true;
        if(!textureSector[0]) drawSector0 = false;
        else if(textureSector[0]->isBottomlessPit) {
            drawSector0 = false;
        }
        
        if(nTextures == 2 && !drawSector0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        if(!textureSector[t] || textureSector[t]->isBottomlessPit) {
            continue;
        }
        
        size_t nVertexes = sPtr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[nVertexes];
        
        SectorTexture* textureInfoToUse =
            &textureSector[t]->textureInfo;
            
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -textureInfoToUse->translation.x,
            -textureInfoToUse->translation.y,
            1.0f / textureInfoToUse->scale.x,
            1.0f / textureInfoToUse->scale.y,
            -textureInfoToUse->rot
        );
        
        for(size_t v = 0; v < nVertexes; v++) {
        
            const Triangle* tPtr = &sPtr->triangles[floor(v / 3.0)];
            Vertex* vPtr = tPtr->points[v % 3];
            float vx = vPtr->x;
            float vy = vPtr->y;
            
            float alphaMult = 1;
            float brightnessMult = textureSector[t]->brightness / 255.0;
            
            if(t == 1) {
                if(!drawSector0) {
                    alphaMult = 0;
                    for(
                        size_t e = 0; e < textureSector[1]->edges.size(); e++
                    ) {
                        if(
                            textureSector[1]->edges[e]->vertexes[0] == vPtr ||
                            textureSector[1]->edges[e]->vertexes[1] == vPtr
                        ) {
                            alphaMult = 1;
                        }
                    }
                } else {
                    for(
                        size_t e = 0; e < textureSector[0]->edges.size(); e++
                    ) {
                        if(
                            textureSector[0]->edges[e]->vertexes[0] == vPtr ||
                            textureSector[0]->edges[e]->vertexes[1] == vPtr
                        ) {
                            alphaMult = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            if(textureSector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba_f(
                    textureSector[t]->textureInfo.tint.r * brightnessMult,
                    textureSector[t]->textureInfo.tint.g * brightnessMult,
                    textureSector[t]->textureInfo.tint.b * brightnessMult,
                    textureSector[t]->textureInfo.tint.a * alphaMult *
                    opacity
                );
        }
        
        for(size_t v = 0; v < nVertexes; v++) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            textureSector[t] ?
            textureSector[t]->textureInfo.bitmap :
            textureSector[t == 0 ? 1 : 0]->textureInfo.bitmap;
            
        al_draw_prim(
            av, nullptr, tex,
            0, (int) nVertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
}


/**
 * @brief Draws a status effect's bitmap.
 *
 * @param m Mob that has this status effect.
 * @param effects List of bitmap effects to use.
 */
void drawStatusEffectBmp(const Mob* m, BitmapEffect& effects) {
    float statusBmpScale;
    ALLEGRO_BITMAP* statusBmp = m->getStatusBitmap(&statusBmpScale);
    
    if(!statusBmp) return;
    
    drawBitmap(
        statusBmp,
        m->pos,
        Point(m->radius * 2 * statusBmpScale, -1)
    );
}


/**
 * @brief Draws string tokens.
 *
 * @param tokens Vector of tokens to draw.
 * @param textFont Text font.
 * @param inputFont Font for control bind input icons.
 * @param inputCondensed Whether control bind inputs should be condensed.
 * @param where Top-left coordinates to draw at.
 * @param flags Allegro text flags.
 * @param maxSize Maximum width and height of the whole thing.
 * @param scale Scale each token by this amount.
 * @param tint Color to tint the tokens with.
 */
void drawStringTokens(
    const vector<StringToken>& tokens, const ALLEGRO_FONT* const textFont,
    const ALLEGRO_FONT* const inputFont, bool inputCondensed,
    const Point& where, int flags, const Point& maxSize,
    const Point& scale, const ALLEGRO_COLOR& tint
) {
    unsigned int totalWidth = 0;
    float xScale = 1.0f;
    for(size_t t = 0; t < tokens.size(); t++) {
        totalWidth += tokens[t].width;
    }
    if(totalWidth > maxSize.x) {
        xScale = maxSize.x / totalWidth;
    }
    float yScale = 1.0f;
    unsigned int lineHeight = al_get_font_line_height(textFont);
    if(lineHeight > maxSize.y) {
        yScale = maxSize.y / lineHeight;
    }
    
    float startX = where.x;
    if(hasFlag(flags, ALLEGRO_ALIGN_CENTER)) {
        startX -= (totalWidth * xScale) / 2.0f;
    } else if(hasFlag(flags, ALLEGRO_ALIGN_RIGHT)) {
        startX -= totalWidth * xScale;
    }
    
    float caret = startX;
    for(size_t t = 0; t < tokens.size(); t++) {
        float tokenFinalWidth = tokens[t].width * xScale;
        switch(tokens[t].type) {
        case STRING_TOKEN_CHAR: {
            drawText(
                tokens[t].content, textFont, Point(caret, where.y),
                Point(LARGE_FLOAT), tint,
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
                TEXT_SETTING_FLAG_CANT_GROW,
                Point(xScale * scale.x, yScale * scale.y)
            );
            break;
        }
        case STRING_TOKEN_BIND_INPUT: {
            drawPlayerInputSourceIcon(
                inputFont,
                game.controls.findBind(tokens[t].content).inputSource,
                inputCondensed,
                Point(
                    caret + tokenFinalWidth / 2.0f,
                    where.y + maxSize.y / 2.0f
                ),
                Point(tokenFinalWidth * scale.x, maxSize.y * scale.y),
                tint
            );
            break;
        }
        default: {
            break;
        }
        }
        caret += tokenFinalWidth;
    }
}


/**
 * @brief Returns information about how a control bind input icon should
 * be drawn.
 *
 * @param s Info on the player input source.
 * If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param shape The shape is returned here.
 * @param bitmapSprite If it's one of the icons in the control bind
 * input icon spritesheet, the index of the sprite is returned here.
 * @param text The text to be written inside is returned here, or an
 * empty string is returned if there's nothing to write.
 */
void getPlayerInputIconInfo(
    const Inpution::InputSource& s, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* shape,
    PLAYER_INPUT_ICON_SPRITE* bitmapSprite,
    string* text
) {
    *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
    *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_LMB;
    *text = "(NONE)";
    
    if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
    
    //Figure out if it's one of those that has a bitmap icon.
    //If so, just return that.
    if(s.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON) {
        if(s.buttonNr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_LMB;
            return;
        } else if(s.buttonNr == 2) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_RMB;
            return;
        } else if(s.buttonNr == 3) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_MMB;
            return;
        }
    } else if(s.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_MWU;
        return;
    } else if(s.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_MWD;
        return;
    } else if(s.type == Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY) {
        if(s.buttonNr == ALLEGRO_KEY_RIGHT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_RIGHT;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_DOWN) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_DOWN;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_LEFT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_LEFT;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_UP) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_UP;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_BACKSPACE) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_BACKSPACE;
            return;
        } else if(
            condensed &&
            (
                s.buttonNr == ALLEGRO_KEY_LSHIFT ||
                s.buttonNr == ALLEGRO_KEY_RSHIFT
            )
        ) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_SHIFT;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_TAB) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_TAB;
            return;
        } else if(s.buttonNr == ALLEGRO_KEY_ENTER) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_ENTER;
            return;
        }
    } else if(s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG && condensed) {
        if(s.axisNr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_STICK_LEFT;
            return;
        } else if(s.axisNr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_STICK_UP;
            return;
        }
    } else if(s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS && condensed) {
        if(s.axisNr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_STICK_RIGHT;
            return;
        } else if(s.axisNr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmapSprite = PLAYER_INPUT_ICON_SPRITE_STICK_DOWN;
            return;
        }
    }
    
    //Otherwise, use an actual shape and some text inside.
    switch(s.type) {
    case Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY: {
        *shape = PLAYER_INPUT_ICON_SHAPE_RECTANGLE;
        *text = getKeyName(s.buttonNr, condensed);
        break;
        
    } case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG:
    case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(s.deviceNr + 1) +
                " stick " + i2s(s.stickNr + 1);
            if(
                s.axisNr == 0 &&
                s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " left";
            } else if(
                s.axisNr == 0 &&
                s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " right";
            } else if(
                s.axisNr == 1 &&
                s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " up";
            } else if(
                s.axisNr == 1 &&
                s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " down";
            } else {
                *text +=
                    " axis " + i2s(s.axisNr) +
                    (
                        s.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG ?
                        "-" :
                        "+"
                    );
            }
            
        } else {
            *text = "Stick " + i2s(s.stickNr);
        }
        break;
        
    } case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(s.deviceNr + 1) +
                " button " + i2s(s.buttonNr + 1);
        } else {
            *text = i2s(s.buttonNr + 1);
        }
        break;
        
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse button " + i2s(s.buttonNr);
        } else {
            *text = "M" + i2s(s.buttonNr);
        }
        break;
        
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse wheel left";
        } else {
            *text = "MWL";
        }
        break;
        
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT: {
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


/**
 * @brief Returns the width of a control bind input icon, for drawing purposes.
 *
 * @param font Font to use for the name, if necessary.
 * @param s Info on the player input. If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param maxBitmapHeight If bitmap icons need to be condensed vertically
 * to fit a certain space, then their width will be affected too.
 * Specify the maximum height here. Use 0 to indicate no maximum height.
 * @return The width.
 */
float getPlayerInputIconWidth(
    const ALLEGRO_FONT* font, const Inpution::InputSource& s, bool condensed,
    float maxBitmapHeight
) {
    PLAYER_INPUT_ICON_SHAPE shape;
    PLAYER_INPUT_ICON_SPRITE bitmapSprite;
    string text;
    getPlayerInputIconInfo(
        s, condensed,
        &shape, &bitmapSprite, &text
    );
    
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int bmpHeight =
            al_get_bitmap_height(game.sysContent.bmpPlayerInputIcons);
        if(maxBitmapHeight == 0.0f || bmpHeight < maxBitmapHeight) {
            return bmpHeight;
        } else {
            return maxBitmapHeight;
        }
    } else {
        return
            al_get_text_width(font, text.c_str()) +
            BIND_INPUT_ICON::PADDING * 2;
    }
}


/**
 * @brief Draws a rectangular region that is highlighted with an outline
 * and some pulsating inward waves. Used for drawing either on the area
 * or on the radar.
 *
 * @param center Center coordinates of the region.
 * @param size Width and height of the region.
 * @param color Color of the highlight.
 * @param timeSpent Total time spent. Used for animating.
 */
void drawHighlightedRectRegion(
    const Point& center, const Point& size, const ALLEGRO_COLOR& color,
    float timeSpent
) {
    const float CORNER_RADIUS = 2.0f;
    const float DURATION = 3.0f;
    const size_t N_INNER_RECTS = 2;
    const float SIZE_OFFSET = 30.0f;
    const float THICKNESS = 4.0f;
    
    //Outer rectangle.
    drawRoundedRectangle(center, size, CORNER_RADIUS, color, THICKNESS);
    
    //Inner rectangles.
    for(size_t i = 0; i < N_INNER_RECTS; i++) {
        float iTotalTime = timeSpent + (DURATION / (float) N_INNER_RECTS) * i;
        float iAnimTime = fmod(iTotalTime, DURATION);
        Point iSize =
            interpolatePoint(
                iAnimTime, 0.0f, DURATION, size, size - SIZE_OFFSET
            );
        float alpha =
            interpolateNumber(
                iAnimTime, 0.0f, DURATION, 1.0f, 0.0f
            );
            
        drawRoundedRectangle(
            center, iSize, CORNER_RADIUS, multAlpha(color, alpha), THICKNESS
        );
    }
}
