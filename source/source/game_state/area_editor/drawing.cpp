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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/allegro_utils.h"
#include "../../util/container_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the area editor.
 */
void AreaEditor::doDrawing() {
    if(hackSkipDrawing) {
        //Skip drawing for one frame.
        //This hack fixes a weird glitch where if you quick play an area
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
    const Point& start, const Point& end,
    float startOffset, float endOffset,
    float thickness, const ALLEGRO_COLOR& color
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
    RectCorners canvasCorners = game.editorsView.getWindowCorners();
    
    al_set_clipping_rectangle(
        canvasCorners.tl.x, canvasCorners.tl.y,
        game.editorsView.windowRect.size.x, game.editorsView.windowRect.size.y
    );
    
    al_clear_to_color(COLOR_BLACK);
    
    if(!game.curArea) {
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
    
    if(
        game.options.areaEd.viewMode == VIEW_MODE_HEIGHTMAP &&
        !game.curArea->sectors.empty()
    ) {
        style.lowestSectorZ = game.curArea->sectors[0]->z;
        style.highestSectorZ = style.lowestSectorZ;
        
        for(size_t s = 1; s < game.curArea->sectors.size(); s++) {
            style.lowestSectorZ =
                std::min(style.lowestSectorZ, game.curArea->sectors[s]->z);
            style.highestSectorZ =
                std::max(style.highestSectorZ, game.curArea->sectors[s]->z);
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
    
    //Draw!
    drawSectors(style);
    
    drawGrid(
        game.options.areaEd.gridInterval,
        multAlpha(EDITOR::GRID_COLOR_MAJOR, style.gridAlpha),
        multAlpha(EDITOR::GRID_COLOR_MINOR, style.gridAlpha)
    );
    
    //0,0 marker.
    al_draw_line(
        -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2, 0,
        multAlpha(EDITOR::GRID_COLOR_ORIGIN, style.gridAlpha),
        1.0f / game.editorsView.cam.zoom
    );
    al_draw_line(
        0, -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2,
        multAlpha(EDITOR::GRID_COLOR_ORIGIN, style.gridAlpha),
        1.0f / game.editorsView.cam.zoom
    );
    
    drawEdges(style);
    
    drawVertexes(style);
    
    if(state == EDITOR_STATE_LAYOUT) {
        drawSelectionAndTransformationThings(
            layoutSelCtrl, curTransformationWidget,
            !game.options.areaEd.selTrans
        );
    }
    
    drawMobs(style);
    
    drawPaths(style);
    
    if(state == EDITOR_STATE_DETAILS) {
        drawTreeShadows(style);
        drawRegions(style);
        drawSelectionAndTransformationThings(
            detailsSelCtrl, curTransformationWidget, false
        );
    }
    
    drawReminders(style);
    
    //Cross-section points and line.
    if(state == EDITOR_STATE_REVIEW && showCrossSection) {
        const ALLEGRO_COLOR POINT_BG_COLOR = al_map_rgb(255, 255, 32);
        const ALLEGRO_COLOR POINT_TEXT_COLOR = al_map_rgb(0, 64, 64);
        const ALLEGRO_COLOR LINE_COLOR = al_map_rgb(255, 0, 0);
        
        for(unsigned char p = 0; p < 2; p++) {
            string letter = (p == 0 ? "A" : "B");
            float radius =
                AREA_EDITOR::CROSS_SECTION_POINT_RADIUS /
                game.editorsView.cam.zoom;
            al_draw_filled_rectangle(
                crossSectionCheckpoints[p].x - radius,
                crossSectionCheckpoints[p].y - radius,
                crossSectionCheckpoints[p].x + radius,
                crossSectionCheckpoints[p].y + radius,
                POINT_BG_COLOR
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
                POINT_TEXT_COLOR
            );
        }
        al_draw_line(
            crossSectionCheckpoints[0].x,
            crossSectionCheckpoints[0].y,
            crossSectionCheckpoints[1].x,
            crossSectionCheckpoints[1].y,
            LINE_COLOR, 3.0 / game.editorsView.cam.zoom
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
            referenceRect.center, referenceRect.size,
            0, referenceTint
        );
        
        if(state == EDITOR_STATE_TOOLS) {
            curTransformationWidget.draw(
                game.editorsView.mouseCursorWorldPos,
                &referenceRect.center, &referenceRect.size,
                nullptr, 1.0f / game.editorsView.cam.zoom
            );
        }
    }
    
    //Sector drawing.
    if(subState == EDITOR_SUB_STATE_DRAWING) {
        const ALLEGRO_COLOR ERROR_COLOR = al_map_rgb(255, 0, 0);
        
        for(size_t n = 1; n < drawingNodes.size(); n++) {
            al_draw_line(
                drawingNodes[n - 1].snappedSpot.x,
                drawingNodes[n - 1].snappedSpot.y,
                drawingNodes[n].snappedSpot.x,
                drawingNodes[n].snappedSpot.y,
                AREA_EDITOR::DRAWING_OLD_LINE_COLOR,
                3.0 / game.editorsView.cam.zoom
            );
        }
        if(!drawingNodes.empty()) {
            ALLEGRO_COLOR newLineColor =
                interpolateColor(
                    newSectorErrorTintTimer.getRatioLeft(),
                    1.0f, 0.0f, ERROR_COLOR, AREA_EDITOR::DRAWING_NEW_LINE_COLOR
                );
            Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
            
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
        const ALLEGRO_COLOR ERROR_COLOR = al_map_rgb(255, 0, 0);
        
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
                AREA_EDITOR::DRAWING_NEW_LINE_COLOR,
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
            forIdx(p, newCircleSectorPoints) {
                Point curPoint = newCircleSectorPoints[p];
                Point nextPoint =
                    getNextInVectorByIdx(newCircleSectorPoints, p);
                ALLEGRO_COLOR color =
                    newCircleSectorValidEdges[p] ?
                    AREA_EDITOR::DRAWING_NEW_LINE_COLOR :
                    ERROR_COLOR;
                    
                al_draw_line(
                    curPoint.x, curPoint.y,
                    nextPoint.x, nextPoint.y,
                    color, 3.0 / game.editorsView.cam.zoom
                );
            }
            
            forIdx(p, newCircleSectorPoints) {
                al_draw_filled_circle(
                    newCircleSectorPoints[p].x,
                    newCircleSectorPoints[p].y,
                    3.0 / game.editorsView.cam.zoom,
                    AREA_EDITOR::DRAWING_OLD_LINE_COLOR
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
        const ALLEGRO_COLOR TEXT_COLOR = al_map_rgb(64, 255, 64);
        Point nrCoords = quickHeightSetStartPos;
        nrCoords.x += 100.0f;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &nrCoords.x, &nrCoords.y
        );
        float offset = getQuickHeightSetOffset();
        Sector* singleSector = nullptr;
        if(sectorSelection.hasOne()) {
            singleSector =
                game.curArea->sectors[sectorSelection.getSingleItemIdx()];
        }
        drawDebugText(
            TEXT_COLOR, nrCoords,
            "Height " +
            string(offset < 0 ? "" : "+") + i2s(offset) + "" +
            (singleSector ? " (" + f2s(singleSector->z) + ")" : "")
        );
    }
    
    //Path drawing.
    if(subState == EDITOR_SUB_STATE_PATH_DRAWING) {
        if(pathDrawingStop1) {
            Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
            al_draw_line(
                pathDrawingStop1->center.x,
                pathDrawingStop1->center.y,
                hotspot.x,
                hotspot.y,
                AREA_EDITOR::DRAWING_NEW_LINE_COLOR,
                3.0 / game.editorsView.cam.zoom
            );
            
            if(game.options.areaEd.showPathLinkLength) {
                drawLineDist(hotspot, pathDrawingStop1->center);
            }
        }
    }
    
    //New thing marker.
    if(
        subState == EDITOR_SUB_STATE_DRAWING ||
        subState == EDITOR_SUB_STATE_CIRCLE_SECTOR ||
        subState == EDITOR_SUB_STATE_NEW_MOB ||
        subState == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        subState == EDITOR_SUB_STATE_NEW_MOB_LINK ||
        subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        subState == EDITOR_SUB_STATE_PATH_DRAWING ||
        subState == EDITOR_SUB_STATE_NEW_SHADOW
    ) {
        Point marker = game.editorsView.mouseCursorWorldPos;
        
        if(subState != EDITOR_SUB_STATE_NEW_MOB_LINK) {
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
        Point marker = game.editorsView.mouseCursorWorldPos;
        
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
    const ALLEGRO_COLOR DEF_BG_COLOR = al_map_rgb(0, 0, 64);
    const ALLEGRO_COLOR LINE_COLOR = al_map_rgb(160, 96, 96);
    
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
            DEF_BG_COLOR;
            
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
        
            //--- Public members ---
            
            //Sector pointers.
            Sector* sectorPtrs[2] = { nullptr, nullptr };
            
            //Line 1 intersection point.
            float l1r = 0.0f;
            
            //Line 2 intersection point.
            float l2r = 0.0f;
            
            //--- Public function definitions ---
            
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
        forIdx(e, game.curArea->edges) {
            Edge* ePtr = game.curArea->edges[e];
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
            [] (const Split & i1, const Split & i2) -> bool {
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
            Point(
                game.editorsView.mouseCursorWorldPos.x,
                game.editorsView.mouseCursorWorldPos.y
            ),
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
                mapAlpha(128), 1
            );
        }
        
        float crossSectionX2 =
            showCrossSectionGrid ? crossSectionZWindowEnd.x :
            crossSectionWindowEnd.x;
        al_draw_line(
            crossSectionWindowStart.x, crossSectionWindowEnd.y + 1,
            crossSectionX2 + 2, crossSectionWindowEnd.y + 1,
            LINE_COLOR, 2
        );
        al_draw_line(
            crossSectionX2 + 1, crossSectionWindowStart.y,
            crossSectionX2 + 1, crossSectionWindowEnd.y + 2,
            LINE_COLOR, 2
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
    const ALLEGRO_COLOR DEF_SECTOR_COLOR = al_map_rgb(0, 64, 0);
    const ALLEGRO_COLOR LINE_COLOR = al_map_rgb(192, 192, 192);
    
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
        DEF_SECTOR_COLOR;
        
    al_draw_filled_rectangle(
        rectangleX1, rectangleY,
        rectangleX2 + 1, crossSectionWindowEnd.y + 1,
        color
    );
    al_draw_line(
        rectangleX1 + 0.5, rectangleY,
        rectangleX1 + 0.5, crossSectionWindowEnd.y,
        LINE_COLOR, 1
    );
    al_draw_line(
        rectangleX2 + 0.5, rectangleY,
        rectangleX2 + 0.5, crossSectionWindowEnd.y,
        LINE_COLOR, 1
    );
    al_draw_line(
        rectangleX1, rectangleY + 0.5,
        rectangleX2, rectangleY + 0.5,
        LINE_COLOR, 1
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
    const ALLEGRO_COLOR color, const Point& where, const string& text,
    unsigned char dots
) {
    const ALLEGRO_COLOR BG_COLOR = al_map_rgba(0, 0, 0, 128);
    const ALLEGRO_COLOR DOT_BG_COLOR = al_map_rgba(0, 0, 0, 128);
    
    int dox = 0;
    int doy = 0;
    int dw = 0;
    int dh = 0;
    al_get_text_dimensions(
        game.sysContent.fntBuiltin, text.c_str(),
        &dox, &doy, &dw, &dh
    );
    
    float bboxW =
        (dw * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.editorsView.cam.zoom;
    float bboxH =
        (dh * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.editorsView.cam.zoom;
        
    al_draw_filled_rectangle(
        where.x - bboxW * 0.5, where.y - bboxH * 0.5,
        where.x + bboxW * 0.5, where.y + bboxH * 0.5,
        BG_COLOR
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
            DOT_BG_COLOR
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
    const ALLEGRO_COLOR VALID_EDGE_COLOR = al_map_rgb(192, 32, 32);
    const ALLEGRO_COLOR ONE_SIDED_EDGE_COLOR = al_map_rgb(128, 128, 128);
    const ALLEGRO_COLOR SAME_Z_EDGE_COLOR = al_map_rgb(128, 128, 128);
    const ALLEGRO_COLOR NORMAL_EDGE_COLOR = al_map_rgb(150, 150, 150);
    const ALLEGRO_COLOR DEBUG_TRIANGLE_COLOR = al_map_rgb(192, 0, 160);
    const ALLEGRO_COLOR DEBUG_SECTOR_A_COLOR = al_map_rgb(216, 224, 128);
    const ALLEGRO_COLOR DEBUG_SECTOR_B_COLOR = al_map_rgb(128, 224, 160);
    const ALLEGRO_COLOR DEBUG_EDGE_IDX_COLOR = al_map_rgb(255, 192, 192);
    
    bool canSelectEdges =
        (
            selectionFilter == SELECTION_FILTER_SECTORS ||
            selectionFilter == SELECTION_FILTER_EDGES
        ) && state == EDITOR_STATE_LAYOUT;
        
    size_t nEdges = game.curArea->edges.size();
    for(size_t e = 0; e < nEdges; e++) {
        //Setup.
        Edge* ePtr = game.curArea->edges[e];
        
        if(!ePtr->isValid()) continue;
        
        bool isOneSided = !ePtr->sectors[0] || !ePtr->sectors[1];
        bool isSameZ = false;
        bool isValid = true;
        bool isSelected = edgeSelection.contains(e);
        bool isHighlighted = canSelectEdges && (ePtr == highlightedEdge);
        
        if(problemSectorPtr) {
            if(
                ePtr->sectors[0] == problemSectorPtr ||
                ePtr->sectors[1] == problemSectorPtr
            ) {
                isValid = false;
            }
            
        }
        if(
            problemEdgeIntersection.e1 == ePtr ||
            problemEdgeIntersection.e2 == ePtr
        ) {
            isValid = false;
        }
        
        if(isInContainer(game.curArea->problems.loneEdges, ePtr)) {
            isValid = false;
        }
        
        if(
            isInMap(game.curArea->problems.nonSimples, ePtr->sectors[0]) ||
            isInMap(game.curArea->problems.nonSimples, ePtr->sectors[1])
        ) {
            isValid = false;
        }
        
        if(
            !isOneSided &&
            ePtr->sectors[0]->z == ePtr->sectors[1]->z &&
            ePtr->sectors[0]->type == ePtr->sectors[1]->type
        ) {
            isSameZ = true;
        }
        
        //Pick the color.
        ALLEGRO_COLOR color;
        if(!isValid) {
            color = VALID_EDGE_COLOR;
        } else if(isOneSided) {
            color = ONE_SIDED_EDGE_COLOR;
        } else if(isSameZ) {
            color = SAME_Z_EDGE_COLOR;
        } else {
            color = NORMAL_EDGE_COLOR;
        }
        if(isSelected) {
            color = getSelectionEffectReplacementColor(color);
        } else if(isHighlighted) {
            color = getHighlightEffectReplacementColor(color);
        }
        
        //Draw it.
        al_draw_line(
            ePtr->vertexes[0]->x,
            ePtr->vertexes[0]->y,
            ePtr->vertexes[1]->x,
            ePtr->vertexes[1]->y,
            multAlpha(color, style.edgeAlpha),
            (isSelected ? 3.0 : 2.0) / game.editorsView.cam.zoom
        );
        
        //Draw the edge lengths.
        if(
            state == EDITOR_STATE_LAYOUT &&
            moving &&
            game.options.areaEd.showEdgeLength
        ) {
            bool drawDist = false;
            Point otherPoint;
            if(
                ePtr->vertexes[0] == preMovePivotVertex &&
                !vertexSelection.contains(ePtr->vertexIdxs[1])
            ) {
                otherPoint.x = ePtr->vertexes[1]->x;
                otherPoint.y = ePtr->vertexes[1]->y;
                drawDist = true;
            } else if(
                ePtr->vertexes[1] == preMovePivotVertex &&
                !vertexSelection.contains(ePtr->vertexIdxs[0])
            ) {
                otherPoint.x = ePtr->vertexes[0]->x;
                otherPoint.y = ePtr->vertexes[0]->y;
                drawDist = true;
            }
            
            if(drawDist) {
                drawLineDist(v2p(preMovePivotVertex), otherPoint);
            }
        }
        
        //Draw the debug triangulation.
        if(debugTriangulation && sectorSelection.hasAny()) {
            Sector* sPtr =
                game.curArea->sectors[sectorSelection.getFirstItemIdx()];
            forIdx(t, sPtr->triangles) {
                Triangle* tPtr = &sPtr->triangles[t];
                al_draw_triangle(
                    tPtr->points[0]->x,
                    tPtr->points[0]->y,
                    tPtr->points[1]->x,
                    tPtr->points[1]->y,
                    tPtr->points[2]->x,
                    tPtr->points[2]->y,
                    DEBUG_TRIANGLE_COLOR,
                    2.0f / game.editorsView.cam.zoom
                );
            }
        }
        
        //Draw the debug sector indexes.
        if(debugSectorIdxs) {
            Point middle(
                (ePtr->vertexes[0]->x + ePtr->vertexes[1]->x) / 2.0f,
                (ePtr->vertexes[0]->y + ePtr->vertexes[1]->y) / 2.0f
            );
            float edgeAngle =
                getAngle(v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]));
            float distFromEdge = 16.0f / game.editorsView.cam.zoom;
            
            Point pos1(
                middle.x + cos(edgeAngle - TAU / 4) * distFromEdge,
                middle.y + sin(edgeAngle - TAU / 4) * distFromEdge
            );
            Point pos2(
                middle.x + cos(edgeAngle + TAU / 4) * distFromEdge,
                middle.y + sin(edgeAngle + TAU / 4) * distFromEdge
            );
            string text1 =
                "A" +
                (
                    ePtr->sectorIdxs[0] == INVALID ?
                    "-" :
                    i2s(ePtr->sectorIdxs[0])
                );
            string text2 =
                "B" +
                (
                    ePtr->sectorIdxs[1] == INVALID ?
                    "-" :
                    i2s(ePtr->sectorIdxs[1])
                );
                
            al_draw_line(
                middle.x, middle.y, pos1.x, pos1.y, DEBUG_SECTOR_A_COLOR,
                2.0f / game.editorsView.cam.zoom
            );
            drawDebugText(DEBUG_SECTOR_A_COLOR, pos1, text1);
            
            al_draw_line(
                middle.x, middle.y, pos2.x, pos2.y, DEBUG_SECTOR_B_COLOR,
                2.0f / game.editorsView.cam.zoom
            );
            drawDebugText(DEBUG_SECTOR_B_COLOR, pos2, text2);
        }
        
        //Draw the debug edge index.
        if(debugEdgeIdxs) {
            Point middle(
                (ePtr->vertexes[0]->x + ePtr->vertexes[1]->x) / 2.0f,
                (ePtr->vertexes[0]->y + ePtr->vertexes[1]->y) / 2.0f
            );
            drawDebugText(DEBUG_EDGE_IDX_COLOR, middle, i2s(e));
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
    const Point& focus, const Point& other, const string& prefix
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
    const ALLEGRO_COLOR LINK_COLOR = al_map_rgb(160, 224, 64);
    const ALLEGRO_COLOR STORE_COLOR = al_map_rgb(224, 200, 200);
    const ALLEGRO_COLOR DEF_TYPE_COLOR = al_map_rgb(255, 0, 0);
    const ALLEGRO_COLOR ANGLE_ARROW_COLOR = COLOR_BLACK;
    const ALLEGRO_COLOR TERRITORY_RADIUS_COLOR = al_map_rgb(240, 240, 192);
    const ALLEGRO_COLOR TERRAIN_RADIUS_COLOR = al_map_rgb(240, 192, 192);
    
    //Links and stores.
    if(state == EDITOR_STATE_MOBS && style.mobAlpha > 0.0f) {
        forIdx(m, game.curArea->mobGenerators) {
            MobGen* mPtr = game.curArea->mobGenerators[m];
            MobGen* m2Ptr = nullptr;
            
            if(!mPtr->type) continue;
            
            bool isSelected = mobSelection.contains(m);
            
            forIdx(l, mPtr->links) {
                m2Ptr = mPtr->links[l];
                if(!m2Ptr->type) continue;
                
                bool isM2Selected = mobSelection.contains(mPtr->linkIdxs[l]);
                bool showLink = isSelected || isM2Selected;
                
                if(showLink) {
                    drawArrow(
                        mPtr->center, m2Ptr->center,
                        mPtr->type->radius, m2Ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        LINK_COLOR
                    );
                }
            }
            
            if(mPtr->storedInside != INVALID) {
                m2Ptr =
                    game.curArea->mobGenerators[mPtr->storedInside];
                if(!m2Ptr->type) continue;
                
                bool isM2Selected = mobSelection.contains(mPtr->storedInside);
                bool showStore = isSelected || isM2Selected;
                
                if(showStore) {
                    drawArrow(
                        mPtr->center, m2Ptr->center,
                        mPtr->type->radius, m2Ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        STORE_COLOR
                    );
                }
            }
        }
    }
    
    //The generators themselves.
    forIdx(m, game.curArea->mobGenerators) {
        //Setup.
        MobGen* mPtr = game.curArea->mobGenerators[m];
        float radius = getMobGenRadius(mPtr);
        bool isSelected = mobSelection.contains(m);
        bool isInMobGroup =
            subState == EDITOR_SUB_STATE_MISSION_MOBS &&
            isInContainer(
                game.curArea->mission.mobGroups[curMobGroupIdx].mobIdxs, m
            );
        bool isHighlighted =
            highlightedMob == mPtr && state == EDITOR_STATE_MOBS;
            
        //Pick the color.
        ALLEGRO_COLOR simpleColor = DEF_TYPE_COLOR;
        if(mPtr->type && mPtr != problemMobPtr) {
            simpleColor = mPtr->type->category->editorColor;
        }
        ALLEGRO_COLOR color = simpleColor;
        if(isSelected || isInMobGroup) {
            color = getSelectionEffectReplacementColor(color);
        } else if(isHighlighted) {
            color = getHighlightEffectReplacementColor(color);
        }
        ALLEGRO_COLOR arrowColor = ANGLE_ARROW_COLOR;
        ALLEGRO_COLOR territoryColor = TERRITORY_RADIUS_COLOR;
        ALLEGRO_COLOR terrainColor = TERRAIN_RADIUS_COLOR;
        
        simpleColor = multAlpha(simpleColor, style.mobAlpha);
        color = multAlpha(color, style.mobAlpha);
        arrowColor = multAlpha(arrowColor, style.mobAlpha);
        territoryColor = multAlpha(territoryColor, style.mobAlpha);
        terrainColor = multAlpha(terrainColor, style.mobAlpha);
        
        //Draw children of this mob.
        if(mPtr->type) {
            forIdx(c, mPtr->type->children) {
                MobType::Child* childInfo =
                    &mPtr->type->children[c];
                MobType::SpawnInfo* spawnInfo =
                    getSpawnInfoFromChildInfo(mPtr->type, childInfo);
                if(!spawnInfo) continue;
                
                Point cPos =
                    mPtr->center +
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
                        cRot, simpleColor,
                        1.0f / game.editorsView.cam.zoom
                    );
                } else {
                    al_draw_circle(
                        cPos.x, cPos.y, cType->radius,
                        simpleColor, 1.0f / game.editorsView.cam.zoom
                    );
                }
                
            }
        }
        
        //Draw the mob.
        if(mPtr->type && mPtr->type->rectangularDim.x != 0) {
            drawRotatedRectangle(
                mPtr->center, mPtr->type->rectangularDim,
                mPtr->angle, color,
                1.0f / game.editorsView.cam.zoom
            );
        }
        
        al_draw_filled_circle(
            mPtr->center.x, mPtr->center.y,
            radius, color
        );
        
        float lrw = cos(mPtr->angle) * radius;
        float lrh = sin(mPtr->angle) * radius;
        float lt = radius / 8.0;
        
        al_draw_line(
            mPtr->center.x - lrw * 0.8, mPtr->center.y - lrh * 0.8,
            mPtr->center.x + lrw * 0.8, mPtr->center.y + lrh * 0.8,
            arrowColor, lt
        );
        
        float tx1 = mPtr->center.x + lrw;
        float ty1 = mPtr->center.y + lrh;
        float tx2 = tx1 + cos(mPtr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty2 = ty1 + sin(mPtr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float tx3 = tx1 + cos(mPtr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty3 = ty1 + sin(mPtr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
        
        al_draw_filled_triangle(
            tx1, ty1, tx2, ty2, tx3, ty3, arrowColor
        );
        
        //Draw territory and such.
        if(isSelected) {
            if(
                game.options.areaEd.showTerritory &&
                mPtr->type &&
                mPtr->type->territoryRadius > 0.0f
            ) {
                al_draw_circle(
                    mPtr->center.x, mPtr->center.y, mPtr->type->territoryRadius,
                    territoryColor, 1.0f / game.editorsView.cam.zoom
                );
            }
            if(
                game.options.areaEd.showTerritory &&
                mPtr->type &&
                mPtr->type->terrainRadius > 0.0f
            ) {
                al_draw_circle(
                    mPtr->center.x, mPtr->center.y, mPtr->type->terrainRadius,
                    terrainColor, 1.0f / game.editorsView.cam.zoom
                );
            }
        }
        
    }
    
    drawSelectionAndTransformationThings(
        mobsSelCtrl, curTransformationWidget, !game.options.areaEd.selTrans
    );
}


/**
 * @brief Draws the path stops and links onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawPaths(const AreaEdCanvasStyle& style) {
    const ALLEGRO_COLOR SCRIPT_STOP_COLOR = al_map_rgba(187, 102, 34, 224);
    const ALLEGRO_COLOR LIGHT_STOP_COLOR = al_map_rgba(102, 170, 34, 224);
    const ALLEGRO_COLOR AIRBORNE_STOP_COLOR = al_map_rgba(187, 102, 153, 224);
    const ALLEGRO_COLOR NORMAL_STOP_COLOR = al_map_rgb(88, 177, 177);
    const ALLEGRO_COLOR DEBUG_STOP_COLOR = al_map_rgb(80, 192, 192);
    const ALLEGRO_COLOR NORMAL_LINK_COLOR = al_map_rgba(34, 136, 187, 224);
    const ALLEGRO_COLOR LEDGE_LINK_COLOR = al_map_rgba(180, 96, 32, 224);
    const ALLEGRO_COLOR DEBUG_LINK_COLOR = al_map_rgb(96, 104, 224);
    const ALLEGRO_COLOR CLOSEST_LINE_COLOR = al_map_rgb(192, 128, 32);
    const ALLEGRO_COLOR PREVIEW_OK_COLOR = al_map_rgb(255, 187, 136);
    const ALLEGRO_COLOR PREVIEW_INVALID_COLOR = al_map_rgb(221, 17, 17);
    const ALLEGRO_COLOR PREVIEW_CP_BG_COLOR = al_map_rgb(240, 224, 160);
    const ALLEGRO_COLOR PREVIEW_CP_TEXT_COLOR = al_map_rgb(0, 64, 64);
    
    if(state == EDITOR_STATE_PATHS) {
        //Stops.
        forIdx(s, game.curArea->pathStops) {
            //Setup.
            PathStop* sPtr = game.curArea->pathStops[s];
            bool isHighlighted = highlightedPathStop == sPtr;
            bool isSelected = pathStopSelection.contains(s);
            
            //Pick the color.
            ALLEGRO_COLOR color;
            if(hasFlag(sPtr->flags, PATH_STOP_FLAG_SCRIPT_ONLY)) {
                color = SCRIPT_STOP_COLOR;
            } else if(hasFlag(sPtr->flags, PATH_STOP_FLAG_LIGHT_LOAD_ONLY)) {
                color = LIGHT_STOP_COLOR;
            } else if(hasFlag(sPtr->flags, PATH_STOP_FLAG_AIRBORNE_ONLY)) {
                color = AIRBORNE_STOP_COLOR;
            } else {
                color = NORMAL_STOP_COLOR;
            }
            if(isSelected) {
                color = getSelectionEffectReplacementColor(color);
            } else if(isHighlighted) {
                color = getHighlightEffectReplacementColor(color);
            }
            
            //Draw the stop.
            al_draw_filled_circle(
                sPtr->center.x, sPtr->center.y,
                sPtr->radius,
                color
            );
            
            //Draw the debug path stop index.
            if(debugPathIdxs) {
                drawDebugText(
                    DEBUG_STOP_COLOR, sPtr->center, i2s(s)
                );
            }
        }
        
        //Links.
        forIdx(l, game.curArea->editorPathLinks) {
            //Setup.
            EditorPathLink* elPtr = &game.curArea->editorPathLinks[l];
            PathStop* s1Ptr = elPtr->link1->startPtr;
            PathStop* s2Ptr = elPtr->link1->endPtr;
            size_t s1Idx =
                game.curArea->findPathStopIdx(elPtr->link1->startPtr);
            size_t s2Idx = elPtr->link1->endIdx;
            float angle =
                getAngle(s1Ptr->center, s2Ptr->center);
            Point offset1 =
                angleToCoordinates(angle, s1Ptr->radius);
            Point offset2 =
                angleToCoordinates(angle, s2Ptr->radius);
            bool isOneWay = elPtr->link2 == nullptr;
            bool isSelected =
                pathLinkSelection.contains(
                    game.curArea->findEditorPathLinkIdx(elPtr)
                );
            bool isHighlighted = highlightedEditorPathLink == elPtr;
            
            //Pick the color.
            ALLEGRO_COLOR color;
            switch(elPtr->link1->type) {
            case PATH_LINK_TYPE_NORMAL: {
                color = NORMAL_LINK_COLOR;
                break;
            } case PATH_LINK_TYPE_LEDGE: {
                color = LEDGE_LINK_COLOR;
                break;
            }
            }
            if(!isOneWay) {
                color = changeColorLighting(color, 0.33f);
            }
            if(isSelected) {
                color = getSelectionEffectReplacementColor(color);
            } else if(isHighlighted) {
                color = getHighlightEffectReplacementColor(color);
            }
            
            //Draw the link.
            al_draw_line(
                s1Ptr->center.x + offset1.x,
                s1Ptr->center.y + offset1.y,
                s2Ptr->center.x - offset2.x,
                s2Ptr->center.y - offset2.y,
                color,
                AREA_EDITOR::PATH_LINK_THICKNESS / game.editorsView.cam.zoom
            );
            
            //Draw a triangle down the middle for one-ways.
            if(isOneWay) {
                float midX =
                    (s1Ptr->center.x + s2Ptr->center.x) / 2.0f;
                float midY =
                    (s1Ptr->center.y + s2Ptr->center.y) / 2.0f;
                const float delta =
                    (AREA_EDITOR::PATH_LINK_THICKNESS * 4) /
                    game.editorsView.cam.zoom;
                    
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
            
            //Draw the link length.
            if(
                state == EDITOR_STATE_PATHS &&
                moving &&
                game.options.areaEd.showPathLinkLength
            ) {
                bool drawDist = false;
                Point otherPoint;
                if(s1Ptr == preMovePivotStop) {
                    otherPoint.x = s2Ptr->center.x;
                    otherPoint.y = s2Ptr->center.y;
                    drawDist = true;
                } else if(s2Ptr == preMovePivotStop) {
                    otherPoint.x = s1Ptr->center.x;
                    otherPoint.y = s1Ptr->center.y;
                    drawDist = true;
                }
                
                if(drawDist) {
                    drawLineDist(preMovePivotStop->center, otherPoint);
                }
            }
            
            //Draw the debug link index.
            if(debugPathIdxs && (isOneWay || s1Idx < s2Idx)) {
                Point middle = (s1Ptr->center + s2Ptr->center) / 2.0f;
                drawDebugText(
                    DEBUG_LINK_COLOR,
                    Point(
                        middle.x + cos(angle + TAU / 4) * 4,
                        middle.y + sin(angle + TAU / 4) * 4
                    ),
                    f2s(game.curArea->editorPathLinks[l].link1->distance)
                );
            }
        }
        
        //Closest stop line.
        if(showClosestStop) {
            PathStop* closest = nullptr;
            float closestDist = FLT_MAX;
            forIdx(s, game.curArea->pathStops) {
                PathStop* sPtr = game.curArea->pathStops[s];
                float d =
                    Distance(
                        game.editorsView.mouseCursorWorldPos, sPtr->center
                    ).toFloat() -
                    sPtr->radius;
                    
                if(!closest || d < closestDist) {
                    closest = sPtr;
                    closestDist = d;
                }
            }
            
            if(closest) {
                al_draw_line(
                    game.editorsView.mouseCursorWorldPos.x,
                    game.editorsView.mouseCursorWorldPos.y,
                    closest->center.x, closest->center.y,
                    CLOSEST_LINE_COLOR, 2.0 / game.editorsView.cam.zoom
                );
            }
        }
        
        //Path preview.
        if(showPathPreview) {
            //Draw the lines of the path.
            float linesThickness = 4.0f / game.editorsView.cam.zoom;
            
            if(!pathPreview.empty()) {
                al_draw_line(
                    pathPreviewCheckpoints[0].x,
                    pathPreviewCheckpoints[0].y,
                    pathPreview[0]->center.x,
                    pathPreview[0]->center.y,
                    PREVIEW_OK_COLOR, linesThickness
                );
                for(size_t s = 0; s < pathPreview.size() - 1; s++) {
                    al_draw_line(
                        pathPreview[s]->center.x,
                        pathPreview[s]->center.y,
                        pathPreview[s + 1]->center.x,
                        pathPreview[s + 1]->center.y,
                        PREVIEW_OK_COLOR, linesThickness
                    );
                }
                al_draw_line(
                    pathPreview.back()->center.x,
                    pathPreview.back()->center.y,
                    pathPreviewCheckpoints[1].x,
                    pathPreviewCheckpoints[1].y,
                    PREVIEW_OK_COLOR, linesThickness
                );
            } else if(
                pathPreviewResult == PATH_RESULT_DIRECT ||
                pathPreviewResult == PATH_RESULT_DIRECT_NO_STOPS ||
                pathPreviewResult == PATH_RESULT_DIRECT_NO_ACCESSIBLE_STOPS
            ) {
                al_draw_line(
                    pathPreviewCheckpoints[0].x,
                    pathPreviewCheckpoints[0].y,
                    pathPreviewCheckpoints[1].x,
                    pathPreviewCheckpoints[1].y,
                    PREVIEW_OK_COLOR, linesThickness
                );
            } else {
                for(size_t c = 0; c < 2; c++) {
                    if(pathPreviewClosest[c]) {
                        al_draw_line(
                            pathPreviewClosest[c]->center.x,
                            pathPreviewClosest[c]->center.y,
                            pathPreviewCheckpoints[c].x,
                            pathPreviewCheckpoints[c].y,
                            PREVIEW_INVALID_COLOR, linesThickness
                        );
                    }
                }
            }
            
            //Draw the checkpoints.
            for(unsigned char c = 0; c < 2; c++) {
                string letter = (c == 0 ? "A" : "B");
                
                const float factor =
                    AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS /
                    game.editorsView.cam.zoom;
                al_draw_filled_rectangle(
                    pathPreviewCheckpoints[c].x - factor,
                    pathPreviewCheckpoints[c].y - factor,
                    pathPreviewCheckpoints[c].x + factor,
                    pathPreviewCheckpoints[c].y + factor,
                    PREVIEW_CP_BG_COLOR
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
                    PREVIEW_CP_TEXT_COLOR
                );
            }
        }
        
        drawSelectionAndTransformationThings(
            pathsSelCtrl, curTransformationWidget, !game.options.areaEd.selTrans
        );
        
    }
}


/**
 * @brief Draws the regions onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawRegions(const AreaEdCanvasStyle& style) {
    const ALLEGRO_COLOR REGION_OUTLINE_COLOR = al_map_rgb(192, 128, 64);
    
    if(state == EDITOR_STATE_DETAILS) {
        forIdx(r, game.curArea->regions) {
            AreaRegion* rPtr = game.curArea->regions[r];
            
            drawRotatedRectangle(
                rPtr->pose.pos, rPtr->pose.size, rPtr->pose.angle,
                regionSelection.contains(r) ?
                getSelectionEffectReplacementColor(REGION_OUTLINE_COLOR) :
                REGION_OUTLINE_COLOR,
                4.0 / game.editorsView.cam.zoom
            );
        }
    }
}


/**
 * @brief Draws the reminders onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawReminders(const AreaEdCanvasStyle& style) {
    const ALLEGRO_COLOR REMINDER_BG_COLOR = al_map_rgb(224, 180, 64);
    
    if(state == EDITOR_STATE_REVIEW) {
        forIdx(r, game.curArea->reminders) {
            //Setup.
            AreaMakerReminder* rPtr = &game.curArea->reminders[r];
            bool isSelected = reminderSelection.contains(r);
            
            //Pick the color.
            ALLEGRO_COLOR color = REMINDER_BG_COLOR;
            if(isSelected) {
                color = getSelectionEffectReplacementColor(color);
            }
            
            //Draw the reminder.
            drawFilledRoundedRectangle(
                rPtr->center, Point(AREA_EDITOR::REMINDER_SIZE), 8.0f, color
            );
            drawText(
                "!", game.sysContent.fntAreaName, rPtr->center,
                Point(AREA_EDITOR::REMINDER_SIZE)
            );
        }
    }
    
    drawSelectionAndTransformationThings(
        reviewSelCtrl, curTransformationWidget, !game.options.areaEd.selTrans
    );
}


/**
 * @brief Draws the sectors onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawSectors(const AreaEdCanvasStyle& style) {
    const ALLEGRO_COLOR INVALID_COLOR = al_map_rgba(160, 16, 16, 224);
    const ALLEGRO_COLOR BRIGHTNESS_MAP_COLOR = al_map_rgb(180, 200, 180);
    const ALLEGRO_COLOR HEIGHT_MAP_COLOR = al_map_rgb(224, 255, 224);
    
    //Edge offset effect updates.
    if(style.wallShadowAlpha > 0.0f) {
        updateOffsetEffectBuffer(
            game.editorsView.worldCorners,
            game.liquidLimitEffectCaches,
            game.liquidLimitEffectBuffer,
            true, game.editorsView
        );
        updateOffsetEffectBuffer(
            game.editorsView.worldCorners,
            game.wallSmoothingEffectCaches,
            game.wallOffsetEffectBuffer,
            true, game.editorsView
        );
        updateOffsetEffectBuffer(
            game.editorsView.worldCorners,
            game.wallShadowEffectCaches,
            game.wallOffsetEffectBuffer,
            false, game.editorsView
        );
    }
    
    //Draw each one.
    size_t nSectors = game.curArea->sectors.size();
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
            sPtr = game.curArea->sectors[s];
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
                    LiquidType* lPtr = sPtr->hazard->associatedLiquid;
                    if(lPtr) {
                        drawLiquid(
                            sPtr, lPtr, Point(), 1.0f, game.timePassed, 1.0f
                        );
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
        
        //Selection effect setup.
        bool isSelected = sectorSelection.contains(s);
        bool isValid = true;
        bool isHighlighted =
            sPtr == highlightedSector &&
            selectionFilter == SELECTION_FILTER_SECTORS &&
            state == EDITOR_STATE_LAYOUT;
            
        if(isInMap(game.curArea->problems.nonSimples, sPtr)) {
            isValid = false;
        }
        if(sPtr == problemSectorPtr) {
            isValid = false;
        }
        
        //Pick the color.
        ALLEGRO_COLOR color;
        if(!isValid) {
            color = INVALID_COLOR;
        } else if(showBlockingSectors) {
            color =
                sPtr->type == SECTOR_TYPE_BLOCKING ?
                AREA_EDITOR::BLOCKING_SECTOR_COLOR :
                AREA_EDITOR::NON_BLOCKING_COLOR;
        } else if(viewBrightness) {
            color =
                tintColor(
                    BRIGHTNESS_MAP_COLOR, mapGray(sPtr->brightness)
                );
        } else if(viewHeightmap) {
            float h =
                interpolateNumber(
                    sPtr->z,
                    style.lowestSectorZ, style.highestSectorZ,
                    0, 1.0f
                );
            color =
                tintColor(HEIGHT_MAP_COLOR, mapGray(h * 255));
        } else if(isSelected) {
            color = getSelectionEffectOverlayColor();
        } else if(isHighlighted) {
            color = getHighlightEffectOverlayColor();
        } else {
            color = COLOR_EMPTY;
        }
        
        //Draw the colored overlay.
        if(color.a > 0.0f) {
            forIdx(t, sPtr->triangles) {
                ALLEGRO_VERTEX av[3];
                for(size_t v = 0; v < 3; v++) {
                    av[v].u = 0;
                    av[v].v = 0;
                    av[v].x = sPtr->triangles[t].points[v]->x;
                    av[v].y = sPtr->triangles[t].points[v]->y;
                    av[v].z = 0;
                    av[v].color = color;
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
    const ALLEGRO_COLOR SHADOW_OUTLINE_COLOR = al_map_rgb(128, 192, 128);
    
    if(
        state == EDITOR_STATE_DETAILS ||
        (previewMode && showShadows)
    ) {
        forIdx(s, game.curArea->treeShadows) {
        
            TreeShadow* sPtr = game.curArea->treeShadows[s];
            if(!previewMode && shadowSelection.contains(s)) {
                //Draw a white rectangle to contrast the shadow better.
                ALLEGRO_TRANSFORM tra, current;
                al_identity_transform(&tra);
                al_rotate_transform(&tra, sPtr->pose.angle);
                al_translate_transform(
                    &tra, sPtr->pose.pos.x, sPtr->pose.pos.y
                );
                al_copy_transform(&current, al_get_current_transform());
                al_compose_transform(&tra, &current);
                al_use_transform(&tra);
                
                al_draw_filled_rectangle(
                    -sPtr->pose.size.x / 2.0,
                    -sPtr->pose.size.y / 2.0,
                    sPtr->pose.size.x / 2.0,
                    sPtr->pose.size.y / 2.0,
                    multAlpha(sPtr->tint, 0.33f)
                );
                
                al_use_transform(&current);
            }
            
            drawBitmap(
                sPtr->bitmap, sPtr->pose.pos, sPtr->pose.size,
                sPtr->pose.angle, sPtr->tint
            );
            
            if(state == EDITOR_STATE_DETAILS) {
                drawRotatedRectangle(
                    sPtr->pose.pos, sPtr->pose.size, sPtr->pose.angle,
                    shadowSelection.contains(s) ?
                    getSelectionEffectReplacementColor(SHADOW_OUTLINE_COLOR) :
                    SHADOW_OUTLINE_COLOR,
                    4.0 / game.editorsView.cam.zoom
                );
            }
        }
    }
}


/**
 * @brief Draws the vertex points onto the canvas.
 *
 * @param style Canvas style.
 */
void AreaEditor::drawVertexes(const AreaEdCanvasStyle& style) {
    const ALLEGRO_COLOR INVALID_COLOR = al_map_rgb(192, 32, 32);
    const ALLEGRO_COLOR NORMAL_COLOR = al_map_rgb(80, 160, 255);
    const ALLEGRO_COLOR DEBUG_COLOR = al_map_rgb(192, 192, 255);
    
    if(state == EDITOR_STATE_LAYOUT) {
        size_t nVertexes = game.curArea->vertexes.size();
        for(size_t v = 0; v < nVertexes; v++) {
            //Setup.
            Vertex* vPtr = game.curArea->vertexes[v];
            bool isSelected = vertexSelection.contains(v);
            bool isValid = vPtr != problemVertexPtr;
            bool isHighlighted =
                highlightedVertex == vPtr &&
                (
                    selectionFilter == SELECTION_FILTER_SECTORS ||
                    selectionFilter == SELECTION_FILTER_EDGES ||
                    selectionFilter == SELECTION_FILTER_VERTEXES
                );
                
            //Pick the color.
            ALLEGRO_COLOR color;
            if(!isValid) {
                color = INVALID_COLOR;
            } else {
                color = NORMAL_COLOR;
            }
            if(isSelected) {
                color = getSelectionEffectReplacementColor(color);
            } else if(isHighlighted) {
                color = getHighlightEffectReplacementColor(color);
            }
            color = multAlpha(color, style.edgeAlpha);
            
            //Draw the vertex.
            drawFilledDiamond(
                v2p(vPtr), 3.0 / game.editorsView.cam.zoom, color
            );
            
            //Draw the debug vertex index.
            if(debugVertexIdxs) {
                drawDebugText(DEBUG_COLOR, v2p(vPtr), i2s(v));
            }
        }
    }
}
