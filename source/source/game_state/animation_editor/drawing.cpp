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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"



/**
 * @brief Handles the drawing part of the main loop of the animation editor.
 */
void AnimationEditor::doDrawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    drawOpErrorCursor();
}


/**
 * @brief Draw the canvas.
 *
 * This is called as a callback inside the Dear ImGui rendering process.
 */
void AnimationEditor::drawCanvas() {
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y, game.editorsView.size.x, game.editorsView.size.y
    );
    
    if(useBg && bg) {
        Point textureTL = canvasTL;
        Point textureBR = canvasBR;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &textureTL.x, &textureTL.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &textureBR.x, &textureBR.y
        );
        ALLEGRO_VERTEX bgVertexes[4];
        for(size_t v = 0; v < 4; v++) {
            bgVertexes[v].z = 0;
            bgVertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bgVertexes[0].x = canvasTL.x;
        bgVertexes[0].y = canvasTL.y;
        bgVertexes[0].u = textureTL.x;
        bgVertexes[0].v = textureTL.y;
        //Top-right vertex.
        bgVertexes[1].x = canvasBR.x;
        bgVertexes[1].y = canvasTL.y;
        bgVertexes[1].u = textureBR.x;
        bgVertexes[1].v = textureTL.y;
        //Bottom-right vertex.
        bgVertexes[2].x = canvasBR.x;
        bgVertexes[2].y = canvasBR.y;
        bgVertexes[2].u = textureBR.x;
        bgVertexes[2].v = textureBR.y;
        //Bottom-left vertex.
        bgVertexes[3].x = canvasTL.x;
        bgVertexes[3].y = canvasBR.y;
        bgVertexes[3].u = textureTL.x;
        bgVertexes[3].v = textureBR.y;
        
        al_draw_prim(
            bgVertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
    Sprite* s = nullptr;
    
    if(state == EDITOR_STATE_ANIMATION && curAnimInst.validFrame()) {
        s = curAnimInst.curAnim->frames[curAnimInst.curFrameIdx].spritePtr;
        
    } else if(
        state == EDITOR_STATE_SPRITE || state == EDITOR_STATE_TOP ||
        state == EDITOR_STATE_HITBOXES ||
        state == EDITOR_STATE_SPRITE_BITMAP ||
        state == EDITOR_STATE_SPRITE_TRANSFORM
    ) {
        s = curSprite;
        
    }
    
    bool drawHitboxes = hitboxesVisible;
    bool drawMobRadius = mobRadiusVisible;
    bool drawLeaderSilhouette = leaderSilhouetteVisible;
    float gridOpacity = gridVisible ? 0.33f : 0.0f;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_TOP) {
        drawHitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        gridOpacity = 0.0f;
        drawMobRadius = false;
        drawLeaderSilhouette = false;
        
        if(s && s->parentBmp) {
            int bmpW = al_get_bitmap_width(s->parentBmp);
            int bmpH = al_get_bitmap_height(s->parentBmp);
            int bmpX = -bmpW / 2.0;
            int bmpY = -bmpH / 2.0;
            al_draw_bitmap(s->parentBmp, bmpX, bmpY, 0);
            
            Point sceneTL = Point(-1.0f);
            Point sceneBR = Point(canvasBR.x + 1, canvasBR.y + 1);
            al_transform_coordinates(
                &game.editorsView.windowToWorldTransform, &sceneTL.x, &sceneTL.y
            );
            al_transform_coordinates(
                &game.editorsView.windowToWorldTransform, &sceneBR.x, &sceneBR.y
            );
            
            for(unsigned char x = 0; x < 3; x++) {
                Point recTL, recBR;
                switch(x) {
                case 0: {
                    recTL.x = sceneTL.x;
                    recBR.x = bmpX + s->bmpPos.x;
                    break;
                } case 1: {
                    recTL.x = bmpX + s->bmpPos.x;
                    recBR.x = bmpX + s->bmpPos.x + s->bmpSize.x;
                    break;
                } default: {
                    recTL.x = bmpX + s->bmpPos.x + s->bmpSize.x;
                    recBR.x = sceneBR.x;
                    break;
                }
                }
                
                for(unsigned char y = 0; y < 3; y++) {
                    if(x == 1 && y == 1) continue;
                    
                    switch(y) {
                    case 0: {
                        recTL.y = sceneTL.y;
                        recBR.y = bmpY + s->bmpPos.y;
                        break;
                    } case 1: {
                        recTL.y = bmpY + s->bmpPos.y;
                        recBR.y = bmpY + s->bmpPos.y + s->bmpSize.y;
                        break;
                    } default: {
                        recTL.y = bmpY + s->bmpPos.y + s->bmpSize.y;
                        recBR.y = sceneBR.y;
                        break;
                    }
                    }
                    
                    al_draw_filled_rectangle(
                        recTL.x, recTL.y,
                        recBR.x, recBR.y,
                        al_map_rgba(0, 0, 0, 128)
                    );
                }
            }
            
            if(s->bmpSize.x > 0 && s->bmpSize.y > 0) {
            
                unsigned char outlineAlpha =
                    255 * ((sin(curHitboxAlpha) / 2.0) + 0.5);
                al_draw_rectangle(
                    bmpX + s->bmpPos.x + 0.5,
                    bmpY + s->bmpPos.y + 0.5,
                    bmpX + s->bmpPos.x + s->bmpSize.x - 0.5,
                    bmpY + s->bmpPos.y + s->bmpSize.y - 0.5,
                    al_map_rgba(224, 192, 0, outlineAlpha), 1.0
                );
            }
        }
        
    } else if(s) {
    
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            drawSideViewSprite(s);
        } else {
            drawTopDownViewSprite(s);
        }
        
        if(drawHitboxes) {
            unsigned char hitboxOutlineAlpha =
                63 + 192 * ((sin(curHitboxAlpha) / 2.0) + 0.5);
            size_t nHitboxes = s->hitboxes.size();
            
            for(int h = (int) nHitboxes - 1; h >= 0; --h) {
                //Iterate the hitboxes in reverse order, since this is
                //the order of priority the engine has when checking for
                //collisions. Making higher priority hitboxes appear above
                //lower ones makes it all more intuitive and cohesive.
                Hitbox* hPtr = &s->hitboxes[h];
                ALLEGRO_COLOR hitboxColor, hitboxOutlineColor;
                float hitboxOutlineThickness = 2.0f / game.editorsView.cam.zoom;
                
                switch(hPtr->type) {
                case HITBOX_TYPE_NORMAL: {
                    hitboxColor = al_map_rgba(0, 128, 0, 128);
                    hitboxOutlineColor = al_map_rgba(0, 64, 0, 255);
                    break;
                } case HITBOX_TYPE_ATTACK: {
                    hitboxColor = al_map_rgba(128, 0, 0, 128);
                    hitboxOutlineColor = al_map_rgba(64, 0, 0, 255);
                    break;
                } case HITBOX_TYPE_DISABLED: {
                    hitboxColor = al_map_rgba(128, 128, 0, 128);
                    hitboxOutlineColor = al_map_rgba(64, 64, 0, 255);
                    break;
                }
                }
                
                if(
                    curHitboxIdx == (size_t) h &&
                    state == EDITOR_STATE_HITBOXES
                ) {
                    hitboxOutlineThickness =
                        3.0f / game.editorsView.cam.zoom;
                    hitboxOutlineColor =
                        changeAlpha(hitboxColor, hitboxOutlineAlpha);
                }
                
                if(sideView && state == EDITOR_STATE_HITBOXES) {
                    drawSideViewHitbox(
                        hPtr, hitboxColor,
                        hitboxOutlineColor, hitboxOutlineThickness
                    );
                } else {
                    drawTopDownViewHitbox(
                        hPtr, hitboxColor,
                        hitboxOutlineColor, hitboxOutlineThickness
                    );
                }
            }
        }
        
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            Point curSpriteSize = curSprite->scale * curSprite->bmpSize;
            curTransformationWidget.draw(
                &curSprite->offset,
                &curSpriteSize,
                &curSprite->angle,
                1.0f / game.editorsView.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_TOP && s->topVisible) {
            curTransformationWidget.draw(
                &s->topPos,
                &s->topSize,
                &s->topAngle,
                1.0f / game.editorsView.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_HITBOXES && curHitbox) {
            if(!sideView) {
                Point hitboxSize(
                    curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                );
                curTransformationWidget.draw(
                    &curHitbox->pos,
                    &hitboxSize,
                    nullptr,
                    1.0f / game.editorsView.cam.zoom
                );
            } else if(curHitbox->height != 0.0f) {
                Point hitboxCenter(
                    curHitbox->pos.x,
                    (-(curHitbox->height / 2.0f)) - curHitbox->z
                );
                Point hitboxSize(
                    curHitbox->radius * 2.0f, curHitbox->height
                );
                curTransformationWidget.draw(
                    &hitboxCenter,
                    &hitboxSize,
                    nullptr,
                    1.0f / game.editorsView.cam.zoom
                );
            }
            
        }
    }
    
    //Grid.
    if(gridOpacity != 0.0f) {
    
        drawGrid(
            ANIM_EDITOR::GRID_INTERVAL,
            al_map_rgba(64, 64, 64, gridOpacity * 255),
            al_map_rgba(48, 48, 48, gridOpacity * 255)
        );
        
        Point camTLCorner(0, 0);
        Point camBRCorner(canvasBR.x, canvasBR.y);
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camTLCorner.x, &camTLCorner.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camBRCorner.x, &camBRCorner.y
        );
        
        al_draw_line(
            0, camTLCorner.y, 0, camBRCorner.y,
            al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
        );
        al_draw_line(
            camTLCorner.x, 0, camBRCorner.x, 0,
            al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
        );
    }
    
    if(drawMobRadius && loadedMobType) {
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            //The radius isn't meant to be shown in side view.
        } else {
            drawTopDownViewMobRadius(loadedMobType);
        }
    }
    
    if(drawLeaderSilhouette) {
        float xOffset = 32;
        if(loadedMobType) {
            xOffset += loadedMobType->radius;
        }
        
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            drawSideViewLeaderSilhouette(xOffset);
        } else {
            drawTopDownViewLeaderSilhouette(xOffset);
        }
    }
    
    if(state == EDITOR_STATE_ANIMATION) {
        al_use_transform(&game.identityTransform);
        drawTimeline();
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}


