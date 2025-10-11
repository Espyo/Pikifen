/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor event handler function.
 */

#include "editor.h"

#include "../../core/game.h"


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleKeyCharCanvas(const ALLEGRO_EVENT& ev) {
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
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        gridIntervalDecreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        gridIntervalIncreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        zoomAndPosResetCmd(1.0f);
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        gridToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        loadCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        quickPlayCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quitCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        saveCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE, false, true)) {
        partMgrPlaybackToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        if(!guiFocusedText()) {
            partGenPlaybackToggleCmd(1.0f);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        if(!guiFocusedText()) {
            clearParticlesCmd(1.0f);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        leaderSilhouetteToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        emissionShapeToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        escapeWasPressed = true;
        
        if(!dialogs.empty()) {
            closeTopDialog();
        } else {
            quitCmd(1.0f);
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleKeyDownCanvas(const ALLEGRO_EVENT& ev) {

}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleLmbDoubleClick(const ALLEGRO_EVENT& ev) {
    handleLmbDown(ev);
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleLmbDown(const ALLEGRO_EVENT& ev) {
    generatorPosOffset = game.editorsView.mouseCursorWorldPos;
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleLmbDrag(const ALLEGRO_EVENT& ev) {
    generatorPosOffset = game.editorsView.mouseCursorWorldPos;
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleLmbUp(const ALLEGRO_EVENT& ev) {
    generatorPosOffset = Point();
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleMmbDown(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        zoomAndPosResetCmd(1.0f);
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleMmbDrag(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleMouseUpdate(const ALLEGRO_EVENT& ev) {
    Editor::handleMouseUpdate(ev);
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleMouseWheel(const ALLEGRO_EVENT& ev) {
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
void ParticleEditor::handleRmbDown(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        zoomAndPosResetCmd(1.0f);
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::handleRmbDrag(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
