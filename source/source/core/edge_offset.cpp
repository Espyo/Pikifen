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
 * @param e_idx Index of the edge whose effects to draw.
 */
void drawEdgeOffsetOnBuffer(
    const vector<EdgeOffsetCache> &caches, size_t e_idx
) {
    //Keep the end opacity as a constant. Changing it helps with debugging.
    const float END_OPACITY = 0.0f;
    Edge* e_ptr = game.curAreaData->edges[e_idx];
    
    //End vertexes. Like in updateOffsetEffectCaches, order is important.
    Vertex* end_vertexes[2];
    //Relative coordinates of the tip of the rectangle, for each end vertex.
    Point end_rel_coords[2];
    //Number of elbow triangles to use, for each end vertex.
    unsigned char n_elbow_tris[2] = {0, 0};
    //Relative coords of the elbow points, for each end vertex, each tri.
    Point elbow_rel_coords[2][2];
    //Color of the effect, for each end vertex.
    ALLEGRO_COLOR end_colors[2];
    
    if(caches[e_idx].firstEndVertexIdx == 0) {
        end_vertexes[0] = e_ptr->vertexes[0];
        end_vertexes[1] = e_ptr->vertexes[1];
    } else {
        end_vertexes[0] = e_ptr->vertexes[1];
        end_vertexes[1] = e_ptr->vertexes[0];
    }
    
    for(unsigned char end = 0; end < 2; end++) {
        //For each end of the effect...
        
        float length = caches[e_idx].lengths[end];
        if(length == 0.0f) continue;
        
        float angle = caches[e_idx].angles[end];
        float elbow_length = caches[e_idx].elbowLengths[end];
        float elbow_angle = caches[e_idx].elbowAngles[end];
        end_colors[end] = caches[e_idx].colors[end];
        
        //This end of the effect starts at the vertex,
        //and spreads to this point.
        end_rel_coords[end] = rotatePoint(Point(length, 0), angle);
        
        if(elbow_length > 0.0f) {
            //We need to also draw an elbow connecting this end of the
            //effect to something else. Usually another effect's elbow, but
            //it could just be another effect's edge.
            //The elbow is either one triangle or two triangles, depending
            //on how much it needs to bend.
            
            float rect_to_elbow_diff =
                end == 0 ?
                getAngleCwDiff(elbow_angle, angle) :
                getAngleCwDiff(angle, elbow_angle);
                
            if(rect_to_elbow_diff > TAU / 8.00001f) {
                //We add a small amount to the threshold because of floating
                //point imperfections. A perfectly square sector
                //(easy to do in the editor) may result in elbows where
                //one side gets one triangle, and the other gets two.
                //At least this small bump in the angle threshold makes it
                //much less likely to happen.
                n_elbow_tris[end] = 2;
                float mid_elbow_angle =
                    end == 0 ?
                    angle - rect_to_elbow_diff / 2.0f :
                    angle + rect_to_elbow_diff / 2.0f;
                elbow_rel_coords[end][0] =
                    rotatePoint(Point(elbow_length, 0), mid_elbow_angle);
            } else {
                n_elbow_tris[end] = 1;
            }
            
            elbow_rel_coords[end][n_elbow_tris[end] - 1] =
                rotatePoint(Point(elbow_length, 0), elbow_angle);
        }
        
    }
    
    //Start setting up the vertexes for the drawing process. These do not
    //take into account the elbow, and are just the standard "rectangle".
    ALLEGRO_VERTEX av[4];
    for(size_t e = 0; e < 2; e++) {
        av[e].x = end_vertexes[e]->x;
        av[e].y = end_vertexes[e]->y;
        av[e].color = end_colors[e];
        av[e].z = 0;
    }
    
    av[2].x = end_rel_coords[1].x + av[1].x;
    av[2].y = end_rel_coords[1].y + av[1].y;
    av[2].color = end_colors[1];
    av[2].color.a = END_OPACITY;
    av[2].z = 0;
    av[3].x = end_rel_coords[0].x + av[0].x;
    av[3].y = end_rel_coords[0].y + av[0].y;
    av[3].color = end_colors[0];
    av[3].color.a = END_OPACITY;
    av[3].z = 0;
    
    //Let's transform the "rectangle" coordinates for the buffer.
    for(unsigned char v = 0; v < 4; v++) {
        al_transform_coordinates(
            &game.worldToScreenTransform, &av[v].x, &av[v].y
        );
    }
    
    //Draw the "rectangle"!
    al_draw_prim(av, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
    
    if(n_elbow_tris[0] == 0 && n_elbow_tris[1] == 0) {
        //If we don't need to draw elbows, we can quit now.
        return;
    }
    
    //Now for the elbows's vertexes. For each side, we have:
    //0: the edge vertex,
    //1: the tip of the "rectangle",
    //2: the first elbow triangle,
    //3: the second elbow triangle (if any).
    ALLEGRO_VERTEX elbow_av[2][4];
    elbow_av[0][0] = av[0];
    elbow_av[0][1] = av[3];
    elbow_av[1][0] = av[1];
    elbow_av[1][1] = av[2];
    
    for(unsigned char e = 0; e < 2; e++) {
        for(unsigned char v = 0; v < n_elbow_tris[e]; v++) {
            elbow_av[e][v + 2].x =
                end_vertexes[e]->x + elbow_rel_coords[e][v].x;
            elbow_av[e][v + 2].y =
                end_vertexes[e]->y + elbow_rel_coords[e][v].y;
            elbow_av[e][v + 2].z = 0.0f;
            elbow_av[e][v + 2].color = end_colors[e];
            elbow_av[e][v + 2].color.a = END_OPACITY;
            al_transform_coordinates(
                &game.worldToScreenTransform,
                &elbow_av[e][v + 2].x, &elbow_av[e][v + 2].y
            );
        }
    }
    
    //Draw the elbows!
    for(unsigned char e = 0; e < 2; e++) {
        if(n_elbow_tris[e] == 0) continue;
        al_draw_prim(
            elbow_av[e], nullptr, nullptr, 0,
            n_elbow_tris[e] + 2,
            ALLEGRO_PRIM_TRIANGLE_FAN
        );
    }
}


/**
 * @brief Draws edge offset effects onto the given sector. This requires that
 * the effects have been drawn onto a buffer, from which this algorithm samples.
 *
 * @param s_ptr Sector to draw the effects of.
 * @param buffer Buffer to draw from.
 * @param opacity Draw at this opacity, 0 - 1.
 */
void drawSectorEdgeOffsets(
    Sector* s_ptr, ALLEGRO_BITMAP* buffer, float opacity
) {
    if(s_ptr->isBottomlessPit) return;
    
    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; v++) {
        const Triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
        Vertex* v_ptr = t_ptr->points[v % 3];
        float vx = v_ptr->x;
        float vy = v_ptr->y;
        
        av[v].x = vx;
        av[v].y = vy;
        al_transform_coordinates(
            &game.worldToScreenTransform, &vx, &vy
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
        0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
    
    delete[] av;
}


/**
 * @brief Returns information about one of the ends of an edge offset effect.
 *
 * @param e_ptr Edge with the effect.
 * @param end_vertex Vertex of the end being processed.
 * @param end_idx Index of the end being processed. 0 is the end of the
 * edge where the sector receiving the effect is to the left, if you face
 * from end 0 to end 1.
 * @param edge_process_angle Angle that the edge makes from the current end
 * to the opposite one.
 * @param checker Pointer to a function that checks if the edge should have the
 * intended effect or not. It also returns what sector of the edge
 * will be affected by the effect, and which won't.
 * @param length_getter Function that returns the length of the effect.
 * @param color_getter Function that returns the color of the effect.
 * @param out_angle The angle of the tip of this end of the effect's
 * "rectangle" is returned here.
 * @param out_length The length of the tip of this end of the effect's
 * "rectangle" is returned here.
 * @param out_color The color at this end of the effect's "rectangle" is
 * returned here.
 * @param out_elbow_angle The angle that the elbow must finish at is
 * returned here. 0 if no elbow is needed.
 * @param out_elbow_length The length of the line at the end of the elbow
 * is returned here. 0 if no elbow is needed.
 */
void getEdgeOffsetEdgeInfo(
    Edge* e_ptr, Vertex* end_vertex, unsigned char end_idx,
    float edge_process_angle,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter,
    float* out_angle, float* out_length, ALLEGRO_COLOR* out_color,
    float* out_elbow_angle, float* out_elbow_length
) {
    *out_elbow_angle = 0.0f;
    *out_elbow_length = 0.0f;
    *out_color = color_getter(e_ptr);
    
    float base_effect_length = length_getter(e_ptr);
    float base_effect_angle =
        end_idx == 0 ?
        edge_process_angle - TAU / 4.0f :
        edge_process_angle + TAU / 4.0f;
    base_effect_angle = normalizeAngle(base_effect_angle);
    bool edge_effect_cw = end_idx == 1;
    
    Edge* next_edge = nullptr;
    float next_edge_angle = 0.0f;
    float next_edge_diff = 0.0f;
    Edge* next_eff_edge = nullptr;
    float next_eff_edge_angle = 0.0f;
    float next_eff_edge_diff = 0.0f;
    bool next_eff_edge_effect_cw = false;
    float next_eff_edge_base_effect_angle = 0.0f;
    
    //Start by getting some information about the edges
    //that comes after this one.
    getNextEdge(
        end_vertex, edge_process_angle, edge_effect_cw, e_ptr,
        &next_edge, &next_edge_angle, &next_edge_diff
    );
    
    getNextOffsetEffectEdge(
        end_vertex, edge_process_angle, edge_effect_cw, e_ptr,
        checker,
        &next_eff_edge, &next_eff_edge_angle, &next_eff_edge_diff,
        &next_eff_edge_base_effect_angle,
        &next_eff_edge_effect_cw
    );
    
    //Now either this end of the effect is drawn forward,
    //or it's slanted inward to merge with another effect.
    //In addition, we may need an elbow attached to this end or not.
    bool effects_need_merge =
        next_eff_edge && next_eff_edge_effect_cw != edge_effect_cw;
    if(
        effects_need_merge &&
        next_eff_edge_diff < (TAU / 2.0f - 0.0001f)
    ) {
        //Next edge that casts an effect faces ours.
        //Merge our effect with its effect.
        //The effect's final point should be where they both intersect.
        //The other effect's edge will do the same when it's its turn.
        //The reason we're docking some values away from exactly 180 degrees
        //is because floating point imperfections may make 180-degree edges
        //attempt to be merged, and then the intersection algorithm fails.
        float next_edge_base_effect_length = length_getter(next_eff_edge);
        float mid_effect_length =
            (base_effect_length + next_edge_base_effect_length) / 2.0f;
            
        getEdgeOffsetIntersection(
            e_ptr, next_eff_edge, end_vertex,
            base_effect_angle, next_eff_edge_base_effect_angle,
            mid_effect_length,
            out_angle, out_length
        );
        
        *out_color =
            interpolateColor(
                0.5, 0, 1,
                *out_color,
                color_getter(next_eff_edge)
            );
            
    } else if(
        next_eff_edge && next_eff_edge_effect_cw == edge_effect_cw &&
        next_eff_edge_diff < TAU / 4.0f
    ) {
        //Next edge has an effect that goes in the same direction,
        //and that edge imposes over our effect.
        //As such, skew our effect inwards to align with that edge.
        *out_angle = next_eff_edge_angle;
        *out_length = base_effect_length / sin(next_eff_edge_diff);
        
    } else if(
        !next_eff_edge
    ) {
        //There's nothing to connect to in any way, so we might as well shrink
        //this end. Shrinking it to 0 will make effects of edges where there's
        //nothing on both sides disappear, which may mislead the user. So
        //instead just make it a fraction of the usual size.
        *out_angle = base_effect_angle;
        *out_length = base_effect_length / 5.0f;
        
    } else {
        //We can draw our end of the effect forward without a care.
        *out_angle = base_effect_angle;
        *out_length = base_effect_length;
        
        if(next_eff_edge_effect_cw != edge_effect_cw) {
            //On this end there is a neighboring effect we'll want to connect
            //to. But because that neighboring effect is so far away in
            //terms of angle, we'll need to implement an elbow between them
            //so they can be connected. This edge will draw half of the elbow,
            //and the other will draw its half when it's its turn.
            float next_edge_base_effect_length =
                length_getter(next_eff_edge);
            float mid_effect_length =
                (base_effect_length + next_edge_base_effect_length) / 2.0f;
                
            *out_length = mid_effect_length;
            *out_elbow_length = mid_effect_length;
            *out_elbow_angle =
                end_idx == 0 ?
                next_eff_edge_angle +
                getAngleCwDiff(
                    next_eff_edge_angle, edge_process_angle
                ) / 2.0f :
                edge_process_angle +
                getAngleCwDiff(
                    edge_process_angle, next_eff_edge_angle
                ) / 2.0f;
            *out_color =
                interpolateColor(
                    0.5, 0, 1,
                    *out_color,
                    color_getter(next_eff_edge)
                );
                
        } else {
            //There is a neighboring edge that has the effect, but in
            //the same direction as ours. As such, our effect will have
            //to connect to that effect's edge so there's a snug fit.
            //But because that neighboring effect is so far away in terms of
            //angle, we'll need to implement an elbow between them so they
            //can be connected. This edge will be in charge of drawing the full
            //elbow.
            *out_elbow_angle = next_eff_edge_angle;
            *out_elbow_length = base_effect_length;
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
 * @param common_vertex The vertex shared between these two edges.
 * @param base_effect_angle1 The base angle at which edge 1's effect
 * is projected.
 * @param base_effect_angle2 Same as base_effect_angle1, but for edge 2.
 * @param effect_length Length of either effect.
 * @param out_angle The angle from the common vertex to the
 * intersection point is returned here.
 * @param out_length The length from the common vertex to the
 * intersection point is returned here.
 */
void getEdgeOffsetIntersection(
    const Edge* e1, const Edge* e2, const Vertex* common_vertex,
    float base_effect_angle1, float base_effect_angle2,
    float effect_length,
    float* out_angle, float* out_length
) {
    Vertex* other_vertex1 = e1->getOtherVertex(common_vertex);
    float base_cos1 = cos(base_effect_angle1);
    float base_sin1 = sin(base_effect_angle1);
    Point effect1_p0(
        common_vertex->x + base_cos1 * effect_length,
        common_vertex->y + base_sin1 * effect_length
    );
    Point effect1_p1(
        other_vertex1->x + base_cos1 * effect_length,
        other_vertex1->y + base_sin1 * effect_length
    );
    
    Vertex* other_vertex2 = e2->getOtherVertex(common_vertex);
    float base_cos2 = cos(base_effect_angle2);
    float base_sin2 = sin(base_effect_angle2);
    Point effect2_p0(
        common_vertex->x + base_cos2 * effect_length,
        common_vertex->y + base_sin2 * effect_length
    );
    Point effect2_p1(
        other_vertex2->x + base_cos2 * effect_length,
        other_vertex2->y + base_sin2 * effect_length
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
            effect1_p0, effect1_p1,
            effect2_p0, effect2_p1,
            &r, nullptr
        )
    ) {
        //Clamp r to prevent long, close edges from
        //creating jagged effects outside the edge.
        r = std::clamp(r, 0.0f, 1.0f);
        Point p(
            effect1_p0.x + (effect1_p1.x - effect1_p0.x) * r,
            effect1_p0.y + (effect1_p1.y - effect1_p0.y) * r
        );
        coordinatesToAngle(
            p - v2p(common_vertex),
            out_angle, out_length
        );
    } else {
        //Okay, they don't really intersect. This should never happen... Maybe
        //a floating point imperfection? Oh well, let's go for a failsafe.
        *out_angle = 0.0f;
        *out_length = 0.0f;
    }
}


/**
 * @brief Returns the next edge in a vertex's list of edges.
 * It checks in a given direction, starting from some pivot angle.
 *
 * @param v_ptr Vertex to work on.
 * @param pivot_angle Angle to check from.
 * @param clockwise True to check in a clockwise direction,
 * false for counterclockwise.
 * @param ignore Edge to ignore while checking, if any.
 * @param out_edge The found edge is returned here, or nullptr.
 * @param out_angle The angle of the found edge is returned here.
 * @param out_diff The difference in angle between the two is returned here.
 */
void getNextEdge(
    Vertex* v_ptr, float pivot_angle, bool clockwise,
    const Edge* ignore, Edge** out_edge, float* out_angle, float* out_diff
) {
    Edge* best_edge = nullptr;
    float best_edge_diff = 0.0f;
    float best_edge_angle = 0.0f;
    
    for(size_t e = 0; e < v_ptr->edges.size(); e++) {
        Edge* e_ptr = v_ptr->edges[e];
        
        if(e_ptr == ignore) continue;
        
        unsigned char other_vertex_idx = e_ptr->vertexes[0] == v_ptr ? 1 : 0;
        Vertex* other_vertex = e_ptr->vertexes[other_vertex_idx];
        
        float angle = getAngle(v2p(v_ptr), v2p(other_vertex));
        
        float diff =
            clockwise ?
            getAngleCwDiff(pivot_angle, angle) :
            getAngleCwDiff(angle, pivot_angle);
            
        if(!best_edge || diff < best_edge_diff) {
            best_edge = e_ptr;
            best_edge_diff = diff;
            best_edge_angle = angle;
        }
    }
    
    *out_edge = best_edge;
    *out_angle = best_edge_angle;
    *out_diff = best_edge_diff;
}


/**
 * @brief Returns the next edge that needs the given edge offset event,
 * in a vertex's list of edges. It checks in a given direction,
 * starting from some pivot angle.
 *
 * @param v_ptr Vertex to work on.
 * @param pivot_angle Angle to check from.
 * @param clockwise True to check in a clockwise direction,
 * false for counterclockwise.
 * @param ignore Edge to ignore while checking, if any.
 * @param edge_checker Function that returns whether or not a given edge
 * should use the effect.
 * @param out_edge The found edge is returned here, or nullptr.
 * @param out_angle The angle of the found edge is returned here.
 * @param out_diff The difference in angle between the two is returned here.
 * @param out_base_effect_angle The base effect angle of the found edge
 * is returned here.
 * @param out_effect_cw Whether the effect is cast clockwise is returned here.
 */
void getNextOffsetEffectEdge(
    Vertex* v_ptr, float pivot_angle, bool clockwise,
    const Edge* ignore, offset_effect_checker_t edge_checker,
    Edge** out_edge, float* out_angle, float* out_diff,
    float* out_base_effect_angle,
    bool* out_effect_cw
) {
    Edge* best_edge = nullptr;
    float best_edge_diff = 0;
    float best_edge_angle = 0;
    bool best_edge_effect_cw = false;
    
    for(size_t e = 0; e < v_ptr->edges.size(); e++) {
        Edge* e_ptr = v_ptr->edges[e];
        
        if(e_ptr == ignore) continue;
        
        Sector* affected_sector;
        Sector* unaffected_sector;
        if(!edge_checker(e_ptr, &affected_sector, &unaffected_sector)) {
            //This edge does not use the effect.
            continue;
        }
        unsigned char unaffected_sector_idx =
            e_ptr->sectors[0] == unaffected_sector ? 0 : 1;
            
        unsigned char other_vertex_idx = e_ptr->vertexes[0] == v_ptr ? 1 : 0;
        Vertex* other_vertex = e_ptr->vertexes[other_vertex_idx];
        
        //Standing on the common vertex, facing the edge,
        //to what side does the effect go?
        bool effect_is_cw = other_vertex_idx != unaffected_sector_idx;
        
        float angle = getAngle(v2p(v_ptr), v2p(other_vertex));
        
        float diff =
            clockwise ?
            getAngleCwDiff(pivot_angle, angle) :
            getAngleCwDiff(angle, pivot_angle);
            
        if(!best_edge || diff < best_edge_diff) {
            best_edge = e_ptr;
            best_edge_diff = diff;
            best_edge_angle = angle;
            best_edge_effect_cw = effect_is_cw;
        }
    }
    
    *out_edge = best_edge;
    *out_diff = best_edge_diff;
    *out_angle = best_edge_angle;
    *out_effect_cw = best_edge_effect_cw;
    if(best_edge_effect_cw) {
        *out_base_effect_angle =
            normalizeAngle(best_edge_angle + TAU / 4.0f);
    } else {
        *out_base_effect_angle =
            normalizeAngle(best_edge_angle - TAU / 4.0f);
    }
}


/**
 * @brief Draws edge offset effects for all edges on-screen onto a buffer image,
 * so that sectors may then sample from it to draw what effects they need.
 *
 * @param cam_tl Top-left corner of the camera boundaries.
 * The edges of any sector that is beyond these boundaries will be ignored.
 * @param cam_br Same as cam_tl, but for the bottom-right boundaries.
 * @param caches List of caches to fetch edge info from.
 * @param buffer Buffer to draw to.
 * @param clear_first If true, the bitmap is cleared before any drawing is done.
 */
void updateOffsetEffectBuffer(
    const Point &cam_tl, const Point &cam_br,
    const vector<EdgeOffsetCache> &caches, ALLEGRO_BITMAP* buffer,
    bool clear_first
) {
    unordered_set<size_t> edges;
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        
        if(
            !rectanglesIntersect(
                s_ptr->bbox[0], s_ptr->bbox[1],
                cam_tl, cam_br
            )
        ) {
            //Sector is off-camera.
            continue;
        }
        
        bool fully_on_camera = false;
        if(
            s_ptr->bbox[0].x > cam_tl.x &&
            s_ptr->bbox[1].x < cam_br.x &&
            s_ptr->bbox[0].y > cam_tl.y &&
            s_ptr->bbox[1].y < cam_br.y
        ) {
            fully_on_camera = true;
        }
        
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            if(!fully_on_camera) {
                //If the sector's fully on-camera, it's faster to not bother
                //with the edge-by-edge check.
                Point edge_tl = v2p(s_ptr->edges[e]->vertexes[0]);
                Point edge_br = edge_tl;
                updateMinMaxCoords(
                    edge_tl, edge_br,
                    v2p(s_ptr->edges[e]->vertexes[1])
                );
                
                if(!rectanglesIntersect(edge_tl, edge_br, cam_tl, cam_br)) {
                    //Edge is off-camera.
                    continue;
                }
            }
            
            edges.insert(s_ptr->edgeIdxs[e]);
        }
    }
    
    //Save the current state of some things.
    ALLEGRO_BITMAP* target_bmp = al_get_target_bitmap();
    int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
    al_get_separate_blender(
        &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
    );
    
    //Set the new operation modes.
    al_set_target_bitmap(buffer);
    al_set_separate_blender(
        ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO,
        ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA
    );
    al_hold_bitmap_drawing(true);
    
    //Draw!
    if(clear_first) {
        al_clear_to_color(COLOR_EMPTY);
    }
    
    for(size_t e_idx : edges) {
        drawEdgeOffsetOnBuffer(caches, e_idx);
    }
    
    //Return to the old state of things.
    al_hold_bitmap_drawing(false);
    al_set_separate_blender(
        old_op, old_src, old_dst, old_aop, old_asrc, old_adst
    );
    al_set_target_bitmap(target_bmp);
}


/**
 * @brief Updates the cached information about all edge offsets.
 *
 * @param caches List of caches to update.
 * @param vertexes_to_update List of vertexes whose edges need updating.
 * @param checker Pointer to a function that checks if the edge should have the
 * intended effect or not. It also returns what sector of the edge
 *  will be affected by the effect, and which won't.
 * @param length_getter Function that returns the length of the effect.
 * @param color_getter Function that returns the color of the effect.
 */
void updateOffsetEffectCaches (
    vector<EdgeOffsetCache> &caches,
    const unordered_set<Vertex*> &vertexes_to_update,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter
) {
    unordered_set<size_t> edges_to_update;
    for(Vertex* v : vertexes_to_update) {
        edges_to_update.insert(v->edgeIdxs.begin(), v->edgeIdxs.end());
    }
    
    for(size_t e : edges_to_update) {
        Edge* e_ptr = game.curAreaData->edges[e];
        
        Sector* unaffected_sector = nullptr;
        Sector* affected_sector = nullptr;
        
        if(!checker(e_ptr, &affected_sector, &unaffected_sector)) {
            //This edge doesn't get the effect.
            caches[e].lengths[0] = 0.0f;
            caches[e].lengths[1] = 0.0f;
            continue;
        }
        
        //We need to process the two vertexes of the edge in a specific
        //order, such that if you stand on the first one being processed,
        //and you face the second one, the affected sector is to the left.
        
        Vertex* ends_to_process[2];
        if(e_ptr->sectors[0] == affected_sector) {
            ends_to_process[0] = e_ptr->vertexes[0];
            ends_to_process[1] = e_ptr->vertexes[1];
            caches[e].firstEndVertexIdx = 0;
        } else {
            ends_to_process[0] = e_ptr->vertexes[1];
            ends_to_process[1] = e_ptr->vertexes[0];
            caches[e].firstEndVertexIdx = 1;
        }
        float edge_process_angle =
            getAngle(v2p(ends_to_process[0]), v2p(ends_to_process[1]));
            
        for(unsigned char end = 0; end < 2; end++) {
            //For each end of the effect...
            
            float length = 0.0f;
            float angle = 0.0f;
            float elbow_length = 0.0f;
            float elbow_angle = 0.0f;
            ALLEGRO_COLOR end_color;
            
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
                e_ptr, ends_to_process[end], end,
                end == 0 ? edge_process_angle : edge_process_angle + TAU / 2.0f,
                checker, length_getter, color_getter,
                &angle, &length, &end_color,
                &elbow_angle, &elbow_length
            );
            
            caches[e].lengths[end] = length;
            caches[e].angles[end] = normalizeAngle(angle);
            caches[e].colors[end] = end_color;
            caches[e].elbowAngles[end] = normalizeAngle(elbow_angle);
            caches[e].elbowLengths[end] = elbow_length;
        }
    }
}
