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
 * @brief Constructs a new distance object, given two points.
 *
 * @param p1 First point.
 * @param p2 Second point.
 */
Distance::Distance(const Point& p1, const Point& p2) :
    distanceSquared(
        (p2.x - p1.x) * (p2.x - p1.x) +
        (p2.y - p1.y) * (p2.y - p1.y)
    ) {
    
}


/**
 * @brief Constructs a new distance object, given a non-squared distance.
 *
 * @param d Regular, non-squared distance.
 */
Distance::Distance(float d) :
    distanceSquared(d * d),
    normalDistance(d),
    hasNormalDistance(true) {
    
}


/**
 * @brief Sets the value given a non-squared distance.
 *
 * @param d Regular, non-squared distance.
 * @return The current object.
 */
Distance& Distance::operator =(float d) {
    distanceSquared = d * d;
    normalDistance = d;
    hasNormalDistance = true;
    return *this;
}


/**
 * @brief Checks if this distance is smaller than the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is smaller.
 */
bool Distance::operator<(float d2) const {
    return distanceSquared < (d2 * d2);
}


/**
 * @brief Checks if this distance is larger than the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is larger.
 */
bool Distance::operator>(float d2) const {
    return distanceSquared > (d2 * d2);
}


/**
 * @brief Checks if this distance is the same as the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is the same.
 */
bool Distance::operator==(float d2) const {
    return distanceSquared == (d2 * d2);
}


/**
 * @brief Checks if this distance is smaller than or equal to the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is smaller or equal.
 */
bool Distance::operator<=(float d2) const {
    return !operator>(d2);
}


/**
 * @brief Checks if this distance is larger than or equal to the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is larger or equal.
 */
bool Distance::operator>=(float d2) const {
    return !operator<(d2);
}


/**
 * @brief Checks if this distance is different from the specified one.
 *
 * @param d2 Regular, non-squared distance to check.
 * @return Whether it is different.
 */
bool Distance::operator!=(float d2) const {
    return !operator==(d2);
}


/**
 * @brief Checks if this distance is smaller than the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is smaller.
 */
bool Distance::operator<(const Distance& d2) const {
    return distanceSquared < d2.distanceSquared;
}


/**
 * @brief Checks if this distance is larger than the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is larger.
 */
bool Distance::operator>(const Distance& d2) const {
    return distanceSquared > d2.distanceSquared;
}


/**
 * @brief Checks if this distance is the same as the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is the same.
 */
bool Distance::operator==(const Distance& d2) const {
    return distanceSquared == d2.distanceSquared;
}


/**
 * @brief Checks if this distance is smaller than or equal to the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is smaller or equal.
 */
bool Distance::operator<=(const Distance& d2) const {
    return !operator>(d2);
}


/**
 * @brief Checks if this distance is larger than or equal to the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is larger or equal.
 */
bool Distance::operator>=(const Distance& d2) const {
    return !operator<(d2);
}


/**
 * @brief Checks if this distance is different from the specified one.
 *
 * @param d2 Distance object to check.
 * @return Whether it is different.
 */
bool Distance::operator!=(const Distance& d2) const {
    return !operator==(d2);
}


/**
 * @brief Adds some distance to the current one.
 *
 * @param d2 Amount of distance to add.
 */
void Distance::operator+=(float d2) {
    if(!hasNormalDistance) {
        normalDistance = (float) sqrt(distanceSquared);
        hasNormalDistance = true;
    }
    normalDistance += d2;
    distanceSquared = normalDistance * normalDistance;
}


/**
 * @brief Adds some distance to the current one.
 *
 * @param d2 Amount of distance to add.
 */
void Distance::operator+=(const Distance& d2) {
    distanceSquared += d2.distanceSquared;
    if(hasNormalDistance) {
        if(d2.hasNormalDistance) {
            normalDistance += d2.normalDistance;
        } else {
            normalDistance = (float) sqrt(distanceSquared);
        }
    }
}


/**
 * @brief Removes some distance from the current one.
 *
 * @param d2 Amount of distance to remove.
 */
void Distance::operator-=(float d2) {
    operator+=(-d2);
}


/**
 * @brief Removes some distance from the current one.
 *
 * @param d2 Amount of distance to remove.
 */
void Distance::operator-=(const Distance& d2) {
    distanceSquared -= d2.distanceSquared;
    if(hasNormalDistance) {
        if(d2.hasNormalDistance) {
            normalDistance -= d2.normalDistance;
        } else {
            normalDistance = (float) sqrt(distanceSquared);
        }
    }
}


/**
 * @brief Returns the regular, non-squared distance as a number.
 *
 * @return The non-squared distance.
 */
float Distance::toFloat() {
    if(!hasNormalDistance) {
        normalDistance = (float) sqrt(distanceSquared);
        hasNormalDistance = true;
    }
    return normalDistance;
}


/**
 * @brief Constructs a new point object, given its coordinates.
 *
 * @param x X coordinate.
 * @param y Y coordinate.
 */
Point::Point(float x, float y) :
    x(x),
    y(y) {
    
}


/**
 * @brief Constructs a new point object, with the given value on both
 * coordinates.
 *
 * @param xy X and Y coordinate.
 */
Point::Point(float xy) :
    x(xy),
    y(xy) {
    
}


/**
 * @brief Constructs a new point object, with the coordinates set to 0,0.
 */
Point::Point() {
}


/**
 * @brief Adds the coordinates of two points.
 *
 * @param p Point with values to add with.
 * @return The new point.
 */
const Point Point::operator +(const Point& p) const {
    return Point(x + p.x, y + p.y);
}


/**
 * @brief Subtracts the coordinates of two points.
 *
 * @param p Point with values to subtract with.
 * @return The new point.
 */
const Point Point::operator -(const Point& p) const {
    return Point(x - p.x, y - p.y);
}


/**
 * @brief Multiplies the coordinates of two points.
 *
 * @param p Point with values to multiply with.
 * @return The new point.
 */
const Point Point::operator *(const Point& p) const {
    return Point(x * p.x, y * p.y);
}


/**
 * @brief Divides the coordinates of two points.
 *
 * @param p Point with values to divide with.
 * @return The new point.
 */
const Point Point::operator /(const Point& p) const {
    return Point(x / p.x, y / p.y);
}


/**
 * @brief Adds a number to the coordinates.
 *
 * @param n Number to add to both coordinates.
 * @return The new point.
 */
const Point Point::operator +(float n) const {
    return Point(x + n, y + n);
}


/**
 * @brief Subtracts a number from each coordinates.
 *
 * @param n Number to subtract from both coordinates.
 * @return The new point.
 */
const Point Point::operator -(float n) const {
    return Point(x - n, y - n);
}


/**
 * @brief Multiplies the coordinates by a number.
 *
 * @param n Value to multiply both coordinates with.
 * @return The new point.
 */
const Point Point::operator *(float n) const {
    return Point(x * n, y * n);
}


/**
 * @brief Divides the coordinates by a number.
 *
 * @param n Number to divide both coordinates with.
 * @return The new point.
 */
const Point Point::operator /(float n) const {
    return Point(x / n, y / n);
}


/**
 * @brief Adds the coordinates of another point to this one's.
 *
 * @param p Point with the values to add with.
 * @return The current object.
 */
Point Point::operator +=(const Point& p) {
    x += p.x;
    y += p.y;
    return Point(x, y);
}


