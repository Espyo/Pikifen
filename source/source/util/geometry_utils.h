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

#pragma once

#define _USE_MATH_DEFINES
#include <cstddef>
#include <math.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


//Ways to vertically align something.
enum V_ALIGN_MODE {

    //Align to the top.
    V_ALIGN_MODE_TOP,
    
    //Align to the center.
    V_ALIGN_MODE_CENTER,
    
    //Align to the bottom.
    V_ALIGN_MODE_BOTTOM,
    
};


/**
 * @brief Simple 2D point.
 */
struct Point {

    //--- Members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    
    //--- Function declarations ---
    
    Point(float x, float y);
    Point(float xy);
    Point();
    const Point operator +(const Point &p) const;
    const Point operator -(const Point &p) const;
    const Point operator *(const Point &p) const;
    const Point operator /(const Point &p) const;
    const Point operator +(float n) const;
    const Point operator -(float n) const;
    const Point operator *(float m) const;
    const Point operator /(float n) const;
    Point operator +=(const Point &p);
    Point operator -=(const Point &p);
    Point operator *=(const Point &p);
    Point operator /=(const Point &p);
    Point operator +=(float n);
    Point operator -=(float n);
    Point operator *=(float n);
    Point operator /=(float n);
    bool operator ==(const Point &p) const;
    bool operator !=(const Point &p) const;
    
};

const Point operator+(float n, const Point &p);
const Point operator-(float n, const Point &p);
const Point operator*(float n, const Point &p);
const Point operator/(float n, const Point &p);


