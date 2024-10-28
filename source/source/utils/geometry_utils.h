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
struct point {

    //--- Members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    
    //--- Function declarations ---
    
    point(float x, float y);
    point();
    const point operator +(const point &p) const;
    const point operator -(const point &p) const;
    const point operator *(const point &p) const;
    const point operator /(const point &p) const;
    const point operator +(float n) const;
    const point operator -(float n) const;
    const point operator *(float m) const;
    const point operator /(float n) const;
    point operator +=(const point &p);
    point operator -=(const point &p);
    point operator +=(float n);
    point operator *=(float n);
    bool operator ==(const point &p) const;
    bool operator !=(const point &p) const;
    
};


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
struct dist {

    public:
    
    //--- Function declarations ---
    
    dist(const point &p1, const point &p2);
    explicit dist(float d = 0.0f);
    dist &operator =(float d);
    bool operator <(float d2) const;
    bool operator <(const dist &d2) const;
    bool operator <=(float d2) const;
    bool operator <=(const dist &d2) const;
    bool operator >(float d2) const;
    bool operator >(const dist &d2) const;
    bool operator >=(float d2) const;
    bool operator >=(const dist &d2) const;
    bool operator ==(float d2) const;
    bool operator ==(const dist &d2) const;
    bool operator !=(float d2) const;
    bool operator !=(const dist &d2) const;
    void operator +=(float d2);
    void operator +=(const dist &d2);
    void operator -=(float d2);
    void operator -=(const dist &d2);
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



point angle_to_coordinates(
    float angle, float magnitude
);
float angular_dist_to_linear(float angular_dist, float radius);
bool bbox_check(const point &center1, const point &center2, float r);
bool bbox_check(
    const point &tl1, const point &br1,
    const point &center2, float r
);
void calculate_throw(
    const point &start_xy, float start_z,
    const point &target_xy, float target_z,
    float max_h, float gravity,
    point* req_speed_xy, float* req_speed_z, float* out_h_angle
);
bool circle_intersects_line_seg(
    const point &circle, float cr,
    const point &line_p1, const point &line_p2,
    float* out_lix = nullptr, float* out_liy = nullptr
);
bool circle_intersects_rectangle(
    const point &circle, float cr,
    const point &rectangle, const point &rect_dim,
    float rect_angle,
    float* out_overlap_dist = nullptr, float* out_rectangle_side_angle = nullptr
);
bool collinear_line_segs_intersect(
    const point &a, const point &b, const point &c, const point &d,
    point* out_intersection_tl = nullptr, point* out_intersection_br = nullptr
);
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
);
float deg_to_rad(float rad);
float dot_product(const point &v1, const point &v2);
float get_angle(const point &focus);
float get_angle(const point &center, const point &focus);
float get_angle_cw_diff(float a1, float a2);
float get_angle_smallest_dif(float a1, float a2);
point get_closest_point_in_line_seg(
    const point &l1, const point &l2, const point &p,
    float* out_segment_ratio = nullptr
);
point get_closest_point_in_rotated_rectangle(
    const point &p,
    const point &rect_center, const point &rect_dim, float rect_angle,
    bool* out_is_inside
);
void get_miter_points(
    const point &a, const point &b, const point &c, float thickness,
    point* miter_point_1, point* miter_point_2, float max_miter_length = 0.0f
);
float get_point_sign(
    const point &p, const point &lp1, const point &lp2
);
void get_transformed_rectangle_bounding_box(
    const point &center, const point &dimensions, float angle,
    point* min_coords, point* max_coords
);
float interpolate_angle(
    float input, float input_start, float input_end,
    float &output_start, float &output_end
);
point interpolate_point(
    float input, float input_start, float input_end,
    const point &output_start, const point &output_end
);
bool is_point_in_rectangle(
    const point &p, const point &rect_center, const point &rect_size
);
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    bool loq
);
float linear_dist_to_angular(float linear_dist, float radius);
bool line_segs_are_collinear(
    const point &a, const point &b, const point &c, const point &d
);
bool line_seg_intersects_rectangle(
    const point &r1, const point &r2,
    const point &l1, const point &l2
);
bool line_seg_intersects_rotated_rectangle(
    const point &lp1, const point &lp2,
    const point &rect_center, const point &rect_dim, float rect_angle
);
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* out_final_l1r, float* out_final_l2r
);
bool line_segs_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* out_intersection
);
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    float* out_l1r, float* out_l2r
);
bool lines_intersect(
    const point &l1p1, const point &l1p2,
    const point &l2p1, const point &l2p2,
    point* out_point
);
void move_point(
    const point &start, const point &target,
    float speed, float reach_radius, point* mov,
    float* angle, bool* reached, float delta_t
);
float normalize_angle(float a);
point normalize_vector(const point &v);
bool points_are_collinear(
    const point &a, const point &b, const point &c
);
void project_vertexes(
    const vector<point> &v, const point axis, float* min, float* max
);
string p2s(const point &p, const float* z = nullptr);
float rad_to_deg(float deg);
bool rectangles_intersect(
    const point &tl1, const point &br1,
    const point &tl2, const point &br2
);
bool rectangles_intersect(
    const point &rect1, const point &rect_dim1,
    float rect_angle1,
    const point &rect2, const point &rect_dim2,
    float rect_angle2,
    float* out_overlap_dist = nullptr, float* out_overlap_angle = nullptr
);
point resize_to_box_keeping_aspect_ratio(
    const point &original_size,
    const point &box_size
);
point rotate_point(const point &coords, float angle);
point s2p(const string &s, float* out_z = nullptr);
point scale_rectangle_to_box(
    const point &rect_size, const point &box_size,
    bool can_grow_x, bool can_grow_y,
    bool can_shrink_x, bool can_shrink_y,
    bool can_change_ratio
);
size_t select_next_item_directionally(
    const vector<point> &item_coordinates, size_t selected_item,
    float direction, const point &loop_region
);
float get_vertical_align_offset(V_ALIGN_MODE mode, float height);
