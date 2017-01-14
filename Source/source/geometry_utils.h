/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for geometry-related utility functions.
 */

#ifndef GEOMETRY_UTILS_INCLUDED
#define GEOMETRY_UTILS_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>

#include "sector.h"

void angle_to_coordinates(
    const float angle, const float magnitude, float* x_coord, float* y_coord
);
bool bbox_check(
    const float cx1, const float cy1, const float cx2, const float cy2,
    const float r
);
bool circle_intersects_line(
    const float cx, const float cy, const float cr,
    const float x1, const float y1, const float x2, const float y2,
    float* lix = NULL, float* liy = NULL
);
vertex* get_merge_vertex(
    const float x, const float y,
    vector<vertex*> &all_vertexes, const float merge_radius,
    size_t* nr = NULL
);
bool is_edge_valid(edge* l);
bool is_polygon_clockwise(vector<vertex*> &vertexes);
void move_point(
    const float x, const float y, const float tx, const float ty,
    const float speed, const float reach_radius, float* mx, float* my,
    float* angle, bool* reached, const float delta_t
);
float normalize_angle(float a);
void rotate_point(
    const float x, const float y, const float angle,
    float* final_x, float* final_y
);
bool square_intersects_line(
    const float sx1, const float sy1, const float sx2, const float sy2,
    const float lx1, const float ly1, const float lx2, const float ly2
);

#endif //ifndef GEOMETRY_UTILS_INCLUDED