/**
 * @brief A distance.
 *
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
struct Distance {

    public:
    
    //--- Function declarations ---
    
    Distance(const Point &p1, const Point &p2);
    explicit Distance(float d = 0.0f);
    Distance &operator =(float d);
    bool operator <(float d2) const;
    bool operator <(const Distance &d2) const;
    bool operator <=(float d2) const;
    bool operator <=(const Distance &d2) const;
    bool operator >(float d2) const;
    bool operator >(const Distance &d2) const;
    bool operator >=(float d2) const;
    bool operator >=(const Distance &d2) const;
    bool operator ==(float d2) const;
    bool operator ==(const Distance &d2) const;
    bool operator !=(float d2) const;
    bool operator !=(const Distance &d2) const;
    void operator +=(float d2);
    void operator +=(const Distance &d2);
    void operator -=(float d2);
    void operator -=(const Distance &d2);
    float to_float();
    
    private:
    
    //--- Members ---
    
    //Distance squared. Most operations are based on this number.
    float distance_squared = 0.0f;
    
    //Square root of the distance squared. Only used if necessary.
    float normal_distance = 0.0f;
    
    //Do we know the normal distance?
    bool has_normal_distance = false;
    
};



Point angle_to_coordinates(
    float angle, float magnitude
);
float angular_dist_to_linear(float angular_dist, float radius);
bool bbox_check(const Point &center1, const Point &center2, float r);
bool bbox_check(
    const Point &tl1, const Point &br1,
    const Point &center2, float r
);
void calculate_throw(
    const Point &start_xy, float start_z,
    const Point &target_xy, float target_z,
    float max_h, float gravity,
    Point* req_speed_xy, float* req_speed_z, float* out_h_angle
);
bool circle_intersects_line_seg(
    const Point &circle, float cr,
    const Point &line_p1, const Point &line_p2,
    float* out_lix = nullptr, float* out_liy = nullptr
);
bool circle_intersects_rectangle(
    const Point &circle, float cr,
    const Point &rectangle, const Point &rect_dim,
    float rect_angle,
    float* out_overlap_dist = nullptr, float* out_rectangle_side_angle = nullptr
);
bool collinear_line_segs_intersect(
    const Point &a, const Point &b, const Point &c, const Point &d,
    Point* out_intersection_tl = nullptr, Point* out_intersection_br = nullptr
);
void coordinates_to_angle(
    const Point &coordinates, float* angle, float* magnitude
);
float deg_to_rad(float rad);
float dot_product(const Point &v1, const Point &v2);
float get_angle(const Point &focus);
float get_angle(const Point &center, const Point &focus);
float get_angle_cw_diff(float a1, float a2);
float get_angle_smallest_dif(float a1, float a2);
Point get_closest_point_in_line_seg(
    const Point &l1, const Point &l2, const Point &p,
    float* out_segment_ratio = nullptr
);
Point get_closest_point_in_rotated_rectangle(
    const Point &p,
    const Point &rect_center, const Point &rect_dim, float rect_angle,
    bool* out_is_inside
);
void get_miter_points(
    const Point &a, const Point &b, const Point &c, float thickness,
    Point* miter_point_1, Point* miter_point_2, float max_miter_length = 0.0f
);
float get_point_sign(
    const Point &p, const Point &lp1, const Point &lp2
);
Point get_random_point_in_rectangular_ring(
    const Point &inner_dist, const Point &outer_dist,
    int axis_random_int, float axis_random_float, float px_random_float,
    float py_random_float, int side_random_int
);
Point get_random_point_in_ring(
    float inner_dist, float outer_dist,
    float arc, float arc_rot,
    float radius_random_float, float angle_random_float
);
Point get_ratio_point_in_ring(
    float inner_dist, float outer_dist,
    float arc, float arc_rot, float ratio
);
void get_transformed_rectangle_bounding_box(
    const Point &center, const Point &dimensions, float angle,
    Point* min_coords, Point* max_coords
);
float interpolate_angle(
    float input, float input_start, float input_end,
    float &output_start, float &output_end
);
Point interpolate_point(
    float input, float input_start, float input_end,
    const Point &output_start, const Point &output_end
);
bool is_point_in_rectangle(
    const Point &p, const Point &rect_center, const Point &rect_size
);
bool is_point_in_triangle(
    const Point &p, const Point &tp1, const Point &tp2, const Point &tp3,
    bool loq
);
float linear_dist_to_angular(float linear_dist, float radius);
bool line_segs_are_collinear(
    const Point &a, const Point &b, const Point &c, const Point &d
);
bool line_seg_intersects_rectangle(
    const Point &r1, const Point &r2,
    const Point &l1, const Point &l2
);
bool line_seg_intersects_rotated_rectangle(
    const Point &lp1, const Point &lp2,
    const Point &rect_center, const Point &rect_dim, float rect_angle
);
bool line_segs_intersect(
    const Point &l1p1, const Point &l1p2, const Point &l2p1, const Point &l2p2,
    float* out_final_l1r, float* out_final_l2r
);
bool line_segs_intersect(
    const Point &l1p1, const Point &l1p2, const Point &l2p1, const Point &l2p2,
    Point* out_intersection
);
bool lines_intersect(
    const Point &l1p1, const Point &l1p2,
    const Point &l2p1, const Point &l2p2,
    float* out_l1r, float* out_l2r
);
bool lines_intersect(
    const Point &l1p1, const Point &l1p2,
    const Point &l2p1, const Point &l2p2,
    Point* out_point
);
void move_point(
    const Point &start, const Point &target,
    float speed, float reach_radius, Point* mov,
    float* angle, bool* reached, float delta_t
);
float normalize_angle(float a);
Point normalize_vector(const Point &v);
bool points_are_collinear(
    const Point &a, const Point &b, const Point &c
);
void update_max_coords(Point &max_coords, const Point &new_coords);
void update_min_coords(Point &min_coords, const Point &new_coords);
void update_min_max_coords(
    Point &min_coords, Point &max_coords, const Point &new_coords
);
void project_vertexes(
    const vector<Point> &v, const Point axis, float* min, float* max
);
string p2s(const Point &p, const float* z = nullptr);
float rad_to_deg(float deg);
bool rectangles_intersect(
    const Point &tl1, const Point &br1,
    const Point &tl2, const Point &br2
);
bool rectangles_intersect(
    const Point &rect1, const Point &rect_dim1,
    float rect_angle1,
    const Point &rect2, const Point &rect_dim2,
    float rect_angle2,
    float* out_overlap_dist = nullptr, float* out_overlap_angle = nullptr
);
Point resize_to_box_keeping_aspect_ratio(
    const Point &original_size,
    const Point &box_size
);
Point rotate_point(const Point &coords, float angle);
Point s2p(const string &s, float* out_z = nullptr);
Point scale_rectangle_to_box(
    const Point &rect_size, const Point &box_size,
    bool can_grow_x, bool can_grow_y,
    bool can_shrink_x, bool can_shrink_y,
    bool can_change_ratio
);
size_t select_next_item_directionally(
    const vector<Point> &item_coordinates, size_t selected_item,
    float direction, const Point &loop_region
);
float get_vertical_align_offset(V_ALIGN_MODE mode, float height);
