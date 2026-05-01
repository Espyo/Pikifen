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
    const ALLEGRO_COLOR BG_COLOR = al_map_rgb(128, 144, 128);
    RectCorners canvasCorners = game.editorsView.getWindowCorners();
    
    al_set_clipping_rectangle(
        canvasCorners.tl.x, canvasCorners.tl.y,
        game.editorsView.windowRect.size.x, game.editorsView.windowRect.size.y
    );
    
    if(useBg && bg) {
        RectCorners textureCorners = canvasCorners;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &textureCorners.tl.x, &textureCorners.tl.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &textureCorners.br.x, &textureCorners.br.y
        );
        ALLEGRO_VERTEX bgVertexes[4];
        for(size_t v = 0; v < 4; v++) {
            bgVertexes[v].z = 0;
            bgVertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bgVertexes[0].x = canvasCorners.tl.x;
        bgVertexes[0].y = canvasCorners.tl.y;
        bgVertexes[0].u = textureCorners.tl.x;
        bgVertexes[0].v = textureCorners.tl.y;
        //Top-right vertex.
        bgVertexes[1].x = canvasCorners.br.x;
        bgVertexes[1].y = canvasCorners.tl.y;
        bgVertexes[1].u = textureCorners.br.x;
        bgVertexes[1].v = textureCorners.tl.y;
        //Bottom-right vertex.
        bgVertexes[2].x = canvasCorners.br.x;
        bgVertexes[2].y = canvasCorners.br.y;
        bgVertexes[2].u = textureCorners.br.x;
        bgVertexes[2].v = textureCorners.br.y;
        //Bottom-left vertex.
        bgVertexes[3].x = canvasCorners.tl.x;
        bgVertexes[3].y = canvasCorners.br.y;
        bgVertexes[3].u = textureCorners.tl.x;
        bgVertexes[3].v = textureCorners.br.y;
        
        al_draw_prim(
            bgVertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(BG_COLOR);
    }
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
    Sprite* s = nullptr;
    
    if(state == EDITOR_STATE_ANIMATION && curAnimInst.validFrame()) {
        s = curAnimInst.curAnim->frames[curAnimInst.curFrameIdx].spritePtr;
        
    } else if(
        state == EDITOR_STATE_SPRITE || state == EDITOR_STATE_PIKMIN_TOP ||
        state == EDITOR_STATE_HITBOXES ||
        state == EDITOR_STATE_SPRITE_BITMAP ||
        state == EDITOR_STATE_SPRITE_TRANSFORM
    ) {
        s = curSprite;
        
    }
    
    bool drawHitboxes = hitboxesVisible;
    bool drawMobRadius = mobRadiusVisible;
    bool drawLeaderSilhouette = leaderSilhouetteVisible;
    float gridAlpha = gridVisible ? 0.33f : 0.0f;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_PIKMIN_TOP) {
        drawHitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        const ALLEGRO_COLOR UNSELECTED_COLOR = al_map_rgba(0, 0, 0, 128);
        gridAlpha = 0.0f;
        drawMobRadius = false;
        drawLeaderSilhouette = false;
        
        if(s && s->parentBmp) {
            int bmpW = al_get_bitmap_width(s->parentBmp);
            int bmpH = al_get_bitmap_height(s->parentBmp);
            int bmpX = -bmpW / 2.0;
            int bmpY = -bmpH / 2.0;
            al_draw_bitmap(s->parentBmp, bmpX, bmpY, 0);
            
            RectCorners sceneCorners(
                Point(-1.0f),
                Point(canvasCorners.br.x + 1, canvasCorners.br.y + 1)
            );
            al_transform_coordinates(
                &game.editorsView.windowToWorldTransform,
                &sceneCorners.tl.x, &sceneCorners.tl.y
            );
            al_transform_coordinates(
                &game.editorsView.windowToWorldTransform,
                &sceneCorners.br.x, &sceneCorners.br.y
            );
            
            //Draw the darkening effect.
            for(unsigned char x = 0; x < 3; x++) {
                RectCorners effCorners;
                switch(x) {
                case 0: {
                    effCorners.tl.x = sceneCorners.tl.x;
                    effCorners.br.x = bmpX + s->bmpPos.x;
                    break;
                } case 1: {
                    effCorners.tl.x = bmpX + s->bmpPos.x;
                    effCorners.br.x = bmpX + s->bmpPos.x + s->bmpSize.x;
                    break;
                } default: {
                    effCorners.tl.x = bmpX + s->bmpPos.x + s->bmpSize.x;
                    effCorners.br.x = sceneCorners.br.x;
                    break;
                }
                }
                
                for(unsigned char y = 0; y < 3; y++) {
                    if(x == 1 && y == 1) continue;
                    
                    switch(y) {
                    case 0: {
                        effCorners.tl.y = sceneCorners.tl.y;
                        effCorners.br.y = bmpY + s->bmpPos.y;
                        break;
                    } case 1: {
                        effCorners.tl.y = bmpY + s->bmpPos.y;
                        effCorners.br.y = bmpY + s->bmpPos.y + s->bmpSize.y;
                        break;
                    } default: {
                        effCorners.tl.y = bmpY + s->bmpPos.y + s->bmpSize.y;
                        effCorners.br.y = sceneCorners.br.y;
                        break;
                    }
                    }
                    
                    al_draw_filled_rectangle(
                        effCorners.tl.x, effCorners.tl.y,
                        effCorners.br.x, effCorners.br.y,
                        UNSELECTED_COLOR
                    );
                }
            }
            
            //Draw the separation outline.
            if(s->bmpSize.x > 0 && s->bmpSize.y > 0) {
                ALLEGRO_COLOR color = getSelectionEffectOverlayColor();
                al_draw_rectangle(
                    bmpX + s->bmpPos.x + 0.5f,
                    bmpY + s->bmpPos.y + 0.5f,
                    bmpX + s->bmpPos.x + s->bmpSize.x - 0.5f,
                    bmpY + s->bmpPos.y + s->bmpSize.y - 0.5f,
                    color, 1.0f
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
            size_t nHitboxes = s->hitboxes.size();
            
            for(int h = (int) nHitboxes - 1; h >= 0; --h) {
                //Iterate the hitboxes in reverse order, since this is
                //the order of priority the engine has when checking for
                //collisions. Making higher priority hitboxes appear above
                //lower ones makes it all more intuitive and cohesive.
                Hitbox* hPtr = &s->hitboxes[h];
                bool isSelected =
                    hitboxSelection.contains(h) &&
                    state == EDITOR_STATE_HITBOXES;
                    
                ALLEGRO_COLOR hitboxColor, hitboxOutlineColor;
                float hitboxOutlineThickness = 2.0f / game.editorsView.cam.zoom;
                
                switch(hPtr->type) {
                case HITBOX_TYPE_NORMAL: {
                    hitboxColor = DRAWING::HITBOX_COLOR_NORMAL;
                    hitboxOutlineColor = DRAWING::HITBOX_OUTLINE_COLOR_NORMAL;
                    break;
                } case HITBOX_TYPE_ATTACK: {
                    hitboxColor = DRAWING::HITBOX_COLOR_ATTACK;
                    hitboxOutlineColor = DRAWING::HITBOX_OUTLINE_COLOR_ATTACK;
                    break;
                } case HITBOX_TYPE_DISABLED: {
                    hitboxColor = DRAWING::HITBOX_COLOR_DISABLED;
                    hitboxOutlineColor = DRAWING::HITBOX_OUTLINE_COLOR_DISABLED;
                    break;
                }
                }
                if(isSelected) {
                    hitboxOutlineColor =
                        getSelectionEffectReplacementColor(hitboxOutlineColor);
                    hitboxOutlineThickness =
                        3.0f / game.editorsView.cam.zoom;
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
        
        drawSelectionAndTransformationThings(
            hitboxSelCtrl, curTransformationWidget, false
        );
        
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            Point curSpriteSize =
                curSprite->tf.scale * curSprite->bmpSize;
            curTransformationWidget.draw(
                game.editorsView.mouseCursorWorldPos,
                &curSprite->tf.trans,
                &curSpriteSize,
                &curSprite->tf.rot,
                1.0f / game.editorsView.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_PIKMIN_TOP && s->topVisible) {
            curTransformationWidget.draw(
                game.editorsView.mouseCursorWorldPos,
                &s->topPose.pos,
                &s->topPose.size,
                &s->topPose.angle,
                1.0f / game.editorsView.cam.zoom
            );
            
        }
    }
    
    //Grid.
    if(gridAlpha != 0.0f) {
    
        drawGrid(
            ANIM_EDITOR::GRID_INTERVAL,
            multAlpha(EDITOR::GRID_COLOR_MAJOR, gridAlpha),
            multAlpha(EDITOR::GRID_COLOR_MINOR, gridAlpha)
        );
        
        RectCorners camCorners(Point(0.0f), canvasCorners.br);
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camCorners.tl.x, &camCorners.tl.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camCorners.br.x, &camCorners.br.y
        );
        
        al_draw_line(
            0, camCorners.tl.y, 0, camCorners.br.y,
            EDITOR::GRID_COLOR_ORIGIN, 1.0f / game.editorsView.cam.zoom
        );
        al_draw_line(
            camCorners.tl.x, 0, camCorners.br.x, 0,
            EDITOR::GRID_COLOR_ORIGIN, 1.0f / game.editorsView.cam.zoom
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
    const ALLEGRO_COLOR COMPARISON_TINT = al_map_rgb(255, 128, 0);
    if(
        comparison && comparisonBlinkShow &&
        comparisonSprite && comparisonSprite->bitmap
    ) {
        ALLEGRO_COLOR tint;
        if(comparisonTint) {
            tint = COMPARISON_TINT;
        } else {
            tint = comparisonSprite->tint;
        }
        drawBitmap(
            comparisonSprite->bitmap,
            comparisonSprite->tf.trans,
            Point(
                comparisonSprite->bmpSize.x * comparisonSprite->tf.scale.x,
                comparisonSprite->bmpSize.y * comparisonSprite->tf.scale.y
            ),
            comparisonSprite->tf.rot, tint
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
    Hitbox* hPtr, const ALLEGRO_COLOR& color,
    const ALLEGRO_COLOR& outlineColor, float outlineThickness
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
        hPtr->center.x - hPtr->radius,
        -zToUse,
        hPtr->center.x + hPtr->radius,
        -zToUse - hToUse,
        color
    );
    
    al_draw_rectangle(
        hPtr->center.x - hPtr->radius,
        -zToUse,
        hPtr->center.x + hPtr->radius,
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
        0, EDITOR::SILHOUETTE_COLOR
    );
}


/**
 * @brief Draws a sprite on the canvas in the sideways view.
 *
 * @param s Sprite to draw.
 */
void AnimationEditor::drawSideViewSprite(const Sprite* s) {
    const ALLEGRO_COLOR DEF_COLOR = al_map_rgb(128, 32, 128);
    
    ALLEGRO_COLOR color = COLOR_EMPTY;
    RectCorners corners =
        getTransformedRectangleBBox(
            Rect(s->tf.trans, s->bmpSize * s->tf.scale), s->tf.rot
        );
    corners.br.y = 0; //Bottom aligns with the floor.
    
    if(loadedMobType) {
        color = loadedMobType->mainColor;
        corners.tl.y = loadedMobType->height;
    } else {
        corners.tl.y = corners.br.x - corners.tl.x;
    }
    if(color.a == 0) {
        color = DEF_COLOR;
    }
    corners.tl.y = -corners.tl.y; //Up is negative Y.
    al_draw_filled_rectangle(
        corners.tl.x, corners.tl.y, corners.br.x, corners.br.y, color
    );
}


/**
 * @brief Draws a timeline for the current animation.
 */
void AnimationEditor::drawTimeline() {
    const ALLEGRO_COLOR TIMELINE_COLOR = al_map_rgb(160, 180, 160);
    const ALLEGRO_COLOR FRAME_EVEN_COLOR = al_map_rgb(128, 132, 128);
    const ALLEGRO_COLOR FRAME_ODD_COLOR = al_map_rgb(148, 152, 148);
    const ALLEGRO_COLOR LOOP_FRAME_COLOR = al_map_rgb(64, 64, 96);
    const ALLEGRO_COLOR HEAD_COLOR = al_map_rgb(128, 48, 48);
    const ALLEGRO_COLOR MILESTONE_COLOR = al_map_rgb(32, 32, 32);
    
    if(!curAnimInst.validFrame()) return;
    
    //Some initial calculations.
    float animTotalDuration = 0;
    float animCurTime = 0;
    float animLoopTime = 0;
    forIdx(f, curAnimInst.curAnim->frames) {
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
    
    RectCorners canvasCorners = game.editorsView.getWindowCorners();
    float scale =
        (
            canvasCorners.br.x - canvasCorners.tl.x -
            ANIM_EDITOR::TIMELINE_PADDING * 2.0f
        ) /
        animTotalDuration;
    float milestoneInterval = 32.0f / scale;
    milestoneInterval = floor(milestoneInterval * 100.0f) / 100.0f;
    milestoneInterval = std::max(milestoneInterval, 0.01f);
    
    //Draw the entire timeline's rectangle.
    al_draw_filled_rectangle(
        canvasCorners.tl.x, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        canvasCorners.br.x, canvasCorners.br.y, TIMELINE_COLOR
    );
    
    //Draw every frame as a rectangle.
    float frameRectanglesCurX = canvasCorners.tl.x + ANIM_EDITOR::TIMELINE_PADDING;
    float frameRectangleTop =
        canvasCorners.br.y -
        ANIM_EDITOR::TIMELINE_HEIGHT + ANIM_EDITOR::TIMELINE_HEADER_HEIGHT;
    float frameRectangleBottom =
        canvasCorners.br.y - ANIM_EDITOR::TIMELINE_PADDING;
    forIdx(f, curAnimInst.curAnim->frames) {
        float endX =
            frameRectanglesCurX +
            curAnimInst.curAnim->frames[f].duration * scale;
        ALLEGRO_COLOR color =
            f % 2 == 0 ? FRAME_EVEN_COLOR : FRAME_ODD_COLOR;
            
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
            canvasCorners.tl.x + ANIM_EDITOR::TIMELINE_PADDING +
            animLoopTime * scale;
        al_draw_filled_triangle(
            loopX,
            frameRectangleBottom,
            loopX,
            frameRectangleBottom - ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            loopX + ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            frameRectangleBottom,
            LOOP_FRAME_COLOR
        );
    }
    
    //Draw a line indicating where we are in the animation.
    float curTimeLineX =
        canvasCorners.tl.x + ANIM_EDITOR::TIMELINE_PADDING + animCurTime * scale;
    al_draw_line(
        curTimeLineX, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        curTimeLineX, canvasCorners.br.y,
        HEAD_COLOR, 2.0f
    );
    
    //Draw the milestone markers.
    float nextMarkerX = 0.0f;
    unsigned char nextMarkerType = 0;
    
    while(
        nextMarkerX <
        canvasCorners.br.x - canvasCorners.tl.x - ANIM_EDITOR::TIMELINE_PADDING * 2
    ) {
        float xToUse =
            nextMarkerX + canvasCorners.tl.x + ANIM_EDITOR::TIMELINE_PADDING;
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
                    canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT + 2
                ),
                Point(LARGE_FLOAT, 8.0f), MILESTONE_COLOR,
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
            );
            al_draw_line(
                xToUse + 0.5, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT,
                MILESTONE_COLOR, 1.0f
            );
            break;
            
        } case 1:
        case 3: {
            al_draw_line(
                xToUse + 0.5, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5,
                canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.66f,
                MILESTONE_COLOR, 1.0f
            );
            break;
            
        } case 2: {
            al_draw_line(
                xToUse + 0.5, canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                xToUse + 0.5,
                canvasCorners.br.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.33f,
                MILESTONE_COLOR, 1.0f
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
    Hitbox* hPtr, const ALLEGRO_COLOR& color,
    const ALLEGRO_COLOR& outlineColor, float outlineThickness
) {
    if(hPtr->radius <= 0) return;
    
    al_draw_filled_circle(
        hPtr->center.x, hPtr->center.y, hPtr->radius, color
    );
    
    al_draw_circle(
        hPtr->center.x, hPtr->center.y,
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
        0, EDITOR::SILHOUETTE_COLOR
    );
}


/**
 * @brief Draws the mob radius on the canvas in the standard top-down view.
 *
 * @param mt Type of the mob to draw.
 */
void AnimationEditor::drawTopDownViewMobRadius(MobType* mt) {
    const ALLEGRO_COLOR COLOR = al_map_rgb(240, 240, 240);
    al_draw_circle(
        0, 0, mt->radius, COLOR, 1.0f / game.editorsView.cam.zoom
    );
    if(mt->rectangularDim.x != 0) {
        al_draw_rectangle(
            -mt->rectangularDim.x / 2.0, -mt->rectangularDim.y / 2.0,
            mt->rectangularDim.x / 2.0, mt->rectangularDim.y / 2.0,
            COLOR, 1.0f / game.editorsView.cam.zoom
        );
    }
}


/**
 * @brief Draws a sprite on the canvas in the standard top-down view.
 *
 * @param s Sprite to draw.
 */
void AnimationEditor::drawTopDownViewSprite(Sprite* s) {
    const ALLEGRO_COLOR COMPARISON_TINT = al_map_rgb(240, 240, 240);
    
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
            tint = COMPARISON_TINT;
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
        s->topVisible && loadedMobType &&
        (
            loadedMobType->category->id == MOB_CATEGORY_PIKMIN ||
            loadedMobType->category->id == MOB_CATEGORY_LEADERS
        ) &&
        topBmp[curMaturity]
    ) {
        Point coords;
        float angle;
        Point size;
        getSpriteBasicTopEffects(
            s, nextS, interpolationFactor,
            &coords, &angle, &size
        );
        drawBitmap(topBmp[curMaturity], coords, size, angle, topTint);
    }
    
    if(comparisonAbove) {
        drawComparison();
    }
}
