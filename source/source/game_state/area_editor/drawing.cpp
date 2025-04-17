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
 * @param start_offset When considering where to place the triangle
 * in the line, pretend that the starting point is actually this distance
 * away from start. Useful for when mobs of different radii are involved.
 * @param end_offset Same as start_offset, but for the end point.
 * @param thickness Thickness of the arrow's line.
 * @param color Arrow color.
 */
void AreaEditor::drawArrow(
    const Point &start, const Point &end,
    float start_offset, float end_offset,
    float thickness, const ALLEGRO_COLOR &color
) {
    al_draw_line(
        start.x, start.y, end.x, end.y,
        color, thickness / game.cam.zoom
    );
    
    if(game.cam.zoom >= 0.25) {
        float angle =
            getAngle(start, end);
        Point final_start = Point(start_offset, 0);
        final_start = rotatePoint(final_start, angle);
        final_start += start;
        Point final_end = Point(end_offset, 0);
        final_end = rotatePoint(final_end, angle + TAU / 2.0);
        final_end += end;
        
        Point pivot(
            final_start.x + (final_end.x - final_start.x) * 0.55,
            final_start.y + (final_end.y - final_start.y) * 0.55
        );
        const float delta =
            (thickness * 4) / game.cam.zoom;
            
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
    al_use_transform(&game.worldToScreenTransform);
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y,
        canvasBR.x - canvasTL.x, canvasBR.y - canvasTL.y
    );
    
    al_clear_to_color(COLOR_BLACK);
    
    if(!game.curAreaData) {
        al_reset_clipping_rectangle();
        al_use_transform(&game.identityTransform);
        return;
    }
    
    float lowest_sector_z = 0.0f;
    float highest_sector_z = 0.0f;
    if(
        game.options.areaEd.viewMode == VIEW_MODE_HEIGHTMAP &&
        !game.curAreaData->sectors.empty()
    ) {
        lowest_sector_z = game.curAreaData->sectors[0]->z;
        highest_sector_z = lowest_sector_z;
        
        for(size_t s = 1; s < game.curAreaData->sectors.size(); s++) {
            lowest_sector_z =
                std::min(lowest_sector_z, game.curAreaData->sectors[s]->z);
            highest_sector_z =
                std::max(highest_sector_z, game.curAreaData->sectors[s]->z);
        }
    }
    
    float selection_min_opacity = 0.25f;
    float selection_max_opacity = 0.75f;
    float textures_opacity = 0.4f;
    float wall_shadows_opacity = 0.0f;
    float edges_opacity = 0.25f;
    float grid_opacity = 1.0f;
    float mob_opacity = 0.15f;
    ALLEGRO_COLOR highlight_color = COLOR_WHITE;
    if(game.options.editors.useCustomStyle) {
        highlight_color = game.options.editors.highlightColor;
    }
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        textures_opacity = 0.5f;
        edges_opacity = 1.0f;
        break;
        
    } case EDITOR_STATE_MOBS: {
        mob_opacity = 1.0f;
        break;
        
    } case EDITOR_STATE_MAIN:
    case EDITOR_STATE_REVIEW: {
        textures_opacity = 0.6f;
        edges_opacity = 0.5f;
        grid_opacity = 0.3f;
        mob_opacity = 0.75f;
        break;
        
    }
    }
    
    if(previewMode) {
        textures_opacity = 1.0f;
        wall_shadows_opacity = 1.0f;
        edges_opacity = 0.0f;
        grid_opacity = 0.0f;
        mob_opacity = 0.0f;
    } else if(subState == EDITOR_SUB_STATE_OCTEE) {
        quickPreviewTimer.start();
    }
    
    if(quickPreviewTimer.timeLeft > 0) {
        float t =
            std::min(
                quickPreviewTimer.timeLeft,
                quickPreviewTimer.duration / 2.0f
            );
        selection_min_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                selection_min_opacity, 0.0f
            );
        selection_max_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                selection_max_opacity, 0.0f
            );
        textures_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                textures_opacity, 1.0f
            );
        wall_shadows_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                wall_shadows_opacity, 1.0f
            );
        edges_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                edges_opacity, 0.0f
            );
        grid_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                grid_opacity, 0.0f
            );
        mob_opacity =
            interpolateNumber(
                t, 0.0f, quickPreviewTimer.duration / 2.0f,
                mob_opacity, 0.0f
            );
    }
    
    float selection_opacity =
        selection_min_opacity +
        (sin(selectionEffect) + 1) *
        (selection_max_opacity - selection_min_opacity) / 2.0;
        
    //Sectors.
    if(wall_shadows_opacity > 0.0f) {
        updateOffsetEffectBuffer(
            game.cam.box[0], game.cam.box[1],
            game.liquidLimitEffectCaches,
            game.liquidLimitEffectBuffer,
            true
        );
        updateOffsetEffectBuffer(
            game.cam.box[0], game.cam.box[1],
            game.wallSmoothingEffectCaches,
            game.wallOffsetEffectBuffer,
            true
        );
        updateOffsetEffectBuffer(
            game.cam.box[0], game.cam.box[1],
            game.wallShadowEffectCaches,
            game.wallOffsetEffectBuffer,
            false
        );
    }
    size_t n_sectors = game.curAreaData->sectors.size();
    for(size_t s = 0; s < n_sectors; s++) {
        Sector* s_ptr;
        if(
            preMoveAreaData &&
            moving &&
            (
                state == EDITOR_STATE_LAYOUT
            )
        ) {
            s_ptr = preMoveAreaData->sectors[s];
        } else {
            s_ptr = game.curAreaData->sectors[s];
        }
        
        bool view_heightmap = false;
        bool view_brightness = false;
        
        if(
            game.options.areaEd.viewMode == VIEW_MODE_TEXTURES ||
            previewMode
        ) {
            if(previewMode) {
                bool has_liquid = false;
                if(s_ptr->hazard) {
                    Liquid* l_ptr = s_ptr->hazard->associatedLiquid;
                    if(l_ptr) {
                        drawLiquid(s_ptr, l_ptr, Point(), 1.0f, game.timePassed);
                        has_liquid = true;
                    }
                }
                if(!has_liquid) {
                    drawSectorTexture(s_ptr, Point(), 1.0, textures_opacity);
                }
            } else {
                drawSectorTexture(s_ptr, Point(), 1.0, textures_opacity);
            }
            
            if(wall_shadows_opacity > 0.0f) {
                drawSectorEdgeOffsets(
                    s_ptr, game.liquidLimitEffectBuffer, 1.0f
                );
                drawSectorEdgeOffsets(
                    s_ptr, game.wallOffsetEffectBuffer, wall_shadows_opacity
                );
            }
            
        } else if(game.options.areaEd.viewMode == VIEW_MODE_HEIGHTMAP) {
            view_heightmap = true;
            
        } else if(game.options.areaEd.viewMode == VIEW_MODE_BRIGHTNESS) {
            view_brightness = true;
            
        }
        
        bool selected =
            selectedSectors.find(s_ptr) != selectedSectors.end();
        bool valid = true;
        bool highlighted =
            s_ptr == highlightedSector &&
            selectionFilter == SELECTION_FILTER_SECTORS &&
            state == EDITOR_STATE_LAYOUT;
            
        if(
            game.curAreaData->problems.nonSimples.find(s_ptr) !=
            game.curAreaData->problems.nonSimples.end()
        ) {
            valid = false;
        }
        if(s_ptr == problemSectorPtr) {
            valid = false;
        }
        
        if(
            selected || !valid || view_heightmap ||
            view_brightness || showBlockingSectors || highlighted
        ) {
            for(size_t t = 0; t < s_ptr->triangles.size(); t++) {
            
                ALLEGRO_VERTEX av[3];
                for(size_t v = 0; v < 3; v++) {
                    if(!valid) {
                        av[v].color = al_map_rgba(160, 16, 16, 224);
                    } else if(showBlockingSectors) {
                        av[v].color =
                            s_ptr->type == SECTOR_TYPE_BLOCKING ?
                            AREA_EDITOR::BLOCKING_COLOR :
                            AREA_EDITOR::NON_BLOCKING_COLOR;
                    } else if(view_brightness) {
                        av[v].color =
                            al_map_rgba(
                                s_ptr->brightness * 0.7,
                                s_ptr->brightness * 0.8,
                                s_ptr->brightness * 0.7,
                                255
                            );
                    } else if(view_heightmap) {
                        unsigned char g =
                            interpolateNumber(
                                s_ptr->z, lowest_sector_z, highest_sector_z,
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
                                selection_opacity * 0.5 * 255
                            );
                        if(highlighted && !selected) {
                            av[v].color =
                                al_map_rgba(
                                    highlight_color.r * 255,
                                    highlight_color.g * 255,
                                    highlight_color.b * 255,
                                    16
                                );
                        }
                    }
                    av[v].u = 0;
                    av[v].v = 0;
                    av[v].x = s_ptr->triangles[t].points[v]->x;
                    av[v].y = s_ptr->triangles[t].points[v]->y;
                    av[v].z = 0;
                }
                
                al_draw_prim(
                    av, nullptr, nullptr,
                    0, 3, ALLEGRO_PRIM_TRIANGLE_LIST
                );
                
            }
        }
    }
    
    //Grid.
    drawGrid(
        game.options.areaEd.gridInterval,
        al_map_rgba(64, 64, 64, grid_opacity * 255),
        al_map_rgba(48, 48, 48, grid_opacity * 255)
    );
    
    //0,0 marker.
    al_draw_line(
        -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2, 0,
        al_map_rgba(192, 192, 224, grid_opacity * 255),
        1.0f / game.cam.zoom
    );
    al_draw_line(
        0, -(AREA_EDITOR::COMFY_DIST * 2), 0,
        AREA_EDITOR::COMFY_DIST * 2,
        al_map_rgba(192, 192, 224, grid_opacity * 255),
        1.0f / game.cam.zoom
    );
    
    //Edges.
    size_t n_edges = game.curAreaData->edges.size();
    for(size_t e = 0; e < n_edges; e++) {
        Edge* e_ptr = game.curAreaData->edges[e];
        
        if(!e_ptr->isValid()) continue;
        
        bool one_sided = true;
        bool same_z = false;
        bool valid = true;
        bool selected = false;
        bool highlighted =
            e_ptr == highlightedEdge &&
            (
                selectionFilter == SELECTION_FILTER_SECTORS ||
                selectionFilter == SELECTION_FILTER_EDGES
            ) &&
            state == EDITOR_STATE_LAYOUT;
            
        if(problemSectorPtr) {
            if(
                e_ptr->sectors[0] == problemSectorPtr ||
                e_ptr->sectors[1] == problemSectorPtr
            ) {
                valid = false;
            }
            
        }
        if(
            problemEdgeIntersection.e1 == e_ptr ||
            problemEdgeIntersection.e2 == e_ptr
        ) {
            valid = false;
        }
        
        if(
            game.curAreaData->problems.loneEdges.find(e_ptr) !=
            game.curAreaData->problems.loneEdges.end()
        ) {
            valid = false;
        }
        
        if(
            game.curAreaData->problems.nonSimples.find(e_ptr->sectors[0]) !=
            game.curAreaData->problems.nonSimples.end() ||
            game.curAreaData->problems.nonSimples.find(e_ptr->sectors[1]) !=
            game.curAreaData->problems.nonSimples.end()
        ) {
            valid = false;
        }
        
        if(e_ptr->sectors[0] && e_ptr->sectors[1]) one_sided = false;
        
        if(
            !one_sided &&
            e_ptr->sectors[0]->z == e_ptr->sectors[1]->z &&
            e_ptr->sectors[0]->type == e_ptr->sectors[1]->type
        ) {
            same_z = true;
        }
        
        if(selectedEdges.find(e_ptr) != selectedEdges.end()) {
            selected = true;
        }
        
        al_draw_line(
            e_ptr->vertexes[0]->x,
            e_ptr->vertexes[0]->y,
            e_ptr->vertexes[1]->x,
            e_ptr->vertexes[1]->y,
            (
                selected ?
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    selection_opacity * 255
                ) :
                !valid ?
                al_map_rgba(192, 32,  32,  edges_opacity * 255) :
                highlighted ?
                al_map_rgba(
                    highlight_color.r * 255,
                    highlight_color.g * 255,
                    highlight_color.b * 255,
                    edges_opacity * 255
                ) :
                one_sided ?
                al_map_rgba(128, 128, 128, edges_opacity * 255) :
                same_z ?
                al_map_rgba(128, 128, 128, edges_opacity * 255) :
                al_map_rgba(150, 150, 150, edges_opacity * 255)
            ),
            (selected ? 3.0 : 2.0) / game.cam.zoom
        );
        
        if(
            state == EDITOR_STATE_LAYOUT &&
            moving &&
            game.options.areaEd.showEdgeLength
        ) {
            bool draw_dist = false;
            Point other_point;
            if(
                e_ptr->vertexes[0] == moveClosestVertex &&
                selectedVertexes.find(e_ptr->vertexes[1]) ==
                selectedVertexes.end()
            ) {
                other_point.x = e_ptr->vertexes[1]->x;
                other_point.y = e_ptr->vertexes[1]->y;
                draw_dist = true;
            } else if(
                e_ptr->vertexes[1] == moveClosestVertex &&
                selectedVertexes.find(e_ptr->vertexes[0]) ==
                selectedVertexes.end()
            ) {
                other_point.x = e_ptr->vertexes[0]->x;
                other_point.y = e_ptr->vertexes[0]->y;
                draw_dist = true;
            }
            
            if(draw_dist) {
                drawLineDist(v2p(moveClosestVertex), other_point);
            }
        }
        
        if(debugTriangulation && !selectedSectors.empty()) {
            Sector* s_ptr = *selectedSectors.begin();
            for(size_t t = 0; t < s_ptr->triangles.size(); t++) {
                Triangle* t_ptr = &s_ptr->triangles[t];
                al_draw_triangle(
                    t_ptr->points[0]->x,
                    t_ptr->points[0]->y,
                    t_ptr->points[1]->x,
                    t_ptr->points[1]->y,
                    t_ptr->points[2]->x,
                    t_ptr->points[2]->y,
                    al_map_rgb(192, 0, 160),
                    2.0f / game.cam.zoom
                );
            }
        }
        
        if(debugSectorIdxs) {
            Point middle(
                (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f,
                (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f
            );
            float angle =
                getAngle(v2p(e_ptr->vertexes[1]), v2p(e_ptr->vertexes[0]));
            drawDebugText(
                al_map_rgb(192, 255, 192),
                Point(
                    middle.x + cos(angle + TAU / 4) * 4,
                    middle.y + sin(angle + TAU / 4) * 4
                ),
                (
                    e_ptr->sectorIdxs[0] == INVALID ?
                    "-" :
                    i2s(e_ptr->sectorIdxs[0])
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
                    e_ptr->sectorIdxs[1] == INVALID ?
                    "-" :
                    i2s(e_ptr->sectorIdxs[1])
                ),
                2
            );
        }
        
        if(debugEdgeIdxs) {
            Point middle(
                (e_ptr->vertexes[0]->x + e_ptr->vertexes[1]->x) / 2.0f,
                (e_ptr->vertexes[0]->y + e_ptr->vertexes[1]->y) / 2.0f
            );
            drawDebugText(al_map_rgb(255, 192, 192), middle, i2s(e));
        }
    }
    
    //Vertexes.
    if(state == EDITOR_STATE_LAYOUT) {
        size_t n_vertexes = game.curAreaData->vertexes.size();
        for(size_t v = 0; v < n_vertexes; v++) {
            Vertex* v_ptr = game.curAreaData->vertexes[v];
            bool selected =
                (selectedVertexes.find(v_ptr) != selectedVertexes.end());
            bool valid =
                v_ptr != problemVertexPtr;
            bool highlighted =
                highlightedVertex == v_ptr &&
                (
                    selectionFilter == SELECTION_FILTER_SECTORS ||
                    selectionFilter == SELECTION_FILTER_EDGES ||
                    selectionFilter == SELECTION_FILTER_VERTEXES
                );
            drawFilledDiamond(
                v2p(v_ptr), 3.0 / game.cam.zoom,
                selected ?
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    selection_opacity * 255
                ) :
                !valid ?
                al_map_rgb(192, 32, 32) :
                highlighted ?
                al_map_rgba(
                    highlight_color.r * 255,
                    highlight_color.g * 255,
                    highlight_color.b * 255,
                    edges_opacity * 255
                ) :
                al_map_rgba(80, 160, 255, edges_opacity * 255)
            );
            
            if(debugVertexIdxs) {
                drawDebugText(
                    al_map_rgb(192, 192, 255),
                    v2p(v_ptr), i2s(v)
                );
            }
        }
    }
    
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
            1.0f / game.cam.zoom
        );
    }
    
    //Mobs.
    if(state == EDITOR_STATE_MOBS && mob_opacity > 0.0f) {
        for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
            MobGen* m_ptr = game.curAreaData->mobGenerators[m];
            MobGen* m2_ptr = nullptr;
            
            if(!m_ptr->type) continue;
            
            bool is_selected =
                selectedMobs.find(m_ptr) != selectedMobs.end();
                
            for(size_t l = 0; l < m_ptr->links.size(); l++) {
                m2_ptr = m_ptr->links[l];
                if(!m2_ptr->type) continue;
                
                bool show_link =
                    is_selected ||
                    selectedMobs.find(m2_ptr) != selectedMobs.end();
                    
                if(show_link) {
                    drawArrow(
                        m_ptr->pos, m2_ptr->pos,
                        m_ptr->type->radius, m2_ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        al_map_rgb(160, 224, 64)
                    );
                }
            }
            
            if(m_ptr->storedInside != INVALID) {
                m2_ptr =
                    game.curAreaData->mobGenerators[m_ptr->storedInside];
                if(!m2_ptr->type) continue;
                
                bool show_store =
                    is_selected ||
                    selectedMobs.find(m2_ptr) != selectedMobs.end();
                    
                if(show_store) {
                    drawArrow(
                        m_ptr->pos, m2_ptr->pos,
                        m_ptr->type->radius, m2_ptr->type->radius,
                        AREA_EDITOR::MOB_LINK_THICKNESS,
                        al_map_rgb(224, 200, 200)
                    );
                }
            }
        }
    }
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        
        float radius = getMobGenRadius(m_ptr);
        ALLEGRO_COLOR color = al_map_rgb(255, 0, 0);
        if(m_ptr->type && m_ptr != problemMobPtr) {
            color =
                changeAlpha(
                    m_ptr->type->category->editorColor, mob_opacity * 255
                );
        }
        
        if(m_ptr->type && m_ptr->type->rectangularDim.x != 0) {
            drawRotatedRectangle(
                m_ptr->pos, m_ptr->type->rectangularDim,
                m_ptr->angle, color, 1.0f / game.cam.zoom
            );
        }
        
        //Draw children of this mob.
        if(m_ptr->type) {
            for(size_t c = 0; c < m_ptr->type->children.size(); c++) {
                MobType::Child* child_info =
                    &m_ptr->type->children[c];
                MobType::SpawnInfo* spawn_info =
                    getSpawnInfoFromChildInfo(m_ptr->type, child_info);
                if(!spawn_info) continue;
                
                Point c_pos =
                    m_ptr->pos +
                    rotatePoint(spawn_info->coordsXY, m_ptr->angle);
                MobType* c_type =
                    game.mobCategories.findMobType(
                        spawn_info->mobTypeName
                    );
                if(!c_type) continue;
                
                if(c_type->rectangularDim.x != 0) {
                    float c_rot = m_ptr->angle + spawn_info->angle;
                    drawRotatedRectangle(
                        c_pos, c_type->rectangularDim,
                        c_rot, color, 1.0f / game.cam.zoom
                    );
                } else {
                    al_draw_circle(
                        c_pos.x, c_pos.y, c_type->radius,
                        color, 1.0f / game.cam.zoom
                    );
                }
                
            }
        }
        
        al_draw_filled_circle(
            m_ptr->pos.x, m_ptr->pos.y,
            radius, color
        );
        
        float lrw = cos(m_ptr->angle) * radius;
        float lrh = sin(m_ptr->angle) * radius;
        float lt = radius / 8.0;
        
        al_draw_line(
            m_ptr->pos.x - lrw * 0.8, m_ptr->pos.y - lrh * 0.8,
            m_ptr->pos.x + lrw * 0.8, m_ptr->pos.y + lrh * 0.8,
            al_map_rgba(0, 0, 0, mob_opacity * 255), lt
        );
        
        float tx1 = m_ptr->pos.x + lrw;
        float ty1 = m_ptr->pos.y + lrh;
        float tx2 =
            tx1 + cos(m_ptr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty2 =
            ty1 + sin(m_ptr->angle - (TAU / 4 + TAU / 8)) * radius * 0.5;
        float tx3 =
            tx1 + cos(m_ptr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
        float ty3 =
            ty1 + sin(m_ptr->angle + (TAU / 4 + TAU / 8)) * radius * 0.5;
            
        al_draw_filled_triangle(
            tx1, ty1,
            tx2, ty2,
            tx3, ty3,
            al_map_rgba(0, 0, 0, mob_opacity * 255)
        );
        
        bool is_selected =
            selectedMobs.find(m_ptr) != selectedMobs.end();
        bool is_mission_requirement =
            subState == EDITOR_SUB_STATE_MISSION_MOBS &&
            game.curAreaData->mission.goalMobIdxs.find(m) !=
            game.curAreaData->mission.goalMobIdxs.end();
        bool is_highlighted =
            highlightedMob == m_ptr &&
            state == EDITOR_STATE_MOBS;
            
        if(is_selected || is_mission_requirement) {
            al_draw_filled_circle(
                m_ptr->pos.x, m_ptr->pos.y, radius,
                al_map_rgba(
                    AREA_EDITOR::SELECTION_COLOR[0],
                    AREA_EDITOR::SELECTION_COLOR[1],
                    AREA_EDITOR::SELECTION_COLOR[2],
                    selection_opacity * 255
                )
            );
            
            if(
                game.options.areaEd.showTerritory &&
                m_ptr->type &&
                m_ptr->type->territoryRadius > 0 &&
                is_selected
            ) {
                al_draw_circle(
                    m_ptr->pos.x, m_ptr->pos.y, m_ptr->type->territoryRadius,
                    al_map_rgb(240, 240, 192), 1.0f / game.cam.zoom
                );
            }
            if(
                game.options.areaEd.showTerritory &&
                m_ptr->type &&
                m_ptr->type->terrainRadius > 0 &&
                is_selected
            ) {
                al_draw_circle(
                    m_ptr->pos.x, m_ptr->pos.y, m_ptr->type->terrainRadius,
                    al_map_rgb(240, 192, 192), 1.0f / game.cam.zoom
                );
            }
        } else if(is_highlighted) {
            al_draw_filled_circle(
                m_ptr->pos.x, m_ptr->pos.y, radius,
                al_map_rgba(
                    highlight_color.r * 255,
                    highlight_color.g * 255,
                    highlight_color.b * 255,
                    64
                )
            );
        }
        
    }
    
    //Paths.
    if(state == EDITOR_STATE_PATHS) {
    
        //Stops.
        for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
            PathStop* s_ptr = game.curAreaData->pathStops[s];
            bool highlighted = highlightedPathStop == s_ptr;
            ALLEGRO_COLOR color;
            if(hasFlag(s_ptr->flags, PATH_STOP_FLAG_SCRIPT_ONLY)) {
                color = al_map_rgba(187, 102, 34, 224);
            } else if(hasFlag(s_ptr->flags, PATH_STOP_FLAG_LIGHT_LOAD_ONLY)) {
                color = al_map_rgba(102, 170, 34, 224);
            } else if(hasFlag(s_ptr->flags, PATH_STOP_FLAG_AIRBORNE_ONLY)) {
                color = al_map_rgba(187, 102, 153, 224);
            } else {
                color = al_map_rgb(88, 177, 177);
            }
            al_draw_filled_circle(
                s_ptr->pos.x, s_ptr->pos.y,
                s_ptr->radius,
                color
            );
            
            if(
                selectedPathStops.find(s_ptr) !=
                selectedPathStops.end()
            ) {
                al_draw_filled_circle(
                    s_ptr->pos.x, s_ptr->pos.y, s_ptr->radius,
                    al_map_rgba(
                        AREA_EDITOR::SELECTION_COLOR[0],
                        AREA_EDITOR::SELECTION_COLOR[1],
                        AREA_EDITOR::SELECTION_COLOR[2],
                        selection_opacity * 255
                    )
                );
            } else if(highlighted) {
                al_draw_filled_circle(
                    s_ptr->pos.x, s_ptr->pos.y, s_ptr->radius,
                    al_map_rgba(
                        highlight_color.r * 255,
                        highlight_color.g * 255,
                        highlight_color.b * 255,
                        128
                    )
                );
            }
            
            if(debugPathIdxs) {
                drawDebugText(
                    al_map_rgb(80, 192, 192), s_ptr->pos, i2s(s)
                );
            }
        }
        
        //Links.
        for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
            PathStop* s_ptr = game.curAreaData->pathStops[s];
            for(size_t l = 0; l < s_ptr->links.size(); l++) {
                PathLink* l_ptr = s_ptr->links[l];
                PathStop* s2_ptr = l_ptr->endPtr;
                bool one_way =
                    !l_ptr->endPtr->get_link(s_ptr);
                bool selected =
                    selectedPathLinks.find(l_ptr) !=
                    selectedPathLinks.end();
                bool highlighted = highlightedPathLink == l_ptr;
                ALLEGRO_COLOR color = COLOR_WHITE;
                if(selected) {
                    color =
                        al_map_rgba(
                            AREA_EDITOR::SELECTION_COLOR[0],
                            AREA_EDITOR::SELECTION_COLOR[1],
                            AREA_EDITOR::SELECTION_COLOR[2],
                            selection_opacity * 255
                        );
                } else if(highlighted) {
                    color =
                        al_map_rgba(
                            highlight_color.r * 255,
                            highlight_color.g * 255,
                            highlight_color.b * 255,
                            255
                        );
                } else {
                    switch(l_ptr->type) {
                    case PATH_LINK_TYPE_NORMAL: {
                        color = al_map_rgba(34, 136, 187, 224);
                        break;
                    } case PATH_LINK_TYPE_LEDGE: {
                        color = al_map_rgba(180, 180, 64, 224);
                        break;
                    }
                    }
                    if(!one_way) {
                        color = changeColorLighting(color, 0.33f);
                    }
                }
                
                float angle =
                    getAngle(s_ptr->pos, s2_ptr->pos);
                Point offset1 =
                    angleToCoordinates(angle, s_ptr->radius);
                Point offset2 =
                    angleToCoordinates(angle, s2_ptr->radius);
                al_draw_line(
                    s_ptr->pos.x + offset1.x,
                    s_ptr->pos.y + offset1.y,
                    s2_ptr->pos.x - offset2.x,
                    s2_ptr->pos.y - offset2.y,
                    color,
                    AREA_EDITOR::PATH_LINK_THICKNESS / game.cam.zoom
                );
                
                if(
                    state == EDITOR_STATE_PATHS &&
                    moving &&
                    game.options.areaEd.showPathLinkLength
                ) {
                    bool draw_dist = false;
                    Point other_point;
                    if(
                        l_ptr->startPtr == moveClosestStop &&
                        selectedPathStops.find(l_ptr->endPtr) ==
                        selectedPathStops.end()
                    ) {
                        other_point.x = l_ptr->endPtr->pos.x;
                        other_point.y = l_ptr->endPtr->pos.y;
                        draw_dist = true;
                    } else if(
                        l_ptr->endPtr == moveClosestStop &&
                        selectedPathStops.find(l_ptr->startPtr) ==
                        selectedPathStops.end()
                    ) {
                        other_point.x = l_ptr->startPtr->pos.x;
                        other_point.y = l_ptr->startPtr->pos.y;
                        draw_dist = true;
                    }
                    
                    if(draw_dist) {
                        drawLineDist(moveClosestStop->pos, other_point);
                    }
                }
                
                if(debugPathIdxs && (one_way || s < s_ptr->links[l]->endIdx)) {
                    Point middle = (s_ptr->pos + s2_ptr->pos) / 2.0f;
                    drawDebugText(
                        al_map_rgb(96, 104, 224),
                        Point(
                            middle.x + cos(angle + TAU / 4) * 4,
                            middle.y + sin(angle + TAU / 4) * 4
                        ),
                        f2s(s_ptr->links[l]->distance)
                    );
                }
                
                if(one_way) {
                    //Draw a triangle down the middle.
                    float mid_x =
                        (s_ptr->pos.x + s2_ptr->pos.x) / 2.0f;
                    float mid_y =
                        (s_ptr->pos.y + s2_ptr->pos.y) / 2.0f;
                    const float delta =
                        (AREA_EDITOR::PATH_LINK_THICKNESS * 4) / game.cam.zoom;
                        
                    al_draw_filled_triangle(
                        mid_x + cos(angle) * delta,
                        mid_y + sin(angle) * delta,
                        mid_x + cos(angle + TAU / 4) * delta,
                        mid_y + sin(angle + TAU / 4) * delta,
                        mid_x + cos(angle - TAU / 4) * delta,
                        mid_y + sin(angle - TAU / 4) * delta,
                        color
                    );
                }
            }
        }
        
        //Closest stop line.
        if(showClosestStop) {
            PathStop* closest = nullptr;
            float closest_dist = FLT_MAX;
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* s_ptr = game.curAreaData->pathStops[s];
                float d =
                    Distance(game.mouseCursor.wPos, s_ptr->pos).toFloat() -
                    s_ptr->radius;
                    
                if(!closest || d < closest_dist) {
                    closest = s_ptr;
                    closest_dist = d;
                }
            }
            
            if(closest) {
                al_draw_line(
                    game.mouseCursor.wPos.x, game.mouseCursor.wPos.y,
                    closest->pos.x, closest->pos.y,
                    al_map_rgb(192, 128, 32), 2.0 / game.cam.zoom
                );
            }
        }
        
        //Path preview.
        if(showPathPreview) {
            //Draw the lines of the path.
            ALLEGRO_COLOR lines_color = al_map_rgb(255, 187, 136);
            ALLEGRO_COLOR invalid_lines_color = al_map_rgb(221, 17, 17);
            float lines_thickness = 4.0f / game.cam.zoom;
            
            if(!pathPreview.empty()) {
                al_draw_line(
                    pathPreviewCheckpoints[0].x,
                    pathPreviewCheckpoints[0].y,
                    pathPreview[0]->pos.x,
                    pathPreview[0]->pos.y,
                    lines_color, lines_thickness
                );
                for(size_t s = 0; s < pathPreview.size() - 1; s++) {
                    al_draw_line(
                        pathPreview[s]->pos.x,
                        pathPreview[s]->pos.y,
                        pathPreview[s + 1]->pos.x,
                        pathPreview[s + 1]->pos.y,
                        lines_color, lines_thickness
                    );
                }
                al_draw_line(
                    pathPreview.back()->pos.x,
                    pathPreview.back()->pos.y,
                    pathPreviewCheckpoints[1].x,
                    pathPreviewCheckpoints[1].y,
                    lines_color, lines_thickness
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
                    lines_color, lines_thickness
                );
            } else {
                for(size_t c = 0; c < 2; c++) {
                    if(pathPreviewClosest[c]) {
                        al_draw_line(
                            pathPreviewClosest[c]->pos.x,
                            pathPreviewClosest[c]->pos.y,
                            pathPreviewCheckpoints[c].x,
                            pathPreviewCheckpoints[c].y,
                            invalid_lines_color, lines_thickness
                        );
                    }
                }
            }
            
            //Draw the checkpoints.
            for(unsigned char c = 0; c < 2; c++) {
                string letter = (c == 0 ? "A" : "B");
                
                const float factor =
                    AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS / game.cam.zoom;
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
                        game.cam.zoom,
                        AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS * 1.8f /
                        game.cam.zoom
                    ),
                    al_map_rgb(0, 64, 64)
                );
            }
        }
    }
    
    //Tree shadows.
    if(
        state == EDITOR_STATE_DETAILS ||
        (previewMode && showShadows)
    ) {
        for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
        
            TreeShadow* s_ptr = game.curAreaData->treeShadows[s];
            if(
                !previewMode &&
                s_ptr == selectedShadow
            ) {
                //Draw a white rectangle to contrast the shadow better.
                ALLEGRO_TRANSFORM tra, current;
                al_identity_transform(&tra);
                al_rotate_transform(&tra, s_ptr->angle);
                al_translate_transform(
                    &tra, s_ptr->center.x, s_ptr->center.y
                );
                al_copy_transform(&current, al_get_current_transform());
                al_compose_transform(&tra, &current);
                al_use_transform(&tra);
                
                al_draw_filled_rectangle(
                    -s_ptr->size.x / 2.0,
                    -s_ptr->size.y / 2.0,
                    s_ptr->size.x / 2.0,
                    s_ptr->size.y / 2.0,
                    al_map_rgba(255, 255, 255, 96 * (s_ptr->alpha / 255.0))
                );
                
                al_use_transform(&current);
            }
            
            drawBitmap(
                s_ptr->bitmap, s_ptr->center, s_ptr->size,
                s_ptr->angle, mapAlpha(s_ptr->alpha)
            );
            
            if(state == EDITOR_STATE_DETAILS) {
                Point min_coords, max_coords;
                getTransformedRectangleBBox(
                    s_ptr->center, s_ptr->size, s_ptr->angle,
                    &min_coords, &max_coords
                );
                
                if(selectedShadow != s_ptr) {
                    al_draw_rectangle(
                        min_coords.x, min_coords.y, max_coords.x, max_coords.y,
                        al_map_rgb(128, 128, 64), 2.0 / game.cam.zoom
                    );
                }
            }
        }
        if(selectedShadow) {
            curTransformationWidget.draw(
                &selectedShadow->center,
                &selectedShadow->size,
                &selectedShadow->angle,
                1.0f / game.cam.zoom
            );
        }
    }
    
    //Mission exit region transformation widget.
    if(subState == EDITOR_SUB_STATE_MISSION_EXIT) {
        curTransformationWidget.draw(
            &game.curAreaData->mission.goalExitCenter,
            &game.curAreaData->mission.goalExitSize,
            nullptr,
            1.0f / game.cam.zoom
        );
    }
    
    //Cross-section points and line.
    if(state == EDITOR_STATE_REVIEW && showCrossSection) {
        for(unsigned char p = 0; p < 2; p++) {
            string letter = (p == 0 ? "A" : "B");
            
            al_draw_filled_rectangle(
                crossSectionCheckpoints[p].x -
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                crossSectionCheckpoints[p].y -
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                crossSectionCheckpoints[p].x +
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                crossSectionCheckpoints[p].y +
                (AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom),
                al_map_rgb(255, 255, 32)
            );
            drawText(
                letter, game.sysContent.fntBuiltin,
                crossSectionCheckpoints[p],
                Point(
                    AREA_EDITOR::CROSS_SECTION_POINT_RADIUS * 1.8f /
                    game.cam.zoom,
                    AREA_EDITOR::CROSS_SECTION_POINT_RADIUS * 1.8f /
                    game.cam.zoom
                ),
                al_map_rgb(0, 64, 64)
            );
        }
        al_draw_line(
            crossSectionCheckpoints[0].x,
            crossSectionCheckpoints[0].y,
            crossSectionCheckpoints[1].x,
            crossSectionCheckpoints[1].y,
            al_map_rgb(255, 0, 0), 3.0 / game.cam.zoom
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
                1.0f / game.cam.zoom
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
                3.0 / game.cam.zoom
            );
        }
        if(!drawingNodes.empty()) {
            ALLEGRO_COLOR new_line_color =
                interpolateColor(
                    newSectorErrorTintTimer.getRatioLeft(),
                    1, 0,
                    al_map_rgb(255, 0, 0),
                    al_map_rgb(64, 255, 64)
                );
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            
            al_draw_line(
                drawingNodes.back().snappedSpot.x,
                drawingNodes.back().snappedSpot.y,
                hotspot.x,
                hotspot.y,
                new_line_color,
                3.0 / game.cam.zoom
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
            float circle_radius =
                Distance(
                    newCircleSectorCenter, newCircleSectorAnchor
                ).toFloat();
            al_draw_circle(
                newCircleSectorCenter.x,
                newCircleSectorCenter.y,
                circle_radius,
                al_map_rgb(64, 255, 64),
                3.0 / game.cam.zoom
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
                Point cur_point = newCircleSectorPoints[p];
                Point next_point =
                    getNextInVector(newCircleSectorPoints, p);
                ALLEGRO_COLOR color =
                    newCircleSectorValidEdges[p] ?
                    al_map_rgb(64, 255, 64) :
                    al_map_rgb(255, 0, 0);
                    
                al_draw_line(
                    cur_point.x, cur_point.y,
                    next_point.x, next_point.y,
                    color, 3.0 / game.cam.zoom
                );
            }
            
            for(size_t p = 0; p < newCircleSectorPoints.size(); p++) {
                al_draw_filled_circle(
                    newCircleSectorPoints[p].x,
                    newCircleSectorPoints[p].y,
                    3.0 / game.cam.zoom, al_map_rgb(192, 255, 192)
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
        Point nr_coords = quickHeightSetStartPos;
        nr_coords.x += 100.0f;
        al_transform_coordinates(
            &game.screenToWorldTransform, &nr_coords.x, &nr_coords.y
        );
        float offset = getQuickHeightSetOffset();
        drawDebugText(
            al_map_rgb(64, 255, 64),
            nr_coords,
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
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            al_draw_line(
                pathDrawingStop1->pos.x,
                pathDrawingStop1->pos.y,
                hotspot.x,
                hotspot.y,
                al_map_rgb(64, 255, 64),
                3.0 / game.cam.zoom
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
            2.0 / game.cam.zoom
            
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
        Point marker = game.mouseCursor.wPos;
        
        if(subState != EDITOR_SUB_STATE_ADD_MOB_LINK) {
            marker = snapPoint(marker);
        }
        
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y,
            marker.x + 10 / game.cam.zoom,
            marker.y,
            COLOR_WHITE, 2.0 / game.cam.zoom
        );
        al_draw_line(
            marker.x,
            marker.y - 10 / game.cam.zoom,
            marker.x,
            marker.y + 10 / game.cam.zoom,
            COLOR_WHITE, 2.0 / game.cam.zoom
        );
    }
    
    //Delete thing marker.
    if(
        subState == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        Point marker = game.mouseCursor.wPos;
        
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y - 10 / game.cam.zoom,
            marker.x + 10 / game.cam.zoom,
            marker.y + 10 / game.cam.zoom,
            COLOR_WHITE, 2.0 / game.cam.zoom
        );
        al_draw_line(
            marker.x - 10 / game.cam.zoom,
            marker.y + 10 / game.cam.zoom,
            marker.x + 10 / game.cam.zoom,
            marker.y - 10 / game.cam.zoom,
            COLOR_WHITE, 2.0 / game.cam.zoom
        );
    }
    
    al_use_transform(&game.identityTransform);
    
    //Cross-section graph.
    if(state == EDITOR_STATE_REVIEW && showCrossSection) {
    
        Distance cross_section_world_length(
            crossSectionCheckpoints[0], crossSectionCheckpoints[1]
        );
        float proportion =
            (crossSectionWindowEnd.x - crossSectionWindowStart.x) /
            cross_section_world_length.toFloat();
            
        ALLEGRO_COLOR bg_color =
            game.options.editors.useCustomStyle ?
            changeColorLighting(game.options.editors.primaryColor, -0.3f) :
            al_map_rgb(0, 0, 64);
            
        al_draw_filled_rectangle(
            crossSectionWindowStart.x, crossSectionWindowStart.y,
            crossSectionWindowEnd.x, crossSectionWindowEnd.y,
            bg_color
        );
        
        if(showCrossSectionGrid) {
            al_draw_filled_rectangle(
                crossSectionZWindowStart.x, crossSectionZWindowStart.y,
                crossSectionZWindowEnd.x, crossSectionZWindowEnd.y,
                COLOR_BLACK
            );
        }
        
        Sector* cs_left_sector =
            getSector(crossSectionCheckpoints[0], nullptr, false);
        Sector* cs_right_sector =
            getSector(crossSectionCheckpoints[1], nullptr, false);
            
        /**
         * @brief Info about a split.
         */
        struct Split {
        
            //--- Members ---
            
            //Sector pointers.
            Sector* sector_ptrs[2] = { nullptr, nullptr };
            
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
                sector_ptrs[0] = s1;
                sector_ptrs[1] = s2;
                this->l1r = l1r;
                this->l2r = l2r;
            }
            
        };
        vector<Split> splits;
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            float l1r = 0;
            float l2r = 0;
            if(
                lineSegsIntersect(
                    v2p(e_ptr->vertexes[0]),
                    v2p(e_ptr->vertexes[1]),
                    crossSectionCheckpoints[0],
                    crossSectionCheckpoints[1],
                    &l1r, &l2r
                )
            ) {
                splits.push_back(
                    Split(e_ptr->sectors[0], e_ptr->sectors[1], l1r, l2r)
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
                Split(cs_left_sector, cs_left_sector, 0, 0)
            );
            splits.push_back(
                Split(cs_right_sector, cs_right_sector, 1, 1)
            );
            
            for(size_t s = 1; s < splits.size(); s++) {
                if(splits[s].sector_ptrs[0] != splits[s - 1].sector_ptrs[1]) {
                    std::swap(
                        splits[s].sector_ptrs[0], splits[s].sector_ptrs[1]
                    );
                }
            }
            
            float lowest_z = 0;
            bool got_lowest_z = false;
            for(size_t sp = 1; sp < splits.size(); sp++) {
                for(size_t se = 0; se < 2; se++) {
                    if(
                        splits[sp].sector_ptrs[se] &&
                        (
                            splits[sp].sector_ptrs[se]->z < lowest_z ||
                            !got_lowest_z
                        )
                    ) {
                        lowest_z = splits[sp].sector_ptrs[se]->z;
                        got_lowest_z = true;
                    }
                }
            }
            
            int ocr_x, ocr_y, ocr_w, ocr_h;
            al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
            al_set_clipping_rectangle(
                crossSectionWindowStart.x, crossSectionWindowStart.y,
                crossSectionWindowEnd.x - crossSectionWindowStart.x,
                crossSectionWindowEnd.y - crossSectionWindowStart.y
            );
            
            for(size_t s = 1; s < splits.size(); s++) {
                if(!splits[s].sector_ptrs[0]) continue;
                drawCrossSectionSector(
                    splits[s - 1].l2r, splits[s].l2r, proportion,
                    lowest_z, splits[s].sector_ptrs[0]
                );
            }
            
            Sector* central_sector = nullptr;
            for(size_t s = 1; s < splits.size(); s++) {
                if(splits[s].l2r > 0.5) {
                    central_sector = splits[s].sector_ptrs[0];
                    break;
                }
            }
            
            if(central_sector) {
                float leader_silhouette_w =
                    game.config.leaders.standardRadius * 2.0 * proportion;
                float leader_silhouette_h =
                    game.config.leaders.standardHeight * proportion;
                float leader_silhouette_pivot_x =
                    (
                        crossSectionWindowStart.x +
                        crossSectionWindowEnd.x
                    ) / 2.0;
                float leader_silhouette_pivot_y =
                    crossSectionWindowEnd.y - 8 -
                    ((central_sector->z - lowest_z) * proportion);
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
                    leader_silhouette_pivot_x - leader_silhouette_w / 2.0,
                    leader_silhouette_pivot_y - leader_silhouette_h,
                    leader_silhouette_w, leader_silhouette_h,
                    0
                );
            }
            
            al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
            
            float highest_z =
                lowest_z + crossSectionWindowEnd.y / proportion;
                
            if(showCrossSectionGrid) {
                for(float z = lowest_z; z <= highest_z; z += 50) {
                    float line_y =
                        crossSectionWindowEnd.y - 8 -
                        ((z - lowest_z) * proportion);
                    al_draw_line(
                        crossSectionWindowStart.x, line_y,
                        crossSectionZWindowStart.x + 6, line_y,
                        COLOR_WHITE, 1
                    );
                    
                    drawText(
                        i2s(z), game.sysContent.fntBuiltin,
                        Point(
                            (crossSectionZWindowStart.x + 8),
                            line_y
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
        
        float cursor_segment_ratio = 0;
        getClosestPointInLineSeg(
            crossSectionCheckpoints[0], crossSectionCheckpoints[1],
            Point(game.mouseCursor.wPos.x, game.mouseCursor.wPos.y),
            &cursor_segment_ratio
        );
        if(cursor_segment_ratio >= 0 && cursor_segment_ratio <= 1) {
            al_draw_line(
                crossSectionWindowStart.x +
                (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
                cursor_segment_ratio,
                crossSectionWindowStart.y,
                crossSectionWindowStart.x +
                (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
                cursor_segment_ratio,
                crossSectionWindowEnd.y,
                al_map_rgba(255, 255, 255, 128), 1
            );
        }
        
        float cross_section_x2 =
            showCrossSectionGrid ? crossSectionZWindowEnd.x :
            crossSectionWindowEnd.x;
        al_draw_line(
            crossSectionWindowStart.x, crossSectionWindowEnd.y + 1,
            cross_section_x2 + 2, crossSectionWindowEnd.y + 1,
            al_map_rgb(160, 96, 96), 2
        );
        al_draw_line(
            cross_section_x2 + 1, crossSectionWindowStart.y,
            cross_section_x2 + 1, crossSectionWindowEnd.y + 2,
            al_map_rgb(160, 96, 96), 2
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}


/**
 * @brief Draws a sector on the cross-section view.
 *
 * @param start_ratio Where the sector starts on the graph ([0, 1]).
 * @param end_ratio Where the sector end on the graph ([0, 1]).
 * @param proportion Ratio of how much to resize the heights by.
 * @param lowest_z What z coordinate represents the bottom of the graph.
 * @param sector_ptr Pointer to the sector to draw.
 */
void AreaEditor::drawCrossSectionSector(
    float start_ratio, float end_ratio, float proportion,
    float lowest_z, const Sector* sector_ptr
) {
    float rectangle_x1 =
        crossSectionWindowStart.x +
        (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
        start_ratio;
    float rectangle_x2 =
        crossSectionWindowStart.x +
        (crossSectionWindowEnd.x - crossSectionWindowStart.x) *
        end_ratio;
    float rectangle_y =
        crossSectionWindowEnd.y - 8 -
        ((sector_ptr->z - lowest_z) * proportion);
        
    ALLEGRO_COLOR color =
        game.options.editors.useCustomStyle ?
        changeColorLighting(game.options.editors.secondaryColor, -0.2f) :
        al_map_rgb(0, 64, 0);
        
    al_draw_filled_rectangle(
        rectangle_x1, rectangle_y,
        rectangle_x2 + 1, crossSectionWindowEnd.y + 1,
        color
    );
    al_draw_line(
        rectangle_x1 + 0.5, rectangle_y,
        rectangle_x1 + 0.5, crossSectionWindowEnd.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangle_x2 + 0.5, rectangle_y,
        rectangle_x2 + 0.5, crossSectionWindowEnd.y,
        al_map_rgb(192, 192, 192), 1
    );
    al_draw_line(
        rectangle_x1, rectangle_y + 0.5,
        rectangle_x2, rectangle_y + 0.5,
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
    
    float bbox_w = (dw * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.cam.zoom;
    float bbox_h = (dh * AREA_EDITOR::DEBUG_TEXT_SCALE) / game.cam.zoom;
    
    al_draw_filled_rectangle(
        where.x - bbox_w * 0.5, where.y - bbox_h * 0.5,
        where.x + bbox_w * 0.5, where.y + bbox_h * 0.5,
        al_map_rgba(0, 0, 0, 128)
    );
    
    drawText(
        text, game.sysContent.fntBuiltin, where,
        Point(bbox_w, bbox_h) * 0.80f, color
    );
    
    if(dots > 0) {
        al_draw_filled_rectangle(
            where.x - 3.0f / game.cam.zoom,
            where.y + bbox_h * 0.5f,
            where.x + 3.0f / game.cam.zoom,
            where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
            al_map_rgba(0, 0, 0, 128)
        );
        
        if(dots == 1) {
            al_draw_filled_rectangle(
                where.x - 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x + 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
        } else {
            al_draw_filled_rectangle(
                where.x - 3.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x - 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
            al_draw_filled_rectangle(
                where.x + 1.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 1.0f / game.cam.zoom,
                where.x + 3.0f / game.cam.zoom,
                where.y + bbox_h * 0.5f + 3.0f / game.cam.zoom,
                color
            );
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
    Point length_nr_pos;
    length_nr_pos.x = focus.x + cos(angle) * 64.0;
    length_nr_pos.y = focus.y + sin(angle) * 64.0;
    length_nr_pos.y -= 12;
    
    drawDebugText(
        AREA_EDITOR::MEASUREMENT_COLOR, length_nr_pos, prefix + i2s(d)
    );
}