/**
 * @brief Subtracts the coordinates of another point to this one's.
 *
 * @param p Point with the values to subtract with.
 * @return The current object.
 */
Point Point::operator -=(const Point& p) {
    x -= p.x;
    y -= p.y;
    return Point(x, y);
}


/**
 * @brief Multiplies the coordinates of another point with this one's.
 *
 * @param p Point with the values to multiply by.
 * @return The current object.
 */
Point Point::operator *=(const Point& p) {
    x *= p.x;
    y *= p.y;
    return Point(x, y);
}


/**
 * @brief Divides the coordinates of another point with this one's.
 *
 * @param p Point with the values to divide by.
 * @return The current object.
 */
Point Point::operator /=(const Point& p) {
    x /= p.x;
    y /= p.y;
    return Point(x, y);
}


/**
 * @brief Adds a given number to the coordinates.
 *
 * @param n Value to add to both coordinates with.
 * @return The current object.
 */
Point Point::operator +=(float n) {
    x += n;
    y += n;
    return Point(x, y);
}


/**
 * @brief Subtracts a given number from the coordinates.
 *
 * @param n Value to subtract from both coordinates with.
 * @return The current object.
 */
Point Point::operator -=(float n) {
    x -= n;
    y -= n;
    return Point(x, y);
}


/**
 * @brief Multiplies the coordinates by a given number.
 *
 * @param n Value to multiply both coordinates with.
 * @return The current object.
 */
Point Point::operator *=(float n) {
    x *= n;
    y *= n;
    return Point(x, y);
}


/**
 * @brief Divides the coordinates by a given number.
 *
 * @param n Value to divide both coordinates with.
 * @return The current object.
 */
Point Point::operator /=(float n) {
    x /= n;
    y /= n;
    return Point(x, y);
}


/**
 * @brief Compares if two points are the same.
 *
 * @param p Other point to compare against.
 * @return Whether they are the same.
 */
bool Point::operator ==(const Point& p) const {
    return x == p.x && y == p.y;
}


/**
 * @brief Compares if two points are different.
 *
 * @param p Other point to compare against.
 * @return Whether they are different.
 */
bool Point::operator !=(const Point& p) const {
    return x != p.x || y != p.y;
}


/**
 * @brief Adds a number to the coordinates.
 *
 * @param n Number to add to both coordinates.
 * @param p Coordinates to add to.
 * @return The new point.
 */
const Point operator+(float n, const Point& p) {
    return Point(n + p.x, n + p.y);
}



/**
 * @brief Subtracts a number with the coordinates.
 *
 * @param n Number to subtract with both coordinates.
 * @param p Coordinates to subtract with.
 * @return The new point.
 */
const Point operator-(float n, const Point& p) {
    return Point(n - p.x, n - p.y);
}



/**
 * @brief Multiplies a number by the coordinates.
 *
 * @param n Number to multiply with both coordinates.
 * @param p Coordinates to multiply with.
 * @return The new point.
 */
const Point operator*(float n, const Point& p) {
    return Point(n * p.x, n * p.y);
}



/**
 * @brief Divides a number with the coordinates.
 *
 * @param n Number to divide with both coordinates.
 * @param p Coordinates to divide with.
 * @return The new point.
 */
const Point operator/(float n, const Point& p) {
    return Point(n / p.x, n / p.y);
}



/**
 * @brief Returns the vector coordinates of an angle.
 *
 * @param angle The angle.
 * @param magnitude Its magnitude.
 * @return The coordinates.
 */
Point angleToCoordinates(
    float angle, float magnitude
) {
    return
        Point(
            (float) cos(angle) * magnitude,
            (float) sin(angle) * magnitude
        );
}


/**
 * @brief Converts angular distance to linear distance.
 *
 * @param angularDist Angular distance value.
 * @param radius Radius of the circle.
 * @return The linear distance.
 */
float angularDistToLinear(float angularDist, float radius) {
    return (float) (2 * radius * tan(angularDist / 2));
}


/**
 * @brief Checks if two spheres are colliding via a bounding-box check.
 *
 * @param center1 Coordinates of the first sphere.
 * @param center2 Coordinates of the second sphere.
 * @param r Range of the bounding box.
 * @return Whether they are colliding.
 */
