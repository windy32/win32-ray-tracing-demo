#ifndef GRID_H
#define GRID_H

#include "Point.h"
#include "Ray.h"

class Grid
{
private:
    static const float epsilon;

public:
    Point pos;
    Vector size;

public:
    Grid(const Point &pos, const Vector &size);
    Grid(const Point &near, const Point &far);
    bool contains(const Point &p);
    bool intersect(const Ray &ray, float &entry, float &exit);
};

#endif
