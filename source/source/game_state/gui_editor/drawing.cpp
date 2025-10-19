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
    Point canvasTL = game.editorsView.getTopLeft();
    
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y, game.editorsView.size.x, game.editorsView.size.y
    );
    
    //Background.
    al_clear_to_color(COLOR_BLACK);
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
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
        1.0f / game.editorsView.cam.zoom
    );
    al_draw_line(
        50.0f, 0.0f, 50.0f, 100.0f,
        al_map_rgba(208, 208, 224, 84),
        1.0f / game.editorsView.cam.zoom
    );
    
    //Items.
    int origClipX = 0;
    int origClipY = 0;
    int origClipW = 0;
    int origClipH = 0;
    al_get_clipping_rectangle(
        &origClipX, &origClipY, &origClipW, &origClipH
    );
    for(size_t i = 0; i < allItems.size(); i++) {
        GuiItemDef* item = allItems[i];
        if(item->size.x == 0.0f) continue;
        bool isCustom = i >= hardcodedItems.size();
        ALLEGRO_COLOR color =
            isCustom ?
            al_map_rgb(160, 224, 160) :
            al_map_rgb(224, 160, 160);
            
        drawFilledRoundedRectangle(
            item->center, item->size,
            8.0f / game.editorsView.cam.zoom, changeAlpha(color, 64)
        );
        
        float clipX = item->center.x - item->size.x / 2.0f;
        float clipY = item->center.y - item->size.y / 2.0f;
        al_transform_coordinates(
            &game.editorsView.worldToWindowTransform, &clipX, &clipY
        );
        float clipW = item->size.x * game.editorsView.cam.zoom;
        float clipH = item->size.y * game.editorsView.cam.zoom;
        setCombinedClippingRectangles(
            origClipX, origClipY, origClipW, origClipH,
            clipX, clipY, clipW, clipH
        );
        drawText(
            item->name, game.sysContent.fntBuiltin,
            Point(
                (item->center.x - item->size.x / 2.0f) +
                (4.0f / game.editorsView.cam.zoom),
                (item->center.y - item->size.y / 2.0f) +
                (4.0f / game.editorsView.cam.zoom)
            ),
            Point(LARGE_FLOAT, 8.0 / game.editorsView.cam.zoom),
            al_map_rgb(40, 40, 96), ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
        al_set_clipping_rectangle(
            origClipX, origClipY, origClipW, origClipH
        );
        
        if(curItemIdx != i) {
            drawRoundedRectangle(
                item->center,
                item->size,
                8.0f / game.editorsView.cam.zoom,
                color, 2.0f / game.editorsView.cam.zoom
            );
        }
    }
    
    if(curItemIdx != INVALID && allItems[curItemIdx]->size.x != 0.0f) {
        curTransformationWidget.draw(
            &allItems[curItemIdx]->center,
            &allItems[curItemIdx]->size,
            nullptr,
            1.0f / game.editorsView.cam.zoom
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
