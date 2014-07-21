#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"

class Sphere : public Geometry
{
private:
    Point center;
    float radius;

public:
    Sphere(const Point &center, float radius);
    virtual IntersectResult intersect(Ray &ray);
};

#endif