/**
 * @brief Draws the comparison sprite on the canvas, all tinted and everything.
 */
void AnimationEditor::drawComparison() {
    if(
        comparison && comparisonBlinkShow &&
        comparisonSprite && comparisonSprite->bitmap
    ) {
        ALLEGRO_COLOR tint;
        if(comparisonTint) {
            tint = al_map_rgb(255, 128, 0);
        } else {
            tint = comparisonSprite->tint;
        }
        drawBitmap(
            comparisonSprite->bitmap,
            comparisonSprite->offset,
            Point(
                comparisonSprite->bmpSize.x * comparisonSprite->scale.x,
                comparisonSprite->bmpSize.y * comparisonSprite->scale.y
            ),
            comparisonSprite->angle, tint
        );
    }
}


/**
 * @brief Draws a hitbox on the canvas in the sideways view.
 *
 * @param hPtr Hitbox to draw.
 * @param color Color to use for the hitbox's main shape.
 * @param outlineColor Color to use for the hitbox's outline.
 * @param outlineThickness Thickness of the hitbox's outline.
 */
void AnimationEditor::drawSideViewHitbox(
    Hitbox* hPtr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outlineColor, float outlineThickness
) {
    float dummy = 0;
    float zToUse = hPtr->z;
    float hToUse = hPtr->height;
    
    if(hPtr->height == 0) {
        //Set the coordinates to the window top and window bottom. Add some
        //padding just to make sure.
        zToUse = game.winH + 1;
        hToUse = 0 - 1;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &dummy, &zToUse
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &dummy, &hToUse
        );
        //The height is the height from the top of the window to the bottom.
        hToUse = zToUse - hToUse;
        //Z needs to be flipped.
        zToUse = -zToUse;
    }
    
    al_draw_filled_rectangle(
        hPtr->pos.x - hPtr->radius,
        -zToUse,
        hPtr->pos.x + hPtr->radius,
        -zToUse - hToUse,
        color
    );
    
    al_draw_rectangle(
        hPtr->pos.x - hPtr->radius,
        -zToUse,
        hPtr->pos.x + hPtr->radius,
        -zToUse - hToUse,
        outlineColor, outlineThickness
    );
}


