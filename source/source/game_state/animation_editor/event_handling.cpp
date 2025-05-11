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
        game.editorsView.cam.targetPos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.editorsView.cam.targetPos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.editorsView.cam.targetPos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.editorsView.cam.targetPos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
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
        Point curSpriteSize = curSprite->scale * curSprite->bmpSize;
        if(
            curTransformationWidget.handleMouseDown(
                game.editorsView.cursorWorldPos,
                &curSprite->offset,
                &curSpriteSize,
                &curSprite->angle,
                1.0f / game.editorsView.cam.zoom
            )
        ) {
            curSprite->scale = curSpriteSize / curSprite->bmpSize;
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite) {
            bool twHandled = false;
            if(curHitbox) {
                if(!sideView) {
                    Point hitboxSize(
                        curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                    );
                    twHandled =
                        curTransformationWidget.handleMouseDown(
                            game.editorsView.cursorWorldPos,
                            &curHitbox->pos,
                            &hitboxSize,
                            nullptr,
                            1.0f / game.editorsView.cam.zoom
                        );
                } else {
                    Point hitboxCenter(
                        curHitbox->pos.x,
                        (-(curHitbox->height / 2.0f)) - curHitbox->z
                    );
                    Point hitboxSize(
                        curHitbox->radius * 2.0f, curHitbox->height
                    );
                    twHandled =
                        curTransformationWidget.handleMouseDown(
                            game.editorsView.cursorWorldPos,
                            &hitboxCenter,
                            &hitboxSize,
                            nullptr,
                            1.0f / game.editorsView.cam.zoom
                        );
                }
            }
            
            if(!twHandled) {
                vector<size_t> clickedHitboxes;
                for(size_t h = 0; h < curSprite->hitboxes.size(); h++) {
                    Hitbox* hPtr = &curSprite->hitboxes[h];
                    
                    if(sideView) {
                        Point tl(hPtr->pos.x - hPtr->radius, 0.0f);
                        Point br(hPtr->pos.x + hPtr->radius, 0.0f);
                        if(hPtr->height != 0.0f) {
                            tl.y = -hPtr->z - hPtr->height;
                            br.y = -hPtr->z;
                        } else {
                            tl.y = -FLT_MAX;
                            br.y = FLT_MAX;
                        }
                        if(
                            BBoxCheck(
                                tl, br,
                                game.editorsView.cursorWorldPos, 1.0f / game.editorsView.cam.zoom
                            )
                        ) {
                            clickedHitboxes.push_back(h);
                        }
                    } else {
                        if(
                            Distance(game.editorsView.cursorWorldPos, hPtr->pos) <=
                            hPtr->radius
                        ) {
                            clickedHitboxes.push_back(h);
                        }
                    }
                }
                
                if(clickedHitboxes.empty()) {
                    curHitbox = nullptr;
                    curHitboxIdx = INVALID;
                    
                } else {
                    size_t curHitboxIdxIdx = INVALID;
                    for(size_t i = 0; i < clickedHitboxes.size(); i++) {
                        if(curHitboxIdx == clickedHitboxes[i]) {
                            curHitboxIdxIdx = i;
                            break;
                        }
                    }
                    
                    if(curHitboxIdxIdx == INVALID) {
                        curHitboxIdxIdx = 0;
                    } else {
                        curHitboxIdxIdx =
                            sumAndWrap(
                                (int) curHitboxIdxIdx, 1,
                                (int) clickedHitboxes.size()
                            );
                    }
                    curHitboxIdx = clickedHitboxes[curHitboxIdxIdx];
                    curHitbox = &curSprite->hitboxes[curHitboxIdx];
                }
            }
        }
        break;
        
    } case EDITOR_STATE_SPRITE_BITMAP: {
        if(curSprite && curSprite->parentBmp) {
            Point bmpSize = getBitmapDimensions(curSprite->parentBmp);
            Point bmpPos = 0.0f - bmpSize / 2.0f;
            Point bmpClickPos = game.editorsView.cursorWorldPos;
            bmpClickPos.x = floor(bmpClickPos.x - bmpPos.x);
            bmpClickPos.y = floor(bmpClickPos.y - bmpPos.y);
            
            if(bmpClickPos.x < 0 || bmpClickPos.y < 0) return;
            if(bmpClickPos.x > bmpSize.x || bmpClickPos.y > bmpSize.y) return;
            
            Point selectionTL;
            Point selectionBR;
            if(
                (
                    curSprite->bmpSize.x == 0 ||
                    curSprite->bmpSize.y == 0
                ) || !spriteBmpAddMode
            ) {
                selectionTL = bmpClickPos;
                selectionBR = bmpClickPos;
            } else {
                selectionTL = curSprite->bmpPos;
                selectionBR.x =
                    curSprite->bmpPos.x + curSprite->bmpSize.x - 1;
                selectionBR.y =
                    curSprite->bmpPos.y + curSprite->bmpSize.y - 1;
            }
            
            bool* selectionPixels = new bool[(int) (bmpSize.x * bmpSize.y)];
            memset(selectionPixels, 0, sizeof(bool) * ((int) (bmpSize.x * bmpSize.y)));
            
            al_lock_bitmap(
                curSprite->parentBmp,
                ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READONLY
            );
            
            spriteBmpFloodFill(
                curSprite->parentBmp, selectionPixels,
                bmpClickPos.x, bmpClickPos.y
            );
            
            al_unlock_bitmap(curSprite->parentBmp);
            
            size_t p;
            for(size_t y = 0; y < (size_t) bmpSize.y; y++) {
                for(size_t x = 0; x < (size_t) bmpSize.x; x++) {
                    p = y * bmpSize.x + x;
                    if(!selectionPixels[p]) continue;
                    updateMinMaxCoords(
                        selectionTL, selectionBR, Point(x, y)
                    );
                }
            }
            
            delete[] selectionPixels;
            
            curSprite->bmpPos = selectionTL;
            curSprite->bmpSize = selectionBR - selectionTL + 1;
            curSprite->setBitmap(
                curSprite->bmpName, curSprite->bmpPos, curSprite->bmpSize
            );
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(curSprite && curSprite->topVisible) {
            curTransformationWidget.handleMouseDown(
                game.editorsView.cursorWorldPos,
                &curSprite->topPos,
                &curSprite->topSize,
                &curSprite->topAngle,
                1.0f / game.editorsView.cam.zoom
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
        Point curSpriteSize = curSprite->scale * curSprite->bmpSize;
        if(
            curTransformationWidget.handleMouseMove(
                game.editorsView.cursorWorldPos,
                &curSprite->offset,
                &curSpriteSize,
                &curSprite->angle,
                1.0f / game.editorsView.cam.zoom,
                curSpriteKeepAspectRatio,
                curSpriteKeepArea,
                -FLT_MAX,
                isAltPressed
            )
        ) {
            curSprite->scale = curSpriteSize / curSprite->bmpSize;
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite && curHitbox) {
            bool twHandled;
            if(!sideView) {
                Point hitboxSize(
                    curHitbox->radius * 2.0f, curHitbox->radius * 2.0f
                );
                twHandled =
                    curTransformationWidget.handleMouseMove(
                        game.editorsView.cursorWorldPos,
                        &curHitbox->pos,
                        &hitboxSize,
                        nullptr,
                        1.0f / game.editorsView.cam.zoom,
                        true,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        isAltPressed
                    );
                curHitbox->radius = hitboxSize.x / 2.0f;
            } else {
                Point hitboxCenter(
                    curHitbox->pos.x,
                    (-(curHitbox->height / 2.0f)) - curHitbox->z
                );
                Point hitboxSize(
                    curHitbox->radius * 2.0f, curHitbox->height
                );
                twHandled =
                    curTransformationWidget.handleMouseMove(
                        game.editorsView.cursorWorldPos,
                        &hitboxCenter,
                        &hitboxSize,
                        nullptr,
                        1.0f / game.editorsView.cam.zoom,
                        false,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        isAltPressed
                    );
                curHitbox->pos.x = hitboxCenter.x;
                curHitbox->radius = hitboxSize.x / 2.0f;
                curHitbox->z = -(hitboxCenter.y + hitboxSize.y / 2.0f);
                curHitbox->height = hitboxSize.y;
            }
            
            if(twHandled) {
                changesMgr.markAsChanged();
            }
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(curSprite && curSprite->topVisible) {
            if(
                curTransformationWidget.handleMouseMove(
                    game.editorsView.cursorWorldPos,
                    &curSprite->topPos,
                    &curSprite->topSize,
                    &curSprite->topAngle,
                    1.0f / game.editorsView.cam.zoom,
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
    float cursorTime = getCursorTimelineTime();
    size_t oldFrameIdx = curAnimInst.curFrameIdx;
    curAnimInst.curAnim->getFrameAndTime(
        cursorTime, &curAnimInst.curFrameIdx, &curAnimInst.curFrameTime
    );
    if(curAnimInst.curFrameIdx != oldFrameIdx) {
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
    Editor::handleMouseUpdate(ev);
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMouseWheel(const ALLEGRO_EVENT &ev) {
    zoomWithCursor(game.editorsView.cam.zoom + (game.editorsView.cam.zoom * ev.mouse.dz * 0.1));
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
