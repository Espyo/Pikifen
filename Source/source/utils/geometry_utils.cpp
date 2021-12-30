/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Geometry-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <algorithm>
#include <cmath>
#include <math.h>

#include "geometry_utils.h"
#include "math_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new distance number, given two points.
 * p1:
 *   First point.
 * p2:
 *   Second point.
 */
dist::dist(const point &p1, const point &p2) :
    distance_squared(
        (p2.x - p1.x) * (p2.x - p1.x) +
        (p2.y - p1.y) * (p2.y - p1.y)
    ),
    normal_distance(0),
    has_normal_distance(false) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new distance number, given a non-squared distance.
 * d:
 *   Regular, non-squared distance.
 */
dist::dist(const float d) :
    distance_squared(d * d),
    normal_distance(d),
    has_normal_distance(true) {
    
}


/* ----------------------------------------------------------------------------
 * Sets the value given a non-squared distance.
 * d:
 *   Regular, non-squared distance.
 */
dist &dist::operator =(const float d) {
    distance_squared = d * d;
    normal_distance = d;
    has_normal_distance = true;
    return *this;
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is smaller than the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator<(const float d2) const {
    return distance_squared < (d2 * d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is larger than the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator>(const float d2) const {
    return distance_squared > (d2 * d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is the same as the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator==(const float d2) const {
    return distance_squared == (d2 * d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is smaller than or equal to the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator<=(const float d2) const {
    return !operator>(d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is larger than or equal to the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator>=(const float d2) const {
    return !operator<(d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is different from the specified one.
 * d2:
 *   Regular, non-squared distance to check.
 */
bool dist::operator!=(const float d2) const {
    return !operator==(d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is smaller than the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator<(const dist &d2) const {
    return distance_squared < d2.distance_squared;
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is larger than the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator>(const dist &d2) const {
    return distance_squared > d2.distance_squared;
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is the same as the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator==(const dist &d2) const {
    return distance_squared == d2.distance_squared;
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is smaller than or equal to the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator<=(const dist &d2) const {
    return !operator>(d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is larger than or equal to the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator>=(const dist &d2) const {
    return !operator<(d2);
}


/* ----------------------------------------------------------------------------
 * Checks if this distance is different from the specified one.
 * d2:
 *   Distance object to check.
 */
bool dist::operator!=(const dist &d2) const {
    return !operator==(d2);
}


/* ----------------------------------------------------------------------------
 * Adds some distance to the current one.
 * d2:
 *   Amount of distance to add.
 */
void dist::operator+=(const dist &d2) {
    distance_squared += d2.distance_squared;
    if(has_normal_distance && d2.has_normal_distance) {
        normal_distance += d2.normal_distance;
    } else {
        has_normal_distance = false;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the regular, non-squared distance as a number.
 */
float dist::to_float() {
    if(!has_normal_distance) {
        normal_distance = sqrt(distance_squared);
        has_normal_distance = true;
    }
    return normal_distance;
}


/* ----------------------------------------------------------------------------
 * Constructs a point, given its coordinates.
 * x:
 *   X coordinate.
 * y:
 *   Y coordinate.
 */
point::point(const float x, const float y) :
    x(x),
    y(y) {
    
}


/* ----------------------------------------------------------------------------
 * Constructs a point, with the coordinates set to 0,0.
 */
point::point() :
    x(0),
    y(0) {
    
}


/* ----------------------------------------------------------------------------
 * Adds the coordinates of two points.
 * p:
 *   Point with values to add with.
 */
const point point::operator +(const point &p) const {
    return point(x + p.x, y + p.y);
}


/* ----------------------------------------------------------------------------
 * Subtracts the coordinates of two points.
 * p:
 *   Point with values to subtract with.
 */
const point point::operator -(const point &p) const {
    return point(x - p.x, y - p.y);
}


/* ----------------------------------------------------------------------------
 * Multiplies the coordinates of two points.
 * p:
 *   Point with values to multiply with.
 */
const point point::operator *(const point &p) const {
    return point(x * p.x, y * p.y);
}


/* ----------------------------------------------------------------------------
 * Divides the coordinates of two points.
 * p:
 *   Point with values to divide with.
 */
const point point::operator /(const point &p) const {
    return point(x / p.x, y / p.y);
}


/* ----------------------------------------------------------------------------
 * Adds a number to the coordinates.
 * n:
 *   Number to add to both coordinates.
 */
const point point::operator +(const float n) const {
    return point(x + n, y + n);
}


/* ----------------------------------------------------------------------------
 * Subtracts a number from each coordinates.
 * n:
 *   Number to subtract from both coordinates.
 */
const point point::operator -(const float n) const {
    return point(x - n, y - n);
}


/* ----------------------------------------------------------------------------
 * Divides the coordinates by a number.
 * n:
 *   Number to divide both coordinates with.
 */
const point point::operator /(const float n) const {
    return point(x / n, y / n);
}


/* ----------------------------------------------------------------------------
 * Adds the coordinates of another point to this one's.
 * p:
 *   Point with the values to add with.
 */
point point::operator +=(const point &p) {
    x += p.x;
    y += p.y;
    return point(x, y);
}


/* ----------------------------------------------------------------------------
 * Subtracts the coordinates of another point to this one's.
 * p:
 *   Point with the values to subtract with.
 */
point point::operator -=(const point &p) {
    x -= p.x;
    y -= p.y;
    return point(x, y);
}


/* ----------------------------------------------------------------------------
 * Adds a given number to the coordinates.
 * n:
 *   Value to add to both coordinates with.
 */
point point::operator +=(const float n) {
    x += n;
    y += n;
    return point(x, y);
}


/* ----------------------------------------------------------------------------
 * Multiplies the coordinates by a given number.
 * n:
 *   Value to multiply both coordinates with.
 */
point point::operator *=(const float n) {
    x *= n;
    y *= n;
    return point(x, y);
}


/* ----------------------------------------------------------------------------
 * Compares if two points are the same.
 * p:
 *   Other point to compare against.
 */
const bool point::operator ==(const point &p) const {
    return x == p.x && y == p.y;
}


/* ----------------------------------------------------------------------------
 * Compares if two points are different.
 * p:
 *   Other point to compare against.
 */
const bool point::operator !=(const point &p) const {
    return x != p.x || y != p.y;
}


/* ----------------------------------------------------------------------------
 * Multiplies the coordinates by a number.
 * m:
 *   Value to multiply both coordinates with.
 */
const point point::operator *(const float m) const {
    return point(x * m, y * m);
}



/* ----------------------------------------------------------------------------
 * Returns the vector coordinates of an angle.
 * angle:
 *   The angle.
 * magnitude:
 *   Its magnitude.
 */
point angle_to_coordinates(
    const float angle, const float magnitude
) {
    return point(cos(angle) * magnitude, sin(angle) * magnitude);
}


/* ----------------------------------------------------------------------------
 * Converts angular distance to linear distance.
 * angular_dist:
 *   Angular distance value.
 * radius:
 *   Radius of the circle.
 */
float angular_dist_to_linear(const float angular_dist, const float radius) {
    return 2 * radius * tan(angular_dist / 2);
}


/* ----------------------------------------------------------------------------
 * Checks if two spheres are colliding via a bounding-box check.
 * center1:
 *   Coordinates of the first sphere.
 * center2:
 *   Coordinates of the second sphere.
 * r:
 *   Range of the bounding box.
 */
bool bbox_check(const point &center1, const point &center2, const float r) {
    return
        (
            fabs(center1.x - center2.x) <= r &&
            fabs(center1.y - center2.y) <= r
        );
}


/* ----------------------------------------------------------------------------
 * Checks if a rectangle and a sphere are colliding via a bounding-box check.
 * tl1:
 *   Top-left coordinates of the rectangle.
 * br1:
 *   Bottom-right coordinates of the rectangle.
 * center2:
 *   Coordinates of the sphere.
 * r:
 *   Radius of the sphere.
 */
bool bbox_check(
    const point &tl1, const point &br1,
    const point &center2, const float r
) {
    return
        rectangles_intersect(
            tl1, br1,
            center2 - r, center2 + r
        );
}


/* ----------------------------------------------------------------------------
 * Calculates the requires horizontal and vertical speed in order to
 * throw something to the specified coordinates, such that it reaches a
 * specific peak height.
 * If the calculation is impossible (like if the peak height is lower than the
 * starting height), the speed variables will all be set to 0.
 * start_xy:
 *   Starting X and Y coordinates.
 * start_z:
 *   Starting Z coordinate.
 * target_xy:
 *   Target destination's X and Y coordinates.
 * target_z:
 *   Target destination's Z coordinate.
 * max_h:
 *   Maximum height, using the starting Z as the reference.
 * gravity:
 *   Constant for the force of gravity, in units per second squared.
 * req_speed_xy:
 *   The required X and Y speed is returned here.
 * req_speed_z:
 *   The required Z speed is returned here.
 * final_h_angle:
 *   The final horizontal angle is returned here (if not NULL).
 */
void calculate_throw(
    const point &start_xy, const float start_z,
    const point &target_xy, const float target_z,
    const float max_h, const float gravity,
    point* req_speed_xy, float* req_speed_z, float* final_h_angle
) {

    if(target_z - start_z > max_h) {
        //If the target is above the maximum height it can be thrown...
        //Then this is an impossible throw.
        *req_speed_xy = point();
        *req_speed_z = 0;
        return;
    }
    
    //Code from https://physics.stackexchange.com/questions/515688
    //First, we calculate stuff in 2D, with horizontal and vertical components
    //only.
    
    //We start with the vertical speed. This will be constant regardless
    //of how far the mob is thrown. In order to reach the required max height,
    //the vertical speed needs to be set thusly:
    *req_speed_z = sqrt(2.0 * (-gravity) * max_h);
    
    //Now that we know the vertical speed, we can figure out how long it takes
    //for the mob to land at the target vertical coordinate. The formula for
    //this can be found on Wikipedia, for instance.
    float height_delta = start_z - target_z;
    //Because of floating point precision problems, the result of the sqrt
    //could end up negative. Let's cap it to zero.
    float sqrt_part =
        std::max(
            0.0,
            sqrt(
                (*req_speed_z) * (*req_speed_z) +
                2.0 * (-gravity) * (height_delta)
            )
        );
    float flight_time = ((*req_speed_z) + sqrt_part) / (-gravity);
    
    //Once we know the total flight time, we can divide the horizontal reach
    //by the total time to get the horizontal speed.
    float h_angle, h_reach;
    coordinates_to_angle(target_xy - start_xy, &h_angle, &h_reach);
    
    float h_speed = h_reach / flight_time;
    
    //Now that we know the vertical and horizontal speed, just split the
    //horizontal speed into X and Y 3D world components.
    *req_speed_xy = angle_to_coordinates(h_angle, h_speed);
    
    //Return the final horizontal angle, if needed.
    if(final_h_angle) {
        *final_h_angle = h_angle;
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether a circle is touching a line segment or not.
 * circle:
 *   Coordinates of the circle.
 * radius:
 *   Radius of the circle.
 * line_p1:
 *   Starting point of the line segment.
 * line_p2:
 *   Ending point of the line segment.
 * lix:
 *   If not NULL, the line intersection's X coordinate is returned here.
 * liy:
 *   If not NULL, the line intersection's Y coordinate is returned here.
 */
bool circle_intersects_line(
    const point &circle, const float radius,
    const point &line_p1, const point &line_p2,
    float* lix, float* liy
) {

    //Code by
    //  http://www.melloland.com/scripts-and-tutos/
    //  collision-detection-between-circles-and-lines
    
    float vx = line_p2.x - line_p1.x;
    float vy = line_p2.y - line_p1.y;
    float xdiff = line_p1.x - circle.x;
    float ydiff = line_p1.y - circle.y;
    float a = vx * vx + vy * vy;
    float b = 2 * ((vx * xdiff) + (vy * ydiff));
    float c = xdiff * xdiff + ydiff * ydiff - radius * radius;
    float quad = b * b - (4 * a * c);
    if(quad >= 0) {
        //An infinite collision is happening, but let's not stop here
        float quadsqrt = sqrt(quad);
        for (int i = -1; i <= 1; i += 2) {
            //Returns the two coordinates of the intersection points
            float t = (i * -b + quadsqrt) / (2 * a);
            float x = line_p1.x + (i * vx * t);
            float y = line_p1.y + (i * vy * t);
            //If one of them is in the boundaries of the segment, it collides
            if(
                x >= std::min(line_p1.x, line_p2.x) &&
                x <= std::max(line_p1.x, line_p2.x) &&
                y >= std::min(line_p1.y, line_p2.y) &&
                y <= std::max(line_p1.y, line_p2.y)
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
 * Returns whether a circle is touching a rotated rectangle or not.
 * This includes being completely inside the rectangle.
 * circle:
 *   Coordinates of the circle.
 * radius:
 *   Radius of the circle.
 * rectangle:
 *   Central coordinates of the rectangle.
 * rect_dim:
 *   Dimensions of the rectangle.
 * rect_angle:
 *   Angle the rectangle is facing.
 * overlap_dist:
 *   If not NULL, the amount of overlap is returned here.
 * rectangle_side_angle:
 *   If not NULL, the angle of the side of the rectangle
 *   that the circle is on, aligned to the sides of the rectangle, is
 *   returned here.
 */
bool circle_intersects_rectangle(
    const point &circle, const float radius,
    const point &rectangle, const point &rect_dim,
    const float rect_angle,
    float* overlap_dist, float* rectangle_side_angle
) {
    point circle_rel_pos = circle - rectangle;
    circle_rel_pos = rotate_point(circle_rel_pos, -rect_angle);
    point nearest;
    
    bool inside_x =
        circle_rel_pos.x > -rect_dim.x / 2.0 &&
        circle_rel_pos.x < rect_dim.x / 2.0;
    bool inside_y =
        circle_rel_pos.y > -rect_dim.y / 2.0 &&
        circle_rel_pos.y < rect_dim.y / 2.0;
        
    if(inside_x && inside_y) {
        point dist_to_pos(
            rect_dim.x / 2.0 - circle_rel_pos.x,
            rect_dim.y / 2.0 - circle_rel_pos.y
        );
        point dist_to_neg(
            -(-rect_dim.x / 2.0 - circle_rel_pos.x),
            -(-rect_dim.y / 2.0 - circle_rel_pos.y)
        );
        float smallest_x = std::min(dist_to_neg.x, dist_to_pos.x);
        float smallest_y = std::min(dist_to_neg.y, dist_to_pos.y);
        float smallest = std::min(smallest_x, smallest_y);
        
        if(smallest == dist_to_pos.x) {
            nearest = point(rect_dim.x / 2, circle_rel_pos.y);
        } else if(smallest == dist_to_neg.x) {
            nearest = point(-rect_dim.x / 2, circle_rel_pos.y);
        } else if(smallest == dist_to_pos.y) {
            nearest = point(circle_rel_pos.x, rect_dim.y / 2);
        } else if(smallest == dist_to_neg.y) {
            nearest = point(circle_rel_pos.x, -rect_dim.y / 2);
        }
    } else {
        nearest =
            point(
                clamp(circle_rel_pos.x, -rect_dim.x / 2.0, rect_dim.x / 2.0),
                clamp(circle_rel_pos.y, -rect_dim.y / 2.0, rect_dim.y / 2.0)
            );
    }
    
    float d = dist(circle_rel_pos, nearest).to_float();
    if(overlap_dist) {
        if(inside_x && inside_y) {
            *overlap_dist = d + radius;
        } else {
            *overlap_dist = radius - d;
        }
    }
    
    if(rectangle_side_angle) {
        float angle;
        if(inside_x && inside_y) {
            angle = get_angle(circle_rel_pos, nearest);
        } else {
            angle = get_angle(nearest, circle_rel_pos);
        }
        
        angle = floor((angle + (TAU / 8)) / (TAU / 4)) * (TAU / 4);
        *rectangle_side_angle = angle + rect_angle;
    }
    
    if(inside_x && inside_y) {
        return true;
    }
    
    return d < radius;
}


/* ----------------------------------------------------------------------------
 * Returns whether the two line segments, which are known to be collinear,
 * are intersecting.
 * a:
 *   Starting point of the first line segment.
 * b:
 *   Ending point of the first line segment.
 * c:
 *   Starting point of the second line segment.
 * d:
 *   Ending point of the second line segment.
 * intersection_tl:
 *   If not NULL, and if there is an intersection, return the top-left
 *   corner of the intersection here.
 * intersection_br:
 *   If not NULL, and if there is an intersection, return the bottom-right
 *   corner of the intersection here.
 */
bool collinear_lines_intersect(
    const point &a, const point &b, const point &c, const point &d,
    point* intersection_tl, point* intersection_br
) {
    point min1(std::min(a.x, b.x), std::min(a.y, b.y));
    point max1(std::max(a.x, b.x), std::max(a.y, b.y));
    point min2(std::min(c.x, d.x), std::min(c.y, d.y));
    point max2(std::max(c.x, d.x), std::max(c.y, d.y));
    
    point i_tl(std::max(min1.x, min2.x), std::max(min1.y, min2.y));
    point i_br(std::min(max1.x, max2.x), std::min(max1.y, max2.y));
    
    if(i_tl.x == i_br.x && i_tl.y == i_br.y) {
        //Special case -- they share just one point. Let it slide.
        return false;
    }
    
    if(i_tl.x <= i_br.x && i_tl.y <= i_br.y) {
        if(intersection_tl) *intersection_tl = i_tl;
        if(intersection_br) *intersection_br = i_br;
        return true;
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the angle and magnitude of vector coordinates.
 * coordinates:
 *   The coordinates.
 * angle:
 *   Variable to return the angle to.
 * magnitude:
 *   Variable to return the magnitude to.
 */
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
) {
    if(angle) {
        *angle = atan2(coordinates.y, coordinates.x);
    }
    if(magnitude) {
        *magnitude = dist(point(0, 0), coordinates).to_float();
    }
}


/* ----------------------------------------------------------------------------
 * Converts an angle from degrees to radians.
 * deg:
 *   Angle, in degrees.
 */
float deg_to_rad(const float deg) {
    return (M_PI / 180.0f) * deg;
}


/* ----------------------------------------------------------------------------
 * Returns the angle between two points.
 * In other words, this is the angle "center" is facing when it is looking
 * at "focus".
 * center:
 *   Center point.
 * focus:
 *   Point that the center is focusing on.
 */
float get_angle(const point &center, const point &focus) {
    return atan2(focus.y - center.y, focus.x - center.x);
}


/* ----------------------------------------------------------------------------
 * Returns the clockwise distance between a1 and a2, in radians.
 * a1:
 *   First angle.
 * a2:
 *   Second angle.
 */
float get_angle_cw_dif(float a1, float a2) {
    a1 = normalize_angle(a1);
    a2 = normalize_angle(a2);
    if(a1 > a2) a1 -= TAU;
    return a2 - a1;
}


/* ----------------------------------------------------------------------------
 * Returns the smallest distance between two angles.
 * a1:
 *   First angle.
 * a2:
 *   Second angle.
 */
float get_angle_smallest_dif(const float a1, const float a2) {
    return
        M_PI - std::abs(
            std::abs(normalize_angle(a1) - normalize_angle(a2)) - M_PI
        );
}


/* ----------------------------------------------------------------------------
 * Returns the closest point in a line segment to a given point.
 * l1:
 *   Starting point of the line segment.
 * l2:
 *   Ending point of the line segment.
 * p:
 *   Reference point.
 * segment_ratio:
 *   If not NULL, the ratio from l1 to l2 is returned here.
 *   Between 0 and 1, it belongs to the line segment. If not, it doesn't.
 */
point get_closest_point_in_line(
    const point &l1, const point &l2, const point &p, float* segment_ratio
) {

    //Code by http://stackoverflow.com/a/3122532
    
    point l1_to_p = p - l1;
    point l1_to_l2 = l2 - l1;
    
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
 * Returns the closest point in a rotated rectangle's perimeter
 * to the specified point. This only happens if the point is outside the
 * rectangle, otherwise the reference point's coordinates are returned instead.
 * p:
 *   Reference point.
 * rect_center:
 *   Center of the rectangle.
 * rect_dim:
 *   Width and height of the rectangle.
 * rect_angle:
 *   Angle of the rectangle.
 * is_inside:
 *   If true, returns whether or not the reference point is inside
 *   the rectangle.
 */
point get_closest_point_in_rotated_rectangle(
    const point &p,
    const point &rect_center, const point &rect_dim, const float rect_angle,
    bool* is_inside
) {
    point closest_point;
    point perimeter = rect_dim / 2.0f;
    if(is_inside) *is_inside = false;
    
    //First, transform the coordinates so the rectangle is axis-aligned, and
    //the rectangle's center is at the origin.
    point delta_p = p - rect_center;
    delta_p = rotate_point(delta_p, -rect_angle);
    
    //Check the closest point.
    if(delta_p.x <= -perimeter.x) {
        if(delta_p.y <= -perimeter.y) {
            //Top-left corner.
            closest_point = point(-perimeter.x, -perimeter.y);
        } else if(delta_p.y >= perimeter.y) {
            //Bottom-left corner.
            closest_point = point(-perimeter.x, perimeter.y);
        } else {
            //Left side.
            closest_point = point(-perimeter.x, delta_p.y);
        }
    } else if(delta_p.x >= perimeter.x) {
        if(delta_p.y <= -perimeter.y) {
            //Top-right corner.
            closest_point = point(perimeter.x, -perimeter.y);
        } else if(delta_p.y >= perimeter.y) {
            //Bottom-right corner.
            closest_point = point(perimeter.x, perimeter.y);
        } else {
            //Right side.
            closest_point = point(perimeter.x, delta_p.y);
        }
    } else if(delta_p.y <= -perimeter.y) {
        //Top side.
        closest_point = point(delta_p.x, -perimeter.y);
    } else if(delta_p.y >= perimeter.y) {
        //Bottom side.
        closest_point = point(delta_p.x, perimeter.y);
    } else {
        //Inside.
        closest_point = delta_p;
        if(is_inside) *is_inside = true;
    }
    
    //Now, transform back.
    closest_point = rotate_point(closest_point, rect_angle);
    return closest_point + rect_center;
}


/* ----------------------------------------------------------------------------
 * Returns a point's sign on a line segment,
 * used for detecting if it's inside a triangle.
 * p:
 *   The point to check.
 * lp1:
 *   Starting point of the line segment.
 * lp2:
 *   Ending point of the line segment.
 */
float get_point_sign(const point &p, const point &lp1, const point &lp2) {
    return (p.x - lp2.x) * (lp1.y - lp2.y) - (lp1.x - lp2.x) * (p.y - lp2.y);
}


/* ----------------------------------------------------------------------------
 * Gets the bounding box coordinates of a rectangle that has undergone
 * translation, scale, and/or rotation transformations, and places it
 * in the specified point structs.
 * center:
 *   Center point of the rectangle.
 * dimensions:
 *   The rectangle's width and height.
 * angle:
 *   Angle of rotation.
 * min_coords:
 *   The top-left coordinates are returned here.
 * max_coords:
 *   The bottom-right coordinates are returned here.
 */
void get_transformed_rectangle_bounding_box(
    const point &center, const point &dimensions, const float angle,
    point* min_coords, point* max_coords
) {

    if(!min_coords || !max_coords) return;
    bool got_min_x = false;
    bool got_max_x = false;
    bool got_min_y = false;
    bool got_max_y = false;
    
    for(unsigned char p = 0; p < 4; ++p) {
        point corner, final_corner;
        
        if(p == 0 || p == 1) corner.x = center.x - (dimensions.x * 0.5);
        else                 corner.x = center.x + (dimensions.x * 0.5);
        if(p == 0 || p == 2) corner.y = center.y - (dimensions.y * 0.5);
        else                 corner.y = center.y + (dimensions.y * 0.5);
        
        corner -= center;
        final_corner = rotate_point(corner, angle);
        final_corner += center;
        
        if(final_corner.x < min_coords->x || !got_min_x) {
            min_coords->x = final_corner.x;
            got_min_x = true;
        }
        if(final_corner.y < min_coords->y || !got_min_y) {
            min_coords->y = final_corner.y;
            got_min_y = true;
        }
        if(final_corner.x > max_coords->x || !got_max_x) {
            max_coords->x = final_corner.x;
            got_max_x = true;
        }
        if(final_corner.y > max_coords->y || !got_max_y) {
            max_coords->y = final_corner.y;
            got_max_y = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a triangle or not.
 * p:
 *   The point to check.
 * tp1:
 *   First point of the triangle.
 * tp2:
 *   Second point of the triangle.
 * tp3:
 *   Third point of the triangle.
 * loq:
 *   If true, use a "less or equal" comparison.
 *   Different code requires different precision for on-line cases.
 *   Just...don't overthink this, I added this based on what worked and didn't.
 */
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    const bool loq
) {

    //https://stackoverflow.com/q/2049582
    
    bool b1, b2, b3;
    
    float f1, f2, f3;
    
    f1 = get_point_sign(p, tp1, tp2);
    f2 = get_point_sign(p, tp2, tp3);
    f3 = get_point_sign(p, tp3, tp1);
    
    if(loq) {
        b1 = f1 <= 0.0f;
        b2 = f2 <= 0.0f;
        b3 = f3 <= 0.0f;
    } else {
        b1 = f1 < 0.0f;
        b2 = f2 < 0.0f;
        b3 = f3 < 0.0f;
    }
    
    return ((b1 == b2) && (b2 == b3));
}


/* ----------------------------------------------------------------------------
 * Returns whether a line segment intersects with a rotated rectangle or not.
 * lp1:
 *   First point of the line segment.
 * lp2:
 *   Second point of the line segment.
 * rect_center:
 *   Center point of the rectangle.
 * rect_dim:
 *   Width and height of the rectangle.
 * rect_angle:
 *   Angle of the rectangle.
 */
bool line_segment_intersects_rotated_rectangle(
    const point &lp1, const point &lp2,
    const point &rect_center, const point &rect_dim, const float rect_angle
) {
    //First, transform the coordinates so the rectangle is axis-aligned, and
    //the rectangle's center is at the origin.
    point delta_p1 = lp1 - rect_center;
    delta_p1 = rotate_point(delta_p1, -rect_angle);
    point delta_p2 = lp2 - rect_center;
    delta_p2 = rotate_point(delta_p2, -rect_angle);
    
    //Now, check if the line intersects the rectangle.
    point half_dim = rect_dim / 2.0f;
    //Right side.
    if(
        line_segments_intersect(
            delta_p1,
            delta_p2,
            point(half_dim.x, -half_dim.y),
            point(half_dim.x, half_dim.y),
            NULL
        )
    ) {
        return true;
    }
    
    //Top side.
    if(
        line_segments_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, -half_dim.y),
            point(half_dim.x, -half_dim.y),
            NULL
        )
    ) {
        return true;
    }
    
    //Left side.
    if(
        line_segments_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, -half_dim.y),
            point(-half_dim.x, half_dim.y),
            NULL
        )
    ) {
        return true;
    }
    
    //Bottom side.
    if(
        line_segments_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, half_dim.y),
            point(half_dim.x, half_dim.y),
            NULL
        )
    ) {
        return true;
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether the two line segments are collinear.
 * a:
 *   Starting point of the first line segment.
 * b:
 *   Ending point of the first line segment.
 * c:
 *   Starting point of the second line segment.
 * d:
 *   Ending point of the second line segment.
 */
bool line_segments_are_collinear(
    const point &a, const point &b, const point &c, const point &d
) {
    return
        points_are_collinear(a, b, c) &&
        points_are_collinear(a, b, d);
}


/* ----------------------------------------------------------------------------
 * Returns whether the two line segments intersect.
 * l1p1:
 *   Starting point of the first line segment.
 * l1p2:
 *   Ending point of the first line segment.
 * l2p1:
 *   Starting point of the second line segment.
 * l2p2:
 *   Ending point of the second line segment.
 * final_l1r:
 *   If not NULL and they intersect, returns the distance from
 *   the start of line 1 in which the intersection happens.
 *   This is a ratio, so 0 is the start, 1 is the end of the line.
 * final_l2r:
 *   Same as final_l1r, but for line 2.
 */
bool line_segments_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* final_l1r, float* final_l2r
) {
    float l1r = 0.0f;
    float l2r = 0.0f;
    bool result = lines_intersect(l1p1, l1p2, l2p1, l2p2, &l1r, &l2r);
    
    if(final_l1r) *final_l1r = l1r;
    if(final_l2r) *final_l2r = l2r;
    
    if(result) {
        //Return whether they intersect at the segments.
        return
            l1r >= 0 && l1r <= 1 &&
            l2r >= 0 && l2r <= 1;
    } else {
        return false;
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether the two line segments intersect.
 * l1p1:
 *   Starting point of the first line segment.
 * l1p2:
 *   Ending point of the first line segment.
 * l2p1:
 *   Starting point of the second line segment.
 * l2p2:
 *   Ending point of the second line segment.
 * intersection:
 *   Return the intersection point here, if not NULL.
 */
bool line_segments_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* intersection
) {
    float r;
    if(intersection) {
        intersection->x = 0.0f;
        intersection->y = 0.0f;
    }
    if(!line_segments_intersect(l1p1, l1p2, l2p1, l2p2, &r, NULL)) return false;
    if(intersection) {
        intersection->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        intersection->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Converts linear distance to angular distance.
 * linear_dist:
 *   Linear distance.
 * radius:
 *   Radius of the circle.
 */
float linear_dist_to_angular(const float linear_dist, const float radius) {
    return 2 * atan(linear_dist / (2 * radius));
}


/* ----------------------------------------------------------------------------
 * Returns whether two lines (not line segments) intersect, and returns
 * information about where it happens.
 * l1p1:
 *   Starting point of the first line segment.
 * l1p2:
 *   Ending point of the first line segment.
 * l2p1:
 *   Starting point of the second line segment.
 * l2p2:
 *   Ending point of the second line segment.
 * final_l1r:
 *   If not NULL and they intersect, returns the distance from
 *   the start of line 1 in which the intersection happens.
 *   This is a ratio, so 0 is the start, 1 is the end of the line.
 * final_l2r:
 *   Same as final_l1r, but for line 2.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    float* final_l1r, float* final_l2r
) {
    float div =
        (l2p2.y - l2p1.y) * (l1p2.x - l1p1.x) -
        (l2p2.x - l2p1.x) * (l1p2.y - l1p1.y);
        
    if(div != 0.0f) {
        //They intersect.
        
        if(final_l1r) {
            //Calculate the intersection distance from the start of line 1.
            *final_l1r =
                (
                    (l2p2.x - l2p1.x) * (l1p1.y - l2p1.y) -
                    (l2p2.y - l2p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        if(final_l2r) {
            //Calculate the intersection distance from the start of line 2.
            *final_l2r =
                (
                    (l1p2.x - l1p1.x) * (l1p1.y - l2p1.y) -
                    (l1p2.y - l1p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        return true;
        
    } else {
        //They don't intersect.
        
        if(final_l1r) *final_l1r = 0.0f;
        if(final_l2r) *final_l2r = 0.0f;
        
        return false;
        
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether two lines (not line segments) intersect, and returns
 * information about where it happens.
 * l1p1:
 *   Starting point of the first line segment.
 * l1p2:
 *   Ending point of the first line segment.
 * l2p1:
 *   Starting point of the second line segment.
 * l2p2:
 *   Ending point of the second line segment.
 * final_point:
 *   If not NULL and they intersect,
 *   returns the coordinates of where it happens.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    point* final_point
) {
    if(final_point) {
        final_point->x = 0.0f;
        final_point->y = 0.0f;
    }
    
    float r = 0.0f;
    if(!lines_intersect(l1p1, l1p2, l2p1, l2p2, &r, NULL)) {
        return false;
    }
    
    if(final_point) {
        final_point->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        final_point->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the movement necessary to move a point.
 * start:
 *   Coordinates of the initial point.
 * target:
 *   Coordinates of the target point.
 * speed:
 *   Speed at which the point can move.
 * reach_radius:
 *   If the point is within this range of the target,
 *   consider it as already being there.
 * mov:
 *   Variable to return the amount of movement to.
 * angle:
 *   Variable to return the angle the point faces to.
 * reached:
 *   Variable to return whether the point reached the target.
 * delta_t:
 *   Duration of the current tick.
 */
void move_point(
    const point &start, const point &target,
    const float speed, const float reach_radius,
    point* mov, float* angle, bool* reached, const float delta_t
) {
    point dif = target - start;
    float dis = sqrt(dif.x * dif.x + dif.y * dif.y);
    
    if(dis > reach_radius) {
        float move_amount =
            std::min((double) (dis / delta_t / 2.0f), (double) speed);
            
        dif *= (move_amount / dis);
        
        if(mov) *mov = dif;
        if(angle) *angle = atan2(dif.y, dif.x);
        if(reached) *reached = false;
        
    } else {
    
        if(mov) *mov = point();
        if(reached) *reached = true;
    }
}


/* ----------------------------------------------------------------------------
 * Normalizes an angle so that it's between 0 and TAU (M_PI * 2).
 * a:
 *   Angle to normalize.
 */
float normalize_angle(float a) {
    a = fmod(a, (float) TAU);
    if(a < 0) a += TAU;
    return a;
}


/* ----------------------------------------------------------------------------
 * Returns whether three given points are collinear or not.
 * a:
 *   First point.
 * b:
 *   Second point.
 * c:
 *   Third point.
 */
bool points_are_collinear(
    const point &a, const point &b, const point &c
) {
    //https://math.stackexchange.com/a/405981
    return
        (b.y - a.y) * (c.x - b.x) ==
        (c.y - b.y) * (b.x - a.x);
}


/* ----------------------------------------------------------------------------
 * Converts an angle from radians to degrees.
 * rad:
 *   Angle, in radians.
 */
float rad_to_deg(const float rad) {
    return (180.0f / M_PI) * rad;
}


/* ----------------------------------------------------------------------------
 * Returns whether a rectangle intersects with a line segment.
 * Also returns true if the line is fully inside the rectangle.
 * r1:
 *   Top-left corner of the rectangle.
 * r2:
 *   Bottom-right corner of the rectangle.
 * l1:
 *   Starting point of the line segment.
 * l2:
 *   Ending point of the line segment.
 */
bool rectangle_intersects_line(
    const point &r1, const point &r2,
    const point &l1, const point &l2
) {
    //Line crosses left side?
    if(
        line_segments_intersect(
            l1, l2, point(r1.x, r1.y), point(r1.x, r2.y), NULL, NULL
        )
    ) {
        return true;
    }
    //Line crosses right side?
    if(
        line_segments_intersect(
            l1, l2, point(r2.x, r1.y), point(r2.x, r2.y), NULL, NULL
        )
    ) {
        return true;
    }
    //Line crosses top side?
    if(
        line_segments_intersect(
            l1, l2, point(r1.x, r1.y), point(r2.x, r1.y), NULL, NULL
        )
    ) {
        return true;
    }
    //Line crosses bottom side?
    if(
        line_segments_intersect(
            l1, l2, point(r1.x, r2.y), point(r2.x, r2.y), NULL, NULL
        )
    ) {
        return true;
    }
    
    //Are both points inside the rectangle?
    if(
        (l1.x >= r1.x && l2.x >= r1.x) &&
        (l1.x <= r2.x && l2.x <= r2.x) &&
        (l1.y >= r1.y && l2.y >= r1.y) &&
        (l1.y <= r2.y && l2.y <= r2.y)
    ) {
        return true;
    }
    
    return false;
    
}


/* ----------------------------------------------------------------------------
 * Checks if two rectangles are colliding.
 * tl1:
 *   Coordinates of the first box's top-left.
 * br1:
 *   Coordinates of the first box's bottom-right.
 * tl2:
 *   Coordinates of the second box's top-left.
 * br2:
 *   Coordinates of the second box's bottom-right.
 */
bool rectangles_intersect(
    const point &tl1, const point &br1,
    const point &tl2, const point &br2
) {
    if(tl1.x > br2.x) return false;
    if(br1.x < tl2.x) return false;
    if(tl1.y > br2.y) return false;
    if(br1.y < tl2.y) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Rotates a point by an angle.
 * The x and y are meant to represent the difference
 * between the point and the center of the rotation.
 * coords:
 *   Coordinates to rotate.
 * angle:
 *   Angle to rotate by.
 */
point rotate_point(const point &coords, const float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return point(c * coords.x - s * coords.y, s * coords.x + c * coords.y);
}
