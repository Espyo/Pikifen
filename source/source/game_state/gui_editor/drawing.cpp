/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor drawing logic.
 */

#include "editor.h"

#include "../../core/game.h"
#include "../../util/allegro_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the GUI editor.
 */
void GuiEditor::doDrawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    drawOpErrorCursor();
}


/**
 * @brief Draw the canvas. This is called as a callback inside the
 * Dear ImGui rendering process.
 */
void GuiEditor::drawCanvas() {
    Point canvasTL = game.view.getTopLeft();
    
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y, game.view.size.x, game.view.size.y
    );
    
    //Background.
    al_clear_to_color(COLOR_BLACK);
    
    al_use_transform(&game.view.worldToWindowTransform);
    
    //Virtual game window.
    al_draw_filled_rectangle(
        0.0f, 0.0f, 100.0f, 100.0f, al_map_rgb(96, 128, 96)
    );
    
    //Grid.
    drawGrid(
        game.options.guiEd.gridInterval,
        al_map_rgba(64, 64, 64, 84),
        al_map_rgba(64, 64, 64, 40)
    );
    
    //50%,50% marker.
    al_draw_line(
        0.0f, 50.0f, 100.0f, 50.0f,
        al_map_rgba(208, 208, 224, 84),
        1.0f / game.view.cam.zoom
    );
    al_draw_line(
        50.0f, 0.0f, 50.0f, 100.0f,
        al_map_rgba(208, 208, 224, 84),
        1.0f / game.view.cam.zoom
    );
    
    //Items.
    int origClipX = 0;
    int origClipY = 0;
    int origClipW = 0;
    int origClipH = 0;
    al_get_clipping_rectangle(
        &origClipX, &origClipY, &origClipW, &origClipH
    );
    for(size_t i = 0; i < items.size(); i++) {
        if(items[i].size.x == 0.0f) continue;
        
        drawFilledRoundedRectangle(
            items[i].center,
            items[i].size,
            8.0f / game.view.cam.zoom,
            al_map_rgba(224, 224, 224, 64)
        );
        
        float clipX = items[i].center.x - items[i].size.x / 2.0f;
        float clipY = items[i].center.y - items[i].size.y / 2.0f;
        al_transform_coordinates(
            &game.view.worldToWindowTransform, &clipX, &clipY
        );
        float clipW = items[i].size.x * game.view.cam.zoom;
        float clipH = items[i].size.y * game.view.cam.zoom;
        setCombinedClippingRectangles(
            origClipX, origClipY, origClipW, origClipH,
            clipX, clipY, clipW, clipH
        );
        drawText(
            items[i].name, game.sysContent.fntBuiltin,
            Point(
                (items[i].center.x - items[i].size.x / 2.0f) +
                (4.0f / game.view.cam.zoom),
                (items[i].center.y - items[i].size.y / 2.0f) +
                (4.0f / game.view.cam.zoom)
            ),
            Point(LARGE_FLOAT, 8.0 / game.view.cam.zoom),
            al_map_rgb(40, 40, 96), ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
        al_set_clipping_rectangle(
            origClipX, origClipY, origClipW, origClipH
        );
        
        if(curItem != i) {
            drawRoundedRectangle(
                items[i].center,
                items[i].size,
                8.0f / game.view.cam.zoom,
                al_map_rgb(224, 224, 224),
                2.0f / game.view.cam.zoom
            );
        }
    }
    
    if(curItem != INVALID && items[curItem].size.x != 0.0f) {
        curTransformationWidget.draw(
            &items[curItem].center,
            &items[curItem].size,
            nullptr,
            1.0f / game.view.cam.zoom
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
