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

    //--- Public members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    
    //--- Public function declarations ---
    
    Point(float x, float y);
    Point(float xy);
    Point();
    const Point operator +(const Point& p) const;
    const Point operator -(const Point& p) const;
    const Point operator *(const Point& p) const;
    const Point operator /(const Point& p) const;
    const Point operator +(float n) const;
    const Point operator -(float n) const;
    const Point operator *(float m) const;
    const Point operator /(float n) const;
    Point operator +=(const Point& p);
    Point operator -=(const Point& p);
    Point operator *=(const Point& p);
    Point operator /=(const Point& p);
    Point operator +=(float n);
    Point operator -=(float n);
    Point operator *=(float n);
    Point operator /=(float n);
    bool operator ==(const Point& p) const;
    bool operator !=(const Point& p) const;
};


const Point operator+(float n, const Point& p);
const Point operator-(float n, const Point& p);
const Point operator*(float n, const Point& p);
const Point operator/(float n, const Point& p);


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

    //--- Public function declarations ---
    
    Distance(const Point& p1, const Point& p2);
    explicit Distance(float d = 0.0f);
    Distance& operator =(float d);
    bool operator <(float d2) const;
    bool operator <(const Distance& d2) const;
    bool operator <=(float d2) const;
    bool operator <=(const Distance& d2) const;
    bool operator >(float d2) const;
    bool operator >(const Distance& d2) const;
    bool operator >=(float d2) const;
    bool operator >=(const Distance& d2) const;
    bool operator ==(float d2) const;
    bool operator ==(const Distance& d2) const;
    bool operator !=(float d2) const;
    bool operator !=(const Distance& d2) const;
    void operator +=(float d2);
    void operator +=(const Distance& d2);
    void operator -=(float d2);
    void operator -=(const Distance& d2);
    float toFloat();
    
    
    private:
    
    //--- Private members ---
    
    //Distance squared. Most operations are based on this number.
    float distanceSquared = 0.0f;
    
    //Square root of the distance squared. Only used if necessary.
    float normalDistance = 0.0f;
    
    //Do we know the normal distance?
    bool hasNormalDistance = false;
    
};


/**
 * @brief Basic data about a 2D geometrical affine transformation:
 * translation, scale, and rotation.
 *
 * Essentially the same as Pose2d, but separated for code clarity,
 * based on needs.
 */
struct Transform2d {

    //--- Public members ---
    
    //Translation.
    Point trans;
    
    //Scale.
    Point scale = Point(1.0f);
    
    //Rotation.
    float rot = 0.0f;
    
};


/**
 * @brief Basic data about an element's position, size, and orientation in 2D
 * space.
 *
 * Essentially the same as Transform2d, but separated for code clarity,
 * based on needs.
 */
struct Pose2d {

    //--- Public members ---
    
    //Position.
    Point pos;
    
    //Size.
    Point size;
    
    //Angle.
    float angle = 0.0f;
    
};


/**
 * @brief A rectangle, defined by a center point and a size.
 */
struct Rect {

    //--- Public members ---
    
    //Center point.
    Point center;
    
    //Width and height.
    Point size;
    
    
    //--- Public function declarations ---
    
    Rect(const Point& center = Point(), const Point& size = Point());
    
};


/**
 * @brief A rectangle, defined by its top-left corner (minimum coordinates)
 * and its bottom-right corner (maximum coordinates).
 */
struct RectCorners {

    //--- Public members ---
    
    //Top-left corner coordinates.
    Point tl;
    
    //Bottom-right coordinates.
    Point br;
    
    
    //--- Public function declarations ---
    
    RectCorners(const Point& tl = Point(), const Point& br = Point());
    
    
    //--- Public misc. definitions ---
    
    //A RectCorners with the values ready for when you need to search for
    //the top-leftmost and bottom-rightmost item in a series of items.
    static const RectCorners readyForSearch;
    
};



