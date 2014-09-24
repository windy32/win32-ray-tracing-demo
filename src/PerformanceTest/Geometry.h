#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Ray.h"
#include "IntersectResult.h"

enum GeometryType { PLANE, TRIANGLE, TUNNEL, SET };

class Geometry
{
public:
    GeometryType type;

public:
    Geometry();
    virtual ~Geometry();
    virtual IntersectResult intersect(Ray &ray) = 0;
};

#endif
