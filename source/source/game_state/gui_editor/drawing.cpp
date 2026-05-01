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
    const ALLEGRO_COLOR BG_COLOR = al_map_rgb(96, 128, 96);
    const ALLEGRO_COLOR GRID_MAJOR_COLOR = al_map_rgba(64, 64, 64, 84);
    const ALLEGRO_COLOR GRID_MINOR_COLOR = al_map_rgba(64, 64, 64, 40);
    const ALLEGRO_COLOR GRID_ORIGIN_COLOR = al_map_rgba(208, 208, 224, 84);
    const ALLEGRO_COLOR HC_ITEM_COLOR = al_map_rgb(224, 160, 160);
    const ALLEGRO_COLOR CUSTOM_ITEM_COLOR = al_map_rgb(160, 224, 160);
    const ALLEGRO_COLOR ITEM_NAME_COLOR = al_map_rgb(40, 40, 96);
    
    RectCorners canvasCorners = game.editorsView.getWindowCorners();
    
    al_set_clipping_rectangle(
        canvasCorners.tl.x, canvasCorners.tl.y,
        game.editorsView.windowRect.size.x, game.editorsView.windowRect.size.y
    );
    
    //Background.
    al_clear_to_color(COLOR_BLACK);
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
    //Virtual game window.
    al_draw_filled_rectangle(
        0.0f, 0.0f, 100.0f, 100.0f, BG_COLOR
    );
    
    //Grid.
    drawGrid(
        game.options.guiEd.gridInterval, GRID_MAJOR_COLOR, GRID_MINOR_COLOR
    );
    
    //50%,50% marker.
    al_draw_line(
        0.0f, 50.0f, 100.0f, 50.0f,
        GRID_ORIGIN_COLOR, 1.0f / game.editorsView.cam.zoom
    );
    al_draw_line(
        50.0f, 0.0f, 50.0f, 100.0f,
        GRID_ORIGIN_COLOR, 1.0f / game.editorsView.cam.zoom
    );
    
    //Items.
    int origClipX = 0;
    int origClipY = 0;
    int origClipW = 0;
    int origClipH = 0;
    al_get_clipping_rectangle(
        &origClipX, &origClipY, &origClipW, &origClipH
    );
    
    //Sort them by layer.
    vector<pair<GuiItemDef*, unsigned char> > drawingSortedItems;
    map<GuiItemDef*, size_t> itemIdxMap;
    forIdx(i, allItems) {
        itemIdxMap[allItems[i]] = i;
        bool isCustom = i >= hardcodedItems.size();
        unsigned char drawingLayer;
        if(isCustom) {
            CustomGuiItemDef* customData = (CustomGuiItemDef*) allItems[i];
            drawingLayer =
                customData->drawBeforeHardcoded ?
                GUI::DRAWING_LAYER_CUSTOM_BEFORE :
                GUI::DRAWING_LAYER_CUSTOM_AFTER;
        } else {
            drawingLayer = GUI::DRAWING_LAYER_NORMAL;
        }
        drawingSortedItems.push_back(std::make_pair(allItems[i], drawingLayer));
    }
    std::stable_sort(
        drawingSortedItems.begin(), drawingSortedItems.end(),
    [] (const auto & i1, const auto & i2) {
        return i1.second < i2.second;
    }
    );
    
    forIdx(i, drawingSortedItems) {
        //Setup.
        size_t itemIdx = itemIdxMap[drawingSortedItems[i].first];
        GuiItemDef* item = drawingSortedItems[i].first;
        if(item->rect.size.x == 0.0f) continue;
        bool isCustom =
            drawingSortedItems[i].second != GUI::DRAWING_LAYER_NORMAL;
        bool isSelected = itemSelection.contains(itemIdx);
        
        //Pick the color.
        ALLEGRO_COLOR color;
        if(isCustom) {
            color = CUSTOM_ITEM_COLOR;
        } else {
            color = HC_ITEM_COLOR;
        }
        if(isSelected) {
            color = getSelectionEffectReplacementColor(color);
        }
        
        //Draw the item's background.
        drawFilledRoundedRectangle(
            item->rect.center, item->rect.size,
            8.0f / game.editorsView.cam.zoom, changeAlpha(color, 64)
        );
        
        //Draw the item's text.
        float clipX = item->rect.center.x - item->rect.size.x / 2.0f;
        float clipY = item->rect.center.y - item->rect.size.y / 2.0f;
        al_transform_coordinates(
            &game.editorsView.worldToWindowTransform, &clipX, &clipY
        );
        float clipW = item->rect.size.x * game.editorsView.cam.zoom;
        float clipH = item->rect.size.y * game.editorsView.cam.zoom;
        setCombinedClippingRectangles(
            origClipX, origClipY, origClipW, origClipH,
            clipX, clipY, clipW, clipH
        );
        drawText(
            item->name, game.sysContent.fntBuiltin,
            Point(
                (item->rect.center.x - item->rect.size.x / 2.0f) +
                (4.0f / game.editorsView.cam.zoom),
                (item->rect.center.y - item->rect.size.y / 2.0f) +
                (4.0f / game.editorsView.cam.zoom)
            ),
            Point(LARGE_FLOAT, 8.0 / game.editorsView.cam.zoom),
            ITEM_NAME_COLOR, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
        al_set_clipping_rectangle(
            origClipX, origClipY, origClipW, origClipH
        );
        
        //Draw the item's outline.
        drawRoundedRectangle(
            item->rect.center, item->rect.size,
            8.0f / game.editorsView.cam.zoom,
            color, 2.0f / game.editorsView.cam.zoom
        );
    }
    
    drawSelectionAndTransformationThings(
        itemSelCtrl, curTransformationWidget, false
    );
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
