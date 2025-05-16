/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing logic.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the area editor.
 */
void AreaEditor::doDrawing() {
    if(hackSkipDrawing) {
        //Skip drawing for one frame.
        //This hack fixes a weird glitch where if you quick-play an area
        //with no leaders and get booted back into the area editor, the
        //engine would crash.
        hackSkipDrawing = false;
        return;
    }
    
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    drawOpErrorCursor();
}


/**
 * @brief Draws an arrow, usually used for one mob to point to another.
 *
 * @param start Starting point of the arrow.
 * @param end Ending point of the arrow, where the arrow points to.
 * @param startOffset When considering where to place the triangle
 * in the line, pretend that the starting point is actually this distance
 * away from start. Useful for when mobs of different radii are involved.
 * @param endOffset Same as startOffset, but for the end point.
 * @param thickness Thickness of the arrow's line.
 * @param color Arrow color.
 */
void AreaEditor::drawArrow(
    const Point &start, const Point &end,
    float startOffset, float endOffset,
    float thickness, const ALLEGRO_COLOR &color
) {
    al_draw_line(
        start.x, start.y, end.x, end.y,
        color, thickness / game.editorsView.cam.zoom
    );
    
    if(game.editorsView.cam.zoom >= 0.25) {
        float angle =
            getAngle(start, end);
        Point finalStart = Point(startOffset, 0);
        finalStart = rotatePoint(finalStart, angle);
        finalStart += start;
        Point finalEnd = Point(endOffset, 0);
        finalEnd = rotatePoint(finalEnd, angle + TAU / 2.0);
        finalEnd += end;
        
        Point pivot(
            finalStart.x + (finalEnd.x - finalStart.x) * 0.55,
            finalStart.y + (finalEnd.y - finalStart.y) * 0.55
        );
        const float delta =
            (thickness * 4) / game.editorsView.cam.zoom;
            
        al_draw_filled_triangle(
            pivot.x + cos(angle) * delta,
            pivot.y + sin(angle) * delta,
            pivot.x + cos(angle + TAU / 4) * delta,
            pivot.y + sin(angle + TAU / 4) * delta,
            pivot.x + cos(angle - TAU / 4) * delta,
            pivot.y + sin(angle - TAU / 4) * delta,
            color
        );
    }
}


/**
 * @brief Draw the canvas.
 *
 * This is called as a callback inside the Dear ImGui rendering process.
 */
