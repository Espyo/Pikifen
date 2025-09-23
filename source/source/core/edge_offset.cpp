/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Functions related to offseting edges, for the purposes of creating
 * wall shadow, ledge smoothing, or water foam effects.
 * These effects are obtained by drawing some geometry that follows
 * neighboring edges around, but is offset forward (or backward).
 * Some calculations and trickery are needed when neighboring edges meet at
 * acute or obtuse angles.
 */

#include <allegro5/allegro_image.h>

#include "../content/area/sector.h"
#include "../util/allegro_utils.h"
#include "../util/drawing_utils.h"
#include "drawing.h"
#include "game.h"
#include "misc_functions.h"


/**
 * @brief Draws an edge offset effect of a given edge onto the current
 * target bitmap, which acts as a buffer.
 *
 * @param caches List of caches to fetch edge info from.
 * @param eIdx Index of the edge whose effects to draw.
 * @param view Viewport to draw to.
 */
void drawEdgeOffsetOnBuffer(
    const vector<EdgeOffsetCache>& caches, size_t eIdx,
    const Viewport& view
) {
    //Keep the end opacity as a constant. Changing it helps with debugging.
    const float END_OPACITY = 0.0f;
    Edge* ePtr = game.curAreaData->edges[eIdx];
    
    //End vertexes. Like in updateOffsetEffectCaches, order is important.
    Vertex* endVertexes[2];
    //Relative coordinates of the tip of the rectangle, for each end vertex.
    Point endRelCoords[2];
    //Number of elbow triangles to use, for each end vertex.
    unsigned char nElbowTris[2] = {0, 0};
    //Relative coords of the elbow points, for each end vertex, each tri.
    Point elbowRelCoords[2][2];
    //Color of the effect, for each end vertex.
    ALLEGRO_COLOR endColors[2];
    
    if(caches[eIdx].firstEndVertexIdx == 0) {
        endVertexes[0] = ePtr->vertexes[0];
        endVertexes[1] = ePtr->vertexes[1];
    } else {
        endVertexes[0] = ePtr->vertexes[1];
        endVertexes[1] = ePtr->vertexes[0];
    }
    
    for(unsigned char end = 0; end < 2; end++) {
        //For each end of the effect...
        
        float length = caches[eIdx].lengths[end];
        if(length == 0.0f) continue;
        
        float angle = caches[eIdx].angles[end];
        float elbowLength = caches[eIdx].elbowLengths[end];
        float elbowAngle = caches[eIdx].elbowAngles[end];
        endColors[end] = caches[eIdx].colors[end];
        
        //This end of the effect starts at the vertex,
        //and spreads to this point.
        endRelCoords[end] = rotatePoint(Point(length, 0), angle);
        
        if(elbowLength > 0.0f) {
            //We need to also draw an elbow connecting this end of the
            //effect to something else. Usually another effect's elbow, but
            //it could just be another effect's edge.
            //The elbow is either one triangle or two triangles, depending
            //on how much it needs to bend.
            
            float rectToElbowDiff =
                end == 0 ?
                getAngleCwDiff(elbowAngle, angle) :
                getAngleCwDiff(angle, elbowAngle);
                
            if(rectToElbowDiff > TAU / 8.00001f) {
                //We add a small amount to the threshold because of floating
                //point imperfections. A perfectly square sector
                //(easy to do in the editor) may result in elbows where
                //one side gets one triangle, and the other gets two.
                //At least this small bump in the angle threshold makes it
                //much less likely to happen.
                nElbowTris[end] = 2;
                float midElbowAngle =
                    end == 0 ?
                    angle - rectToElbowDiff / 2.0f :
                    angle + rectToElbowDiff / 2.0f;
                elbowRelCoords[end][0] =
                    rotatePoint(Point(elbowLength, 0), midElbowAngle);
            } else {
                nElbowTris[end] = 1;
            }
            
            elbowRelCoords[end][nElbowTris[end] - 1] =
                rotatePoint(Point(elbowLength, 0), elbowAngle);
        }
        
    }
    
    //Start setting up the vertexes for the drawing process. These do not
    //take into account the elbow, and are just the standard "rectangle".
    ALLEGRO_VERTEX av[4];
    for(size_t e = 0; e < 2; e++) {
        av[e].x = endVertexes[e]->x;
        av[e].y = endVertexes[e]->y;
        av[e].color = endColors[e];
        av[e].z = 0;
    }
    
    av[2].x = endRelCoords[1].x + av[1].x;
    av[2].y = endRelCoords[1].y + av[1].y;
    av[2].color = endColors[1];
    av[2].color.a = END_OPACITY;
    av[2].z = 0;
    av[3].x = endRelCoords[0].x + av[0].x;
    av[3].y = endRelCoords[0].y + av[0].y;
    av[3].color = endColors[0];
    av[3].color.a = END_OPACITY;
    av[3].z = 0;
    
    //Let's transform the "rectangle" coordinates for the buffer.
    for(unsigned char v = 0; v < 4; v++) {
        al_transform_coordinates(
            &view.worldToWindowTransform, &av[v].x, &av[v].y
        );
    }
    
    //Draw the "rectangle"!
    al_draw_prim(av, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
    
    if(nElbowTris[0] == 0 && nElbowTris[1] == 0) {
        //If we don't need to draw elbows, we can quit now.
        return;
    }
    
    //Now for the elbows's vertexes. For each side, we have:
    //0: the edge vertex,
    //1: the tip of the "rectangle",
    //2: the first elbow triangle,
    //3: the second elbow triangle (if any).
    ALLEGRO_VERTEX elbowAV[2][4];
    elbowAV[0][0] = av[0];
    elbowAV[0][1] = av[3];
    elbowAV[1][0] = av[1];
    elbowAV[1][1] = av[2];
    
    for(unsigned char e = 0; e < 2; e++) {
        for(unsigned char v = 0; v < nElbowTris[e]; v++) {
            elbowAV[e][v + 2].x =
                endVertexes[e]->x + elbowRelCoords[e][v].x;
            elbowAV[e][v + 2].y =
                endVertexes[e]->y + elbowRelCoords[e][v].y;
            elbowAV[e][v + 2].z = 0.0f;
            elbowAV[e][v + 2].color = endColors[e];
            elbowAV[e][v + 2].color.a = END_OPACITY;
            al_transform_coordinates(
                &view.worldToWindowTransform,
                &elbowAV[e][v + 2].x, &elbowAV[e][v + 2].y
            );
        }
    }
    
    //Draw the elbows!
    for(unsigned char e = 0; e < 2; e++) {
        if(nElbowTris[e] == 0) continue;
        al_draw_prim(
            elbowAV[e], nullptr, nullptr, 0,
            nElbowTris[e] + 2,
            ALLEGRO_PRIM_TRIANGLE_FAN
        );
    }
}


/**
 * @brief Draws edge offset effects onto the given sector. This requires that
 * the effects have been drawn onto a buffer, from which this algorithm samples.
 *
 * @param sPtr Sector to draw the effects of.
 * @param buffer Buffer to draw from.
 * @param opacity Draw at this opacity, 0 - 1.
 * @param view Viewport to draw to.
 */
void drawSectorEdgeOffsets(
    Sector* sPtr, ALLEGRO_BITMAP* buffer, float opacity,
    const Viewport& view
) {
    if(sPtr->isBottomlessPit) return;
    
    size_t nVertexes = sPtr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[nVertexes];
    
    for(size_t v = 0; v < nVertexes; v++) {
        const Triangle* tPtr = &sPtr->triangles[floor(v / 3.0)];
        Vertex* vPtr = tPtr->points[v % 3];
        float vx = vPtr->x;
        float vy = vPtr->y;
        
        av[v].x = vx;
        av[v].y = vy;
        al_transform_coordinates(
            &view.worldToWindowTransform, &vx, &vy
        );
        av[v].u = vx;
        av[v].v = vy;
        av[v].z = 0;
        av[v].color.r = 1.0f;
        av[v].color.g = 1.0f;
        av[v].color.b = 1.0f;
        av[v].color.a = opacity;
    }
    
    al_draw_prim(
        av, nullptr, buffer,
        0, (int) nVertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
    
    delete[] av;
}


/**
 * @brief Returns information about one of the ends of an edge offset effect.
 *
 * @param ePtr Edge with the effect.
 * @param endVertex Vertex of the end being processed.
 * @param endIdx Index of the end being processed. 0 is the end of the
 * edge where the sector receiving the effect is to the left, if you face
 * from end 0 to end 1.
 * @param edgeProcessAngle Angle that the edge makes from the current end
 * to the opposite one.
 * @param checker Pointer to a function that checks if the edge should have the
 * intended effect or not. It also returns what sector of the edge
 * will be affected by the effect, and which won't.
 * @param lengthGetter Function that returns the length of the effect.
 * @param colorGetter Function that returns the color of the effect.
 * @param outAngle The angle of the tip of this end of the effect's
 * "rectangle" is returned here.
 * @param outLength The length of the tip of this end of the effect's
 * "rectangle" is returned here.
 * @param outColor The color at this end of the effect's "rectangle" is
 * returned here.
 * @param outElbowAngle The angle that the elbow must finish at is
 * returned here. 0 if no elbow is needed.
 * @param outElbowLength The length of the line at the end of the elbow
 * is returned here. 0 if no elbow is needed.
 */
void getEdgeOffsetEdgeInfo(
    Edge* ePtr, Vertex* endVertex, unsigned char endIdx,
    float edgeProcessAngle,
    OffsetEffectChecker checker,
    OffsetEffectLengthGetter lengthGetter,
    OffsetEffectColorGetter colorGetter,
    float* outAngle, float* outLength, ALLEGRO_COLOR* outColor,
    float* outElbowAngle, float* outElbowLength
) {
    *outElbowAngle = 0.0f;
    *outElbowLength = 0.0f;
    *outColor = colorGetter(ePtr);
    
    float baseEffectLength = lengthGetter(ePtr);
    float baseEffectAngle =
        endIdx == 0 ?
        edgeProcessAngle - TAU / 4.0f :
        edgeProcessAngle + TAU / 4.0f;
    baseEffectAngle = normalizeAngle(baseEffectAngle);
    bool edgeEffectCw = endIdx == 1;
    
    Edge* nextEdge = nullptr;
    float nextEdgeAngle = 0.0f;
    float nextEdgeDiff = 0.0f;
    Edge* nextEffEdge = nullptr;
    float nextEffEdgeAngle = 0.0f;
    float nextEffEdgeDiff = 0.0f;
    bool nextEffEdgeEffectCw = false;
    float nextEffEdgeBaseEffectAngle = 0.0f;
    
    //Start by getting some information about the edges
    //that comes after this one.
    getNextEdge(
        endVertex, edgeProcessAngle, edgeEffectCw, ePtr,
        &nextEdge, &nextEdgeAngle, &nextEdgeDiff
    );
    
    getNextOffsetEffectEdge(
        endVertex, edgeProcessAngle, edgeEffectCw, ePtr,
        checker,
        &nextEffEdge, &nextEffEdgeAngle, &nextEffEdgeDiff,
        &nextEffEdgeBaseEffectAngle,
        &nextEffEdgeEffectCw
    );
    
    //Now either this end of the effect is drawn forward,
    //or it's slanted inward to merge with another effect.
    //In addition, we may need an elbow attached to this end or not.
    bool effectsNeedMerge =
        nextEffEdge && nextEffEdgeEffectCw != edgeEffectCw;
    if(
        effectsNeedMerge &&
        nextEffEdgeDiff < (TAU / 2.0f - 0.0001f)
    ) {
        //Next edge that casts an effect faces ours.
        //Merge our effect with its effect.
        //The effect's final point should be where they both intersect.
        //The other effect's edge will do the same when it's its turn.
        //The reason we're docking some values away from exactly 180 degrees
        //is because floating point imperfections may make 180-degree edges
        //attempt to be merged, and then the intersection algorithm fails.
        float nextEdgeBaseEffectLength = lengthGetter(nextEffEdge);
        float midEffectLength =
            (baseEffectLength + nextEdgeBaseEffectLength) / 2.0f;
            
        getEdgeOffsetIntersection(
            ePtr, nextEffEdge, endVertex,
            baseEffectAngle, nextEffEdgeBaseEffectAngle,
            midEffectLength,
            outAngle, outLength
        );
        
        *outColor =
            interpolateColor(
                0.5, 0, 1,
                *outColor,
                colorGetter(nextEffEdge)
            );
            
    } else if(
        nextEffEdge && nextEffEdgeEffectCw == edgeEffectCw &&
        nextEffEdgeDiff < TAU / 4.0f
    ) {
        //Next edge has an effect that goes in the same direction,
        //and that edge imposes over our effect.
        //As such, skew our effect inwards to align with that edge.
        *outAngle = nextEffEdgeAngle;
        *outLength = baseEffectLength / sin(nextEffEdgeDiff);
        
    } else if(
        !nextEffEdge
    ) {
        //There's nothing to connect to in any way, so we might as well shrink
        //this end. Shrinking it to 0 will make effects of edges where there's
        //nothing on both sides disappear, which may mislead the user. So
        //instead just make it a fraction of the usual size.
        *outAngle = baseEffectAngle;
        *outLength = baseEffectLength / 5.0f;
        
    } else {
        //We can draw our end of the effect forward without a care.
        *outAngle = baseEffectAngle;
        *outLength = baseEffectLength;
        
        if(nextEffEdgeEffectCw != edgeEffectCw) {
            //On this end there is a neighboring effect we'll want to connect
            //to. But because that neighboring effect is so far away in
            //terms of angle, we'll need to implement an elbow between them
            //so they can be connected. This edge will draw half of the elbow,
            //and the other will draw its half when it's its turn.
            float nextEdgeBaseEffectLength =
                lengthGetter(nextEffEdge);
            float midEffectLength =
                (baseEffectLength + nextEdgeBaseEffectLength) / 2.0f;
                
            *outLength = midEffectLength;
            *outElbowLength = midEffectLength;
            *outElbowAngle =
                endIdx == 0 ?
                nextEffEdgeAngle +
                getAngleCwDiff(
                    nextEffEdgeAngle, edgeProcessAngle
                ) / 2.0f :
                edgeProcessAngle +
                getAngleCwDiff(
                    edgeProcessAngle, nextEffEdgeAngle
                ) / 2.0f;
            *outColor =
                interpolateColor(
                    0.5, 0, 1,
                    *outColor,
                    colorGetter(nextEffEdge)
                );
                
        } else {
            //There is a neighboring edge that has the effect, but in
            //the same direction as ours. As such, our effect will have
            //to connect to that effect's edge so there's a snug fit.
            //But because that neighboring effect is so far away in terms of
            //angle, we'll need to implement an elbow between them so they
            //can be connected. This edge will be in charge of drawing the full
            //elbow.
            *outElbowAngle = nextEffEdgeAngle;
            *outElbowLength = baseEffectLength;
        }
        
    }
    
}


/**
 * @brief Returns the point in which the far end of two edge offset
 * effects intersect.
 *
 * This calculation is only for the base "rectangle" shape of the effect,
 * and doesn't take into account any inward slants given on the ends, nor
 * does it care about elbows.
 * Normally, this would be the intersection point between the line segments
 * that make up both effects's rectangle ends, but there may be cases, explained
 * below, where that doesn't result in a real collision. In order for the
 * algorithm to always return something that at least can be worked with,
 * the intersection is calculated as if both effect ends were infinitely long
 * lines.
 * We could use getMiterPoints for this, but it's surprisingly not much
 * faster, and this allows us to tackle some visually glitchy edge-cases.
 *
 * @param e1 First effect-casting edge. This is the main edge being processed.
 * @param e2 Second effect-casting edge.
 * @param commonVertex The vertex shared between these two edges.
 * @param baseEffectAngle1 The base angle at which edge 1's effect
 * is projected.
 * @param baseEffectAngle2 Same as baseEffectAngle1, but for edge 2.
 * @param effectLength Length of either effect.
 * @param outAngle The angle from the common vertex to the
 * intersection point is returned here.
 * @param outLength The length from the common vertex to the
 * intersection point is returned here.
 */
void getEdgeOffsetIntersection(
    const Edge* e1, const Edge* e2, const Vertex* commonVertex,
    float baseEffectAngle1, float baseEffectAngle2,
    float effectLength,
    float* outAngle, float* outLength
) {
    Vertex* otherVertex1 = e1->getOtherVertex(commonVertex);
    float baseCos1 = cos(baseEffectAngle1);
    float baseSin1 = sin(baseEffectAngle1);
    Point effect1P0(
        commonVertex->x + baseCos1 * effectLength,
        commonVertex->y + baseSin1 * effectLength
    );
    Point effect1P1(
        otherVertex1->x + baseCos1 * effectLength,
        otherVertex1->y + baseSin1 * effectLength
    );
    
    Vertex* otherVertex2 = e2->getOtherVertex(commonVertex);
    float baseCos2 = cos(baseEffectAngle2);
    float baseSin2 = sin(baseEffectAngle2);
    Point effect2P0(
        commonVertex->x + baseCos2 * effectLength,
        commonVertex->y + baseSin2 * effectLength
    );
    Point effect2P1(
        otherVertex2->x + baseCos2 * effectLength,
        otherVertex2->y + baseSin2 * effectLength
    );
    
    //Let's get where the lines intersect. We're checking the lines and
    //not line segments, since there could be cases where an edge is so short
    //that its base effect line starts and begins inside the other edge's
    //base effect rectangle. This may cause some visual artifacts like
    //triangles being drawn where they shouldn't, but for such a broken
    //scenario, it's an acceptable solution.
    float r;
    if(
        linesIntersect(
            effect1P0, effect1P1,
            effect2P0, effect2P1,
            &r, nullptr
        )
    ) {
        //Clamp r to prevent long, close edges from
        //creating jagged effects outside the edge.
        r = std::clamp(r, 0.0f, 1.0f);
        Point p(
            effect1P0.x + (effect1P1.x - effect1P0.x) * r,
            effect1P0.y + (effect1P1.y - effect1P0.y) * r
        );
        coordinatesToAngle(
            p - v2p(commonVertex),
            outAngle, outLength
        );
    } else {
        //Okay, they don't really intersect. This should never happen... Maybe
        //a floating point imperfection? Oh well, let's go for a failsafe.
        *outAngle = 0.0f;
        *outLength = 0.0f;
    }
}


/**
 * @brief Returns the next edge in a vertex's list of edges.
 * It checks in a given direction, starting from some pivot angle.
 *
 * @param vPtr Vertex to work on.
 * @param pivotAngle Angle to check from.
 * @param clockwise True to check in a clockwise direction,
 * false for counterclockwise.
 * @param ignore Edge to ignore while checking, if any.
 * @param outEdge The found edge is returned here, or nullptr.
 * @param outAngle The angle of the found edge is returned here.
 * @param outDiff The difference in angle between the two is returned here.
 */
void getNextEdge(
    Vertex* vPtr, float pivotAngle, bool clockwise,
    const Edge* ignore, Edge** outEdge, float* outAngle, float* outDiff
) {
    Edge* bestEdge = nullptr;
    float bestEdgeDiff = 0.0f;
    float bestEdgeAngle = 0.0f;
    
    for(size_t e = 0; e < vPtr->edges.size(); e++) {
        Edge* ePtr = vPtr->edges[e];
        
        if(ePtr == ignore) continue;
        
        unsigned char otherVertexIdx = ePtr->vertexes[0] == vPtr ? 1 : 0;
        Vertex* otherVertex = ePtr->vertexes[otherVertexIdx];
        
        float angle = getAngle(v2p(vPtr), v2p(otherVertex));
        
        float diff =
            clockwise ?
            getAngleCwDiff(pivotAngle, angle) :
            getAngleCwDiff(angle, pivotAngle);
            
        if(!bestEdge || diff < bestEdgeDiff) {
            bestEdge = ePtr;
            bestEdgeDiff = diff;
            bestEdgeAngle = angle;
        }
    }
    
    *outEdge = bestEdge;
    *outAngle = bestEdgeAngle;
    *outDiff = bestEdgeDiff;
}


/**
 * @brief Returns the next edge that needs the given edge offset event,
 * in a vertex's list of edges. It checks in a given direction,
 * starting from some pivot angle.
 *
 * @param vPtr Vertex to work on.
 * @param pivotAngle Angle to check from.
 * @param clockwise True to check in a clockwise direction,
 * false for counterclockwise.
 * @param ignore Edge to ignore while checking, if any.
 * @param edgeChecker Function that returns whether or not a given edge
 * should use the effect.
 * @param outEdge The found edge is returned here, or nullptr.
 * @param outAngle The angle of the found edge is returned here.
 * @param outDiff The difference in angle between the two is returned here.
 * @param outBaseEffectAngle The base effect angle of the found edge
 * is returned here.
 * @param outEffectCw Whether the effect is cast clockwise is returned here.
 */
void getNextOffsetEffectEdge(
    Vertex* vPtr, float pivotAngle, bool clockwise,
    const Edge* ignore, OffsetEffectChecker edgeChecker,
    Edge** outEdge, float* outAngle, float* outDiff,
    float* outBaseEffectAngle,
    bool* outEffectCw
) {
    Edge* bestEdge = nullptr;
    float bestEdgeDiff = 0;
    float bestEdgeAngle = 0;
    bool bestEdgeEffectCw = false;
    
    for(size_t e = 0; e < vPtr->edges.size(); e++) {
        Edge* ePtr = vPtr->edges[e];
        
        if(ePtr == ignore) continue;
        
        Sector* affectedSector;
        Sector* unaffectedSector;
        if(!edgeChecker(ePtr, &affectedSector, &unaffectedSector)) {
            //This edge does not use the effect.
            continue;
        }
        unsigned char unaffectedSectorIdx =
            ePtr->sectors[0] == unaffectedSector ? 0 : 1;
            
        unsigned char otherVertexIdx = ePtr->vertexes[0] == vPtr ? 1 : 0;
        Vertex* otherVertex = ePtr->vertexes[otherVertexIdx];
        
        //Standing on the common vertex, facing the edge,
        //to what side does the effect go?
        bool effectIsCw = otherVertexIdx != unaffectedSectorIdx;
        
        float angle = getAngle(v2p(vPtr), v2p(otherVertex));
        
        float diff =
            clockwise ?
            getAngleCwDiff(pivotAngle, angle) :
            getAngleCwDiff(angle, pivotAngle);
            
        if(!bestEdge || diff < bestEdgeDiff) {
            bestEdge = ePtr;
            bestEdgeDiff = diff;
            bestEdgeAngle = angle;
            bestEdgeEffectCw = effectIsCw;
        }
    }
    
    *outEdge = bestEdge;
    *outDiff = bestEdgeDiff;
    *outAngle = bestEdgeAngle;
    *outEffectCw = bestEdgeEffectCw;
    if(bestEdgeEffectCw) {
        *outBaseEffectAngle =
            normalizeAngle(bestEdgeAngle + TAU / 4.0f);
    } else {
        *outBaseEffectAngle =
            normalizeAngle(bestEdgeAngle - TAU / 4.0f);
    }
}


/**
 * @brief Draws edge offset effects for all edges visible in the game window
 * onto a buffer image, so that sectors may then sample from it to draw
 * what effects they need.
 *
 * @param camTL Top-left corner of the camera boundaries.
 * The edges of any sector that is beyond these boundaries will be ignored.
 * @param camBR Same as camTL, but for the bottom-right boundaries.
 * @param caches List of caches to fetch edge info from.
 * @param buffer Buffer to draw to.
 * @param clearFirst If true, the bitmap is cleared before any drawing is done.
 * @param view Viewport to draw to.
 */
void updateOffsetEffectBuffer(
    const Point& camTL, const Point& camBR,
    const vector<EdgeOffsetCache>& caches, ALLEGRO_BITMAP* buffer,
    bool clearFirst, const Viewport& view
) {
    unordered_set<size_t> edges;
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* sPtr = game.curAreaData->sectors[s];
        
        if(
            !rectanglesIntersect(
                sPtr->bbox[0], sPtr->bbox[1],
                camTL, camBR
            )
        ) {
            //Sector is off-camera.
            continue;
        }
        
        bool fullyOnCamera = false;
        if(
            sPtr->bbox[0].x > camTL.x &&
            sPtr->bbox[1].x < camBR.x &&
            sPtr->bbox[0].y > camTL.y &&
            sPtr->bbox[1].y < camBR.y
        ) {
            fullyOnCamera = true;
        }
        
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
            if(!fullyOnCamera) {
                //If the sector's fully on-camera, it's faster to not bother
                //with the edge-by-edge check.
                Point edgeTL = v2p(sPtr->edges[e]->vertexes[0]);
                Point edgeBR = edgeTL;
                updateMinMaxCoords(
                    edgeTL, edgeBR,
                    v2p(sPtr->edges[e]->vertexes[1])
                );
                
                if(!rectanglesIntersect(edgeTL, edgeBR, camTL, camBR)) {
                    //Edge is off-camera.
                    continue;
                }
            }
            
            edges.insert(sPtr->edgeIdxs[e]);
        }
    }
    
    //Save the current state of some things.
    ALLEGRO_BITMAP* targetBmp = al_get_target_bitmap();
    int oldOp, oldSrc, oldDst, oldAop, oldAsrc, oldAdst;
    al_get_separate_blender(
        &oldOp, &oldSrc, &oldDst, &oldAop, &oldAsrc, &oldAdst
    );
    
    //Set the new operation modes.
    al_set_target_bitmap(buffer);
    al_set_separate_blender(
        ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO,
        ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA
    );
    al_hold_bitmap_drawing(true);
    
    //Draw!
    if(clearFirst) {
        al_clear_to_color(COLOR_EMPTY);
    }
    
    for(size_t eIdx : edges) {
        drawEdgeOffsetOnBuffer(caches, eIdx, view);
    }
    
    //Return to the old state of things.
    al_hold_bitmap_drawing(false);
    al_set_separate_blender(
        oldOp, oldSrc, oldDst, oldAop, oldAsrc, oldAdst
    );
    al_set_target_bitmap(targetBmp);
}


