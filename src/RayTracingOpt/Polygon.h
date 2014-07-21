#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "Point.h"

class Polygon
{
public:
    std::vector<Point> vertices;
    std::vector<int> flags; // attributes associated with the vertices

public:
    bool IsConvex();
};

#endif
