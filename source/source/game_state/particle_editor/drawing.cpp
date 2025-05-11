/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor drawing function.
 */

#include "editor.h"

#include <algorithm>

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
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y, game.editorsView.size.x, game.editorsView.size.y
    );
    
    //Background.
    if(useBg && bg) {
        Point textureTL = canvasTL;
        Point textureBR = canvasBR;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &textureTL.x, &textureTL.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform, &textureBR.x, &textureBR.y
        );
        ALLEGRO_VERTEX bgVertexes[4];
        for(size_t v = 0; v < 4; ++v) {
            bgVertexes[v].z = 0;
            bgVertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bgVertexes[0].x = canvasTL.x;
        bgVertexes[0].y = canvasTL.y;
        bgVertexes[0].u = textureTL.x;
        bgVertexes[0].v = textureTL.y;
        //Top-right vertex.
        bgVertexes[1].x = canvasBR.x;
        bgVertexes[1].y = canvasTL.y;
        bgVertexes[1].u = textureBR.x;
        bgVertexes[1].v = textureTL.y;
        //Bottom-right vertex.
        bgVertexes[2].x = canvasBR.x;
        bgVertexes[2].y = canvasBR.y;
        bgVertexes[2].u = textureBR.x;
        bgVertexes[2].v = textureBR.y;
        //Bottom-left vertex.
        bgVertexes[3].x = canvasTL.x;
        bgVertexes[3].y = canvasBR.y;
        bgVertexes[3].u = textureTL.x;
        bgVertexes[3].v = textureBR.y;
        
        al_draw_prim(
            bgVertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.editorsView.worldToWindowTransform);
    
    //Particles.
    vector<WorldComponent> components;
    components.reserve(partMgr.getCount());
    partMgr.fillComponentList(components, game.editorsView.box[0], game.editorsView.box[1]);
    
    for(size_t c = 0; c < components.size(); ++c) {
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
    for(size_t c = 0; c < components.size(); ++c) {
        WorldComponent* cPtr = &components[c];
        if(cPtr->particlePtr) {
            cPtr->particlePtr->draw();
        }
    }
    
    //Grid.
    if(gridVisible) {
        Point camTLCorner(0, 0);
        Point camBRCorner(canvasBR.x, canvasBR.y);
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camTLCorner.x, &camTLCorner.y
        );
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &camBRCorner.x, &camBRCorner.y
        );
        
        al_draw_line(
            0, camTLCorner.y, 0, camBRCorner.y,
            al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
        );
        al_draw_line(
            camTLCorner.x, 0, camBRCorner.x, 0,
            al_map_rgb(240, 240, 240), 1.0f / game.editorsView.cam.zoom
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
                    al_map_rgb(100, 240, 100), 3.0f / game.editorsView.cam.zoom
                );
                al_draw_circle(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    al_map_rgb(240, 100, 100), 3.0f / game.editorsView.cam.zoom
                );
            } else {
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleOuterDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    al_map_rgb(100, 240, 100), 3.0f / game.editorsView.cam.zoom
                );
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    al_map_rgb(240, 100, 100), 3.0f / game.editorsView.cam.zoom
                );
            }
            break;
            
        } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
    
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectOuterDist * 2.0f,
                generatorAngleOffset,
                al_map_rgb(100, 240, 100), 3.0f / game.editorsView.cam.zoom
            );
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectInnerDist * 2.0f,
                generatorAngleOffset,
                al_map_rgb(240, 100, 100), 3.0f / game.editorsView.cam.zoom
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
            0, al_map_rgba(240, 240, 240, 160)
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
