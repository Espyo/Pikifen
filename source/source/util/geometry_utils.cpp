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
#include <cfloat>
#include <cmath>
#include <math.h>
#include <vector>

#include "geometry_utils.h"
#include "math_utils.h"
#include "string_utils.h"

using std::vector;


/**
 * @brief Constructs a new dist object, given two points.
 *
 * @param p1 First point.
 * @param p2 Second point.
 */
dist::dist(const point &p1, const point &p2) :
    distance_squared(
        (p2.x - p1.x) * (p2.x - p1.x) +
        (p2.y - p1.y) * (p2.y - p1.y)
    ) {
    
}


/**
 * @brief Constructs a new dist object, given a non-squared distance.
 *
 * @param d Regular, non-squared distance.
 */
dist::dist(float d) :
    distance_squared(d * d),
    normal_distance(d),
    has_normal_distance(true) {
    
}


/**
 * @brief Sets the value given a non-squared distance.
 *
 * @param d Regular, non-squared distance.
 * @return The current object.
 */
dist &dist::operator =(float d) {
    distance_squared = d * d;
    normal_distance = d;
    has_normal_distance = true;
    return *this;
}


/**
 * @brief Checks if this distance is smaller than the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is smaller.
 */
bool dist::operator<(float d2) const {
    return distance_squared < (d2 * d2);
}


/**
 * @brief Checks if this distance is larger than the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is larger.
 */
bool dist::operator>(float d2) const {
    return distance_squared > (d2 * d2);
}


/**
 * @brief Checks if this distance is the same as the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is the same.
 */
bool dist::operator==(float d2) const {
    return distance_squared == (d2 * d2);
}


/**
 * @brief Checks if this distance is smaller than or equal to the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is smaller or equal.
 */
bool dist::operator<=(float d2) const {
    return !operator>(d2);
}


/**
 * @brief Checks if this distance is larger than or equal to the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is larger or equal.
 */
bool dist::operator>=(float d2) const {
    return !operator<(d2);
}


/**
 * @brief Checks if this distance is different from the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is different.
 */
bool dist::operator!=(float d2) const {
    return !operator==(d2);
}


/**
 * @brief Checks if this distance is smaller than the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is smaller.
 */
bool dist::operator<(const dist &d2) const {
    return distance_squared < d2.distance_squared;
}


/**
 * @brief Checks if this distance is larger than the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is larger.
 */
bool dist::operator>(const dist &d2) const {
    return distance_squared > d2.distance_squared;
}


/**
 * @brief Checks if this distance is the same as the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is the same.
 */
bool dist::operator==(const dist &d2) const {
    return distance_squared == d2.distance_squared;
}


/**
 * @brief Checks if this distance is smaller than or equal to the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is smaller or equal.
 */
bool dist::operator<=(const dist &d2) const {
    return !operator>(d2);
}


/**
 * @brief Checks if this distance is larger than or equal to the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is larger or equal.
 */
bool dist::operator>=(const dist &d2) const {
    return !operator<(d2);
}


/**
 * @brief Checks if this distance is different from the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is different.
 */
bool dist::operator!=(const dist &d2) const {
    return !operator==(d2);
}


/**
 * @brief Adds some distance to the current one.
 *
 * @param d2 Amount of distance to add.
 */
void dist::operator+=(float d2) {
    if(!has_normal_distance) {
        normal_distance = (float) sqrt(distance_squared);
        has_normal_distance = true;
    }
    normal_distance += d2;
    distance_squared = normal_distance * normal_distance;
}


/**
 * @brief Adds some distance to the current one.
 *
 * @param d2 Amount of distance to add.
 */
void dist::operator+=(const dist &d2) {
    distance_squared += d2.distance_squared;
    if(has_normal_distance) {
        if(d2.has_normal_distance) {
            normal_distance += d2.normal_distance;
        } else {
            normal_distance = (float) sqrt(distance_squared);
        }
    }
}


/**
 * @brief Removes some distance from the current one.
 *
 * @param d2 Amount of distance to remove.
 */
void dist::operator-=(float d2) {
    operator+=(-d2);
}


/**
 * @brief Removes some distance from the current one.
 *
 * @param d2 Amount of distance to remove.
 */
void dist::operator-=(const dist &d2) {
    distance_squared -= d2.distance_squared;
    if(has_normal_distance) {
        if(d2.has_normal_distance) {
            normal_distance -= d2.normal_distance;
        } else {
            normal_distance = (float) sqrt(distance_squared);
        }
    }
}


/**
 * @brief Returns the regular, non-squared distance as a number.
 *
 * @return The non-squared distance.
 */
float dist::to_float() {
    if(!has_normal_distance) {
        normal_distance = (float) sqrt(distance_squared);
        has_normal_distance = true;
    }
    return normal_distance;
}


/**
 * @brief Constructs a new point object, given its coordinates.
 *
 * @param x X coordinate.
 * @param y Y coordinate.
 */
point::point(float x, float y) :
    x(x),
    y(y) {
    
}


/**
 * @brief Constructs a new point object, with the given value on both
 * coordinates.
 *
 * @param xy X and Y coordinate.
 */
point::point(float xy) :
    x(xy),
    y(xy) {
    
}


/**
 * @brief Constructs a new point object, with the coordinates set to 0,0.
 */
point::point() {
}


/**
 * @brief Adds the coordinates of two points.
 *
 * @param p Point with values to add with.
 * @return The new point.
 */
const point point::operator +(const point &p) const {
    return point(x + p.x, y + p.y);
}


/**
 * @brief Subtracts the coordinates of two points.
 *
 * @param p Point with values to subtract with.
 * @return The new point.
 */
const point point::operator -(const point &p) const {
    return point(x - p.x, y - p.y);
}


/**
 * @brief Multiplies the coordinates of two points.
 *
 * @param p Point with values to multiply with.
 * @return The new point.
 */
const point point::operator *(const point &p) const {
    return point(x * p.x, y * p.y);
}


/**
 * @brief Divides the coordinates of two points.
 *
 * @param p Point with values to divide with.
 * @return The new point.
 */
const point point::operator /(const point &p) const {
    return point(x / p.x, y / p.y);
}


/**
 * @brief Adds a number to the coordinates.
 *
 * @param n Number to add to both coordinates.
 * @return The new point.
 */
const point point::operator +(float n) const {
    return point(x + n, y + n);
}


/**
 * @brief Subtracts a number from each coordinates.
 *
 * @param n Number to subtract from both coordinates.
 * @return The new point.
 */
const point point::operator -(float n) const {
    return point(x - n, y - n);
}


/**
 * @brief Multiplies the coordinates by a number.
 *
 * @param n Value to multiply both coordinates with.
 * @return The new point.
 */
const point point::operator *(float n) const {
    return point(x * n, y * n);
}


/**
 * @brief Divides the coordinates by a number.
 *
 * @param n Number to divide both coordinates with.
 * @return The new point.
 */
const point point::operator /(float n) const {
    return point(x / n, y / n);
}


/**
 * @brief Adds the coordinates of another point to this one's.
 *
 * @param p Point with the values to add with.
 * @return The current object.
 */
