#ifndef PLANE_H
#define PLANE_H

#include "Geometry.h"

class Plane : public Geometry
{
private:
    Vector normal;
    Point position;
    float dist;

public:
    Plane(const Vector& normal, float d);
    virtual IntersectResult intersect(Ray &ray);
};

#endif
