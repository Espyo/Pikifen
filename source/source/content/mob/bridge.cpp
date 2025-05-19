/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include <algorithm>

#include "bridge.h"

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


namespace BRIDGE {

//Width of the bridge's main floor, i.e., sans rails.
const float FLOOR_WIDTH = 192.0f;

//How far apart bridge steps are, vertically.
const float STEP_HEIGHT = 10;

}


/**
 * @brief Constructs a new bridge object.
 *
 * @param pos Starting coordinates.
 * @param type Bridge type this mob belongs to.
 * @param angle Starting angle.
 */
Bridge::Bridge(const Point &pos, BridgeType* type, float angle) :
    Mob(pos, type, angle),
    startPos(pos),
    briType(type) {
    
    team = MOB_TEAM_OBSTACLE;
    
    startZ = z;
}


/**
 * @brief Checks the bridge's health, and updates the chunks if necessary.
 *
 * @return Whether new chunks were created.
 */
bool Bridge::checkHealth() {
    //Figure out how many chunks should exist based on the bridge's completion.
    float completion = 1.0f - std::clamp(health / maxHealth, 0.0f, 1.0f);
    size_t expectedChunks = floor(totalChunksNeeded * completion);
    
    if(chunks >= expectedChunks) {
        //Nothing to do here.
        return false;
    }
    
    MobCategory* customCategory =
        game.mobCategories.get(MOB_CATEGORY_CUSTOM);
    MobType* bridgeComponentType =
        customCategory->getType("bridge_component");
    float chunkWidth = totalLength / totalChunksNeeded;
    vector<Mob*> newMobs;
    
    //Start creating all the necessary chunks.
    while(chunks < expectedChunks) {
        float xOffset = chunkWidth / 2.0 + chunkWidth * chunks;
        
        //Find the Z that this chunk should be at.
        float zOffset;
        if(chunks == totalChunksNeeded - 1) {
            zOffset = deltaZ;
        } else {
            size_t stepsNeeded =
                ceil(fabs(deltaZ) / BRIDGE::STEP_HEIGHT) + 1;
            float curCompletion =
                chunks / (float) totalChunksNeeded;
            size_t stepIdx =
                curCompletion * stepsNeeded;
            zOffset =
                stepIdx * BRIDGE::STEP_HEIGHT * sign(deltaZ);
        }
        
        if(zOffset == prevChunkZOffset) {
        
            //Just expand the existing components!
            float oldComponentWidth = chunkWidth * prevChunkCombo;
            prevChunkCombo++;
            float newComponentWidth = chunkWidth * prevChunkCombo;
            Point offset =
                rotatePoint(
                    Point(
                        (newComponentWidth - oldComponentWidth) / 2.0f,
                        0.0f
                    ),
                    angle
                );
                
            for(size_t m = 0; m < prevChunkComponents.size(); m++) {
                prevChunkComponents[m]->pos +=
                    offset;
                prevChunkComponents[m]->setRectangularDim(
                    Point(
                        newComponentWidth,
                        prevChunkComponents[m]->rectangularDim.y
                    )
                );
            }
            
        } else {
        
            //Create new components. First, the floor component.
            Point offset(xOffset, 0.0f);
            offset = rotatePoint(offset, angle);
            Mob* floorComponent =
                createMob(
                    customCategory,
                    startPos + offset,
                    bridgeComponentType,
                    angle,
                    "side=center; offset=" + f2s(xOffset - chunkWidth / 2.0f)
                );
            if(!floorComponent->centerSector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            floorComponent->z = startZ + zOffset;
            floorComponent->setRectangularDim(
                Point(chunkWidth, BRIDGE::FLOOR_WIDTH)
            );
            newMobs.push_back(floorComponent);
            
            //Then, the left rail component.
            offset.x = xOffset;
            offset.y =
                -BRIDGE::FLOOR_WIDTH / 2.0f - briType->railWidth / 2.0f;
            offset = rotatePoint(offset, angle);
            Mob* leftRailComponent =
                createMob(
                    customCategory,
                    startPos + offset,
                    bridgeComponentType,
                    angle,
                    "side=left; offset=" + f2s(xOffset - chunkWidth / 2.0f)
                );
            if(!leftRailComponent->centerSector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            leftRailComponent->z = startZ + zOffset;
            leftRailComponent->setRectangularDim(
                Point(
                    floorComponent->rectangularDim.x,
                    briType->railWidth
                )
            );
            leftRailComponent->height += GEOMETRY::STEP_HEIGHT * 2.0 + 1.0f;
            newMobs.push_back(leftRailComponent);
            
            //Finally, the right rail component.
            offset.x = xOffset;
            offset.y = BRIDGE::FLOOR_WIDTH / 2.0f + briType->railWidth / 2.0f;
            offset = rotatePoint(offset, angle);
            Mob* rightRailComponent =
                createMob(
                    customCategory,
                    startPos + offset,
                    bridgeComponentType,
                    angle,
                    "side=right; offset=" + f2s(xOffset - chunkWidth / 2.0f)
                );
            if(!rightRailComponent->centerSector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            rightRailComponent->z = startZ + zOffset;
            rightRailComponent->setRectangularDim(
                leftRailComponent->rectangularDim
            );
            rightRailComponent->height = leftRailComponent->height;
            newMobs.push_back(rightRailComponent);
            
            prevChunkZOffset = zOffset;
            prevChunkComponents.clear();
            prevChunkComponents.push_back(floorComponent);
            prevChunkComponents.push_back(leftRailComponent);
            prevChunkComponents.push_back(rightRailComponent);
            prevChunkCombo = 1;
            
        }
        
        chunks++;
        
    }
    
    //Finish setting up the new component mobs.
    for(size_t m = 0; m < newMobs.size(); m++) {
        Mob* mPtr = newMobs[m];
        enableFlag(mPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
        mPtr->push_anonymous_link(this);
    }
    
    //Move the bridge object proper to the farthest point of the bridge.
    float mobRadius =
        rectangularDim.x != 0.0f ?
        rectangularDim.x / 2.0f :
        radius;
    Point offset(chunkWidth * chunks - mobRadius, 0);
    offset = rotatePoint(offset, angle);
    pos = startPos + offset;
    z = startZ + prevChunkComponents[0]->z;
    groundSector = prevChunkComponents[0]->groundSector;
    
    return true;
}


/**
 * @brief Draws a bridge component, making sure to follow the right dimensions.
 *
 * @param m Bridge component mob.
 */
void Bridge::drawComponent(Mob* m) {
    if(m->link_anon_size==0 || !m->links["0"]) return;
    
    BitmapEffect eff;
    m->getSpriteBitmapEffects(
        nullptr, nullptr, 0.0f, &eff,
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS
    );
    
    Bridge* briPtr = (Bridge*) m->links["0"];
    string side = m->vars["side"];
    ALLEGRO_BITMAP* texture =
        side == "left" ?
        briPtr->briType->bmpLeftRailTexture :
        side == "right" ?
        briPtr->briType->bmpRightRailTexture :
        briPtr->briType->bmpMainTexture;
    int textureH = al_get_bitmap_height(texture);
    int textureV0 = textureH / 2.0f - m->rectangularDim.y / 2.0f;
    float textureOffset = s2f(m->vars["offset"]);
    
    ALLEGRO_TRANSFORM angleTransform;
    al_identity_transform(&angleTransform);
    al_rotate_transform(&angleTransform, m->angle);
    
    ALLEGRO_VERTEX vertexes[8];
    for(size_t v = 0; v < 8; v++) {
        vertexes[v].color = eff.tintColor;
        vertexes[v].z = 0.0f;
    }
    
    vertexes[0].color = mapGray(100);
    vertexes[0].x = m->rectangularDim.x / 2.0f;
    vertexes[0].y = -m->rectangularDim.y / 2.0f;
    vertexes[0].u = textureOffset + m->rectangularDim.x;
    vertexes[0].v = textureV0;
    
    vertexes[1].color = mapGray(100);
    vertexes[1].x = -m->rectangularDim.x / 2.0f;
    vertexes[1].y = -m->rectangularDim.y / 2.0f;
    vertexes[1].u = textureOffset;
    vertexes[1].v = textureV0;
    
    vertexes[2].x = vertexes[0].x;
    vertexes[2].y = -0.5f * m->rectangularDim.y / 2.0f;
    vertexes[2].u = textureOffset + m->rectangularDim.x;
    vertexes[2].v = textureV0 + 0.25f * m->rectangularDim.y;
    
    vertexes[3].x = vertexes[1].x;
    vertexes[3].y = -0.5f * m->rectangularDim.y / 2.0f;
    vertexes[3].u = textureOffset;
    vertexes[3].v = textureV0 + 0.25f * m->rectangularDim.y;
    
    vertexes[4].x = vertexes[0].x;
    vertexes[4].y = 0.5f * m->rectangularDim.y / 2.0f;
    vertexes[4].u = textureOffset + m->rectangularDim.x;
    vertexes[4].v = textureV0 + 0.75f * m->rectangularDim.y;
    
    vertexes[5].x = vertexes[1].x;
    vertexes[5].y = 0.5f * m->rectangularDim.y / 2.0f;
    vertexes[5].u = textureOffset;
    vertexes[5].v = textureV0 + 0.75f * m->rectangularDim.y;
    
    vertexes[6].color = mapGray(100);
    vertexes[6].x = vertexes[0].x;
    vertexes[6].y = m->rectangularDim.y / 2.0f;
    vertexes[6].u = textureOffset + m->rectangularDim.x;
    vertexes[6].v = textureV0 + m->rectangularDim.y;
    
    vertexes[7].color = mapGray(100);
    vertexes[7].x = vertexes[1].x;
    vertexes[7].y = m->rectangularDim.y / 2.0f;
    vertexes[7].u = textureOffset;
    vertexes[7].v = textureV0 + m->rectangularDim.y;
    
    for(size_t v = 0; v < 8; v++) {
        al_transform_coordinates(
            &angleTransform, &vertexes[v].x, &vertexes[v].y
        );
        vertexes[v].x += m->pos.x;
        vertexes[v].y += m->pos.y;
    }
    
    al_draw_prim(vertexes, nullptr, texture, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP);
}


/**
 * @brief Returns the starting point of the bridge.
 *
 * @return The point.
 */
Point Bridge::getStartPoint() {
    return startPos;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Bridge::readScriptVars(const ScriptVarReader &svr) {
    Mob::readScriptVars(svr);
    
    svr.get("chunks", totalChunksNeeded);
}


/**
 * @brief Sets up the bridge with the data surrounding it,
 * like its linked destination object.
 */
void Bridge::setup() {
    if(!link_anon_size == 0 && links["0"]) {
        totalLength = Distance(pos, links["0"]->pos).toFloat();
        face(getAngle(pos, links["0"]->pos), nullptr, true);
        deltaZ = links["0"]->z - z;
        totalChunksNeeded =
            std::max(
                totalChunksNeeded,
                (size_t) (ceil(fabs(deltaZ) / BRIDGE::STEP_HEIGHT) + 1)
            );
    }
    
    checkHealth();
}