/**
 * @brief Draws a leader's silhouette on the canvas in the sideways view.
 *
 * @param xOffset From the center, offset the silhouette this much
 * to the right.
 */
void AnimationEditor::drawSideViewLeaderSilhouette(float xOffset) {
    drawBitmap(
        game.sysContent.bmpLeaderSilhouetteSide,
        Point(xOffset, -game.config.leaders.standardHeight / 2.0),
        Point(-1, game.config.leaders.standardHeight),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/**
 * @brief Draws a sprite on the canvas in the sideways view.
 *
 * @param s Sprite to draw.
 */
void AnimationEditor::drawSideViewSprite(const Sprite* s) {
    Point min, max;
    ALLEGRO_COLOR color = COLOR_EMPTY;
    
    getTransformedRectangleBBox(
        s->offset, s->bmpSize * s->scale, s->angle,
        &min, &max
    );
    max.y = 0; //Bottom aligns with the floor.
    
    if(loadedMobType) {
        color = loadedMobType->mainColor;
        min.y = loadedMobType->height;
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
void AnimationEditor::drawTimeline() {
    if(!curAnimInst.validFrame()) return;
    
    //Some initial calculations.
    float animTotalDuration = 0;
    float animCurTime = 0;
    float animLoopTime = 0;
    for(size_t f = 0; f < curAnimInst.curAnim->frames.size(); f++) {
        float fDur = curAnimInst.curAnim->frames[f].duration;
        
        if(f < curAnimInst.curFrameIdx) {
            animCurTime += fDur;
        } else if(f == curAnimInst.curFrameIdx) {
            animCurTime += curAnimInst.curFrameTime;
        }
        
        if(f < curAnimInst.curAnim->loopFrame) {
            animLoopTime += fDur;
        }
        
        animTotalDuration += fDur;
    }
    if(animTotalDuration == 0.0f) return;
    
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    float scale =
        (canvasBR.x - canvasTL.x - ANIM_EDITOR::TIMELINE_PADDING * 2.0f) /
        animTotalDuration;
    float milestoneInterval = 32.0f / scale;
    milestoneInterval = floor(milestoneInterval * 100.0f) / 100.0f;
    milestoneInterval = std::max(milestoneInterval, 0.01f);
    
    //Draw the entire timeline's rectangle.
    al_draw_filled_rectangle(
        canvasTL.x, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        canvasBR.x, canvasBR.y,
        al_map_rgb(160, 180, 160)
    );
    
    //Draw every frame as a rectangle.
    float frameRectanglesCurX = canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
    float frameRectangleTop =
        canvasBR.y -
        ANIM_EDITOR::TIMELINE_HEIGHT + ANIM_EDITOR::TIMELINE_HEADER_HEIGHT;
    float frameRectangleBottom =
        canvasBR.y - ANIM_EDITOR::TIMELINE_PADDING;
    for(size_t f = 0; f < curAnimInst.curAnim->frames.size(); f++) {
        float endX =
            frameRectanglesCurX +
            curAnimInst.curAnim->frames[f].duration * scale;
        ALLEGRO_COLOR color =
            f % 2 == 0 ?
            al_map_rgb(128, 132, 128) :
            al_map_rgb(148, 152, 148);
            
        al_draw_filled_rectangle(
            frameRectanglesCurX, frameRectangleTop,
            endX, frameRectangleBottom,
            color
        );
        frameRectanglesCurX = endX;
    }
    
    //Draw a triangle for the start of the loop frame.
    if(animTotalDuration) {
        float loopX =
            canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING +
            animLoopTime * scale;
        al_draw_filled_triangle(
            loopX,
            frameRectangleBottom,
            loopX,
            frameRectangleBottom - ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            loopX + ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            frameRectangleBottom,
            al_map_rgb(64, 64, 96)
        );
    }
    
    //Draw a line indicating where we are in the animation.
    float curTimeLineX =
        canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING + animCurTime * scale;
    al_draw_line(
        curTimeLineX, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        curTimeLineX, canvasBR.y,
        al_map_rgb(128, 48, 48), 2.0f
    );
    
    //Draw the milestone markers.
    float nextMarkerX = 0.0f;
    unsigned char nextMarkerType = 0;
    
    while(
        nextMarkerX <
        canvasBR.x - canvasTL.x - ANIM_EDITOR::TIMELINE_PADDING * 2
    ) {
        float xToUse =
            nextMarkerX + canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
        switch(nextMarkerType) {
        case 0: {
            string text = f2s(nextMarkerX / scale);
            if(text.size() >= 4) {
                text = text.substr(1, 3);
            }
            drawText(
                text, game.sysContent.fntBuiltin,
                Point(
                    floor(xToUse) + 2,
                    canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT + 2
                ),
                Point(LARGE_FLOAT, 8.0f), al_map_rgb(32, 32, 32),
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
            );
            al_draw_line(
                xToUse + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 1:
        case 3: {
            al_draw_line(
                xToUse + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5,
                canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.66f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 2: {
            al_draw_line(
                xToUse + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5,
                canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.33f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        }
        }
        
        nextMarkerX += scale * milestoneInterval;
        nextMarkerType = sumAndWrap(nextMarkerType, 1, 4);
    }
}


/**
 * @brief Draws a hitbox on the canvas in the standard top-down view.
 *
 * @param hPtr Hitbox to draw.
 * @param color Color of the hitbox's main shape.
 * @param outlineColor Color of the hitbox's outline.
 * @param outlineThickness Thickness of the hitbox's outline.
 */
void AnimationEditor::drawTopDownViewHitbox(
    Hitbox* hPtr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outlineColor, float outlineThickness
) {
    if(hPtr->radius <= 0) return;
    
    al_draw_filled_circle(
        hPtr->pos.x, hPtr->pos.y, hPtr->radius, color
    );
    
    al_draw_circle(
        hPtr->pos.x, hPtr->pos.y,
        hPtr->radius, outlineColor, outlineThickness
    );
}


/**
 * @brief Draws a leader silhouette on the canvas in the standard top-down view.
 *
 * @param xOffset From the center, offset the leader silhouette this much
 * to the right.
 */
void AnimationEditor::drawTopDownViewLeaderSilhouette(
    float xOffset
) {
    drawBitmap(
        game.sysContent.bmpLeaderSilhouetteTop, Point(xOffset, 0),
        Point(-1, game.config.leaders.standardRadius * 2.0f),
        0, al_map_rgba(240, 240, 240, 160)
    );
}


/**
 * @brief Draws the mob radius on the canvas in the standard top-down view.
 *
 * @param mt Type of the mob to draw.
 */
void AnimationEditor::drawTopDownViewMobRadius(MobType* mt) {
    al_draw_circle(
        0, 0, mt->radius,
        al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
    );
    if(mt->rectangularDim.x != 0) {
        al_draw_rectangle(
            -mt->rectangularDim.x / 2.0, -mt->rectangularDim.y / 2.0,
            mt->rectangularDim.x / 2.0, mt->rectangularDim.y / 2.0,
            al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
        );
    }
}


/**
 * @brief Draws a sprite on the canvas in the standard top-down view.
 *
 * @param s Sprite to draw.
 */
void AnimationEditor::drawTopDownViewSprite(Sprite* s) {
    if(!comparisonAbove) {
        drawComparison();
    }
    
    Sprite* nextS = nullptr;
    float interpolationFactor = 0.0f;
    if(state == EDITOR_STATE_ANIMATION && curAnimInst.validFrame()) {
        curAnimInst.getSpriteData(
            nullptr, &nextS, &interpolationFactor
        );
    }
    
    if(s->bitmap) {
        Point coords;
        float angle;
        Point scale;
        ALLEGRO_COLOR tint;
        
        getSpriteBasicEffects(
            Point(), 0, LARGE_FLOAT, LARGE_FLOAT,
            s, nextS, interpolationFactor,
            &coords, &angle, &scale, &tint
        );
        
        if(
            state == EDITOR_STATE_SPRITE_TRANSFORM &&
            comparison && comparisonTint &&
            comparisonSprite && comparisonSprite->bitmap
        ) {
            tint = al_map_rgb(0, 128, 255);
        }
        drawBitmap(
            s->bitmap, coords,
            Point(
                s->bmpSize.x * scale.x,
                s->bmpSize.y * scale.y
            ),
            angle, tint
        );
    }
    
    if(
        s->topVisible && loadedMobType
        && loadedMobType->category->id == MOB_CATEGORY_PIKMIN
    ) {
        Point coords;
        float angle;
        Point size;
        getSpriteBasicTopEffects(
            s, nextS, interpolationFactor,
            &coords, &angle, &size
        );
        drawBitmap(topBmp[curMaturity], coords, size, angle);
    }
    
    if(comparisonAbove) {
        drawComparison();
    }
}
