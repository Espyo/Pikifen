/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor event handler function.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleKeyCharCanvas(const ALLEGRO_EVENT &ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.cam.targetPos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.cam.targetPos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.cam.targetPos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.cam.targetPos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS)) {
        zoomOutCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        zoomInCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        zoomAndPosResetCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            comparison = !comparison;
        }
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        gridToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_H, true)) {
        hitboxesToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        loadCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        leaderSilhouetteToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quitCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        mobRadiusToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        saveCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
    
        escapeWasPressed = true;
        
        if(!dialogs.empty()) {
            closeTopDialog();
            
        } else {
            switch(state) {
            case EDITOR_STATE_MAIN: {
                quitCmd(1.0f);
                break;
            }
            }
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleKeyDownCanvas(const ALLEGRO_EVENT &ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        playPauseAnimCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE, false, true)) {
        restartAnimCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        zoomEverythingCmd(1.0f);
        
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleLmbDoubleClick(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_HITBOXES || state == EDITOR_STATE_SPRITE_BITMAP) {
        handleLmbDown(ev);
    }
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleLmbDown(const ALLEGRO_EVENT &ev) {
    if(isCursorInTimeline()) {
        handleLmbDragInTimeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        Point cur_sprite_size = curSprite->scale * curSprite->bmpSize;
        if(
            curTransformationWidget.handleMouseDown(
                game.mouseCursor.wPos,
                &curSprite->offset,
                &cur_sprite_size,
                &curSprite->angle,
                1.0f / game.cam.zoom
            )
        ) {
            curSprite->scale = cur_sprite_size / curSprite->bmpSize;
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite) {
            bool tw_handled = false;
            if(curHitbox) {
                if(!sideView) {
                    Point hitbox_size(
                        curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                    );
                    tw_handled =
                        curTransformationWidget.handleMouseDown(
                            game.mouseCursor.wPos,
                            &curHitbox->pos,
                            &hitbox_size,
                            nullptr,
                            1.0f / game.cam.zoom
                        );
                } else {
                    Point hitbox_center(
                        curHitbox->pos.x,
                        (-(curHitbox->height / 2.0f)) - curHitbox->z
                    );
                    Point hitbox_size(
                        curHitbox->radius * 2.0f, curHitbox->height
                    );
                    tw_handled =
                        curTransformationWidget.handleMouseDown(
                            game.mouseCursor.wPos,
                            &hitbox_center,
                            &hitbox_size,
                            nullptr,
                            1.0f / game.cam.zoom
                        );
                }
            }
            
            if(!tw_handled) {
                vector<size_t> clicked_hitboxes;
                for(size_t h = 0; h < curSprite->hitboxes.size(); h++) {
                    Hitbox* h_ptr = &curSprite->hitboxes[h];
                    
                    if(sideView) {
                        Point tl(h_ptr->pos.x - h_ptr->radius, 0.0f);
                        Point br(h_ptr->pos.x + h_ptr->radius, 0.0f);
                        if(h_ptr->height != 0.0f) {
                            tl.y = -h_ptr->z - h_ptr->height;
                            br.y = -h_ptr->z;
                        } else {
                            tl.y = -FLT_MAX;
                            br.y = FLT_MAX;
                        }
                        if(
                            BBoxCheck(
                                tl, br,
                                game.mouseCursor.wPos, 1.0f / game.cam.zoom
                            )
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    } else {
                        if(
                            Distance(game.mouseCursor.wPos, h_ptr->pos) <=
                            h_ptr->radius
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    }
                }
                
                if(clicked_hitboxes.empty()) {
                    curHitbox = nullptr;
                    curHitboxIdx = INVALID;
                    
                } else {
                    size_t cur_hitbox_idx_idx = INVALID;
                    for(size_t i = 0; i < clicked_hitboxes.size(); i++) {
                        if(curHitboxIdx == clicked_hitboxes[i]) {
                            cur_hitbox_idx_idx = i;
                            break;
                        }
                    }
                    
                    if(cur_hitbox_idx_idx == INVALID) {
                        cur_hitbox_idx_idx = 0;
                    } else {
                        cur_hitbox_idx_idx =
                            sumAndWrap(
                                (int) cur_hitbox_idx_idx, 1,
                                (int) clicked_hitboxes.size()
                            );
                    }
                    curHitboxIdx = clicked_hitboxes[cur_hitbox_idx_idx];
                    curHitbox = &curSprite->hitboxes[curHitboxIdx];
                }
            }
        }
        break;
        
    } case EDITOR_STATE_SPRITE_BITMAP: {
        if(curSprite && curSprite->parentBmp) {
            Point bmp_size = getBitmapDimensions(curSprite->parentBmp);
            Point bmp_pos = 0.0f - bmp_size / 2.0f;
            Point bmp_click_pos = game.mouseCursor.wPos;
            bmp_click_pos.x = floor(bmp_click_pos.x - bmp_pos.x);
            bmp_click_pos.y = floor(bmp_click_pos.y - bmp_pos.y);
            
            if(bmp_click_pos.x < 0 || bmp_click_pos.y < 0) return;
            if(bmp_click_pos.x > bmp_size.x || bmp_click_pos.y > bmp_size.y) return;
            
            Point selection_tl;
            Point selection_br;
            if(
                (
                    curSprite->bmpSize.x == 0 ||
                    curSprite->bmpSize.y == 0
                ) || !spriteBmpAddMode
            ) {
                selection_tl = bmp_click_pos;
                selection_br = bmp_click_pos;
            } else {
                selection_tl = curSprite->bmpPos;
                selection_br.x =
                    curSprite->bmpPos.x + curSprite->bmpSize.x - 1;
                selection_br.y =
                    curSprite->bmpPos.y + curSprite->bmpSize.y - 1;
            }
            
            bool* selection_pixels = new bool[(int) (bmp_size.x * bmp_size.y)];
            memset(selection_pixels, 0, sizeof(bool) * ((int) (bmp_size.x * bmp_size.y)));
            
            al_lock_bitmap(
                curSprite->parentBmp,
                ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READONLY
            );
            
            spriteBmpFloodFill(
                curSprite->parentBmp, selection_pixels,
                bmp_click_pos.x, bmp_click_pos.y
            );
            
            al_unlock_bitmap(curSprite->parentBmp);
            
            size_t p;
            for(size_t y = 0; y < (size_t) bmp_size.y; y++) {
                for(size_t x = 0; x < (size_t) bmp_size.x; x++) {
                    p = y * bmp_size.x + x;
                    if(!selection_pixels[p]) continue;
                    updateMinMaxCoords(
                        selection_tl, selection_br, Point(x, y)
                    );
                }
            }
            
            delete[] selection_pixels;
            
            curSprite->bmpPos = selection_tl;
            curSprite->bmpSize = selection_br - selection_tl + 1;
            curSprite->setBitmap(
                curSprite->bmpName, curSprite->bmpPos, curSprite->bmpSize
            );
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(curSprite && curSprite->topVisible) {
            curTransformationWidget.handleMouseDown(
                game.mouseCursor.wPos,
                &curSprite->topPos,
                &curSprite->topSize,
                &curSprite->topAngle,
                1.0f / game.cam.zoom
            );
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleLmbDrag(const ALLEGRO_EVENT &ev) {
    if(isCursorInTimeline()) {
        handleLmbDragInTimeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        Point cur_sprite_size = curSprite->scale * curSprite->bmpSize;
        if(
            curTransformationWidget.handleMouseMove(
                game.mouseCursor.wPos,
                &curSprite->offset,
                &cur_sprite_size,
                &curSprite->angle,
                1.0f / game.cam.zoom,
                curSpriteKeepAspectRatio,
                curSpriteKeepArea,
                -FLT_MAX,
                isAltPressed
            )
        ) {
            curSprite->scale = cur_sprite_size / curSprite->bmpSize;
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite && curHitbox) {
            bool tw_handled;
            if(!sideView) {
                Point hitbox_size(
                    curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                );
                tw_handled =
                    curTransformationWidget.handleMouseMove(
                        game.mouseCursor.wPos,
                        &curHitbox->pos,
                        &hitbox_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        true,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        isAltPressed
                    );
                curHitbox->radius = hitbox_size.x / 2.0f;
            } else {
                Point hitbox_center(
                    curHitbox->pos.x,
                    (-(curHitbox->height / 2.0f)) - curHitbox->z
                );
                Point hitbox_size(
                    curHitbox->radius * 2.0f, curHitbox->height
                );
                tw_handled =
                    curTransformationWidget.handleMouseMove(
                        game.mouseCursor.wPos,
                        &hitbox_center,
                        &hitbox_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        isAltPressed
                    );
                curHitbox->pos.x = hitbox_center.x;
                curHitbox->radius = hitbox_size.x / 2.0f;
                curHitbox->z = -(hitbox_center.y + hitbox_size.y / 2.0f);
                curHitbox->height = hitbox_size.y;
            }
            
            if(tw_handled) {
                changesMgr.markAsChanged();
            }
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(curSprite && curSprite->topVisible) {
            if(
                curTransformationWidget.handleMouseMove(
                    game.mouseCursor.wPos,
                    &curSprite->topPos,
                    &curSprite->topSize,
                    &curSprite->topAngle,
                    1.0f / game.cam.zoom,
                    topKeepAspectRatio,
                    false,
                    ANIM_EDITOR::TOP_MIN_SIZE,
                    isAltPressed
                )
            ) {
                changesMgr.markAsChanged();
            }
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the mouse being clicked/dragged in the animation timeline.
 */
void AnimationEditor::handleLmbDragInTimeline() {
    float cursor_time = getCursorTimelineTime();
    size_t old_frame_idx = curAnimInst.curFrameIdx;
    curAnimInst.curAnim->getFrameAndTime(
        cursor_time, &curAnimInst.curFrameIdx, &curAnimInst.curFrameTime
    );
    if(curAnimInst.curFrameIdx != old_frame_idx) {
        Frame* f = &curAnimInst.curAnim->frames[curAnimInst.curFrameIdx];
        if(f->soundIdx != INVALID) {
            playSound(f->soundIdx);
        }
    }
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleLmbUp(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        curTransformationWidget.handleMouseUp();
        break;
        
    } case EDITOR_STATE_TOP: {
        if(curSprite && curSprite->topVisible) {
            curTransformationWidget.handleMouseUp();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite && curHitbox) {
            curTransformationWidget.handleMouseUp();
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMmbDoubleClick(const ALLEGRO_EVENT &ev) {
    if(!game.options.editors.mmbPan) {
        resetCamXY();
    }
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMmbDown(const ALLEGRO_EVENT &ev) {
    if(!game.options.editors.mmbPan) {
        resetCamZoom();
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMmbDrag(const ALLEGRO_EVENT &ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMouseUpdate(const ALLEGRO_EVENT &ev) {
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMouseWheel(const ALLEGRO_EVENT &ev) {
    zoomWithCursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/**
 * @brief Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleRmbDoubleClick(const ALLEGRO_EVENT &ev) {
    if(game.options.editors.mmbPan) {
        resetCamXY();
    }
}


/**
 * @brief Handles the right mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleRmbDown(const ALLEGRO_EVENT &ev) {
    if(game.options.editors.mmbPan) {
        resetCamZoom();
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleRmbDrag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