point point::operator +=(const point &p) {
    x += p.x;
    y += p.y;
    return point(x, y);
}


/**
 * @brief Subtracts the coordinates of another point to this one's.
 *
 * @param p Point with the values to subtract with.
 * @return The current object.
 */
point point::operator -=(const point &p) {
    x -= p.x;
    y -= p.y;
    return point(x, y);
}


/**
 * @brief Multiplies the coordinates of another point with this one's.
 *
 * @param p Point with the values to multiply by.
 * @return The current object.
 */
point point::operator *=(const point &p) {
    x *= p.x;
    y *= p.y;
    return point(x, y);
}


/**
 * @brief Divides the coordinates of another point with this one's.
 *
 * @param p Point with the values to divide by.
 * @return The current object.
 */
point point::operator /=(const point &p) {
    x /= p.x;
    y /= p.y;
    return point(x, y);
}


/**
 * @brief Adds a given number to the coordinates.
 *
 * @param n Value to add to both coordinates with.
 * @return The current object.
 */
point point::operator +=(float n) {
    x += n;
    y += n;
    return point(x, y);
}


/**
 * @brief Subtracts a given number from the coordinates.
 *
 * @param n Value to subtract from both coordinates with.
 * @return The current object.
 */
point point::operator -=(float n) {
    x -= n;
    y -= n;
    return point(x, y);
}


/**
 * @brief Multiplies the coordinates by a given number.
 *
 * @param n Value to multiply both coordinates with.
 * @return The current object.
 */
point point::operator *=(float n) {
    x *= n;
    y *= n;
    return point(x, y);
}


/**
 * @brief Divides the coordinates by a given number.
 *
 * @param n Value to divide both coordinates with.
 * @return The current object.
 */
point point::operator /=(float n) {
    x /= n;
    y /= n;
    return point(x, y);
}


/**
 * @brief Compares if two points are the same.
 *
 * @param p Other point to compare against.
 * @return Whether they are the same.
 */
bool point::operator ==(const point &p) const {
    return x == p.x && y == p.y;
}


/**
 * @brief Compares if two points are different.
 *
 * @param p Other point to compare against.
 * @return Whether they are different.
 */
bool point::operator !=(const point &p) const {
    return x != p.x || y != p.y;
}



/**
 * @brief Returns the vector coordinates of an angle.
 *
 * @param angle The angle.
 * @param magnitude Its magnitude.
 * @return The coordinates.
 */
point angle_to_coordinates(
    float angle, float magnitude
) {
    return
        point(
            (float) cos(angle) * magnitude,
            (float) sin(angle) * magnitude
        );
}


/**
 * @brief Converts angular distance to linear distance.
 *
 * @param angular_dist Angular distance value.
 * @param radius Radius of the circle.
 * @return The linear distance.
 */
float angular_dist_to_linear(float angular_dist, float radius) {
    return (float) (2 * radius * tan(angular_dist / 2));
}


/**
 * @brief Checks if two spheres are colliding via a bounding-box check.
 *
 * @param center1 Coordinates of the first sphere.
 * @param center2 Coordinates of the second sphere.
 * @param r Range of the bounding box.
 * @return Whether they are colliding.
 */
bool bbox_check(const point &center1, const point &center2, float r) {
    return
        (
            fabs(center1.x - center2.x) <= r &&
            fabs(center1.y - center2.y) <= r
        );
}


/**
 * @brief Checks if a rectangle and a sphere are colliding via a
 * bounding-box check.
 *
 * @param tl1 Top-left coordinates of the rectangle.
 * @param br1 Bottom-right coordinates of the rectangle.
 * @param center2 Coordinates of the sphere.
 * @param r Radius of the sphere.
 * @return Whether they are colliding.
 */
bool bbox_check(
    const point &tl1, const point &br1,
    const point &center2, float r
) {
    return
        rectangles_intersect(
            tl1, br1,
            center2 - r, center2 + r
        );
}


/**
 * @brief Calculates the required horizontal and vertical speed in order to
 * throw something to the specified coordinates, such that it reaches a
 * specific peak height.
 *
 * If the calculation is impossible (like if the peak height is lower than the
 * starting height), the speed variables will all be set to 0.
 *
 * @param start_xy Starting X and Y coordinates.
 * @param start_z Starting Z coordinate.
 * @param target_xy Target destination's X and Y coordinates.
 * @param target_z Target destination's Z coordinate.
 * @param max_h Maximum height, using the starting Z as the reference.
 * @param gravity Constant for the force of gravity, in units per
 * second squared.
 * @param req_speed_xy The required X and Y speed is returned here.
 * @param req_speed_z The required Z speed is returned here.
 * @param out_h_angle If not nullptr, the final horizontal angle is
 * returned here.
 */
void calculate_throw(
    const point &start_xy, float start_z,
    const point &target_xy, float target_z,
    float max_h, float gravity,
    point* req_speed_xy, float* req_speed_z, float* out_h_angle
) {

    if(target_z - start_z > max_h) {
        //If the target is above the maximum height it can be thrown...
        //Then this is an impossible throw.
        *req_speed_xy = point();
        *req_speed_z = 0.0f;
        if(out_h_angle) *out_h_angle = 0.0f;
        return;
    }
    
    //Code from https://physics.stackexchange.com/questions/515688
    //First, we calculate stuff in 2D, with horizontal and vertical components
    //only.
    
    //We start with the vertical speed. This will be constant regardless
    //of how far the mob is thrown. In order to reach the required max height,
    //the vertical speed needs to be set thusly:
    *req_speed_z = (float) sqrt(2.0 * (-gravity) * max_h);
    
    //Now that we know the vertical speed, we can figure out how long it takes
    //for the mob to land at the target vertical coordinate. The formula for
    //this can be found on Wikipedia, for instance.
    float height_delta = start_z - target_z;
    //Because of floating point precision problems, the result of the sqrt
    //could end up negative. Let's cap it to zero.
    float sqrt_part =
        std::max(
            0.0f,
            (float)
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
    if(out_h_angle) {
        *out_h_angle = h_angle;
    }
}


/**
 * @brief Returns whether a circle is touching a line segment or not.
 *
 * @param circle Coordinates of the circle.
 * @param radius Radius of the circle.
 * @param line_p1 Starting point of the line segment.
 * @param line_p2 Ending point of the line segment.
 * @param out_lix If not nullptr, the line intersection's X coordinate is
 * returned here.
 * @param out_liy If not nullptr, the line intersection's Y coordinate is
 * returned here.
 * @return Whether they intersect.
 */
bool circle_intersects_line_seg(
    const point &circle, float radius,
    const point &line_p1, const point &line_p2,
    float* out_lix, float* out_liy
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
        //An infinite collision is happening, but let's not stop here.
        float quadsqrt = (float) sqrt(quad);
        for(int i = -1; i <= 1; i += 2) {
            //Returns the two coordinates of the intersection points.
            float t = (i * -b + quadsqrt) / (2 * a);
            float x = line_p1.x + (i * vx * t);
            float y = line_p1.y + (i * vy * t);
            //If one of them is in the boundaries of the segment, it collides.
            if(
                x >= std::min(line_p1.x, line_p2.x) &&
                x <= std::max(line_p1.x, line_p2.x) &&
                y >= std::min(line_p1.y, line_p2.y) &&
                y <= std::max(line_p1.y, line_p2.y)
            ) {
                if(out_lix) *out_lix = x;
                if(out_liy) *out_liy = y;
                return true;
            }
        }
    }
    return false;
}