bool bBoxCheck(const Point& center1, const Point& center2, float r) {
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
bool bBoxCheck(
    const Point& tl1, const Point& br1,
    const Point& center2, float r
) {
    return
        rectanglesIntersect(
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
 * @param startXY Starting X and Y coordinates.
 * @param startZ Starting Z coordinate.
 * @param targetXY Target destination's X and Y coordinates.
 * @param targetZ Target destination's Z coordinate.
 * @param maxH Maximum height, using the starting Z as the reference.
 * @param gravity Constant for the force of gravity, in units per
 * second squared.
 * @param reqSpeedXY The required X and Y speed is returned here.
 * @param reqSpeedZ The required Z speed is returned here.
 * @param outHAngle If not nullptr, the final horizontal angle is
 * returned here.
 */
void calculateThrow(
    const Point& startXY, float startZ,
    const Point& targetXY, float targetZ,
    float maxH, float gravity,
    Point* reqSpeedXY, float* reqSpeedZ, float* outHAngle
) {

    if(targetZ - startZ > maxH) {
        //If the target is above the maximum height it can be thrown...
        //Then this is an impossible throw.
        *reqSpeedXY = Point();
        *reqSpeedZ = 0.0f;
        if(outHAngle) *outHAngle = 0.0f;
        return;
    }
    
    //Code from https://physics.stackexchange.com/questions/515688
    //First, we calculate stuff in 2D, with horizontal and vertical components
    //only.
    
    //We start with the vertical speed. This will be constant regardless
    //of how far the mob is thrown. In order to reach the required max height,
    //the vertical speed needs to be set thusly:
    *reqSpeedZ = (float) sqrt(2.0 * (-gravity) * maxH);
    
    //Now that we know the vertical speed, we can figure out how long it takes
    //for the mob to land at the target vertical coordinate. The formula for
    //this can be found on Wikipedia, for instance.
    float heightDelta = startZ - targetZ;
    //Because of floating point precision problems, the result of the sqrt
    //could end up negative. Let's cap it to zero.
    float sqrtPart =
        std::max(
            0.0f,
            (float)
            sqrt(
                (*reqSpeedZ) * (*reqSpeedZ) +
                2.0 * (-gravity) * (heightDelta)
            )
        );
    float flightTime = ((*reqSpeedZ) + sqrtPart) / (-gravity);
    
    //Once we know the total flight time, we can divide the horizontal reach
    //by the total time to get the horizontal speed.
    float hAngle, hReach;
    coordinatesToAngle(targetXY - startXY, &hAngle, &hReach);
    
    float hSpeed = hReach / flightTime;
    
    //Now that we know the vertical and horizontal speed, just split the
    //horizontal speed into X and Y 3D world components.
    *reqSpeedXY = angleToCoordinates(hAngle, hSpeed);
    
    //Return the final horizontal angle, if needed.
    if(outHAngle) {
        *outHAngle = hAngle;
    }
}


/**
 * @brief Returns whether a circle is touching a line segment or not.
 *
 * @param circle Coordinates of the circle.
 * @param radius Radius of the circle.
 * @param lineP1 Starting point of the line segment.
 * @param lineP2 Ending point of the line segment.
 * @param outLix If not nullptr, the line intersection's X coordinate is
 * returned here.
 * @param outLiy If not nullptr, the line intersection's Y coordinate is
 * returned here.
 * @return Whether they intersect.
 */
bool circleIntersectsLineSeg(
    const Point& circle, float radius,
    const Point& lineP1, const Point& lineP2,
    float* outLix, float* outLiy
) {

    //Code by
    //  http://www.melloland.com/scripts-and-tutos/
    //  collision-detection-between-circles-and-lines
    
    float vx = lineP2.x - lineP1.x;
    float vy = lineP2.y - lineP1.y;
    float xdiff = lineP1.x - circle.x;
    float ydiff = lineP1.y - circle.y;
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
            float x = lineP1.x + (i * vx * t);
            float y = lineP1.y + (i * vy * t);
            //If one of them is in the boundaries of the segment, it collides.
            if(
                x >= std::min(lineP1.x, lineP2.x) &&
                x <= std::max(lineP1.x, lineP2.x) &&
                y >= std::min(lineP1.y, lineP2.y) &&
                y <= std::max(lineP1.y, lineP2.y)
            ) {
                if(outLix) *outLix = x;
                if(outLiy) *outLiy = y;
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
 * @param rectDim Dimensions of the rectangle.
 * @param rectAngle Angle the rectangle is facing.
 * @param outOverlapDist If not nullptr, the amount of overlap is
 * returned here.
 * @param outRectangleSideAngle If not nullptr, the angle of the side of the
 * rectangle that the circle is on, aligned to the sides of the rectangle, is
 * returned here.
 * @return Whether they intersect.
 */
bool circleIntersectsRectangle(
    const Point& circle, float radius,
    const Point& rectangle, const Point& rectDim,
    float rectAngle,
    float* outOverlapDist, float* outRectangleSideAngle
) {
    Point circleRelPos = circle - rectangle;
    circleRelPos = rotatePoint(circleRelPos, -rectAngle);
    Point nearest;
    
    bool insideX =
        circleRelPos.x > -rectDim.x / 2.0 &&
        circleRelPos.x < rectDim.x / 2.0;
    bool insideY =
        circleRelPos.y > -rectDim.y / 2.0 &&
        circleRelPos.y < rectDim.y / 2.0;
        
    if(insideX && insideY) {
        Point distToPos(
            rectDim.x / 2.0f - circleRelPos.x,
            rectDim.y / 2.0f - circleRelPos.y
        );
        Point distToNeg(
            -(-rectDim.x / 2.0f - circleRelPos.x),
            -(-rectDim.y / 2.0f - circleRelPos.y)
        );
        float smallestX = std::min(distToNeg.x, distToPos.x);
        float smallestY = std::min(distToNeg.y, distToPos.y);
        float smallest = std::min(smallestX, smallestY);
        
        if(smallest == distToPos.x) {
            nearest = Point(rectDim.x / 2, circleRelPos.y);
        } else if(smallest == distToNeg.x) {
            nearest = Point(-rectDim.x / 2, circleRelPos.y);
        } else if(smallest == distToPos.y) {
            nearest = Point(circleRelPos.x, rectDim.y / 2);
        } else if(smallest == distToNeg.y) {
            nearest = Point(circleRelPos.x, -rectDim.y / 2);
        }
    } else {
        nearest =
            Point(
                std::clamp(circleRelPos.x, -rectDim.x / 2.0f, rectDim.x / 2.0f),
                std::clamp(circleRelPos.y, -rectDim.y / 2.0f, rectDim.y / 2.0f)
            );
    }
    
    float d = Distance(circleRelPos, nearest).toFloat();
    if(outOverlapDist) {
        if(insideX && insideY) {
            *outOverlapDist = d + radius;
        } else {
            *outOverlapDist = radius - d;
        }
    }
    
    if(outRectangleSideAngle) {
        float angle;
        if(insideX && insideY) {
            angle = getAngle(circleRelPos, nearest);
        } else {
            angle = getAngle(nearest, circleRelPos);
        }
        
        angle = (float) floor((angle + (TAU / 8)) / (TAU / 4)) * (TAU / 4);
        *outRectangleSideAngle = angle + rectAngle;
    }
    
    if(insideX && insideY) {
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
 * @param outIntersectionTL If not nullptr, and if there is an intersection,
 * return the top-left corner of the intersection here.
 * @param outIntersectionBR If not nullptr, and if there is an intersection,
 * return the bottom-right corner of the intersection here.
 * @return Whether they intersect.
 */
bool collinearLineSegsIntersect(
    const Point& a, const Point& b, const Point& c, const Point& d,
    Point* outIntersectionTL, Point* outIntersectionBR
) {
    Point min1(std::min(a.x, b.x), std::min(a.y, b.y));
    Point max1(std::max(a.x, b.x), std::max(a.y, b.y));
    Point min2(std::min(c.x, d.x), std::min(c.y, d.y));
    Point max2(std::max(c.x, d.x), std::max(c.y, d.y));
    
    Point iTL(std::max(min1.x, min2.x), std::max(min1.y, min2.y));
    Point iBR(std::min(max1.x, max2.x), std::min(max1.y, max2.y));
    
    if(iTL.x == iBR.x && iTL.y == iBR.y) {
        //Special case -- they share just one point. Let it slide.
        return false;
    }
    
    if(iTL.x <= iBR.x && iTL.y <= iBR.y) {
        if(outIntersectionTL) *outIntersectionTL = iTL;
        if(outIntersectionBR) *outIntersectionBR = iBR;
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
void coordinatesToAngle(
    const Point& coordinates, float* angle, float* magnitude
) {
    if(angle) {
        *angle = (float) atan2(coordinates.y, coordinates.x);
    }
    if(magnitude) {
        *magnitude = Distance(Point(0.0f), coordinates).toFloat();
    }
}


/**
 * @brief Converts an angle from degrees to radians.
 *
 * @param deg Angle, in degrees.
 * @return The radians.
 */
float degToRad(float deg) {
    return (float) (M_PI / 180.0f) * deg;
}


/**
 * @brief Returns the dot product between two vectors.
 *
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return The dot product.
 */
float dotProduct(const Point& v1, const Point& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}


/**
 * @brief Returns the angle from the origin and the specified point.
 *
 * @param focus Point that the origin is focusing on.
 * @return The angle.
 */
float getAngle(const Point& focus) {
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
float getAngle(const Point& center, const Point& focus) {
    return (float) atan2(focus.y - center.y, focus.x - center.x);
}


/**
 * @brief Returns the clockwise distance between a1 and a2, in radians.
 *
 * @param a1 First angle.
 * @param a2 Second angle.
 * @return The distance.
 */
float getAngleCwDiff(float a1, float a2) {
    a1 = normalizeAngle(a1);
    a2 = normalizeAngle(a2);
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
float getAngleSmallestDiff(float a1, float a2) {
    return
        (float) M_PI -
        (float) std::abs(
            std::abs(normalizeAngle(a1) - normalizeAngle(a2)) - M_PI
        );
}


/**
 * @brief Returns the closest point in a line segment to a given point.
 *
 * @param l1 Starting point of the line segment.
 * @param l2 Ending point of the line segment.
 * @param p Reference point.
 * @param outSegmentRatio If not nullptr, the ratio from l1 to l2 is
 * returned here. Between 0 and 1, it belongs to the line segment.
 * If not, it doesn't.
 * @return The closest point.
 */
Point getClosestPointInLineSeg(
    const Point& l1, const Point& l2, const Point& p, float* outSegmentRatio
) {

    //Code by http://stackoverflow.com/a/3122532
    
    Point l1ToP = p - l1;
    Point l1ToL2 = l2 - l1;
    
    float l1ToL2Squared =
        l1ToL2.x * l1ToL2.x +
        l1ToL2.y * l1ToL2.y;
        
    float l1ToPDotL1ToL2 =
        l1ToP.x * l1ToL2.x +
        l1ToP.y * l1ToL2.y;
        
    float r = l1ToPDotL1ToL2 / l1ToL2Squared;
    
    if(outSegmentRatio) *outSegmentRatio = r;
    
    return Point(l1.x + l1ToL2.x * r, l1.y + l1ToL2.y * r);
}


/**
 * @brief Returns the closest point in a rotated rectangle's perimeter
 * to the specified point. This only happens if the point is outside the
 * rectangle, otherwise the reference point's coordinates are returned instead.
 *
 * @param p Reference point.
 * @param rectCenter Center of the rectangle.
 * @param rectDim Width and height of the rectangle.
 * @param rectAngle Angle of the rectangle.
 * @param outIsInside If not nullptr, whether or not the reference point
 * is inside the rectangle is returned here.
 * @return The closest point.
 */
Point getClosestPointInRotatedRectangle(
    const Point& p,
    const Point& rectCenter, const Point& rectDim, float rectAngle,
    bool* outIsInside
) {
    Point closestPoint;
    Point perimeter = rectDim / 2.0f;
    if(outIsInside) *outIsInside = false;
    
    //First, transform the coordinates so the rectangle is axis-aligned, and
    //the rectangle's center is at the origin.
    Point deltaP = p - rectCenter;
    deltaP = rotatePoint(deltaP, -rectAngle);
    
    //Check the closest point.
    if(deltaP.x <= -perimeter.x) {
        if(deltaP.y <= -perimeter.y) {
            //Top-left corner.
            closestPoint = Point(-perimeter.x, -perimeter.y);
        } else if(deltaP.y >= perimeter.y) {
            //Bottom-left corner.
            closestPoint = Point(-perimeter.x, perimeter.y);
        } else {
            //Left side.
            closestPoint = Point(-perimeter.x, deltaP.y);
        }
    } else if(deltaP.x >= perimeter.x) {
        if(deltaP.y <= -perimeter.y) {
            //Top-right corner.
            closestPoint = Point(perimeter.x, -perimeter.y);
        } else if(deltaP.y >= perimeter.y) {
            //Bottom-right corner.
            closestPoint = Point(perimeter.x, perimeter.y);
        } else {
            //Right side.
            closestPoint = Point(perimeter.x, deltaP.y);
        }
    } else if(deltaP.y <= -perimeter.y) {
        //Top side.
        closestPoint = Point(deltaP.x, -perimeter.y);
    } else if(deltaP.y >= perimeter.y) {
        //Bottom side.
        closestPoint = Point(deltaP.x, perimeter.y);
    } else {
        //Inside.
        closestPoint = deltaP;
        if(outIsInside) *outIsInside = true;
    }
    
    //Now, transform back.
    closestPoint = rotatePoint(closestPoint, rectAngle);
    return closestPoint + rectCenter;
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
 * @param miterPoint1 The first point is returned here.
 * @param miterPoint2 The second point is returned here.
 * @param maxMiterLength If not 0, the miter is limited to this length.
 */
void getMiterPoints(
    const Point& a, const Point& b, const Point& c, float thickness,
    Point* miterPoint1, Point* miterPoint2, float maxMiterLength
) {
    //https://blog.scottlogic.com/2019/11/18/drawing-lines-with-webgl.html
    
    //Get the miter point's direction.
    Point vecAB = b - a;
    Point vecBC = c - b;
    Point normVecAB = normalizeVector(vecAB);
    Point normVecBC = normalizeVector(vecBC);
    Point tangent = normVecAB + normVecBC;
    Point normTangent = normalizeVector(tangent);
    Point miterDirection(-normTangent.y, normTangent.x);
    
    //Get the miter point's distance.
    Point normalA(-vecAB.y, vecAB.x);
    normalA = normalizeVector(normalA);
    float miterLength =
        (thickness / 2.0f) / dotProduct(miterDirection, normalA);
        
    if(isinf(miterLength)) {
        miterLength = 1.0f;
    }
    if(maxMiterLength > 0.0f && fabs(miterLength) > maxMiterLength) {
        float miterSign = miterLength >= 0.0f ? 1.0f : -1.0f;
        miterLength =
            std::min((float) fabs(miterLength), maxMiterLength);
        miterLength *= miterSign;
    }
    
    //Return the final point.
    *miterPoint1 = b + miterDirection * miterLength;
    *miterPoint2 = b - miterDirection * miterLength;
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
float getPointSign(const Point& p, const Point& lp1, const Point& lp2) {
    return (p.x - lp2.x) * (lp1.y - lp2.y) - (lp1.x - lp2.x) * (p.y - lp2.y);
}


/**
 * @brief Returns a deterministically random point inside of a rectangular
 * ring, with uniform distribution.
 *
 * @param innerDist Width and height of the inner rectangle of the ring.
 * @param outerDist Width and height of the outer rectangle of the ring.
 * @param axisRandomInt A previously-determined random int to
 * calculate the axis with [0, 1].
 * @param axisRandomFloat A previously-determined random float to
 * calculate the axis with [0, 1].
 * @param pxRandomFloat A previously-determined random float to
 * calculate the X coordinate with [0, 1].
 * @param pyRandomFloat A previously-determined random float to
 * calculate the Y coordinate with [0, 1].
 * @param sideRandomInt A previously-determined random int to
 * calculate the side with [0, 1].
 * @return The point.
 */
Point getRandomPointInRectangularRing(
    const Point& innerDist, const Point& outerDist,
    int axisRandomInt, float axisRandomFloat, float pxRandomFloat,
    float pyRandomFloat, int sideRandomInt
) {
    float ringThickness[2] {
        outerDist.x - innerDist.x,
        outerDist.y - innerDist.y
    };
    
    //The idea is to split the ring into four rectangles, organized in a
    //pinwheel pattern.
    //In this pattern, the north and south rectangles have the exact same area,
    //and the same is true for the west and east ones. We can simplify the
    //process with this in mind.
    Point rectSizes[2] = {
        Point(
            ringThickness[0],
            outerDist.y * 2.0f - ringThickness[1]
        ),
        Point(
            outerDist.x * 2.0f - ringThickness[0],
            ringThickness[1]
        )
    };
    float rectAreas[2] = {
        rectSizes[0].x* rectSizes[0].y,
        rectSizes[1].x* rectSizes[1].y
    };
    
    //Pick one of the four rectangles (or in this case, one of the two axes),
    //with weighted probability depending on the area.
    size_t chosenAxis;
    if(rectAreas[0] == 0.0f && rectAreas[1] == 0.0f) {
        chosenAxis = axisRandomInt;
    } else {
        chosenAxis =
            getRandomIdxWithWeights(
                vector<float>(rectAreas, rectAreas + 2),
                axisRandomFloat
            );
    }
    
    Point pInRectangle(
        pxRandomFloat * rectSizes[chosenAxis].x,
        pyRandomFloat * rectSizes[chosenAxis].y
    );
    Point finalP;
    
    if(chosenAxis == 0) {
        //West or east rectangle. Let's assume the east rectangle.
        finalP.x = innerDist.x + pInRectangle.x,
        finalP.y = -outerDist.y + pInRectangle.y;
    } else {
        //North or south rectangle. Let's assume the south rectangle.
        finalP.x = -innerDist.x + pInRectangle.x;
        finalP.y = innerDist.y + pInRectangle.y;
    }
    
    if(sideRandomInt == 0) {
        //Return our point.
        return finalP;
    } else {
        //Swap to the rectangle on the opposite side.
        return Point() - finalP;
    }
}


/**
 * @brief Returns a deterministically random point inside of a circular
 * ring, with uniform distribution.
 *
 * @param innerDist Radius of the inner circle of the ring.
 * @param outerDist Radius of the outer circle of the ring.
 * @param arc Arc of the ring, or M_TAU for the whole ring.
 * @param arcRot Rotation of the arc.
 * @param radiusRandomFloat A previously-determined random float to
 * calculate the radius with [0, 1].
 * @param angleRandomFloat A previously-determined random float to
 * calculate the angle with [0, 1].
 * @return The point.
 */
Point getRandomPointInRing(
    float innerDist, float outerDist,
    float arc, float arcRot,
    float radiusRandomFloat, float angleRandomFloat
) {
    //https://stackoverflow.com/q/30564015
    
    float r =
        innerDist +
        (outerDist - innerDist) * (float) sqrt(radiusRandomFloat);
        
    float theta =
        interpolateNumber(
            angleRandomFloat, 0.0f, 1.0f,
            -arc / 2.0f + arcRot,
            arc / 2.0f + arcRot
        );
        
    return Point(r * (float) cos(theta), r * (float) sin(theta));
}


/**
 * @brief Returns a point inside of a circular ring. Used when you want multiple
 * points inside the ring, evenly distributed. Which of the points this is
 * is defined by the ratio, which is
 * <current point number> / <total number of points>. The distance from the
 * center point is the mid-point of the inner and outer ring.
 *
 * @param innerDist Radius of the inner circle of the ring.
 * @param outerDist Radius of the outer circle of the ring.
 * @param arc Arc of the ring, or M_TAU for the whole ring.
 * @param arcRot Rotation of the arc.
 * @param ratio Ratio of the current point number.
 * @return The point.
 */
Point getRatioPointInRing(
    float innerDist, float outerDist,
    float arc, float arcRot, float ratio
) {

    float radius = (innerDist + outerDist) / 2.0f;
    float angle1 = -arc / 2.0f + arcRot;
    float angle2 = arc / 2.0f + arcRot;
    float finalAngle = (angle2 - angle1) * ratio + angle1;
    
    return angleToCoordinates(finalAngle, radius);
}


/**
 * @brief Gets the bounding box coordinates of a rectangle that has undergone
 * translation, scale, and/or rotation transformations, and places it
 * in the specified point structs.
 *
 * @param center Center point of the rectangle.
 * @param dimensions The rectangle's width and height.
 * @param angle Angle of rotation.
 * @param minCoords The top-left coordinates are returned here.
 * @param maxCoords The bottom-right coordinates are returned here.
 */
void getTransformedRectangleBBox(
    const Point& center, const Point& dimensions, float angle,
    Point* minCoords, Point* maxCoords
) {

    if(!minCoords || !maxCoords) return;
    bool gotMinX = false;
    bool gotMaxX = false;
    bool gotMinY = false;
    bool gotMaxY = false;
    
    for(unsigned char p = 0; p < 4; p++) {
        Point corner, finalCorner;
        
        if(p == 0 || p == 1) corner.x = center.x - (dimensions.x * 0.5f);
        else                 corner.x = center.x + (dimensions.x * 0.5f);
        if(p == 0 || p == 2) corner.y = center.y - (dimensions.y * 0.5f);
        else                 corner.y = center.y + (dimensions.y * 0.5f);
        
        corner -= center;
        finalCorner = rotatePoint(corner, angle);
        finalCorner += center;
        
        if(finalCorner.x < minCoords->x || !gotMinX) {
            minCoords->x = finalCorner.x;
            gotMinX = true;
        }
        if(finalCorner.y < minCoords->y || !gotMinY) {
            minCoords->y = finalCorner.y;
            gotMinY = true;
        }
        if(finalCorner.x > maxCoords->x || !gotMaxX) {
            maxCoords->x = finalCorner.x;
            gotMaxX = true;
        }
        if(finalCorner.y > maxCoords->y || !gotMaxY) {
            maxCoords->y = finalCorner.y;
            gotMaxY = true;
        }
    }
}


/**
 * @brief Returns how much to vertically offset something so that it aligns
 * with either the top, center, or bottom of a box.
 *
 * @param mode Vertical alignment mode.
 * @param height Total height of the box.
 * @return The vertical offset.
 */
float getVerticalAlignOffset(V_ALIGN_MODE mode, float height) {
    return
        mode == V_ALIGN_MODE_BOTTOM ?
        height :
        mode == V_ALIGN_MODE_CENTER ?
        height / 2.0f :
        0.0f;
}


/**
 * @brief Returns the interpolation between two angles, given a number
 * in an interval.
 *
 * @param input The input number.
 * @param inputStart Start of the interval the input number falls on,
 * inclusive. The closer to inputStart, the closer the output is to
 * outputStart.
 * @param inputEnd End of the interval the number falls on, inclusive.
 * @param outputStart Angle on the starting tip of the interpolation.
 * @param outputEnd Angle on the ending tip of the interpolation.
 * @return The interpolated angle.
 */
float interpolateAngle(
    float input, float inputStart, float inputEnd,
    float& outputStart, float& outputEnd
) {
    float angleCwDiff = getAngleCwDiff(outputStart, outputEnd);
    float angleDelta;
    if(angleCwDiff < TAU / 2.0f) {
        angleDelta = angleCwDiff;
    } else {
        angleDelta = -(TAU - angleCwDiff);
    }
    return
        outputStart +
        interpolateNumber(input, inputStart, inputEnd, 0, angleDelta);
}


/**
 * @brief Returns the interpolation between two points, given a number
 * in an interval.
 *
 * @param input The input number.
 * @param inputStart Start of the interval the input number falls on,
 * inclusive. The closer to inputStart, the closer the output is to
 * outputStart.
 * @param inputEnd End of the interval the number falls on, inclusive.
 * @param outputStart Point on the starting tip of the interpolation.
 * @param outputEnd Point on the ending tip of the interpolation.
 * @return The interpolated point.
 */
Point interpolatePoint(
    float input, float inputStart, float inputEnd,
    const Point& outputStart, const Point& outputEnd
) {
    return
        Point(
            interpolateNumber(
                input, inputStart, inputEnd, outputStart.x, outputEnd.x
            ),
            interpolateNumber(
                input, inputStart, inputEnd, outputStart.y, outputEnd.y
            )
        );
        
}


/**
 * @brief Returns whether a point is inside a triangle or not.
 *
 * @param p The point to check.
 * @param rectCenter Center coordinates of the rectangle.
 * @param rectSize Width and height of the rectangle.
 * @return Whether it is inside.
 */
bool isPointInRectangle(
    const Point& p, const Point& rectCenter, const Point& rectSize
) {
    if(p.x < rectCenter.x - rectSize.x / 2.0f) return false;
    if(p.x > rectCenter.x + rectSize.x / 2.0f) return false;
    if(p.y < rectCenter.y - rectSize.y / 2.0f) return false;
    if(p.y > rectCenter.y + rectSize.y / 2.0f) return false;
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
bool isPointInTriangle(
    const Point& p, const Point& tp1, const Point& tp2, const Point& tp3,
    bool loq
) {

    //https://stackoverflow.com/q/2049582
    
    bool b1, b2, b3;
    
    float f1, f2, f3;
    
    f1 = getPointSign(p, tp1, tp2);
    f2 = getPointSign(p, tp2, tp3);
    f3 = getPointSign(p, tp3, tp1);
    
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
bool lineSegIntersectsRectangle(
    const Point& r1, const Point& r2,
    const Point& l1, const Point& l2
) {
    //Line crosses left side?
    if(
        lineSegsIntersect(
            l1, l2, Point(r1.x, r1.y), Point(r1.x, r2.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses right side?
    if(
        lineSegsIntersect(
            l1, l2, Point(r2.x, r1.y), Point(r2.x, r2.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses top side?
    if(
        lineSegsIntersect(
            l1, l2, Point(r1.x, r1.y), Point(r2.x, r1.y), nullptr, nullptr
        )
    ) {
        return true;
    }
    //Line crosses bottom side?
    if(
        lineSegsIntersect(
            l1, l2, Point(r1.x, r2.y), Point(r2.x, r2.y), nullptr, nullptr
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
 * @param rectCenter Center point of the rectangle.
 * @param rectDim Width and height of the rectangle.
 * @param rectAngle Angle of the rectangle.
 * @return Whether they intersect.
 */
bool lineSegIntersectsRotatedRectangle(
    const Point& lp1, const Point& lp2,
    const Point& rectCenter, const Point& rectDim, float rectAngle
) {
    //First, transform the coordinates so the rectangle is axis-aligned, and
    //the rectangle's center is at the origin.
    Point deltaP1 = lp1 - rectCenter;
    deltaP1 = rotatePoint(deltaP1, -rectAngle);
    Point deltaP2 = lp2 - rectCenter;
    deltaP2 = rotatePoint(deltaP2, -rectAngle);
    
    //Now, check if the line intersects the rectangle.
    Point halfDim = rectDim / 2.0f;
    //Right side.
    if(
        lineSegsIntersect(
            deltaP1,
            deltaP2,
            Point(halfDim.x, -halfDim.y),
            Point(halfDim.x, halfDim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Top side.
    if(
        lineSegsIntersect(
            deltaP1,
            deltaP2,
            Point(-halfDim.x, -halfDim.y),
            Point(halfDim.x, -halfDim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Left side.
    if(
        lineSegsIntersect(
            deltaP1,
            deltaP2,
            Point(-halfDim.x, -halfDim.y),
            Point(-halfDim.x, halfDim.y),
            nullptr
        )
    ) {
        return true;
    }
    
    //Bottom side.
    if(
        lineSegsIntersect(
            deltaP1,
            deltaP2,
            Point(-halfDim.x, halfDim.y),
            Point(halfDim.x, halfDim.y),
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
bool lineSegsAreCollinear(
    const Point& a, const Point& b, const Point& c, const Point& d
) {
    return
        pointsAreCollinear(a, b, c) &&
        pointsAreCollinear(a, b, d);
}


/**
 * @brief Returns whether the two line segments intersect.
 *
 * @param l1p1 Starting point of the first line segment.
 * @param l1p2 Ending point of the first line segment.
 * @param l2p1 Starting point of the second line segment.
 * @param l2p2 Ending point of the second line segment.
 * @param outFinalL1r If not nullptr and they intersect, the distance from
 * the start of line 1 in which the intersection happens is returned here.
 * This is a ratio, so 0 is the start, 1 is the end of the line.
 * @param outFinalL2r Same as outFinalL1r, but for line 2.
 * @return Whether they intersect.
 */
bool lineSegsIntersect(
    const Point& l1p1, const Point& l1p2, const Point& l2p1, const Point& l2p2,
    float* outFinalL1r, float* outFinalL2r
) {
    float l1r = 0.0f;
    float l2r = 0.0f;
    bool result = linesIntersect(l1p1, l1p2, l2p1, l2p2, &l1r, &l2r);
    
    if(outFinalL1r) *outFinalL1r = l1r;
    if(outFinalL2r) *outFinalL2r = l2r;
    
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
 * @param outIntersection If not null, return the intersection point here.
 * @return Whether they intersect.
 */
bool lineSegsIntersect(
    const Point& l1p1, const Point& l1p2, const Point& l2p1, const Point& l2p2,
    Point* outIntersection
) {
    float r;
    if(outIntersection) {
        outIntersection->x = 0.0f;
        outIntersection->y = 0.0f;
    }
    if(!lineSegsIntersect(l1p1, l1p2, l2p1, l2p2, &r, nullptr)) return false;
    if(outIntersection) {
        outIntersection->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        outIntersection->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    return true;
}


/**
 * @brief Converts linear distance to angular distance.
 *
 * @param linearDist Linear distance.
 * @param radius Radius of the circle.
 * @return The angular distance.
 */
float linearDistToAngular(float linearDist, float radius) {
    return (float) (2 * atan(linearDist / (2 * radius)));
}


/**
 * @brief Returns whether two lines (not line segments) intersect, and returns
 * information about where it happens.
 *
 * @param l1p1 Point 1 of the first line.
 * @param l1p2 Point 2 of the first line.
 * @param l2p1 Point 1 of the second line.
 * @param l2p2 Point 2 of the second line.
 * @param outL1r If not nullptr and they intersect, returns the distance from
 * the start of line 1 in which the intersection happens.
 * This is a ratio, so 0 is the start, 1 is the end of the line.
 * @param outL2r Same as outL1r, but for line 2.
 * @return Whether they intersect.
 */
bool linesIntersect(
    const Point& l1p1, const Point& l1p2,
    const Point& l2p1, const Point& l2p2,
    float* outL1r, float* outL2r
) {
    float div =
        (l2p2.y - l2p1.y) * (l1p2.x - l1p1.x) -
        (l2p2.x - l2p1.x) * (l1p2.y - l1p1.y);
        
    if(div != 0.0f) {
        //They intersect.
        
        if(outL1r) {
            //Calculate the intersection distance from the start of line 1.
            *outL1r =
                (
                    (l2p2.x - l2p1.x) * (l1p1.y - l2p1.y) -
                    (l2p2.y - l2p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        if(outL2r) {
            //Calculate the intersection distance from the start of line 2.
            *outL2r =
                (
                    (l1p2.x - l1p1.x) * (l1p1.y - l2p1.y) -
                    (l1p2.y - l1p1.y) * (l1p1.x - l2p1.x)
                ) / div;
        }
        
        return true;
        
    } else {
        //They don't intersect.
        
        if(outL1r) *outL1r = 0.0f;
        if(outL2r) *outL2r = 0.0f;
        
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
 * @param outPoint If not nullptr and they intersect,
 * the coordinates of where it happens is returned here.
 * @return Whether they intersect.
 */
bool linesIntersect(
    const Point& l1p1, const Point& l1p2,
    const Point& l2p1, const Point& l2p2,
    Point* outPoint
) {
    if(outPoint) {
        outPoint->x = 0.0f;
        outPoint->y = 0.0f;
    }
    
    float r = 0.0f;
    if(!linesIntersect(l1p1, l1p2, l2p1, l2p2, &r, nullptr)) {
        return false;
    }
    
    if(outPoint) {
        outPoint->x = l1p1.x + (l1p2.x - l1p1.x) * r;
        outPoint->y = l1p1.y + (l1p2.y - l1p1.y) * r;
    }
    
    return true;
}


/**
 * @brief Returns the movement necessary to move a point.
 *
 * @param start Coordinates of the initial point.
 * @param target Coordinates of the target point.
 * @param speed Speed at which the point can move.
 * @param reachRadius If the point is within this range of the target,
 * consider it as already being there.
 * @param mov Variable to return the amount of movement to.
 * @param angle Variable to return the angle the point faces to.
 * @param reached Variable to return whether the point reached the target.
 * @param deltaT How long the frame's tick is, in seconds.
 */
void movePoint(
    const Point& start, const Point& target,
    float speed, float reachRadius,
    Point* mov, float* angle, bool* reached, float deltaT
) {
    Point diff = target - start;
    float dis = (float) sqrt(diff.x * diff.x + diff.y * diff.y);
    
    if(dis > reachRadius) {
        float moveAmount =
            (float) std::min((double) (dis / deltaT / 2.0f), (double) speed);
            
        diff *= (moveAmount / dis);
        
        if(mov) *mov = diff;
        if(angle) *angle = (float) atan2(diff.y, diff.x);
        if(reached) *reached = false;
        
    } else {
    
        if(mov) *mov = Point();
        if(reached) *reached = true;
    }
}


/**
 * @brief Normalizes an angle so that it's between 0 and TAU (M_PI * 2).
 *
 * @param a Angle to normalize.
 * @return The normalized angle.
 */
float normalizeAngle(float a) {
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
Point normalizeVector(const Point& v) {
    float length = Distance(Point(), v).toFloat();
    if(length == 0.0f) return v;
    return
        Point(
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
string p2s(const Point& p, const float* z) {
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
bool pointsAreCollinear(
    const Point& a, const Point& b, const Point& c
) {
    //https://math.stackexchange.com/a/405981
    return
        (b.y - a.y) * (c.x - b.x) ==
        (c.y - b.y) * (b.x - a.x);
}


/**
 * @brief Projects a set of vertexes onto an axis.
 *
 * @param v Vertexes to project.
 * @param axis The axis to project onto.
 * @param min The smallest value of all the vertexes.
 * @param max The largest value of all the vertexes.
 */
void projectVertexes(
    const vector<Point>& v, const Point axis, float* min, float* max
) {
    for(size_t i = 0; i < v.size(); i++) {
        Point p = v[i];
        float proj = dotProduct(p, axis);
        
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
float radToDeg(float rad) {
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
bool rectanglesIntersect(
    const Point& tl1, const Point& br1,
    const Point& tl2, const Point& br2
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
 * @param rectDim1 Dimensions of the first rectangle.
 * @param rectAngle1 Angle the first rectangle is facing.
 * @param rect2 Center coordinates of the second rectangle.
 * @param rectDim2 Dimensions of the second rectangle.
 * @param rectAngle2 Angle the second rectangle is facing.
 * @param outOverlapDist If not nullptr, the amount of overlap is
 * returned here.
 * @param outOverlapAngle If not nullptr, the direction that rectangle 1 would
 * push rectangle 2 away with is returned here.
 * @return Whether they intersect.
 */
bool rectanglesIntersect(
    const Point& rect1, const Point& rectDim1, float rectAngle1,
    const Point& rect2, const Point& rectDim2, float rectAngle2,
    float* outOverlapDist, float* outOverlapAngle
) {
    //Start by getting the vertexes of the rectangles.
    Point tl(-rectDim1.x / 2.0f, -rectDim1.y / 2.0f);
    Point br(rectDim1.x / 2.0f, rectDim1.y / 2.0f);
    vector<Point> rect1Vertexes {
        rotatePoint(tl, rectAngle1) + rect1,
        rotatePoint(Point(tl.x, br.y), rectAngle1) + rect1,
        rotatePoint(br, rectAngle1) + rect1,
        rotatePoint(Point(br.x, tl.y), rectAngle1) + rect1
    };
    
    tl = Point(-rectDim2.x / 2, -rectDim2.y / 2);
    br = Point(rectDim2.x / 2, rectDim2.y / 2);
    
    vector<Point> rect2Vertexes {
        rotatePoint(tl, rectAngle2) + rect2,
        rotatePoint(Point(tl.x, br.y), rectAngle2) + rect2,
        rotatePoint(br, rectAngle2) + rect2,
        rotatePoint(Point(br.x, tl.y), rectAngle2) + rect2
    };
    
    //Code from https://www.youtube.com/watch?v=SUyG3aV
    //(Polygon Collision Resolution)
    //https://www.youtube.com/watch?v=Zgf1DYrmSnk
    //(Separating Axis Theorem).
    
    Point normal(0, 0);
    float minOverlap = INFINITY;
    
    vector<Point> shape1 = rect1Vertexes;
    
    for(int s = 0; s < 2; s++) {
        if(s == 1) {
            shape1 = rect2Vertexes;
        }
        
        //We only need to test the first two edges,
        //since the other two are parallel.
        for(int e = 0; e < 2; e++) {
            Point a = shape1[e];
            Point b = shape1[(e + 1) % 4];
            
            Point edge = b - a;
            Point axis(-edge.y, edge.x);
            
            float min1 = INFINITY;
            float max1 = -INFINITY;
            float min2 = INFINITY;
            float max2 = -INFINITY;
            
            //Project each vertex onto the axis.
            projectVertexes(rect1Vertexes, axis, &min1, &max1);
            projectVertexes(rect2Vertexes, axis, &min2, &max2);
            
            if(min1 >= max2 || min2 >= max1) {
                //We found an opening, there can't be a collision.
                return false;
            }
            
            float curOverlap = std::min(max1 - min2, max2 - min1);
            if(curOverlap < minOverlap) {
                minOverlap = curOverlap;
                normal = axis;
            }
        }
    }
    
    //The size of the axis results in a much bigger overlap,
    //so we correct it here.
    minOverlap /= Distance(Point(0.0f), normal).toFloat();
    
    //Ensure the normal is facing outwards.
    Point dir = rect2 - rect1;
    if(dotProduct(dir, normal) > 0) {
        normal *= -1;
    }
    
    if(outOverlapDist) {
        *outOverlapDist = minOverlap;
    }
    if(outOverlapAngle) {
        *outOverlapAngle = getAngle(Point(0.0f), normal);
    }
    
    return true;
}


/**
 * @brief Resizes a pair of size-related coordinates such that they fit the
 * specified "box" size as snuggly as possible, whilst keeping their original
 * aspect ratio.
 *
 * @param originalSize The original size coordinates.
 * @param boxSize Width and height of the box to fit into.
 * @return The resized dimensions.
 */
Point resizeToBoxKeepingAspectRatio(
    const Point& originalSize,
    const Point& boxSize
) {
    if(originalSize.y == 0.0f || boxSize.y == 0.0f) return Point();
    float originalAspectRatio = originalSize.x / originalSize.y;
    float boxAspectRatio = boxSize.x / boxSize.y;
    if(boxAspectRatio > originalAspectRatio) {
        return
            Point(
                originalSize.x * boxSize.y / originalSize.y,
                boxSize.y
            );
    } else {
        return
            Point(
                boxSize.x,
                originalSize.y * boxSize.x / originalSize.x
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
Point rotatePoint(const Point& coords, float angle) {
    float c = (float) cos(angle);
    float s = (float) sin(angle);
    return Point(c * coords.x - s * coords.y, s * coords.x + c * coords.y);
}


/**
 * @brief Converts a string to a point.
 *
 * @param s String to convert.
 * @param outZ If not nullptr, the third word is returned here.
 * @return The (X and Y) coordinates.
 */
Point s2p(const string& s, float* outZ) {
    vector<string> words = split(s);
    Point p;
    if(words.size() >= 1) {
        p.x = (float) s2f(words[0]);
    }
    if(words.size() >= 2) {
        p.y = (float) s2f(words[1]);
    }
    if(outZ && words.size() >= 3) {
        *outZ = (float) s2f(words[2]);
    }
    return p;
}


/**
 * @brief Scales a rectangle so that it fits as much of the box as possible,
 * based on a number of settings. If any of the settings cannot be respected,
 * a scale of 1,1 will be returned, even if that goes against the box.
 *
 * @param rectSize Width and height of the rectangle to scale.
 * @param boxSize Box width and height.
 * @param canGrowX Whether it's possible to increase the width.
 * @param canGrowY Whether it's possible to increase the height.
 * @param canShrinkX Whether it's possible to decrease the width.
 * @param canShrinkY Whether it's possible to decrease the height.
 * @param canChangeRatio Whether it's possible to change the aspect ratio
 * of the rectangle.
 * @return The scale factor for X and for Y.
 */
Point scaleRectangleToBox(
    const Point& rectSize, const Point& boxSize,
    bool canGrowX, bool canGrowY,
    bool canShrinkX, bool canShrinkY,
    bool canChangeRatio
) {
    Point finalScale(1.0f, 1.0f);
    
    if(
        rectSize.x == 0.0f || rectSize.y == 0.0f ||
        boxSize.x == 0.0f || boxSize.y == 0.0f
    ) {
        return finalScale;
    }
    
    Point boxToUse =
        canChangeRatio ?
        boxSize :
        resizeToBoxKeepingAspectRatio(rectSize, boxSize);
    bool canScaleX =
        (rectSize.x < boxToUse.x && canGrowX) ||
        (rectSize.x > boxToUse.x && canShrinkX);
    bool canScaleY =
        (rectSize.y < boxToUse.y && canGrowY) ||
        (rectSize.y > boxToUse.y && canShrinkY);
        
    if(canChangeRatio) {
        if(canScaleX) finalScale.x = boxToUse.x / rectSize.x;
        if(canScaleY) finalScale.y = boxToUse.y / rectSize.y;
    } else {
        if(canScaleX && canScaleY) {
            finalScale.x = boxToUse.x / rectSize.x;
            finalScale.y = boxToUse.y / rectSize.y;
        }
    }
    
    return finalScale;
}


/**
 * @brief Given a list of items, chooses which item comes next
 * geometrically in the specified direction. Useful for menus with
 * several buttons the player can select multidirectionally in.
 * Also, it loops around.
 *
 * @param itemCoordinates Vector with the center coordinates of all items.
 * @param selectedItem Index of the selected item.
 * @param direction Angle specifying the direction.
 * @param loopRegion Width and height of the loop region.
 * @return The next item's index in the list.
 */
size_t selectNextItemDirectionally(
    const vector<Point>& itemCoordinates, size_t selectedItem,
    float direction, const Point& loopRegion
) {
    const float MIN_BLINDSPOT_ANGLE = (float) (TAU * 0.17f);
    const float MAX_BLINDSPOT_ANGLE = (float) (TAU * 0.33f);
    
    float normalizedDir = normalizeAngle(direction);
    const Point& selCoords = itemCoordinates[selectedItem];
    float bestScore = FLT_MAX;
    size_t bestItem = selectedItem;
    
    //Check each item that isn't the current one.
    for(size_t i = 0; i < itemCoordinates.size(); i++) {
    
        if(i == selectedItem) continue;
        
        Point iBaseCoords = itemCoordinates[i];
        
        //Get the standard coordinates for this item, and make them relative.
        Point iCoords = iBaseCoords;
        iCoords = iCoords - selCoords;
        
        //Rotate the coordinates such that the specified direction
        //lands to the right.
        iCoords = rotatePoint(iCoords, -normalizedDir);
        
        //Check if it's between the blind spot angles.
        //We get the same result whether the Y is positive or negative,
        //so let's simplify things and make it positive.
        float relAngle =
            getAngle(Point(iCoords.x, (float) fabs(iCoords.y)));
        if(
            relAngle >= MIN_BLINDSPOT_ANGLE &&
            relAngle <= MAX_BLINDSPOT_ANGLE
        ) {
            //If so, never let this item be chosen, no matter what. This is
            //useful to stop a list of items with no vertical variance from
            //picking another item when the direction is up, for instance.
            continue;
        }
        
        if(iCoords.x > 0.0f) {
            //If this item is in front of the selected one,
            //give it a score like normal.
            float score = iCoords.x + (float) fabs(iCoords.y);
            if(score < bestScore) {
                bestScore = score;
                bestItem = i;
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
                    iCoords = iBaseCoords;
                    iCoords.x += loopRegion.x * c;
                    iCoords.y += loopRegion.y * r;
                    iCoords = iCoords - selCoords;
                    
                    //Rotate the coordinates such that the specified direction
                    //lands to the right.
                    iCoords = rotatePoint(iCoords, -normalizedDir);
                    
                    //If these coordinates are behind the selected item,
                    //they cannot be selected.
                    if(iCoords.x < 0.0f) {
                        continue;
                    }
                    
                    //Finally, figure out if this is the new best item.
                    float score = iCoords.x + (float) fabs(iCoords.y);
                    if(score < bestScore) {
                        bestScore = score;
                        bestItem = i;
                    }
                }
            }
        }
    }
    
    return bestItem;
}


/**
 * @brief Given new coordinates, updates the maximum coordinates record,
 * if the new coordinates are a new maximum in either axis.
 * Each axis is processed separately.
 *
 * @param maxCoords Maximum coordinates so far.
 * @param newCoords New coordinates to process and, if necessary, update with.
 */
void updateMaxCoords(Point& maxCoords, const Point& newCoords) {
    maxCoords.x =
        std::max(maxCoords.x, newCoords.x);
    maxCoords.y =
        std::max(maxCoords.y, newCoords.y);
}


/**
 * @brief Given new coordinates, updates the minimum coordinates record,
 * if the new coordinates are a new minimum in either axis.
 * Each axis is processed separately.
 *
 * @param minCoords Minimum coordinates so far.
 * @param newCoords New coordinates to process and, if necessary, update with.
 */
void updateMinCoords(Point& minCoords, const Point& newCoords) {
    minCoords.x =
        std::min(minCoords.x, newCoords.x);
    minCoords.y =
        std::min(minCoords.y, newCoords.y);
}


/**
 * @brief Given new coordinates, updates the minimum coordinates record
 * and maximum coordinates record, if the new coordinates are a new
 * minimum or maximum in either axis. Each axis is processed separately.
 *
 * @param minCoords Minimum coordinates so far.
 * @param maxCoords Maximum coordinates so far.
 * @param newCoords New coordinates to process and, if necessary, update with.
 */
void updateMinMaxCoords(
    Point& minCoords, Point& maxCoords, const Point& newCoords
) {
    updateMinCoords(minCoords, newCoords);
    updateMaxCoords(maxCoords, newCoords);
}
