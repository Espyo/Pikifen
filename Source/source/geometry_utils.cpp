/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Geometry-related utility functions.
 */

#include "geometry_utils.h"
#include "functions.h"


/* ----------------------------------------------------------------------------
 * Returns the vector coordinates of an angle.
 * angle:     The angle.
 * magnitude: Its magnitude.
 * *_coord:   Variables to return the coordinates to.
 */
void angle_to_coordinates(
    const float angle, const float magnitude, float* x_coord, float* y_coord
) {
    *x_coord = cos(angle) * magnitude;
    *y_coord = sin(angle) * magnitude;
}


/* ----------------------------------------------------------------------------
 * Checks if two spheres are colliding via a bounding-box check.
 * c*1: Coordinates of the first sphere.
 * c*2: Coordinates of the second sphere.
 * r:   Range of the bounding box.
 */
bool bbox_check(
    const float cx1, const float cy1, const float cx2, const float cy2,
    const float r
) {
    return (fabs(cx1 - cx2) <= r && fabs(cy1 - cy2) <= r);
}


/* ----------------------------------------------------------------------------
 * Returns whether a circle is touching a line segment or not.
 * cx, cy: Coordinates of the circle.
 * cr:     Radius of the circle.
 * x*, y*: Coordinates of the line.
 * li*:    If not NULL, the line intersection coordinates are returned here.
 */
