#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Ray.h"
#include "IntersectResult.h"
#include "Material.h"
#include "SmartPointer.h"

class Geometry
{
public:
    Ptr<Material> material;

public:
    Geometry();
    virtual ~Geometry();
    virtual IntersectResult intersect(Ray &ray) = 0;
    void setMaterial(const Ptr<Material> &materail);
};

#endif