void AreaEditor::drawCanvas() {
    //Setup.
    Point canvasTL = game.editorsView.getTopLeft();
    
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y, game.editorsView.size.x, game.editorsView.size.y
    );
    
    al_clear_to_color(COLOR_BLACK);
    
    if(!game.curAreaData) {
        al_reset_clipping_rectangle();
        return;
    }
    
    al_use_transform(&game.editorsView.worldToWindowTransform);

    AreaEdCanvasStyle style {
        .textureAlpha = 0.4f,
        .wallShadowAlpha = 0.0f,
        .edgeAlpha = 0.25f,
        .mobAlpha = 0.15f
    };
    float selectionMinAlpha = 0.25f;
    float selectionMaxAlpha = 0.25f;
    
    if(game.options.editors.useCustomStyle) {
        style.highlightColor = game.options.editors.highlightColor;
    }
    
    if(
        game.options.areaEd.viewMode == VIEW_MODE_HEIGHTMAP &&
        !game.curAreaData->sectors.empty()
    ) {
        style.lowestSectorZ = game.curAreaData->sectors[0]->z;
        style.highestSectorZ = style.lowestSectorZ;
        
        for(size_t s = 1; s < game.curAreaData->sectors.size(); s++) {
            style.lowestSectorZ =
                std::min(style.lowestSectorZ, game.curAreaData->sectors[s]->z);
            style.highestSectorZ =
                std::max(style.highestSectorZ, game.curAreaData->sectors[s]->z);
        }
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        style.textureAlpha = 0.5f;
        style.edgeAlpha = 1.0f;
        break;
        
    } case EDITOR_STATE_MOBS: {
        style.mobAlpha = 1.0f;
        break;
        
    } case EDITOR_STATE_MAIN:
    case EDITOR_STATE_REVIEW: {
        style.textureAlpha = 0.6f;
        style.edgeAlpha = 0.5f;
        style.gridAlpha = 0.3f;
        style.mobAlpha = 0.75f;
        break;
        
    }
    }
    
    if(previewMode) {
        style.textureAlpha = 1.0f;
        style.wallShadowAlpha = 1.0f;
        style.edgeAlpha = 0.0f;
        style.gridAlpha = 0.0f;
        style.mobAlpha = 0.0f;
    } else if(subState == EDITOR_SUB_STATE_OCTEE) {
        quickPreviewTimer.start();
    }
    
    if(quickPreviewTimer.timeLeft > 0) {
        float t =
            std::min(
                quickPreviewTimer.timeLeft,
                quickPreviewTimer.duration / 2.0f
            );
        selectionMinAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                selectionMinAlpha, 0.0f
            );
        selectionMaxAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                selectionMaxAlpha, 0.0f
            );
        style.textureAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                style.textureAlpha, 1.0f
            );
        style.wallShadowAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                style.wallShadowAlpha, 1.0f
            );
        style.edgeAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                style.edgeAlpha, 0.0f
            );
        style.gridAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                style.gridAlpha, 0.0f
            );
        style.mobAlpha =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                style.mobAlpha, 0.0f
            );
    }
    
    style.selectionAlpha =
        selectionMinAlpha +
        (sin(selectionEffect) + 1) *
        (selectionMaxAlpha - selectionMinAlpha) / 2.0;

    //Draw!
    drawSectors(style);
    
    drawGrid(
        game.options.areaEd.gridInterval,
        al_map_rgba(64, 64, 64, style.gridAlpha * 255),
        al_map_rgba(48, 48, 48, style.gridAlpha * 255)
    );
    
    //0,0 marker.
    al_draw_line(
        -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2, 0,
        al_map_rgba(192, 192, 224, style.gridAlpha * 255),
        1.0f / game.editorsView.cam.zoom
    );
    al_draw_line(
        0, -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2,
        al_map_rgba(192, 192, 224, style.gridAlpha * 255),
        1.0f / game.editorsView.cam.zoom
    );

    drawEdges(style);

    drawVertexes(style);
    
    //Selection transformation widget.
    if(
        game.options.areaEd.selTrans &&
        selectedVertexes.size() >= 2 &&
        (!moving || curTransformationWidget.isMovingHandle())
    ) {
        curTransformationWidget.draw(
            &selectionCenter,
            &selectionSize,
            &selectionAngle,
            1.0f / game.editorsView.cam.zoom
        );
    }

    drawMobs(style);

    drawPaths(style);
    
    drawTreeShadows(style);
    
    //Mission exit region transformation widget.
    if(subState == EDITOR_SUB_STATE_MISSION_EXIT) {
        curTransformationWidget.draw(
            &game.curAreaData->mission.goalExitCenter,
            &game.curAreaData->mission.goalExitSize,
            nullptr,
            1.0f / game.editorsView.cam.zoom
        );
    }
    
    //Cross-section points and line.
    if(state == EDITOR_STATE_REVIEW && showCrossSection) {
        for(unsigned char p = 0; p < 2; p++) {
            string letter = (p == 0 ? "A" : "B");
            
            al_draw_filled_rectangle(
                crossSectionCheckpoints[p].x -
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.editorsView.cam.zoom),
                crossSectionCheckpoints[p].y -
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.editorsView.cam.zoom),
                crossSectionCheckpoints[p].x +
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.editorsView.cam.zoom),
                crossSectionCheckpoints[p].y +
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.editorsView.cam.zoom),
                al_map_rgb(255, 255, 32)
            );
            drawText(
                letter, game.sysContent.fntBuiltin,
                crossSectionCheckpoints[p],
                Point(
                    AREA_EDITOR::CROSS_SECTION_POINT_RADIUS * 1.8f /
                    game.editorsView.cam.zoom,
                    AREA_EDITOR::CROSS_SECTION_POINT_RADIUS * 1.8f /
                    game.editorsView.cam.zoom
                ),
                al_map_rgb(0, 64, 64)
            );
        }
        al_draw_line(
            crossSectionCheckpoints[0].x,
            crossSectionCheckpoints[0].y,
            crossSectionCheckpoints[1].x,
            crossSectionCheckpoints[1].y,
            al_map_rgb(255, 0, 0), 3.0 / game.editorsView.cam.zoom
        );
    }
    
    //Reference image.
    if(
        referenceBitmap &&
        !previewMode &&
        (showReference || state == EDITOR_STATE_TOOLS)
    ) {
        drawBitmap(
            referenceBitmap,
            referenceCenter,
            referenceSize,
            0,
            mapAlpha(referenceAlpha)
        );
        
        if(state == EDITOR_STATE_TOOLS) {
            curTransformationWidget.draw(
                &referenceCenter,
                &referenceSize,
                nullptr,
                1.0f / game.editorsView.cam.zoom
            );
        }
    }
    
    //Sector drawing.
    if(subState == EDITOR_SUB_STATE_DRAWING) {
        for(size_t n = 1; n < drawingNodes.size(); n++) {
            al_draw_line(
                drawingNodes[n - 1].snappedSpot.x,
                drawingNodes[n - 1].snappedSpot.y,
                drawingNodes[n].snappedSpot.x,
                drawingNodes[n].snappedSpot.y,
                al_map_rgb(128, 255, 128),
                3.0 / game.editorsView.cam.zoom
            );
        }
        if(!drawingNodes.empty()) {
            ALLEGRO_COLOR newLineColor =
                interpolateColor(
                    newSectorErrorTintTimer.getRatioLeft(),
                    1, 0,
                    al_map_rgb(255, 0, 0),
                    al_map_rgb(64, 255, 64)
                );
            Point hotspot = snapPoint(game.editorsView.cursorWorldPos);
            
            al_draw_line(
                drawingNodes.back().snappedSpot.x,
                drawingNodes.back().snappedSpot.y,
                hotspot.x,
                hotspot.y,
                newLineColor,
                3.0 / game.editorsView.cam.zoom
            );
            
            if(game.options.areaEd.showEdgeLength) {
                drawLineDist(hotspot, drawingNodes.back().snappedSpot);
            }
        }
    }
    
    //New circular sector drawing.
    if(subState == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        switch(newCircleSectorStep) {
        case 1: {
            float circleRadius =
                Distance(
                    newCircleSectorCenter, newCircleSectorAnchor
                ).toFloat();
            al_draw_circle(
                newCircleSectorCenter.x,
                newCircleSectorCenter.y,
                circleRadius,
                al_map_rgb(64, 255, 64),
                3.0 / game.editorsView.cam.zoom
            );
            if(game.options.areaEd.showCircularInfo) {
                drawLineDist(
                    newCircleSectorAnchor, newCircleSectorCenter,
                    "Radius: "
                );
            }
            break;
            
        } case 2: {
            for(size_t p = 0; p < newCircleSectorPoints.size(); p++) {
                Point curPoint = newCircleSectorPoints[p];
                Point nextPoint =
                    getNextInVector(newCircleSectorPoints, p);
                ALLEGRO_COLOR color =
                    newCircleSectorValidEdges[p] ?
                    al_map_rgb(64, 255, 64) :
                    al_map_rgb(255, 0, 0);
                    
                al_draw_line(
                    curPoint.x, curPoint.y,
                    nextPoint.x, nextPoint.y,
                    color, 3.0 / game.editorsView.cam.zoom
                );
            }
            
            for(size_t p = 0; p < newCircleSectorPoints.size(); p++) {
                al_draw_filled_circle(
                    newCircleSectorPoints[p].x,
                    newCircleSectorPoints[p].y,
                    3.0 / game.editorsView.cam.zoom, al_map_rgb(192, 255, 192)
                );
            }
            
            if(game.options.areaEd.showCircularInfo) {
                drawDebugText(
                    AREA_EDITOR::MEASUREMENT_COLOR,
                    newCircleSectorPoints[0],
                    "Vertexes: " + i2s(newCircleSectorPoints.size())
                );
            }
            break;
        }
        }
    }
    
    //Quick sector height set.
    if(subState == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        Point nrCoords = quickHeightSetStartPos;
        nrCoords.x += 100.0f;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &nrCoords.x, &nrCoords.y
        );
        float offset = getQuickHeightSetOffset();
        drawDebugText(
            al_map_rgb(64, 255, 64),
            nrCoords,
            "Height " +
            string(offset < 0 ? "" : "+") + i2s(offset) + "" +
            (
                selectedSectors.size() == 1 ?
                " (" + f2s((*selectedSectors.begin())->z) + ")" :
                ""
            )
        );
    }
    
    //Path drawing.
    if(subState == EDITOR_SUB_STATE_PATH_DRAWING) {
        if(pathDrawingStop1) {
            Point hotspot = snapPoint(game.editorsView.cursorWorldPos);
            al_draw_line(
                pathDrawingStop1->pos.x,
                pathDrawingStop1->pos.y,
                hotspot.x,
                hotspot.y,
                al_map_rgb(64, 255, 64),
                3.0 / game.editorsView.cam.zoom
            );
            
            if(game.options.areaEd.showPathLinkLength) {
                drawLineDist(hotspot, pathDrawingStop1->pos);
            }
        }
    }
    
    //Selection box.
    if(selecting) {
        al_draw_rectangle(
            selectionStart.x,
            selectionStart.y,
            selectionEnd.x,
            selectionEnd.y,
            al_map_rgb(
                AREA_EDITOR::SELECTION_COLOR[0],
                AREA_EDITOR::SELECTION_COLOR[1],
                AREA_EDITOR::SELECTION_COLOR[2]
            ),
            2.0 / game.editorsView.cam.zoom
            
        );
    }
    
    //New thing marker.
    if(
        subState == EDITOR_SUB_STATE_DRAWING ||
        subState == EDITOR_SUB_STATE_CIRCLE_SECTOR ||
        subState == EDITOR_SUB_STATE_NEW_MOB ||
        subState == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        subState == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        subState == EDITOR_SUB_STATE_PATH_DRAWING ||
        subState == EDITOR_SUB_STATE_NEW_SHADOW
    ) {
        Point marker = game.editorsView.cursorWorldPos;
        
        if(subState != EDITOR_SUB_STATE_ADD_MOB_LINK) {
            marker = snapPoint(marker);
        }
        
        al_draw_line(
            marker.x - 10 / game.editorsView.cam.zoom,
            marker.y,
            marker.x + 10 / game.editorsView.cam.zoom,
            marker.y,
            COLOR_WHITE, 2.0 / game.editorsView.cam.zoom
        );
        al_draw_line(
            marker.x,
            marker.y - 10 / game.editorsView.cam.zoom,
            marker.x,
            marker.y + 10 / game.editorsView.cam.zoom,
            COLOR_WHITE, 2.0 / game.editorsView.cam.zoom
        );
    }
    
    //Delete thing marker.
    if(
        subState == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        Point marker = game.editorsView.cursorWorldPos;
        
        al_draw_line(
            marker.x - 10 / game.editorsView.cam.zoom,
            marker.y - 10 / game.editorsView.cam.zoom,
            marker.x + 10 / game.editorsView.cam.zoom,
            marker.y + 10 / game.editorsView.cam.zoom,
            COLOR_WHITE, 2.0 / game.editorsView.cam.zoom
        );
        al_draw_line(
            marker.x - 10 / game.editorsView.cam.zoom,
            marker.y + 10 / game.editorsView.cam.zoom,
            marker.x + 10 / game.editorsView.cam.zoom,
            marker.y - 10 / game.editorsView.cam.zoom,
            COLOR_WHITE, 2.0 / game.editorsView.cam.zoom
        );
    }
    
    al_use_transform(&game.identityTransform);

    drawCrossSectionGraph();
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}


