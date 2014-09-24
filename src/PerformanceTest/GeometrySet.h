#ifndef GEOMETRY_SET_H
#define GEOMETRY_SET_H

#include <vector>
#include "Geometry.h"
#include "Matrix.h"

class GeometrySet : public Geometry
{
private:
    std::vector<Geometry *> geometries;

public:
    GeometrySet();
    virtual ~GeometrySet();

    void add(Geometry* geometry);
    Geometry *last();
    void clear();

    virtual IntersectResult intersect(Ray &ray);
};

#endif