/**
 * @brief Returns whether a circle is touching a rotated rectangle or not.
 * This includes being completely inside the rectangle.
 *
 * @param circle Coordinates of the circle.
 * @param radius Radius of the circle.
 * @param rectangle Central coordinates of the rectangle.
 * @param rect_dim Dimensions of the rectangle.
 * @param rect_angle Angle the rectangle is facing.
 * @param out_overlap_dist If not nullptr, the amount of overlap is
 * returned here.
 * @param out_rectangle_side_angle If not nullptr, the angle of the side of the
 * rectangle that the circle is on, aligned to the sides of the rectangle, is
 * returned here.
 * @return Whether they intersect.
 */
bool circle_intersects_rectangle(
    const point &circle, float radius,
    const point &rectangle, const point &rect_dim,
    float rect_angle,
    float* out_overlap_dist, float* out_rectangle_side_angle
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
            rect_dim.x / 2.0f - circle_rel_pos.x,
            rect_dim.y / 2.0f - circle_rel_pos.y
        );
        point dist_to_neg(
            -(-rect_dim.x / 2.0f - circle_rel_pos.x),
            -(-rect_dim.y / 2.0f - circle_rel_pos.y)
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
                clamp(circle_rel_pos.x, -rect_dim.x / 2.0f, rect_dim.x / 2.0f),
                clamp(circle_rel_pos.y, -rect_dim.y / 2.0f, rect_dim.y / 2.0f)
            );
    }
    
    float d = dist(circle_rel_pos, nearest).to_float();
    if(out_overlap_dist) {
        if(inside_x && inside_y) {
            *out_overlap_dist = d + radius;
        } else {
            *out_overlap_dist = radius - d;
        }
    }
    
    if(out_rectangle_side_angle) {
        float angle;
        if(inside_x && inside_y) {
            angle = get_angle(circle_rel_pos, nearest);
        } else {
            angle = get_angle(nearest, circle_rel_pos);
        }
        
        angle = (float) floor((angle + (TAU / 8)) / (TAU / 4)) * (TAU / 4);
        *out_rectangle_side_angle = angle + rect_angle;
    }
    
    if(inside_x && inside_y) {
        return true;
    }
    
    return d < radius;
}


/**
 * @brief Returns whether the two line segments, which are known to be
 * collinear, are intersecting.
 *
 * @param a Starting point of the first line segment.
 * @param b Ending point of the first line segment.
 * @param c Starting point of the second line segment.
 * @param d Ending point of the second line segment.
 * @param out_intersection_tl If not nullptr, and if there is an intersection,
 * return the top-left corner of the intersection here.
 * @param out_intersection_br If not nullptr, and if there is an intersection,
 * return the bottom-right corner of the intersection here.
 * @return Whether they intersect.
 */
bool collinear_line_segs_intersect(
    const point &a, const point &b, const point &c, const point &d,
    point* out_intersection_tl, point* out_intersection_br
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
        if(out_intersection_tl) *out_intersection_tl = i_tl;
        if(out_intersection_br) *out_intersection_br = i_br;
        return true;
    }
    
    return false;
}


/**
 * @brief Returns the angle and magnitude of vector coordinates.
 *
 * @param coordinates The coordinates.
 * @param angle Variable to return the angle to.
 * @param magnitude Variable to return the magnitude to.
 */
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
) {
    if(angle) {
        *angle = (float) atan2(coordinates.y, coordinates.x);
    }
    if(magnitude) {
        *magnitude = dist(point(0.0f), coordinates).to_float();
    }
}


/**
 * @brief Converts an angle from degrees to radians.
 *
 * @param deg Angle, in degrees.
 * @return The radians.
 */
float deg_to_rad(float deg) {
    return (float) (M_PI / 180.0f) * deg;
}


/**
 * @brief Returns the dot product between two vectors.
 *
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return The dot product.
 */
float dot_product(const point &v1, const point &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}


/**
 * @brief Returns the angle from the origin and the specified point.
 *
 * @param focus Point that the origin is focusing on.
 * @return The angle.
 */
float get_angle(const point &focus) {
    return (float) atan2(focus.y, focus.x);
}


/**
 * @brief Returns the angle between two points.
 * In other words, this is the angle "center" is facing when it is looking
 * at "focus".
 *
 * @param center Center point.
 * @param focus Point that the center is focusing on.
 * @return The angle.
 */
float get_angle(const point &center, const point &focus) {
    return (float) atan2(focus.y - center.y, focus.x - center.x);
}


/**
 * @brief Returns the clockwise distance between a1 and a2, in radians.
 *
 * @param a1 First angle.
 * @param a2 Second angle.
 * @return The distance.
 */
float get_angle_cw_diff(float a1, float a2) {
    a1 = normalize_angle(a1);
    a2 = normalize_angle(a2);
    if(a1 > a2) a1 -= TAU;
    return a2 - a1;
}


/**
 * @brief Returns the smallest distance between two angles.
 *
 * @param a1 First angle.
 * @param a2 Second angle.
 * @return The distance.
 */
float get_angle_smallest_dif(float a1, float a2) {
    return
        (float) M_PI -
        (float) std::abs(
            std::abs(normalize_angle(a1) - normalize_angle(a2)) - M_PI
        );
}


/**
 * @brief Returns the closest point in a line segment to a given point.
 *
 * @param l1 Starting point of the line segment.
 * @param l2 Ending point of the line segment.
 * @param p Reference point.
 * @param out_segment_ratio If not nullptr, the ratio from l1 to l2 is
 * returned here. Between 0 and 1, it belongs to the line segment.
 * If not, it doesn't.
 * @return The closest point.
 */
point get_closest_point_in_line_seg(
    const point &l1, const point &l2, const point &p, float* out_segment_ratio
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
    
    if(out_segment_ratio) *out_segment_ratio = r;
    
    return point(l1.x + l1_to_l2.x * r, l1.y + l1_to_l2.y * r);
}


/**
 * @brief Returns the closest point in a rotated rectangle's perimeter
 * to the specified point. This only happens if the point is outside the
 * rectangle, otherwise the reference point's coordinates are returned instead.
 *
 * @param p Reference point.
 * @param rect_center Center of the rectangle.
 * @param rect_dim Width and height of the rectangle.
 * @param rect_angle Angle of the rectangle.
 * @param out_is_inside If not nullptr, whether or not the reference point
 * is inside the rectangle is returned here.
 * @return The closest point.
 */
point get_closest_point_in_rotated_rectangle(
    const point &p,
    const point &rect_center, const point &rect_dim, float rect_angle,
    bool* out_is_inside
) {
    point closest_point;
    point perimeter = rect_dim / 2.0f;
    if(out_is_inside) *out_is_inside = false;
    
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
        if(out_is_inside) *out_is_inside = true;
    }
    
    //Now, transform back.
    closest_point = rotate_point(closest_point, rect_angle);
    return closest_point + rect_center;
}