bool circle_intersects_line(
    const float cx, const float cy, const float cr,
    const float x1, const float y1, const float x2, const float y2,
    float* lix, float* liy
) {

    //Code by
    //http://www.melloland.com/scripts-and-tutos/
    //collision-detection-between-circles-and-lines
    
    float vx = x2 - x1;
    float vy = y2 - y1;
    float xdiff = x1 - cx;
    float ydiff = y1 - cy;
    float a = vx * vx + vy * vy;
    float b = 2 * ((vx * xdiff) + (vy * ydiff));
    float c = xdiff * xdiff + ydiff * ydiff - cr * cr;
    float quad = b * b - (4 * a * c);
    if (quad >= 0) {
        //An infinite collision is happening, but let's not stop here
        float quadsqrt = sqrt(quad);
        for (int i = -1; i <= 1; i += 2) {
            //Returns the two coordinates of the intersection points
            float t = (i * -b + quadsqrt) / (2 * a);
            float x = x1 + (i * vx * t);
            float y = y1 + (i * vy * t);
            //If one of them is in the boundaries of the segment, it collides
            if (
                x >= min(x1, x2) && x <= max(x1, x2) &&
                y >= min(y1, y2) && y <= max(y1, y2)
            ) {
                if(lix) *lix = x;
                if(liy) *liy = y;
                return true;
            }
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the closest point in a line to a given point.
 * l1, l2:        Points of the line.
 * p:             Reference point.
 * segment_ratio: If not NULL, the ratio from l1 to l2 is returned here.
   * Between 0 and 1, it belongs to the line segment. If not, it doesn't.
 */
point get_closest_point_in_line(
    const point l1, const point l2, const point p, float* segment_ratio
) {

    //Code by http://stackoverflow.com/a/3122532
    
    point l1_to_p(p.x - l1.x, p.y - l1.y);
    point l1_to_l2(l2.x - l1.x, l2.y - l1.y);
    
    float l1_to_l2_squared =
        l1_to_l2.x * l1_to_l2.x +
        l1_to_l2.y * l1_to_l2.y;
        
    float l1_to_p_dot_l1_to_l2 =
        l1_to_p.x * l1_to_l2.x +
        l1_to_p.y * l1_to_l2.y;
        
    float r = l1_to_p_dot_l1_to_l2 / l1_to_l2_squared;
    
    if(segment_ratio) *segment_ratio = r;
    
    return point(l1.x + l1_to_l2.x * r, l1.y + l1_to_l2.y * r);
}


/* ----------------------------------------------------------------------------
 * Returns the closest vertex that can be merged with the specified point.
 * Returns NULL if there is no vertex close enough to merge.
 * x, y:         Coordinates of the point.
 * all_vertexes: Vector with all of the vertexes in the area.
 * merge_radius: Minimum radius to merge.
 * v_nr:         If not NULL, the vertex's number is returned here.
 */
vertex* get_merge_vertex(
    const float x, const float y, vector<vertex*> &all_vertexes,
    const float merge_radius, size_t* v_nr
) {
    dist closest_dist = 0;
    vertex* closest_v = NULL;
    size_t closest_nr = INVALID;
    
    for(size_t v = 0; v < all_vertexes.size(); ++v) {
        vertex* v_ptr = all_vertexes[v];
        dist d(x, y, v_ptr->x, v_ptr->y);
        if(
            d <= merge_radius &&
            (d < closest_dist || !closest_v)
        ) {
            closest_dist = d;
            closest_v = v_ptr;
            closest_nr = v;
        }
    }
    
    if(v_nr) *v_nr = closest_nr;
    return closest_v;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an edge is valid.
 * An edge is valid if it has non-NULL vertexes.
 */
bool is_edge_valid(edge* l) {
    if(!l->vertexes[0]) return false;
    if(!l->vertexes[1]) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns whether a polygon was created clockwise or anti-clockwise,
 * given the order of its vertexes.
 */
bool is_polygon_clockwise(vector<vertex*> &vertexes) {
    //Solution by http://stackoverflow.com/a/1165943
    float sum = 0;
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        vertex* v2_ptr = get_next_in_vector(vertexes, v);
        sum += (v2_ptr->x - v_ptr->x) * (v2_ptr->y + v_ptr->y);
    }
    return sum < 0;
}


/* ----------------------------------------------------------------------------
 * Returns the movement necessary to move a point.
 * x/y:          Coordinates of the initial point.
 * tx/ty:        Coordinates of the target point.
 * speed:        Speed at which the point can move.
 * reach_radius: If the point is within this range of the target,
   * consider it as already being there.
 * mx/my:        Variables to return the amount of movement to.
 * angle:        Variable to return the angle the point faces to.
 * reached:      Variable to return whether the point reached the target.
 */
void move_point(
    const float x, const float y, const float tx, const float ty,
    const float speed, const float reach_radius, float* mx, float* my,
    float* angle, bool* reached, const float delta_t
) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);
    
    if(dist > reach_radius) {
        float move_amount =
            min((double) (dist / delta_t / 2.0f), (double) speed);
            
        dx *= move_amount / dist;
        dy *= move_amount / dist;
        
        if(mx) *mx = dx;
        if(my) *my = dy;
        if(angle) *angle = atan2(dy, dx);
        if(reached) *reached = false;
        
    } else {
    
        if(mx) *mx = 0;
        if(my) *my = 0;
        if(reached) *reached = true;
    }
}


/* ----------------------------------------------------------------------------
 * Normalizes an angle so that it's between 0 and M_PI * 2.
 */
float normalize_angle(float a) {
    a = fmod((double) a, M_PI * 2);
    if(a < 0) a += M_PI * 2;
    return a;
}


/* ----------------------------------------------------------------------------
 * Rotates a point by an angle.
 * The x and y are meant to represent the difference
 * between the point and the center of the rotation.
 */
void rotate_point(
    const float x, const float y, const float angle,
    float* final_x, float* final_y
) {
    float c = cos(angle);
    float s = sin(angle);
    if(final_x) *final_x = c * x - s * y;
    if(final_y) *final_y = s * x + c * y;
}


/* ----------------------------------------------------------------------------
 * Returns whether a square intersects with a line segment.
 * Also returns true if the line is fully inside the square.
 * s**: Square coordinates.
 * l**: Line coordinates.
 */
bool square_intersects_line(
    const float sx1, const float sy1, const float sx2, const float sy2,
    const float lx1, const float ly1, const float lx2, const float ly2
) {
    if(
        //Line crosses left side?
        lines_intersect(lx1, ly1, lx2, ly2, sx1, sy1, sx1, sy2, NULL, NULL) ||
        //Line crosses right side?
        lines_intersect(lx1, ly1, lx2, ly2, sx2, sy1, sx2, sy2, NULL, NULL) ||
        //Line crosses top side?
        lines_intersect(lx1, ly1, lx2, ly2, sx1, sy1, sx2, sy1, NULL, NULL) ||
        //Line crosses bottom side?
        lines_intersect(lx1, ly1, lx2, ly2, sx1, sy2, sx2, sy2, NULL, NULL)
    ) {
        return true;
    }
    
    if(
        (lx1 >= sx1 && lx2 >= sx1) &&
        (lx1 <= sx2 && lx2 <= sx2) &&
        (ly1 >= sy1 && ly2 >= sy1) &&
        (ly1 <= sy2 && ly2 <= sy2)
    ) return true;
    
    return false;
    
}
