#include "GeometrySet.h"
#include "Triangle.h"
#include <float.h>
#include <stdio.h>

GeometrySet::GeometrySet()
{
    type = GeometryType::SET;
}

GeometrySet::~GeometrySet()
{
    clear();
}

void GeometrySet::add(Geometry* geometry)
{
    geometries.push_back(geometry);
}

Geometry *GeometrySet::last()
{
    if (geometries.size() == 0)
        return NULL;
    return geometries.back();
}

void GeometrySet::clear()
{
    for (unsigned int i = 0; i < geometries.size(); i++)
        delete geometries[i];
    geometries.clear();
}

IntersectResult GeometrySet::intersect(Ray &ray)
{
    float minDistance = FLT_MAX;
    IntersectResult minResult(false);

    for (unsigned int i = 0; i < geometries.size(); i++)
    {
        IntersectResult result = geometries[i]->intersect(ray);
        if (result.hit && (result.distance < minDistance)) 
        {
            minDistance = result.distance;
            minResult = result;
        }
    }
    return minResult;
}