/**
 * @brief Given two line segments that share a point, and have some thickness,
 * this returns the location of the inner point and outer point of their
 * miter joint.
 *
 * @param a First point of the first line segment.
 * @param b Common point of both line segments. It is on this point that
 * the miter takes place, meaning this is the point between a and c.
 * @param c Final point of the second line segment.
 * @param thickness Line thickness.
 * @param miter_point_1 The first point is returned here.
 * @param miter_point_2 The second point is returned here.
 * @param max_miter_length If not 0, the miter is limited to this length.
 */
void get_miter_points(
    const point &a, const point &b, const point &c, float thickness,
    point* miter_point_1, point* miter_point_2, float max_miter_length
) {
    //https://blog.scottlogic.com/2019/11/18/drawing-lines-with-webgl.html
    
    //Get the miter point's direction.
    point vec_ab = b - a;
    point vec_bc = c - b;
    point norm_vec_ab = normalize_vector(vec_ab);
    point norm_vec_bc = normalize_vector(vec_bc);
    point tangent = norm_vec_ab + norm_vec_bc;
    point norm_tangent = normalize_vector(tangent);
    point miter_direction(-norm_tangent.y, norm_tangent.x);
    
    //Get the miter point's distance.
    point normal_a(-vec_ab.y, vec_ab.x);
    normal_a = normalize_vector(normal_a);
    float miter_length =
        (thickness / 2.0f) / dot_product(miter_direction, normal_a);
        
    if(isinf(miter_length)) {
        miter_length = 1.0f;
    }
    if(max_miter_length > 0.0f && fabs(miter_length) > max_miter_length) {
        float miter_sign = miter_length >= 0.0f ? 1.0f : -1.0f;
        miter_length =
            std::min((float) fabs(miter_length), max_miter_length);
        miter_length *= miter_sign;
    }
    
    //Return the final point.
    *miter_point_1 = b + miter_direction * miter_length;
    *miter_point_2 = b - miter_direction * miter_length;
}


/**
 * @brief Returns a point's sign on a line segment,
 * used for detecting if it's inside a triangle.
 *
 * @param p The point to check.
 * @param lp1 Starting point of the line segment.
 * @param lp2 Ending point of the line segment.
 * @return The sign.
 */
float get_point_sign(const point &p, const point &lp1, const point &lp2) {
    return (p.x - lp2.x) * (lp1.y - lp2.y) - (lp1.x - lp2.x) * (p.y - lp2.y);
}


/**
 * @brief Returns a point inside of a circular ring. Used when you want multiple
 * points inside the ring, evenly distributed. Which of the points this is
 * is defined by the ratio, which is
 * <current point number> / <total number of points>. The distance from the
 * center point is the mid-point of the inner and outer ring.
 *
 * @param inner_dist Radius of the inner circle of the ring.
 * @param outer_dist Radius of the outer circle of the ring.
 * @param arc Arc of the ring, or M_TAU for the whole ring.
 * @param arc_rot Rotation of the arc.
 * @param ratio Ratio of the current point number.
 * @return The point.
 */
point get_ratio_point_in_ring(
    float inner_dist, float outer_dist,
    float arc, float arc_rot, float ratio
) {

    float radius = (inner_dist + outer_dist) / 2.0f;
    float angle1 = -arc / 2.0f + arc_rot;
    float angle2 = arc / 2.0f + arc_rot;
    float final_angle = (angle2 - angle1) * ratio + angle1;
    
    return angle_to_coordinates(final_angle, radius);
}


/**
 * @brief Returns a random point inside of a rectangular ring, with uniform
 * distribution.
 *
 * @param inner_dist Width and height of the inner rectangle of the ring.
 * @param outer_dist Width and height of the outer rectangle of the ring.
 * @param seed Pointer to the randomness seed to use.
 * @return The point.
 */
point get_random_point_in_rectangular_ring(
    const point &inner_dist, const point &outer_dist, unsigned int* seed
) {
    float ring_thickness[2] {
        outer_dist.x - inner_dist.x,
        outer_dist.y - inner_dist.y
    };
    
    //The idea is to split the ring into four rectangles, organized in a
    //pinwheel pattern.
    //In this pattern, the north and south rectangles have the exact same area,
    //and the same is true for the west and east ones. We can simplify the
    //process with this in mind.
    point rect_sizes[2] = {
        point(
            ring_thickness[0],
            outer_dist.y * 2.0f - ring_thickness[1]
        ),
        point(
            outer_dist.x * 2.0f - ring_thickness[0],
            ring_thickness[1]
        )
    };
    float rect_areas[2] = {
        rect_sizes[0].x* rect_sizes[0].y,
        rect_sizes[1].x* rect_sizes[1].y
    };
    
    //Pick one of the four rectangles (or in this case, one of the two axes),
    //with weighted probability depending on the area.
    size_t chosen_axis;
    if(rect_areas[0] == 0.0f && rect_areas[1] == 0.0f) {
        chosen_axis = randomi(0, 1, seed);
    } else {
        chosen_axis = randomw(vector<float>(rect_areas, rect_areas + 2), seed);
    }
    
    point p_in_rectangle(
        randomf(0.0f, rect_sizes[chosen_axis].x, seed),
        randomf(0.0f, rect_sizes[chosen_axis].y, seed)
    );
    point final_p;
    
    if(chosen_axis == 0) {
        //West or east rectangle. Let's assume the east rectangle.
        final_p.x = inner_dist.x + p_in_rectangle.x,
        final_p.y = -outer_dist.y + p_in_rectangle.y;
    } else {
        //North or south rectangle. Let's assume the south rectangle.
        final_p.x = -inner_dist.x + p_in_rectangle.x;
        final_p.y = inner_dist.y + p_in_rectangle.y;
    }
    
    if(randomi(0, 1, seed) == 0) {
        //Return our point.
        return final_p;
    } else {
        //Swap to the rectangle on the opposite side.
        return point() - final_p;
    }
}


/**
 * @brief Returns a random point inside of a circular ring, with uniform
 * distribution.
 *
 * @param inner_dist Radius of the inner circle of the ring.
 * @param outer_dist Radius of the outer circle of the ring.
 * @param arc Arc of the ring, or M_TAU for the whole ring.
 * @param arc_rot Rotation of the arc.
 * @param seed Pointer to the randomness seed to use.
 * @return The point.
 */
point get_random_point_in_ring(
    float inner_dist, float outer_dist,
    float arc, float arc_rot, unsigned int* seed
) {
    //https://stackoverflow.com/q/30564015
    
    float r =
        inner_dist +
        (outer_dist - inner_dist) * (float) sqrt(randomf(0.0f, 1.0f, seed));
        
    float theta =
        randomf(
            -arc / 2.0f + arc_rot,
            arc / 2.0f + arc_rot,
            seed
        );
        
    return point(r * (float) cos(theta), r * (float) sin(theta));
}


