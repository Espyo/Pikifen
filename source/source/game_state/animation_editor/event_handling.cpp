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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleKeyCharCanvas(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.editorsView.cam.targetCenter.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.editorsView.cam.targetCenter.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.editorsView.cam.targetCenter.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.editorsView.cam.targetCenter.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS)) {
        zoomOutCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        zoomInCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        zoomAndPosResetCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_A, true)) {
        selectAllCmd(1.0f);
        
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
void AnimationEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_E, true)) {
        leaderSilhouetteToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        gridToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_H, true)) {
        hitboxesToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        loadCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        quickPlayCmd(1.0f);
        
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
            
        } else if(!clearSelections()) {
            switch(state) {
            case EDITOR_STATE_MAIN: {
                quitCmd(1.0f);
                break;
                
            } case EDITOR_STATE_ANIMATION:
            case EDITOR_STATE_SPRITE:
            case EDITOR_STATE_BODY_PART:
            case EDITOR_STATE_INFO:
            case EDITOR_STATE_TOOLS: {
                changeState(EDITOR_STATE_MAIN);
                break;
                
            } case EDITOR_STATE_SPRITE_BITMAP: {
                game.editorsView.cam.setPos(preSpriteBmpCamPos);
                game.editorsView.cam.setZoom(preSpriteBmpCamZoom);
                changeState(EDITOR_STATE_SPRITE);
                break;
                
            } case EDITOR_STATE_SPRITE_TRANSFORM: {
                changeState(EDITOR_STATE_SPRITE);
                break;
                
            } case EDITOR_STATE_HITBOXES: {
                changeState(EDITOR_STATE_SPRITE);
                break;
                
            } case EDITOR_STATE_PIKMIN_TOP: {
                changeState(EDITOR_STATE_SPRITE);
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
void AnimationEditor::handleKeyDownCanvas(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        playPauseAnimCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE, false, true)) {
        restartAnimCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        if(state == EDITOR_STATE_SPRITE_BITMAP) {
            centerCameraOnSpriteBitmap(false);
        } else {
            zoomEverythingCmd(1.0f);
        }
        
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleLmbDoubleClick(const ALLEGRO_EVENT& ev) {
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
void AnimationEditor::handleLmbDown(const ALLEGRO_EVENT& ev) {
    if(isCursorInTimeline()) {
        handleLmbDragInTimeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        Point curSpriteSize = curSprite->tf.scale * curSprite->bmpSize;
        if(
            curTransformationWidget.handleMouseDown(
                game.editorsView.mouseCursorWorldPos,
                &curSprite->tf.trans,
                &curSpriteSize,
                &curSprite->tf.rot,
                1.0f / game.editorsView.cam.zoom
            )
        ) {
            curSprite->tf.scale = curSpriteSize / curSprite->bmpSize;
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite) {
            handleSelectionAndTransformationLmbDown(
                hitboxSelCtrl, curTransformationWidget, false
            );
            prevHitboxSelection = hitboxSelection.getItemIdxs();
        }
        break;
        
    } case EDITOR_STATE_SPRITE_BITMAP: {
        if(curSprite && curSprite->parentBmp) {
            Point bmpSize = getBitmapDimensions(curSprite->parentBmp);
            Point bmpPos = 0.0f - bmpSize / 2.0f;
            Point bmpClickPos = game.editorsView.mouseCursorWorldPos;
            bmpClickPos.x = floor(bmpClickPos.x - bmpPos.x);
            bmpClickPos.y = floor(bmpClickPos.y - bmpPos.y);
            
            if(bmpClickPos.x < 0 || bmpClickPos.y < 0) return;
            if(bmpClickPos.x > bmpSize.x || bmpClickPos.y > bmpSize.y) return;
            
            RectCorners selectionCorners;
            if(
                (
                    curSprite->bmpSize.x == 0 ||
                    curSprite->bmpSize.y == 0
                ) || !spriteBmpAddMode
            ) {
                selectionCorners.tl = bmpClickPos;
                selectionCorners.br = bmpClickPos;
            } else {
                selectionCorners.tl = curSprite->bmpPos;
                selectionCorners.br.x =
                    curSprite->bmpPos.x + curSprite->bmpSize.x - 1;
                selectionCorners.br.y =
                    curSprite->bmpPos.y + curSprite->bmpSize.y - 1;
            }
            
            bool* selectionPixels = new bool[(int) (bmpSize.x * bmpSize.y)];
            memset(
                selectionPixels, 0,
                sizeof(bool) * ((int) (bmpSize.x * bmpSize.y))
            );
            
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
                        selectionCorners, Point(x, y)
                    );
                }
            }
            
            delete[] selectionPixels;
            
            curSprite->bmpPos = selectionCorners.tl;
            curSprite->bmpSize = selectionCorners.br - selectionCorners.tl + 1;
            curSprite->setBitmap(
                curSprite->bmpName, curSprite->bmpPos, curSprite->bmpSize
            );
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_PIKMIN_TOP: {
        if(curSprite && curSprite->topVisible) {
            curTransformationWidget.handleMouseDown(
                game.editorsView.mouseCursorWorldPos,
                &curSprite->topPose.pos,
                &curSprite->topPose.size,
                &curSprite->topPose.angle,
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
void AnimationEditor::handleLmbDrag(const ALLEGRO_EVENT& ev) {
    if(isCursorInTimeline()) {
        handleLmbDragInTimeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        Point curSpriteSize = curSprite->tf.scale * curSprite->bmpSize;
        Bitmask8 flags = 0;
        if(curSpriteKeepAspectRatio) {
            enableFlag(flags, TransformationWidget::TW_FLAG_KEEP_RATIO);
        }
        if(curSpriteKeepArea) {
            enableFlag(flags, TransformationWidget::TW_FLAG_KEEP_AREA);
        }
        if(isAltPressed) {
            enableFlag(flags, TransformationWidget::TW_FLAG_LOCK_CENTER);
        }
        if(
            curTransformationWidget.handleMouseMove(
                game.editorsView.mouseCursorWorldPos,
                &curSprite->tf.trans,
                &curSpriteSize,
                &curSprite->tf.rot,
                1.0f / game.editorsView.cam.zoom, flags, 0.0f, -FLT_MAX,
        [this] (const Point & p) { return snapPoint(p); }
            )
        ) {
            curSprite->tf.scale = curSpriteSize / curSprite->bmpSize;
            changesMgr.markAsChanged();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite) {
            bool changesMade =
                handleSelectionAndTransformationLmbDrag(
                    hitboxSelCtrl, curTransformationWidget, false,
                    snapPoint(game.editorsView.mouseCursorWorldPos)
                );
            if(changesMade) {
                changesMgr.markAsChanged();
            }
        }
        break;
        
    } case EDITOR_STATE_PIKMIN_TOP: {
        if(curSprite && curSprite->topVisible) {
            Bitmask8 flags = 0;
            if(topKeepAspectRatio) {
                enableFlag(flags, TransformationWidget::TW_FLAG_KEEP_RATIO);
            }
            if(isAltPressed) {
                enableFlag(flags, TransformationWidget::TW_FLAG_LOCK_CENTER);
            }
            if(
                curTransformationWidget.handleMouseMove(
                    game.editorsView.mouseCursorWorldPos,
                    &curSprite->topPose.pos,
                    &curSprite->topPose.size,
                    &curSprite->topPose.angle,
                    1.0f / game.editorsView.cam.zoom, flags,
                    0.0f, ANIM_EDITOR::TOP_MIN_SIZE,
            [this] (const Point & p) { return snapPoint(p); }
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
void AnimationEditor::handleLmbUp(const ALLEGRO_EVENT& ev) {
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        curTransformationWidget.handleMouseUp();
        break;
        
    } case EDITOR_STATE_PIKMIN_TOP: {
        if(curSprite && curSprite->topVisible) {
            curTransformationWidget.handleMouseUp();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(curSprite) {
            handleSelectionAndTransformationLmbUp(
                hitboxSelCtrl, curTransformationWidget, false
            );
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
void AnimationEditor::handleMmbDoubleClick(const ALLEGRO_EVENT& ev) {
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
void AnimationEditor::handleMmbDown(const ALLEGRO_EVENT& ev) {
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
void AnimationEditor::handleMmbDrag(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMouseUpdate(const ALLEGRO_EVENT& ev) {
    Editor::handleMouseUpdate(ev);
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleMouseWheel(const ALLEGRO_EVENT& ev) {
    zoomWithCursor(
        game.editorsView.cam.zoom +
        (game.editorsView.cam.zoom * ev.mouse.dz * 0.1)
    );
}


/**
 * @brief Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::handleRmbDoubleClick(const ALLEGRO_EVENT& ev) {
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
void AnimationEditor::handleRmbDown(const ALLEGRO_EVENT& ev) {
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
void AnimationEditor::handleRmbDrag(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
