/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for geometry-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#ifndef GEOMETRY_UTILS_INCLUDED
#define GEOMETRY_UTILS_INCLUDED

#define _USE_MATH_DEFINES
#include <cstddef>
#include <math.h>
#include <string>


/* ----------------------------------------------------------------------------
 * Simple 2D point.
 */
struct point {
    //X coordinate.
    float x;
    //Y coordinate.
    float y;
    
    point(const float x, const float y);
    point();
    const point operator +(const point &p) const;
    const point operator -(const point &p) const;
    const point operator *(const point &p) const;
    const point operator /(const point &p) const;
    const point operator +(const float n) const;
    const point operator -(const float n) const;
    const point operator /(const float n) const;
    point operator +=(const point &p);
    point operator -=(const point &p);
    point operator +=(const float n);
    point operator *=(const float n);
    const bool operator ==(const point &p) const;
    const bool operator !=(const point &p) const;
    const point operator *(const float m) const;
};


/* ----------------------------------------------------------------------------
 * A distance.
 * Basically this is just a number, but for optimization's sake,
 * this number is actually the distance SQUARED.
 * It's faster to compare two squared distances than square-rooting them both,
 * since sqrt() is so costly. If we do need to sqrt() a number, we keep it in
 * a cache inside the class, so that we can use it at will next time.
 * Fun fact, keeping an extra boolean in the class that indicates whether or
 * not the sqrt()'d number is in cache is around twice as fast as keeping
 * only the squared and sqrt()'d numbers, and setting the sqrt()'d number
 * to LARGE_FLOAT if it is uncached.
 */
struct dist {
public:
    dist(const point &p1, const point &p2);
    dist(const float d = 0.0f);
    dist &operator =(const float d);
    bool operator <(const float d2) const;
    bool operator <(const dist &d2) const;
    bool operator <=(const float d2) const;
    bool operator <=(const dist &d2) const;
    bool operator >(const float d2) const;
    bool operator >(const dist &d2) const;
    bool operator >=(const float d2) const;
    bool operator >=(const dist &d2) const;
    bool operator ==(const float d2) const;
    bool operator ==(const dist &d2) const;
    bool operator !=(const float d2) const;
    bool operator !=(const dist &d2) const;
    void operator +=(const float d2);
    void operator +=(const dist &d2);
    void operator -=(const float d2);
    void operator -=(const dist &d2);
    float to_float();
    
private:
    //Distance squared. Most operations are based on this number.
    float distance_squared;
    //Square root of the distance squared. Only used if necessary.
    float normal_distance;
    //Do we know the normal distance?
    bool has_normal_distance;
    
};



point angle_to_coordinates(
    const float angle, const float magnitude
);
float angular_dist_to_linear(const float angular_dist, const float radius);
bool bbox_check(const point &center1, const point &center2, const float r);
bool bbox_check(
    const point &tl1, const point &br1,
    const point &center2, const float r
);
void calculate_throw(
    const point &start_xy, const float start_z,
    const point &target_xy, const float target_z,
    const float max_h, const float gravity,
    point* req_speed_xy, float* req_speed_z, float* final_h_angle
);
bool circle_intersects_line_seg(
    const point &circle, const float cr,
    const point &line_p1, const point &line_p2,
    float* lix = NULL, float* liy = NULL
);
bool circle_intersects_rectangle(
    const point &circle, const float cr,
    const point &rectangle, const point &rect_dim,
    const float rect_angle,
    float* overlap_dist = NULL, float* rectangle_side_angle = NULL
);
bool collinear_line_segs_intersect(
    const point &a, const point &b, const point &c, const point &d,
    point* intersection_tl = NULL, point* intersection_br = NULL
);
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
);
float deg_to_rad(const float rad);
float dot_product(const point &v1, const point &v2);
float get_angle(const point &focus);
float get_angle(const point &center, const point &focus);
float get_angle_cw_dif(float a1, float a2);
float get_angle_smallest_dif(const float a1, const float a2);
point get_closest_point_in_line_seg(
    const point &l1, const point &l2, const point &p,
    float* segment_ratio = NULL
);
point get_closest_point_in_rotated_rectangle(
    const point &p,
    const point &rect_center, const point &rect_dim, const float rect_angle,
    bool* is_inside
);
void get_miter_points(
    const point &a, const point &b, const point &c, const float thickness,
    point* miter_point_1, point* miter_point_2
);
float get_point_sign(
    const point &p, const point &lp1, const point &lp2
);
void get_transformed_rectangle_bounding_box(
    const point &center, const point &dimensions, const float angle,
    point* min_coords, point* max_coords
);
point interpolate_point(
    const float input, const float input_start, const float input_end,
    const point &output_start, const point &output_end
);
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    bool loq
);
bool is_point_in_rectangle(
    const point &p, const point &rect_center, const point &rect_size
);
float linear_dist_to_angular(const float linear_dist, const float radius);
bool line_segs_are_collinear(
    const point &a, const point &b, const point &c, const point &d
);
bool line_seg_intersects_rectangle(
    const point &r1, const point &r2,
    const point &l1, const point &l2
);
bool line_seg_intersects_rotated_rectangle(
    const point &lp1, const point &lp2,
    const point &rect_center, const point &rect_dim, const float rect_angle
);
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* final_l1r, float* final_l2r
);
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* intersection
);
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    float* final_l1r, float* final_l2r
);
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    point* final_point
);
void move_point(
    const point &start, const point &target,
    const float speed, const float reach_radius, point* mov,
    float* angle, bool* reached, const float delta_t
);
float normalize_angle(float a);
point normalize_vector(const point &v);
bool points_are_collinear(
    const point &a, const point &b, const point &c
);
float rad_to_deg(const float deg);
bool rectangles_intersect(
    const point &tl1, const point &br1,
    const point &tl2, const point &br2
);
point resize_to_box_keeping_aspect_ratio(
    const point &original_size,
    const point &box_size
);
point rotate_point(const point &coords, const float angle);

#endif //ifndef GEOMETRY_UTILS_INCLUDED