/**
 * @brief Updates the cached information about all edge offsets.
 *
 * @param caches List of caches to update.
 * @param vertexesToUpdate List of vertexes whose edges need updating.
 * @param checker Pointer to a function that checks if the edge should have the
 * intended effect or not. It also returns what sector of the edge
 *  will be affected by the effect, and which won't.
 * @param lengthGetter Function that returns the length of the effect.
 * @param colorGetter Function that returns the color of the effect.
 */
void updateOffsetEffectCaches (
    vector<EdgeOffsetCache>& caches,
    const unordered_set<Vertex*>& vertexesToUpdate,
    OffsetEffectChecker checker,
    OffsetEffectLengthGetter lengthGetter,
    OffsetEffectColorGetter colorGetter
) {
    unordered_set<size_t> edgesToUpdate;
    for(Vertex* v : vertexesToUpdate) {
        edgesToUpdate.insert(v->edgeIdxs.begin(), v->edgeIdxs.end());
    }
    
    for(size_t e : edgesToUpdate) {
        Edge* ePtr = game.curAreaData->edges[e];
        
        Sector* unaffectedSector = nullptr;
        Sector* affectedSector = nullptr;
        
        if(!checker(ePtr, &affectedSector, &unaffectedSector)) {
            //This edge doesn't get the effect.
            caches[e].lengths[0] = 0.0f;
            caches[e].lengths[1] = 0.0f;
            continue;
        }
        
        //We need to process the two vertexes of the edge in a specific
        //order, such that if you stand on the first one being processed,
        //and you face the second one, the affected sector is to the left.
        
        Vertex* endsToProcess[2];
        if(ePtr->sectors[0] == affectedSector) {
            endsToProcess[0] = ePtr->vertexes[0];
            endsToProcess[1] = ePtr->vertexes[1];
            caches[e].firstEndVertexIdx = 0;
        } else {
            endsToProcess[0] = ePtr->vertexes[1];
            endsToProcess[1] = ePtr->vertexes[0];
            caches[e].firstEndVertexIdx = 1;
        }
        float edgeProcessAngle =
            getAngle(v2p(endsToProcess[0]), v2p(endsToProcess[1]));
            
        for(unsigned char end = 0; end < 2; end++) {
            //For each end of the effect...
            
            float length = 0.0f;
            float angle = 0.0f;
            float elbowLength = 0.0f;
            float elbowAngle = 0.0f;
            ALLEGRO_COLOR endColor;
            
            //The edge's effect is simply a rectangle, although one or both
            //of its ends could be angled inward, either to merge with a
            //neighboring effect or to fit snugly against a different
            //effect's edge.
            //In addition, we may also need to draw an "elbow" shape to
            //connect to a different edge.
            //Start by getting information on how this effect should behave.
            //We don't need to worry about why it's drawn the way it is, since
            //getEdgeOffsetEdgeInfo is in charge of that.
            getEdgeOffsetEdgeInfo(
                ePtr, endsToProcess[end], end,
                end == 0 ? edgeProcessAngle : edgeProcessAngle + TAU / 2.0f,
                checker, lengthGetter, colorGetter,
                &angle, &length, &endColor,
                &elbowAngle, &elbowLength
            );
            
            caches[e].lengths[end] = length;
            caches[e].angles[end] = normalizeAngle(angle);
            caches[e].colors[end] = endColor;
            caches[e].elbowAngles[end] = normalizeAngle(elbowAngle);
            caches[e].elbowLengths[end] = elbowLength;
        }
    }
}
