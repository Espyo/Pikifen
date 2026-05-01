/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor event handler function.
 */

#include "editor.h"

#include "../../core/game.h"


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleKeyCharCanvas(const ALLEGRO_EVENT& ev) {
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
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        gridIntervalDecreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        gridIntervalIncreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        resetCam(false);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_A, true)) {
        selectAllCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_N)) {
        switch(state) {
        case EDITOR_STATE_CUSTOM: {
            addNewCustomItemCmd(1.0f);
            break;
        }
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_X)) {
        snapModeCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DELETE)) {
        deleteCmd(1.0f);
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        loadCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        quickPlayCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quitCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        saveCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        escapeWasPressed = true;
        
        if(!dialogs.empty()) {
            closeTopDialog();
        } else if(!clearSelections()) {
            if(state == EDITOR_STATE_MAIN) {
                quitCmd(1.0f);
            } else {
                changeState(EDITOR_STATE_MAIN);
            }
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleKeyDownCanvas(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        resetCam(false);
        
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleLmbDoubleClick(const ALLEGRO_EVENT& ev) {
    handleLmbDown(ev);
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleLmbDown(const ALLEGRO_EVENT& ev) {
    size_t prevSelectedItemIdx = itemSelection.getSingleItemIdx();
    
    handleSelectionAndTransformationLmbDown(
        itemSelCtrl, curTransformationWidget, false
    );
    
    size_t newSelectedItemIdx = itemSelection.getSingleItemIdx();
    if(
        newSelectedItemIdx != INVALID &&
        newSelectedItemIdx != prevSelectedItemIdx
    ) {
        mustFocusOnCurItem = true;
    }
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleLmbDrag(const ALLEGRO_EVENT& ev) {
    bool changesMade =
        handleSelectionAndTransformationLmbDrag(
            itemSelCtrl, curTransformationWidget, false,
            snapPoint(game.editorsView.mouseCursorWorldPos)
        );
    if(changesMade) {
        changesMgr.markAsChanged();
    }
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleLmbUp(const ALLEGRO_EVENT& ev) {
    handleSelectionAndTransformationLmbUp(
        itemSelCtrl, curTransformationWidget, false
    );
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleMmbDown(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        resetCam(false);
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleMmbDrag(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleMouseUpdate(const ALLEGRO_EVENT& ev) {
    Editor::handleMouseUpdate(ev);
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleMouseWheel(const ALLEGRO_EVENT& ev) {
    zoomWithCursor(
        game.editorsView.cam.zoom +
        (game.editorsView.cam.zoom * ev.mouse.dz * 0.1)
    );
}


/**
 * @brief Handles the right mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleRmbDown(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        resetCam(false);
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void GuiEditor::handleRmbDrag(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
