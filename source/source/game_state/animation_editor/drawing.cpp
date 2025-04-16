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
#include "../../core/misc_functions.h"
#include "../../core/game.h"
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
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y,
        canvasBR.x - canvasTL.x, canvasBR.y - canvasTL.y
    );
    
    if(useBg && bg) {
        Point texture_tl = canvasTL;
        Point texture_br = canvasBR;
        al_transform_coordinates(
            &game.cam.screenToWorldTransform, &texture_tl.x, &texture_tl.y
        );
        al_transform_coordinates(
            &game.cam.screenToWorldTransform, &texture_br.x, &texture_br.y
        );
        ALLEGRO_VERTEX bg_vertexes[4];
        for(size_t v = 0; v < 4; v++) {
            bg_vertexes[v].z = 0;
            bg_vertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bg_vertexes[0].x = canvasTL.x;
        bg_vertexes[0].y = canvasTL.y;
        bg_vertexes[0].u = texture_tl.x;
        bg_vertexes[0].v = texture_tl.y;
        //Top-right vertex.
        bg_vertexes[1].x = canvasBR.x;
        bg_vertexes[1].y = canvasTL.y;
        bg_vertexes[1].u = texture_br.x;
        bg_vertexes[1].v = texture_tl.y;
        //Bottom-right vertex.
        bg_vertexes[2].x = canvasBR.x;
        bg_vertexes[2].y = canvasBR.y;
        bg_vertexes[2].u = texture_br.x;
        bg_vertexes[2].v = texture_br.y;
        //Bottom-left vertex.
        bg_vertexes[3].x = canvasTL.x;
        bg_vertexes[3].y = canvasBR.y;
        bg_vertexes[3].u = texture_tl.x;
        bg_vertexes[3].v = texture_br.y;
        
        al_draw_prim(
            bg_vertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.cam.worldToScreenTransform);
    
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
    
    bool draw_hitboxes = hitboxesVisible;
    bool draw_mob_radius = mobRadiusVisible;
    bool draw_leader_silhouette = leaderSilhouetteVisible;
    float grid_opacity = gridVisible ? 0.33f : 0.0f;
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM || state == EDITOR_STATE_TOP) {
        draw_hitboxes = false;
    }
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        grid_opacity = 0.0f;
        draw_mob_radius = false;
        draw_leader_silhouette = false;
        
        if(s && s->parentBmp) {
            int bmp_w = al_get_bitmap_width(s->parentBmp);
            int bmp_h = al_get_bitmap_height(s->parentBmp);
            int bmp_x = -bmp_w / 2.0;
            int bmp_y = -bmp_h / 2.0;
            al_draw_bitmap(s->parentBmp, bmp_x, bmp_y, 0);
            
            Point scene_tl = Point(-1.0f);
            Point scene_br = Point(canvasBR.x + 1, canvasBR.y + 1);
            al_transform_coordinates(
                &game.cam.screenToWorldTransform, &scene_tl.x, &scene_tl.y
            );
            al_transform_coordinates(
                &game.cam.screenToWorldTransform, &scene_br.x, &scene_br.y
            );
            
            for(unsigned char x = 0; x < 3; x++) {
                Point rec_tl, rec_br;
                switch(x) {
                case 0: {
                    rec_tl.x = scene_tl.x;
                    rec_br.x = bmp_x + s->bmpPos.x;
                    break;
                } case 1: {
                    rec_tl.x = bmp_x + s->bmpPos.x;
                    rec_br.x = bmp_x + s->bmpPos.x + s->bmpSize.x;
                    break;
                } default: {
                    rec_tl.x = bmp_x + s->bmpPos.x + s->bmpSize.x;
                    rec_br.x = scene_br.x;
                    break;
                }
                }
                
                for(unsigned char y = 0; y < 3; y++) {
                    if(x == 1 && y == 1) continue;
                    
                    switch(y) {
                    case 0: {
                        rec_tl.y = scene_tl.y;
                        rec_br.y = bmp_y + s->bmpPos.y;
                        break;
                    } case 1: {
                        rec_tl.y = bmp_y + s->bmpPos.y;
                        rec_br.y = bmp_y + s->bmpPos.y + s->bmpSize.y;
                        break;
                    } default: {
                        rec_tl.y = bmp_y + s->bmpPos.y + s->bmpSize.y;
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
            
            if(s->bmpSize.x > 0 && s->bmpSize.y > 0) {
            
                unsigned char outline_alpha =
                    255 * ((sin(curHitboxAlpha) / 2.0) + 0.5);
                al_draw_rectangle(
                    bmp_x + s->bmpPos.x + 0.5,
                    bmp_y + s->bmpPos.y + 0.5,
                    bmp_x + s->bmpPos.x + s->bmpSize.x - 0.5,
                    bmp_y + s->bmpPos.y + s->bmpSize.y - 0.5,
                    al_map_rgba(224, 192, 0, outline_alpha), 1.0
                );
            }
        }
        
    } else if(s) {
    
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            drawSideViewSprite(s);
        } else {
            drawTopDownViewSprite(s);
        }
        
        if(draw_hitboxes) {
            unsigned char hitbox_outline_alpha =
                63 + 192 * ((sin(curHitboxAlpha) / 2.0) + 0.5);
            size_t n_hitboxes = s->hitboxes.size();
            
            for(int h = (int) n_hitboxes - 1; h >= 0; --h) {
                //Iterate the hitboxes in reverse order, since this is
                //the order of priority the engine has when checking for
                //collisions. Making higher priority hitboxes appear above
                //lower ones makes it all more intuitive and cohesive.
                Hitbox* h_ptr = &s->hitboxes[h];
                ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                float hitbox_outline_thickness = 2.0f / game.cam.zoom;
                
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
                
                if(
                    curHitboxIdx == (size_t) h &&
                    state == EDITOR_STATE_HITBOXES
                ) {
                    hitbox_outline_thickness =
                        3.0f / game.cam.zoom;
                    hitbox_outline_color =
                        changeAlpha(hitbox_color, hitbox_outline_alpha);
                }
                
                if(sideView && state == EDITOR_STATE_HITBOXES) {
                    drawSideViewHitbox(
                        h_ptr, hitbox_color,
                        hitbox_outline_color, hitbox_outline_thickness
                    );
                } else {
                    drawTopDownViewHitbox(
                        h_ptr, hitbox_color,
                        hitbox_outline_color, hitbox_outline_thickness
                    );
                }
            }
        }
        
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            Point cur_sprite_size = curSprite->scale * curSprite->bmpSize;
            curTransformationWidget.draw(
                &curSprite->offset,
                &cur_sprite_size,
                &curSprite->angle,
                1.0f / game.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_TOP && s->topVisible) {
            curTransformationWidget.draw(
                &s->topPos,
                &s->topSize,
                &s->topAngle,
                1.0f / game.cam.zoom
            );
            
        } else if(state == EDITOR_STATE_HITBOXES && curHitbox) {
            if(!sideView) {
                Point hitbox_size(
                    curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                );
                curTransformationWidget.draw(
                    &curHitbox->pos,
                    &hitbox_size,
                    nullptr,
                    1.0f / game.cam.zoom
                );
            } else if(curHitbox->height != 0.0f) {
                Point hitbox_center(
                    curHitbox->pos.x,
                    (-(curHitbox->height / 2.0f)) - curHitbox->z
                );
                Point hitbox_size(
                    curHitbox->radius * 2.0f, curHitbox->height
                );
                curTransformationWidget.draw(
                    &hitbox_center,
                    &hitbox_size,
                    nullptr,
                    1.0f / game.cam.zoom
                );
            }
            
        }
    }
    
    //Grid.
    if(grid_opacity != 0.0f) {
    
        drawGrid(
            ANIM_EDITOR::GRID_INTERVAL,
            al_map_rgba(64, 64, 64, grid_opacity * 255),
            al_map_rgba(48, 48, 48, grid_opacity * 255)
        );
        
        Point cam_top_left_corner(0, 0);
        Point cam_bottom_right_corner(canvasBR.x, canvasBR.y);
        al_transform_coordinates(
            &game.cam.screenToWorldTransform,
            &cam_top_left_corner.x, &cam_top_left_corner.y
        );
        al_transform_coordinates(
            &game.cam.screenToWorldTransform,
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
    
    if(draw_mob_radius && loadedMobType) {
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            //The radius isn't meant to be shown in side view.
        } else {
            drawTopDownViewMobRadius(loadedMobType);
        }
    }
    
    if(draw_leader_silhouette) {
        float x_offset = 32;
        if(loadedMobType) {
            x_offset += loadedMobType->radius;
        }
        
        if(sideView && state == EDITOR_STATE_HITBOXES) {
            drawSideViewLeaderSilhouette(x_offset);
        } else {
            drawTopDownViewLeaderSilhouette(x_offset);
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
 * @param h_ptr Hitbox to draw.
 * @param color Color to use for the hitbox's main shape.
 * @param outline_color Color to use for the hitbox's outline.
 * @param outline_thickness Thickness of the hitbox's outline.
 */
void AnimationEditor::drawSideViewHitbox(
    Hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, float outline_thickness
) {
    float dummy = 0;
    float z_to_use = h_ptr->z;
    float h_to_use = h_ptr->height;
    
    if(h_ptr->height == 0) {
        //Set the coordinates to the screen top and screen bottom. Add some
        //padding just to make sure.
        z_to_use = game.winH + 1;
        h_to_use = 0 - 1;
        al_transform_coordinates(
            &game.cam.screenToWorldTransform, &dummy, &z_to_use
        );
        al_transform_coordinates(
            &game.cam.screenToWorldTransform, &dummy, &h_to_use
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


/**
 * @brief Draws a leader's silhouette on the canvas in the sideways view.
 *
 * @param x_offset From the center, offset the silhouette this much
 * to the right.
 */
void AnimationEditor::drawSideViewLeaderSilhouette(float x_offset) {
    drawBitmap(
        game.sysContent.bmpLeaderSilhouetteSide,
        Point(x_offset, -game.config.leaders.standardHeight / 2.0),
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
    float anim_total_duration = 0;
    float anim_cur_time = 0;
    float anim_loop_time = 0;
    for(size_t f = 0; f < curAnimInst.curAnim->frames.size(); f++) {
        float f_dur = curAnimInst.curAnim->frames[f].duration;
        
        if(f < curAnimInst.curFrameIdx) {
            anim_cur_time += f_dur;
        } else if(f == curAnimInst.curFrameIdx) {
            anim_cur_time += curAnimInst.curFrameTime;
        }
        
        if(f < curAnimInst.curAnim->loopFrame) {
            anim_loop_time += f_dur;
        }
        
        anim_total_duration += f_dur;
    }
    if(anim_total_duration == 0.0f) return;
    
    float scale =
        (canvasBR.x - canvasTL.x - ANIM_EDITOR::TIMELINE_PADDING * 2.0f) /
        anim_total_duration;
    float milestone_interval = 32.0f / scale;
    milestone_interval = floor(milestone_interval * 100.0f) / 100.0f;
    milestone_interval = std::max(milestone_interval, 0.01f);
    
    //Draw the entire timeline's rectangle.
    al_draw_filled_rectangle(
        canvasTL.x, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        canvasBR.x, canvasBR.y,
        al_map_rgb(160, 180, 160)
    );
    
    //Draw every frame as a rectangle.
    float frame_rectangles_cur_x = canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
    float frame_rectangle_top =
        canvasBR.y -
        ANIM_EDITOR::TIMELINE_HEIGHT + ANIM_EDITOR::TIMELINE_HEADER_HEIGHT;
    float frame_rectangle_bottom =
        canvasBR.y - ANIM_EDITOR::TIMELINE_PADDING;
    for(size_t f = 0; f < curAnimInst.curAnim->frames.size(); f++) {
        float end_x =
            frame_rectangles_cur_x +
            curAnimInst.curAnim->frames[f].duration * scale;
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
            canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING +
            anim_loop_time * scale;
        al_draw_filled_triangle(
            loop_x,
            frame_rectangle_bottom,
            loop_x,
            frame_rectangle_bottom - ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            loop_x + ANIM_EDITOR::TIMELINE_LOOP_TRI_SIZE,
            frame_rectangle_bottom,
            al_map_rgb(64, 64, 96)
        );
    }
    
    //Draw a line indicating where we are in the animation.
    float cur_time_line_x =
        canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING + anim_cur_time * scale;
    al_draw_line(
        cur_time_line_x, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
        cur_time_line_x, canvasBR.y,
        al_map_rgb(128, 48, 48), 2.0f
    );
    
    //Draw the milestone markers.
    float next_marker_x = 0.0f;
    unsigned char next_marker_type = 0;
    
    while(
        next_marker_x <
        canvasBR.x - canvasTL.x - ANIM_EDITOR::TIMELINE_PADDING * 2
    ) {
        float x_to_use =
            next_marker_x + canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
        switch(next_marker_type) {
        case 0: {
            string text = f2s(next_marker_x / scale);
            if(text.size() >= 4) {
                text = text.substr(1, 3);
            }
            drawText(
                text, game.sysContent.fntBuiltin,
                Point(
                    floor(x_to_use) + 2,
                    canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT + 2
                ),
                Point(LARGE_FLOAT, 8.0f), al_map_rgb(32, 32, 32),
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
            );
            al_draw_line(
                x_to_use + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 1:
        case 3: {
            al_draw_line(
                x_to_use + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.66f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        } case 2: {
            al_draw_line(
                x_to_use + 0.5, canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT,
                x_to_use + 0.5,
                canvasBR.y - ANIM_EDITOR::TIMELINE_HEIGHT +
                ANIM_EDITOR::TIMELINE_HEADER_HEIGHT * 0.33f,
                al_map_rgb(32, 32, 32), 1.0f
            );
            break;
            
        }
        }
        
        next_marker_x += scale * milestone_interval;
        next_marker_type = sumAndWrap(next_marker_type, 1, 4);
    }
}


/**
 * @brief Draws a hitbox on the canvas in the standard top-down view.
 *
 * @param h_ptr Hitbox to draw.
 * @param color Color of the hitbox's main shape.
 * @param outline_color Color of the hitbox's outline.
 * @param outline_thickness Thickness of the hitbox's outline.
 */
void AnimationEditor::drawTopDownViewHitbox(
    Hitbox* h_ptr, const ALLEGRO_COLOR &color,
    const ALLEGRO_COLOR &outline_color, float outline_thickness
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


/**
 * @brief Draws a leader silhouette on the canvas in the standard top-down view.
 *
 * @param x_offset From the center, offset the leader silhouette this much
 * to the right.
 */
void AnimationEditor::drawTopDownViewLeaderSilhouette(
    float x_offset
) {
    drawBitmap(
        game.sysContent.bmpLeaderSilhouetteTop, Point(x_offset, 0),
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
        al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
    );
    if(mt->rectangularDim.x != 0) {
        al_draw_rectangle(
            -mt->rectangularDim.x / 2.0, -mt->rectangularDim.y / 2.0,
            mt->rectangularDim.x / 2.0, mt->rectangularDim.y / 2.0,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
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
    
    Sprite* next_s = nullptr;
    float interpolation_factor = 0.0f;
    if(state == EDITOR_STATE_ANIMATION && curAnimInst.validFrame()) {
        curAnimInst.getSpriteData(
            nullptr, &next_s, &interpolation_factor
        );
    }
    
    if(s->bitmap) {
        Point coords;
        float angle;
        Point scale;
        ALLEGRO_COLOR tint;
        
        getSpriteBasicEffects(
            Point(), 0, LARGE_FLOAT, LARGE_FLOAT,
            s, next_s, interpolation_factor,
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
            s, next_s, interpolation_factor,
            &coords, &angle, &size
        );
        drawBitmap(topBmp[curMaturity], coords, size, angle);
    }
    
    if(comparisonAbove) {
        drawComparison();
    }
}
