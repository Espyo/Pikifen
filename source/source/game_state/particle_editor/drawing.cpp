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
    al_set_clipping_rectangle(
        canvasTL.x, canvasTL.y,
        canvasBR.x - canvasTL.x, canvasBR.y - canvasTL.y
    );
    
    //Background.
    if(useBg && bg) {
        Point texture_tl = canvasTL;
        Point texture_br = canvasBR;
        al_transform_coordinates(
            &game.screenToWorldTransform, &texture_tl.x, &texture_tl.y
        );
        al_transform_coordinates(
            &game.screenToWorldTransform, &texture_br.x, &texture_br.y
        );
        ALLEGRO_VERTEX bg_vertexes[4];
        for(size_t v = 0; v < 4; ++v) {
            bg_vertexes[v].z = 0;
            bg_vertexes[v].color = COLOR_WHITE;
        }
        //Top-left vertex.
        bg_vertexes[0].x = canvasTL.x;
        bg_vertexes[0].y = canvasTL.y;
        bg_vertexes[0].u = texture_tl.x;
        bg_vertexes[0].v = texture_tl.y;
        //Top-right vertex.
        bg_vertexes[1].x = canvasBR.x;
        bg_vertexes[1].y = canvasTL.y;
        bg_vertexes[1].u = texture_br.x;
        bg_vertexes[1].v = texture_tl.y;
        //Bottom-right vertex.
        bg_vertexes[2].x = canvasBR.x;
        bg_vertexes[2].y = canvasBR.y;
        bg_vertexes[2].u = texture_br.x;
        bg_vertexes[2].v = texture_br.y;
        //Bottom-left vertex.
        bg_vertexes[3].x = canvasTL.x;
        bg_vertexes[3].y = canvasBR.y;
        bg_vertexes[3].u = texture_tl.x;
        bg_vertexes[3].v = texture_br.y;
        
        al_draw_prim(
            bg_vertexes, nullptr, bg,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
    } else {
        al_clear_to_color(al_map_rgb(128, 144, 128));
    }
    
    al_use_transform(&game.worldToScreenTransform);
    
    //Particles.
    vector<WorldComponent> components;
    components.reserve(partMgr.getCount());
    partMgr.fillComponentList(components, game.cam.box[0], game.cam.box[1]);
    
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
        WorldComponent* c_ptr = &components[c];
        if(c_ptr->particle_ptr) {
            c_ptr->particle_ptr->draw();
        }
    }
    
    //Grid.
    if(gridVisible) {
        Point cam_top_left_corner(0, 0);
        Point cam_bottom_right_corner(canvasBR.x, canvasBR.y);
        al_transform_coordinates(
            &game.screenToWorldTransform,
            &cam_top_left_corner.x, &cam_top_left_corner.y
        );
        al_transform_coordinates(
            &game.screenToWorldTransform,
            &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
        );
        
        al_draw_line(
            0, cam_top_left_corner.y, 0, cam_bottom_right_corner.y,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
        );
        al_draw_line(
            cam_top_left_corner.x, 0, cam_bottom_right_corner.x, 0,
            al_map_rgb(240, 240, 240), 1.0f / game.cam.zoom
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
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_circle(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );
            } else {
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleOuterDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
                );
                al_draw_arc(
                    generatorPosOffset.x, generatorPosOffset.y,
                    loadedGen.emission.circleInnerDist,
                    -loadedGen.emission.circleArc / 2.0f +
                    loadedGen.emission.circleArcRot +
                    generatorAngleOffset,
                    loadedGen.emission.circleArc,
                    al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
                );
            }
            break;
            
        } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
    
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectOuterDist * 2.0f,
                generatorAngleOffset,
                al_map_rgb(100, 240, 100), 3.0f / game.cam.zoom
            );
            drawRotatedRectangle(
                generatorPosOffset,
                loadedGen.emission.rectInnerDist * 2.0f,
                generatorAngleOffset,
                al_map_rgb(240, 100, 100), 3.0f / game.cam.zoom
            );
            break;
            
        }
        }
    }
    
    //Leader silhouette.
    if(leaderSilhouetteVisible) {
        float x_offset = 32.0f;
        
        drawBitmap(
            game.sysContent.bmpLeaderSilhouetteTop, Point(x_offset, 0),
            Point(-1, game.config.leaders.standardRadius * 2.0f),
            0, al_map_rgba(240, 240, 240, 160)
        );
    }
    
    //Finish up.
    al_reset_clipping_rectangle();
    al_use_transform(&game.identityTransform);
}
