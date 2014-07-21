#ifndef INTERSECT_RESULT_H
#define INTERSECT_RESULT_H

#include "Vector.h"
#include "Geometry.h"

class Geometry;

struct IntersectResult
{
    bool      hit;
    Geometry* geometry;
    float     distance;
    Point     position;

    // The normal vector that points to the outside of the object
    Vector    normal;

    IntersectResult()
    {
    }

    IntersectResult(bool hit)
    {
        this->hit = hit; 
    }
};

#endif
