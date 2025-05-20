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

    public:
    
    //--- Function declarations ---
    
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
    
    //--- Members ---
    
    //Distance squared. Most operations are based on this number.
    float distanceSquared = 0.0f;
    
    //Square root of the distance squared. Only used if necessary.
    float normalDistance = 0.0f;
    
    //Do we know the normal distance?
    bool hasNormalDistance = false;
    
};



Point angleToCoordinates(
    float angle, float magnitude
);
float angularDistToLinear(float angularDist, float radius);
bool bBoxCheck(const Point& center1, const Point& center2, float r);
bool bBoxCheck(
    const Point& tl1, const Point& br1,
    const Point& center2, float r
);
void calculateThrow(
    const Point& startXY, float startZ,
    const Point& targetXY, float targetZ,
    float maxH, float gravity,
    Point* reqSpeedXY, float* reqSpeedZ, float* outHAngle
);
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
    Point* outIntersectionTL = nullptr, Point* outIntersectionBR = nullptr
);
void coordinatesToAngle(
    const Point& coordinates, float* angle, float* magnitude
);
float degToRad(float rad);
float dotProduct(const Point& v1, const Point& v2);
float getAngle(const Point& focus);
float getAngle(const Point& center, const Point& focus);
float getAngleCwDiff(float a1, float a2);
float getAngleSmallestDiff(float a1, float a2);
Point getClosestPointInLineSeg(
    const Point& l1, const Point& l2, const Point& p,
    float* outSegmentRatio = nullptr
);
Point getClosestPointInRotatedRectangle(
    const Point& p,
    const Point& rectCenter, const Point& rectDim, float rectAngle,
    bool* outIsInside
);
void getMiterPoints(
    const Point& a, const Point& b, const Point& c, float thickness,
    Point* miterPoint1, Point* miterPoint2, float maxMiterLength = 0.0f
);
float getPointSign(
    const Point& p, const Point& lp1, const Point& lp2
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
void getTransformedRectangleBBox(
    const Point& center, const Point& dimensions, float angle,
    Point* minCoords, Point* maxCoords
);
float interpolateAngle(
    float input, float inputStart, float inputEnd,
    float& outputStart, float& outputEnd
);
Point interpolatePoint(
    float input, float inputStart, float inputEnd,
    const Point& outputStart, const Point& outputEnd
);
bool isPointInRectangle(
    const Point& p, const Point& rectCenter, const Point& rectSize
);
bool isPointInTriangle(
    const Point& p, const Point& tp1, const Point& tp2, const Point& tp3,
    bool loq
);
float linearDistToAngular(float linearDist, float radius);
bool lineSegsAreCollinear(
    const Point& a, const Point& b, const Point& c, const Point& d
);
bool lineSegIntersectsRectangle(
    const Point& r1, const Point& r2,
    const Point& l1, const Point& l2
);
bool lineSegIntersectsRotatedRectangle(
    const Point& lp1, const Point& lp2,
    const Point& rectCenter, const Point& rectDim, float rectAngle
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
void updateMaxCoords(Point& maxCoords, const Point& newCoords);
void updateMinCoords(Point& minCoords, const Point& newCoords);
void updateMinMaxCoords(
    Point& minCoords, Point& maxCoords, const Point& newCoords
);
void projectVertexes(
    const vector<Point>& v, const Point axis, float* min, float* max
);
string p2s(const Point& p, const float* z = nullptr);
float radToDeg(float deg);
bool rectanglesIntersect(
    const Point& tl1, const Point& br1,
    const Point& tl2, const Point& br2
);
bool rectanglesIntersect(
    const Point& rect1, const Point& rectDim1,
    float rectAngle1,
    const Point& rect2, const Point& rectDim2,
    float rectAngle2,
    float* outOverlapDist = nullptr, float* outOverlapAngle = nullptr
);
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
size_t selectNextItemDirectionally(
    const vector<Point>& itemCoordinates, size_t selectedItem,
    float direction, const Point& loopRegion
);
float getVerticalAlignOffset(V_ALIGN_MODE mode, float height);