/**
 * @brief Draws the cross-section graph onto the canvas.
 */
void AreaEditor::drawCrossSectionGraph() {
    if(state == EDITOR_STATE_REVIEW && showCrossSection) {
    
        Distance crossSectionWorldLength(
            crossSectionCheckpoints[0], crossSectionCheckpoints[1]
        );
        float proportion =
            (crossSectionWindowEnd.x - crossSectionWindowStart.x) /
            crossSectionWorldLength.toFloat();
            
        ALLEGRO_COLOR bgColor =
            game.options.editors.useCustomStyle ?
            changeColorLighting(game.options.editors.primaryColor, -0.3f) :
            al_map_rgb(0, 0, 64);
            
        al_draw_filled_rectangle(
            crossSectionWindowStart.x, crossSectionWindowStart.y,
            crossSectionWindowEnd.x, crossSectionWindowEnd.y,
            bgColor
        );
        
        if(showCrossSectionGrid) {
            al_draw_filled_rectangle(
                crossSectionZWindowStart.x, crossSectionZWindowStart.y,
                crossSectionZWindowEnd.x, crossSectionZWindowEnd.y,
                COLOR_BLACK
            );
        }
        
        Sector* csLeftSector =
            getSector(crossSectionCheckpoints[0], nullptr, false);
        Sector* csRightSector =
            getSector(crossSectionCheckpoints[1], nullptr, false);
            
        /**
         * @brief Info about a split.
         */
        struct Split {
        
            //--- Members ---
            
            //Sector pointers.
            Sector* sectorPtrs[2] = { nullptr, nullptr };
            
            //Line 1 intersection point.
            float l1r = 0.0f;
            
            //Line 2 intersection point.
            float l2r = 0.0f;
            
            //--- Function definitions ---
            
            /**
             * @brief Constructs a new split info object.
             *
             * @param s1 Sector 1.
             * @param s2 Sector 2.
             * @param l1r Line 1 intersection point.
             * @param l2r Line 2 intersection point.
             */
            Split(
                Sector* s1, Sector* s2, float l1r, float l2r
            ) {
                sectorPtrs[0] = s1;
                sectorPtrs[1] = s2;
                this->l1r = l1r;
                this->l2r = l2r;
            }
            
        };
        vector<Split> splits;
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* ePtr = game.curAreaData->edges[e];
            float l1r = 0;
            float l2r = 0;
            if(
                lineSegsIntersect(
                    v2p(ePtr->vertexes[0]),
                    v2p(ePtr->vertexes[1]),
                    crossSectionCheckpoints[0],
                    crossSectionCheckpoints[1],
                    &l1r, &l2r
                )
            ) {
                splits.push_back(
                    Split(ePtr->sectors[0], ePtr->sectors[1], l1r, l2r)
                );
            }
        }
        
        if(!splits.empty()) {
            sort(
                splits.begin(), splits.end(),
            [] (const Split  & i1, const Split  & i2) -> bool {
                return i1.l2r < i2.l2r;
            }
            );
            
            splits.insert(
                splits.begin(),
                Split(csLeftSector, csLeftSector, 0, 0)
            );
            splits.push_back(
                Split(csRightSector, csRightSector, 1, 1)
            );
            
            for(size_t s = 1; s < splits.size(); s++) {
                if(splits[s].sectorPtrs[0] != splits[s - 1].sectorPtrs[1]) {
                    std::swap(
                        splits[s].sectorPtrs[0], splits[s].sectorPtrs[1]
                    );
                }
            }
            
            float lowestZ = 0;
            bool gotLowestZ = false;
            for(size_t sp = 1; sp < splits.size(); sp++) {
                for(size_t se = 0; se < 2; se++) {
                    if(
                        splits[sp].sectorPtrs[se] &&
                        (
                            splits[sp].sectorPtrs[se]->z < lowestZ ||
                            !gotLowestZ
                        )
                    ) {
                        lowestZ = splits[sp].sectorPtrs[se]->z;
                        gotLowestZ = true;
                    }
                }
            }
            
            int ocrX, ocrY, ocrW, ocrH;
            al_get_clipping_rectangle(&ocrX, &ocrY, &ocrW, &ocrH);
            al_set_clipping_rectangle(
                crossSectionWindowStart.x, crossSectionWindowStart.y,
                crossSectionWindowEnd.x - crossSectionWindowStart.x,
                crossSectionWindowEnd.y - crossSectionWindowStart.y
            );
            
            for(size_t s = 1; s < splits.size(); s++) {
                if(!splits[s].sectorPtrs[0]) continue;
                drawCrossSectionSector(
                    splits[s - 1].l2r, splits[s].l2r, proportion,
                    lowestZ, splits[s].sectorPtrs[0]
                );
            }
            
            Sector* centralSector = nullptr;
            for(size_t s = 1; s < splits.size(); s++) {
                if(splits[s].l2r > 0.5) {
                    centralSector = splits[s].sectorPtrs[0];
                    break;
                }
            }
            
            if(centralSector) {
                float leaderSilhouetteW =
                    game.config.leaders.standardRadius * 2.0 * proportion;
                float leaderSilhouetteH =
                    game.config.leaders.standardHeight * proportion;
                float leaderSilhouettePivotX =
                    (
                        crossSectionWindowStart.x +
                        crossSectionWindowEnd.x
                    ) / 2.0;
                float leaderSilhouettePivotY =
                    crossSectionWindowEnd.y - 8 -
                    ((centralSector->z - lowestZ) * proportion);
                al_draw_tinted_scaled_bitmap(
                    game.sysContent.bmpLeaderSilhouetteSide,
                    COLOR_TRANSPARENT_WHITE,
                    0, 0,
                    al_get_bitmap_width(
                        game.sysContent.bmpLeaderSilhouetteSide
                    ),
                    al_get_bitmap_height(
                        game.sysContent.bmpLeaderSilhouetteSide
                    ),
                    leaderSilhouettePivotX - leaderSilhouetteW / 2.0,
                    leaderSilhouettePivotY - leaderSilhouetteH,
                    leaderSilhouetteW, leaderSilhouetteH,
                    0
                );
            }
            
            al_set_clipping_rectangle(ocrX, ocrY, ocrW, ocrH);
            
            float highestZ =
                lowestZ + crossSectionWindowEnd.y / proportion;
                
            if(showCrossSectionGrid) {
                for(float z = lowestZ; z <= highestZ; z += 50) {
                    float lineY =
                        crossSectionWindowEnd.y - 8 -
                        ((z - lowestZ) * proportion);
                    al_draw_line(
                        crossSectionWindowStart.x, lineY,
                        crossSectionZWindowStart.x + 6, lineY,
                        COLOR_WHITE, 1
                    );
                    
                    drawText(
                        i2s(z), game.sysContent.fntBuiltin,
                        Point(
                            (crossSectionZWindowStart.x + 8),
                            lineY
                        ),
                        Point(LARGE_FLOAT, 8.0f), COLOR_WHITE,
                        ALLEGRO_ALIGN_LEFT
                    );
                }
            }
            
        } else {
        
            drawText(
                "Please cross some edges.",
                game.sysContent.fntBuiltin,
                Point(
                    (
                        crossSectionWindowStart.x +
                        crossSectionWindowEnd.x
                    ) * 0.5,
                    (
                        crossSectionWindowStart.y +
                        crossSectionWindowEnd.y
                    ) * 0.5
                ),
                Point(LARGE_FLOAT, 8.0f), COLOR_WHITE
            );
            
        }
        
        float cursorSegmentRatio = 0;
        getClosestPointInLineSeg(
            crossSectionCheckpoints[0], crossSectionCheckpoints[1],
            Point(game.editorsView.cursorWorldPos.x, game.editorsView.cursorWorldPos.y),
            &cursorSegmentRatio
        );
        if(cursorSegmentRatio >= 0 && cursorSegmentRatio <= 1) {
            al_draw_line(
                crossSectionWindowStart.x +
                (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
                cursorSegmentRatio,
                crossSectionWindowStart.y,
                crossSectionWindowStart.x +
                (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
                cursorSegmentRatio,
                crossSectionWindowEnd.y,
                al_map_rgba(255, 255, 255, 128), 1
            );
        }
        
        float crossSectionX2 =
            showCrossSectionGrid ? crossSectionZWindowEnd.x :
            crossSectionWindowEnd.x;
        al_draw_line(
            crossSectionWindowStart.x, crossSectionWindowEnd.y + 1,
            crossSectionX2 + 2, crossSectionWindowEnd.y + 1,
            al_map_rgb(160, 96, 96), 2
        );
        al_draw_line(
            crossSectionX2 + 1, crossSectionWindowStart.y,
            crossSectionX2 + 1, crossSectionWindowEnd.y + 2,
            al_map_rgb(160, 96, 96), 2
        );
    }
}


/**
 * @brief Draws a sector on the cross-section view.
 *
 * @param startRatio Where the sector starts on the graph ([0, 1]).
 * @param endRatio Where the sector end on the graph ([0, 1]).
 * @param proportion Ratio of how much to resize the heights by.
 * @param lowestZ What z coordinate represents the bottom of the graph.
 * @param sectorPtr Pointer to the sector to draw.
 */
void AreaEditor::drawCrossSectionSector(
    float startRatio, float endRatio, float proportion,
    float lowestZ, const Sector* sectorPtr
) {
    float rectangleX1 =
        crossSectionWindowStart.x +
        (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
        startRatio;
    float rectangleX2 =
        crossSectionWindowStart.x +
        (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
        endRatio;
    float rectangleY =
        crossSectionWindowEnd.y - 8 -
        ((sectorPtr->z - lowestZ) * proportion);
        
    ALLEGRO_COLOR color =
        game.options.editors.useCustomStyle ?
        changeColorLighting(game.options.editors.secondaryColor, -0.2f) :
        al_map_rgb(0, 64, 0);
        
    al_draw_filled_rectangle(
        rectangleX1, rectangleY,
        rectangleX2 + 1, crossSectionWindowEnd.y + 1,
        color
    );
    al_draw_line(
        rectangleX1 + 0.5, rectangleY,
        rectangleX1 + 0.5, crossSectionWindowEnd.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangleX2 + 0.5, rectangleY,
        rectangleX2 + 0.5, crossSectionWindowEnd.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangleX1, rectangleY + 0.5,
        rectangleX2, rectangleY + 0.5,
        al_map_rgb(192, 192, 192), 1
    );
    
}


/**
 * @brief Draws debug text, used to identify edges, sectors, or vertexes.
 *
 * @param color Text color.
 * @param where Where to draw, in world coordinates.
 * @param text Text to show.
 * @param dots How many dots to draw above the text. 0, 1, or 2.
 */
void AreaEditor::drawDebugText(
    const ALLEGRO_COLOR color, const Point &where, const string &text,
    unsigned char dots
) {
    int dox = 0;
    int doy = 0;
    int dw = 0;
    int dh = 0;
    al_get_text_dimensions(
        game.sysContent.fntBuiltin, text.c_str(),
        &dox, &doy, &dw, &dh
    );
    
    float bboxW = (dw * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.editorsView.cam.zoom;
    float bboxH = (dh * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.editorsView.cam.zoom;
    
    al_draw_filled_rectangle(
        where.x - bboxW * 0.5, where.y - bboxH * 0.5,
        where.x + bboxW * 0.5, where.y + bboxH * 0.5,
        al_map_rgba(0, 0, 0, 128)
    );
    
    drawText(
        text, game.sysContent.fntBuiltin, where,
        Point(bboxW, bboxH) * 0.80f, color
    );
    
    if(dots > 0) {
        al_draw_filled_rectangle(
            where.x - 3.0f / game.editorsView.cam.zoom,
            where.y + bboxH * 0.5f,
            where.x + 3.0f / game.editorsView.cam.zoom,
            where.y + bboxH * 0.5f + 3.0f / game.editorsView.cam.zoom,
            al_map_rgba(0, 0, 0, 128)
        );
        
        if(dots == 1) {
            al_draw_filled_rectangle(
                where.x - 1.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 1.0f / game.editorsView.cam.zoom,
                where.x + 1.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 3.0f / game.editorsView.cam.zoom,
                color
            );
        } else {
            al_draw_filled_rectangle(
                where.x - 3.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 1.0f / game.editorsView.cam.zoom,
                where.x - 1.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 3.0f / game.editorsView.cam.zoom,
                color
            );
            al_draw_filled_rectangle(
                where.x + 1.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 1.0f / game.editorsView.cam.zoom,
                where.x + 3.0f / game.editorsView.cam.zoom,
                where.y + bboxH * 0.5f + 3.0f / game.editorsView.cam.zoom,
                color
            );
        }
    }
}


/**
 * @brief Draws the edge lines onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawEdges(const AreaEdCanvasStyle& style) {
    size_t nEdges = game.curAreaData->edges.size();
    for(size_t e = 0; e < nEdges; e++) {
        Edge* ePtr = game.curAreaData->edges[e];
        
        if(!ePtr->isValid()) continue;
        
        bool oneSided = true;
        bool sameZ = false;
        bool valid = true;
        bool selected = false;
        bool highlighted =
            ePtr == highlightedEdge &&
            (
                selectionFilter == SELECTION_FILTER_SECTORS ||
                selectionFilter == SELECTION_FILTER_EDGES
            ) &&
            state == EDITOR_STATE_LAYOUT;
            
        if(problemSectorPtr) {
            if(
                ePtr->sectors[0] == problemSectorPtr ||
                ePtr->sectors[1] == problemSectorPtr
            ) {
                valid = false;
            }
            
        }
        if(
            problemEdgeIntersection.e1 == ePtr ||
            problemEdgeIntersection.e2 == ePtr
        ) {
            valid = false;
        }
        
        if(isInContainer(game.curAreaData->problems.loneEdges, ePtr)) {
            valid = false;
        }
        
        if(
            isInMap(game.curAreaData->problems.nonSimples, ePtr->sectors[0]) ||
            isInMap(game.curAreaData->problems.nonSimples, ePtr->sectors[1])
        ) {
            valid = false;
        }
        
        if(ePtr->sectors[0] && ePtr->sectors[1]) oneSided = false;
        
        if(
            !oneSided &&
            ePtr->sectors[0]->z == ePtr->sectors[1]->z &&
            ePtr->sectors[0]->type == ePtr->sectors[1]->type
        ) {
            sameZ = true;
        }
        
        if(isInContainer(selectedEdges, ePtr)) {
            selected = true;
        }
        
        al_draw_line(
            ePtr->vertexes[0]->x,
            ePtr->vertexes[0]->y,
            ePtr->vertexes[1]->x,
            ePtr->vertexes[1]->y,
            (
                selected ?
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    style.selectionAlpha * 255
                ) :
                !valid ?
                al_map_rgba(192, 32,  32,  style.edgeAlpha * 255) :
                highlighted ?
                al_map_rgba(
                    style.highlightColor.r * 255,
                    style.highlightColor.g * 255,
                    style.highlightColor.b * 255,
                    style.edgeAlpha * 255
                ) :
                oneSided ?
                al_map_rgba(128, 128, 128, style.edgeAlpha * 255) :
                sameZ ?
                al_map_rgba(128, 128, 128, style.edgeAlpha * 255) :
                al_map_rgba(150, 150, 150, style.edgeAlpha * 255)
            ),
            (selected ? 3.0 : 2.0) / game.editorsView.cam.zoom
        );
        
        if(
            state == EDITOR_STATE_LAYOUT &&
            moving &&
            game.options.areaEd.showEdgeLength
        ) {
            bool drawDist = false;
            Point otherPoint;
            if(
                ePtr->vertexes[0] == moveClosestVertex &&
                !isInContainer(selectedVertexes, ePtr->vertexes[1])
            ) {
                otherPoint.x = ePtr->vertexes[1]->x;
                otherPoint.y = ePtr->vertexes[1]->y;
                drawDist = true;
            } else if(
                ePtr->vertexes[1] == moveClosestVertex &&
                !isInContainer(selectedVertexes, ePtr->vertexes[0])
            ) {
                otherPoint.x = ePtr->vertexes[0]->x;
                otherPoint.y = ePtr->vertexes[0]->y;
                drawDist = true;
            }
            
            if(drawDist) {
                drawLineDist(v2p(moveClosestVertex), otherPoint);
            }
        }
        
        if(debugTriangulation && !selectedSectors.empty()) {
            Sector* sPtr = *selectedSectors.begin();
            for(size_t t = 0; t < sPtr->triangles.size(); t++) {
                Triangle* tPtr = &sPtr->triangles[t];
                al_draw_triangle(
                    tPtr->points[0]->x,
                    tPtr->points[0]->y,
                    tPtr->points[1]->x,
                    tPtr->points[1]->y,
                    tPtr->points[2]->x,
                    tPtr->points[2]->y,
                    al_map_rgb(192, 0, 160),
                    2.0f / game.editorsView.cam.zoom
                );
            }
        }
        
        if(debugSectorIdxs) {
            Point middle(
                (ePtr->vertexes[0]->x + ePtr->vertexes[1]->x) / 2.0f,
                (ePtr->vertexes[0]->y + ePtr->vertexes[1]->y) / 2.0f
            );
            float angle =
                getAngle(v2p(ePtr->vertexes[1]), v2p(ePtr->vertexes[0]));
            drawDebugText(
                al_map_rgb(192, 255, 192),
                Point(
                    middle.x + cos(angle + TAU / 4) * 4,
                    middle.y + sin(angle + TAU / 4) * 4
                ),
                (
                    ePtr->sectorIdxs[0] == INVALID ?
                    "-" :
                    i2s(ePtr->sectorIdxs[0])
                ),
                1
            );
            
            drawDebugText(
                al_map_rgb(192, 255, 192),
                Point(
                    middle.x + cos(angle - TAU / 4) * 4,
                    middle.y + sin(angle - TAU / 4) * 4
                ),
                (
                    ePtr->sectorIdxs[1] == INVALID ?
                    "-" :
                    i2s(ePtr->sectorIdxs[1])
                ),
                2
            );
        }
        
        if(debugEdgeIdxs) {
            Point middle(
                (ePtr->vertexes[0]->x + ePtr->vertexes[1]->x) / 2.0f,
                (ePtr->vertexes[0]->y + ePtr->vertexes[1]->y) / 2.0f
            );
            drawDebugText(al_map_rgb(255, 192, 192), middle, i2s(e));
        }
    }
}


/**
 * @brief Draws a number signifying the distance between two points.
 * The number is drawn next to the main point.
 *
 * @param focus The main point.
 * @param other The point to measure against.
 * @param prefix Text to show before the measurement, if any.
 */
void AreaEditor::drawLineDist(
    const Point &focus, const Point &other, const string &prefix
) {
    float d = Distance(other, focus).toFloat();
    if(d < 64) return;
    
    float angle = getAngle(focus, other);
    Point lengthNrPos;
    lengthNrPos.x = focus.x + cos(angle) * 64.0;
    lengthNrPos.y = focus.y + sin(angle) * 64.0;
    lengthNrPos.y -= 12;
    
    drawDebugText(
        AREA_EDITOR::MEASUREMENT_COLOR, lengthNrPos, prefix + i2s(d)
    );
}



/**
 * @brief Draws the mob generators onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawMobs(const AreaEdCanvasStyle& style) {
    //Linking and containing.
    if(state == EDITOR_STATE_MOBS && style.mobAlpha > 0.0f) {
        for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
            MobGen* mPtr = game.curAreaData->mobGenerators[m];
            MobGen* m2Ptr = nullptr;
            
            if(!mPtr->type) continue;
            
            bool isSelected = isInContainer(selectedMobs, mPtr);
            
            for(size_t l = 0; l < mPtr->links.size(); l++) {
                m2Ptr = mPtr->links[l];
                if(!m2Ptr->type) continue;
                
                bool showLink =
                    isSelected || isInContainer(selectedMobs, m2Ptr);
                    
                if(showLink) {
                    drawArrow(
                        mPtr->pos, m2Ptr->pos,
                        mPtr->type->radius, m2Ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        al_map_rgb(160, 224, 64)
                    );
                }
            }
            
            if(mPtr->storedInside != INVALID) {
                m2Ptr =
                    game.curAreaData->mobGenerators[mPtr->storedInside];
                if(!m2Ptr->type) continue;
                
                bool showStore =
                    isSelected || isInContainer(selectedMobs, m2Ptr);
                    
                if(showStore) {
                    drawArrow(
                        mPtr->pos, m2Ptr->pos,
                        mPtr->type->radius, m2Ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        al_map_rgb(224, 200, 200)
                    );
                }
            }
        }
    }
    
    //The generators themselves.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* mPtr = game.curAreaData->mobGenerators[m];
        
        float radius = getMobGenRadius(mPtr);
        ALLEGRO_COLOR color = al_map_rgb(255, 0, 0);
        if(mPtr->type && mPtr != problemMobPtr) {
            color =
                changeAlpha(
                    mPtr->type->category->editorColor, style.mobAlpha * 255
                );
        }
        
        if(mPtr->type && mPtr->type->rectangularDim.x != 0) {
            drawRotatedRectangle(
                mPtr->pos, mPtr->type->rectangularDim,
                mPtr->angle, color, 1.0f / game.editorsView.cam.zoom
            );
        }
        
        //Draw children of this mob.
        if(mPtr->type) {
            for(size_t c = 0; c < mPtr->type->children.size(); c++) {
                MobType::Child* childInfo =
                    &mPtr->type->children[c];
                MobType::SpawnInfo* spawnInfo =
                    getSpawnInfoFromChildInfo(mPtr->type, childInfo);
                if(!spawnInfo) continue;
                
                Point cPos =
                    mPtr->pos +
                    rotatePoint(spawnInfo->coordsXY, mPtr->angle);
                MobType* cType =
                    game.mobCategories.findMobType(
                        spawnInfo->mobTypeName
                    );
                if(!cType) continue;
                
                if(cType->rectangularDim.x != 0) {
                    float cRot = mPtr->angle + spawnInfo->angle;
                    drawRotatedRectangle(
                        cPos, cType->rectangularDim,
                        cRot, color, 1.0f / game.editorsView.cam.zoom
                    );
                } else {
                    al_draw_circle(
                        cPos.x, cPos.y, cType->radius,
                        color, 1.0f / game.editorsView.cam.zoom
                    );
                }
                
            }
        }
        
        al_draw_filled_circle(
            mPtr->pos.x, mPtr->pos.y,
            radius, color
        );
        
        float lrw = cos(mPtr->angle) * radius;
        float lrh = sin(mPtr->angle) * radius;
        float lt = radius / 8.0;
        
        al_draw_line(
            mPtr->pos.x - lrw * 0.8, mPtr->pos.y - lrh * 0.8,
            mPtr->pos.x + lrw * 0.8, mPtr->pos.y + lrh * 0.8,
            al_map_rgba(0, 0, 0, style.mobAlpha * 255), lt
        );
        
        float tx1 = mPtr->pos.x + lrw;
        float ty1 = mPtr->pos.y + lrh;
        float tx2 =
            tx1 + cos(mPtr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty2 =
            ty1 + sin(mPtr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float tx3 =
            tx1 + cos(mPtr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty3 =
            ty1 + sin(mPtr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
            
        al_draw_filled_triangle(
            tx1, ty1,
            tx2, ty2,
            tx3, ty3,
            al_map_rgba(0, 0, 0, style.mobAlpha * 255)
        );
        
        bool isSelected = isInContainer(selectedMobs, mPtr);
        bool isMissionRequirement =
            subState == EDITOR_SUB_STATE_MISSION_MOBS &&
            isInContainer(game.curAreaData->mission.goalMobIdxs, m);
        bool isHighlighted =
            highlightedMob == mPtr &&
            state == EDITOR_STATE_MOBS;
            
        if(isSelected || isMissionRequirement) {
            al_draw_filled_circle(
                mPtr->pos.x, mPtr->pos.y, radius,
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    style.selectionAlpha * 255
                )
            );
            
            if(
                game.options.areaEd.showTerritory &&
                mPtr->type &&
                mPtr->type->territoryRadius > 0 &&
                isSelected
            ) {
                al_draw_circle(
                    mPtr->pos.x, mPtr->pos.y, mPtr->type->territoryRadius,
                    al_map_rgb(240, 240, 192), 1.0f / game.editorsView.cam.zoom
                );
            }
            if(
                game.options.areaEd.showTerritory &&
                mPtr->type &&
                mPtr->type->terrainRadius > 0 &&
                isSelected
            ) {
                al_draw_circle(
                    mPtr->pos.x, mPtr->pos.y, mPtr->type->terrainRadius,
                    al_map_rgb(240, 192, 192), 1.0f / game.editorsView.cam.zoom
                );
            }
        } else if(isHighlighted) {
            al_draw_filled_circle(
                mPtr->pos.x, mPtr->pos.y, radius,
                al_map_rgba(
                    style.highlightColor.r * 255,
                    style.highlightColor.g * 255,
                    style.highlightColor.b * 255,
                    64
                )
            );
        }
        
    }
}


/**
 * @brief Draws the path stops and links onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawPaths(const AreaEdCanvasStyle& style) {
    if(state == EDITOR_STATE_PATHS) {
        //Stops.
        for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
            PathStop* sPtr = game.curAreaData->pathStops[s];
            bool highlighted = highlightedPathStop == sPtr;
            ALLEGRO_COLOR color;
            if(hasFlag(sPtr->flags, PATH_STOP_FLAG_SCRIPT_ONLY)) {
                color = al_map_rgba(187, 102, 34, 224);
            } else if(hasFlag(sPtr->flags, PATH_STOP_FLAG_LIGHT_LOAD_ONLY)) {
                color = al_map_rgba(102, 170, 34, 224);
            } else if(hasFlag(sPtr->flags, PATH_STOP_FLAG_AIRBORNE_ONLY)) {
                color = al_map_rgba(187, 102, 153, 224);
            } else {
                color = al_map_rgb(88, 177, 177);
            }
            al_draw_filled_circle(
                sPtr->pos.x, sPtr->pos.y,
                sPtr->radius,
                color
            );
            
            if(isInContainer(selectedPathStops, sPtr)) {
                al_draw_filled_circle(
                    sPtr->pos.x, sPtr->pos.y, sPtr->radius,
                    al_map_rgba(
                        AREA_EDITOR::SELECTION_COLOR[0],
                        AREA_EDITOR::SELECTION_COLOR[1],
                        AREA_EDITOR::SELECTION_COLOR[2],
                        style.selectionAlpha * 255
                    )
                );
            } else if(highlighted) {
                al_draw_filled_circle(
                    sPtr->pos.x, sPtr->pos.y, sPtr->radius,
                    al_map_rgba(
                        style.highlightColor.r * 255,
                        style.highlightColor.g * 255,
                        style.highlightColor.b * 255,
                        128
                    )
                );
            }
            
            if(debugPathIdxs) {
                drawDebugText(
                    al_map_rgb(80, 192, 192), sPtr->pos, i2s(s)
                );
            }
        }
        
        //Links.
        for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
            PathStop* sPtr = game.curAreaData->pathStops[s];
            for(size_t l = 0; l < sPtr->links.size(); l++) {
                PathLink* lPtr = sPtr->links[l];
                PathStop* s2Ptr = lPtr->endPtr;
                bool oneWay = !lPtr->endPtr->getLink(sPtr);
                bool selected = isInContainer(selectedPathLinks, lPtr);
                bool highlighted = highlightedPathLink == lPtr;
                ALLEGRO_COLOR color = COLOR_WHITE;
                if(selected) {
                    color =
                        al_map_rgba(
                            AREA_EDITOR::SELECTION_COLOR[0],
                            AREA_EDITOR::SELECTION_COLOR[1],
                            AREA_EDITOR::SELECTION_COLOR[2],
                            style.selectionAlpha * 255
                        );
                } else if(highlighted) {
                    color =
                        al_map_rgba(
                            style.highlightColor.r * 255,
                            style.highlightColor.g * 255,
                            style.highlightColor.b * 255,
                            255
                        );
                } else {
                    switch(lPtr->type) {
                    case PATH_LINK_TYPE_NORMAL: {
                        color = al_map_rgba(34, 136, 187, 224);
                        break;
                    } case PATH_LINK_TYPE_LEDGE: {
                        color = al_map_rgba(180, 180, 64, 224);
                        break;
                    }
                    }
                    if(!oneWay) {
                        color = changeColorLighting(color, 0.33f);
                    }
                }
                
                float angle =
                    getAngle(sPtr->pos, s2Ptr->pos);
                Point offset1 =
                    angleToCoordinates(angle, sPtr->radius);
                Point offset2 =
                    angleToCoordinates(angle, s2Ptr->radius);
                al_draw_line(
                    sPtr->pos.x + offset1.x,
                    sPtr->pos.y + offset1.y,
                    s2Ptr->pos.x - offset2.x,
                    s2Ptr->pos.y - offset2.y,
                    color,
                    AREA_EDITOR::PATH_LINK_THICKNESS / game.editorsView.cam.zoom
                );
                
                if(
                    state == EDITOR_STATE_PATHS &&
                    moving &&
                    game.options.areaEd.showPathLinkLength
                ) {
                    bool drawDist = false;
                    Point otherPoint;
                    if(
                        lPtr->startPtr == moveClosestStop &&
                        !isInContainer(selectedPathStops, lPtr->endPtr)
                    ) {
                        otherPoint.x = lPtr->endPtr->pos.x;
                        otherPoint.y = lPtr->endPtr->pos.y;
                        drawDist = true;
                    } else if(
                        lPtr->endPtr == moveClosestStop &&
                        !isInContainer(selectedPathStops, lPtr->startPtr)
                    ) {
                        otherPoint.x = lPtr->startPtr->pos.x;
                        otherPoint.y = lPtr->startPtr->pos.y;
                        drawDist = true;
                    }
                    
                    if(drawDist) {
                        drawLineDist(moveClosestStop->pos, otherPoint);
                    }
                }
                
                if(debugPathIdxs && (oneWay || s < sPtr->links[l]->endIdx)) {
                    Point middle = (sPtr->pos + s2Ptr->pos) / 2.0f;
                    drawDebugText(
                        al_map_rgb(96, 104, 224),
                        Point(
                            middle.x + cos(angle + TAU / 4) * 4,
                            middle.y + sin(angle + TAU / 4) * 4
                        ),
                        f2s(sPtr->links[l]->distance)
                    );
                }
                
                if(oneWay) {
                    //Draw a triangle down the middle.
                    float midX =
                        (sPtr->pos.x + s2Ptr->pos.x) / 2.0f;
                    float midY =
                        (sPtr->pos.y + s2Ptr->pos.y) / 2.0f;
                    const float delta =
                        (AREA_EDITOR::PATH_LINK_THICKNESS * 4) / game.editorsView.cam.zoom;
                        
                    al_draw_filled_triangle(
                        midX + cos(angle) * delta,
                        midY + sin(angle) * delta,
                        midX + cos(angle + TAU / 4) * delta,
                        midY + sin(angle + TAU / 4) * delta,
                        midX + cos(angle - TAU / 4) * delta,
                        midY + sin(angle - TAU / 4) * delta,
                        color
                    );
                }
            }
        }
        
        //Closest stop line.
        if(showClosestStop) {
            PathStop* closest = nullptr;
            float closestDist = FLT_MAX;
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* sPtr = game.curAreaData->pathStops[s];
                float d =
                    Distance(game.editorsView.cursorWorldPos, sPtr->pos).toFloat() -
                    sPtr->radius;
                    
                if(!closest || d < closestDist) {
                    closest = sPtr;
                    closestDist = d;
                }
            }
            
            if(closest) {
                al_draw_line(
                    game.editorsView.cursorWorldPos.x, game.editorsView.cursorWorldPos.y,
                    closest->pos.x, closest->pos.y,
                    al_map_rgb(192, 128, 32), 2.0 / game.editorsView.cam.zoom
                );
            }
        }
        
        //Path preview.
        if(showPathPreview) {
            //Draw the lines of the path.
            ALLEGRO_COLOR linesColor = al_map_rgb(255, 187, 136);
            ALLEGRO_COLOR invalidLinesColor = al_map_rgb(221, 17, 17);
            float linesThickness = 4.0f / game.editorsView.cam.zoom;
            
            if(!pathPreview.empty()) {
                al_draw_line(
                    pathPreviewCheckpoints[0].x,
                    pathPreviewCheckpoints[0].y,
                    pathPreview[0]->pos.x,
                    pathPreview[0]->pos.y,
                    linesColor, linesThickness
                );
                for(size_t s = 0; s < pathPreview.size() - 1; s++) {
                    al_draw_line(
                        pathPreview[s]->pos.x,
                        pathPreview[s]->pos.y,
                        pathPreview[s + 1]->pos.x,
                        pathPreview[s + 1]->pos.y,
                        linesColor, linesThickness
                    );
                }
                al_draw_line(
                    pathPreview.back()->pos.x,
                    pathPreview.back()->pos.y,
                    pathPreviewCheckpoints[1].x,
                    pathPreviewCheckpoints[1].y,
                    linesColor, linesThickness
                );
            } else if(
                pathPreviewResult == PATH_RESULT_DIRECT ||
                pathPreviewResult == PATH_RESULT_DIRECT_NO_STOPS
            ) {
                al_draw_line(
                    pathPreviewCheckpoints[0].x,
                    pathPreviewCheckpoints[0].y,
                    pathPreviewCheckpoints[1].x,
                    pathPreviewCheckpoints[1].y,
                    linesColor, linesThickness
                );
            } else {
                for(size_t c = 0; c < 2; c++) {
                    if(pathPreviewClosest[c]) {
                        al_draw_line(
                            pathPreviewClosest[c]->pos.x,
                            pathPreviewClosest[c]->pos.y,
                            pathPreviewCheckpoints[c].x,
                            pathPreviewCheckpoints[c].y,
                            invalidLinesColor, linesThickness
                        );
                    }
                }
            }
            
            //Draw the checkpoints.
            for(unsigned char c = 0; c < 2; c++) {
                string letter = (c == 0 ? "A" : "B");
                
                const float factor =
                    AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS / game.editorsView.cam.zoom;
                al_draw_filled_rectangle(
                    pathPreviewCheckpoints[c].x - factor,
                    pathPreviewCheckpoints[c].y - factor,
                    pathPreviewCheckpoints[c].x + factor,
                    pathPreviewCheckpoints[c].y + factor,
                    al_map_rgb(240, 224, 160)
                );
                drawText(
                    letter, game.sysContent.fntBuiltin,
                    pathPreviewCheckpoints[c],
                    Point(
                        AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS * 1.8f /
                        game.editorsView.cam.zoom,
                        AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS * 1.8f /
                        game.editorsView.cam.zoom
                    ),
                    al_map_rgb(0, 64, 64)
                );
            }
        }
    }
}


/**
 * @brief Draws the sectors onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawSectors(const AreaEdCanvasStyle& style) {
    //Edge offset effect updates.
    if(style.wallShadowAlpha > 0.0f) {
        updateOffsetEffectBuffer(
            game.editorsView.box[0], game.editorsView.box[1],
            game.liquidLimitEffectCaches,
            game.liquidLimitEffectBuffer,
            true, game.editorsView
        );
        updateOffsetEffectBuffer(
            game.editorsView.box[0], game.editorsView.box[1],
            game.wallSmoothingEffectCaches,
            game.wallOffsetEffectBuffer,
            true, game.editorsView
        );
        updateOffsetEffectBuffer(
            game.editorsView.box[0], game.editorsView.box[1],
            game.wallShadowEffectCaches,
            game.wallOffsetEffectBuffer,
            false, game.editorsView
        );
    }

    //Draw each one.
    size_t nSectors = game.curAreaData->sectors.size();
    for(size_t s = 0; s < nSectors; s++) {
        Sector* sPtr;
        if(
            preMoveAreaData &&
            moving &&
            (
                state == EDITOR_STATE_LAYOUT
            )
        ) {
            sPtr = preMoveAreaData->sectors[s];
        } else {
            sPtr = game.curAreaData->sectors[s];
        }
        
        bool viewHeightmap = false;
        bool viewBrightness = false;
        
        //Textures, liquids, etc.
        if(
            game.options.areaEd.viewMode == VIEW_MODE_TEXTURES ||
            previewMode
        ) {
            if(previewMode) {
                bool hasLiquid = false;
                if(sPtr->hazard) {
                    Liquid* lPtr = sPtr->hazard->associatedLiquid;
                    if(lPtr) {
                        drawLiquid(sPtr, lPtr, Point(), 1.0f, game.timePassed);
                        hasLiquid = true;
                    }
                }
                if(!hasLiquid) {
                    drawSectorTexture(sPtr, Point(), 1.0, style.textureAlpha);
                }
            } else {
                drawSectorTexture(sPtr, Point(), 1.0, style.textureAlpha);
            }
            
            if(style.wallShadowAlpha > 0.0f) {
                drawSectorEdgeOffsets(
                    sPtr, game.liquidLimitEffectBuffer, 1.0f, game.editorsView
                );
                drawSectorEdgeOffsets(
                    sPtr, game.wallOffsetEffectBuffer, style.wallShadowAlpha,
                    game.editorsView
                );
            }
            
        } else if(game.options.areaEd.viewMode == VIEW_MODE_HEIGHTMAP) {
            viewHeightmap = true;
            
        } else if(game.options.areaEd.viewMode == VIEW_MODE_BRIGHTNESS) {
            viewBrightness = true;
            
        }
        
        //Selection effect.
        bool selected = isInContainer(selectedSectors, sPtr);
        bool valid = true;
        bool highlighted =
            sPtr == highlightedSector &&
            selectionFilter == SELECTION_FILTER_SECTORS &&
            state == EDITOR_STATE_LAYOUT;
            
        if(isInMap(game.curAreaData->problems.nonSimples, sPtr)) {
            valid = false;
        }
        if(sPtr == problemSectorPtr) {
            valid = false;
        }
        
        if(
            selected || !valid || viewHeightmap ||
            viewBrightness || showBlockingSectors || highlighted
        ) {
            for(size_t t = 0; t < sPtr->triangles.size(); t++) {
            
                ALLEGRO_VERTEX av[3];
                for(size_t v = 0; v < 3; v++) {
                    if(!valid) {
                        av[v].color = al_map_rgba(160, 16, 16, 224);
                    } else if(showBlockingSectors) {
                        av[v].color =
                            sPtr->type == SECTOR_TYPE_BLOCKING ?
                            AREA_EDITOR::BLOCKING_COLOR :
                            AREA_EDITOR::NON_BLOCKING_COLOR;
                    } else if(viewBrightness) {
                        av[v].color =
                            al_map_rgba(
                                sPtr->brightness * 0.7,
                                sPtr->brightness * 0.8,
                                sPtr->brightness * 0.7,
                                255
                            );
                    } else if(viewHeightmap) {
                        unsigned char g =
                            interpolateNumber(
                                sPtr->z,
                                style.lowestSectorZ, style.highestSectorZ,
                                0, 224
                            );
                        av[v].color =
                            al_map_rgba(g, g + 31, g, 255);
                    } else {
                        av[v].color =
                            al_map_rgba(
                                AREA_EDITOR::SELECTION_COLOR[0],
                                AREA_EDITOR::SELECTION_COLOR[1],
                                AREA_EDITOR::SELECTION_COLOR[2],
                                style.selectionAlpha * 0.5 * 255
                            );
                        if(highlighted && !selected) {
                            av[v].color =
                                al_map_rgba(
                                    style.highlightColor.r * 255,
                                    style.highlightColor.g * 255,
                                    style.highlightColor.b * 255,
                                    16
                                );
                        }
                    }
                    av[v].u = 0;
                    av[v].v = 0;
                    av[v].x = sPtr->triangles[t].points[v]->x;
                    av[v].y = sPtr->triangles[t].points[v]->y;
                    av[v].z = 0;
                }
                
                al_draw_prim(
                    av, nullptr, nullptr,
                    0, 3, ALLEGRO_PRIM_TRIANGLE_LIST
                );
                
            }
        }
    }
}


/**
 * @brief Draws the tree shadows onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawTreeShadows(const AreaEdCanvasStyle& style) {
    if(
        state == EDITOR_STATE_DETAILS ||
        (previewMode && showShadows)
    ) {
        for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
        
            TreeShadow* sPtr = game.curAreaData->treeShadows[s];
            if(
                !previewMode &&
                sPtr == selectedShadow
            ) {
                //Draw a white rectangle to contrast the shadow better.
                ALLEGRO_TRANSFORM tra, current;
                al_identity_transform(&tra);
                al_rotate_transform(&tra, sPtr->angle);
                al_translate_transform(
                    &tra, sPtr->center.x, sPtr->center.y
                );
                al_copy_transform(&current, al_get_current_transform());
                al_compose_transform(&tra, &current);
                al_use_transform(&tra);
                
                al_draw_filled_rectangle(
                    -sPtr->size.x / 2.0,
                    -sPtr->size.y / 2.0,
                    sPtr->size.x / 2.0,
                    sPtr->size.y / 2.0,
                    al_map_rgba(255, 255, 255, 96 * (sPtr->alpha / 255.0))
                );
                
                al_use_transform(&current);
            }
            
            drawBitmap(
                sPtr->bitmap, sPtr->center, sPtr->size,
                sPtr->angle, mapAlpha(sPtr->alpha)
            );
            
            if(state == EDITOR_STATE_DETAILS) {
                Point minCoords, maxCoords;
                getTransformedRectangleBBox(
                    sPtr->center, sPtr->size, sPtr->angle,
                    &minCoords, &maxCoords
                );
                
                if(selectedShadow != sPtr) {
                    al_draw_rectangle(
                        minCoords.x, minCoords.y, maxCoords.x, maxCoords.y,
                        al_map_rgb(128, 128, 64), 2.0 / game.editorsView.cam.zoom
                    );
                }
            }
        }
        if(selectedShadow) {
            curTransformationWidget.draw(
                &selectedShadow->center,
                &selectedShadow->size,
                &selectedShadow->angle,
                1.0f / game.editorsView.cam.zoom
            );
        }
    }
}


/**
 * @brief Draws the vertex points onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawVertexes(const AreaEdCanvasStyle& style) {
    if(state == EDITOR_STATE_LAYOUT) {
        size_t nVertexes = game.curAreaData->vertexes.size();
        for(size_t v = 0; v < nVertexes; v++) {
            Vertex* vPtr = game.curAreaData->vertexes[v];
            bool selected = isInContainer(selectedVertexes, vPtr);
            bool valid = vPtr != problemVertexPtr;
            bool highlighted =
                highlightedVertex == vPtr &&
                (
                    selectionFilter == SELECTION_FILTER_SECTORS ||
                    selectionFilter == SELECTION_FILTER_EDGES ||
                    selectionFilter == SELECTION_FILTER_VERTEXES
                );
            drawFilledDiamond(
                v2p(vPtr), 3.0 / game.editorsView.cam.zoom,
                selected ?
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    style.selectionAlpha * 255
                ) :
                !valid ?
                al_map_rgb(192, 32, 32) :
                highlighted ?
                al_map_rgba(
                    style.highlightColor.r * 255,
                    style.highlightColor.g * 255,
                    style.highlightColor.b * 255,
                    style.edgeAlpha * 255
                ) :
                al_map_rgba(80, 160, 255, style.edgeAlpha * 255)
            );
            
            if(debugVertexIdxs) {
                drawDebugText(
                    al_map_rgb(192, 192, 255),
                    v2p(vPtr), i2s(v)
                );
            }
        }
    }
}
