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
    void add(Geometry* geometry);
    Geometry *last();
    bool addStlFile(const char *filename, Ptr<Material> material);
    bool addStlFile(const char *filename, Ptr<Material> material, const Matrix &matrix, const Vector &offset);
    void clear();

    virtual IntersectResult intersect(Ray &ray);
    virtual ~GeometrySet();
};

#endif