/**
 * @brief Gets the bounding box coordinates of a rectangle that has undergone
 * translation, scale, and/or rotation transformations, and places it
 * in the specified point structs.
 *
 * @param center Center point of the rectangle.
 * @param dimensions The rectangle's width and height.
 * @param angle Angle of rotation.
 * @param min_coords The top-left coordinates are returned here.
 * @param max_coords The bottom-right coordinates are returned here.
 */
void get_transformed_rectangle_bounding_box(
    const point &center, const point &dimensions, float angle,
    point* min_coords, point* max_coords
) {

    if(!min_coords || !max_coords) return;
    bool got_min_x = false;
    bool got_max_x = false;
    bool got_min_y = false;
    bool got_max_y = false;
    
    for(unsigned char p = 0; p < 4; p++) {
        point corner, final_corner;
        
        if(p == 0 || p == 1) corner.x = center.x - (dimensions.x * 0.5f);
        else                 corner.x = center.x + (dimensions.x * 0.5f);
        if(p == 0 || p == 2) corner.y = center.y - (dimensions.y * 0.5f);
        else                 corner.y = center.y + (dimensions.y * 0.5f);
        
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


/**
 * @brief Returns the interpolation between two angles, given a number
 * in an interval.
 *
 * @param input The input number.
 * @param input_start Start of the interval the input number falls on,
 * inclusive. The closer to input_start, the closer the output is to
 * output_start.
 * @param input_end End of the interval the number falls on, inclusive.
 * @param output_start Angle on the starting tip of the interpolation.
 * @param output_end Angle on the ending tip of the interpolation.
 * @return The interpolated angle.
 */
float interpolate_angle(
    float input, float input_start, float input_end,
    float &output_start, float &output_end
) {
    float angle_cw_diff = get_angle_cw_diff(output_start, output_end);
    float angle_delta;
    if(angle_cw_diff < TAU / 2.0f) {
        angle_delta = angle_cw_diff;
    } else {
        angle_delta = -(TAU - angle_cw_diff);
    }
    return
        output_start +
        interpolate_number(input, input_start, input_end, 0, angle_delta);
}


/**
 * @brief Returns the interpolation between two points, given a number
 * in an interval.
 *
 * @param input The input number.
 * @param input_start Start of the interval the input number falls on,
 * inclusive. The closer to input_start, the closer the output is to
 * output_start.
 * @param input_end End of the interval the number falls on, inclusive.
 * @param output_start Point on the starting tip of the interpolation.
 * @param output_end Point on the ending tip of the interpolation.
 * @return The interpolated point.
 */
point interpolate_point(
    float input, float input_start, float input_end,
    const point &output_start, const point &output_end
) {
    return
        point(
            interpolate_number(
                input, input_start, input_end, output_start.x, output_end.x
            ),
            interpolate_number(
                input, input_start, input_end, output_start.y, output_end.y
            )
        );
        
}


/**
 * @brief Returns whether a point is inside a triangle or not.
 *
 * @param p The point to check.
 * @param rect_center Center coordinates of the rectangle.
 * @param rect_size Width and height of the rectangle.
 * @return Whether it is inside.
 */
bool is_point_in_rectangle(
    const point &p, const point &rect_center, const point &rect_size
) {
    if(p.x < rect_center.x - rect_size.x / 2.0f) return false;
    if(p.x > rect_center.x + rect_size.x / 2.0f) return false;
    if(p.y < rect_center.y - rect_size.y / 2.0f) return false;
    if(p.y > rect_center.y + rect_size.y / 2.0f) return false;
    return true;
}


/**
 * @brief Returns whether a point is inside a triangle or not.
 *
 * @param p The point to check.
 * @param tp1 First point of the triangle.
 * @param tp2 Second point of the triangle.
 * @param tp3 Third point of the triangle.
 * @param loq If true, use a "less or equal" comparison.
 * Different code requires different precision for on-line cases.
 * Just...don't overthink this, I added this based on what worked and didn't.
 * @return Whether it is inside.
 */
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    bool loq
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


/**
 * @brief Returns whether a line segment intersects with a rectangle.
 * Also returns true if the line is fully inside the rectangle.
 *
 * @param r1 Top-left corner of the rectangle.
 * @param r2 Bottom-right corner of the rectangle.
 * @param l1 Starting point of the line segment.
 * @param l2 Ending point of the line segment.
 * @return Whether they intersect.
 */
bool line_seg_intersects_rectangle(
    const point &r1, const point &r2,
    const point &l1, const point &l2
) {
    //Line crosses left side?
    if(
        line_segs_intersect(
            l1, l2, point(r1.x, r1.y), point(r1.x, r2.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses right side?
    if(
        line_segs_intersect(
            l1, l2, point(r2.x, r1.y), point(r2.x, r2.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses top side?
    if(
        line_segs_intersect(
            l1, l2, point(r1.x, r1.y), point(r2.x, r1.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses bottom side?
    if(
        line_segs_intersect(
            l1, l2, point(r1.x, r2.y), point(r2.x, r2.y), nullptr, nullptr
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


/**
 * @brief Returns whether a line segment intersects with a rotated rectangle
 * or not.
 *
 * @param lp1 First point of the line segment.
 * @param lp2 Second point of the line segment.
 * @param rect_center Center point of the rectangle.
 * @param rect_dim Width and height of the rectangle.
 * @param rect_angle Angle of the rectangle.
 * @return Whether they intersect.
 */
bool line_seg_intersects_rotated_rectangle(
    const point &lp1, const point &lp2,
    const point &rect_center, const point &rect_dim, float rect_angle
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
        line_segs_intersect(
            delta_p1,
            delta_p2,
            point(half_dim.x, -half_dim.y),
            point(half_dim.x, half_dim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Top side.
    if(
        line_segs_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, -half_dim.y),
            point(half_dim.x, -half_dim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Left side.
    if(
        line_segs_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, -half_dim.y),
            point(-half_dim.x, half_dim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Bottom side.
    if(
        line_segs_intersect(
            delta_p1,
            delta_p2,
            point(-half_dim.x, half_dim.y),
            point(half_dim.x, half_dim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    return false;
}


/**
 * @brief Returns whether the two line segments are collinear.
 *
 * @param a Starting point of the first line segment.
 * @param b Ending point of the first line segment.
 * @param c Starting point of the second line segment.
 * @param d Ending point of the second line segment.
 * @return Whether they are collinear.
 */
bool line_segs_are_collinear(
    const point &a, const point &b, const point &c, const point &d
) {
    return
        points_are_collinear(a, b, c) &&
        points_are_collinear(a, b, d);
}


/**
 * @brief Returns whether the two line segments intersect.
 *
 * @param l1p1 Starting point of the first line segment.
 * @param l1p2 Ending point of the first line segment.
 * @param l2p1 Starting point of the second line segment.
 * @param l2p2 Ending point of the second line segment.
 * @param out_final_l1r If not nullptr and they intersect, the distance from
 * the start of line 1 in which the intersection happens is returned here.
 * This is a ratio, so 0 is the start, 1 is the end of the line.
 * @param out_final_l2r Same as out_l1r, but for line 2.
 * @return Whether they intersect.
 */
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* out_final_l1r, float* out_final_l2r
) {
    float l1r = 0.0f;
    float l2r = 0.0f;
    bool result = lines_intersect(l1p1, l1p2, l2p1, l2p2, &l1r, &l2r);
    
    if(out_final_l1r) *out_final_l1r = l1r;
    if(out_final_l2r) *out_final_l2r = l2r;
    
    if(result) {
        //Return whether they intersect at the segments.
        return
            l1r >= 0 && l1r <= 1 &&
            l2r >= 0 && l2r <= 1;
    } else {
        return false;
    }
}


/**
 * @brief Returns whether the two line segments intersect.
 *
 * @param l1p1 Starting point of the first line segment.
 * @param l1p2 Ending point of the first line segment.
 * @param l2p1 Starting point of the second line segment.
 * @param l2p2 Ending point of the second line segment.
 * @param out_intersection If not null, return the intersection point here.
 * @return Whether they intersect.
 */
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* out_intersection
) {
    float r;
    if(out_intersection) {
        out_intersection->x = 0.0f;
        out_intersection->y = 0.0f;
    }
    if(!line_segs_intersect(l1p1, l1p2, l2p1, l2p2, &r, nullptr)) return false;
    if(out_intersection) {
        out_intersection->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        out_intersection->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    return true;
}


/**
 * @brief Converts linear distance to angular distance.
 *
 * @param linear_dist Linear distance.
 * @param radius Radius of the circle.
 * @return The angular distance.
 */
float linear_dist_to_angular(float linear_dist, float radius) {
    return (float) (2 * atan(linear_dist / (2 * radius)));
}


/**
 * @brief Returns whether two lines (not line segments) intersect, and returns
 * information about where it happens.
 *
 * @param l1p1 Point 1 of the first line.
 * @param l1p2 Point 2 of the first line.
 * @param l2p1 Point 1 of the second line.
 * @param l2p2 Point 2 of the second line.
 * @param out_l1r If not nullptr and they intersect, returns the distance from
 * the start of line 1 in which the intersection happens.
 * This is a ratio, so 0 is the start, 1 is the end of the line.
 * @param out_l2r Same as out_l1r, but for line 2.
 * @return Whether they intersect.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    float* out_l1r, float* out_l2r
) {
    float div =
        (l2p2.y - l2p1.y) * (l1p2.x - l1p1.x) -
        (l2p2.x - l2p1.x) * (l1p2.y - l1p1.y);
        
    if(div != 0.0f) {
        //They intersect.
        
        if(out_l1r) {
            //Calculate the intersection distance from the start of line 1.
            *out_l1r =
                (
                    (l2p2.x - l2p1.x) * (l1p1.y - l2p1.y) -
                    (l2p2.y - l2p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        if(out_l2r) {
            //Calculate the intersection distance from the start of line 2.
            *out_l2r =
                (
                    (l1p2.x - l1p1.x) * (l1p1.y - l2p1.y) -
                    (l1p2.y - l1p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        return true;
        
    } else {
        //They don't intersect.
        
        if(out_l1r) *out_l1r = 0.0f;
        if(out_l2r) *out_l2r = 0.0f;
        
        return false;
        
    }
}


/**
 * @brief Returns whether two lines (not line segments) intersect, and returns
 * information about where it happens.
 *
 * @param l1p1 Point 1 of the first line.
 * @param l1p2 Point 2 of the first line.
 * @param l2p1 Point 1 of the second line.
 * @param l2p2 Point 2 of the second line.
 * @param out_point If not nullptr and they intersect,
 * the coordinates of where it happens is returned here.
 * @return Whether they intersect.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    point* out_point
) {
    if(out_point) {
        out_point->x = 0.0f;
        out_point->y = 0.0f;
    }
    
    float r = 0.0f;
    if(!lines_intersect(l1p1, l1p2, l2p1, l2p2, &r, nullptr)) {
        return false;
    }
    
    if(out_point) {
        out_point->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        out_point->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    
    return true;
}


/**
 * @brief Returns the movement necessary to move a point.
 *
 * @param start Coordinates of the initial point.
 * @param target Coordinates of the target point.
 * @param speed Speed at which the point can move.
 * @param reach_radius If the point is within this range of the target,
 * consider it as already being there.
 * @param mov Variable to return the amount of movement to.
 * @param angle Variable to return the angle the point faces to.
 * @param reached Variable to return whether the point reached the target.
 * @param delta_t How long the frame's tick is, in seconds.
 */
void move_point(
    const point &start, const point &target,
    float speed, float reach_radius,
    point* mov, float* angle, bool* reached, float delta_t
) {
    point diff = target - start;
    float dis = (float) sqrt(diff.x * diff.x + diff.y * diff.y);
    
    if(dis > reach_radius) {
        float move_amount =
            (float) std::min((double) (dis / delta_t / 2.0f), (double) speed);
            
        diff *= (move_amount / dis);
        
        if(mov) *mov = diff;
        if(angle) *angle = (float) atan2(diff.y, diff.x);
        if(reached) *reached = false;
        
    } else {
    
        if(mov) *mov = point();
        if(reached) *reached = true;
    }
}


/**
 * @brief Normalizes an angle so that it's between 0 and TAU (M_PI * 2).
 *
 * @param a Angle to normalize.
 * @return The normalized angle.
 */
float normalize_angle(float a) {
    a = (float) fmod(a, (float) TAU);
    if(a < 0) a += TAU;
    return a;
}


/**
 * @brief Normalizes the specified vector so its magnitude is 1.
 *
 * @param v Vector to normalize.
 * @return The normalized vector.
 */
point normalize_vector(const point &v) {
    float length = dist(point(), v).to_float();
    if(length == 0.0f) return v;
    return
        point(
            v.x / length,
            v.y / length
        );
}


/**
 * @brief Converts a point to a string.
 *
 * @param p Point to convert.
 * @param z If not nullptr, add a third word which is this Z coordinate.
 * @return The string.
 */
string p2s(const point &p, const float* z) {
    return f2s(p.x) + " " + f2s(p.y) + (z ? " " + f2s(*z) : "");
}


/**
 * @brief Returns whether three given points are collinear or not.
 *
 * @param a First point.
 * @param b Second point.
 * @param c Third point.
 * @return Whether they are collinear.
 */
bool points_are_collinear(
    const point &a, const point &b, const point &c
) {
    //https://math.stackexchange.com/a/405981
    return
        (b.y - a.y) * (c.x - b.x) ==
        (c.y - b.y) * (b.x - a.x);
}


/**
 * @brief Given new coordinates, updates the maximum coordinates record,
 * if the new coordinates are a new maximum in either axis.
 * Each axis is processed separately.
 *
 * @param max_coords Maximum coordinates so far.
 * @param new_coords New coordinates to process and, if necessary, update with.
 */
void update_max_coords(point &max_coords, const point &new_coords) {
    max_coords.x =
        std::max(max_coords.x, new_coords.x);
    max_coords.y =
        std::max(max_coords.y, new_coords.y);
}


/**
 * @brief Given new coordinates, updates the minimum coordinates record,
 * if the new coordinates are a new minimum in either axis.
 * Each axis is processed separately.
 *
 * @param min_coords Minimum coordinates so far.
 * @param new_coords New coordinates to process and, if necessary, update with.
 */
void update_min_coords(point &min_coords, const point &new_coords) {
    min_coords.x =
        std::min(min_coords.x, new_coords.x);
    min_coords.y =
        std::min(min_coords.y, new_coords.y);
}


/**
 * @brief Given new coordinates, updates the minimum coordinates record
 * and maximum coordinates record, if the new coordinates are a new
 * minimum or maximum in either axis. Each axis is processed separately.
 *
 * @param min_coords Minimum coordinates so far.
 * @param max_coords Maximum coordinates so far.
 * @param new_coords New coordinates to process and, if necessary, update with.
 */
void update_min_max_coords(
    point &min_coords, point &max_coords, const point &new_coords
) {
    update_min_coords(min_coords, new_coords);
    update_max_coords(max_coords, new_coords);
}


/**
 * @brief Projects a set of vertexes onto an axis.
 *
 * @param v Vertexes to project.
 * @param axis The axis to project onto.
 * @param min The smallest value of all the vertexes.
 * @param max The largest value of all the vertexes.
 */
void project_vertexes(
    const vector<point> &v, const point axis, float* min, float* max
) {
    for(size_t i = 0; i < v.size(); i++) {
        point p = v[i];
        float proj = dot_product(p, axis);
        
        *min = std::min(*min, proj);
        *max = std::max(*max, proj);
    }
}


/**
 * @brief Converts an angle from radians to degrees.
 *
 * @param rad Angle, in radians.
 * @return The degrees.
 */
float rad_to_deg(float rad) {
    return (float) (180.0f / M_PI) * rad;
}


/**
 * @brief Checks if two rectangles are colliding.
 *
 * @param tl1 Coordinates of the first box's top-left.
 * @param br1 Coordinates of the first box's bottom-right.
 * @param tl2 Coordinates of the second box's top-left.
 * @param br2 Coordinates of the second box's bottom-right.
 * @return Whether they intersect.
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


/**
 * @brief Returns whether a rotated rectangle is touching another rotated
 * rectangle or not. This includes being completely inside the rectangle.
 *
 * @param rect1 Center coordinates of the first rectangle.
 * @param rect_dim1 Dimensions of the first rectangle.
 * @param rect_angle1 Angle the first rectangle is facing.
 * @param rect2 Center coordinates of the second rectangle.
 * @param rect_dim2 Dimensions of the second rectangle.
 * @param rect_angle2 Angle the second rectangle is facing.
 * @param out_overlap_dist If not nullptr, the amount of overlap is
 * returned here.
 * @param out_overlap_angle If not nullptr, the direction that rectangle 1 would
 * push rectangle 2 away with is returned here.
 * @return Whether they intersect.
 */
bool rectangles_intersect(
    const point &rect1, const point &rect_dim1, float rect_angle1,
    const point &rect2, const point &rect_dim2, float rect_angle2,
    float* out_overlap_dist, float* out_overlap_angle
) {
    //Start by getting the vertexes of the rectangles.
    point tl(-rect_dim1.x / 2.0f, -rect_dim1.y / 2.0f);
    point br(rect_dim1.x / 2.0f, rect_dim1.y / 2.0f);
    vector<point> rect1_vertexes {
        rotate_point(tl, rect_angle1) + rect1,
        rotate_point(point(tl.x, br.y), rect_angle1) + rect1,
        rotate_point(br, rect_angle1) + rect1,
        rotate_point(point(br.x, tl.y), rect_angle1) + rect1
    };
    
    tl = point(-rect_dim2.x / 2, -rect_dim2.y / 2);
    br = point(rect_dim2.x / 2, rect_dim2.y / 2);
    
    vector<point> rect2_vertexes {
        rotate_point(tl, rect_angle2) + rect2,
        rotate_point(point(tl.x, br.y), rect_angle2) + rect2,
        rotate_point(br, rect_angle2) + rect2,
        rotate_point(point(br.x, tl.y), rect_angle2) + rect2
    };
    
    //Code from https://www.youtube.com/watch?v=SUyG3aV
    //(Polygon Collision Resolution)
    //https://www.youtube.com/watch?v=Zgf1DYrmSnk
    //(Separating Axis Theorem).
    
    point normal(0, 0);
    float min_overlap = INFINITY;
    
    vector<point> shape1 = rect1_vertexes;
    
    for(int s = 0; s < 2; s++) {
        if(s == 1) {
            shape1 = rect2_vertexes;
        }
        
        //We only need to test the first two edges,
        //since the other two are parallel.
        for(int e = 0; e < 2; e++) {
            point a = shape1[e];
            point b = shape1[(e + 1) % 4];
            
            point edge = b - a;
            point axis(-edge.y, edge.x);
            
            float min_1 = INFINITY;
            float max_1 = -INFINITY;
            float min_2 = INFINITY;
            float max_2 = -INFINITY;
            
            //Project each vertex onto the axis.
            project_vertexes(rect1_vertexes, axis, &min_1, &max_1);
            project_vertexes(rect2_vertexes, axis, &min_2, &max_2);
            
            if(min_1 >= max_2 || min_2 >= max_1) {
                //We found an opening, there can't be a collision.
                return false;
            }
            
            float cur_overlap = std::min(max_1 - min_2, max_2 - min_1);
            if(cur_overlap < min_overlap) {
                min_overlap = cur_overlap;
                normal = axis;
            }
        }
    }
    
    //The size of the axis results in a much bigger overlap,
    //so we correct it here.
    min_overlap /= dist(point(0.0f), normal).to_float();
    
    //Ensure the normal is facing outwards.
    point dir = rect2 - rect1;
    if(dot_product(dir, normal) > 0) {
        normal *= -1;
    }
    
    if(out_overlap_dist) {
        *out_overlap_dist = min_overlap;
    }
    if(out_overlap_angle) {
        *out_overlap_angle = get_angle(point(0.0f), normal);
    }
    
    return true;
}


/**
 * @brief Resizes a pair of size-related coordinates such that they fit the
 * specified "box" size as snuggly as possible, whilst keeping their original
 * aspect ratio.
 *
 * @param original_size The original size coordinates.
 * @param box_size Width and height of the box to fit into.
 * @return The resized dimensions.
 */
point resize_to_box_keeping_aspect_ratio(
    const point &original_size,
    const point &box_size
) {
    if(original_size.y == 0.0f || box_size.y == 0.0f) return point();
    float original_aspect_ratio = original_size.x / original_size.y;
    float box_aspect_ratio = box_size.x / box_size.y;
    if(box_aspect_ratio > original_aspect_ratio) {
        return
            point(
                original_size.x * box_size.y / original_size.y,
                box_size.y
            );
    } else {
        return
            point(
                box_size.x,
                original_size.y * box_size.x / original_size.x
            );
    }
}


/**
 * @brief Rotates a point by an angle.
 * The x and y are meant to represent the difference
 * between the point and the center of the rotation.
 *
 * @param coords Coordinates to rotate.
 * @param angle Angle to rotate by.
 * @return The rotated point.
 */
point rotate_point(const point &coords, float angle) {
    float c = (float) cos(angle);
    float s = (float) sin(angle);
    return point(c * coords.x - s * coords.y, s * coords.x + c * coords.y);
}


/**
 * @brief Converts a string to a point.
 *
 * @param s String to convert.
 * @param out_z If not nullptr, the third word is returned here.
 * @return The (X and Y) coordinates.
 */
point s2p(const string &s, float* out_z) {
    vector<string> words = split(s);
    point p;
    if(words.size() >= 1) {
        p.x = (float) s2f(words[0]);
    }
    if(words.size() >= 2) {
        p.y = (float) s2f(words[1]);
    }
    if(out_z && words.size() >= 3) {
        *out_z = (float) s2f(words[2]);
    }
    return p;
}


/**
 * @brief Scales a rectangle so that it fits as much of the box as possible,
 * based on a number of settings. If any of the settings cannot be respected,
 * a scale of 1,1 will be returned, even if that goes against the box.
 *
 * @param rect_size Width and height of the rectangle to scale.
 * @param box_size Box width and height.
 * @param can_grow_x Whether it's possible to increase the width.
 * @param can_grow_y Whether it's possible to increase the height.
 * @param can_shrink_x Whether it's possible to decrease the width.
 * @param can_shrink_y Whether it's possible to decrease the height.
 * @param can_change_ratio Whether it's possible to change the aspect ratio
 * of the rectangle.
 * @return The scale factor for X and for Y.
 */
point scale_rectangle_to_box(
    const point &rect_size, const point &box_size,
    bool can_grow_x, bool can_grow_y,
    bool can_shrink_x, bool can_shrink_y,
    bool can_change_ratio
) {
    point final_scale(1.0f, 1.0f);
    
    if(
        rect_size.x == 0.0f || rect_size.y == 0.0f ||
        box_size.x == 0.0f || box_size.y == 0.0f
    ) {
        return final_scale;
    }
    
    point box_to_use =
        can_change_ratio ?
        box_size :
        resize_to_box_keeping_aspect_ratio(rect_size, box_size);
    bool can_scale_x =
        (rect_size.x < box_to_use.x && can_grow_x) ||
        (rect_size.x > box_to_use.x && can_shrink_x);
    bool can_scale_y =
        (rect_size.y < box_to_use.y && can_grow_y) ||
        (rect_size.y > box_to_use.y && can_shrink_y);
        
    if(can_change_ratio) {
        if(can_scale_x) final_scale.x = box_to_use.x / rect_size.x;
        if(can_scale_y) final_scale.y = box_to_use.y / rect_size.y;
    } else {
        if(can_scale_x && can_scale_y) {
            final_scale.x = box_to_use.x / rect_size.x;
            final_scale.y = box_to_use.y / rect_size.y;
        }
    }
    
    return final_scale;
}


/**
 * @brief Given a list of items, chooses which item comes next
 * geometrically in the specified direction. Useful for menus with
 * several buttons the player can select multidirectionally in.
 * Also, it loops around.
 *
 * @param item_coordinates Vector with the center coordinates of all items.
 * @param selected_item Index of the selected item.
 * @param direction Angle specifying the direction.
 * @param loop_region Width and height of the loop region.
 * @return The next item's index in the list.
 */
size_t select_next_item_directionally(
    const vector<point> &item_coordinates, size_t selected_item,
    float direction, const point &loop_region
) {
    const float MIN_BLINDSPOT_ANGLE = (float) (TAU * 0.17f);
    const float MAX_BLINDSPOT_ANGLE = (float) (TAU * 0.33f);
    
    float normalized_dir = normalize_angle(direction);
    const point &sel_coords = item_coordinates[selected_item];
    float best_score = FLT_MAX;
    size_t best_item = selected_item;
    
    //Check each item that isn't the current one.
    for(size_t i = 0; i < item_coordinates.size(); i++) {
    
        if(i == selected_item) continue;
        
        point i_base_coords = item_coordinates[i];
        
        //Get the standard coordinates for this item, and make them relative.
        point i_coords = i_base_coords;
        i_coords = i_coords - sel_coords;
        
        //Rotate the coordinates such that the specified direction
        //lands to the right.
        i_coords = rotate_point(i_coords, -normalized_dir);
        
        //Check if it's between the blind spot angles.
        //We get the same result whether the Y is positive or negative,
        //so let's simplify things and make it positive.
        float rel_angle =
            get_angle(point(i_coords.x, (float) fabs(i_coords.y)));
        if(
            rel_angle >= MIN_BLINDSPOT_ANGLE &&
            rel_angle <= MAX_BLINDSPOT_ANGLE
        ) {
            //If so, never let this item be chosen, no matter what. This is
            //useful to stop a list of items with no vertical variance from
            //picking another item when the direction is up, for instance.
            continue;
        }
        
        if(i_coords.x > 0.0f) {
            //If this item is in front of the selected one,
            //give it a score like normal.
            float score = i_coords.x + (float) fabs(i_coords.y);
            if(score < best_score) {
                best_score = score;
                best_item = i;
            }
            
        } else {
            //If the item is behind, we'll need to loop its coordinates
            //and score those loop coordinates that land in front.
            //Unfortunately, there's no way to know how the coordinates
            //should be looped in order to land in front of the selected
            //item, so we should just check all loop variations: above, below
            //to the left, to the right, and combinations.
            
            for(char c = -1; c < 2; c++) {
                for(char r = -1; r < 2; r++) {
                
                    //If it's the same "screen" as the regular one,
                    //forget it, since we already checked above.
                    if(c == 0 && r == 0) {
                        continue;
                    }
                    
                    //Get the coordinates in this parallel region, and make
                    //them relative.
                    i_coords = i_base_coords;
                    i_coords.x += loop_region.x * c;
                    i_coords.y += loop_region.y * r;
                    i_coords = i_coords - sel_coords;
                    
                    //Rotate the coordinates such that the specified direction
                    //lands to the right.
                    i_coords = rotate_point(i_coords, -normalized_dir);
                    
                    //If these coordinates are behind the selected item,
                    //they cannot be selected.
                    if(i_coords.x < 0.0f) {
                        continue;
                    }
                    
                    //Finally, figure out if this is the new best item.
                    float score = i_coords.x + (float) fabs(i_coords.y);
                    if(score < best_score) {
                        best_score = score;
                        best_item = i;
                    }
                }
            }
        }
    }
    
    return best_item;
}


/**
 * @brief Returns how much to vertically offset something so that it aligns
 * with either the top, center, or bottom of a box.
 *
 * @param mode Vertical alignment mode.
 * @param height Total height of the box.
 * @return The vertical offset.
 */
float get_vertical_align_offset(V_ALIGN_MODE mode, float height) {
    return
        mode == V_ALIGN_MODE_BOTTOM ?
        height :
        mode == V_ALIGN_MODE_CENTER ?
        height / 2.0f :
        0.0f;
}
