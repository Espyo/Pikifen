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

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "sector.h"


/* ----------------------------------------------------------------------------
 * Draws an edge offset effect of a given edge onto the current target bitmap,
 * which acts as a buffer.
 * e_ptr:
 *   Edge whose effects to draw.
 * checker:
 *   Pointer to a function that checks if the edge should have the
 *   intended effect or not. It also returns what sector of the edge
 *   will be affected by the effect, and which won't.
 * length_getter:
 *   Pointer to a function that returns the length of a given edge's effect.
 * color_getter:
 *   Pointer to a function that returns the color of a given edge's effect.
 */
void draw_edge_offset_on_buffer(
    edge* e_ptr,
    offset_effect_checker_ptr checker,
    offset_effect_length_getter_ptr length_getter,
    offset_effect_color_getter_ptr color_getter
) {
    const float END_OPACITY = 0.0f;
    
    sector* unaffected_sector = NULL;
    sector* affected_sector = NULL;
    
    if(!checker(e_ptr, &affected_sector, &unaffected_sector)) {
        //This edge doesn't get the effect.
        return;
    }
    
    //We need to process the two vertexes of the edge in a specific
    //order, such that if you stand on the first one being processed,
    //and you face the second one, the affected sector is to the left.
    
    vertex* ends_to_process[2];
    if(e_ptr->sectors[0] == affected_sector) {
        ends_to_process[0] = e_ptr->vertexes[0];
        ends_to_process[1] = e_ptr->vertexes[1];
    } else {
        ends_to_process[0] = e_ptr->vertexes[1];
        ends_to_process[1] = e_ptr->vertexes[0];
    }
    float edge_process_angle =
        get_angle(
            point(ends_to_process[0]->x, ends_to_process[0]->y),
            point(ends_to_process[1]->x, ends_to_process[1]->y)
        );
        
    point end_rel_coords[2];
    unsigned char n_elbow_tris[2] = {0, 0};
    point elbow_rel_coords[2][2];
    ALLEGRO_COLOR end_colors[2];
    
    for(unsigned char e = 0; e < 2; ++e) {
        //For each end of the effect...
        
        float length = 0.0f;
        float angle = 0.0f;
        float elbow_length = 0.0f;
        float elbow_angle = 0.0f;
        
        //The edge's effect is simply a rectangle, although one or both
        //of its ends could be angled inward, either to merge with a
        //neighboring effect or to fit snugly against a different effect's edge.
        //In addition, we may also need to draw an "elbow" shape to connect to
        //a different edge.
        //Start by getting information on how this effect should behave.
        //We don't need to worry about why it's drawn the way it is, since
        //get_edge_offset_edge_info is in charge of that.
        get_edge_offset_edge_info(
            e_ptr, ends_to_process[e], e,
            e == 0 ? edge_process_angle : edge_process_angle + TAU / 2.0f,
            affected_sector, unaffected_sector,
            checker, length_getter, color_getter,
            &angle, &length, &end_colors[e],
            &elbow_angle, &elbow_length
        );
        
        //This end of the effect starts at the vertex and spreads to this point.
        end_rel_coords[e] = rotate_point(point(length, 0), angle);
        
        if(elbow_length > 0.0f) {
            //We need to also draw an elbow connecting this end of the
            //effect to something else. Usually another effect's elbow, but
            //it could just be another effect's edge.
            //The elbow is either one triangle or two triangles, depending
            //on how much it needs to bend.
            
            float rect_to_elbow_diff =
                e == 0 ?
                get_angle_cw_dif(elbow_angle, angle) :
                get_angle_cw_dif(angle, elbow_angle);
                
            if(rect_to_elbow_diff > TAU / 8.00001f) {
                //We add a small amount to the threshold because of floating
                //point imperfections. A perfectly square sector (easy to do in
                //the editor) may result in elbows where one side gets one
                //triangle, and the other gets two. At least this small bump
                //in the angle threshold makes it much less likely to happen.
                n_elbow_tris[e] = 2;
                float mid_elbow_angle =
                    e == 0 ?
                    angle - rect_to_elbow_diff / 2.0f :
                    angle + rect_to_elbow_diff / 2.0f;
                elbow_rel_coords[e][0] =
                    rotate_point(point(elbow_length, 0), mid_elbow_angle);
            } else {
                n_elbow_tris[e] = 1;
            }
            
            elbow_rel_coords[e][n_elbow_tris[e] - 1] =
                rotate_point(point(elbow_length, 0), elbow_angle);
        }
        
    }
    
    //Start setting up the vertexes for the drawing process. These do not
    //take into account the elbow, and are just the standard "rectangle".
    ALLEGRO_VERTEX av[4];
    for(size_t e = 0; e < 2; ++e) {
        av[e].x = ends_to_process[e]->x;
        av[e].y = ends_to_process[e]->y;
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
    for(unsigned char v = 0; v < 4; ++v) {
        al_transform_coordinates(
            &game.world_to_screen_transform, &av[v].x, &av[v].y
        );
    }
    
    //Draw the "rectangle"!
    al_draw_prim(av, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
    
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
    
    for(unsigned char e = 0; e < 2; ++e) {
        for(unsigned char v = 0; v < n_elbow_tris[e]; ++v) {
            elbow_av[e][v + 2].x =
                ends_to_process[e]->x + elbow_rel_coords[e][v].x;
            elbow_av[e][v + 2].y =
                ends_to_process[e]->y + elbow_rel_coords[e][v].y;
            elbow_av[e][v + 2].z = 0.0f;
            elbow_av[e][v + 2].color = end_colors[e];
            elbow_av[e][v + 2].color.a = END_OPACITY;
            al_transform_coordinates(
                &game.world_to_screen_transform,
                &elbow_av[e][v + 2].x, &elbow_av[e][v + 2].y
            );
        }
    }
    
    //Draw the elbows!
    for(unsigned char e = 0; e < 2; ++e) {
        if(n_elbow_tris[e] == 0) continue;
        al_draw_prim(
            elbow_av[e], NULL, NULL, 0,
            n_elbow_tris[e] + 2,
            ALLEGRO_PRIM_TRIANGLE_FAN
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws edge offset effects onto the given sector. This requires that
 * the effects have been drawn onto a buffer, from which this algorithm samples.
 * s_ptr:
 *   Sector to draw the effects of.
 * buffer:
 *   Buffer to draw from.
 * opacity:
 *   Draw at this opacity, 0 - 1.
 */
void draw_sector_edge_offsets(
    sector* s_ptr, ALLEGRO_BITMAP* buffer, const float opacity
) {
    if(s_ptr->is_bottomless_pit) return;
    
    size_t n_vertexes = s_ptr->triangles.size() * 3;
    ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
        vertex* v_ptr = t_ptr->points[v % 3];
        float vx = v_ptr->x;
        float vy = v_ptr->y;
        
        av[v].x = vx;
        av[v].y = vy;
        al_transform_coordinates(
            &game.world_to_screen_transform, &vx, &vy
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
        av, NULL, buffer,
        0, n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
    );
    
    delete[] av;
}


/* ----------------------------------------------------------------------------
 * Returns information about one of the ends of an edge offset effect.
 * e_ptr:
 *   Edge with the effect.
 * end_vertex:
 *   Vertex of the end being processed.
 * end_idx:
 *   Index of the end being processed. 0 is the end of the edge where the
 *   sector receiving the effect is to the left, if you face
 *   from end 0 to end 1.
 * edge_process_angle:
 *   Angle that the edge makes from the current end to the opposite one.
 * affected_sector:
 *   Sector that gets affected by the effect.
 * unaffected_sector:
 *   Sector that is unaffected by the effect.
 * checker:
 *   Pointer to a function that checks if the edge should have the
 *   intended effect or not. It also returns what sector of the edge
 *   will be affected by the effect, and which won't.
 * length_getter:
 *   Function that returns the length of the effect.
 * color_getter:
 *   Function that returns the color of the effect.
 * final_angle:
 *   The angle of the tip of this end of the effect's "rectangle".
 * final_length:
 *   The length of the tip of this end of the effect's "rectangle".
 * final_color:
 *   The color at this end of the effect's "rectangle".
 * final_elbow_angle:
 *   The angle that the elbow must finish at. 0 if no elbow is needed.
 * final_elbow_length:
 *   The length of the line at the end of the elbow. 0 if no elbow is needed.
 */
void get_edge_offset_edge_info(
    edge* e_ptr, vertex* end_vertex, const unsigned char end_idx,
    const float edge_process_angle,
    sector* affected_sector, sector* unaffected_sector,
    offset_effect_checker_ptr checker,
    offset_effect_length_getter_ptr length_getter,
    offset_effect_color_getter_ptr color_getter,
    float* final_angle, float* final_length, ALLEGRO_COLOR* final_color,
    float* final_elbow_angle, float* final_elbow_length
) {
    *final_elbow_angle = 0.0f;
    *final_elbow_length = 0.0f;
    *final_color = color_getter(e_ptr);
    
    float base_effect_length = length_getter(e_ptr);
    float base_effect_angle =
        end_idx == 0 ?
        edge_process_angle - TAU / 4.0f :
        edge_process_angle + TAU / 4.0f;
    base_effect_angle = normalize_angle(base_effect_angle);
    bool edge_effect_cw = end_idx == 1;
    
    edge* next_edge = NULL;
    float next_edge_angle = 0.0f;
    float next_edge_diff = 0.0f;
    edge* next_eff_edge = NULL;
    float next_eff_edge_angle = 0.0f;
    float next_eff_edge_diff = 0.0f;
    bool next_eff_edge_effect_cw = false;
    float next_eff_edge_base_effect_angle = 0.0f;
    
    //Start by getting some information about the edges
    //that comes after this one.
    get_next_edge(
        end_vertex, edge_process_angle, edge_effect_cw, e_ptr,
        &next_edge, &next_edge_angle, &next_edge_diff
    );
    
    get_next_offset_effect_edge(
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
            
        get_edge_offset_intersection(
            e_ptr, next_eff_edge, end_vertex,
            base_effect_angle, next_eff_edge_base_effect_angle,
            mid_effect_length, end_idx,
            final_angle, final_length
        );
        
        *final_color =
            interpolate_color(
                0.5, 0, 1,
                *final_color,
                color_getter(next_eff_edge)
            );
            
    } else if(
        next_eff_edge && next_eff_edge_effect_cw == edge_effect_cw &&
        next_eff_edge_diff < TAU / 4.0f
    ) {
        //Next edge has an effect that goes in the same direction,
        //and that edge imposes over our effect.
        //As such, skew our effect inwards to align with that edge.
        *final_angle = next_eff_edge_angle;
        *final_length = base_effect_length / sin(next_eff_edge_diff);
        
    } else if(
        !next_eff_edge
    ) {
        //There's nothing to connect to in any way, so we might as well shrink
        //this end. Shrinking it to 0 will make effects of edges where there's
        //nothing on both sides disappear, which may mislead the user. So
        //instead just make it a fraction of the usual size.
        *final_angle = base_effect_angle;
        *final_length = base_effect_length / 5.0f;
        
    } else {
        //We can draw our end of the effect forward without a care.
        *final_angle = base_effect_angle;
        *final_length = base_effect_length;
        
        if(next_eff_edge && next_eff_edge_effect_cw != edge_effect_cw) {
            //On this end there is a neighboring effect we'll want to connect
            //to. But because that neighboring effect is so far away in
            //terms of angle, we'll need to implement an elbow between them
            //so they can be connected. This edge will draw half of the elbow,
            //and the other will draw its half when it's its turn.
            float next_edge_base_effect_length =
                length_getter(next_eff_edge);
            float mid_effect_length =
                (base_effect_length + next_edge_base_effect_length) / 2.0f;
                
            *final_length = mid_effect_length;
            *final_elbow_length = mid_effect_length;
            *final_elbow_angle =
                end_idx == 0 ?
                next_eff_edge_angle +
                get_angle_cw_dif(
                    next_eff_edge_angle, edge_process_angle
                ) / 2.0f :
                edge_process_angle +
                get_angle_cw_dif(
                    edge_process_angle, next_eff_edge_angle
                ) / 2.0f;
            *final_color =
                interpolate_color(
                    0.5, 0, 1,
                    *final_color,
                    color_getter(next_eff_edge)
                );
                
        } else if(next_eff_edge && next_eff_edge_effect_cw == edge_effect_cw) {
            //There is a neighboring edge that has the effect, but in
            //the same direction as ours. As such, our effect will have
            //to connect to that effect's edge so there's a snug fit.
            //But because that neighboring effect is so far away in terms of
            //angle, we'll need to implement an elbow between them so they
            //can be connected. This edge will be in charge of drawing the full
            //elbow.
            *final_elbow_angle = next_eff_edge_angle;
            *final_elbow_length = base_effect_length;
        }
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Returns the point in which the far end of two edge offset effects intersect.
 * This calculation is only for the base "rectangle" shape of the effect,
 * and doesn't take into account any inward slants given on the ends, nor
 * does it care about elbows.
 * Normally, this would be the intersection point between the line segments
 * that make up both effects's rectangle ends, but there may be cases, explained
 * below, where that doesn't result in a real collision. In order for the
 * algorithm to always return something that at least can be worked with,
 * the intersection is calculated as if both effect ends were infinitely long
 * lines.
 * e1:
 *   First effect-casting edge. This is the main edge being processed.
 * e2:
 *   Second effect-casting edge.
 * common_vertex:
 *   The vertex shared between these two edges.
 * base_effect_angle1:
 *   The base angle at which edge 1's effect is projected.
 * base_effect_angle2:
 *   Same as base_effect_angle1, but for edge 2.
 * effect_length:
 *   Length of either effect.
 * end_idx:
 *   Index of the end being processed. 0 means that the sector that receives
 *   the effect is to the left, if you stand on end 0 and face end 1.
 * final_angle:
 *   The angle from the common vertex to the intersection point.
 * final_length:
 *   The length from the common vertex to the intersection point.
 */
void get_edge_offset_intersection(
    edge* e1, edge* e2, vertex* common_vertex,
    const float base_effect_angle1, const float base_effect_angle2,
    const float effect_length, const unsigned char end_idx,
    float* final_angle, float* final_length
) {
    vertex* other_vertex1 = e1->get_other_vertex(common_vertex);
    float base_cos1 = cos(base_effect_angle1);
    float base_sin1 = sin(base_effect_angle1);
    point effect1_p0(
        common_vertex->x + base_cos1 * effect_length,
        common_vertex->y + base_sin1 * effect_length
    );
    point effect1_p1(
        other_vertex1->x + base_cos1 * effect_length,
        other_vertex1->y + base_sin1 * effect_length
    );
    
    vertex* other_vertex2 = e2->get_other_vertex(common_vertex);
    float base_cos2 = cos(base_effect_angle2);
    float base_sin2 = sin(base_effect_angle2);
    point effect2_p0(
        common_vertex->x + base_cos2 * effect_length,
        common_vertex->y + base_sin2 * effect_length
    );
    point effect2_p1(
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
        lines_intersect(
            effect1_p0, effect1_p1,
            effect2_p0, effect2_p1,
            &r, NULL
        )
    ) {
        //Clamp r to prevent long, close edges from
        //creating jagged effects outside the edge.
        r = clamp(r, 0.0f, 1.0f);
        point p(
            effect1_p0.x + (effect1_p1.x - effect1_p0.x) * r,
            effect1_p0.y + (effect1_p1.y - effect1_p0.y) * r
        );
        coordinates_to_angle(
            p - point(common_vertex->x, common_vertex->y),
            final_angle, final_length
        );
    } else {
        //Okay, they don't really intersect. This should never happen... Maybe
        //a floating point imperfection? Oh well, let's go for a failsafe.
        *final_angle = 0.0f;
        *final_length = 0.0f;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the next edge in a vertex's list of edges.
 * It checks in a given direction, starting from some pivot angle.
 * v_ptr:
 *   Vertex to work on.
 * pivot_angle:
 *   Angle to check from.
 * clockwise:
 *   True to check in a clockwise direction, false for counter-clockwise.
 * ignore:
 *   Edge to ignore while checking, if any.
 * final_edge:
 *   The found edge is returned here, or NULL.
 * final_angle:
 *   Angle of the found edge.
 * final_diff:
 *   Difference in angle between the two.
 */
void get_next_edge(
    vertex* v_ptr, const float pivot_angle, const bool clockwise, edge* ignore,
    edge** final_edge, float* final_angle, float* final_diff
) {
    edge* best_edge = NULL;
    float best_edge_diff = 0.0f;
    float best_edge_angle = 0.0f;
    
    for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
        edge* e_ptr = v_ptr->edges[e];
        
        if(e_ptr == ignore) continue;
        
        unsigned char other_vertex_idx = e_ptr->vertexes[0] == v_ptr ? 1 : 0;
        vertex* other_vertex = e_ptr->vertexes[other_vertex_idx];
        
        float angle =
            get_angle(
                point(v_ptr->x, v_ptr->y),
                point(other_vertex->x, other_vertex->y)
            );
            
        float diff =
            clockwise ?
            get_angle_cw_dif(pivot_angle, angle) :
            get_angle_cw_dif(angle, pivot_angle);
            
        if(!best_edge || diff < best_edge_diff) {
            best_edge = e_ptr;
            best_edge_diff = diff;
            best_edge_angle = angle;
        }
    }
    
    *final_edge = best_edge;
    *final_angle = best_edge_angle;
    *final_diff = best_edge_diff;
}


/* ----------------------------------------------------------------------------
 * Returns the next edge that needs the given edge offset event,
 * in a vertex's list of edges. It checks in a given direction,
 * starting from some pivot angle.
 * v_ptr:
 *   Vertex to work on.
 * pivot_angle:
 *   Angle to check from.
 * clockwise:
 *   True to check in a clockwise direction, false for counter-clockwise.
 * ignore:
 *   Edge to ignore while checking, if any.
 * edge_checker:
 *   Function that returns whether or not a given edge should use the effect.
 * final_edge:
 *   The found edge is returned here, or NULL.
 * final_angle:
 *   Angle of the found edge.
 * final_diff:
 *   Difference in angle between the two.
 * final_base_effect_angle:
 *   The base effect angle of the found edge.
 * final_effect_cw:
 *   Is the effect cast clockwise?
 */
void get_next_offset_effect_edge(
    vertex* v_ptr, const float pivot_angle, const bool clockwise, edge* ignore,
    offset_effect_checker_ptr edge_checker,
    edge** final_edge, float* final_angle, float* final_diff,
    float* final_base_effect_angle,
    bool* final_effect_cw
) {
    edge* best_edge = NULL;
    float best_edge_diff = 0;
    float best_edge_angle = 0;
    bool best_edge_effect_cw = false;
    
    for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
        edge* e_ptr = v_ptr->edges[e];
        
        if(e_ptr == ignore) continue;
        
        sector* affected_sector;
        sector* unaffected_sector;
        if(!edge_checker(e_ptr, &affected_sector, &unaffected_sector)) {
            //This edge does not use the effect.
            continue;
        }
        unsigned char unaffected_sector_idx =
            e_ptr->sectors[0] == unaffected_sector ? 0 : 1;
            
        unsigned char other_vertex_idx = e_ptr->vertexes[0] == v_ptr ? 1 : 0;
        vertex* other_vertex = e_ptr->vertexes[other_vertex_idx];
        
        //Standing on the common vertex, facing the edge,
        //to what side does the effect go?
        bool effect_is_cw = other_vertex_idx != unaffected_sector_idx;
        
        float angle =
            get_angle(
                point(v_ptr->x, v_ptr->y),
                point(other_vertex->x, other_vertex->y)
            );
            
        float diff =
            clockwise ?
            get_angle_cw_dif(pivot_angle, angle) :
            get_angle_cw_dif(angle, pivot_angle);
            
        if(!best_edge || diff < best_edge_diff) {
            best_edge = e_ptr;
            best_edge_diff = diff;
            best_edge_angle = angle;
            best_edge_effect_cw = effect_is_cw;
        }
    }
    
    *final_edge = best_edge;
    *final_diff = best_edge_diff;
    *final_angle = best_edge_angle;
    *final_effect_cw = best_edge_effect_cw;
    if(best_edge_effect_cw) {
        *final_base_effect_angle =
            normalize_angle(best_edge_angle + TAU / 4.0f);
    } else {
        *final_base_effect_angle =
            normalize_angle(best_edge_angle - TAU / 4.0f);
    }
}


/* ----------------------------------------------------------------------------
 * Draws edge offset effects for all edges on-screen onto a buffer,
 * so that sectors may then sample from it to draw what effects they need.
 * cam_tl:
 *   Top-left corner of the camera boundaries. The edges of any sector that is
 *   beyond these boundaries will be ignored.
 * cam_br:
 *   Same as cam_tl, but for the bottom-right boundaries.
 * buffer:
 *   Buffer to draw to.
 * clear_first:
 *   If true, the bitmap is cleared before any drawing is done.
 * checker:
 *   Pointer to a function that checks whether an edge uses the specified
 *   edge offset effect.
 * length_getter:
 *   Pointer to a function that returns the length of the edge offset effect.
 * color_getter:
 *   Pointer to a function that returns the color of the edge offset effect.
 */
void update_offset_effect_buffer(
    const point &cam_tl, const point &cam_br,
    ALLEGRO_BITMAP* buffer, const bool clear_first,
    offset_effect_checker_ptr checker,
    offset_effect_length_getter_ptr length_getter,
    offset_effect_color_getter_ptr color_getter
) {
    unordered_set<edge*> edges;
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        
        if(
            !rectangles_intersect(
                s_ptr->bbox[0], s_ptr->bbox[1],
                cam_tl, cam_br
            )
        ) {
            //Off-camera.
            continue;
        }
        
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            edges.insert(s_ptr->edges[e]);
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
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
    }
    
    for(edge* e_ptr : edges) {
        draw_edge_offset_on_buffer(
            e_ptr, checker, length_getter, color_getter
        );
    }
    
    //Return to the old state of things.
    al_hold_bitmap_drawing(false);
    al_set_separate_blender(
        old_op, old_src, old_dst, old_aop, old_asrc, old_adst
    );
    al_set_target_bitmap(target_bmp);
}
