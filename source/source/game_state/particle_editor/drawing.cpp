/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor drawing function.
 */

#include <algorithm>

#include "editor.h"

#include "../../content/mob/mob_utils.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"


/**
 * @brief Handles the drawing part of the main loop of the particle editor.
 */
void ParticleEditor::doDrawing() {
    //The canvas drawing is handled by Dear ImGui elsewhere.
    
    al_clear_to_color(COLOR_BLACK);
    drawOpErrorCursor();
}


/**
 * @brief Draw the canvas. This is called as a callback inside the
 * Dear ImGui rendering process.
 */
void ParticleEditor::drawCanvas() {
    const ALLEGRO_COLOR BG_COLOR = al_map_rgb(128, 144, 128);
    const ALLEGRO_COLOR OUTER_EMISSION_COLOR = al_map_rgb(100, 240, 100);
    const ALLEGRO_COLOR INNER_EMISSION_COLOR = al_map_rgb(240, 100, 100);
    
    RectCorners canvasCorners = game.editorsView.getWindowCorners();
    al_set_clipping_rectangle(
        canvasCorners.tl.x, canvasCorners.tl.y,
        game.editorsView.windowRect.size.x, game.editorsView.windowRect.size.y
    );
    
    //Background.
    if(useBg && bg) {
        RectCorners textureCorners = canvasCorners;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &textureCorners.tl.x, &textureCorners.tl.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &textureCorners.br.x, &textureCorners.br.y
        );
        ALLEGRO_VERTEX bgVertexes[4];
        for(size_t v = 0; v < 4; v++) {
            bgVertexes[v].z = 0;
            bgVertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bgVertexes[0].x = canvasCorners.tl.x;
        bgVertexes[0].y = canvasCorners.tl.y;
        bgVertexes[0].u = textureCorners.tl.x;
        bgVertexes[0].v = textureCorners.tl.y;
        //Top-right vertex.
        bgVertexes[1].x = canvasCorners.br.x;
        bgVertexes[1].y = canvasCorners.tl.y;
        bgVertexes[1].u = textureCorners.br.x;
        bgVertexes[1].v = textureCorners.tl.y;
        //Bottom-right vertex.
        bgVertexes[2].x = canvasCorners.br.x;
        bgVertexes[2].y = canvasCorners.br.y;
        bgVertexes[2].u = textureCorners.br.x;
        bgVertexes[2].v = textureCorners.br.y;
        //Bottom-left vertex.
        bgVertexes[3].x = canvasCorners.tl.x;
        bgVertexes[3].y = canvasCorners.br.y;
        bgVertexes[3].u = textureCorners.tl.x;
        bgVertexes[3].v = textureCorners.br.y;
        
        al_draw_prim(
            bgVertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(BG_COLOR);
    }
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
    //Particles.
    vector<WorldComponent> components;
    components.reserve(partMgr.getCount());
    partMgr.fillComponentList(components, game.editorsView.worldCorners);
    
    forIdx(c, components) {
        components[c].idx = c;
    }
    
    sort(
        components.begin(), components.end(),
    [](const WorldComponent & c1, const WorldComponent & c2) -> bool {
        if(c1.z == c2.z) {
            return c1.idx < c2.idx;
        }
        return c1.z < c2.z;
    }
    );
    forIdx(c, components) {
        WorldComponent* cPtr = &components[c];
        if(cPtr->particlePtr) {
            cPtr->particlePtr->draw();
        }
    }
    
    //Grid.
    if(gridVisible) {
        RectCorners camCorners(Point(0.0f), canvasCorners.br);
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camCorners.tl.x, &camCorners.tl.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camCorners.br.x, &camCorners.br.y
        );
        
        al_draw_line(
            0, camCorners.tl.y, 0, camCorners.br.y,
            EDITOR::GRID_COLOR_ORIGIN, 1.0f / game.editorsView.cam.zoom
        );
        al_draw_line(
            camCorners.tl.x, 0, camCorners.br.x, 0,
            EDITOR::GRID_COLOR_ORIGIN, 1.0f / game.editorsView.cam.zoom
        );
    }
    
    //Emission shapes.
    if(emissionShapeVisible) {
        switch (loadedGen.emission.shape) {
        case PARTICLE_EMISSION_SHAPE_CIRCLE: {
    
            if(loadedGen.emission.circleArc == TAU) {
                al_draw_circle(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleOuterDist,
                    OUTER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
                );
                al_draw_circle(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    INNER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
                );
            } else {
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleOuterDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    OUTER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
                );
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    INNER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
                );
            }
            break;
            
        } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
    
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectOuterDist * 2.0f,
                generatorAngleOffset,
                OUTER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
            );
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectInnerDist * 2.0f,
                generatorAngleOffset,
                INNER_EMISSION_COLOR, 3.0f / game.editorsView.cam.zoom
            );
            break;
            
        }
        }
    }
    
    //Leader silhouette.
    if(leaderSilhouetteVisible) {
        float xOffset = 32.0f;
        
        drawBitmap(
            game.sysContent.bmpLeaderSilhouetteTop, Point(xOffset, 0),
            Point(-1, game.config.leaders.standardRadius * 2.0f),
            0, EDITOR::SILHOUETTE_COLOR
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