Point angleToCoordinates(float angle, float magnitude);
float angularDistToLinear(float angularDist, float radius);
bool bBoxCheck(const Point& center1, const Point& center2, float r);
bool bBoxCheck(const RectCorners& rect, const Point& circleCenter, float r);
void calculateThrow(
    const Point& startXY, float startZ,
    const Point& targetXY, float targetZ,
    float maxH, float gravity,
    Point* outSpeedXY, float* outSpeedZ, float* outHAngle
);
Rect combineBBoxes(const Rect& bBox1, const Rect& bBox2);
bool circleIntersectsLineSeg(
    const Point& circle, float cr,
    const Point& lineP1, const Point& lineP2,
    float* outLix = nullptr, float* outLiy = nullptr
);
bool circleIntersectsRectangle(
    const Point& circle, float cr,
    const Point& rectangle, const Point& rectDim,
    float rectAngle,
    float* outOverlapDist = nullptr, float* outRectangleSideAngle = nullptr
);
bool collinearLineSegsIntersect(
    const Point& a, const Point& b, const Point& c, const Point& d,
    RectCorners* outIntersection = nullptr
);
void coordinatesToAngle(
    const Point& coordinates, float* angle, float* magnitude
);
float degToRad(float rad);
float dotProduct(const Point& v1, const Point& v2);
float expSmoothingAngle(
    float currentAngle, float targetAngle,
    float smoothingFactor, float deltaT
);
float getAngle(const Point& focus);
float getAngle(const Point& center, const Point& focus);
float getAngleCwDiff(float a1, float a2);
float getAngleSmallestDiff(float a1, float a2);
Point getClosestPointInLineSeg(
    const Point& l1, const Point& l2, const Point& p,
    float* outSegmentRatio = nullptr
);
Point getClosestPointInRotatedRectangle(
    const Point& point, const Rect& rect, float rectAngle,
    bool* outIsInside
);
void getMiterPoints(
    const Point& a, const Point& b, const Point& c, float thickness,
    Point* miterPoint1, Point* miterPoint2, float maxMiterLength = 0.0f
);
Point getPointPosRatioInRectangle(const Point& point, const Rect& rect);
float getPointSign(
    const Point& point, const Point& lineSegPoint1, const Point& lineSegPoint2
);
Point getRandomPointInRectangularRing(
    const Point& innerDist, const Point& outerDist,
    int axisRandomInt, float axisRandomFloat, float pxRandomFloat,
    float pyRandomFloat, int sideRandomInt
);
Point getRandomPointInRing(
    float innerDist, float outerDist,
    float arc, float arcRot,
    float radiusRandomFloat, float angleRandomFloat
);
Point getRatioPointInRing(
    float innerDist, float outerDist,
    float arc, float arcRot, float ratio
);
RectCorners getTransformedRectangleBBox(const Rect& rect, float angle);
float getVerticalAlignOffset(V_ALIGN_MODE mode, float height);
float inchTowardsAngle(float start, float target, float maxStep);
float interpolateAngle(
    float input, float inputStart, float inputEnd,
    float& outputStart, float& outputEnd
);
Point interpolatePoint(
    float input, float inputStart, float inputEnd,
    const Point& outputStart, const Point& outputEnd
);
bool isPointInRectangle(
    const Point& point, const Rect& rect
);
bool isPointInTriangle(
    const Point& point,
    const Point& triPoint1, const Point& triPoint2, const Point& triPoint3,
    bool lessOrEqualComp
);
float linearDistToAngular(float linearDist, float radius);
bool lineSegIntersectsRectangle(
    const Point& r1, const Point& r2,
    const Point& l1, const Point& l2
);
bool lineSegIntersectsRotatedRectangle(
    const Point& lineSegPoint1, const Point& lineSegPoint2,
    const Rect& rect, float rectAngle
);
bool lineSegsAreCollinear(
    const Point& a, const Point& b, const Point& c, const Point& d
);
bool lineSegsIntersect(
    const Point& l1p1, const Point& l1p2, const Point& l2p1, const Point& l2p2,
    float* outFinalL1r, float* outFinalL2r
);
bool lineSegsIntersect(
    const Point& l1p1, const Point& l1p2, const Point& l2p1, const Point& l2p2,
    Point* outIntersection
);
bool linesIntersect(
    const Point& l1p1, const Point& l1p2,
    const Point& l2p1, const Point& l2p2,
    float* outL1r, float* outL2r
);
bool linesIntersect(
    const Point& l1p1, const Point& l1p2,
    const Point& l2p1, const Point& l2p2,
    Point* outPoint
);
void movePoint(
    const Point& start, const Point& target,
    float speed, float reachRadius, Point* mov,
    float* angle, bool* reached, float deltaT
);
float normalizeAngle(float a);
Point normalizeVector(const Point& v);
bool pointsAreCollinear(
    const Point& a, const Point& b, const Point& c
);
void projectVertexes(
    const vector<Point>& v, const Point axis, float* min, float* max
);
string p2s(const Point& p, const float* z = nullptr);
float radToDeg(float deg);
bool rectanglesIntersect(
    const RectCorners& corners1, const RectCorners& corners2
);
bool rectanglesIntersect(
    const Rect& rect1, float rect1Angle,
    const Rect& rect2, float rect2Angle,
    float* outOverlapDist = nullptr, float* outOverlapAngle = nullptr
);
Rect rectCornersToRect(const RectCorners& corners);
RectCorners rectToRectCorners(const Rect& rect);
Point resizeToBoxKeepingAspectRatio(
    const Point& originalSize,
    const Point& boxSize
);
Point rotatePoint(const Point& coords, float angle);
Point s2p(const string& s, float* outZ = nullptr);
Point scaleRectangleToBox(
    const Point& rectSize, const Point& boxSize,
    bool canGrowX, bool canGrowY,
    bool canShrinkX, bool canShrinkY,
    bool canChangeRatio
);
void updateMaxCoords(Point& maxCoords, const Point& newCoords);
void updateMinCoords(Point& minCoords, const Point& newCoords);
void updateMinMaxCoords(
    Point& minCoords, Point& maxCoords, const Point& newCoords
);
void updateMinMaxCoords(
    RectCorners& cornerCoords, const Point& newCoords
);
